#include <gic_v3.h>
#include <gic_v3_reg.h>
#include <arm64_sysreg.h>
#include <print.h>
#include <smp.h>
#include <cpu.h>
#include <assert.h>
#include <io.h>

#define NR_CPUS                 CONFIG_NR_CPUS
#define GICD_RWP_BITMASK        BIT(31)
#define GICR_RWP_BITMASK        BIT(3)
#define GIC_DEADLINE_MS         (2u)

#define PRIORITY_MASK           (0xff)
#define PRIORITY_IDLE           (0xff)
#define PRIORITY_IRQ_LOWEST     (0xf0)
#define PRIORITY_IRQ_DEFAULT    (0xa0)
#define PRIORITY_IRQ_HIGHEST    (0x80) /* Higher for Secure-World */

static struct gic_dist_map          *gicd_map;
static struct gic_rdist_map         *gicr_rd_map[NR_CPUS];
static struct gic_rdist_sgi_ppi_map *gicr_sgi_map[NR_CPUS];

static inline bool gicd_map_ok(struct gic_dist_map *gicd)
{
    /* See gicv3-architecture-specification for more details */
    return ((ioread32(&gicd->iidr) & 0xff) == 0x3b) ? true : false;
}

static inline bool gicr_map_ok(int coreid)
{
    // uint64_t typer = ioread32(&gicr_rd_map[coreid]->typer);
    // uint32_t affinity = read_sysreg32(MPIDR_EL1) & (BIT(31)-1);
    // printf("affinity: 0x%lx\n", affinity);
    // return (((typer >> 32) & (BIT(32)-1)) == affinity) 
    //         ? true : false;
    return true;
}

/* Wait for completion of a distributor change */
static uint32_t gicv3_do_wait_for_rwp(volatile uint32_t *ctlr_addr, uint32_t rwp_bitmask)
{
    uint32_t ret;
    uint64_t ddl_cnt, cnt;
    bool done = false;

    /* Check the value before reading the generic timer */
    if (!(*ctlr_addr & rwp_bitmask)) return 0;

    /* the granularity is count */
    ddl_cnt = GIC_DEADLINE_MS << 30;
    cnt = 0;
    while (done == false) {
        if (cnt++ >= ddl_cnt) {
            ret = 1;
            done = true;

        } else if (!(*ctlr_addr & rwp_bitmask)) {
            ret = 0;
            done = true;
        }
    }
    return ret;
}

static void gicv3_dist_wait_for_rwp(void)
{
    gicv3_do_wait_for_rwp(&gicd_map->ctlr, GICD_RWP_BITMASK);
}
static void gicv3_redist_wait_for_rwp(int coreid)
{
    gicv3_do_wait_for_rwp(&gicr_rd_map[coreid]->ctlr, GICR_RWP_BITMASK);
}


/* 使能系统寄存器访问接口，可以通过系统寄存器形式访问 */
static inline void gicv3_enable_sre(void)
{
    uint32_t val = read_sysreg32(ICC_SRE_EL1);

    val |= (1 << 0);
    write_sysreg32(val, ICC_SRE_EL1);
    isb();
}

void gicd_gicr_map()
{
    /**
     * do pagetable map in kern_map_const_region(),
     * 所以这里仅仅给指针赋值即可.
     */

    gicd_map = (struct gic_dist_map *)CONFIG_GICD_BASE;
    for (u64 i = 0; i < NR_CPUS; i++) {
      gicr_rd_map[i] = (struct gic_rdist_map *)
                          (CONFIG_GICR_BASE + i*CONFIG_GICR_IO_SIZE_PERCPU);
      gicr_sgi_map[i] = (struct gic_rdist_sgi_ppi_map *)
                          ((void *)gicr_rd_map[i] + CONFIG_GICR_RD_SIZE);
    }
}

static void gicc_init(void) 
{
    uint64_t ctlr = 0;

    /* Enable system registers */
    gicv3_enable_sre();

    /* Try to set no priority grouping, but actually ICC_BPR1_EL1 has
       minimum value. See gicv3 specification */
    write_sysreg32(0, ICC_BPR1_EL1);
    /* Set priority mask register: ICC_PMR_EL1 */
    write_sysreg32(PRIORITY_MASK, ICC_PMR_EL1);

    /* EOI drops priority and deactivates the interrupt at same time */
    ctlr = read_sysreg64(ICC_CTLR_EL1);
    ctlr &= ~(1 << 1);
    write_sysreg64(ctlr, ICC_CTLR_EL1);

    /* Enable Group1 interrupts. Otherwise interrupts are always pending */
    write_sysreg32(1, ICC_IGRPEN1_EL1);
    isb();
}

static void gicd_init(void)
{
    uint32_t priority, nr_lines;
    uint32_t typer, i;
    uint64_t affinity;

    assert(cpu_id() == 0);
    assert(gicd_map_ok(gicd_map));

    /* Disable GIC Distributor */
    iowrite32(0, &gicd_map->ctlr);
    gicv3_dist_wait_for_rwp();

    /* Calculate MAX SPI number */
    typer = ioread32(&gicd_map->typer);
    nr_lines = 32 * ((typer&0x1f) + 1);
    printf("Max spi number: %d\n", nr_lines);

    /* default all SPIs to level, active low */
    for (i = SPI_MIN; i < nr_lines; i += 16) 
        iowrite32(0, &gicd_map->icfgrn[(i / 16)]);

    /* Default priority for SPIs */
    priority = (PRIORITY_IRQ_DEFAULT << 24 | PRIORITY_IRQ_DEFAULT << 16 
               |PRIORITY_IRQ_DEFAULT << 8  | PRIORITY_IRQ_DEFAULT);
    for (i = SPI_MIN; i < nr_lines; i += 4) {
        iowrite32(priority, &gicd_map->ipriorityrn[(i / 4)]);
    }

    /* Disable and clear all SPIs */
    for (i = SPI_MIN; i < nr_lines; i += 32) {
        iowrite32(~0, &gicd_map->icenablern[(i / 32)]);
        iowrite32(~0, &gicd_map->icpendrn[(i / 32)]);
    }
    gicv3_dist_wait_for_rwp();

    /* Set all SPIs as Group1 */
    for (i = 0; i <= 31; i++) {
        iowrite32(~0, &gicd_map->igrouprn[i]);
    }

    /* Route all SPIs to main core */
    affinity = cpuid_to_affinity(0);
    for (i = SPI_MIN; i < nr_lines; i++) {
        iowrite64(affinity, &gicd_map->iroutern[i]);
    };  
	gicv3_dist_wait_for_rwp();

	/* enable the gicd */
	iowrite32(GICD_CTLR_ENABLE_GRP1 | GICD_CTLR_ENABLE_GRP1A |
		         GICD_CTLR_ARE_NS, &gicd_map->ctlr);
	isb();
}

/* Distributor manages SGIs and PPIs  */
static void  gicr_init(int cpuid) {
    uint32_t priority;
    int i;

    assert(gicr_map_ok(cpuid));

    /* Deactivation is needed when saving or 
       restoring GIC configuration */
    iowrite32(~0, &gicr_sgi_map[cpuid]->icactiver0);
    
    /* Default priority for PPIs and SGIs */
    priority = (PRIORITY_IRQ_DEFAULT << 24 | PRIORITY_IRQ_DEFAULT << 16 
               |PRIORITY_IRQ_DEFAULT << 8  | PRIORITY_IRQ_DEFAULT);
    for (i = SGI_MIN; i < PPI_MAX; i += 4) {
        iowrite32(priority, &gicr_sgi_map[cpuid]->ipriorityrn[i / 4]);
    }

    /* disable all PPI and enable all SGI */
    iowrite32(0xffff0000, &gicr_sgi_map[cpuid]->icenabler0);
    iowrite32(0x0000ffff, &gicr_sgi_map[cpuid]->isenabler0);

	/* configure SGI and PPI as non-secure Group-1 */
	iowrite32(0xffffffff, &gicr_sgi_map[cpuid]->igroupr0); // 可能在安全上有问题

    /* Set PPIs to level-triggered, SGIs are  keeping default */
    iowrite32(0, &gicr_sgi_map[cpuid]->icfgr1);
    iowrite32(0, &gicr_sgi_map[cpuid]->icfgr0);

    gicv3_redist_wait_for_rwp(cpuid);
}


void gicv3_init() 
{
    gicd_gicr_map();

    gicd_init();
    gicr_init(cpu_id());
    gicc_init();
    printf("Init done\n");
}


void gicv3_secondary_init() 
{
    gicr_init(cpu_id());
    gicc_init();
    printf("Init done\n");
}


void gicv3_irq_enable(u32 irq)
{
  u32 n, x;
  if (irq < SPI_MIN) {
    iowrite32(BIT(irq), &gicr_sgi_map[cpu_id()]->isenabler0);
    gicv3_redist_wait_for_rwp(cpu_id());
  }
  else {
    n = irq / 32;
    x = irq % 32;
    iowrite32(BIT(x), &gicd_map->isenablern[n]);
    gicv3_dist_wait_for_rwp();
  }
}

void gicv3_irq_disable(u32 irq)
{
  u32 n, x;
  if (irq < SPI_MIN) {
    iowrite32(BIT(irq), &gicr_sgi_map[cpu_id()]->icenabler0);
    gicv3_redist_wait_for_rwp(cpu_id());
  }
  else {
    n = irq / 32;
    x = irq % 32;
    iowrite32(BIT(x), &gicd_map->icenablern[n]);
    gicv3_dist_wait_for_rwp();
  }
}
    
void gicv3_send_sgi(u32 sgi)
{
    assert(0); // TODO
    u64 val_cluster0 = 1; // CORE 0

    val_cluster0 |= (sgi << 24);
    write_sysreg64(val_cluster0, ICC_SGI1R_EL1);
}

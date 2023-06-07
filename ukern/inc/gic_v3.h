#pragma once
#include <config/config.h>
#include <types.h>



#define SGI_MIN           (   0)
#define SGI_MAX           (  15)
#define PPI_MAX           (  16)
#define PPI_LAST          (  31)
#define SPI_MIN           (  32)
#define SPI_MAX           (1019)
/* Special IRQ's */
#define SPECIAL_IRQ_START (1020)
#define IRQ_SPURIOUS      (1023)


/* Memory map for GIC distributor */
struct gic_dist_map {
  uint32_t ctlr;             /* 0x0000 */
  uint32_t typer;            /* 0x0004 */
  uint32_t iidr;             /* 0x0008 */
  uint32_t res0;             /* 0x000C */
  uint32_t statusr;          /* 0x0010 */
  uint32_t res1[11];         /* [0x0014, 0x0040) */
  uint32_t setspi_nsr;       /* 0x0040 */
  uint32_t res2;             /* 0x0044 */
  uint32_t clrspi_nsr;       /* 0x0048 */
  uint32_t res3;             /* 0x004C */
  uint32_t setspi_sr;        /* 0x0050 */
  uint32_t res4;             /* 0x0054 */
  uint32_t clrspi_sr;        /* 0x0058 */
  uint32_t res5[9];          /* [0x005C, 0x0080) */
  uint32_t igrouprn[32];     /* [0x0080, 0x0100) */

  uint32_t isenablern[32];   /* [0x100, 0x180) */
  uint32_t icenablern[32];   /* [0x180, 0x200) */
  uint32_t ispendrn[32];     /* [0x200, 0x280) */
  uint32_t icpendrn[32];     /* [0x280, 0x300) */
  uint32_t isactivern[32];   /* [0x300, 0x380) */
  uint32_t icactivern[32];   /* [0x380, 0x400) */

  uint32_t ipriorityrn[255]; /* [0x400, 0x7FC) */
  uint32_t res6;             /* 0x7FC */

  uint32_t itargetsrn[254];  /* [0x800, 0xBF8) */
  uint32_t res7[2];          /* 0xBF8 */

  uint32_t icfgrn[64];       /* [0xC00, 0xD00) */
  uint32_t igrpmodrn[64];    /* [0xD00, 0xE00) */
  uint32_t nsacrn[64];       /* [0xE00, 0xF00) */
  uint32_t sgir;             /* 0xF00 */
  uint32_t res8[3];          /* [0xF04, 0xF10) */
  uint32_t cpendsgirn[4];    /* [0xF10, 0xF20) */
  uint32_t spendsgirn[4];    /* [0xF20, 0xF30) */
  uint32_t res9[5235];       /* [0x0F30, 0x6100) */

  uint64_t iroutern[960];    /* [0x6100, 0x7F00) */
};

/* Memory map for GIC Redistributor Registers for control and physical LPI's */
struct gic_rdist_map { /* Starting */
  uint32_t ctlr;       /* 0x0000 */
  uint32_t iidr;       /* 0x0004 */
  uint64_t typer;      /* 0x0008 */
  uint32_t statusr;    /* 0x0010 */
  uint32_t waker;      /* 0x0014 */
  uint32_t res0[10];   /* 0x0018 */
  uint64_t setlpir;    /* 0x0040 */
  uint64_t clrlpir;    /* 0x0048 */
  uint32_t res1[8];    /* 0x0050 */
  uint64_t propbaser;  /* 0x0070 */
  uint64_t pendbaser;  /* 0x0078 */
  uint32_t res2[8];    /* 0x0080 */
  uint64_t invlpir;    /* 0x00a0 */
  uint32_t res3[2];    /* 0x00a8 */
  uint64_t invallr;    /* 0x00b0 */
  uint32_t res4[2];    /* 0x00b8 */
  uint32_t syncr;      /* 0x00c0 */
};

/* Memory map for the GIC Redistributor Registers for the SGI and PPI's */
struct gic_rdist_sgi_ppi_map { /* Starting */
  uint32_t res0[32];           /* 0x0000 */
  uint32_t igroupr0;           /* 0x0080 */
  uint32_t res1[31];           /* 0x0084 */
  uint32_t isenabler0;         /* 0x0100 */
  uint32_t res2[31];           /* 0x0104 */
  uint32_t icenabler0;         /* 0x0180 */
  uint32_t res3[31];           /* 0x0184 */
  uint32_t ispendr0;           /* 0x0200 */
  uint32_t res4[31];           /* 0x0204 */
  uint32_t icpendr0;           /* 0x0280 */
  uint32_t res5[31];           /* 0x0284 */
  uint32_t isactiver0;         /* 0x0300 */
  uint32_t res6[31];           /* 0x0304 */
  uint32_t icactiver0;         /* 0x0380 */
  uint32_t res7[31];           /* 0x0384 */
  uint32_t ipriorityrn[8];     /* 0x0400 */
  uint32_t res8[504];          /* 0x0420 */
  uint32_t icfgr0;             /* 0x0C00 */
  uint32_t icfgr1;             /* 0x0C04 */
  uint32_t res9[62];           /* 0x0C08 */
  uint32_t igrpmodr0;          /* 0x0D00*/
  uint32_t res10[63];          /* 0x0D04 */
  uint32_t nsacr;              /* 0x0E00 */
};


void gicv3_init();
void gicv3_secondary_init();

void gicv3_irq_enable(u32 irq);
void gicv3_irq_disable(u32 irq);
void gicv3_send_sgi(u32 sgi);
#include <kernel.h>
#include <console.h>
#include <mm.h>
#include <gic_v3.h>
#include <cpu.h>
#include <cfi.h>
#include <ramdisk.h>
#include <timer.h>
#include <page.h>
#include <proc.h>
#include <sched2.h>
#include <exec.h>
#include <irq.h>
#include <init.h>



static void 
call_init_func(unsigned long fn_start, unsigned long fn_end)
{
	init_call *fn;
	int size, i;

	size = (fn_end - fn_start) / sizeof(init_call);
	LOG_DEBUG("call init func : 0x%x 0x%x %d\n", fn_start, fn_end, size);

	if (size <= 0)
		return;

	fn = (init_call *)fn_start;
	for (i = 0; i < size; i++) {
		(*fn)();
		fn++;
	}
}

void
init_uspace(void)
{
  call_init_func((unsigned long)&__init_func_5_start,
                  (unsigned long)&__init_func_6_start);
}

void
init_irqhooks(void)
{
  call_init_func((unsigned long)&__init_func_3_start,
                  (unsigned long)&__init_func_4_start);
}



/*
 * root service will create the first user space
 * process, then response for the memory management
 * for all the task
 */
int 
load_root_service(void)
{
    struct proc *proc = 0;
    char *argv[] = {"roots.elf", 0};
    // int ret;

	
    if ((proc = create_root_proc()) == NULL) {
        goto failed;
    }

    if (exec("roots.elf", argv) < 0) {
        goto failed;
    }
    return 0;

failed:
	return -EFAULT;
}

void 
kernel_init(void)
{
    init_console();
    init_mm();
    init_gicv3();
    init_cpus();
    init_sched_timer();
    init_uspace();
    init_irqhooks();
    
    
    if (init_cfi() < 0) {
        LOG_ERROR("Init PFLASH ERROR\n"); 
        goto init_failed;
    }

    if (init_ramdisk() < 0) {
       LOG_ERROR("Init RAMDISK ERROR\n"); 
       goto init_failed;
    }

    
#ifdef UNITTEST_ON
    unittest(); 
#endif

    

    // Kernel component init ok, load No.0 user process
    // Load root service and enter user space
    LOG_INFO("Loading root service...\n");
    if (load_root_service() < 0) {
        LOG_ERROR("Load root service err\n");
        goto init_failed;
    }








    // Start scheduling
    sched_timer_setup(MILLISECS(500));
    cpu_intr_on();
    sched_timer_start();
    while (1);
    

init_failed:
    // Should not be here
    panic("[SPRING-OS end, byebye]\n");
}

#if 0
extern void idle(void);
extern void start_system_task(int cpuid);
void kernel_init()
{
  int cpuid = cpu_id();
  int ret;

  console_init();



  mm_init();

  gicv3_init();

  sched_init();
  sched_local_init();

  percpu_init();
  
  ret = cfi_init();
  if (ret)
    panic("cfi-pflash FAILED!\n");

  ramdisk_copy_from_flash();
  ramdisk_init();

  create_idle_task();
  
  printf("=============\n");
  printf("HI, SPRING-OS\n");
  printf("=============\n");
  // start_system_task(cpuid);

  if (cpuid == 0) {
    procinfo_init();
    LOG_INFO("Load Root Service...");
    extern int load_root_service(void);
    load_root_service();
  }

  // FIXME: 临时放在这
  extern int stdio_kobject_init(void);
  stdio_kobject_init();
    
  // debug
  // DBG_pagetable(kernel_pgd_base());
  // extern int DBG_bitmap(void);
  // DBG_bitmap();

  /* asm ("brk #0"); */
  
  idle();
  panic("[SPRING-OS end, byebye]\n");
}

#endif
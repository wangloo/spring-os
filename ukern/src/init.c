#include <kernel.h>
#include <console.h>
#include <mm.h>
#include <gic_v3.h>
#include <cpu.h>
#include <cfi.h>
#include <ramdisk.h>
#include <timer.h>
// #include <pagetable.h>
// #include <mm.h>
// #include <gic_v3.h>
// #include <sched.h>
// #include <task.h>
// #include <pcpu.h>
// #include <cfi.h>
// #include <ramdisk.h>
// #include <procinfo.h>
#include <proc.h>
#include <exec.h>
#include <irq.h>

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
    int ret;

	
    if ((proc = create_root_proc()) == NULL) {
        goto failed;
    }

    if (exec("roots.elf", argv) < 0) {
        goto failed;
    }
    return 0;

	// proc = create_root_process(NULL, NULL, OS_PRIO_SYSTEM, TASK_AFF_ANY,
	// 		TASK_FLAGS_SRV | TASK_FLAGS_NO_AUTO_START);
	// assert(proc != NULL);
    
    // entry = elf_load_process(proc, &file);
    // assert(entry > 0);

	// arch_set_task_entry_point(proc->root_task, entry);

	// if (setup_root_service_env(proc)) {
	// 	printf("setup root service env failed\n");
	// 	goto failed;
	// }

	// ret = setup_root_service_ustack(proc);
	// if (ret) {
	// 	printf("serup user stack for root service failed\n");
	// 	goto failed;
	// }

	// LOG_INFO("LOAD", "Root service load successfully prepare to run...");

	// return wake_up_process(proc);

failed:
	return -EFAULT;
}

void timer_handler(int intid)
{
    printf("timer!!\n");
    timer_stop();
}

void 
kernel_init(void)
{
    init_console();
    init_mm();
    init_gicv3();
    init_cpus();
    init_timer();
    
    
    if (init_cfi() < 0) {
        LOG_ERROR("INIT", "Init PFLASH ERROR\n"); 
        goto init_failed;
    }

    if (init_ramdisk() < 0) {
       LOG_ERROR("INIT", "Init RAMDISK ERROR\n"); 
       goto init_failed;
    }

    // Kernel component init ok, load No.0 user process
    // Load root service and enter user space
    LOG_INFO("INIT", "Loading root service...\n");
    if (load_root_service() < 0) {
        LOG_ERROR("INIT", "Load root service err\n");
    }

    // Start scheduling
    timer_setup(MILLISECS(500));
    if (irq_register(INTID_VTIMER, timer_handler, "timer") < 0)
        goto init_failed;
    cpu_intr_on();
    timer_start();
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
    LOG_INFO("INIT", "Load Root Service...");
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
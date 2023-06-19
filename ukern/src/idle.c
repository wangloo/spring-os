#include <pcpu.h>
#include <irq.h>


// 每个CPU都会执行到此
void idle(void)
{
	struct pcpu *pcpu = get_pcpu(cpu_id());

	set_os_running();
	pstate_irq_enable();

	extern void DBG_sched_newlist(struct pcpu *pcpu);
	extern void DBG_sched_readylist(struct pcpu *pcpu);
	DBG_sched_newlist(pcpu);
	DBG_sched_readylist(pcpu);
	pcpu_irqwork(pcpu->pcpu_id);
	// while (1);

	// while (1) {
	// 	sched();

	// 	/*
	// 	 * need to check whether the pcpu can go to idle
	// 	 * state to avoid the interrupt happend before wfi
	// 	 */
	// 	while (!need_resched() && pcpu_can_idle(pcpu)) {
	// 		local_irq_disable();
	// 		if (pcpu_can_idle(pcpu)) {
	// 			pcpu->state = PCPU_STATE_IDLE;
	// 			wfi();
	// 			nop();
	// 			pcpu->state = PCPU_STATE_RUNNING;
	// 		}
	// 		local_irq_enable();
	// 	}
	// }
}
#include <pcpu.h>
#include <current.h>


int kworker_task(void *data)
{
	struct pcpu *pcpu = get_pcpu(cpu_id());
	// flag_t flag;

	pcpu->kworker = current();
	// flag_init(&pcpu->kworker_flag, 0);

	for (;;) {
		// flag = flag_pend(&pcpu->kworker_flag, KWORKER_FLAG_MASK,
		// 		FLAG_WAIT_SET_ANY | FLAG_CONSUME, 0);
		// if (flag == 0) {
		// 	pr_err("kworker: no event trigger\n");
		// 	continue;
		// }

		// if (flag & KWORKER_TASK_RECYCLE)
		// 	pcpu_release_task(pcpu);
	}

	return 0;
}
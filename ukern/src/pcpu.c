#include <pcpu.h>
#include <arm64_pcpu.h>
#include <kmem.h>

struct pcpu pcpus[NR_CPUS];

struct pcpu *get_pcpu(int cpuid)
{
  assert(cpuid < NR_CPUS);
  return &pcpus[cpuid];
}

static void set_percpu_pcpu(struct pcpu *pcpu)
{
  arch_set_percpu_pcpu(pcpu);
}

void pcpu_sched_init(struct pcpu *pcpu)
{
  INIT_LIST_HEAD(&pcpu->new_list);
  INIT_LIST_HEAD(&pcpu->stop_list);
  INIT_LIST_HEAD(&pcpu->die_process);
  INIT_LIST_HEAD(&pcpu->ready_list[0]);
  INIT_LIST_HEAD(&pcpu->ready_list[1]);
  INIT_LIST_HEAD(&pcpu->ready_list[2]);
  INIT_LIST_HEAD(&pcpu->ready_list[3]);
  INIT_LIST_HEAD(&pcpu->ready_list[4]);
  INIT_LIST_HEAD(&pcpu->ready_list[5]);
  INIT_LIST_HEAD(&pcpu->ready_list[6]);
  INIT_LIST_HEAD(&pcpu->ready_list[7]);
}

void percpu_init(void)
{
  struct pcpu *pcpu = get_pcpu(cpu_id());
  int pages = TASK_STACK_SIZE >> PAGE_SHIFT;

  // put pcpu in TPIDR
  set_percpu_pcpu(pcpu);

  // set pcpu stack
  pcpu->stack = get_free_pages(pages, GFP_KERNEL);
  assert(pcpu->stack);
  pcpu->stack += TASK_STACK_SIZE;
  pcpu->state = PCPU_STATE_RUNNING;
}
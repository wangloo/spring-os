#include <pcpu.h>
#include <task.h>
#include <barrier.h>
#include <assert.h>
#include <print.h>

void DBG_sched_readylist(struct pcpu *pcpu)
{
    struct task *task;
    int i;

    printf("[DEBUG] sched readylist:\n");
    for (i = 0; i < OS_PRIO_MAX; i++) {
        printf("ready list[%d]:\n", i);
        list_for_each_entry(task, &pcpu->ready_list[i], state_list) {
            printf("task: %s\n", task->name);
        }
    }
}

void DBG_sched_newlist(struct pcpu *pcpu)
{
    struct task *task;

    printf("[DEBUG] sched newlist:\n");
    list_for_each_entry(task, &pcpu->new_list, state_list) {
        printf("task: %s\n", task->name);
    }
}
/**
 * @file  task_config.h
 * @brief 放置task的配置项
 * @level
 * @date 2023-07-29
 */
#pragma once
#include <config/config.h>
#include <page.h>

#define NR_CPUS                   CONFIG_NR_CPUS

/* 系统最大的task数 */
#define OS_NR_TASKS               CONFIG_NR_TASKS

#define OS_PRIO_MAX               8
#define OS_PRIO_DEFAULT_0         0
#define OS_PRIO_DEFAULT_1         1
#define OS_PRIO_DEFAULT_2         2
#define OS_PRIO_DEFAULT_3         3
#define OS_PRIO_DEFAULT_4         4
#define OS_PRIO_DEFAULT_5         5
#define OS_PRIO_DEFAULT_6         6
#define OS_PRIO_DEFAULT_7         7

#define OS_PRIO_REALTIME          OS_PRIO_DEFAULT_0
#define OS_PRIO_SRV               OS_PRIO_DEFAULT_2
#define OS_PRIO_SYSTEM            OS_PRIO_DEFAULT_3
#define OS_PRIO_VCPU              OS_PRIO_DEFAULT_4
#define OS_PRIO_DEFAULT           OS_PRIO_DEFAULT_5
#define OS_PRIO_IDLE              OS_PRIO_DEFAULT_7
#define OS_PRIO_LOWEST            OS_PRIO_IDLE

#define TASK_FLAGS_SRV            BIT(0) // should not change, need keep same as pangu
#define TASK_FLAGS_DRV            BIT(1)
#define TASK_FLAGS_VCPU           BIT(2)
#define TASK_FLAGS_REALTIME       BIT(3)
#define TASK_FLAGS_IDLE           BIT(4)
#define TASK_FLAGS_NO_AUTO_START  BIT(5)
#define TASK_FLAGS_32BIT          BIT(6)
#define TASK_FLAGS_PERCPU         BIT(7)
#define TASK_FLAGS_DEDICATED_HEAP BIT(8)
#define TASK_FLAGS_ROOT           BIT(9)
#define TASK_FLAGS_KERNEL         BIT(10)

#define TASK_STATE_RUNNING        0x00
#define TASK_STATE_READY          0x01
#define TASK_STATE_WAIT_EVENT     0x02
#define TASK_STATE_WAKING         0x04
#define TASK_STATE_SUSPEND        0x08
#define TASK_STATE_STOP           0x10

/* task运行时间片 */
#ifdef CONFIG_TASK_RUN_TIME
#define TASK_RUN_TIME CONFIG_TASK_RUN_TIME
#else
#define TASK_RUN_TIME 50
#endif

/* task栈空间大小 */
#ifdef CONFIG_TASK_STACK_SIZE
#define TASK_STACK_SIZE CONFIG_TASK_STACK_SIZE
#else
#define TASK_STACK_SIZE (2 * PAGE_SIZE)
#endif

#define TASK_AFF_ANY   (-1)
#define TASK_NAME_SIZE (32)

#define TASK_STATE_PEND_OK    0u /* Pending status OK, not pending, or pending complete */
#define TASK_STATE_PEND_TO    1u /* Pending timed out */
#define TASK_STATE_PEND_ABORT 2u /* Pending aborted */

#define TASK_WAIT_FOREVER     (0xfffffffe)

#pragma once

#include <types.h>
#include <compiler.h>

struct kobject;

struct handle_desc {
  struct kobject *kobj;
  int right;
  int padding;
} __packed;


struct handle_table_desc {
  uint32_t index;
  uint32_t left;
  struct handle_desc *next;
} __packed;

#define HANDLE_NULL      (-1)

#define NR_DESC_PER_PAGE (PAGE_SIZE / sizeof(struct handle_desc) - 1)
#define PROC_MAX_HANDLE  (NR_DESC_PER_PAGE * 128)

#define WRONG_HANDLE(handle)                                                   \
  ((handle == HANDLE_NULL) || (handle >= PROC_MAX_HANDLE))

void __release_handle(struct process *proc, handle_t handle);
int release_handle(handle_t handle, struct kobject **kobj, right_t *right);

handle_t __alloc_handle(struct process *proc, struct kobject *kobj,
                        right_t right);

handle_t alloc_handle(struct kobject *kobj, right_t right);

int get_kobject_from_process(struct process *proc, handle_t handle,
                             struct kobject **kobj, right_t *right);

int get_kobject(handle_t handle, struct kobject **kobj, right_t *right);

int put_kobject(struct kobject *kobj);

void release_proc_kobjects(struct process *proc);

void process_handles_deinit(struct process *proc);

int init_proc_handles(struct process *proc);

handle_t send_handle(struct process *psrc, struct process *pdst,
                     handle_t handle, right_t right_send);


#define KOBJ_RIGHT_NONE    0x0000    // do not have any right.
#define KOBJ_RIGHT_READ    0x0001    // can read this kobject, usually for IPC between two process.
#define KOBJ_RIGHT_WRITE   0x0002    // can write this kobject, usually for IPC between two process.
#define KOBJ_RIGHT_EXEC    0x0004    // can be exectued.
#define KOBJ_RIGHT_MMAP    0x0008    // can be mapped to process address space
#define KOBJ_RIGHT_CTL     0x0010    // can call kobject_ctl for this kobject
#define KOBJ_RIGHT_MASK    0x001f

#define KOBJ_RIGHT_RW     (KOBJ_RIGHT_READ | KOBJ_RIGHT_WRITE)
#define KOBJ_RIGHT_RO     (KOBJ_RIGHT_READ)
#define KOBJ_RIGHT_WO     (KOBJ_RIGHT_WRITE)
#define KOBJ_RIGHT_RWX    (KOBJ_RIGHT_RW | KOBJ_RIGHT_EXEC)

enum {
  KOBJ_TYPE_NONE,
  KOBJ_TYPE_PROCESS,  // process, can be only created by root service
  KOBJ_TYPE_NOTIFY,  // a port, which is a service hub
  KOBJ_TYPE_PMA,    // physical memory region, usually used to shared with each other.  
  KOBJ_TYPE_ENDPOINT,  // endpoint, an point to point ipc way
  KOBJ_TYPE_SOCKET,  // point to point ipc way.
  KOBJ_TYPE_VM,    // virtual machine, for Virtualization
  KOBJ_TYPE_VCPU,    // vcpu for vm
  KOBJ_TYPE_IRQ,    // irq for user-space driver
  KOBJ_TYPE_VIRQ,    // virq for vcpu process in user-space.
  KOBJ_TYPE_STDIO,  // dedicated for system debuging
  KOBJ_TYPE_POLLHUB,  // hub for events need to send.
  KOBJ_TYPE_PORT,
  KOBJ_TYPE_MAX
};

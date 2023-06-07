#pragma once

#define __VM_IO			(0x00000001)	/* IO memory */
#define __VM_NORMAL		(0x00000002)	/* Normal memory */
#define __VM_NORMAL_NC		(0x00000004)
#define __VM_WT			(0x00000008)	/* Write thought */

#define __VM_PFNMAP		(0x00000100)	/* map to the physical normal memory 
                                       directly */
#define __VM_HUGE_2M		(0x00000200)
#define __VM_HUGE_1G		(0x00000400)
#define __VM_DEVMAP		(0x00000800)
#define __VM_KERN		(0x00002000)
#define __VM_USER		(0x00004000)
#define __VM_SHARED		(0x00001000)	/* do not release the memory, 
                                       kobject will release it */
#define __VM_SHMEM		(0x00008000)	/* prviate memory, will not be shared */
#define __VM_PMA		(0x00010000)

#define __VM_RW_NON     (0x00000000)
#define __VM_READ       (0x00100000)
#define __VM_WRITE      (0x00200000)
#define __VM_EXEC       (0x00400000)
#define __VM_RO         (__VM_READ)
#define __VM_WO         (__VM_WRITE)
#define __VM_RW         (__VM_READ | __VM_WRITE)

#define VM_TYPE_MASK		(__VM_IO | __VM_NORMAL | __VM_NORMAL_NC | __VM_WT)

#define VM_KERN         (__VM_KERN)
#define VM_USER         (__VM_USER)

#define VM_RO           (__VM_RO)
#define VM_WO           (__VM_WO)
#define VM_RW           (__VM_READ | __VM_WRITE)
#define VM_RWX          (__VM_READ | __VM_WRITE | __VM_EXEC)
#define VM_RW_MASK      (__VM_READ | __VM_WRITE)

#define VM_IO           (__VM_IO | __VM_DEVMAP | __VM_PFNMAP)
#define VM_NORMAL       (__VM_NORMAL)
#define VM_NORMAL_NC    (__VM_NORMAL_NC)
#define VM_NORMAL_WT    (__VM_WT)
#define VM_DMA          (__VM_NORMAL_NC)
#define VM_HUGE         (__VM_HUGE_2M)
#define VM_SHARED       (__VM_SHARED)
#define VM_SHMEM        (__VM_SHMEM)
#define VM_PFNMAP       (__VM_PFNMAP)
#define VM_DEVMAP       (__VM_DEVMAP)
#define VM_PMA			(__VM_PMA)


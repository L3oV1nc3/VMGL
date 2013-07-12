/****************************************************************************
 * Copyright (c) 2006-2007 H. Andres Lagar-Cavilla                                                                          *
 * Modifications to the ati fglrx kernel module open source wrapper made
 * in the context of the vmgl project                                                                         *
****************************************************************************/
/****************************************************************************
 *                                                                          *
 * Copyright 1999-2005 ATI Technologies Inc., Markham, Ontario, CANADA.     *
 * All Rights Reserved.                                                     *
 *                                                                          *
 * Your use and or redistribution of this software in source and \ or       *
 * binary form, with or without modification, is subject to: (i) your       *
 * ongoing acceptance of and compliance with the terms and conditions of    *
 * the ATI Technologies Inc. software End User License Agreement; and (ii)  *
 * your inclusion of this notice in any version of this software that you   *
 * use or redistribute.  A copy of the ATI Technologies Inc. software End   *
 * User License Agreement is included with this software and is also        *
 * available by contacting ATI Technologies Inc. at http://www.ati.com      *
 *                                                                          *
 ****************************************************************************/

#ifndef _FIREGL_PUBLIC_H_
#define _FIREGL_PUBLIC_H_

#define FGL_DEVICE_SIGNATURE    0x10020000
#define FGL_DEBUG_SIGNATURE     "fglrx"

#define __KE_NO_VSPRINTF

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,71) 
#define _agp_memory             agp_memory 
#endif 

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)

#ifdef CONFIG_XEN
#define REMAP_PAGE_RANGE_FN io_remap_pfn_range
#define REMAP_PAGE_RANGE_STR "io_remap_pfn_range"

#else
#define REMAP_PAGE_RANGE_FN remap_pfn_range
#define REMAP_PAGE_RANGE_STR "remap_pfn_range"
#endif

#define REMAP_PAGE_RANGE_OFF(offset) ((offset) >> PAGE_SHIFT)

#else /* LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,9) */

#define REMAP_PAGE_RANGE_FN remap_page_range
#define REMAP_PAGE_RANGE_STR "remap_page_range"
#define REMAP_PAGE_RANGE_OFF(offset) (offset)

#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9) */

#ifdef CONFIG_XEN
#define REMAP_PAGE_RANGE(vma,offset) \
    REMAP_PAGE_RANGE_FN((vma), \
                        (vma)->vm_start,	\
                        REMAP_PAGE_RANGE_OFF(offset), \
                        (vma)->vm_end - (vma)->vm_start, \
                        (vma)->vm_page_prot)
#else
#define REMAP_PAGE_RANGE(vma,offset) \
    REMAP_PAGE_RANGE_FN(FGL_VMA_API_PASS \
                        (vma)->vm_start,	\
                        REMAP_PAGE_RANGE_OFF(offset), \
                        (vma)->vm_end - (vma)->vm_start, \
                        (vma)->vm_page_prot)
#endif

/* Page table macros */

#define PGD_OFFSET(mm, pgd_p, pte_linear)	\
do { \
    pgd_p = pgd_offset(mm, pte_linear); \
} while(0)

#define PGD_OFFSET_K(pgd_p, pte_linear)	\
do { \
    pgd_p = pgd_offset_k(pte_linear); \
} while(0)

#define PGD_PRESENT(pgd_p) \
do { \
    if (!pgd_present(*(pgd_p)))	\
    { \
        __KE_ERROR("FATAL ERROR: User queue buffer not present! (pgd)\n"); \
        return (unsigned long)NOPAGE_SIGBUS;   /* Something bad happened; generate SIGBUS */ \
        /* alternatively we could generate a NOPAGE_OOM "out of memory" */ \
    } \
} while(0)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)

#define PMD_OFFSET(pmd_p, pgd_p, pte_linear) \
do { \
    pud_t *pud_p = pud_offset(pgd_p, pte_linear); \
    if (!pud_present(*(pud_p)))	\
    { \
        __KE_ERROR("FATAL ERROR: User queue buffer not present! (pud)\n"); \
        return (unsigned long)NOPAGE_SIGBUS;   /* Something bad happened; generate SIGBUS */ \
        /* alternatively we could generate a NOPAGE_OOM "out of memory" */ \
    } \
    pmd_p = pmd_offset(pud_p, pte_linear); \
} while(0)

#else /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11) */

#define PMD_OFFSET(pmd_p, pgd_p, pte_linear)	\
do { \
    pmd_p = pmd_offset(pgd_p, pte_linear); \
} while(0)

#endif

#define PMD_PRESENT(pmd_p) \
do { \
    if (!pmd_present(*(pmd_p)))	\
    { \
        __KE_ERROR("FATAL ERROR: User queue buffer not present! (pmd)\n"); \
        return (unsigned long)NOPAGE_SIGBUS;   /* Something bad happened; generate SIGBUS */ \
        /* alternatively we could generate a NOPAGE_OOM "out of memory" */ \
    } \
} while(0)

#ifdef pte_offset_atomic
#define PTE_OFFSET(pte, pmd_p, pte_linear) \
do { \
    pte_t* pte_p; \
    pte_p = pte_offset_atomic(pmd_p, pte_linear); \
    pte = *pte_p; \
    pte_kunmap(pte_p); \
} while(0)
#else
#ifdef pte_offset_map
#define PTE_OFFSET(pte, pmd_p, pte_linear) \
do { \
    pte_t* pte_p; \
    pte_p = pte_offset_map(pmd_p, pte_linear); \
    pte = *pte_p; \
    pte_unmap(pte_p); \
} while(0)
#else
#ifdef pte_offset_kernel
#define PTE_OFFSET(pte, pmd_p, pte_linear) \
do { \
    pte_t* pte_p; \
    pte_p = pte_offset_kernel(pmd_p, pte_linear); \
    pte = *pte_p; \
} while(0)
#else
#define PTE_OFFSET(pte, pmd_p, pte_linear) \
do { \
    pte_t* pte_p; \
    pte_p = pte_offset(pmd_p, pte_linear); \
    pte = *pte_p; \
} while(0)
#endif
#endif
#endif

#define PTE_PRESENT(pte) \
do { \
    if (!pte_present(pte)) \
    { \
        __KE_ERROR("FATAL ERROR: User queue buffer not present! (pte)\n"); \
        return (unsigned long)NOPAGE_SIGBUS;   /* Something bad happened; generate SIGBUS */ \
        /* alternatively we could generate a NOPAGE_OOM "out of memory" */ \
    } \
} while(0)

#ifdef pfn_to_page
#define PMD_PAGE(pmd) pmd_page(pmd)
#else /* for old 2.4 kernels */
/* on 2.6 x86_64 kernel, pfn_to_page may not be defined in some cases (like with NUMA option on). 
   This is intended, we should not redefine it to break the build.
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#define pfn_to_page(pfn) (mem_map + (pfn))
#endif
#define PMD_PAGE(pmd) (pfn_to_page(pmd_val(pmd) >> PAGE_SHIFT))
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#if !defined(CONFIG_SMP) || defined(CONFIG_SUSPEND_SMP) // ACPI not working on older SMP kernel (prior to 2.6.13) 
#define FIREGL_POWER_MANAGEMENT
#endif
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14)
#define PMSG_EVENT(pmsg_state) (pmsg_state).event
#else
#define PMSG_EVENT(pmsg_state) (pmsg_state)
/* For old kernels without PM_EVENT_xxx defines, define them 
   in consistent with the power state used in these kernels.
 */
#define PM_EVENT_ON 0
#define PM_EVENT_SUSPEND 3
#define PM_EVENT_FREEZE 3
#endif

#if defined(CONFIG_COMPAT) && LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
#define FIREGL_IOCTL_COMPAT
#endif

/*****************************************************************************/

struct inode;
struct file;
struct vm_area_struct;
struct wait_queue;
struct semaphore;
struct _agp_memory;
struct mm_struct;
struct pci_dev;
struct pci_driver;
struct list_head;
struct pci_bus;
struct drm_device;
struct drm_agp_mem;
struct drm_map;
struct firegl_pcie_mem;
struct firegl_pcie;
struct proc_dir_entry;

#if !defined(ATI_API_CALL)
#define ATI_API_CALL __attribute__((regparm(0)))
#endif

/*****************************************************************************/

typedef unsigned long __ke_dev_t;
typedef unsigned long __ke_size_t;
typedef unsigned long __ke_off_t;
typedef long __ke_ssize_t;
typedef unsigned char __ke_u8;
typedef unsigned short __ke_u16;
typedef unsigned int __ke_u32;
typedef unsigned long long __ke_u64;
typedef unsigned long long __ke_dma_addr_t;
typedef long long __ke_loff_t;

// note: assigning uniqe types to originally non interchangeable types
typedef struct { int uniqe1; } __ke_wait_queue_head_t;
typedef struct { int uniqe2; } __ke_wait_queue_t;
typedef struct { int uniqe3; } __ke_sigset_t;
typedef struct { int uniqe4; } __ke_pci_dev_t;
typedef struct { int uniqe5; } __ke_priv_device_t;
typedef struct { int uniqe6; } __ke_pci_bus_t;
typedef struct { int uniqe7; } __ke_file_operations_t;
typedef struct { int uniqe8; } __ke_poll_table;

typedef	int (*__ke_read_proc_t)(
    char* page, char** start, __ke_off_t off, int count, int* eof, void* data);

typedef struct {
    const char*             name;
    __ke_read_proc_t        f;
    __ke_file_operations_t* fops;
} __ke_proc_list_t;

extern __ke_proc_list_t firegl_proc_list[];

typedef struct {
    unsigned long           signature;
    __ke_priv_device_t *    privdev;
    __ke_proc_list_t *      proclist;
    const char *            name;
	unsigned int	        major_version;
	unsigned int	        minor_version;
	unsigned int            patchlevel;
	const char *	        date;
    __ke_sigset_t *         psigmask;
} __ke_device_t;

typedef struct __ke_pci_device_info
{
    unsigned short	vendor;
	unsigned short	device;
	unsigned short	subsystem_vendor;
	unsigned short	subsystem_device;
} __ke_pci_device_info_t;

/*****************************************************************************/

extern int ATI_API_CALL drm_name_info(char* buf, char** start, __ke_off_t offset, int len, int* eof, void* data);
extern int ATI_API_CALL firegl_bios_version(char* buf, char** start, __ke_off_t offset, int len, int* eof, void* data);
extern int ATI_API_CALL drm_mem_info(char* buf, char** start, __ke_off_t offset, int len, int *eof, void *data);
extern int ATI_API_CALL drm_mem_info1(char* buf, char** start, __ke_off_t offset, int len, int *eof, void *data);
extern int ATI_API_CALL drm_vm_info(char* buf, char** start, __ke_off_t offset, int len, int* eof, void* data);
extern int ATI_API_CALL drm_clients_info(char* buf, char** start, __ke_off_t offset, int len, int* eof, void* data);
extern int ATI_API_CALL firegl_lock_info(char* buf, char** start, __ke_off_t offset, int len, int* eof, void* data);
extern int ATI_API_CALL firegl_umm_info(char* buf, char** start, __ke_off_t offset, int len, int* eof, void* data);
#ifdef DEBUG
extern int ATI_API_CALL drm_bq_info(char* buf, char** start, __ke_off_t offset, int len, int* eof, void* data);
extern int ATI_API_CALL firegl_debug_info(char* buf, char** start, __ke_off_t offset, int len, int* eof, void* data);
#endif
extern int ATI_API_CALL firegl_interrupt_open(struct inode* inode, struct file* file);
extern int ATI_API_CALL firegl_interrupt_release(struct inode* inode, struct file* file);
extern unsigned int ATI_API_CALL firegl_interrupt_read(
                                    struct file *user_file, 
                                    char *user_buf, 
                                    __ke_size_t user_buf_size, 
                                    __ke_loff_t *user_file_pos);
extern unsigned int ATI_API_CALL firegl_interrupt_poll(struct file *user_file, __ke_poll_table *pt);
extern int ATI_API_CALL firegl_interrupt_write(
                                    struct file *user_file, 
                                    char *user_buf, 
                                    __ke_size_t user_buf_size, 
                                    __ke_loff_t *user_file_pos);

/*****************************************************************************/

extern int ATI_API_CALL firegl_init(__ke_device_t*);
extern void ATI_API_CALL firegl_cleanup(__ke_device_t*);
extern int ATI_API_CALL firegl_open(struct inode* inode, struct file* filp);
extern int ATI_API_CALL firegl_release(struct inode* inode, struct file* filp);
extern int ATI_API_CALL firegl_ioctl(struct inode* inode,
                        struct file* filp,
                        unsigned int cmd,
                        unsigned long arg);

#ifdef __x86_64__
extern long ATI_API_CALL firegl_compat_ioctl(
                        struct file* filp,
                        unsigned int cmd,
                        unsigned long arg);
#endif

/*****************************************************************************/

extern int ATI_API_CALL firegl_mmap(struct file* filp, struct vm_area_struct* vma);
extern void ATI_API_CALL drm_vm_open(struct vm_area_struct* vma);
extern void ATI_API_CALL drm_vm_close(struct vm_area_struct* vma);

extern unsigned long ATI_API_CALL firegl_get_virt_agp_mem( void *pt, unsigned long baddr,
										struct vm_area_struct* vma
										);
extern struct drm_agp_mem* ATI_API_CALL firegl_get_agpmem_from_drmmap(struct drm_device* dev,
							struct drm_map *p_drmmap, 
							unsigned long baddr);
extern int ATI_API_CALL firegl_cant_use_agp(void *pt);
extern void* ATI_API_CALL firegl_get_dev_from_vm(  struct vm_area_struct* vma );
extern void* ATI_API_CALL firegl_get_pcie_from_vm(  struct vm_area_struct* vma );
extern void* ATI_API_CALL firegl_get_pciemem_from_addr( struct vm_area_struct* vma, unsigned long addr );
extern unsigned long ATI_API_CALL firegl_get_pcie_pageaddr_from_vm(  struct vm_area_struct* vma, struct firegl_pcie_mem* pciemem, unsigned long offset);
extern void* ATI_API_CALL firegl_get_pagelist_from_vm(  struct vm_area_struct* vma );
extern unsigned long ATI_API_CALL firegl_get_addr_from_vm(  struct vm_area_struct* vma);
extern unsigned long ATI_API_CALL firegl_get_pagetable_page_from_vm(struct vm_area_struct* vma);


/*****************************************************************************/

extern __ke_wait_queue_head_t* ATI_API_CALL __ke_alloc_wait_queue_head_struct(void);
extern void ATI_API_CALL __ke_free_wait_queue_head_struct(__ke_wait_queue_head_t* queue_head);
extern __ke_wait_queue_t* ATI_API_CALL __ke_alloc_wait_queue_struct(void);
extern void ATI_API_CALL __ke_free_wait_queue_struct(__ke_wait_queue_t* queue);

extern void ATI_API_CALL __ke_wait_event_interruptible(__ke_wait_queue_head_t* queue_head, int condition);
extern void ATI_API_CALL __ke_wake_up_interruptible(__ke_wait_queue_head_t* queue_head);
extern void ATI_API_CALL __ke_poll_wait(struct file* filp, __ke_wait_queue_head_t* queue_head, __ke_poll_table* pt);
    
extern void ATI_API_CALL __ke_add_wait_queue(__ke_wait_queue_head_t* queue_head, __ke_wait_queue_t* entry);
extern void ATI_API_CALL __ke_remove_wait_queue(__ke_wait_queue_head_t* queue_head, __ke_wait_queue_t* entry);
extern void ATI_API_CALL __ke_init_waitqueue_head(__ke_wait_queue_head_t* queue_head);

extern void ATI_API_CALL __ke_schedule(void);
extern int ATI_API_CALL __ke_signal_pending(void);
extern void ATI_API_CALL __ke_yield(void);

extern void ATI_API_CALL __ke_set_current_state_task_interruptible(void);
extern void ATI_API_CALL __ke_set_current_state_task_running(void);
extern void ATI_API_CALL __ke_configure_sigmask(__ke_sigset_t *pSigMask);
extern int ATI_API_CALL firegl_sig_notifier(void *priv);
extern int              firegl_sig_notifier_wrap(void *priv);
extern void ATI_API_CALL __ke_block_all_signals(int (*notifier)(void *priv), void *pPriv, __ke_sigset_t *pSigMask);
extern void ATI_API_CALL __ke_unblock_all_signals(void);

extern unsigned long ATI_API_CALL __ke__cmpxchg(volatile void *ptr, unsigned long old,                      
                      unsigned long new, int size);
extern unsigned long ATI_API_CALL __ke_cpu_to_le32(unsigned long _u);
extern unsigned long long ATI_API_CALL __ke_cpu_to_le64(unsigned long long _u);

#define __ke_cmpxchg(ptr,o,n)                        \
  ((__typeof__(*(ptr)))__ke__cmpxchg((ptr),(unsigned long)(o),      \
                 (unsigned long)(n),sizeof(*(ptr))))
/*****************************************************************************/

extern __ke_dev_t ATI_API_CALL __ke_getdevice(__ke_device_t *dev);
extern const char* ATI_API_CALL __ke_module_parm(void);
extern int ATI_API_CALL __ke_inode_rdev_minor(struct inode* inode);
extern void* ATI_API_CALL __ke_get_proc_dir_entry_priv(struct proc_dir_entry* pde);
extern void* ATI_API_CALL __ke_get_inode_priv(struct inode* inode);
extern void* ATI_API_CALL __ke_get_file_priv(struct file* filp);
extern void ATI_API_CALL __ke_set_file_priv(struct file* filp, void* private_data);
extern int ATI_API_CALL __ke_file_excl_open(struct file* filp);
extern int ATI_API_CALL __ke_file_rw_open(struct file* filp);
extern unsigned int ATI_API_CALL __ke_file_counter(struct file* filp);
extern struct inode* ATI_API_CALL __ke_get_file_inode(struct file* filp);
extern int ATI_API_CALL __ke_getpid(void);
extern int ATI_API_CALL __ke_geteuid(void);
extern unsigned long ATI_API_CALL __ke_jiffies(void);
extern void ATI_API_CALL __ke_udelay(unsigned long usecs);
extern void ATI_API_CALL __ke_mdelay(unsigned long msecs);
extern unsigned long ATI_API_CALL __ke_virt_to_bus(void* address);
extern unsigned long ATI_API_CALL __ke_virt_to_phys(void* address);
extern void* ATI_API_CALL __ke_high_memory(void);
#if defined(__x86_64__) || defined(__ia64__)
void* ATI_API_CALL __ke_pci_alloc_consistent(__ke_pci_dev_t* dev, int size, void *dma_handle);
void ATI_API_CALL __ke_pci_free_consistent(__ke_pci_dev_t* dev, int size, unsigned long cpu_addr,
						 unsigned int dma_handle);
#endif


enum __ke_error_num
{
    __KE_EBUSY,
    __KE_EINVAL,
    __KE_EACCES,
    __KE_EFAULT,
    __KE_EIO,
    __KE_EBADSLT,
    __KE_ENOMEM,
    __KE_EPERM,
    __KE_ENODEV,
    __KE_EINTR,
    __KE_ERESTARTSYS,
    __KE_ELIBBAD,
};
extern int ATI_API_CALL __ke_error_code(enum __ke_error_num errcode);

extern void ATI_API_CALL __ke_mod_inc_use_count(void);
extern void ATI_API_CALL __ke_mod_dec_use_count(void);

extern void ATI_API_CALL __ke_down_struct_sem(__ke_device_t *dev, int idx);
extern void ATI_API_CALL __ke_up_struct_sem(__ke_device_t *dev, int idx);
	#define __KE_MAX_SEMAPHORES 2
extern void ATI_API_CALL __ke_sema_init(struct semaphore* sem, int value);
extern __ke_size_t ATI_API_CALL __ke_sema_size(void);
extern void ATI_API_CALL __ke_down(struct semaphore* sem);
extern void ATI_API_CALL __ke_up(struct semaphore* sem);
extern void ATI_API_CALL __ke_atomic_inc(void* v);
extern void ATI_API_CALL __ke_atomic_dec(void* v);
extern void ATI_API_CALL __ke_atomic_add(int val, void* v);
extern void ATI_API_CALL __ke_atomic_sub(int val, void* v);
extern int ATI_API_CALL __ke_atomic_read(void* v);
extern void ATI_API_CALL __ke_atomic_set(void* v, int val);
extern void ATI_API_CALL __ke_spin_lock(__ke_device_t *dev, int ndx);
extern void ATI_API_CALL __ke_spin_unlock(__ke_device_t *dev, int ndx);
	#define __KE_MAX_SPINLOCKS 6
extern void ATI_API_CALL __ke_lock_kernel(void);
extern void ATI_API_CALL __ke_unlock_kernel(void);
extern int ATI_API_CALL __ke_sys_mlock(unsigned long start, __ke_size_t len);
extern int ATI_API_CALL __ke_sys_mlock(unsigned long start, __ke_size_t len);
extern int ATI_API_CALL __ke_sys_munlock(unsigned long start, __ke_size_t len);
extern int ATI_API_CALL __ke_sys_modify_ldt(int func, void *ptr, unsigned long bytecount);
int ATI_API_CALL __ke_vsprintf(char *buf, const char *fmt, va_list ap);
int ATI_API_CALL __ke_vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);
#ifdef __KE_NO_VSPRINTF
extern void ATI_API_CALL __ke_printk(const char* fmt, ...);
#else // !__KE_NO_VSPRINTF
extern void ATI_API_CALL __ke_print_info(const char* fmt, ...);
extern void ATI_API_CALL __ke_print_error(const char* fmt, ...);
extern void ATI_API_CALL __ke_print_debug(const char* fmt, ...);
#endif // !__KE_NO_VSPRINTF

enum __ke_cap
{
    __KE_CAP_SYS_ADMIN,
    __KE_CAP_IPC_LOCK,
};
extern int ATI_API_CALL __ke_capable(enum __ke_cap cap);
extern void ATI_API_CALL __ke_cap_effective_raise(enum __ke_cap cap);
extern __ke_u32 ATI_API_CALL __ke_get_cap_effective(void);
extern void ATI_API_CALL __ke_set_cap_effective(__ke_u32 cap);
extern unsigned long ATI_API_CALL __ke_ram_available(void);

#ifdef __x86_64__
extern void* ATI_API_CALL __ke_compat_alloc_user_space(long size);
#endif
extern int ATI_API_CALL __ke_copy_from_user(void* to, const void* from, __ke_size_t size);
extern int ATI_API_CALL __ke_copy_to_user(void* to, const void* from, __ke_size_t size);
extern int ATI_API_CALL __ke_verify_area(int type, const void * addr, unsigned long size);

extern void* ATI_API_CALL __ke_malloc(__ke_size_t size);
extern void* ATI_API_CALL __ke_malloc_atomic(__ke_size_t size);
extern void ATI_API_CALL __ke_free(void* p);
extern void ATI_API_CALL __ke_free_s(void* p, __ke_size_t size);
extern void* ATI_API_CALL __ke_vmalloc(__ke_size_t size);
extern void* ATI_API_CALL __ke_vmalloc_32(__ke_size_t size);
extern void* ATI_API_CALL __ke_vmalloc_atomic(__ke_size_t size);
extern void ATI_API_CALL __ke_vfree(void* p);
extern void* ATI_API_CALL __ke_get_free_page(void);
extern void* ATI_API_CALL __ke_get_free_pages(int order);
extern void ATI_API_CALL __ke_free_page(void* pt);
extern void ATI_API_CALL __ke_free_pages(void* pt, int order);
extern void ATI_API_CALL __ke_get_page(void* pt);
extern void ATI_API_CALL __ke_put_page(void* pt);
extern void ATI_API_CALL __ke_lock_page(void* pt);
extern void ATI_API_CALL __ke_unlock_page(void *virt);
extern int ATI_API_CALL __ke_PageCompound(void *virt);
extern void ATI_API_CALL __ke_mem_map_reserve(void* pt);
extern void ATI_API_CALL __ke_mem_map_unreserve(void* pt);
extern void ATI_API_CALL __ke_virt_reserve(void* pt);
extern void ATI_API_CALL __ke_virt_unreserve(void* pt);
extern void* ATI_API_CALL __ke_get_vmptr( struct _agp_memory* memory );
extern void* ATI_API_CALL __ke_ioremap(unsigned long offset, unsigned long size);
extern void* ATI_API_CALL __ke_ioremap_nocache(unsigned long offset, unsigned long size);
extern void ATI_API_CALL __ke_iounmap(void* pt);
extern int ATI_API_CALL __ke_verify_read_access(void* addr, __ke_size_t size);
extern int ATI_API_CALL __ke_verify_write_access(void* addr, __ke_size_t size);
extern struct ATI_API_CALL mm_struct* ATI_API_CALL __ke_init_mm(void);
extern struct ATI_API_CALL mm_struct* ATI_API_CALL __ke_current_mm(void);
extern __ke_dma_addr_t ATI_API_CALL __ke_get_vm_phys_addr(struct mm_struct* mm, unsigned long virtual_addr);
extern unsigned long ATI_API_CALL __ke_get_vm_page_table(struct mm_struct* mm, unsigned long virtual_addr, unsigned long *page_addr, int* is_highpte);
extern void ATI_API_CALL __ke_put_vm_page_table(unsigned long page_addr);
extern __ke_dma_addr_t* ATI_API_CALL __ke_get_vm_phys_addr_list(struct mm_struct* mm, unsigned long virtual_addr, unsigned long pages);
extern void* ATI_API_CALL __ke_memset(void* s, int c, __ke_size_t count);
extern void* ATI_API_CALL __ke_memcpy(void* d, const void* s, __ke_size_t count);
extern ATI_API_CALL __ke_size_t __ke_strlen(const char *s);
extern char* ATI_API_CALL __ke_strcpy(char* d, const char* s);
extern char* ATI_API_CALL __ke_strncpy(char* d, const char* s, __ke_size_t count);
extern int ATI_API_CALL __ke_strcmp(const char *s1, const char *s2);
extern int ATI_API_CALL __ke_strncmp(const char* str1, const char* str2, __ke_size_t count);
extern char* ATI_API_CALL __ke_strchr(const char *s, int c);
extern int ATI_API_CALL __ke_sprintf(char* buf, const char* fmt, ...);
extern int ATI_API_CALL __ke_snprintf(char* buf, size_t size, const char* fmt, ...);
extern int ATI_API_CALL __ke_vm_test_and_clear_dirty(struct mm_struct* mm, unsigned long virtual_addr);
extern void* ATI_API_CALL __ke_vmap(unsigned long *pagelist, unsigned int count);
extern void ATI_API_CALL __ke_vunmap(void* addr);

/*****************************************************************************/

extern void ATI_API_CALL __ke_set_bit(int nr, volatile void * addr);
extern void ATI_API_CALL __ke_clear_bit(int nr, volatile void * addr);
extern void ATI_API_CALL __ke_change_bit(int nr, volatile void* addr);
extern int ATI_API_CALL __ke_test_bit(int nr, volatile void* addr);
extern int ATI_API_CALL __ke_test_and_set_bit(int nr, volatile void* addr);
extern int ATI_API_CALL __ke_test_and_clear_bit(int nr, volatile void* addr);
extern int ATI_API_CALL __ke_test_and_change_bit(int nr, volatile void* addr);

/*****************************************************************************/

extern int ATI_API_CALL __ke_flush_cache(void);

/*****************************************************************************/

extern int ATI_API_CALL __ke_config_mtrr(void);
extern int ATI_API_CALL __ke_mtrr_add_wc(unsigned long base, unsigned long size);
extern int ATI_API_CALL __ke_mtrr_del(int reg, unsigned long base, unsigned long size);
extern int ATI_API_CALL __ke_has_vmap(void);

#ifdef __x86_64__
extern int ATI_API_CALL __ke_config_iommu(void);
extern int ATI_API_CALL __ke_no_iommu(void);
#endif

/*****************************************************************************/

extern int ATI_API_CALL __ke_get_pci_device_info(__ke_pci_dev_t* dev, __ke_pci_device_info_t *pinfo);
extern int ATI_API_CALL __ke_check_pci(int busnum, int devnum, int funcnum, __ke_u16* vendor, __ke_u16* device, unsigned int* irq);
extern int ATI_API_CALL __ke_pci_get_irq(__ke_pci_dev_t *dev, unsigned int* irq);
extern __ke_pci_dev_t* ATI_API_CALL __ke_pci_find_device (unsigned int vendor, unsigned int dev, __ke_pci_dev_t* from);
extern int ATI_API_CALL __ke_pci_read_config_byte(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u8 *val);
extern int ATI_API_CALL __ke_pci_read_config_word(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u16 *val);
extern int ATI_API_CALL __ke_pci_read_config_dword(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u32 *val);
extern int ATI_API_CALL __ke_pci_write_config_byte(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u8 val);
extern int ATI_API_CALL __ke_pci_write_config_word(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u16 val);
extern int ATI_API_CALL __ke_pci_write_config_dword(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u32 val);
extern int ATI_API_CALL __ke_pci_enable_device(__ke_pci_dev_t* dev);
extern __ke_pci_dev_t* ATI_API_CALL __ke_pci_find_slot(__ke_u32 bus, __ke_u32 slot);
extern __ke_u8 ATI_API_CALL __ke_pci_get_busnr(__ke_pci_dev_t* pcidev);

/*****************************************************************************/

extern void ATI_API_CALL __ke_outb(unsigned char value, unsigned short port);
extern void ATI_API_CALL __ke_outw(unsigned short value, unsigned short port);
extern void ATI_API_CALL __ke_out(unsigned int value, unsigned short port);
extern char ATI_API_CALL __ke_inb(unsigned short port);
extern short ATI_API_CALL __ke_inw(unsigned short port);
extern int ATI_API_CALL __ke_in(unsigned short port);

/*****************************************************************************/

extern void ATI_API_CALL __ke_enable_irq(int irq);
extern void ATI_API_CALL __ke_disable_irq(int irq);
extern int ATI_API_CALL __ke_request_irq(unsigned int irq, void (*ATI_API_CALL handler)(int, void *, void *), const char *dev_name, void *dev_id);
extern void ATI_API_CALL __ke_free_irq(unsigned int irq, void *dev_id);

/*****************************************************************************/

extern void* ATI_API_CALL __ke_vma_file_priv(struct vm_area_struct* vma);
extern unsigned long ATI_API_CALL __ke_vm_start(struct vm_area_struct* vma);
extern unsigned long ATI_API_CALL __ke_vm_end(struct vm_area_struct* vma);
extern unsigned long ATI_API_CALL __ke_vm_offset(struct vm_area_struct* vma);
enum __ke_vm_maptype
{
    __KE_ADPT,
    __KE_SHM,
    __KE_CTX,
    __KE_PCI_BQS,
    __KE_AGP_BQS,
    __KE_AGP,
    __KE_SG,
    __KE_KMAP,
};
extern char* ATI_API_CALL __ke_vm_flags_str(struct vm_area_struct* vma, char* buf);
extern char* ATI_API_CALL __ke_vm_page_prot_str(struct vm_area_struct* vma, char* buf);
extern char* ATI_API_CALL __ke_vm_phys_addr_str(struct vm_area_struct* vma, 
                                   char* buf, 
                                   unsigned long linear_address, 
                                   __ke_dma_addr_t* phys_address);
extern int ATI_API_CALL __ke_vm_map(struct file* filp,
                                    struct vm_area_struct* vma,
                                    unsigned long offset,
                                    enum __ke_vm_maptype type,
                                    int readonly);

#ifdef __x86_64__
/*
typedef int (*__ke_ioctl_trans_handler_t)(unsigned int, unsigned int,
                                          unsigned long, struct file *);
*/
extern int ATI_API_CALL __ke_register_ioctl32_conversion(unsigned int cmd, int (*handler)(unsigned int, unsigned int, unsigned long, struct file*));
extern void ATI_API_CALL __ke_unregister_ioctl32_conversion(unsigned int cmd);
#endif

/*****************************************************************************/

typedef struct __ke_agp_version
{
    __ke_u16 major;
    __ke_u16 minor;
} __ke_agp_version_t;

typedef struct __ke_agp_kern_info
{
    __ke_agp_version_t version;
    __ke_u16 vendor;
    __ke_u16 device;
    unsigned long mode;
    __ke_off_t aper_base;
    __ke_size_t aper_size;
    int max_memory;     /* In pages */
    int current_memory;
    int cant_use_aperture;
    unsigned long page_mask;
} __ke_agp_kern_info_t;

/*****************************************************************************/

extern int __ke_agp_try_unsupported;

int ATI_API_CALL __ke_agp_available(__ke_pci_dev_t *pcidev, int use_internal);
void ATI_API_CALL __ke_agp_uninit(void);
#ifdef FGL
struct _agp_memory* ATI_API_CALL __ke_agp_allocate_memory_phys_list(
    __ke_size_t pages, unsigned long type, __ke_dma_addr_t * phys_addr);
#endif
void ATI_API_CALL __ke_agp_free_memory(struct _agp_memory* handle);
struct _agp_memory* ATI_API_CALL __ke_agp_allocate_memory(__ke_size_t pages, 
                                             unsigned long type);
int ATI_API_CALL __ke_agp_bind_memory(struct _agp_memory* handle, __ke_off_t start);
int ATI_API_CALL __ke_agp_unbind_memory(struct _agp_memory* handle);
int ATI_API_CALL __ke_agp_enable(unsigned long mode);
int ATI_API_CALL __ke_read_agp_caps_registers(__ke_pci_dev_t* dev, unsigned int *caps);
int ATI_API_CALL __ke_agp_acquire(__ke_pci_dev_t* dev);
void ATI_API_CALL __ke_agp_release(void);
void ATI_API_CALL __ke_agp_copy_info(__ke_agp_kern_info_t* info);
unsigned long ATI_API_CALL __ke_agp_memory_handle(struct _agp_memory* handle);
unsigned long ATI_API_CALL __ke_agp_memory_page_count(struct _agp_memory* handle);

int ATI_API_CALL __ke_is_athlon(void);
int ATI_API_CALL __ke_has_PSE(void);
int ATI_API_CALL __ke_amd_adv_spec_cache_feature(void);
void ATI_API_CALL __ke_smp_call_function( void (*ATI_API_CALL func)(void *info) );
int ATI_API_CALL __ke_smp_processor_id(void);

/*****************************************************************************/

#ifdef FGL
struct _agp_memory* ATI_API_CALL firegl_agp_allocate_memory_phys_list(__ke_size_t page_count, __ke_u32 type, unsigned long * phys_addr);
#endif
struct _agp_memory* ATI_API_CALL firegl_agp_allocate_memory(__ke_size_t page_count, __ke_u32 type);
void ATI_API_CALL firegl_agp_free_memory(struct _agp_memory* curr);
int ATI_API_CALL firegl_agp_bind_memory(struct _agp_memory* curr, __ke_off_t pg_start);
int ATI_API_CALL firegl_agp_unbind_memory(struct _agp_memory* curr);
void ATI_API_CALL firegl_agp_enable(__ke_u32 mode);
int ATI_API_CALL firegl_agp_acquire(struct inode* inode __attribute__((unused)), struct file* filp, unsigned int cmd __attribute__((unused)), unsigned long arg __attribute__((unused)));
void ATI_API_CALL firegl_agp_release(void);
void ATI_API_CALL firegl_agp_copy_info(__ke_agp_kern_info_t* info);
int ATI_API_CALL firegl_agp_init(void);
void ATI_API_CALL firegl_agp_cleanup(void);
unsigned long ATI_API_CALL firegl_agp_memory_handle(struct _agp_memory* handle);
unsigned long ATI_API_CALL firegl_agp_memory_page_count(struct _agp_memory* handle);

/* IGP -- related API calls */
int ATI_API_CALL __ke_agp_memory_get_page_count(struct _agp_memory* agpmem);
void ATI_API_CALL __ke_agp_memory_get_memory(struct _agp_memory* agpmem,
                                unsigned long **memory_ptr);

#ifdef __x86_64__

int ATI_API_CALL firegl_get_user_ptr(__ke_u32 *src,   void **dst);
int ATI_API_CALL firegl_get_user_u16(__ke_u16 *src,   __ke_u16 *dst);
int ATI_API_CALL firegl_get_user_u32(__ke_u32 *src,   __ke_u32 *dst);
int ATI_API_CALL firegl_get_user_u64(__ke_u32 *src,   __ke_u64 *dst);

int ATI_API_CALL firegl_put_user_ptr(void *src,      __ke_u32 *dst);
int ATI_API_CALL firegl_put_user_u16(__ke_u16 src,   __ke_u16 *dst);
int ATI_API_CALL firegl_put_user_u32(__ke_u32 src,   __ke_u32 *dst);
int ATI_API_CALL firegl_put_user_u64(__ke_u64 src,   __ke_u32 *dst);
int ATI_API_CALL firegl_init_32compat_ioctls(void);
void ATI_API_CALL firegl_kill_32compat_ioctls(void);

#endif

/*****************************************************************************/

extern __ke_dma_addr_t ATI_API_CALL __ke_pci_map_single (__ke_pci_dev_t *pdev, void *buffer, __ke_size_t size, int direction); 
extern void ATI_API_CALL __ke_pci_unmap_single (__ke_pci_dev_t *pdev, __ke_dma_addr_t bus_addr, __ke_size_t size, int direction); 
extern __ke_dma_addr_t ATI_API_CALL __ke_pci_map_page (__ke_pci_dev_t *pdev, unsigned long buffer, unsigned long offset, __ke_size_t size, int direction);
extern void ATI_API_CALL __ke_pci_unmap_page (__ke_pci_dev_t *pdev, __ke_dma_addr_t bus_addr, __ke_size_t size, int direction); 
extern int ATI_API_CALL __ke_log2(unsigned long x);
extern __ke_u8 ATI_API_CALL __ke_readb(void* p, __ke_u32 offset);
extern int firegl_xServer_lock(struct inode* inode __attribute__((unused)), struct file* filp, unsigned int cmd __attribute__((unused)), unsigned long arg);
extern int firegl_init_asic(struct inode* inode __attribute__((unused)), struct file* filp, unsigned int cmd __attribute__((unused)), unsigned long arg);
extern int ATI_API_CALL firegl_pci_save_state(struct drm_device* dev);
extern int ATI_API_CALL firegl_pci_restore_state(struct drm_device* dev);
extern int ATI_API_CALL firegl_powerdown(struct drm_device* dev, __ke_u32 state);
extern int ATI_API_CALL firegl_powerup(struct drm_device* dev);


#define __ke_PCI_DMA_BIDIRECTIONAL  0
#define __ke_PCI_DMA_TODEVICE       1
#define __ke_PCI_DMA_FROMDEVICE     2
#define __ke_PCI_DMA_NONE           3


/* globals */
extern int __ke_debuglevel;
#define DEBUGFLAG_DEADBEAF      0x00010000
#define DEBUGFLAG_NO_HW_ACCESS  0x00020000
#define DEBUGLEVEL_MASK         0x0000ffff
#define debuglevel              (__ke_debuglevel &  DEBUGLEVEL_MASK)

extern int __ke_moduleflags;

#define AGPGART_INUSE_NONE		0x00000000
#define KERNEL26_AGPGART_INUSE		0x00000001
#define KERNEL24_AGPGART_INUSE		0x00000002
#define INTERNAL_AGPGART_INUSE		0x00000004

extern unsigned int __ke_firegl_agpgart_inuse;

/* module flags*/
#define __KE_FLAG_DEBUG                     0x00000001
#define __KE_FLAG_NOAUTH                    0x00000002
#define __KE_FLAG_DISABLE_AGPGART           0x00000004
#define __KE_FLAG_DISABLE_FIREGL_AGPGART    0x00000008
#define __KE_FLAG_DISABLE_FIREGL_AGPLOCK    0x00000010
#define __KE_FLAG_DISABLE_DYNAMIC_PCIE      0x00000020

#define __KE_PM_SUSPEND_ON                  0
#define __KE_PM_SUSPEND_STANDBY             1
#define __KE_PM_SUSPEND_MEM                 3
#define __KE_PM_SUSPEND_DISK                4
#define __KE_PM_SUSPEND_MAX                 5

/* global constants */
extern const char*          __ke_UTS_RELEASE;
extern const unsigned long  __ke_LINUX_VERSION_CODE;
extern const unsigned int   __ke_PAGE_SHIFT;
extern const unsigned int   __ke_PAGE_SIZE;
extern const unsigned long  __ke_PAGE_MASK;

extern const unsigned long  __ke_MODVERSIONS_State;
extern const unsigned long  __ke_SMP_State;
extern const unsigned long  __ke_PAE_State;

/* global vars that are in fact constants */
extern unsigned long        __ke_HZ;

/*****************************************************************************/

#ifdef __KE_NO_VSPRINTF
#define	__KE_KERN_ERR	"<3>"
#define	__KE_KERN_INFO	"<6>"
#define	__KE_KERN_DEBUG	"<7>"

                /* Macros to make printk easier */
#define __KE_ERROR(fmt, arg...)                                             \
    __ke_printk(__KE_KERN_ERR "[" FGL_DEBUG_SIGNATURE ":"                   \
        "%s" "] *ERROR* " fmt , __FUNCTION__ , ##arg)

#define __KE_INFO(fmt, arg...)                                              \
    __ke_printk(__KE_KERN_INFO "[" FGL_DEBUG_SIGNATURE "] " fmt , ##arg)

#if 1 /* Leave this on to be able to debug in case of any problems */
#define __KE_DEBUG0(fmt, arg...)                                            \
    do                                                                      \
    {                                                                       \
        __ke_printk(__KE_KERN_DEBUG "[" FGL_DEBUG_SIGNATURE ":"		        \
            "%s" "] " fmt , __FUNCTION__ , ##arg);                          \
    } while (0)
#define __KE_DEBUG(fmt, arg...)                                             \
    do                                                                      \
    {                                                                       \
        if (debuglevel > 0)                                                 \
            __ke_printk(__KE_KERN_DEBUG "[" FGL_DEBUG_SIGNATURE ":"         \
                "%s" "] " fmt , __FUNCTION__ , ##arg);                      \
    } while (0)
#define __KE_DEBUG2(fmt, arg...)                                            \
    do                                                                      \
    {                                                                       \
        if (debuglevel > 1)                                                 \
            __ke_printk(__KE_KERN_DEBUG "[" FGL_DEBUG_SIGNATURE ":"         \
                "%s" "] " fmt , __FUNCTION__ , ##arg);                      \
    } while (0)
#define __KE_DEBUG3(fmt, arg...)                                            \
    do                                                                      \
    {                                                                       \
        if (debuglevel > 2)                                                 \
            __ke_printk(__KE_KERN_DEBUG "[" FGL_DEBUG_SIGNATURE ":"         \
                "%s" "] " fmt , __FUNCTION__ , ##arg);                      \
    } while (0)
#else
#define __KE_DEBUG0(fmt, arg...)    do { } while (0)
#define __KE_DEBUG(fmt, arg...)     do { } while (0)
#define __KE_DEBUG2(fmt, arg...)    do { } while (0)
#define __KE_DEBUG3(fmt, arg...)    do { } while (0)
#endif
#else // !__KE_NO_VSPRINTF
                /* Macros to make printk easier */
#define __KE_ERROR(fmt, arg...)                                             \
    __ke_print_error("[" FGL_DEBUG_SIGNATURE ":"                            \
        "%s" "] *ERROR* " fmt , __FUNCTION__ , ##arg)
#define __KE_INFO(fmt, arg...)                                              \
    __ke_print_info("[" FGL_DEBUG_SIGNATURE "] " fmt , ##arg)

#if 1 /* Leave this on to be able to debug in case of any problems */
#define __KE_DEBUG(fmt, arg...)                                             \
    do                                                                      \
    {                                                                       \
        if (debuglevel > 0)                                                 \
            __ke_print_debug("[" FGL_DEBUG_SIGNATURE ":"                    \
                "%s" "] " fmt , __FUNCTION__ , ##arg);                      \
    } while (0)
#define __KE_DEBUG2(fmt, arg...)                                            \
    do                                                                      \
    {                                                                       \
        if (debuglevel > 1)                                                 \
            __ke_print_debug("[" FGL_DEBUG_SIGNATURE ":"                    \
                "%s" "] " fmt , __FUNCTION__ , ##arg);                      \
    } while (0)
#define __KE_DEBUG3(fmt, arg...)                                            \
    do                                                                      \
    {                                                                       \
        if (debuglevel > 2)                                                 \
            __ke_print_debug("[" FGL_DEBUG_SIGNATURE ":"                    \
                "%s" "] " fmt , __FUNCTION__ , ##arg);                      \
    } while (0)
#else
#define __KE_DEBUG(fmt, arg...)     do { } while (0)
#define __KE_DEBUG2(fmt, arg...)    do { } while (0)
#define __KE_DEBUG3(fmt, arg...)    do { } while (0)
#endif
#endif // !__KE_NO_VSPRINTF

/*****************************************************************************/
#ifdef _KE_SERIAL_DEBUG
extern int  ATI_API_CALL __ke_SerPrint(const char *fmt, ...);
extern void ATI_API_CALL __ke_SetSerialPort(void);
extern void ATI_API_CALL __ke_printstr(const char *str);
#endif

#ifndef __GFP_COMP
#define __GFP_COMP 0
#endif

#endif /* _FIREGL_PUBLIC_H_ */

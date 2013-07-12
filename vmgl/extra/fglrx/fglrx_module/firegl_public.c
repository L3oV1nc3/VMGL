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

#ifdef __KERNEL__

#ifndef MODULE
!!! This is not currently supported,
!!! since it requires changes to linux/init/main.c.
#endif /* !MODULE */

// ============================================================
#include <linux/version.h>
#ifdef MODVERSIONS
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
#include <linux/modversions.h>
#endif
#endif
#include <linux/autoconf.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
#define EXPORT_SYMTAB	1
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,71)
#if !defined(CONFIG_X86_PC) 
#if !defined(CONFIG_X86_64)
#if !defined(CONFIG_X86_VOYAGER)
#if !defined(CONFIG_X86_NUMAQ)
#if !defined(CONFIG_X86_SUMMIT)
#if !defined(CONFIG_X86_BIGSMP)
#if !defined(CONFIG_X86_VISWS)
#if !defined(CONFIG_X86_GENERICARCH)
#if !defined(CONFIG_X86_XEN)
#error unknown or undefined architecture configured
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif /* LINUX_VERSION_CODE */

/* The dirty-page-tracking patch included in NLD 9 SMP kernels defines
 * a static inline function that uses a GPL-only symbol in a header
 * file. Therefore any non-GPL module built against such a kernel
 * configuration is broken and cannot be loaded. We work around that
 * problem by disabling the respective kernel configuration option for
 * our module build.
 *
 * This will break page tracking when this kernel module is
 * used. However, on a standard system page tracking is disabled
 * anyways. It is only activated and used by specific in-kernel agents
 * for example for CPU hot-plugging. I wonder why a desktop
 * distribution would even include such a kernel patch. */
#ifdef CONFIG_MEM_MIRROR
/* Prevent linux/config.h from being included again in subsequent
 * kernel headers as that would redefine CONFIG_MEM_MIRROR. */
#include <linux/config.h>
#warning "Disabling CONFIG_MEM_MIRROR because it does not work with non-GPL modules."
#warning "This will break page tracking when the fglrx kernel module is used."
#undef CONFIG_MEM_MIRROR
#endif

// ============================================================

// always defined
#define __AGP__BUILTIN__

//#define FGL_USE_SCT /* for developer use only */
// ============================================================

#include <asm/unistd.h> /* for installing the patch wrapper */
#include <linux/module.h>

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/file.h>
#include <linux/pci.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
#include <linux/wrapper.h>
#endif
#include <linux/wait.h>
#include <linux/miscdevice.h>
#include <linux/smp_lock.h>
// newer SuSE kernels need this
#include <linux/highmem.h>
#include <linux/pagemap.h> // for lock_page and unlock_page

#include <linux/vmalloc.h>
#if defined(__ia64__)
#include <asm/unistd.h>
#endif

#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/console.h>

//#include <linux/signal.h>
#include <asm/io.h>
#include <asm/mman.h>
#include <asm/uaccess.h>
#include <asm/processor.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <asm/tlbflush.h> // for flush_tlb_page
#else
#include <asm/pgalloc.h> // for flush_tlb_page
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,71)
#include <asm/cpufeature.h>
#endif
#ifdef CONFIG_MTRR
#include <asm/mtrr.h>
#endif
#include <asm/delay.h>
#include <linux/agp_backend.h>

#ifndef EXPORT_NO_SYMBOLS
#define EXPORT_NO_SYMBOLS
#endif

#ifdef __x86_64__
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
#include "linux/ioctl32.h"
#else
#include "asm/ioctl32.h"
#endif

#ifdef __x86_64__
#include "asm/compat.h"
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,2)
#include "linux/syscalls.h"
#endif
#endif

#include <linux/kmod.h>
#include "firegl_public.h"

// ============================================================
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL (void*)0
#endif

#ifdef FGL_USE_SCT
// get direct function pointers from sys_call_table for calling
#else /* FGL_USE_SCT */
// call functions indirectly by using the syscall macros,
// entrypoints get defined by below constructs

#if !defined(__ia64__)
// the macros do use errno variable
int errno;
#endif // __ia64__

// int mlock(const void *addr, size_t len);
_syscall2(int, mlock, const void *, addr, size_t, len )
// int munlock(const void *addr, size_t len);
_syscall2(int, munlock, const void *, addr, size_t, len )
#if !defined(__ia64__)
#if !defined(__x86_64__)
// TODO: ia64
// int modify_ldt(int func, void *ptr, unsigned long bytecount);
_syscall3( int, modify_ldt, int, func, void *, ptr, unsigned long, bytecount )
#endif
#endif
#endif /* FGL_USE_SCT */

#ifdef FGL_LINUX253P1_VMA_API
// Linux 2.5.3-pre1 and compatibles
#define FGL_VMA_API_TYPE        struct vm_area_struct *
#define FGL_VMA_API_NAME        vma
#define FGL_VMA_API_PROTO       FGL_VMA_API_TYPE FGL_VMA_API_NAME,
#define FGL_VMA_API_PASS        FGL_VMA_API_NAME,
#else /* FGL_253P1_VMA_API */
// Linux 2.4.0 and compatibles
#define FGL_VMA_API_TYPE        /* none */
#define FGL_VMA_API_NAME        /* none */
#define FGL_VMA_API_PROTO       /* none */
#define FGL_VMA_API_PASS        /* none */
#endif /* FGL_253P1_VMA_API */

#ifndef preempt_disable
#define preempt_disable()
#define preempt_enable()
#endif

// ============================================================
/* globals */

char* firegl = NULL;
int __ke_debuglevel = 0;
int __ke_moduleflags = 0;

/* global module vars and constants - defined trough macros */
MODULE_AUTHOR("Fire GL - ATI Research GmbH, Germany");
MODULE_DESCRIPTION("ATI Fire GL");
#ifdef MODULE_PARM
MODULE_PARM(firegl, "s");
#else
module_param(firegl, charp, 0);
#endif
#ifdef MODULE_LICENSE
MODULE_LICENSE("Proprietary. (C) 2002 - ATI Technologies, Starnberg, GERMANY");
#endif

#define FIREGL_KERNEL_DRIVER_NAME   "fglrx"   // needed for kernel message macros

/* globals constants */
const char*         __ke_UTS_RELEASE        = UTS_RELEASE;
const unsigned int  __ke_PAGE_SHIFT         = PAGE_SHIFT;
const unsigned int  __ke_PAGE_SIZE          = PAGE_SIZE;
const unsigned long __ke_PAGE_MASK          = PAGE_MASK;
const unsigned long __ke_LINUX_VERSION_CODE = LINUX_VERSION_CODE;

// create global constants and hint symbols (i.e. for objdump checking)
#ifdef MODVERSIONS
const unsigned long __ke_MODVERSIONS_State = 1;
const char BUILD_KERNEL_HAS_MODVERSIONS_SET;
#else
const unsigned long __ke_MODVERSIONS_State = 0;
const char BUILD_KERNEL_HAS_MODVERSIONS_CLEARED;
#endif

#ifdef __SMP__
const unsigned long __ke_SMP_State = 1;
const char BUILD_KERNEL_HAS_SMP_SET;
#else
const unsigned long __ke_SMP_State = 0;
const char BUILD_KERNEL_HAS_SMP_CLEARED;
#endif

/* PAE is always disabled if it's not x86_64 or CONFIG_X86_PAE is disabled on a 32 bit system.*/
#if !defined(__x86_64__) && !defined(CONFIG_X86_PAE)
const unsigned long __ke_PAE_State = 0;
#else
const unsigned long __ke_PAE_State = 1;
#endif

/* globals vars that are in fact constants */
unsigned long __ke_HZ;

// ============================================================
/* global structures */
int ip_firegl_open(struct inode* inode, struct file* filp)
{ return firegl_open(inode, filp); }
int ip_firegl_release(struct inode* inode, struct file* filp)
{ return firegl_release(inode, filp); }
int ip_firegl_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg)
{ return firegl_ioctl(inode, filp, cmd, arg); }
int ip_firegl_mmap(struct file* filp, struct vm_area_struct* vma)
{ return firegl_mmap(filp, vma); }

#if defined(FIREGL_IOCTL_COMPAT) && defined(__x86_64__)
long ip_firegl_compat_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{ return firegl_compat_ioctl(filp, cmd, arg); }
#endif

static struct file_operations firegl_fops =
{
#ifdef THIS_MODULE
    owner:   THIS_MODULE,
#endif
    open:    ip_firegl_open,
    release: ip_firegl_release,
    ioctl:   ip_firegl_ioctl,
    mmap:    ip_firegl_mmap,

#if defined(FIREGL_IOCTL_COMPAT) && defined(__x86_64__)
    compat_ioctl: ip_firegl_compat_ioctl,
#endif
};

typedef struct {
    __ke_device_t       pubdev;     // MUST BE FIRST MEMBER
    dev_t               device;     // Device number for mknod

    /* Locking */
    spinlock_t          spinlock[__KE_MAX_SPINLOCKS];      /* For inuse, open_count, buf_use   */
    struct semaphore    struct_sem[__KE_MAX_SEMAPHORES];   /* For linked list manipulations    */
    sigset_t            sigmask;
} device_t;

static device_t     firegl_public_device;   // The Fire GL public device

/*****************************************************************************/
// standard XFree86 DRM proc support

#define DRM(x) FGLDRM_##x
#include "drm.h"

// mem_info() is missing in drm_proc.h. But, it is a DRM design problem anyway!
// The first registered DRM device could never report memory statisticts of another
// DRM device, cause the DRM mem_info routine uses local variables. So, let's use a dummy.
static int DRM(mem_info)(char *buf __attribute__((unused)), char **start __attribute__((unused)), off_t offset __attribute__((unused)), int len __attribute__((unused)), int *eof, void *data __attribute__((unused)))
{
    *eof = 1;
    return 0;
}
#if 0
#ifdef vmalloc_to_page
#undef vmalloc_to_page
#endif
#define vmalloc_to_page     drm_inline_vmalloc_to_page
#endif
#include "drm_proc.h"

#ifndef DRM_MAJOR
#define DRM_MAJOR       226
#endif // DRM_MAJOR

static __ke_proc_list_t *drm_proclist = NULL;

/*****************************************************************************/
// Fire GL DRM stub support (compatible with standard DRM stub)

#define FIREGL_STUB_MAXCARDS    16

typedef struct firegl_stub_list_tag {
	const char             *name;
	struct file_operations *fops;
	struct proc_dir_entry  *dev_root;
    __ke_proc_list_t       *proclist;
} firegl_stub_list_t;
static firegl_stub_list_t firegl_stub_list[FIREGL_STUB_MAXCARDS];

static struct proc_dir_entry *firegl_stub_root;
static int firegl_minor;

typedef struct firegl_drm_stub_info_tag {
    int (*info_register)(const char *name, struct file_operations *fops, device_t *dev);
	int (*info_unregister)(int minor);
    unsigned long signature; // to check for compatible Fire GL DRM device
} firegl_drm_stub_info_t;
static firegl_drm_stub_info_t firegl_stub_info;

static char *__ke_pte_phys_addr_str(pte_t pte, char *buf, __ke_dma_addr_t* phys_address);

unsigned long ATI_API_CALL __ke_cpu_to_le32(unsigned long _u)
{
    return cpu_to_le32(_u);
}

unsigned long long ATI_API_CALL __ke_cpu_to_le64(unsigned long long _u)
{
    return cpu_to_le64(_u);
}

#define READ_PROC_WRAP(func)                                            \
static int func##_wrap(char *buf, char **start, __ke_off_t offset,      \
                       int len, int* eof, void* data)                   \
{                                                                       \
    return func(buf, start, offset, len, eof, data);                    \
}

READ_PROC_WRAP(drm_name_info)
READ_PROC_WRAP(drm_mem_info)
READ_PROC_WRAP(drm_mem_info1)
READ_PROC_WRAP(drm_vm_info)
READ_PROC_WRAP(drm_clients_info)
READ_PROC_WRAP(firegl_lock_info)
READ_PROC_WRAP(firegl_umm_info)
#ifdef DEBUG
READ_PROC_WRAP(drm_bq_info)
READ_PROC_WRAP(firegl_debug_info)
#endif
READ_PROC_WRAP(firegl_bios_version)

static int 
firegl_interrupt_open_wrap(
        struct inode *inode, 
        struct file *file) 
{
    return firegl_interrupt_open(inode, file);
}

static int 
firegl_interrupt_release_wrap(
        struct inode *inode, 
        struct file *file) 
{
    return firegl_interrupt_release(inode, file);
}

static ssize_t 
firegl_interrupt_read_wrap(
        struct file *user_file, 
        char __user *user_buf, 
        size_t user_buf_size, 
        loff_t *user_file_pos)
{
    return (ssize_t) firegl_interrupt_read(user_file, user_buf, user_buf_size, user_file_pos);
}

static unsigned int 
firegl_interrupt_poll_wrap(struct file *user_file, poll_table *pt) 
{
    if(firegl_interrupt_poll(user_file, (__ke_poll_table*)pt))
    {
        return POLLIN | POLLRDNORM;
    }
    else
    {
        return 0;
    }
}

static ssize_t 
firegl_interrupt_write_wrap(
        struct file *user_file, 
        char __user *user_buf, 
        size_t user_buf_size, 
        loff_t *user_file_pos)
{
    return (ssize_t) firegl_interrupt_write(user_file, user_buf, user_buf_size, user_file_pos);
}

static struct file_operations firegl_interrupt_file_ops = {
    .open       = firegl_interrupt_open_wrap,
    .read       = firegl_interrupt_read_wrap,
    .release    = firegl_interrupt_release_wrap,
    .poll       = firegl_interrupt_poll_wrap,
    .write      = firegl_interrupt_write_wrap
};

__ke_proc_list_t firegl_proc_list[] = 
{
    { "name",           drm_name_info_wrap,         NULL},
    { "mem",            drm_mem_info_wrap,          NULL},
    { "mem1",           drm_mem_info1_wrap,         NULL},
    { "vm",             drm_vm_info_wrap,           NULL},
    { "clients",        drm_clients_info_wrap,      NULL},
    { "lock",           firegl_lock_info_wrap,      NULL},
    { "umm",            firegl_umm_info_wrap,       NULL},
#ifdef DEBUG
    { "bq_info",        drm_bq_info_wrap,           NULL},
    { "debug",          firegl_debug_info_wrap,     NULL},
#endif
    { "biosversion",    firegl_bios_version_wrap,   NULL},
    { "interrupts",     NULL,                       (__ke_file_operations_t*)&firegl_interrupt_file_ops},
    { "NULL",           NULL,                       NULL} // Terminate List!!!
};

static struct proc_dir_entry *firegl_proc_init( device_t *dev,
                                                int minor,
				                                struct proc_dir_entry *root,
				                                struct proc_dir_entry **dev_root,
                                                __ke_proc_list_t *proc_list ) // proc_list must be terminated!
{
	struct proc_dir_entry *ent;
	char    name[64];
    __ke_proc_list_t *list = proc_list;
    __KE_DEBUG("minor %d, proc_list 0x%08lx\n", minor, (unsigned long)proc_list);
	if (!minor)
        root = create_proc_entry("dri", S_IFDIR, NULL);

	if (!root) {
		__KE_ERROR("Cannot create /proc/dri\n");
		return NULL;
	}

	sprintf(name, "%d", minor);
	*dev_root = create_proc_entry(name, S_IFDIR, root);
	if (!*dev_root) {
		__KE_ERROR("Cannot create /proc/%s\n", name);
		return NULL;
	}

    while (list->f || list->fops)
    {
		ent = create_proc_entry(list->name, S_IFREG|S_IRUGO, *dev_root);
		if (!ent)
        {
			__KE_ERROR("Cannot create /proc/dri/%s/%s\n", name, list->name);
            while (proc_list != list)
            {
				remove_proc_entry(proc_list->name, *dev_root);
                proc_list++;
            }
			remove_proc_entry(name, root);
			if (!minor)
                remove_proc_entry("dri", NULL);
			return NULL;
		}

        if (list->f)
        {
		    ent->read_proc = (read_proc_t*)list->f;
        }

        if (list->fops)
        {
		    ent->proc_fops = (struct file_operations*)list->fops;
        }
        
        ent->data      = (dev->pubdev.signature == FGL_DEVICE_SIGNATURE) ? 
                            ((void*)dev->pubdev.privdev) : (dev);
        
        list++;
	}

	return root;
}

static int firegl_proc_cleanup( int minor,
                                struct proc_dir_entry *root,
		                        struct proc_dir_entry *dev_root,
                                __ke_proc_list_t *proc_list )
{
	char name[64];
    __KE_DEBUG("minor %d\n", minor);

	if (!root || !dev_root)
    {
        __KE_ERROR("no root\n");
        return 0;
    }

    while (proc_list->f || proc_list->fops)
    {
		remove_proc_entry(proc_list->name, dev_root);
        proc_list++;
    }
	sprintf(name, "%d", minor);
	remove_proc_entry(name, root);
	if (!minor)
        remove_proc_entry("dri", NULL);

	return 0;
}

static int firegl_stub_open(struct inode *inode, struct file *filp)
{
#ifndef MINOR
	int                    minor = minor(inode->i_rdev);
#else
	int                    minor = MINOR(inode->i_rdev);
#endif
	int                    err   = -ENODEV;
	struct file_operations *old_fops;
    __KE_DEBUG("\n");

        if (minor >= FIREGL_STUB_MAXCARDS)
            return -ENODEV;
	if (!firegl_stub_list[minor].fops)
        return -ENODEV;
	old_fops   = filp->f_op;
	filp->f_op = fops_get(firegl_stub_list[minor].fops);
	if (filp->f_op->open && (err = filp->f_op->open(inode, filp))) {
		fops_put(filp->f_op);
		filp->f_op = fops_get(old_fops);
	}
	fops_put(old_fops);
	return err;
}

static struct file_operations firegl_stub_fops = {
	owner:   THIS_MODULE,
	open:	 firegl_stub_open
};

static int firegl_stub_getminor(const char *name, struct file_operations *fops, device_t *dev)
{
	int i;

        __KE_DEBUG("name=\"%s\"\n", name);
	for( i = 0; i < FIREGL_STUB_MAXCARDS; i++ ) {
	    if( !firegl_stub_list[i].fops ) {
		firegl_stub_list[i].name = name;
		firegl_stub_list[i].fops = fops;
                firegl_stub_list[i].proclist = (dev->pubdev.signature == FGL_DEVICE_SIGNATURE) ? dev->pubdev.proclist : drm_proclist;
                firegl_stub_root = firegl_proc_init(dev, i, firegl_stub_root, &firegl_stub_list[i].dev_root, firegl_stub_list[i].proclist);
                __KE_DEBUG("minor=%d\n", i);
		return i;
	    }
	}
       __KE_DEBUG("no more free minor\n");
	return -1;
}

static int firegl_stub_putminor(int minor)
{
        __KE_DEBUG("minor=%d\n", minor);

	if (minor < 0 || minor >= FIREGL_STUB_MAXCARDS)
        return -1;

	firegl_proc_cleanup(minor, firegl_stub_root, firegl_stub_list[minor].dev_root, firegl_stub_list[minor].proclist);
	firegl_stub_list[minor].name = NULL;
	firegl_stub_list[minor].fops = NULL;
        firegl_stub_list[minor].proclist = NULL;

	if( !minor ) {
		unregister_chrdev(DRM_MAJOR, "drm");
	}
	return 0;
}

static int __init firegl_stub_register(const char *name, struct file_operations *fops, device_t *dev)
{
    int err;
    __KE_DEBUG("name=\"%s\"\n", name);

    // try to register a drm char device for the firegl module
    err = register_chrdev(DRM_MAJOR, "drm", &firegl_stub_fops);
    if(err == 0)
    {
        __KE_DEBUG("register_chrdev() succeeded\n");

        // register our own module handler will handle the DRM device
	firegl_stub_info.info_register   = firegl_stub_getminor;
	firegl_stub_info.info_unregister = firegl_stub_putminor;

    } else if(err == -EINVAL) {
        __KE_ERROR("register_chrdev() failed with -EINVAL\n");
        return -1;
    } else if(err == -EBUSY) {

        // the registering of the module's device has failed 
        // because there was already some other drm module loaded.
        __KE_DEBUG("register_chrdev() failed with -EBUSY\n");
	return -1;
    }
    else
    {
        __KE_ERROR("register_chrdev() failed with %i\n", err);
        return -1;
    }

    return firegl_stub_info.info_register(name, fops, dev);
}

static int __exit firegl_stub_unregister(int minor)
{
	__KE_DEBUG("%d\n", minor);
	if (firegl_stub_info.info_unregister)
		return firegl_stub_info.info_unregister(minor);
	return -1;
}

#ifdef FIREGL_POWER_MANAGEMENT

static struct pci_device_id fglrx_pci_table[] = 
{
    {
        .vendor      = PCI_VENDOR_ID_ATI,
        .device      = PCI_ANY_ID,
        .subvendor   = PCI_ANY_ID,
        .subdevice   = PCI_ANY_ID,
        .class       = (PCI_CLASS_DISPLAY_VGA << 8),
        .class_mask  = ~0,
    },
    { 0, }
};

static int fglrx_pci_probe(struct pci_dev *dev, const struct pci_device_id *id_table)
{
    return 0;
}

/* Starting from 2.6.14, kernel has new struct defined for pm_message_t,
   we have to handle this case separately.
   2.6.11/12/13 kernels have pm_message_t defined as int and older kernels
   don't have pm_message_t defined. 
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14)
static int fglrx_pci_suspend(struct pci_dev *pdev, pm_message_t pm_event)
#else
static int fglrx_pci_suspend(struct pci_dev *pdev, u32 pm_event)
#endif
{
    device_t* dev = &firegl_public_device;
    struct drm_device* privdev;
    int ret = 0, state;
    privdev = (struct drm_device*) dev->pubdev.privdev;


    state = PMSG_EVENT(pm_event);
    if (state == PMSG_EVENT(pdev->dev.power.power_state)) return 0;
    __KE_DEBUG("power state: %d-%d\n", state, PMSG_EVENT(pdev->dev.power.power_state));


    /* lock console output to prevent kernel hang with vesafb
     * A temporal workaround for current kernel issue, the workaround
     * itself may cause a different deadlock, but it appears to
     * happen much less frequent then without this workaround.
     */
    if (state == PM_EVENT_SUSPEND)
        acquire_console_sem();

    if (firegl_powerdown(privdev, state))
        ret = -EIO;

    if (!ret)
    {
        firegl_pci_save_state(privdev);
        pci_disable_device(pdev);
        PMSG_EVENT(pdev->dev.power.power_state) = state;
    }

    return ret;
}

static int fglrx_pci_resume(struct pci_dev *pdev)
{
    device_t* dev = &firegl_public_device;
    struct drm_device* privdev;
    privdev = (struct drm_device*) dev->pubdev.privdev;

#ifdef _KE_SERIAL_DEBUG
    __ke_SetSerialPort();
#endif

    __KE_DEBUG("%d \n", PMSG_EVENT(pdev->dev.power.power_state));

    if (PMSG_EVENT(pdev->dev.power.power_state) == 0) return 0;

    // PCI config space needs to be restored very early, in particular
    // before pci_set_master!
    firegl_pci_restore_state(privdev);

    if (pci_enable_device(pdev)) 
    {
        __KE_ERROR("Cannot enable PCI device.\n");
    }    

    pci_set_master(pdev);
 
    firegl_powerup(privdev);

    if (PMSG_EVENT(pdev->dev.power.power_state) == PM_EVENT_SUSPEND)
        release_console_sem();

    PMSG_EVENT(pdev->dev.power.power_state) = 0;

    return 0;
}

static struct pci_driver fglrx_pci_driver = 
{
    .name           = "fglrx_pci",
    .id_table       = fglrx_pci_table,
    .probe          = fglrx_pci_probe,
#ifdef CONFIG_PM
    .suspend        = fglrx_pci_suspend,
    .resume         = fglrx_pci_resume,
#endif /* CONFIG_PM */
};
#endif // FIREGL_POWER_MANAGEMENT


/*****************************************************************************/
/* init_module is called when insmod is used to load the module */
static int __init firegl_init_module(void)
{
    device_t* dev = &firegl_public_device;
    unsigned int i;
    int retcode;

	EXPORT_NO_SYMBOLS;

    // init global vars that are in fact constants
    __ke_HZ = HZ;

#ifdef _KE_SERIAL_DEBUG  
    __ke_SetSerialPort();
#endif
     
    // init DRM proc list
    drm_proclist = kmalloc((DRM_PROC_ENTRIES + 1) * sizeof(__ke_proc_list_t), GFP_KERNEL);
    if ( drm_proclist == NULL )
        return -ENOMEM;

    for ( i=0; i<DRM_PROC_ENTRIES; i++ )
    {
        drm_proclist[i].name = DRM(proc_list)[i].name;
        drm_proclist[i].f = (__ke_read_proc_t)DRM(proc_list)[i].f;
    }
    drm_proclist[i].f = NULL; // terminate list

    memset(&firegl_stub_list, 0, sizeof(firegl_stub_list));
    memset(&firegl_stub_info, 0, sizeof(firegl_stub_info));
    firegl_stub_info.signature = FGL_DEVICE_SIGNATURE;

    memset(dev, 0, sizeof(*dev));
    dev->pubdev.signature = FGL_DEVICE_SIGNATURE;

    for (i = 0; i < __KE_MAX_SPINLOCKS; i++)
        dev->spinlock[i] = SPIN_LOCK_UNLOCKED;

    for (i=0; i < __KE_MAX_SEMAPHORES; i++)
        sema_init(&dev->struct_sem[i], 1);

    dev->pubdev.psigmask = (__ke_sigset_t*)&dev->sigmask;

    if ( (retcode = firegl_init(&dev->pubdev)) )
    {
        __KE_ERROR("firegl_init failed\n");
        kfree(drm_proclist);
        return retcode;
    }

#if !defined(FIREGL_IOCTL_COMPAT) && defined(__x86_64__)
    if(!firegl_init_32compat_ioctls())
    {
        kfree(drm_proclist);
	__KE_ERROR("Couldn't register compat32 ioctls!\n");
	return -ENODEV;
    }
#endif

    // get the minor number
    firegl_minor = firegl_stub_register(dev->pubdev.name, &firegl_fops, dev);
    if (firegl_minor < 0)
    {
        __KE_ERROR("firegl_stub_register failed\n");
        kfree(drm_proclist);
		return -EPERM;
    }

    dev->device = MKDEV(DRM_MAJOR, firegl_minor);

    __KE_INFO("module loaded - %s %d.%d.%d [%s] on minor %d\n",
            dev->pubdev.name,
	        dev->pubdev.major_version,
	        dev->pubdev.minor_version,
	        dev->pubdev.patchlevel,
	        dev->pubdev.date,
		    firegl_minor);

    
#ifdef FIREGL_POWER_MANAGEMENT
    if (pci_register_driver (&fglrx_pci_driver) < 0)
    {
        __KE_ERROR("Failed to register fglrx as PCI driver\n");
    }
#endif // FIREGL_POWER_MANAGEMENT

    return 0; // OK!
}

/* cleanup_module is called when rmmod is used to unload the module */
static void __exit firegl_cleanup_module(void)
{
    device_t* dev = &firegl_public_device;
    __KE_DEBUG("module cleanup started\n");

#ifdef FIREGL_POWER_MANAGEMENT
    pci_unregister_driver (&fglrx_pci_driver);
#endif

    if ( firegl_stub_unregister(firegl_minor) ) {
        __KE_ERROR("Cannot unload module\n");
    }

    firegl_cleanup(&dev->pubdev);

#if !defined(FIREGL_IOCTL_COMPAT) && defined(__x86_64__)
    firegl_kill_32compat_ioctls();
#endif

    if (drm_proclist)
        kfree(drm_proclist);

	__KE_INFO("module unloaded - %s %d.%d.%d [%s] on minor %d\n",
            dev->pubdev.name,
	        dev->pubdev.major_version,
	        dev->pubdev.minor_version,
	        dev->pubdev.patchlevel,
	        dev->pubdev.date,
		    firegl_minor);
}

module_init( firegl_init_module );
module_exit( firegl_cleanup_module );

/*****************************************************************************/
// wait queues

__ke_wait_queue_head_t* ATI_API_CALL __ke_alloc_wait_queue_head_struct(void)
{
    __ke_wait_queue_head_t* queue_head;
    queue_head = kmalloc(sizeof(wait_queue_head_t), GFP_ATOMIC);

    if (queue_head)
    {
        init_waitqueue_head((wait_queue_head_t*)(void *)queue_head);
    }

    return queue_head;
}

void ATI_API_CALL __ke_free_wait_queue_head_struct(__ke_wait_queue_head_t* queue_head)
{
    if (queue_head)
        kfree(queue_head);
}

__ke_wait_queue_t* ATI_API_CALL __ke_alloc_wait_queue_struct(void)
{
    __ke_wait_queue_t* queue;
    queue = kmalloc(sizeof(wait_queue_t), GFP_KERNEL);

    return queue;
}

void ATI_API_CALL __ke_free_wait_queue_struct(__ke_wait_queue_t* queue)
{
    if (queue)
        kfree(queue);
}

void ATI_API_CALL __ke_wake_up_interruptible(__ke_wait_queue_head_t* queue_head)
{
    wake_up_interruptible((wait_queue_head_t*)(void *)queue_head);
}

void ATI_API_CALL __ke_add_wait_queue(__ke_wait_queue_head_t* queue_head, __ke_wait_queue_t* entry)
{
    // initialisation (delayed)
#ifdef __WAITQUEUE_INITIALIZER
    wait_queue_t template =
        __WAITQUEUE_INITIALIZER(((wait_queue_t*)(void *)entry), current);

    *((wait_queue_t*)(void *)entry) = template;
#else
    ((wait_queue_t*)(void *)entry)->task = current;
    ((wait_queue_t*)(void *)entry)->flags = 0x0;
    ((wait_queue_t*)(void *)entry)->task_list.next = NULL;
    ((wait_queue_t*)(void *)entry)->task_list.prev = NULL;
    ((wait_queue_t*)(void *)entry)->func = NULL;
#if WAITQUEUE_DEBUG
    ((wait_queue_t*)(void *)entry)->__magic = &(((wait_queue_t*)(void *)entry)->__magic);
    ((wait_queue_t*)(void *)entry)->__waker = 0;
#endif /* WAITQUEUE_DEBUG */
#endif /* __WAITQUEUE_INITIALIZER */

    // addition
    add_wait_queue((wait_queue_head_t*)(void *)queue_head, (wait_queue_t*)(void *)entry);
}

void ATI_API_CALL __ke_remove_wait_queue(__ke_wait_queue_head_t* queue_head, __ke_wait_queue_t* entry)
{
//    current->state = TASK_RUNNING;
    remove_wait_queue((wait_queue_head_t*)(void *)queue_head, 
									(wait_queue_t*)(void *)entry);
}

void ATI_API_CALL __ke_init_waitqueue_head(__ke_wait_queue_head_t* queue_head)
{
    init_waitqueue_head((wait_queue_head_t*)(void *)queue_head); 
}

void ATI_API_CALL __ke_wait_event_interruptible(__ke_wait_queue_head_t* queue_head, int condition)
{
    wait_event_interruptible(*((wait_queue_head_t*)(void *)queue_head), condition); 
}

void ATI_API_CALL __ke_poll_wait(struct file* filp, __ke_wait_queue_head_t* queue_head, __ke_poll_table* pt)
{
    poll_wait(filp, (wait_queue_head_t*)(void*)queue_head, (poll_table*)(void*)pt);
}

// scheduler
void ATI_API_CALL __ke_schedule(void)
{
	schedule();
}
/** /brief This routine will let the current processor yield for other threads.
 */
void ATI_API_CALL __ke_yield(void)
{
    yield();
}


int ATI_API_CALL __ke_signal_pending(void)
{
    return signal_pending(current);
}

void ATI_API_CALL __ke_set_current_state_task_interruptible(void)
{
    current->state = TASK_INTERRUPTIBLE;
}

void ATI_API_CALL __ke_set_current_state_task_running(void)
{
    current->state = TASK_RUNNING;
}

void ATI_API_CALL __ke_configure_sigmask(__ke_sigset_t *pSigMask)
{
    sigemptyset((sigset_t*)(void *)pSigMask);
    sigaddset((sigset_t*)(void *)pSigMask, SIGSTOP);
    sigaddset((sigset_t*)(void *)pSigMask, SIGTSTP);
    sigaddset((sigset_t*)(void *)pSigMask, SIGTTIN);
    sigaddset((sigset_t*)(void *)pSigMask, SIGTTOU);
}

int
firegl_sig_notifier_wrap(void *priv)
{
    return firegl_sig_notifier(priv);
}

void ATI_API_CALL __ke_block_all_signals(int (*notifier)(void *priv), void *pPriv, __ke_sigset_t *pSigMask)
{
    block_all_signals(notifier,pPriv,(sigset_t*)(void *)pSigMask);
}

void ATI_API_CALL __ke_unblock_all_signals(void)
{
    unblock_all_signals();
}

#if defined(__i386__) 
#ifndef __HAVE_ARCH_CMPXCHG
static inline 
unsigned long __fgl_cmpxchg(volatile void *ptr, unsigned long old,            
                        unsigned long new, int size)                      
{                                                                                       
    unsigned long prev;                                                             
    switch (size) {                                                                 
    case 1:                                                                         
        __asm__ __volatile__(LOCK_PREFIX "cmpxchgb %b1,%2"
                             : "=a"(prev)
                             : "q"(new), "m"(*__xg(ptr)), "0"(old)
                             : "memory");
        return prev;
    case 2:
        __asm__ __volatile__(LOCK_PREFIX "cmpxchgw %w1,%2"
                             : "=a"(prev)
                             : "q"(new), "m"(*__xg(ptr)), "0"(old)
                             : "memory");
        return prev;
    case 4:
        __asm__ __volatile__(LOCK_PREFIX "cmpxchgl %1,%2"
                             : "=a"(prev)
                             : "q"(new), "m"(*__xg(ptr)), "0"(old)
                             : "memory");
        return prev;
    }
    return old;
}
#endif /* cmpxchg */
#elif defined(__alpha__)
todo !!!
#endif

#if !defined(__ia64__)
unsigned long ATI_API_CALL __ke__cmpxchg(volatile void *ptr, unsigned long old,
         unsigned long new, int size)
{
#ifndef __HAVE_ARCH_CMPXCHG
    return __fgl_cmpxchg(ptr,old,new,size);
#else
    return __cmpxchg(ptr,old,new,size);
#endif
}
#endif

/*****************************************************************************/

__ke_dev_t ATI_API_CALL __ke_getdevice(__ke_device_t *dev)
{
    return ((device_t*)dev)->device;
}

const char* ATI_API_CALL __ke_module_parm(void)
{
    return firegl;
}

/*****************************************************************************/

int ATI_API_CALL __ke_inode_rdev_minor(struct inode* inode)
{
#ifndef MINOR
    return minor(inode->i_rdev);
#else
    return MINOR(inode->i_rdev);
#endif
}

/*****************************************************************************/

void* ATI_API_CALL __ke_get_proc_dir_entry_priv(struct proc_dir_entry *pde)
{
    return pde->data;
}

void* ATI_API_CALL __ke_get_inode_priv(struct inode* inode)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,4)
    return inode->u.generic_ip;
#else
    return PDE(inode);
#endif    
}

void* ATI_API_CALL __ke_get_file_priv(struct file* filp)
{
    return filp->private_data;
}

void ATI_API_CALL __ke_set_file_priv(struct file* filp, void* private_data)
{
    filp->private_data = private_data;
}

int ATI_API_CALL __ke_file_excl_open(struct file* filp)
{
    return (filp->f_flags & O_EXCL) != 0;
}

int ATI_API_CALL __ke_file_rw_open(struct file* filp)
{
    return (filp->f_flags & 3) != 0;
}

unsigned int ATI_API_CALL __ke_file_counter(struct file *filp)
{
    return filp->f_count.counter;
}

struct inode* ATI_API_CALL __ke_get_file_inode(struct file* filp)
{
    return (struct inode*)filp->f_dentry->d_inode;
}

/*****************************************************************************/

int ATI_API_CALL __ke_getpid(void)
{
    return current->pid;
}

int ATI_API_CALL __ke_geteuid(void)
{
    return current->euid;
}

/*****************************************************************************/

unsigned long ATI_API_CALL __ke_jiffies(void)
{
    return jiffies;
}

void ATI_API_CALL __ke_udelay(unsigned long usecs) // delay in usec
{
    unsigned long start;
    unsigned long stop;
    unsigned long period;
    unsigned long wait_period;
    struct timespec tval;

#ifdef NDELAY_LIMIT
    // kernel provides delays with nano(=n) second accuracy
#define UDELAY_LIMIT    (NDELAY_LIMIT/1000) /* supposed to be 10 msec */
#else
    // kernel provides delays with micro(=u) second accuracy
#define UDELAY_LIMIT    (10000)             /* 10 msec */
#endif

    if (usecs > UDELAY_LIMIT)
    {
        start = jiffies;
        tval.tv_sec = usecs / 1000000;
        tval.tv_nsec = (usecs - tval.tv_sec * 1000000) * 1000;
        wait_period = timespec_to_jiffies(&tval);
        do {
            stop = jiffies;

            if (stop < start) // jiffies overflow
                period = ((unsigned long)-1 - start) + stop + 1;
            else
                period = stop - start;

        } while (period < wait_period);
    }
    else
        udelay(usecs);  /* delay value might get checked once again */
}

void ATI_API_CALL __ke_mdelay(unsigned long msecs) // delay in msec
{
        mdelay(msecs);
}

__ke_u8 ATI_API_CALL __ke_readb(void* p, __ke_u32 offset)
{
    return readb((u8*)p + offset);
}

/*****************************************************************************/
// TODO: These here get obsolete in future, use the ia64 code below
// Johannes
unsigned long ATI_API_CALL __ke_virt_to_bus(void* address)
{
    return virt_to_bus(address);
}

unsigned long ATI_API_CALL __ke_virt_to_phys(void* address)
{
    return virt_to_phys(address);
}

void* ATI_API_CALL __ke_high_memory(void)
{
    return high_memory;
}

int ATI_API_CALL __ke_pci_enable_device(__ke_pci_dev_t* dev)
{
    return (pci_enable_device( (struct pci_dev*)(void *)dev ));
}

#if defined(__x86_64__) || defined(__ia64__)
void* ATI_API_CALL __ke_pci_alloc_consistent(__ke_pci_dev_t* dev, int size, void *dma_handle)
{
	return (pci_alloc_consistent( (struct pci_dev*)(void *)dev, size, dma_handle)); 
}

void ATI_API_CALL __ke_pci_free_consistent(__ke_pci_dev_t* dev, int size, unsigned long cpu_addr,
						 unsigned int dma_handle)
{
	pci_free_consistent( (struct pci_dev*)(void *)dev, size, (void *)cpu_addr, 
		(unsigned long)dma_handle);
}
#endif // __ia64__

/*****************************************************************************/

int ATI_API_CALL __ke_error_code(enum __ke_error_num errcode)
{
    switch (errcode)
    {
        case __KE_EBUSY:
            return EBUSY;
        case __KE_EINVAL:
            return EINVAL;
        case __KE_EACCES:
            return EACCES;
        case __KE_EFAULT:
            return EFAULT;
        case __KE_EIO:
            return EIO;
        case __KE_EBADSLT:
            return EBADSLT;
        case __KE_ENOMEM:
            return ENOMEM;
        case __KE_EPERM:
            return EPERM;
        case __KE_ENODEV:
            return ENODEV;
        case __KE_EINTR:
            return EINTR;
        case __KE_ERESTARTSYS:
            return ERESTARTSYS;
        case __KE_ELIBBAD:
            return ELIBBAD;
        default:
            return EFAULT;
    }
}

/*****************************************************************************/
#ifdef __x86_64__

int ATI_API_CALL firegl_get_user_ptr(u32 *src, void **dst)
{
  unsigned long temp;
  int err = get_user(temp, src); 
  *dst = (void*) temp;
  return err;
}

int ATI_API_CALL firegl_get_user_u16(u16 *src, u16 *dst)
{
  u16 temp;
  int err = get_user(temp, src);
  *dst = temp;
  return err;
}

int ATI_API_CALL firegl_get_user_u32(u32 *src, u32 *dst)
{
  u32 temp;
  int err = get_user(temp, src);
  *dst = temp;
  return err;
}

int ATI_API_CALL firegl_get_user_u64(u32 *src, u64 *dst)
{
  u64 temp;
  int err = get_user(temp, src);
  *dst = temp;
  return err;
}

int ATI_API_CALL firegl_put_user_ptr(void *src, u32 *dst)
{
  void *temp = src;
  return put_user(temp, dst);
}

int ATI_API_CALL firegl_put_user_u16(u16 src, u16 *dst)
{
  u16 temp = src;
  return put_user(temp, dst);
}

int ATI_API_CALL firegl_put_user_u32(u32 src, u32 *dst)
{
  u32 temp = src;
  return put_user(temp, dst);
}

int ATI_API_CALL firegl_put_user_u64(u64 src, u32 *dst)
{
  u64 temp = src;
  return put_user(temp, dst);
}
#endif /* __x86_64__ */


/*****************************************************************************/

void ATI_API_CALL __ke_mod_inc_use_count(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    __module_get(THIS_MODULE);
#else
    MOD_INC_USE_COUNT;
#endif
}

void ATI_API_CALL __ke_mod_dec_use_count(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    module_put(THIS_MODULE);
#else
    MOD_DEC_USE_COUNT;
#endif
}

/*****************************************************************************/

void ATI_API_CALL __ke_down_struct_sem(__ke_device_t *dev, int index)
{
    down(&(((device_t*)dev)->struct_sem[index]));
}

void ATI_API_CALL __ke_up_struct_sem(__ke_device_t *dev, int index)
{
    up(&(((device_t*)dev)->struct_sem[index]));
}

void ATI_API_CALL __ke_sema_init(struct semaphore* sem, int value)
{
    sema_init(sem, value);
}

__ke_size_t ATI_API_CALL __ke_sema_size(void)
{
    return sizeof(struct semaphore);
}

void ATI_API_CALL __ke_down(struct semaphore* sem)
{
    down(sem);
}

void ATI_API_CALL __ke_up(struct semaphore* sem)
{
    up(sem);
}

/*****************************************************************************/

void ATI_API_CALL __ke_atomic_inc(void* v)
{
    atomic_inc((atomic_t*)v);
}

void ATI_API_CALL __ke_atomic_dec(void* v)
{
    atomic_dec((atomic_t*)v);
}

void ATI_API_CALL __ke_atomic_add(int val, void* v)
{
    atomic_add(val, (atomic_t*)v);
}

void ATI_API_CALL __ke_atomic_sub(int val, void* v)
{
    atomic_sub(val, (atomic_t*)v);
}

int ATI_API_CALL __ke_atomic_read(void* v)
{
    return atomic_read((atomic_t*)v);
}

void ATI_API_CALL __ke_atomic_set(void* v, int val)
{
    atomic_set((atomic_t*)v, val);
}

/*****************************************************************************/

void ATI_API_CALL __ke_spin_lock(__ke_device_t *dev, int ndx)
{
    spin_lock(&(((device_t*)dev)->spinlock[ndx]));
}

void ATI_API_CALL __ke_spin_unlock(__ke_device_t *dev __attribute__((unused)), int ndx __attribute__((unused)))
{
    spin_unlock(&(((device_t*)dev)->spinlock[ndx]));
}

void ATI_API_CALL __ke_lock_kernel(void)
{
    lock_kernel();
}

void ATI_API_CALL __ke_unlock_kernel(void)
{
    unlock_kernel();
}

/*****************************************************************************/

#ifdef FGL_USE_SCT
extern unsigned long sys_call_table[];
#endif

typedef int (*PFNMLOCK)(unsigned long start, __ke_size_t len);
typedef int (*PFNMUNLOCK)(unsigned long start, __ke_size_t len);


int ATI_API_CALL __ke_sys_mlock(unsigned long start, __ke_size_t len)
{
#ifdef FGL_USE_SCT
    PFNMLOCK sys_mlock = (PFNMLOCK)sys_call_table[__NR_mlock];
    if (!sys_mlock) {
        __KE_ERROR("sys_call_table[__NR_mlock] == 0\n");
        return -1;
    }

    return (*sys_mlock)(start, len);
#else
    return mlock((void*)start, len);
#endif
}

int ATI_API_CALL __ke_sys_munlock(unsigned long start, __ke_size_t len)
{
#ifdef FGL_USE_SCT
    PFNMUNLOCK sys_munlock = (PFNMUNLOCK)sys_call_table[__NR_munlock];
    if (!sys_munlock) {
        __KE_ERROR("sys_call_table[__NR_munlock] == 0\n");
        return -1;
    }

    return (*sys_munlock)(start, len);
#else
    return munlock((void*)start, len);
#endif
}


typedef int (*PFNMODIFYLDT)(int func, void *ptr, unsigned long bytecount);

int ATI_API_CALL __ke_sys_modify_ldt(int func, void *ptr, unsigned long bytecount)
{
#ifdef FGL_USE_SCT
    PFNMODIFYLDT sys_modify_ldt = (PFNMODIFYLDT)sys_call_table[__NR_modify_ldt];
    if (!sys_modify_ldt) {
        __KE_ERROR("sys_call_table[__NR_modify_ldt] == 0\n");
        return -1;
    }

    return (*sys_modify_ldt)(func, ptr, bytecount);
#else
#if !defined(__ia64__) && !defined(__x86_64__)
    return modify_ldt(func, ptr, bytecount);
#else
	// TODO: how should this be down on ia64????
	return 0;
#endif
#endif
}

/*****************************************************************************/
int ATI_API_CALL __ke_vsprintf(char *buf, const char *fmt, va_list ap)
{
    return vsprintf(buf, fmt, ap);
}

int ATI_API_CALL __ke_vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
    return vsnprintf(buf, size, fmt, ap);
}

#ifdef __KE_NO_VSPRINTF

void ATI_API_CALL __ke_printk(const char* fmt, ...)
{
	char buffer[256];
    va_list marker;

    va_start(marker, fmt);
    vsprintf(buffer, fmt, marker);
    va_end(marker);

    printk(buffer);
#ifdef _KE_SERIAL_DEBUG
    __ke_SerPrint(buffer);
#endif
}

#else

void ATI_API_CALL __ke_print_info(const char* fmt, ...)
{
    char msg[256] = KERN_INFO;:
    va_list marker;

    va_start(marker, fmt);
    vsprintf(msg + strlen(msg), fmt, marker);
    va_end(marker);
}

void ATI_API_CALL __ke_print_error(const char* fmt, ...)
{
    char msg[256] = KERN_ERR;
    va_list marker;

    va_start(marker, fmt);
    vsprintf(msg + strlen(msg), fmt, marker);
    va_end(marker);
}

void ATI_API_CALL __ke_print_debug(const char* fmt, ...)
{
    char msg[256] = KERN_DEBUG;
    va_list marker;

    va_start(marker, fmt);
    vsprintf(msg + strlen(msg), fmt, marker);
    va_end(marker);
}

#endif

#ifdef _KE_SERIAL_DEBUG
// To enable serial port debug message dumping,just define _KE_SERIAL_DEBUG in firegl_public.h file. 
// Connect two PC with a null modern serial cable. run Hyper ternimal on the remote machine. 
// It's useful to debug resume if network not works properly and serial port is not recovered 
// properly when fglrx resume hook is called...
 
 
#define SER_DATA_PORT       0x3f8
#define SER_INT_CTRL_PORT   SER_DATA_PORT + 1
#define SER_INT_STAT_PORT   SER_DATA_PORT + 2
#define SER_LINE_CTRL_PORT  SER_DATA_PORT + 3
#define SER_MODEM_CTRL_PORT SER_DATA_PORT + 4
#define SER_LINE_STAT_PORT  SER_DATA_PORT + 5
    
void ATI_API_CALL __ke_printc(char c)
{
     while((inb(SER_LINE_STAT_PORT) & 0x20) == 0 ); //wait until Transmitter Holding Register Empty
     outb(c,SER_DATA_PORT);
}

void ATI_API_CALL __ke_printstr(const char *str)
{
    int len = strlen(str);
    while(len--)__ke_printc(*str++);  
}

int ATI_API_CALL __ke_SerPrint(const char *format, ...)
{
    char buffer[256];
    va_list ap;

    va_start(ap, format);

    vsprintf(buffer, format, ap);

    va_end(ap);
    
    __ke_printstr(buffer);
    
    return 0;
}
void ATI_API_CALL __ke_SetSerialPort()
{
    DRM_INFO("setup serial port\n");
    outb(0x00,  SER_INT_CTRL_PORT);   // Turn off interrupts 

    outb(0x80,  SER_LINE_CTRL_PORT);  // SET DLAB ON 
    outb(0x01,  SER_DATA_PORT);  // Set Baud rate - Divisor Latch Low Byte 
                             // 0x01 = 115,200 ,0x02 =  57,600,  0x06 =  19,200 BPS, 0x0C =   9,600 BPS  
    outb(0x00,  SER_DATA_PORT + 1);  // Set Baud rate - Divisor Latch High Byte 
    outb(0x03,  SER_LINE_CTRL_PORT); // reset DLAB ,8 Bits, No Parity, 1 Stop Bit 
    outb(0xC7,  SER_DATA_PORT + 2);  // FIFO Control Register 
    outb(0x0b,  SER_DATA_PORT + 4);  // Turn on DTR, RTS, and OUT2
  
    __ke_printstr("serial port 0x3f8 is set ready for message print out \n");
}  
#endif

/*****************************************************************************/

int ATI_API_CALL __ke_capable(enum __ke_cap cap)
{
    switch (cap)
    {
        case __KE_CAP_SYS_ADMIN:
            cap = CAP_SYS_ADMIN;
			break;
        case __KE_CAP_IPC_LOCK:
            cap = CAP_IPC_LOCK;
			break;
        default:
            return 0;
    }
    return capable(cap);
}

void ATI_API_CALL __ke_cap_effective_raise(enum __ke_cap cap)
{
    switch (cap)
    {
        case __KE_CAP_SYS_ADMIN:
            cap = CAP_SYS_ADMIN;
			break;
        case __KE_CAP_IPC_LOCK:
            cap = CAP_IPC_LOCK;
			break;
        default:
            return;
    }
    cap_raise(current->cap_effective, cap);
}

__ke_u32 ATI_API_CALL __ke_get_cap_effective()
{
    return cap_t(current->cap_effective);
}

void ATI_API_CALL __ke_set_cap_effective(__ke_u32 cap)
{
    cap_t(current->cap_effective) = cap;
}

unsigned long ATI_API_CALL __ke_ram_available(void)
{
	struct sysinfo si;

    si_meminfo(&si);
#if LINUX_VERSION_CODE < 0x020317
    /* Changed to page count in 2.3.23 */
	return si.totalram >> PAGE_SHIFT;
#else
	return si.totalram;
#endif
}

#ifdef __x86_64__
void* ATI_API_CALL __ke_compat_alloc_user_space(long size)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    return compat_alloc_user_space(size);
#else
    struct pt_regs *regs = (void *)current->thread.rsp0 - sizeof(struct pt_regs);
    return (void __user *)regs->rsp - size;
#endif
}
#endif

int ATI_API_CALL __ke_copy_from_user(void* to, const void* from, __ke_size_t size)
{
    return copy_from_user(to, from, size);
}

int ATI_API_CALL __ke_copy_to_user(void* to, const void* from, __ke_size_t size)
{
    return copy_to_user(to, from, size);
}

int ATI_API_CALL __ke_verify_area(int type, const void * addr, unsigned long size)
{
#ifdef access_ok
    return access_ok(type,addr,size) ? 0 : -EFAULT;
#else
    return verify_area(type, addr, size);
#endif
}

int ATI_API_CALL __ke_get_pci_device_info(__ke_pci_dev_t* dev, __ke_pci_device_info_t *pinfo)
{
    if ( dev )
    {
        pinfo->vendor = ((struct pci_dev*)(void *)dev)->vendor;
        pinfo->device = ((struct pci_dev*)(void *)dev)->device;
        pinfo->subsystem_vendor = ((struct pci_dev*)(void *)dev)->subsystem_vendor;
        pinfo->subsystem_device = ((struct pci_dev*)(void *)dev)->subsystem_device;
    }
    return -EINVAL;
}

int ATI_API_CALL __ke_check_pci(int busnum, int devnum, int funcnum, __ke_u16* vendor, __ke_u16* device, unsigned int* irq)
{
    struct pci_dev* pci_dev;

    pci_dev = pci_find_slot(busnum, PCI_DEVFN(devnum, funcnum));
    if (!pci_dev)
        return 0;

    if (vendor)
        *vendor = pci_dev->vendor;

    if (device)
        *device = pci_dev->device;

    if (irq)
        *irq = pci_dev->irq;

    return 1;
}

int ATI_API_CALL __ke_pci_get_irq(__ke_pci_dev_t *dev, unsigned int* irq)
{
    if (!dev)
        return 0;
    if (!irq)
        return 0;

    *irq = ((struct pci_dev*)dev)->irq;
    return 1;
}

__ke_pci_dev_t* ATI_API_CALL __ke_pci_find_device (unsigned int vendor, unsigned int dev, __ke_pci_dev_t* from)
{
	return (__ke_pci_dev_t*)pci_find_device( vendor, dev, (struct pci_dev *)(void *)from );
}

void* ATI_API_CALL __ke_malloc(__ke_size_t size)
{
    return kmalloc(size, GFP_KERNEL);
}

void* ATI_API_CALL __ke_malloc_atomic(__ke_size_t size)
{
    return kmalloc(size, GFP_ATOMIC);
}

void ATI_API_CALL __ke_free(void* p)
{
    kfree(p);
}

void ATI_API_CALL __ke_free_s(void* p, __ke_size_t size)
{
    kfree(p);
}

void* ATI_API_CALL __ke_vmalloc(__ke_size_t size)
{
    return vmalloc(size);
}
void* ATI_API_CALL __ke_vmalloc_32(__ke_size_t size)
{
    return vmalloc_32(size);
}
void* ATI_API_CALL __ke_vmalloc_atomic(__ke_size_t size)
{
    return __vmalloc(size, GFP_ATOMIC, PAGE_KERNEL);
}

void ATI_API_CALL __ke_vfree(void* p)
{
    return vfree(p);
}

void* ATI_API_CALL __ke_get_free_page(void)
{
    return (void*)__get_free_page(GFP_KERNEL);
}

void* ATI_API_CALL __ke_get_free_pages(int order)
{
    return (void*)__get_free_pages(GFP_KERNEL|__GFP_COMP, order);
}

void ATI_API_CALL __ke_free_page(void* pt)
{
    free_page((unsigned long)pt);
}

void ATI_API_CALL __ke_free_pages(void* pt, int order)
{
    free_pages((unsigned long)pt, order);
}

void ATI_API_CALL __ke_get_page(void* pt)
{
    get_page(virt_to_page((unsigned long)pt));
}

void ATI_API_CALL __ke_put_page(void* pt)
{
    put_page(virt_to_page((unsigned long)pt));
}

void ATI_API_CALL __ke_unlock_page(void *virt)
{
    unlock_page(virt_to_page((unsigned long)virt));
}

int ATI_API_CALL __ke_PageCompound(void *virt)
{
#ifdef PageCompound
    return PageCompound(virt_to_page((unsigned long)virt));
#else
    return 0;
#endif
}

#if defined(VM_MAP) || defined(vunmap)
void* ATI_API_CALL __ke_vmap(unsigned long *pagelist, unsigned int count)
{
    struct page * pages[count];
    int i;

    for (i=0; i<count; i++)
    {
        pages[i] = virt_to_page(pagelist[i]);
    }
#ifdef FGL_LINUX_SUSE90_VMAP_API
    ///Here's  a special implementation of vmap for Suse 9.0 support
    /// This will be defined in make.sh if needed
    return (void *) vmap(pages, count);
#else
#ifdef VM_MAP
    return (void *) vmap(pages, count, VM_MAP, PAGE_KERNEL);
#else
    return (void *) vmap(pages, count, 0, PAGE_KERNEL);
#endif
#endif
}

void ATI_API_CALL __ke_vunmap(void* addr)
{
    vunmap(addr);
}
#else   // defined(VM_MAP) || defined(vunmap)
void* ATI_API_CALL __ke_vmap(unsigned long *pagelist, unsigned int count)
{
    return NULL;
}
void ATI_API_CALL __ke_vunmap(void* addr)
{
}
#endif  // defined(VM_MAP) || defined(vunmap)

void ATI_API_CALL __ke_mem_map_reserve(void* pt)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
    mem_map_reserve(virt_to_page((unsigned long)pt));
#else
    SetPageReserved(virt_to_page((unsigned long)pt));
#endif
}

void ATI_API_CALL __ke_mem_map_unreserve(void* pt)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
    mem_map_unreserve(virt_to_page((unsigned long)pt));
#else
    ClearPageReserved(virt_to_page((unsigned long)pt));
#endif
}

void ATI_API_CALL __ke_virt_reserve(void* virt)
{
    set_bit(PG_reserved,
        &virt_to_page((unsigned long)virt)->flags);
}

void ATI_API_CALL __ke_virt_unreserve(void* virt)
{
    clear_bit(PG_reserved,
        &virt_to_page((unsigned long)virt)->flags);
}

void ATI_API_CALL __ke_lock_page(void* virt)
{
    lock_page(virt_to_page((unsigned long)virt));
}

#ifdef __ia64__
void* ATI_API_CALL __ke_get_vmptr( struct _agp_memory* memory )
{
	return memory->vmptr;
}
#endif
                              
void* ATI_API_CALL __ke_ioremap(unsigned long offset, unsigned long size)
{
    return ioremap(offset, size);
}

void* ATI_API_CALL __ke_ioremap_nocache(unsigned long offset, unsigned long size)
{
    return ioremap_nocache(offset, size);
}

void ATI_API_CALL __ke_iounmap(void* pt)
{
    iounmap(pt);
}

int ATI_API_CALL __ke_verify_read_access(void* addr, __ke_size_t size)
{
    return access_ok(VERIFY_READ, addr, size) ? 0 : -EFAULT;
}

int ATI_API_CALL __ke_verify_write_access(void* addr, __ke_size_t size)
{
    return access_ok(VERIFY_WRITE, addr, size) ? 0 : -EFAULT;
}

struct mm_struct* ATI_API_CALL __ke_init_mm(void)
{
    return &init_mm;
}

struct mm_struct* ATI_API_CALL __ke_current_mm(void)
{
    return current->mm;
}

__ke_dma_addr_t ATI_API_CALL __ke_get_vm_phys_addr(struct mm_struct* mm, unsigned long virtual_addr)
{
    pgd_t* pgd_p;
    pmd_t* pmd_p;
    pte_t  pte;

    PGD_OFFSET(mm, pgd_p, virtual_addr);
    PGD_PRESENT(pgd_p);
    PMD_OFFSET(pmd_p, pgd_p, virtual_addr);
    PMD_PRESENT(pmd_p);
    PTE_OFFSET(pte, pmd_p, virtual_addr);
    PTE_PRESENT(pte);

#if defined(__x86_64__) || defined(__ia64__)
    return pte.pte & PAGE_MASK;
#else
    return pte_val(pte) & (u64)((u64)PAGE_MASK | (u64)0xf<<32);
#endif
}

unsigned long ATI_API_CALL __ke_get_vm_page_table(struct mm_struct* mm, unsigned long virtual_addr, unsigned long* page_addr, int* is_highpte)
{
    pgd_t* pgd_p;
    pmd_t* pmd_p;
    unsigned long page_table;

    __KE_DEBUG("virtual_addr=0x%08lx\n", virtual_addr);

    PGD_OFFSET(mm, pgd_p, virtual_addr);
    PGD_PRESENT(pgd_p);
    __KE_DEBUG("pgd_p=0x%08lx\n", (unsigned long)pgd_p);

    PMD_OFFSET(pmd_p, pgd_p, virtual_addr);
    PMD_PRESENT(pmd_p);
    __KE_DEBUG("pmd_p=0x%08lx\n", (unsigned long)pmd_p);

    *page_addr = (unsigned long) PMD_PAGE(*pmd_p);
    if (PageHighMem((struct page*)(*page_addr)))
    {
        page_table = (unsigned long) kmap((struct page*)(*page_addr));
        *is_highpte = 1;
    }
    else
    {
        page_table = (unsigned long)phys_to_virt(pmd_val(*pmd_p) & PAGE_MASK);
        *is_highpte = 0;
    }

    __KE_DEBUG("page_table %lx, page_addr %lx, is_highpte %d\n", page_table, *page_addr, *is_highpte);
    return page_table;
}

void ATI_API_CALL __ke_put_vm_page_table(unsigned long page_addr)
{
    kunmap((struct page*) page_addr);
}

#ifndef ptep_clear_flush_dirty
#define ptep_clear_flush_dirty(__vma, __address, __ptep) \
({							 \
    int __dirty = ptep_test_and_clear_dirty(__ptep);	 \
    if (__dirty)					 \
        flush_tlb_page(__vma, __address);		 \
    __dirty;						 \
})
#endif

int ATI_API_CALL __ke_vm_test_and_clear_dirty(struct mm_struct* mm, unsigned long virtual_addr)
{
    int ret = -1; // init with page not present
    pgd_t* pgd_p;
    pmd_t* pmd_p;
    pte_t* pte_p;
    pte_t  pte;
    struct vm_area_struct *vma;

    __KE_DEBUG("virtual_addr=0x%08lx\n", virtual_addr);

    vma = find_vma(mm, virtual_addr);;
    if (NULL == vma)
    {
        __KE_DEBUG("%s", "ERROR: find_vma failed\n");
        return -1;
    }

    PGD_OFFSET(mm, pgd_p, virtual_addr);
    if (!pgd_present(*pgd_p))
    {
        __KE_DEBUG("ERROR: !pgd_present\n");
        return -1;
    }
    __KE_DEBUG("pgd_p=0x%08lx\n", (unsigned long)pgd_p);

    PMD_OFFSET(pmd_p, pgd_p, virtual_addr);
    if (!pmd_present(*pmd_p))
    {
        __KE_DEBUG("ERROR: !pmd_present\n");
        return -1;
    }
    __KE_DEBUG("pmd_p=0x%08lx\n", (unsigned long)pmd_p);

#ifdef pte_offset_atomic
    pte_p = pte_offset_atomic(pmd_p, virtual_addr);
    if (pte_present(*pte_p))
        ret = (ptep_clear_flush_dirty(vma, virtual_addr, pte_p) ? 1 : 0);
    else
        __KE_DEBUG("page not exists!\n");
    pte_kunmap(pte_p);
#else
#ifdef pte_offset_map
    pte_p = pte_offset_map(pmd_p, virtual_addr);
    if (pte_present(*pte_p))
        ret = (ptep_clear_flush_dirty(vma, virtual_addr, pte_p) ? 1 : 0);
    else
        __KE_DEBUG("page not exists!\n");
    pte_unmap(pte_p);
#else
#ifdef pte_offset_kernel
    pte_p = pte_offset_kernel(pmd_p, virtual_addr);
    if (pte_present(*pte_p))
        ret = (ptep_clear_flush_dirty(vma, virtual_addr, pte_p) ? 1 : 0);
    else
        __KE_DEBUG("page not exists!\n");
#else
    pte_p = pte_offset(pmd_p, virtual_addr);
    if (pte_present(*pte_p))
        ret = (ptep_clear_flush_dirty(vma, virtual_addr, pte_p) ? 1 : 0);
    else
        __KE_DEBUG("page not exists!\n");
#endif
#endif
#endif

    if (debuglevel > 2)
    {
        char buf[50];
        __ke_dma_addr_t page_phys_addr;
        __KE_DEBUG("pte: %s\n", __ke_pte_phys_addr_str(pte, buf, &page_phys_addr));
    }

    __KE_DEBUG("return %d\n", ret);
    return ret;
}

__ke_dma_addr_t* ATI_API_CALL __ke_get_vm_phys_addr_list(struct mm_struct* mm, unsigned long virtual_addr, unsigned long pages)
{
    __ke_dma_addr_t *phys_table, *pt;
    unsigned long   n, va;

    /* caller is responsible for the respective kfree call */
    phys_table = kmalloc(pages*sizeof(__ke_dma_addr_t), GFP_KERNEL);
    if (!phys_table)
        return NULL;

    pt = phys_table;
    va = virtual_addr;
    for(n=0; n<pages; n++)
    {
        *pt = __ke_get_vm_phys_addr(mm,va);
        pt++;
        va += PAGE_SIZE;
    }

    return phys_table;
}


void* ATI_API_CALL __ke_memset(void* s, int c, __ke_size_t count)
{
    return memset(s, c, count);
}

void* ATI_API_CALL __ke_memcpy(void* d, const void* s, __ke_size_t count)
{
    return memcpy(d, s, count);
}

__ke_size_t ATI_API_CALL __ke_strlen(const char *s)
{
    return strlen(s);
}

char* ATI_API_CALL __ke_strcpy(char* d, const char* s)
{
    return strcpy(d, s);
}

char* ATI_API_CALL __ke_strncpy(char* d, const char* s, __ke_size_t count)
{
    return strncpy(d, s, count);
}

int ATI_API_CALL __ke_strcmp(const char* string1, const char* string2)
{
    return strcmp(string1, string2);
}

int ATI_API_CALL __ke_strncmp(const char* string1, const char* string2, __ke_size_t count)
{
    return strncmp(string1, string2, count);
}

char* ATI_API_CALL __ke_strchr(const char *s, int c)
{
    return strchr(s, c);
}

int ATI_API_CALL __ke_sprintf(char* buf, const char* fmt, ...)
{
    va_list marker;

    va_start(marker, fmt);
    vsprintf(buf, fmt, marker);
    va_end(marker);

    return strlen(buf);
}

int ATI_API_CALL __ke_snprintf(char* buf, size_t size, const char* fmt, ...)
{
    va_list marker;

    va_start(marker, fmt);
    vsnprintf(buf, size, fmt, marker);
    va_end(marker);

    return strlen(buf);
}

/*****************************************************************************/

void ATI_API_CALL __ke_set_bit(int nr, volatile void* addr)
{
    set_bit(nr, addr);
}

void ATI_API_CALL __ke_clear_bit(int nr, volatile void* addr)
{
    clear_bit(nr, addr);
}

void ATI_API_CALL __ke_change_bit(int nr, volatile void* addr)
{
    change_bit(nr, addr);
}

int ATI_API_CALL __ke_test_bit(int nr, volatile void* addr)
{
    return test_bit(nr, addr);
}

int ATI_API_CALL __ke_test_and_set_bit(int nr, volatile void* addr)
{
    return test_and_set_bit(nr, addr);
}

int ATI_API_CALL __ke_test_and_clear_bit(int nr, volatile void* addr)
{
    return test_and_clear_bit(nr, addr);
}

int ATI_API_CALL __ke_test_and_change_bit(int nr, volatile void* addr)
{
    return test_and_change_bit(nr, addr);
}

/*****************************************************************************/

#ifdef __SMP__
static atomic_t cpus_waiting;

static void deferred_flush(void* contextp)
{
#if defined(__i386__) || defined(__x86_64__)
	asm volatile ("wbinvd":::"memory");
#elif defined(__alpha__) || defined(__ia64__) || defined(__sparc__)
	mb();
#else
#error "Please define flush_cache."
#endif
	atomic_dec(&cpus_waiting);
	while (atomic_read(&cpus_waiting) > 0)
		barrier();
}
#endif /* __SMP__ */

int ATI_API_CALL __ke_flush_cache(void)
{
#ifdef __SMP__
#if LINUX_VERSION_CODE < 0x020501
	atomic_set(&cpus_waiting, smp_num_cpus - 1);
#endif

    /* write back invalidate all other CPUs (exported by kernel) */
	if (smp_call_function(deferred_flush, NULL, 1, 0) != 0)
		panic("timed out waiting for the other CPUs!\n");

    /* invalidate this CPU */
#if defined(__i386__) || defined(__x86_64__)
	asm volatile ("wbinvd":::"memory");
#elif defined(__alpha__) || defined(__ia64__) || defined(__sparc__)
	mb();
#else
#error "Please define flush_cache for your architecture."
#endif

	while (atomic_read(&cpus_waiting) > 0)
		barrier();
#else /* !__SMP__ */
#if defined(__i386__) || defined(__x86_64__)
	asm volatile ("wbinvd":::"memory");
#elif defined(__alpha__) || defined(__ia64__) || defined(__sparc__)
	mb();
#else
#error "Please define flush_cache for your architecture."
#endif
#endif /* !__SMP__ */
    return 0;
}

/*****************************************************************************/

int ATI_API_CALL __ke_config_mtrr(void)
{
#ifdef CONFIG_MTRR
    return 1;
#else /* !CONFIG_MTRR */
    return 0;
#endif /* !CONFIG_MTRR */
}

int ATI_API_CALL __ke_mtrr_add_wc(unsigned long base, unsigned long size)
{
#ifdef CONFIG_MTRR
    return mtrr_add(base, size, MTRR_TYPE_WRCOMB, 1);
#else /* !CONFIG_MTRR */
    return -EPERM;
#endif /* !CONFIG_MTRR */
}

int ATI_API_CALL __ke_mtrr_del(int reg, unsigned long base, unsigned long size)
{
#ifdef CONFIG_MTRR
    return mtrr_del(reg, base, size);
#else /* !CONFIG_MTRR */
    return -EPERM;
#endif /* !CONFIG_MTRR */
}

int ATI_API_CALL __ke_has_vmap(void)
{
#if defined(VM_MAP) || defined(vunmap)
    return 1;
#else
    return 0;
#endif
}

#ifdef __x86_64__
int ATI_API_CALL __ke_config_iommu(void)
{
#ifdef CONFIG_GART_IOMMU
    return 1;
#else /* !CONFIG_GART_IOMMU */
    return 0;
#endif /* !CONFIG_GART_IOMMU */
}

int ATI_API_CALL __ke_no_iommu(void)
{
    return 0;
}
#endif 

/*****************************************************************************/

int ATI_API_CALL __ke_pci_read_config_byte(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u8 *val)
{
    return pci_read_config_byte((struct pci_dev*)(void *)dev, where, val);
}

int ATI_API_CALL __ke_pci_read_config_word(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u16 *val)
{
    return pci_read_config_word((struct pci_dev*)(void *)dev, where, val);
}

int ATI_API_CALL __ke_pci_read_config_dword(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u32 *val)
{
    return pci_read_config_dword((struct pci_dev*)(void *)dev, where, val);
}

int ATI_API_CALL __ke_pci_write_config_byte(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u8 val)
{
    return pci_write_config_byte((struct pci_dev*)(void *)dev, where, val);
}

int ATI_API_CALL __ke_pci_write_config_word(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u16 val)
{
    return pci_write_config_word((struct pci_dev*)(void *)dev, where, val);
}

int ATI_API_CALL __ke_pci_write_config_dword(__ke_pci_dev_t* dev, __ke_u8 where, __ke_u32 val)
{
    return pci_write_config_dword((struct pci_dev*)(void *)dev, where, val);
}

__ke_pci_dev_t* ATI_API_CALL __ke_pci_find_slot(__ke_u32 bus, __ke_u32 slot)
{
    return (__ke_pci_dev_t*)pci_find_slot(bus, slot);
}

__ke_u8 ATI_API_CALL __ke_pci_get_busnr(__ke_pci_dev_t* pcidev)
{
    struct pci_dev* dev = (struct pci_dev*)pcidev;
    return dev->bus->number;
}

__ke_dma_addr_t ATI_API_CALL __ke_pci_map_single (__ke_pci_dev_t *pdev, void *buffer, __ke_size_t size, int direction)
{
    return pci_map_single((struct pci_dev*)(void*)pdev, buffer, size, direction);
}

void ATI_API_CALL __ke_pci_unmap_single (__ke_pci_dev_t *pdev, __ke_dma_addr_t bus_addr, __ke_size_t size, int direction)
{
    pci_unmap_single((struct pci_dev*)(void*)pdev, bus_addr, size, direction);
}

__ke_dma_addr_t ATI_API_CALL __ke_pci_map_page (__ke_pci_dev_t *pdev, unsigned long buffer, unsigned long offset, __ke_size_t size, int direction)
{
    return pci_map_page((struct pci_dev*)(void*)pdev, virt_to_page(buffer), offset, size, direction);
}

void ATI_API_CALL __ke_pci_unmap_page (__ke_pci_dev_t *pdev, __ke_dma_addr_t bus_addr, __ke_size_t size, int direction)
{
    pci_unmap_page((struct pci_dev*)(void*)pdev, bus_addr, size, direction);
}


/*****************************************************************************/

void ATI_API_CALL __ke_outb(unsigned char value, unsigned short port)
{
    outb(value, port);
}

void ATI_API_CALL __ke_outw(unsigned short value, unsigned short port)
{
    outw(value, port);
}

void ATI_API_CALL __ke_outl(unsigned int value, unsigned short port)
{
    outl(value, port);
}

char ATI_API_CALL __ke_inb(unsigned short port)
{
    return inb(port);
}

short ATI_API_CALL __ke_inw(unsigned short port)
{
    return inw(port);
}

int ATI_API_CALL __ke_inl(unsigned short port)
{
    return inl(port);
}

int ATI_API_CALL __ke_log2(unsigned long x)
{
   return ffz(~(x));
}

/*****************************************************************************/
// Interrupt support

void ATI_API_CALL __ke_enable_irq(int irq)
{
    enable_irq( irq );
}

void ATI_API_CALL __ke_disable_irq(int irq)
{
    disable_irq( irq );
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
int ATI_API_CALL __ke_request_irq(unsigned int irq, 
    void (*ATI_API_CALL handler)(int, void *, void *),
    const char *dev_name, void *dev_id)
{
    return request_irq(irq,
        (void(*)(int, void *, struct pt_regs *))handler,
        SA_SHIRQ, dev_name, dev_id);
}

void ATI_API_CALL __ke_free_irq(unsigned int irq, void *dev_id)
{
    free_irq(irq, dev_id);
}
#else
static void ATI_API_CALL (*irq_handler_func)(int, void*, void*); /* function pointer variable */

static irqreturn_t ke_irq_handler_wrap(int irq, void *arg1, struct pt_regs *regs)
{
    irq_handler_func(irq, arg1, regs);
    return IRQ_HANDLED;
}

int ATI_API_CALL __ke_request_irq(unsigned int irq, 
    void (*ATI_API_CALL handler)(int, void *, void *),
    const char *dev_name, void *dev_id)
{
    irq_handler_func = handler;
    return request_irq(irq,
        ke_irq_handler_wrap,
        SA_SHIRQ, dev_name, dev_id);
}

void ATI_API_CALL __ke_free_irq(unsigned int irq, void *dev_id)
{
    free_irq(irq, dev_id);
    irq_handler_func = NULL;
}
#endif

#ifdef __x86_64__
int ATI_API_CALL __ke_register_ioctl32_conversion(unsigned int cmd, int (*handler)(unsigned int, unsigned int, unsigned long, struct file*))
{
#ifdef FIREGL_IOCTL_COMPAT
    return 0;
#else
    return register_ioctl32_conversion(cmd, handler);
#endif
}

void ATI_API_CALL __ke_unregister_ioctl32_conversion(unsigned int cmd)
{
#ifdef FIREGL_IOCTL_COMPAT
    return 0;
#else
    unregister_ioctl32_conversion(cmd);
#endif
}
#endif

/* agp_memory related routine for IGP */
int ATI_API_CALL __ke_agp_memory_get_page_count(struct _agp_memory* agpmem)
{
    return (int)(agpmem->page_count);
}

void ATI_API_CALL __ke_agp_memory_get_memory(struct _agp_memory* agpmem, 
                                             unsigned long **memory_ptr)
{
    __KE_DEBUG("[%s] agpmem=0x%016lx agpmem->memory=0x%016lx [0]=0x%016x",
               __FUNCTION__, 
               (unsigned long)agpmem, 
               (unsigned long)agpmem->memory,
               (agpmem->memory)[0]);

    *memory_ptr = agpmem->memory;
}

/*****************************************************************************/

#ifndef NOPAGE_SIGBUS
#define NOPAGE_SIGBUS 0
#endif /* !NOPAGE_SIGBUS */

#if LINUX_VERSION_CODE > 0x020500
typedef struct page mem_map_t;
typedef mem_map_t *vm_nopage_ret_t;
#elif LINUX_VERSION_CODE >= 0x020400
typedef mem_map_t* vm_nopage_ret_t;
#endif /* LINUX_VERSION_CODE */

static __inline__ vm_nopage_ret_t do_vm_nopage(struct vm_area_struct* vma,
                                                     unsigned long address)
{
    return 0;   /* Disallow mremap */
}

#ifdef __AGP__BUILTIN__
#ifdef __ia64__
static __inline__ vm_nopage_ret_t do_vm_cant_nopage(struct vm_area_struct* vma,
                                                          unsigned long address)
{
	void *dev;
	unsigned long offset = address - vma->vm_start;
	unsigned long baddr = VM_OFFSET(vma) + offset;
	unsigned long mem;
	struct page *page;

    // TODO
    dev = firegl_get_dev_from_vm(vma);
//	if (firegl_cant_use_agp(dev))
	{
		mem = firegl_get_virt_agp_mem( dev, baddr, vma);
		if (mem)
		{
			page = virt_to_page((unsigned long)__va(mem));
			get_page(page);
			return page;
		}
	}
	return NOPAGE_SIGBUS;		/* Disallow mremap */
}

#endif /* __ia64__ */
#endif /* __AGP__BUILTIN__ */


static __inline__ vm_nopage_ret_t do_vm_shm_nopage(struct vm_area_struct* vma,
                                                   unsigned long address)
{
    pgd_t* pgd_p;
    pmd_t* pmd_p;
    pte_t  pte;
    unsigned long vma_offset;
    unsigned long pte_linear;
    mem_map_t* pMmPage;

    /*
        vm_start           => start of vm-area,  regular address
        vm_end             => end of vm-area,    regular address
        vm_offset/vm_pgoff => start of area,     linear address
        address            => requested address, regular address

        Check range
        Seems the surrounding framework already does that test -
        skip it here, anyone does.
     */
#if 0
    if (address < vma->vm_start)
        return NOPAGE_SIGBUS; /* Address is out of range */
#endif
    /*
        Note: vm_end is not member of range but this border
        hmm, might be used when growing the VMA, not sure - keep it as it is.
     */

    __KE_DEBUG3("start=0x%08lx, "
            "end=0x%08lx, "
            "offset=0x%08lx\n",
            vma->vm_start,
            vma->vm_end,
            (unsigned long)__ke_vm_offset(vma));

    if (address > vma->vm_end)
        return NOPAGE_SIGBUS; /* address is out of range */

    /*  Calculate offset into VMA */
    vma_offset = address - vma->vm_start;

    /*
      Find the map with the given handle (vm_offset) and get the
      linear address.
    */
    pte_linear = firegl_get_addr_from_vm(vma);
    if (!pte_linear)
    {
        return NOPAGE_SIGBUS; /* bad address */
    }
    pte_linear += vma_offset;

    /*
        Locate responsible kernel PTE for this linear address
        in paging system of the kernel VM
        (or is it the init process? - not sure yet)
     */
    PGD_OFFSET_K(pgd_p, pte_linear);
    PGD_PRESENT(pgd_p);
    /*  locate medium level page table (x86 => nop) */
    PMD_OFFSET(pmd_p, pgd_p, pte_linear);
    PMD_PRESENT(pmd_p);
    /*  locate page table entry itself */
    PTE_OFFSET(pte, pmd_p, pte_linear);
    PTE_PRESENT(pte);

    /*
        Pretty straight in new kernel. The pte is represented by the requested
        pointer to the page table entry.
     */
    pMmPage = pte_page(pte);

    get_page(pMmPage);  /* inc usage count of page */

    //  __KE_DEBUG3("vm-address 0x%08lx => kernel-page-address 0x%p\n",
    //    address, page_address(pMmPage));
    return pMmPage;
}

/*

    This routine is intended to remap addresses of a OpenGL context
      (which is one ore more pages in size)

*/
static __inline__ vm_nopage_ret_t do_vm_dma_nopage(struct vm_area_struct* vma, unsigned long address)
{
    unsigned long kaddr;
    mem_map_t* pMmPage;

    if (address > vma->vm_end)
        return 0; /* Disallow mremap */

    /*
        Have we ever got an acces from user land into context structure?

        Resolve the kernel (mem_map/page) address for the VMA-address
        we got queried about.
    */
    kaddr = firegl_get_addr_from_vm(vma);
    if (!kaddr)
    {
        return NOPAGE_SIGBUS; /* bad address */
    }
    kaddr += (address - vma->vm_start);

    pMmPage = virt_to_page(kaddr);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 15)
    // WARNING WARNINIG WARNNING WARNNING WARNNING WARNNING WARNNING WARNNING
    // Don't increment page usage count, cause ctx pages are allocated
    // with drm_alloc_pages, which marks all pages as reserved. Reserved
    // pages' usage count is not decremented by the kernel during unmap!!!
    //
    // For kernel >= 2.6.15, We should reenable this, because the VM sub-system 
    // will decrement the pages' usage count even for the pages marked as reserved 
    // 								- MC.
    get_page(pMmPage); /* inc usage count of page */
#endif

    __KE_DEBUG3("vm-address 0x%08lx => kernel-page-address 0x%p\n",
        address, page_address(pMmPage));
    return pMmPage;
}

static __inline__ vm_nopage_ret_t do_vm_kmap_nopage(struct vm_area_struct* vma, unsigned long address)
{
    unsigned long kaddr;
    mem_map_t* pMmPage;

    if (address > vma->vm_end)
        return 0; /* Disallow mremap */

    if ((pMmPage = (mem_map_t*) firegl_get_pagetable_page_from_vm(vma)))
    {
        get_page(pMmPage);
        return pMmPage;
    }

    kaddr = firegl_get_addr_from_vm(vma);
    if (!kaddr)
    {
        return NOPAGE_SIGBUS; /* bad address */
    }
    kaddr += (address - vma->vm_start);

    __KE_DEBUG3("kaddr=0x%08lx\n", kaddr);

    pMmPage = virt_to_page(kaddr);
    __KE_DEBUG3("pMmPage=0x%08lx\n", (unsigned long)pMmPage);

    get_page(pMmPage); /* inc usage count of page */

    __KE_DEBUG3("vm-address 0x%08lx => kernel-page-address 0x%p\n", address, page_address(pMmPage));

    return pMmPage;
}

/** 
 **
 **  This routine is intented to locate the page table through the 
 **  pagelist table created earlier in dev-> pcie
 **/
static __inline__ vm_nopage_ret_t do_vm_pcie_nopage(struct vm_area_struct* vma,
                                                         unsigned long address)
{

    unsigned long vma_offset;
    unsigned long i; 
    mem_map_t* pMmPage;
    struct firegl_pcie_mem* pciemem;
    unsigned long* pagelist;
    
    drm_device_t *dev = (drm_device_t *)firegl_get_dev_from_vm(vma);
    if (dev == NULL)
    {
        __KE_ERROR("dev is NULL\n");
        return NOPAGE_SIGBUS;
    }

    if (address > vma->vm_end)
    {
        __KE_ERROR("address out of range\n");
        return NOPAGE_SIGBUS; /* address is out of range */
    }
    pciemem = firegl_get_pciemem_from_addr ( vma, address);
    if (pciemem == NULL)
    {
        __KE_ERROR("No pciemem found! \n");
        return NOPAGE_SIGBUS;
    }    
    pagelist = firegl_get_pagelist_from_vm(vma);

    if (pagelist == NULL) 
    {
        __KE_ERROR("No pagelist! \n");
        return NOPAGE_SIGBUS;
    }
     
    /** Find offset in  vma */
    vma_offset = address - vma->vm_start;
    /** Which entry in the pagelist */
    i = vma_offset >> PAGE_SHIFT;
    pMmPage = virt_to_page(firegl_get_pcie_pageaddr_from_vm(vma,pciemem, i));

    get_page(pMmPage);

    if (page_address(pMmPage) == 0x0)
    {
        __KE_ERROR("Invalid page address\n");
        return NOPAGE_SIGBUS;
    }
    return pMmPage;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)

static vm_nopage_ret_t vm_nopage(struct vm_area_struct* vma,
                                 unsigned long address,
                                 int *type)
{
    if (type) *type = VM_FAULT_MINOR;
        return do_vm_nopage(vma, address);
}

#ifdef __AGP__BUILTIN__
#ifdef __ia64__


static vm_nopage_ret_t vm_cant_nopage(struct vm_area_struct* vma,
                                      unsigned long address,
                                      int *type)
{
    if (type) *type = VM_FAULT_MINOR;
        return do_cant_nopage(vma, address);

}
#endif /* __ia64__ */
#endif /* __AGP__BUILTIN__ */

/*

    This function is called when a page of a mmap()'ed area is not currently
    visible in the specified VMA.
    Return value is the associated physical address for the requested page.
    (If not implemented, then the kernel default routine would allocate a new,
     zeroed page for servicing us)

    Possible errors: SIGBUS, OutOfMem

    This routine is intended to remap addresses of SHM SAREA
    (which is one or more pages in size)

 */
static vm_nopage_ret_t vm_shm_nopage(struct vm_area_struct* vma,
                                     unsigned long address,
                                     int *type)
{
    if (type) *type = VM_FAULT_MINOR;
        return do_vm_shm_nopage(vma, address);
}

/*

    This routine is intended to remap addresses of a OpenGL context
      (which is one ore more pages in size)

*/
static vm_nopage_ret_t vm_dma_nopage(struct vm_area_struct* vma,
                                     unsigned long address,
                                     int *type)
{
    if (type) *type = VM_FAULT_MINOR;
        return do_vm_dma_nopage(vma, address);
}

static vm_nopage_ret_t vm_kmap_nopage(struct vm_area_struct* vma,
                                     unsigned long address,
                                     int *type)
{
    if (type) *type = VM_FAULT_MINOR;
        return do_vm_kmap_nopage(vma, address);
}

static vm_nopage_ret_t vm_pcie_nopage(struct vm_area_struct* vma,
                                     unsigned long address,
                                     int *type)
{  
       return do_vm_pcie_nopage(vma, address);
}

#else   /* LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,0) */

static vm_nopage_ret_t vm_nopage(struct vm_area_struct* vma,
                                 unsigned long address,
                                 int write_access)
{
    return do_vm_nopage(vma, address);
}

#ifdef __AGP__BUILTIN__
#ifdef __ia64__


static vm_nopage_ret_t vm_cant_nopage(struct vm_area_struct* vma,
                                 unsigned long address,
                                 int write_access)
{
    return do_vm_cant_nopage(vma, address);
}
#endif /* __ia64__ */
#endif /* __AGP__BUILTIN__ */

/*

    This function is called when a page of a mmap()'ed area is not currently
    visible in the specified VMA.
    Return value is the associated physical address for the requested page.
    (If not implemented, then the kernel default routine would allocate a new,
     zeroed page for servicing us)

    Possible errors: SIGBUS, OutOfMem

    This routine is intended to remap addresses of SHM SAREA
    (which is one or more pages in size)

 */
static vm_nopage_ret_t vm_shm_nopage(struct vm_area_struct* vma,
                                     unsigned long address,
                                     int write_access)
{
    return do_vm_shm_nopage(vma, address);
}

/*

    This routine is intended to remap addresses of a OpenGL context
      (which is one ore more pages in size)

*/
static vm_nopage_ret_t vm_dma_nopage(struct vm_area_struct* vma,
                                     unsigned long address,
                                     int write_access)
{
     return do_vm_dma_nopage(vma, address);
}

static vm_nopage_ret_t vm_kmap_nopage(struct vm_area_struct* vma,
                                     unsigned long address,
                                     int write_access)
{
     return do_vm_kmap_nopage(vma, address);
}

static vm_nopage_ret_t vm_pcie_nopage(struct vm_area_struct* vma,
                                     unsigned long address,
                                     int write_access)
{
        return do_vm_pcie_nopage(vma, address);
}

#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0) */

void* ATI_API_CALL __ke_vma_file_priv(struct vm_area_struct* vma)
{
    return vma->vm_file->private_data;
}

unsigned long ATI_API_CALL __ke_vm_start(struct vm_area_struct* vma)
{
    return vma->vm_start;
}

unsigned long ATI_API_CALL __ke_vm_end(struct vm_area_struct* vma)
{
    return vma->vm_end;
}

unsigned long ATI_API_CALL __ke_vm_offset(struct vm_area_struct* vma)
{
#if LINUX_VERSION_CODE < 0x020319
    return vma->vm_offset;
#else /* LINUX_VERSION_CODE >= 0x020319 */
    return vma->vm_pgoff << PAGE_SHIFT;
#endif/* LINUX_VERSION_CODE >= 0x020319 */
}

char* ATI_API_CALL __ke_vm_flags_str(struct vm_area_struct* vma, char* buf)
{
   *(buf + 0) = vma->vm_flags & VM_READ	    ? 'r' : '-';
   *(buf + 1) = vma->vm_flags & VM_WRITE	? 'w' : '-';
   *(buf + 2) = vma->vm_flags & VM_EXEC	    ? 'x' : '-';
   *(buf + 3) = vma->vm_flags & VM_MAYSHARE ? 's' : 'p';
   *(buf + 4) = vma->vm_flags & VM_LOCKED   ? 'l' : '-';
   *(buf + 5) = vma->vm_flags & VM_IO	    ? 'i' : '-';
   *(buf + 6) = 0;
   return buf;
}

char* ATI_API_CALL __ke_vm_page_prot_str(struct vm_area_struct* vma, char* buf)
{
    int i = 0;

#ifdef __i386__
	unsigned int pgprot;

    pgprot = pgprot_val(vma->vm_page_prot);
    *(buf + i++) = pgprot & _PAGE_PRESENT  ? 'p' : '-';
    *(buf + i++) = pgprot & _PAGE_RW       ? 'w' : 'r';
    *(buf + i++) = pgprot & _PAGE_USER     ? 'u' : 's';
    *(buf + i++) = pgprot & _PAGE_PWT      ? 't' : 'b';
    *(buf + i++) = pgprot & _PAGE_PCD      ? 'u' : 'c';
    *(buf + i++) = pgprot & _PAGE_ACCESSED ? 'a' : '-';
    *(buf + i++) = pgprot & _PAGE_DIRTY    ? 'd' : '-';
    *(buf + i++) = pgprot & _PAGE_PSE      ? 'm' : 'k';
    *(buf + i++) = pgprot & _PAGE_GLOBAL   ? 'g' : 'l';
#endif /* __i386__ */		
    *(buf + i++) = 0;

    return buf;
}

static
char *__ke_pte_phys_addr_str(pte_t pte, char *buf, __ke_dma_addr_t* phys_address)
{
    if (pte_present(pte))
    {
#if defined(__x86_64__) || defined(__ia64__)
        *phys_address = pte.pte & PAGE_MASK;
#else
        *phys_address = pte_val(pte) & (u64)((u64)PAGE_MASK | (u64)0xf<<32);
#endif
        sprintf(buf, "0x%Lx %c%c%c%c%c%c\n",
           *phys_address,
           pte_present (pte) ? 'p' : '-',
           pte_read    (pte) ? 'r' : '-',
           pte_write   (pte) ? 'w' : '-',
           pte_exec    (pte) ? 'x' : '-',
           pte_dirty   (pte) ? 'd' : '-',
           pte_young   (pte) ? 'a' : '-');
    }
    else
        *buf = 0;

    return buf;
}

char* ATI_API_CALL __ke_vm_phys_addr_str(struct vm_area_struct* vma, 
                            char* buf, 
                            unsigned long virtual_addr, 
                            __ke_dma_addr_t* phys_address)
{
    pgd_t* pgd_p;
    pmd_t* pmd_p;
    pte_t  pte;

    PGD_OFFSET(vma->vm_mm, pgd_p, virtual_addr);
    PGD_PRESENT(pgd_p);
    PMD_OFFSET(pmd_p, pgd_p, virtual_addr);
    PMD_PRESENT(pmd_p);
    PTE_OFFSET(pte, pmd_p, virtual_addr);
    PTE_PRESENT(pte);

    return __ke_pte_phys_addr_str(pte, buf, phys_address);
}

void ip_drm_vm_open(struct vm_area_struct* vma)
{ drm_vm_open(vma); }
void ip_drm_vm_close(struct vm_area_struct* vma)
{ drm_vm_close(vma); }

static struct vm_operations_struct vm_ops =
{
    nopage:  vm_nopage,
    open:    ip_drm_vm_open,
    close:   ip_drm_vm_close,
};

#ifdef __AGP__BUILTIN__
#ifdef __ia64__
static struct vm_operations_struct vm_cant_ops =
{
    nopage:  vm_cant_nopage,
    open:    ip_drm_vm_open,
    close:   ip_drm_vm_close,
};
#endif /* __ia64_ */
#endif /* __AGP__BUILTIN__ */

static struct vm_operations_struct vm_shm_ops =
{
    nopage:  vm_shm_nopage,
    open:    ip_drm_vm_open,
    close:   ip_drm_vm_close,
};

static struct vm_operations_struct vm_pci_bq_ops =
{
    nopage:  vm_dma_nopage,
    open:    ip_drm_vm_open,
    close:   ip_drm_vm_close,
};

static struct vm_operations_struct vm_ctx_ops =
{
    nopage:  vm_dma_nopage,
    open:    ip_drm_vm_open,
    close:   ip_drm_vm_close,
};

static struct vm_operations_struct vm_pcie_ops = 
{
    nopage:  vm_pcie_nopage,
    open:    ip_drm_vm_open,
    close:   ip_drm_vm_close,
};

static struct vm_operations_struct vm_kmap_ops =
{
    nopage:  vm_kmap_nopage,
    open:    ip_drm_vm_open,
    close:   ip_drm_vm_close,
};

#ifdef __AGP__BUILTIN__
#ifndef __ia64__
static struct vm_operations_struct vm_agp_bq_ops =
{
    nopage:  vm_nopage,
    open:    ip_drm_vm_open,
    close:   ip_drm_vm_close,
};
#else		
static struct vm_operations_struct vm_cant_agp_bq_ops =
{
    nopage:  vm_cant_nopage,
    open:    ip_drm_vm_open,
    close:   ip_drm_vm_close,
};
#endif /* __ia64_ */
#endif /* __AGP__BUILTIN__ */

int ATI_API_CALL __ke_vm_map(struct file* filp,
                             struct vm_area_struct* vma, unsigned long offset,
                             enum __ke_vm_maptype type,
                             int readonly)
{
    unsigned int pages;

    __KE_DEBUG3("start=0x%08lx, "
            "end=0x%08lx, "
            "offset=0x%08lx\n",
            vma->vm_start,
            vma->vm_end,
            (unsigned long)offset);

    switch (type)
    {
        case __KE_ADPT:
			{
#ifdef __ia64__
			struct page 	*page = virt_to_page((unsigned long)__va(VM_OFFSET(vma)));
			if (!VALID_PAGE(page) || PageReserved(page))
#else
            if (offset >= __pa(high_memory))
#endif
            {
#ifdef __i386__
                if (boot_cpu_data.x86 > 3)
                {
                    pgprot_val(vma->vm_page_prot) |= _PAGE_PCD;
                    pgprot_val(vma->vm_page_prot) &= ~_PAGE_PWT;
                }
#endif /* __i386__ */
#ifdef __ia64__
				vma->vm_page_prot =
					pgprot_writecombine(vma->vm_page_prot);
#endif /* __ia64__ */
                vma->vm_flags |= VM_IO; /* not in core dump */
            }
            if (REMAP_PAGE_RANGE(vma,offset))
            {
                __KE_DEBUG(REMAP_PAGE_RANGE_STR " failed\n");
                return -EAGAIN;
            }
            vma->vm_flags |= VM_SHM | VM_RESERVED; /* Don't swap */
            vma->vm_ops = &vm_ops;
            }
			break;

        case __KE_SHM:
            vma->vm_flags |= VM_SHM | VM_RESERVED; /* Don't swap */
            vma->vm_ops = &vm_shm_ops;
            break;

        case __KE_SG:

            pages = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;

#if LINUX_VERSION_CODE <= 0x02040e /* KERNEL_VERSION(2,4,14) */
            vma->vm_flags |= VM_LOCKED | VM_SHM; /* Don't swap */
#else
            vma->vm_flags |= VM_RESERVED;
#endif

            //vma->vm_flags |=  VM_SHM | VM_LOCKED; /* DDDDDDDDDDon't swap */
            //vma->vm_mm->locked_vm += pages; /* Kernel tracks aqmount of locked pages */
            vma->vm_ops = &vm_pcie_ops;
            break;

        case __KE_CTX:
            pages = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
            vma->vm_flags |= VM_LOCKED | VM_SHM | VM_RESERVED; /* Don't swap */
            vma->vm_mm->locked_vm += pages; /* Kernel tracks aqmount of locked pages */
            vma->vm_ops = &vm_ctx_ops;
            break;

        case __KE_PCI_BQS:
            pages = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
            vma->vm_flags |= VM_LOCKED | VM_SHM | VM_RESERVED; /* Don't swap */
            vma->vm_mm->locked_vm += pages; /* Kernel tracks aqmount of locked pages */
            vma->vm_ops = &vm_pci_bq_ops;
            break;

#ifdef __AGP__BUILTIN__
        case __KE_AGP:
            // if(dev->agp->cant_use_aperture == 1) 
#ifdef __ia64__
            {
                /*
                 * On some systems we can't talk to bus dma address from
                 * the CPU, so for memory of type DRM_AGP, we'll deal
                 * with sorting out the real physical pages and mappings
                 * in our page fault handler
                 */
                vma->vm_flags |= VM_SHM | VM_RESERVED; /* Don't swap */
                vma->vm_ops = &vm_cant_ops;
            }
#else
            // else
            {
                if (offset >= __pa(high_memory))
                    vma->vm_flags |= VM_IO; /* not in core dump */
                if (REMAP_PAGE_RANGE(vma,offset))
                {
                    __KE_DEBUG(REMAP_PAGE_RANGE_STR " failed\n");
                    return -EAGAIN;
                }
#ifdef __x86_64__
                vma->vm_flags |= VM_RESERVED;
#else
                vma->vm_flags |= VM_SHM | VM_RESERVED; /* Don't swap */
#endif
                vma->vm_ops = &vm_ops;
            }
#endif
            break;
        case __KE_AGP_BQS:
            // if(dev->agp->cant_use_aperture == 1) 
#ifdef __ia64__			
            {
                /*
                 * On some systems we can't talk to bus dma address from
                 * the CPU, so for memory of type DRM_AGP, we'll deal
                 * with sorting out the real physical pages and mappings
                 * in our page fault handler
                 */
                vma->vm_flags |= VM_SHM | VM_RESERVED; /* Don't swap */
                vma->vm_ops = &vm_cant_agp_bq_ops;
            }
            // else
#else
            {
                if (offset >= __pa(high_memory))
                    vma->vm_flags |= VM_IO; /* not in core dump */
                if (REMAP_PAGE_RANGE(vma,offset))
                {
                    __KE_DEBUG(REMAP_PAGE_RANGE_STR " failed\n");
                    return -EAGAIN;
                }
#ifdef __x86_64__
                vma->vm_flags |= VM_RESERVED;
#else
                vma->vm_flags |= VM_SHM | VM_RESERVED; /* Don't swap */
#endif
                vma->vm_ops = &vm_agp_bq_ops;
            }
#endif
            break;
#endif /* __AGP__BUILTIN__ */

        case __KE_KMAP:
		    vma->vm_flags |= VM_SHM | VM_RESERVED;
            vma->vm_ops = &vm_kmap_ops;
            if (readonly && (vma->vm_flags & VM_WRITE))
            {
                __KE_DEBUG("ERROR: cannot map a readonly map with PROT_WRITE!\n");
                return -EINVAL; // write not allowed - explicitly fail the map!
            }
            break;

        default:
            /*  This should never happen anyway! */
            __KE_ERROR("__ke_vm_map: Unknown type %d\n", type);
            return -EINVAL;
    }

    if (readonly)
    {
        vma->vm_flags &= ~(VM_WRITE | VM_MAYWRITE);
#if defined(__i386__)
		pgprot_val(vma->vm_page_prot) &= ~_PAGE_RW;
#else
		vma->vm_page_prot = __pgprot(pte_val(pte_wrprotect(
			__pte(pgprot_val(vma->vm_page_prot)))));
#endif
    }

    vma->vm_file = filp;    /* Needed for drm_vm_open() */

    return 0;
}

int __ke_agp_try_unsupported = 0;

#define _X(_x) __fgl_##_x
extern int _X(agp_init)(void);
extern void _X(agp_cleanup)(void);
extern int _X(agp_try_unsupported);
extern struct _agp_memory *_X(agp_allocate_memory_phys_list)(size_t, u32, u64 *);

#define FIREGL_agp_init             _X(agp_init)
#define FIREGL_agp_cleanup          _X(agp_cleanup)
#define FIREGL_agp_free_memory      _X(agp_free_memory)
#define FIREGL_agp_allocate_memory  _X(agp_allocate_memory)
#ifdef FGL
#define FIREGL_agp_allocate_memory_phys_list    _X(agp_allocate_memory_phys_list)
#endif
#define FIREGL_agp_bind_memory      _X(agp_bind_memory)
#define FIREGL_agp_unbind_memory    _X(agp_unbind_memory)
#define FIREGL_agp_enable           _X(agp_enable)
#define FIREGL_agp_backend_acquire  _X(agp_backend_acquire)
#define FIREGL_agp_backend_release  _X(agp_backend_release)
#define FIREGL_agp_memory           _X(agp_memory)

unsigned int __ke_firegl_agpgart_inuse = AGPGART_INUSE_NONE;				

#if defined(CONFIG_AGP) || defined(CONFIG_AGP_MODULE)
/*****************************************************************************/

#if LINUX_VERSION_CODE >= 0x02060b

typedef struct {
	void			(*free_memory)(struct agp_memory *);
	struct agp_memory *	(*allocate_memory)(size_t, u32);
	int			(*bind_memory)(struct agp_memory *, off_t);
	int			(*unbind_memory)(struct agp_memory *);
	void			(*enable)(u32);
	int			(*acquire)(void);
	void			(*release)(void);
	int			(*copy_info)(struct agp_kern_info *);
} drm_agp_t;

#if LINUX_VERSION_CODE >= 0x02060c

/* In Linux >= 2.6.12, due to support for multiple AGP bridges, some
 * AGP functions need a pointer to the AGP bridge. __ke_agp_acquire
 * stores a pointer to the pci device in a global
 * variable. firegl_wrap_agp_backend_acquire below uses it to retrieve
 * the pointer to the bridge and stores it in the global variable
 * firegl_agp_bridge. All AGP functions that need the bridge pointer
 * are wrapped here and get the global bridge pointer. */
static struct agp_bridge_data *firegl_agp_bridge = NULL;
static struct pci_dev *firegl_pci_device = NULL;

static struct agp_memory *
firegl_wrap_agp_allocate_memory (size_t pg_count, u32 type)
{
        return agp_allocate_memory (firegl_agp_bridge, pg_count, type);
}

static void firegl_wrap_agp_enable (u32 mode)
{
	agp_enable (firegl_agp_bridge, mode);
}

static int firegl_wrap_agp_backend_acquire (void)
{
        firegl_agp_bridge = agp_backend_acquire (firegl_pci_device);
        return firegl_agp_bridge != NULL ? 0 : -EBUSY;
}

static void firegl_wrap_agp_backend_release (void)
{
        agp_backend_release (firegl_agp_bridge);
        firegl_agp_bridge = NULL;
}

static int firegl_wrap_agp_copy_info (struct agp_kern_info *kinfo)
{
        return agp_copy_info (firegl_agp_bridge, kinfo);
}


static const drm_agp_t drm_agp = {
	&agp_free_memory,
	&firegl_wrap_agp_allocate_memory,
	&agp_bind_memory,
	&agp_unbind_memory,
	&firegl_wrap_agp_enable,
	&firegl_wrap_agp_backend_acquire,
	&firegl_wrap_agp_backend_release,
	&firegl_wrap_agp_copy_info
};


#else

static const drm_agp_t drm_agp = {
	&agp_free_memory,
	&agp_allocate_memory,
	&agp_bind_memory,
	&agp_unbind_memory,
	&agp_enable,
	&agp_backend_acquire,
	&agp_backend_release,
	&agp_copy_info
};

#endif /* >= 2.6.12 */

#else

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)

static const drm_agp_t drm_agp = {
	&agp_free_memory,
	&agp_allocate_memory,
	&agp_bind_memory,
	&agp_unbind_memory,
	&agp_enable,
	&agp_backend_acquire,
	&agp_backend_release,
	&agp_copy_info
};

#else

extern drm_agp_t drm_agp;

#endif

#endif

static const drm_agp_t  *drm_agp_module_stub = NULL;

#define AGP_FUNCTIONS		8
#define AGP_AVAILABLE(func)	(drm_agp_module_stub && drm_agp_module_stub-> func )
#define AGP_FUNC(func)		(*drm_agp_module_stub-> func )
// note: avoid ##-tokens with latest GCC (i.e. RedHat 7.1 beta)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#define DRM_AGP_MODULE_GET      (drm_agp_t *)inter_module_get_request("drm_agp","drm_agp")
#define DRM_AGP_MODULE_PUT      inter_module_put("drm_agp")


static
int ATI_API_CALL __ke_firegl_agpgart_available(void)
{
    int retval;

    if (__ke_agp_try_unsupported)
        _X(agp_try_unsupported) = __ke_agp_try_unsupported;

    retval = FIREGL_agp_init();
    if (retval) {
        __KE_INFO("Initialization of built-in AGP-support failed (ret=%d).\n", retval);
    	__ke_firegl_agpgart_inuse = AGPGART_INUSE_NONE; 	
        return 0;
    }

    __ke_firegl_agpgart_inuse = INTERNAL_AGPGART_INUSE; 	
    drm_agp_module_stub = &drm_agp;	
    __KE_INFO("Initialization of built-in AGP-support successful.\n");
    return 1;
}


static
int ATI_API_CALL __ke_agpgart_available(__ke_pci_dev_t *pcidev, int use_internal)
{
    unsigned int found = 0;

    drm_agp_module_stub = DRM_AGP_MODULE_GET;

    if (!drm_agp_module_stub)
    {
        __KE_DEBUG("getting module stub failed for AGP/GART kernel module\n");
        return 0; /* failed */
    }

    __ke_firegl_agpgart_inuse = KERNEL24_AGPGART_INUSE; 	

    if (drm_agp_module_stub->free_memory) {
        __KE_DEBUG("agp_free_memory resolves to 0x%08lx\n", drm_agp_module_stub->free_memory);
        found++;
    }
    if (drm_agp_module_stub->allocate_memory) {
       __KE_DEBUG("agp_allocate_memory resolves to 0x%08lx\n", drm_agp_module_stub->allocate_memory);
       found++;
    }
    if (drm_agp_module_stub->bind_memory) {
       __KE_DEBUG("agp_bind_memory resolves to 0x%08lx\n", drm_agp_module_stub->bind_memory);
       found++;
    }
    if (drm_agp_module_stub->unbind_memory) {
       __KE_DEBUG("agp_unbind_memory resolves to 0x%08lx\n", drm_agp_module_stub->unbind_memory);
       found++;
    }
    if (drm_agp_module_stub->enable) {
       __KE_DEBUG("agp_enable resolves to 0x%08lx\n", drm_agp_module_stub->enable);
       found++;
    }
    if (drm_agp_module_stub->acquire) {
       __KE_DEBUG("agp_acquire resolves to 0x%08lx\n", drm_agp_module_stub->acquire);
       found++;
    }
    if (drm_agp_module_stub->release) {
       __KE_DEBUG("agp_release resolves to 0x%08lx\n", drm_agp_module_stub->release);
       found++;
    }
    if (drm_agp_module_stub->copy_info) {
       __KE_DEBUG("agp_copy_info resolves to 0x%08lx\n", drm_agp_module_stub->copy_info);
       found++;
    }

    if (found == AGP_FUNCTIONS)
    {
        if (AGP_FUNC(acquire)() == 0)
        {
           AGP_FUNC(release)();
           return 1; /* success */
        }
    }

    if (found == 0)
        __KE_DEBUG("AGP/GART kernel module not found\n");
    else
    {
        __KE_DEBUG("AGP/GART kernel module present but API is incomplete\n");

        /* do a complete cleanup of our entry point table now */
        __ke_agp_uninit();
    }

    return 0; /* failed */
}

#else  // !LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

static
int ATI_API_CALL __ke_agpgart_available(__ke_pci_dev_t *pcidev, int use_internal)
{
    drm_agp_module_stub = &drm_agp;
    __ke_firegl_agpgart_inuse = KERNEL26_AGPGART_INUSE;	
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
        firegl_pci_device = (struct pci_dev*)(void*)pcidev;
#endif
        if (AGP_FUNC(acquire)() == 0)
        {
            AGP_FUNC(release)();
            return 1; /* success */
        }
    }

    {
        __KE_DEBUG("AGP/GART kernel module present but API is incomplete\n");

        /* do a complete cleanup of our entry point table now */
        __ke_agp_uninit();
    }

    return 0; /* failed */
}

static
int ATI_API_CALL __ke_firegl_agpgart_available(void)
{
     __KE_INFO("Internal AGP is not supported in 2.6 kernel.\n");
     return 0;
}

#endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

int ATI_API_CALL __ke_agp_available(__ke_pci_dev_t *pcidev, int use_internal)
{
    int available = __ke_agpgart_available(pcidev, use_internal);

    if ( available )
    {
        if ( use_internal )
        {
            __KE_INFO("Internal AGP support requested, but kernel AGP support active.\n");
            __KE_INFO("Have to use kernel AGP support to avoid conflicts.\n");
        }

        if ( __ke_moduleflags & __KE_FLAG_DISABLE_AGPGART )
        {
            __KE_ERROR("Have to use kernel AGP support, but module parameters forbid that\n");
            __ke_agp_uninit();
            available = 0;
        }
    } else {
         available = __ke_firegl_agpgart_available();
    }
	
    return available;
}

void ATI_API_CALL __ke_agp_uninit(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
    if( (__ke_firegl_agpgart_inuse & INTERNAL_AGPGART_INUSE) ) {
        FIREGL_agp_cleanup();
    } else if( (__ke_firegl_agpgart_inuse & KERNEL24_AGPGART_INUSE) && drm_agp_module_stub ) {	
        DRM_AGP_MODULE_PUT;
    }
#endif

    __ke_firegl_agpgart_inuse = AGPGART_INUSE_NONE;
    drm_agp_module_stub = NULL;
}

void ATI_API_CALL __ke_agp_free_memory(struct _agp_memory* handle)
{
    if (AGP_AVAILABLE(free_memory))
       AGP_FUNC(free_memory)(handle);
}

struct _agp_memory* ATI_API_CALL __ke_agp_allocate_memory(__ke_size_t pages, unsigned long type)
{
    if (AGP_AVAILABLE(allocate_memory))
       return AGP_FUNC(allocate_memory)(pages, type);
    else
       return NULL;
}

int ATI_API_CALL __ke_agp_bind_memory(struct _agp_memory* handle, __ke_off_t start)
{
    if (AGP_AVAILABLE(bind_memory))
       return AGP_FUNC(bind_memory)(handle, start);
    else
       return -EINVAL;
}

int ATI_API_CALL __ke_agp_unbind_memory(struct _agp_memory* handle)
{
    if (AGP_AVAILABLE(unbind_memory))
       return AGP_FUNC(unbind_memory)(handle);
    else
       return -EINVAL;
}

int ATI_API_CALL __ke_agp_enable(unsigned long mode)
{
    if (AGP_AVAILABLE(enable)) {
       AGP_FUNC(enable)(mode);
       return 0;
    } else
       return -EINVAL;
}

int ATI_API_CALL __ke_read_agp_caps_registers(__ke_pci_dev_t* dev, unsigned int *caps)
{
    u8 capndx;
    u32 cap_id;

    if (!caps)
        return -EINVAL;

    if (!(struct pci_dev*)(void *)dev)
        return -ENODEV;

    pci_read_config_byte((struct pci_dev*)(void *)dev, 0x34, &capndx);
    if (capndx == 0x00)
        return -ENODATA;

    do { // search capability list for AGP caps
        pci_read_config_dword((struct pci_dev*)(void *)dev, capndx, &cap_id);
        if ((cap_id & 0xff) == 0x02)
        {
            pci_read_config_dword((struct pci_dev*)(void *)dev, capndx + 0, &(caps[0])); /* AGP CAPPTR */
            pci_read_config_dword((struct pci_dev*)(void *)dev, capndx + 4, &(caps[1])); /* AGP STATUS */
            pci_read_config_dword((struct pci_dev*)(void *)dev, capndx + 8, &(caps[2])); /* AGP COMMAND */

            return 0; /* success */
        }
        capndx = (cap_id >> 8) & 0xff;
    } while (capndx != 0x00);

    return -ENODATA;
}

int ATI_API_CALL __ke_agp_acquire(__ke_pci_dev_t* dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
    firegl_pci_device = (struct pci_dev*)(void*)dev;
#endif
    if (AGP_AVAILABLE(acquire))
       return AGP_FUNC(acquire)();
    else
       return -EINVAL;
}

void ATI_API_CALL __ke_agp_release(void)
{
    if (AGP_AVAILABLE(release))
       AGP_FUNC(release)();
}

void ATI_API_CALL __ke_agp_copy_info(__ke_agp_kern_info_t* info)
{
    struct pci_dev *device = NULL;

    memset(info, 0, sizeof(__ke_agp_kern_info_t));

    if (AGP_AVAILABLE(copy_info))
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,71)
        struct
#endif
        agp_kern_info kern;    

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
        if (firegl_agp_bridge == NULL)
        {
                AGP_FUNC(acquire)();
                AGP_FUNC(copy_info)(&kern);
                AGP_FUNC(release)();
        }
        else
#endif
                AGP_FUNC(copy_info)(&kern);
        device = kern.device;

        info->version.major = kern.version.major;
        info->version.minor = kern.version.minor;
        if( kern.device )
        {
            info->vendor = kern.device->vendor;
            info->device = kern.device->device;
        }
        info->mode = kern.mode;
        info->aper_base = kern.aper_base;
        info->aper_size = kern.aper_size;
        info->max_memory = kern.max_memory;
        info->current_memory = kern.current_memory;
#if LINUX_VERSION_CODE <= 0x020408
        info->cant_use_aperture = 0;
        info->page_mask = ~(0xfff);
#else
		info->cant_use_aperture = kern.cant_use_aperture;
		info->page_mask = kern.page_mask;
#endif
    }

    /* FGL_FIX: some chipset drivers do not read the mode member from hardware */
    if( device )
    {
        if( !info->mode )
        {
            u8 capptr;

            capptr = pci_find_capability(device, PCI_CAP_ID_AGP);
            if( capptr )
            {
                u32 tmp;
                pci_read_config_dword(device,
                    capptr + PCI_AGP_STATUS, &tmp);

                info->mode = tmp; /* note: unsigned int (32/64) = u32; */
            }
        }
    }
}

unsigned long ATI_API_CALL __ke_agp_memory_handle(struct _agp_memory* handle)
{

    return (unsigned long)handle->memory;
}

unsigned long ATI_API_CALL __ke_agp_memory_page_count(struct _agp_memory* handle)
{
    return handle->page_count;
}

#else //!defined(CONFIG_AGP) || defined(CONFIG_AGP_MODULE)

int ATI_API_CALL __ke_agp_available(__ke_pci_dev_t *pcidev, int use_internal)
{
    return 0;
}

void ATI_API_CALL __ke_agp_uninit( void )                { }


void ATI_API_CALL __ke_agp_free_memory(struct _agp_memory* handle)         { }

struct _agp_memory* ATI_API_CALL __ke_agp_allocate_memory(__ke_size_t pages, unsigned long type)
{
    return NULL;
}

int ATI_API_CALL __ke_agp_bind_memory(struct _agp_memory* handle, __ke_off_t start)
{
    return -EINVAL;
}

int ATI_API_CALL __ke_agp_unbind_memory(struct _agp_memory* handle)
{
    return -EINVAL;
}

int ATI_API_CALL __ke_agp_enable(unsigned long mode)
{
    return -EINVAL;
}

int ATI_API_CALL __ke_read_agp_caps_registers(__ke_pci_dev_t* dev, unsigned int *caps)
{
    return -EINVAL;	
}

int ATI_API_CALL __ke_agp_acquire(__ke_pci_dev_t* dev)
{
    return -EINVAL;
}

void ATI_API_CALL __ke_agp_release(void)   { }

int ATI_API_CALL __ke_agp_read_caps_registers(__ke_pci_dev_t* dev, unsigned int *caps)
{
    return -EINVAL;
}

void ATI_API_CALL __ke_agp_copy_info(__ke_agp_kern_info_t* info)             {  }

unsigned long ATI_API_CALL __ke_agp_memory_handle(struct _agp_memory* handle)
{
    return 0;
}

unsigned long ATI_API_CALL __ke_agp_memory_page_count(struct _agp_memory* handle)
{
    return 0;
}

#endif //defined(CONFIG_AGP) || defined(CONFIG_AGP_MODULE)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)

struct _agp_memory* ATI_API_CALL __ke_agp_allocate_memory_phys_list(
    __ke_size_t pages, unsigned long type, __ke_dma_addr_t *phys_addr)
{
    return NULL;
}

#else // LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#ifdef FGL
struct _agp_memory * ATI_API_CALL __ke_agp_allocate_memory_phys_list(__ke_size_t pages, unsigned long type, __ke_dma_addr_t * phys_addr)
{
    return (struct _agp_memory *)FIREGL_agp_allocate_memory_phys_list(pages, type, phys_addr);
}
#endif

#endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)

int ATI_API_CALL __ke_smp_processor_id(void)
{
	return (int)(smp_processor_id());
}


void ATI_API_CALL __ke_smp_call_function( void (*ATI_API_CALL func)(void *info) )
{
#ifdef CONFIG_SMP
	smp_call_function( func, NULL, 0, 1 );
#endif
}

#if !defined(__x86_64__) && !defined(__ia64__)

#define vendorStringLen 12

static inline int check_cpu_vendor(const char* vendorName)
{
    int sVendor[vendorStringLen/sizeof(int)];
    __asm
    (
        "\txor %%eax, %%eax\n"  // function #0: get vendor string
        "\tcpuid\n"             // perform CPUID
        "\tmov %%ebx, %0\n"     // store 3 times 4 bytes of vendor string
        "\tmov %%edx, %1\n"
        "\tmov %%ecx, %2\n"
        : "=m" (sVendor[0]), "=m" (sVendor[1]), "=m"(sVendor[2])   // results go to this location
        :                       // no input
        : "eax", "ebx", "ecx", "edx"    // modifiys this registers
    );

    if( strncmp((char*)sVendor,vendorName,vendorStringLen)==0 )
        return 1;   // thats a match
    else
        return 0;   // does not match
}

int ATI_API_CALL __ke_is_athlon(void)
{
    register int bAthlon;
    __asm
    (
        // Check for CPUID instruction
        "\tpushf\n"              	//; save EFLAGS
        "\tpop %%eax\n"             	//; store EFLAGS in EAX
        "\tmov %%eax, %%ebx\n"        	//; save in EBX for later testing
        "\txor $0x00200000, %%eax\n"  	//; toggle bit 21
        "\tpush %%eax\n"            	//; put to stack
        "\tpopf\n"               	//; save changed EAX to EFLAGS
        "\tpushf\n"              	//; push EFLAGS to TOS
        "\tpop %%eax\n"             	//; store EFLAGS in EAX
        "\tcmp %%ebx, %%eax\n"        	//; see if bit 21 has changed
        "\tjz NO_ATHLON\n"        	//; if no change, no CPUID

        // Check for extended functions
        "\tmov $0x80000000, %%eax\n"  	//; query for extended functions
        "\tCPUID\n"               	//; get extended function limit
        "\tcmp $0x80000000, %%eax\n"  	//; is 8000_0001h supported?
        "\tjbe NO_ATHLON\n"       	//; if not, 3DNow! command set not supported

        "\tmov $0x80000001, %%eax\n"  	//; setup extended function 1
        "\tCPUID\n"               	//; call the function
        "\ttest $0x80000000, %%edx\n" 	//; test bit 31
        "\tjz NO_ATHLON\n"        	//; 3DNow! command set not supported
        "\tmov $1, %%eax\n"
        "\tjmp DONE\n"
"NO_ATHLON:\n"
        "\txor %%eax, %%eax\n"
"DONE:\n"
        "\tmov %%eax, %0\n"         //; store to bAthlon
        // optimizer hints
        : "=r" (bAthlon)            // let compiler choose output register
        :                           // no input selection for compiler
        : "eax", "ebx", "ecx", "edx"	// modifiys
    );

    if( bAthlon )
    {
        if( !check_cpu_vendor("AuthenticAMD") )
            bAthlon = 0;
    }

    return bAthlon;
}

/*=======================================================================
 * amd_adv_spec_cache_feature
 *
 * Test if this is an AMD Athlon processor that exposes a conflicting
 * cache attribute bug in the kernel.
 =======================================================================*/

#if ( (PAGE_ATTR_FIX == 1) || (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,19)) )
// not used
#else
/* Standard macro to see if a specific flag is changeable */
static inline int flag_is_changeable_p(u32 flag)
{
	u32 f1, f2;
	asm("pushfl\n\t"
	    "pushfl\n\t"
	    "popl %0\n\t"
	    "movl %0,%1\n\t"
	    "xorl %2,%0\n\t"
	    "pushl %0\n\t"
	    "popfl\n\t"
	    "pushfl\n\t"
	    "popl %0\n\t"
	    "popfl\n\t"
	    : "=&r" (f1), "=&r" (f2)
	    : "ir" (flag));

	return ((f1^f2) & flag) != 0;
}


/* Probe for the CPUID instruction */
static int __init have_cpuid_p(void)
{
	return flag_is_changeable_p(X86_EFLAGS_ID);
}
#endif


int ATI_API_CALL __ke_amd_adv_spec_cache_feature(void)
{
#if ( (PAGE_ATTR_FIX == 1) || (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,19)) )
/* the kernel already does provide a fix for the AMD Athlon
   big page attribute / cache flush data consistency system bug on its own.
   (AMD claimed that CPU cache behaviour for such pages is not specified.)
   - kernel 2.4.19 has the early fix which just removes some page attributes.
   - later kernels do have implementeed the full big page split code fix.
*/
#else /* PAGE_ATTR_FIX */
/* the kernel does not provide any fix for the AMD problem. */
        char vendor_id[16];
        int ident;
        int family, model;
 
        /* Must have CPUID */
        if(!have_cpuid_p())
                goto donthave;
        if(cpuid_eax(0)<1)
                goto donthave;
        
        /* Must be x86 architecture */
        cpuid(0, &ident,  
                (int *)&vendor_id[0],
                (int *)&vendor_id[8],
                (int *)&vendor_id[4]);

        if (memcmp(vendor_id, "AuthenticAMD", 12)) 
               goto donthave;

        ident = cpuid_eax(1);
        family = (ident >> 8) & 0xf;
        model  = (ident >> 4) & 0xf;
        if (((family == 6)  && (model >= 6)) || (family == 15)) {
                return 1;
        }

donthave:
#endif /* PAGE_ATTR_FIX */
        return 0;
}

int ATI_API_CALL __ke_has_PSE(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
	if (test_bit(X86_FEATURE_PSE, &boot_cpu_data.x86_capability))
#else
	if (cpu_has_pse)
#endif
    {
		return 1;
	}
	return 0;
}

#endif /* !__x86_64__ */

#endif /* __KERNEL__ */

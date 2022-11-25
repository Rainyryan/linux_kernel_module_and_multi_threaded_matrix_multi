#include <linux/init.h> /* Needed for the macros */ 
#include <linux/kernel.h> /* Needed for pr_info() */ 
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/proc_fs.h> /* Necessary because we use the proc fs */ 
#include <linux/uaccess.h> /* for copy_from_user */ 
#include <linux/version.h>
#include <asm/thread_info.h>
#include <linux/string.h>
 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0) 
#define HAVE_PROC_OPS 
#endif 
 
#define PROCFS_MAX_SIZE 4096 
#define PROCFS_NAME "thread_info" 
 
/* This structure hold information about the /proc file */ 
static struct proc_dir_entry *our_proc_file; 
 
/* The buffer used to store character for this module */ 
static char procfs_buffer[PROCFS_MAX_SIZE]; 
 
/* The size of the buffer */ 
static unsigned long procfs_buffer_size = 0; 
 
/* This function is called then the /proc file is read */ 
static ssize_t procfile_read(struct file *filePointer, char __user *buffer, 
                             size_t buffer_length, loff_t *offset) 
{ 

    int len = strlen(procfs_buffer); 
    ssize_t ret = len; 
 
    if (*offset >= len || copy_to_user(buffer, procfs_buffer, procfs_buffer_size)) { 
        pr_info("copy_to_user failed\n"); 
        ret = 0; 
    } else { 
        pr_info("procfile read %s\n", filePointer->f_path.dentry->d_name.name); 
        *offset += len; 
    } 
    strcpy(procfs_buffer, "");
    return ret; 
} 
 
/* This function is called with the /proc file is written. */ 
static ssize_t procfile_write(struct file *file, const char __user *buff, 
                              size_t len, loff_t *off) 
{ 
    procfs_buffer_size = len;
    char temp[1000]; 
    if (procfs_buffer_size > PROCFS_MAX_SIZE) 
        procfs_buffer_size = PROCFS_MAX_SIZE; 
 
    if (copy_from_user(temp, buff, len)) 
        return -EFAULT; 
    
    
    unsigned long long tid;
    kstrtoull(buff, 10, &tid);
    sprintf(temp, "\tThreadID:%lld Time:%lld(ms) context switch times: %ld\n", tid, current->utime/1000000, current->nvcsw+current->nivcsw);
    strcat(procfs_buffer, temp);

    procfs_buffer_size = strlen(procfs_buffer);
    procfs_buffer[procfs_buffer_size & (PROCFS_MAX_SIZE - 1)] = '\0'; 
    *off += procfs_buffer_size; 
    pr_info("procfile write %s\n", procfs_buffer); 
 
    return procfs_buffer_size; 
} 
 
#ifdef HAVE_PROC_OPS 
static const struct proc_ops proc_file_fops = { 
    .proc_read = procfile_read, 
    .proc_write = procfile_write, 
}; 
#else 
static const struct file_operations proc_file_fops = { 
    .read = procfile_read, 
    .write = procfile_write, 
}; 
#endif 
 
static int __init procfs2_init(void) 
{ 
    our_proc_file = proc_create(PROCFS_NAME, 0777, NULL, &proc_file_fops); 
    if (NULL == our_proc_file) { 
        proc_remove(our_proc_file); 
        pr_alert("Error:Could not initialize /proc/%s\n", PROCFS_NAME); 
        return -ENOMEM; 
    } 
 
    pr_info("/proc/%s created\n", PROCFS_NAME); 
    return 0; 
} 
 
static void __exit procfs2_exit(void) 
{ 
    proc_remove(our_proc_file); 
    pr_info("/proc/%s removed\n", PROCFS_NAME); 
} 
 
module_init(procfs2_init); 
module_exit(procfs2_exit); 
 
MODULE_LICENSE("GPL");
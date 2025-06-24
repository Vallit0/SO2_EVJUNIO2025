#ifndef _SYSCALLS_PROJECT_H
#define _SYSCALLS_PROJECT_H

#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long sys_memory_info(struct mem_info __user *info);
asmlinkage long sys_swapinfo(struct swap_info __user *info);
asmlinkage long sys_pagefaults(struct page_faults_info __user *info);
asmlinkage long sys_memorypages(struct memory_pages_info __user *info);
asmlinkage long sys_top_memory_processes(struct process_info __user *top_procs);

#endif

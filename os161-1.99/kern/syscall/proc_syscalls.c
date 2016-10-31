#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <mips/trapframe.h>
#include <kern/wait.h>
#include <lib.h>
#include <limits.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>
#include <synch.h>
#include "opt-A2.h"
  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */
#if OPT_A2
void temppass(void *tf,unsigned long data);
void temppass(void *tf,unsigned long data)
{
  (void)data;
  struct trapframe *tf1=tf;
  enter_forked_process(tf1);
}
int sys_fork(struct trapframe *tf, pid_t *pid)
{
  //assign pid already done
  struct proc *childproc = proc_create_runprogram(curproc->p_name);
  if (childproc == NULL) {
    kprintf("1");
        return ENOMEM;
    }
    //address space
  int errno = as_copy(curproc->p_addrspace, &childproc->p_addrspace);
  if(errno)
  {
    proc_destroy(childproc);
    kprintf("2");
    return errno;
  }
  struct trapframe *childtrapframe = kmalloc(sizeof(struct trapframe));
    if (childtrapframe == NULL) {
      kprintf("3");
        kfree(childtrapframe);
        as_destroy(childproc->p_addrspace);
        proc_destroy(childproc);
        return ENOMEM;
    }
     memcpy(childtrapframe, tf, sizeof(struct trapframe));
     childproc->parentproc=curproc;
     procarray_add(&curproc->childrenproc, childproc, NULL);
     if(thread_fork(curproc->p_name, childproc, &temppass, childtrapframe, 0))
     {
      kprintf("4");
      proc_destroy(childproc);
      kfree(childtrapframe);
      return ENOMEM;
     }
     *pid=childproc->pid;
     intarray_add(&curproc->childrenpid,pid,NULL);
     
     kprintf("taeget pid is %d",*pid);
     return 0;

}
#endif
void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);
  
#if OPT_A2
    if(spinlock_do_i_hold(&curproc->p_lock))
  {
    kprintf("222");
  }
    spinlock_acquire(&curproc->p_lock);
    for (unsigned i = 0; i < intarray_num(&curproc->childrenpid); i++) {
        int targetpid = *intarray_get(&curproc->childrenpid, i);
        kprintf("i is %d and pid is %d\n",i,targetpid);
        if(searchpid(targetpid)==NULL)
        {
          spinlock_release(&curproc->p_lock);
          freepid(targetpid);
            if(spinlock_do_i_hold(&curproc->p_lock))
  {
    kprintf("333");
  }
          spinlock_acquire(&curproc->p_lock);
        }
        else
        {
          struct proc *targetproc=searchpid(targetpid);
          targetproc->parentproc=NULL;
        }
    }
    spinlock_release(&curproc->p_lock);
    saveexitcode(curproc->pid,exitcode);
    if(curproc->parentproc==NULL)
    {
      freepid(curproc->pid);
    }
    else
    {

      lock_acquire(curproc->pidlock);
      cv_broadcast(curproc->pidcv, curproc->pidlock); 
      lock_release(curproc->pidlock);
    }
#endif
  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  /* for now, this is just a stub that always returns a PID of 1 */
  /* you need to fix this to make it work properly */
  #if OPT_A2
  KASSERT(curproc != NULL);
  *retval = curproc->pid;
  return(0);
  #else
  *retval = 1;
  return(0);
  #endif
}

/* stub handler for waitpid() system call                */

int
sys_waitpid(pid_t pid,
	    userptr_t status,
	    int options,
	    pid_t *retval)
{
  int exitstatus;
  int result;

  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */

  if (options != 0) {
    return(EINVAL);
  }
 #if OPT_A2
  if(pid < 0 || pid > PID_MAX) {
    return ESRCH;
  }
  if(spinlock_do_i_hold(&curproc->p_lock))
  {
    kprintf("111");
  }
  //spinlock_acquire(&curproc->p_lock);
  struct proc *target=searchpid(pid);
  int code=getexitcode(pid);
  if((target==NULL)&&(code==-1))
  {
    *retval=-1;
    return ESRCH;
  }
  if(target->parentproc->pid!=curproc->pid)
  {
    *retval = -1;
    return ECHILD;
  }
  if((target==NULL)&&(code!=-1))
  {
   // spinlock_release(&curproc->p_lock);//target akready exit get exitcode
  }
  else
  {
    //spinlock_release(&curproc->p_lock);
    lock_acquire(target->pidlock);
    while(getexitcode(pid) == -1) {
            cv_wait(target->pidcv, target->pidlock);
        }
    lock_release(target->pidlock);
    code=getexitcode(pid);
  }

  exitstatus = _MKWAIT_EXIT(code);
#else

  /* for now, just pretend the exitstatus is 0 */
  exitstatus = 0;
#endif
  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);

}


#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "pinfo.h"
extern struct proc proc[NPROC];
extern struct spinlock wait_lock;
uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
uint64
sys_getprocs(void)
{
  uint64 ubuf;
  uint64 bytes;
  int max;
  int count = 0;
  struct proc *p;
  struct proc *parent;
  struct proc *cur = myproc();
  struct pinfo *infos;

  argaddr(0, &ubuf);
  argint(1, &max);

  if (max <= 0 || ubuf == 0 || ubuf >= MAXVA)
    return -1;

  if (max > NPROC)
    max = NPROC;

  infos = (struct pinfo *)kalloc();
  if (infos == 0)
    return -1;

  acquire(&wait_lock);

  for (p = proc; p < &proc[NPROC] && count < max; p++) {
    acquire(&p->lock);

    if (p->state != UNUSED) {
      infos[count].pid = p->pid;
      infos[count].state = p->state;
      infos[count].sz = p->sz;
      safestrcpy(infos[count].name, p->name,
                 sizeof(infos[count].name));

      parent = p->parent;
      release(&p->lock);

      if (parent == 0) {
        infos[count].ppid = 0;
      } else {
        acquire(&parent->lock);
        infos[count].ppid = parent->pid;
        release(&parent->lock);
      }

      count++;
    } else {
      release(&p->lock);
    }
  }

  release(&wait_lock);

  bytes = (uint64)count * sizeof(struct pinfo);

  if (bytes > MAXVA - ubuf) {
    kfree(infos);
    return -1;
  }

  if (bytes > 0 &&
      copyout(cur->pagetable, cur->sz, ubuf,
              (char *)infos, bytes) < 0) {
    kfree(infos);
    return -1;
  }

  kfree(infos);

  return count;
}

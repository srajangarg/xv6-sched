#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

void pagefault(void)
{
  // cprintf("pagefault\n");
  uint va = rcr2();  // Reading the page fault register
  uint pa;
  int pagenum, refcount;
  pte_t *pte;
  char *mem;

  if(proc == 0){
  cprintf("pagefault with no user process from cpu %d, addr 0x%x\n", cpu->apicid, va);
  panic("pagefault");
  }

  if(va >= KERNBASE || (pte = walkpgdir(proc->pgdir, (void*)va, 0)) == 0 
     || !(*pte & PTE_P) || !(*pte & PTE_U)) {
    cprintf("pid %d %s: illegal memory access on cpu %d addr 0x%x -- kill proc\n",
            proc->pid, proc->name, cpu->apicid, va);
    proc->killed = 1;
    return;
  }

  if(*pte & PTE_W) {
    panic("pagefault writeable");
  } else {

    pa = PTE_ADDR(*pte);  //Address of the flag register
    pagenum = pa / PGSIZE;
    // cprintf("pagenum %d\n", pagenum);
    refcount = getpageref(pagenum);

    if(refcount == 1) {
      *pte |= PTE_W;
      // cprintf("refcount 1 : made writable\n");
    } else if(refcount > 1) {

      mem = kalloc();
      if(mem == 0) {
        cprintf("pid %d %s: pagefault out of memory--kill proc\n", proc->pid, proc->name);
        proc->killed = 1;
        return;
      }
      memmove(mem, (char*)P2V(pa), PGSIZE);
      decpageref(pagenum);
      *pte = V2P(mem) | PTE_P | PTE_W | PTE_U;

      // cprintf("new mem allocated\n");
    } else {
      panic("pagefault wrong refcount");
    }

    lcr3(V2P(proc->pgdir));
  }
} 

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpunum() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpunum(), tf->cs, tf->eip);
    lapiceoi();
    break;

  case T_PGFLT:
    pagefault();
    break;

  //PAGEBREAK: 13
  default:
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpunum(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            proc->pid, proc->name, tf->trapno, tf->err, cpunum(), tf->eip,
            rcr2());
    proc->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();
}

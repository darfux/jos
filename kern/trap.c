#include <inc/mmu.h>
#include <inc/x86.h>
#include <inc/assert.h>

#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/env.h>
#include <kern/syscall.h>
#include <kern/sched.h>
#include <kern/kclock.h>
#include <kern/picirq.h>

static struct Taskstate ts;

/* Interrupt descriptor table.  (Must be built at run time because
 * shifted function addresses can't be represented in relocation records.)
 */
struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
	sizeof(idt) - 1, (uint32_t) idt
};


static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
		"Divide error",
		"Debug",
		"Non-Maskable Interrupt",
		"Breakpoint",
		"Overflow",
		"BOUND Range Exceeded",
		"Invalid Opcode",
		"Device Not Available",
		"Double Falt",
		"Coprocessor Segment Overrun",
		"Invalid TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection",
		"Page Fault",
		"(unknown trap)",
		"x87 FPU Floating-Point Error",
		"Alignment Check",
		"Machine-Check",
		"SIMD Floating-Point Exception"
	};

	if (trapno < sizeof(excnames)/sizeof(excnames[0]))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";
	return "(unknown trap)";
}

//	the func need to use SETGATE
void _divide();
void _debug();
void _nmi();
void _brkpt();
void _oflow();
void _bound();
void _illop();
void _device();
void _dblflt();
void _tss();
void _segnp();
void _stack();
void _gpflt();
void _pgflt();
void _fperr();
void _align();
void _mchk();
void _simderr();
void _20();
void _21();
void _22();
void _23();
void _24();
void _25();
void _26();
void _27();
void _28();
void _29();
void _30();
void _31();
void _32();
void _33();
void _34();
void _35();
void _36();
void _37();
void _38();
void _39();
void _40();
void _41();
void _42();
void _43();
void _44();
void _45();
void _46();
void _47();
void  _syscall();
void _default();

void
idt_init(void)
{
	extern struct Segdesc gdt[];
	
	// LAB 3: Your code here.
	SETGATE(idt[T_DIVIDE], 1, GD_KT, _divide, 0);
	SETGATE(idt[T_DEBUG], 1, GD_KT, _debug, 0);
	SETGATE(idt[T_NMI], 0, GD_KT, _nmi, 0);
	SETGATE(idt[T_BRKPT], 1, GD_KT, _brkpt, 3);
	SETGATE(idt[T_OFLOW], 1, GD_KT, _oflow, 0);
	SETGATE(idt[T_BOUND], 1, GD_KT, _bound, 0);
	SETGATE(idt[T_ILLOP], 1, GD_KT, _illop, 0);
	SETGATE(idt[T_DEVICE], 1, GD_KT, _device, 0);
	SETGATE(idt[T_DBLFLT], 1, GD_KT, _dblflt, 0);
	SETGATE(idt[T_TSS], 1, GD_KT, _tss, 0);
	SETGATE(idt[T_SEGNP], 1, GD_KT, _segnp, 0);
	SETGATE(idt[T_STACK], 1, GD_KT, _stack, 0);
	SETGATE(idt[T_GPFLT], 1, GD_KT, _gpflt, 0);
	SETGATE(idt[T_PGFLT], 1, GD_KT, _pgflt, 0);
	SETGATE(idt[T_FPERR], 1, GD_KT, _fperr, 0);
	SETGATE(idt[T_ALIGN], 1, GD_KT, _align, 0);
	SETGATE(idt[T_MCHK], 1, GD_KT, _mchk, 0);
	SETGATE(idt[T_SIMDERR], 1, GD_KT, _simderr, 0);
	SETGATE(idt[20], 1, GD_KT, _20, 3);
	SETGATE(idt[21], 1, GD_KT, _21, 3);
	SETGATE(idt[22], 1, GD_KT, _22, 3);
	SETGATE(idt[23], 1, GD_KT, _23, 3);
	SETGATE(idt[24], 1, GD_KT, _24, 3);
	SETGATE(idt[25], 1, GD_KT, _25, 3);
	SETGATE(idt[26], 1, GD_KT, _26, 3);
	SETGATE(idt[27], 1, GD_KT, _27, 3);
	SETGATE(idt[28], 1, GD_KT, _28, 3);
	SETGATE(idt[29], 1, GD_KT, _29, 3);
	SETGATE(idt[30], 1, GD_KT, _30, 3);
	SETGATE(idt[31], 1, GD_KT, _31, 3);
	SETGATE(idt[32], 1, GD_KT, _32, 3);
	SETGATE(idt[33], 1, GD_KT, _33, 3);
	SETGATE(idt[34], 1, GD_KT, _34, 3);
	SETGATE(idt[35], 1, GD_KT, _35, 3);
	SETGATE(idt[36], 1, GD_KT, _36, 3);
	SETGATE(idt[37], 1, GD_KT, _37, 3);
	SETGATE(idt[38], 1, GD_KT, _38, 3);
	SETGATE(idt[39], 1, GD_KT, _39, 3);
	SETGATE(idt[40], 1, GD_KT, _40, 3);
	SETGATE(idt[41], 1, GD_KT, _41, 3);
	SETGATE(idt[42], 1, GD_KT, _42, 3);
	SETGATE(idt[43], 1, GD_KT, _43, 3);
	SETGATE(idt[44], 1, GD_KT, _44, 3);
	SETGATE(idt[45], 1, GD_KT, _45, 3);
	SETGATE(idt[46], 1, GD_KT, _46, 3);
	SETGATE(idt[47], 1, GD_KT, _47, 3);
	SETGATE(idt[T_SYSCALL], 1, GD_KT, _syscall, 3);
	SETGATE(idt[T_DEFAULT], 1, GD_KT, _default, 0);
	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	ts.ts_esp0 = KSTACKTOP;
	ts.ts_ss0 = GD_KD;
	ts.ts_cr3 = 0;

	// Initialize the TSS field of the gdt.
	gdt[GD_TSS >> 3] = SEG16(STS_T32A, (uint32_t) (&ts),
					sizeof(struct Taskstate), 0);
	gdt[GD_TSS >> 3].sd_s = 0;

	// Load the TSS
	ltr(GD_TSS);

	// Load the IDT
	asm volatile("lidt idt_pd");
}

void
print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p\n", tf);
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
	cprintf("  err  0x%08x\n", tf->tf_err);
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	cprintf("  esp  0x%08x\n", tf->tf_esp);
	cprintf("  ss   0x----%04x\n", tf->tf_ss);
}

void
print_regs(struct PushRegs *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}

static void
trap_dispatch(struct Trapframe *tf)
{
	// Handle processor exceptions.
	// LAB 3: Your code here.
	struct PushRegs pr = tf->tf_regs;//can not put in the switch statement
	switch(tf->tf_trapno)
	{
		case T_PGFLT:
			page_fault_handler(tf);
			return;
		case T_BRKPT:
			monitor(tf);
			return;
		case T_SYSCALL:
			//syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
			// up to five parameters in DX, CX, BX, DI, SI. --from lib/syscall.c
			// syscall(pr.reg_eax, pr.reg_edx, pr.reg_ecx, pr.reg_ebx, pr.reg_edi, pr.reg_esi);
			tf->tf_regs.reg_eax = syscall(pr.reg_eax, pr.reg_edx, pr.reg_ecx, pr.reg_ebx, pr.reg_edi, pr.reg_esi);
			return;
	}
	// Handle clock and serial interrupts.
	// LAB 4: Your code here.

	// Unexpected trap: The user process or the kernel has a bug.
	print_trapframe(tf);
	if (tf->tf_cs == GD_KT)
		panic("unhandled trap in kernel");
	else {
		env_destroy(curenv);
		return;
	}
}

void
trap(struct Trapframe *tf)
{
	if ((tf->tf_cs & 3) == 3) {
		// Trapped from user mode.
		// Copy trap frame (which is currently on the stack)
		// into 'curenv->env_tf', so that running the environment
		// will restart at the trap point.
		assert(curenv);
		curenv->env_tf = *tf;
		// The trapframe on the stack should be ignored from here on.
		tf = &curenv->env_tf;
	}
	
	// Dispatch based on what type of trap occurred
	trap_dispatch(tf);

	// If we made it to this point, then no other environment was
	// scheduled, so we should return to the current environment
	// if doing so makes sense.
	if (curenv && curenv->env_status == ENV_RUNNABLE)
		env_run(curenv);
	else
		sched_yield();
}

#include <string.h>
void
page_fault_handler(struct Trapframe *tf)
{
	uint32_t fault_va;

	// Read processor's CR2 register to find the faulting address
	fault_va = rcr2();

	// Handle kernel-mode page faults.
	
	// LAB 3: Your code here.
	if(!(tf->tf_cs&0x3)) panic("Page fault in kernel(0x%08x)\n", fault_va);
	// We've already handled kernel-mode exceptions, so if we get here,
	// the page fault happened in user mode.

	// Call the environment's page fault upcall, if one exists.  Set up a
	// page fault stack frame on the user exception stack (below
	// UXSTACKTOP), then branch to curenv->env_pgfault_upcall.
	//
	// The page fault upcall might cause another page fault, in which case
	// we branch to the page fault upcall recursively, pushing another
	// page fault stack frame on top of the user exception stack.
	//
	// The trap handler needs one word of scratch space at the top of the
	// trap-time stack in order to return.  In the non-recursive case, we
	// don't have to worry about this because the top of the regular user
	// stack is free.  In the recursive case, this means we have to leave
	// an extra word between the current top of the exception stack and
	// the new stack frame because the exception stack _is_ the trap-time
	// stack.
	//
	// If there's no page fault upcall, the environment didn't allocate a
	// page for its exception stack, or the exception stack overflows,
	// then destroy the environment that caused the fault.
	//
	// Hints:
	//   user_mem_assert() and env_run() are useful here.
	//   To change what the user environment runs, modify 'curenv->env_tf'
	//   (the 'tf' variable points at 'curenv->env_tf').
	
	// LAB 4: Your code here.

	//If there's no page fault upcall, the environment didn't allocate 
	//a page for its exception stack,... then destroy the environment 
	//that caused the fault.

	int noFaultCall = (curenv->env_pgfault_upcall==NULL);
	if(noFaultCall)
	{
		cprintf("[%08x] user fault va %08x ip %08x\n",
			curenv->env_id, fault_va, tf->tf_eip);
		print_trapframe(tf);
		env_destroy(curenv);	
	}

	uint32_t* currentEsp;

	// Set up apage fault stack frame on the user exception stack (below
	// UXSTACKTOP)
	user_mem_assert(curenv, (void *) UXSTACKTOP - 1, 1, PTE_U | PTE_W | PTE_P); 

	if(tf->tf_esp <= UXSTACKTOP-1 && (tf->tf_esp >= UXSTACKTOP-PGSIZE)) 
	{
		//Already in page fault upcall
		currentEsp = (uint32_t *) (tf->tf_esp);
		currentEsp--;
		*currentEsp = 0;
	}
	else
	{
		currentEsp = (uint32_t *) UXSTACKTOP;
	}
	
	//check stack overflow
	int tfsize = sizeof(struct UTrapframe);
	uint32_t* espWill = currentEsp-tfsize/sizeof(uint32_t*);
	user_mem_assert(curenv, (void*)espWill, tfsize, PTE_U | PTE_W);	
	
	//w\\
	//below is a trick using inc/string.c's memcpy
	//to 'push' a UTrapframe in the stack

	//init a temp UTrapframe
	struct UTrapframe tmp = 
	{
		.utf_fault_va	= fault_va, 
		.utf_err		= tf->tf_err,
		.utf_regs		= tf->tf_regs,
		.utf_eip		= tf->tf_eip,
		.utf_eflags		= tf->tf_eflags,
		.utf_esp		= tf->tf_esp
	}; 

	//move the currentEsp to the position after 'push'
	//a UTrapframe
	currentEsp -= tfsize/sizeof(uint32_t*);

	//use memcpy to 'push' the UTrapframe tmp into uxstack
	memcpy(currentEsp, &tmp, sizeof(struct UTrapframe));

	//set proper eip&esp
	curenv->env_tf.tf_eip = (uintptr_t) curenv->env_pgfault_upcall;
	curenv->env_tf.tf_esp = (uintptr_t) currentEsp;
	
	//run env
	env_run(curenv);
	
	// Destroy the environment that caused the fault.
	cprintf("[%08x] user fault va %08x ip %08x\n",
		curenv->env_id, fault_va, tf->tf_eip);
	print_trapframe(tf);
	env_destroy(curenv);
}


/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* The TRAPHANDLER macro defines a globally-visible function for handling
 * a trap.  It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 */ 
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 * 	Add you own code here .
 */

/*w*\
**See the 80386 manual to determine for
**which exception numbers the processor pushes an error code, and what the error code
**means in that case.*/

TRAPHANDLER_NOEC(_divide, T_DIVIDE);    // divide error
TRAPHANDLER_NOEC(_debug, T_DEBUG);      // debug exception
TRAPHANDLER_NOEC(_nmi, T_NMI);  // non-maskable interrupt
TRAPHANDLER_NOEC(_brkpt, T_BRKPT);      // breakpoint
TRAPHANDLER_NOEC(_oflow, T_OFLOW);      // overflow
TRAPHANDLER_NOEC(_bound, T_BOUND);      // bounds check
TRAPHANDLER_NOEC(_illop, T_ILLOP);      // illegal opcode
TRAPHANDLER_NOEC(_device, T_DEVICE);    // device not available
TRAPHANDLER(_dblflt, T_DBLFLT); // double fault
TRAPHANDLER(_tss, T_TSS);       // invalid task switch segment
TRAPHANDLER(_segnp, T_SEGNP);   // segment not present
TRAPHANDLER(_stack, T_STACK);   // stack exception
TRAPHANDLER(_gpflt, T_GPFLT);   // genernal protection fault
TRAPHANDLER(_pgflt, T_PGFLT);   // page fault
TRAPHANDLER_NOEC(_fperr, T_FPERR);      // floating point error
TRAPHANDLER_NOEC(_align, T_ALIGN);      // aligment check
TRAPHANDLER_NOEC(_mchk, T_MCHK);        // machine check
TRAPHANDLER_NOEC(_simderr, T_SIMDERR);  // SIMD floating point error

TRAPHANDLER_NOEC(_20, 20);
TRAPHANDLER_NOEC(_21, 21);
TRAPHANDLER_NOEC(_22, 22);
TRAPHANDLER_NOEC(_23, 23);
TRAPHANDLER_NOEC(_24, 24);
TRAPHANDLER_NOEC(_25, 25);
TRAPHANDLER_NOEC(_26, 26);
TRAPHANDLER_NOEC(_27, 27);
TRAPHANDLER_NOEC(_28, 28);
TRAPHANDLER_NOEC(_29, 29);
TRAPHANDLER_NOEC(_30, 30);
TRAPHANDLER_NOEC(_31, 31);
TRAPHANDLER_NOEC(_32, 32);
TRAPHANDLER_NOEC(_33, 33);
TRAPHANDLER_NOEC(_34, 34);
TRAPHANDLER_NOEC(_35, 35);
TRAPHANDLER_NOEC(_36, 36);
TRAPHANDLER_NOEC(_37, 37);
TRAPHANDLER_NOEC(_38, 38);
TRAPHANDLER_NOEC(_39, 39);
TRAPHANDLER_NOEC(_40, 40);
TRAPHANDLER_NOEC(_41, 41);
TRAPHANDLER_NOEC(_42, 42);
TRAPHANDLER_NOEC(_43, 43);
TRAPHANDLER_NOEC(_44, 44);
TRAPHANDLER_NOEC(_45, 45);
TRAPHANDLER_NOEC(_46, 46);
TRAPHANDLER_NOEC(_47, 47);

TRAPHANDLER_NOEC(_syscall, T_SYSCALL);
TRAPHANDLER_NOEC(_default, T_DEFAULT);


/*
 * Lab 3: Your code here for _alltraps
 *      This code has finished for you ,so just read it.
 */

	_alltraps:
        # Build trap frame.
        pushl %ds
        pushl %es
        pushal
        # Set up data segments.
        movl $GD_KD, %eax
        movw %ax,%ds
        movw %ax,%es
        # Call trap(tf), where tf=%esp
        pushl %esp
        call trap
        popl %esp

        # Cleanup pushes and ret
        popal
        popl %es
        popl %ds
        iret
	


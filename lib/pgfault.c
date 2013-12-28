// User-level page fault handler support.
// We use an assembly language wrapper around a C function.
// The assembly wrapper is in pfentry.S.

#include <inc/lib.h>


// Assembly language pgfault entrypoint defined in lib/pgfaultentry.S.
extern void _pgfault_upcall(void);

// Pointer to currently installed C-language pgfault handler.
void (*_pgfault_handler)(struct UTrapframe *utf);
//-------------------LX--------------
extern void _divzero_upcall(void);
int flag_except_statick_alloc=0;
void (*_divzero_handler)(struct UTrapframe * utf);
//-------------------LX--------------

//
// Set the page fault handler function.
// If there isn't one yet, _pgfault_handler will be 0.
// The first time we register a handler, we need to 
// allocate an exception stack (one page of memory with its top
// at UXSTACKTOP), and tell the kernel to call the assembly-language
// _pgfault_upcall routine when a page fault occurs.
//
//-----------------------------------LX------------------------
void
set_pgfault_handler(void (*handler)(struct UTrapframe *utf))
{
	int r;

	if (flag_except_statick_alloc== 0) 
	{
		// First time through!
		// LAB 4: Your code here.
		r=sys_page_alloc(sys_getenvid(),(void *)(UXSTACKTOP-PGSIZE),PTE_W|PTE_U|PTE_P);
		if(r<0)
		{
			panic ("set_pgfault_handler: %e", r);
		}	
		
		flag_except_statick_alloc=1;
		//panic("set_pgfault_handler not implemented");
	}
	sys_env_set_pgfault_upcall(sys_getenvid(),(void *)_pgfault_upcall);
	// Save handler pointer for assembly to call.
	_pgfault_handler = handler;
}
//-------------LX---------------------

//-------------LX---------------------
void
set_divzero_handler(void (*handler)(struct UTrapframe *utf))
{
	int r;
	if (flag_except_statick_alloc == 0) 
	{
		// First time through!
		// LAB 4: Your code here.
		r=sys_page_alloc(sys_getenvid(),(void *)(UXSTACKTOP-PGSIZE),PTE_W|PTE_U|PTE_P);
		if(r<0)
		{
			panic ("set_divezero_handler: %e", r);
		}	
		
		flag_except_statick_alloc=1;
		cprintf("entry the set_divzero_handler2\n");
		//panic("set_pgfault_handler not implemented");
	}
	sys_env_set_divzero_upcall(sys_getenvid(),(void *)_divzero_upcall);
	// Save handler pointer for assembly to call.
	_divzero_handler = handler;
}
//---------------LX-----------------------


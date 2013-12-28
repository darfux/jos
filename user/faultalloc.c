// test user-level fault handler -- alloc pages to fix faults

#include <inc/lib.h>
void 
handler1(struct UTrapframe *utf)
{
	
	int r;
	void *addr = (void*)utf->utf_fault_va;

	cprintf("fault %x\n", addr);
	if ((r = sys_page_alloc(0, ROUNDDOWN(addr, PGSIZE),
				PTE_P|PTE_U|PTE_W)) < 0)
		panic("allocating at %x in page fault handler: %e", addr, r);
	snprintf((char*) addr, 100, "this string was faulted in at %x", addr);

	//---------------------------------LX-------------------------

	//cprintf("handler the dive zero\n");
	//__asm __volatile("xchg %bx, %bx");
}

void
handler(struct UTrapframe *utf)
{
	/*
	int r;
	void *addr = (void*)utf->utf_fault_va;

	cprintf("fault %x\n", addr);
	if ((r = sys_page_alloc(0, ROUNDDOWN(addr, PGSIZE),
				PTE_P|PTE_U|PTE_W)) < 0)
		panic("allocating at %x in page fault handler: %e", addr, r);
	snprintf((char*) addr, 100, "this string was faulted in at %x", addr);
*/
	//---------------------------------LX-------------------------

	cprintf("handler the dive zero\n");
	//cprintf("%s\n", (char*)0xDeadBeef);
	//cprintf("%s\n", (char*)0xCafeBffe);
	//__asm __volatile("xchg %bx, %bx");
}
int a;
void
umain(void)
{
	//-------------------LX---------------
	set_pgfault_handler(handler1);
	set_divzero_handler(handler);
	//cprintf("entry the div zero example\n");
	int b=2;
	//a=b/a;
	cprintf("result %d",a=b/a);
	
	//-------------------LX----------------
	//a=a/(a-b);
	
	
	
}



#include <inc/x86.h>
#include <inc/stdio.h>
#include <inc/isareg.h>

#include <kern/kide.h>
#include <kern/picirq.h>

void
kide_init()
{
	cprintf("	Setup ide(IRQ14) interrupts via 8259A\n");
	irq_setmask_8259A(irq_mask_8259A & ~(1<<14));
	cprintf("	unmasked ide(IRQ14) interrupt\n");
}
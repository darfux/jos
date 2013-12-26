#include <inc/x86.h>
#include <inc/stdio.h>
#include <inc/isareg.h>

#include <kern/kide.h>
#include <kern/picirq.h>

void
kide_init()
{
	cprintf("	Setup ide(IRQ%d) interrupts via 8259A\n", IDENUM);
	irq_setmask_8259A(irq_mask_8259A & ~(1<<IDENUM));
	outb(0x3F6, (0<<1));
	cprintf("	unmasked ide(IRQ%d) interrupt\n", IDENUM);
}
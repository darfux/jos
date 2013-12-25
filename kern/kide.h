#ifndef JOS_KERN_KIDE_H
#define JOS_KERN_KIDE_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

void kide_init(void);

#endif	// !JOS_KERN_KIDE_H
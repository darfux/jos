// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int error;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at vpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	//(1) a write
	if (!(err & FEC_WR)) panic ("faulting access not a write");
	//(2) to a copy-on-write page
	pte_t pte = (pte_t)vpt[VPN(addr)];
	if ( (pte&0xF00) != PTE_COW) panic ("faulting access not to a copy-on-write page");
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.
	
	// LAB 4: Your code here.
	int envid = sys_getenvid();//first system call

	//Allocate a new page, map it at a temporary location (PFTEMP)
	error = sys_page_alloc(envid, PFTEMP, PTE_U|PTE_P|PTE_W);//second system call
	if (error<0) sys_env_destroy(envid);

	//copy the data from the old page to the new page
	void* va = (void*)ROUNDDOWN(addr, PGSIZE);
	memcpy(PFTEMP, va, PGSIZE);

	//move the new page to the old page's address
	error = sys_page_map(envid, PFTEMP, envid, va, PTE_U|PTE_P|PTE_W);//third system call
	if (error< 0) sys_env_destroy(envid);
	
	// panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why mark ours copy-on-write again
// if it was already copy-on-write?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
// 
static int
duppage(envid_t envid, unsigned pn)
{
	int error;
	void *addr;
	pte_t pte;

	// LAB 4: Your code here.
	// panic("duppage not implemented");

	envid_t _envid;

	_envid = sys_getenvid();
	addr = (void *)(pn*PGSIZE);
	pte = (pte_t)vpt[VPN(addr)];
	
	int writable = pte&PTE_W;
	int cow = ((pte & 0xF00)==PTE_COW);
	if(writable||cow)
	{
		// Map our virtual page pn (address pn*PGSIZE) into the target envid
		// at the same virtual address
		error = sys_page_map(_envid, addr, envid, addr, PTE_COW|PTE_U|PTE_P);
		if(error< 0) panic("map to w/cow target failed");

		//and then our mapping must be marked copy-on-write as well
		error = sys_page_map(_envid, addr, _envid, addr, PTE_COW|PTE_U|PTE_P);
		if (error< 0) panic("mark self failed");
	}
	else
	{
		error = sys_page_map(_envid, addr, envid, addr, PTE_U|PTE_P);
		if(error< 0) panic("map to r target failed!");
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use vpd, vpt, and duppage.
//   Remember to fix "env" and the user exception stack in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	//panic("fork not implemented");

	// Assembly language pgfault entrypoint defined in lib/pgfaultentry.S.
	extern void _pgfault_upcall(void);
	
	envid_t envid;	

	// Set up our page fault handler appropriately.
	set_pgfault_handler(pgfault);

	// Create a child.
	envid = sys_exofork();
	if (envid< 0)
	{
		panic("no free env to fork");
	}
	else if (envid == 0)
	{
		env = &envs[ENVX(sys_getenvid())];	
		return 0;
	}

	// Copy our address space and page fault handler setup to the child.
	uint32_t addr;
	for(addr=UTEXT; addr < UXSTACKTOP-PGSIZE; addr+=PGSIZE)
	{
		//=w=
		//do not use assignment for encapsulation
		//as the if judge statement has short-circuit evaluation 
		//which avoid a fault read when forward action is false
		#define vpdP ((vpd[VPD(addr)] & PTE_P) > 0)
		#define vptP ((vpt[VPN(addr)] & PTE_P) > 0)
		#define vptU ((vpt[VPN(addr)] & PTE_U) > 0)
		if(vpdP && vptP && vptU) duppage(envid, VPN(addr));
	}

	int error;
	error = sys_page_alloc(envid, (void *)UXSTACKTOP-PGSIZE, PTE_U|PTE_P|PTE_W);
	if(error< 0) panic("allocate uxstack failed");	

	error = sys_env_set_pgfault_upcall(envid, (void *) _pgfault_upcall);
	if(error< 0) panic("set page fault entrypoint failed");
	
	// Then mark the child as runnable and return.
	error = sys_env_set_status(envid, ENV_RUNNABLE);
	if(error< 0) panic("set child status failed");

	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}

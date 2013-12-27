/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/sched.h>

// Print a string to the system console.
// The string is exactly 'len' characters long.
// Destroys the environment on memory errors.
static void
sys_cputs(const char *s, size_t len)
{
	// Check that the user has permission to read memory [s, s+len).
	// Destroy the environment if not.
	
	// LAB 3: Your code here.
	user_mem_assert(curenv, s, len, PTE_U);//see in pmap.c, the Destroy will be done also in the function
	// Print the string supplied by the user.
	cprintf("%.*s", len, s);
}

// Read a character from the system console.
// Returns the character.
static int
sys_cgetc(void)
{
	int c;

	// The cons_getc() primitive doesn't wait for a character,
	// but the sys_cgetc() system call does.
	while ((c = cons_getc()) == 0)
		/* do nothing */;

	return c;
}

// Returns the current environment's envid.
static envid_t
sys_getenvid(void)
{
	return curenv->env_id;
}

// Destroy a given environment (possibly the currently running environment).
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_destroy(envid_t envid)
{
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0)
		return r;
	if (e == curenv)
		cprintf("[%08x] exiting gracefully\n", curenv->env_id);
	else
		cprintf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	env_destroy(e);
	return 0;
}

// Deschedule current environment and pick a different one to run.
// The system call returns 0.
static void
sys_yield(void)
{
	sched_yield();
}

// Allocate a new environment.
// Returns envid of new environment, or < 0 on error.  Errors are:
//	-E_NO_FREE_ENV if no free environment is available.
static envid_t
sys_exofork(void)
{
	// Create the new environment with env_alloc(), from kern/env.c.
	// It should be left as env_alloc created it, except that
	// status is set to ENV_NOT_RUNNABLE, and the register set is copied
	// from the current environment -- but tweaked so sys_exofork
	// will appear to return 0.
	
	// LAB 4: Your code here.
	struct Env* envBaby;
	int error = env_alloc(&envBaby, curenv->env_id);
	if(error<0) return error;
	envBaby->env_status = ENV_NOT_RUNNABLE;
	envBaby->env_tf = curenv->env_tf;
	envBaby->env_tf.tf_regs.reg_eax = 0;
	return envBaby->env_id;

}

// Set envid's env_status to status, which must be ENV_RUNNABLE
// or ENV_NOT_RUNNABLE.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if status is not a valid status for an environment.
static int
sys_env_set_status(envid_t envid, int status)
{
	// Hint: Use the 'envid2env' function from kern/env.c to translate an
	// envid to a struct Env.
	// You should set envid2env's third argument to 1, which will
	// check whether the current environment has permission to set
	// envid's status.
	
	// LAB 4: Your code here.
	// panic("sys_env_set_status not implemented");

	//must be ENV_RUNNABLE or ENV_NOT_RUNNABLE.
	if(status!=ENV_RUNNABLE&&status!=ENV_NOT_RUNNABLE) return -E_INVAL;
	
	struct Env* env;
	//Use the 'envid2env' function
	int error = envid2env(envid, &env, 1);
	if(error<0) return error;

	//set status
	env->env_status = status;
	// Returns 0 on success
	return 0;
}

// Set envid's trap frame to 'tf'.
// tf is modified to make sure that user environments always run at code
// protection level 3 (CPL 3) with interrupts enabled.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_trapframe(envid_t envid, struct Trapframe *tf)
{
	// LAB 4: Your code here.
	// Remember to check whether the user has supplied us with a good
	// address!
	// panic("sys_set_trapframe not implemented");

	//come from lab5 TAT
	struct Env *e;
	int error;

	error = envid2env(envid, &e, 1);
	if (error< 0) return error;

	user_mem_assert(curenv, (const void *)tf, sizeof(struct Trapframe), 0);
	
	tf->tf_eflags |= FL_IF;
	tf->tf_cs |= 3;
	e->env_tf = *tf;
	
	return 0;
}

// Set the page fault upcall for 'envid' by modifying the corresponding struct
// Env's 'env_pgfault_upcall' field.  When 'envid' causes a page fault, the
// kernel will push a fault record onto the exception stack, then branch to
// 'func'.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_pgfault_upcall(envid_t envid, void *func)
{
	// LAB 4: Your code here.
	// panic("sys_env_set_pgfault_upcall not implemented");

	struct Env* env;
	int error;

	//check existance&permission
	error = envid2env(envid, &env, 1);

	if (error < 0) return error;
	env->env_pgfault_upcall = func;		
	return 0;
}

// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of 'envid'.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
//
// perm -- PTE_U | PTE_P must be set, PTE_AVAIL | PTE_W may or may not be set,
//         but no other bits may be set.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
//	-E_INVAL if perm is inappropriate (see above).
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	// Hint: This function is a wrapper around page_alloc() and
	//   page_insert() from kern/pmap.c.
	//   Most of the new code you write should be to check the
	//   parameters for correctness.
	//   If page_insert() fails, remember to free the page you
	//   allocated!

	// LAB 4: Your code here.
	// panic("sys_page_alloc not implemented");
	struct Env* env;
	struct Page* page;
	int error;


	error = envid2env(envid, &env, 1);
	//	-E_BAD_ENV if environment envid doesn't currently exist,
	//		or the caller doesn't have permission to change envid.
	if(error<0) return error;

	//-E_INVAL if va >= UTOP, or va is not page-aligned.
	if((uint32_t)va>=(uint32_t)UTOP || (uint32_t)va%PGSIZE!=0) return -E_INVAL;//Do not convert type to int for comparison!

	//-E_INVAL if perm is inappropriate (see above).
	//PTE_USER@mmu.h:
	//	Only flags in PTE_USER may be used in system calls.
	//	#define PTE_USER	(PTE_AVAIL | PTE_P | PTE_W | PTE_U)
	if(!(perm & PTE_P) || !(perm & PTE_U) || perm&~PTE_USER) return -E_INVAL;
	
	//	-E_NO_MEM if there's no memory to allocate the new page,
	//		or to allocate any necessary page tables.
	error = page_alloc(&page);
	if(error<0) return error;

	error = page_insert(env->env_pgdir, page, va, perm);
	if(error < 0)
	{
		page_free(page);//If page_insert() fails, remember to free the page you allocated!
		return error;
	}

	// The page's contents are set to 0.
	memset(page2kva(page), 0, PGSIZE);
	return 0;
}

// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
//		or the caller doesn't have permission to change one of them.
//	-E_INVAL if srcva >= UTOP or srcva is not page-aligned,
//		or dstva >= UTOP or dstva is not page-aligned.
//	-E_INVAL is srcva is not mapped in srcenvid's address space.
//	-E_INVAL if perm is inappropriate (see sys_page_alloc).
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int
sys_page_map(envid_t srcenvid, void *srcva,
		 envid_t dstenvid, void *dstva, int perm)
{
	// Hint: This function is a wrapper around page_lookup() and
	//   page_insert() from kern/pmap.c.
	//   Again, most of the new code you write should be to check the
	//   parameters for correctness.
	//   Use the third argument to page_lookup() to
	//   check the current permissions on the page.

	// LAB 4: Your code here.
	// panic("sys_page_map not implemented");
	struct Env* srcEnv;
	struct Env* dstEnv;
	struct Page* page;
	int error;
	
	//-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
	//	or the caller doesn't have permission to change one of them.
	error = envid2env(srcenvid, &srcEnv, 1);
	if(error<0) return -E_BAD_ENV;
	error = envid2env(dstenvid, &dstEnv, 1);
	if(error<0) return -E_BAD_ENV;

	if((uint32_t)srcva>=UTOP || (uint32_t)srcva%PGSIZE!=0) return -E_INVAL;
	if((uint32_t)dstva>=UTOP || (uint32_t)dstva%PGSIZE!=0) return -E_INVAL;

	if(!(perm & PTE_P) || !(perm & PTE_U) || perm&~PTE_USER) return -E_INVAL;

	//-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
	//	address space.
	pte_t* pte= pgdir_walk(srcEnv->env_pgdir, srcva, 0);
	if(perm&PTE_W)
	{
		if(((*pte)&PTE_W)==0) return -E_INVAL;
	}

	page = page_lookup(srcEnv->env_pgdir, srcva, 0);
	//-E_NO_MEM if there's no memory to allocate the new page,
	//	or to allocate any necessary page tables.
	error = page_insert(dstEnv->env_pgdir, page, dstva, perm);
	if(error<0) return -E_NO_MEM;

	return 0;
}

// Unmap the page of memory at 'va' in the address space of 'envid'.
// If no page is mapped, the function silently succeeds.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
static int
sys_page_unmap(envid_t envid, void *va)
{
	// Hint: This function is a wrapper around page_remove().
	
	// LAB 4: Your code here.
	// panic("sys_page_unmap not implemented");
	struct Env * env;
	int error;
	error = envid2env(envid, &env, 1);
	if(error<0)	return error;

	if((uint32_t)va >= UTOP && (uint32_t)va%PGSIZE!=0) return -E_INVAL;

	page_remove(env->env_pgdir, va);
	return 0;
}

// Try to send 'value' to the target env 'envid'.
// If va != 0, then also send page currently mapped at 'va',
// so that receiver gets a duplicate mapping of the same page.
//
// The send fails with a return value of -E_IPC_NOT_RECV if the
// target has not requested IPC with sys_ipc_recv.
//
// Otherwise, the send succeeds, and the target's ipc fields are
// updated as follows:
//    env_ipc_recving is set to 0 to block future sends;
//    env_ipc_from is set to the sending envid;
//    env_ipc_value is set to the 'value' parameter;
//    env_ipc_perm is set to 'perm' if a page was transferred, 0 otherwise.
// The target environment is marked runnable again.
//
// If the sender sends a page but the receiver isn't asking for one,
// then no page mapping is transferred, but no error occurs.
// The ipc doesn't happen unless no errors occur.
//
// Returns 0 on success where no page mapping occurs,
// 1 on success where a page mapping occurs, and < 0 on error.
// Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist.
//		(No need to check permissions.)
//	-E_IPC_NOT_RECV if envid is not currently blocked in sys_ipc_recv,
//		or another environment managed to send first.
//	-E_INVAL if srcva < UTOP but srcva is not page-aligned.
//	-E_INVAL if srcva < UTOP and perm is inappropriate
//		(see sys_page_alloc).
//	-E_INVAL if srcva < UTOP but srcva is not mapped in the caller's
//		address space.
//	-E_NO_MEM if there's not enough memory to map srcva in envid's
//		address space.
static int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
	// LAB 4: Your code here.
	// panic("sys_ipc_try_send not implemented");

	int error;
	struct Env* env;
	struct Page *page;
	int mapped = 0;

	error = envid2env(envid, &env, 0);
	if(error< 0) return error;
	
	if(!env->env_ipc_recving) return -E_IPC_NOT_RECV;
	
	if(env->env_ipc_dstva!=0 && (uint32_t)srcva < UTOP)
	{
		//	-E_INVAL if srcva < UTOP but srcva is not page-aligned.
		int notAligned = (uintptr_t)srcva%PGSIZE;
		if(notAligned) return -E_INVAL;
		
		//	-E_INVAL if srcva < UTOP and perm is inappropriate
		//		(see sys_page_alloc).
		int permWrong = (perm & (PTE_U|PTE_P)) != (PTE_U|PTE_P) && (perm & ~PTE_USER);
		if(permWrong) return -E_INVAL;

		//	-E_INVAL if srcva < UTOP but srcva is not mapped in the caller's
		//		address space.
		page = page_lookup(curenv->env_pgdir, srcva, 0);
		int notMap = (page==NULL);
		if(notMap) return -E_INVAL;

		//	-E_NO_MEM if there's not enough memory to map srcva in envid's
		//		address space.
		int noMem =  (page_insert(env->env_pgdir, page, env->env_ipc_dstva, perm) < 0);
		if (noMem) return -E_NO_MEM;

		mapped = 1;
	}

	env->env_ipc_recving = 0;
	env->env_ipc_from = curenv->env_id;
	env->env_ipc_value = value;
	mapped ? (env->env_ipc_perm = perm) : (env->env_ipc_perm = 0);

	env->env_status = ENV_RUNNABLE;

	// Returns 0 on success where no page mapping occurs,
	// 1 on success where a page mapping occurs, and < 0 on error.
	return mapped;
}

// Block until a value is ready.  Record that you want to receive
// using the env_ipc_recving and env_ipc_dstva fields of struct Env,
// mark yourself not runnable, and then give up the CPU.
//
// If 'dstva' is < UTOP, then you are willing to receive a page of data.
// 'dstva' is the virtual address at which the sent page should be mapped.
//
// This function only returns on error, but the system call will eventually
// return 0 on success.
// Return < 0 on error.  Errors are:
//	-E_INVAL if dstva < UTOP but dstva is not page-aligned.
static int
sys_ipc_recv(void *dstva)
{
	// LAB 4: Your code here.
	// panic("sys_ipc_recv not implemented");
	// return 0;
	int overTop = ((uintptr_t)dstva > UTOP);
	int notAligned = (uintptr_t)dstva%PGSIZE;
	if(overTop||notAligned) return -E_INVAL;
	
	curenv->env_ipc_recving = 1;
	curenv->env_ipc_dstva = dstva;
	curenv->env_status = ENV_NOT_RUNNABLE;

	curenv->env_tf.tf_regs.reg_eax = 0;

	sched_yield();
}

// static int 
// sys_env_set_ide_upcall()
// {
// 	panic("sys_env_set_ide_upcall not implemented");
// }
static int 
sys_exec(envid_t id, const char *prog, const char **argv)
{
	// env_destroy(&envs[ENVX(id)]);
	panic("sys_exec not implemented");

	return 0;
}


// Dispatches to the correct kernel function, passing the arguments.
uint32_t
syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	// LAB 3: Your code here.
	uint32_t tmp;
	switch(syscallno)
	{
		case SYS_cputs:
			sys_cputs((char *)a1, (size_t)a2);
			return 0;
		case SYS_cgetc:
			return sys_cgetc();
		case SYS_getenvid:
			return sys_getenvid();
		case SYS_env_destroy:
			return sys_env_destroy((envid_t)a1);
		//LAB4:
		case SYS_yield:
			sys_yield();
			return 0;
		case SYS_exofork:
			return sys_exofork();
		case SYS_env_set_status:
			//sys_env_set_status(envid_t envid, int status)
			return sys_env_set_status((envid_t)a1, (int)a2);
		case SYS_page_alloc:
			//sys_page_alloc(envid_t envid, void *va, int perm)
			return sys_page_alloc((envid_t)a1, (void*)a2, (int)a3);
		case SYS_page_map:
			//sys_page_map(envid_t srcenvid, void *srcva,
			// envid_t dstenvid, void *dstva, int perm)
			return sys_page_map((envid_t)a1, (void *)a2, (envid_t)a3, (void *)a4, (int)a5);
		case SYS_page_unmap:
			//sys_page_unmap(envid_t envid, void *va)
			return sys_page_unmap((envid_t)a1, (void *)a2);
		case SYS_env_set_pgfault_upcall:
			//sys_env_set_pgfault_upcall(envid_t envid, void *func)
			return sys_env_set_pgfault_upcall((envid_t)a1, (void *)a2);
		case SYS_ipc_try_send:
			//sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
			return sys_ipc_try_send((envid_t)a1, (uint32_t)a2, (void *)a3, (unsigned)a4);
		case SYS_ipc_recv:
			//sys_ipc_recv(void *dstva)
			return sys_ipc_recv((void *)a1);
		case SYS_env_set_trapframe:
			//sys_env_set_trapframe(envid_t envid, struct Trapframe *tf)
			return sys_env_set_trapframe((envid_t)a1, (struct Trapframe *)a2);				
		case SYS_exec:
			//sys_exec(envid_t id, const char *prog, const char **argv)
			return sys_exec((envid_t)a1, (const char *)a2, (const char **)a3);		
		// case SYS_env_set_ide_upcall:
		// 	return sys_env_set_ide_upcall();
		default:
			return -E_INVAL;
	}
	// panic("syscall not implemented");

 //        int ret = 0;
 //        void* func = NULL;
	// //	To help you finish this job,TA has write part of the code
	// //	Now you just have to set the pointer func to the write function
	// 	switch (syscallno)
	// 	{
	// 		case SYS_cputs:
	// 			func = &sys_cputs;
	// 			break;
	// 		case SYS_cgetc:
	// 			func = &sys_cgetc;
	// 			break;
	// 		case SYS_getenvid:
	// 			func = &sys_getenvid;
	// 			break;
	// 		case SYS_env_destroy:
	// 			func = &sys_env_destroy;
	// 			break;
	// 		default: return -E_INVAL; //do as the ex_description said
	// 	}

 //        asm volatile("push %%esi\n"
 //                "push %%edi\n"
 //                "push %%ebx\n"
 //                "push %%ecx\n"
 //                "push %%edx\n"
 //                "call *%1\n"
 //                : "=a" (ret)
 //                : "m" (func),
 //                  "d" (a1),
 //                  "c" (a2),
 //                  "b" (a3),
 //                  "D" (a4),
 //                  "S" (a5)
 //                : "cc", "memory");
	//asm volatile("add $0x14,%esp");
 //        return ret;
}


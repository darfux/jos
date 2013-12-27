// User-level IPC library routines

#include <inc/lib.h>
// Receive a value via IPC and return it.
// If 'pg' is nonnull, then any page sent by the sender will be mapped at
//	that address.
// If 'fromenv' is nonnull, then store the IPC sender's envid in *fromenv.
// If 'perm' is nonnull, then store the IPC sender's page permission in *perm
//	(this is nonzero iff a page was successfully transferred to 'pg').
// If the system call fails, then store 0 in *fromenv and *perm (if
//	they're nonnull) and return the error.
//
// Hint:
//   Use 'env' to discover the value and who sent it.
//   If 'pg' is null, pass sys_ipc_recv a value that it will understand
//   as meaning "no page".  (Zero is not the right value.)
uint32_t
ipc_recv(envid_t *from_env_store, void *pg, int *perm_store)
{
	// LAB 4: Your code here.
	// panic("ipc_recv not implemented");
	int error;
	void* parm = pg;

	//   If 'pg' is null, pass sys_ipc_recv a value that it will understand
	//   as meaning "no page".  (Zero is not the right value.
	if(pg==NULL) parm=(void*)UTOP;
	error = sys_ipc_recv(parm);
	if(!error)
	{
		//If 'fromenv' is nonnull, then store the IPC sender's envid in *fromenv.
		if(from_env_store!=0) (*from_env_store) = env->env_ipc_from;
		
		// If 'perm' is nonnull, then store the IPC sender's page permission in *perm
		//	(this is nonzero iff a page was successfully transferred to 'pg').
		if(perm_store!=0) (*perm_store) = env->env_ipc_perm;

		return env->env_ipc_value;		
	}
	else
	{
		(*from_env_store) = 0;
		(*perm_store) = 0;
		
		return error;	
	}
	return 0;
}

// Send 'val' (and 'pg' with 'perm', assuming 'pg' is nonnull) to 'toenv'.
// This function keeps trying until it succeeds.
// It should panic() on any error other than -E_IPC_NOT_RECV.
//
// Hint:
//   Use sys_yield() to be CPU-friendly.
//   If 'pg' is null, pass sys_ipc_recv a value that it will understand
//   as meaning "no page".  (Zero is not the right value.)
void
ipc_send(envid_t to_env, uint32_t val, void *pg, int perm)
{
	// LAB 4: Your code here.
	// panic("ipc_send not implemented");

	//   If 'pg' is null, pass sys_ipc_recv a value that it will understand
	//   as meaning "no page".  (Zero is not the right value.)
	if(pg==NULL) pg=(void*)UTOP;

	int error = sys_ipc_try_send(to_env, val, pg, perm);

	while(error<0)
	{
		if(error != -E_IPC_NOT_RECV) panic("not E_IPC_NOT_RECV %e", error);
		//   Use sys_yield() to be CPU-friendly.
		sys_yield();
		error = sys_ipc_try_send(to_env, val, pg, perm);	
	}
}

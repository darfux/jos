#include <inc/lib.h>

void
umain(void)
{
	envid_t id = env->env_id;
	cprintf("i am parent environment %08x\n", id);
	// if ((r = execl("hello", "hello", 0)) < 0)
		// panic("spawn(hello) failed: %e", r);
	// ipc_send(envs[EXESERV].env_id, id, 0, 0);
	exec("spawnhello");
	sys_env_set_status(id, ENV_NOT_RUNNABLE);
	cprintf("[%08x]i send 9, goodbye~\n", id);
	
}
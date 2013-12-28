#include <inc/lib.h>

void
umain(void)
{
	envid_t id = env->env_id;
	cprintf("[exechello]i am parent environment %08x\n", id);
	exec("spawnhello");
	cprintf("[%08x]i send 9, goodbye~\n", id);
	
}

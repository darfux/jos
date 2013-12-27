#include <inc/lib.h>
#include <inc/execer.h>

extern uint8_t execipcbuf[PGSIZE*2];

void
exec(const char *program)
{
	// cprintf("%s", program);
	int perm;
	struct Exreq *req;

	req = (struct Exreq*)execipcbuf;

	if (strlen(program) >= MAXPROGPATH) panic("error in exec: %e\n", -E_BAD_PATH);

	strcpy(req->prog, program);
	cprintf("here");
	ipc_send(envs[EXESERV].env_id, env->env_id, req,  PTE_P | PTE_W | PTE_U);
}
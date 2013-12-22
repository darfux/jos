#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>


// Choose a user environment to run and run it.
void
sched_yield(void)
{
	// Implement simple round-robin scheduling.
	// Search through 'envs' for a runnable environment,
	// in circular fashion starting after the previously running env,
	// and switch to the first such environment found.
	// It's OK to choose the previously running env if no other env
	// is runnable.
	// But never choose envs[0], the idle environment,
	// unless NOTHING else is runnable.

	// LAB 4: Your code here.

	// Run the special idle environment when nothing else is runnable.
	struct Env* env2run;

	int index, curenIndex;
	if(curenv==NULL)
	{
		index = 1;
	}
	else
	{
		curenIndex = ENVX(curenv->env_id);
		index = curenIndex>=NENV-1 ? 1 : curenIndex+1;
	}
	env2run = &envs[index];

	int i;
	for(i=1; i<NENV; i++)
	{
		if (env2run->env_status == ENV_RUNNABLE) env_run(env2run);
		curenIndex = ENVX(env2run->env_id);
		index = curenIndex>=NENV-1 ? 1 : curenIndex+1;
		env2run = &envs[index];
	}
    
	// Run the special idle environment when nothing else is runnable.
	if (envs[0].env_status == ENV_RUNNABLE)
		env_run(&envs[0]);
	else {
		cprintf("Destroyed all environments - nothing more to do!\n");
		while (1)
			monitor(NULL);
	}
}
#include <inc/lib.h>
 int test=1;
void 
umain(void)
{
   
	int who=sfork();
	if(who!=0)
	{
		cprintf("I am parent with test=%d\n",test);
		test=2;
	}
	else
	{
		cprintf("I am child with test=%d\n",test);
		test=3;
	}
	if(who!=0)
	{
		cprintf("I am parent with test=%d\n",test);
		//test=2;
	}
	else
	{
		cprintf("I am child with test=%d\n",test);
		//test=3;
	}
}


#include <inc/lib.h>
#define FVA (struct Fd*)0xCCCCC000
const char *msg = "This is the NEW message of the day!\n\n";
void
umain(void)
{
	int r;
	// int fileid;
	// struct Fd *fd;
	int fdnum;
	fdnum = open("/newmotd", O_ACCMODE);
	cprintf("get fdnum:%d\n", fdnum);

	r = ftruncate(fdnum, 4*1024*1024*128);
	panic("%e\n", r);

}


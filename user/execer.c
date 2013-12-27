#include <inc/lib.h>
#include <inc/elf.h>

#define REQVA		0x0ffff000
#define UTEMP2USTACK(addr)	((void*) (addr) + (USTACKTOP - PGSIZE) - UTEMP)
#define UTEMP2			(UTEMP + PGSIZE)
#define UTEMP3			(UTEMP2 + PGSIZE)

extern uint8_t execipcbuf[PGSIZE];	// page-aligned, declared in entry.S

static int init_stack(envid_t child, const char **argv, uintptr_t *init_esp);

int
exec_help(const char *prog, envid_t id)
{
	#define ELF_BUF_SIZE 512
	unsigned char elf_buf[ELF_BUF_SIZE];
	struct Trapframe child_tf;
	envid_t child = id;

	const char* argv[] = { 0 };

	int fd;
	int r;

	fd = open(prog, O_RDONLY);
	if(fd<0) return fd;

	char *elf_tmp;
	r = read_map(fd, 0, (void **)&elf_tmp);
	if(r<0)
	{
		close(fd);
		return r;
	}

	memcpy(elf_buf, elf_tmp, ELF_BUF_SIZE);


	struct Elf * header;
	uintptr_t init_esp;
	struct Proghdr *ph, *eph;
	void *blk;
	int i, how_many, perm = 0, read, no_bytes;
	uint32_t p_va;
	header = (struct Elf *)elf_buf;
	if (header->e_magic != ELF_MAGIC){		
		close(fd);
		return -E_NOT_EXEC;
	}

	//we are in parent - ren
	
	child_tf = envs[ENVX(child)].env_tf;
	// envid_t tempPaID = envs[ENVX(child)].env_parent_id;

	// envs[ENVX(child)].env_parent_id = sys_getenvid();
	// sys_env_destroy(child);

	child_tf.tf_eip = header->e_entry;
	
	if (( r = init_stack(child, argv, &init_esp)) < 0)
		goto error;
	
	child_tf.tf_esp = init_esp;


	//"load" executable in child's address space - ren

	ph = (struct Proghdr *) (elf_buf + header->e_phoff);
	eph = ph + header->e_phnum;

	for (; ph < eph; ph++){
		read=0;
		no_bytes=0;
		if (ph->p_type == ELF_PROG_LOAD){
	//we treat writable and readable segments in similar way (not like instructions tell) - ren
			if ( ph->p_flags & ELF_PROG_FLAG_WRITE){			//segment is writable - ren	
				how_many = ph->p_memsz/PGSIZE;
				if (ph->p_memsz % PGSIZE)
					how_many++;
				perm = 	PTE_U | PTE_P | PTE_W;
			}else{
				how_many = ph->p_filesz/PGSIZE;
				if (ph->p_filesz % PGSIZE)
					how_many++;
				perm = PTE_U | PTE_P;
			}
			
			p_va = ph->p_va;

			if ((r = seek(fd, ph->p_offset)) < 0)
				goto error;

			if(ph->p_va % PGSIZE){
				if ((r = sys_page_alloc(sys_getenvid(), UTEMP, PTE_U | PTE_P | PTE_W)) < 0)
					goto error;

				if (ph->p_filesz <= (PGSIZE - (ph->p_va % PGSIZE)))
					read = ph->p_filesz;
				else
					read = (PGSIZE - (ph->p_va % PGSIZE));				

				if ((r = readn(fd, (void *)(UTEMP + (ph->p_va % PGSIZE)), read )) < 0)
					goto error;
				
				if ((r = sys_page_map(sys_getenvid(), (void *)UTEMP,
									  child, (void *)ph->p_va - (ph->p_va % PGSIZE), perm)) < 0 )
					goto error;

				p_va = ROUNDUP(ph->p_va, PGSIZE);
			
				if ( ph->p_flags & ELF_PROG_FLAG_WRITE){								
						how_many = (ph->p_memsz - (PGSIZE - (ph->p_va % PGSIZE)))/PGSIZE;
						if ((ph->p_memsz - (PGSIZE - (ph->p_va % PGSIZE))) % PGSIZE)
							how_many++;	
				}else{
					how_many = (ph->p_filesz - (PGSIZE - (ph->p_va % PGSIZE)))/PGSIZE;
					if ((ph->p_filesz - (PGSIZE - (ph->p_va % PGSIZE))) % PGSIZE)
						how_many++;
				}
			}			

			for(i=0; i < how_many; i++){
				if ((r = sys_page_alloc(sys_getenvid(), UTEMP, PTE_U | PTE_P | PTE_W)) < 0)
					goto error;	
				
				if (read < ph->p_filesz){	
					if( (ph->p_filesz - read) > PGSIZE)
						no_bytes = PGSIZE;
					else
						no_bytes = ph->p_filesz - read;
					if ((r = readn(fd, (void *)UTEMP, no_bytes)) < 0)
						goto error;								//java exceptions are great - ren
					read+=no_bytes;				
				}	

				if ((r = sys_page_map(sys_getenvid(), (void *)UTEMP,
									  child, (void *)(p_va+i*PGSIZE), perm)) < 0 )
					goto error;			
			}			
		}
	}

	sys_page_unmap(sys_getenvid(), UTEMP);

	if ((r = sys_env_set_trapframe(child, &child_tf)) < 0)
		goto error;

	cprintf("[execer]I will raise up env %08x!\n", child);
	if ((r = sys_env_set_status(child, ENV_RUNNABLE)) < 0)
		goto error;
	cprintf("[execer]Raising up over\n");

	close(fd);
	return 0;

error:
	close(fd);
	sys_env_destroy(child);						
	return r;	
}

void
serve()
{
	uint32_t req, whom;
	int perm, error;
	cprintf("[execer]Server start,waiting for exec req...\n");
	while (1)
	{
		perm = 0;
		req = ipc_recv((int32_t *) &whom, (void*)REQVA, &perm);
		char* prog = ((struct Exreq*)REQVA)->prog;
		cprintf("[execer]i get %08x, prog: %s\n", req, prog) ;
		error = exec_help(prog, req);
		if(error<0) panic("[execer]error in execer: %e\n", error);
		cprintf("[execer]exec %08x over\n", req);
	}
}

void
umain(void)
{
	int r;
	cprintf("i am execer %08x\n", env->env_id);
	serve();
}

// Set up the initial stack page for the new child process with envid 'child'
// using the arguments array pointed to by 'argv',
// which is a null-terminated array of pointers to null-terminated strings.
//
// On success, returns 0 and sets *init_esp
// to the initial stack pointer with which the child should start.
// Returns < 0 on failure.
static int
init_stack(envid_t child, const char **argv, uintptr_t *init_esp)
{
	size_t string_size;
	int argc, i, r;
	char *string_store;
	uintptr_t *argv_store;

	// Count the number of arguments (argc)
	// and the total amount of space needed for strings (string_size).
	string_size = 0;
	for (argc = 0; argv[argc] != 0; argc++)
		string_size += strlen(argv[argc]) + 1;

	// Determine where to place the strings and the argv array.
	// Set up pointers into the temporary page 'UTEMP'; we'll map a page
	// there later, then remap that page into the child environment
	// at (USTACKTOP - PGSIZE).
	// strings is the topmost thing on the stack.
	string_store = (char*) UTEMP + PGSIZE - string_size;
	// argv is below that.  There's one argument pointer per argument, plus
	// a null pointer.
	argv_store = (uintptr_t*) (ROUNDDOWN(string_store, 4) - 4 * (argc + 1));
	
	// Make sure that argv, strings, and the 2 words that hold 'argc'
	// and 'argv' themselves will all fit in a single stack page.
	if ((void*) (argv_store - 2) < (void*) UTEMP)
		return -E_NO_MEM;

	// Allocate the single stack page at UTEMP.
	if ((r = sys_page_alloc(0, (void*) UTEMP, PTE_P|PTE_U|PTE_W)) < 0)
		return r;

	// Replace this with your code to:
	//
	//	* Initialize 'argv_store[i]' to point to argument string i,
	//	  for all 0 <= i < argc.
	//	  Also, copy the argument strings from 'argv' into the
	//	  newly-allocated stack page.
	//	  Hint: Copy the argument strings into string_store.
	//	  Hint: Make sure that argv_store uses addresses valid in the
	//	  CHILD'S environment!  The string_store variable itself
	//	  points into page UTEMP, but the child environment will have
	//	  this page mapped at USTACKTOP - PGSIZE.  Check out the
	//	  UTEMP2USTACK macro defined above.
	//
	//	* Set 'argv_store[argc]' to 0 to null-terminate the args array.
	//
	//	* Push two more words onto the child's stack below 'args',
	//	  containing the argc and argv parameters to be passed
	//	  to the child's umain() function.
	//	  argv should be below argc on the stack.
	//	  (Again, argv should use an address valid in the child's
	//	  environment.)
	//
	//	* Set *init_esp to the initial stack pointer for the child,
	//	  (Again, use an address valid in the child's environment.)
	//
	// LAB 5: Your code here.
	// *init_esp = USTACKTOP;	// Change this!






	//	* Initialize 'argv_store[i]' to point to argument string i,
	//	  for all 0 <= i < argc.
	int offset = 0;
	void* argvPos;
	for(i=0; i<argc; i++)
	{
		argvPos = string_store+offset;
		argv_store[i] = UTEMP2USTACK(argvPos);
		memcpy(argvPos, argv[i], strlen(argv[i])+1);
		offset += strlen(argv[i])+1;
	}


	//	* Set 'argv_store[argc]' to 0 to null-terminate the args array.
	#define NULL_TERMINATE 0
	argv_store[argc] = NULL_TERMINATE;



	//	* Push two more words onto the child's stack below 'args',
	//	  containing the argc and argv parameters to be passed
	//	  to the child's umain() function.
	//	  argv should be below argc on the stack.
	//	  (Again, argv should use an address valid in the child's
	//	  environment.)
	uintptr_t* forChildArgv = argv_store - 1;
	uintptr_t* forChildArgc = argv_store - 2;
	(*forChildArgv) = UTEMP2USTACK(argv_store);	
	*(forChildArgc) = argc;


	//	* Set *init_esp to the initial stack pointer for the child,
	//	  (Again, use an address valid in the child's environment.)

	//=w=
	//Note:mention the memlayout difference between stack and array
	//that a stack is from high to low
	//while an array is form low to high.
	//As we have allocated space for the array
	//we just need to put the head of array to a proper
	//position near the stack bottom.
	(*init_esp) = UTEMP2USTACK(forChildArgc);

	// After completing the stack, map it into the child's address space
	// and unmap it from ours!
	if ((r = sys_page_map(0, UTEMP, child, (void*) (USTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W)) < 0)
		goto error;
	if ((r = sys_page_unmap(0, UTEMP)) < 0)
		goto error;

	return 0;

error:
	sys_page_unmap(0, UTEMP);
	return r;
}
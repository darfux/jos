#ifndef JOS_INC_EXECER_H
#define JOS_INC_EXECER_H

#define MAXPROGPATH 1024
struct Exreq {
	char prog[MAXPROGPATH];
	char argv[MAXPROGPATH];
};

#endif /* !JOS_INC_EXECER_H */


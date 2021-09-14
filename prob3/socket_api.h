#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BUFFER_SIZE 	1024

#define THE_END(assertion, exit_string)		\
	if (assertion){				\
		perror(exit_string);			\
		exit(-1);				\
	}
	
	
#define THE_END_CLEANUP(assertion, exit_string, cleanup_f)		\
	if (assertion){						\
		perror(exit_string);					\
		cleanup_f();						\
		exit(-1);						\
	}								

#define THE_RETURN(assertion, ret_string, ret_val)			\
	if (assertion){						\
		perror(ret_string);					\
		return ret_val;					\
	}					


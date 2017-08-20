/*
 * simple strace clone
 *
 * references:
 * http://www.linuxjournal.com/article/6100
 * http://theantway.com/2013/01/notes-for-playing-with-ptrace-on-64-bits-ubuntu-12-10/
 * 
 */
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <libaudit.h>

#define UNUSED(x) x __attribute__((unused))

/*
 * Linux specific - requires libaudit
 */
static const char *syscall_name(long syscall)
{
	return audit_syscall_to_name(syscall,
								 audit_detect_machine());
}

static void err_exit(const char *prefix)
{
	fprintf(stderr, "%s: %s\n", prefix, strerror(errno));
	exit(-1);
}

static void _child(UNUSED(int argc), UNUSED(char **argv))
{
	if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
		err_exit("exit failed");
	}
	execl("/bin/ls", "ls", NULL);
}

static int _parent()
{
	int status;
	int rv = -1;
	long syscall;
	pid_t child;
	bool syscall_enter = true;

	while (true) {
		if ((child = waitpid(-1, &status, 0)) == -1) {
			break;
		}

		if (WIFEXITED(status)) {
			rv = 0;
			break;
		}

		if (syscall_enter) {
			syscall_enter = false;
			if ((syscall = ptrace(PTRACE_PEEKUSER,
						 	  	  child,
						 	  	  #ifdef __x86_64__
						 	  	  8 * ORIG_RAX,
							  	  #else
							  	  4 * ORIG_EAX,
							  	  #endif
						      	  NULL)) == -1) {
				break;
			}
			printf("%s\n", syscall_name(syscall));
		} else {
			syscall_enter = true;
		}
		
		/*
		 * resume the child until the next system call
		 */
		if (ptrace(PTRACE_SYSCALL,
				   child, NULL, NULL) == -1) {
			break;
		}
	}
	return rv;
}

int main(int argc, char **argv)
{
	pid_t child;
	child = fork();
	if (child == 0) {
		_child(argc, argv);
	} else {
		if (_parent() != 0) {
			err_exit("parent failed");
		}
	}
	return 0;
}

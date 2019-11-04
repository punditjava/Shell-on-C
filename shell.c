#include "shell.h"

char         *prog;
char         *temp;
struct _Job  *job;
extern char **environ;
unsigned      Quit = 0;
int           isFile = 0;
char **argvv;
int    argcc;

int main(int argc, char **argv)
{
	int argcc = argc;
    argvv = argv;
    char *freeprog;

	signal(SIGINT, handler_CtrlC);

	initMS();

	init_terminal();

	do {
		invite();

		prog = mgets();
		if (strcmp(prog, "\n") != NULL) {
			freeprog = prog;
			init_job();
			run_job();
			
			free_job();
		}

	} while (!Quit);


	restore_terminal();

	return 0;
}
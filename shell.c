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
	argcc = argc;
    argvv = argv;
    char *freeprog;

	signal(SIGINT, handler_CtrlC);

	initMS();

	if (!isatty(fileno(stdin))) {
		isFile = 1;
		prog = mgets();
		freeprog = prog;

		init_job();

		run_job();

		free_job();
		free(freeprog);
		freeprog = NULL;
		return 0;
	}


	init_terminal();

	do {
		invite();

		prog = mgets();
		if (strcmp(prog, "\n") != 0) {
			freeprog = prog;

			init_job();

			run_job();
			
			free_job();

			free(freeprog);
			freeprog = NULL;
			prog = NULL;
		}

	} while (!Quit);

	free_history();
	restore_terminal();

	return 0;
}
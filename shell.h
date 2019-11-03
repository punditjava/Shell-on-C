#ifndef _MS_H_
#define _MS_H_

#include <ctype.h>
#include <wchar.h>
#include <time.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <pwd.h>
#include <grp.h>

#include <sys/wait.h>
#include <fcntl.h>
#include <regex.h>

#define INVITE printf("\x1b[1;33m"); printf("mShell"); printf("\x1B[0m"); printf("$ "); fflush(stdout);

struct _Program {
    char  *name;
    int    number_of_arguments;
    char **arguments;
    char  *input_file;
    char  *output_file;
    int    output_type; //1 - rewrite, 2 - append
    int    conveyer;
    pid_t  pid;
    
}; 

struct _Job {
    int               background;
    struct _Program **programs; 

    int n; //number_of_programs;
    int convcount;
};

void  handler_CtrlC(int);
void invite(void);

void init_job(void);
void  initMS(void);

char *mgets(void);
void  serror(int error);

int init_terminal(void);
int restore_terminal(void);

#endif
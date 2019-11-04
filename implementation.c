#include "shell.h"
#define __RELISE__

	/***ПАРАМЕТРЫ***/

const unsigned int   bufBlock = 256;
unsigned             Conv = 0;
extern unsigned      Quit;
extern char        **argvv;
extern int           isFile;
extern struct _Job  *job;
extern char         *prog; /* указатель на список комманд */
extern char         *temp; /* указатель на временный список комманд */
char                 tempBuf[BUFSIZ];
char                 tempVar[BUFSIZ];
char                 lastDelim;

struct  termios old_attributes,
            new_attributes;


	/***INIT***/

void initMS(void) 
{
#ifdef __RELISE__
    char dir[1024];
    if( getcwd(dir, 1024) ) {
        if (setenv("SHELL", dir, 1) != 0) {
            fprintf (stderr, "setenv: Cannot set 'SHELL'\n");
        }
    } else
        serror(0);

    pid_t shellPid;
    shellPid = getpid();
    char shellPidS[256];
    sprintf(shellPidS, "%d", (int)shellPid);

    if (setenv("PID", shellPidS, 1) != 0) {
        fprintf (stderr, "setenv: Cannot set 'PID'\n");
    }

#endif

#ifdef __DEVELOP__
    pid_t shellPid;
    shellPid = getpid();
    if (setenv("PID", shellPid, 0) != 0) {
        fprintf (stderr, "setenv: Cannot set 'PID'\n");
    } else
        serror(0);

    if (setenv("NUMB", shellPid, 0) != 0) {
        fprintf (stderr, "setenv: Cannot set 'PID'\n");
    } else
        serror(0);

    if (setenv("HASH", shellPid, 0) != 0) {
        fprintf (stderr, "setenv: Cannot set 'PID'\n");
    } else
        serror(0);

    if (setenv("QUEST", shellPid, 0) != 0) {
        fprintf (stderr, "setenv: Cannot set 'PID'\n");
    } else
        serror(0);

	#endif
}

	/***ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ОШИБОК И Т.Д.**/

int isdelim(char c) {
    //if( c==';' || c=='\n' || c==0 || c==EOF )

    if( c==';' || c=='\n' || c==0 || c==EOF) {

        if(lastDelim == '|')
            Conv = 1;

        lastDelim = c;
        return 1;
    } else if (c=='|') {
        lastDelim = c;
        Conv = 1;
        return 1;
    }

    return 0;
}

void serror(int error) 
{
    static char *e[]= {
        "Ошибка памяти",
        "Несбалансированные кавычки",
        "Нет выражения",
        "Деление на нуль"
    };
    fprintf(stderr, "%s\n", e[error]);
}

void handler_CtrlC(int sig) 
{
    /*
       Остановка текущей работы
    */
    printf("\n");
    INVITE;

    Quit = 1;
}
	


	/***ПРИЁМ ДАННЫХ***/

static inline unsigned long inc(unsigned long *i, char *buf, size_t *bufSize) {
    (*i)++;
    if((*i) >= (*bufSize-1)) { /* -1 для \0 в конце строки */
        *bufSize += bufBlock;
        buf = (char*) realloc(buf, sizeof(char)*(*bufSize));
    }
    return (*i);
}

char * mgets(void) {
    unsigned long i = 0;
    size_t bufSize = bufBlock;
    char *buf = NULL;
    char c;
 
    char quote;
    unsigned _bool = 1;
 
    buf = (char*) realloc(buf, sizeof(char)*bufSize);
    memset(buf, 0, bufSize);
    do {
        c = getchar();
        if(c == EOF) {
            break;
        }

        if(c == '\\') {
            buf[i] = c;
            inc(&i, buf, &bufSize);
            if((c=getchar()) == '\n') {
                if(i > 0)
                    i--;
                if(!isFile) {
                    printf("> ");
                
                }
            } else {
                buf[i] = c;
                inc(&i, buf, &bufSize);
            }
            continue;
        }

        if(c == '#') {
            do {
                c = getchar();
                //while( (c != '\n') || (c != EOF) );

            } while( (c != '\n') && (c != EOF) );
        }

        if(c == '\'' || c == '\"') {
            quote = c;
            buf[i] = c;
            inc(&i, buf, &bufSize);
            do {
                c = getchar();
                if(c == '\n' && !isFile) {
                    printf("> ");
                    //fflush(stdout);
                }
                buf[i] = c;
                inc(&i, buf, &bufSize);
            } while((c != EOF) && (c != quote));
            continue;
        }

        buf[i] = c;
        inc(&i, buf, &bufSize);

        if (isFile)
            _bool = (c != EOF);
        else
            _bool = ((c != EOF) && (buf[i-1] != '\n'));
    } while( _bool );
    buf[i++] = '\0';

    buf = (char*) realloc(buf, sizeof(char)*i);

    return buf;
}
	/***JOB***/

void find_environment(const char *string) {
    char *temp = tempVar;
    char buff[500];
    char *buf = buff;
    char quote;


    memset(buff, 0, 500);
 
    while(  (*string != 0) ) {
        if(*string == '\\') {
            string++;
            *temp++ = *string++;
        } else if(*string == '\'') {
            quote = *string;
            
            *temp++ = *string++;
            while(*string != quote) {
                
                *temp++ = *string++;
            }
            
            *temp++ = *string++;
        } else if(*string == '$') {
            string++;
            if(isdigit(*string)) {
                int p = 0;
                p = (int)(*string) - (int)'0';


                if(argvv[p] != NULL) {
                    buf = argvv[p];

                    while (*buf != 0) {
                        *temp++ = *buf++;
                    }
                    buf = NULL;
                } else {
                    
                    fprintf(stderr, "-ERROR: Неправильный номер аргумента!\n");
                }
                string++;

            } else {
                string += 1;
                while(*string != '}') {
                    *buf++ = *string++;
                }
                string += 1;

                buf  = getenv(buff);
            
                while (*buf != 0) {
                    *temp++ = *buf++;
                }

                memset(buff, 0, 500);
                buf = buff;
            }
        } else {
            *temp++ = *string++;
        }   
    }
}

void parse_prog(void) {
    int i;
    int n = 0;
    char buff[50];
    char *buf;
    char quote;

    char *str = NULL;
    struct _Program *prg = NULL;

    for( n = 0; n < job->n; n++) {
    
        prg = job->programs[n];
       
        str = prg->name;
     
        i = 0;
        while(*str) {
            memset(tempBuf, 0, BUFSIZ);
            temp = tempBuf;
            while((!isspace(*str)) && (*str != 0)) {

                if(*str == '\"') {
                    quote = *str;
                    *temp++ = *str++;
                    while( (*str != quote) && (*str != 0) )
                        *temp++ = *str++;
                    *temp++ = *str++;

                } else if (*str == '\'') {
                    quote = *str;
                    *temp++ = *str++;
                    while( (*str != quote) && (*str != 0) )
                        *temp++ = *str++;
                    *temp++ = *str++;
                } else if( (*str == '<')  ) {
                    memset(buff, 0, 50);
                    buf = buff;

                    str++;
                    while(isspace(*str))
                        str++;

                    while(!isspace(*str) && !isdelim(*str) && (*str != '>') )
                        *buf++ = *str++;
                   

                    prg->input_file = (char*)malloc(sizeof(char)*(strlen(buff)));
                    strcpy(prg->input_file, buff);

                } else if( *str == '>' ) {
                    memset(buff, 0, 50);
                    buf = buff;

                    prg->output_type = 1;
                    str++;
                    if( *str == '>' ) {
                        prg->output_type = 2;
                        str++;
                    }

                    while(isspace(*str))
                        str++;

                    while(!isspace(*str) && !isdelim(*str))
                        *buf++ = *str++;

                    prg->output_file = (char*)malloc(sizeof(char)*(strlen(buff)));
                    strcpy(prg->output_file, buff);

                }

                else {
                    *temp++ = *str++;
                }

            }

            
            if(strlen(tempBuf) > 0) {
                find_environment(tempBuf);
                strcpy(tempBuf, tempVar);
                memset(tempVar, 0, BUFSIZ);
                

                prg->arguments = (char**)realloc(prg->arguments, sizeof(char*)*(i+2));
                prg->arguments[i+1] = NULL;
                prg->arguments[i] = (char*)malloc(sizeof(char)*(strlen(tempBuf)+1));
                strcpy(prg->arguments[i], tempBuf);
                i++;
            }


            while(isspace(*str))
                str++;

            memset(tempBuf, 0, BUFSIZ);

        }

        prg->number_of_arguments = i;

        memset(tempBuf, 0, BUFSIZ);
    }

}

struct _Program *new_prog(void) {

    struct _Program *program = (struct _Program *)malloc(sizeof(struct _Program));
    program->name = (char*)malloc(strlen(tempBuf));
    strcpy(program->name, tempBuf);
    program->number_of_arguments = 0;
    program->arguments = NULL;
    program->input_file = NULL;
    program->output_file = NULL;
    program->output_type = 0;
    if(Conv == 1) {
        program->conveyer = 1;
        job->convcount += 1;
        
        Conv = 0;
    } else {
        program->conveyer = 0;
    }
    

    memset(tempBuf, 0, BUFSIZ);

    
    return program;
}

struct _Program *get_prog(void) {
    memset(tempBuf, 0, BUFSIZ);
    temp = tempBuf;



    if(!*prog)
        return NULL; // конец выражения

    while(isspace(*prog)) // пропустить пробелы, символы табуляции и пустой строки
        ++prog;

    if(isalpha(*prog)) {
        while(!isdelim(*prog))
            *temp++ = *prog++;

        *temp = *prog;
        prog++;


        if(isspace(*temp)) {
            while(isspace(*temp)) // обрезать пробелы, символы табуляции и пустой строки
                --temp;
            temp++;
        }

       
        *temp++ = '\0';
    }
    *temp = '\0';

    return new_prog();
}


int add_progs(void) {
    int i = -1;
    int n =  0;
    struct _Program *prg;

    while( (prg = get_prog()) ) {
        i++;
        n++;
        job->programs = (struct _Program**)realloc(job->programs, n*sizeof(struct _Program*));
        job->programs[i] = (struct _Program *)malloc(sizeof(struct _Program *));
        job->programs[i] = prg;
    }

    return n;
}

void io_redirect(void) {

    int i,j;
    int current_argc;
    struct _Program *current_program = NULL;

    for( i = 0; i < job->n; i++) {
        current_program = job->programs[i];
        current_argc = current_program->number_of_arguments;

        for( j = 0; j < current_argc; j++) {
            if( *(current_program->arguments[j]) == '<' || *(current_program->arguments[j]) == '>') {
                if( *(current_program->arguments[j]) == '<') {
                    current_program->input_file = (char*)malloc(sizeof(char)*(strlen(current_program->arguments[j+1])+1));
                    strcpy(current_program->input_file, current_program->arguments[j+1]);
                }

                if( *(current_program->arguments[j]) == '>') {
                    current_program->output_type = 1;
                    if( *(current_program->arguments[j]+1) == '>')
                        current_program->output_type = 2;

                    current_program->output_file = (char*)malloc(sizeof(char)*(strlen(current_program->arguments[j+1])+1));
                    strcpy(current_program->output_file, current_program->arguments[j+1]);
                }

                free(current_program->arguments[j]);
                current_program->arguments[j] = NULL;
                free(current_program->arguments[j+1]);
                current_program->arguments[j+1] = NULL;
                current_program->number_of_arguments -= 2;
                j++;
            }
        }
    }
}


void init_job(void) {
    job = (struct _Job*)malloc(sizeof(struct _Job));
    job->background = 0;
    job->convcount = 0;
    job->programs = (struct _Program**)malloc(1*sizeof(struct _Program*));
    job->n = add_progs();

    parse_prog();

    io_redirect();
}

/***RUN JOBS***/

int cd(char **args) {
    int cdres = chdir(args[1]);
    //printf("cdres=%d args[1]=%s\n", cdres, args[1]);
    if(cdres == -1) {
        printf("Нет такого каталога %s\n", args[1]);
        return 1;
    }
    return 0;
}


void run_job(void) {
    int i = 0;
    int j = 0;
    int pipes[job->n][2];
    int fd;
    
    for( i = 0; i < job->n; i++) {
        pipe(pipes[i]);
    }

    for( i = 0; i < job->n; i++) {

        if( strcmp(job->programs[i]->arguments[0], "pwd") == 0) {
            char dir[1024];

            getcwd(dir, 1024);
            printf("%s\n", dir);
            continue;
        }
        if( strcmp(job->programs[i]->arguments[0], "cd") == 0) {
            cd(job->programs[i]->arguments);
            continue;
        }
        if( strcmp(job->programs[i]->arguments[0], "exit") == 0) {
            Quit = 1;
            continue;
        }
                for(int k = 0; k < i; k++) {
                    close(pipes[k][0]);
                    close(pipes[k][1]);
                }

                if(i > 0)
                    waitpid(job->programs[i-1]->pid, NULL, 0);

            } 
            wait(NULL);
            i--;
} 

void free_job(void) {
    int i = 0;
    for( i = 0; i < job->n; i++) {
        free(job->programs[i]->name);
        free(job->programs[i]);
    }
    free(job);
}


	/***РАБОТА С ТЕРМИНАЛОМ***/

int init_terminal(void) 
{
    printf("\x1B[2J");
    printf("\x1B[0;0H");

    tcgetattr(0,&old_attributes);
    tcgetattr(0,&new_attributes);

    tcsetattr(0,TCSANOW,&new_attributes);

    return 0;
}

int restore_terminal(void) 
{
    tcsetattr(0,TCSANOW,&old_attributes);
    printf("\x1B[2J");
    printf("\x1B[0;0H");
    return 0;
}

void invite(void) 
{
    printf("\x1b[1;33m");
    printf("%s", getenv("USER"));
    printf("\x1B[0m");
    printf("$ ");
    fflush(stdout);
}
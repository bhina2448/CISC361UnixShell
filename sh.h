
#include "get_path.h"

//int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void cd(char *argument);
void printwd();
void list ( char *dir );
void printpid();
void killsig(char *ar1, char *ar2);
char *changeprompt(char *prefix);
void printenv(char **envp);
void printenvvar(char *name);
void setenvironment(char *variable, char *value);
void execommand(char *commandpath, char **argv, char **envp);
void wildcards(char *commandpath, char **argv, char **envp, int argsct);

#define PROMPTMAX 32
#define MAXARGS 10


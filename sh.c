#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"
#include <glob.h>

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		/* Home directory to start
						  out with*/
     
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  prompt[0] = ' '; prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();

  while ( go )
  {
    /* print your prompt */
 	printf("%s [%s]>",prompt,getcwd(NULL,PATH_MAX+1));	
    /* get command line and process */
	fgets(commandline,MAX_CANON,stdin);
	if(commandline[strlen(commandline)-1]=='\n'){
		commandline[strlen(commandline)-1]=0;
	}
       arg=strtok(commandline," ");
       argsct=0;
       while(arg != NULL){
	       args[argsct]=arg;
	       arg=strtok(NULL, " ");
	       argsct ++;
       }
       command=args[0];
	       
    /* check for each built in command and implement */
       if(strcmp(command, "exit")==0){
	       printf("\nExecuting Built in command exit\n");
	       go=0;
       }
       else if(strcmp(command, "which")==0){
	      if(which(args[1],pathlist)!=NULL){
		      printf("\nComplete\n");
	      }
	      else{
		      printf("\nNot Found\n");
	      }
       }
       else if(strcmp(command,"where")==0){
	       if(where(args[1],pathlist)!=NULL){
		       printf("\nComplete\n");
	       }
	       else{
		       printf("\nNot Found\n");
	       }
       }
       else if(strcmp(command, "cd")==0){
	       if(args[1]==NULL){
		       //home dir
		       cd(homedir);
	       }
	       else{
		       cd(args[1]);
	       }
       }
       else if(strcmp(command, "pwd")==0){
	       printwd();
       }
       else if(strcmp(command, "list")==0){
	       printf("\nExecuting built in command list");
	       //can handle any set of arguments
	       if(args[1]==NULL){
		       //no arguments
		       list(getcwd(NULL, PATH_MAX+1));
	       }
	       else{
		       //call list() for each value in args
		       for(int i=1; i<argsct; i++){
			       printf("\n");
			       printf("\n%s:",args[i]);
			       list(args[i]);
		       }
	       }
	       printf("\nComplete\n");
       }
       else if(strcmp(command, "pid")==0){
	       printpid();
       }
       else if(strcmp(command, "kill")==0){
	       if(argsct>2){
	      	 killsig(args[1],args[2]);
	       }
	       else if(argsct>1){
		       killsig(args[1],NULL);
	       }
	       else{
		       killsig(NULL,NULL);
	       }
       }
       else if(strcmp(command, "prompt")==0){
	       prompt=changeprompt(args[1]);
       }
       else if(strcmp(command,"printenv")==0){
	       if(argsct==1){
		       printf("\nExecuting built in command printenv");
		       printenv(envp);
	       }
	       else if(argsct==2){
		       printenvvar(args[1]);
	       }
	       else{
		       printf("\nError: incorrect number of arguments\n");
	       }
       }
       else if(strcmp(command, "setenv")==0){
	       if(argsct==1){
		       //print environement
		       printf("\nExecuting built in command setenv");
		       printenv(envp);
	       }
	       else if(argsct==2){
		       setenvironment(args[1],NULL);
	       }
	       else if(argsct==3){
		       setenvironment(args[1],args[2]);
		       //check if PATH or HOME was changed
		       if(strcmp(args[1], "HOME")==0){
			       homedir=args[1];
		       }
		       else if(strcmp(args[1], "PATH")==0){
			       struct pathelement *tmp=pathlist;
			       while(tmp!=NULL){
				       free(pathlist->element);
				       tmp=pathlist->next;
				       free(pathlist);
				       pathlist=tmp;
			       }
			       pathlist=get_path();
		       }
	       }
	       else{
		       printf("\nError: incorrect number of arguments\n");
	       }

       }
     /*  else  program to exec */
       else{
	       /*check if path or absolute path*/
	       if(access(command,X_OK)==0){
		       commandpath=command;
	       }
	       else{
		       struct pathelement *p= pathlist;
		       char cmd[64];
		       while(p){
			       sprintf(cmd, "%s/%s",p->element,command);
			       if(access(cmd, X_OK)==0){
				       commandpath=cmd;
				       break;
			       }
			       p=p->next;
		       }
	       }
	       if(commandpath){
		       if(strchr(commandpath,'*')!=NULL){
			       wildcards(commandpath,args,envp,argsct);
		       }
		       else if(strchr(commandpath,'?')!=NULL){
			       wildcards(commandpath,args,envp,argsct);
		       }
		       else{
		       		execommand(commandpath, args, envp);
		       }
	       }
	       else{
		       fprintf(stderr, "%s: Command not found.\n",args[0]);
	       }
	 } 
    }
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist )
{
   /* loop through pathlist until finding command and return it.  Return
   NULL when not found. */
	printf("\nExecuting built in command which");
	char cmd[64];
	while(pathlist){
		sprintf(cmd, "%s/%s", pathlist->element,command);
		if(access(cmd, X_OK)==0){
			printf("\nfound at: [%s]",cmd);
			break;
		}
		pathlist=pathlist->next;
	}
	if(pathlist){
		return command;
	}
	else{
		return NULL;
	}

} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
	printf("\nExecuting built in command where");
	char cmd[64];
	int found=0;
	while(pathlist){
		sprintf(cmd, "%s/%s", pathlist->element,command);
		if(access(cmd, F_OK)==0){
			printf("\n[%s]",cmd);
			found=1;
		}
		pathlist=pathlist->next;
	}
	if(found){
		return command;
	}
	else{
		return NULL;
	}
} /* where() */
/*
 *chdir(2) to a directory given
 with no args- chdir to home directory
 with a "-" arg- chdir to directory previously in
 */
void cd(char *argument){
	printf("\nExecuting built in command cd");
	if(strcmp(argument, "-") ==0){
		if(chdir("..")==0){
			printf("\nComplete\n");
		}
	}
	else{
		if(chdir(argument)==0){
			printf("\nComplete\n");
		}
		else{
			perror("cd");
		}
	}
}/* cd() */

/*
 * Prints the current working directory
 */
void printwd(){
	printf("\nExecuting built in command pwd");
	char *cwd= getcwd(NULL,PATH_MAX+1);
	if(cwd != NULL){
		printf("\nCurrent Working Directory: %s",cwd);
		printf("\nComplete\n");
	}
	else{
		perror("pwd");
	}
}/* printwd() */

/*
 * lists the files in the dir passed to the function
 * uses opendir(),closedir(), and readdir()
 * */
void list ( char *dir )
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
	DIR *opened;
	struct dirent *f;
	opened=opendir(dir);
	if(opened!=NULL){
		//loop through all the files
		for(f=readdir(opened); f!=NULL; f=readdir(opened)){
			printf("\n%s",f->d_name);
		}
	}
	else{
		perror("couldnt open directory");
	}
	closedir(opened);

} /* list() */

/*
 * prints the pid of the shell
 */
void printpid(){
	printf("\nExecuting built in command pid");
	pid_t pid=getpid();
	printf("\nProcess ID: %d", pid);
	printf("\nComplete\n");
}/* printpid() */

/*
 * when given just a pid(ar1) sends a SIGTERM to it with kill(2)
 * when given a signal number(ar1) with a - in front of it, sends that signal to the pid(ar2)
 */
void killsig(char *ar1, char *ar2){
	printf("\nExecuting built in command kill");
	int pid;
	if(ar1==NULL){
		printf("\nno areguments given for kill\n");
	}
	else if(ar2 == NULL){
		//just given the PID
		pid= atoi(ar1);
		if(kill(pid, SIGTERM)==0){
			printf("\nComplete\n");
		}
		else{
			perror("kill");
		}
	}
	else if(ar1[0]=='-'){
		int signal=atoi(ar1)*-1;
		pid=atoi(ar2);
		if(kill(pid,signal)==0){
			printf("\nComplete\n");
		}
		else{
			perror("kill");
		}
	}
	else{
		perror("Invalid arguments for kill command");
	}

}/*killsig()*/

/*
 * when prefix is NULL, prompts user to enter a new prefix for prompt
 * else, prefix becomes new prompt prefix
 * returns new prompt prefix
 */
char *changeprompt(char *prefix){
	printf("\nExecuting built in command prompt");
	char *buf;
	if(prefix ==NULL){
		printf("\ninput prompt prefix:");
		fgets(buf,PROMPTMAX,stdin);
		if(buf[strlen(buf)-1]=='\n'){
			buf[strlen(buf)-1]=0;
		}
		printf("\nComplete\n");
		return buf;
	}
	else{
		printf("\nComplete\n");
		return prefix;
	}
}/*changePrompt()*/

/*
 * when run with no arguments, prints the whole environment
 */
void printenv(char **envp){
	int i=0;
	while(envp[i] != NULL){
		printf("\n%s",envp[i]);
		i++;
	}
	printf("\nComplete\n");
}/*printenv()*/

/*
 * prints the environment variable with the given name using getenv(3)
 */
void printenvvar(char *name){
	printf("\nExecuting built in commant printenv");
	printf("\n%s", getenv(name));
	printf("\nComplete\n");
}/*printenvvar()*/

/*
 * sets the environtment variable given as the value given
 * if no value given, make variable an empty environment variable
 */
void setenvironment(char *variable, char *value){
	printf("\nExecuting built in command setenv");
	if(setenv(variable, value, 1)==0){
		printf("\nComplete\n");
	}
	else{
		perror("setenv");
	}
}/*setenv()*/

/*
 * used to execute a command that is located at the commandpath
 * calls fork() execve() and waitpid()
 */
void execommand(char *commandpath,char **args, char **envp){
	printf("\nExecuting %s",commandpath);
         /*do fork(), execve() and waitpid()*/
        int pid=fork();
        int *st;
        if(pid==0){
		//child
                if(execve(commandpath,args,envp)==-1){
			perror("unable to run command");
                }
        }
        else if(pid<0){
		perror("forking error");
        }
       	else{
		//parent
                waitpid(pid,st,0);
                if(WIFEXITED(*st)){
			printf("\nCommand ran successful with return status %d\n",WEXITSTATUS(*st));
                }
                else{
			perror("waitpid");
                }
         }
}/*execommand*/

/*
 *runs execommand for all of the possible files found from a wildcard
 */
void wildcards(char *commandpath, char **args, char **envp,int argsct){
	printf("\nWildcard discovered");
	glob_t globbuf;
	int indx;
	for(int i=1; i<argsct; i++){
		if((strchr(args[i],'*')!=NULL)||(strchr(args[i],'?')!=NULL)){
			glob(args[i],0, NULL, &globbuf);
			indx=i;
			break;
		}
	}
	for(int i=0;i<globbuf.gl_pathc;i++){
		char **nwargs=args;
		nwargs[indx]=globbuf.gl_pathv[i];
		execommand(commandpath,nwargs,envp);
	}

}/*wildcards()*/

#include "pingpong.h"

//global flags for handler to toggle
int pflag = 0;
int cflag = 0;
// parent and child termination flags
int cterm = 0;
int pterm = 0;




void child_sig_handler(int signum){
  switch (signum) {
  case SIGUSR1: printf("sighandler caught SIGUSR1\n");
                break;
  case SIGUSR2: printf("sighandler caught SIGUSR2\n");
                cterm=1;
                break;
  default:      printf("sighandler caught unexpected signal %d\n",signum);
  } 
}
void parent_sig_handler(int signum){
  switch (signum) {
     case SIGCHLD: printf("sighandler caught SIGUSR1\n");
       pterm=1;
                   break;
     case SIGUSR2: printf("sighandler caught SIGUSR2\n");
                   break;
     default:      printf("sighandler caught unexpected signal %d\n",signum);
  } 
}


FILE * createLogFile(void) { 
  FILE* logfp = NULL;
	logfp = fopen("./log/log_file.txt", "w");
  
	return logfp;
}


int main(int argc, char *argv[]) {

	pid_t pid1;
	pid_t ppid; 
  //parent flag init to true to set chain off
  int countChild = 0;
  int countParent = 0;
  int running = 1; 
  if(argc < 2) {
    printf("expected 2 arguments, received 1. Exiting... \n");
    return -1;
  }

  prctl( PR_SET_PDEATHSIG, SIGUSR2);


  struct sigaction sa;
  sigset_t sigset;


  int maxCount = atoi(argv[1]);
	ppid = getpid();

	//create log file
	FILE* logFilePtr =  NULL;
	logFilePtr = createLogFile();

  if(!logFilePtr){
    printf("log file pointer is null\n"); 
    exit(EXIT_FAILURE);
  }

	int fd = fileno(logFilePtr);

  //printing to standard out will print to log file
  dup2(fd,STDOUT_FILENO);
		pid1 = fork();
		//printf("pid1 = %d\n",pid1);
    //check if child creation failed
		if(pid1 < 0) {
			printf("err: creation of child process unsuccessful\n"); 
			return -1;
		};

    //child execution code
		if(pid1 == 0) {
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0;
      sa.sa_handler = child_sig_handler;
      sigaction(SIGUSR1,&sa,NULL);
      sigaction(SIGUSR2,&sa,NULL);

      //signal polling and handling
      while(running) {
        sigsuspend(&sa.sa_mask);
        printf("SIGUSR1 received in child \n");

        //exit if child receives SIGUSR2 
        if(cterm==1){
          exit(0);
        }
        fflush(stdout); 
        kill(getppid(),SIGUSR2);
        if(countChild == maxCount-1){
          running = 0;
          break;
        }
        countChild++;
      }
        }
    //parent execution code
    else {

      //setting up signal mask
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0;
      sa.sa_handler = parent_sig_handler;
      sigaction(SIGUSR2,&sa,NULL);
      sigaction(SIGCHLD,&sa,NULL);

      //signal polling and handling
      while(running){
        sigsuspend(&sa.sa_mask);
        printf("SIGUSR2 received in parent \n");
        //parent should terminate if child dies
        if(pterm==1){
          exit(0);
        }
        fflush(stdout); 
        kill(pid1,SIGUSR1);
        if(countParent == maxCount-1){
          running = 0;
          break;
        }
        countParent++;
      }

	// Waits for all the Mapper children
	pid_t terminated_pid_mappers;
	while((terminated_pid_mappers = wait(NULL)) > 0)
	{
		printf("Terminated a child process (pid: %d)\n", terminated_pid_mappers);
	}

	return EXIT_SUCCESS;

	fclose(logFilePtr);
  }
}

#include "pingpong.h"

// parent and child termination flags
int cterm = 0;
int pterm = 0;

//child signal handler
void child_sig_handler(int signum){
  switch (signum) {
  case SIGUSR1: break;
  case SIGUSR2: printf("Child-sighandler caught SIGUSR2\n");
                cterm=1;
                break;
  default:      printf("Child-sighandler caught unexpected signal %d\n",signum);
  } 
}

//parent signal handler
void parent_sig_handler(int signum){
  switch (signum) {
     case SIGCHLD: printf("Parent-sighandler caught SIGCHLD\n");
       		   pterm=1;
                   break;
     case SIGUSR2: break;
     default:      printf("Parent-sighandler caught unexpected signal %d\n",signum);
  } 
}


//creates log file 
FILE * createLogFile(void) { 
  FILE* logfp = NULL;
	logfp = fopen("./log/log_file.txt", "w");
  
	return logfp;
}


int main(int argc, char *argv[]) {

  pid_t pid1;
  pid_t ppid; 
  int countChild = 0;
  int countParent = 0;
  int running = 1; 

//argument checking
  if(argc < 2) {
    printf("Expected 2 arguments, received 1. Exiting... \n");
    return -1;
  }

//send SIGUSR2 to child if parent terminates
  prctl(PR_SET_PDEATHSIG, SIGUSR2);

//construct signal struct
  struct sigaction sa;
  sigset_t sigset;

  int maxCount = atoi(argv[1]);
	ppid = getpid();

  //create log file
  FILE* logFilePtr =  NULL;
  logFilePtr = createLogFile();

//check if log file was created successfully
  if(!logFilePtr){
    printf("Log file pointer is null\n"); 
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
	printf("CHILD::child process executing with LIMIT = %d\n",maxCount);
	printf("CHILD::child process id = %d\n",getpid());
	fflush(stdout);
  //initializing signal struct, mask, and handler
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0;
      sa.sa_handler = child_sig_handler;
  //signals child is suposed to receive
      sigaction(SIGUSR1,&sa,NULL);
      sigaction(SIGUSR2,&sa,NULL);
  //child will receive signal if parent terminates
      prctl(PR_SET_PDEATHSIG, SIGUSR2, 0, 0, 0);

  //signal polling and handling
      while(running) {
        sigsuspend(&sa.sa_mask);

        //exit if child receives SIGUSR2 
        if(cterm==1){
		printf("Parent terminated, terminating child process. . .\n");
        	exit(0);
        }

        printf("Child %d\n",countChild);
        fflush(stdout); 
  //signal parent
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
  //logging
	printf("PARENT::parent process executing with LIMIT = %d\n",maxCount);
	printf("PARENT::parent process id = %d\n",getpid());
	fflush(stdout);

  //setting up signal mask
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0;
      sa.sa_handler = parent_sig_handler;
      sigaction(SIGUSR2,&sa,NULL);
      sigaction(SIGCHLD,&sa,NULL);

      //signal polling and handling
      while(running){
        sigsuspend(&sa.sa_mask);
        if(pterm==1){
	  printf("Child terminated, terminating parent process. . .\n");
          exit(0);
        }
        printf("Parent %d\n",countParent);

        //parent should terminate if child dies
        fflush(stdout); 
  //signal child
        kill(pid1,SIGUSR1);
        if(countParent == maxCount-1){
          running = 0;
          break;
        }
        countParent++;
      }

	// parent waits for child to exit
	pid_t terminated_pid_mappers;
	while((terminated_pid_mappers = wait(NULL)) > 0)
	{
		printf("Terminated a child process (pid: %d)\n", terminated_pid_mappers);
	}

	return EXIT_SUCCESS;
  //closing log file
	fclose(logFilePtr);
  }
}

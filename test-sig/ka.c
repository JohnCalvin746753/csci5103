#define _POSIX_SOURCE
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void catcher(int signum) {
  puts("catcher() has gained control");
}

int main() {
  struct   sigaction sact;
  sigset_t sigset;

  sigemptyset(&sact.sa_mask);
  sact.sa_flags = 0;
  sact.sa_handler = catcher;
  sigaction(SIGUSR1, &sact, NULL);

  sigfillset(&sigset);
  sigprocmask(SIG_SETMASK, &sigset, NULL);

  puts("before kill()");
  kill(getpid(), SIGUSR1);

  puts("before unblocking SIGUSR1");
  sigdelset(&sigset, SIGUSR1);
  sigprocmask(SIG_SETMASK, &sigset, NULL);
  puts("after unblocking SIGUSR1");
}

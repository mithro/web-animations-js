#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <stdio.h>
#include <sys/wait.h>

#if !defined(CLONE_NEWPID)
#define CLONE_NEWPID 0x20000000
#endif

int worker(void* arg) {
  const pid_t pid = getpid();
  if (pid == 1) {
    printf("PID namespaces are working\n");
    return 0;
  } else {
    printf("PID namespaces ARE NOT working. Child pid: %d\n", pid);
    return 1;
  }
}

int main() {
  char stack[8192];
  const pid_t child = clone(worker, stack + sizeof(stack), CLONE_NEWPID, NULL);
  if (child == -1) {
    perror("clone");
    fprintf(stderr, "Clone failed. PID namespaces ARE NOT supported\n");
    return 1;
  }

  int status;
  do {
    waitpid(child, &status, 0);
  } while(!WIFEXITED(status));

  return WEXITSTATUS(status);
}

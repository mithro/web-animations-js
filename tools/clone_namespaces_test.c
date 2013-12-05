#define _GNU_SOURCE
#include <netdb.h>
#include <unistd.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#if !defined(CLONE_NEWPID)
#define CLONE_NEWPID 0x20000000
#endif

int pid_namespaces(void* arg) {
  const pid_t pid = getpid();
  if (pid == 1) {
    fprintf(stderr, "PID namespaces are working\n");
    return 0;
  } else {
    fprintf(stderr, "PID namespaces ARE NOT working. Child pid: %d\n", pid);
    return 1;
  }
}

int port = 0;

int net_socket_bind() {
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo(NULL, "0", &hints, &res) == -1) {
    perror("getaddrinfo\n");
    return -1;
  }

  ((struct sockaddr_in*)res->ai_addr)->sin_port = port;

  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
    perror("bind\n");
    return -1;
  }

  if (listen(sockfd, 0) == -1) {
    perror("listen");
    return -1;
  }

  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(sockfd, (struct sockaddr*)&sin, &len) == -1) {
    perror("getsockname\n");
    return -1;
  } else {
    return ntohs(sin.sin_port);
  }
}

int net_namespaces(void* arg) {
  // If we are in a new net namespace we should be able to bind to the same
  // socket number again.
  if (net_socket_bind() == -1) {
    fprintf(stderr, "NET namespaces ARE NOT working\n");
    return 1;
  } else {
    fprintf(stderr, "NET namespaces are working\n");
    return 0;
  }
}

int main() {
  // Check pid namespaces are supported
  fprintf(stderr, "Checking PID namespaces\n");
  char pid_stack[8192];
  int pid_status;
  const pid_t pid_child = clone(pid_namespaces, pid_stack + sizeof(pid_stack), CLONE_NEWPID, NULL);
  if (pid_child == -1) {
    perror("clone\n");
    fprintf(stderr, "Clone failed. PID namespaces ARE NOT supported\n");
  } else {
    do {
      waitpid(pid_child, &pid_status, WNOHANG);
    } while(!WIFEXITED(pid_status));
  }

  // Check net namespaces are supported
  fprintf(stderr, "Checking NET namespaces\n");
  if ((port = net_socket_bind()) == -1) {
    return 1;
  }

  char net_stack[8192];
  int net_status;
  const pid_t net_child = clone(net_namespaces, net_stack + sizeof(net_stack), CLONE_NEWNET, NULL);
  if (net_child == -1) {
    perror("clone\n");
    fprintf(stderr, "Clone failed. NET namespaces ARE NOT supported\n");
  } else {
    do {
      waitpid(net_child, &net_status, WNOHANG);
    } while(!WIFEXITED(net_status));
  }

  return WEXITSTATUS(pid_status) == 0 && WEXITSTATUS(pid_status) == 0;
}

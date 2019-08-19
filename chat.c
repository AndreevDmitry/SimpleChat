#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server.h"
#include "client.h"


void help(void);

int main(int argc, char *argv[])
{
  unsigned long serverAddress;
  int serverPort;

  if (argc == 1)
  {
    printf("Too few arguments\n");
    help();
  }
  else if (argc == 2)
  {
    serverPort = atoi(argv[1]);
    printf("Start server on port %d\n", serverPort);
    server((unsigned short)serverPort);
  }
  else if (argc == 3)
  {
    serverAddress = inet_addr(argv[1]);
    serverPort = atoi(argv[2]);
    printf("Start client on port %d\n", serverPort);
    printf("Use /exit or Ctrl+D for leave chat\n");
    client(serverAddress, serverPort);
  }
  else
  {
    printf("Too many arguments\n");
    help();
  }

  return 0;
}

void help(void)
{
  printf("Pass port only if you want to run a server\n");
  printf("Pass IP and port of server for run client\n");
}

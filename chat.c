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
  int serverPort;

  if (argc == 1)
  {
    printf("Too few arguments\n");
    help();
  }
  else if (argc == 2)
  {
    serverPort = atoi(argv[1]);
    if (serverPort > 1024 && serverPort < 65535)
    {
      server(argv[1]);
    }
    else
    {
      perror("Port is not correct");
      exit(EXIT_FAILURE);
    }
  }
  else if (argc == 3)
  {
    client(argv[1], argv[2]);
  }
  else
  {
    printf("Too many arguments\n");
    help();
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}

void help(void)
{
  printf("Pass port (e.g. 2115) only if you want to run a server\n");
  printf("Pass IP (e.g. 127.0.0.1) and port (e.g 2115) of server for run client\n");
}

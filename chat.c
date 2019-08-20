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
  unsigned int serverAddress;
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
      printf("Start server on port %d\n", serverPort);
      printf("Use Ctrl+C for shutdown the server\n");
      server((unsigned short)serverPort);
    }
    else
    {
      perror("Port is not correct");
      exit(EXIT_FAILURE);
    }
  }
  else if (argc == 3)
  {
    if(inet_pton(AF_INET, argv[1], &serverAddress) > 0)
    {
      serverPort = atoi(argv[2]);
      if (serverPort > 1024 && serverPort < 65535)
      {
        printf("Start client, connect with server at port %d\n", serverPort);
        printf("Use /QUIT or Ctrl+D for leave chat\n");
        client(serverAddress, serverPort);
      }
      else
      {
        perror("Port is not correct");
        exit(EXIT_FAILURE);
      }
    }
    else
    {
      perror("IP address is not correct");
      help();
      exit(EXIT_FAILURE);
    }
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

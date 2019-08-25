#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MSG_SIZE 1100

static int udpClientInit(char *pNode, char *pService);
static void sendMsgActivity(int socketDescriptor, pthread_t *threadId);
static void *recvMsgThread(void *socketDescriptor);

int client(char *pNode, char *pService)
{
  int socketDescriptor;
  pthread_t threadId;
  pthread_attr_t threadAttr;

  socketDescriptor = udpClientInit(pNode, pService);

  /*Receiver and sender should be runned in separated threads, because reading
    from stdin can block thread indefinitely and messages will not be received
    in time*/
  pthread_attr_init(&threadAttr);
  pthread_create(&threadId, &threadAttr, (void *)recvMsgThread, &socketDescriptor);

  sendMsgActivity(socketDescriptor, &threadId);
  close(socketDescriptor);
  return EXIT_SUCCESS;
}

static int udpClientInit(char *pNode, char *pService)
{
  int status;
  int socketDescriptor;
  struct addrinfo preSettings = {};
  struct addrinfo *pServerInfo;

  preSettings.ai_family = AF_UNSPEC;
  preSettings.ai_socktype = SOCK_DGRAM;
  preSettings.ai_flags = AI_PASSIVE;

  status = getaddrinfo(pNode, pService, &preSettings, &pServerInfo);
  if (status != 0)
  {
    fprintf(stderr, "\ngetaddrinfo error: %s\n", gai_strerror(status));
    exit(EXIT_FAILURE);
  }

  socketDescriptor = socket(pServerInfo->ai_family,
                            pServerInfo->ai_socktype,
                            pServerInfo->ai_protocol);
  if (socketDescriptor < 0)
  {
      perror("\nSocket creation failed\n");
      exit(EXIT_FAILURE);
  }

  status = connect(socketDescriptor, pServerInfo->ai_addr, pServerInfo->ai_addrlen);
  if(status < 0)
  {
    perror("\nError : Connect Failed\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(pServerInfo);
  return socketDescriptor;
}

static void *recvMsgThread(void *socketDescriptor)
{
  char serverMsg[MSG_SIZE];
  int recvStatus;
  do
  {
    recvStatus = recv(*((int *)socketDescriptor), serverMsg, sizeof(serverMsg), 0);
    puts(serverMsg);
  }
  while (recvStatus > 0);
  pthread_exit(NULL);
}

static void sendMsgActivity(int socketDescriptor, pthread_t *threadId)
{
  char message[MSG_SIZE];
  size_t sendStatus;

  printf("\nWelcome to the SimpleChat\nWhat is your name?\n\nMy name (max 24 symbols) is: ");

  do
  {
    /*Ctrl+D handling*/
    if (fgets(message, sizeof(message), stdin) == NULL)
    {
      strcpy(message, "/QUIT\n");
    }

    sendStatus = send(socketDescriptor, &message, strlen(message), 0);
    if (sendStatus == -1)
    {
      perror("sendto failed");
      break;
    }

    if(strcmp(message, "/QUIT\n") == 0)
    {
      pthread_cancel(*threadId);
      break;
    }
  }
  while(1);
}

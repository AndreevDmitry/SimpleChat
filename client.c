#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MSG_SIZE 1100

static int udpClientInit(unsigned long serverIp, unsigned short serverPort);
static void sendMsgActivity(int socketDescriptor, pthread_t *tid);
static void *recvMsgThread(void *socketDescriptor);

int client(unsigned long serverIp, unsigned short serverPort)
{
  int socketDescriptor;
  pthread_t tid;
  pthread_attr_t attr;

  socketDescriptor = udpClientInit(serverIp, serverPort);

  /*Receiver and sender should be runned in separated threads, because reading
    from stdin can block thread indefinitely and messages will not be received
    in time*/
  pthread_attr_init(&attr);
  pthread_create(&tid,&attr,(void *)recvMsgThread, &socketDescriptor);

  sendMsgActivity(socketDescriptor, &tid);
  close(socketDescriptor);
  return EXIT_SUCCESS;
}

static int udpClientInit(unsigned long serverIp, unsigned short serverPort)
{
  int connectionStatus;
  int socketDescriptor;
  struct sockaddr_in serverAddress;

  socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if ( socketDescriptor < 0 )
  {
      perror("UDP socket creation failed");
      exit(EXIT_FAILURE);
  }

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(serverPort);
  serverAddress.sin_addr.s_addr = serverIp;

  connectionStatus = connect(socketDescriptor,
                             (struct sockaddr *)&serverAddress,
                             sizeof(struct sockaddr_in));
  if(connectionStatus < 0)
  {
    perror("\n Error : Connect Failed \n");
    exit(EXIT_FAILURE);
  }
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

static void sendMsgActivity(int socketDescriptor, pthread_t *tid)
{
  char message[MSG_SIZE];
  size_t sendtoStatus;

  do
  {
    if (fgets(message, sizeof(message), stdin) == NULL)
    {
      strcpy(message, "/QUIT\n");
    }

    sendtoStatus = send(socketDescriptor, &message, strlen(message), 0);
    if (sendtoStatus == -1)
    {
      perror("sendto failed");
      break;
    }

    if(strcmp(message, "/QUIT\n") == 0)
    {
      pthread_cancel(*tid);
      break;
    }
  }
  while(1);
}

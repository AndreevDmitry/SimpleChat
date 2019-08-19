#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MSG_SIZE 1000

typedef struct
{
  int socketDescriptor;
  struct sockaddr_in serverAddress;
}
tUdp;

void udpClientInit(tUdp *pUdp, unsigned long serverIp, unsigned short serverPort);
void sendMsgActivity(tUdp *pUdp);
void *recvMsgThread(void *pUdp);

int client(unsigned long serverIp, unsigned short serverPort)
{
  tUdp udp = {};
  pthread_t tid;
  pthread_attr_t attr;

  udpClientInit(&udp, serverIp, serverPort);

  /*Receiver and sender should be runned in separated threads, because reading
    from stdin can block thread indefinitely and messages will not be received
    in time*/
  pthread_attr_init(&attr);
  pthread_create(&tid,&attr,(void *)recvMsgThread, &udp);

  sendMsgActivity(&udp);

  pthread_join(tid,NULL);

  return EXIT_SUCCESS;
}

void udpClientInit(tUdp *pUdp, unsigned long serverIp, unsigned short serverPort)
{
  int connectionStatus;

  pUdp->socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if ( pUdp->socketDescriptor < 0 )
  {
      perror("UDP socket creation failed");
      exit(EXIT_FAILURE);
  }

  pUdp->serverAddress.sin_family = AF_INET;
  pUdp->serverAddress.sin_port = htons(serverPort);
  pUdp->serverAddress.sin_addr.s_addr = serverIp;

  connectionStatus = connect(pUdp->socketDescriptor,
                             (struct sockaddr *)&(pUdp->serverAddress),
                             sizeof(struct sockaddr_in));
  if(connectionStatus < 0)
  {
     perror("\n Error : Connect Failed \n");
     exit(EXIT_FAILURE);
   }
}

void *recvMsgThread(void *pUdp)
{
  char serverMsg[MSG_SIZE];

  do
  {
    recv(((tUdp*)pUdp)->socketDescriptor,
             serverMsg,
             sizeof(serverMsg),
             0);
    puts(serverMsg);
  }
  while (1);
}

void sendMsgActivity(tUdp *pUdp)
{
  char clientMsg[MSG_SIZE];
  size_t sendtoStatus;

  do
  {
    if (fgets(clientMsg, sizeof(clientMsg), stdin) == NULL)
    {
      strcpy(clientMsg, "/closeClient\n");
    }

    sendtoStatus = send(pUdp->socketDescriptor,
                          &clientMsg,
                          strlen(clientMsg),
                          0);
    if (sendtoStatus == -1)
    {
      perror("sendto failed");
      break;
    }

    if(strcmp(clientMsg, "/closeClient\n") == 0)
    {
      close(pUdp->socketDescriptor);
      exit(EXIT_SUCCESS);
    }
  }
  while(1);
}

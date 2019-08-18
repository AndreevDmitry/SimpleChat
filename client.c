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

void udpInit(tUdp *pUdp, unsigned long serverIp, unsigned short serverPort);
void sendMsgActivity(tUdp *pUdp);
void recvMsgThread(void *pUdp); /* Thread function */

int client(unsigned long serverIp, unsigned short serverPort)
{
  tUdp udp = {};
  pthread_t tid;
  pthread_attr_t attr;

  udpInit(&udp, serverIp, serverPort);

  /*Receiver and sender should be runned in separated threads, because reading
    from stdin can block thread indefinitely and messages will not be received
    in time*/
  pthread_attr_init(&attr);
  pthread_create(&tid,&attr,(void *)recvMsgThread, &udp);

  sendMsgActivity(&udp);

  pthread_join(tid,NULL);

  close(udp.socketDescriptor);
  return 0;
}

void udpInit(tUdp *pUdp, unsigned long serverIp, unsigned short serverPort)
{
  int connectionStatus;
  // Creating socket descriptor
  pUdp->socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if ( pUdp->socketDescriptor < 0 )
  {
      perror("UDP socket creation failed");
      exit(EXIT_FAILURE);
  }

  // Filling server information
  pUdp->serverAddress.sin_family = AF_INET;
  pUdp->serverAddress.sin_port = htons(serverPort);
  pUdp->serverAddress.sin_addr.s_addr = serverIp;

  // connect to server
  connectionStatus = connect(pUdp->socketDescriptor,
                             (struct sockaddr *)&(pUdp->serverAddress),
                             sizeof((pUdp->serverAddress)));
  if(connectionStatus < 0)
  {
     perror("\n Error : Connect Failed \n");
     exit(EXIT_FAILURE);
   }
}

void recvMsgThread(void *pUdp)
{
  char serverMsg[MSG_SIZE];

  do
  {
    recv(((tUdp*)pUdp)->socketDescriptor,
             serverMsg,
             sizeof(serverMsg),
             0);

    if (strcmp(serverMsg, "/exit\n") == 0)
    {
      pthread_exit(0);
      break;
    }
    else
    {
      puts(serverMsg);
    }
  }
  while(1);
}

void sendMsgActivity(tUdp *pUdp)
{
  char clientMsg[MSG_SIZE];
  size_t sendtoStatus;

  do
  {
    if (fgets(clientMsg, sizeof(clientMsg), stdin) == NULL)
    {
      perror("fgets failed");
      break;
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

    if(strcmp(clientMsg, "/exit\n") == 0)
    {
       break;
    }
  }
  while(1);
}

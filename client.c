#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef struct
{
  int socketDescriptor;
  struct sockaddr_in serverAddress;
}
tUdp;

#define PORT     2115
#define MSG_SIZE 1000

void udpInit(tUdp *pUdp);
void sendMsgActivity(tUdp *pUdp);
void recvMsgThread(void *pUdp); /* Thread function */

int main()
{
  tUdp udp = {};

  pthread_t tid;
  pthread_attr_t attr;

  udpInit(&udp);

  /* Get default attributes */
  pthread_attr_init(&attr);
  /* Create new thread*/
  pthread_create(&tid,&attr,(void *)recvMsgThread, &udp);

  sendMsgActivity(&udp);
  pthread_exit(0);
  /* Wait for end of thread execution */
  pthread_join(tid,NULL);

  close(udp.socketDescriptor);
  return 0;
}

void udpInit(tUdp *pUdp)
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
  pUdp->serverAddress.sin_port = htons(PORT);
  pUdp->serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

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
  char buffer[MSG_SIZE];

  do
  {
    recvfrom(((tUdp*)pUdp)->socketDescriptor,
             buffer,
             sizeof(buffer),
             0,
             (struct sockaddr*)NULL,
             NULL);
    puts(buffer);
  }
  while(strcmp(buffer, "/exit\n") != 0);
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

    if(strcmp(clientMsg, "/exit\n") == 0)
    {
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
  }
  while(1);
}

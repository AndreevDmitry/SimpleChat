#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define MSG_SIZE 1000
#define SERVER_ONLINE 1
#define SERVER_0FFLINE 0

typedef struct
{
  int socketDescriptor;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
}
tUdp;

static unsigned char serverControl = SERVER_ONLINE;
static struct sockaddr_in clientDatabase[255];
static unsigned char lastClientIndex = 0;

void udpServerInit(tUdp *pUdp, unsigned short port);
void recvSendMsgActivity(tUdp *pUdp);
void *waitExitThread(void *pUdp);
void sendToAll(tUdp *pUdp, char * buffer);

int server(unsigned short port)
{
  tUdp udp = {};
  pthread_t tid;
  pthread_attr_t attr;

  udpServerInit(&udp, port);

  pthread_attr_init(&attr);
  pthread_create(&tid,&attr,(void *)waitExitThread, &udp);

  recvSendMsgActivity(&udp);

  pthread_join(tid,NULL);

  return EXIT_SUCCESS;
}

void udpServerInit(tUdp *pUdp, unsigned short port)
{
  int bindStatus;

  pUdp->socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if(pUdp->socketDescriptor < 0)
  {
      perror("UDP socket creation failed");
      exit(EXIT_FAILURE);
  }
  pUdp->serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  pUdp->serverAddress.sin_port = htons(port);
  pUdp->serverAddress.sin_family = AF_INET;

  bindStatus = bind(pUdp->socketDescriptor,
                    (struct sockaddr*)&(pUdp->serverAddress),
                    sizeof(struct sockaddr_in));

  if(bindStatus < 0)
  {
    perror("\n Error : Bind Failed \n");
    exit(EXIT_FAILURE);
  }
}

void recvSendMsgActivity(tUdp *pUdp)
{
  char buffer[MSG_SIZE];
  unsigned char clientExists = 0;
  socklen_t socketLengh = sizeof(struct sockaddr_in);

  while(serverControl)
  {
    clientExists = 0;
    int n = recvfrom(pUdp->socketDescriptor,
                     buffer,
                     sizeof(buffer),
                     0,
                     (struct sockaddr*)&(pUdp->clientAddress),
                     &socketLengh);

    buffer[n] = '\0';
    puts(buffer);

    /*Add new client to dispribution list*/
    for(unsigned char clientNumber = 0; clientNumber < lastClientIndex; clientNumber++)
    {
       if (pUdp->clientAddress.sin_addr.s_addr == clientDatabase[clientNumber].sin_addr.s_addr &&
           pUdp->clientAddress.sin_port == clientDatabase[clientNumber].sin_port)
        {
          clientExists = 1;
          break;
        }
    }
    if (clientExists == 0)
    {
      clientDatabase[lastClientIndex] = pUdp->clientAddress;
      lastClientIndex++;
    }

    sendToAll(pUdp, buffer);
  }
}

void *waitExitThread(void *pUdp)
{
  char* fgetsStatus;
  char controlMsg[MSG_SIZE];

  fgetsStatus = fgets(controlMsg, sizeof(controlMsg), stdin);
  if ((fgetsStatus == NULL) || (strcmp(controlMsg, "/exit\n") == 0))
  {
     serverControl = SERVER_0FFLINE;
     strcpy(controlMsg, "Server goes to offline...\n");
     puts(controlMsg);
     sendToAll(((tUdp*)pUdp), controlMsg);
     close(((tUdp*)pUdp)->socketDescriptor);
     exit(EXIT_SUCCESS);
  }
}

void sendToAll(tUdp *pUdp, char * buffer)
{
  // send received message to all known clients
  for (unsigned char clientNumber = 0; clientNumber < lastClientIndex; clientNumber++)
  {
    sendto(pUdp->socketDescriptor,
           buffer,
           MSG_SIZE,
           0,
           (struct sockaddr*)&(clientDatabase[clientNumber]),
           sizeof(struct sockaddr_in));
  }
}

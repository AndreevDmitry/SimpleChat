#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define MSG_SIZE 1000
#define UNKNOWN_CLIENT (char)(-1)

typedef struct
{
  int socketDescriptor;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
}
tUdp;

static struct sockaddr_in clientDatabase[255];
static unsigned char nextClientIndex = 0;

void udpServerInit(tUdp *pUdp, unsigned short port);
void recvSendMsgActivity(tUdp *pUdp);
void sendToAll(tUdp *pUdp, char * buffer);
char clientSearch(tUdp *pUdp);
void clientHandler(tUdp *pUdp, char currentClient, char *buffer);

int server(unsigned short port)
{
  tUdp udp = {};

  udpServerInit(&udp, port);
  recvSendMsgActivity(&udp);
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
  socklen_t socketLengh = sizeof(struct sockaddr_in);
  char currentClient;

  while(1)
  {
    int n = recvfrom(pUdp->socketDescriptor,
                     buffer,
                     sizeof(buffer),
                     0,
                     (struct sockaddr*)&(pUdp->clientAddress),
                     &socketLengh);

    buffer[n] = '\0';
    puts(buffer);

    currentClient = clientSearch(pUdp);
    clientHandler(pUdp, currentClient, buffer);
    sendToAll(pUdp, buffer);
  }
}

void sendToAll(tUdp *pUdp, char *buffer)
{
  // send received message to all known clients
  for (unsigned char clientNumber = 0; clientNumber < nextClientIndex; clientNumber++)
  {
    sendto(pUdp->socketDescriptor,
           buffer,
           MSG_SIZE,
           0,
           (struct sockaddr*)&(clientDatabase[clientNumber]),
           sizeof(struct sockaddr_in));
  }
}

char clientSearch(tUdp *pUdp)
{
  char clientPosition = UNKNOWN_CLIENT;

  for(unsigned char clientNumber = 0; clientNumber < nextClientIndex; clientNumber++)
  {
     if (pUdp->clientAddress.sin_addr.s_addr == clientDatabase[clientNumber].sin_addr.s_addr &&
         pUdp->clientAddress.sin_port == clientDatabase[clientNumber].sin_port)
      {
        clientPosition = clientNumber;
      }
  }
  return clientPosition;
}

void clientHandler(tUdp *pUdp, char currentClient, char *buffer)
{
  /*Respond with same message for current client and all known clients*/
  if (currentClient == UNKNOWN_CLIENT)
  {

     sendto(pUdp->socketDescriptor,
           buffer,
           MSG_SIZE,
           0,
           (struct sockaddr*)&(pUdp->clientAddress),
           sizeof(struct sockaddr_in));
     sendToAll(pUdp, buffer);
   }

  /*Add new client to the end of clientDatabase*/
  if ((currentClient == UNKNOWN_CLIENT) && !(strcmp(buffer, "/closeClient\n") == 0))
  {
    sprintf(buffer,
           "Client with IP: %d, port: %d connected to the chat\n",
           pUdp->clientAddress.sin_addr.s_addr,
           pUdp->clientAddress.sin_port);
    puts(buffer);
    clientDatabase[nextClientIndex] = pUdp->clientAddress;
    nextClientIndex++;
  }
  /*Remove client from clientDatabase when it leave chat*/
  else if ((currentClient > UNKNOWN_CLIENT) && (strcmp(buffer, "/closeClient\n") == 0))
  {
    sprintf(buffer,
           "Client with IP: %d, port: %d leave the chat\n",
           clientDatabase[(unsigned char)currentClient].sin_addr.s_addr,
           clientDatabase[(unsigned char)currentClient].sin_port);
    puts(buffer);
    if (nextClientIndex > 0)
    {
      clientDatabase[(unsigned char)currentClient] = clientDatabase[(unsigned char)(nextClientIndex-1)];
      memset (&(clientDatabase[nextClientIndex-1]), 0, sizeof(struct sockaddr_in));
      nextClientIndex--;
    }
    else
    {
      memset (&(clientDatabase[nextClientIndex]), 0, sizeof(struct sockaddr_in));
    }
  }
}

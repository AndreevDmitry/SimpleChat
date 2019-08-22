#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define HANDLE_BUF_SIZE 100
#define MSG_SIZE 1000 + HANDLE_BUF_SIZE

#define USERNAME_LENGTH 24
#define MAX_CLIENTS 255
#define UNKNOWN_CLIENT (char)(-1)

typedef struct
{
  int socketDescriptor;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
} tUdp;

typedef struct
{
  char name[USERNAME_LENGTH];
  struct sockaddr_in address;
} tClientData;

static tClientData clientData[MAX_CLIENTS] = {};
static unsigned char nextClientIndex = 0;

static void udpServerInit(tUdp *pUdp, unsigned short port);
static void recvSendMsgActivity(tUdp *pUdp);
static void clientHandler(struct sockaddr_in *pCurrentClient, char *buffer);
static void sendToAll(int socket, char *buffer);

int server(unsigned short port)
{
  tUdp udp = {};

  udpServerInit(&udp, port);
  recvSendMsgActivity(&udp);
  return EXIT_SUCCESS;
}

static void udpServerInit(tUdp *pUdp, unsigned short port)
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

static void recvSendMsgActivity(tUdp *pUdp)
{
  char buffer[MSG_SIZE];
  socklen_t socketLengh = sizeof(struct sockaddr_in);

  while(1)
  {
    int n = recvfrom(pUdp->socketDescriptor,
                     buffer,
                     sizeof(buffer),
                     0,
                     (struct sockaddr*)&(pUdp->clientAddress),
                     &socketLengh);

    buffer[n] = '\0';

    clientHandler(&(pUdp->clientAddress), buffer);
    sendToAll(pUdp->socketDescriptor, buffer);
  }
}

static void clientHandler(struct sockaddr_in *pCurrentClient, char *buffer)
{
  char clientPosition = UNKNOWN_CLIENT;
  static char handlerBuffer[HANDLE_BUF_SIZE] = {0};

  /*Search current client in data base*/
  for(unsigned char i = 0; i <= nextClientIndex; i++)
  {
     if (pCurrentClient->sin_addr.s_addr == clientData[i].address.sin_addr.s_addr &&
         pCurrentClient->sin_port == clientData[i].address.sin_port)
      {
        clientPosition = i;
        break;
      }
  }

  if ((clientPosition == UNKNOWN_CLIENT) && !(strcmp(buffer, "/QUIT\n") == 0))
  {
    if (nextClientIndex == MAX_CLIENTS-1)
    {
      sprintf(handlerBuffer,
             "Chat is full\nClient with IP: %d, port: %d WILL NOT be add the chat\n",
             pCurrentClient->sin_addr.s_addr,
             pCurrentClient->sin_port);
    }
    else
    { /*Add new client to the end of clientDatabase*/
      memcpy(&(clientData[nextClientIndex].name), buffer, (strlen(buffer)));
      sprintf(handlerBuffer, "connected to the chat\n");
      strcat(buffer, handlerBuffer);
      clientData[nextClientIndex].address = *pCurrentClient;
      nextClientIndex++;
    }
  }
  else if (clientPosition > UNKNOWN_CLIENT)
  {
    if (strcmp(buffer, "/QUIT\n") == 0)
    { /*Remove client from clientDatabase when it leave chat*/
      memset(buffer, 0, MSG_SIZE);
      strcpy(buffer, clientData[(unsigned char)clientPosition].name);
      strcat(buffer, "leave the chat\n");
      if (nextClientIndex > 0)
      {
        clientData[(unsigned char)clientPosition].address = clientData[(unsigned char)(nextClientIndex-1)].address;
        memcpy(&(clientData[(unsigned char)clientPosition].name),
               &(clientData[nextClientIndex-1].name),
               USERNAME_LENGTH);
        nextClientIndex--;
      }
    }
    else /*Add current user name to input message*/
    {
      strcat(buffer, clientData[(unsigned char)clientPosition].name);
    }
  }
}

static void sendToAll(int socket, char *buffer)
{
  // send received message to all known clients
  for (unsigned char i = 0; i < nextClientIndex; i++)
  {
    sendto(socket,
           buffer,
           MSG_SIZE,
           0,
           (struct sockaddr*)&(clientData[i].address),
           sizeof(struct sockaddr_in));
  }
  // And to server screen
  puts(buffer);
}

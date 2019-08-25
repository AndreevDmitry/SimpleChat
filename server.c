#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

#define HANDLE_BUF_SIZE 100
#define MSG_SIZE 1000 + HANDLE_BUF_SIZE

#define USERNAME_LENGTH 24
#define MAX_CLIENTS 255
#define UNKNOWN_CLIENT (char)(-1)

typedef struct
{
  char name[USERNAME_LENGTH];
  struct sockaddr_in address;
} tClientData;

static tClientData clientData[MAX_CLIENTS] = {};
static unsigned char nextClientIndex = 0;

static int udpServerInit(char *pService);
static void recvSendMsgActivity(int socketDescriptor);
static void clientHandler(struct sockaddr_in *pCurrentClient, char *buffer);
static void sendToAll(int socket, char *buffer);

int server(char *pService)
{
  int socketDescriptor = udpServerInit(pService);
  recvSendMsgActivity(socketDescriptor);
  return EXIT_SUCCESS;
}

static int udpServerInit(char *pService)
{
  int status;
  int socketDescriptor;
  struct addrinfo preSettings = {};
  struct addrinfo *pServerInfo;

  preSettings.ai_family = AF_UNSPEC;
  preSettings.ai_socktype = SOCK_DGRAM;
  preSettings.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL, pService, &preSettings, &pServerInfo);
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

  status = bind(socketDescriptor, pServerInfo->ai_addr, pServerInfo->ai_addrlen);
  if(status < 0)
  {
    perror("\nError : Bind Failed\n");
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(pServerInfo);

  printf("\nStart server\n");
  printf("Use Ctrl+C for shutdown the server\n");

  return socketDescriptor;
}

static void recvSendMsgActivity(int socketDescriptor)
{
  char buffer[MSG_SIZE];
  struct sockaddr_in clientAddress = {};
  socklen_t socketLengh = sizeof(struct sockaddr_in);

  while(1)
  {
    int n = recvfrom(socketDescriptor,
                     buffer,
                     sizeof(buffer),
                     0,
                     (struct sockaddr*)&(clientAddress),
                     &socketLengh);
    buffer[n] = '\0';

    clientHandler(&clientAddress, buffer);
    sendToAll(socketDescriptor, buffer);
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

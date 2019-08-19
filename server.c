#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define MSG_SIZE 1000

typedef struct
{
  int socketDescriptor;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
}
tUdp;

void udpServerInit(tUdp *pUdp, unsigned short port);
void recvSendMsgActivity(tUdp *pUdp);

int server(unsigned short port)
{
  tUdp udp = {};

  udpServerInit(&udp, port);
  recvSendMsgActivity(&udp);
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
  struct sockaddr_in clientDatabase[255];
  unsigned char lastClientIndex = 0;
  unsigned char clientExists = 0;
  socklen_t len;

  len = sizeof(pUdp->clientAddress);
  while(1)
  {
    clientExists = 0;
    int n = recvfrom(pUdp->socketDescriptor,
                     buffer,
                     sizeof(buffer),
                     0,
                     (struct sockaddr*)&(pUdp->clientAddress),
                     &len);

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

    // send received message to all known clients
    for (unsigned char clientNumber = 0; clientNumber < lastClientIndex; clientNumber++)
    {
      sendto(pUdp->socketDescriptor,
             &buffer,
             MSG_SIZE,
             0,
             (struct sockaddr*)&(clientDatabase[clientNumber]),
             sizeof(struct sockaddr_in));
    }
  }
}

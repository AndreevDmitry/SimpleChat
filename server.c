#include <stdio.h>
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

int server(unsigned short port)
{
  tUdp udp = {};
  char buffer[MSG_SIZE];
  struct sockaddr_in clientDatabase[255];
  socklen_t len;
  unsigned char lastClientIndex = 0;
  unsigned char clientExists = 0;

  // Create a UDP Socket
  udp.socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  udp.serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  udp.serverAddress.sin_port = htons(port);
  udp.serverAddress.sin_family = AF_INET;

  // bind server address to socket descriptor
  bind(udp.socketDescriptor,
       (struct sockaddr*)&(udp.serverAddress),
       sizeof((udp.serverAddress)));

  //receive the datagram
  len = sizeof(udp.clientAddress);
  while(1)
  {
    clientExists = 0;
    int n = recvfrom(udp.socketDescriptor,
                     buffer,
                     sizeof(buffer),
                     0,
                     (struct sockaddr*)&(udp.clientAddress),
                     &len); //receive message from server

    buffer[n] = '\0';
    puts(buffer);

    for(unsigned char clientNumber = 0; clientNumber < lastClientIndex; clientNumber++)
    {
       if (udp.clientAddress.sin_addr.s_addr == clientDatabase[clientNumber].sin_addr.s_addr &&
           udp.clientAddress.sin_port == clientDatabase[clientNumber].sin_port)
        {
          clientExists = 1;
          break;
        }
    }
    if (clientExists == 0)
    {
      clientDatabase[lastClientIndex] = udp.clientAddress;
      lastClientIndex++;
    }


    // send received message to all known clients
    for (unsigned char clientNumber = 0; clientNumber < lastClientIndex; clientNumber++)
    {
      sendto(udp.socketDescriptor,
             &buffer,
             MSG_SIZE,
             0,
             (struct sockaddr*)&(clientDatabase[clientNumber]),
             sizeof(struct sockaddr_in));
    }
  }
}

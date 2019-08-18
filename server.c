#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<netinet/in.h>

#define PORT     2115
#define MSG_SIZE 1000

typedef struct
{
  int socketDescriptor;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
}
tUdp;

int main()
{
  tUdp udp = {};
  char buffer[MSG_SIZE];
  socklen_t len;

  // Create a UDP Socket
  udp.socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  udp.serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  udp.serverAddress.sin_port = htons(PORT);
  udp.serverAddress.sin_family = AF_INET;

  // bind server address to socket descriptor
  bind(udp.socketDescriptor,
       (struct sockaddr*)&(udp.serverAddress),
       sizeof((udp.serverAddress)));

  //receive the datagram
  len = sizeof(udp.clientAddress);
  while(1)
  {
    int n = recvfrom(udp.socketDescriptor,
                     buffer,
                     sizeof(buffer),
                     0,
                     (struct sockaddr*)&(udp.clientAddress),
                     &len); //receive message from server
    buffer[n] = '\0';
    puts(buffer);

    // send the response
    sendto(udp.socketDescriptor,
           &buffer,
           MSG_SIZE,
           0,
           (struct sockaddr*)&(udp.clientAddress),
           sizeof((udp.clientAddress)));
  }
}

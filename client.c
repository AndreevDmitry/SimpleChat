#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT     2115
#define MSG_SIZE 1000

int main()
{
    int udpSocketDescriptor;
    char clientMsg[MSG_SIZE];
    struct sockaddr_in udpServerAddress = {};

    // Creating socket descriptor
    udpSocketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if ( udpSocketDescriptor < 0 )
    {
        perror("UDP socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Filling server information
    udpServerAddress.sin_family = AF_INET;
    udpServerAddress.sin_port = htons(PORT);
    udpServerAddress.sin_addr.s_addr = INADDR_ANY;

    do
    {
      size_t sendtoStatus;

      if (fgets(clientMsg, sizeof(clientMsg), stdin) == NULL)
      {
        perror("fgets failed");
        close(udpSocketDescriptor);
        exit(EXIT_FAILURE);
      }
      else if(strcmp(clientMsg, "/exit\n") == 0)
      {
        close(udpSocketDescriptor);
        exit(EXIT_SUCCESS);
      }

      sendtoStatus = sendto(udpSocketDescriptor, (const char *)clientMsg, strlen(clientMsg),
          MSG_CONFIRM, (const struct sockaddr *) &udpServerAddress,
              sizeof(udpServerAddress));
      if (sendtoStatus == -1)
      {
        perror("fgets failed");
        close(udpSocketDescriptor);
        exit(EXIT_FAILURE);
      }
    }
    while(1);

    close(udpSocketDescriptor);
    return 0;
}

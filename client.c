#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT     2115
#define MSG_SIZE 1000

void *clientThread(void); /* Thread function */

int main()
{
    pthread_t tid;
    pthread_attr_t attr;

    /* Get default attributes */
    pthread_attr_init(&attr);
    /* Create new thread*/
    pthread_create(&tid,&attr,(void *)clientThread, NULL);

    /* Wait for end of thread execution */
    pthread_join(tid,NULL);

    return 0;
}

void *clientThread()
{
  int udpSocketDescriptor;

  struct sockaddr_in udpServerAddress = {};
  char clientMsg[MSG_SIZE];

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
      break;
    }
    else if(strcmp(clientMsg, "/exit\n") == 0)
    {
      break;
    }

    sendtoStatus = sendto(udpSocketDescriptor,
                          (const char *)clientMsg,
                          strlen(clientMsg),
                          MSG_CONFIRM,
                          (const struct sockaddr *) &udpServerAddress,
                          sizeof(udpServerAddress));
    if (sendtoStatus == -1)
    {
      perror("sendto failed");
      break;
    }
  }
  while(1);

  close(udpSocketDescriptor);
  pthread_exit(0);
}

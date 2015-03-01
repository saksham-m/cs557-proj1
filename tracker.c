#include "common.h"

int main(int argc, char *argv[])
{

  int p;
  int socketfd,udpfd;
  struct sockaddr_in serverIp, myaddr;
  
  p = atoi(argv[1]);
  
  printf("Child(%d) received value: %d\n", getpid(), p);

  socketfd = socket (AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    printf("Failed to create socket");
    exit(0);
  }

  serverIp.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &(serverIp.sin_addr));
  serverIp.sin_port = htons(p); 

  if( connect(socketfd, (struct sockaddr *)&serverIp, sizeof(serverIp) ) < 0 ) {
    printf("\n Failed to connect");
    exit(0);
  }

      udpfd = socket (AF_INET, SOCK_STREAM, 0);
  if (udpfd < 0) {
    printf("Failed to create socket");
    exit(0);
  }
  memset(&serverIp, '0', sizeof(serverIp));
  
  serverIp.sin_family = AF_INET;
  serverIp.sin_addr.s_addr = htonl(INADDR_ANY);
  serverIp.sin_port = 0; 
  
  if( bind(udpfd, (struct sockaddr*)&serverIp, sizeof(serverIp)) < 0 ) {
    printf("Failed to bind socket");
    exit(0);
  }
  
  struct sockaddr_in adr_inet;
  int len_inet, val;
  char port[8] = {0};
  getsockname(udpfd, (struct sockaddr *)&adr_inet, &len_inet);
  //  printf("\nudp port = %d", ntohs(adr_inet.sin_port));  
  char message[10]= {0};
  sprintf(message, "%d", adr_inet.sin_port);

  send(socketfd, (void *) &message, 10, 0);
  close(socketfd);

  
  //while(1);
  
}

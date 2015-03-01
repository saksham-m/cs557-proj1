#include "common.h"
#include "client.h"

void recv_config(int p)
{

  int socketfd,udpfd;
  struct sockaddr_in serverIp, myaddr;
  
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

  char msg[100]= {0};
  recv(socketfd, msg, sizeof(struct config_msg_pkt_t), 0);

  struct config_msg_pkt_t *ptr = (struct config_msg_pkt_t*)msg;

  node_config.node_config.node_id = ntohl(ptr->node_config.node_id);
  node_config.node_config.delay = ntohl(ptr->node_config.delay);
  node_config.node_config.drop_probability = ntohl(ptr->node_config.drop_probability);
  node_config.tracker_port = ntohl(ptr->tracker_port);

   printf("\n%d - %d - %d - %d", node_config.node_config.node_id,  node_config.node_config.delay,  node_config.node_config.drop_probability,  node_config.tracker_port);
  
}


int main(int argc, char *argv[])
{

  int p;

  p = atoi(argv[1]);

  recv_config(p);
  
}

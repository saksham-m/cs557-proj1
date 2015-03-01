#include "common.h"
#include "manager.h"

void parser ()
{
  char *s, buff[256];
  FILE *fp = fopen (CONFIG, "r");
  int read_count =0;
  int id=0,delay=0,drop_prob=0;
  
  memset(&basic_config, -1, sizeof(basic_config));
  
  if (fp == NULL)
    {
      return;
    }

  /* Read next line */
  while ((s = fgets (buff, sizeof buff, fp)) != NULL)
    {

      /* Skip blank lines and comments */
      if (buff[0] == '\n' || buff[0] == '#')
	continue;

      switch(read_count){
      case 0:
	// Read number of Nodes.
	sscanf(buff, "%d", &basic_config.number_of_nodes);
	read_count++;
	break;
      case 1:
	//Timeout
	sscanf(buff, "%d", &basic_config.timeout);
	read_count++;
	break;
      case 2:
	// Scan node data
	read_count++;
	int i =0;
	while(id!= -1) {
	  //	  printf("stuck");	  
	  sscanf(buff, "%d %d %d", &id, &delay, &drop_prob);
	  if(id == -1) continue;

	  basic_config.node_config[i].node_id = id;
	  basic_config.node_config[i].delay = delay;
	  basic_config.node_config[i].drop_probability = drop_prob;
	  i++;
	  fgets (buff, sizeof buff, fp);
	}
	break;
      case 3:
	//read file information
	break;
      case 4:
	//read download tasks
	break;
      }
    }
  /* Close file */
  fclose (fp);
}

void spawn_tracker()
{

  struct sockaddr_in serverIp;
  //  int fd[2];
  int socketfd, newfd;

  //create tcp socket
  socketfd = socket (AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    printf("Failed to create socket");
    exit(0);
  }
  memset(&serverIp, '0', sizeof(serverIp));
  
  serverIp.sin_family = AF_INET;
  serverIp.sin_addr.s_addr = htonl(INADDR_ANY);
  serverIp.sin_port = 0; 
  
  if( bind(socketfd, (struct sockaddr*)&serverIp, sizeof(serverIp)) < 0 ) {
    printf("Failed to bind socket");
    exit(0);
  }
  
  
  struct sockaddr_in adr_inet;
  int len_inet, val;
  char port[8] = {0};
  getsockname(socketfd, (struct sockaddr *)&adr_inet, &len_inet);

  //Send over socket data
  val = ntohs(adr_inet.sin_port);
  sprintf(port, "%d", val);
  char *argv[] = {"./tracker", port, 0};

  pid_t parent = getpid();
  pid_t pid = fork();
  
  if (pid == -1){
    printf("Failed to spawn tracker. Exiting");
    exit(0);
  } 
  else if(pid == 0) {
    //execve(...);
    execv("./tracker",argv);
    printf("\nTracker");
  }
  
  listen (socketfd, 1);

  newfd = accept (socketfd, (struct sockaddr*)NULL, NULL);
  printf("\nGot incoming connection\n");
  char msg[10] = {0};
  recv (newfd, msg, sizeof(msg), 0);
  // printf("\nstring = %s\n", msg);
  int p = ntohs(atoi(msg));

  basic_config.tracker_port = p;

  close(newfd);
  close(socketfd);
  
}

void spawn_client(int node_id)
{


  struct sockaddr_in serverIp;
  //  int fd[2];
  int socketfd, newfd;

  //create tcp socket
  socketfd = socket (AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    printf("Failed to create socket");
    exit(0);
  }
  memset(&serverIp, '0', sizeof(serverIp));
  
  serverIp.sin_family = AF_INET;
  serverIp.sin_addr.s_addr = htonl(INADDR_ANY);
  serverIp.sin_port = 0; 
  
  if( bind(socketfd, (struct sockaddr*)&serverIp, sizeof(serverIp)) < 0 ) {
    printf("Failed to bind socket");
    exit(0);
  }
  
  
  struct sockaddr_in adr_inet;
  int len_inet, val;
  char port[8] = {0};
  getsockname(socketfd, (struct sockaddr *)&adr_inet, &len_inet);

  //Send over socket data
  val = ntohs(adr_inet.sin_port);
  sprintf(port, "%d", val);
  char *argv[] = {"./tracker", port, 0};

  pid_t parent = getpid();
  pid_t pid = fork();
  
  if (pid == -1){
    printf("Failed to spawn tracker. Exiting");
    exit(0);
  } 
  else if(pid == 0) {
    //execve(...);
    execv("./client",argv);
    printf("\nClient");
  }
  
  listen (socketfd, 1);

  newfd = accept (socketfd, (struct sockaddr*)NULL, NULL);
  printf("\nGot incoming connection\n");

  char msg[100] = {0};
  struct config_msg_pkt_t *ptr = (struct config_msg_pkt_t*)msg;

  ptr->node_config.node_id = htonl(basic_config.node_config[node_id].node_id);
  ptr->node_config.delay = htonl(basic_config.node_config[node_id].delay);
  ptr->node_config.drop_probability = htonl(basic_config.node_config[node_id].drop_probability);
  ptr->tracker_port = htonl(basic_config.tracker_port);

  
  // printf("\nsending =%d - %d - %d - %d", ntohl(ptr->node_config.node_id),  ntohl(ptr->node_config.delay),  ntohl(ptr->node_config.drop_probability), ntohl( ptr->tracker_port));
  
  
  send(newfd, (void*)msg, 100, 0);

  close(newfd);
  close(socketfd);
}

int main()
{

  int i;
  parser();

  /*
  printf("\n %d,%d", basic_config.number_of_nodes, basic_config.timeout);

  for(i=0;i<basic_config.number_of_nodes;i++)
    printf("\n %d-%d-%d", basic_config.node_config[i].node_id,  basic_config.node_config[i].delay,  basic_config.node_config[i].drop_probability);

  */
  spawn_tracker();


  for(i=0;i<basic_config.number_of_nodes;i++){
    if(basic_config.node_config[i].node_id == -1)
      break;

    spawn_client(i);
    
    }
  
  return 0;
}

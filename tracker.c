

#include "common.h"
#include "tracker.h"
#include <errno.h>

time_t start,end; 

int respond_query(struct sockaddr_in adr_inet, char *msg, char *msg2)
{
  //  int clientfd = *((int *)p);
  //1. Recieve request packet 
  //2. Build map data structure
  //3. Respond with group info
  //  char msg[1500] = {0};
  //  char msg2[1500] = {0};
  int len=0,i, len_inet,j;
  int port;
  int node,num_file,type,msg_type;
  char filename[32];
  char outbuf[150];
  //  struct sockaddr_in adr_inet;
  
  //  getsockname(clientfd, (struct sockaddr *)&adr_inet, &len_inet);


  //  printf("\nincoming\n");
  
  struct group_interest_preamble_t* ptr = (struct group_interest_preamble_t*)msg;

  node = ntohs(ptr->node);
  msg_type = ntohs(ptr->msg);
  num_file = ntohs(ptr->num_file);

  if(msg_type != GROUP_SHOW_INTEREST)
    return 0;

  struct group_interest_pkt_t* ptr1 = (struct group_interest_pkt_t*)(msg + sizeof(struct group_interest_preamble_t));

  memcpy(filename, ptr1->filename, 32);

  for (i=0;i<NUM_FILES;i++){
    if(strncmp(filename,tracker_data[i].filename,32) == 0){
      // match found. update table
      for(j=0;j<tracker_data[i].num_neighbours;j++){
	if(tracker_data[i].node_data[j].node_id == node){
	  //Data exists. Donot add it again.
	  goto exit_loop;
	}
      }
      int n= ++tracker_data[i].num_neighbours;

      tracker_data[i].node_data[n].node_id = node;
      tracker_data[i].node_data[n].node_ip = ntohl(adr_inet.sin_addr.s_addr);
      tracker_data[i].node_data[n].node_port = ntohs(adr_inet.sin_port);
      tracker_data[i].node_data[n].type = ptr1->type;
       printf("\n1Getting port = %d,%d\n\n",  tracker_data[i].node_data[n].node_port,node);
      break;
    }

    if(tracker_data[i].num_neighbours == -1){
      //File doesnt exist. Update table

      int n = ++tracker_data[i].num_neighbours;

      strncpy(tracker_data[i].filename, filename, 32);
      tracker_data[i].node_data[n].node_id = node;
      tracker_data[i].node_data[n].node_ip = ntohl(adr_inet.sin_addr.s_addr);
      tracker_data[i].node_data[n].node_port = ntohs(adr_inet.sin_port);
      printf("\nGetting port = %d,%d\n",  tracker_data[i].node_data[n].node_port,node);

      break;
    }
    
  }
 exit_loop:

  // return in case a seed. No need to reply

  time (&end); 
  sprintf(outbuf, "%d - %f- %s", node,  difftime (end,start) ,filename);
  
  fprintf(tracker_out,"%s\n", outbuf);

  fflush(tracker_out);

  if(ptr1->type == SEED){
    //    printf("\nseed. not sending\n");
    return 0;
  }

  //use i to build return packet and send

  
  struct group_assign_pkt_t* rsp = (struct group_assign_pkt_t *) msg2;

  rsp->msg = htons(GROUP_ASSIGN);
  rsp->num_file = 1;
  strncpy(rsp->filename, tracker_data[i].filename, 32);

  int n = tracker_data[i].num_neighbours + 1;
  int num =n;

  struct node_info_pkt_t *node_ptr;
  for(j=0;j<n;j++){
    if(tracker_data[i].node_data[j].type == LEECH || tracker_data[i].node_data[j].node_id == node){
      num--;
      continue;
    }
    
    node_ptr = (struct node_info_pkt_t *)(msg2 + sizeof(struct group_assign_pkt_t) + i*sizeof(struct node_info_pkt_t));

    node_ptr->node_id = tracker_data[i].node_data[j].node_id;
    node_ptr->node_ip = htonl(tracker_data[i].node_data[j].node_ip);
    node_ptr->node_port = htons(tracker_data[i].node_data[j].node_port);
    
  }

  rsp->num_neighbours = htons(num);
  
  for(i=0;i<1500;i++){
    if(msg2[i] == 0 && msg2[i+1]==0 && msg2[i+2] == 0 && msg2[i+3] == 0)
      break;
    fprintf(tracker_out, "%02x ", msg2[i]);
  }
  fprintf(tracker_out,"\n");
  fflush(tracker_out);
  return 0;
}


int main(int argc, char *argv[])
{

  int p, errno;
  int socketfd,udpfd,clientfd;
  struct sockaddr_in serverIp, myaddr, udpIp;
  char outbuf[50];
  time (&start);
  memset(&tracker_data,0, sizeof(tracker_data));

  tracker_out = fopen("tracker.out", "wb");

  fprintf(tracker_out, "type Tracker\n");
  
  sprintf(outbuf,"pid %d", getpid());
  fprintf(tracker_out, "%s\n", outbuf);

  p = atoi(argv[1]);

  socketfd = socket (AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    printf("Failed to create socket");
    exit(0);
  }
  
  serverIp.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &(serverIp.sin_addr));
  serverIp.sin_port = htons(p); 

  
  if( connect(socketfd, (struct sockaddr *)&serverIp, sizeof(serverIp) ) < 0 ) {
    printf("\n Failed to connect= %d", errno);
    exit(0);
  }

  udpfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (udpfd < 0) {
    printf("Failed to create socket");
    exit(0);
  }
  memset(&udpIp, '0', sizeof(serverIp));
  
  udpIp.sin_family = AF_INET;
  //  serverIp.sin_addr.s_addr = htonl(INADDR_ANY);
  inet_pton(AF_INET, "127.0.0.1", &(udpIp.sin_addr));
  udpIp.sin_port = 0; 
  
  if( bind(udpfd, (struct sockaddr*)&udpIp, sizeof(udpIp)) < 0 ) {
    printf("Failed to bind socket");
    exit(0);
  }
  
  struct sockaddr_in adr_inet;
  int len_inet, val;
  char port[8] = {0};
  getsockname(udpfd, (struct sockaddr *)&adr_inet, &len_inet);
  
  printf("\nudp port = %d\n", ntohs(adr_inet.sin_port));  
  char message[10]= {0};
  sprintf(message, "%d", adr_inet.sin_port);
  
  sprintf(outbuf,"tPort %d", ntohs(adr_inet.sin_port));
  fprintf(tracker_out, "%s\n", outbuf);

  fflush(tracker_out);
  
  send(socketfd, (void *) &message, 10, 0);
  close(socketfd);
  
  memset(&tracker_data, -1, sizeof(struct tracker_data_t));
  
  //  listen (udpfd, 20);
  //clientfd = accept (udpfd, (struct sockaddr*)NULL, NULL);

  char msg[1500] = {0};
  char rsp[1500] = {0};

  
  int len,rv;
  struct sockaddr_in remaddr;
  socklen_t addrlen = sizeof(remaddr);

  struct timeval tv;

  fd_set readfds;
 
  
  while(1){
    memset(msg,0,1500);
    memset(rsp,0,1500);
    
    tv.tv_sec = 30;
    tv.tv_usec = 0;
    
    FD_ZERO(&readfds); 
    FD_SET(udpfd, &readfds);
    
    rv = select(udpfd+1, &readfds, NULL, NULL, &tv);

    if(rv == 0) {
      printf("\nQuiting tracker\n");
      fflush(stdout);
      exit (0);
    }
    else{
      //printf("\nRecv\n");
      fflush(stdout);
      len =recvfrom(udpfd, (void*)msg, 1500 ,0, (struct sockaddr *)&remaddr, &addrlen);
      
      //      printf("\t\t%d", ntohs(remaddr.sin_port));
      
      fflush(stdout);
      respond_query(remaddr, msg, rsp);
      
      len =sendto(udpfd, (void *)rsp, 1500, 0, (struct sockaddr *)&remaddr, sizeof(struct sockaddr_in));
      printf("\nassignlen = %d", len);
    
      /*
	pthread_t thread;
	printf("\nGot incoming connection\n");
	pthread_create(&thread, NULL, (void *)&respond_query, (void *) &clientfd);
      */
    }
  }
  
  printf("\nconnection\n");	
  close(udpfd);
  
  //while(1);
  
}

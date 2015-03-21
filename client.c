
#include "common.h"
#include "client.h"
#include "timers-c.h"
#include <sys/stat.h>
#include <errno.h>

int udpfd;

void start()
{
	struct timeval tmv;
	int status;

	/* Change while condition to reflect what is required for Project 1
	   ex: Routing table stabalization.  */
	while (1) {
		Timers_NextTimerTime(&tmv);
		if (tmv.tv_sec == 0 && tmv.tv_usec == 0) {
		  /* The timer at the head on the queue has expired  */
		        Timers_ExecuteNextTimer();
			continue;
		}
		if (tmv.tv_sec == MAXVALUE && tmv.tv_usec == 0){
		  /* There are no timers in the event queue */
		        break;
		}
		  
		/* The select call here will wait for tv seconds before expiring 
		 * You need to  modifiy it to listen to multiple sockets and add code for 
		 * packet processing. Refer to the select man pages or "Unix Network 
		 * Programming" by R. Stevens Pg 156.
		 */
		status = select(0, NULL, NULL, NULL, &tmv);
		
		if (status < 0){
		  /* This should not happen */
			fprintf(stderr, "Select returned %d\n", status);
		}else{
			if (status == 0){
				/* Timer expired, Hence process it  */
			        Timers_ExecuteNextTimer();
				/* Execute all timers that have expired.*/
				Timers_NextTimerTime(&tmv);
				while(tmv.tv_sec == 0 && tmv.tv_usec == 0){
				  /* Timer at the head of the queue has expired  */
				        Timers_ExecuteNextTimer();
					Timers_NextTimerTime(&tmv);
					
				}
			}
			if (status > 0){
				/* The socket has received data.
				   Perform packet processing. */
		    
			}
		}
	}
	return;
}


void recv_config(int p)
{

  int socketfd;
  struct sockaddr_in serverIp, myaddr;

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

  char msg[1500]= {0};
  recv(socketfd, msg, sizeof(struct config_msg_pkt_t), 0);

  struct config_msg_pkt_t *ptr = (struct config_msg_pkt_t*)msg;
  /*
  node_config.node_config.node_id = ntohl(ptr->node_config.node_id);
  node_config.node_config.delay = ntohl(ptr->node_config.delay);
  node_config.node_config.drop_probability = ntohl(ptr->node_config.drop_probability);
  */
  node_config.tracker_port = ntohl(ptr->tracker_port);
  node_config.timeout = ptr->timeout;

  memcpy(&node_config.node_config, &ptr->node_config, sizeof(struct node_config_t));
  //  printf("\n%d - %d - %d - %d - %s", node_config.node_config.node_id,  node_config.node_config.delay,  node_config.node_config.drop_probability,  node_config.tracker_port, node_config.node_config.initfiles[0].filename);

    
  struct sockaddr_in  transferIp;
  udpfd = socket (AF_INET, SOCK_DGRAM, 0);

  if (udpfd < 0) {
    printf("Failed to create socket");
    exit(0);
  }
  
  memset(&transferIp, '0', sizeof(serverIp));
  
  transferIp.sin_family = AF_INET;
  transferIp.sin_addr.s_addr = htonl(INADDR_ANY);
  transferIp.sin_port = 0; 


  if( bind(udpfd, (struct sockaddr*)&transferIp, sizeof(transferIp)) < 0 ) {
    printf("Failed to bind socket");
    exit(0);
  }
  char outbuf[50];
  struct sockaddr_in adr_inet;
  int len_inet, val;

  getsockname(udpfd, (struct sockaddr *)&adr_inet, &len_inet);
  
  sprintf(outbuf,"%d.out",node_config.node_config.node_id);
  client_out = fopen(outbuf,"wb");

  fprintf( client_out,"type Client\n");

  sprintf(outbuf,"myID %d", node_config.node_config.node_id);
  fprintf( client_out,"%s\n", outbuf);

  sprintf(outbuf,"pid %d", getpid());
  fprintf( client_out,"%s\n", outbuf);

  sprintf(outbuf,"tracker port %d", node_config.tracker_port);
  fprintf( client_out,"%s\n", outbuf);

  fflush(client_out);

}

void accept_file_req(int index, char *msg, char*msg2, int seed)
{
  
  struct client_pkt_t* ptr, *ptr2;
  struct stat st;
  char outbuf[150];

  FILE *file_read;
  int file_size = 0;
  int i;
  int errno,len=0;
  
  //  while(1){
  //memset(msg,0,1500);
  //memset(msg2,0,1500);

  //    recvfrom(udpfd, (void*)msg,1500 ,0, (struct sockaddr *)&remaddr, &addrlen);

    ptr = (struct client_pkt_t*)msg;
    //Send out file segments
    //    printf("123,%d\n",errno);
   
    if(ptr->msg_type == SEG_REQ){
      ptr2 = (struct client_pkt_t*)msg2;
      ptr2->node_id = node_config.node_config.node_id;
      ptr2->msg_type = SEG_UPDATE;
      ptr2->type = seed;

      sprintf(outbuf, "From %d    SEG_REQ - %s", ptr->node_id, node_config.node_config.files[index].filename);
      fprintf( client_out,"%s\n", outbuf);

      if(seed){
	//I know file size
	//	printf("here");
	char name[50] = {0};
	sprintf(name, "%d-%s", node_config.node_config.node_id, node_config.node_config.files[index].filename);

	 fflush(stdout);

	//	file_read = fopen(name, "rb");
	
	//if(file_read == NULL)
	// return 0;
	stat(name, &st);

	file_size = st.st_size;

	//printf("file details: %d,%s",file_size,name);
	
	ptr2->filesize = file_size;

	sprintf(outbuf, "To %d    SEG_UPDATE - %s", ptr->node_id, node_config.node_config.files[index].filename);

	 fprintf( client_out,"%s\n", outbuf);
      }
      /* else {
	
	for(i=0;i<625;i++){
	  if(file_data->fdat[i].have == 1){
	    	    sprintf(ptr2->msg);
		    sprintf(Buffer,"Hello World");
		    sprintf(Buffer + strlen(Buffer),"Good Morning");
	    * (ptr2->msg) = itoa(i);
	    ptr2++;
	    *(ptr2->msg) = ',';
	    ptr2++;
  
	  }
	}
      }*/
      // }
     
      //sendto(udpfd, (void *)msg2, 1500, 0, (struct sockaddr *)&remaddr, sizeof(struct sockaddr_in));
  
  }

    if(ptr->msg_type == FILE_REQ){
      ptr2 = (struct client_pkt_t*)msg2;
      ptr2->msg_type = FILE_RESP;
      ptr2->type = seed;
      ptr2->node_id = node_config.node_config.node_id;

      sprintf(outbuf, "From %d    FILE_REQ - %s - %d", ptr->node_id, node_config.node_config.files[index].filename, ptr->seg_num);
      
      fprintf( client_out,"%s\n", outbuf);
	  
      if(seed){

	char name[50] = {0};
	sprintf(name, "%d-%s", node_config.node_config.node_id, node_config.node_config.files[index].filename);

	fflush(stdout);

       	file_read = fopen(name, "rb");
	
	if(file_read == NULL)
	  return ;
	stat(name, &st);

	file_size = st.st_size;
	int seg_num = ptr->seg_num;
	//printf("file details: %d,%s",file_size,name);
	if(ceil(file_size/32) == seg_num){
	  //last segment
	  ptr2->len=file_size%32;
	  fseek(file_read, -file_size%32, SEEK_END);
	}
	else {
	  ptr2->len=32;
	  fseek(file_read, seg_num*32, SEEK_SET);
	}
	ptr2->seg_num = seg_num;	
	fread(ptr2->msg, sizeof(char), ptr2->len, file_read);

	sprintf(outbuf, "To %d    FILE_RESP - %s - %d", ptr->node_id, node_config.node_config.files[index].filename, ptr->seg_num);

	fprintf( client_out,"%s\n", outbuf);
	 
	usleep(node_config.node_config.delay*1000);
      }
    }
    fflush(client_out);
}

int get_group_data(int index)
{
  //send file stuff to tracker and recieve group data


  
  char msg[1500]= {0};
  char msg2[1500]= {0};
  int i=0,len=0;
  struct group_data_t group_data;
  struct sockaddr_in remaddr, serverIp;
  socklen_t addrlen = sizeof(remaddr);  
  struct group_interest_preamble_t *ptr = (struct group_interest_preamble_t *) msg;
  struct group_interest_pkt_t *ptr2;
  struct node_info_pkt_t *node_ptr;
  struct group_assign_pkt_t* rsp;
  int flag = 0;
  FILE *file_write;
  int cnt = 0,f=0,f2=0;
  
  char outbuf[50];
  
  struct sockaddr_in adr_inet;
  int len_inet, val;

  getsockname(udpfd, (struct sockaddr *)&adr_inet, &len_inet);

  sprintf(outbuf,"my port %d", ntohs(adr_inet.sin_port));
  fprintf( client_out,"%s\n", outbuf);

  fflush(client_out);
  
  //printf("\nlaunching\n");
  	  
  
  struct file_data_t file_data;
  memset(&file_data, 0, sizeof(file_data));


 top:

  memset(&serverIp, '0', sizeof(serverIp));
  
  serverIp.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &(serverIp.sin_addr));
  serverIp.sin_port = htons(node_config.tracker_port); 
  //  serverIp.sin_port = htons(5000);
  
  
  memset(msg,0,1500);
  memset(&group_data,-1,sizeof(group_data));
  
  ptr->msg = htons(GROUP_SHOW_INTEREST);
  ptr->node = htons(node_config.node_config.node_id);

  ptr->num_file = 1;

  ptr2 = (struct group_interest_pkt_t *)(msg+ sizeof(struct group_interest_preamble_t));

  strncpy(ptr2->filename, node_config.node_config.files[index].filename, 32);
  ptr2->type = node_config.node_config.files[index].share;
  if(ptr2->type){
    //SEED if already have file
    for(i=0;i<MAX_FILES;i++){
      if(strncmp( node_config.node_config.files[index].filename,  node_config.node_config.initfiles[i].filename,32) == 0){

	ptr2->type =3;
	memset(&file_data, 1, sizeof(file_data));
      }
    }
  }
  
  len = sendto(udpfd, (void *)msg, 1500, 0, (struct sockaddr *)&serverIp, sizeof(struct sockaddr_in));


  fflush(stdout);
  sprintf(outbuf, "To T    GROUP_SHOW_INTEREST - %s", node_config.node_config.files[index].filename);
  fprintf(client_out, "%s\n", outbuf);

  fflush(client_out);
  
  if( ptr2->type != 3 ){
    //NOT SEED 
    memset(msg,0,1500);
    len = recvfrom(udpfd, (void*)msg,1500 ,0, (struct sockaddr *)&remaddr, &addrlen);

       
    //build group table. File index + node_id + node_ip + node_port

    sprintf(outbuf, "FROM T    GROUP_ASSIGN -" );
    
    fprintf( client_out,"%s\n", outbuf);

    fflush(client_out);
    
    memset(&group_data, -1, sizeof(group_data));
    
    rsp = (struct group_assign_pkt_t *) msg;
    group_data.num_neighbours = ntohs(rsp->num_neighbours);

    /*    for(i=0;i<group_data.num_neighbours;i++){
      node_ptr = (struct node_info_pkt_t *)(msg + sizeof(struct group_assign_pkt_t) + i*sizeof(struct node_info_pkt_t));
      fprintf(" %d ", node_ptr->node_id);
    }
    */
    for(i=0;i<group_data.num_neighbours;i++){
      node_ptr = (struct node_info_pkt_t *)(msg + sizeof(struct group_assign_pkt_t) + i*sizeof(struct node_info_pkt_t));
      
      group_data.node_data[i].node_id = node_ptr->node_id;
      group_data.node_data[i].node_ip =  node_ptr->node_ip;
      group_data.node_data[i].node_port = node_ptr->node_port;
      
      //printf("\n final groups: %s %d %d %d\n\n\n", node_config.node_config.files[index].filename, group_data.node_data[i].node_id, group_data.node_data[i].node_ip, group_data.node_data[i].node_port);

       fflush(stdout);
       
      //Send Sync request
       memset(msg2,0,1500);
       
       struct client_pkt_t* pkt = (struct client_pkt_t*)msg2;
       pkt-> msg_type = SEG_REQ;
       pkt-> node_id = node_config.node_config.node_id;
       memset(&serverIp, '0', sizeof(serverIp));
       
       serverIp.sin_family = AF_INET;
       serverIp.sin_addr.s_addr = group_data.node_data[i].node_ip;
       serverIp.sin_port = group_data.node_data[i].node_port;

       //printf("sending= %d\n", pkt->msg_type);
       
       len =  sendto(udpfd, (void *)msg2, 1500, 0, (struct sockaddr *)&serverIp, sizeof(struct sockaddr_in));

       sprintf(outbuf, "To %d    SEG_REQ - %s", node_ptr->node_id, node_config.node_config.files[index].filename);
       fprintf( client_out,"%s\n", outbuf);
       
      //printf("snd=%d\n", len);
       fflush(stdout);
       memset(msg2,0,1500);

       len = recvfrom(udpfd, (void*)msg,1500 ,0, (struct sockaddr *)&remaddr, &addrlen);


       //printf("rcv=%d from %d\n", len, ntohs(remaddr.sin_port));
       fflush(stdout);
       pkt = (struct client_pkt_t*)msg;
       //printf("\nRecieveing : %d, %d, %d\n", pkt->msg_type, pkt->type, pkt->filesize);

       if(pkt->msg_type == SEG_UPDATE){
	 sprintf(outbuf, "From %d    SEG_UPDATE - %s", pkt->node_id, node_config.node_config.files[index].filename);

	 fprintf( client_out,"%s\n", outbuf);
	 file_data.size = pkt->filesize;

	 fflush(client_out);
	 int n = ceil(file_data.size/32);
	 file_data.seg = n;
	 for(i=0;i<=n;i++){
	   if(file_data.fdat[i].have == 0 && pkt->type == 1){  //Type seed has packets
	     file_data.fdat[i].id = pkt->node_id;
	   }
	 }
       }
    }
 
    for(int req_cnt=0;req_cnt<8;req_cnt++){
      cnt =0;
      f=0;
      //      printf("\nlooping\n");
      while(cnt<file_data.seg){
	if(file_data.fdat[cnt].have == 0 && file_data.fdat[cnt].id!=0){
	  f=1;

	  f2=0;
	  //send req
	  memset(msg2,0,1500);
	  
	  struct client_pkt_t* pkt = (struct client_pkt_t*)msg2;
	  pkt-> msg_type = FILE_REQ;
	  pkt-> node_id = node_config.node_config.node_id;
	  pkt->seg_num = cnt;
	  memset(&serverIp, '0', sizeof(serverIp));
	  
	  for(i=0;i<group_data.num_neighbours;i++){
	    if(group_data.node_data[i].node_id == file_data.fdat[cnt].id){
	      serverIp.sin_family = AF_INET;
	      serverIp.sin_addr.s_addr = group_data.node_data[i].node_ip;
	      serverIp.sin_port = group_data.node_data[i].node_port;
	      break;
	    }
	  }
	  
	  // printf("sending= %d\n", pkt->msg_type);
	  //printf("\n%d,%d,%d\n",ntohs(serverIp.sin_port)m, cnt, file_data.fdat[cnt].id);
	  len =  sendto(udpfd, (void *)msg2, 1500, 0, (struct sockaddr *)&serverIp, sizeof(struct sockaddr_in));

	  sprintf(outbuf, "To %d    FILE_REQ - %s - %d", group_data.node_data[i].node_id, node_config.node_config.files[index].filename, cnt);

	  fprintf( client_out,"%s\n", outbuf);
	  //printf("snd req=%d,err= %d\n", len,errno);
	  fflush(stdout);
	  memset(msg,0,1500);

	  if(!flag){	  
	    char name[50] = {0};
	    sprintf(name, "%d-%s", node_config.node_config.node_id, node_config.node_config.files[index].filename);
	    
	    fflush(stdout);
	    file_write = fopen(name, "wb");
	    
	    if(file_write == NULL)
	      return 0;
	    flag =1;
	  }
	  
	  len = recvfrom(udpfd, (void*)msg,1500 ,0, (struct sockaddr *)&remaddr, &addrlen);
	  //printf("rcv=%d from %d no %d\n", len, ntohs(remaddr.sin_port), cnt);
	  fflush(stdout);
	  pkt = (struct client_pkt_t*)msg;
	  //  printf("\n123Recieveing : %d, segnum = %d\n", pkt->msg_type,pkt->seg_num);
	  file_data.fdat[pkt->seg_num].have =1;

	  sprintf(outbuf, "From %d    FILE_RESP - %s - %d", pkt->node_id, node_config.node_config.files[index].filename, pkt->seg_num);

	  fprintf( client_out,"%s\n", outbuf);
	  
	  fseek(file_write, pkt->seg_num*32, SEEK_SET);
	  fwrite(pkt->msg, sizeof(char), pkt->len, file_write);
	  break;
	}
	cnt++;
      }
      if (!f){

	if(f2 == 4){
	  return -1;
	}
	f2++;
	sleep(node_config.timeout);

	goto top;
      }
    }
    for(i=0;i<file_data.seg;i++){
      if (file_data.fdat[i].have ==0){
	//	printf("\nNot complete. Top\n");

	goto top;
      }
    }
    //send 8 requests
    //then goto top
    //logic for detecting finish
  }
  else {
    // Seed. Wait for request
    struct timeval tv;
    fd_set readfds;
    int rv;
    
    while(1){
      memset(msg,0,1500);
      memset(msg2,0,1500);

      tv.tv_sec = 30;
      tv.tv_usec = 0;
      
      FD_ZERO(&readfds); 
      FD_SET(udpfd, &readfds);

      rv = select(udpfd+1, &readfds, NULL, NULL, &tv);

      if(rv == 0) {
	fflush(stdout);
	exit (0);
      }
      
      len = recvfrom(udpfd, (void*)msg,1500 ,0, (struct sockaddr *)&serverIp, &addrlen);
      //printf("seed rcv=%d from %d\n", len,ntohs(serverIp.sin_port));
      fflush(stdout);
      struct client_pkt_t* pkt = (struct client_pkt_t*)msg;
      if(pkt->msg_type == 0)
	continue;
      accept_file_req(index, msg, msg2, 1);
      pkt = (struct client_pkt_t *)msg2;
      //printf("\nSending : %d, %d, %d\n", pkt->msg_type, pkt->type, pkt->filesize);
      
      sendto(udpfd, (void *)msg2, 1500, 0, (struct sockaddr *)&serverIp, sizeof(struct sockaddr_in));
    }
  }

  fclose(file_write);
  sprintf(outbuf, "Completed %s", node_config.node_config.files[index].filename);
  fwrite(outbuf, sizeof(char), 150, client_out);
  fclose(client_out);
  //close(udpfd);
  return -1;
  
}

int main(int argc, char *argv[])
{

  int p,i;
  int udpfd;

  memset(&node_config , -1, sizeof(struct local_node_config_t));
  
  p = atoi(argv[1]);
  recv_config(p);
  int (*fp)();
  
  fp = get_group_data;
  //Add timers to my files

  for(i=0;i<MAX_FILES;i++){
  if(node_config.node_config.files[i].starttime == -1)
    break;
  //  printf("\n Setting timer at %d for index %d\n", node_config.node_config.files[i].starttime, i);
  Timers_AddTimer(node_config.node_config.files[i].starttime*1000, fp, (int*)i);
  } 

  start();

}

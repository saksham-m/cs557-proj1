#ifndef COMMON_H
#define COMMON_H

#define _BSD_SOURCE
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pcap.h>
#include <time.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#define MAX_FILES 10

struct file_info_t {
  char filename[50];
};

struct file_share_t {

  char filename[50];
  int starttime;
  int share;
   
};

struct node_config_t {

  int node_id;
  int delay;
  int drop_probability;
  int file_num;
  int init_num;
  struct file_info_t  initfiles[MAX_FILES];
  struct file_share_t files[MAX_FILES]; 
};

struct config_msg_pkt_t {

  int tracker_port;
  int timeout;
  struct node_config_t node_config;
    
};

#define GROUP_SHOW_INTEREST 1
#define GROUP_ASSIGN 2

#define LEECH 0
#define PEER 2
#define SEED 3

struct group_interest_preamble_t {
  short msg;
  short node;
  short num_file;
};

struct group_interest_pkt_t {
  char filename[32];
  short type;
};

struct group_assign_pkt_t {
  short msg;
  short num_file;
  char filename[32];
  short num_neighbours;
};

struct node_info_pkt_t {
  short node_id;
  int   node_ip;
  int   node_port;
};


#endif

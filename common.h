#ifndef COMMON_H
#define COMMON_H

#define _BSD_SOURCE
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pcap.h>
#include <time.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

struct node_config_t {

  int node_id;
  int delay;
  int drop_probability;
  
};

struct config_msg_pkt_t {

  int tracker_port;
  struct node_config_t node_config;
    
};


#endif

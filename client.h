#ifndef CLIENT_H
#define CLIENT_H

struct local_node_config_t{
  int tracker_port;
  struct node_config_t node_config;
};

struct local_node_config_t node_config;

#endif

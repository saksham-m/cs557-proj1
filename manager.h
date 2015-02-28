#ifndef MANAGER_H
#define MANAGER_H

#define CONFIG "manager.conf"

struct node_config_t {

  int node_id;
  int delay;
  int drop_probability;
  
};

struct basic_config_t {

  uint number_of_nodes;
  uint timeout;
  struct node_config_t node_config[25];

};

struct basic_config_t basic_config;

#endif

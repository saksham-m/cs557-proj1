#ifndef CLIENT_H
#define CLIENT_H

#define NUM_FILES 10



struct local_node_config_t{
  int tracker_port;
  int timeout;
  struct node_config_t node_config;
};

struct local_node_config_t node_config;

struct file_transfer_t {
  int have;
  int id;
};


//malloc this for each file
struct file_data_t{
  int size;
  int seg;
  struct file_transfer_t fdat[625];
};


struct group_data_t {
  short num_neighbours;
  struct node_info_pkt_t node_data[25];
};

#define SEG_REQ 1
#define SEG_UPDATE 2
#define FILE_REQ 3
#define FILE_RESP 4

struct client_pkt_t {
  short node_id;
  short msg_type;
  short type;
  short filesize;
  short len;
  short seg_num;
  char msg[1300];
};

FILE *client_out;

#endif

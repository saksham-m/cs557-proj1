#ifndef TRACKER_H
#define TRACKER_H

#define NUM_FILES 10

struct node_data_pkt_t {
  short node_id;
  int   node_ip;
  int   node_port;
  short type;
};


struct tracker_data_t {

  char filename[32];
  short num_neighbours;
  struct node_data_pkt_t node_data[25];

};

struct tracker_data_t tracker_data[NUM_FILES];

FILE *tracker_out;
#endif

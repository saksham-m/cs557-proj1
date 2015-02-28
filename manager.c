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


int main()
{

  int i;
  parser();

  printf("\n %d,%d", basic_config.number_of_nodes, basic_config.timeout);

  for(i=0;i<basic_config.number_of_nodes;i++)
    printf("\n %d-%d-%d", basic_config.node_config[i].node_id,  basic_config.node_config[i].delay,  basic_config.node_config[i].drop_probability);
  
  return 0;
}

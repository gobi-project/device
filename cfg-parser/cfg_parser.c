#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void printCfgInfo( const char* cfg_file )
{
  config_t cfg;
  config_init(&cfg);

  if(0 == access(cfg_file, F_OK)) 
  {
    config_read_file(&cfg, cfg_file);
  } 
  else 
  {
    fprintf(stderr, "Unable to read config file.\n");
    exit(EXIT_FAILURE);
  }
  
  const char *c_filename;
  const char *cfg_filename;  
  const char *device_name;

  config_lookup_string(&cfg, "input", &c_filename);
  config_lookup_string(&cfg, "output", &cfg_filename);
  config_lookup_string(&cfg, "name", &device_name);
  
  char* tmp;

  //c file
  tmp = strstr(c_filename, "_econotag");
  strncpy(tmp,"\0",1);

  char c_string[20];
  strcpy(c_string, c_filename);
  strcat(c_string, ".c");

  //cfg file
  tmp = strstr(cfg_filename, "_econotag");
  strncpy(tmp,"\0",1);

  char cfg_string[20];
  strcpy(cfg_string, cfg_filename);
  strcat(cfg_string, ".cfg");

  fprintf(stdout, "%-8s\t| %-15s\t| %-30s\n", cfg_string, c_string, device_name);
}

int main(int nArgs, char **argv)
{
  if(nArgs < 2) 
  {

    fprintf(stderr, "No directory specified!\n");
    exit(EXIT_FAILURE);	 
  }

  DIR *d;
  struct dirent *dir;
  d = opendir(argv[1]);
  if(d)
  {
    fprintf(stdout, "%-8s\t| %-15s\t| %-30s\n", "CFG-FILE", "C-FILE", "DEVICE"); 
    fprintf(stdout, "%-8s\t| %-15s\t| %-30s\n", "--------", "---------------", "------------------------------"); 
    while(NULL != (dir = readdir(d)))
    {
      if(dir->d_type == DT_REG)
      {
	if(NULL != strstr(dir->d_name,".cfg"))
	{
          printCfgInfo(dir->d_name);
        }
      }      
    }
    closedir(d);
  }

  exit(EXIT_SUCCESS);
}

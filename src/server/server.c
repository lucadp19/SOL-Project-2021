#include "server.h"

server_config_t server_config;

int main(int argc, char* argv[]){ 
    if(argc > 2){
        fprintf(stderr, "There must be at most one additional argument: the path to the config file.\n");
        fprintf(stderr, "If no argument is supplied, the default is \"./config/config.txt\".\n");
        return -1;
    }

    char* config_path = (argc == 1) ? "config/config.txt" : argv[1];
     
    if( get_server_config(config_path) == -1){
        perror("Error in reading config file");
        return -1;
    }

    return 0;
}
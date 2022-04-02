#include "main.h"

int main(int argc, char *argv[]){
    int opt;
    bool is_server = false;
    char address[MAX_NAME_LENGHT] = "localhost";
    int port = 5000;
    char save_dir[MAX_NAME_LENGHT] = "save_dir";
    char server_filename[MAX_NAME_LENGHT] = "file";
    bool is_server_filename_set = false;

    while ((opt = getopt(argc, argv, "sp:a:d:n:")) != -1){
        switch(opt){
            case 's': 
                    is_server = true;
                    break;
            case 'p':
                    port = atoi(optarg);
                    break;
            case 'a':
                    strncpy(address, optarg, strlen(optarg));
                    break;
            case 'd':
                    strncpy(save_dir, optarg, strlen(optarg));
                    break;
            case 'n':
                    strncpy(server_filename, optarg, strlen(optarg));
                    is_server_filename_set = true;
                    break;
            default:
                   fprintf(stderr, 
                    "Usage: %s -s [-p port] [-d save_dir] \nor\n%s [-a address] [-p port] [-n server_filename] filename\n",
                     argv[0], argv[0]);
                   exit(EXIT_FAILURE);
        }
    }
    if (is_server) {
        server(port, save_dir);
    } else {
        if (optind >= argc) {
            fprintf(stderr, "Expected filename argument after options\n");
            exit(EXIT_FAILURE);
        }
        client(address, port, argv[optind], 
            is_server_filename_set ? server_filename : argv[optind]);
    }
    return 0;
}
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

void server(const int port, const char *save_dir){
    printf("server: port=%d, save_dir=%s\n", port, save_dir);
}

void client(const char *address, const int port, const char *filename){
    printf("client: address=%s, port=%d, filename=%s\n", address, port, filename);
}

int main(int argc, char *argv[]) {
    int opt;
    bool is_server = false;

    char *address[128];
    char localhost[] = "127.0.0.1";
    *address = (char *) &localhost;

    int port = 5000;

    char *save_dir[128];
    char dir[] = "save_dir";
    *save_dir = (char *) &dir;

    while ((opt = getopt(argc, argv, "sp:a:d:n:")) != -1){
        switch(opt){
            case 's': 
                    is_server = true;
                    break;
            case 'p':
                    port = atoi(optarg);
                    break;
            case 'a':
                    strncpy(*address, optarg, sizeof(optarg));
                    break;
            case 'd':
                    strncpy(*save_dir, optarg, sizeof(optarg));
                    break;
            default:
                   fprintf(stderr, "Usage: %s -s [-p port] [-d save_dir] \nor\n%s [-a address] [-p port] filename\n", argv[0], argv[0]);
                   exit(EXIT_FAILURE);
        }
    }
    if (is_server) {
        server(port, *save_dir);
    } else {
        if (optind >= argc) {
            fprintf(stderr, "Expected filename argument after options\n");
            exit(EXIT_FAILURE);
        }
        client(*address, port, argv[optind]);
    }
    return 0;
}

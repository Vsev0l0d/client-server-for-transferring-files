#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>
#include <sys/stat.h>

#define MAX_NAME_LENGHT 256

struct sockaddr_in serv_addr;
char buffer[4096];

void server(const int port, const char *save_dir){
    printf("start server: port=%d, save_dir=%s\n", port, save_dir);

    if (access(save_dir, F_OK) < 0) {
        if (mkdir(save_dir, 0700) < 0) {
            perror("Could not create directory");
            exit(EXIT_FAILURE);
        }
    } else if (access(save_dir, W_OK) < 0) {
        perror("Could not write to directory");
        exit(EXIT_FAILURE);
    }

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }
    if (listen(listenfd, 10) < 0) {
        perror("Listen Failed");
        exit(EXIT_FAILURE);
    }

    while(1) {
        int connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        if (connfd < 0) {
            perror("Accept Failed");
            exit(EXIT_FAILURE);
        }

        char filename[MAX_NAME_LENGHT];
        read(connfd, filename, sizeof(filename));
        char path[MAX_NAME_LENGHT*2 + 1];
        snprintf(path, sizeof(path), "%s%s%s", save_dir, 
            save_dir[strlen(save_dir) - 1] == '/' ? "":"/", filename);

        FILE *fp = fopen(path, "wb");
        if (fp != NULL) lockf(fileno(fp), F_TLOCK, 0);
        snprintf(buffer, sizeof(buffer), "%d", errno);
        write(connfd, buffer, strlen(buffer) + 1);
        if (errno == 0) {
            int count;
            while((count = read(connfd, buffer, sizeof(buffer))) > 0) {
                fwrite(buffer, sizeof(char), count, fp);
                snprintf(buffer, sizeof(buffer), "%d", ferror(fp) == 0 ? errno : EIO);
                write(connfd, buffer, strlen(buffer) + 1);
            }
            lockf(fileno(fp), F_ULOCK, 0);
        }
        if (fp != NULL) fclose(fp);
        close(connfd);
    }

}

void client(const char *address, const int port, const char *filename, const char *server_filename){
    printf("start client: address=%s, port=%d, filename=%s, server_filename=%s\n",
     address, port, filename, server_filename);

    FILE *fp;
    if ((fp = fopen(filename, "rb")) == NULL) {
        perror("Cannot open file");
        exit(EXIT_FAILURE);
    }
    struct stat st;
    stat(filename, &st); 

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }
    
    if (inet_aton(address, &serv_addr.sin_addr) <= 0) {
        perror("inet_aton error occured");
        exit(EXIT_FAILURE);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connect Failed");
        exit(EXIT_FAILURE);
    }

    write(sockfd, server_filename, sizeof(server_filename));
    read(sockfd, buffer, sizeof(buffer));
    if (atoi(buffer) != 0) {
        errno = atoi(buffer);
        perror("Server error");
        exit(EXIT_FAILURE);
    }

    int percent = 0;
    char bar[40];
    for (int i = 0; i < sizeof(bar); i++) bar[i] = '.';
    printf("Sent %d%% [%s]", percent, bar);
    
    size_t bytes_sent_counter = 0;
    int count;
    while((count = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0) {
        write(sockfd, buffer, count);
        read(sockfd, buffer, sizeof(buffer));
        if (atoi(buffer) != 0) {
            errno = atoi(buffer);
            perror("\nServer error");
            exit(EXIT_FAILURE);
        }
        bytes_sent_counter += count;
        percent = bytes_sent_counter * 100 / st.st_size;
        for (int i = 0; i < sizeof(bar)*percent/100; i++) bar[i] = '=';
        printf("\rSent %d%% [%s]", percent, bar);
        fflush(stdout);
    }
    printf("\n");

    close(sockfd);
    fclose(fp);
}

int main(int argc, char *argv[]) {
    int opt;
    bool is_server = false;
    char address[MAX_NAME_LENGHT] = "127.0.0.1";
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
                    strncpy(address, optarg, sizeof(optarg));
                    break;
            case 'd':
                    strncpy(save_dir, optarg, sizeof(optarg));
                    break;
            case 'n':
                    strncpy(server_filename, optarg, sizeof(optarg));
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
            perror("Expected filename argument after options");
            exit(EXIT_FAILURE);
        }
        client(address, port, argv[optind], 
            is_server_filename_set ? server_filename : argv[optind]);
    }
    return 0;
}
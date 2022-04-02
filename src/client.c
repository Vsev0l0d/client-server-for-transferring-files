#include "main.h"

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

    const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    struct hostent *h = gethostbyname(address);
    if (h == NULL) {
        herror("gethostbyname error occured");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_addr = *((struct in_addr *) h->h_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connect Failed");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];

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
#include "main.h"

typedef struct {
    int connfd;
    const char *save_dir;
} pthrData;

static void* server_thread(void* thread_data){
    pthrData *data = (pthrData*) thread_data;
    int connfd = data->connfd;

    char buffer[BUFFER_SIZE];

    char filename[MAX_NAME_LENGHT];
    read(connfd, filename, sizeof(filename));
    char path[MAX_NAME_LENGHT*2 + 1];
    snprintf(path, sizeof(path), "%s%s%s", data->save_dir, 
        data->save_dir[strlen(data->save_dir) - 1] == '/' ? "":"/", filename);

    FILE *fp = fopen(path, "wb");
    if (fp != NULL) flock(fileno(fp), LOCK_EX|LOCK_NB);
    snprintf(buffer, sizeof(buffer), "%d", errno);
    write(connfd, buffer, strlen(buffer) + 1);

    if (errno == 0) {
        int count;
        while((count = read(connfd, buffer, sizeof(buffer))) > 0) {
            fwrite(buffer, sizeof(char), count, fp);
            snprintf(buffer, sizeof(buffer), "%d", ferror(fp) == 0 ? errno : EIO);
            write(connfd, buffer, strlen(buffer) + 1);
        }
        flock(fileno(fp), LOCK_UN);
    }
    if (fp != NULL) fclose(fp);
    close(connfd);
    pthread_exit(0);
}

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

    const int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
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
            errno = 0;
            continue;
        }

        pthread_t thread;
        pthrData thread_data = {
            thread_data.connfd = connfd,
            thread_data.save_dir = save_dir
        };

        pthread_create(&thread, NULL, server_thread, &thread_data);
        pthread_detach(thread);
    }
}
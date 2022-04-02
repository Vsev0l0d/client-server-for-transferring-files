#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/file.h>

#define MAX_NAME_LENGHT 256
#define BUFFER_SIZE 4096

void client(const char *address, const int port, const char *filename, const char *server_filename);
void server(const int port, const char *save_dir);

#endif
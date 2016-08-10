#ifndef _SERVER_H
#define _SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <duet/duet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <dirent.h>
#include <uuid/uuid.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>

#define PORT		20000
#define DATASIZE        524242
#define UUIDSIZE        32
#define LOG_MAX_SIZE    10000
#define LOG_MAX_PATH    32
#define FETCH_ITEMS     512
#define MAX_LOGS        32
#define OFFSETSIZE	4
#define MAXMSG          2000000
#define METASIZE        44
#define MAXPATH         256
#define BUFFERSIZE      524288
#define DATESIZE        8

struct date {
        char year[5];
        char month[3];
        char day[3];
        char date[9];
};

void error(const char *msg);
int make_socket(uint16_t port);
void get_date(struct date *);
void check_dir(const char *path);
int open_file(char *path);
int open_new_file(char *path);
int read_from_client(int sockfd);
#endif /* _SERVER_H */

#ifndef _DUETTEL_H
#define _DUETTEL_H
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

#define DATASIZE        524242
#define UUIDSIZE        32
#define LOG_MAX_SIZE    10000
#define LOG_MAX_PATH    32
#define FETCH_ITEMS     512
#define MAX_LOGS        32

#define MAXMSG          2000000
#define METASIZE        44
#define MAXPATH         256
#define BUFFERSIZE      524288
#define DATESIZE        8


struct node {
	char path[LOG_MAX_PATH];
	int safe;
	struct node *next;
};

struct queue {
	int size;
	struct node *front;
	struct node *back;
};

struct date {
        char year[5];
        char month[3];
        char day[3];
        char date[9];
};


void error(const char *msg);
struct queue *init_queue();
void enqueue(struct queue *queue, struct node *node);
void check_dir(const char *path);
struct node *dequeue(struct queue *queue);
void get_date(struct date *);
struct node *find_safe(struct node **array, int size);
int is_empty(struct queue *queue);
void *sendLog(void *tmp);
void handle_sigint(int signal);
void usage(int err);
FILE *find_uuid();
FILE *create_uuid();

#endif /* _DUETTEL_H */

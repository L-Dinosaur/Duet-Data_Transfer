#ifndef _DUET_COLLECT_H
#define _DUET_COLLECT_H
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

#define LOG_MAX_SIZE	10000
#define LOG_MAX_PATH	32
#define FETCH_ITEMS     512
#define MAX_LOGS	32
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

void error(const char *msg);
struct queue *init_queue();
void enqueue(struct queue *queue, struct node *node);
struct node *dequeue(struct queue *queue);
struct node *find_safe(struct node **array, int size);
int is_empty(struct queue *queue);
void *sendLog(void *tmp);
void handle_sigint(int signal);
void usage(int err);

#endif /* _DUET_COLLECT_H */

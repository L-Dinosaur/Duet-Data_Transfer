/* A simple server in the internet domain using TCP
   The port number is passed as an argument
   This version runs forever, forking off a separate
   process for each connection
*/
#ifndef _SERVER_H
#define _SERVER_H
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

struct date {
        char year[5];
        char month[3];
        char day[3];
};

void dostuff(int); // function prototype
void check_dir(const char *);
void error(const char *);
void get_date(struct date *);

#endif

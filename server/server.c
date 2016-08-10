#include "server.h"
void error(const char *msg)
{
        perror(msg);
        exit(0);
}

int make_socket(uint16_t port)
{
  	int sock;
  	struct sockaddr_in name;

  	/* Create the socket. */
  	sock = socket(PF_INET, SOCK_STREAM, 0);
  	if(sock < 0)
    	{
      		perror("socket");
      		exit(EXIT_FAILURE);
    	}

  	/* Give the socket a name. */
  	name.sin_family = AF_INET;
  	name.sin_port = htons(port);
  	name.sin_addr.s_addr = htonl(INADDR_ANY);
  	if(bind (sock, (struct sockaddr *)&name, sizeof(name)) < 0)
    	{
      		perror("bind");
      		exit(EXIT_FAILURE);
    	}

  	return sock;
}

void check_dir(const char *path)
{
        DIR * dir;
        int n;
        dir = opendir(path);
        if(dir == NULL){
                n = mkdir(path, 0777) == -1;
                if(n<0)
                        error("Error: Failed to create directory");
        }
        else
                closedir(dir);
}


void get_date(struct date * date)
{
        time_t timeval;
        struct tm * l_time;
        char mon_char[2];
        time(&timeval);
        l_time = localtime(&timeval);
        int year = l_time->tm_year + 1900;
        int month = l_time->tm_mon + 1;
        int day = l_time->tm_mday;
        sprintf(date->year, "%d", year);
        if(month < 10) {
                sprintf(date->month, "0%d", month);
        }
        else
                sprintf(date->month, "%d", month);
        if(day < 10)
                sprintf(date->day, "0%d", day);
        else
                sprintf(date->day, "%d", day);
        sprintf(date->date, "%s%s%s", date->year, date->month, date->day);

}

int open_file(char *path)
{
	int i=0;
	char filepath[MAXPATH];
	sprintf(filepath, "%s%d", path, i);
	while(access(filepath, F_OK) != -1) {
		i++;
		sprintf(filepath, "%s%d", path, i);
	}
	i--;
	return i;
}

int open_new_file(char *path)
{
        int i=0;
        char filepath[MAXPATH];
        sprintf(filepath, "%s%d", path, i);
        while(access(filepath, F_OK) != -1) {
                i++;
                sprintf(filepath, "%s%d", path, i);
        }
        return i;
}


int read_from_client(int sockfd)
{
        char *data = (char *)malloc(DATASIZE * sizeof(char));
	char uptr[UUIDSIZE+1];
	char dptr[DATESIZE+1];
	uint32_t offset;
	uint32_t *optr = &offset;
	char *ptr = data + OFFSETSIZE;
	int n,i,readed = 0, infd;
	bzero(uptr, UUIDSIZE+1);
	bzero(dptr, DATESIZE+1);
       	char path[MAXPATH] = "/media/data/";
	char filepath[MAXPATH];

	/* Meta Data */
	//  uuid
	n = read(sockfd, uptr, UUIDSIZE);
	if(n<0)
		error("read uuid");
	else if(n==0)
		return -1;
	//printf("uuid is %s\n", uptr);
	strcat(path, uptr);
	strcat(path, "/");
	check_dir(path);

	//  date
	n = read(sockfd, dptr, DATESIZE);
	if(n<0)
		error("read date");
	strcat(path, dptr);
	strcat(path, "/");
	check_dir(path);

	//  offset
	n = read(sockfd, optr, OFFSETSIZE);
	if(n<0)
		error("read offset");

	if(offset==0){
		i = open_new_file(path);
		sprintf(filepath, "%s%d", path, i);
		printf("path is %s\n", filepath);
		infd = open(filepath, O_CREAT | O_WRONLY, 0666);
	}
	else{
		i = open_file(path); 
		sprintf(filepath, "%s%d", path, i);
		printf("path is %s\n", filepath);
		infd = open(filepath, O_APPEND | O_WRONLY, 0666);
	}


	while(readed < DATASIZE) {
		n = read(sockfd, data, DATASIZE);
		if(n<0)
			error("read data");
		else if(n==0) {
			printf("read %d bytes\n", readed);
			free(data);
			close(infd);
			return -1;
		}
		else {
			readed += n;
			i = write(infd, data, n);
			if(i<0)
				error("write");
		}	
	}
	printf("read %d bytes\n", readed);
	free(data);
	close(infd);
	return 0;
}



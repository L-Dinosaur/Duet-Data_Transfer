#include "server.h"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void get_date(struct date * date)
{
        time_t timeval;
        struct tm * l_time;
        char mon_char[2];
        time(&timeval);
        l_time = localtime(&timeval);
        int year = l_time->tm_year + 1900, month = l_time->tm_mon + 1, day = l_time->tm_mday;
        sprintf(date->year, "%d", year);
        if(month < 10) {
                sprintf(date->month, "0%d", month);
        }
        else
                sprintf(date->month, "%d", month);
        sprintf(date->day, "%d", day);

}

void check_dir(const char *path)
{
	DIR * dir;
	dir = opendir(path);
	if(!dir){
		if(mkdir(path, 0777) == -1)
			error("Error: Failed to create directory");
	}
	else
		closedir(dir);
}
	
void dostuff (int sock)
{
	FILE *output = fopen("raw.txt", "a+");
	int n;
	char buffer[256];
	bzero(buffer,256);
	n = read(sock,buffer,255);
	if(n<0)
		error("Error: Failed reading from socket");
	while(buffer != "999") {
		fprintf(output,"%s",buffer);
		bzero(buffer,256);
		n = read(sock,buffer,255);
		if(n<0) 
			error("Error: Failed reading from socket in while loop");
	} 
	fclose(output);
}

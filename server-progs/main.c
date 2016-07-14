#include "server.h"

int main(int argc, char *argv[])
{
	struct date date;
	get_date(&date);
	printf("%s%s%s\n", date.year, date.month, date.day);

        /* Declaration */
        int sockfd, newsockfd, portno, pid;
        socklen_t clilen;
        struct sockaddr_in serv_addr, cli_addr;
        char uuid_char[33];
        char path[256] = "/var/log/duet/";


        if (argc < 2) {
                fprintf(stderr,"ERROR, no port provided\n");
                exit(1);
        }



        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                error("ERROR opening socket");
        bzero((char *) &serv_addr, sizeof(serv_addr));
        portno = atoi(argv[1]);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) < 0)
                error("ERROR on binding");
        listen(sockfd,5);
        clilen = sizeof(cli_addr);



     /* receive uuid */
        newsockfd = accept(sockfd,
                        (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
                error("ERROR on accept");
        bzero(uuid_char, 33);
        if(read(newsockfd,uuid_char,33)<0)
                error("Error: Failed reading uuid from socket");

        check_dir(path);
        strcat(path, uuid_char);
        check_dir(path);
        while (1) {
                newsockfd = accept(sockfd,
                                        (struct sockaddr *) &cli_addr, &clilen);
                if (newsockfd < 0)
                        error("ERROR on accept");
                pid = fork();
                if (pid < 0)
                        error("ERROR on fork");
                if (pid == 0)  {
                        close(sockfd);
                        dostuff(newsockfd);
                        close(newsockfd);
                        exit(0);
                }
                else
                        close(newsockfd);
        } /* end of while */
        close(sockfd);
        return 0; /* we never get here */
}


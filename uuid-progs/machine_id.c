#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <uuid/uuid.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main()
{

	/* Declaration */
	// General & uuid
	FILE *output;
	char uuid_char[33];
	int i;
	
	// Client setup
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	/* Setting up the client */
	portno = 20000;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	//check////////////////////////////////////
	if(sockfd <0)
		error("Error: Failed opening socket");
	//////////////////////////////////////////




	server = gethostbyname("192.168.122.1");
	
	//check////////////////////////////////////
	if(server ==NULL)
		error("Error: Could not find host");
	//////////////////////////////////////////
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	if(connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
		error("Error: Could not connect");
	

	/* Access or create uuid */
	if(access("uuid.txt", F_OK) == -1) {
		uuid_t uuid;
		output = fopen("uuid.txt", "a+");
		uuid_generate(uuid);
			
		for(i=0;i<sizeof(uuid);i++) {
			fprintf(output, "%02x", uuid[i]);
		}
	}
	else
		output = fopen("uuid.txt", "a+");
	if(output == NULL)
		error("Error: Can't open uuid.txt\n");
	n = fscanf(output,"%s", uuid_char);
	if(!n)
		error("Error: Failed reading uuid.txt\n");
	fclose(output);	

	/* Sending the uuid */
	n = write(sockfd,uuid_char,strlen(uuid_char));
	
	//check////////////////////////////////////
	if(n == -1)
		error("Error: Failed writing to socket");
	close(sockfd);
	printf("Sent uuid: %s\n", uuid_char);
	return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <uuid/uuid.h>
#include <fcntl.h>
#include <unistd.h>

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main()
{
	/* Declaration */
	FILE *output;
	char uuid_char[33];
	int n,i;
	if(access("uuid.txt", F_OK) != -1) {
		output = fopen("uuid.txt", "r");
		if(output == NULL)
			error("Error: Can't open uuid.txt\n");
		n = fscanf(output,"%s", uuid_char);
		if(!n)
			error("Error: Failed reading uuid.txt\n");
		printf("uuid.txt exists, uuid is: %s\n", uuid_char);
	}
	else {
		uuid_t uuid;
		output = fopen("uuid.txt", "w");
		uuid_generate(uuid);
			
		for(i=0;i<sizeof(uuid);i++) {
			fprintf(output, "%02x", uuid[i]);
		}
	}
	fclose(output);	
	return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	if(argc < 1)
		error("Too few arguments\n");
	char *path = argv[1];
	DIR *dir = opendir(path);
	struct dirent *cdir;
	if(dir == NULL)
		error("Cannot find directory");
	while((cdir = readdir(dir)) != NULL) {
		printf("%s\n", cdir->d_name);
	}
	closedir(dir);
	return 0;
}

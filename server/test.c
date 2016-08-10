#include "server.h"

int main()
{
	char path[64] = "/media/data/210d4e763753483782c2b3f98f65b773/20160802/";
	char filepath[64];
	int fd, i = open_file(path);
	printf("i is %d\n", i);
	sprintf(filepath, "%s%d", path, i);
	char buffer[25] = "This is Dino testing.\n";
	printf("file path is %s\n", filepath);
	fd = open(filepath, O_APPEND | O_WRONLY, 0666);
	
	int n = write(fd, buffer, 22);
	if(n<=0)
		error("write");
	return 0;
	
}

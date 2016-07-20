 /*
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 */
/*This is to show*/
#include "duet_collect.h"

//int count;




void error(const char *msg)
{
        perror(msg);
        exit(0);
}

struct queue *init_queue()
{
        struct queue *queue = (struct queue*)malloc(sizeof(struct queue));
        queue->size = 0;
        queue->front = NULL;
        queue->back = NULL;
        return queue;
}


void enqueue(struct queue *queue, struct node *node)
{
        if(!node->safe) {
                fprintf(stderr, "Error: Failed to enqueue\n");
                return;
        }
        queue->size++;
	node->safe = 0;
        if(queue->front == NULL && queue->back == NULL) {
                queue->front = node;
                queue->back = node;
                node->next = NULL;
                return;
        }

        else {
                queue->back->next = node;
                queue->back = node;
                node->next = NULL;
                return;
	}
}


struct node *dequeue(struct queue *queue)
{

        struct node *temp = queue->front;
        if(queue->front == NULL) {
                fprintf(stderr, "Error: Queue is empty\n");
                return NULL;
        }
        queue->size--;
        if(queue->front == queue->back) {
                queue->front = NULL;
                queue->back = NULL;
                return temp;
        }
        else {
                queue->front = queue->front->next;
                temp->next = NULL;
                return temp;
        }
}


struct node *find_safe(struct node **array, int size)
{
	int i;
	struct node *temp;
	for(i=0;i<size;i++) {
		if(array[i]->safe) {
			return array[i];
		}
	}
	fprintf(stderr, "Error: Run out of thread safe nodes\n");
	return NULL;
}

int is_empty(struct queue *queue)
{
	if(!queue->size) 
		return 1;
	else
		return 0;
}

FILE *find_uuid()
{
	DIR *dir = opendir("/var/log/duet");
	uuid_t uuid;
	FILE *output;
	int i,n;
	char uuid_char[UUIDSIZE];
	if(dir) {
		output = fopen("/var/log/duet/uuid.txt", "r");	
		if(output == NULL)
			error("fopen1");
		return output;
	}
	else if (ENOENT == errno) {
		n = mkdir("/var/log/duet", 0777);
		if(n<0)
			error("mkdir");
		uuid_generate(uuid);
		output = fopen("/var/log/duet/uuid.txt", "a+");
		if(output == NULL)
			error("fopen2");
		for(i=0;i<sizeof(uuid);i++) {
			fprintf(output, "%02x", uuid[i]);
		}
		fclose(output);
		output = fopen("/var/log/duet/uuid.txt", "r");
		return output;
	}
	else
		error("opendir");
		
}	
void *sendLog(void *tmp)
{
	int i, outfd;
        /* Declaration */
	FILE *output;
        //For reading the log
        char * buffer = (char *)malloc(BUFFERSIZE * sizeof(char));
	struct queue * queue = (struct queue*)tmp;
        // For sending data through socket
        int sockfd, portno, n;
        struct sockaddr_in serv_addr;
        struct hostent *server;
	// For queue //
	struct node *temp;
        /* Initialization */
	char uuid_char[UUIDSIZE];
        // Initializing the server address instance
        portno = 20000;
        server = gethostbyname("192.168.122.65");
        if(server == NULL)
                error("Error: Did not find server");
        bzero((char *)&serv_addr, sizeof(serv_addr));
	
        // Filling in serv_addr
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(portno);
	
	/* Checking for queue */
	while(1){
		if(is_empty(queue)) {
			sleep(1);
			continue;
		}
		else {
			temp = dequeue(queue);
			outfd = open(temp->path, O_CREAT | O_RDWR);
			if(output < 0) 
				error("open");
			/* setting up the socket */
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd < 0)
				error("socket");
			if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
				error("connect");



			/* sending the uuid */
			n = write(sockfd, uuid_char, UUIDSIZE);
			if(n < 0)
				error("write");


			/* sending data */
			bzero(buffer,BUFFERSIZE);
			close(sockfd);
			while(read(outfd, buffer, BUFFERSIZE) > 0) {
				sockfd = socket(AF_INET, SOCK_STREAM, 0);
				if(sockfd < 0)
					error("socket");
				if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
					error("connect");
				n = write(sockfd, buffer, BUFFERSIZE);
				if(n < 0)
					error("write");
				bzero(buffer,BUFFERSIZE);
				close(sockfd);
			}
			/* Clean up after the used file */
			//freopen(temp->path, "w", output);
			//fclose(output);			
			temp->safe = 1;

		}
	}       
    	pthread_exit(NULL);
	//return;
}





//static volatile int got_sigint = 0;
/*
void handle_sigint(int signal)
{
	if (signal != SIGINT)
		return;

	got_sigint = 1;
}
*/
void usage(int err)
{
	fprintf(stderr,
		"\n"
		"dummy is a program meant to demonstrate how to use the Duet\n"
		"framework. For development purposes, it can also be used during\n"
		"testing.\n"
		"\n"
		"Usage: dummy [OPTION]...\n"
		"\n"
		"Program Options\n"
		" -f <freq>     event fetching frequency in msec (def: 10ms)\n"
		" -d <dur>      program execution time in sec\n"
		" -o            use Duet (if not set, Duet Options are ignored)\n"
		" -h            print this usage information\n"
		"\n"
		"Duet Options\n"
		" -e            register for event-based Duet (def: state-based)\n"
		" -p <path>     directory to register with Duet (def: '/')\n"
		" -g            get file path for every event received\n"
		"\n");

	exit(err);
}



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
#include "duettel.h"

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

FILE *create_uuid()
{
	FILE *output;
	uuid_t uuid;
	int i;
	uuid_generate(uuid);
        output = fopen("/var/log/duet/uuid.txt", "a+");
        if(output == NULL)
	        error("fopen");
        for(i=0;i<sizeof(uuid);i++) {
                fprintf(output, "%02x", uuid[i]);
        }
        fclose(output);
        output = fopen("/var/log/duet/uuid.txt", "r");
        return output;

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
		if(output == NULL){
			if(ENOENT == errno) {
				output = create_uuid();
			}
			else	
				error("fopen");
		}
		else
			return output;
	}
	else if (ENOENT == errno) {
		n = mkdir("/var/log/duet", 0777);
		if(n<0)
			error("mkdir");
		output = create_uuid();
		return output;
	}
	else
		error("opendir");
		
}	
void *sendLog(void *tmp)
{
        /* Declaration */
	int i, outfd;
	FILE *output;
	long readed = 0, msec = 0;
	struct timeval * tv1 = (struct timeval *)malloc(sizeof(struct timeval));
	struct timeval * tv2 = (struct timeval *)malloc(sizeof(struct timeval));
	struct timeval * res = (struct timeval *)malloc(sizeof(struct timeval));

        //For reading the log
        char *buffer = (char *)malloc(BUFFERSIZE * sizeof(char));
	bzero(buffer, BUFFERSIZE);
	struct queue * queue = (struct queue*)tmp;
	
        // For sending data through socket
        int sockfd, portno, n;
        struct sockaddr_in serv_addr;
        struct hostent *server;

	// For queue //
	struct node *temp;

        /* Meta data */
	char uuid_char[UUIDSIZE+1];
	struct date date;
	char *u_ptr = buffer, *date_ptr = u_ptr + UUIDSIZE, *ofs_ptr = date_ptr + DATESIZE, *data_ptr = ofs_ptr + sizeof(uint32_t);
	uint32_t *offset = (uint32_t *)ofs_ptr;
	
	
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
			gettimeofday(tv1, NULL);
			temp = dequeue(queue);
			outfd = open(temp->path, O_CREAT | O_RDWR);
			if(outfd < 0) 
				error("open");


			/* preparing the meta data */
			// uuid
			output = find_uuid();
			if(output == NULL)
				error("find_uuid");
			n = fscanf(output, "%s", u_ptr);
			
			if(n<=0)
				error("fscanf");

			// date
			get_date(&date);
			n = sprintf(date_ptr, "%s", date.date);
			if(n<=0)
				error("sprintf");
				
			//offset
			*offset = 0;
			readed = 0;
			while((i = read(outfd, data_ptr, DATASIZE)) > 0) {
				sockfd = socket(AF_INET, SOCK_STREAM, 0);
				if(sockfd < 0)
					error("socket");
				if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
					error("connect");
				n = write(sockfd, buffer, i+METASIZE);
				readed += n;
				if(n < 0)
					error("write");
				printf("sent %d\n", n);	
				*offset += n;
				close(sockfd);
			}
			gettimeofday(tv2, NULL);
			timersub(tv2,tv1,res);
			msec = res->tv_sec * 1000000 + res->tv_usec;
			printf("%lumb/s\n", readed/msec);
			close(outfd);
			temp->safe = 1;

		}
	}       
	free(tv1);
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
		"Duet telemetry is a program written based on the Duet dummy\n"
		"task. It uses the event collecting Duet framework to collect\n"
		"file usage/modification data for the study of cache patterns\n"
		"\n"
		"Usage: duettel [OPTION]...\n"
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



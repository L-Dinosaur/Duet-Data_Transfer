#include "duettel.h"
int count;
static volatile int got_sigint = 0;

void handle_sigint(int signal)
{
        if (signal != SIGINT)
                return;

        got_sigint = 1;
}

int main(int argc, char *argv[])
{

	/* Original Declaration */
	unsigned long long counter = 0;
					
	int freq = 10, duration = 0, o3 = 0, evtbased = 0, getpath = 0, n;
	int keep_running = 0;
	char path[DUET_MAX_PATH] = "/";
	int ret = 0, tid, c, duet_fd = 0, itret = 0, tmp;
	long total_items = 0;
	long total_fetches = 0;
	__u32 regmask;
	struct duet_item buf[FETCH_ITEMS];
	struct timespec slp = {0, 0};


	/* Duet_Collect Declaration */
	int help_flg = 0; // Used for suppressing the call of usage() function
	char rpath[DUET_MAX_PATH] = ""; // Used for holding the relative path of the file fetched
	char ppath[DUET_MAX_PATH] = "/"; // Used for holding the path that the program prints to the log
	struct stat st; // Used for extracting information about the file fetched (mainly the sizes)
	FILE *output; // File descriptor used for printing to log
	char time[20]; // Used as a printing buffer of time
	int line_counter = 0;
	
	/* Pthread Declaration */
	pthread_t thd;
	int rc;
	void *arg;
	
	int pid = getpid();
	/* Queue Declaration */
	struct node ** logs = (struct node **)malloc(MAX_LOGS * sizeof(struct node *));
	struct node * temp;
	struct queue *queue = init_queue();
	char log_path[LOG_MAX_PATH];
	int i, size = MAX_LOGS;
	check_dir("/media/data");
	for(i=0;i<size;i++) {
		logs[i] = (struct node *)malloc(sizeof(struct node));
		sprintf(log_path, "/media/data/%d", i);
		strcpy(logs[i]->path, log_path);
		strcat(logs[i]->path, ".txt");
		logs[i]->safe = 1;
		logs[i]->next = NULL;
	}
	
	/* Main Body */
	
		
	signal(SIGINT, handle_sigint); // Specify the handler of interrupt signal

	/* Process the options and their arguments */
	while ((c = getopt(argc, argv, "f:d:ohep:g")) != -1) {
		switch (c) {
		case 'f': /* Fetching frequency, in mseconds */
			freq = atoi(optarg);
			if (freq <= 0) {
				fprintf(stderr, "Error: invalid fetching frequency specified\n");
				usage(1);
			}
			break;
		case 'd': /* Program execution duration, in seconds */
			duration = atoi(optarg);
			if (duration < 0) {
				fprintf(stderr, "Error: invalid execution duration specified\n");
				usage(1);
			}
			break;
		case 'o': /* Use Duet */
			o3 = 1;
			break;
		case 'h': /* Display usage info */
			usage(0);
			break;
		case 'e': /* Register for event-based Duet */
			evtbased = 1;
			break;
		case 'p': /* Specify directory to register with Duet */
			if (strnlen(optarg, DUET_MAX_PATH + 1) > DUET_MAX_PATH) {
				fprintf(stderr, "Error: specified path too long\n");
				usage(1);
			}
			strncpy(path, optarg, DUET_MAX_PATH);
			break;
		case 'g': /* Get file path for every event */
			getpath = 1;
			break;
		default:
			fprintf(stderr, "Unknown argument!\n");
			usage(1);
		}
	}
	
	/* Processing the duration and frequency option */
	if (!duration) {
		fprintf(stdout, "Warning: Dummy task will run until Ctrl-C is pressed.\n");
		keep_running = 1;
	}

	printf("Running dummy for %d sec. Fetching every %d ms.\n",
		duration, freq);

	/* Convert duration to mseconds and set nanosleep time */
	duration *= 1000;
	slp.tv_nsec = (freq * (long) 1E6) % (long) 1E9;
	slp.tv_sec = (freq * (long) 1E6) / (long) 1E9;

	/* Open Duet device */
	if (o3 && ((duet_fd = open_duet_dev()) == -1)) {
		fprintf(stderr, "Error: failed to open Duet device\n");
		return 1;
	}

	if (evtbased)
		regmask = DUET_PAGE_ADDED | DUET_PAGE_REMOVED | DUET_FILE_TASK;
	else
		regmask = DUET_PAGE_EXISTS | DUET_FILE_TASK;

	/* Register with Duet framework */
	if (o3 && (duet_register(duet_fd, path, regmask, 1, "dummy", &tid))) {
		fprintf(stderr, "Error: failed to register with Duet\n");
		ret = 1;
		goto done_close;
	}
	/* Open the log file */
	temp = find_safe(logs, size);
	output = fopen(temp->path, "w");
	if(output == NULL) {
		perror("fopen failed");
		usage(1);
	}
	rc = pthread_create(&thd, NULL, sendLog, (void *)queue);
	if(rc) {
		printf("Error: Failed creating thread, return code is: %d\n", rc);
		exit(-1);
	}	
	/* Use specified fetching frequency */
	while (duration > 0 || keep_running) {
		if(line_counter >= LOG_MAX_SIZE){
			line_counter = 0;
			fclose(output);
			enqueue(queue, temp);
			temp = find_safe(logs, size);
			output = fopen(temp->path, "w");
			
			if(!output) {
				perror("fopen failed in while loop");
				exit(-1);
			}
		}
		if (o3) {
			itret =	FETCH_ITEMS;
			if (duet_fetch(duet_fd, tid, buf, &itret)) {
				fprintf(stderr, "Error: Duet fetch failed\n");
				ret = 1;
				goto done_dereg;
			}
			
			if (getpath) {
				for (c = 0; c < itret; c++) {
					tmp = duet_get_path(duet_fd, tid, buf[c].uuid, ppath);
					if (tmp < 0) {
						fprintf(stderr, "Error: Duet get_path failed\n");
						ret = 1;
						goto done_dereg;
					}
					/* Extract stat from file */
					if((n=stat(ppath, &st))){
						perror("stat");
						//if(EACCES == errno)
						//	printf("no permission\n");
						fprintf(stderr, "Error: stat failed for %s\n", ppath);
					//	ret = 1;
					//	goto done_dereg;
						continue;
					}
					
					if (!tmp){
						//fprintf(stdout, "Getpath code %d (evt %x). Got %s, line: %lli\n", tmp, buf[c].state, ppath,counter);
						fprintf(output, "%d,%x,%s,%lu,%lu\n",tmp, buf[c].state, ppath, buf[c].idx, (unsigned long) st.st_size);
						/*
						1. getpath code
						2. event code
						3. path
						4. page
						5. size
								*/
						line_counter += 1;	
						counter += 1;
					}
					else{
						//fprintf(stdout, "Getpath code %d (evt %x).\n", tmp, buf[c].state);
						fprintf(output, "%d,%x\n", tmp, buf[c].state);
						/*
						1. getpath code
						2. event code
								*/
						line_counter += 1;
						counter += 1;	
					}
				}
			}

			total_items += itret;
			total_fetches++;
		}

		if (nanosleep(&slp, NULL) < 0) {
			fprintf(stderr, "Error: nanosleep failed\n");
			ret = 1;
			goto done_dereg;
		}

		if (!keep_running) {
			fprintf(stdout, "nanoslept, duration left %d\n", duration);
//			fprintf(output, "nanoslept, duration left %d\n", duration);
			duration -= freq;
		}

		if (keep_running && got_sigint)
			keep_running = 0;
	}
	fclose(output);
	
	/* Deregister with the Duet framework */
done_dereg:
	if (o3 && duet_deregister(duet_fd, tid))
		fprintf(stderr, "Error: failed to deregister with Duet\n");

done_close:
	if (o3) {
		close_duet_dev(duet_fd);
		fprintf(stdout, "Fetched %ld events, or %lf events/ms\n",
			total_items, ((double) total_items)/total_fetches);
	}

	for(i=0;i<size;i++) {
		free(logs[i]);
	}
	free(logs);
	free(queue);
	return ret;

}

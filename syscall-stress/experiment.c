#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>

void printErrorAndExit(char *);
void* runExperiment(void *);

int main(int argc, char **argv) {

	if(argc != 3) {
		printErrorAndExit(argv[0]);
	}

	int threads = strtoimax(argv[1], NULL, 10); 
	char* filename = argv[2];

	if(access(filename, F_OK) == -1) {
		fprintf(stderr, "File %s does not exists\n", filename);
		exit(EXIT_FAILURE);
	} 

	pthread_t thread_id[threads];

	for (int i = 0; i < threads; i++) {
		printf("Starting reader %d\n", i);
		pthread_create(&thread_id[i], NULL, runExperiment, filename);
	}

	for(int i=0; i < threads; i++) {
		pthread_join(thread_id[i], NULL);
	}

	return EXIT_SUCCESS;
}

void printErrorAndExit(char *progName) {
	fprintf(stderr, "Usage %s <Threads> <Filename>\n", progName);
	exit(EXIT_FAILURE);
}


void* runExperiment(void *arg) {
	
    	char *filename = arg;

	FILE* fd = fopen(filename, "r");

	// Small buffer to generate a lot of syscalls
	char buffer[1] = ""; 

	while(true) {
		if(feof(fd)) {
			fseek(fd, 0, SEEK_SET);
		}
		fread(buffer, 1, 1, fd);
	}

	fclose(fd);
}

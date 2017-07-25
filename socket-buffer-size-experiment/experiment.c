#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

// 1 GB
#define BYTES_TO_TRANSFER 10737418240UL 

void printErrorAndExit(char *);
void runExperiment(int, char *, char *);
int openSocket(char *, char *);

int main(int argc, char **argv) {

	if(argc != 3) {
		printErrorAndExit(argv[0]);
	}
	
	int experimentBytes[] = {16384, 8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1};

	int elements = sizeof(experimentBytes) / sizeof(experimentBytes[0]);

	for (int i = 0; i < elements; i++) {
		int bufferSize = experimentBytes[i];
		runExperiment(bufferSize, argv[1], argv[2]);
	}
	
	return EXIT_SUCCESS;
}

void printErrorAndExit(char *progName) {
	fprintf(stderr, "Usage %s <Destination Host> <Destination Port>\n", progName);
	exit(EXIT_FAILURE);
}


int openSocket(char *hostChar, char *portChar) {
	struct sockaddr_in server_addr;
	int fd;

	int port = atoi(portChar);

	printf("#Opening connection to %s:%i\n", hostChar, port);

	fd = socket(AF_INET, SOCK_STREAM, 0);

	if(fd < 0) {
		fprintf(stderr, "Unable to open socket\n");
		exit(EXIT_FAILURE);
	}

	struct hostent* server = gethostbyname(hostChar);

	if(! server) {
		fprintf(stderr, "Unable to resolve %s\n", hostChar);
		exit(EXIT_FAILURE);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	
	server_addr.sin_addr.s_addr =
		((struct in_addr *)server->h_addr_list[0])->s_addr;	

	if(connect(fd, (struct sockaddr*) 
		&server_addr, sizeof(struct sockaddr)) < 0) { 

		fprintf(stderr, "Unable to connect\n");
		exit(EXIT_FAILURE);
    	}  

	return fd;
}

void runExperiment(int bufferSize, char *hostChar, char *portChar) {
	printf("#Running experiment with a buffer size of %i bytes\n", bufferSize);
	int fd = openSocket(hostChar, portChar);

	char* buffer = (char *) malloc(bufferSize * sizeof(char));
	memset(buffer, 'a', bufferSize * sizeof(char));

	struct timeval startTime;
	struct timeval stopTime;

	gettimeofday(&startTime, NULL);

	long sendBytes = 0;
	int ret = 0;
	while(sendBytes < BYTES_TO_TRANSFER) {
		
		for (int n = 0; n < bufferSize; ) {
			ret = write(fd, (char *)buffer + n, bufferSize - n);
			
			if (ret < 0) { 
                		if (errno == EINTR || errno == EAGAIN) {
                   			continue;
                		}    
                		break;
           		} else {
              			n += ret; 
           		}    
		}

		sendBytes += bufferSize;
	} 

	gettimeofday(&stopTime, NULL);	
	long diff = ((stopTime.tv_sec - startTime.tv_sec) * 1000000L 
            + stopTime.tv_usec) - startTime.tv_usec;
	printf("Time for transfer data: %lu\n", diff);   
	shutdown(fd, 2);

	free(buffer);
	buffer = NULL;
}

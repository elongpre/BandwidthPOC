#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>


#define PORT 20001
#define LENGTH 512 


void error(const char *msg){
	perror(msg);
	printf("\n");
	exit(1);
}

int main(int argc, char *argv[]){
	int sockfd; 
	int nsockfd;
	char revbuf[LENGTH]; 
	struct sockaddr_in remote_addr;
	struct timeval start_time, finish_time;
	int fr_block_sz = 0;

	if ( sizeof(argv) < 1){
		error("invalid number of arguements");
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		error("failed to obtain socket");
	}

	remote_addr.sin_family = AF_INET; 
	remote_addr.sin_port = htons(PORT); 
	inet_pton(AF_INET, "127.0.0.1", &remote_addr.sin_addr); 
	memset(&(remote_addr.sin_zero), 0, 8);

	if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1){
		error("failed to connect to the host");
	}

	printf("[client] connected to port %d on the server\n", PORT);

	const char* fs_name = argv[1];
	char sdbuf[LENGTH]; 
	printf("[client] sending %s... ", fs_name);
	FILE *fs = fopen(fs_name, "r");
	if(fs == NULL){
		error(strcat( (char*)"cannot find", fs_name));
	}

	memset(sdbuf, 0, LENGTH); 
	int fs_block_sz; 
	gettimeofday(&start_time, NULL);
	while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0){
	    send(sockfd, sdbuf, fs_block_sz, 0);
	    memset(sdbuf, 0, LENGTH);
	}
	fclose(fs);
	if ( (fr_block_sz = recv(sockfd, revbuf, LENGTH, 0)) < 0){
		error("error when running recv");
	}
	if( revbuf[0] != '1'){
		error("file was not successfully sent");
	}
	gettimeofday(&finish_time, NULL);
	printf("%s was sent\n", fs_name);
	printf("[client] the upload took %.0f.%06lus\n", difftime(finish_time.tv_sec, start_time.tv_sec), finish_time.tv_usec-start_time.tv_usec);


	printf("[client] receiving file and saving as final.txt... ");
	const char* fr_name = "./final.txt";
	FILE *fr = fopen(fr_name, "a");
	if(fr == NULL){
		error("final.txt cannot be opened");
	}

	memset(revbuf, 0, LENGTH); 
	gettimeofday(&start_time, NULL);
    while((fr_block_sz = recv(sockfd, revbuf, LENGTH, 0)) > 0){
		int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
        if(write_sz < fr_block_sz){
            error("writing to file failed.");
        }
		memset(revbuf, 0, LENGTH);
		if (fr_block_sz != 512){
			break;
		}
	}
	if(fr_block_sz < 0){
        error("error when running recv");
  	}

	gettimeofday(&finish_time, NULL);
	fclose(fr);
    printf("final.txt was received and saved\n");	    
	
	printf("[client] the download took %.0f.%06lus\n", difftime(finish_time.tv_sec, start_time.tv_sec), finish_time.tv_usec-start_time.tv_usec);
	close(sockfd);
	printf("[client] closing connection\n");
	return (0);
}
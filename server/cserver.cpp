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

#define PORT 20001 
#define PORT_STR "20001"
#define BACKLOG 5
#define LENGTH 512 

void error(const char *msg){
    perror(msg);
    printf("\n");
    exit(1);
}

char* runserver(int sockfd){

    int nsockfd;
    int sin_size;  
    struct sockaddr_in addr_remote; 
    char revbuf[LENGTH]; 
    char sdbuf[LENGTH];

    while(true)
    {
        sin_size = sizeof(struct sockaddr_in);

        if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, (socklen_t *)&sin_size)) == -1){
            return (char*)  "failed to accept socket";
        } 
        printf("[server] server has connected to %s\n", inet_ntoa(addr_remote.sin_addr));

        const char* fr_name = "./receive.txt";
        FILE *fr = fopen(fr_name, "a");
        if(fr == NULL){
             return (char*)  "receive.txt cannot be opened";
        }

        memset(revbuf, 0, LENGTH); 
        int fr_block_sz = 0;
        printf("[server] waiting to receive file... \n");
        while((fr_block_sz = recv(nsockfd, revbuf, LENGTH, 0)) > 0){
            int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
            if(write_sz < fr_block_sz){
                 return (char*)  "failed to write to receive.txt";
            }
            memset(revbuf, 0, LENGTH);
            if (fr_block_sz != 512){
                break;
            }
        }
        if(fr_block_sz < 0){
            return (char*)  "error when running recv";
        }
        sdbuf[0] = '1';
        send(nsockfd, sdbuf, 1, 0);
        printf("file received and saved\n");
        fclose(fr); 

        system("cp receive.txt output.txt");

        const char* fs_name = "./output.txt";
        printf("[server] sending %s... ", fs_name);
        FILE *fs = fopen(fs_name, "r");
        if(fs == NULL){
            return strcat( (char*) "cannot find", fs_name);
        }

        memset(sdbuf, 0, LENGTH); 
        int fs_block_sz; 
        while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0){
            send(nsockfd, sdbuf, fs_block_sz, 0);
            memset(sdbuf, 0, LENGTH);
        }
        close(nsockfd);
        fclose(fs);
        system("rm receive.txt output.txt");
        printf("%s sent\n", fs_name);
        printf("[server] closing connection with client\n");
    }
}

int main (){
    int sockfd; 
    struct sockaddr_in addr_local; 

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        error("failed to obtain socket");
    }

    addr_local.sin_family = AF_INET; 
    addr_local.sin_port = htons(PORT);
    addr_local.sin_addr.s_addr = INADDR_ANY;
    memset(&(addr_local.sin_zero), 0, 8);

    if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){
        error(strcat( (char*)"failed to bind port", PORT_STR));
    }
    printf("[server] sucessfully binded tcp port %d for address 127.0.0.1.\n",PORT);

    if(listen(sockfd,BACKLOG) == -1){
        error("failed to listen");
    }
    printf ("[server] listening to port %d\n", PORT);

    while(true){
        char *message = runserver(sockfd);
        printf("%s", message);
        printf("\n");
    }

}

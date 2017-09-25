#include <iostream>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#define BUFFERSIZE 1048576 // 1 MB Buffersize
#define LISTENPORT 10025

using namespace std;

void handleRead(int connfd) {
   char buffer[BUFFERSIZE];
   size_t bytesRead = 1;
   
   cout << "[Info] Handling new client connection" << endl;
   
   // Set socket to blocking mode
   int flags = fcntl(connfd, F_GETFL, 0);
   flags = (flags&~O_NONBLOCK);
   fcntl(connfd, F_SETFL, flags);
   
   while(bytesRead > 0) {
      bytesRead = read(connfd, buffer, BUFFERSIZE);
   }
   
   if(connfd != 0) {
      shutdown(connfd, 2);
   }
}

/*
2.7 Main function

The function open a socket, generate requested data and close the socket

*/
int main(int argc, char* argv[]) {
   
   // Our server socket
   struct sockaddr_in serv_addr;   // Server address  
   struct sockaddr_in client_addr; // Client address
   
   int listenfd = socket(AF_INET, SOCK_STREAM, 0);
   int connfd = 0;

   memset(&serv_addr, 0, sizeof(serv_addr));
   memset(&client_addr, 0, sizeof(client_addr));

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   serv_addr.sin_port = htons(LISTENPORT); 

   // Bind our socket
   if ((bind(listenfd, (struct sockaddr *) 
       &serv_addr, sizeof(serv_addr)) ) < 0) {
     
     cerr << "[Error] Bind failed" << endl;
     return false;
   }

   // Listen
   if(( listen(listenfd, 10)) < 0 ){
     cerr << "[Error] Listen failed " << endl;
     return false;
   }
   
   unsigned int clientlen = sizeof(client_addr);

   // Accept connection
   if(! (connfd = accept(listenfd, (struct sockaddr *) &client_addr, 
         &clientlen))) {
           
     cerr << "[Error] Accept failed" << endl;
     return false;
   } else {
      handleRead(connfd);
   }
   
   if(listenfd != 0) {
     shutdown(connfd, 2);
     connfd = 0;
   }

   return EXIT_SUCCESS;  
}

/*
/ Eric Olson and James Baker
/ CSE 5462 Network Programming
/ Lab 3 - September 15, 2016
/ 
/ This file contains our implementations of a TCP server.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#define LOCALPORT 9999
#define LOCALADDRESS "127.0.0.1"
#define MSS 1000

int main(int argc, char* argv[]) {
	
    	int sock;                     /* initial socket descriptor */
    	struct sockaddr_in sin_addr; /* structure for socket name setup */


    	char fileName[20] = {'\0'};
    	int fileSize = 0;
    	char readBuffer[MSS] = {0};
	struct stat st = {0};
	char pathName[] = "recvd/";
    	printf("TCP server waiting for remote connection from clients ...\n\n");
 
    	/*initialize socket connection in unix domain*/
    	if((sock = SOCKET(AF_INET, SOCK_STREAM, 0)) < 0){
    		perror("Error opening socket");
    		exit(1);
    	}

    	sin_addr.sin_family = AF_INET;
    	sin_addr.sin_port = htons(LOCALPORT);
    	sin_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr(LOCALADDRESS);

    	/* bind socket name to socket */
    	if(BIND(sock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
      		perror("Error binding stream socket");
      		exit(1);
    	}
  
	/* listen for socket connection and set max opened socket connetions to 5 */
	//listen(sock, 5);
  
  	/* accept a connection in socket msgsocket */ 
  	if((sock = ACCEPT(sock, (struct sockaddr *)NULL, (socklen_t *)NULL)) == -1) { 
    		perror("Error connecting stream socket");
    		exit(1);
  	}
  	/* get the size of the payload */
  	if (RECV(sock, &fileSize, 4, 0, NULL, NULL) < 4) {
  		printf("%s\n", "Error: The size read returned less than 4");
  		exit(1);
  	}
	printf("Recieved size: %d bytes\n\n", fileSize);

  	/* get the name of the file */
  	if (RECV(sock, fileName, sizeof(fileName), 0) < 20) {
  		printf("%s\n", "Error: The name read returned less than 20");
  		exit(1);
  	}
	printf("Received name: %s\n\n", fileName);

  	/* create a directory if one does not already exist */  
	if (stat("recvd", &st) == -1) {
    		mkdir("recvd", 0700);
	}

  	FILE* output = fopen(strcat(pathName, fileName), "wb");
	
  	if (NULL == output) {
		fprintf(stderr, "%s\n", "Error opening the output file");
		exit(1);
	}

  	/* read the payload from the stream until the whole payload has been read */
  	int amtReadTotal = 0;
  	int amtRead = 0;
  	while (amtReadTotal < fileSize) { 
  		amtRead = RECV(sock, readBuffer, sizeof(readBuffer), 0);
		
  		amtReadTotal += amtRead;
  		if (amtRead < 0) {
  			fprintf(stderr, "%s\n\n", "Error reading from the connection stream. Server terminating");
  			exit(1);
  		} 

  		/* write the received data to the output file */
  		fwrite(readBuffer, 1, amtRead, output);
		//fflush(output);
		bzero(readBuffer, sizeof(readBuffer));
		amtRead = 0;
		//printf("Bytes Recieved: %i\n", amtReadTotal);
  	}

	printf("Recieved file.\n");
  
  	/* close the output file and connections */
  	//close(msgsock);
  	close(sock);
  	fclose(output);

  	return 0;
}


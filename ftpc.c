/*
/ Eric Olson and James Baker
/ CSE 5462 Network Programming
/ Lab 3 - September 15, 2016
/ 
/ This file contains our implementation of a TCP client.
*/


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#define MSS 1000
#define LOCALPORT 9999
#define LOCALADDRESS "127.0.0.1"

/* client program called with host name where server is run */
int main(int argc, char *argv[]) {
	/* validate input args */
	if(argc < 1) {
		fprintf(stderr, "Error: Include file name is arguments.\n");
		exit(1);
	}

	int sock;                     /* initial socket descriptor */
	int rval;                     /* returned value from a read */
	int num_bytes;		      /* number of bytes of file to be sent */
	int num_read;		      /* bytes read from file stream */
	struct sockaddr_in sin_addr;  /* structure for socket name setup */
	char * file_name = argv[1];   /* file name */
	char buf[MSS] = {0};          /* bytes to send to server */
	struct hostent *hp;	      /* host */
	
	printf("%s\n\n", "TCP client preparing to send file...");

	/* initialize socket connection in unix domain */
	if((sock = SOCKET(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Error opening socket");
		exit(1);
	}

  	/* construct name of socket to send to */
  	sin_addr.sin_family = AF_INET;
  	sin_addr.sin_port = 0; /* fixed by adding htons() */
	sin_addr.sin_addr.s_addr = INADDR_ANY;
  
  	/* establish connection with server */
  	if(CONNECT(sock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) 
	{
    		close(sock);
    		perror("Error connecting stream socket");
    		exit(1);
  	}
  
  	/* Open file for transfer */
  	FILE *fp = fopen(file_name,"r");
  	if(fp==NULL) 
	{
    		perror("Error opening file");
    		exit(1);  
  	}

  	/* Read number of bytes in file, rewind file to start index */  
	fseek(fp, 0L, SEEK_END);
	num_bytes = ftell(fp);
	rewind(fp);
        
	/* Send file size in 4 bytes */
	SEND(sock, &num_bytes, 4, 0);
	printf("Sent: %i bytes.\n\n", num_bytes);
	sleep(0.1);
	

	/* Send file name in 20 bytes */
	SEND(sock, file_name, 20, 0);
	printf("Sent: %s.\n\n", file_name);
	sleep(0.1);

	while(1)
	{
	  	/* Read file in chunks of 512 bytes */
		num_read = fread(buf,1,MSS,fp);

		/* If read was successful send data. */
		if(num_read > 0)
		{
		   	SEND(sock, buf, num_read, 0);
			printf("Sent: %s\n\n", buf);
			sleep(0.1);
		}

		/* Handle end of file or read error */
		if (num_read < MSS)
		{
		  	if (feof(fp)) 
		  	{
		  		break;
		  	}
		  	else if (ferror(fp))
		  	{
				perror("Error reading file\n");
				exit(1);
		  	}
		}
	}

	printf("%s\n", "File Sent.");

	/* Close file and connection */
	close(fp);
	close(sock);
	return(0); 
}

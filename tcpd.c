#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netdb.h>
#define MSS 1000
#define LOCAL_PORT 9999
#define TROLL_PORT 8888
#define LOCAL_ADDRESS "127.0.0.1"
#define POLY 0x8408

unsigned short crc16(char *data_p, unsigned short length);

//Troll message struct
typedef struct MyMessage {
	struct sockaddr_in msg_header;
	char body[MSS];
	
} MyMessage;

//Local to tcpd message struct
typedef struct tcpdHeader {
	int flag;
	size_t maxData;
	char body[MSS];
} tcpdHeader;

/* for lint */
void bzero(), bcopy(), exit(), perror();
double atof();
#define Printf if (!qflag) (void)printf
#define Fprintf (void)fprintf

int main(int argc, char *argv[])
{
	if (argc < 1) {
		fprintf(stderr, "%s\n", "There are not enough arguments.");
		exit(1);
	}
	
	/* Run on client side */
	if (atoi(argv[1]) == 1) {
		if (argc < 5) {
			fprintf(stderr, "%s\n", "There are not enough arguments. Please be sure to include Local Host, Local Troll Port, Remote Host, and Remote Port.");
			exit(1);
		}

		printf("%s\n\n", "Running on client machine.");

		int troll_sock;	/* a socket for sending messages to the local troll process */
		int local_sock; /* a socket to communicate with the client process */
		MyMessage message; /* Packet sent to troll process */
		tcpdHeader tcpd_head;/* Packet type from client */
		struct hostent *host; /* Hostname identifier */
		struct sockaddr_in trolladdr, destaddr, localaddr, clientaddr; /* Addresses */
		fd_set troll_selectmask;
		fd_set client_selectmask;
	
		/* TROLL ADDRESSS */
		/* this is the addr that troll is running on */

		if ((host = gethostbyname(argv[2])) == NULL) {
			printf("Unknown troll host '%s'\n",argv[2]);
			exit(1);
		}  
		
		bzero ((char *)&trolladdr, sizeof trolladdr);
		trolladdr.sin_family = AF_INET;
		bcopy(host->h_addr, (char*)&trolladdr.sin_addr, host->h_length);
		trolladdr.sin_port = htons(atoi(argv[3]));

		/* DESTINATION ADDRESS */
		/* This is the destination address that the troll will forward packets to */

		if ((host = gethostbyname(argv[4])) == NULL) {
			printf("Unknown troll host '%s'\n",argv[4]);
			exit(1);
		} 

		bzero ((char *)&destaddr, sizeof destaddr);
		destaddr.sin_family = htons(AF_INET);
    		bcopy(host->h_addr, (char*)&destaddr.sin_addr, host->h_length);
		destaddr.sin_port = htons(atoi(argv[5]));

		/* SOCKET TO TROLL */
		/* This creates a socket to communicate with the local troll process */

		if ((troll_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("totroll socket");
			exit(1);
		}
		FD_ZERO(&troll_selectmask);
		FD_SET(troll_sock, &troll_selectmask);

		bzero((char *)&localaddr, sizeof localaddr);
		localaddr.sin_family = AF_INET;
		localaddr.sin_addr.s_addr = INADDR_ANY; /* let the kernel fill this in */
		localaddr.sin_port = 0;					/* let the kernel choose a port */
		if (bind(troll_sock, (struct sockaddr *)&localaddr, sizeof localaddr) < 0) {
			perror("client bind");
			exit(1);
		}

		/* SOCKET TO CLIENT */
		/* This creates a socket to communicate with the local troll process */

		if ((local_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("client socket");
			exit(1);
		}
		FD_ZERO(&client_selectmask);
		FD_SET(local_sock, &client_selectmask);
		

		bzero((char *)&clientaddr, sizeof clientaddr);
		clientaddr.sin_family = AF_INET;
		clientaddr.sin_addr.s_addr = inet_addr(LOCAL_ADDRESS); /* let the kernel fill this in */
		clientaddr.sin_port = htons(LOCAL_PORT);
		if (bind(local_sock, (struct sockaddr *)&clientaddr, sizeof clientaddr) < 0) {
			perror("client bind");
			exit(1);
		}

		/* SEND DATA TO TROLL */
	
		for(;;) {
			// Block until input arrives on one or more sockets
			if(select(FD_SETSIZE, &client_selectmask, NULL, NULL, NULL) < 0) {
			    fprintf(stderr, "%s\n", "There was an issue with select()");
			    exit(1);
			}

			if (FD_ISSET(local_sock, &client_selectmask)) {
				int amtFromClient = 0;
				//receive data from the local socket
				amtFromClient = recvfrom(local_sock, (char *)&tcpd_head, sizeof(tcpd_head), 0, NULL, NULL);

				if (tcpd_head.flag == 1) {
					//forward the data to remote machine via troll
					printf("Received message from client.\n");
				
					//create troll message
					strcpy(message.body,tcpd_head.body);
					message.msg_header = destaddr;

					unsigned short chksum = crc16((char *)&message.body,sizeof(message.body));
					printf("Checksum of message body: %hu\n", chksum);
					
					int amtToTroll = 0;
					amtToTroll = sendto(troll_sock, (char *)&message, sizeof message, 0, (struct sockaddr *)&trolladdr, sizeof trolladdr);
					printf("Sent message to troll.\n\n");
					if (amtToTroll != sizeof message) {
						perror("totroll sendto");
						exit(1);
					}
					
			     	} else {
					fprintf(stderr, "%s\n", "Message from unknown source");
				 	exit(1);
			   	} 
			} 

		}
		
		
		

	/* Run on server side */
	} else if (atoi(argv[1]) == 0) {

		if (argc < 2) {
			fprintf(stderr, "%s\n", "There are not enough arguments. Please be sure to include the local port.");
			exit(1);
		}
		printf("%s\n\n", "Running on server machine.");		

		int troll_sock;	/* a socket for sending messages and receiving responses */
		int local_sock; /* a socket to communicate with the client process */
		MyMessage message; /* recieved packet from remote troll process */
		struct sockaddr_in trolladdr, localaddr, serveraddr; /* Addresses */
		struct hostent *host; /* Hostname identifier */
		int n; /* for data recieved */
		fd_set troll_selectmask;
		
		/* SOCKET FROM TROLL */

		if ((troll_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("fromtroll socket");
			exit(1);
		}
		bzero((char *)&localaddr, sizeof localaddr);
		localaddr.sin_family = AF_INET;
		localaddr.sin_addr.s_addr = INADDR_ANY; /* let the kernel fill this in */
		localaddr.sin_port = htons(atoi(argv[2]));
		if (bind(troll_sock, (struct sockaddr *)&localaddr, sizeof localaddr) < 0) {
			perror("client bind");
			exit(1);
		}
		FD_ZERO(&troll_selectmask);
		FD_SET(troll_sock, &troll_selectmask);

		/* SOCKET TO SERVER */
		/* This creates a socket to communicate with the local troll process */

		if ((local_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("client socket");
			exit(1);
		}
		
		/* ADDRESS TO CONNECT WITH THE SERVER */

		struct sockaddr_in destaddr;
		destaddr.sin_family = AF_INET;
		destaddr.sin_port = htons(LOCAL_PORT);
		destaddr.sin_addr.s_addr = inet_addr(LOCAL_ADDRESS);

		/* RECEIVE DATA */

		for(;;) {
			//block until data arrives on troll port
			if(select(FD_SETSIZE, &troll_selectmask, NULL, NULL, NULL) < 0) {
			    fprintf(stderr, "%s\n", "There was an issue with select()");
			    exit(1);
			}
			if (FD_ISSET(troll_sock, &troll_selectmask)) {
				int len = sizeof trolladdr;
	
				/* read in one message from the troll */
				n = recvfrom(troll_sock, (char *)&message, sizeof message, 0,
					(struct sockaddr *)&trolladdr, &len);
				if (n<0) {
					perror("fromtroll recvfrom");
					exit(1);
				}
				printf("Recieved message from troll.\n");

				unsigned short chksum = crc16((char *)&message.body,sizeof(message.body));
				printf("Checksum of message rec: %hu\n", chksum);

				//forward to server
				int amtToServer = 0;
				char body[MSS] = {0};
				strcpy(body, message.body);
					
				amtToServer = sendto(local_sock, (char *)&body, sizeof body, 0, (struct sockaddr *)&destaddr, sizeof destaddr);
				printf("Sent message to server.\n\n");
				if (amtToServer != sizeof body) {
					perror("totroll sendto");
					exit(1);
				}
			}
			
		
		
		}
	}
    
}

// SOURCE: http://www8.cs.umu.se/~isak/snippets/crc-16.c

/*
//                                      16   12   5
// this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
// This works out to be 0x1021, but the way the algorithm works
// lets us use 0x8408 (the reverse of the bit pattern).  The high
// bit is always assumed to be set, thus we only use 16 bits to
// represent the 17 bit value.
*/

unsigned short crc16(char *data_p, unsigned short length)
{
      unsigned char i;
      unsigned int data;
      unsigned int crc = 0xffff;

      if (length == 0)
            return (~crc);

      do
      {
            for (i=0, data=(unsigned int)0xff & *data_p++;
                 i < 8; 
                 i++, data >>= 1)
            {
                  if ((crc & 0x0001) ^ (data & 0x0001))
                        crc = (crc >> 1) ^ POLY;
                  else  crc >>= 1;
            }
      } while (--length);

      crc = ~crc;
      data = crc;
      crc = (crc << 8) | (data >> 8 & 0xff);

      return (crc);
}

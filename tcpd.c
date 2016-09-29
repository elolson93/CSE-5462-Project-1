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
/* CRC algorithm used from http://www.barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code */
#include "crc/crc.c"
#include "crc/crc.h"
#define MSS 1000
#define LOCAL_PORT 9999
#define TROLL_PORT 8888
#define LOCAL_ADDRESS "127.0.0.1"

//packet struct
typedef struct Packet {
	char body[MSS];
	int bytes_to_read;
	//add additional vars here
} Packet;

//Troll message struct
typedef struct MyMessage {
	struct sockaddr_in msg_header;
	struct Packet msg_pack;
	int chksum;

	
} MyMessage;


//Local to tcpd message struct
typedef struct tcpdHeader {
	int flag;
	size_t maxData;
	char body[MSS];
	int bytes_to_read;
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
		Packet packet;
		tcpdHeader tcpd_head;/* Packet type from client */
		struct hostent *host; /* Hostname identifier */
		struct sockaddr_in trolladdr, destaddr, localaddr, clientaddr; /* Addresses */
		fd_set selectmask;
		char buffer[MSS] = {0};
		int amtFromClient = 0;
		int chksum = 0;
		int amtToTroll = 0;
	
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
		
		

		bzero((char *)&clientaddr, sizeof clientaddr);
		clientaddr.sin_family = AF_INET;
		clientaddr.sin_addr.s_addr = inet_addr(LOCAL_ADDRESS); /* let the kernel fill this in */
		clientaddr.sin_port = htons(LOCAL_PORT);
		if (bind(local_sock, (struct sockaddr *)&clientaddr, sizeof clientaddr) < 0) {
			perror("client bind");
			exit(1);
		}

		/* SEND DATA TO TROLL */

		//for checksum		
		crcInit();
		
		FD_ZERO(&selectmask);
		FD_SET(local_sock, &selectmask);

		bzero ((char *)&packet, sizeof packet);
		bzero ((char *)&message, sizeof message);
		bzero ((char *)&tcpd_head, sizeof tcpd_head);
		int total = 0;
		for(;;) {
			if (FD_ISSET(local_sock, &selectmask)) {
				amtFromClient = 0;
				//receive data from the local socket
				amtFromClient = recvfrom(local_sock, (char *)&tcpd_head, sizeof(tcpd_head), 0, NULL, NULL);

				if (tcpd_head.flag == 1) {
					//forward the data to remote machine via troll
					printf("Received message from client.\n");
					bcopy(tcpd_head.body,buffer, tcpd_head.bytes_to_read);
				
					//create packet
					bcopy(buffer,packet.body,tcpd_head.bytes_to_read);
					packet.bytes_to_read = tcpd_head.bytes_to_read;

					//create troll wrapper
					message.msg_pack = packet;
					message.msg_header = destaddr;

				
					chksum = crcFast((char *)&message.msg_pack,sizeof(message.msg_pack));
					message.chksum = chksum;
					printf("Checksum of packet: %X\n", chksum);

					amtToTroll = sendto(troll_sock, (char *)&message, sizeof message, 0, (struct sockaddr *)&trolladdr, sizeof trolladdr);
					printf("Sent message to troll.\n\n");
					if (amtToTroll != sizeof message) {
						perror("totroll sendto");
						exit(1);
					}
					total += amtToTroll;
					//printf("Sent byes: %i\n", total);
					bzero ((char *)&packet, sizeof packet);
					bzero ((char *)&message, sizeof message);
					bzero ((char *)&tcpd_head, sizeof tcpd_head);
					//nanosleep(1);
			     	} else {
					fprintf(stderr, "%s\n", "Message from unknown source");
				 	exit(1);
			   	} 
			FD_ZERO(&selectmask);
			FD_SET(local_sock, &selectmask);
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
		fd_set selectmask;
		char buffer[MSS] = {0};
		int amtToServer = 0;
		int chksum = 0;
		int len = 0;
		
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

		//for checksum		
		crcInit();

		FD_ZERO(&selectmask);
		FD_SET(troll_sock, &selectmask);

		bzero ((char *)&buffer, sizeof buffer);
		bzero ((char *)&message, sizeof message);
		int total = 0;
		for(;;) {
			if (FD_ISSET(troll_sock, &selectmask)) {
				len = sizeof trolladdr;
	
				/* read in one message from the troll */
				n = recvfrom(troll_sock, (char *)&message, sizeof message, 0,
					(struct sockaddr *)&trolladdr, &len);
				if (n<0) {
					perror("fromtroll recvfrom");
					exit(1);
				}
				printf("Recieved message from troll.\n");
				//printf("Message body: %s\n", message.msg_pack.body);

				chksum = crcFast((char *)&message.msg_pack,sizeof(message.msg_pack));
				printf("Checksum of message rec: %X\n", chksum);
				if (chksum != message.chksum) {
					printf("CHECKSUM ERROR: Expected: %X Actual: %X\n", message.chksum, chksum);
				}

				

				//forward to server
				amtToServer = 0;
				
				bcopy(message.msg_pack.body,buffer,message.msg_pack.bytes_to_read);
					
				amtToServer = sendto(local_sock, (char *)&buffer, message.msg_pack.bytes_to_read, 0, (struct sockaddr *)&destaddr, sizeof destaddr);
				printf("Sent message to server.\n\n");
				if (amtToServer != message.msg_pack.bytes_to_read) {
					perror("totroll sendto");
					exit(1);
				}
				total += n;
				//printf("Sent byes: %i\n", total);
				bzero ((char *)&buffer, sizeof buffer);
				bzero ((char *)&message, sizeof message);
			}
			
		
			FD_ZERO(&selectmask);
			FD_SET(troll_sock, &selectmask);
		}
	}
    
}


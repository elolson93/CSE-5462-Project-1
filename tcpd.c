/*
/ Eric Olson and James Baker
/ CSE 5462 Network Programming
/ Project 1 - Checkpoint 1 - September 29, 2016
/ 
/ This file contains our tcpd daemon implementation
/ Note: /* CRC algorithm used from public domain implmentation from
/ http://www.barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
*/

#include "header.h"
#include "crc/crc.c"
#include "crc/crc.h"

/* for lint */
void bzero(), bcopy(), exit(), perror();
double atof();
#define Printf if (!qflag) (void)printf
#define Fprintf (void)fprintf

/* main */
int main(int argc, char *argv[])
{
	/* Validate initial args */
	if (argc < 1) {
		fprintf(stderr, "%s\n", "There are not enough arguments.");
		exit(1);
	}
	
	/* Run on client side */
	if (atoi(argv[1]) == 1) {
		/* Validate input args */
		if (argc < 5) {
			fprintf(stderr, "%s\n", "There are not enough arguments. Please be sure to include Local Host, Local Troll Port, Remote Host, and Remote Port.");
			exit(1);
		}

		printf("%s\n\n", "Running on client machine...");

		int troll_sock;							/* a socket for sending messages to the local troll process */
		int local_sock; 						/* a socket to communicate with the client process */
		MyMessage message; 						/* Packet sent to troll process */
		Packet packet;							/* Packet sent to server tcpd */
		tcpdHeader tcpd_head;						/* Packet type from client */
		struct hostent *host; 						/* Hostname identifier */
		struct sockaddr_in trolladdr, destaddr, localaddr, clientaddr;  /* Addresses */
		fd_set selectmask; 						/* Socket descriptor for select */
		int amtFromClient, amtToTroll, total = 0; 			/* Bookkeeping vars for sending */
		int chksum = 0;							/* Checksum */
		
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
		clientaddr.sin_addr.s_addr = inet_addr(LOCALADDRESS); /* let the kernel fill this in */
		clientaddr.sin_port = htons(LOCALPORT);
		if (bind(local_sock, (struct sockaddr *)&clientaddr, sizeof clientaddr) < 0) {
			perror("client bind");
			exit(1);
		}

		/* SEND DATA TO TROLL */

		/* Initialize checksum table */	
		crcInit();
		
		/* Prepare for select */
		FD_ZERO(&selectmask);
		FD_SET(local_sock, &selectmask);
		
		/* Begin send loop */
		for(;;) {
			
			/* Wait for data on socket from cleint */
			if (FD_ISSET(local_sock, &selectmask)) {
				
				/* Receive data from the local socket */
				amtFromClient = recvfrom(local_sock, (char *)&tcpd_head, sizeof(tcpd_head), MSG_WAITALL, NULL, NULL);

				/* Packet from client (client = 1)*/
				if (tcpd_head.flag == 1) {
					
					printf("Received message from client.\n");
					
					/* Copy payload from client to tcpd packet */
					bcopy(tcpd_head.body,packet.body, tcpd_head.bytes_to_read);
				
					/* Pass along actualy bytes to be read from payload */
					packet.bytes_to_read = tcpd_head.bytes_to_read;

					/* Prepare troll wrapper */
					message.msg_pack = packet;
					message.msg_header = destaddr;

					/* Calculate checksum */				
					chksum = crcFast((char *)&message.msg_pack,sizeof(message.msg_pack));
					printf("Checksum of packet: %X\n", chksum);

					/* Attach checksum to troll packet */
					message.chksum = chksum;

					/* Send packet to troll */
					amtToTroll = sendto(troll_sock, (char *)&message, sizeof message, 0, (struct sockaddr *)&trolladdr, sizeof trolladdr);
					printf("Sent message to troll.\n\n");
					if (amtToTroll != sizeof message) {
						perror("totroll sendto");
						exit(1);
					}

					/* For bookkeeping/debugging */
					total += amtToTroll;

				/* Packet from server (server = 0)*/
			     	} else if (tcpd_head.flag == 0) {
					/* Fill in later: Useful for checkpoint 2 */
				/* Unexpected packet type */
			   	} else {
					fprintf(stderr, "%s\n", "Message from unknown source");
				 	exit(1);
				}

			/* Reset socket descriptor for select */
			FD_ZERO(&selectmask);
			FD_SET(local_sock, &selectmask);
			} 
		}
		
	/* Run on server side */
	} else if (atoi(argv[1]) == 0) {

		/* Validate args */
		if (argc < 2) {
			fprintf(stderr, "%s\n", "There are not enough arguments. Please be sure to include the local port.");
			exit(1);
		}

		printf("%s\n\n", "Running on server machine...");		

		int troll_sock;						/* a socket for sending messages and receiving responses */
		int local_sock; 					/* a socket to communicate with the client process */
		MyMessage message; 					/* recieved packet from remote troll process */
		struct sockaddr_in trolladdr, localaddr, serveraddr;    /* Addresses */
		struct hostent *host; 					/* Hostname identifier */
		fd_set selectmask;					/* Socket descriptor for select */
		int amtFromTcpd, amtToServer, len, total = 0; 		/* Bookkeeping vars */
		int chksum = 0;						/* Checksum */
		
		
		/* SOCKET FROM TROLL */
		/* This is the socket to recieve from the troll running on the client machine */
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
		destaddr.sin_port = htons(LOCALPORT);
		destaddr.sin_addr.s_addr = inet_addr(LOCALADDRESS);

		/* RECEIVE DATA */

		/* Initialize checksum table */		
		crcInit();

		/* Prepare descriptor*/
		FD_ZERO(&selectmask);
		FD_SET(troll_sock, &selectmask);

		/* Begin recieve loop */
		for(;;) {
		
			/* If data is ready to be recieved from troll on client machien */
			if (FD_ISSET(troll_sock, &selectmask)) {
				
				/* length of addr for recieve call */
				len = sizeof trolladdr;
	
				/* read in one packet from the troll */
				amtFromTcpd = recvfrom(troll_sock, (char *)&message, sizeof message, MSG_WAITALL,
					(struct sockaddr *)&trolladdr, &len);
				if (amtFromTcpd < 0) {
					perror("fromtroll recvfrom");
					exit(1);
				}

				printf("Recieved message from troll.\n");

				/* Calculate checksum of packet recieved */
				chksum = crcFast((char *)&message.msg_pack,sizeof(message.msg_pack));
				printf("Checksum of message rec: %X\n", chksum);

				/* Compare expected checksum to one caluclated above. Print Error if conflict. */
				if (chksum != message.chksum) {
					printf("CHECKSUM ERROR: Expected: %X Actual: %X\n", message.chksum, chksum);
				}

				/* Forward packet body to server */
				amtToServer = sendto(local_sock, (char *)&message.msg_pack.body, message.msg_pack.bytes_to_read, 0, (struct sockaddr *)&destaddr, sizeof destaddr);
				if (amtToServer < 0) {
					perror("totroll sendto");
					/* To keep daemon running for grable demo */
					//exit(1);
				}
				printf("Sent message to server.\n\n");

				/* Bookkeeping/Debugging */
				total += amtFromTcpd;
			}
			
			/* Reset decriptor */
			FD_ZERO(&selectmask);
			FD_SET(troll_sock, &selectmask);
		}
	}
}



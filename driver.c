/*
/ Eric Olson and James Baker
/ CSE 5462 Project 1, Checkpoint 1
/ September 29, 2016
/ This file is meant to interact with the timer-process
*/

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

/* These are the prototypes for the functions described in this file */
void starttimer(float seconds, int packet, int sock);
void canceltimer(int packet, int sock);


/*This is the definition of a message that is passed from the driver to the timer */
typedef struct message {
	int type;
	int p_num;
	float time;
//	int ret_port;
} message_t;


int main(int argc, char* argv[]) {
	printf("%s\n", "The driver is beginning...");

	int sock;
	struct sockaddr_in sin_addr;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    	perror("Error opening socket");
    	exit(1);
    } 

    //socket info stuff for binding

    //bind


	starttimer(20.0,1, sock);
	starttimer(10.0,2, sock);
	starttimer(30.0,3, sock);
	sleep(5);
	canceltimer(2, sock);
	starttimer(20.0,4, sock);
	sleep(5);
	starttimer(18.0,5, sock);
	canceltimer(4, sock);
	canceltimer(8, sock);

	close(sock);

	printf("%s\n", "The driver is ending...");
}


/*This function creates a start timer message and sends it to the timer process */
void starttimer(float seconds, int packet, int sock) {
	message_t send_msg;
	bzero((char*)&send_msg, sizeof(send_msg));
	send_msg.type = 0;
	send_msg.p_num = packet;
	send_msg.time = seconds;

	if(send(sock, &send_msg, sizeof(send_msg), 0) < 0) {
		perror("There was an error sending to the socket in the driver "
			"(starttimer)");
		exit(1);
	}
}


/* This function creates a cancel timer message and sends it to the timer process */
void canceltimer(int packet, int sock) {
	message_t send_msg;
	bzero((char*)&send_msg, sizeof(send_msg));
	send_msg.type = 1;
	send_msg.p_num = packet;
	send_msg.time = 0;

	if(send(sock, &send_msg, sizeof(send_msg), 0) < 0) {
		perror("There was an error sending to the socket in the driver " 
			"(canceltimer)");
		exit(1);
	}
}
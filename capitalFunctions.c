/*
/ Eric Olson and James Baker
/ CSE 5462 Network Programming
/ Lab 3 - September 15, 2016
/ 
/ This file contains our implementations of TCP functions using UDP.
*/
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define LOCALPORT 9999
#define LOCALADDRESS "127.0.0.1"
#define MSS 1000


typedef struct tcpdHeader {
	int flag;
	size_t maxData;
	char body[MSS];
	int bytes_to_read;
} tcpdHeader;

tcpdHeader clientHeader;

/* Prototypes for each function */ 
ssize_t SEND(int socket, const void* buffer, size_t length, int flags);
ssize_t RECV(int socket, void* buffer, size_t length, int flags);
int BIND(int socket, struct sockaddr *my_addr, socklen_t addrlen);
int ACCEPT(int sock, struct sockaddr *address, socklen_t *address_len);
int CONNECT(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen);
int SOCKET(int domain, int type, int protocol);



int ACCEPT(int sock, struct sockaddr *address, socklen_t *address_len) {
	return sock;
}

int CONNECT(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	return 0;
}

int SOCKET(int domain, int type, int protocol) {
	return socket(domain, SOCK_DGRAM, protocol);
}

int BIND(int socket, struct sockaddr *my_addr, socklen_t addrlen) {
        return bind(socket, (struct sockaddr *)my_addr, addrlen);
}

	

ssize_t SEND(int socket, const void* buffer, size_t length, int flags) {
	/* define address of local socket to pass data to */
	struct sockaddr_in daemon_addr;
	daemon_addr.sin_family = AF_INET;
	daemon_addr.sin_port = htons(LOCALPORT);
	daemon_addr.sin_addr.s_addr = INADDR_ANY;


	clientHeader.flag = 1;
	clientHeader.bytes_to_read = length;
	bcopy(buffer,clientHeader.body, length);

	//printf("Body sent: %s\n", clientHeader.body);
	//sleep(0.00001);
	/* pass data to local socket in tcpd */
	ssize_t sent = 0;
	sent = sendto(socket, (char *)&clientHeader, sizeof(clientHeader), flags, 
		(struct sockaddr *)&daemon_addr, sizeof(daemon_addr));
	
	//free(*clientHeader);
	//nanosleep(0.1);
	return sent;
}

ssize_t RECV(int socket, void* buffer, size_t length, int flags) {

	/*int troll_sock;
	if((troll_sock = SOCKET(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Error opening socket");
		exit(1);
	}	
	struct sockaddr_in daemon_addr;
	daemon_addr.sin_family = AF_INET;
	daemon_addr.sin_port = htons(LOCALPORT);
	daemon_addr.sin_addr.s_addr = INADDR_ANY;
	
	tcpdHeader serverHeader;
	serverHeader.flag = 0;
	serverHeader.maxData = length;
	
	sendto(troll_sock, (char *)&serverHeader, sizeof(serverHeader), 0, (struct sockaddr *)&daemon_addr, sizeof(daemon_addr));*/
	

	/* RECEIVE DATA */
	size_t temp;
	temp = recvfrom(socket, buffer, length, 0, NULL, NULL);
	nanosleep(10000);
	return temp;
}

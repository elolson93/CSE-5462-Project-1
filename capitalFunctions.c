/*
/ Eric Olson and James Baker
/ CSE 5462 Network Programming
/ Project 1 - Checkpoint 1 - September 29, 2016
/ 
/ This file contains our implementations of TCP functions using UDP.
*/

#include "header.h"

/* Prototypes for each function */ 
ssize_t SEND(int socket, const void* buffer, size_t length, int flags);
ssize_t RECV(int socket, void* buffer, size_t length, int flags);
int BIND(int socket, struct sockaddr *my_addr, socklen_t addrlen);
int ACCEPT(int sock, struct sockaddr *address, socklen_t *address_len);
int CONNECT(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen);
int SOCKET(int domain, int type, int protocol);

/* Function implmentations */
int ACCEPT(int sock, struct sockaddr *address, socklen_t *address_len) {
	return 0;
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

	/* wrapper for sending data to tcpd */
	tcpdHeader clientHeader;

	/* indicates packet is from client process */
	/* client = 1/server = 0 */
	clientHeader.flag = 1;

	/* Passes along the amount of actual bytes to be read from payload */
	/* IMPORTANT for only for first two and last packet sent */
	clientHeader.bytes_to_read = length;

	/* Copy buffer contents to payload to tcpd */
	bcopy(buffer,clientHeader.body, length);

	/* Send packet to client side tcpd */
	return sendto(socket, (char *)&clientHeader, sizeof(clientHeader), flags, 
		(struct sockaddr *)&daemon_addr, sizeof(daemon_addr));
}

ssize_t RECV(int socket, void* buffer, size_t length, int flags) {

	/* receive data from tcpd */
	return recvfrom(socket, buffer, length, flags, NULL, NULL);
}

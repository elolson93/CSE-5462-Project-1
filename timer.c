/*
/ Eric Olson and James Baker
/ CSE 5462 Project1, Checkpoint 1
/ September 29, 2016
/ This file contains the delta timer description.
*/


#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/select.h>
#include <stdlib.h>
#include <netinet/in.h>
#define WAIT_TIME 1
#define LISTENING_PORT 9090



/* This is the definition for each node within our delta timer linked list */
typedef struct timer_node {
	float time;
	//int port;
	int p_num;
	struct timer_node* next;
	struct timer_node* prev;
} timer_node_t;



/* These are the prototypes for the functions defined within this file */
void delete_node(int packet);
void add_node(float seconds, int packet);
void update_times(timer_node_t* start, int val, char op);
int insert_node(timer_node_t** head, timer_node_t* node);
int remove_node(int del_val, timer_node_t** head);
void print_full_list(timer_node_t* head);
timer_node_t* create_node(float time, int port/*, int p_num*/);
void destroy_node(timer_node_t* node);



/* This pointer points to the head of the delta list */
timer_node_t* head = NULL;



/* This method deallocates the memory held by a node */
void destroy_node(timer_node_t* node) {
	free(node);
}



/* This method creates a new node for our linked list with the supplied criteria.
   It returns a pointer to the newly created node */
timer_node_t* create_node(float time, int port/*, int p_num*/) {
	timer_node_t* node = (timer_node_t*)malloc(sizeof(timer_node_t));
	if (NULL == node) {
		printf("%s\n", "ERROR: Failed to allocate memory for a new timer node");
	} else {
		node->next = NULL;
		node->prev = NULL;
		node->time = time;
		node->p_num = port;
		//node->p_num = p_num;
	}
	return node;
}



/* This method prints out the linked list in its entirety */
void print_full_list(timer_node_t* list) {
	if (NULL == list) {
		printf("%s\n", "The delta timer list is empty");
	} else {
		while(NULL != list) {
			printf("%s%.2f%s%d%s\n", "NODE:: time: ", list->time, " p_num: ", 
				list->p_num," ---> ");
			list = list->next;
		}
	}
}



/* This method updates the time values for each node in the linked list whenever
   an addition to or removal from the list is made */
void update_times(timer_node_t* list, int val, char op) {
	if (NULL == list) {
		return;
	}

	while (NULL != list) {
		if (op == '+') {
			list->time += val; 
		} else if (op == '-') {
			list->time -= val;
		} else {
			printf("%s\n", "ERROR: unrecognized operation.");
			return;
		}
		list = list->next;
	}
}



/* This function inserts the supplied node into the linked list. It returns 1
   for a successful insertion and 0 otherwise */
int insert_node(timer_node_t** head, timer_node_t* node) {
	if (NULL == *head) {
		*head = node;
		return 1;
	} 

	timer_node_t* curr_node = *head;
	timer_node_t* prev_node = *head;
	
	while (NULL != curr_node && node->time > curr_node->time) {
		node->time -= curr_node->time;
		prev_node = curr_node;
		curr_node = curr_node->next;
	}
	
	if (NULL == curr_node) {
		prev_node->next = node;
		node->prev = prev_node;
		return 1;
	} 
	
	if (NULL == prev_node) {
		//new head node
		curr_node->prev = node;
		node->next = curr_node;
		*head = node;
		update_times(node->next, node->time, '-');
		return 1;
	} 
	
	prev_node->next = node;
	curr_node->prev = node;
	node->prev = prev_node;
	node->next = curr_node;
	update_times(node->next, node->time, '-');
	return 1;
}



/* This function removes a node with a particular packet number from the list.
   It returns 1 for a successful removal and 0 otherwise */
int remove_node(int del_val, timer_node_t** head) {
	if (NULL == *head) {
		printf("%s\n", "ERROR: The list is empty. Cannot delete specified node.");
		return 0;
	}

	timer_node_t* trash = NULL;
	if ((*head)->p_num == del_val) {
		trash = *head;
		*head = (*head)->next;
		(*head)->prev = NULL;
		update_times(*head, trash->time, '+');
		destroy_node(trash);
		return 1;
	}

	timer_node_t* tracker = *head;
	while (NULL != tracker && tracker->p_num != del_val) {
		tracker = tracker->next;
	}

	if (NULL == tracker) {
		printf("%s%d%s\n", "ERROR: The node with packet number: ", del_val,  
			" was never found. Failed to delete.");
		return 0;
	}

	trash = tracker;
	(tracker->prev)->next = tracker->next;
	if (NULL != tracker->next) {
		(tracker->next)->prev = tracker->prev;
	}
	update_times(tracker->next, tracker->time, '+');
	destroy_node(trash);
	return 1;
}



/* This function is meant as a wrapper around inser_node(). It contains 
   additional print statements for work flow traking */
void add_node(float seconds, int packet) {
	timer_node_t* node = create_node(seconds, packet);
	int check = insert_node(&head, node);

	if (check == 0) {
		printf("%s%.2f%s%d\n", "There was an issue adding the node with " 
			"seconds: ", seconds, " and packet number: ", packet);
	} else {
		printf("%s%.2f%s%d%s\n", "Node with seconds: ", seconds, " and packet "
			"number: ", packet, " added successfully");
	}

	printf("%s\n\n", "State of the delta timer:");
	print_full_list(head);
	printf("\n\n");
}



/* This function is meant as a wrapper around remove_node(). It contains
   additional print statements for work flow tracking */
void delete_node(int packet) {
	int check = remove_node(packet, &head);

	if (check == 0) {
		printf("%s%d%s\n", "There was an issue deleting the node with " 
			"packet number: ", packet, ". See above message.");
	} else {
		printf("%s%d%s\n", "Node with packet number: ", packet, " added "
			"successfully");	
	}

	printf("%s\n\n", "State of the delta timer:");
	print_full_list(head);
	printf("\n\n");

}

int main(int argc, char *argv[]) {
	printf("Timer process booting up...\n\n");

	int sock;
	struct sockaddr_in sin_addr;
	char read_buf[1000] = {0};

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    	perror("Error opening socket");
    	exit(1);
    }

    sin_addr.sin_family = AF_INET;
    sin_addr.sin_port = htons(LISTENING_PORT);
    sin_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
      	perror("Error binding stream socket");
      	exit(1);
    }

    fd_set read_set;
	clock_t start;
	float elapsed_time = 0.0;
	struct timeval wait_timer;
	wait_timer.tv_sec = WAIT_TIME;
	wait_timer.tv_usec = 0;

	for(;;) {
		FD_ZERO(&read_set);
    	FD_SET(sock, &read_set);
		start = clock();
		if (select(FD_SETSIZE, &read_set, NULL, NULL, &wait_timer) < 0) {
			fprintf(stderr, "%s\n", "There was an issue with select()");
			exit(1);
		}
		/* Should I do this before or after the FD_ISSET check? */
		elapsed_time = (float) (clock() - start)/CLOCKS_PER_SEC;
		if(NULL != head) {
			head->time -= elapsed_time;
			if (head->time <= 0) {
				printf("%s%d%s\n", "Packet number: ", head->p_num, " has timed out.");
				remove_node(head->p_num, &head);
				printf("%s\n", "Status of the delta timer list:");
				print_full_list(head);
				printf("\n\n");
			}
		}
			
		if(FD_ISSET(sock, &read_set)) {
				
				//check received packet
				//do apprpriate action

		} 
	}//end for

	return 0;
}
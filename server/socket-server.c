#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "struct/pqueue.h"

#define BUFFER 2000

typedef struct Player Player;
struct Player {
	int id;
	int score;
};
void print_player(void* data)
{
    Player* player = (Player*) data;
    fprintf(stderr, "Player{id=%d, score=%d}", player->id, player->score);
}

typedef struct Target Target;
struct Target {
	int id;
	int positionx;
	int positiony;
	int type;
	int isAlive;
};

// global variables
int id_counter = 1;

// main resources
pqueue* players = NULL;
pqueue* communicates = NULL;

void*
connection_handler(void* socket_desc) {

	/* Get the socket descriptor */
	int sock = *(int*)socket_desc;
	int read_size;
	char* message, client_message[BUFFER];

	/* Send some messages to the client */
	message = "Greetings! I am your connection handler\n";
	write(sock, message, strlen(message));

	message = "Now type something and i shall repeat what you type\n";
	write(sock, message, strlen(message));

	message = "Empty line will close the connection\n";
	write(sock, message, strlen(message));

	do {
		read_size = recv(sock, client_message, BUFFER, 0);
		client_message[read_size] = '\0';

		/* Send the message back to client */
		write(sock, client_message, strlen(client_message));

		/* Clear the message buffer */
		memset(client_message, 0, BUFFER);
	} while (read_size > 2); /* Wait for empty line */

	fprintf(stderr, "Client disconnected\n");

	close(sock);
	pthread_exit(NULL);
}

int
main(int argc, char* argv[]) {
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;

	pthread_t thread_id;

	// initialize structs
	pqueue* players = NULL;
	pqueue* targets = NULL;

	Player* player = (Player*)malloc(sizeof(Player));
	player->id = 1;
	player->score = 300;
	qinsert(&players, player, 1);
	qlist(players, print_player);

    // create socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

    // configure connection
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(5000);

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	listen(listenfd, 10);

	for (;;) {
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
		fprintf(stderr, "Connection accepted\n");
		pthread_create(&thread_id, NULL, connection_handler, (void*)&connfd);
	}
}

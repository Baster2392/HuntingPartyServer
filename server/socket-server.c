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
unsigned int id_counter = 256;

// main resources
pqueue* players = NULL;
pqueue* communicates = NULL;

void*
connection_handler(void* socket_desc) {

	/* Get the socket descriptor */
	int sock = *(int*)socket_desc;
	int read_size;
	char* message, client_message[BUFFER];

	/* Create new player */
	Player* player = (Player*)malloc(sizeof(Player));
	player->id = id_counter++;
	player->score = 0;
	qinsert(&players, player, player->id);
	fprintf(stderr, "Player %d joined.\n", player->id);

	/* Send client id to client */
	send(sock, &(player->id), sizeof(player->id), 0);

	do {
		read_size = recv(sock, client_message, BUFFER, 0);
		client_message[read_size] = '\0';
        fprintf(stderr, "%s\n", client_message);

		if (strcmp(client_message, "ready") == 0)
		{
		    fprintf(stderr, "Player %d ready.\n", player->id);
		}
		else
		{
		    fprintf(stderr, "Player %d not ready.\n", player->id);
		}

		send(sock, &(player->id), sizeof(player->id), 0);

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

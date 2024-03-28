#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "struct/pqueue.h"
#include "struct/list.h"

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
	int position_x;
	int position_y;
	int type;
	int isAlive;
};

// global variables
unsigned int id_counter = 1;
unsigned int player_counter = 0;

// main resources
List* players = NULL;
List* communicates = NULL;
List* threads = NULL;

void*
connection_handler(void* socket_desc) {

	/* Get the socket descriptor */
	int sock = *(int*)socket_desc;
	int read_size;
	char* message, client_message[BUFFER];

	/* Create new player */
	Player* player = (Player*)malloc(sizeof(Player));
	player->id = (id_counter++);
	player->score = 0;

    players = insertAtEnd(players, player, player->id);
	fprintf(stderr, "Player %d joined.\n", player->id);
    player_counter++;
    display(players, print_player);

	/* Send client id to client */
	send(sock, &(player->id), sizeof(int), 0);
    read_size = recv(sock, client_message, BUFFER, 0);
    client_message[read_size] = '\0';
    if (strcmp(client_message, "received_id") != 0)
    {
        fprintf(stderr, "Communication error.");
        break;
    }

    /* Send player number */
    send(sock, &player_counter, sizeof(int), 0);

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

		/* Clear the message buffer */
		memset(client_message, 0, BUFFER);
	} while (1 == 1); /* Wait for empty line */

    // Disconnect player
	fprintf(stderr, "Client disconnected\n");
    deleteByIndex(&players, player->id);
    player_counter--;

    // Delete thread from threads
    deleteById(&threads, (int)pthread_self());
	close(sock);
	pthread_exit(NULL);
}

int
main(int argc, char* argv[]) {
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;

	pthread_t thread_id;

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
        // save thread
        //                          NULL!!!          our data
        insertAtEnd(threads, NULL, thread_id);  // we store data in node_id parameter !!!!!!
        // to access thread id we call getIdByIndex
	}
}

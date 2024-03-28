#include <sys/socket.h>
#include <netinet/in.h>
//#include <arpa/inet.h>
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
unsigned int ready_counter = 0;
int isGameStarted = 0;

// main resources
List* players = NULL;
List* communicates = NULL;
List* threads = NULL;
List* socks = NULL;

void* connection_handler(void* socket_desc) {

	/* Get the socket descriptor */
	int sock = *(int*)socket_desc;
	int read_size;
	char client_message[BUFFER];
    memset(client_message, 0, BUFFER);

    int thread_id = id_counter++;   // different from in threads
    insertAtEnd(socks, &sock, thread_id);

	/* Create new player */
	Player* player = (Player*)malloc(sizeof(Player));
	player->id = thread_id;
	player->score = 0;

    players = insertAtEnd(players, player, player->id);
	fprintf(stderr, "Player %d joined.\n", player->id);
    player_counter++;
    display(players, print_player);

	/* Send client_id to client */
	send(sock, &(player->id), sizeof(int), 0);
    // get confirmation
    read_size = recv(sock, client_message, BUFFER, 0);
    client_message[read_size] = '\0';
    if (strcmp(client_message, "received_id") != 0)
    {
        fprintf(stderr, "Communication error.\n");
        exit(0);
    }
    memset(client_message, 0, BUFFER);  // clean buffer

    /* Wait for "ready" status */
    read_size = recv(sock, client_message, BUFFER, 0);
    client_message[read_size] = '\0';

    if (strcmp(client_message, "ready") == 0)
    {
        memset(client_message, 0, BUFFER);
        ready_counter += 1;
        fprintf(stderr, "Player %d ready.\n", player->id);

        if (ready_counter == player_counter && player_counter > 1)
        {
            fprintf(stderr, "Game staring...");
            /* Send "starting game" status */
            strcpy(client_message, "starting_game\0");
            send(sock, &client_message, strlen(client_message), 0);
            memset(client_message, 0, BUFFER);
            /* We can call startGame() from player's thread, because only one player
             * will meet condition (ready_counter == player_counter && player_counter > 1) */
            // TODO: Call start of game
        }
    } else
    {
        fprintf(stderr, "Communication error.\n");
        exit(0);
    }
    memset(client_message, 0, BUFFER);

    // Disconnect player
	fprintf(stderr, "Client disconnected\n");
    deleteById(&players, player->id);
    player_counter--;
    ready_counter--;

    // Delete thread from threads
    deleteById(&threads, (int)pthread_self());
	close(sock);
	pthread_exit(NULL);
}

int main() {
	int listenfd, connfd = 0;
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
        threads = insertAtEnd(threads, NULL, thread_id);
        // to access thread id we call getIdByIndex
	}
}

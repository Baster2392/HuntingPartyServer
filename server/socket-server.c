#include <sys/socket.h>
#include <netinet/in.h>
//#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "struct/list.h"
#include "struct/structs.h"

#define BUFFER 2000

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
    Message* message = createNewEmptyMessage();

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
    setContent(message, itos(player->id));
	send(sock, message, sizeof(Message), 0);

    /* Wait for "ready" status */
    recv(sock, message, sizeof(Message), 0);
    // TODO: check if controlSum has correct value

    if (strcmp(message->content, "ready") == 0)
    {
        ready_counter += 1;
        fprintf(stderr, "Player %d ready.\n", player->id);

        if (ready_counter == player_counter && player_counter > 1)
        {
            fprintf(stderr, "Game staring...\n");
            /* Send "starting game" status */
            setContent(message, "starting_game");
            send(sock, message, sizeof(Message), 0);

            /* We can call startGame() from player's thread, because only one player
             * will meet condition (ready_counter == player_counter && player_counter > 1) */
            // TODO: Call start of game
        }
    } else
    {
        fprintf(stderr, "Communication error.\n");
        exit(0);
    }

    // Waiting for other players to join
    while (!isGameStarted)
    {
        /* TODO: send number of players if players are not ready
         * or "starting_game" communicate if players are ready
         * and number of players is bigger than 1*/
    }

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
        // to access thread_id we call getIdByIndex
	}
}

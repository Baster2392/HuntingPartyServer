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
#include "struct/message.h"

#define GAME_TIME 180 // in seconds
#define BUFFER 2000
#define TRUE 1
#define FALSE 0

// global variables
unsigned int id_counter = 1;
unsigned int player_counter = 0;
unsigned int ready_counter = 0;
int isGameStarted = FALSE;

// Mutex is used to synchronise threads and
// secure shared resources
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// main resources
List* players = NULL;
List* targets = NULL;
List* threads = NULL;
List* socks = NULL;

/* game() is running on other threads
  than main and connection handlers */
void* game()
{
    // wait for players
    while (player_counter != ready_counter || player_counter < 2);
    // TODO: send "starting_game_communicate" to all players

    isGameStarted = TRUE;
    fprintf(stderr, "Starting game\n");
    // game loop
    //while (TRUE)
    {
        // TODO: End loop after GAME_TIME
        /* In this loop we will be sending generated
         * targets etc. using socks list */
        sleep(4);
    }
    isGameStarted = FALSE;
    fprintf(stderr, "Game ended\n");

    pthread_exit(NULL);
}

void* connection_handler(void* socket_desc) {
	/* Get the socket descriptor */
	int sock = *(int*)socket_desc;
    Message* message;

    int thread_id = id_counter++;   // different from in threads
    insertAtEnd(socks, &sock, thread_id);

	/* Create new player */
	Player* player = (Player*)malloc(sizeof(Player));
	player->id = thread_id;
	player->score = 0;
    player->isInGame = FALSE;

    pthread_mutex_lock(&mutex);
    players = insertAtEnd(players, player, player->id);
	fprintf(stderr, "Player %d joined.\n", player->id);
    player_counter++;
    display(players, print_player);
    pthread_mutex_unlock(&mutex);

	/* Send client_id to client */
    sendMessage(sock, itos(player->id));

    /* Wait for "ready" status */
    message = receiveMessage(sock);

    if (strcmp(message->content, "ready") == 0)
    {
        ready_counter += 1;
        player->isInGame = TRUE;
        fprintf(stderr, "Player %d ready.\n", player->id);
    } else
    {
        fprintf(stderr, "Communication error.\n");
        free(message);
        exit(0);
    }
    free(message);

    // Waiting for other players to join
    while (!isGameStarted)
    {
        // TODO: send number of players if players are not ready
    }

    // Waiting for game to end
    while (isGameStarted)
    {
        // TODO: In this loop we will be receiving and checking of shoot is accurate
    }

    // TODO: send communicate "game_ended" to client and scores

    // Disconnect player
    pthread_mutex_lock(&mutex);
	fprintf(stderr, "Client disconnected\n");
    deleteById(&players, player->id);
    deleteById(&socks, sock);
    player_counter--;
    ready_counter--;

    // Delete thread from threads
    deleteById(&threads, (int)pthread_self());
    pthread_mutex_unlock(&mutex);

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

    // create game thread
    pthread_t game_thread_id;
    pthread_create(&game_thread_id, NULL, game, NULL);

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

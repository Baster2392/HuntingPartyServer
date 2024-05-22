#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "struct/list.h"
#include "struct/structs.h"
#include "struct/message.h"

#define PORT 5000
#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 2000
#define GAME_TIME 40 // in seconds

// Global variables
unsigned int id_counter = 1;
unsigned int player_counter = 0;
unsigned int ready_counter = 0;
bool isGameStarted = false;
time_t gameStartTime;
time_t gameEndTime;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

List* players = NULL;
List* targets = NULL;
List* threads = NULL;
List* socks = NULL;

// Function prototypes
void freeTargets(List* list);
void print_target(void* data);
Player* findById(List* list, int id);
void* game(void* arg);
void* connection_handler(void* socket_desc);

int main() {
    int listenfd, connfd = 0;
    struct sockaddr_in serv_addr;
    pthread_t game_thread_id, thread_id;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    listen(listenfd, MAX_CONNECTIONS);

    pthread_create(&game_thread_id, NULL, game, NULL);

    while (true) {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        if (connfd < 0) {
            perror("Error accepting connection");
            continue;
        }
        printf("Connection accepted\n");
        int *new_sock = malloc(sizeof(int));
        *new_sock = connfd;
        pthread_create(&thread_id, NULL, connection_handler, (void*)new_sock);
        threads = insertAtEnd(threads, NULL, thread_id);
    }

    return 0;
}

void* game(void* arg) {
    while (player_counter != ready_counter || player_counter < 1);

    pthread_mutex_lock(&mutex);
    isGameStarted = true;
    fprintf(stderr, "Starting game\n");

    gameStartTime = time(NULL);
    gameEndTime = gameStartTime + GAME_TIME;

    // Broadcast starting message to all players
    List* current = socks;
    while (current != NULL) {
        int sock = *((int*)current->data);
        sendMessage(sock, "starting_game");
        current = current->next;
    }

    pthread_mutex_unlock(&mutex);

    while (time(NULL) < gameEndTime) {
        // Random target generation
        Target* target = (Target*)malloc(sizeof(Target));
        target->id = rand() % 100;
        target->position_x = rand() % 5000;
        target->position_y = rand() % 836;
        target->type = rand() % 3;
        target->isAlive = true;

        pthread_mutex_lock(&mutex);
        targets = insertAtEnd(targets, target, target->id);
        pthread_mutex_unlock(&mutex);

        fprintf(stderr, "Targets List:\n");
        display(targets, print_target);

        current = socks;
        while (current != NULL) {
            int sock = *((int*)current->data);
            sendMessage(sock, "target");
            sendTarget(sock, target);
            current = current->next;
        }
        // Interval between new targets
        usleep(5000000);
    }

    freeTargets(targets);  // Free memory for all targets
    targets = NULL;        // Reset the targets list

    isGameStarted = false;
    fprintf(stderr, "Game ended\n");

    pthread_exit(NULL);
}

void* connection_handler(void* socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc);
    Message* message;

    pthread_mutex_lock(&mutex);
    int thread_id = id_counter++;
    pthread_mutex_unlock(&mutex);

    int* sockPtr = malloc(sizeof(int));
    if (sockPtr == NULL) {
        perror("Error allocating memory");
        close(sock);
        pthread_exit(NULL);
    }
    *sockPtr = sock;

    pthread_mutex_lock(&mutex);
    socks = insertAtEnd(socks, sockPtr, thread_id);
    pthread_mutex_unlock(&mutex);

    Player* player = (Player*)malloc(sizeof(Player));
    if (player == NULL) {
        perror("Error allocating memory");
        close(sock);
        free(sockPtr);
        pthread_exit(NULL);
    }
    player->id = thread_id;
    player->score = 0;
    player->isInGame = false;

    pthread_mutex_lock(&mutex);
    players = insertAtEnd(players, player, player->id);
    fprintf(stderr, "Player %d joined.\n", player->id);
    player_counter++;
    display(players, print_player);
    pthread_mutex_unlock(&mutex);

    sendMessage(sock, itos(player->id));

    message = receiveMessage(sock);
    if (message == NULL) {
        perror("Error receiving message");
        close(sock);
        pthread_exit(NULL);
    }

    if (strcmp(message->content, "ready") == 0) {
        ready_counter++;
        player->isInGame = true;
        fprintf(stderr, "Player %d ready.\n", player->id);
    }
    else {
        fprintf(stderr, "Communication error.\n");
        free(message);
        close(sock);
        pthread_exit(NULL);
    }
    free(message);

    while (!isGameStarted) {
        // Wait for game to start
    }

    while (isGameStarted) {
        Shot* shot = receiveShot(sock);
        if (shot != NULL) {
            pthread_mutex_lock(&mutex);
            List* current_target = targets;
            pthread_mutex_unlock(&mutex);

            while (current_target != NULL) {
                Target* target = (Target*)current_target->data;

                if (shot->x == target->position_x && shot->y == target->position_y && target->isAlive) {
                    target->isAlive = false;

                    pthread_mutex_lock(&mutex);
                    List* current_sock = socks;
                    while (current_sock != NULL) {
                        int sock = *((int*)current_sock->data);
                        sendMessage(sock, "destroy");
                        sendTarget(sock, target);
                        current_sock = current_sock->next;
                    }
                    pthread_mutex_unlock(&mutex);
                    break;
                }
                current_target = current_target->next;
            }
        }
        free(shot);
    }

    sendMessage(sock, "end_game");

    pthread_mutex_lock(&mutex);
    fprintf(stderr, "Client disconnected\n");
    deleteById(&players, player->id);
    deleteById(&socks, sock);
    player_counter--;
    ready_counter--;
    deleteById(&threads, (int)pthread_self());
    pthread_mutex_unlock(&mutex);

    close(sock);
    pthread_exit(NULL);
}

void freeTargets(List* list) {
    List* current = list;
    while (current != NULL) {
        Target* target = (Target*)current->data;
        free(target);
        current = current->next;
    }
    freeList(list);
}

void print_target(void* data) {
    Target* target = (Target*)data;
    printf("Target ID: %d, Position: (%d, %d), Type: %d, Alive: %d\n",
        target->id, target->position_x, target->position_y, target->type, target->isAlive);
}

Player* findById(List* list, int id) {
    List* current = list;
    while (current != NULL) {
        Player* player = (Player*)current->data;
        if (player->id == id) {
            return player;
        }
        current = current->next;
    }
    return NULL;
}

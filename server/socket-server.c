#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "struct/list.h"
#include "struct/structs.h"
#include "struct/message.h"

#define PORT 5000
#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 2000
#define GAME_TIME 60
#define MAP_WIDTH 5464
#define MAP_HEIGHT 1080
#define TARGET_WIDTH 214
#define TARGET_HEIGHT 244

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

void freeTargets(List* list);
void print_target(void* data);
Player* findById(List* list, int id);
void* game(void* arg);
void* connection_handler(void* socket_desc);
List* insertSorted(List* list, void* data, int score);
void calculateTargetPosition(Target* target, time_t currentTime);
void sendLeaderboard(int sock,List* leaderboard);
int length(List* list);
void displayLeaderboard(List* leaderboard);


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
    //serv_addr.sin_addr.s_addr = inet_addr("192.168.56.1");

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
    while (true) {
        pthread_mutex_lock(&mutex);
        if (player_counter == ready_counter && player_counter > 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
        usleep(100000);
    }
    pthread_mutex_lock(&mutex);
    isGameStarted = true;
    fprintf(stderr, "Starting game\n");
    printf("\nReady status: %d of %d\n",ready_counter,player_counter);
    gameStartTime = time(NULL);
    gameEndTime = gameStartTime + GAME_TIME;

    List* current = socks;
    while (current != NULL) {
        int sock = *((int*)current->data);
        sendMessage(sock, "starting_game");
        current = current->next;
    }

    pthread_mutex_unlock(&mutex);

    while (time(NULL) < gameEndTime) {
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

        List* leaderboard = NULL;
        current = players;
        while (current != NULL) {
            Player* player = (Player*)current->data;
            leaderboard = insertSorted(leaderboard, player, player->score);
            current = current->next;
        }

        current = socks;
        displayLeaderboard(leaderboard);

        while (current != NULL) {
            int sock = *((int*)current->data);
            sendMessage(sock, "leaderboard");
            sendLeaderboard(sock, leaderboard);
            current = current->next;
        }

        usleep(5000000 / player_counter);
    }

    freeTargets(targets);
    targets = NULL;

    List* leaderboard = NULL;
    current = players;
    while (current != NULL) {
        Player* player = (Player*)current->data;
        leaderboard = insertSorted(leaderboard, player, player->score);
        current = current->next;
    }

    current = socks;
    while (current != NULL) {
        int sock = *((int*)current->data);
        sendMessage(sock, "final_leaderboard");
        sendLeaderboard(sock, leaderboard);
        current = current->next;
    }

    fprintf(stderr, "Leaderboard:\n");
    display(leaderboard, print_player);

    if (leaderboard != NULL) {
        Player* winner = (Player*)leaderboard->data;
        fprintf(stderr, "Winner: Player %d with score %d\n", winner->id, winner->score);
    }

    freeList(leaderboard);

    isGameStarted = false;
    fprintf(stderr, "Game ended\n");

    pthread_exit(NULL);
}

int calculate_checksum(const char* data, int length) {
    int checksum = 0;
    for (int i = 0; i < length; i++) {
        checksum += data[i];
    }
    return checksum%256;
}

void sendLeaderboard(int sock, List* leaderboard) {
    int num_entries = length(leaderboard);

    char leaderboard_message[256];
    memset(leaderboard_message, 0, sizeof(leaderboard_message));

    List* current = leaderboard;
    while (current != NULL) {
        Player* player = (Player*)current->data;
        char leaderboard_entry[64];
        snprintf(leaderboard_entry, sizeof(leaderboard_entry), "Player %d: %d\n", player->id, player->score);
        strcat(leaderboard_message, leaderboard_entry);
        current = current->next;
    }

    int message_length = strlen(leaderboard_message) + 1;
    int checksum = calculate_checksum(leaderboard_message, message_length);


    char buffer[260];
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, leaderboard_message, message_length);
    memcpy(buffer + 256, &checksum, sizeof(int));

    send(sock, buffer, message_length + sizeof(int), 0);
}
int length(List* list) {
    int len = 0;
    List* current = list;
    while (current != NULL) {
        len++;
        current = current->next;
    }
    return len;
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

    pthread_mutex_lock(&mutex);
    if (strcmp(message->content, "ready") == 0) {
        ready_counter++;
        player->isInGame = true;
        fprintf(stderr, "Player %d ready.\n", player->id);
    } else {
        fprintf(stderr, "Communication error.\n");
        pthread_mutex_unlock(&mutex);
        free(message);
        close(sock);
        pthread_exit(NULL);
    }
    pthread_mutex_unlock(&mutex);
    free(message);

    while (!isGameStarted) {
    }

    while (isGameStarted) {
        Shot* shot = receiveShot(sock);
        if (shot != NULL) {
            pthread_mutex_lock(&mutex);
            List* current_target = targets;
            pthread_mutex_unlock(&mutex);

            while (current_target != NULL) {
                Target* target = (Target*)current_target->data;
                time_t current_time = time(NULL);
                pthread_mutex_lock(&mutex);

                calculateTargetPosition(target, current_time);
                target->id, target->position_x, target->position_y, target->type, target->isAlive);

                printf("Shot Position: (%d, %d)\n",
                shot->x, shot->y);
                pthread_mutex_unlock(&mutex);

                if (shot->x >= target->position_x - TARGET_WIDTH && shot->x <= target->position_x + TARGET_WIDTH &&
                    shot->y >= target->position_y - TARGET_HEIGHT && shot->y <= target->position_y + TARGET_HEIGHT && target->isAlive) {
                    target->isAlive = false;

                    Player* shooter = findById(players, player->id);
                    if (shooter != NULL) {
                        shooter->score++;
                        printf("Player %d scored! New score: %d\n", shooter->id, shooter->score);
                    }

                    List* current_sock = socks;
                    while (current_sock != NULL) {
                        int sock = *((int*)current_sock->data);
                        sendMessage(sock, "destroy");
                        sendTarget(sock, target);
                        current_sock = current_sock->next;
                    }
                    deleteById(&targets, target->id);
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
List* insertSorted(List* list, void* data, int score) {
    List* new_node = (List*)malloc(sizeof(List));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    new_node->data = data;
    new_node->next = NULL;

    if (list == NULL || score > ((Player*)(list->data))->score) {
        new_node->next = list;
        return new_node;
    }

    List* current = list;
    while (current->next != NULL && score <= ((Player*)(current->next->data))->score) {
        current = current->next;
    }

    new_node->next = current->next;
    current->next = new_node;

    return list;
}

void calculateTargetPosition(Target* target, time_t currentTime) {
    double elapsedTime = difftime(currentTime, gameStartTime);
    if (target->type == 0){
        target->position_y = MAP_HEIGHT / 2 + 100 * cos(elapsedTime + target->id);
        target->position_x = (int)(100 * elapsedTime + target->id * 50) % MAP_WIDTH;
    }

    else if (target->type == 1){
        int radius = 250;
        int center_x = (int)(100 * elapsedTime + target->id * 50) % MAP_WIDTH;
        int center_y = MAP_HEIGHT/2;
        target->position_x = center_x + radius * cos(elapsedTime + target->id);
        target->position_y = center_y + radius * sin(elapsedTime + target->id);
    }

    else {
        target->position_y = MAP_HEIGHT / 2 + 100 * cos(elapsedTime + target->id);
        target->position_x = (int)(100 * (60-elapsedTime) + target->id * 50) % MAP_WIDTH;
    }
    if (target->position_x > MAP_WIDTH) {
        target->position_x -= MAP_WIDTH;
    }
    if (target->position_y > MAP_HEIGHT) {
        target->position_y -= MAP_HEIGHT;
    }

    if (target->position_x < 0) {
        target->position_x += MAP_WIDTH;
    }
    if (target->position_y < 0) {
        target->position_y += MAP_HEIGHT;
    }

    printf("Calculated Target ID: %d, New Position: (%d, %d)\n", target->id, target->position_x, target->position_y);

}

void displayLeaderboard(List* leaderboard) {
    List* current = leaderboard;
    printf("Leaderboard:\n");
    while (current != NULL) {
        Player* player = (Player*)current->data;
        printf("Player %d: %d\n", player->id, player->score);
        current = current->next;
    }
}


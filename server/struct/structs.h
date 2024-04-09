#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTENT_SIZE 256

typedef struct Player Player;
struct Player {
    int id;
    int score;
    int isInGame;
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




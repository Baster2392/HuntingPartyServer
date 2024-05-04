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
    unsigned int control_sum;
};

unsigned int calculateTargetControlSum(Target* target)
{
    unsigned int crc = 0;
    crc += target->id;
    crc += target->position_x;
    crc += target->position_y;
    crc += target->type;
    crc += target->isAlive;
    return crc % 256;
}

void setTarget(Target* target, int id, int position_x, int position_y, int type, int isAlive)
{
    target->id = id;
    target->position_x = position_x;
    target->position_y = position_y;
    target->type = type;
    target->isAlive = isAlive;
    target->control_sum = calculateTargetControlSum(target);
}

void sendTarget(int sock, Target* target)
{
    send(sock, target, sizeof(Target), 0);
}

Target* receiveTarget(int sock)
{
    Target* target = (Target*)malloc(sizeof(Target));
    recv(sock, target, sizeof(Target), 0);

    if (target->control_sum != calculateTargetControlSum(target))
    {
        fprintf(stderr, "Target data corrupted\n");
        free(target);
        return NULL;
    }

    return target;
}




typedef struct Shot Shot;
struct Shot {
    int x;
    int y;
    unsigned int control_sum;
};

unsigned int calculateShotControlSum(Shot* shot)
{
    unsigned int crc = 0;
    crc += shot->x;
    crc += shot->y;
    return crc % 256;
}


void setShot(Shot* shot, int x, int y)
{
    shot->x = x;
    shot->y = y;
    shot->control_sum = calculateShotControlSum(shot);
}

void sendShot(int sock, Shot* shot)
{
    send(sock, shot, sizeof(Shot), 0);
}

Shot* receiveShot(int sock)
{
    Shot* shot = (Shot*)malloc(sizeof(Shot));
    recv(sock, shot, sizeof(Shot), 0);

    if (shot->control_sum != calculateShotControlSum(shot))
    {
        free(shot);
        return NULL;
    }

    return shot;
}



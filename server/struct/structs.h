#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTENT_SIZE 256

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

typedef struct Message Message;
struct Message {
    char content[CONTENT_SIZE];
    unsigned int control_sum;
};

Message* getNewEmptyMessage()
{
    Message* newMessage = (Message*)malloc(sizeof(Message));
    newMessage->content[0] = '\0';
    newMessage->control_sum = 0;
    return newMessage;
}

unsigned int calculateControlSum(char* content)
{
    unsigned int crc = 0;
    for (int i = 0; i < strlen(content); i++)
    {
        crc += content[i];
    }

    return crc % 256;
}

void setContent(Message* message, char* content)
{
    int i = 0;
    for (; i < strlen(content); i++)
    {
        message->content[i] = content[i];
    }

    i++;
    message->content[i] = '\0';
    message->control_sum = calculateControlSum(message->content);
}



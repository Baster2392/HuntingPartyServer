#include <sys/socket.h>
#include <netinet/in.h>
//#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define CONTENT_SIZE 256

typedef struct Message Message;
struct Message {
    char content[CONTENT_SIZE];
    unsigned int control_sum;
};

Message* createNewEmptyMessage()
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
    //fprintf(stderr, "Content: %s, Sum: %d\n", message->content, message->control_sum);
}

char* itos(int number) {
    static char str[20];
    snprintf(str, sizeof(str), "%d", number);
    return str;
}

void sendMessage(int sock, char* content)
{
    Message* message = createNewEmptyMessage();
    setContent(message, content);
    send(sock, message, sizeof(Message), 0);
    free(message);
}

Message * receiveMessage(int sock)
{
    Message* message = createNewEmptyMessage();
    recv(sock, message, sizeof(Message), 0);

    // check if message is correct
    if (message->control_sum != calculateControlSum(message->content))
    {
        fprintf(stderr, "Content lost\n");
        free(message);
        return NULL;
    }

    return message;
}
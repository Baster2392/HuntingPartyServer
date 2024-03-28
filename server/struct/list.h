#include <stdio.h>
#include <stdlib.h>

typedef struct List {
    int id;
    void* data;
    struct List* next;
} List;

List* createNode(void* data, int node_id) {
    List* newNode = (List*)malloc(sizeof(List));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    newNode->id = node_id;
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

List* insertAtBeginning(List* head, void* data, int node_id) {
    List* newNode = createNode(data, node_id);
    newNode->next = head;
    return newNode;
}

List* insertAtEnd(List* head, void* data, int node_id) {
    List* newNode = createNode(data, node_id);
    if (head == NULL)
        return newNode;

    List* temp = head;
    while (temp->next != NULL)
        temp = temp->next;
    temp->next = newNode;
    return head;
}

void display(List * head, void (*displayFunc)(void*)) {
    List * temp = head;
    while (temp != NULL) {
        fprintf(stderr, "Id: %d ", temp->id);
        displayFunc(temp->data);
        temp = temp->next;
    }
    fprintf(stderr,"\n");
}

void* getDataByIndex(List* head, int index) {
    List* temp = head;
    int currentIndex = 0;
    while (temp != NULL) {
        if (currentIndex == index)
            return temp->data;
        temp = temp->next;
        currentIndex++;
    }
    fprintf(stderr,"Index out of range\n");
    return NULL;
}

int getIdByIndex(List* head, int index)
{
    List* temp = head;
    int currentIndex = 0;
    while (temp != NULL) {
        if (currentIndex == index)
            return temp->id;
        temp = temp->next;
        currentIndex++;
    }
    fprintf(stderr,"Index out of range\n");
    return -1;
}

void deleteByData(List** head, void* data) {
    List* current = *head;
    List* previous = NULL;

    while (current != NULL) {
        if (current->data == data) {
            if (previous == NULL) {
                *head = current->next;
            } else {
                previous->next = current->next;
            }
            free(current->data);
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void deleteById(List** head, int id) {
    List* current = *head;
    List* previous = NULL;

    while (current != NULL) {
        if (current->id == id) {
            if (previous == NULL) {
                *head = current->next;
            } else {
                previous->next = current->next;
            }
            free(current->data);
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void deleteByIndex(List** head, int index) {
    List* current = *head;
    List* previous = NULL;
    int currentIndex = 0;

    while (current != NULL) {
        if (currentIndex == index) {
            if (previous == NULL) {
                *head = current->next;
            } else {
                previous->next = current->next;
            }
            free(current->data);
            free(current);
            return;
        }
        previous = current;
        current = current->next;
        currentIndex++;
    }

    fprintf(stderr, "Index out of range\n");
}

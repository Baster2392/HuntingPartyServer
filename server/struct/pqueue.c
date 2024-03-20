#include <stdio.h>
#include <stdlib.h>
#include "pqueue.h"

void
qlist(pqueue *head, void (*print_data)(void *)) {
	pqueue *p;
	
	for (p = head; p != NULL; p = p->next) {
		printf("%d: ", p->k);
		print_data(p->data);
		printf("\n");
	}
	
}

void
qinsert(pqueue **phead, void *data, int k) {
	if (*phead == NULL)
	{
		*phead = (pqueue*)malloc(sizeof(pqueue));
		(*phead)->data = data;
		(*phead)->k = k;
		(*phead)->prev = NULL;
		(*phead)->next = NULL;
		return;
	}

	if ((*phead)->k < k)
	{
		insertBefore(*phead, data, k);
		*phead = (*phead)->prev;
		return;
	}

	pqueue* currentNode = *phead;
	while (currentNode->next != NULL)
	{
		if (currentNode->k < k)
		{
			insertBefore(currentNode, data, k);
			return;
		}
		else
		{
			currentNode = currentNode->next;
		}
	}

	insertAtEnd(currentNode, data, k);
}

void
insertBefore(pqueue* node, void* data, int k)
{
	pqueue* newNode = (pqueue*)malloc(sizeof(pqueue));
	newNode->data = data;
	newNode->k = k;

	newNode->next = node;
	newNode->prev = node->prev;
	node->prev = newNode;
}

void
insertAtEnd(pqueue* lastNode, void* data, int k)
{
	pqueue* newNode = (pqueue*)malloc(sizeof(pqueue));
	newNode->data = data;
	newNode->k = k;

	newNode->prev = lastNode;
	newNode->next = NULL;
	lastNode->next = newNode;
}


void
qremove(pqueue **phead, int k)
{
	if (*phead == NULL)
	{
		return;
	}

	if ((*phead)->k == k)
	{
		pqueue* next = (*phead)->next;
		free(*phead);
		*phead = next;
		return;
	}

	pqueue* currentNode = *phead;
	while (currentNode != NULL)
	{
		if (currentNode->k == k)
		{
			currentNode->prev->next = currentNode->next;

			if (currentNode->next != NULL)
			{
				currentNode->next->prev = currentNode->prev;
			}
			
			free(currentNode);
			return;
		}
		else
		{
			currentNode = currentNode->next;
		}
	}
}


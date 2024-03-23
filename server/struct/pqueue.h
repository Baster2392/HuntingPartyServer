#include <stdio.h>

typedef struct pqueue pqueue;
struct pqueue {
	int k;
	void *data;
	pqueue *next;
	pqueue *prev;
};

pqueue* createnewnode(void* data, int k)
{
    pqueue* newNode = (pqueue*)malloc(sizeof(pqueue));
	newNode->k = k;
	newNode->data = data;
	newNode->next = NULL;
	newNode->prev = NULL;
	return newNode;
}

void
qinsert(pqueue **phead, void *data, int k)
{
    // insert first as head
	if (*phead == NULL)
	{
	    *phead = createnewnode(data, k);
	    return;
	}

    // insert as new head
	if (k >= (*phead)->k)
	{
	    pqueue* newNode = createnewnode(data, k);
	    newNode->next = (*phead);
	    newNode->prev = NULL;
	    (*phead)->prev = newNode;
	    (*phead) = newNode;
	    return;
	}

    // insert before end
	pqueue* currentNode = *phead;
	while (1 == 1)
	{
	    if (k >= currentNode->k)
	    {
	        pqueue* newNode = createnewnode(data, k);
	        newNode->next = currentNode;
	        newNode->prev = currentNode->prev;
	        currentNode->prev->next = newNode;
	        currentNode->prev = newNode;
	        return;
	    }

	    if (currentNode->next == NULL)
	    {
	        break;
	    }

	    currentNode = currentNode->next;
	}

	// insert at end
	pqueue* newNode = createnewnode(data, k);
    newNode->next = NULL;
    newNode->prev = currentNode;
    currentNode->next = newNode;
};

void
qlist(pqueue *head, void (*print_data)(void *))
{
	pqueue* currentNode = head;
	while (currentNode != NULL)
	{
	    fprintf(stderr, "%d: ", currentNode->k);
	    print_data(currentNode->data);
	    fprintf(stderr, "\n");
	    currentNode = currentNode->next;
	}
};

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
};

void* qgetbyk(pqueue** head, int k)
{
    pqueue* currentNode = *head;
    while (currentNode != NULL)
    {
        if (currentNode->k == k)
        {
            return currentNode->data;
        }

        currentNode = currentNode->next;
    }

    return NULL;
}

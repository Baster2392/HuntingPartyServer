typedef struct pqueue pqueue;
struct pqueue {
	int k;
	void *data;
	pqueue *next;
	pqueue *prev;
};

void
qinsert(pqueue **phead, void *data, int k);

void
qlist(pqueue *head, void (*print_data)(void *));

void
insertBefore(pqueue* node, void* data, int k);

void
insertAtEnd(pqueue* lastNode, void* data, int k);

void
qremove(pqueue **phead, int k);

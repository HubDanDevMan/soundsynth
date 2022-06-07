#include <stdlib.h>
#include "debug.h"


typedef struct {
	void * ptr;
	struct MemNode_t * next;
	struct MemNode_t * prev;
} MemNode_t;


MemNode_t *head;
MemNode_t *tail;
unsigned long nodeCount;

// Implements a memory safety wrapper around regular malloc
void * sndmalloc(unsigned long count)
{
	MemNode_t * new = malloc(sizeof(MemNode_t));
	
	// first entry;
	if (new == NULL){
		fprintf(stderr, "UNABLE TO ALLOCATE MEMORY FOR NODE");
		exit(-1);		// exits the whole application, no cleanup
	}

	if (tail == NULL){
		new->prev = NULL;
		new->next = NULL;
		head = new;
		tail = new;
		return;
	}

	tail->next = new;
	new->prev = tail;
	new->next = NULL
	tail = new;
	
	nodeCount++;
	
}

void sndFree(void * ptr)
{
	MemNode_t * current = head;
	MemNode_t * prev;
	MemNode_t * next;
	
	for (int i = 0; current != tail; i++){
		if (i > nodeCount){
			DEBUG("Mem access error: Pointer %p has not been allocated", ptr);
			sndOverview();
			exit(-1);
		}
		if (current->ptr == ptr){
			break;
		}
		current = current->next;
	}
	next = current->next;
	prev = current->prev;
	if (next == NULL){
		// current node is last node
		prev->next = NULL;
		free(ptr);	// guaranteed to be valid pointer
		free(next);
	}

	nodeCount--;
}

// prints all the pointers in the list
static void sndOverview(void)
{
	MemNode_t * current = head;
	int i = 0;
	while (current != tail){
		fprintf(stderr, "Pointer nr. %d is %p\n", i, current->ptr);
		i++;
		current = current->next;
	}
	if (i != nodeCount){
		fprintf(stderr, "Perhaps not all nodes iterated over");
	}
}

// deletes node and returns the pointer
static void * delNode(unsigned long index)
{
	void * res;
	MemNode_t current;
	MemNode_t previous;
	MemNode_t * next;
	if (index == 0LU){ // end reached
		head = current;
		return;
	}

	for (int i = 0; i < index; i++){
		if (index < nodeCount){
			previous = current;
			current = *current.next;
		}
	}
	res = current.ptr;
	DEBUG("freeing node");
	current.next->prev = current.prev;
	previous.next = current.next;
	free(previous.next);
	return res;
}

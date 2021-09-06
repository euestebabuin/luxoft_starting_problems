#include "sl_api.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

// Private function which shows node info
static void print_node(int val){
	printf("|%d|->", val);
}

// Global mutex used for list access. All list accesses, including the read must be considered critical regions
// in order to avoid any thread inconsistencies
static pthread_mutex_t list_lock;

// Functions used to initialize/destroy the mutex required for list access
void start_list_work(){
	pthread_mutex_init(&list_lock, NULL);
}

void stop_list_work(){
	pthread_mutex_destroy(&list_lock);
}

// Function which adds a new node to the list
// There are two cases, and in both of them we need to allocate a new node, stored isnide the new_node variable:
// 	1. Adding a node to an empty list
// 		In this case the vector pointing to the head of the list will change since new memory will be 
// 		allocated. This is why we need double indirection for the L variable. 
// 	2. Adding a node to a non-empty list
// 		In this case all we need to do is traverse the list and "attach" to the last node the newly 
// 		created node
int add_node(sl_list **L, int new_val){
	sl_list *new_node, *l_traverse;

	// invalid pointer
	if (L == NULL)
		return -1;
	
	// creating the new node
	new_node = (sl_list*)malloc(sizeof(sl_list));
	new_node->val = new_val;
	new_node->callback_print = &print_node;
	new_node->next = NULL;

	pthread_mutex_lock(&list_lock);
	// first case
	if (*L == NULL){
		*L = new_node;	
	}
	else{
		//second case
		l_traverse = *L;
		while (l_traverse->next != NULL){
			l_traverse = l_traverse->next;
		}
		l_traverse->next = new_node;
	}
	pthread_mutex_unlock(&list_lock);

	return 0;
}

// This function removes a node form the list based on a given value. Similar to how adding a node behaves, there
// are three cases: 
// 	1. The first node of the list is to be deleted.
// 	2. Another node of the list is to be deleted
// 	3. The target node is not in the list
// The deletion operation simply "skips" the target node, pointing its parent node to its child node, while 
// also freeing the target's node memory.
int delete_node(sl_list **L, int val){
	sl_list *l_traverse, *l_delete;

	if (L == NULL)
		return -1;

	if (*L == NULL)
		return -2;

	pthread_mutex_lock(&list_lock);
	if ((*L)->val == val){
		// first case
		l_delete = *L;
		(*L)= (*L)->next;
		free(l_delete);
		pthread_mutex_unlock(&list_lock);
		return 0;
	}
	else{
		l_traverse = *L;
		while (l_traverse->next != NULL && l_traverse->next->val != val){
			l_traverse = l_traverse->next;
		}
		if (l_traverse->next == NULL){
			// third case
			pthread_mutex_unlock(&list_lock);
			return -3;
		}
		else{
			// second case
			l_delete = l_traverse->next;
			l_traverse->next = l_delete->next;
			free(l_delete);
			pthread_mutex_unlock(&list_lock);
			return 0;
		}
	}
}
// This functions uses the print_node private function to needlesly complicated present the details of all the
// nodes
void print_list(sl_list *L){
	pthread_mutex_lock(&list_lock);
	while(L != NULL){
		(L->callback_print)(L->val);
		L = L->next;	
	}
	printf(" (/)\n");
	pthread_mutex_unlock(&list_lock);
}

// This function sorts the list in a very simple pointer interpretation of the "for in for" method of sorting 
// an array. Thus, the elements at the first "index" (l_traverse_a) which traverses the list from the first position to the last but one, are compared to the elements at the the second "index" (l_traverse_b), which traverses 
// the list from the l_traverse_a + 1 position to the last one.
int sort_list(sl_list *L){
	sl_list *l_traverse_a, *l_traverse_b;
	int aux;

	if (L == NULL || L->next == NULL)
		return 1;
	
	pthread_mutex_lock(&list_lock);
	l_traverse_a = L;
	while (l_traverse_a->next != NULL){
		l_traverse_b = l_traverse_a->next;
		while (l_traverse_b != NULL){
			if (l_traverse_a->val > l_traverse_b->val){
				aux = l_traverse_a->val;
				l_traverse_a->val = l_traverse_b->val;
				l_traverse_b->val = aux;
			}
			l_traverse_b = l_traverse_b->next;
		}
		l_traverse_a = l_traverse_a->next;
	}
	pthread_mutex_unlock(&list_lock);
		
	return 0;
}
// This function frees the entire list, by using two variables: next_node and delete_node. The delete node 
// "follows" the next node variable as it traverses the list, while also disposing the nodes one by one. When 
// next_node reaches the last node it stops, and delete_node performs the last "free" function call.
int flush_list(sl_list **L){
	sl_list *next_node, *delete_node;

	if (L == NULL)
		return -1;
	if (*L == NULL)
		return 0;

	pthread_mutex_lock(&list_lock);
	delete_node = *L;
	next_node = (*L)->next;
	*L = NULL;
	while(delete_node != NULL){
		free(delete_node);
		delete_node = next_node;
		if (next_node != NULL){
			next_node = next_node->next;
		}
	}
	pthread_mutex_unlock(&list_lock);

	return 0;
}

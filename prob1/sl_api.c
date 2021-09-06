#include "sl_api.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

static void print_node(int val){
	printf("|%d|->", val);
}

static pthread_mutex_t list_lock;

void start_list_work(){
	pthread_mutex_init(&list_lock, NULL);
}

void stop_list_work(){
	pthread_mutex_destroy(&list_lock);
}

//we need double indirection for the case when we are adding the first node
int add_node(sl_list **L, int new_val){
	sl_list *new_node, *l_traverse;

	if (L == NULL)
		return -1;
	
	// creating the new node
	new_node = (sl_list*)malloc(sizeof(sl_list));
	new_node->val = new_val;
	new_node->callback_print = &print_node;
	new_node->next = NULL;

	pthread_mutex_lock(&list_lock);
	if (*L == NULL){
		*L = new_node;	
	}
	else{
		l_traverse = *L;
		while (l_traverse->next != NULL){
			l_traverse = l_traverse->next;
		}
		l_traverse->next = new_node;
	}
	pthread_mutex_unlock(&list_lock);

	return 0;
}

int delete_node(sl_list **L, int val){
	sl_list *l_traverse, *l_delete;

	if (L == NULL)
		return -1;

	if (*L == NULL)
		return -2;

	pthread_mutex_lock(&list_lock);
	if ((*L)->val == val){
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
			pthread_mutex_unlock(&list_lock);
			return -3;
		}
		else{
			l_delete = l_traverse->next;
			l_traverse->next = l_delete->next;
			free(l_delete);
			pthread_mutex_unlock(&list_lock);
			return 0;
		}
	}
}

void print_list(sl_list *L){
	pthread_mutex_lock(&list_lock);
	while(L != NULL){
		(L->callback_print)(L->val);
		L = L->next;	
	}
	printf(" (/)\n");
	pthread_mutex_unlock(&list_lock);
}

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

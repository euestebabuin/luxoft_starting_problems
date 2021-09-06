#include "sl_api.h"
#include <stdio.h>
#include <stdlib.h>

static void print_node(int val){
	printf("|%d|->", val);
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
	return 0;
}

int delete_node(sl_list **L, int val){
	sl_list *l_traverse, *l_delete;

	if (L == NULL)
		return -1;

	if (*L == NULL)
		return -2;

	if ((*L)->val == val){
		l_delete = *L;
		(*L)= (*L)->next;
		free(l_delete);
	}
	else{
		l_traverse = *L;
		while (l_traverse->next != NULL && l_traverse->next->val != val){
			l_traverse = l_traverse->next;
		}
		if (l_traverse->next == NULL){
			return -3;
		}
		else{
			l_delete = l_traverse->next;
			l_traverse->next = l_delete->next;
			free(l_delete);
			return 0;
		}
	}
	return 0;
}

void print_list(sl_list *L){
	while(L != NULL){
		(L->callback_print)(L->val);
		L = L->next;	
	}
	printf(" (/)\n");
}

int sort_list(sl_list *L){
	
	return 0;
}

void flush_list(sl_list *L){

}

#include <stdio.h>
#include <stdlib.h>
//单哨兵双向循环链表 
typedef struct DNode
{
    int data;
    struct DNode *prev;
    struct DNode *next;
} DNode_t;

int DNode_delete(DNode_t *target,DNode_t *head)
{
	if (target == head)
    {
        return 0;
    }
	DNode_t  *front = target->prev;
	DNode_t  *behind = target->next;
	front->next = behind;
	behind->prev = front;
	target->next = NULL;
	target->prev = NULL;
	free(target);
	return 1;
} 


int DNode_insert(DNode_t *target,
				 DNode_t *front,  
				 DNode_t *behind
				)
{
	front->next = target;
	behind->prev = target;
	target->next = behind;
	target->prev = front;
	return 1;
}
//头插，插入head之后 
int head_behind_append(DNode_t *first,DNode_t *target) 
{
	DNode_insert(target,first,first->next);
	return 1;
}

int head_front_append(DNode_t *last,DNode_t *target) 
{
	DNode_insert(target,last->prev,last);
	return 1;
}

void check_list_empty(DNode_t *head)
{
	if(head->next == head && head->prev == head)
	{
		printf("链表为空\r\n"); 
	}
} 

int main()
{
	DNode_t head;
	head.prev = &head;
	head.next = &head;
	check_list_empty(&head);
	
}

#include <stdio.h>
#include <stdlib.h>
typedef struct Node
{
    int data;
    struct Node *next_address;
} Node_t;

void node_append(Node_t *node,int value)    
{
	Node_t *old_node;
	Node_t *current_node;
	
	old_node = node;  //获取头节点地址
	while(old_node->next_address != NULL)
	{
		old_node = old_node->next_address;
	} 
	current_node = (Node_t *)malloc(sizeof(Node_t));
	old_node->next_address = current_node;
	current_node->data = value;
	current_node->next_address = NULL;
}

void node_delete(Node_t *node,int num) 
{
	Node_t *current_node;
	Node_t *before_node; 
	current_node = node;
	int count = 0;
	while(count < num && current_node->next_address!=NULL)  //找到目标删除节点 
	{

		before_node =  current_node;
		current_node = current_node->next_address;
		count ++; 
	}
	if(count == num)
	{
		before_node->next_address = current_node->next_address;
		free(current_node);
	}
	else if(current_node->next_address==NULL && count < num)  
	{
		printf("超过范围,不可删除\r\n");
		return; 
	}
	
	
}

void node_print(Node_t *node)
{
    Node_t *current_node;

    // 跳过头节点，从第一个数据节点开始
    current_node = node->next_address;

    while (current_node != NULL)
    {
        printf("%d\r\n", current_node->data);
        current_node = current_node->next_address;
    }
}

int main()
{
	Node_t head;
	head.next_address = NULL;
	node_append(&head,10);
	node_append(&head,20);
	node_append(&head,30);
	node_delete(&head,2);
	node_print(&head);
	return 0; 
} 

#include <stdio.h>
#include <stdlib.h>

typedef struct DCNode {
    int data;
    struct DCNode *prev;
    struct DCNode *next;
} DCNode;

DCNode *createDCNode(int val) {
    DCNode *node = (DCNode *)malloc(sizeof(DCNode));
    node->data = val;
    node->prev = node;
    node->next = node;
    return node;
}

void pushBack(DCNode **head, int val) {
    DCNode *node = createDCNode(val);

    if (*head == NULL) {
    *head = node;
    return;
    }

    DCNode *tail = (*head)->prev;

    node->next = *head;
    node->prev = tail;
    tail->next = node;
    (*head)->prev = node;
}

void printForward(DCNode *head) {
    if (head == NULL)
    return;

    DCNode *cur = head;
    do {
    printf("%d <-> ", cur->data);
    cur = cur->next;
    } while (cur != head);

    printf("(back to head)\n");
}

void deleteNode(DCNode **head, DCNode *node) {
    if (*head == NULL || node == NULL)
    return;

    if (node->next == node) {
    free(node);
    *head = NULL;
    return;
    }

    node->prev->next = node->next;
    node->next->prev = node->prev;

    if (*head == node)
    *head = node->next;
    free(node);
}

int main(void) {
    DCNode *head = NULL;

    pushBack(&head, 10);
    pushBack(&head, 20);
    pushBack(&head, 30);

    printForward(head); // 10 <-> 20 <-> 30 <-> (back to head)

    deleteNode(&head, head->next); // É¾³ý 20
    printForward(head); // 10 <-> 30 <-> (back to head)

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    if (list_empty(head)) {
        free(head);
        return;
    }
    struct list_head *c = head->next;
    while (c != head) {
        struct list_head *next = c->next;  // used to store the next node of c
        element_t *n = container_of(c, element_t, list);
        q_release_element(n);
        // free(n);
        // free(n->value);
        c = next;
    }
    free(head);
    return;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;
    element_t *new_node = malloc(sizeof(element_t));
    if (!new_node)
        return false;
    INIT_LIST_HEAD(&new_node->list);
    new_node->value = strdup(s);
    if (!new_node->value) {
        free(new_node);
        return false;
    }
    list_add(&new_node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *node = list_entry(head->next, element_t, list);
    list_del(head->next);
    if (sp && bufsize > 0) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *node = list_last_entry(head, element_t, list);
    if (sp && bufsize > 0) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->prev);
    return node;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each(li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    struct list_head *slow = head->next;
    struct list_head *fast = head->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    list_del(slow);
    element_t *del_node = list_entry(slow, element_t, list);
    // free(del_node);
    // free(del_node->value);
    q_release_element(del_node);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head *cur = head->next;
    bool check = false;

    while (cur != head) {
        struct list_head *next = cur->next;
        element_t *node = list_entry(cur, element_t, list);

        if (next != head) {
            element_t *next_node = list_entry(next, element_t, list);
            if (strcmp(node->value, next_node->value) == 0) {
                check = true;
                list_del(next);
                q_release_element(next_node);
                continue;
            }
        }

        if (check) {
            check = false;
            list_del(cur);
            q_release_element(node);
        }

        cur = next;
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *cur = head->next;
    while (cur != head && cur->next != head) {
        list_move(cur->next, cur->prev);
        cur = cur->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *cur = head;
    struct list_head *tmp = NULL;
    do {
        tmp = cur->next;
        cur->next = cur->prev;
        cur->prev = tmp;
        cur = tmp;
    } while (cur != head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;
    int count = q_size(head);
    struct list_head *cur = head->next;
    while (count >= k) {
        struct list_head *prev, *next;
        prev = cur->prev;
        next = cur->next;
        for (int i = 0; i < k; i++) {
            list_move(cur, prev);
            cur = next;
            next = next->next;
        }
        count -= k;
    }
}

void q_merge_list(struct list_head *left_list,
                  struct list_head *right_list,
                  bool descend)
{
    if (!left_list || !right_list)
        return;

    struct list_head temp;
    INIT_LIST_HEAD(&temp);

    while (!list_empty(left_list) && !list_empty(right_list)) {
        element_t *first_node = list_first_entry(left_list, element_t, list);
        element_t *second_node = list_first_entry(right_list, element_t, list);
        bool tag = (descend)
                       ? strcmp(first_node->value, second_node->value) > 0
                       : strcmp(first_node->value, second_node->value) < 0;
        element_t *add_first = tag ? first_node : second_node;
        list_move_tail(&add_first->list, &temp);
    }
    list_splice_tail_init(left_list, &temp);
    list_splice_tail_init(right_list, &temp);
    list_splice(&temp, left_list);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *slow = head;
    struct list_head *fast = head->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    struct list_head first;
    list_cut_position(&first, head, slow);
    q_sort(&first, descend);
    q_sort(head, descend);
    q_merge_list(head, &first, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    struct list_head *cur = head->prev;
    while (cur->prev != head) {
        if (strcmp(list_entry(cur, element_t, list)->value,
                   list_entry(cur->prev, element_t, list)->value) < 0) {
            struct list_head *tmp = cur->prev;
            list_del(tmp);
            q_release_element(list_entry(tmp, element_t, list));
        } else {
            cur = cur->prev;
        }
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    struct list_head *cur = head->prev;
    while (cur->prev != head) {
        if (strcmp(list_entry(cur, element_t, list)->value,
                   list_entry(cur->prev, element_t, list)->value) > 0) {
            struct list_head *tmp = cur->prev;
            list_del(tmp);
            q_release_element(list_entry(tmp, element_t, list));
        } else {
            cur = cur->prev;
        }
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;

    queue_contex_t *first = list_first_entry(head, queue_contex_t, chain);
    struct list_head *cur = first->chain.next;

    while (cur != head) {
        queue_contex_t *cur_q = list_entry(cur, queue_contex_t, chain);
        list_splice_init(cur_q->q, first->q);
        first->size += cur_q->size;
        cur_q->size = 0;
        cur = cur->next;
    }
    q_sort(first->q, descend);

    return first->size;
}
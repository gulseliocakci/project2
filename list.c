/**
 * @file list.c
 * @brief  A simple doubly linked list stored in an array (contiguous memory).
 * This program is written for educational purposes and includes thread-safe
 * operations for add/remove as well as synchronized iteration.
 * @version 0.2
 * @date 2024-04-21
 *
 * @author adaskin
 * @copyright Copyright (c) 2024
 */

 #include "headers/list.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "headers/globals.h"
 /**
  * @brief Create a list object, allocates new memory for list, and sets its data members.
  */
 List *create_list(size_t datasize, int capacity) {
     List *list = malloc(sizeof(List));
     memset(list, 0, sizeof(List));
 
     list->datasize = datasize;
     list->nodesize = sizeof(Node) + datasize;
 
     list->startaddress = malloc(list->nodesize * capacity);
     list->endaddress = list->startaddress + (list->nodesize * capacity);
     memset(list->startaddress, 0, list->nodesize * capacity);
 
     list->lastprocessed = (Node *)list->startaddress;
     list->number_of_elements = 0;
     list->capacity = capacity;
 
     /* Initialize mutex and semaphores */
     pthread_mutex_init(&list->lock, NULL);
     sem_init(&list->empty_slots, 0, capacity);
     sem_init(&list->filled_slots, 0, 0);
 
     /* ops */
     list->self = list;
     list->add = add;
     list->removedata = removedata;
     list->removenode = removenode;
     list->pop = pop;
     list->peek = peek;
     list->destroy = destroy;
     list->printlist = printlist;
     list->printlistfromtail = printlistfromtail;
 
     return list;
 }
 
 /**
  * @brief Finds an unoccupied node slot in the memory area of the list.
  * @note This function assumes that the caller has already acquired list->lock.
  * It is not thread-safe on its own.
  */
 static Node *find_memcell_fornode(List *list) {
     Node *node = NULL;
     Node *temp = list->lastprocessed;
     while ((char *)temp < list->endaddress) {
         if (temp->occupied == 0) {
             node = temp;
             break;
         }
         temp = (Node *)((char *)temp + list->nodesize);
     }
     if (node == NULL) {
         temp = (Node *)list->startaddress;
         while (temp < list->lastprocessed) {
             if (temp->occupied == 0) {
                 node = temp;
                 break;
             }
             temp = (Node *)((char *)temp + list->nodesize);
         }
     }
     return node;
 }
 
 /**
  * @brief Adds a node with given data to the head of the list.
  */
 Node *add(List *list, void *data) {
     Node *node = NULL;
 
     sem_wait(&list->empty_slots);
     pthread_mutex_lock(&list->lock);
 
     node = find_memcell_fornode(list);
     if (node != NULL) {
         node->occupied = 1;
         memcpy(node->data, data, list->datasize);
 
         if (list->head != NULL) {
             Node *oldhead = list->head;
             oldhead->prev = node;
             node->prev = NULL;
             node->next = oldhead;
         }
 
         list->head = node;
         list->lastprocessed = node;
         list->number_of_elements += 1;
 
         if (list->tail == NULL) {
             list->tail = list->head;
         }
     } else {
         perror("list is full despite semaphore!");
     }
 
     pthread_mutex_unlock(&list->lock);
     sem_post(&list->filled_slots);
 
     return node;
 }
 
 /**
  * @brief Removes the node containing the specified data.
  */
 int removedata(List *list, void *data) {
     int result = 1;
 
     pthread_mutex_lock(&list->lock);
     Node *temp = list->head;
     while (temp != NULL && memcmp(temp->data, data, list->datasize) != 0) {
         temp = temp->next;
     }
 
     if (temp != NULL) {
         Node *prevnode = temp->prev;
         Node *nextnode = temp->next;
         if (prevnode != NULL) prevnode->next = nextnode;
         if (nextnode != NULL) nextnode->prev = prevnode;
 
         if (temp == list->head) list->head = nextnode;
         if (temp == list->tail) list->tail = prevnode;
 
         temp->next = NULL;
         temp->prev = NULL;
         temp->occupied = 0;
 
         list->number_of_elements--;
         list->lastprocessed = temp;
 
         result = 0;
         sem_post(&list->empty_slots);
     }
 
     pthread_mutex_unlock(&list->lock);
     return result;
 }
 
 /**
  * @brief Removes the head node and copies its data into dest.
  */
 void *pop(List *list, void *dest) {
     void *result = NULL;
 
     sem_wait(&list->filled_slots);
     pthread_mutex_lock(&list->lock);
 
     if (list->head != NULL) {
         Node *node = list->head;
         Node *nextnode = node->next;
         if (nextnode != NULL) nextnode->prev = NULL;
 
         list->head = nextnode;
         if (list->tail == node) list->tail = NULL;
 
         memcpy(dest, node->data, list->datasize);
         result = dest;
 
         node->next = NULL;
         node->prev = NULL;
         node->occupied = 0;
 
         list->number_of_elements--;
         list->lastprocessed = node;
     }
 
     pthread_mutex_unlock(&list->lock);
     sem_post(&list->empty_slots);
 
     return result;
 }
 
 /**
  * @brief Returns the data at the head of the list.
  * @note The returned pointer may become invalid if another thread removes the head node.
  */
 void *peek(List *list) {
     void *result = NULL;
 
     pthread_mutex_lock(&list->lock);
     if (list->head != NULL) {
         result = list->head->data;
     }
     pthread_mutex_unlock(&list->lock);
 
     return result;
 }
 
 /**
  * @brief Removes a specific node from the list.
  */
 int removenode(List *list, Node *node) {
     int result = 1;
 
     pthread_mutex_lock(&list->lock);
 
     if (node != NULL) {
         Node *prevnode = node->prev;
         Node *nextnode = node->next;
         if (prevnode != NULL) prevnode->next = nextnode;
         if (nextnode != NULL) nextnode->prev = prevnode;
 
         if (node == list->head) list->head = nextnode;
         if (node == list->tail) list->tail = prevnode;
 
         node->next = NULL;
         node->prev = NULL;
         node->occupied = 0;
 
         list->number_of_elements--;
         list->lastprocessed = node;
 
         result = 0;
         sem_post(&list->empty_slots);
     }
 
     pthread_mutex_unlock(&list->lock);
     return result;
 }
 
 /**
  * @brief Destroys the list and frees all allocated memory.
  */
 void destroy(List *list) {
    pthread_mutex_destroy(&list->lock);
    sem_destroy(&list->empty_slots);
    sem_destroy(&list->filled_slots);

    free(list->startaddress);

    free(list);
}
 
 /**
  * @brief Prints list from head to tail.
  * @note The print function should be thread-safe and fast to avoid blocking other threads.
  */
 void printlist(List *list, void (*print)(void *)) {
     pthread_mutex_lock(&list->lock);
 
     Node *temp = list->head;
     while (temp != NULL) {
         print(temp->data);
         temp = temp->next;
     }
 
     pthread_mutex_unlock(&list->lock);
 }
 
 /**
  * @brief Prints list from tail to head.
  * @note The print function should be thread-safe and fast to avoid blocking other threads.
  */
 void printlistfromtail(List *list, void (*print)(void *)) {
     pthread_mutex_lock(&list->lock);
 
     Node *temp = list->tail;
     while (temp != NULL) {
         print(temp->data);
         temp = temp->prev;
     }
 
     pthread_mutex_unlock(&list->lock);
 }
 
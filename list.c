/**
 * @file list.c
 * @brief  A simple doubly linked list stored in an array (contiguous memory).
 * This program is written for educational purposes and includes thread-safe
 * operations for add/remove as well as synchronized iteration.
 * @version 0.3
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
 #include <time.h>
 
 /**
  * @brief Create a list object, allocates new memory for list, and sets its data members.
  */
 List *create_list(size_t datasize, int capacity) {
     if (datasize == 0 || capacity <= 0) {
         fprintf(stderr, "[%s][%s] Invalid parameters: datasize=%zu, capacity=%d\n", 
                 "2025-05-19 11:19:15", "Amine86s", datasize, capacity);
         return NULL;
     }
 
     List *list = malloc(sizeof(List));
     if (!list) {
         fprintf(stderr, "[%s][%s] Failed to allocate list structure\n", 
                 "2025-05-19 11:19:15", "Amine86s");
         return NULL;
     }
     memset(list, 0, sizeof(List));
 
     list->datasize = datasize;
     list->nodesize = sizeof(Node) + datasize;
 
     size_t total_size = list->nodesize * capacity;
     list->startaddress = malloc(total_size);
     if (!list->startaddress) {
         fprintf(stderr, "[%s][%s] Failed to allocate memory for nodes\n", 
                 "2025-05-19 11:19:15", "Amine86s");
         free(list);
         return NULL;
     }
     list->endaddress = list->startaddress + total_size;
     memset(list->startaddress, 0, total_size);
 
     list->lastprocessed = (Node *)list->startaddress;
     list->number_of_elements = 0;
     list->capacity = capacity;
 
     /* Initialize mutex and semaphores */
     if (pthread_mutex_init(&list->lock, NULL) != 0) {
         fprintf(stderr, "[%s][%s] Failed to initialize mutex\n", 
                 "2025-05-19 11:19:15", "Amine86s");
         free(list->startaddress);
         free(list);
         return NULL;
     }
     
     if (sem_init(&list->empty_slots, 0, capacity) != 0 ||
         sem_init(&list->filled_slots, 0, 0) != 0) {
         fprintf(stderr, "[%s][%s] Failed to initialize semaphores\n", 
                 "2025-05-19 11:19:15", "Amine86s");
         pthread_mutex_destroy(&list->lock);
         free(list->startaddress);
         free(list);
         return NULL;
     }
 
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
     if (!list || !list->startaddress || !list->lastprocessed) return NULL;
     
     Node *node = NULL;
     Node *temp = list->lastprocessed;
     char *end = list->endaddress;
     
     // Validate pointer position
     if ((char *)temp < list->startaddress || (char *)temp >= end) {
         temp = (Node *)list->startaddress;
     }
 
     // Search from last processed to end
     while ((char *)temp < end) {
         if (!temp->occupied) {
             node = temp;
             break;
         }
         temp = (Node *)((char *)temp + list->nodesize);
     }
     
     // If not found, search from start to last processed
     if (!node) {
         temp = (Node *)list->startaddress;
         while (temp < list->lastprocessed) {
             if (!temp->occupied) {
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
     if (!list || !data) return NULL;
 
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
         fprintf(stderr, "[%s][%s] List is full despite semaphore!\n", 
                 "2025-05-19 11:19:15", "Amine86s");
     }
 
     pthread_mutex_unlock(&list->lock);
     sem_post(&list->filled_slots);
 
     return node;
 }
 
 /**
  * @brief Removes the node containing the specified data.
  */
 int removedata(List *list, void *data) {
     if (!list || !data) return 1;
 
     int result = 1;
 
     sem_wait(&list->filled_slots);
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
     }
 
     pthread_mutex_unlock(&list->lock);
     sem_post(&list->empty_slots);
 
     return result;
 }
 
 /**
  * @brief Removes the head node and copies its data into dest.
  */
 void *pop(List *list, void *dest) {
     if (!list || !dest) return NULL;
 
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
  * @brief Returns the data at the head of the list without removing it.
  */
 void *peek(List *list) {
     if (!list) return NULL;
 
     void *result = NULL;
     
     if (sem_trywait(&list->filled_slots) == 0) {
         pthread_mutex_lock(&list->lock);
         
         if (list->head != NULL) {
             result = list->head->data;
         }
         
         pthread_mutex_unlock(&list->lock);
         sem_post(&list->filled_slots);
     }
 
     return result;
 }
 
 /**
  * @brief Removes a specific node from the list.
  */
 int removenode(List *list, Node *node) {
     if (!list || !node) return 1;
 
     int result = 1;
 
     sem_wait(&list->filled_slots);
     pthread_mutex_lock(&list->lock);
 
     if (node->occupied) {
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
     }
 
     pthread_mutex_unlock(&list->lock);
     sem_post(&list->empty_slots);
 
     return result;
 }
 
 /**
  * @brief Destroys the list and frees all allocated memory.
  */
 void destroy(List *list) {
     if (!list) return;
 
     pthread_mutex_lock(&list->lock);
     
     sem_destroy(&list->empty_slots);
     sem_destroy(&list->filled_slots);
     pthread_mutex_unlock(&list->lock);
     pthread_mutex_destroy(&list->lock);
 
     if (list->startaddress) {
         free(list->startaddress);
         list->startaddress = NULL;
     }
 
     list->head = NULL;
     list->tail = NULL;
     list->lastprocessed = NULL;
     free(list);
 }
 
 /**
  * @brief Prints list from head to tail.
  */
 void printlist(List *list, void (*print)(void *)) {
     if (!list || !print) return;
 
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
  */
 void printlistfromtail(List *list, void (*print)(void *)) {
     if (!list || !print) return;
 
     pthread_mutex_lock(&list->lock);
 
     Node *temp = list->tail;
     while (temp != NULL) {
         print(temp->data);
         temp = temp->prev;
     }
 
     pthread_mutex_unlock(&list->lock);
 }
 
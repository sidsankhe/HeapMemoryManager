#ifndef __GLTHREAD__
#define __GLTHREAD__

#include <stdlib.h> 
#include <stdint.h> 

typedef struct glnode {
    struct glnode* left; 
    struct glnode* right; 
} glnode_t; 

typedef struct glthread {
    glnode_t* head; 
    uint64_t offset; 
} glthread_t; 

void add_node(glthread_t* thread, glnode_t* node); 

void add_node_sorted(glthread_t* thread, glnode_t* node, int (*compare_fn)(void*, void*)); 

void remove_node(glthread_t* thread, glnode_t* node);

void init_glthread(glthread_t* glthread, uint64_t offset); 

void delete_and_free_thread(glthread_t*); 

#define init_glnode(glnode_ptr)        \
    glnode_ptr->left = NULL;            \
    glnode_ptr->right = NULL; 

#define offsetof(struct_ptr, field_name)        \
    ((uint64_t)&((struct_ptr*)0)->field_name)

//iterative macros to make iteration over the doubly-linkedlist easy
#define GLTHREAD_ITERATOR_BEGIN(struct_type, curr, thread_ptr)     \
{                                                                  \
    glnode_t* curr_node = NULL, *next = NULL;                       \
    for(curr_node = thread_ptr->head; curr_node; curr_node = next) {       \
        next = curr_node->right;                                    \
        curr = (struct_type*)((char*)curr_node - thread_ptr->offset); 

#define GLTHREAD_ITERATOR_END }} 

#endif 



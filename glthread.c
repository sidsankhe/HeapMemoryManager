#include "glthread.h"
#include <stdio.h> 
#include <assert.h> 

void init_glthread(glthread_t* glthread, uint64_t offset) {
    glthread->head = NULL; 
    glthread->offset = offset; 
}

void add_node(glthread_t* thread, glnode_t* node) {
    //add to the head of the glnode
    node->left = NULL; 
    node->right = NULL; 
    if (!thread->head) {
        thread->head = node; 
        return; 
    } 
    node->right = thread->head; 
    thread->head->left = node; 
    thread->head = node; 
}

static void add_before_node(glthread_t* thread, glnode_t* to_be_added, glnode_t* node) {
    if (node == thread->head) {
        to_be_added->left = NULL; 
        to_be_added->right = thread->head; 
        thread->head->right = to_be_added; 
        thread->head = to_be_added; 
        return; 
    }
    to_be_added->right = node; 
    to_be_added->left = node->left; 
    if (node->left) node->left->right = to_be_added; 
    node->left = to_be_added; 

}

static void add_after_node(glthread_t* thread, glnode_t* to_be_added, glnode_t* node) {
    to_be_added->right = node->right; 
    to_be_added->left = node; 
    if (node->right) node->right->left = to_be_added; 
    node->right = to_be_added; 
}

void add_node_sorted(glthread_t* thread, glnode_t* node, int (*compare_fn)(void*, void*)) {
    if (!thread->head) {
        node->left = NULL; 
        node->right = NULL; 
        thread->head = node; 
        return; 
    }
    glnode_t* curr = thread->head, *prev = NULL, *next = NULL; 
    int offset = thread->offset; 
    void* curr_obj = NULL;
    void* compare_obj = (void*)((char*)node - offset);  
    for (; curr; curr = next) {
        next = curr->right; 
        curr_obj = (void*)((char*)curr - offset); 
        if (compare_fn(compare_obj, curr_obj) == 1 && next) continue; 
        else if (compare_fn(compare_obj, curr_obj) == 1 && !next) add_after_node(thread, node, curr); 
        else {
            add_before_node(thread, node, curr); 
            return; 
        }
    }
    return; 
}

void remove_node(glthread_t* thread, glnode_t* node) {
    if (node == thread->head) {
        thread->head = node->right; 
        node->left = NULL; 
        node->right = NULL;  
        return; 
    }
    if (node->left) {
        node->left->right = node->right; 
        if (node->right) {
            node->right->left = node->left;
            node->left = NULL; 
            node->right = NULL; 
            return;   
        } 
        node->left = NULL; 
        node->right = NULL; 
        return; 
    }
}

void delete_and_free(glthread_t* thread) {
    while (thread->head) {
        remove_node(thread, thread->head); 
    }
    free(thread); 
}
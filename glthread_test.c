#include "glthread.h"
#include <string.h> 
#include <stdio.h> 

typedef struct glue_stud{
    int grade; 
    int year; 
    char name[64];
    glnode_t glnode; 
} student_t; 

static int compare(void* first, void* second) {
    student_t* ptr1 = (student_t*)first; 
    student_t* ptr2 = (student_t*)second; 
    // printf("First: Name: %s, Grade: %d\n", ptr1->name, ptr1->grade); 
    // printf("Second: Name: %s\n", ptr2->name); 
    if (ptr1->grade > ptr2->grade) {
        return 1; 
    } else if (ptr1->grade == ptr2->grade) {
        return 0; 
    } else {
        return -1; 
    }
}

int main(int argc, char** argv) {
    glthread_t* thread_ptr = (glthread_t*)calloc(1, sizeof(glthread_t)); 
    init_glthread(thread_ptr, offsetof(student_t, glnode)); 
    //printf("Offset: %ld", offsetof(student_t, glnode)); 

    //Student 1
    student_t* student_1 = (student_t*)calloc(1, sizeof(student_t)); 
    student_1->grade = 99; 
    student_1->year = 3; 
    strncpy(student_1->name, "Alex", 64); 
    init_glnode((&student_1->glnode)); 

    //Student 2
    student_t* student_2 = (student_t*)calloc(1, sizeof(student_t)); 
    student_2->grade = 79; 
    student_2->year = 2; 
    strncpy(student_2->name, "Neville", 64); 
    init_glnode((&student_2->glnode)); 

    //Student 3
    student_t* student_3 = (student_t*)calloc(1, sizeof(student_t)); 
    student_3->grade = 84; 
    student_3->year = 2; 
    strncpy(student_3->name, "Johnson", 64); 
    init_glnode((&student_3->glnode)); 
 
    add_node_sorted(thread_ptr, &student_2->glnode, &compare); 
    add_node_sorted(thread_ptr, &student_1->glnode, &compare);
    add_node_sorted(thread_ptr, &student_3->glnode, &compare); 

    remove_node(thread_ptr, &student_3->glnode); 

    student_t* curr = NULL; 
    int count = 0; 
    GLTHREAD_ITERATOR_BEGIN(student_t, curr, thread_ptr) {
        printf("Name: %s, Year: %d, Grade: %d\n", curr->name, curr->year, curr->grade); 
        count++; 
        //if (count >= 4) break; 
    } GLTHREAD_ITERATOR_END; 

    free(student_1); 
    free(student_2); 
    free(student_3); 
    delete_and_free(thread_ptr); 

}
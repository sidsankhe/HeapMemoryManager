#include <stdio.h> 
#include "u_api.h"

typedef struct student{
    int gpa; 
    char name[64]; 
} student_t; 

typedef struct professor{
    int salary; 
    char name[64]; 
} professor_t; 

int main(int argc, char** argv) {
    mm_init(); 
    REGISTER_STRUCT(student_t); 
    REGISTER_STRUCT(professor_t); 
    print_registered_page_families(); 

    xcalloc("professor_t", 1); 
    xcalloc("professor_t", 1);
    xcalloc("professor_t", 1);  
    xcalloc("professor_t", 1); 
    xcalloc("professor_t", 1);
    xcalloc("professor_t", 1);
    xcalloc("professor_t", 1); 
    xcalloc("professor_t", 1);
    xcalloc("professor_t", 1);
    // xcalloc("student_t", 1);
    // print_memory_usage("student_t"); 
    // xcalloc("student_t", 1); 
    print_memory_usage("professor_t");
    //print_memory_usage("student_t");  
    // 
    // xcalloc("professor_t", 1); 

    // xcalloc("student_t", 1); 
    // xcalloc("student_t", 1); 
    // xcalloc("student_t", 1); 

}
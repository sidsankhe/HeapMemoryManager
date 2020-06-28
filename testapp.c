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
    REGISTER_STRUCT(professor_t); 
    REGISTER_STRUCT(student_t); 
    print_registered_page_families(); 
    
}
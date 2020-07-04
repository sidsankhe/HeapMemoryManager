# A Linux Heap Memory Manager

Implemented a simple heap allocator along with calls to calloc and free with a slightly different interface, as a small side project. Uses a priority queue to service malloc requests with a worst-fit algorithm. Example usage for the library is below: 

```
#include "u_api.h" 

typedef struct stack_node {
    void* data; 
    struct stack_node* next; 
} stack_node_t; 

typedef struct stack {
    stack_node_t* head; 
    int length; 
} stack_t; 
 
int main(int argc, char** argv) {
    //init call to initialize the memory manager
    mm_init();  

    //register the structs for easy allocation
    REGISTER_STRUCT(stack_node_t); 
    REGISTER_STRUCT(stack_t); 

    stack_node_t* ptr = (stack_node_t*)_calloc(stack_node_t, 3); //allocates a section of memory for 3 stack nodes

    stack_t* stack = (stack_t*)_calloc(stack, 1); //allocates a section of memory for 1 stack struct

    char* string = (char*)_calloc(char, 20); //allocates a section to memory for a string of length 20

    _free(ptr); 
    _free(stack); 
    _free(string); 
}


#include "memorymanager.h"


#define REGISTER_STRUCT(name)         \
    (instantiate_new_page_family(#name, sizeof(name)))


void mm_init(); 
void print_registered_page_families(); 
void* xcalloc(char* struct_name, int units); 
void print_memory_usage(char* struct_name); 
void xfree(void* ptr); 
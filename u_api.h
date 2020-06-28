#include "memorymanager.h"


#define REGISTER_STRUCT(name)         \
    (instantiate_new_page_family(#name, sizeof(name)))


void mm_init(); 
void print_registered_page_families(); 
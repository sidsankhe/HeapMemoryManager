#include <stdint.h> 

#define MAX_STRUCT_SIZE 64

typedef struct vm_page_family {
    char struct_name[MAX_STRUCT_SIZE];
    uint32_t struct_size;  
} vm_page_family_t; 

typedef struct page_for_structs {
    struct page_for_structs *next; 
    vm_page_family_t vm_page_family[0]; 
} page_for_structs_t; 

#define MAX_FAMILIES_PER_PAGE           \
    (SYSTEM_PAGE_SIZE - sizeof(page_for_structs_t*)) / sizeof(vm_page_family_t)

#define PAGE_FAMILY_ITERATOR_BEGIN(head, curr)      \
{                                                                   \
    uint32_t count = 0;                                             \
    for (curr = (vm_page_family_t*)&head->vm_page_family[0]; \
    curr->struct_size && count < MAX_FAMILIES_PER_PAGE; curr++, count++) {   

#define PAGE_FAMILY_ITERATOR_END(head, curr) }}

void instantiate_new_page_family(char* name, uint32_t size); 

vm_page_family_t* lookup(char* struct_name); 

typedef enum {
    FALSE,
    TRUE
} bool_t;

typedef struct block_metadata {
    bool_t is_free; 
    uint32_t block_size; 
    uint32_t offset; 
    struct block_metadata* prev_block; 
    struct block_metadata* next_block; 
} block_metadata_t; 

#define offset_of(container_structure, field_name)    \
    (unsigned int)&(((container_structure*)0)->field_name)

#define get_page_from_block(ptr) \
    (void*)((char*)ptr - ptr->offset)

#define next_block(ptr)         \
    (ptr->next_block)

#define next_block_by_size(ptr)     \
    (block_meta_data_t*)((char*)(ptr+1) + ptr->block_size)

#define prev_block(ptr) \
    (ptr->prev_block)

#define bind_blocks_for_allocation(allocated_block, free_block)    \
    free_block->next_block = allocated_block->next_block;                       \
    free_block->prev_block = allocated_block;                             \
    allocated_block->next_block = free_block;                             \
    if (free_block->next) free_block->next_block->prev_block = free_block 









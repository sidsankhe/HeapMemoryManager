#include <stdint.h> 
#include "glthread.h"

#define MAX_STRUCT_SIZE 64


//will be stored at the bottom of each virtual memory page 
typedef struct vm_page_family {
    char struct_name[MAX_STRUCT_SIZE];
    uint32_t struct_size;  
    struct vm_page* first_page; 
    glthread_t free_blocks; 
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
    glnode_t glnode; 
    struct block_metadata* prev_block; 
    struct block_metadata* next_block; 
} block_metadata_t; 

#define offset_of(container_structure, field_name)    \
    (uint64_t)&(((container_structure*)0)->field_name)

#define get_page_from_block(ptr) \
    (void*)((char*)ptr - ptr->offset)

#define next_block(ptr)         \
    (ptr->next_block)

#define next_block_by_size(ptr)     \
    (block_metadata_t*)((char*)(ptr+1) + ptr->block_size)

#define prev_block(ptr) \
    (ptr->prev_block)

#define bind_blocks_for_allocation(allocated_block, free_block)    \
    free_block->next_block = allocated_block->next_block;                       \
    free_block->prev_block = allocated_block;                             \
    allocated_block->next_block = free_block;                             \
    if (free_block->next_block) free_block->next_block->prev_block = free_block 

typedef struct vm_page {
    struct vm_page* prev_page; 
    struct vm_page* next_page; 
    struct vm_page_family* pg_family; 
    block_metadata_t block_meta; 
    char page_memory[0]; 
} vm_page_t; 

bool_t is_page_empty(vm_page_t* vm_page); 

#define MARK_PAGE_EMPTY(ptr)        \
    ptr->block_meta.next_block = NULL;  \
    ptr->block_meta.prev_block = NULL;  \
    ptr->block_meta.is_free = TRUE; 


#define VM_PAGE_ITERATOR_BEGIN(ptr, curr)   \
{                                           \
    for(curr = ptr->first_page; curr; curr = curr->next_page) { 

#define VM_PAGE_ITERATOR_END(ptr, curr) }}

#define METABLOCK_ITERATOR_BEGIN(ptr, curr)         \
{                                                   \
    for (curr = (block_metadata_t*)(&ptr->block_meta); curr; curr = curr->next_block) {

#define METABLOCK_ITERATOR_END(ptr, curr) }} 

vm_page_t* allocate_vm_page(vm_page_family_t* vm_page_family); 

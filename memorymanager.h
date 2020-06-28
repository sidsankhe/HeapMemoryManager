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

)



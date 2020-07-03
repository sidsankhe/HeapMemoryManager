#include "memorymanager.h"
#include <stdio.h> 
#include <unistd.h> 
#include <memory.h> 
#include <sys/mman.h> 
#include <assert.h> 

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static size_t SYSTEM_PAGE_SIZE = 0; 
static page_for_structs_t *head = NULL; 

void mm_init() {
    SYSTEM_PAGE_SIZE = getpagesize();       
}

static void* get_new_vm_page(int units) {
    char *vm_page = mmap(0, units * SYSTEM_PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, 
                                MAP_ANON | MAP_PRIVATE, 0, 0);
    if (vm_page == MAP_FAILED) {
        printf("Error allocating virtual pages"); 
        return NULL; 
    } 
    memset(vm_page, 0, units*SYSTEM_PAGE_SIZE); 
    return (void*)vm_page; 
}

static void release_vm_page(void* vm_page, int units) {
    if (munmap(vm_page, units*SYSTEM_PAGE_SIZE)) {
        printf("Errors deallocating virtual pages"); 
    }
}

void instantiate_new_page_family(char* name, uint32_t size) {
    vm_page_family_t* curr = NULL; 
    page_for_structs_t* new_page = NULL;
    if (size > SYSTEM_PAGE_SIZE) {
        //for now print an error and return 
        printf("Not supported yet"); 
        return; 
    }
    if (!head) {
        head = (page_for_structs_t*)get_new_vm_page(1); 
        head->next = NULL; 
        strncpy(head->vm_page_family[0].struct_name, name, MAX_STRUCT_SIZE);
        head->vm_page_family[0].struct_size = size; 
        head->vm_page_family[0].first_page = NULL; 
        init_glthread(&head->vm_page_family[0].free_blocks, offset_of(block_metadata_t, glnode)); 
        return; 
    } 

    uint32_t count = 0; 
    PAGE_FAMILY_ITERATOR_BEGIN(head, curr) {
        if (strncmp(curr->struct_name, name, MAX_STRUCT_SIZE) != 0) {
            count++; 
            continue; 
        }
        assert(0); 
    } PAGE_FAMILY_ITERATOR_END(head, curr); 

    if (count == MAX_FAMILIES_PER_PAGE) {
        new_page = (page_for_structs_t*)get_new_vm_page(1); 
        new_page->next = head; 
        head = new_page; 
        curr = &head->vm_page_family[0]; 
    } 

    strncpy(curr->struct_name, name, MAX_STRUCT_SIZE); 
    curr->struct_size = size; 
    curr->first_page = NULL; 
    init_glthread(&curr->free_blocks, offset_of(block_metadata_t, glnode));
}

void print_registered_page_families() {
    vm_page_family_t* curr = NULL; 
    page_for_structs_t* curr_page = NULL; 
    for (curr_page = head; curr_page; curr_page = curr_page->next) {
        PAGE_FAMILY_ITERATOR_BEGIN(curr_page, curr) {
            printf("Name of the struct: %s, Size of the struct: %d\n", curr->struct_name, curr->struct_size); 
        } PAGE_FAMILY_ITERATOR_END(curr_page, curr); 
    }
}

vm_page_family_t* lookup(char* struct_name) {
    vm_page_family_t* curr = NULL; 
    page_for_structs_t* curr_page = NULL; 
    for (curr_page = head; curr_page; curr_page = curr_page->next) {
        PAGE_FAMILY_ITERATOR_BEGIN(head, curr) {
            if (strncmp(curr->struct_name, struct_name, MAX_STRUCT_SIZE) == 0) {
                return curr; 
            } 
        } PAGE_FAMILY_ITERATOR_END(head, curr); 
    }
    return NULL; 
}

static int comparison_function(void* block1, void* block2) {
    block_metadata_t* ptr1 = (block_metadata_t*)block1;
    block_metadata_t* ptr2 = (block_metadata_t*)block2; 
    if (ptr1->block_size > ptr2->block_size) return -1; 
    else if (ptr1->block_size == ptr2->block_size) return 0; 
    else return 1; 
}

static void add_free_block(vm_page_family_t* vm_page_family, block_metadata_t* free_block) {
    assert(free_block->is_free); 
    add_node_sorted(&vm_page_family->free_blocks, &free_block->glnode, comparison_function); 
}

static void remove_free_block(vm_page_family_t* vm_page_family, block_metadata_t* free_block) {
    remove_node(&vm_page_family->free_blocks, &free_block->glnode); 
} 

static void combine_free_blocks(block_metadata_t* first, block_metadata_t* second) {
    assert(first->is_free && second->is_free); 
    first->block_size += sizeof(block_metadata_t) + second->block_size; 
    first->next_block = second->next_block; 
    if (second->next_block) second->next_block->prev_block = first; 
}

bool_t is_page_empty(vm_page_t* vm_page) {
    if (vm_page->block_meta.next_block == NULL && vm_page->block_meta.is_free 
    && vm_page->block_meta.prev_block == NULL) {
        return TRUE; 
    }
    return FALSE; 
}

static inline uint32_t max_page_allocatable_memory(int units) {
    return (uint32_t)((SYSTEM_PAGE_SIZE*units) - offset_of(vm_page_t, page_memory));   
}

vm_page_t* allocate_vm_page(vm_page_family_t* vm_page_family) {
    vm_page_t* page = (vm_page_t*)get_new_vm_page(1); 
    MARK_PAGE_EMPTY(page); 
    page->block_meta.block_size = max_page_allocatable_memory(1); 
    page->block_meta.offset = offset_of(vm_page_t, block_meta); 
    page->block_meta.is_free = TRUE; 
    page->next_page = NULL; 
    page->prev_page = NULL; 
    page->pg_family = vm_page_family; 
    init_glnode((&page->block_meta.glnode)); 
    add_free_block(vm_page_family, &page->block_meta); 
    if (!vm_page_family->first_page) {
        vm_page_family->first_page = page; 
        return page; 
    } 
    //init_glnode((&page->block_meta.glnode)); 
    page->next_page = vm_page_family->first_page; 
    vm_page_family->first_page->prev_page = page; 
    vm_page_family->first_page = page; 
}

void delete_and_free(vm_page_t* vm_page) {
    vm_page_family_t* pg_family = vm_page->pg_family; 
    if (pg_family->first_page == vm_page) {
        pg_family->first_page = vm_page->next_page; 
        if (vm_page->next_page) vm_page->next_page->prev_page = NULL; 
        vm_page->next_page = NULL; 
        vm_page->prev_page = NULL; 
        release_vm_page((void*)vm_page, 1); 
        return; 
    }
    if (vm_page->next_page)
        vm_page->next_page->prev_page = vm_page->prev_page; 
    if (vm_page->prev_page)
        vm_page->prev_page->next_page = vm_page->next_page; 
    vm_page->next_page = NULL; 
    vm_page->prev_page = NULL; 
    release_vm_page((void*)vm_page, 1); 
}



static inline block_metadata_t* get_largest_block(vm_page_family_t* vm_page_family) {
    if (vm_page_family->free_blocks.head == NULL) {
        return NULL; 
    }
    return (block_metadata_t*)((char*)vm_page_family->free_blocks.head - vm_page_family->free_blocks.offset); 
}

bool_t split_free_for_alloc(vm_page_family_t* vm_page_family, block_metadata_t* data, uint32_t size) {
    block_metadata_t* next = NULL; 
    fprintf(stderr, "Address: %p\n", data); 
    assert(data->is_free); 
    if (data->block_size < size) {
        return FALSE; 
    }
    uint32_t remaining_size = data->block_size - size; 
    data->is_free = FALSE; 
    data->block_size = size; 
    remove_node(&vm_page_family->free_blocks, &data->glnode); 
    if (remaining_size == 0) {
        return TRUE; 
    } else if (sizeof(block_metadata_t) < remaining_size && 
    remaining_size < sizeof(block_metadata_t) + vm_page_family->struct_size) {
        next = next_block_by_size(data); 
        next->is_free = TRUE; 
        next->block_size = remaining_size - sizeof(block_metadata_t); 
        next->offset = data->offset + sizeof(block_metadata_t) + data->block_size; 
        init_glnode((&next->glnode)); 
        add_free_block(vm_page_family, next); 
        bind_blocks_for_allocation(data, next); 
        return TRUE; 
    } else if (remaining_size < sizeof(block_metadata_t)) {
        return TRUE;  
    } else {
        next = next_block_by_size(data); 
        next->is_free = TRUE; 
        next->block_size = remaining_size - sizeof(block_metadata_t); 
        next->offset = data->offset + sizeof(block_metadata_t) + data->block_size; 
        init_glnode((&next->glnode)); 
        add_free_block(vm_page_family, next); 
        bind_blocks_for_allocation(data, next); 
        return TRUE; 
    }
    return FALSE; 
}

static block_metadata_t* allocate_free_datablock(vm_page_family_t* vm_page_family, uint32_t size) {
    //fprintf(stderr, "Here\n"); 
    block_metadata_t* largest_block = get_largest_block(vm_page_family); 
    bool_t status = FALSE; 
    if (!largest_block || largest_block->block_size < size) {
        //fprintf(stderr, "Here1\n");
        //obtain a new VM page for it 
        vm_page_t* vm_page = allocate_vm_page(vm_page_family); 
        //fprintf(stderr, "Here2\n");
        status = split_free_for_alloc(vm_page_family, &vm_page->block_meta, size); 
        if (status) return &vm_page->block_meta; 
        return NULL; 
    } 
    if (largest_block) {
        status = split_free_for_alloc(vm_page_family, largest_block, size); 
        if (status) return largest_block; 
        return NULL; 
    }
    return NULL; 
}

void* xcalloc(char* struct_name, int units) {
    vm_page_family_t* pg_family = lookup(struct_name); 
    if (!pg_family) {
        printf("Error: Structure hasn't been registered"); 
        return NULL; 
    }
    if (units * pg_family->struct_size > max_page_allocatable_memory(1)) {
        printf("Error: Memory requested chunk is too large");
        return NULL;  
    }
    block_metadata_t* free_block_metadata = allocate_free_datablock(pg_family, units * pg_family->struct_size); 
    if (free_block_metadata) {
        memset((char*)(free_block_metadata+1), 0, free_block_metadata->block_size); 
        return (void*)(free_block_metadata+1); 
    }
    return NULL; 
}

void print_vm_page_details(vm_page_t* vm_page, uint32_t i) {
    printf("\tPage Index : %u , address = %p\n", i, vm_page);
    printf("\t\t next = %p, prev = %p\n", vm_page->next_page, vm_page->prev_page);
    printf("\t\t page family = %s\n", vm_page->pg_family->struct_name);

    uint32_t j = 0;
    block_metadata_t *curr;
    METABLOCK_ITERATOR_BEGIN(vm_page, curr){

        printf(ANSI_COLOR_YELLOW "\t\t\t%-14p Block %-3u %s  block_size = %-6u  "
                "offset = %-6u  prev = %-14p  next = %p\n"
                ANSI_COLOR_RESET, curr,
                j++, curr->is_free ? "F R E E D" : "ALLOCATED",
                curr->block_size, curr->offset, 
                curr->prev_block,
                curr->next_block);
    } METABLOCK_ITERATOR_END(vm_page, curr);
}

void print_memory_usage(char* struct_name) {
    uint32_t i = 0;
    vm_page_t *vm_page = NULL;
    vm_page_family_t *vm_page_family_curr; 
    uint32_t number_of_struct_families = 0;
    uint32_t total_memory_in_use_by_application = 0;
    uint32_t cumulative_vm_pages_claimed_from_kernel = 0;

    printf("\nPage Size = %zu Bytes\n", SYSTEM_PAGE_SIZE);

    PAGE_FAMILY_ITERATOR_BEGIN(head, vm_page_family_curr){
        
        if(struct_name){
            if(strncmp(struct_name, vm_page_family_curr->struct_name,
                        strlen(vm_page_family_curr->struct_name))){
                continue;
            }
        }

        number_of_struct_families++;

        printf(ANSI_COLOR_GREEN "vm_page_family : %s, struct size = %u\n" 
                ANSI_COLOR_RESET,
                vm_page_family_curr->struct_name,
                vm_page_family_curr->struct_size);

        i = 0;

        VM_PAGE_ITERATOR_BEGIN(vm_page_family_curr, vm_page){
      
            cumulative_vm_pages_claimed_from_kernel++;
            print_vm_page_details(vm_page, i++);

        } VM_PAGE_ITERATOR_END(vm_page_family_curr, vm_page);
        printf("\n");
    } PAGE_FAMILY_ITERATOR_END(head, vm_page_family_curr);

    printf(ANSI_COLOR_MAGENTA "\nTotal Applcation Memory Usage : %u Bytes\n"
        ANSI_COLOR_RESET, total_memory_in_use_by_application);

    float memory_app_use_to_total_memory_ratio = 0.0;
    
    if(cumulative_vm_pages_claimed_from_kernel){
        memory_app_use_to_total_memory_ratio = 
        (float)(total_memory_in_use_by_application * 100)/\
        (float)(cumulative_vm_pages_claimed_from_kernel * SYSTEM_PAGE_SIZE);
    }
    printf(ANSI_COLOR_MAGENTA "Memory In Use by Application = %f%%\n"
        ANSI_COLOR_RESET,
        memory_app_use_to_total_memory_ratio);

    printf("Total Memory being used by Memory Manager = %lu Bytes\n",
        ((cumulative_vm_pages_claimed_from_kernel *\
        SYSTEM_PAGE_SIZE) + 
        (number_of_struct_families * sizeof(vm_page_family_t))));
}



#include "memorymanager.h"
#include <stdio.h> 
#include <unistd.h> 
#include <memory.h> 
#include <sys/mman.h> 
#include <assert.h> 

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
}

void print_registered_page_families() {
    vm_page_family_t* curr = NULL; 
    PAGE_FAMILY_ITERATOR_BEGIN(head, curr) {
        printf("Name of the struct: %s, Size of the struct: %d\n", curr->struct_name, curr->struct_size); 
    } PAGE_FAMILY_ITERATOR_END(head, curr); 
}

vm_page_family_t* lookup(char* struct_name) {
    vm_page_family_t* curr = NULL; 
    PAGE_FAMILY_ITERATOR_BEGIN(head, curr) {
        if (strncmp(curr->struct_name, struct_name, MAX_STRUCT_SIZE) == 0) {
            return curr; 
        } 
    } PAGE_FAMILY_ITERATOR_END(head, curr); 
    return NULL; 
}
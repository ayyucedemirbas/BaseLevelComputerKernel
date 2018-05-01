#ifndef MEMORY_H
#define MEMORY_H

#include "types.hpp"

struct mmapentry {
    std::size_t base;
    std::size_t size;
    std::size_t type;
};

const char* str_e820_type(std::size_t type);
void load_memory_map();
bool mmap_failed();
std::size_t mmap_entry_count();
const mmapentry& mmap_entry(std::size_t i);
void init_memory_manager();
std::size_t* k_malloc(std::size_t bytes);
void k_free(std::size_t* block);

#endif

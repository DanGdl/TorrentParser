/*
 * linked_list.h
 *
 *  Created on: Nov 23, 2020
 *      Author: max
 */
#ifndef UTIL_H_
#define UTIL_H_

#include <strings.h>

size_t str_hash(const char* const value);

size_t default_hash(const void* const value, size_t size);

size_t default_hash_iteraction(size_t hash);

size_t default_hash_end(size_t hash);

char* ptr_stringify(const void* const ptr);

#endif

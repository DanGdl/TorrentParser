
#ifndef BENCODED_H_
#define BENCODED_H_

#include "linked_list.h"
#include "hash_map.h"
#include "util.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define TYPE_STRING 1
#define TYPE_LIST 2
#define TYPE_MAP 3
#define TYPE_INT 4

typedef struct Bencoded Bencoded_t;


Bencoded_t* Bencoded_create(int type, void* value);

void Bencoded_free(void* p_bencoded);

int Bencoded_compare(const void* const p_bencoded1, const void* const p_bencoded2);

size_t Bencoded_hash(const void* const p_bencoded);

char* Bencoded_stringify(const void* const p_bencoded);

void Bencoded_print(const Bencoded_t* const bencoded);


int Bencoded_is_number(const Bencoded_t* const bencoded);

int Bencoded_is_text(const Bencoded_t* const bencoded);

int Bencoded_is_list(const Bencoded_t* const bencoded);

int Bencoded_is_map(const Bencoded_t* const bencoded);

void* Bencoded_get_value(const Bencoded_t* const bencoded);

Bencoded_t* Bencoded_get_value_by_key(const Bencoded_t* const bencoded, const char* const key);

#endif

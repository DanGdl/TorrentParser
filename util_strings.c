
#include "util_strings.h"

#include <stdlib.h>


unsigned long long UtilStrings_hash(const char* const value) {
    if (value == NULL) {
        return 0;
    }
    unsigned long long hash = 0;
	char* k = (char*) value;
	while (k != NULL && *k != '\0') {
		hash += *k;
		hash += (hash << 10);
		hash ^= (hash >> 6);
		k++;
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

//size_t hashmap_hash_default(const void *data, size_t len)
//{
//    const uint8_t *byte = (const uint8_t *)data;
//    size_t hash = 0;
//
//    for (size_t i = 0; i < len; ++i) {
//        hash += *byte++;
//        hash += (hash << 10);
//        hash ^= (hash >> 6);
//    }
//    hash += (hash << 3);
//    hash ^= (hash >> 11);
//    hash += (hash << 15);
//    return hash;
//}

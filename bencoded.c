
#include "bencoded.h"

#define TYPE_STRING 1
#define TYPE_LIST 2
#define TYPE_MAP 3
#define TYPE_INT 4


#define TEMPLATE_NUMBER "Bencoded[ type: number, value: %lld ]"
#define TEMPLATE_STRING "Bencoded[ type: text, value: %s ]"
#define TEMPLATE_LIST "Bencoded[ type: list, value: %s ]"
#define TEMPLATE_MAP "Bencoded[ type: map, value: %s ]"

typedef struct Bencoded {
    void* value;
    uint8_t type;
} Bencoded_t;


void* Bencoded_get_value(const Bencoded_t* const bencoded) {
	if (bencoded == NULL) {
		return NULL;
	}
	return bencoded -> value;
}

int Bencoded_is_type(int type, const Bencoded_t* const bencoded) {
	return bencoded != NULL && bencoded -> type == type;
}

int Bencoded_is_number(const Bencoded_t* const bencoded) {
	return Bencoded_is_type(TYPE_INT, bencoded);
}

int Bencoded_is_text(const Bencoded_t* const bencoded) {
	return Bencoded_is_type(TYPE_STRING, bencoded);
}

int Bencoded_is_list(const Bencoded_t* const bencoded) {
	return Bencoded_is_type(TYPE_LIST, bencoded);
}

int Bencoded_is_map(const Bencoded_t* const bencoded) {
	return Bencoded_is_type(TYPE_MAP, bencoded);
}




Bencoded_t* Bencoded_create(int type, void* value) {
	Bencoded_t* result;
	result = calloc(sizeof(*result), 1);
	if (result == NULL) {
		printf("Failed to allocate memory\n");
		return NULL;
	}
	result -> value = value;
	result -> type = type;
	return result;
}

void Bencoded_free(void* p_bencoded) {
	if (p_bencoded == NULL) {
		return;
	}
	Bencoded_t* bencoded = (Bencoded_t*) p_bencoded;

	if (bencoded -> type == TYPE_INT) {
        // printf("Free number %llu\n", *((long long*) bencoded -> value));
		free((long long*) bencoded -> value);
	}
	else if (bencoded -> type == TYPE_STRING) {
        // printf("Free text %s\n", (char*) bencoded -> value);
		free((char*) bencoded -> value);
	}
	else if (bencoded -> type == TYPE_LIST) {
		LinkedList_clear_and_free((LinkedList_t*) bencoded -> value, Bencoded_free);
	}
	else if (bencoded -> type == TYPE_MAP) {
		HashMap_clear_and_free((HashMap_t*) bencoded -> value, Bencoded_free, Bencoded_free);
	}
    free(bencoded);
}

int Bencoded_key_compare(const void* const p_key1, const void* const p_key2) {
	return strcmp((char*) p_key1, (char*) p_key2);
}

size_t Bencoded_key_hash(const void* const p_key) {
	return str_hash((char*) p_key);
}

char* Bencoded_key_stringify(const void* const p_key) {
	char* v = (char*) p_key;
	char* copy = calloc(sizeof(char), strlen(v) + 1);
	memcpy(copy, v, sizeof(char) * strlen(v));
	return copy;
}

int Bencoded_compare(const void* const p_bencoded1, const void* const p_bencoded2) {
    if (p_bencoded1 == NULL && p_bencoded2 != NULL) {
        return 1;
    } else if (p_bencoded1 != NULL && p_bencoded2 == NULL) {
        return -1;
    }
    if (p_bencoded1 == p_bencoded2) {
        return 0;
    }
    const Bencoded_t* const bencoded1 = (Bencoded_t*) p_bencoded1;
    const Bencoded_t* const bencoded2 = (Bencoded_t*) p_bencoded2;

    if (bencoded1 -> type != bencoded2 -> type) {
        return bencoded1 -> type > bencoded2 -> type;
    }
    if (bencoded1 -> type == TYPE_INT) {
        return *((long long*) bencoded1 -> value) > *((long long*) bencoded2 -> value);
    }
    else if (bencoded1 -> type == TYPE_STRING) {
        return strcmp((char*) bencoded1 -> value, (char*) bencoded2 -> value) == 0;
    }
    else if (bencoded1 -> type == TYPE_LIST) {
        const LinkedList_t* const list1 = (LinkedList_t*) bencoded1 -> value;
        const LinkedList_t* const list2 = (LinkedList_t*) bencoded2 -> value;
        return LinkedList_compare(list1, list2, Bencoded_compare) == 0;
    }
    else if (bencoded1 -> type == TYPE_MAP) {
        const HashMap_t* const map1 = (HashMap_t*) bencoded1 -> value;
        const HashMap_t* const map2 = (HashMap_t*) bencoded2 -> value;
        return HashMap_compare(map1, map2, Bencoded_key_compare, Bencoded_compare) == 0;
    }

    return 0;
}

size_t Bencoded_hash(const void* const p_bencoded) {
    if (p_bencoded == NULL) {
        return 0;
    }
    const Bencoded_t* const bencoded = (Bencoded_t*) p_bencoded;
    size_t hash = default_hash(&bencoded -> type, sizeof(bencoded -> type));

    if (bencoded -> type == TYPE_INT) {
        return default_hash_end(default_hash_iteraction(hash + default_hash(bencoded -> value, sizeof(*bencoded -> value))));
    }
    else if (bencoded -> type == TYPE_STRING) {
        return default_hash_end(default_hash_iteraction(hash + str_hash((char*) bencoded -> value)));
    }
    else if (bencoded -> type == TYPE_LIST) {
        return default_hash_end(default_hash_iteraction(hash + LinkedList_hash((LinkedList_t*) bencoded -> value, Bencoded_hash)));
    }
    else if (bencoded -> type == TYPE_MAP) {
        return default_hash_end(default_hash_iteraction(hash + HashMap_hash((HashMap_t*) bencoded -> value, Bencoded_key_hash, Bencoded_hash)));
    }
    return hash;
}

char* Bencoded_stringify(const void* const p_bencoded) {
    if (p_bencoded == NULL) {
        return NULL;
    }

    const Bencoded_t* const bencoded = (Bencoded_t*) p_bencoded;
    char* string = NULL;
    if (bencoded -> type == TYPE_INT) {
    	long long val = *((long long*) bencoded -> value);
    	int digits = 0;
    	while(val != 0) {
    		val = val / 10;
    		digits++;
    	}
        string = calloc(sizeof(char), strlen(TEMPLATE_NUMBER) + digits + 1);
        sprintf(string, TEMPLATE_NUMBER, *((long long*) bencoded -> value));
    }
    else if (bencoded -> type == TYPE_STRING) {
        string = calloc(sizeof(char), strlen(TEMPLATE_STRING) + strlen((char*) bencoded -> value) + 1);
        sprintf(string, TEMPLATE_STRING, (char*) bencoded -> value);
    }
    else if (bencoded -> type == TYPE_LIST) {
        char* list_text = LinkedList_stringify((LinkedList_t*) bencoded -> value, Bencoded_stringify);

        string = calloc(sizeof(char), strlen(TEMPLATE_LIST) + strlen(list_text) + 1);
        sprintf(string, TEMPLATE_LIST, list_text);
        free(list_text);
    }
    else if (bencoded -> type == TYPE_MAP) {
        char* map_text = HashMap_stringify((HashMap_t*) bencoded -> value, Bencoded_stringify, Bencoded_stringify);

        string = calloc(sizeof(char), strlen(TEMPLATE_MAP) + strlen(map_text) + 1);
        sprintf(string, TEMPLATE_MAP, map_text);
        free(map_text);
    }
    return string;
}


Bencoded_t* Bencoded_get_value_by_key_inner(const Bencoded_t* const bencoded, const Bencoded_t* const key) {
	if (bencoded == NULL || key == NULL) {
		return NULL;
	}
	Bencoded_t* item = bencoded;
	int is_map = Bencoded_is_map(item);
	int do_loop = is_map || Bencoded_is_list(item);
	while (do_loop) {
		LinkedList_t* list = NULL;
		if (is_map) {
			HashMap_t* map = (HashMap_t*) item -> value;
			item = HashMap_get(map, key, Bencoded_compare);
			if (item == NULL) {
				list = HashMap_get_values(map);
			} else {
				return item;
			}
		} else {
			list = (LinkedList_t*) item -> value;
		}
		for (int i = 0; i < LinkedList_size(list); i++) {
			item = Bencoded_get_value_by_key_inner((Bencoded_t*) LinkedList_get(list, i), key);
			if (item != NULL) {
				if (is_map) {
					LinkedList_clear_and_free(list, NULL);
				}
				return item;
			}
		}
		if (is_map) {
			LinkedList_clear_and_free(list, NULL);
		}
		is_map = Bencoded_is_map(item);
		do_loop = is_map || Bencoded_is_list(item);
	}
	return NULL;
}

Bencoded_t* Bencoded_get_value_by_key(const Bencoded_t* const bencoded, const char* const key) {
	if (bencoded == NULL || key == NULL) {
		return NULL;
	}
	Bencoded_t encoded_key = {.type = TYPE_STRING, .value = key };
	return Bencoded_get_value_by_key_inner(bencoded, &encoded_key);
}

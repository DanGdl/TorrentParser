
#include "hash_map.h"


#define ITEM_NULL "NULL"
#define SIZE_HASH_MAP 20
#define TEMPLATE_HASHMAP_EMPTY "HashMap[]"
#define TEMPLATE_HASHMAP "HashMap[ %s ]"
#define TEMPLATE_NODE "%s -> %s"

typedef struct MapNode {
	void* key;
    void* value;
} MapNode_t;

typedef struct HashMap {
    LinkedList_t** head;
    size_t (*hash_func)(const void* const key);
	unsigned int size;
    unsigned int capacity;
} HashMap_t;


HashMap_t* HashMap_create(size_t (*hash_func)(const void* const key)) {
    HashMap_t* map;
    map = malloc(sizeof(*map));
    if (map == NULL) {
        return NULL;
    }
    map -> head = calloc(sizeof(map -> head), SIZE_HASH_MAP);
	if (map -> head == NULL) {
		return NULL;
	}
    map -> hash_func = hash_func;
	map -> size = 0;
    map -> capacity = SIZE_HASH_MAP;
	return map;
}

int HashMap_size(const HashMap_t* const map) {
    if (map == NULL) {
		return 0;
	}
    return map -> size;
}

int HashMap_add(HashMap_t* map, void* key, void* data) {
    if (map == NULL) {
        return -1;
    }
    const size_t key_hash = map -> hash_func(key);
    const int position = key_hash % (map -> capacity);
    LinkedList_t* list = *(map -> head + position);
    if (list == NULL) {
    	list = LinkedList_create();
    	if (list == NULL) {
    		printf("Failed to allocate memory\n");
			return -1;
    	}
    	*(map -> head + position) = list;
    }
    for (int i = 0; i < LinkedList_size(list); i++) {
    	MapNode_t* node = (MapNode_t*) LinkedList_get(list, i);
		if (node -> key == key) {
			node -> value = data;
			return 0;
		}
	}
    MapNode_t* node;
	node = malloc(sizeof(*node));
	if (node == NULL) {
		printf("Failed to allocate memory for MapNode\n");
		if (LinkedList_is_empty(list)) {
			*(map -> head + position) = NULL;
			free(list);
		}
		return -1;
	}
    if (LinkedList_add(list, node) < 0) {
        printf("Failed to insert MapNode into LinkedList\n");
        free(node);
        if (LinkedList_is_empty(list)) {
			*(map -> head + position) = NULL;
			free(list);
		}
        return -1;
    }
    node -> key = key;
    node -> value = data;
    map -> size += 1;
    return 0;
}

void HashMap_remove(HashMap_t* map, const void* const key, void** data) {
	if (map == NULL) {
		*data = NULL;
		return;
	}
	const size_t key_hash = map -> hash_func(key);
	const int position = key_hash % (map -> capacity);
	LinkedList_t* list = *(map -> head + position);

	MapNode_t* node = NULL;
	for (int i = 0; i < LinkedList_size(list); i++) {
		node = (MapNode_t*) LinkedList_get(list, i);
		if (node -> key == key) {
			break;
		}
	}
	LinkedList_remove(list, node);
	*data = node -> value;
	map -> size -= 1;
	free(node);
}

void* HashMap_get(
		const HashMap_t* const map, const void* const key,
		int (*key_compare)(const void* const key1, const void* const key2)
) {
	if (map == NULL) {
		return NULL;
	}
	const size_t key_hash = map -> hash_func(key);
	const int position = key_hash % (map -> capacity);
	LinkedList_t* list = *(map -> head + position);

	MapNode_t* node = NULL;
	for (int i = 0; i < LinkedList_size(list); i++) {
		node = (MapNode_t*) LinkedList_get(list, i);
		if ((key_compare == NULL && node -> key == key)
				|| (key_compare != NULL && key_compare(key, node -> key))
		) {
			return node -> value;
		}
	}
	return NULL;
}

LinkedList_t* HashMap_get_values(HashMap_t* map) {
	if (map == NULL) {
		return NULL;
	}
	LinkedList_t* list = LinkedList_create();
	if (list == NULL) {
		printf("Failed to allocate memory for list\n");
		return NULL;
	}
	for (int j = 0; j < map -> capacity; j++) {
		LinkedList_t* map_list = *(map -> head + j);
		MapNode_t* node = NULL;

		for (int i = 0; i < LinkedList_size(map_list); i++) {
			node = (MapNode_t*) LinkedList_get(map_list, i);
			LinkedList_add(list, node -> value);
		}
	}

	return list;
}


void HashMap_clear_and_free(HashMap_t* map, void (*key_free)(void* key), void (*value_free)(void* item)) {
	if (map == NULL) {
		return;
	}

	for (int j = 0; j < map -> capacity; j++) {
		LinkedList_t* list = *(map -> head + j);
		MapNode_t* node = NULL;

		while (LinkedList_size(list) != 0) {
			node = (MapNode_t*) LinkedList_remove_by_idx(list, 0);
			if (node -> key != NULL && key_free != NULL) {
				key_free(node -> key);
                node -> key = NULL;
			}
			if (node -> value != NULL && value_free != NULL) {
				value_free(node -> value);
                node -> value = NULL;
			}
			map -> size--;
			free(node);
		}
		free(list);
	}
	free(map -> head);
	free(map);
}



char* HashMapNode_stringify(
		const void* const p_node,
		char* (*key_stringify)(const void* const key),
		char* (*item_stringify)(const void* const item)
) {
	if (p_node == NULL) {
		return NULL;
	}
	const MapNode_t* const node = (MapNode_t*) p_node;
	char* item_str = NULL;
	char* key_str = NULL;
	char* value_str = NULL;
	if (key_stringify == NULL) {
		key_str = ptr_stringify(node -> key);
	} else {
		key_str = key_stringify(node -> key);
	}
	if (key_str == NULL) {
		key_str = calloc(sizeof(char), strlen(ITEM_NULL) + 1);
		memcpy(key_str, ITEM_NULL, sizeof(char) * strlen(ITEM_NULL));
	}

	if (item_stringify == NULL) {
		value_str = ptr_stringify(node -> key);
	} else {
		value_str = item_stringify(node -> value);
	}
	if (value_str == NULL) {
		value_str = calloc(sizeof(char), strlen(ITEM_NULL) + 1);
		memcpy(value_str, ITEM_NULL, sizeof(char) * strlen(ITEM_NULL));
	}

	item_str = calloc(sizeof(char), strlen(TEMPLATE_NODE) - 2 + strlen(key_str) + strlen(value_str) + 1);
	sprintf(item_str, TEMPLATE_NODE, key_str, value_str);

	free(key_str);
	free(value_str);

	return item_str;
}

char* HashMap_resize_items_str(char* items, const char* const item_str, int extra) {
	if (items == NULL) {
		const int new_length = strlen(item_str) + extra;
		items = calloc(sizeof(char), new_length);
	} else {
		const int new_length = strlen(items) + strlen(item_str) + extra;
		items = realloc(items, sizeof(char) * new_length);
	}
	return items;
}

char* HashMap_stringify(
		const void* const p_map,
		char* (*key_stringify)(const void* const key),
		char* (*item_stringify)(const void* const item)
) {
	if (p_map == NULL) {
		return NULL;
	}
	char* items = NULL;
	const HashMap_t* const map = (HashMap_t*) p_map;
	for (int i = 0; i < map -> capacity; i++) {
		const LinkedList_t* const list = *(map -> head + i);
		for (int j = 0; j < LinkedList_size(list); j++) {
			const MapNode_t* const node = LinkedList_get(list, j);
			if (node == NULL) {
				continue;
			}
			char* item_str = HashMapNode_stringify(node, key_stringify, item_stringify);

			const int old_length = items == NULL ? 0 :strlen(items);
			if (old_length == 0) {
				items = HashMap_resize_items_str(items, item_str, 1);
				sprintf(items + old_length, "%s", item_str);
			} else {
				items = HashMap_resize_items_str(items, item_str, 3);
				sprintf(items + old_length, ", %s", item_str);
			}
			free(item_str);
		}
	}

	char* result = NULL;
	if (items == NULL) {
		result = calloc(sizeof(char), strlen(TEMPLATE_HASHMAP_EMPTY) + 1);
		sprintf(result, TEMPLATE_HASHMAP_EMPTY);
	} else {
		result = calloc(sizeof(char), strlen(TEMPLATE_HASHMAP) - 2 + strlen(items) + 1);
		sprintf(result, TEMPLATE_HASHMAP, items);
		free(items);
	}
	return result;
}

size_t HashMap_hash(
		const void* const p_map,
		size_t (*key_hash)(const void* const key),
		size_t (*item_hash)(const void* const item)
) {
	if (p_map == NULL) {
		return 0;
	}
	size_t hash = 0;
	const HashMap_t* const map = (HashMap_t*) p_map;
	LinkedList_t* list = *(map -> head);
	for (int i = 0; i < map -> capacity; ++i) {
		for (int j = 0; j < LinkedList_size(list); j++) {
			const MapNode_t* const node = LinkedList_get(list, j);
			if (node == NULL) {
				continue;
			}
			size_t node_hash = 0;
			if (key_hash == NULL) {
				node_hash = default_hash(node -> key, sizeof(node -> key));
			} else {
				node_hash = key_hash(node -> key);
			}
			if (item_hash == NULL) {
				node_hash = default_hash_iteraction(node_hash + default_hash(node -> value, sizeof(node -> value)));
			} else {
				node_hash = default_hash_iteraction(node_hash + item_hash(node -> value));
			}
			hash = default_hash_iteraction(hash + node_hash);
		}
		list = *(map -> head + i);
	}
	return default_hash_end(hash);
}


int HashMap_node_compare(
		const void* const p_node1, const void* const p_node2,
		int (*key_compare)(const void* const key1, const void* const key2),
		int (*item_compare)(const void* const item1, const void* const item2)
) {
	if (p_node1 == NULL && p_node2 != NULL) {
		return 1;
	} else if (p_node1 != NULL && p_node2 == NULL) {
		return -1;
	}
	const MapNode_t* const node1 = (MapNode_t*) p_node1;
	const MapNode_t* const node2 = (MapNode_t*) p_node2;
	if (key_compare == NULL) {
		if (node1 -> key > node2 -> key) {
			return 1;
		} else if(node1 -> key < node2 -> key) {
			return -1;
		}
	} else {
		int res = key_compare(node1 -> key, node2 -> key);
		if (res != 0) {
			return res;
		}
	}
	if (item_compare == NULL) {
		if (node1 -> value > node2 -> value) {
			return 1;
		} else if(node1 -> value < node2 -> value) {
			return -1;
		}
	} else {
		int res = item_compare(node1 -> value, node2 -> value);
		if (res != 0) {
			return res;
		}
	}
	return 0;
}

int HashMap_compare(
		const void* const p_map1, const void* const p_map2,
		int (*key_compare)(const void* const key1, const void* const key2),
		int (*item_compare)(const void* const item1, const void* const item2)
) {
	if (p_map1 == NULL && p_map2 != NULL) {
		return 1;
	} else if (p_map1 != NULL && p_map2 == NULL) {
		return -1;
	}
	if (p_map1 == p_map2) {
		return 0;
	}
	const HashMap_t* const map1 = (HashMap_t*) p_map1;
	const HashMap_t* const map2 = (HashMap_t*) p_map2;

	if (map1 -> size != map2 -> size) {
		return map1 -> size > map2 -> size ? 1 : -1;
	}
	LinkedList_t* list1 = *(map1 -> head);
	LinkedList_t* list2 = *(map2 -> head);
	for (int i = 0; i < map1 -> capacity; ++i) {
		if (LinkedList_size(list1) != LinkedList_size(list2)) {
			return LinkedList_size(list1) > LinkedList_size(list2) ? 1 : -1;
		}
		for (int j = 0; j < LinkedList_size(list1); j++) {
			const MapNode_t* const node1 = LinkedList_get(list1, j);
			const MapNode_t* const node2 = LinkedList_get(list2, j);
			int res = HashMap_node_compare(node1, node2, key_compare, item_compare);
			if (res != 0) {
				return res;
			}
		}
		list1 = *(map1 -> head + i);
		list2 = *(map2 -> head + i);
	}

	return 0;
}

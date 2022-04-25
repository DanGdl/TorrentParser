
#include "parser_torrent.h"

#define TOKEN_END 'e'
#define TOKEN_LIST 'l'
#define TOKEN_INT 'i'
#define TOKEN_DICTIONARY 'd'

typedef struct TorrentInfo {
	Bencoded_t* map;
} TorrentInfo_t;


void TorrentInfo_free(TorrentInfo_t* info) {
    if (info == NULL) {
        return;
    }
    Bencoded_free(info -> map);
    free(info);
}

Bencoded_t* TorrentInfo_get_value_by_key(const TorrentInfo_t* const info, const char* const key) {
	if (info == NULL || key == NULL) {
		return NULL;
	}
	return Bencoded_get_value_by_key(info -> map, key);
}

size_t read_single_value(const char* blob, Bencoded_t** value);


size_t read_dictionary(const char* blob, Bencoded_t** value) {
    if (*blob != TOKEN_DICTIONARY) {
        printf("Error parsing list. Was expecting a 'd' but got %c\n", *blob);
        return 0;
    }

    HashMap_t* map = HashMap_create(Bencoded_hash);
	if (map == NULL) {
		printf("Failed to create map\n");
		return 0;
	}

    size_t idx = 1;
    char c;
    while ((c = *(blob + idx)) != TOKEN_END) {
    	Bencoded_t* key = NULL;
    	size_t counter = read_single_value(blob + idx, &key);
        if (key == NULL) {
        	printf("Failed to read map key\n");
        	HashMap_clear_and_free(map, Bencoded_free, Bencoded_free);
            return 0;
        } else {
            idx += counter;
        }
        Bencoded_t* value = NULL;
		counter = read_single_value(blob + idx, &value);
        if (value == NULL) {
            free(key);
            printf("Failed to read map value\n");
            HashMap_clear_and_free(map, Bencoded_free, Bencoded_free);
            return 0;
        } else {
            idx += counter;
        }
        HashMap_add(map, key, value);
    }

    if (*(blob + idx) != TOKEN_END) {
        printf("Read length of byte string and was expecting 'e' but got %c\n", *(blob + idx));
        HashMap_clear_and_free(map, Bencoded_free, Bencoded_free);
        return 0;
    }
    idx++; // Skip the 'e'

    *value = Bencoded_create(TYPE_MAP, map);
    if (*value == NULL) {
    	printf("Failed to create map bencoded\n");
    	HashMap_clear_and_free(map, Bencoded_free, Bencoded_free);
		return 0;
    }
    return idx;
}

size_t read_list(const char* blob, Bencoded_t** value) {
    if (*blob != TOKEN_LIST) {
        printf("Error parsing list. Was expecting a 'l' but got %c\n", *blob);
        return 0;
    }

    LinkedList_t* list = LinkedList_create();
	if (list == NULL) {
		printf("Failed to create list\n");
		return 0;
	}

    size_t idx = 1;
    char c;
    while ((c = *(blob + idx)) != TOKEN_END) {
        Bencoded_t* result = NULL;
		size_t count = read_single_value(blob + idx, &result);
        if (result == NULL) {
        	printf("Failed to create bencoded item\n");
        	LinkedList_clear_and_free(list, Bencoded_free);
            return 0;
        } else {
            LinkedList_add(list, result);
            idx += count;
        }
    }
    if (*(blob + idx) != TOKEN_END) {
        printf("Error parsing list. Was expecting a 'e' but got %c\n", *(blob + idx));
        LinkedList_clear_and_free(list, Bencoded_free);
        return 0;
    }
    idx++; // Skip the 'e'

    *value = Bencoded_create(TYPE_LIST, list);
    if (*value == NULL) {
    	printf("Failed to create bencoded for list\n");
    	LinkedList_clear_and_free(list, Bencoded_free);
    	return 0;
    }
    return idx;
}

size_t read_string(const char* blob, Bencoded_t** value) {
    // Build up a string of ascii chars representing the size.
    const int page = 200;
    int pages = 0;
    char* buffer = NULL;

    int idx = 0;
    char c = *(blob + idx);
    while(c >= 48 && c <= 57) {
        if (pages * page <= idx) {
            pages++;
            if (buffer == NULL) {
                 buffer = calloc(sizeof(*buffer), pages * page);
                 if (buffer == NULL) {
                    printf("Failed to allocate memory for string\n");
                    return 0;
                 }
            } else {
                buffer = realloc(buffer, pages * page * sizeof(*buffer));
                if (buffer == NULL) {
                    printf("Failed to reallocate memory for string\n");
                    return 0;
                 }
            }
        }

        *(buffer + idx) = c;

        idx++;
        c = *(blob + idx);
    }
    *(buffer + idx) = '\0';

    if (*(blob + idx) != ':') {
        free(buffer);
        printf("Read length of byte string and was expecting ':' but got %c\n", *(blob + idx));
        return 0;
    }
    idx++;

    const int length = atoi(buffer);
    free(buffer);

    buffer = calloc(sizeof(*buffer), length + 1);
    if (buffer == NULL) {
        printf("Failed to allocate memory for string\n");
        return 0;
    }

    // Read the actual data
    for (int i = 0; i < length; i++) {
        *(buffer + i) = *(blob + idx);
        idx++;
    }
    *value = Bencoded_create(TYPE_STRING, buffer);
	if (*value == NULL) {
		printf("Failed to create string bencoded\n");
		free(buffer);
		return 0;
	}

    return idx;
}

size_t read_int(const char* blob, Bencoded_t** value) {
    if (*blob != TOKEN_INT) {
        printf("Error parsing integer. Was expecting a 'i' but got %c\n", *blob);
        return 0;
    }

    const int page = 200;
    int pages = 0;
    char* buffer = NULL;

    int idx = 1;
    char c = *(blob + idx);
    while (c == 45 /*negative mark*/ || (c >= 48 && c <= 57 /*ASCII numbers*/)) {
        if (pages * page <= idx) {
            pages++;
            if (buffer == NULL) {
                 buffer = calloc(sizeof(*buffer), pages * page);
                 if (buffer == NULL) {
                    printf("Failed to allocate memory for number\n");
                    return 0;
                 }
            } else {
                buffer = realloc(buffer, pages * page * sizeof(*buffer));
                if (buffer == NULL) {
                    printf("Failed to reallocate memory for number\n");
                    return 0;
                 }
            }
        }
        *(buffer + idx - 1) = c;
        idx++;
        c = *(blob + idx);
    }

    if (*(blob + idx) != TOKEN_END) {
        printf("Error parsing integer. Was expecting a 'e' but got %c\n", *(blob + idx));
        free(buffer);
        return 0;
    }

    idx++; // Skip the 'e'

    long long* number;
    number = calloc(sizeof(*value), 1);
    if (number == NULL) {
        printf("Failed to allocate memory\n");
        free(buffer);
        return 0;
    }
    if (strcmp(buffer, "0") == 0) {
    	*number = 0;
    } else {
		*number = strtoll(buffer, NULL, 10);
		if (*number == 0 && errno == EINVAL) {
			printf("Provided string is not a number %s\n", buffer);
			free(buffer);
			free(number);
			return 0;
		} else if (*number == LLONG_MIN || (*number == LLONG_MAX && errno == ERANGE)) {
			printf("Provided number is too big %s\n", buffer);
			free(buffer);
			free(number);
			return 0;
		}
    }
    free(buffer);

    *value = Bencoded_create(TYPE_INT, number);
   	if (*value == NULL) {
   		printf("Failed to create number bencoded\n");
   		free(number);
   		return 0;
   	}
    return idx;
}


size_t read_single_value(const char* blob, Bencoded_t** value) {
    switch (*blob) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return read_string(blob, value);

        case TOKEN_DICTIONARY:
            return read_dictionary(blob, value);

        case TOKEN_INT:
            return read_int(blob, value);

        case TOKEN_LIST:
            return read_list(blob, value);

        default: {
            printf("Unsupported type %c\n", *blob);
            return 0;
        }
    }
}



int parse_torrent(const char* const path, TorrentInfo_t** info) {
    char* real_path = realpath(path, NULL);
    printf("Parsing file %s\n", real_path);

    int fd = open(real_path, O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        printf("Failed to open file %s\n", real_path);
        return -1;
    }

    struct stat sb;
    if (stat(path, &sb) == -1) {
        printf("Failed to get file info %s\n", real_path);
        close(fd);
        free(real_path);
        return -1;
    }
    // size_t pagesize = getpagesize();
    // printf("Page size %zu\n", pagesize);
    char* mapped = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        printf("Failed to map file to memory\n");
        close(fd);
        free(real_path);
        return -1;
    }

	Bencoded_t* result = NULL;
	size_t count = read_single_value(mapped, &result);
	if (result != NULL) {
		*info = calloc(sizeof(TorrentInfo_t), 1);
		(*info) -> map = result;
	}
	int unmap = munmap(mapped, sb.st_size);
	if (unmap != 0) {
		printf("Could not munmap");
	}
    close(fd);
    free(real_path);
    return count == 0 ? -1 : 0;
}

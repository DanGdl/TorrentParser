
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>     // getcwd
#include <dirent.h>     // check directories
#include <sys/stat.h>   // check file info
#include <errno.h>
#include <limits.h>

#include "parser_torrent.h"

void rename_torrent_file(const TorrentInfo_t* const info, const char* const file_path, const char* const path, const char* const file_name) {
    Bencoded_t* value = TorrentInfo_get_value_by_key(info, "name");
    if (Bencoded_is_text(value)) {
        char* name = (char*) Bencoded_get_value(value);
        char* new_filename = calloc(sizeof(*name), strlen(path) + strlen(name) + strlen(SUFFIX_TORRENT) + 2);

        if (new_filename == NULL) {
            printf("Failed to allocate memory for new path\n");
            return;
        }
        else {
            sprintf(new_filename, "%s/%s%s", path, name, SUFFIX_TORRENT);

            if (strcmp(file_path, new_filename) == 0) {
                free(new_filename);
                return;
            }
            if (rename(file_path, new_filename) == 0) {
                printf("%s renamed to %s\n", file_name, new_filename);
            } else {
                printf("Failed to rename file from %s to %s, error %s\n", file_name, new_filename, strerror(errno));
            }
            free(new_filename);
        }
    } else {
        printf("Value \"name\" is not text in %s\n", file_name);
    }
}

void check_file_in_path(const char* const path) {

    struct dirent* file;
	struct stat stats;
	DIR* dir = NULL;

	dir = opendir(path);
	if (dir == NULL) {
		printf("Directory cannot be opened!\n");
		return;
	}
	while ((file = readdir(dir)) != NULL) {
		const char* file_name = file -> d_name;
        if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0) {
            continue;
        }
        char* file_path = realpath(file_name, NULL);
        if (file_path == NULL) {
            file_path = calloc(sizeof(*file_path), strlen(path) + strlen(file_name) + 2);
            sprintf(file_path, "%s/%s", path, file_name);
        }

		if (stat(file_path, &stats) != 0) {
            printf("Can't get state for file %s, path %s\n", file_name, file_path);
            free(file_path);
            continue;
        }
        if (S_ISDIR(stats.st_mode)) {
            check_file_in_path(file_path);
        }
        else if (strstr(file_name, SUFFIX_TORRENT)) {
            TorrentInfo_t* info;
            if (parse_torrent(file_path, &info) == 0) {
                rename_torrent_file(info, file_path, path, file_name);
                TorrentInfo_free(info);
            }
        }
        free(file_path);
	}
	closedir(dir);
}

int main(int argc, char* argv[]) {
    char* path = NULL;
    if (argc == 2) {
        path = argv[1];
    } else {
        path = ".";
    }
    check_file_in_path(path);
    return EXIT_SUCCESS; // from stdlib
}

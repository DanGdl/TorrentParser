
#ifndef PARSER_TORRENT_H_
#define PARSER_TORRENT_H_

#include <sys/stat.h>   // check file info
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <limits.h>
#include <sys/mman.h>	// mmap
#include <fcntl.h>		// open
#include <unistd.h>		// close

#include "linked_list.h"
#include "hash_map.h"
#include "bencoded.h"

#define SUFFIX_TORRENT ".torrent"

typedef struct TorrentInfo TorrentInfo_t;

Bencoded_t* TorrentInfo_get_value_by_key(const TorrentInfo_t* const info, const char* const key);

int parse_torrent(const char* const path, TorrentInfo_t** info);

void TorrentInfo_free(TorrentInfo_t* info);

#endif

torrent:
	gcc -ggdb util_torrent.c parser_torrent.c linked_list.c hash_map.c bencoded.c util.c -o util_torrent

all: torrent
	
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions torrent

.PHONY: clean 

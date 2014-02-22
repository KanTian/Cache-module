#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#include "tm_global.h"
#include "tm_hash.h"

unsigned tm_hash(const char *str, unsigned table_size){
	unsigned long hash = 5381;
	int c;
	while(c = *str++){
		hash = ((hash << 5) + hash) + c;
	}
	hash = table_size > 0 ? hash % table_size : hash;
	return hash;
}

struct tm_hash_t *tm_hcreate(unsigned table_size){
	struct tm_hash_t *hashtable;
	int i;

	if(table_size < 1){
		return NULL;
	}
	if((hashtable = malloc( sizeof(struct tm_hash_t)) ) == NULL){
		return NULL;
	}
	if((hashtable->table = malloc(table_size * sizeof(struct tm_hash_entry_t *))) == NULL){
		return NULL;
	}
	for(i = 0; i< table_size; i++){
		hashtable->table[i] = NULL;
	}
	hashtable->size = table_size;
	hashtable-total = 0;
	return hashtable;
}

struct tm_hinsert(struct tm_hash_t *hashtable, char *key, char *data, unsigned length, unsigned lifetime, short mode ){
	unsigned hash;
	struct tm_hash_entry_t *entry, *tmpentry;
	time_t currtime;

	hash = tm_hash(key, hashtable->size - 1);

	if(hash < 0 || hash >= hashtable->size){
		return ERROR;
	}

	if(tm_hfind(hashtable, key) != NULL){
		if(mode == MODE_ADD){
			return SUCCESS;
		}else{
			tm_hremove(hashtable, key);
		}
	}
	if((entry = malloc( sizeof(struct tm_hash_entry_t))) == NULL){
		return ERROR;
	}

	currtime = time( (time_t *)NULL);

	entry->key = key;
	entry->data = data;
	entry->length = length;
	entry->created = currtime;
	entry->expired = currtime + lifetime;
	entry->next = hashtable->table[hash] != NULL ? hashtable->table[hash] : NULL;
	hashtable->table[hash] = entry;
	return OK;
}

struct tm_hash_entry_t  *tm_hfind(struct tm_hash_t *hashtable, char *key){
	unsigned hash;
	struct tm_hash_entry_t *entry;

	hash = tm_hash(key, hashtable->size - 1);

	if(hash < 0 || hash >= hashtable->size){
		return NULL;
	}

	for(entry = hashtable->table[hash]; entry != NULL; entry = entry->next){
		if(strcmp(entry->key, key) == 0){
			return entry;
		}
	}
	return NULL;
}

status tm_hremove(struct tm_hash_t *hashtable, char *key){
	unsigned hash;
	struct tm_hash_entry_t *entry, *tmpentry;

	hash = tm_hash(key, hashtable->size - 1);
	entry = hashtable->table[hash];

	if(entry && strcmp(entry->key, key) == 0){
		tmpentry = entry->next;
		free(entry);
		hashtable->table[hash] = tmpentry;
		return OK;
	}else{
		while(entry){
			if(entry->next && strcmp(entry->next->key, key) == 0){
				tmpentry = entry->next;
				entry->next = entry->next->next;
				free(tmpentry);
				return OK;
			}
			entry = entry->next;
		}
	}
	return ERROR;
}

status tm_hdestroy( struct tm_hash_t *hashtable){
	int i;
	struct tm_hash_entry_t *entry, *prev;

	for(i = 0; i < hashtable->size;i++){
		entry = hashtable->table[i];
		while(entry){
			prev = entry;
			entry = entry->next;
			free(prev);
		}
	}
	free(hashtable->table);
	free(hashtable);
	return OK;
}

void tm_hvisit(struct tm_hash_t *hashtable){
	int i;
	assert(hashtable != NULL);
	printf("\n[Debug]\n Visit hashtable start...\n");
	for(i=0;i<hashtable->size;i++){
		if(hashtable->table[i] != NULL){
			printf("Item %d not null, key:%s data:%s addr:%d\n", i, hashtable->table[i]->key,hashtable->table[i]->data, hashtable->table[i]);
		}
	}
	printf("Visit hashtable done.\n\n");
}

status tm_hprint(struct tm_hash_entry_t *node){
	if(node == NULL){
		printf("[Debug] Hash Node Print: the node is null");
		return ERROR;
	}
	printf("\n[Debug]Hash Node Print: key %s, data:%s,length:%d, created:%d,expired:%d\n");
	return OK;
}
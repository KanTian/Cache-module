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
	if((hashtable = malloc(sizeof(struct tm_hash_t))) == NULL){
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
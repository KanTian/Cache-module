#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#include "tm_global.h"
#include "tm_queue.h"

struct tm_queue_t *tm_qcreate(){
	struct tm_queue_t *queuelist;
	struct tm_queue_node_t *node;

	queuelist = (struct tm_queue_t *)malloc( sizeof(struct tm_queue_t));
	queuelist->front = queuelist->rear = (struct tm_queue_node_t *)malloc( sizeof(struct tm_queue_node_t));

	if(!queuelist->front){
		return NULL;
	}
	queuelist->total = queuelist->memsize = 0;
	queuelist->front->next = NULL;
	return queuelist;
}

status tm_qempty(struct tm_queue_t *queuelist){
	if(queuelist->front == queuelist->rear){
		return TRUE;
	}
	return FALSE;
}

unsigned tm_qlength( struct tm_queue_t *queuelist){
	if(queuelist == NULL){
		return ERROR;
	}
	return queuelist->total;
}

unsigned tm_qmemsize(struct tm_queue_t *queuelist){
	if(queuelist == NULL){
		return ERROR;
	}
	return queuelist->memsize;
}

status tm_qentry(struct tm_queue_t *queuelist, char *key, unsigned data_len){
	struct tm_queue_node_t *node;
	node = (struct tm_queue_node_t *)malloc(sizeof(struct tm_queue_node_t));

	if(!node){
		return ERROR;
	}

	node->key = key;
	node->data_len = data_len;

	queuelist->total ++;
	queuelist->memsize += data_len;
	queuelist->rear->next = node;
	queuelist->rear = node;
	return OK;
}

struct tm_queue_node_t *tm_qremove(struct tm_queue_t *queuelist){
	struct tm_queue_node_t *node;
	if(queuelist->front == queuelist->rear){
		return NULL;
	}
	node = queuelist->front->next;
	queuelist->front->next = node->next;
	if(queuelist->rear == node){
		queuelist->rear = queuelist->front;
	}
	queuelist->total --;
	queuelist->memsize -= node->data_len;

	return node;
}

status tm_qdestroy(struct tm_queue_t *queuelist){
	//struct tm_queue_node_t *temp;
	if(queuelist == NULL){
		return ERROR;
	}
	while(queuelist->front){
		queuelist->rear = queuelist->front->next;
		free(queuelist->front);
		queuelist->front = queuelist->rear;
	}
	return OK;
}

void tm_qvisit(struct tm_queue_t *queuelist){
	int i = 0;
	struct tm_queue_node_t *node;

	assert(queuelist != NULL);
	node = queuelist->front->next;

	printf("\n------------- Queue list debug ---------\n");
	printf("Queue List Total: %d\n", queuelist->total);
	printf("Queue List Memsize: %d\n", queuelist->memsize);
	for(;node;i++,node = node->next){
		printf("Num:%d Key:%s Length:%d Addr:%d NextAddr:%d\n",i, node->key, node->data_len, node, node->next);
	}
	printf("\n");
}
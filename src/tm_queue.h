struct tm_queue_node_t{
	char *key;
	unsigned data_len;
	struct tm_queue_node_t *next;
};

struct tm_queue_t{
	unsigned total;
	unsigned memsize;
	struct tm_queue_node_t *front;
	struct tm_queue_node_t *rear;
};

//create queue list
struct tm_queue_t *tm_qcreate();

//check queue is empty
status tm_qempty(struct tm_queue_t *queuelist);

//get queue list length
unsigned tm_qlength(struct tm_queue_t *queuelist);

//get queue list memory size bytes
unsigned tm_qmemsize(struct tm_queue_t *queuelist);

//push a node into the bottom of the queue
status tm_qentry(struct tm_queue_t *queuelist, char *key, unsigned data_len);

//pop a node the top of queue
struct tm_queue_node_t *tm_qremove(struct tm_queue_t *queuelist);

//destory queue list
struct tm_qdestroy(struct tm_queue_t *queuelist);

//visit queue list
void tm_qvisit(struct tm_queue_t *queuelist);

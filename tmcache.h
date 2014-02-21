struct tm_status{
	char version[32];
	pid_t pid;
	time_t start_time;
	time_t run_time;
	unsigned mem_total;
	unsigned mem_used;
	unsigned item_total;
	unsigned visit_add;
	unsigned visit_del;
	unsigned visit_get;
};

//print status
void print_status();

//init status
void init_status(unsigned max_memsize);

//settint status
void set_status(unsigned mem_used, unsigned item_total,unsigned visit_add, unsigned visit_del, unsigned visit_get);

//get status
struct tm_status *get_status();

//get memory used size
unsigned get_mem_used();

//fetch status
struct tm_status *fetch_status();


//Data operate function define

//remove data
status remove_data(char *key);

//flush expired data probability
status get_gc_probability(unsigned probability,unsigned divisor);

//flush all data
status flush_data();

//store data
status store_data();

//fetch data
status tm_hash_entry_t *fetch_data(char *key);

/**
 * process function defined 
 */
 //usage message
void usage(char *exefile);

//output environment and configure information
void print_config();

//send message to client
void send_msg(int client_sock, char *msg, int length);

//send error message to client
void send_error(int client_sock, short error_type, char *msg);

//process client request
int proc_request(int client_sock);

/*handle a client */
//void handle_client(int client_sock);

//init server socket listen
static void init_server_listen(unsigned int port, unsigned int max_client);

//parse cmd options 
int parse_options(int argc, char *argv[]);
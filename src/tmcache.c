#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <dirent.h>
#include <time.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netint/tcp.h>
#include <arpa/inet.h>
#define _GNU_SOURCE
#include <pthread.h>

#include "tm_global.h"
#include "tm_common.h"
#include "tm_hash.h"
#include "tm_queue.h"
#include "tmcache.h"

//调试之用
unsigned short g_is_debug = IS_DEBUG;
unsigned short g_is_daemon = IS_DAEMON;
unsigned int g_port = PORT;
//最大客户端数目
unsigned int g_max_client = MAX_CLIENTS;
//最大内存size
unsigned int g_max_mem_size = MAX_MEM_SIZE;
//最大表size
unsigned int g_max_tablesize = MAX_TABLE_SIZE;

static struct tm_hash_t *g_htable;
static struct tm_queue_t *g_qlist;
static struct tm_status *g_status;

void print_status(){
	char buf[8192];
	struct tm_status *status;
	int visit_total;

	status = g_status;
	visit_total = status->visit_add + status->visit_del + status->visit_get;

	sprintf(buf, "=============Status===============\n\
	STAT version %s\r\n\
	STAT pid %d\r\n\
	STAT start_time %d\r\n\
	STAT run_time %d\r\n\
	STAT mem_total %d\r\n\
	STAT mem_used %d\r\n\
	STAT item_total %d\r\n\
	STAT visit_total %d\r\n\
	STAT visit_add %d\r\n\
	STAT visit_del %d\r\n\
	STAT visit_get %d\r\n\
	===============END================\n", 
	status->version, status->pid, status->start_time, status->run_time, 
	status->mem_total, status->mem_used, status->item_total, 
	visit_total, status->visit_add, status->visit_del, status->visit_get);

	printf("%s", buf);
}

void init_status(unsigned max_mimsize){
	g_status = (struct tm_status *)malloc(sizeof(struct tm_status));
	memset(g_status, 0, sizeof(struct tm_status));
	sprintf(g_status->version,"%s",VERSION);
	g_status->pid = getpid();
	g_status->start_time = time((time_t *)NULL);
	g_status->run_time = 0;
	g_status->mem_total = max_memsize;
	g_status->mem_used = 0;
	g_status->item_total  = 0;
	g_status->visit_add = 0;
	g_status->visit_del = 0;
	g_status->visit_get = 0;
}

void set_status(unsigned mem_used, unsigned item_total, unsigned visit_add, unsigned visit_del, unsigned visit_get){
	time_t currtime;
	currtime = time((time_t *)NULL);

	g_status->run_time = currtime - g_status->start_time;
	g_status->mem_used += mem_used;
	g_status->item_total += item_total;
	g_status->visit_add += visit_add;
	g_status->visit_del += visit_del;
	g_status->visit_get += visit_get;
}

struct tm_status *get_status(){
	time_t currtime;
	currtime = time((time_t *)NULL);
	g_status->run_time = currtime - g_status->start_time;
	return g_status;
}

unsigned get_mem_used(){
	unsigned ret;
	ret = g_status->mem_used;
	return ret;
}

struct tm_status *fetch_status(){
	struct tm_status *status;
	status = g_status;
	return status;
}

status store_data(char *key, char *data, unsigned length, unsigned lifetime, short mode){
	status ret;
	if((get_mem_used() + length) > g_max_mem_size){
		struct tm_queue_node_t *qnode;
		while((get_mem_used() + length) > g_max_mem_size){
			qnode = tm_qremove(g_qlist);
			remove_data(qnode->key);
		}
	}

	ret = tm_hinsert(g_htable, key, data, length, lifetime, mode);
	if(g_is_debug){
		printf("Key:%s, data:%s, length:%d, lifetime:%d, mode %d\n",key, data,length,lifetime,mode );
	}

	if(ret == SUCCESS || ret == ERROR){
		return FALSE;
	}

	set_status(length, 1, 1, 0, 0);

	if(get_gc_probability(GC_PROBABILITY, GC_DIVISOR) == TRUE){
		flush_expire_data();
	}
	return TRUE;
}

struct tm_hash_entry_t *fetch_data(char *key){
	struct tm_hash_entry_t *hnode;

	hnode = tm_hfind(g_htable, key);

	set_status(0, 0, 0, 0, 1);

	return hnode;
}

struct remove_data(char *key){
	status ret;
	int length;
	struct tm_hash_entry_t *hnode;

	if((hnode = tm_hfind(g_htable, key)) == NULL){
		return FALSE;
	}

	length = 0 - hnode->length;
	set_status(length, -1, 0, -1, 0);

	ret = tm_hremove(g_htable, key);

	return TRUE;
}

status flush_data(){
	tm_hdestroy(g_htable);
	tm_qdestroy(g_qlist);

	g_htable = tm_hcreate(g_max_tablesize);
	g_qlist = tm_qcreate();

	init_status(g_max_mem_size);

	return TRUE;
}

status flush_expire_data(){
	int i;
	time_t currtime;
	char *key;

	currtime = time((time_t *)NULL);
	for(i=0;i<g_htable->size;i++){
		if(g_htable->table[i] != NULL && g_htable->table[i]->expire < currtime){
			remove_data(g_htable[i]->key);
		}
	}
	return TRUE;
}

status get_gc_probability(unsigned probability, unsigned divisor){
	int n;
	struct timeval tv;
	gettimeofday(&v, (struct timezone *)NULL);
	srand((int)(tv.tv_usec + tv.tv_sec));
	n = 1 + (int)((float)divisor * rand() / (RAND_MAX + 1.0));
	return (n <= probability ? TRUE : FALSE);
}

void usage(char *exefile){
	/* Print copyright information */
	fprintf(stderr, "#===============================================\n");
	fprintf(stderr, "# tmcache - TieMa(Tiny&Mini) Memory Cache Server\n");
	fprintf(stderr, "# Version %s\n", VERSION);
	fprintf(stderr, "# \n");
	fprintf(stderr, "# heiyeluren <blog.csdn.net/heiyeshuwu>\n");
	fprintf(stderr, "#===============================================\n\n");
    fprintf(stderr, "usage: %s [OPTION] ... \n", exefile);

	/* Print options information */
	fprintf(stderr, "\nOptions: \n\
  -p <num>	port number to listen on,default 11211\n\
  -d		run as a daemon, default No\n\
  -m <num>	max memory to use for items in megabytes, default is 16M\n\
  -c <num>	max simultaneous connections, default is 1024\n\
  -v		print version information\n\
  -h		print this help and exit\n");

	/* Print example information */
	fprintf(stderr, "\nExample: \n  %s -p 11211 -m 16 -c 1024 -d\n", exefile);

}


void print_config(){
	fprintf(stderr, "===================================\n");
	fprintf(stderr, " tmcache Configure information\n");
	fprintf(stderr, "===================================\n");
	fprintf(stderr, "Is-Debug\t = %s\n", g_is_debug ? "Yes" : "No");
	fprintf(stderr, "Is-Daemon\t = %s\n", g_is_daemon ? "Yes" : "No");
	fprintf(stderr, "Port\t\t = %d\n", g_port);
	fprintf(stderr, "Max-Client\t = %d\n", g_max_client);
	fprintf(stderr, "Max-MemSize\t = %d\n", g_max_mem_size);
	fprintf(stderr, "===================================\n\n");
}

	/* Parse every options */
	while ( (opt = getopt_long(argc, argv, ":dp:m:c:hv", longopts, NULL)) != -1){
		switch (opt){
			case 'h': 
				usage(argv[0]); 
				return(-1);
				break;
			case 'v':
				fprintf(stderr, "%s %s\n", SERVER_NAME, VERSION);
				return(-1);
				break;
			case 'd': g_is_daemon = 1; break;
			case 'p':
				g_port = atoi(optarg);
				if ( g_port < 1 || g_port > 65535 ){
					fprintf(stderr, "Options -p error: input port number %s invalid, must between 1 - 65535.\n\n", optarg);
					return(-1);
				}
				break;
			case 'c':
				g_max_client = atoi(optarg);
				if ( !isdigit(g_max_client) ){
					fprintf(stderr, "Options -c error: input clients %s invalid, must number, proposal between 32 - 2048.\n\n", optarg);
					return(-1);
				}
				break;
			case 'm':
				if ( !is_numeric(optarg) || atoi(optarg) < 1 ){
					fprintf(stderr, "Options -m error: input memory size %s invalid, recommend 16 - 2048 (unit is MB, megabytes)\n\n", optarg);
					return(-1);
				}
				g_max_mem_size = atoi(optarg) * 1024 * 1024;
				break;
		}
	}

	return(0);
}

void send_msg(int client_sock, char *msg, int length){
	int len;
	len = length > 0 ? length : strlen(msg);
	write(client_sock, msg, len);
}

void send_error(int client_sock, short error_type, char *msg){
	char buf[BUFFER_SIZE];
	memset(buf, 0, BUFFER_SIZE);
	switch(error_type){
		case E_GENERAL:	
			sprintf(buf, "ERROR\r\n");break;
		case E_CLIENT:	
			sprintf(buf, "CLIENT_ERROR %s\r\n", msg);break;
		case E_SERVER:	
			sprintf(buf, "SERVER_ERROR %s\r\n", msg);break;
		default: 
			sprintf(buf, "ERROR\r\n");

	}
	send_msg(client_sock, buf, 0);
}

int proc_request( int client_sock ){//, struct sockaddr_in client_addr ){
    char head[BUFFER_SIZE], **head_arr, *method;
	int head_num;
	FILE *fp;

	/* Read & explode header */
	fp = fdopen(client_sock, "r");
	memset(head, 0, sizeof(head));
	if ( fgets(head, BUFFER_SIZE, fp) == NULL ){
		send_error(client_sock, E_SERVER, "not recv client message");
		return FALSE;
	}
	explode(head, ' ', &head_arr, &head_num);
	method  = trim( strtolower( head_arr[0] ) );

	/* debug message */
	if ( g_is_debug ){
		printf("Method:%s| Header:%s", method, head);
	}

	/**
	 * method: store 
	 * format: <cmd> <key> [<flag>] <expire> <length>\r\n
	 *		   cmd: set/add/replace
	 */
	if ( strcmp(method, "set")==0 || strcmp(method, "add")==0 || strcmp(method, "replace")==0 || strcmp(method, "append")==0 ){
		if ( head_num != 5 && head_num != 4 ){
			send_error(client_sock, E_GENERAL, "");
			return FALSE;
		}
		/* store variables */
		short mode;
		char *key, *data;
		unsigned length, lifetime;

		/* pick every store item */
		if ( head_num == 5 ){
			lifetime = atoi(trim(head_arr[3]));
			length   = atoi(trim(head_arr[4]));
		} 
        else {
			lifetime = atoi(trim(head_arr[2]));
			length	 = atoi(trim(head_arr[3]));
		}
		
		/* data too large */
		if ( length > MAX_BUF_SIZE ){
			send_error(client_sock, E_CLIENT, "data too large, to exceed MAX_BUF_SIZE" );
			return FALSE;
		}

		/* item assign */
		key		 = head_arr[1];
		data	 = (char *)malloc(length + 1);
		lifetime = lifetime > MAX_LIFE_TIME ? MAX_LIFE_TIME : lifetime;
		mode	 = ( strcmp(method, "add")==0 || strcmp(method, "append")==0 ? MODE_ADD : 
				   ( strcmp(method, "replace")==0 ? MODE_REPLACE : MODE_SET ) );

		/* debug message */
		if ( g_is_debug ){
			printf("Store method, Key: %s | Length: %d | Lifetime: %d\n", key, length, lifetime);
		}

		/* read data (data rear \r\n) */
		if ( fread(data, 1, length + 1, fp) <= 0 ){
			send_error(client_sock, E_CLIENT, "data too short, not match LENGTH");
			return FALSE;
		}

		/* store data */
		if ( store_data( key, data, length, lifetime, mode ) == TRUE ){
			send_msg(client_sock, "STORED\r\n", 0);
		} 
        else {
			send_msg(client_sock, "NOT_STORED\r\n", 0);
		}

		struct tm_hash_entry_t *node;
		node = tm_hfind(g_htable, key);

        if (g_is_debug){
            printf("[Node info] key:%s, data:%s, length:%d, created:%d, expred:%d, next:%d\n", node->key, node->data, node->length, node->created, node->expired, node->next);
        }


		return OK;
	}

	/**
	 * method: fetch 
	 * format: <cmd> <key>\r\n
	 *		   cmd: get/fetch
	 */
	else if ( strcmp(method, "get")==0 || strcmp(method, "fetch")==0 || strcmp(method, "gets")==0 ){
		if ( head_num != 2 ){
			send_error(client_sock, E_GENERAL, "");
			return FALSE;
		}
		char *key, buf[BUFFER_SIZE];
		struct tm_hash_entry_t *hnode;
		time_t currtime;

		key = trim(head_arr[1]);
		hnode = fetch_data( key );
        if ( hnode == NULL ){
			send_msg(client_sock, "END\r\n", 0);
            return FALSE;
        }
        if (g_is_debug){
            printf("FIND KEY: %s  DATA: %s LENGTH: %d CREATED: %d EXPIRED: %d\n", hnode->key, hnode->data, hnode->length, hnode->created, hnode->expired);
        }

		/* item out expire time, remove it */
		currtime = time((time_t *)NULL);
		if ( hnode->expired < currtime ){
			remove_data( key );
			send_msg(client_sock, "END\r\n", 0);
			return FALSE;
		}

		/* send data to client */
		memset(buf, 0, BUFFER_SIZE);
		sprintf(buf, "VALUE %s 0 %d\r\n", hnode->key, hnode->length);
		send_msg(client_sock, buf, 0);
		send_msg(client_sock, hnode->data, hnode->length);
		send_msg(client_sock, "\r\nEND\r\n", 0);
		return TRUE;
	}

	/**
	 * method: delete 
	 * format: <cmd> <key> [<longtime>]\r\n
	 *		   cmd: delete/del/remove
	 */
	else if ( strcmp(method, "delete")==0 || strcmp(method, "del")==0 || strcmp(method, "remove")==0 || strcmp(method, "rm")==0){
		if ( head_num != 2 && head_num != 3 ){
			send_error(client_sock, E_GENERAL, "");
			return FALSE;
		}
		char *key;

		key = trim(head_arr[1]);
        //printf("Remove data.\n");
		if ( remove_data( key ) == FALSE ){
			send_msg(client_sock, "NOT_FOUND\r\n", 0);
			return FALSE;
		} 
        else {
			send_msg(client_sock, "DELETED\r\n", 0);
			return TRUE;
		}
	}
		
	/**
	 * method: flush_all 
	 * format: <cmd>\r\n
	 *		   cmd: flush_all/flush
	 */	
	else if ( strcmp(method, "flush_all")==0 || strcmp(method, "flush")==0 ){
		if ( head_num != 1 ){
			send_error(client_sock, E_GENERAL, "");
			return FALSE;
		}
		flush_data();
		send_msg(client_sock, "OK\r\n", 0);
		return TRUE;
	}
	
	/**
	 * method: stats 
	 * format: <cmd> [<option>]\r\n
	 *		   cmd: stats/state/stat/status
	 */	
	else if ( strcmp(method, "stats")==0 || strcmp(method, "state")==0 || strcmp(method, "stat")==0 || strcmp(method, "status")==0 ){
		if ( head_num != 1 && head_num != 2 ){
			send_error(client_sock, E_GENERAL, "");
			return FALSE;
		}
		char buf[BUFFER_SIZE], line[1024];
		struct tm_status *status;
		int visit_total;

		status = get_status();
		visit_total = status->visit_add + status->visit_del + status->visit_get;
		memset(buf, 0, BUFFER_SIZE);

		sprintf(buf, "\
STAT pid %d\r\n\
STAT start_time %d\r\n\
STAT run_time %d\r\n\
STAT version %s\r\n\
STAT mem_total %d\r\n\
STAT mem_used %d\r\n\
STAT item_total %d\r\n\
STAT visit_total %d\r\n\
STAT visit_add %d\r\n\
STAT visit_del %d\r\n\
STAT visit_get %d\r\n\
END\r\n", 
		status->pid, status->start_time, status->run_time, status->version, 
		status->mem_total, status->mem_used, status->item_total, 
		visit_total, status->visit_add, status->visit_del, status->visit_get);

		send_msg(client_sock, buf, 0);
		return TRUE;
	}

	/**
	 * method: version 
	 * format: <cmd>\r\n
	 *		   cmd: version/ver
	 */	
	else if ( strcmp(method, "version")==0 || strcmp(method, "ver")==0 ){
		if ( head_num != 1 ){
			send_error(client_sock, E_GENERAL, "");
			return FALSE;
		}
		unsigned short version_len = strlen(VERSION) + 2 + 8;
		char version[version_len];
		sprintf(version, "VERSION %s\r\n", VERSION);
		send_msg(client_sock, version, version_len);
		//send_msg(client_sock, VERSION, 0);
		//send_msg(client_sock, "\r\n", 0);
		return TRUE;
	}

	/**
	 * method: quit 
	 * format: <cmd>\r\n
	 *		   cmd: quit/exit
	 */	
	else if ( strcmp(method, "quit")==0 || strcmp(method, "exit")==0 ){
		if ( head_num != 1 ){
			send_error(client_sock, E_GENERAL, "");
			return FALSE;
		}
        return EXIT;
	 }

	/**
	 * exception
	 */
	else {
		send_error(client_sock, E_CLIENT, "That method is not implemented");
		return FALSE;
	}

}



/**
 * Handle a client
 *
 */
void handle_client( int client_sock){//, struct sockaddr_in client_addr ){
    int nread;
    int ret;

    while(1){
        /* Not read data */
        ioctl(client_sock, FIONREAD, &nread);
        if ( nread > 0 ){
            ret = proc_request( client_sock);//, client_addr );
            if (ret == EXIT){
                close(client_sock);
                break;
            }
            //printf("proc_requrest result %d\n", ret);
        }
        //sleep(1);
    }
}



/**
 * Thread callback function
 *
 */
void *tm_thread_callback(void *arg) {
    int *clientsock;
    clientsock = (int *)arg;
    handle_client(*clientsock);//, (struct sockaddr_in *)NULL);
    //close(*clientsock);
    if (g_is_debug){
        printf("thread close socket id: %d\n", *clientsock);
        printf("thread exit\n");
    }
    pthread_exit(NULL);
    return NULL;
}


/**
 * Posix thread process new connection
 *
 */
void tm_thread( int serversock, unsigned int max_client ){
    int clientsock, *arg;
    struct sockaddr_in client_addr;
    char currtime[32];
    unsigned clientlen;
    pthread_attr_t thread_attr;
    void *thread_result;
    
    /* Setting pthread attribute */
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    /* Run until cancelled */
    while (1){
        pthread_t thread;
        unsigned int clientlen = sizeof(client_addr);
        memset(currtime, 0, sizeof(currtime));
        getdate(currtime);

        /* Wait for client connection */
        if ((clientsock = accept(serversock, (struct sockaddr *) &client_addr, &clientlen)) < 0){
            die("Failed to accept client connection");
        }

        if (g_is_debug){
            printf("socket id: %d\n", *arg);
        }

        /* Use thread process new connection */
        arg = &clientsock;
        if (pthread_create(&thread, &thread_attr, tm_thread_callback, (void *)arg) != 0){
            die("Create new thread failed");
        }
        /* if (pthread_join(thread, &thread_result) != 0){
            die("Thread join failed");
        }
        pthread_t thread;
        */
        if (g_is_debug){
            printf("socket id: %d\n", *arg);
            printf("Main exit.\n");
        }
    }
    /* Destory pthread attribute */
    (void)pthread_attr_destroy(&thread_attr);
}


/**
 * Initialize server socket listen
 *
 */
void init_server_listen( unsigned int port, unsigned int max_client ){
    int serversock;
    struct sockaddr_in server_addr;
    char currtime[32];
  
    /* Create the TCP socket */
    if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        die("Failed to create socket");
    }

    /* Construct the server sockaddr_in structure */
    memset(&server_addr, 0, sizeof(server_addr));		/* Clear struct */
    server_addr.sin_family = AF_INET;					/* Internet/IP */
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	/* Incoming addr */
    server_addr.sin_port = htons(port);					/* server port */

    /* Bind the server socket */
    if (bind(serversock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        die("Failed to bind the server socket");
    }

    /* Listen on the server socket */
    if (listen(serversock, max_client) < 0){
        die("Failed to listen on server socket");
    }

    /* Print listening message */
    getdate(currtime);
    fprintf(stdout, "[%s] Start server listening at port %d ...\n", currtime, port);
    fprintf(stdout, "[%s] Waiting client connection ...\n", currtime);

	/* Multiplexing IO */
	tm_thread(serversock, max_client);

}




/********************************
 *
 *     Server running
 *
 ********************************/


/**
 * Main function 
 *
 */
int main( int argc, char *argv[] ){

	/* Parse cli input options */
	if ( argc > 1 ){
		if ( parse_options( argc, argv ) != 0 ){
			exit(-1);
		}
	}
	
	/* Set is daemon mode run */
	if ( g_is_daemon ){
		pid_t pid;
		if ( (pid = fork()) < 0 ){
			die("daemon fork error");
		} else if ( pid != 0){
			exit(1);
		}
	}

	/* Debug mode out configure information */
	if ( g_is_debug ){
		print_config();
	}

	/* Init hash table & queue */
	g_htable = tm_hcreate( g_max_tablesize );
	g_qlist  = tm_qcreate();
	init_status( g_max_mem_size );

	/* Start server listen */
	init_server_listen( g_port, g_max_client );
	
	return(0);
}
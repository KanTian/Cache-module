typedef short status;

#define SUCCESS      1
#define OK           0
#define ERROR		 -1

#define FALSE        0
#define TRUE         1
#define EXIT         2

#define MODE_SET     0
#define MODE_ADD     1
#define MODE_REPLACE 2

#define E_GENERAL    0
#define E_CLIENT     1
#define E_SERVER     2

#define SERVER_NAME  "tmached"
#define VERSION       "1.0.0"

#define BUFFER_SIZE     8192
#define MAX_BUF_SIZE    1048576
#define MAX_TABLE_SIZE  10000
#define MAX_LIFE_TIME   2592000
#define GC_PROBAILITY   1
#define GC_DIVISOR      100

#define IS_DEBUG        0
#define IS_DAEMON       0
#define PORT            11211
#define MAX_CLIENTS     1024
#define MAX_MEN_SIZE    16777216


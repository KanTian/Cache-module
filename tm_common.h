void die(char *msg);

char *substr(const char *s, int start_pos, int length, char *ret);

void explod(char *from, char delim, char ***to, int *item_num);

char *strtolower(char *s);

char *strtoupper(char *s);

int strpos(const char *s, char c);

int strrpos(const char *s, char c);

char *trim(char *s);

char *ltrim(char *s);

int is_numberic(const char *s);

void getdate(char *s);

int socket_set_nonblock(int sockfd);


struct tm_hash_entry_t
{
	char *key;
	char *data;
	size_t length;
	unsigned created;
	unsigned expired;
	struct tm_hash_entry_t *next;
	/* data */
};

struct tm_hash_t{
	struct tm_hash_entry_t **table;
	unsigned size;
	unsigned total;
};
//hash core function
unsigned tm_hash(const char *str, unsigned table_size);

//create hash table
struct tm_hash_t *tm_hcreate(unsigned table_size);

//insert a recored into hash table
status tm_hinsert(struct tm_hash_t *hashtable, char *key, char *data, unsigned length, unsigned lifetime, short mode);

//find special key data item
struct tm_hash_entry_t *tm_hfind(struct tm_hash_t *hashtable, char *key);

//delete special key and data
status tm_hremove(struct tm_hash_t *hashtable, char *key);

//destory hash table
sattus tm_hdestory(struct tm_hash_t *hashtable);

//visit hashtable every item
void tm_hvisit(struct tm_hash_t *hashtable);


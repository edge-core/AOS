#ifndef HASH__H
#define HASH__H 1

#include <stddef.h>                               /* For size_t     */
//#include "rdwr.h"

#define READ_LOCK(__hash, __nodeval)
/*pthread_rdwr_rlock_np( &(__hash->node[__nodeval]->rwlock))*/
#define READ_UNLOCK(__hash, __nodeval)
/*pthread_rdwr_runlock_np( &(__hash->node[__nodeval]->rwlock))*/
#define WRITE_LOCK(__hash, __nodeval)
/*pthread_rdwr_wlock_np( &(__hash->node[__nodeval]->rwlock))*/
#define WRITE_UNLOCK(__hash, __nodeval)
/*pthread_rdwr_wunlock_np( &(__hash->node[__nodeval]->rwlock))*/

#define HASH_FLAG_IGNORE_CASE 1

typedef struct
{
    void        *data;
    unsigned int size;
}
datum_t;

typedef struct bucket
{
   datum_t *key;
   datum_t *val;
   struct bucket *next;
}
bucket_t;

typedef struct
{
   bucket_t *bucket;
//   pthread_rdwr_t rwlock;
}
node_t;

typedef size_t (*hashval_t) (datum_t *key, void *hash);
typedef int (*keycmp_t) (void *hash, datum_t *key1, datum_t *key2);
typedef void (*data_free_t) (void *data, unsigned int size);

typedef struct
{
    hashval_t hashval_fn;
    keycmp_t  keycmp_fn;

    data_free_t data_free_fn;

    size_t size;
    node_t **node;
    int flags;
}
hash_t;

hash_t  *hash_create (size_t size, hashval_t hashval_fn, keycmp_t keycmp_fn, data_free_t data_free_fn);
void     hash_destroy(hash_t *hash);

int      hash_get_flags(hash_t *hash);
void     hash_set_flags(hash_t *hash, int flags);

datum_t *hash_insert (datum_t *key, datum_t *val, hash_t *hash);
datum_t *hash_delete (datum_t *key, hash_t *hash);

datum_t *hash_lookup (datum_t *key, hash_t *hash);
int hash_foreach (hash_t *hash, int (*func)(datum_t *key, datum_t *val, void *), void *arg);
int hash_walkfrom (hash_t *hash, size_t from, int (*func)(datum_t *key, datum_t *val, void *), void *arg);

datum_t *datum_new  ( void *data, size_t size );
void     datum_free ( datum_t *datum );

size_t hashval ( datum_t *key, hash_t *hash );
int hash_keycmp(hash_t *hash, datum_t *key1, datum_t *key2);

#endif /* HASH__H */
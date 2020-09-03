//
//  mm.c
//
//  Created by JunYing Yeh on 2014/4/22.
//
//

#include <stdio.h>
#include "l_lib.h"
#include "mm.h"

#ifndef _countof
#define _countof(_ARY) (sizeof(_ARY)/sizeof(*_ARY))
#endif

void debug_msg(const char *format, ...){}

#include "hash.h"
#include "hash.c"

enum
{
    MM_MAX_NUMBER_OF_MONITOR_TASK = 30,
    MM_TOTAL_NUMBER_OF_HASH_ENTRIES = 100
};

typedef struct
{
    int tid;
    hash_t *hash;
} HashHash_T;

typedef struct
{
    void *pointer;
} HashKey_T;

typedef struct
{
    char file[30];
    int line;
    void *pointer;
    void *caller_addr;
    size_t size;
} HashData_T;

static HashHash_T hash_pool[MM_MAX_NUMBER_OF_MONITOR_TASK];

static mm_lock_t mm_lock_fn;

static int mm_options;

void mm_write_lock() {
    if (mm_lock_fn) {
        mm_lock_fn(MM_LOCK | MM_WRITE);
    }
}

void mm_write_unlock() {
    if (mm_lock_fn) {
        mm_lock_fn(MM_UNLOCK | MM_WRITE);
    }
}

void mm_read_lock() {
    if (mm_lock_fn) {
        mm_lock_fn(MM_LOCK | MM_READ);
    }
}

void mm_read_unlock() {
    if (mm_lock_fn) {
        mm_lock_fn(MM_UNLOCK | MM_READ);
    }
}

int mm_keycmp(hash_t *hash, datum_t *key1, datum_t *key2)
{
    return memcmp(key1->data, key2->data, key1->size);
}

hash_t *mm_get_hash(int tid) {
    int i;
    mm_read_lock();
    for (i = 0; i < _countof(hash_pool); ++ i) {
        HashHash_T *h = &hash_pool[i];

        if (h->tid == tid) {
            if (!h->hash) {
                h->hash = hash_create(MM_TOTAL_NUMBER_OF_HASH_ENTRIES, NULL, (keycmp_t)mm_keycmp, NULL);
            }
            mm_read_unlock();
            return h->hash;
        }
    }

    mm_read_unlock();
    mm_write_lock();

    for (i = 0; i < _countof(hash_pool); ++ i) {
        HashHash_T *h = &hash_pool[i];

        if (h->tid == 0) {
            h->tid = tid;

            if (!h->hash) {
                h->hash = hash_create(MM_TOTAL_NUMBER_OF_HASH_ENTRIES, NULL, (keycmp_t)mm_keycmp, NULL);
            }

            mm_write_unlock();
            return h->hash;
        }
    }

    mm_write_unlock();
    return NULL;
}

void *MM_alloc(void *ptr, size_t size, int tid, const char *file, int line, void *caller_addr) {
    if (ptr) {
        HashKey_T hash_key;
        HashData_T hash_data;
        datum_t key = {0};
        datum_t val = {0};
        datum_t *ret;

        int start = 0;

        hash_t *hash = mm_get_hash(tid);
        if (!hash) {
            //printf("poll full, ptr = %p at %s, %d\r\n", ptr, file, line);
            return ptr;
        }

        memset(&hash_key, 0, sizeof(hash_key));
        memset(&hash_data, 0, sizeof(hash_data));

        hash_data.pointer = hash_key.pointer = ptr;
        hash_data.size = size;

        start = strlen(file) - (sizeof(hash_data.file) - 1);
        if (start < 0) {
            start = 0;
        }

        strncpy(hash_data.file, &file[start], sizeof(hash_data.file) - 1);
        hash_data.file[sizeof(hash_data.file) - 1] = '\0';

        hash_data.line = line;

        hash_data.caller_addr = caller_addr;

        key.data = &hash_key;
        key.size = sizeof(hash_key);

        val.data = &hash_data;
        val.size = sizeof(hash_data);

        ret = hash_insert(&key, &val, hash);
        if (!ret) {
            //printf("hash full, ptr = %p at %s, %d\r\n", ptr, file, line);
        }
    }

    return ptr;
}

void MM_free(void *ptr, int tid) {
    HashKey_T hash_key;
    datum_t key = {0};
    datum_t *ret;

    hash_t *hash = mm_get_hash(tid);

    if (!ptr) {
        return;
    }

    if (!hash) {
        return;
    }

    memset(&hash_key, 0, sizeof(hash_key));
    hash_key.pointer = ptr;

    key.data = &hash_key;
    key.size = sizeof(hash_key);

    ret = hash_delete(&key, hash);

    if (ret) {
        datum_free(ret);
    }
    else {
        //printf("look failed, ptr = %p\r\n", ptr);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
static char VisibleCode(unsigned char code)
{
    if ((code<32) || (code>128))
        return  0x2E;

    return (code);
}

static char Hex2Ascii(unsigned hex)
{
    int bin_code = hex;

    if ((bin_code = bin_code&0x0f) > 9)
        return (char)(bin_code + 0x37);

    return  (char) (bin_code + 0x30);
}

static int Code2Hex(unsigned char code, char *buf)
{
    buf[0]  = Hex2Ascii ((0xF0& code) >> 4);
    buf[1]  = Hex2Ascii (0x0F & code);
    return  2;
}

static void DumpHex(const char *title, size_t len, const char *buf)
{
    int  idx = 0, i;
    char hex_buf[69];

    if (title)
        printf("%s", title);

    printf("\r\n");
    memset(hex_buf, 0x20, sizeof(hex_buf));

    for (i = 0; i < len; i++, idx++)
    {
        idx %= 16;
        Code2Hex(buf[i], &(hex_buf[idx*3]));
        hex_buf[50+idx] = VisibleCode(buf[i]);
        if (((i+1) % 16) ==0)
        {
            hex_buf[66] = 0x0d;
            hex_buf[67] = 0x0a;
            hex_buf[68] = 0;
            printf ("%s", hex_buf);
            memset(hex_buf, 0x20, 69);
        }
    }

    if (i % 16)
    {
        hex_buf[66] = 0x0d;
        hex_buf[67] = 0x0a;
        hex_buf[68] = 0;
        printf ("%s", hex_buf);
    }

    fflush(stdout);
}
//////////////////////////////////////////////////////////////////////////////////////////

static int show_memory_block(datum_t *key, datum_t *val, void *arg) {
    HashKey_T *hash_key;
    HashData_T *hash_data;

    hash_key = (HashKey_T *)key->data;
    hash_data = (HashData_T*)val->data;

    printf("Found leakage at %s, %d: addr = %p(%lu), caller = %p\r\n",
        hash_data->file, hash_data->line, hash_data->pointer, hash_data->size, hash_data->caller_addr);

    if (MM_get_options() & MM_OPTION_SHOW_LEAKAGE_DETAIL)
    {
        DumpHex(NULL, hash_data->size, (char *) hash_data->pointer);
    }

    return 0;
}

static int count_one(datum_t *key, datum_t *val, void *arg) {
    size_t *count = (size_t*)arg;
    *count = 1;
    return 1;
}

static int hash_is_empty(hash_t *hash) {
    size_t count = 0;
    hash_foreach(hash, count_one, &count);

    return (count) ? 0 : 1;
}

int mm_dump_allocated_memory(int tid)
{
    hash_t *hash = mm_get_hash(tid);

    if (!hash)
        return 0;

    if (MM_get_options() & MM_OPTION_SHOW_LEAKAGE)
    {
        hash_foreach(hash, show_memory_block, NULL);
    }

    return hash_is_empty(hash) ? 0 : 1;
}

int MM_kill(int tid) {
    int i;
    int ret;

    ret = mm_dump_allocated_memory(tid);

    mm_write_lock();
    for (i = 0; i < _countof(hash_pool); ++ i) {
        HashHash_T *h = &hash_pool[i];

        if (h->tid == tid) {
            if (h->hash) {
                hash_destroy(h->hash);
                h->hash = NULL;
            }

            memset(h, 0, sizeof(*h));

            mm_write_unlock();
            return ret;
        }
    }
    mm_write_unlock();
    return 0;
}

void MM_set_lock_function(mm_lock_t cb) {
    mm_lock_fn = cb;
}

int MM_add_options(int options)
{
    int local;

    mm_write_lock();
    mm_options |= options;
    local = mm_options;
    mm_write_unlock();

    return local;
}

int MM_remove_options(int options)
{
    int local;

    mm_write_lock();
    mm_options &= ~options;
    local = mm_options;
    mm_write_unlock();

    return local;
}

int MM_toggle_options(int options)
{
    int local;

    mm_write_lock();
    mm_options ^= options;
    local = mm_options;
    mm_write_unlock();

    return local;
}

int MM_get_options()
{
    int local;

    mm_read_lock();
    local = mm_options;
    mm_read_unlock();

    return local;
}
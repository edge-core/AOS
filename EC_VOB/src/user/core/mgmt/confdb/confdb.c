//#include <mongoc.h>
#include <stdio.h>
#include <stdlib.h>

#include "confdb_private.h"

#ifndef MAX_PATH
#define MAX_PATH        255
#endif

#define MAX_ENTRY       8

#ifndef _countof
#define _countof(_ARY)  (sizeof(_ARY)/sizeof(*_ARY))
#endif

#define CONFDB_READ_LOCK() \
    CONFDB_LOCK_Lock(CONFDB_LOCK_MODE_LOCK | CONFDB_LOCK_MODE_READ, CONFDB_LOCK_TYPE_CONTEX, __FUNCTION__, __LINE__)

#define CONFDB_READ_WRITE_LOCK() \
    CONFDB_LOCK_Lock(CONFDB_LOCK_MODE_LOCK | CONFDB_LOCK_MODE_WRITE, CONFDB_LOCK_TYPE_CONTEX, __FUNCTION__, __LINE__)

#define CONFDB_UNLOCK() \
    CONFDB_LOCK_Lock(CONFDB_LOCK_MODE_UNLOCK, CONFDB_LOCK_TYPE_CONTEX, __FUNCTION__, __LINE__)

typedef struct
{
    mongoc_client_t                *client;
    char                            server_uri[MAX_PATH + 1];
    char                            default_db_name[MAX_PATH + 1];
    int                             is_server_changed;
    int                             is_connected;
} CONFDB_CONTEX_T;

#define CONFDB_SERVER_URI           "mongodb://127.0.0.1/"
#define CONFDB_DEFAULT_DB_NAME      "test"

static int                          confdb_init;
static CONFDB_CONTEX_T              confdb_ctx;

static void CONFDB_InitContext(CONFDB_CONTEX_T *ctx);

static bson_t * confdb_key_to_bson(const char *key);
static CONFDB_RETURN_CODE_T confdb_find_one(
    const char *db_name,
    const char *collection_name,
    const char *key,
    CONFDB_RETURN_CODE_T(*fn)(const bson_iter_t *doc, void **cookie), void **cookie);

//
// This function shall be called for main thread
//
void CONFDB_Initialize()
{
    CONFDB_LOCK_Create();

    if (!confdb_init)
    {
        CONFDB_InitContext(&confdb_ctx);

        mongoc_init();

        confdb_init = 1;
    }
}

//
// This function shall be called for main thread
//
void CONFDB_FreeSystemResource()
{
    if (!confdb_init)
    {
        return;
    }

    if (confdb_ctx.client)
    {
        mongoc_client_destroy(confdb_ctx.client);
        confdb_ctx.client = NULL;
    }

    mongoc_cleanup();

    confdb_init = 0;
}

void CONFDB_SetServerURI(const char *server_uri)
{
    if (!confdb_init)
    {
        CONFDB_Initialize();
    }

    CONFDB_READ_WRITE_LOCK();

    strncpy(confdb_ctx.server_uri, server_uri, sizeof(server_uri) - 1);
    confdb_ctx.server_uri[sizeof(confdb_ctx.server_uri) - 1] = '\0';

    if (confdb_ctx.is_connected)
    {
        confdb_ctx.is_server_changed = 1;
    }

    CONFDB_UNLOCK();
}

void CONFDB_SetDefaultDB(const char *db)
{
    if (!confdb_init)
    {
        CONFDB_Initialize();
    }

    CONFDB_READ_WRITE_LOCK();

    strncpy(confdb_ctx.default_db_name, db, sizeof(confdb_ctx.default_db_name) - 1);
    confdb_ctx.default_db_name[sizeof(confdb_ctx.default_db_name) - 1] = '\0';

    CONFDB_UNLOCK();
}

static CONFDB_RETURN_CODE_T confdb_bson_to_bool(const bson_iter_t *iter, void **cookie)
{
    bool               *value = (bool *)cookie;

    if (bson_iter_type(iter) == BSON_TYPE_BOOL)
    {
        *value = bson_iter_bool(iter);
    }
    else if (bson_iter_type(iter) == BSON_TYPE_INT32)
    {
        *value = bson_iter_int32(iter) ? 1 : 0;
    }
    else if (bson_iter_type(iter) == BSON_TYPE_INT64)
    {
        *value = bson_iter_int64(iter) ? 1 : 0;
    }
    else if (bson_iter_type(iter) == BSON_TYPE_DOUBLE)
    {
        *value = bson_iter_double(iter) ? 1 : 0;
    }
    else
    {
        return CONFDB_ERROR_TYPE;
    }

    return CONFDB_SUCCESS;
}

CONFDB_RETURN_CODE_T CONFDB_GetBoolValue(const char *collection, const char *key, bool *value)
{
    CONFDB_RETURN_CODE_T    ret;

    if (value == NULL)
    {
        return CONFDB_ERROR_INVALID_PARAMETER;
    }

    if (!confdb_init)
    {
        CONFDB_Initialize();
    }

    ret = confdb_find_one(confdb_ctx.default_db_name, collection, key, confdb_bson_to_bool, (void **)value);
    return ret;
}

static CONFDB_RETURN_CODE_T confdb_bson_to_int32(const bson_iter_t *iter, void **cookie)
{
    int32_t            *value = (int32_t *)cookie;

    if (bson_iter_type(iter) == BSON_TYPE_BOOL)
    {
        *value = bson_iter_bool(iter);
    }
    else if (bson_iter_type(iter) == BSON_TYPE_INT32)
    {
        *value = bson_iter_int32(iter);
    }
    else if (bson_iter_type(iter) == BSON_TYPE_INT64)
    {
        *value = (int32_t)bson_iter_int64(iter);
    }
    else if (bson_iter_type(iter) == BSON_TYPE_DOUBLE)
    {
        *value = (int32_t)bson_iter_double(iter);
    }
    else
    {
        return CONFDB_ERROR_TYPE;
    }

    return CONFDB_SUCCESS;
}

CONFDB_RETURN_CODE_T CONFDB_GetInt32Value(const char *collection, const char *key, int32_t *value)
{
    CONFDB_RETURN_CODE_T    ret;

    if (value == NULL)
    {
        return CONFDB_ERROR_INVALID_PARAMETER;
    }

    if (!confdb_init)
    {
        CONFDB_Initialize();
    }

    ret = confdb_find_one(confdb_ctx.default_db_name, collection, key, confdb_bson_to_int32, (void **)value);
    return ret;
}

static CONFDB_RETURN_CODE_T confdb_bson_to_int64(const bson_iter_t *iter, void **cookie)
{
    int64_t            *value = (int64_t *)cookie;

    if (bson_iter_type(iter) == BSON_TYPE_BOOL)
    {
        *value = bson_iter_bool(iter);
    }
    else if (bson_iter_type(iter) == BSON_TYPE_INT32)
    {
        *value = bson_iter_int32(iter);
    }
    else if (bson_iter_type(iter) == BSON_TYPE_INT64)
    {
        *value = bson_iter_int64(iter);
    }
    else if (bson_iter_type(iter) == BSON_TYPE_DOUBLE)
    {
        *value = (int64_t)bson_iter_double(iter);
    }
    else
    {
        return CONFDB_ERROR_TYPE;
    }

    return CONFDB_SUCCESS;
}

CONFDB_RETURN_CODE_T CONFDB_GetInt64Value(const char *collection, const char *key, int64_t *value)
{
    CONFDB_RETURN_CODE_T    ret;

    if (value == NULL)
    {
        return CONFDB_ERROR_INVALID_PARAMETER;
    }

    if (!confdb_init)
    {
        CONFDB_Initialize();
    }

    ret = confdb_find_one(confdb_ctx.default_db_name, collection, key, confdb_bson_to_int64, (void **)value);
    return ret;
}

static CONFDB_RETURN_CODE_T confdb_bson_to_double(const bson_iter_t *iter, void **cookie)
{
    double             *value = (double *) cookie;

    if (bson_iter_type(iter) == BSON_TYPE_BOOL)
    {
        *value = bson_iter_bool(iter);
    }
    else if (bson_iter_type(iter) == BSON_TYPE_INT32)
    {
        *value = bson_iter_int32(iter);
    }
    else if (bson_iter_type(iter) == BSON_TYPE_INT64)
    {
        *value = (int32_t)bson_iter_int64(iter);
    }
    else if (bson_iter_type(iter) == BSON_TYPE_DOUBLE)
    {
        *value = bson_iter_double(iter);
    }
    else 
    {
        return CONFDB_ERROR_TYPE;
    }

    return CONFDB_SUCCESS;
}

CONFDB_RETURN_CODE_T CONFDB_GetDoubleValue(const char *collection, const char *key, double *value)
{
    CONFDB_RETURN_CODE_T    ret;

    if (value == NULL)
    {
        return CONFDB_ERROR_INVALID_PARAMETER;
    }

    if (!confdb_init)
    {
        CONFDB_Initialize();
    }

    ret = confdb_find_one(confdb_ctx.default_db_name, collection, key, confdb_bson_to_double, (void **)value);
    return ret;
}

typedef struct
{
    char       *buf;
    uint32_t   *length;
} confdb_cstr_buf_t;

static CONFDB_RETURN_CODE_T confdb_bson_to_cstr_buf(const bson_iter_t *iter, void **cookie)
{
    confdb_cstr_buf_t  *cstr_buf = (confdb_cstr_buf_t *) cookie;
    const char         *utf8;
    uint32_t            utf8_length;

    // TODO: Check type first

    utf8 = bson_iter_utf8(iter, &utf8_length);

    if (cstr_buf->length <= 0 || *(cstr_buf->length) < (utf8_length + 1))
    {
        *(cstr_buf->length) = utf8_length + 1;
        return CONFDB_ERROR_BUFFER_TOO_SMALL;
    }

    memcpy(cstr_buf->buf, utf8, utf8_length);
    cstr_buf->buf[utf8_length] = '\0';
    *(cstr_buf->length) = utf8_length + 1;

    return CONFDB_SUCCESS;
}

// @ length: size of value, include null char
CONFDB_RETURN_CODE_T CONFDB_GetUtf8Value(const char *collection, const char *key, char *value, uint32_t *length)
{
    CONFDB_RETURN_CODE_T    ret;
    confdb_cstr_buf_t       cstr_buf;

    if (length == NULL)
    {printf("%s%d invalid parameter\r\n", __FUNCTION__, __LINE__);
        return CONFDB_ERROR_INVALID_PARAMETER;
    }

    if (value == NULL && 0 < *length)
    {printf("%s%d invalid parameter\r\n", __FUNCTION__, __LINE__);
        return CONFDB_ERROR_INVALID_PARAMETER;
    }

    if (!confdb_init)
    {
        CONFDB_Initialize();
    }

    cstr_buf.buf = value;
    cstr_buf.length = length;

    ret = confdb_find_one(confdb_ctx.default_db_name, collection, key, confdb_bson_to_cstr_buf, (void **) &cstr_buf);
printf("%s%d ret=%d\r\n", __FUNCTION__, __LINE__, ret);

    return ret;
}

static CONFDB_RETURN_CODE_T confdb_bson_to_json(const bson_iter_t *iter, void **cookie)
{
    json_t **json = (json_t **) cookie;

    char *str;
    json_error_t error;

    switch (bson_iter_type(iter))
    {
    case BSON_TYPE_DOUBLE:
    {
        *json = json_real(bson_iter_double(iter));
        break;
    }
    case BSON_TYPE_UTF8:
    {
        uint32_t length;
        *json = json_string(bson_iter_utf8(iter, &length));
        break;
    }
    case BSON_TYPE_DOCUMENT:
    {
        const uint8_t *docbuf = NULL;
        uint32_t doclen = 0;
        bson_t b;
        bson_iter_document(iter, &doclen, &docbuf);
        if (bson_init_static(&b, docbuf, doclen) ) {
            str = bson_as_json(&b, NULL);
            // fprintf(stdout, "%s\n", str);
            *json = json_loadb(str, strlen(str), 0, &error);
            bson_free(str);
        }
        break;
    }
    case BSON_TYPE_ARRAY:
    {
        const uint8_t *docbuf = NULL;
        uint32_t doclen = 0;
        bson_t b;
        bson_iter_array(iter, &doclen, &docbuf);
        if (bson_init_static(&b, docbuf, doclen)) {
            str = bson_array_as_json(&b, NULL);
            // fprintf(stdout, "%s\n", str);
            *json = json_loadb(str, strlen(str), 0, &error);
            bson_free(str);
        }
        break;
    }
    case BSON_TYPE_BOOL:
    {
        *json = json_boolean(bson_iter_bool(iter));
        break;
    }
    case BSON_TYPE_INT32:
    {
        *json = json_integer(bson_iter_int32(iter));
        break;
    }
    case BSON_TYPE_INT64:
    {
        *json = json_integer(bson_iter_int64(iter));
        break;
    }
    default:
        printf("The type of key is: 0x%02x\n",
            (int)bson_iter_type(iter));
        return CONFDB_ERROR_UNKNOWN_TYPE;
    }

    return CONFDB_SUCCESS;
}

CONFDB_RETURN_CODE_T CONFDB_GetValue(const char *collection, const char *key, json_t **value)
{
    CONFDB_RETURN_CODE_T ret;

    *value = NULL;

    if (!confdb_init)
    {
        CONFDB_Initialize();
    }

    ret = confdb_find_one(confdb_ctx.default_db_name, collection, key, confdb_bson_to_json, (void **) value);
    
    if (ret != CONFDB_SUCCESS)
    {
        return ret;
    }

    assert(*value != NULL);

    if (*value == NULL)
    {
        ret = CONFDB_ERROR_NOT_FOUND;
    }

    return ret;
}

static void CONFDB_InitContext(CONFDB_CONTEX_T *ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    strncpy(ctx->server_uri, CONFDB_SERVER_URI, sizeof(ctx->server_uri) - 1);
    ctx->server_uri[sizeof(ctx->server_uri) - 1] = '\0';

    strncpy(ctx->default_db_name, CONFDB_DEFAULT_DB_NAME, sizeof(ctx->default_db_name) - 1);
    ctx->default_db_name[sizeof(ctx->default_db_name) - 1] = '\0';
}

static bson_t * confdb_key_to_bson(const char *key)
{
    bson_t *query = bson_new();
    bson_t *sub_query = bson_new();

    if (query == NULL || sub_query == NULL)
    {
        goto fail;
    }

    bson_init (query);

    bson_append_document_begin(query, key, strlen(key), sub_query);
    bson_append_bool(sub_query, "$exists", sizeof("$exists") - 1, true);
    bson_append_document_end(query, sub_query);

    if (0)
    {
        char *str;
        str = bson_as_json (query, NULL);
        printf ("%s\n", str);
        bson_free (str);
    }

    return query;

fail:
    if (query)
    {
        bson_destroy(query);
        query = NULL;
    }

    if (sub_query)
    {
        bson_destroy(sub_query);
        sub_query = NULL;
    }
    return NULL;
}

static mongoc_client_t * confdb_connect_to_server()
{
    mongoc_client_t *client;

    CONFDB_READ_LOCK();
printf("%s%d\r\n", __FUNCTION__, __LINE__);
    if (confdb_ctx.is_connected && confdb_ctx.client && !confdb_ctx.is_server_changed)
    {
        client = confdb_ctx.client;
printf("%s%d\r\n", __FUNCTION__, __LINE__);
        CONFDB_UNLOCK();
        return client;
    }

    CONFDB_UNLOCK();

    CONFDB_READ_WRITE_LOCK();

    if (confdb_ctx.is_connected && confdb_ctx.is_server_changed)
    {
printf("%s%d\r\n", __FUNCTION__, __LINE__);
        mongoc_client_destroy(confdb_ctx.client);
        confdb_ctx.client = NULL;
        confdb_ctx.is_connected = 0;
    }

    if (!confdb_ctx.is_connected)
    {
printf("%s%d\r\n", __FUNCTION__, __LINE__);
        confdb_ctx.client = mongoc_client_new(confdb_ctx.server_uri);
        if (confdb_ctx.client == NULL)
        {
printf("%s%d\r\n", __FUNCTION__, __LINE__);
            CONFDB_UNLOCK();
            return NULL;
        }

        confdb_ctx.is_connected = 1;
        confdb_ctx.is_server_changed = 0;
    }

    client = confdb_ctx.client;

    CONFDB_UNLOCK();
printf("%s%d\r\n", __FUNCTION__, __LINE__);
    return client;
}

static CONFDB_RETURN_CODE_T confdb_find_one(
    const char *db_name,
    const char *collection_name,
    const char *key,
    CONFDB_RETURN_CODE_T(*fn)(const bson_iter_t *iter, void **cookie), void **cookie)
{
    mongoc_client_t *client;
    mongoc_collection_t *collection = NULL;
    mongoc_cursor_t *cursor = NULL;
    bson_error_t error;
    const bson_t *doc = NULL;

    bson_t *query = NULL;

    CONFDB_RETURN_CODE_T ret = CONFDB_ERROR_FAIL;

    client = confdb_connect_to_server();
    if (client == NULL)
    {
        ret = CONFDB_ERROR_CONNECT;
        goto fin;
    }

    query = confdb_key_to_bson(key);
    if (query == NULL)
    {
        ret = CONFDB_ERROR_OUT_OF_MEMORY;
        goto fin;
    }

    collection = mongoc_client_get_collection(client, db_name, collection_name);
    cursor = mongoc_collection_find(collection, /* collection */
        MONGOC_QUERY_NONE,                      /* flags */
        0,                                      /* skip */
        1,                                      /* limit */
        0,
        query,
        NULL,                                   /* Fields, NULL for all. */
        NULL);                                  /* Read Prefs, NULL for default */

    if (mongoc_cursor_next(cursor, &doc))
    {
        bson_iter_t iter;

        if (0)
        {
            char *str;
            str = bson_as_json(doc, NULL);
            fprintf(stdout, "%s\n", str);
            bson_free(str);
        }

        if (bson_iter_init(&iter, doc) &&
            bson_iter_find(&iter, key)) {

            ret = fn(&iter, cookie);

            goto fin;
        }
    }

    if (mongoc_cursor_error(cursor, &error))
    {
        ret = CONFDB_ERROR_NOT_FOUND;
        fprintf(stderr, "Cursor Failure: %s\n", error.message);
        goto fin;
    }

    ret = CONFDB_ERROR_NOT_FOUND;

fin:
    assert(ret != CONFDB_ERROR_FAIL);

    if (query)
    {
        bson_destroy(query);
        query = NULL;
    }

    if (cursor)
    {
        mongoc_cursor_destroy(cursor);
        cursor = NULL;
    }

    return ret;
}

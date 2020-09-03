//
//  confdb.h
//  confdb
//
//  Created by JunYing Yeh on 2014/11/6.
//
//

#ifndef _CONFDB_HEADER_H_
#define _CONFDB_HEADER_H_

#include "jansson.h"
#include <stdbool.h>
#include "config_agent_config.h"

#ifndef MAX_PATH
#define MAX_PATH 255
#endif

typedef enum {
    CONFDB_SUCCESS = 0,
    CONFDB_ERROR_FAIL,
    CONFDB_ERROR_INVALID_PARAMETER,
    CONFDB_ERROR_OUT_OF_MEMORY,
    CONFDB_ERROR_BUFFER_TOO_SMALL,
    CONFDB_ERROR_UNKNOWN_TYPE,
    CONFDB_ERROR_CONNECT,
    CONFDB_ERROR_NOT_FOUND,
    CONFDB_ERROR_TYPE,
    CONFDB_ERROR_FMT
} CONFDB_RETURN_CODE_T;

#if __cplusplus
extern "C" {
#endif

void CONFDB_InitSystemResource();

void CONFDB_FreeSystemResource();

void CONFDB_SetServerURI(const char* server_uri);

void CONFDB_SetDefaultDB(const char* db);

CONFDB_RETURN_CODE_T CONFDB_GetBoolValue(const char* collection, const char* key, bool* value);

CONFDB_RETURN_CODE_T CONFDB_GetInt32Value(const char* collection, const char* key, int32_t* value);

CONFDB_RETURN_CODE_T CONFDB_GetInt64Value(const char* collection, const char* key, int64_t* value);

CONFDB_RETURN_CODE_T CONFDB_GetDoubleValue(const char* collection, const char* key, double* value);

CONFDB_RETURN_CODE_T CONFDB_GetUtf8Value(const char* collection, const char* key, char* value, uint32_t* length);

CONFDB_RETURN_CODE_T CONFDB_GetValue(const char* collection, const char* key, json_t** value);

#if __cplusplus
}
#endif

#endif /* defined(_CONFDB_HEADER_H_) */

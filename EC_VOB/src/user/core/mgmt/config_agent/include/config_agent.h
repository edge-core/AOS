//
//  config_agent.h
//  config_agent
//
//  Created by JunYing Yeh on 2016/9/1.
//
//

#ifndef _CONFIG_AGENT_H_
#define _CONFIG_AGENT_H_

#include <stddef.h>
#include <stdint.h>

#include "jansson.h"
#include "config_agent_config.h"

#ifndef MAX_PATH
#define MAX_PATH 255
#endif

typedef enum
{
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

    typedef enum {
        CONFIG_AGENT_INI,
        CONFIG_AGENT_XML,
        CONFIG_AGENT_JSON,
        CONFIG_AGENT_BSON,
        CONFDB_PARSE_CONTEXT_MAX = CONFIG_AGENT_BSON,
        CONFIG_AGENT_UNKNOWN
    } config_agent_parse_context_type_t;

    typedef struct
    {
        struct {
            config_agent_parse_context_type_t type;
            char path[MAX_PATH + 1];
        } source;

        json_t* json;

        int valid;
        char validate_msg[128];
    } config_agent_doc_t, *config_agent_doc_ptr_t;

    typedef struct {
        int idx;
    } config_agent_array_iter_t;

    typedef struct
    {
        const char* default_value;
        uint32_t min_length;
        uint32_t max_length;
        const char* pattern;
    } config_agent_get_string_options_t, *config_agent_get_string_options_ptr_t;

    typedef struct
    {
        uint32_t max_items;
    } config_agent_get_array_options_t, *config_agent_get_array_options_ptr_t;

    typedef struct
    {
        const char* schema;
    } config_agent_parse_options_t, *config_agent_parse_options_ptr_t;

    typedef struct config_agent_parse_context_s
    {
        char file_path[MAX_PATH + 1];
        char last_error[MAX_PATH + 1];

        config_agent_parse_context_type_t type;
        config_agent_parse_options_t options;

        struct {
            config_agent_doc_ptr_t(*parse_buffer)(struct config_agent_parse_context_s *ctx, const uint8_t *buf, uint32_t buflen);
            config_agent_doc_ptr_t(*parse_string)(struct config_agent_parse_context_s *ctx, const char *str);
            config_agent_doc_ptr_t(*parse_fd)(struct config_agent_parse_context_s *ctx, int fd);
            config_agent_doc_ptr_t(*parse_fstream)(struct config_agent_parse_context_s *ctx, FILE *input);
        } fn;

        struct {
            int line;                   //The line number on which the error occurred.
            int column;                 //The column on which the error occurred.
            int position;               //The position in bytes from the start of the input.
            char text[MAX_PATH + 1];    //The error message (in UTF-8), or an empty string if a message is not available.
        } error;

    } config_agent_context_t, *config_agent_parse_context_ptr_t;

    config_agent_parse_context_ptr_t config_agent_parse_context_create(config_agent_parse_context_type_t type, const config_agent_parse_options_ptr_t options);

    void config_agent_parse_context_free(config_agent_parse_context_ptr_t ctx);

    config_agent_doc_ptr_t config_agent_parse_buffer(config_agent_parse_context_ptr_t ctx, const uint8_t* buf, uint32_t buflen);

    config_agent_doc_ptr_t config_agent_parse_string(config_agent_parse_context_ptr_t ctx, const char* str);

    config_agent_doc_ptr_t config_agent_parse_fstream(config_agent_parse_context_ptr_t ctx, FILE* input);

    config_agent_doc_ptr_t config_agent_parse_file(config_agent_parse_context_ptr_t ctx, const char* path);

    config_agent_doc_ptr_t config_agent_read_document(const char* docname, config_agent_parse_options_ptr_t options);

    const config_agent_get_string_options_t* config_agent_json_default_get_string_options();

    const char* config_agent_json_get_string(const json_t* json, const char* key, const config_agent_get_string_options_t* options);

    const config_agent_get_array_options_t* config_agent_json_default_get_array_options();

    const json_t* config_agent_json_get_array(const json_t* json, const char* key, const config_agent_get_array_options_t* options);

    void config_agent_doc_free(config_agent_doc_ptr_t doc);

#if __cplusplus
}
#endif

#endif /* defined(__CONFIG_AGENT_H__) */

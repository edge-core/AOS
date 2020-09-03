/* ----------------------------------------------------------------------
 * FILE NAME: cgi.h

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K.

 * ABSTRACT:
 * This is part of the embedded program for ES3524.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Module for Fileless CGI.
 * This is the general header file.

 * CLASSES AND FUNCTIONS:

 * cgi_streqi: Private version of "string equal, case-insensitive".
 * cgi_SendText: Send to socket a text string.
 * cgi_SendBin: Send to socket a binary buffer.
 * cgi_SendHeader: Send to socket a URI header.
 * cgi_check_auth: Check authorisation level.
 * cgi_notfound: File not found, send error reply.

 * cgi_dispatch: Send Web screen.
 * cgi_main: Main function of CGI library.

 * cgi_query_init: Initialise linked list.
 * cgi_query_fin: Finish query linked list.
 * cgi_query_lookup: Lookup variable in query string.

 * USAGE:

#include <envcfg.h>
#include <cgi.h>

 * HISTORY:
 * 1997-09-24 (Wed): Created by Daniel K. Chung.
 * 1997-10-02 (Thu): Sends HTTP header.
 * 1997-10-06 (Mon): Checks authorisation level.
 * 1997-10-14 (Tue): Does not use global variables.
 * 1997-10-15 (Wed): Added "cgi_notfound".
 * 1997-10-16 (Thu): Added global constants.
 * 1997-10-24 (Fri): Passes authorisation level to CGI.
 * 1997-10-28 (Tue): Added "cgi_stresc".
 * 1998-04-09 (Thu): Added "cgi_SendBin".
 * 1998-08-12 (Wed): Moved "cgi_check_auth" etc. to "cg_auth.h".

 * COPYRIGHT (C) Accton Technology Corporation, 1997.
 * ---------------------------------------------------------------------- */
#ifndef CGI_H
#define CGI_H

#include "sys_module.h"
#include "cgi_coretype.h"
#include "http_loc.h"
#include "http_envcfg.h"
#include <libxml/parser.h> // FIXME: cgi.h shall not export internal design
#include "jansson.h" // FIXME: cgi.h shall not export internal design

#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>

#include "l_linklist.h"

#include "cgi_path2regex.h"
#include "cgi_json_object_path.h"
#include "cgi_link_list_ex.h"

#if __cplusplus
extern "C" {
#endif

#ifndef ASSERT
 #define ASSERT(eq)
#endif

#ifndef _countof
 #define _countof(_Array) (sizeof(_Array)/sizeof(*_Array))
#endif

#ifndef CGI_VERIFY_HTTP_CONNECTION
#define CGI_VERIFY_HTTP_CONNECTION(c)                   \
    do {                                                \
        ASSERT(c != NULL);                              \
        ASSERT(c->req != NULL);                         \
        ASSERT(c->req->user_ctx != NULL);               \
        ASSERT(c->res != NULL);                         \
        ASSERT(c->res->user_ctx != NULL);               \
    } while (0);
#endif

#define CGI_LOG_ERROR(level, msgtype, funcno, fmt, ...) \
    http_log_core(SYS_MODULE_CGI, level, msgtype, funcno, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum
{
    CGI_FAIL        = -1,
    CGI_NOT_FOUND   = 0,
    CGI_OK          = 1,
} CGI_RETVAL_T;

typedef enum
{
    CGI_TYPE_ACTION_ADD,
    CGI_TYPE_ACTION_DEL,
// FIXME: Split someone to another enum
    CGI_TYPE_ACTION_MODIFY,
    CGI_TYPE_ACTION_ADD_MEMBER,
    CGI_TYPE_ACTION_DEL_MEMBER,
    CGI_TYPE_ACTION_UNKNOWN
} CGI_TYPE_ACTION_T;

typedef enum
{
    CGI_TEXT_FILE   = 1,
    CGI_BIN_FILE    = 2,
} CGI_FILE_TYPE_T;

typedef enum
{
    CGI_SUCCESS                 = 200,
    CGI_BAD_REQUEST             = 400,
    CGI_FORBIDDEN               = 403,
    CGI_URI_NOT_FOUND           = R_NOT_FOUND,  /* patch code */
    CGI_METHOD_NA               = R_METHOD_NA, // TODO: REMOVE

    CGI_INTERNAL_SERVER_ERROR   = 500,

    CGI_AGAIN                   = 1999
} CGI_STATUS_CODE_T;

typedef struct
{
    unsigned long       ref_count;
    char               *ptr;
    unsigned long       size;
} CGI_OBJECT_T;

typedef struct
{
    CGI_OBJECT_T        co;
} CGI_STRING_T;

typedef CGI_STRING_T CGI_FILE_PATH_T;

typedef struct
{
    time_t              mtime;
} CGI_FILE_STATS_T;

typedef struct
{
    CGI_OBJECT_T        co;
    CGI_FILE_TYPE_T     type;
    CGI_FILE_STATS_T    stats;
} CGI_FILE_INFO_T;

// TODO: Rename to CGI_RS_HANDLER_T
typedef CGI_STATUS_CODE_T (*CGI_API_HANDLER_T) (HTTP_Request_T *http_request, HTTP_Response_T *http_resp);

typedef struct
{
    CGI_API_HANDLER_T   create_handler;
    CGI_API_HANDLER_T   read_handler;
    CGI_API_HANDLER_T   update_handler;
    CGI_API_HANDLER_T   delete_handler;
    CGI_API_HANDLER_T   ssi_handler;
} CGI_API_HANDLER_SET_T;

typedef struct
{
    BOOL_T              init;

    CGI_OBJECT_T        raw_value;
    json_t             *view_value;

    json_t             *value;  // model value
    json_t             *error;

    BOOL_T              valid;
} CGI_PARAMETER_T, * CGI_PARAMETER_PTR_T; // TODO: Rename to CGI_API_ARGS_T;

typedef struct
{
    UI32_T              curr_index;
    json_t             *requestset;

    CGI_PARAMETER_T     query;
    CGI_PARAMETER_T     params;
    CGI_PARAMETER_T     body;

    CGI_API_HANDLER_T   handler;

} CGI_REQUEST_USER_CONTEXT_T, * CGI_REQUEST_USER_CONTEXT_PTR_T;

#define CGI_MAX_ERROR_STR 100

typedef struct
{
    xmlNodePtr          root;

    json_t             *response_headers;

    UI32_T              curr_index;
    json_t             *responseset;

    int                 is_jss;
    int                 is_send_create_object;
    int                 is_send_end_string;

    CGI_STATUS_CODE_T (*js_object)(HTTP_Response_T *http_resp);
    CGI_STATUS_CODE_T (*js_add_string)(HTTP_Response_T *http_resp, const char *key, const char *val);
    CGI_STATUS_CODE_T (*js_add_number)(HTTP_Response_T *http_resp, const char *key, const char *val);
    CGI_STATUS_CODE_T (*js_end)(HTTP_Response_T *http_resp);
    CGI_STATUS_CODE_T (*js_cleanup)(HTTP_Response_T *http_resp);

    int                 have_error;
    UI32_T              error_code;
    char                error_str[CGI_MAX_ERROR_STR + 1];
} CGI_RESPONSE_USER_CONTEXT_T, * CGI_RESPONSE_USER_CONTEXT_PTR_T;

// TODO: Check this
#define CGI_HTTP_UPLOAD_PRIVILEGE_REQUEST 15
#define CGI_HTTP_DOWNLOAD_PRIVILEGE_REQUEST 15

#define CGI_ERR_NOT_ENOUGH_PRIVILEGE_VALUE 0

#define LOCATE_MEM_SIZE      256

/* ----------------------------------------------------------------------
 * Global constants.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * CGI_MAIN_EnterMasterMode: This function forces this subsystem enter
 *                           the Master Operation mode.
 * ---------------------------------------------------------------------- */
void CGI_MAIN_EnterMasterMode (void);

/* ----------------------------------------------------------------------
 * CGI_MAIN_EnterTransitionMode: This function forces this subsystem enter
 *                               the Transition Operation mode.
 * ---------------------------------------------------------------------- */
void CGI_MAIN_EnterTransitionMode (void);

/* ----------------------------------------------------------------------
 * CGI_MAIN_EnterSlaveMode: This function forces this subsystem enter
 *                          the Slave Operation mode.
 * ---------------------------------------------------------------------- */
void CGI_MAIN_EnterSlaveMode (void);

BOOL_T CGI_MAIN_Register(const char *path, CGI_API_HANDLER_SET_T *handlers, UI32_T flags);

/* ----------------------------------------------------------------------
 * cgi_streqi: Private version of "string equal, case-insensitive".

 * As of now, there is no known function "strcmpi".

 * Input:
 * s1, s2: Pointer to two strings to be compared.

 * Output:
 * 0: Unequal.
 * non-zero: Equal.
 * ---------------------------------------------------------------------- */

int cgi_streqi (I8_T *s1, I8_T *s2);

/* ----------------------------------------------------------------------
 * cgi_stresc: Convert string to HTML escaped form.

 * Input:
 * szOut, szIn: Pointer to output and input strings.
 * ---------------------------------------------------------------------- */

void cgi_stresc (char *szOut, const char *szIn);
/* ----------------------------------------------------------------------
 * cgi_SendText: Send to socket a text string.

 * Input:
 * nSock: Socket identifier.
 * szText: Pointer to text string.
 * ---------------------------------------------------------------------- */

int cgi_SendText(HTTP_Connection_T *http_connection, int nSock, const char *szText);

/* ----------------------------------------------------------------------
 * cgi_SendBin: Send to socket a binary buffer.

 * Input:
 * nSock: Socket identifier.
 * nLen: Length of binary buffer.
 * szHex: Pointer to binary buffer.
 * ---------------------------------------------------------------------- */

int cgi_SendBin(HTTP_Connection_T *http_connection, int nSock, int nLen, const unsigned char *pBin);

int cgi_SendTextChunk1(HTTP_Connection_T *http_connection, int nSock, const char *szText);
int cgi_SendBinChunk1(HTTP_Connection_T *http_connection, int nSock, int nLen, const unsigned char *pBin);
int cgi_SendEoFChunk1(HTTP_Connection_T *http_connection, int nSock);

/* ----------------------------------------------------------------------
 * cgi_SendHeader: Send to socket a URI header.

 * Available for interface with Harrison Yeh (Ye Youxiong).

 * Input:
 * envcfg: Environment configuration.
 * nSock: Socket identifier.
 * nLen: Length of URI content.
 * ---------------------------------------------------------------------- */

void cgi_SendHeader(HTTP_Connection_T *http_connection, int nSock, int nLen, envcfg_t *envcfg);

/* ----------------------------------------------------------------------
 * cgi_notfound: File not found, send error reply.

 * Input:
 * nSock: Socket identifier.
 * ---------------------------------------------------------------------- */

void cgi_notfound(HTTP_Connection_T *http_connection, int nSock);

// FIXME: THIS function shall not put in here
void cgi_response_end(HTTP_Connection_T *http_connection);

/* ----------------------------------------------------------------------
 * cgi_dispatch: Send Web screen.

 * Input:
 * envcfg: Environment configuration.
 * sock: Socket identifier.
 * auth: Authorisation level: 0=none, 1=guest, 2=administrator.
 * ---------------------------------------------------------------------- */

void cgi_dispatch(HTTP_Event_T *event);

/* ----------------------------------------------------------------------
 * cgi_main: Main function of CGI library.

 * Input:
 * HTTP_ConnectionInfo_T *pConnectInfo.
 * ---------------------------------------------------------------------- */

void cgi_main(HTTP_Event_T *event);

void cgi_main_finalize(HTTP_Event_T *event);

void cgi_main_end(HTTP_Event_T *event);

void cgi_main_close(HTTP_Connection_T *http_connection);

/* ----------------------------------------------------------------------
 * cgi_query_init: Initialise linked list.

 * Parameters:
 * envQuery: Query linked list.
 * envHttp: HTTP environment linked list.
 * ---------------------------------------------------------------------- */

void cgi_query_init (envcfg_t *envQuery, envcfg_t *envHttp);

/* ----------------------------------------------------------------------
 * cgi_query_fin: Finish query linked list.

 * Parameters:
 * envQuery: Query linked list.
 * ---------------------------------------------------------------------- */

void cgi_query_fin (envcfg_t *envQuery);

void cgi_query_parse_text (I8_T *szOut, I8_T *pQuery, int *iQuery);

/* ----------------------------------------------------------------------
 * cgi_query_lookup: Lookup variable in query string.

 * Parameters:
 * envQuery: Query linked list.
 * szName: Name to lookup.

 * Return:
 * Pointer to value corresponding to name.
 * ---------------------------------------------------------------------- */

I8_T *cgi_query_lookup (envcfg_t *envQuery, I8_T *szName);

/* ------------------------------------------------------------------------------
 * cgi_query_env_getfirst: Search through environmental variables for given name.
 * Get the first value of environmental variable specified in name.
 *
 * Parameters:
 * envQuery: Query linked list.
 * name: Name to search.
 * pValue : Buffer to store the value.
 *
 * Return:
 * Pointer to value corresponding to environmenttal field.
 * ---------------------------------------------------------------------- */
envfield *cgi_query_env_getfirst (envcfg_t *envQuery, I8_T *name);

/* ------------------------------------------------------------------------------
 * cgi_query_env_getnext: Get the next value that environmental variable specified
 * in name.

 * Parameters:
 * field: Current field to search.
 * name: Name to search.
 * pValue : Buffer to store the value.
 *
 * Return:
 * 0 for next query exists else return 1.
 * ---------------------------------------------------------------------- */
envfield *cgi_query_env_getnext (envfield *field, I8_T *szName);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_QUERY_ParseQueryString
 *------------------------------------------------------------------------------
 * PURPOSE:  Separate name and value of query from QUERY_STRING,
 *           and set them into envcfg.
 * INPUT:    envcfg       -- QUERY_STRING in environmental configuration.
 * OUTPUT:   envcfg       -- Environmental configuration.
 * RETURN:   TRUE         -- Success.
 *           FALSE        -- Failed.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
BOOL_T CGI_QUERY_ParseQueryString(const char *query_p,
                                  BOOL_T (*fn) (envcfg_t *envcfg, char *name, char *value),
                                  envcfg_t *envcfg);

BOOL_T CGI_QUERY_InsertToEnv(envcfg_t *envcfg, char *name, char *value);
BOOL_T CGI_QUERY_InsertToEnv_Without_Dot_Query(envcfg_t *envcfg, char *name, char *value);

BOOL_T CGI_QUERY_ParseVariableParameter(envcfg_t *envcfg, char **in_pp, char *file_end_p);
BOOL_T CGI_QUERY_ClearVariableParameter(envcfg_t *envcfg);

#if __cplusplus
}
#endif

#endif  /* CGI_H */

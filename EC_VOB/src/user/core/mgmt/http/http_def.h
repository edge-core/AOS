/*
 * Project Name: Mercury
 * Module Name : HTTP_TASK.h
 * Abstract    : to be included in root.c
 * Purpose     : HTTP initiation and HTTP task creation
 *
 * History :
 *          Date        Modifier        Reason
 *
 * Copyright(C)      Accton Corporation, 2001
 *
 * Note    :
 */

#ifndef HTTP_DEF_H
#define HTTP_DEF_H

// TODO: Merge all stuff in this file to http_loc.h and then remove this file

#include "l_lib.h"

#include "sys_type.h"
#include "sys_cpnt.h"

#include "jansson.h"

#if (SYS_CPNT_HTTPS == TRUE)
#include "openssl/ssl.h"
#endif

#include "http_config.h"
#include "http_envcfg.h"

/* When Defined, the following DEBUG definitions will enable some helpful printf's

#define DEBUG_HEADER_PARSING
#define DEBUG_PERROR
#define DEBUG_PRINTF
#define DEBUG_UB
*/

#define HTTP_ERROR                      (-1)
#define HTTP_AGAIN                      (-2)
#define HTTP_OK                         0

/* server settings */

#define HTTP_GATEWAY_INTERFACE          "CGI/1.1"
#define HTTP_FULL_REQUEST               "HTTP/1.0"
#define HTTP_SIMPLE_REQUEST             "HTTP/0.9"

#define HTTP_REQUEST_TASK_NAME          "HT%02d"        /* (01 - 99)*/
#define HTTP_LOG_SEM_NAME               "HTLG"
#define MAX_FILEEXT_SIZE                32              /* maximum size of a filename extension */
//#define MAX_FIELD_LEN                 256             /* maximum length of header fields */
#define MAX_HEADER_LEN                  2048            /* this should probably be same as TCP buf size */
#define MAX_URI_LEN                     256             /* maximum length a URI */
#define MAX_FILE_READ                   512
#define MAX_READ                        512
//#define HTTP_BROWSE_BUF               2048
#define MAX_ERROR_HEADER_LEN            512
#define MAX_SEND                        1024


#define SUPV_STACK                      0x6000
#define USER_STACK                      2048

#define HTTP_COOKIE_SESSION_ID          "__cs"

#define ipa unsigned long

#ifndef ASSERT
#define ASSERT(eq)
#endif /* ASSERT */

#define HTTP_IS_BIT_ON(bmp, bno)        (bmp[(bno)/8] & (1 << (7 - (bno)%8)))
#define HTTP_IS_BIT_OFF(bmp, bno)       (!HTTP_IS_BIT_ON(bmp, bno))
#define HTTP_SET_BIT_ON(bmp, bno)       {bmp[(bno)/8] |= (1 << (7 - (bno)%8));}
#define HTTP_SET_BIT_OFF(bmp, bno)      {bmp[(bno)/8] &= ~(1 << (7 - (bno)%8));}

#define MAX_FILE_PATH_LEN               128                  // TODO: Move into OM ??

#define HTTP_CONFIG_ROOT_DIR_MAX_LEN    MAX_FILE_PATH_LEN    // TODO: Move into OM ??
#define HTTP_CONFIG_FILE_PATH_MAX_LEN   MAX_FILE_PATH_LEN    // TODO: Move into OM ??
#define HTTP_CONFIG_INDEX_PAGE_MAX_LEN  MAX_FILE_PATH_LEN    // TODO: Move into OM ??

#define HTTP_CONFIG_HTTPS_CERTIFICATE_PATH_MAX_LEN      MAX_FILE_PATH_LEN
#define HTTP_CONFIG_HTTPS_PARIVATE_KEY_PATH_MAX_LEN     MAX_FILE_PATH_LEN
#define HTTP_CONFIG_HTTPS_PASS_PHRASE_PATH_MAX_LEN      MAX_FILE_PATH_LEN

#define HTTP_CONFIG_TOTAL_ALIAS         5                    // TODO: Move into OM ??

typedef enum
{
    HTTP_CONNECTION_KEEP_ALIVE          = 0,            /* default value */
    HTTP_CONNECTION_CLOSE
} HTTP_ConnectionType_T;

typedef enum
{
    HTTP_TRANSFER_ENCODING_CHUNKED      = 0,            /* default value */
    HTTP_TRANSFER_ENCODING_NOT_PRESENT  = 1
} HTTP_TransferEncoding_T;

typedef enum
{
    HTTP_OM_CONFIG_SUCCESS,
    HTTP_OM_CONFIG_FAIL,
    HTTP_OM_CONFIG_NO_CHANGE
} HTTP_OM_ConfigState_T; // FIXME: remove _OM_

typedef struct
{
    char                   *uri;                        /* key */
    char                   *path;                       /* file path or directory path */
} HTTP_Alias_T;

struct HTTP_Request_ST
{
    HTTP_Connection_T      *connection;
    int                     fd;                         /* client's socket fd */
    bio_socket_t            bfd;

#define HTTP_DFLT_VERSION_MAJOR_VERSION 1
#define HTTP_DFLT_VERSION_MINOR_VERSION 1

    int                     major_version;
    int                     minor_version;

    int                     method;                     /* M_GET, M_POST, M_HEAD */

    char                    request_uri[MAX_URI_LEN + 1];   /* uri */
    char                    header_in[MAX_HEADER_LEN + 1];  /* full raw header */

#if (SYS_CPNT_CLUSTER == TRUE)
    /*since the parser will remove some character which to judge the end of header,
     *so using header_in_raw to keep the original head content
     */
    char                    header_in_raw[MAX_HEADER_LEN + 1];
#endif /* SYS_CPNT_CLUSTER */

    int                     body_index;                 /* pts to first char after validated header in header_in */
    int                     body_cnt;                   /* number of bytes already read from body */

    int                     header_idx;                 /* index to next unread char */
    int                     header_end_idx;             /* index to one char past last */
    int                     header_length;

    int                     http_simple;                /* true if HTTP version 0.9 (simple) */
    int                     status;                     /* R_OK et al. */

    HTTP_ConnectionType_T   connection_type;

    void                   *user_ctx;

    int                     access_privilege;
    struct envcfg_t        *envcfg;                     /* ptr to environmental vars struct */
    struct envcfg_t        *query;                      /* ptr to querystring vars */

    int                     multipart;                  /* process multipart  */
    char                   *multipart_data;             /* whole multipart data,
                                                         * allocated by BUFFER_MGR
                                                         */
    char                   *multipart_parsed_ptr;
    int                     multipart_state;
    size_t                  multipart_len;
    char                   *boundary;
    size_t                  boundary_len;

#if (SYS_CPNT_HTTPS == TRUE)
    SSL                     *ssl;                       /* SSL object */
    int                     Is_Handshake;               /* TRUE if handshake successful */
#endif /* SYS_CPNT_HTTPS */

};

typedef struct
{
    int                     major_version;
    int                     minor_version;

    int                     http_status;

    size_t                  content_length_n;
    json_t                 *content_type_n;
    json_t                 *cookies_n;

    unsigned                content_length:1;
    unsigned                content_type:1;
    unsigned                set_cookie:1;
    unsigned                chunked:1;
    unsigned                no_cache:1;
} HTTP_HeadersOut_T;

struct HTTP_Response_ST
{
    HTTP_Connection_T      *connection;
    int                     fd;
    bio_socket_t            bfd;

    void                   *user_ctx;

    UI32_T                  send_len;
    I8_T                    send_buf[MAX_SEND];

    // TODO: Rename to write, maybe we need a read
    int                   (*write_response)(struct HTTP_Response_ST *resp, char *in, size_t in_len);

    BOOL_T                  is_send_eof;

    // FIXME: Remove this flag
    HTTP_TransferEncoding_T transfer_encoding;

    HTTP_HeadersOut_T       headers_out;
};

typedef struct HTTP_Request_ST HTTP_Request_T;
#define _TYPEDEF_HTTP_REQUEST_T

typedef struct HTTP_Response_ST HTTP_Response_T;
#define _TYPEDEF_HTTP_RESPONSE_T

typedef enum
{
    HTTP_EVENT_READ,
    HTTP_EVENT_WRITE
} HTTP_Event_Type_T;

#define HTTP_TIMER_INFINITE (time_t)LONG_MAX

struct HTTP_Event_ST
{
    HTTP_Event_Type_T       event_type;
    int                     fd;
    void                   *data;

    struct timeval          tv;

    void                  (*handler)(struct HTTP_Event_ST *ev);
    void                  (*timeout_fn)(struct HTTP_Event_ST *ev);

    unsigned                ready:1;
    unsigned                timeout:1;
    unsigned                remove:1;

#if (HTTP_CFG_EVENT_DEBUG == 1)
    struct timeval          last_access_time;
    char                    function[HTTP_CFG_EVENT_DEBUG_MAX_FUNCTION_SIZE];
#endif /* HTTP_CFG_EVENT_DEBUG */
};

typedef struct HTTP_Event_ST HTTP_Event_T;
#define _TYPEDEF_HTTP_EVENT_T

#define HTTP_WORKER_MASTER          1
#define HTTP_WORKER_OTHER           2

#define HTTP_WORKER_START_HTTP      1
#define HTTP_WORKER_SHUTDOWN_HTTP   2
#define HTTP_WORKER_START_HTTPS     3
#define HTTP_WORKER_SHUTDOWN_HTTPS  4
#define HTTP_WORKER_CLOSE           5
#define HTTP_WORKER_RESTART_HTTP    6
#define HTTP_WORKER_RESTART_HTTPS   7
#define HTTP_WORKER_NEW_CONNECTION  8
#define HTTP_WORKER_DEL_CONNECTION  9

typedef unsigned char HTTP_COMMAND_T;

#pragma pack(1)
typedef struct
{
    HTTP_Connection_T      *connection;
} HTTP_NEW_CONNECTION_T;

typedef struct
{
    L_INET_AddrIp_T         ip_addr;
} HTTP_DEL_CONNECTION_T;

typedef struct
{
    HTTP_COMMAND_T          command;

    union
    {
        HTTP_NEW_CONNECTION_T new_connection;
        HTTP_DEL_CONNECTION_T del_connection;
    } u;

} HTTP_COMMAND_PACKET_T;
#pragma pack()

struct HTTP_Worker_ST
{
    int                     tid;

    int                     kind;

    int                     cmd_channel[2];

    union
    {
        HTTP_COMMAND_PACKET_T   packet;
        unsigned char           raw[1];
    } received_buffer;

    size_t                  received_count;

    int                     http_fd;
    int                     https_fd;

// TODO: Move to http_config.h
/* Max. number of events to monitor on. 3 extra events in scheduler thread are
 * used for monitoring on command channel, HTTP socket and HTTPS socket. 1 extra
 * event in worker thread is used for monitoring on command channel.
 */
#define HTTP_MAX_EVENT      (HTTP_CFG_MAXWAIT + 3)

    void                   *event_ctx;
    HTTP_LIST_T             connections;

    unsigned                close:1;
    unsigned                reload:1;       /* task to worker, 1 means reload service */
    unsigned                start_http:1;
    unsigned                shutdown_http:1;
    unsigned                restart_http:1;
    unsigned                start_https:1;
    unsigned                shutdown_https:1;
    unsigned                restart_https:1;
};

typedef struct HTTP_Worker_ST HTTP_Worker_T;
#define _TYPEDEF_HTTP_WORKER_T

struct HTTP_LOG_RECORD_ST
{
    struct tm               occurred_time;
    HTTP_LOG_LEVEL_T        level;
    HTTP_LOG_MSG_TYPE_T     message_type;

    int                     line;
    char                    function[HTTP_CFG_LOG_MAX_MESSAGE_STR_LEN + 1];

    char                    message[HTTP_CFG_LOG_MAX_MESSAGE_STR_LEN + 1];
};

typedef struct HTTP_LOG_RECORD_ST HTTP_LOG_RECORD_T;
#define _TYPEDEF_HTTP_LOG_RECORD_T

struct HTTP_LOG_DB_ST
{
    HTTP_RING_BUFFER_DESC_T rb;

    HTTP_LOG_RECORD_T       records[HTTP_CFG_LOG_MAX_ENTRIES];
};

typedef struct HTTP_LOG_DB_ST HTTP_LOG_DB_T;
#define _TYPEDEF_HTTP_LOG_DB_T

/* error constants */
#define E_SOCKET        0
#define E_BIND          1
#define E_ACCEPT        2
#define E_RESET         3       /* connection reset by client */
#define E_RECV          4       /* error reading from recv() */
#define E_SEND          5       /* a fatal send error */
#define E_NO_MEMORY     6       /* out of memory */
#define E_LISTEN        7
#define E_OPEN          8       /* file open error */

/* request.method */
#define M_GET           0
#define M_HEAD          1
#define M_POST          2

#define M_PUT           3
#define M_DELETE        4
#define M_SSI           5

/* request.status */
#define R_SILENT        999     /* no response to return to client */

#define R_REQUEST_OK    200
#define R_CREATED       201
#define R_ACCEPTED      202
#define R_PROVISIONAL   203
#define R_NO_CONTENT    204

#define R_MULTIPLE      300
#define R_MOVED_PERM    301
#define R_MOVED_TEMP    302
#define R_NOT_MODIFIED  304

#define R_BAD_REQUEST   400
#define R_UNAUTHORIZED  401
#define R_PAYMENT       402
#define R_FORBIDDEN     403
#define R_NOT_FOUND     404
#define R_METHOD_NA     405 /* method not allowed */
#define R_NONE_ACC      406 /* none acceptable */
#define R_PROXY         407 /* proxy authentication required */
#define R_REQUEST_TO    408 /* request timeout */
#define R_CONFLICT      409
#define R_GONE          410

#define R_ERROR         500 /* internal server error */
#define R_NOT_IMP       501 /* not implemented */
#define R_BAD_GATEWAY   502
#define R_SERVICE_UNAV  503 /* service unavailable */
#define R_GATEWAY_TO    504 /* gateway timeout */

#endif /* #ifndef HTTP_DEF_H */

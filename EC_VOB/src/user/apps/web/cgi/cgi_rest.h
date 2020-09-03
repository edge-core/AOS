#ifndef _CGI_REST_H_
#define _CGI_REST_H_

#include "cgi.h"

#if __cplusplus
extern "C" {
#endif

typedef struct
{
    struct L_list      *apis;
    json_t             *api_json;
} CGI_REST_CONTEXT_T, *CGI_REST_CONTEXT_PTR_T;

typedef struct
{
    char               *path;   // Path Templating. E.g., /items/{itemId}
    int                 method;

    json_t             *schema; // Parameter Object

    CGI_API_HANDLER_T   handler;
    UI32_T              flags;
} CGI_REST_ROUTE_T, *CGI_REST_ROUTE_PTR_T;

CGI_REST_CONTEXT_T *
cgi_rest_new_ctx(
);

void
cgi_rest_free_ctx(
	CGI_REST_CONTEXT_T ** ctx_pp
);

BOOL_T
cgi_rest_add_routes(
    CGI_REST_CONTEXT_PTR_T self,
    const char *path,
    const CGI_API_HANDLER_SET_T *handlers,
    uint32_t flags
);

BOOL_T
cgi_rest_add_route(
    CGI_REST_CONTEXT_PTR_T self,
    const char *path,
    int method,
    CGI_API_HANDLER_T handler,
    uint32_t flags
);

/*const*/ CGI_REST_ROUTE_PTR_T
cgi_rest_find_route(
    CGI_REST_CONTEXT_PTR_T self,
    const char *uri,
    int method
);

CGI_STATUS_CODE_T
cgi_rest_call_route_handler(
    CGI_REST_CONTEXT_PTR_T self,
    HTTP_Connection_T *http_connection,
    CGI_REST_ROUTE_PTR_T route
);

void
cgi_rest_write_response(
    HTTP_Connection_T *http_connection,
    CGI_STATUS_CODE_T status_code
);

#if __cplusplus
}
#endif

#endif /* _CGI_REST_H_ */

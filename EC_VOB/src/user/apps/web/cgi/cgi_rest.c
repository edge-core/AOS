#include "cgi.h"
#include "cgi_rest.h"
#include "cgi_request.h"
#include "cgi_response.h"
#include "cgi_regex.h"
#include "jansson.h"
#include "hash.h"
#include "strtoint.h"

typedef enum
{
    CGI_REST_MEDIA_TYPE_URL_ENCODED,
    CGI_REST_MEDIA_TYPE_JSON,
    CGI_REST_MEDIA_TYPE_UNRECOGNIZED
} CGI_REST_MEDIA_TYPE_T;  // TODO: move to http_def.h or http_type.h

#define CGI_REST_FLAG_ALLOW_INVALID     0x00000001
#define CGI_REST_FLAG_ADD_ANYWAY        0x00000002

typedef struct
{
    char                   *uri;  // TODO: Remove this field
    json_t                 *parsed_args_syntax;
    json_t                 *parsed_id_syntax;
    CGI_API_HANDLER_SET_T  *handlers;
} CGI_REST_ENTRY_T, * CGI_REST_ENTRY_PTR_T;

typedef struct CGI_REST_PARSER_S
{
    HTTP_Request_T         *req;
    CGI_REST_ROUTE_PTR_T    route;

    // TODO: add fntable for all read / parse / validators / formatter ...

    // TODO: uppercase the type
    //typedef void(*cgi_rest_read_fn_t)(const char *here, const char *raw, size_t raw_len, struct CGI_REST_PARSER_S *ctx, CGI_PARAMETER_T *out);
    //typedef void(*cgi_rest_validator_fn_t)(const char *here, const json_t *view_value, struct CGI_REST_PARSER_S *ctx, CGI_PARAMETER_T *out);

    //cgi_rest_read_fn_t      read_fn;
    //cgi_rest_validator_fn_t validator_fn;
    void(*read_fn)(const char *here, const char *raw, size_t raw_len, struct CGI_REST_PARSER_S *ctx, CGI_PARAMETER_T *out);
    void(*validator_fn)(const char *here, const json_t *view_value, struct CGI_REST_PARSER_S *ctx, CGI_PARAMETER_T *out);

} CGI_REST_PARSER_T;

static size_t
cgi_rest_path_level(
	const char *path
);

static CGI_REST_ROUTE_PTR_T
CGI_REST_InternalNewRoute(
	const char *path,
	int method,
	CGI_API_HANDLER_T handler,
	const json_t *schema,
	UI32_T flags
);

static void
CGI_REST_InternalDelRoute(
	CGI_REST_ROUTE_PTR_T *route_pp
);

static I32_T
CGI_REST_InternalCompareRoute(
    void *inlist_element,
    void *input_element
);

static void
CGI_REST_InternalFreeRouteListEntry(
    void *element
);

static BOOL_T
CGI_REST_InternalAddRoute(
	CGI_REST_CONTEXT_PTR_T self,
	const char *path,
	int method,
	CGI_API_HANDLER_T handler_fn,
	uint32_t flags
);

static CGI_STATUS_CODE_T CGI_REST_InternalProcessHandlerArguments(CGI_REST_CONTEXT_PTR_T self, HTTP_Connection_T *http_connection, CGI_REST_ROUTE_PTR_T route);

static void CGI_REST_InternalWriteResponse(HTTP_Connection_T *http_connection, const HTTP_Request_T *http_request, const HTTP_Response_T *http_response, CGI_STATUS_CODE_T status_code);
static const char * CGI_REST_InternalGetHttpReasonPhrase(CGI_STATUS_CODE_T status_code);

static void CGI_REST_InternalHandlerWrapper(HTTP_Event_T *event)
{
    CGI_STATUS_CODE_T               status_code;
    HTTP_Connection_T              *http_connection = (HTTP_Connection_T *) event->data;

    CGI_REQUEST_USER_CONTEXT_PTR_T  req_context = NULL;
    CGI_RESPONSE_USER_CONTEXT_PTR_T res_context = NULL;

    json_t                         *Request;
    json_t                         *Response;

    ASSERT(http_connection);
    ASSERT(http_connection->req);
    ASSERT(http_connection->res);
    ASSERT(http_connection->req->user_ctx);
    ASSERT(http_connection->res->user_ctx);

    ASSERT(((CGI_REQUEST_USER_CONTEXT_PTR_T)http_connection->req->user_ctx)->handler);

    if (http_connection == NULL)
    {
        goto fatal_error;
    }

    req_context = (CGI_REQUEST_USER_CONTEXT_PTR_T)http_connection->req->user_ctx;
    res_context = (CGI_RESPONSE_USER_CONTEXT_PTR_T)http_connection->res->user_ctx;

    Request = json_array_get(req_context->requestset, req_context->curr_index);
    Response = json_array_get(res_context->responseset, res_context->curr_index);

    ASSERT(Request);
    ASSERT(Response);

    if (Request == NULL || Response == NULL)
    {
        goto fatal_error;
    }

    status_code = req_context->handler(http_connection->req, http_connection->res);
    if (status_code == CGI_AGAIN)
    {
        event->remove = 0;
        return;
    }

    http_connection->wait_syscb = 0;

    if (status_code < 400)
    {
        json_object_del(Response, "error");
    }

    if (400 <= status_code)
    {
        json_object_del(Response, "result");
    }

    json_object_set_new(Response, "status", json_integer(status_code));

    CGI_REST_InternalWriteResponse(http_connection, http_connection->req, http_connection->res, status_code);

    ASSERT(http_connection->done == 1);

    event->fd = HTTP_EVENT_READ;
    event->fd = http_connection->fds[HTTP_CONN_FD_NET];
    event->handler = cgi_main_finalize;
    http_event_add(http_connection->worker->event_ctx, event, 0);
    return;

fatal_error:
    ASSERT(0);
}

CGI_REST_CONTEXT_T *
cgi_rest_new_ctx()
{
	CGI_REST_CONTEXT_T *ctx;

    ctx = (CGI_REST_CONTEXT_PTR_T) L_MM_Malloc(sizeof(CGI_REST_CONTEXT_T), 123);

    if (ctx)
    {
    	json_t *api_url;

        memset(ctx, 0, sizeof(CGI_REST_CONTEXT_T));

        ctx->apis = (struct L_list *)L_list_new();

        ctx->apis->cmp = CGI_REST_InternalCompareRoute;
        ctx->apis->del = CGI_REST_InternalFreeRouteListEntry;

        // load api.json
        api_url = (json_t*)HTTP_MGR_GetConfigValue("api.url");
        if (api_url)
        {
            json_error_t error;

            ctx->api_json = json_load_file(json_string_value(api_url), 0, &error);

            if (!ctx->api_json)
            {
                printf("error: line = %d, column = %d, msg = \"%s\"\r\n", error.line, error.column, error.text[0] != '\0' ? error.text : "(none)");
            }

            json_decref(api_url);
        }
    }

	return ctx;
}

void
cgi_rest_free_ctx(
	CGI_REST_CONTEXT_T ** ctx_pp)
{
	CGI_REST_CONTEXT_T *ctx;

	ASSERT(ctx_pp != NULL);
	if (ctx_pp == NULL)
	{
		return;
	}

	if (*ctx_pp == NULL)
	{
		return;
	}

	ctx = *ctx_pp;

    if (ctx->apis)
    {
        L_list_delete_all_node(ctx->apis);

        L_list_free(ctx->apis);
        ctx->apis = NULL;
    }

    if (ctx->api_json)
    {
        json_decref(ctx->api_json);
        ctx->api_json = NULL;
    }

    L_MM_Free(ctx);
    *ctx_pp = NULL;
}


BOOL_T
cgi_rest_add_routes(CGI_REST_CONTEXT_PTR_T self, const char *path, const CGI_API_HANDLER_SET_T *handlers, uint32_t flags)
{
    BOOL_T ret_value = TRUE;

    if (handlers->create_handler)
    {
        ret_value &= CGI_REST_InternalAddRoute(self, path, M_POST, handlers->create_handler, flags);
    }

    if (handlers->update_handler)
    {
        ret_value &= CGI_REST_InternalAddRoute(self, path, M_PUT, handlers->update_handler, flags);
    }

    if (handlers->read_handler)
    {
        ret_value &= CGI_REST_InternalAddRoute(self, path, M_GET, handlers->read_handler, flags);
    }

    if (handlers->delete_handler)
    {
        ret_value &= CGI_REST_InternalAddRoute(self, path, M_DELETE, handlers->delete_handler, flags);
    }

    return ret_value;
}

BOOL_T
cgi_rest_add_route(CGI_REST_CONTEXT_PTR_T self, const char *path, int method, CGI_API_HANDLER_T handler, uint32_t flags)
{
    return CGI_REST_InternalAddRoute(self, path, method, handler, flags);
}

/*const*/ CGI_REST_ROUTE_PTR_T
cgi_rest_find_route(CGI_REST_CONTEXT_PTR_T self, const char *uri, int method)
{
    struct L_listnode *n;
    CGI_REST_ROUTE_PTR_T found;

    for (found = NULL, n = self->apis->head; n; n = n->next)
    {

        if ((re_test(((CGI_REST_ROUTE_PTR_T)n->data)->path, uri) == TRUE) &&
            (((CGI_REST_ROUTE_PTR_T)n->data)->method == method))
        {
            found = (CGI_REST_ROUTE_PTR_T)n->data;
            break;
        }

    }

    return found;
}

CGI_STATUS_CODE_T cgi_rest_call_route_handler(CGI_REST_CONTEXT_PTR_T self, HTTP_Connection_T *http_connection, CGI_REST_ROUTE_PTR_T route)
{
    HTTP_Request_T *http_request;
    HTTP_Response_T *http_response;

    CGI_REQUEST_USER_CONTEXT_PTR_T req_context = NULL;
    CGI_RESPONSE_USER_CONTEXT_PTR_T res_context = NULL;

    CGI_STATUS_CODE_T status_code = CGI_SUCCESS;

    ASSERT(self != NULL);
    ASSERT(http_connection != NULL);
    ASSERT(http_connection->req != NULL);
    ASSERT(http_connection->res != NULL);

    http_request = http_connection->req;
    http_response = http_connection->res;


    req_context = (CGI_REQUEST_USER_CONTEXT_PTR_T)http_request->user_ctx;
    res_context = (CGI_RESPONSE_USER_CONTEXT_PTR_T)http_response->user_ctx;

    // TODO: put into process-handler-arguments
    res_context->responseset = json_array();
    if (res_context->responseset == NULL)
    {
        status_code = CGI_INTERNAL_SERVER_ERROR;
        goto fatal_error;
    }

    CGI_REST_InternalProcessHandlerArguments(self, http_connection, route);

    if (route->handler == NULL)
    {
        json_t *Response; // TODO: use the same way to set error object
        json_t *Error;

        // TODO: Add test case for this !

        status_code = CGI_METHOD_NA;

        json_decref(res_context->responseset);
        res_context->responseset = Response = json_object();

        Error = json_object();

        json_object_set_new(Response, "error", Error);
        json_object_set_new(Response, "status", json_integer(status_code));

        goto failed;
    }

    {
        // json_t *Request;
        json_t *Response;

        CGI_STATUS_CODE_T local_status_code;

        res_context->curr_index = req_context->curr_index;
        Response = json_array_get(res_context->responseset, res_context->curr_index);
        ASSERT(Response != NULL);

        ASSERT(route->handler != NULL);

        req_context->handler = route->handler;

        local_status_code = route->handler(http_request, http_response);

        if (local_status_code == CGI_AGAIN)
        {
            status_code = local_status_code;

            /* Add event !
             */
            {
                HTTP_Event_T _ev;
                memset(&_ev, 0, sizeof(_ev));

                _ev.event_type  = HTTP_EVENT_READ;
                _ev.fd          = http_connection->fds[HTTP_CONN_FD_SYSCB_RCV];
                _ev.data        = http_connection;
                _ev.handler = CGI_REST_InternalHandlerWrapper;

                http_connection->wait_syscb = 1;
                http_event_add(http_connection->worker->event_ctx, &_ev, HTTP_CFG_LONG_POLLING_TIMEOUT_SEC);
            }

            goto fatal_error; // FIXME: using another tag, like wait_SYSCB
        }

        if (local_status_code < 400)
        {
            json_object_del(Response, "error");
        }

        if (400 <= local_status_code)
        {
            json_object_del(Response, "result");
        }

        json_object_set_new(Response, "status", json_integer(local_status_code));

        if (status_code < local_status_code)
        {
            status_code = local_status_code;
        }
    }

failed:

fatal_error:
    return status_code;
}

void cgi_rest_write_response(HTTP_Connection_T *http_connection, CGI_STATUS_CODE_T status_code)
{
    CGI_REST_InternalWriteResponse(http_connection, http_connection->req, http_connection->res, status_code);
}

#if 0 //not used
static void CGI_REST_InternalDeleteRestEntry(CGI_REST_ENTRY_T *entry)
{
    ASSERT(entry != NULL);

    if (entry->uri != NULL)
    {
        L_MM_Free(entry->uri);
        entry->uri = NULL;
    }

    if (entry->parsed_args_syntax != NULL)
    {
        json_decref(entry->parsed_args_syntax);
        entry->parsed_args_syntax = NULL;
    }

    if (entry->parsed_id_syntax != NULL)
    {
        json_decref(entry->parsed_id_syntax);
        entry->parsed_id_syntax = NULL;
    }

    if (entry->handlers != NULL)
    {
        L_MM_Free(entry->handlers);
        entry->handlers = NULL;
    }

    L_MM_Free(entry);
}
#endif

static size_t
cgi_rest_path_level(
	const char *path)
{
    size_t level;
    for (level = 0; *path; path++)
    {
        if (*path == '/')
        {
            ++level;
        }
    }

    return level;
}

static CGI_REST_ROUTE_PTR_T
CGI_REST_InternalNewRoute(
	const char *path,
	int method,
	CGI_API_HANDLER_T handler,
	const json_t *schema,
	UI32_T flags)
{
    CGI_REST_ROUTE_PTR_T route;

    route = (CGI_REST_ROUTE_PTR_T) L_MM_Malloc(sizeof(CGI_REST_ROUTE_T), 123);

    if (route)
    {
        memset(route, 0, sizeof(CGI_REST_ROUTE_T));

        route->path = (char *) L_MM_Malloc(strlen(path) + 1, 123);
        if (route->path == NULL)
        {
            goto malloc_failed;
        }
        memcpy(route->path, path, strlen(path) + 1);

        route->method = method;

        // TODO: the input spec will be a json_t tree
        //       here will convert json tree to Parameter Object (plan A)
        //       deep copy it (plan B)
        if (schema)
        {
            route->schema = json_deep_copy(schema);
            if (route->schema == NULL)
            {
                goto malloc_failed;
            }
        }

        route->handler = handler;
        route->flags = flags;
    }

    if (0)
    {
malloc_failed:
        CGI_REST_InternalDelRoute(&route);
    }

    return route;
}

static void
CGI_REST_InternalDelRoute(
	CGI_REST_ROUTE_PTR_T *route_pp)
{
    CGI_REST_ROUTE_PTR_T ptr;

    ASSERT(route_pp != NULL);
    if (route_pp == NULL)
    {
        return;
    }

    if (*route_pp == NULL)
    {
        return;
    }

    ptr = *route_pp;

    if (ptr->path)
    {
        L_MM_Free(ptr->path);
    }

    if (ptr->schema)
    {
        ASSERT(ptr->schema->refcount == 1);
        json_decref(ptr->schema);
    }

    L_MM_Free(ptr);
    *route_pp = NULL;
}

static I32_T
CGI_REST_InternalCompareRoute(
    void *inlist_element,
    void *input_element)
{
    CGI_REST_ROUTE_PTR_T inlist = (CGI_REST_ROUTE_PTR_T)inlist_element;
    CGI_REST_ROUTE_PTR_T input  = (CGI_REST_ROUTE_PTR_T)input_element;

    size_t inlist_lv;
    size_t input_lv;

    int path_cmp;

    inlist_lv = cgi_rest_path_level(inlist->path);

    input_lv = cgi_rest_path_level(input->path);

    if (inlist_lv < input_lv)
    {
        return 1;
    }
    else if (inlist_lv > input_lv)
    {
        return -1;
    }

    path_cmp = strcmp(inlist->path, input->path) * -1;

    if (path_cmp != 0)
    {
        return path_cmp;
    }

    if (inlist->method < input->method)
    {
        return 1;
    }
    else if (inlist->method > input->method)
    {
        return -1;
    }

    return 0;
}

static void
CGI_REST_InternalFreeRouteListEntry(
    void *element)
{
    CGI_REST_ROUTE_PTR_T e = (CGI_REST_ROUTE_PTR_T)element;

    CGI_REST_InternalDelRoute(&e);
}

static BOOL_T
CGI_REST_InternalAddRoute(
	CGI_REST_CONTEXT_PTR_T self,
	const char *path,
	int method,
	CGI_API_HANDLER_T handler_fn,
	uint32_t flags)
{
    CGI_REST_ROUTE_PTR_T route;
    json_t *spec;

    BOOL_T  add_anyway = flags & CGI_REST_FLAG_ADD_ANYWAY ? TRUE : FALSE;

    const char *method_str = (method == M_POST)   ? "post" :
                             (method == M_PUT)    ? "put" :
                             (method == M_GET)    ? "get" :
                             (method == M_DELETE) ? "delete" : "";

    ASSERT(self != NULL);
//    ASSERT(self->api_json != NULL);
    ASSERT(handler_fn != NULL);

    if (*method_str == '\0')
    {
        return FALSE;
    }

    spec = json_object_get_path(self->api_json, JSON_NO_AUTO_CREATE, "paths", path, method_str, "parameters", NULL);

    if (!spec && !add_anyway)
    {
        return FALSE;
    }

    route = CGI_REST_InternalNewRoute(path, method, handler_fn, spec, flags);

    if (!route)
    {
        return FALSE;
    }

    L_listnode_add_sort_nodup(self->apis, route);
    return TRUE;
}

// TODO: move out !
static CGI_REST_MEDIA_TYPE_T CGI_REST_InternalGetMediaType(const char *content_type)
{
#define MIME_TYPE_LIST                                                                  \
    MIME_TYPE("application/x-www-form-urlencoded",  CGI_REST_MEDIA_TYPE_URL_ENCODED)    \
    MIME_TYPE("application/json",                   CGI_REST_MEDIA_TYPE_JSON)

    struct
    {
        const char *str;
        CGI_REST_MEDIA_TYPE_T type;
    } mime_types[] = {
#define MIME_TYPE(str, type) {str, type},

        MIME_TYPE_LIST

#undef MIME_TYPE
    };

    UI32_T i;
    size_t end_pos;

    if (content_type == NULL)
    {
        return CGI_REST_MEDIA_TYPE_URL_ENCODED;
    }

    end_pos = strcspn(content_type, " ;");

    for (i = 0; i < _countof(mime_types); ++ i)
    {
        if (strlen(mime_types[i].str) == end_pos && strncmp(content_type, mime_types[i].str, end_pos) == 0)
        {
            return mime_types[i].type;
        }
    }

    return CGI_REST_MEDIA_TYPE_UNRECOGNIZED;
}

static BOOL_T CGI_REST_InternalInsertArg(envcfg_t *envcfg, char *name, char *value)
{
    ASSERT(name != NULL);
    ASSERT(value != NULL);

    /* Not allow blank key
     */
    if (name[0] == '\0')
    {
        return FALSE;
    }

    /* Not allow value includes '='
     */
    {
        int pos = strcspn(value, "=");
        if (value[pos] == '=')
        {
            return FALSE;
        }
    }

    {
        int iQuery = 0;
        cgi_query_parse_text(name, (I8_T *) name, &iQuery);
    }

    {
        int iQuery = 0;
        cgi_query_parse_text(value, (I8_T *) value, &iQuery);
    }

    {
        json_t *args = (json_t *) envcfg;
        json_t *js_val = json_string(value);

        if (js_val == NULL)
        {
            return FALSE;
        }

        json_object_set(args, name, js_val);
        json_decref(js_val);
    }

    return TRUE;
}

static json_t * CGI_REST_InternalReadUrlEncodedLine_2(const char *urlencode_line, const json_t *arguments_syntax)
{
    json_t *arguments;
    const char *name;
    json_t *Syntax;

    ASSERT(urlencode_line != NULL);
    ASSERT(arguments_syntax != NULL);

    arguments = json_object();
    if (arguments == NULL)
    {
        return NULL;
    }

    if (CGI_QUERY_ParseQueryString(urlencode_line, CGI_REST_InternalInsertArg, (envcfg_t *) arguments) != TRUE)
    {
        goto read_failed;
    }

    ASSERT(arguments != NULL);
    ASSERT(json_typeof(arguments) == JSON_OBJECT);

    /* convert the universal arguments
     */
    json_object_foreach((json_t *)arguments_syntax, name, Syntax)
    {
        json_t *type = json_object_get(Syntax, "type");

        ASSERT(type != NULL);
        ASSERT(json_typeof(type) == JSON_STRING);

        if (strcmp(json_string_value(type), "number") == 0)
        {
            json_t *arg = json_object_get(arguments, name);

            if (arg != NULL)
            {
                json_int_t new_val;

                /* Blank string is not an valid number
                 */
                if (json_string_value(arg)[0] == '\0')
                {
                    goto read_failed;
                }

                if (StringToInteger(json_string_value(arg), &new_val) != TRUE)
                {
                    goto read_failed;
                }

                json_object_set_new(arguments, name, json_integer(new_val));
            }
        }
    }

    return arguments;

read_failed:
    if (arguments != NULL)
    {
        json_decref(arguments);
    }
    return NULL;
}

static const char* CGI_REST_JSON_Get_As_String(const json_t *obj, const char *key)
{
    json_t *value = json_object_get(obj, key);

    return json_string_value(value);
}

static json_type CGI_REST_JSON_String_To_Type(const char *s)
{
//    JSON_OBJECT,
//    JSON_ARRAY,
//    JSON_STRING,
//    JSON_INTEGER,
//    JSON_REAL,
//    JSON_TRUE,
//    JSON_FALSE,
//    JSON_NULL

    if (!strcmp(s, "object"))
    {
        return JSON_OBJECT;
    }

    if (!strcmp(s, "integer"))
    {
        return JSON_INTEGER;
    }

    if (!strcmp(s, "number"))
    {
        return JSON_INTEGER;
    }

    if (!strcmp(s, "string"))
    {
        return JSON_STRING;
    }

    if (!strcmp(s, "array"))
    {
        return JSON_ARRAY;
    }

    if (!strcmp(s, "boolean"))
    {
        return JSON_TRUE;
    }

    return (json_type)9999;
}

static json_int_t CGI_REST_JSString_To_Value(const json_t *value)
{
    if (json_typeof(value) == JSON_STRING)
    {
        json_int_t new_val;

        if (StringToInteger(json_string_value(value), &new_val) != TRUE)
        {
            return 0;
        }

        return new_val;
    }

    return 0;
}

static json_type CGI_REST_Get_Syntax_Type(const json_t *syntax)
{
    const char *sz_type = CGI_REST_JSON_Get_As_String(syntax, "type");

    if (sz_type)
    {
        return CGI_REST_JSON_String_To_Type(sz_type);
    }

    return JSON_STRING;
}

typedef void(*set_validity_fn_t)(void *cookie, const char *name, const char *_class, const char *reason);

static void CGI_REST_SetValidity(void *cookie, const char *name, const char *_class, const char *reason)
{
    CGI_PARAMETER_T *result = (CGI_PARAMETER_T *) cookie;

    ASSERT(result != NULL);

    result->valid = FALSE;

    if (result->error == NULL)
    {
        result->error = json_object();
    }

    if (result->error)
    {
        json_t *el = json_object_get_path(result->error, 0, name, NULL);
        if (el)
        {
            json_object_set_new(el, _class, json_string(reason));
        }
    }
}

static json_t* CGI_REST_BasicFormatter(const char *name, const json_t *value, const json_t *syntax,
                                       set_validity_fn_t set_validity_fn, void *cookie)
{
    json_t *js_type;

    const char *sz_type;

    ASSERT(value != NULL);
    ASSERT(syntax != NULL);

    js_type = json_object_get(syntax, "type");

    ASSERT(js_type != NULL);
    if (js_type == NULL)
    {
        return NULL;
    }

    sz_type = json_string_value(js_type);

    ASSERT(sz_type != NULL);

    switch (CGI_REST_JSON_String_To_Type(sz_type))
    {
        case JSON_INTEGER:
        {
            if (json_is_string(value))
            {
                json_int_t new_val;

                /* Blank string is not an valid number
                 */
                if (json_string_value(value)[0] == '\0')
                {
                    return NULL;
                }

                if (StringToInteger(json_string_value(value), &new_val) != TRUE)
                {
                    return NULL;
                }

                return json_integer(new_val);
            }
            else if (json_is_false(value))
            {
                return json_integer(0);
            }
            else if (json_is_true(value))
            {
                return json_integer(1);
            }
            else if (!json_is_integer(value))
            {
                return NULL;
            }

            break;
        }

        case JSON_STRING:
            break;

        default:

            if (json_is_string(value))
            {
                if (!strcmp(json_string_value(value), "true"))
                {
                    return json_true();
                }
                else if (!strcmp(json_string_value(value), "false"))
                {
                    return json_false();
                }
            }

            break;
    }

    return json_deep_copy(value);
}

static json_t* CGI_REST_DoNothingFormatter(const char *name, const json_t *value, const json_t *syntax,
                                           set_validity_fn_t set_validity_fn, void *cookie)
{
    return json_deep_copy(value);
}

static json_t* CGI_REST_RunFormatters(const char *name, const json_t *value, const json_t *syntax,
                                      set_validity_fn_t set_validity_fn, void *cookie)
{
    typedef struct
    {
        json_t* (*fn)(const char *, const json_t *, const json_t *, set_validity_fn_t, void *);
    } formatter_t;

    static formatter_t formatters[] =
    {
        {CGI_REST_BasicFormatter},
        {CGI_REST_DoNothingFormatter}
    };

    int i;

    json_t *in_value;
    json_t *out_value = NULL;

    if (value == NULL)
    {
        return NULL;
    }

    in_value = json_deep_copy(value);

    for (i = 0; i < _countof(formatters); ++ i)
    {
        formatter_t *formatter = &formatters[i];

        out_value = formatter->fn(name, in_value, syntax, set_validity_fn, cookie);
        if (out_value == NULL)
        {
            return in_value;
        }

        json_decref(in_value);
        in_value = out_value;
    }

    ASSERT(out_value != NULL);
    return out_value;
}

static BOOL_T CGI_REST_TypeValidator(const char *name, const json_t *value, const json_t *syntax,
                                     set_validity_fn_t set_validity_fn, void *cookie)
{
    json_type type;

    ASSERT(value != NULL);
    ASSERT(syntax != NULL);

    type = CGI_REST_Get_Syntax_Type(syntax);

    if (type != json_typeof(value))
    {
        //
        // FIX: just compare the type
        //
        if (json_typeof(value) == JSON_STRING && type == JSON_INTEGER)
        {
            json_int_t new_val;

            /* Blank string is not an valid number
             */
            if (json_string_value(value)[0] == '\0')
            {
                if (set_validity_fn)
                {
                    set_validity_fn(cookie, name, "type", "blank string is not an valid number");
                }
                return FALSE;
            }

            if (StringToInteger(json_string_value(value), &new_val) != TRUE)
            {
                if (set_validity_fn)
                {
                    set_validity_fn(cookie, name, "type", "convert failed");
                }
                return FALSE;
            }

            return TRUE;
        }

        // FIXME !!
        if (type == JSON_TRUE)
        {
            if (JSON_TRUE == json_typeof(value) || JSON_FALSE == json_typeof(value))
            {
                return TRUE;
            }
        }

        return FALSE;
    }

    return TRUE;
}

static long long CGI_REST_GetLLMin()
{
#if defined(_MSC_VER)
    return _I64_MIN;
#else
    return LONG_MIN;
#endif
}

static long long CGI_REST_GetLLMax()
{
#if defined(_MSC_VER)
    return _I64_MAX;
#else
    return LONG_MAX;
#endif
}

static BOOL_T CGI_REST_RangeValidator(const char *name, const json_t *value, const json_t *syntax,
                                      set_validity_fn_t set_validity_fn, void *cookie)
{
    json_t *js_min;
    json_t *js_max;

    json_int_t int_min = CGI_REST_GetLLMin();
    json_int_t int_max = CGI_REST_GetLLMax();

    ASSERT(value != NULL);
    ASSERT(syntax != NULL);

    js_min = json_object_get(syntax, "minimum");
    if (js_min)
    {
        int_min = json_integer_value(js_min);
    }

    js_max = json_object_get(syntax, "maximum");
    if (js_max)
    {
        int_max = json_integer_value(js_max);
    }

    if (json_typeof(value) == JSON_INTEGER)
    {
        if (json_integer_value(value) < int_min)
        {
            if (set_validity_fn)
            {
                set_validity_fn(cookie, name, "min", "out of range");
            }
            return FALSE;
        }
        if (int_max < json_integer_value(value))
        {
            if (set_validity_fn)
            {
                set_validity_fn(cookie, name, "max", "out of range");
            }
            return FALSE;
        }
    }
    else if (json_typeof(value) == JSON_STRING && CGI_REST_Get_Syntax_Type(syntax) == JSON_INTEGER)
    {
        json_int_t _value = CGI_REST_JSString_To_Value(value);

        if (_value < int_min)
        {
            if (set_validity_fn)
            {
                set_validity_fn(cookie, name, "min", "out of range");
            }
            return FALSE;
        }

        if (int_max < _value)
        {
            if (set_validity_fn)
            {
                set_validity_fn(cookie, name, "max", "out of range");
            }
            return FALSE;
        }
    }
    else if (json_typeof(value) == JSON_STRING && CGI_REST_Get_Syntax_Type(syntax) == JSON_STRING)
    {
        const char *_value = json_string_value(value);
        json_int_t _value_len = strlen(_value);

        ASSERT(_value != NULL);

        if (_value_len < int_min)
        {
            if (set_validity_fn)
            {
                set_validity_fn(cookie, name, "min", "out of range");
            }
            return FALSE;
        }

        if (int_max < _value_len)
        {
            if (set_validity_fn)
            {
                set_validity_fn(cookie, name, "max", "out of range");
            }
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL_T CGI_REST_RequiredValidator(const char *name, const json_t *value, const json_t *syntax,
                                         set_validity_fn_t set_validity_fn, void *cookie)
{
    return TRUE;
}

static BOOL_T CGI_REST_RunValidators(const char *name, const json_t *value, const json_t *syntax,
                                     set_validity_fn_t set_validity_fn, void *cookie)
{
    typedef struct
    {
        BOOL_T (*fn)(const char *, const json_t *, const json_t *, set_validity_fn_t, void *);
    } validator_t;

    static validator_t validators[] =
    {
        {CGI_REST_TypeValidator},
        {CGI_REST_RangeValidator},
        {CGI_REST_RequiredValidator}
    };

    BOOL_T all_valids = TRUE;

    json_type type;

    int i;

    ASSERT(syntax != NULL);

    type = CGI_REST_Get_Syntax_Type(syntax);

    if (!value)
    {
        return FALSE;
    }

    for (i = 0; i < _countof(validators); ++ i)
    {
        validator_t *validator = &validators[i];

        BOOL_T valid = validator->fn(name, value, syntax, set_validity_fn, cookie);

        all_valids = all_valids && valid;
    }

    return all_valids;
}

static json_t* CGI_REST_GetDefaultValue(const char *name, const json_t *syntax)
{
    json_t *value;

    ASSERT(name != NULL);
    ASSERT(syntax != NULL);

    value = json_object_get(syntax, "default");

    return value ? json_deep_copy(value) : NULL;
}

//
// TODO: Split this file into different files
//       api.c
//       parse.c
//       ...

static json_t* CGI_REST_ParseAndValidate(const char *name, const json_t *value, const json_t *syntax, uint32_t flags,
                                         set_validity_fn_t set_validity_fn, void *cookie)
{
    json_t     *view_value;
    json_t     *model_value;

    BOOL_T      valid;
    BOOL_T      allow_invalid = flags & CGI_REST_FLAG_ALLOW_INVALID;

    view_value = (value) ? json_deep_copy(value) : CGI_REST_GetDefaultValue(name, syntax);

    if (view_value == NULL)
    {
        return NULL;
    }

    model_value = CGI_REST_RunFormatters(name, view_value, syntax, set_validity_fn, cookie);
    if (model_value == NULL)
    {
        goto failed;
    }

    json_decref(view_value);
    view_value = NULL;

    valid = CGI_REST_RunValidators(name, model_value, syntax, set_validity_fn, cookie);

    if (!valid && !allow_invalid)
    {
        json_decref(model_value);
        model_value = NULL;
    }

    return model_value;

failed:
    if (view_value != NULL)
    {
        json_decref(view_value);
    }

    return NULL;
}

static json_t *json_object_to_array(const json_t *object)
{
    const char  *key;
    json_t      *value;

    json_t      *array;

    array = json_array();
    if (array == NULL)
    {
        return array;
    }

    json_object_foreach((json_t *)object, key, value)
    {
        json_array_append_new(array, value);
    }

    return array;
}

static BOOL_T CGI_REST_ValidateParameters_2(const char *here, const json_t *view_values, const json_t *params_def_obj, uint32_t flags, CGI_PARAMETER_T *out)
{
    BOOL_T              all_valid;

    CGI_PARAMETER_T     my_result;
    BOOL_T              use_my_result = FALSE;

    my_result.init = FALSE;

    if (!out)
    {
        out = &my_result;
        use_my_result = TRUE;
    }

    if (!out->init)
    {
        memset(out, 0, sizeof(*out)); // TODO: call init fn
        out->valid = TRUE;
    }

    ASSERT(out->value == NULL);
    if (out->value)
    {
        json_decref(out->value);
    }

    out->value = json_object();

    if (!out->value)
    {
        out->valid = FALSE;

        goto failed;
    }

    {

        size_t      index;
        json_t     *param_object;

        json_t     *params_def_array = NULL;

        if (json_typeof(params_def_obj) == JSON_OBJECT)
        {
            params_def_array = json_object_to_array(params_def_obj);
        }

        json_array_foreach((params_def_array ? params_def_array : (json_t *)params_def_obj),
                           index, param_object)
        {
            const char *_name;
            const char *_in;

            json_t *js_name = json_object_get(param_object, "name");
            json_t *js_in = json_object_get(param_object, "in");

            json_t *view_value;
            json_t *model_value;

            ASSERT(js_name != NULL);
            ASSERT(js_in != NULL);

            if (!js_name || !js_in)
            {
                continue;
            }

            _name = json_string_value(js_name);
            _in = json_string_value(js_in);

            if (strcmp(here, _in))
            {
                continue;
            }

            view_value = json_object_get(view_values, _name);
            model_value = CGI_REST_ParseAndValidate(_name, view_value, param_object, flags,
                                                    CGI_REST_SetValidity, out);

            if (model_value)
            {
                ASSERT(out->value != NULL);

                json_object_set(out->value, json_string_value(js_name), model_value);

                json_decref(model_value);
            }
        } // end of foreach

        if (params_def_array)
        {
            json_decref(params_def_array);
            params_def_array = NULL;
        }

    }

failed:
    all_valid = out->valid;

    if (out && use_my_result)
    {
        if (out->value)
        {
            json_decref(out->value);
            out->value = NULL;
        }

        if (out->error)
        {
            json_decref(out->error);
            out->error = NULL;
        }
    }

    return all_valid;
}

static json_t* CGI_REST_InternalReadUrlEncodedLine(const char *urlencode_line, const json_t *arguments_syntax, CGI_PARAMETER_T *result)
{
    ASSERT(arguments_syntax != NULL);
    ASSERT(result != NULL);

    ASSERT(result->view_value == NULL);

    result->view_value = json_object();
    if (result->view_value == NULL)
    {
        return NULL;
    }

    if (urlencode_line == NULL)
    {
        return result->view_value;
    }

    if (CGI_QUERY_ParseQueryString(urlencode_line, CGI_REST_InternalInsertArg, (envcfg_t *) result->view_value) != TRUE)
    {
        goto read_failed;
    }

    ASSERT(result->view_value != NULL);
    ASSERT(json_typeof(result->view_value) == JSON_OBJECT);

    return result->view_value;

read_failed:
    if (result->view_value != NULL)
    {
        json_decref(result->view_value);
        result->view_value = NULL;
    }
    return NULL;
}

#if 0 //not used
static json_t * CGI_REST_InternalReadArgumentsFromText(CGI_REST_MEDIA_TYPE_T media_type, const char *query_string, const json_t *arguments_syntax)
{
    json_t *arguments = NULL;

    if (media_type == CGI_REST_MEDIA_TYPE_UNRECOGNIZED)
    {
        return NULL;
    }

    if (query_string == NULL)
    {
        arguments = json_object();
        return arguments;
    }

    if (media_type == CGI_REST_MEDIA_TYPE_URL_ENCODED)
    {
        arguments = CGI_REST_InternalReadUrlEncodedLine_2(query_string, arguments_syntax);
    }
    else if (media_type == CGI_REST_MEDIA_TYPE_JSON)
    {
        json_error_t error;

        arguments = json_loads(query_string, JSON_DECODE_ANY | JSON_REJECT_DUPLICATES, &error);
    }

    return arguments;
}
#endif

static BOOL_T CGI_REST_InternalIsCapitalLetter(const char *letter, size_t len)
{
    return (letter && 2 < len && *letter == '{' && letter[len - 1] == '}') ? TRUE : FALSE;
}

// TODO: move to substring.c
static char* CGI_REST_InternalSubString(const char *s, size_t start_index, size_t len)
{
    char *sub = (char *)calloc(len + 1, sizeof(char));

    if (sub)
    {
        memcpy( sub, &s[start_index], len );
    }

    return sub;
}

static json_t* CGI_REST_InternalReadArgumentsFromPath(const char *path, const char *templating)
{
    const char *next_path = path + 1;
    const char *next_templating = templating + 1;

    size_t end_path;
    size_t end_templating;

    json_t *object = json_object();

    if (!object)
    {
        return NULL;
    }

    for (;;)
    {
        end_path = strcspn(next_path, "/");
        end_templating = strcspn(next_templating, "/");

//        printf("%.*s\r\n", end_path, next_path);
//        printf("%.*s\r\n", end_templating, next_templating);

        if (CGI_REST_InternalIsCapitalLetter(next_templating, end_templating))
        {
            char *key = CGI_REST_InternalSubString(next_templating, 1, end_templating - 2);
            char *value = CGI_REST_InternalSubString(next_path, 0, end_path);

            if (key && value)
            {
                json_object_set_new(object, key, json_string(value));
            }

            free(key);
            free(value);
        }

        if (next_path[end_path] != '/' || next_templating[end_templating] != '/')
        {
            break;
        }

        end_path += 1;
        end_templating += 1;

        next_path += end_path;
        next_templating += end_templating;
    }

    return object;
}

static void CGI_REST_ReadURIPath(const char *here, const char *raw, size_t raw_len, CGI_REST_PARSER_T *parser, CGI_PARAMETER_T *out)
{
    out->view_value = CGI_REST_InternalReadArgumentsFromPath(raw, parser->route->path);
}

static void CGI_REST_ReadURIQuery(const char *here, const char *raw, size_t raw_len, CGI_REST_PARSER_T *parser, CGI_PARAMETER_T *out)
{
    out->view_value = CGI_REST_InternalReadUrlEncodedLine(raw, parser->route->schema, out);
}

static void CGI_REST_ReadBody(const char *here, const char *raw, size_t raw_len, CGI_REST_PARSER_T *parser, CGI_PARAMETER_T *out)
{
    const char *content_type;
    CGI_REST_MEDIA_TYPE_T mime_type;

    ASSERT(parser != NULL);
    ASSERT(parser->req != NULL);
    ASSERT(parser->req->envcfg != NULL);

    content_type = get_env(parser->req->envcfg, "CONTENT_TYPE");

    mime_type = CGI_REST_InternalGetMediaType(content_type);

    if (mime_type == CGI_REST_MEDIA_TYPE_JSON)
    {
        json_error_t err;
        out->view_value = json_loads(raw, JSON_DECODE_ANY | JSON_REJECT_DUPLICATES, &err); //squid
        if (out->view_value == NULL)
        {
            CGI_REST_SetValidity(out, "$", "parse", err.text);
        }
    }
    else
    {
        out->view_value = CGI_REST_InternalReadUrlEncodedLine(raw, parser->route->schema, out);
    }
}

static void CGI_REST_Validator(const char *here, const json_t *view_value, CGI_REST_PARSER_T *parser, CGI_PARAMETER_T *out)
{
    ASSERT(parser != NULL);
    ASSERT(parser->route != NULL);
    ASSERT(parser->route->schema != NULL);

    CGI_REST_ValidateParameters_2(here, view_value, parser->route->schema, parser->route->flags, out);
}

static void CGI_REST_InternalRunParser(const char *here,
                                const char *raw, size_t raw_len,
                                CGI_REST_PARSER_T *parser,
                                CGI_PARAMETER_T *out)
{
    ASSERT(parser != NULL);
    ASSERT(parser->read_fn != NULL);
    ASSERT(out != NULL);

    memset(out, 0, sizeof(*out));
    out->init = TRUE;
    out->valid = TRUE;

    out->raw_value.ptr = (char *)raw;
    out->raw_value.size = raw_len;
    out->raw_value.ref_count = 0xffffff;

    if (parser->read_fn)
    {
        parser->read_fn(here, raw, raw_len, parser, out);
    }

    ASSERT(out->view_value || out->error);

    if (parser->validator_fn)
    {
        parser->validator_fn(here, out->view_value, parser, out);
    }
    else
    {
        if (out->view_value)
        {
            out->value = json_deep_copy(out->view_value);
        }
        else
        {
            out->value = json_object();
        }
    }

    if (!out->value)
    {
        out->valid = FALSE;
    }
}

static CGI_STATUS_CODE_T CGI_REST_InternalProcessHandlerArguments(CGI_REST_CONTEXT_PTR_T self, HTTP_Connection_T *http_connection, CGI_REST_ROUTE_PTR_T route)
{
    CGI_REQUEST_USER_CONTEXT_PTR_T req_context;
    CGI_RESPONSE_USER_CONTEXT_PTR_T res_context;

    const char *qs;
    const char *path;
    const char *body;

    CGI_REST_PARSER_T parser;

    CGI_STATUS_CODE_T status_code = CGI_SUCCESS;

    ASSERT(http_connection != NULL);
    ASSERT(http_connection->req != NULL);
    ASSERT(http_connection->res != NULL);
    ASSERT(http_connection->req->user_ctx != NULL);

    req_context = (CGI_REQUEST_USER_CONTEXT_PTR_T)http_connection->req->user_ctx;
    res_context = (CGI_RESPONSE_USER_CONTEXT_PTR_T)http_connection->res->user_ctx;

    /* parse input arguments
     */
    qs = get_env(http_connection->req->envcfg, "QUERY_STRING");
    path = get_env(http_connection->req->envcfg, "URI");
    body = get_env(http_connection->req->envcfg, "BODY");

    parser.req = http_connection->req;
    parser.route = route;

    parser.read_fn = CGI_REST_ReadURIQuery;
    parser.validator_fn = CGI_REST_Validator;

    CGI_REST_InternalRunParser("query", qs, qs ? strlen(qs) : 0, &parser,
                               &req_context->query);

    parser.read_fn = CGI_REST_ReadURIPath;

    CGI_REST_InternalRunParser("path", path, path ? strlen(path) : 0, &parser,
                               &req_context->params);

    parser.read_fn = CGI_REST_ReadBody;

    CGI_REST_InternalRunParser("body", body, body ? strlen(body) : 0, &parser,
                               &req_context->body);


    //
    // Prepare args and response object
    //
    {
        // json_t *Request;
        json_t *Response;
        json_t *Result;
        json_t *Error;

        {
            Response = json_object();

            if (Response == NULL)
            {
                goto malloc_failed;
            }

            json_array_append_new(res_context->responseset, Response);

            Result = json_object();

            if (Result == NULL)
            {
                goto malloc_failed;
            }

            json_object_set_new(Response, "result", Result);

            Error = json_object();

            if (Error == NULL)
            {
                goto malloc_failed;
            }

            json_object_set_new(Response, "error", Error);
        }

        if (0)
        {
        malloc_failed:
            status_code = CGI_INTERNAL_SERVER_ERROR;
            json_decref(res_context->responseset);
            res_context->responseset = NULL;
        }
    }

    return status_code;
}

static void CGI_REST_InternalWriteResponse(HTTP_Connection_T *http_connection, const HTTP_Request_T *http_request, const HTTP_Response_T *http_response, CGI_STATUS_CODE_T status_code)
{
    CGI_REQUEST_USER_CONTEXT_T *req_context;
    CGI_RESPONSE_USER_CONTEXT_T *res_context;

    char *body_text = NULL;
    const char *callback = NULL;

    UI32_T content_length = 0;

    char line[100];

    ASSERT(http_request != NULL);
    ASSERT(http_response != NULL);
    ASSERT(http_request->user_ctx != NULL);
    ASSERT(http_response->user_ctx != NULL);

    req_context = (CGI_REQUEST_USER_CONTEXT_PTR_T)http_request->user_ctx;
    res_context = (CGI_RESPONSE_USER_CONTEXT_PTR_T)http_response->user_ctx;

    /* lookup callback from request
     */
    if (req_context->requestset != NULL)
    {
        json_t *_request;

        ASSERT(json_typeof(req_context->requestset) == JSON_ARRAY);
        ASSERT(0 < json_array_size(req_context->requestset));

        _request = json_array_get(req_context->requestset, 0);

        ASSERT(_request != NULL);
        if (_request != NULL)
        {
            json_t *_arguments = json_object_get(_request, "arguments");
            ASSERT(_arguments != NULL);
            if (_arguments != NULL)
            {
                json_t *_callback = json_object_get(_arguments, "callback");
                if (_callback != NULL)
                {
                    callback = json_string_value(_callback);
                }
            }
        }
    }

    //
    // Send response text out
    //
    if (res_context->responseset != NULL)
    {
        if (json_typeof(res_context->responseset) == JSON_OBJECT)
        {
            body_text = json_dumps(res_context->responseset, JSON_SORT_KEYS);
        }
        else if (json_typeof(res_context->responseset) == JSON_ARRAY)
        {
            const char *bulk = get_env(http_request->envcfg, "BULK");

            if (bulk != NULL)
            {
                body_text = json_dumps(res_context->responseset, JSON_SORT_KEYS);
            }
            else
            {
                json_t *Response = json_array_get(res_context->responseset, 0);

                ASSERT(Response != NULL);
                if (Response != NULL)
                {
                    body_text = json_dumps(Response, JSON_SORT_KEYS);
                }
            }
        }
    }

    if (body_text != NULL)
    {
        content_length = strlen(body_text);
        if (callback != NULL)
        {
            content_length += strlen(callback);
            content_length += 2; /* () */
        }
    }
    else
    {
        content_length = 0;
    }

    // status_line:
    // OK --> reasonPhrase

    /* When the status code is 100 with body, we send 200 instead.
     */
    if (status_code == 100 && body_text != NULL)
    {
        status_code = CGI_SUCCESS;
    }

    snprintf(line, sizeof(line), "HTTP/1.0 %d %s\r\n", status_code, /*"OK"*/ CGI_REST_InternalGetHttpReasonPhrase(status_code));
    cgi_SendText(http_connection, http_response->fd, (const char *)line);
    //    if (statusCode === 204 || statusCode === 304 ||
    //        (100 <= statusCode && statusCode <= 199)) {
    // will no body, create another one ??


    // headers
    if (res_context->response_headers != NULL)
    {
        const char *field;
        json_t *value;

        json_object_foreach(res_context->response_headers, field, value)
        {
            if (strcmp(field, "Content-Length") == 0 ||
                strcmp(field, "Content-Type") == 0)
            {
                continue;
            }

            cgi_SendText(http_connection, http_response->fd, (const char *)field);
            cgi_SendText(http_connection, http_response->fd, (const char *)":");
            cgi_SendText(http_connection, http_response->fd, (const char *)json_string_value(value));
            cgi_SendText(http_connection, http_response->fd, (const char *)"\r\n");
        }
    }

    {
        snprintf(line, sizeof(line), "Content-Length: %lu\r\n", content_length);
        cgi_SendText(http_connection, http_response->fd, (const char *)line);

        cgi_SendText(http_connection, http_response->fd, (const char *)"Content-Type: application/json\r\n");
    }

    cgi_SendText(http_connection, http_response->fd, (const char *)"\r\n");

    if (body_text != NULL)
    {
        if (callback != NULL)
        {
            cgi_SendText(http_connection, http_request->fd, callback);
            cgi_SendText(http_connection, http_request->fd, (const char *)"(");
        }

        cgi_SendText(http_connection, http_request->fd, (const char *)body_text);

        if (callback != NULL)
        {
            cgi_SendText(http_connection, http_request->fd, (const char *)")");
        }

        free(body_text);
        body_text = NULL;
    }

    cgi_response_end(http_connection);
}

static const char * CGI_REST_InternalGetHttpReasonPhrase(CGI_STATUS_CODE_T status_code)
{
#define HTTP_REASON_PHRASE_TABLE            \
    HTTP_REASON_PHRASE(100, "Continue")     \
    HTTP_REASON_PHRASE(200, "OK")           \
    HTTP_REASON_PHRASE(400, "Bad Request")  \
    HTTP_REASON_PHRASE(403, "Forbidden")    \
    HTTP_REASON_PHRASE(404, "Not Found")    \
    HTTP_REASON_PHRASE(405, "Method NA")    \
    HTTP_REASON_PHRASE(500, "Internal Server Error")

    enum
    {
        _THE_END = 0L
    };

    typedef struct
    {
        UI32_T status_code;
        const char *reason_phrase;
    } http_reason_phrase_t;

    static http_reason_phrase_t table[] =
    {
#define HTTP_REASON_PHRASE(status_code, reason_phrase) {status_code, reason_phrase},

        HTTP_REASON_PHRASE_TABLE

#undef HTTP_REASON_PHRASE

        {_THE_END, "Internal Server Error"}
    };

    http_reason_phrase_t *ent = &table[0];

    for (; ent->status_code != _THE_END; ++ ent)
    {
        if (ent->status_code == status_code)
        {
            return ent->reason_phrase;
        }
    }

    return table[_countof(table) - 1].reason_phrase;
}


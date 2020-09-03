#include "cgi.h"
#include "cgi_response.h"
#include "cgi_jss.c"

CGI_RESPONSE_USER_CONTEXT_T * CGI_REST_InternalNewResponseUserContext()
{
    CGI_RESPONSE_USER_CONTEXT_PTR_T context = (CGI_RESPONSE_USER_CONTEXT_PTR_T)L_MM_Malloc(sizeof(CGI_RESPONSE_USER_CONTEXT_T), 0xB0);

    if (context == NULL)
    {
        return NULL;
    }

    memset(context, 0, sizeof(*context));

    context->js_object = js_object;
    context->js_add_string = js_add_string;
    context->js_add_number = js_add_number;
    context->js_end = js_end;
    context->js_cleanup = js_cleanup;

    return context;
}

void CGI_REST_InternalDeleteResponseUserContext(CGI_RESPONSE_USER_CONTEXT_T **context_pp)
{
    CGI_RESPONSE_USER_CONTEXT_PTR_T ptr;

    ASSERT(context_pp != NULL);
    if (context_pp == NULL)
    {
        // TODO: log
        return;
    }

    if (*context_pp == NULL)
    {
        return;
    }

    ptr = *context_pp;
    if (ptr->response_headers != NULL)
    {
        json_decref(ptr->response_headers);
        ptr->response_headers = NULL;
    }

    if (ptr->responseset != NULL)
    {
        json_decref(ptr->responseset);
        ptr->responseset = NULL;
    }

    L_MM_Free(ptr);
    *context_pp = NULL;
}

json_t * CGI_RESPONSE_GetResponse(HTTP_Response_T *http_response)
{
    CGI_RESPONSE_USER_CONTEXT_PTR_T context = (CGI_RESPONSE_USER_CONTEXT_PTR_T) http_response->user_ctx;
    json_t *Response;

    ASSERT(context->responseset != NULL);
    if (context->responseset == NULL)
    {
        // TODO: log
        return NULL;
    }

    Response = json_array_get(context->responseset, context->curr_index);
    ASSERT(Response != NULL);
    if (Response == NULL)
    {
        // TODO: log
        return NULL;
    }

    return Response;
}

json_t * CGI_RESPONSE_GetResult(HTTP_Response_T *http_response)
{
    json_t *Response = (json_t *) CGI_RESPONSE_GetResponse(http_response);
    json_t *Result;

    Result = json_object_get(Response, "result");
    ASSERT(Result != NULL);

    return Result;
}

json_t * CGI_RESPONSE_GetError(HTTP_Response_T *http_response)
{
    json_t *Response = (json_t *) CGI_RESPONSE_GetResponse(http_response);
    json_t *Error;

    Error = json_object_get(Response, "error");
    ASSERT(Error != NULL);

    return Error;
}

// TODO: Add variables parameters
CGI_STATUS_CODE_T CGI_RESPONSE_Error(HTTP_Response_T *http_response, CGI_STATUS_CODE_T status_code, const char *code, const char *message)
{
    json_t *error = (json_t *) CGI_RESPONSE_GetError(http_response);

    ASSERT(error != NULL);
    if (error != NULL)
    {
        json_object_set_new(error, "code", json_string(code));
        json_object_set_new(error, "message", json_string(message));
    }

    return status_code;
}

CGI_STATUS_CODE_T CGI_RESPONSE_Success(HTTP_Response_T *http_response, CGI_STATUS_CODE_T status_code)
{
    json_t *Response = (json_t *) CGI_RESPONSE_GetResponse(http_response);

    ASSERT(Response != NULL);
    if (Response != NULL)
    {
        json_object_set_new(Response, "status", json_integer(status_code));
    }

    return status_code;
}

CGI_STATUS_CODE_T CGI_RESPONSE_Again(HTTP_Response_T *http_response)
{
    return CGI_AGAIN;
}

void CGI_RESPONSE_SetHeader(HTTP_Response_T *http_response, const char *field, const char *value)
{
    CGI_RESPONSE_USER_CONTEXT_PTR_T context = (CGI_RESPONSE_USER_CONTEXT_PTR_T) http_response->user_ctx;

    ASSERT(field != NULL);
    ASSERT(value != NULL);

    if (field == NULL || value == NULL)
    {
        // TODO: log
        return;
    }

    if (context->response_headers == NULL)
    {
        context->response_headers = json_object();
        if (context->response_headers == NULL)
        {
            // TODO: log
            return;
        }
    }

    json_object_set_new(context->response_headers, field, json_string(value));
}

const char * CGI_RESPONSE_GetHeader(HTTP_Response_T *http_response, const char *field)
{
    CGI_RESPONSE_USER_CONTEXT_PTR_T context = (CGI_RESPONSE_USER_CONTEXT_PTR_T) http_response->user_ctx;
    json_t *value;

    ASSERT(field != NULL);

    if (field == NULL)
    {
        // TODO: log
        return NULL;
    }

    if (context->response_headers == NULL)
    {
        // TODO: log
        return NULL;
    }

    value = json_object_get(context->response_headers, field);

    ASSERT((value != NULL && json_typeof(value) == JSON_STRING) ||
           value == NULL);

    return json_string_value(value);
}

#include "cgi.h"
#include "cgi_request.h"
#include "jansson.h"

void CGI_REQUEST_InitParameter(CGI_PARAMETER_PTR_T param)
{
    ASSERT(param != NULL);

    memset(param, 0, sizeof(*param));
    param->init = TRUE;
    param->valid = TRUE;
}

void CGI_REQUEST_FreeParameter(CGI_PARAMETER_PTR_T param)
{
    if (param)
    {
        if (param->view_value)
        {
            json_decref(param->view_value);
            param->view_value = NULL;
        }

        if (param->value)
        {
            json_decref(param->value);
            param->value = NULL;
        }

        if (param->error)
        {
            json_decref(param->error);
            param->error = NULL;
        }
    }
}

CGI_REQUEST_USER_CONTEXT_T * CGI_REST_InternalNewRequestUserContext()
{
    CGI_REQUEST_USER_CONTEXT_PTR_T context = (CGI_REQUEST_USER_CONTEXT_PTR_T)L_MM_Malloc(sizeof(CGI_REQUEST_USER_CONTEXT_T), 0xA0);

    if (context != NULL)
    {
        memset(context, 0, sizeof(*context));

        CGI_REQUEST_InitParameter(&context->query);
        CGI_REQUEST_InitParameter(&context->params);
        CGI_REQUEST_InitParameter(&context->body);
    }

    return context;
}

void CGI_REST_InternalDeleteRequestUserContext(CGI_REQUEST_USER_CONTEXT_T ** context_pp)
{
    CGI_REQUEST_USER_CONTEXT_PTR_T ptr;

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
    if (ptr->requestset != NULL)
    {
        json_decref(ptr->requestset);
        ptr->requestset = NULL;
    }

    CGI_REQUEST_FreeParameter(&ptr->query);
    CGI_REQUEST_FreeParameter(&ptr->params);
    CGI_REQUEST_FreeParameter(&ptr->body);

    L_MM_Free(ptr);
    *context_pp = NULL;
}

json_t *CGI_REQUEST_GetParams(HTTP_Request_T *http_request)
{
    CGI_REQUEST_USER_CONTEXT_PTR_T context;

    ASSERT(http_request != NULL);

    if (!http_request || !http_request->user_ctx)
    {
        return NULL;
    }

    context = (CGI_REQUEST_USER_CONTEXT_PTR_T) http_request->user_ctx;
    return context->params.value;
}

json_t *CGI_REQUEST_GetParamsValue(HTTP_Request_T *http_request, const char *key)
{
    json_t *params = CGI_REQUEST_GetParams(http_request);

    if (params)
    {
        return json_object_get(params, key);
    }

    return NULL;
}

json_t *CGI_REQUEST_GetQuery(HTTP_Request_T *http_request)
{
    CGI_REQUEST_USER_CONTEXT_PTR_T context;

    ASSERT(http_request != NULL);

    if (!http_request || !http_request->user_ctx)
    {
        return NULL;
    }

    context = (CGI_REQUEST_USER_CONTEXT_PTR_T) http_request->user_ctx;
    return context->query.value;
}

json_t *CGI_REQUEST_GetQueryValue(HTTP_Request_T *http_request, const char *key)
{
    json_t *params = CGI_REQUEST_GetQuery(http_request);

    if (params)
    {
        return json_object_get(params, key);
    }

    return NULL;
}

json_t *CGI_REQUEST_GetBody(HTTP_Request_T *http_request)
{
    CGI_REQUEST_USER_CONTEXT_PTR_T context;

    ASSERT(http_request != NULL);

    if (!http_request || !http_request->user_ctx)
    {
        return NULL;
    }

    context = (CGI_REQUEST_USER_CONTEXT_PTR_T) http_request->user_ctx;
    return context->body.value;
}

json_t *CGI_REQUEST_GetBodyValue(HTTP_Request_T *http_request, const char *key)
{
    json_t *params = CGI_REQUEST_GetBody(http_request);

    if (params)
    {
        return json_object_get(params, key);
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// don't use below this line
//------------------------------------------------------------------------------

void * CGI_REQUEST_GetArguments(HTTP_Request_T *http_request)
{
    CGI_REQUEST_USER_CONTEXT_PTR_T context = (CGI_REQUEST_USER_CONTEXT_PTR_T) http_request->user_ctx;
    json_t *Request;
    json_t *Args;

    ASSERT(context->requestset != NULL);
    if (context->requestset == NULL)
    {
        // TODO: log
        return NULL;
    }

    Request = json_array_get(context->requestset, context->curr_index);
    ASSERT(Request != NULL);
    if (Request == NULL)
    {
        // TODO: log
        return NULL;
    }

    Args = json_object_get(Request, "arguments");
    ASSERT(Args != NULL);

    return Args;
}

void * CGI_REQUEST_GetUriId(HTTP_Request_T *http_request)
{
    json_t *Args = (json_t *) CGI_REQUEST_GetArguments(http_request);

    ASSERT(Args != NULL);
    if (Args == NULL)
    {
        // TODO: log
        return NULL;
    }

    return json_object_get(Args, "id");
}

void * CGI_REQUEST_GetUriIdAtIndex(HTTP_Request_T *http_request, size_t index)
{
    json_t *Id = (json_t *) CGI_REQUEST_GetUriId(http_request);

    if (Id == NULL)
    {
        return NULL;
    }

    return json_array_get(Id, index);
}

void * CGI_REQUEST_GetStart(HTTP_Request_T *http_request)
{
    json_t *Args = (json_t *) CGI_REQUEST_GetArguments(http_request);

    ASSERT(Args != NULL);
    if (Args == NULL)
    {
        // TODO: log
        return NULL;
    }

    return json_object_get(Args, "start");
}

void * CGI_REQUEST_GetStartAtIndex(HTTP_Request_T *http_request, size_t index)
{
    json_t *Start = (json_t *) CGI_REQUEST_GetStart(http_request);

    if (Start == NULL)
    {
        return NULL;
    }

    return json_array_get(Start, index);
}

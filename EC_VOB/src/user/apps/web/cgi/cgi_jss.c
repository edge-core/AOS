
/* TODO: 1. shall check return value from write_response
 *       2. char buffer[100]; -> allocate new buffer on session
 *       3. command "(function ...", "r.add(...)" ...etc. -> use macro -> read from a file
 *       4. all SSI case shall modify to use this method
 *       5. need a config from html to decide cgi's output (JSON or command)
 */

static CGI_STATUS_CODE_T js_object(HTTP_Response_T *http_resp)
{
    CGI_RESPONSE_USER_CONTEXT_T *ctx = (CGI_RESPONSE_USER_CONTEXT_T *) http_resp->user_ctx;

    if (ctx->is_send_create_object == 0)
    {
        http_resp->write_response(http_resp, "(function (){var r = js_object();", sizeof("(function (){var r = js_object();") - 1);
        ctx->is_send_create_object = 1;
    }
    return CGI_SUCCESS;
}

static CGI_STATUS_CODE_T js_add_string(HTTP_Response_T *http_resp, const char *key, const char *val)
{
    CGI_RESPONSE_USER_CONTEXT_T *ctx = (CGI_RESPONSE_USER_CONTEXT_T *) http_resp->user_ctx;
    char buffer[100];
    int len;

    // TODO: shall check length
    len = snprintf(buffer, sizeof(buffer), "r.add('%s','%s');", key, val);
    buffer[sizeof(buffer)-1] = '\0';

    ctx->js_object(http_resp);
    http_resp->write_response(http_resp, buffer, len);

    return CGI_SUCCESS;
}

static CGI_STATUS_CODE_T js_add_number(HTTP_Response_T *http_resp, const char *key, const char *val)
{
    CGI_RESPONSE_USER_CONTEXT_T *ctx = (CGI_RESPONSE_USER_CONTEXT_T *) http_resp->user_ctx;
    char buffer[100];
    int len;

    // TODO: shall check length
    len = snprintf(buffer, sizeof(buffer), "r.add('%s',%s);", key, val);
    buffer[sizeof(buffer)-1] = '\0';

    ctx->js_object(http_resp);
    http_resp->write_response(http_resp, buffer, len);

    return CGI_SUCCESS;
}

static CGI_STATUS_CODE_T js_end(HTTP_Response_T *http_resp)
{
    CGI_RESPONSE_USER_CONTEXT_T *ctx = (CGI_RESPONSE_USER_CONTEXT_T *) http_resp->user_ctx;

    if (ctx->is_send_end_string == 0)
    {
        ctx->js_object(http_resp);
        http_resp->write_response(http_resp, "return r;})()", sizeof("return r;})()")-1);
        ctx->is_send_end_string = 1;
    }

    return CGI_SUCCESS;
}

static CGI_STATUS_CODE_T js_cleanup(HTTP_Response_T *http_resp)
{
    http_resp->write_response(http_resp, ".cleanup()", sizeof(".cleanup()")-1);
    return CGI_SUCCESS;
}
#ifndef _CGI_RESPONSE_H_
#define _CGI_RESPONSE_H_

#include "http_def.h"

#if __cplusplus
extern "C" {
#endif

#define CGI_RESPONSE_ERROR(status_code, code, message) CGI_RESPONSE_Error(http_response, status_code, code, message)
#define CGI_RESPONSE_SUCCESS(status_code) CGI_RESPONSE_Success(http_response, status_code)
#define CGI_RESPONSE_AGAIN() CGI_RESPONSE_Again(http_response)

CGI_RESPONSE_USER_CONTEXT_T * CGI_REST_InternalNewResponseUserContext();

void CGI_REST_InternalDeleteResponseUserContext(CGI_RESPONSE_USER_CONTEXT_T **context_pp);

json_t * CGI_RESPONSE_GetResponse(HTTP_Response_T *http_response);
json_t * CGI_RESPONSE_GetResult(HTTP_Response_T *http_response);
json_t * CGI_RESPONSE_GetError(HTTP_Response_T *http_response);

CGI_STATUS_CODE_T CGI_RESPONSE_Error(HTTP_Response_T *http_response, CGI_STATUS_CODE_T status_code, const char *code, const char *message);
CGI_STATUS_CODE_T CGI_RESPONSE_Success(HTTP_Response_T *http_response, CGI_STATUS_CODE_T status_code);
CGI_STATUS_CODE_T CGI_RESPONSE_Again(HTTP_Response_T *http_response);

void CGI_RESPONSE_SetHeader(HTTP_Response_T *http_response, const char *field, const char *value);
const char * CGI_RESPONSE_GetHeader(HTTP_Response_T *http_response, const char *field);

#if __cplusplus
}
#endif

#endif /* _CGI_RESPONSE_H_ */

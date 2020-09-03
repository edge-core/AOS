#ifndef _CGI_REQUEST_H_
#define _CGI_REQUEST_H_

#include "http_def.h"

#if __cplusplus
extern "C" {
#endif

CGI_REQUEST_USER_CONTEXT_T * CGI_REST_InternalNewRequestUserContext();
void CGI_REST_InternalDeleteRequestUserContext(CGI_REQUEST_USER_CONTEXT_T ** context_pp);

void * CGI_REQUEST_GetArguments(HTTP_Request_T *http_request);

void * CGI_REQUEST_GetUriId(HTTP_Request_T *http_request);
void * CGI_REQUEST_GetUriIdAtIndex(HTTP_Request_T *http_request, size_t index);

void * CGI_REQUEST_GetStart(HTTP_Request_T *http_request);
void * CGI_REQUEST_GetStartAtIndex(HTTP_Request_T *http_request, size_t index);

/* forward declaration for json_t
 */
typedef struct json_t json_t;

json_t *CGI_REQUEST_GetQueryValue(HTTP_Request_T *http_request, const char *key);

json_t *CGI_REQUEST_GetParamsValue(HTTP_Request_T *http_request, const char *key);

json_t *CGI_REQUEST_GetBodyValue(HTTP_Request_T *http_request, const char *key);

#if __cplusplus
}
#endif

#endif /* _CGI_REQUEST_H_ */

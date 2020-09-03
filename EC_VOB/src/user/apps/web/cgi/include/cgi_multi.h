
#ifndef CGI_MULTIPART_H
#define CGI_MULTIPART_H

#if __cplusplus
extern "C" {
#endif

/**----------------------------------------------------------------------
 * To spawn a thread to do HTTP upgrade
 *
 * @param  con      connection object
 * @return          status code
 * ---------------------------------------------------------------------- */
void CGI_MULTI_AsyncUploadFile(HTTP_Event_T *event);

/**----------------------------------------------------------------------
 * To process HTTP upgrade
 *
 * @param  con      connection object
 * ---------------------------------------------------------------------- */
void CGI_MULTI_UploadFileMain(HTTP_Event_T *event);

/**----------------------------------------------------------------------
 * To set HTTP upgrade flag
 *
 * @param  status   The value of status
 * @return          TRUE means success; elsewise FALSE
 * ---------------------------------------------------------------------- */
BOOL_T CGI_MULTI_SetHttpUpgradeFlag(UI32_T status);

/**----------------------------------------------------------------------
 * To get HTTP upgrade flag
 *
 * @param  status   The value of status will be one of the following
 *                  CGI_TYPE_HTTP_UPGRADE_NOT_COPY, CGI_TYPE_HTTP_UPGRADE_COPYING
 * ---------------------------------------------------------------------- */
void CGI_MULTI_GetHttpUpgradeFlag(UI32_T *status);

/**----------------------------------------------------------------------
 * To free request buffer
 *
 * @param  http_connection   connection object
 * ---------------------------------------------------------------------- */
void CGI_MULTI_Exit(HTTP_Connection_T *http_connection);

/**----------------------------------------------------------------------
 * Parse the receive packets
 *
 * @param  http_connection
 * @param  sock
 * @param  envQuery
 * @param  envcfg
 * @param  firstPkt_P
 * @param  firstPkt_len
 * ---------------------------------------------------------------------- */
extern void cgi_multipart (HTTP_Connection_T *http_connection, int sock, envcfg_t *envQuery, envcfg_t *envcfg, char *firstPkt_P, int firstPkt_len);

#if __cplusplus
}
#endif

#endif  /* CGI_MULTIPART_H */

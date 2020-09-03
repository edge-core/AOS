/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_file.h

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by project manager.

 * Module for Fileless CGI.
 * File to send file to socket.

 * CLASSES AND FUNCTIONS:


 * HISTORY:
 * 1999-02-04 (Thu): Created by Daniel K. Chung.
 * 2001-11-30 : spk change type for func parameters

 * COPYRIGHT (C) Accton Technology Corporation, 1999.
 * ---------------------------------------------------------------------- */
#ifndef CGI_FILE_H
#define CGI_FILE_H

/*---------------------------
 * INCLUDE FILE DECLARATIONS
 *---------------------------*/
#include "cgi_coretype.h"
#include "http_type.h"

#if __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------
 * cgi_file_lineend: Find end of line in file.
 * ---------------------------------------------------------------------- */

char * cgi_file_lineend (char *pLineBegin, char *pFileEnd);

/* ----------------------------------------------------------------------
 * cgi_file_token: Get token from file.
 * ---------------------------------------------------------------------- */

void cgi_file_token (char **pBegin, char **pEnd, char *pFileEnd);

/* ----------------------------------------------------------------------
 * cgi_file_sendtext: Send text file to socket.
 * ---------------------------------------------------------------------- */

void cgi_file_sendtext (HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, envcfg_t *envQuery, char *pData, CGI_SIZE_T nSize);

/* ----------------------------------------------------------------------
 * cgi_file_sendbin: Send binary file to socket.
 * ---------------------------------------------------------------------- */

void cgi_file_sendbin (HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, envcfg_t *envQuery, UI8_T *pData, CGI_SIZE_T nSize);


/* ----------------------------------------------------------------------
 * CGI_FILE_GetFileExt: Get file extension.
 * ---------------------------------------------------------------------- */

const char *
CGI_FILE_GetFileExt(const char *path);

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_CacheClear
 *------------------------------------------------------------------------------
 * Function : Clear all entries in cache
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *------------------------------------------------------------------------------
 */
void
CGI_FILE_CacheClear();

/* forward declaration for CGI_CACHE_INFO_T
 */
typedef struct CGI_CACHE_INFO_S CGI_CACHE_INFO_T;

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_GetCacheInfo
 *------------------------------------------------------------------------------
 * Function : Get file cache stat. information
 * Input    : None
 * Output   : info  - stat. information
 * Return   : CGI_OK; Error code
 * Note     : None
 *------------------------------------------------------------------------------
 */
int
CGI_FILE_GetCacheInfo(CGI_CACHE_INFO_T *info);

/*------------------------------------------------------------------------------
 * Routine Name : CGI_FILE_Main
 *------------------------------------------------------------------------------
 * Function : Read html(gif, xml, ...) file from flash disk
 * Input    : sock      -   Socket identifier
 *            auth      -   Authorisation level
 *            envcfg    -   Environment configuration
 *            envQuery  -   Query parameters.
 * Output   : None
 * Return   :  1: Have sent screen.
 *             0: Have not sent screen.
 *            -1: Error.
 * Note     : None
 *------------------------------------------------------------------------------
 */
int
CGI_FILE_Main(
    HTTP_Connection_T *http_connection,
    int sock,
    int auth,
    envcfg_t *envcfg,
    envcfg_t *envQuery
);

#if __cplusplus
}
#endif

#endif  /* CGI_FILE_H */

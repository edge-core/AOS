/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_real.h

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by project manager.

 * Module for Fileless CGI.
 * File to send "real" database values to socket.

 * CLASSES AND FUNCTIONS:


 * HISTORY:
 * 1999-02-04 (Thu): Created by Daniel K. Chung.

 * COPYRIGHT (C) Accton Technology Corporation, 1999.
 * ---------------------------------------------------------------------- */
#ifndef CGI_REAL_H
#define CGI_REAL_H

#if __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------
 * Constants.
 * ---------------------------------------------------------------------- */

typedef struct CGI_REAL_INFO_S
{
    unsigned int  indexQt;
    /*unsigned long*/ UI32_T indexArr [3];
    char *loopkeyP;
    unsigned int loopToken;  /* autwo 07/11/02  for use loopToken's location maybe be in 1,2,3 field not always in last filed.*/
}CGI_REAL_INFO_T;

typedef struct CGI_REAL_TABLE_S
{
    const char *szName;
    UI8_T mode;
    UI8_T indexQt;
    UI8_T indexType[3];
    UI8_T loopToken;
} CGI_REAL_TABLE_T;


enum CGI_REAL_INDEX_FROM_E
{
    FROM_NONE = 0,
    FROM_QUERY,
    FROM_FILE
};

/* ----------------------------------------------------------------------
 * cgi_real_send: Send "real" block to socket.
 * ---------------------------------------------------------------------- */
int cgi_real_send (HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, envcfg_t *envQuery, char *pRealBegin, char *pRealEnd,
    CGI_REAL_INFO_T *pRealVar);

/* ----------------------------------------------------------------------
 * cgi_real_sendloop: Send loop of "real" block to socket.
 * ---------------------------------------------------------------------- */
int cgi_real_sendloop (HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, envcfg_t *envQuery, char *pRealBegin, char *pRealEnd,
    CGI_REAL_INFO_T *pRealVar);

BOOL_T cgi_real_sendFuncNext (HTTP_Connection_T *http_connection, int sock, const char *szDisplay, const char *szFuncName);
BOOL_T cgi_real_sendValBuf (HTTP_Connection_T *http_connection, CGI_GETVAL_T *get_entry);

#if __cplusplus
}
#endif

#endif /* CGI_REAL_H */

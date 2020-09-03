/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_submt.h

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by project manager.

 * Module for Fileless CGI.
 * Header file for handling the CGI submit.

 * CLASSES AND FUNCTIONS:

 * Basic services:

 * HISTORY:
 * 1999-03-23 (Tue): Created by Zhong Qiyao.
 * 1999-04-26 (Mon): Added error code by Sumei Chung.

 * COPYRIGHT (C) Accton Technology Corporation, 1999.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * cgi_submit: Handle CGI submit.

 * Input:
 * sock: Socket identifier.
 * auth: Authorisation level: 1=guest, 2=administrator.
 * envcfg: Environment configuration.
 * envQuery: Query parameters.

 * Return:
 * 1: Have sent screen.
 * 0: Have not sent screen.
 * -1: Error.
 * ---------------------------------------------------------------------- */
#ifndef CGI_SUBMIT_H
#define CGI_SUBMIT_H

#if __cplusplus
extern "C" {
#endif

int cgi_submit (HTTP_Connection_T *http_connection, int sock, int auth, envcfg_t *envcfg, envcfg_t *envQuery);
int cgi_submit_ErrorStateByStr(HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, const char* errorStr, UI32_T *send);

#if __cplusplus
}
#endif

#endif  /* CGI_SUBMIT_H */

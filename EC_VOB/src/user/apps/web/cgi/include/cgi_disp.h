/* ----------------------------------------------------------------------
 * File name: cg_disp.h

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by project manager.

 * Module for Fileless CGI.
 * Header file for dispatch.

 * FUNCTION:
 * cgi_disp_fixed: Sends Web screen.

 * HISTORY:
 * 1999-03-23 (Tue): Created by Zhong Qiyao.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * cgi_disp_fixed: Dispatch Fixed Web File.

 * Input:
 * sock: Socket identifier.
 * auth: Authorisation level: 1=guest, 2=administrator.
 * envcfg: Environment configuration.
 * envQuery: Query parameters.

 * Return:
 * 0: Have not dispatched.
 * 1: Have dispatched.
 * ---------------------------------------------------------------------- */
#ifndef CGI_DISP_H
#define CGI_DISP_H

#if __cplusplus
extern "C" {
#endif

int cgi_disp_fixed (HTTP_Connection_T *http_connection, int sock, int auth, envcfg_t *envcfg, envcfg_t *envQuery);

#if __cplusplus
}
#endif

#endif  /* CGI_DISP_H */

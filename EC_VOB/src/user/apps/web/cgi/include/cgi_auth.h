/* ----------------------------------------------------------------------
 * FILE NAME: cg_auth.h

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K.

 * ABSTRACT:
 * This is part of the embedded program for ES3524.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Module for Fileless CGI.
 * This is the header file for authorisation check.

 * CLASSES AND FUNCTIONS:

 * cgi_check_auth: Check authorisation level.

 * USAGE:

 * HISTORY:
 * 1998-08-12 (Wed): Created by Daniel K. Chung, from "cgi.h".

 * COPYRIGHT (C) Accton Technology Corporation, 1998.
 * ---------------------------------------------------------------------- */

#ifndef CGI_AUTH_H
#define CGI_AUTH_H

#include "sys_adpt.h"
#include "cgi.h"
#include "userauth.h" // FIXME: [ref: cgi_auth] This is a patch solution. The function shall put in cgi_auth.c not be exported.

#if __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------
 * cgi_check_auth: Check authorisation level.

 * Same as when an attempt is being made to access a secured directory.
 * Get encrypted authorization from env variable and decode.
 * Then compare with registerded user/password.

 * Output:
 * 0: Not authorised;
 * 1: Guest;
 * 2: Administrator.
 * ---------------------------------------------------------------------- */
int cgi_check_auth (HTTP_Connection_T *http_connection, HTTP_Request_T *req, envcfg_t *envQuery);
// FIXME: [cgi_auth] This is a patch solution. The function shall put in cgi_auth.c not be exported.
int cgi_check_pass (CONNECTION_STATE_T connection_status,
                    const char * username,
                    const char * password,
                    L_INET_AddrIp_T *rem_ip_addr,
                    USERAUTH_AuthResult_T *auth_result_p
                    );


// TODO: -- start -- Check, does we still need these code ?
BOOL_T CGI_AUTH_CheckTempRadiusBase(const char *username, const char *encode_password, UI32_T *temp_privilege);
void CGI_AUTH_SetTempRadiusBaseEntry(const char *username, const char *encode_password, UI32_T temp_privilege);

void CGI_AUTH_InitTempRadiusUserBase();
void CGI_AUTH_InitSerialNumber();

typedef struct
{
    UI8_T       username[SYS_ADPT_MAX_USER_NAME_LEN + 1];
    UI8_T       password[SYS_ADPT_MAX_PASSWORD_LEN + 1];
    UI32_T      privilege;
    UI32_T      time;
} CGI_AUTH_LoginLocal_T;
// TODO: -- end --

#if __cplusplus
}
#endif

#endif  /* CGI_AUTH_H */


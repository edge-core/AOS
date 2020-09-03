/* ----------------------------------------------------------------------
 * FILE NAME: cg_res.h

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K.

 * ABSTRACT:
 * This is part of the embedded program for ES3524.
 * Entire firmware is integrated by the project manager.

 * Module for Fileless CGI.
 * File to get and set to resource API.

 * CLASSES AND FUNCTIONS:

 * USAGE:

#include <cg_res.h>

 * HISTORY:
 * 1998-12-29 (Tue): Created by Daniel K. Chung.
 * 1999-03-09 (Tue): One function deals with multiple indexes.

 * COPYRIGHT (C) Accton Technology Corporation, 1998.
 * ---------------------------------------------------------------------- */
#ifndef CGI_RES_H
#define CGI_RES_H

#if __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------
 * Types.
 * ---------------------------------------------------------------------- */
typedef UI32_T (* P_CORE_TEST_FUNC_T) (UI32_T, UI32_T *);

typedef struct tagCGI_ENUMTABLE_S {
    UI8_T               *szName;
    UI8_T               **ppData;
    int                 nQuant;
    P_CORE_TEST_FUNC_T  pTestFunc;
} CGI_ENUMTABLE_S;

typedef struct tagCGI_OPTION_S
{
    UI8_T value [256];
    UI8_T text [256];
} CGI_OPTION_T;

/* auwo add */
typedef struct RS_OID_S
{
    I32_T  	num_components;		/* original:int */
    UI32_T  *component_list;	/* original:WORD */
}
RS_OID_T;

typedef struct RS_OCTET_S
{
    UI8_T 	*octet_P;
    I32_T  	len;				/* original:int */
}
RS_OCTET_T;

typedef union RS_VAL_S
{
#ifdef DOWNGRADE_CPU
    UI32_T      dval;
#else
    UI64_T      dval;
#endif
    UI32_T      ival;			/* original:WORD */
    UI8_T       *str_P;
    RS_OCTET_T  octet;
    RS_OID_T    oid;
}
RS_VAL_T;

/*end autwo */

typedef BOOL_T (* P_CORE_FUNC_GET_VAR_T) (HTTP_Request_T *, HTTP_Response_T *, envcfg_t *);

/*
 * Typedef: struct CGI_VARIABLE_T
 *
 * Description: mapping token name to pointer of function in cgi_var.
 *
 * Fields:
 *  name         - Token name.
 *  func_p       - Pointer of function in cgi_var.
 *
 */
typedef struct
{
    const char            *name;
    P_CORE_FUNC_GET_VAR_T func_p;
} CGI_VARIABLE_T;

/* ----------------------------------------------------------------------
 * Functions.
 * ---------------------------------------------------------------------- */

int cgi_core_get_native (HTTP_Connection_T *http_connection, const char *szName, char *szValue, const char *szStrParam, const UI32_T *nIndexArr);
int cgi_core_get_nextvar (HTTP_Connection_T *http_connection, const char *szName, char *szValue, UI8_T *szOldIndex, const char *szStrParam, const UI32_T *nIndexArr, BOOL_T *bIsFirst);
int cgi_core_get_funcnext (HTTP_Connection_T *http_connection, int sock, const char *szName, char *szValue, const char *szFuncName, const char *szStrParam, const UI32_T *nIndexArr);
int cgi_core_get_variable (HTTP_Connection_T *http_connection, int sock, const char *szName, char *szValue, const char *szStrParam, const UI32_T *nIndexArr);

int CGI_RES_GetVariable(HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, char **in_pp, char *line_end_p, char *display_p, UI32_T len);

#if __cplusplus
}
#endif

#endif /* CGI_RES_H */


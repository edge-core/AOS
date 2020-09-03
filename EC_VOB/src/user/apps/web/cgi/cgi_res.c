/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_res.c

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by project manager.

 * Module for Fileless CGI.
 * File to get and set to resource API.

 * CLASSES AND FUNCTIONS:

 * cgi_core_get_mib: Get string form of MIB variable.

 * HISTORY:
 * 1998-12-29 (Tue): Created by Daniel K. Chung.
 * 1999-01-06 (Wed): Can read/write with index.
 * 1999-03-09 (Tue): One function deals with multiple indexes.
 * 1999-04-26 (Mon): Porting to new platform/api by Sumei Chung.

 * COPYRIGHT (C) Accton Technology Corporation, 1998.
 * ---------------------------------------------------------------------- */

#include "cgi.h"
#include "cgi_res.h"
#include "cgi_lib.h"
#include "cgi_var.h"
#include "sys_mgr.h"
#include "leaf_vbridge.h"
#include "xstp_mgr.h"
#include "leaf_ieee8023lag.h"
#include "leaf_2819.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "fs_type.h"

#include <libxml/parser.h>

#if (SYS_CPNT_DHCP == TRUE)
#include "dhcp_mgr.h"
#endif /* #if (SYS_CPNT_DHCP == TRUE) */

#if (SYS_CPNT_ACL == TRUE)
#include "rule_type.h"
#endif /* #if (SYS_CPNT_ACL == TRUE) */

#if (SYS_CPNT_COS == TRUE)
#include "cos_type.h"
#include "cos_vm.h"
#endif /* #if (SYS_CPNT_COS == TRUE) */

#if (SYS_CPNT_NTP == TRUE)
#include "ntp_mgr.h"
#endif /* #if (SYS_CPNT_NTP == TRUE) */

#if (SYS_CPNT_SNMP == TRUE)
#if (SYS_CPNT_SNMP_VERSION == 3)
#include "snmp_mgr.h"
#include "snmp_pmgr.h"
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */
#endif /* #if (SYS_CPNT_SNMP == TRUE) */

#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_type.h"
#include "leaf_ieeelldp.h"
#endif /* #if (SYS_CPNT_LLDP == TRUE) */

#if (SYS_CPNT_DHCP == TRUE)
#include "dhcp_type.h"
#endif /* #if (SYS_CPNT_DHCP == TRUE) */

#if (SYS_CPNT_ROUTING == TRUE)
#if (SYS_CPNT_RIP == TRUE)
#include "netcfg_type.h"
#endif /* #if (SYS_CPNT_RIP == TRUE) */

#if (SYS_CPNT_OSPF == TRUE)
#include "leaf_1850.h"
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

#if (SYS_CPNT_VRRP == TRUE)
#include "leaf_2787.h"
#endif /* #if (SYS_CPNT_VRRP == TRUE) */

#if (SYS_CPNT_DOT1X == TRUE)
#include "1x_mgr.h"
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if (SYS_CPNT_CFM == TRUE)
#include "cfm_type.h"
#endif  /* #if (SYS_CPNT_CFM == TRUE) */

#if (SYS_CPNT_PPPOE_IA == TRUE)
#include "pppoe_ia_pmgr.h"
#include "pppoe_ia_type.h"
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
#include "nmtr_type.h"
#endif /* #if (SYS_CPNT_NMTR_HISTORY == TRUE) */

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
#include "swctrl.h"
#endif


/* ----------------------------------------------------------------------
 * Types.
 * ---------------------------------------------------------------------- */

typedef enum CGI_TYPE_E
{
    CGI_TYP_STR,
    CGI_TYP_CONST,
    CGI_TYP_CONST_MINUS
} CGI_TYP_T;

typedef BOOL_T (* P_CORE_FUNC_T) ();
typedef BOOL_T (* P_CORE_FUNC_GET_STR_T)    (HTTP_Connection_T *, char *, const char *, const UI32_T *);
typedef BOOL_T (* P_CORE_FUNC_NEXT_T)       (HTTP_Connection_T *, char *, char *, const char *, const UI32_T *, BOOL_T *);
typedef BOOL_T (* P_CORE_FUNC_FUNCNEXT_T)   (HTTP_Connection_T *, int, char *, const char *, const char *, const UI32_T *);
typedef BOOL_T (* P_CORE_FUNC_GETVAL_T)     (HTTP_Connection_T *, CGI_GETVAL_T *);

typedef struct tagCGI_TABLE_S
{
    const char *szName;
    int nType;
    P_CORE_FUNC_T pFunc;
} CGI_TABLE_S;


/* ----------------------------------------------------------------------
 * Global variables (must be kept contant).
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * In-file constants: Tables for all resource API lookup.
 * ---------------------------------------------------------------------- */

/* variables */
/* *** MUST BE IN ASCENDING ALPHABETICAL ORDER *** */

CGI_TABLE_S cgi_core_arGetTable [] =
{
    {"", CGI_TYP_STR, NULL}
};

/* next variables */
/* *** MUST BE IN ASCENDING ALPHABETICAL ORDER *** */

CGI_TABLE_S cgi_core_arGetNextTable [] =
{
    {"", CGI_TYP_STR, NULL}
};

/* next variables */
/* *** MUST BE IN ASCENDING ALPHABETICAL ORDER *** */

CGI_TABLE_S cgi_core_arGetFuncNextTable [] =
{
    {"", CGI_TYP_STR, NULL}
};


/* SSI variables */
CGI_VARIABLE_T cgi_get_var_table[] =
{
    {"", NULL},
};

/* ----------------------------------------------------------------------
 * Forward declarations.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * cgi_core_compar: Global comparison function for binary search.
 * ---------------------------------------------------------------------- */

int cgi_core_compar (const void *key,const void *member)
{
    CGI_TABLE_S *pTableMember = (CGI_TABLE_S *) member;

    return strcmp ((char *) key, pTableMember->szName);
}

/* ----------------------------------------------------------------------
 * cgi_core_get_native: Get value for specified variable.
 * ---------------------------------------------------------------------- */

int cgi_core_get_native (HTTP_Connection_T *http_connection, /* IN */ const char *szName, /* OUT */ char *szValue, /* IN */ const char *szStrParam, /* IN */ const UI32_T *nIndexArr)
{
    UI32_T  nLen = 0;
    UI32_T  nValue = 0;
    int     minus_value = 0;
    int     nResult = 0;
    CGI_TABLE_S         *pTable, *pFound;
    CGI_SIZE_T          nTableSize;

    /* table pointer and size */
    pTable = & cgi_core_arGetTable [0];
    nTableSize = sizeof (cgi_core_arGetTable);

    /* use binary search */
    if ((pFound = (CGI_TABLE_S *) bsearch (szName, pTable,
        nTableSize / sizeof (CGI_TABLE_S),
        sizeof (CGI_TABLE_S),
        &cgi_core_compar)) == NULL)
    {
        nResult = CGI_CORE_NOTFOUND;
        goto Exit;
    }

    /* check function available */
    if ((pFound->pFunc == NULL) &&
        (pFound->nType != CGI_TYP_CONST) &&
        (pFound->nType != CGI_TYP_CONST_MINUS))
    {
        nResult = -2;
        goto Exit;
    }

    switch (pFound->nType)
    {
        case CGI_TYP_CONST:
            nValue = L_CVRT_PTR_TO_UINT(pFound->pFunc);
            sprintf(szValue, "%u", nValue);
            break;
        case CGI_TYP_CONST_MINUS:
            minus_value = L_CVRT_PTR_TO_INT(pFound->pFunc);
            sprintf(szValue, "%d", minus_value);
            break;
        case CGI_TYP_STR:
            nResult = (* (P_CORE_FUNC_GET_STR_T) pFound->pFunc)
                    (http_connection, szValue, szStrParam, nIndexArr) ? 0 : -1;

            /* check result */
            if (nResult != CORE_NO_ERROR)
            {
                goto Exit;
            }

            /* check length */
            nLen = strlen(szValue);

            if (nLen > CGI_MAX_VARIABLE_STR)
            {
                nLen = CGI_MAX_VARIABLE_STR;
            }

            szValue[nLen] = '\0';
            break;
        default:
            nResult = CGI_CORE_NOTFOUND;
            goto Exit;
    }

Exit:
    /* return */
    return nResult;
}

/* ----------------------------------------------------------------------
 * cgi_core_get_variable: Get value for specified variable.
 * ---------------------------------------------------------------------- */

int cgi_core_get_variable (HTTP_Connection_T *http_connection, int sock, const char *szName, char *szValue, const char *szStrParam, const UI32_T *nIndexArr)
{
    UI32_T  nLen = 0;
    UI32_T  nValue = 0;
    int     minus_value = 0;
    int     nResult = 0;
    CGI_TABLE_S   *pTable, *pFound;
    CGI_SIZE_T     nTableSize;
    CGI_GETVAL_T   get_entry;
    memset(&get_entry, 0, sizeof(CGI_GETVAL_T));

    /* table pointer and size */
    pTable = & cgi_core_arGetTable [0];
    nTableSize = sizeof (cgi_core_arGetTable);

    /* use binary search */
    if ((pFound = (CGI_TABLE_S *) bsearch (szName, pTable,
        nTableSize / sizeof (CGI_TABLE_S),
        sizeof (CGI_TABLE_S),
        &cgi_core_compar)) == NULL)
    {
        nResult = CGI_CORE_NOTFOUND;
        goto Exit;
    }

    /* check function available */
    if ((pFound->pFunc == NULL) &&
        (pFound->nType != CGI_TYP_CONST) &&
        (pFound->nType != CGI_TYP_CONST_MINUS))
    {
        nResult = -2;
        goto Exit;
    }

    switch (pFound->nType)
    {
        case CGI_TYP_CONST:
            nValue = L_CVRT_PTR_TO_UINT(pFound->pFunc);
            sprintf(szValue, "%u", nValue);
            break;
        case CGI_TYP_CONST_MINUS:
            minus_value = L_CVRT_PTR_TO_INT(pFound->pFunc);
            sprintf(szValue, "%d", minus_value);
            break;
        case CGI_TYP_STR:
            get_entry.sock = sock;
            get_entry.index_arr = nIndexArr;
            get_entry.str_buf = szValue;
            get_entry.str_param = szStrParam;
            nResult = (* (P_CORE_FUNC_GETVAL_T) pFound->pFunc)
                    (http_connection, &get_entry) ? 0 : -1;

            /* check result */
            if (nResult != CORE_NO_ERROR)
            {
                goto Exit;
            }

            /* check length */
            nLen = strlen(szValue);

            if (nLen > CGI_MAX_VARIABLE_STR)
            {
                nLen = CGI_MAX_VARIABLE_STR;
            }

            szValue[nLen] = '\0';
            break;
        default:
            nResult = CGI_CORE_NOTFOUND;
            goto Exit;
    }

Exit:
    /* return */
    return nResult;
}

/* ----------------------------------------------------------------------
 * cgi_core_get_nextvar: Get next value for specified variable.
 * ---------------------------------------------------------------------- */

int cgi_core_get_nextvar (HTTP_Connection_T *http_connection, const char *szName, char *szValue, UI8_T *szOldIndex, const char *szStrParam, const UI32_T *nIndexArr, BOOL_T *bIsFirst)
{
    UI32_T  nOldIndexLen = 0, nLen = 0;
    int     nResult = 0;
    CGI_TABLE_S         *pTable, *pFound;
    CGI_SIZE_T          nTableSize;

    /* table pointer and size */
    pTable = & cgi_core_arGetNextTable [0];
    nTableSize = sizeof (cgi_core_arGetNextTable);

    /* use binary search */
    if ((pFound = (CGI_TABLE_S *) bsearch (szName, pTable,
        nTableSize / sizeof (CGI_TABLE_S),
        sizeof (CGI_TABLE_S),
        &cgi_core_compar)) == NULL)
    {
        nResult = CGI_CORE_NOTFOUND;
        goto Exit;
    }

    /* check function available */
    if (pFound->pFunc == NULL)
    {
        nResult = -2;
        goto Exit;
    }

    switch (pFound->nType)
    {
        case CGI_TYP_STR:
            nResult = (* (P_CORE_FUNC_NEXT_T) pFound->pFunc)
                      (http_connection, szValue, szOldIndex, szStrParam, nIndexArr, bIsFirst) ? 0 : -1;

            /* check result */
            if (nResult != CORE_NO_ERROR)
            {
                goto Exit;
            }

            /* check length */
            nOldIndexLen = strlen(szOldIndex);
            nLen = strlen(szValue);

            if (nOldIndexLen > CGI_MAX_FUNCNEXT_INDEX)
            {
                nOldIndexLen = CGI_MAX_FUNCNEXT_INDEX;
            }

            if (nLen > CGI_MAX_VARIABLE_STR)
            {
                nLen = CGI_MAX_VARIABLE_STR;
            }

            szOldIndex[nOldIndexLen] = '\0';
            szValue[nLen] = '\0';
            break;
        default:
            nResult = CGI_CORE_NOTFOUND;
            goto Exit;
    }

Exit:
    /* return */
    return nResult;
}

int cgi_core_get_funcnext (HTTP_Connection_T *http_connection, int sock, const char *szName, char *szValue, const char *szFuncName, const char *szStrParam, const UI32_T *nIndexArr)
{
    int nResult = 0;
    CGI_TABLE_S   *pTable, *pFound;
    CGI_SIZE_T    nTableSize;

    /* table pointer and size */
    pTable = & cgi_core_arGetFuncNextTable [0];
    nTableSize = sizeof (cgi_core_arGetFuncNextTable);

    /* use binary search */
    if ((pFound = (CGI_TABLE_S *) bsearch (szName, pTable,
        nTableSize / sizeof (CGI_TABLE_S),
        sizeof (CGI_TABLE_S),
        &cgi_core_compar)) == NULL)
    {
        nResult = CGI_CORE_NOTFOUND;
        goto Exit;
    }

    /* check function available */
    if (pFound->pFunc == NULL)
    {
        nResult = -2;
        goto Exit;
    }

    switch (pFound->nType)
    {
        case CGI_TYP_STR:
            nResult = (* (P_CORE_FUNC_FUNCNEXT_T) pFound->pFunc)
                      (http_connection, sock, szValue, szFuncName, szStrParam, nIndexArr) ? 0 : -1;

            /* check result */
            if (nResult != CORE_NO_ERROR)
            {
                goto Exit;
            }
            break;
        default:
            nResult = CGI_CORE_NOTFOUND;
            goto Exit;
    }

Exit:
    /* return */
    return nResult;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_RES_CompareTable
 *------------------------------------------------------------------------------
 * PURPOSE:  Compare key with name in table
 * INPUT:    key        -- search key
 *           member     -- table member
 * OUTPUT:   None.
 * RETURN:   0          -- match
 *           -1         -- not match
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
static int CGI_RES_CompareTable(const void *key, const void *member)
{
    char            *key_p = (char *) key;
    CGI_VARIABLE_T  *table_member_p = (CGI_VARIABLE_T *) member;

    return strcmp(key_p, table_member_p->name);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_RES_SortTable
 *------------------------------------------------------------------------------
 * PURPOSE:  Compare key with name in table
 * INPUT:    member1_p  -- pointer of member 1
 *           member2_p  -- pointer of member 2
 * OUTPUT:   None.
 * RETURN:   strcmp(table_member1_p->name, table_member2_p->name)
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
static int CGI_RES_SortTable(const void *member1_p, const void *member2_p)
{
    CGI_VARIABLE_T  *table_member1_p = (CGI_VARIABLE_T *) member1_p;
    CGI_VARIABLE_T  *table_member2_p = (CGI_VARIABLE_T *) member2_p;

    return strcmp(table_member1_p->name, table_member2_p->name);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_RES_GetVariableFunction
 *------------------------------------------------------------------------------
 * PURPOSE:  Search mapped function in table by key.
 * INPUT:    envcfg            -- Environmental configuration.
 * OUTPUT:   None.
 * RETURN:   NULL              -- Not found.
 *           found_p->func_p   -- Success.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
static P_CORE_FUNC_GET_VAR_T CGI_RES_GetVariableFunction(envcfg_t *envcfg)
{
    CGI_VARIABLE_T *table_p, *found_p;
    CGI_SIZE_T     table_size;
    const char *key;

    ASSERT(envcfg != NULL);

    /* get variable from env
     */
    key = get_env(envcfg, "tag.variable");

    if (key == NULL)
    {
        return NULL;
    }

    /* table pointer and size
     */
    table_p = &cgi_get_var_table[0];
    table_size = sizeof(cgi_get_var_table);

    //
    // FIXME: Don't do this everytime
    //
    /* sort table
     */
    qsort(cgi_get_var_table,
          table_size / sizeof(CGI_VARIABLE_T),
          sizeof(CGI_VARIABLE_T),
          &CGI_RES_SortTable);

    /* use binary search
     */
    if ((found_p = (CGI_VARIABLE_T *) bsearch (key, table_p,
        table_size / sizeof(CGI_VARIABLE_T),
        sizeof(CGI_VARIABLE_T),
        &CGI_RES_CompareTable)) == NULL)
    {
        return NULL;
    }
    else
    {
        return found_p->func_p;
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_RES_GetVariable
 *------------------------------------------------------------------------------
 * PURPOSE:  Use function mapped token to get variable and output in JSON from.
 * INPUT:    sock              -- Sock.
 *           envcfg            -- Input parameters from cgi_real.
 *           in_pp             -- Pointer of pointer of token begin.
 *           line_end_p        -- Pointer of line end.
 *           len               -- Max length of display.
 * OUTPUT:   display           -- String for output.
 * RETURN:   CORE_NO_ERROR     -- Success.
 *           CORE_SSI_FAIL     -- Failed.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
int CGI_RES_GetVariable(HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, char **in_pp, char *line_end_p, char *display_p, UI32_T len)
{
    UI32_T ret = CORE_NO_ERROR;
    TOKEN_T key;
    P_CORE_FUNC_GET_VAR_T fn = NULL;

    CGI_RESPONSE_USER_CONTEXT_PTR_T user_ctx;

    memset(&key, 0, sizeof(TOKEN_T));

    ASSERT(envcfg != NULL);
    ASSERT(len > 0);
    ASSERT(in_pp != NULL);
    ASSERT(line_end_p != NULL);
    ASSERT(http_connection->res->user_ctx != NULL);

    user_ctx = (CGI_RESPONSE_USER_CONTEXT_PTR_T) http_connection->res->user_ctx;

    /* #SSI <variable>#
     *  ^ token_begin
     *                ^ token_end
     *
     * put token_begin to env variables as
     * tag.name = SSI
     * tag.variable = <variable>
     */
    if (CGI_QUERY_ParseVariableParameter(envcfg, (char **) in_pp, (char *)line_end_p) != TRUE)
    {
        return CORE_SSI_FAIL;
    }

    if ((fn = CGI_RES_GetVariableFunction(envcfg)) == NULL)
    {
        CGI_QUERY_ClearVariableParameter(envcfg);
        return CORE_SSI_FAIL;
    }

    user_ctx->root = xmlNewNode(NULL, BAD_CAST "object");

    //
    // FIXME: Remove this function
    //
    key.p = (char *) "out";
    key.len = sizeof("out");
    set_env_ptr(envcfg, &key, user_ctx->root);

    //
    // init jss variables
    //
    user_ctx->is_send_create_object = 0;
    user_ctx->is_send_end_string = 0;

    fn(http_connection->req, http_connection->res, envcfg);

    if (!user_ctx->is_jss)
    {
        /* turn data to JSON form
         */
        ret = CGI_LIB_JSON_Stringify(user_ctx->root, cgi_SendText, http_connection, http_connection->res->fd);
    }
    else
    {
        //
        // FIXME: This is a patch code,
        // when use js_end() to send jss ending codes, shall also set is_jss to 0.
        // In the feature, those 2 actions shall be put in one function.
        //
        user_ctx->js_end(http_connection->res);
        user_ctx->is_jss = 0;
    }

    /* Clear xmlNodePtr in env
     */
    xmlFreeNode(user_ctx->root);
    user_ctx->root = NULL;

    unset_env(envcfg, "out");

    /* Clear all tag. prefix env variable
     */
    CGI_QUERY_ClearVariableParameter(envcfg);

    return ret;
}

/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_real.c

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by project manager.

 * Module for Fileless CGI.
 * File to send "real" database values to socket.

 * CLASSES AND FUNCTIONS:


 * HISTORY:
 * 1999-03-08 (Mon): Created by Daniel K. Chung.

 * COPYRIGHT (C) Accton Technology Corporation, 1999.
 * ---------------------------------------------------------------------- */
#include <string.h>
#include "cgi.h"
#include "cgi_util.h"
#include "cgi_cache.h"
#include "cgi_file.h"
#include "cgi_real.h"
#include "cgi_res.h"
#include "leaf_2674p.h"
#include "trk_pmgr.h"
#include "l_mm.h"
#include "mm.h"

/* ----------------------------------------------------------------------
 * Forward declarations.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * Constants.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * Global variables (must be kept contant).
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * cgi_real_sendval: Send "real" value of "#...#".

 * Input:
 * pBuf: a buffer, which it can use
 * ---------------------------------------------------------------------- */

int cgi_real_sendval(HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, envcfg_t *envQuery, char **ppIn, char *pLineEnd, int nIndexQt, UI32_T *nIndexArr)
{
    char  token[CGI_MAX_TOKEN + 1] = {0};
    char  string_parameter[CGI_MAX_STRPARAM + 1] = {0};
    char  function_name[CGI_MAX_FUNCNEXT_FUNCNAME + 1] = {0};
    char  *token_begin_p, *token_end_p;
    char  *value_p;
    const char  *none_p = "(none)";
    BOOL_T  is_none_b = TRUE;
    BOOL_T  is_first_b = TRUE;
    BOOL_T  is_need_enclose = FALSE;
    int  ret = 0, rc;
    char  *cgi_real_strBuf = NULL;
    char  *cgi_real_escBuf = NULL;
    char  *cgi_real_disBuf = NULL;
    UI8_T *old_index=NULL;

    if ((cgi_real_strBuf = (char *) L_MM_Malloc(CGI_MAX_VARIABLE_STR + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_PARSE_REQUEST))) == NULL)
    {
        ret = -1;
        goto fin;
    }
    MM_alloc(cgi_real_strBuf, CGI_MAX_VARIABLE_STR + 1, SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    if ((cgi_real_escBuf = (char *) L_MM_Malloc(CGI_MAX_ESCAPE_STR + 2 /* two enclosed single quotes */ + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_PARSE_REQUEST))) == NULL)
    {
        ret = -1;
        goto fin;
    }
    MM_alloc(cgi_real_escBuf, CGI_MAX_ESCAPE_STR + 2 /* two enclosed single quotes */ + 1, SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    if ((cgi_real_disBuf = (char *) L_MM_Malloc(CGI_MAX_DISPLAY_STR + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_PARSE_REQUEST))) == NULL)
    {
        ret = -1;
        goto fin;
    }
    MM_alloc(cgi_real_disBuf, CGI_MAX_DISPLAY_STR + 1, SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    if ((old_index = (UI8_T *) L_MM_Malloc(CGI_MAX_FUNCNEXT_INDEX + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_PARSE_REQUEST))) == NULL)
    {
        ret = -1;
        goto fin;
    }
    MM_alloc(old_index, CGI_MAX_FUNCNEXT_INDEX + 1, SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    /* clear buffer
     */
    memset(cgi_real_strBuf, 0, CGI_MAX_VARIABLE_STR + 1);
    memset(cgi_real_escBuf, 0, CGI_MAX_ESCAPE_STR + 1);
    memset(cgi_real_disBuf, 0, CGI_MAX_DISPLAY_STR + 1);
    memset(old_index, 0, CGI_MAX_FUNCNEXT_INDEX + 1);

    /* SSI
     */
    rc = CGI_RES_GetVariable(http_connection, sock, envcfg, (char **) ppIn, (char *) pLineEnd, (char *) cgi_real_escBuf, CGI_MAX_ESCAPE_STR);

    if (rc == CORE_NO_ERROR)
    {
        goto fin;
    }

    /* get STRPARAM
     */
    if ((value_p = get_env(envQuery,"STRPARAM")) != NULL)
    {
        strncpy(string_parameter, value_p, CGI_MAX_STRPARAM +1);
    }

    /* extract first token
     */
    token_begin_p = (char *) *ppIn;
    cgi_file_token( &token_begin_p, &token_end_p, pLineEnd);

    /* index
     */
    if ((token_end_p - token_begin_p == 5) &&
        (strncmp(token_begin_p, "INDEX", 5) == 0))
    {
        SYSFUN_Snprintf((char *) cgi_real_strBuf, CGI_MAX_VARIABLE_STR +1, "%u", nIndexArr[0]);
        cgi_real_strBuf[CGI_MAX_VARIABLE_STR] = '\0';
    }

    /* 2nd index
     */
    else if ((token_end_p - token_begin_p == 6) &&
        (strncmp(token_begin_p, "INDEX2", 6) == 0))
    {
        SYSFUN_Snprintf((char *) cgi_real_strBuf, CGI_MAX_VARIABLE_STR +1, "%u", nIndexArr[1]);
        cgi_real_strBuf[CGI_MAX_VARIABLE_STR] = '\0';
    }

    /* 3nd index
     */
    else if ((token_end_p - token_begin_p == 6) &&
        (strncmp(token_begin_p, "INDEX3", 6) == 0))
    {
        SYSFUN_Snprintf((char *) cgi_real_strBuf, CGI_MAX_VARIABLE_STR +1, "%u", nIndexArr[2]);
        cgi_real_strBuf[CGI_MAX_VARIABLE_STR] = '\0';
    }

    /* query variable
     */
    else if ((token_end_p - token_begin_p == 5) &&
        (strncmp(token_begin_p, "QUERY", 5) == 0))
    {
        /* find next token
         */
        token_begin_p = token_end_p;
        cgi_file_token(&token_begin_p, &token_end_p, pLineEnd);

        /* copy token
         */
        strncpy(token, token_begin_p, token_end_p - token_begin_p);
        token[token_end_p - token_begin_p] = '\0';

        /* lookup
         */
        if ((value_p = cgi_query_lookup(envQuery, token)) != NULL)
        {
            strncpy((char *) cgi_real_strBuf, value_p, CGI_MAX_VARIABLE_STR +1);
            is_need_enclose = TRUE;
        }
        else
        {
            strncpy((char *) cgi_real_strBuf, "null", CGI_MAX_VARIABLE_STR +1);
        }

        cgi_real_strBuf[CGI_MAX_VARIABLE_STR] = '\0';
    }

    /* function get next
     */
    else if ((token_end_p - token_begin_p == 8) &&
        (strncmp(token_begin_p, "FUNCNEXT", 8) == 0))
    {
        /* find next token
         */
        token_begin_p = token_end_p;
        cgi_file_token(&token_begin_p, &token_end_p, pLineEnd);

        /* copy token
         */
        strncpy(token, token_begin_p, token_end_p - token_begin_p);
        token[token_end_p - token_begin_p] = '\0';

        /* find next token (function name)
         */
        token_begin_p = token_end_p;
        cgi_file_token(&token_begin_p, &token_end_p, pLineEnd);

        /* copy token
         */
        strncpy(function_name, token_begin_p, token_end_p - token_begin_p);
        function_name[token_end_p - token_begin_p] = '\0';

        /* get
         */
        while (cgi_core_get_nextvar(http_connection, token, cgi_real_strBuf, old_index, string_parameter, nIndexArr, &is_first_b) == 0)
        {
            is_none_b = FALSE;

            /* send HTML text
             */
            cgi_stresc(cgi_real_escBuf, cgi_real_strBuf);
            SYSFUN_Snprintf(cgi_real_disBuf, CGI_MAX_DISPLAY_STR +1, "%s('%s');\n", function_name, cgi_real_escBuf);
            cgi_real_disBuf[CGI_MAX_DISPLAY_STR] = '\0';

            if (cgi_SendText(http_connection, sock, (I8_T *) cgi_real_disBuf) != 0)
            {
                ret = -1;
                goto fin;
            }
        }

        if (is_none_b == TRUE)
        {
            /* send HTML text
             */
            cgi_stresc(cgi_real_escBuf, none_p);
            SYSFUN_Snprintf((char *) cgi_real_disBuf, CGI_MAX_DISPLAY_STR +1, "%s('%s');\n", function_name, cgi_real_escBuf);
            cgi_real_disBuf[CGI_MAX_DISPLAY_STR] = '\0';

            if (cgi_SendText(http_connection, sock, (I8_T *) cgi_real_disBuf) != 0)
            {
                ret = -1;
                goto fin;
            }
        }

        goto fin;
    }

    /* function get next
     */
    else if ((token_end_p - token_begin_p == 11) &&
        (strncmp(token_begin_p, "FUNCGETNEXT", 11) == 0))
    {
        /* find next token
         */
        token_begin_p = token_end_p;
        cgi_file_token(&token_begin_p, &token_end_p, pLineEnd);

        /* copy token
         */
        strncpy(token, token_begin_p, token_end_p - token_begin_p);
        token[token_end_p - token_begin_p] = '\0';

        /* find next token (function name)
         */
        token_begin_p = token_end_p;
        cgi_file_token(&token_begin_p, &token_end_p, pLineEnd);

        /* copy token
         */
        strncpy(function_name, token_begin_p, token_end_p - token_begin_p);
        function_name[token_end_p - token_begin_p] = '\0';

        /* get
         */
        if (cgi_core_get_funcnext(http_connection, sock, token, cgi_real_strBuf, function_name, string_parameter, nIndexArr) == CGI_CORE_NOTFOUND)
        {
            /* send HTML text
             */
            cgi_stresc(cgi_real_escBuf, none_p);
            SYSFUN_Snprintf((char *) cgi_real_disBuf, CGI_MAX_DISPLAY_STR +1, "%s('%s');\n", function_name, cgi_real_escBuf);
            cgi_real_disBuf[CGI_MAX_DISPLAY_STR] = '\0';

            if (cgi_SendText(http_connection, sock, (I8_T *) cgi_real_disBuf) != 0)
            {
                ret = -1;
                goto fin;
            }
        }

        goto fin;
    }

    /* get variable
     */
    else if ((token_end_p - token_begin_p == 6) &&
        (strncmp(token_begin_p, "GETVAL", 6) == 0))
    {
        /* find next token
         */
        token_begin_p = token_end_p;
        cgi_file_token(&token_begin_p, &token_end_p, pLineEnd);

        /* copy token
         */
        strncpy(token, token_begin_p, token_end_p - token_begin_p);
        token[token_end_p - token_begin_p] = '\0';

        /* get
         */
        if (cgi_core_get_variable(http_connection, sock, token, cgi_real_strBuf, string_parameter, nIndexArr) == CGI_CORE_NOTFOUND)
        {
            strcpy((char *) cgi_real_disBuf, "-1");

            if (cgi_SendText(http_connection, sock, (I8_T *) cgi_real_disBuf) != 0)
            {
                ret = -1;
                goto fin;
            }
        }

        goto fin;
    }

    /* variable
     */
    else
    {
        /* copy token
         */
        strncpy(token, token_begin_p, token_end_p - token_begin_p);
        token[token_end_p - token_begin_p] = '\0';

        /* get
         */
        if (cgi_core_get_native(http_connection, token, cgi_real_strBuf, string_parameter, nIndexArr) == CGI_CORE_NOTFOUND)
        {
            strcpy((char *) cgi_real_disBuf, "-1");

            if (cgi_SendText(http_connection, sock, (I8_T *) cgi_real_disBuf) != 0)
            {
                ret = -1;
            }

            goto fin;
        }
    }

    /* update input pointer
     */
    *ppIn = token_end_p;

    /* convert to HTML escape form
     */
    if (FALSE == is_need_enclose)
    {
        cgi_stresc (cgi_real_escBuf, cgi_real_strBuf);
    }
    else
    {
        cgi_real_escBuf[0] = '\'';
        cgi_stresc (cgi_real_escBuf + 1, cgi_real_strBuf);
        strcat(cgi_real_escBuf, "\'");
    }

    /* send variable
     */
    if (cgi_SendText(http_connection, sock, (I8_T *) cgi_real_escBuf) != 0)
    {
        ret = -1;
        goto fin;
    }

fin:
    if (cgi_real_strBuf != NULL)
    {
        MM_free(cgi_real_strBuf, SYSFUN_TaskIdSelf());
        L_MM_Free(cgi_real_strBuf);
        cgi_real_strBuf = NULL;
    }

    if (cgi_real_escBuf != NULL)
    {
        MM_free(cgi_real_escBuf, SYSFUN_TaskIdSelf());
        L_MM_Free(cgi_real_escBuf);
        cgi_real_escBuf = NULL;
    }

    if (cgi_real_disBuf != NULL)
    {
        MM_free(cgi_real_disBuf, SYSFUN_TaskIdSelf());
        L_MM_Free(cgi_real_disBuf);
        cgi_real_disBuf = NULL;
    }

    if (old_index != NULL)
    {
        MM_free(old_index, SYSFUN_TaskIdSelf());
        L_MM_Free(old_index);
        old_index = NULL;
    }

    return (ret);
}

/* ----------------------------------------------------------------------
 * cgi_real_sendline: Send "real" line to socket.
 * ---------------------------------------------------------------------- */

int cgi_real_sendline (HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, envcfg_t *envQuery, char *pLineBegin, char *pLineEnd,
    int nIndexQt, UI32_T*nIndexArr)
{
    int xOut;
    char *pIn;
    char *pBuf;
    int nRet = 0;

    /* allocate buffer */
    if ((pBuf = (char *) L_MM_Malloc(CGI_MAX_VARIABLE_STR, L_MM_USER_ID2(SYS_MODULE_CGI, CGI_TYPE_TRACE_ID_CGI_SUBMIT_ALLOCATE_TMPBUF))) == NULL)
    {
        return -1;
    }

    /* read character by charecter */
    pIn = pLineBegin;
    xOut = 0;

    while (pIn < pLineEnd)
    {
        /* special case: # */
        if (*pIn == '#')
        {
            pIn++;

            /* if another #, treat at single # */
            if (*pIn == '#')
            {
                pBuf [xOut] = '#';
                xOut++;
                goto NextChar;
            }

            /* dump current output buffer */
            pBuf [xOut] = '\0';

            if (cgi_SendText(http_connection, sock, pBuf) != 0)
            {
                nRet = -1;
                goto Exit;
            }

            /* get real value of "#...#" */
            if (cgi_real_sendval (http_connection, sock, envcfg, envQuery, &pIn, pLineEnd,
                nIndexQt, nIndexArr) != 0)
            {
                nRet = -1;
                goto Exit;
            }

            /* move pIn to next '#' */
            while ((pIn < pLineEnd) && (*pIn != '#'))
            {
                pIn++;
            }

            /* reset output buffer */
            xOut = 0;

            goto NextChar;
        }

        /* normal charecter */
        else
        {
            pBuf [xOut] = *pIn;
            xOut++;
        }

        /* next */
NextChar:
        pIn++;
    }

    /* end of line */
    pBuf [xOut] = '\n';
    pBuf [xOut + 1] = '\0';
    if (cgi_SendText(http_connection, sock, pBuf) != 0)
    {
        nRet = -1;
        goto Exit;
    }

Exit:
    /* free buffer */
    L_MM_Free (pBuf);
    return (nRet);
}

/* ----------------------------------------------------------------------
 * cgi_real_send: Send "real" block to socket.
 * ---------------------------------------------------------------------- */

int cgi_real_send (HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, envcfg_t *envQuery, char *pRealBegin, char *pRealEnd,
    CGI_REAL_INFO_T *pRealVar)
{
    char *pLineBegin = pRealBegin;
    char *pLineEnd;
    int nRet;

    /* for each line */
    while (pLineBegin < pRealEnd)
    {
        /* find end of line */
        pLineEnd = cgi_file_lineend (pLineBegin, pRealEnd);

        if ((nRet = cgi_real_sendline (http_connection, sock, envcfg, envQuery, pLineBegin, pLineEnd,
            pRealVar->indexQt, pRealVar->indexArr)) != 0)
        {
            return nRet;
        }

        /* next line */
        pLineBegin = pLineEnd + 1;
    }

    return (0);
}

/* ----------------------------------------------------------------------
 * cgi_real_loopnext: Get the next index of a loop.
 * add unit loop 2000.11.20 kelly
 * ---------------------------------------------------------------------- */

int cgi_real_loopnext (const char *szLoopKey, UI32_T *nLoopIndex, UI32_T *nIndexArr, BOOL_T *bIsFirst)
{
    UI32_T  maxIndex = 0;

    if (*bIsFirst)
    {
        *bIsFirst = FALSE;

        if ((strcmp(szLoopKey, "queue") == 0) ||
            (strcmp(szLoopKey, "cos") == 0) ||
            (strcmp(szLoopKey, "ipDscp") == 0) ||
            (strcmp(szLoopKey, "ipPrecedence") == 0) ||
            (strcmp(szLoopKey, "ipPort") == 0) ||
            (strcmp(szLoopKey, "sysUserPrivilege") == 0))

        {
            *nLoopIndex = 0;
        }
        else
        {
            *nLoopIndex = 1;
        }
    }
    else
    {
        (*nLoopIndex)++;
    }

    if (strcmp(szLoopKey, "portNoTrunk") == 0)
    {
        maxIndex = SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT;

        /* check port exist, if not exist, get next loop index */
        while (CGI_UTIL_CheckPortExistNoTrunk(nIndexArr[0], *nLoopIndex) != TRUE)
        {
            if (*nLoopIndex >= maxIndex)
                {
                      return 0;
                }
                else
                {
                      (*nLoopIndex)++;
                }
        }

        return 1;
    }
    else if (strcmp(szLoopKey, "trunk") == 0)
    {
        maxIndex = SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM;

        /* check trunk exist, if not exist, get next loop index */
        while (CGI_UTIL_CheckTrunkExistPort(*nLoopIndex) != TRUE)
        {
            if (*nLoopIndex >= maxIndex)
                {
                      return 0;
                }
                else
                {
                      (*nLoopIndex)++;
                }
        }

        return 1;
    }
    else if (strcmp(szLoopKey, "trunkStatic") == 0)
    {
        maxIndex = SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM;

        /* check trunk exist and is static trunk, if not get next loop index */
        while ((CGI_UTIL_CheckTrunkExistPort(*nLoopIndex) != TRUE) ||
               (TRK_PMGR_IsDynamicTrunkId(*nLoopIndex) == TRUE))
        {
            if (*nLoopIndex >= maxIndex)
                {
                      return 0;
                }
                else
                {
                      (*nLoopIndex)++;
                }
        }

        return 1;
    }
    else if (strcmp(szLoopKey, "trunkDynamic") == 0)
    {
        maxIndex = SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM;

        /* check trunk exist and is dynamic trunk, if not get next loop index */
        while ((CGI_UTIL_CheckTrunkExistPort(*nLoopIndex) != TRUE) ||
               (TRK_PMGR_IsDynamicTrunkId(*nLoopIndex) != TRUE))
        {
            if (*nLoopIndex >= maxIndex)
                {
                      return 0;
                }
                else
                {
                      (*nLoopIndex)++;
                }
        }

        return 1;
    }
    else if (strcmp(szLoopKey, "portAll") == 0)
    {
        maxIndex = SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT;

        /* check port exist, if not exist, get next loop index */
        while (CGI_UTIL_CheckPortExist(nIndexArr[0], *nLoopIndex) != TRUE)
        {
            if (*nLoopIndex >= maxIndex)
                {
                      return 0;
                }
                else
                {
                      (*nLoopIndex)++;
                }
        }

        return 1;
    }
    else if (strcmp(szLoopKey, "trunkAll") == 0)
    {
        maxIndex = SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM;
    }
    else if (strcmp(szLoopKey, "queue") == 0)
    {
        maxIndex = SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE - 1;
    }
    else if (strcmp(szLoopKey, "cos") == 0)
    {
        maxIndex = MAX_dot1dTrafficClassPriority;
    }
    else if (strcmp(szLoopKey, "ipDscp") == 0)
    {
        maxIndex = MAX_prioIpDscpValue;
    }
    else if (strcmp(szLoopKey, "ipPrecedence") == 0)
    {
        maxIndex = MAX_prioIpPrecCos;
    }
    else if (strcmp(szLoopKey, "ipPort") == 0)
    {
        maxIndex = MAX_prioIpPortValue;
    }
    else if (strcmp(szLoopKey, "radiusServerIndex") == 0)
    {
        maxIndex = SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS;
    }
    else if (strcmp(szLoopKey, "tacacsServerIndex") == 0)
    {
        maxIndex = SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS;
    }
    else if (strcmp(szLoopKey, "sysUserPrivilege") == 0)
    {
        maxIndex = SYS_ADPT_MAX_LOGIN_PRIVILEGE;
    }

    if (*nLoopIndex > maxIndex)
    {
        return 0;
    }

    return 1;
}

/* ----------------------------------------------------------------------
 * cgi_real_sendloop: Send loop of "real" block to socket.
 * ---------------------------------------------------------------------- */

int cgi_real_sendloop (HTTP_Connection_T *http_connection, int sock, envcfg_t *envcfg, envcfg_t *envQuery, char *pRealBegin, char *pRealEnd,
    CGI_REAL_INFO_T *pRealVar)
{
    char *pLineBegin;
    char *pLineEnd;
    BOOL_T bIsFirst = TRUE;
    int nRet;

    if (pRealVar->loopkeyP == NULL)
    {
        return (-1);
    }

    /* for each index */
    while (cgi_real_loopnext(pRealVar->loopkeyP,
        &pRealVar->indexArr [pRealVar->loopToken-1],
        pRealVar->indexArr, &bIsFirst))
    {
        pLineBegin = pRealBegin;

        /* for each line */
        while (pLineBegin < pRealEnd)
        {
            /* find end of line */
            pLineEnd = cgi_file_lineend (pLineBegin, pRealEnd);

            if ((nRet = cgi_real_sendline (http_connection, sock, envcfg, envQuery, pLineBegin, pLineEnd,
                pRealVar->indexQt, pRealVar->indexArr)) != 0)
            {
                return nRet;
            }

            /* next line */
            pLineBegin = pLineEnd + 1;
        }
    }

/* Exit: */
    return (0);
}

BOOL_T cgi_real_sendValBuf (HTTP_Connection_T *http_connection, CGI_GETVAL_T *get_entry)
{
    char *cgi_real_escBuf = NULL;
    BOOL_T ret = TRUE;

    if ((cgi_real_escBuf = (char *) L_MM_Malloc(CGI_MAX_ESCAPE_STR + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_PARSE_REQUEST))) == NULL)
    {
        ret = FALSE;
        goto fin;
    }
    MM_alloc(cgi_real_escBuf, CGI_MAX_ESCAPE_STR + 1, SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    /* clear escape buffer */
    memset(cgi_real_escBuf, 0, CGI_MAX_ESCAPE_STR + 1);

    cgi_stresc(cgi_real_escBuf, get_entry->str_buf);

    if (cgi_SendText(http_connection, get_entry->sock, cgi_real_escBuf) != 0)
    {
        ret = FALSE;
        goto fin;
    }

    /* clear str buffer */
    memset(get_entry->str_buf, 0, CGI_MAX_VARIABLE_STR + 1);

fin:
    if (cgi_real_escBuf != NULL)
    {
        MM_free(cgi_real_escBuf, SYSFUN_TaskIdSelf());
        L_MM_Free(cgi_real_escBuf);
        cgi_real_escBuf = NULL;
    }

    return ret;
}

BOOL_T cgi_real_sendFuncNext (HTTP_Connection_T *http_connection, int sock, const char *szDisplay, const char *szFuncName)
{
    char *cgi_real_escBuf = NULL;
    char *cgi_real_disBuf = NULL;
    BOOL_T ret = TRUE;

    if ((cgi_real_escBuf = (char *) L_MM_Malloc(CGI_MAX_ESCAPE_STR + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_PARSE_REQUEST))) == NULL)
    {
        ret = FALSE;
        goto fin;
    }
    MM_alloc(cgi_real_escBuf, CGI_MAX_ESCAPE_STR + 1, SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    if ((cgi_real_disBuf = (char *) L_MM_Malloc(CGI_MAX_DISPLAY_STR + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_PARSE_REQUEST))) == NULL)
    {
        ret = FALSE;
        goto fin;
    }
    MM_alloc(cgi_real_disBuf, CGI_MAX_DISPLAY_STR + 1, SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    /* clear buffer */
    memset(cgi_real_escBuf, 0, CGI_MAX_ESCAPE_STR + 1);
    memset(cgi_real_disBuf, 0, CGI_MAX_DISPLAY_STR + 1);

    cgi_stresc(cgi_real_escBuf, szDisplay);
    SYSFUN_Snprintf(cgi_real_disBuf, CGI_MAX_DISPLAY_STR+1, "%s('%s');\n", szFuncName, cgi_real_escBuf);

    if (cgi_SendText(http_connection, sock, cgi_real_disBuf) != 0)
    {
        ret = FALSE;
        goto fin;
    }

fin:
    if (cgi_real_escBuf != NULL)
    {
        MM_free(cgi_real_escBuf, SYSFUN_TaskIdSelf());
        L_MM_Free(cgi_real_escBuf);
        cgi_real_escBuf = NULL;
    }

    if (cgi_real_disBuf != NULL)
    {
        MM_free(cgi_real_disBuf, SYSFUN_TaskIdSelf());
        L_MM_Free(cgi_real_disBuf);
        cgi_real_disBuf = NULL;
    }

    return ret;
}

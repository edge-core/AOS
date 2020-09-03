/* ----------------------------------------------------------------------
 * FILE NAME: cgi\cg_query.c

 * AUTHOR: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Module for Fileless CGI.
 * File to parse the query string.

 * CLASSES AND FUNCTIONS:

 * HISTORY:
 * 1998-05-06 (Wed): Created by ZHONG Qiyao.
 * 1998-09-30 (Wed): Converted large arrays on stack to heap.
 * 1998-09-30 (Wed): Corrected bug in "cgi_query_parse".
 * 1998-11-16 (Mon): Corrected according to Chuanny (Xiao Guochuan).
 * 1999-04-22 (Thu): Can allow two same names in CGI query (envcfg_t).

 * COPYRIGHT (C) Accton Technology Corporation, 1998.
 * ---------------------------------------------------------------------- */

#include "sys_module.h"
#include "l_mm.h"
#include "cgi.h"

/* ----------------------------------------------------------------------
 * Constants.
 * ---------------------------------------------------------------------- */

#define CGI_QUERY_MAX_TEXT    HTTP_REQ_POSTSIZE
#define CGI_QUERY_MAX_ALL     HTTP_REQ_POSTSIZE

/* query string (up to CONTENT_LENGTH only) */
#define CGI_QUERY_VALID_CHAR(c) ((c) != '\0' && (c) != '=' && (c) != '&')

enum {CGI_QUERY_STATE_NAME, CGI_QUERY_STATE_VALUE};

/* ----------------------------------------------------------------------
 * Global constants.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * In-file constants.
 * ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
 * Forward declarations.
 * ---------------------------------------------------------------------- */

void cgi_query_parse (envcfg_t *envQuery, envcfg_t *envHttp);


/* ----------------------------------------------------------------------
 * cgi_query_init: Initialise linked list.

 * Parameters:
 * envQuery: Query linked list.
 * ---------------------------------------------------------------------- */

void cgi_query_init (envcfg_t *envQuery, envcfg_t *envHttp)
{
/* spk mercury
    init_env (envQuery); */

    envQuery->head = NULL;
    envQuery->tail = NULL;

    cgi_query_parse (envQuery, envHttp);
}

/* ----------------------------------------------------------------------
 * cgi_query_fin: Finish query linked list.

 * Parameters:
 * envQuery: Query linked list.
 * ---------------------------------------------------------------------- */

void cgi_query_fin (envcfg_t *envQuery)
{
    free_env (envQuery);
}

/* ----------------------------------------------------------------------
 * cgi_query_parse_text: Parse a text string, which may be name or value.

 * Paramenters:
 * szOut: buffer to store output string
 * pQuery: pointer to start of query string
 * iQuery: pointer to index in query string, will move forward
 * ---------------------------------------------------------------------- */

void cgi_query_parse_text (I8_T *szOut, I8_T *pQuery, int *iQuery)
{
    int iIn = 0, iOut = 0, iTemp;
    I8_T *pIn = pQuery + *iQuery;
    I8_T cTemp, cOut;

    /* convert '+' to space and "%AB" from hexadecimal to byte */
    while ((*iQuery + iIn < CGI_QUERY_MAX_ALL)
        && (CGI_QUERY_VALID_CHAR (pIn [iIn])))
    {
        switch (pIn [iIn])
        {
        /* convert '+' to space */
        case '+':
            cOut = ' ';
            ++iIn;
            break;

        /* convert "%AB" from hexadecimal to byte */
        case '%':

            /* must exist two more characters */
            if (CGI_QUERY_VALID_CHAR (pIn [iIn + 1])
                && CGI_QUERY_VALID_CHAR (pIn [iIn + 2]))
            {
                cOut = 0;

                /* convert each character */
                for (iTemp = iIn + 1; iTemp <= iIn + 2; iTemp++)
                {
                    cOut <<= 4;
                    cTemp = pIn [iTemp];
                    if (isdigit ((UI8_T)cTemp))
                    {
                        cOut += cTemp - '0';
                    }
                    else
                    {
                        cOut += (UI8_T) toupper (cTemp) - 'A' + 10;
                    }
                }

                iIn += 3;
            }

            /* otherwise, plain character */
            else
            {
                cOut = pIn [iIn];
                ++iIn;
            }

            break;

        /* plain character */
        default:
            cOut = pIn [iIn];
            ++iIn;
            break;
        }

        /* write output character */
        if (iOut < CGI_QUERY_MAX_TEXT - 1)
        {
            szOut [iOut] = cOut;
            ++iOut;
        }
    }

    /* update index in query pointer */
    *iQuery += iIn;

    /* end of output string */
    szOut [iOut] = '\0';
}

/* ----------------------------------------------------------------------
 * cgi_query_parse: Parse query string and put in linked list.

 * Parameters:
 * envQuery: Query linked list.
 * envHttp: HTTP Environment linked list.

 * Query string is in form of:
 * <query> := <pair>[&<query>]
 * <pair> := <name>=<value>

 * <name> and <value> may contain escaped characters:
 * "+" means space
 * "%AB" means hexadecimal AB
 * ---------------------------------------------------------------------- */

void cgi_query_parse (envcfg_t *envQuery, envcfg_t *envHttp)
{
    I8_T *pQuery;
    I8_T *szName, *szValue;
    int iIn = 0, nState = CGI_QUERY_STATE_NAME;

    /* get environment string */
    if ((pQuery = get_env (envHttp, "QUERY_STRING")) == NULL)
    {
        return;
    }

    /* buffers to store parsed results */
    if ((szName = (I8_T *) L_MM_Malloc(CGI_QUERY_MAX_TEXT, L_MM_USER_ID2(SYS_MODULE_CGI, CGI_TYPE_TRACE_ID_CGI_QUERY_PARSE))) == NULL)
    {
        return;
    }
    if ((szValue = (I8_T *) L_MM_Malloc(CGI_QUERY_MAX_TEXT, L_MM_USER_ID2(SYS_MODULE_CGI, CGI_TYPE_TRACE_ID_CGI_QUERY_PARSE))) == NULL)
    {
        L_MM_Free (szName);

        // added by chuanny, 1998-11-15, B3 Tom Rev 0.1.17(2)
        // If we forget to return here, our system will
        // crash while we execute the following code.
        return;
    }

    /* look at every character, and break at '=' and '&' */
    /* (Do not check for end-of-string in header of "while";
       it would miss an empty value.) */
    while (iIn < CGI_QUERY_MAX_ALL)
    {
        switch (nState)
        {
        /* name state, look for '=' or end of string */
        case CGI_QUERY_STATE_NAME:

            /* parse entire name */
            cgi_query_parse_text (szName, pQuery, &iIn);

            /* unexpected end of string */
            if (pQuery [iIn] == '\0')
            {
                goto Exit;
            }

            /* end of name, start of value */
            else if (pQuery [iIn] == '=')
            {
                nState = CGI_QUERY_STATE_VALUE;
            }

            ++iIn;
            break;

        /* value state, look for '&' or end of string */
        case CGI_QUERY_STATE_VALUE:

            /* parse entire value */
            cgi_query_parse_text (szValue, pQuery, &iIn);

            /* store into linked list */
#if 0 /* can allow two same names in CGI query, Zhong Qiyao 1999-04-22 */
            if (get_env (envQuery, szName) != NULL)
            {
                unset_env (envQuery, szName);
            }
#endif

            set_env (envQuery, szName, szValue);

            /* end of string */
            if (pQuery [iIn] == '\0')
            {
                goto Exit;
            }

            /* found '&', has more */
            if (pQuery [iIn] == '&')
            {
                nState = CGI_QUERY_STATE_NAME;
            }

            ++iIn;
            break;
        }
    }

Exit:
    L_MM_Free (szValue);
    L_MM_Free (szName);
}

/* ----------------------------------------------------------------------
 * cgi_query_lookup: Lookup variable in query string.

 * Parameters:
 * envQuery: Query li.
nked list.
 * szName: Name to lookup.

 * Return:
 * Pointer to value corresponding to name.
 * ---------------------------------------------------------------------- */

I8_T *cgi_query_lookup (envcfg_t *envQuery, I8_T *szName)
{
    return get_env (envQuery, szName);
}

/* ------------------------------------------------------------------------------
 * cgi_query_env_getfirst: Search through environmental variables for given name.
 * Get the first value of environmental variable specified in name.

 * Parameters:
 * envQuery: Query linked list.
 * name: Name to search.
 * pValue : Buffer to store the value.
 *
 * Return:
 * Pointer to value corresponding to environmenttal field.
 * ---------------------------------------------------------------------- */
envfield *cgi_query_env_getfirst (envcfg_t *envQuery, I8_T *name)
{
    envfield *field;

    field = envQuery->head;

    while (field != NULL)
    {
        if (strcmp(field->name, name) == 0)
        {
            return field;
        }
        field = field->next;
    }

    return NULL;  /* not found */
}
/* ------------------------------------------------------------------------------
 * cgi_query_env_getnext: Get the next value that environmental variable specified
 * in name.

 * Parameters:
 * field: Current field to search.
 * name: Name to search.
 * pValue : Buffer to store the value.
 *
 * Return:
 * 0 for next query exists else return 1.
 * ---------------------------------------------------------------------- */
envfield *cgi_query_env_getnext (envfield *field, I8_T *szName)
{
    while (field->next != NULL)
    {
        if (strcmp (szName, field->next->name) == 0)
        {
            return field->next;
        }
        else
        {
            field = field->next;
        }
    }
    /* can not get next query */
    return NULL;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_QUERY_GetToken
 *------------------------------------------------------------------------------
 * PURPOSE:  Get next token.
 * INPUT:    begin_pp     -- Pointer of pointer of token begin.
 *           line_end_p   -- Pointer of line end.
 * OUTPUT:   token        -- Token.
 * RETURN:   None.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
static void CGI_QUERY_GetToken(char** begin_pp, char* line_end_p, char* token)
{
    char *token_begin_p, *token_end_p;

    /* find begin
     */
    token_begin_p = *begin_pp;

    while ((token_begin_p < line_end_p) &&
        ! (isalpha (*token_begin_p) || isdigit (*token_begin_p)) )
    {
        token_begin_p++;
    }

    /* find end
     */
    token_end_p = token_begin_p;

    while ((token_end_p < line_end_p) &&
        (isalpha (*token_end_p) || isdigit (*token_end_p)) )
    {
        token_end_p++;
    }

    /* copy token
     */
    strncpy(token, token_begin_p, token_end_p - token_begin_p);
    token[token_end_p - token_begin_p] = '\0';

    /* token_begin will be rewrite after call this function
     */
    *begin_pp = token_end_p;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_QUERY_GetTokenLength
 *------------------------------------------------------------------------------
 * PURPOSE:  Get length of next token.
 * INPUT:    begin_pp     -- Pointer of pointer of token begin.
 *           line_end_p   -- Pointer of line end.
 * OUTPUT:   None.
 * RETURN:   token_length -- Length of token.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
static int CGI_QUERY_GetTokenLength(char** begin_pp, char* line_end_p)
{
    int token_length = 0;
    char *token_begin_p, *token_end_p;

    /* find begin
     */
    token_begin_p = *begin_pp;

    while ((token_begin_p < line_end_p) &&
        ! (isalpha (*token_begin_p) || isdigit (*token_begin_p)) )
    {
        token_begin_p++;
    }

    /* find end
     */
    token_end_p = token_begin_p;

    while ((token_end_p < line_end_p) &&
        (isalpha (*token_end_p) || isdigit (*token_end_p)) )
    {
        token_end_p++;
        token_length++;
    }

    return token_length;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_QUERY_ParseQueryString
 *------------------------------------------------------------------------------
 * PURPOSE:  Separate name and value of query from QUERY_STRING,
 *           and set them into envcfg.
 * INPUT:    envcfg       -- QUERY_STRING in environmental configuration.
 * OUTPUT:   envcfg       -- Environmental configuration.
 * RETURN:   TRUE         -- Success.
 *           FALSE        -- Failed.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
BOOL_T CGI_QUERY_ParseQueryString(const char *query_p,
                                  BOOL_T (*fn) (envcfg_t *envcfg, char *name, char *value),
                                  envcfg_t *envcfg)
{
    BOOL_T retval = TRUE;

    size_t query_len;
    char *dup_query_p;
    char *p;

    ASSERT(query_p);

    query_len = strlen(query_p);

    dup_query_p = (char *) L_MM_Malloc(strlen(query_p) + 1, L_MM_USER_ID2(SYS_MODULE_CGI, CGI_TYPE_TRACE_ID_CGI_QUERY_PARSE));

    if (!dup_query_p)
    {
        return FALSE;
    }

    memcpy(dup_query_p, query_p, query_len + 1);

    p = dup_query_p;

    while (*p)
    {
        int end_pos = strcspn(p, "&");
        int eq_pos;
        char *rhs = p;
        char *lhs = NULL;

        if (p[end_pos] == '&')
        {
            p[end_pos] = '\0';
            p = &p[end_pos + 1];
        }
        else
        {
            p = &p[end_pos];
        }

        /* No any content between two &
         */
        if (end_pos == 0)
        {
            continue;
        }

        eq_pos = strcspn(rhs, "=");

        if (rhs[eq_pos] == '=')
        {
            rhs[eq_pos] = '\0';
            lhs = &rhs[eq_pos + 1];
        }
        else
        {
            lhs = &rhs[eq_pos];
        }

        ASSERT(rhs);
        ASSERT(lhs);

        if (fn(envcfg, rhs, lhs) != TRUE)
        {
            retval = FALSE;
            break;
        }
    }

    if (dup_query_p)
    {
        L_MM_Free(dup_query_p);
        dup_query_p = NULL;
    }

    return retval;
}

BOOL_T CGI_QUERY_InsertToEnv(envcfg_t *envcfg, char *name, char *value)
{
#define PREFIX  "url.query."

    ASSERT(name);
    ASSERT(value);

    /* Not allow blank key
     */
    if (name[0] == '\0')
    {
        return FALSE;
    }

    /* Not allow value includes '='
     */
    {
        int pos = strcspn(value, "=");
        if (value[pos] == '=')
        {
            return FALSE;
        }
    }

    {
        int iQuery = 0;
        cgi_query_parse_text(name, (I8_T *) name, &iQuery);
    }

    {
        int iQuery = 0;
        cgi_query_parse_text(value, (I8_T *) value, &iQuery);
    }

    if (set_env_prefix(envcfg, PREFIX, name, value) != 0)
    {
        return FALSE;
    }

    return TRUE;

#undef PREFIX
}


BOOL_T CGI_QUERY_InsertToEnv_Without_Dot_Query(envcfg_t *envcfg, char *name, char *value)
{
#define PREFIX  ""

    ASSERT(name);
    ASSERT(value);

    /* Not allow blank key
     */
    if (name[0] == '\0')
    {
        return FALSE;
    }

    /* Not allow value includes '='
     */
    {
        int pos = strcspn(value, "=");
        if (value[pos] == '=')
        {
            return FALSE;
        }
    }

    {
        int iQuery = 0;
        cgi_query_parse_text(name, (I8_T *) name, &iQuery);
    }

    {
        int iQuery = 0;
        cgi_query_parse_text(value, (I8_T *) value, &iQuery);
    }

    if (set_env_prefix(envcfg, PREFIX, name, value) != 0)
    {
        return FALSE;
    }

    return TRUE;

#undef PREFIX
}


/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_QUERY_ParseVariableParameter
 *------------------------------------------------------------------------------
 * PURPOSE:  Parse variable name from token and save it to envcfg.
 * INPUT:    in_pp        -- Pointer of pointer of token begin.
 *           line_end_p   -- Pointer of line end.
 * OUTPUT:   envcfg       -- Environmental configuration.
 * RETURN:   None.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
BOOL_T CGI_QUERY_ParseVariableParameter(envcfg_t *envcfg, char **in_pp, char *line_end_p)
{
    int token_len = 0;
    char *token_begin_p, *tag_name_value_p = NULL, *tag_variable_value_p = NULL;
    TOKEN_T tag_name_key,
            tag_name_value,
            tag_variable_key,
            tag_variable_value;

    memset(&tag_name_key, 0, sizeof(TOKEN_T));
    memset(&tag_name_value, 0, sizeof(TOKEN_T));
    memset(&tag_variable_key, 0, sizeof(TOKEN_T));
    memset(&tag_variable_value, 0, sizeof(TOKEN_T));

    /* find first token
     */
    token_begin_p = *in_pp;
    token_len = CGI_QUERY_GetTokenLength(&token_begin_p, line_end_p);

    if ((tag_name_value_p = (char *) L_MM_Malloc(token_len + 1, L_MM_USER_ID2(SYS_MODULE_CGI, CGI_TYPE_TRACE_ID_CGI_QUERY_PARSEVARIABLEPARAMETER_1))) == NULL)
    {
        return FALSE;
    }

    CGI_QUERY_GetToken(&token_begin_p, line_end_p, tag_name_value_p);

    if (strcmp(tag_name_value_p, "SSI") != 0)
    {
        L_MM_Free(tag_name_value_p);
        tag_name_value_p = NULL;
        return FALSE;
    }
    else
    {
        tag_name_value.p = tag_name_value_p;
        tag_name_value.len = token_len;

        /* find next token
         */
        token_len = CGI_QUERY_GetTokenLength(&token_begin_p, line_end_p);

        if ((tag_variable_value_p = (char *) L_MM_Malloc(token_len + 1, L_MM_USER_ID2(SYS_MODULE_CGI, CGI_TYPE_TRACE_ID_CGI_QUERY_PARSEVARIABLEPARAMETER_2))) == NULL)
        {
            L_MM_Free(tag_name_value_p);
            tag_name_value_p = NULL;
            return FALSE;
        }

        CGI_QUERY_GetToken(&token_begin_p, line_end_p, tag_variable_value_p);
        tag_variable_value.p = tag_variable_value_p;
        tag_variable_value.len = token_len;

        tag_name_key.p = (char *) "tag.name";
        tag_name_key.len = sizeof("tag.name") - 1;
        tag_variable_key.p = (char *) "tag.variable";
        tag_variable_key.len = sizeof("tag.variable") - 1;

        //
        // FIXME: Remove these functions
        //

        set_env_token(envcfg, &tag_name_key, &tag_name_value);
        set_env_token(envcfg, &tag_variable_key, &tag_variable_value);

        L_MM_Free(tag_name_value_p);
        tag_name_value_p = NULL;
        L_MM_Free(tag_variable_value_p);
        tag_variable_value_p = NULL;
        return TRUE;
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - CGI_QUERY_ClearVariableParameter
 *------------------------------------------------------------------------------
 * PURPOSE:  Clear all environmental variable whose prefix of key is "tag.".
 * INPUT:    envcfg       -- Environmental configuration.
 * OUTPUT:   envcfg       -- Environmental configuration.
 * RETURN:   None.
 * NOTE:     None.
 *------------------------------------------------------------------------------
 */
BOOL_T CGI_QUERY_ClearVariableParameter(envcfg_t *envcfg)
{
    envfield *env, *temp_env;

    env = envcfg->head;

    while (env != NULL)
    {
        temp_env = env->next;

        if (strncmp(env->name, "tag.", sizeof("tag.") - 1) == 0)
        {
            unset_env(envcfg, env->name);
        }

        env = temp_env;
    }

    return TRUE;
}

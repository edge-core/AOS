#include <time.h>
#include "libtacacs.h"
#include <string.h>
#include "l_mm.h"

#if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE) \
 || (SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD == SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION)

/* NAMING CONSTANT DECLARATIONS
 */
/* define TAC_UTILS_DEBUG_MODE
 * to build tac_packet.c with DEBUG version
 * And let following macros print debug messages
 */

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef TAC_UTILS_DEBUG_MODE

    #define TAC_UTILS_TRACE(msg)                     (printf(msg))
    #define TAC_UTILS_TRACE1(msg, arg)               (printf(msg, arg))
    #define TAC_UTILS_TRACE2(msg, arg1, arg2)        (printf(msg, arg1, arg2))
    #define TAC_UTILS_TRACE3(msg, arg1, arg2, arg3)  (printf(msg, arg1, arg2, arg3))

#else

    #define TAC_UTILS_TRACE(msg)                     ((void)0)
    #define TAC_UTILS_TRACE1(msg, arg)               ((void)0)
    #define TAC_UTILS_TRACE2(msg, arg1, arg2)        ((void)0)
    #define TAC_UTILS_TRACE3(msg, arg1, arg2, arg3)  ((void)0)

#endif /* TAC_UTILS_DEBUG_MODE */

/* DATA TYPE DECLARATIONS
 */
/* snprintf() does not define in c_lib.h */
extern int snprintf(char *str, size_t count, const char *fmt, ...);

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */


/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_FreeAvPairs
 *---------------------------------------------------------------------------
 * PURPOSE  : free Attribute-Value pairs
 * INPUT    : av_pair_list_p
 * OUTPUT   : av_pair_list_p
 * RETURN   : None
 * NOTE     : the last element of av_pair_list_p MUST be NULL or
 *            the size of av_pair_list_p MUST be not large than
 *            TACACS_LIB_MAX_NBR_OF_AV_PAIRS
 *---------------------------------------------------------------------------
 */
void TACACS_LIB_FreeAvPairs(char **av_pair_list_p)
{
    UI32_T  av_index;

    if (NULL == av_pair_list_p)
        return;

    for (av_index = 0; TACACS_LIB_MAX_NBR_OF_AV_PAIRS > av_index; av_index++)
    {
        if (NULL == av_pair_list_p[av_index]) /* no remainder */
            return;

        L_MM_Free(av_pair_list_p[av_index]); /*maggie liu for authorization*/
        av_pair_list_p[av_index] = NULL;
    }
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_FindAttributeInAvPairs
 *---------------------------------------------------------------------------
 * PURPOSE  : search specified attribute in av_pair_list_p
 * INPUT    : search_attribute_p, av_pair_list_p
 * OUTPUT   : None
 * RETURN   : return the array index in av_pair_list_p if attribute found
 *            if return value < 0, implies the attribute not found
 * NOTE     : the last element of av_pair_list_p MUST be NULL or
 *            the size of av_pair_list_p MUST be not large than
 *            TACACS_LIB_MAX_NBR_OF_AV_PAIRS
 *---------------------------------------------------------------------------
 */
I32_T TACACS_LIB_FindAttributeInAvPairs(const char *search_attribute_p, char **av_pair_list_p)
{
    UI32_T      av_index, attribute_len;
    const char  *found_str_p;

    if (NULL == av_pair_list_p)
        return -1;

    attribute_len = strlen(search_attribute_p);

    for (av_index = 0; TACACS_LIB_MAX_NBR_OF_AV_PAIRS > av_index; av_index++)
    {
        if (NULL == av_pair_list_p[av_index]) /* no remainder */
            return -1;

        found_str_p = strstr(av_pair_list_p[av_index], search_attribute_p);

        /* 1, if search_attribute_p is found in av_pair string and
         *    this av_pair string begin with search_attribute_p
         * 2, if the character '*' or '=' follows found_str_p
         */
        if ((found_str_p == av_pair_list_p[av_index]) &&
            ((TACACS_LIB_AV_PAIR_OPTIONAL_CHAR == found_str_p[attribute_len]) ||
             (TACACS_LIB_AV_PAIR_MANDATORY_CHAR == found_str_p[attribute_len])))
        {
            return av_index;
        }
    }

    return -1;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_FindAttributeInAttributeList
 *---------------------------------------------------------------------------
 * PURPOSE  : search specified attribute in a attribute list
 * INPUT    : search_attribute_p, list_p
 * OUTPUT   : None
 * RETURN   : return the array index in list_p if attribute found
 *            if return value < 0, implies the attribute not found
 * NOTE     : the last element of list_p MUST be NULL
 *---------------------------------------------------------------------------
 */
I32_T TACACS_LIB_FindAttributeInAttributeList(const char *search_attribute_p, const char **list_p)
{
    UI32_T  attribute_index;

    for (attribute_index = 0; NULL != list_p[attribute_index]; attribute_index++)
    {
        if (0 == strcmp(search_attribute_p, list_p[attribute_index])) /* found */
            return attribute_index;
    }

    TAC_UTILS_TRACE1("\r\n[TACACS_LIB_FindAttributeInAttributeList] attribute(%s) not found",
        search_attribute_p);
    return -1;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_FindSeparatedCharInAvPair
 *---------------------------------------------------------------------------
 * PURPOSE  : find av-pair separated character
 * INPUT    : av_pair_p
 * OUTPUT   : None
 * RETURN   : return the address of seprated character in av_pair_p
 *            return NULL if not found
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
char* TACACS_LIB_FindSeparatedCharInAvPair(char *av_pair_p)
{
    for ( ; '\0' != *av_pair_p; av_pair_p++)
    {
        if ((TACACS_LIB_AV_PAIR_MANDATORY_CHAR == *av_pair_p) ||
            (TACACS_LIB_AV_PAIR_OPTIONAL_CHAR == *av_pair_p))
        {
            return av_pair_p;
        }
    }

    TAC_UTILS_TRACE1("\r\n[TACACS_LIB_FindSeparatedCharInAvPair] malformed av pair string(%s)",
        av_pair_p);

    return NULL;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_CheckReturnedAvPairs
 *---------------------------------------------------------------------------
 * PURPOSE  : check Attribute-Value pairs by attribute check list
 * INPUT    : check_list_p, av_pair_list_p
 * OUTPUT   : None
 * RETURN   : TRUE -- success / FALSE -- failure
 * NOTE     : 1, return FALSE if a mandatory attribute exist and it is not in
 *               check-list
 *            2, return FALSE if malformed pair is found
 *            3, the last element of av_pair_list_p MUST be NULL or
 *               the size of av_pair_list_p MUST be not large than
 *               TACACS_LIB_MAX_NBR_OF_AV_PAIRS
 *            4, the last element of check_list_p MUST be NULL
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_LIB_CheckReturnedAvPairs(const char **check_list_p, char **av_pair_list_p)
{
    UI32_T  av_index;
    char    seperated_char, *str_p;

    for (av_index = 0; TACACS_LIB_MAX_NBR_OF_AV_PAIRS > av_index; av_index++)
    {
        if (NULL == av_pair_list_p[av_index]) /* no remainder */
            return TRUE;

        TAC_UTILS_TRACE1("\r\n[TACACS_LIB_CheckReturnedAvPairs] av_str(%s)",
            av_pair_list_p[av_index]);

        str_p = TACACS_LIB_FindSeparatedCharInAvPair(av_pair_list_p[av_index]);
        if (NULL == str_p) /* malformed av pair string */
            return FALSE;

        seperated_char = *str_p;

        /* draft: option arguments are ones that may be disregarded
         *        by either client or daemon
         * as a result, no need to verify whether it exist in check-list or not
         */
        if (TACACS_LIB_AV_PAIR_OPTIONAL_CHAR == seperated_char)
            continue;

        *str_p = '\0'; /* to terminate attribute string temporarily */

        /* draft: If the client receives a mandatory argument that
         *        it cannot oblige or does not understand, it MUST
         *        consider the authorization to have failed.
         * as a result, return FALSE if an un-expected mandatory
         * attribute exists
         */
        if (0 > TACACS_LIB_FindAttributeInAttributeList(av_pair_list_p[av_index], check_list_p))
        {
            *str_p = seperated_char; /* restore seperated character */
            return FALSE;
        }

        *str_p = seperated_char; /* restore seperated character */
    }

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_FormatPortString
 *---------------------------------------------------------------------------
 * PURPOSE  : free Attribute-Value pairs
 * INPUT    : ifindex
 * OUTPUT   : port_str_p
 * RETURN   : None
 * NOTE     : None.
 *---------------------------------------------------------------------------
 */
void TACACS_LIB_FormatPortString(UI32_T ifindex, char *port_str_p)
{
    if (ifindex == 0) /*CLI session id is 0*/
    {
        /* There have a known problem in snprintf, so that
         * use sprintf to replace snprintf
         */
        /*snprintf(port_str_p, TACACS_LIB_MAX_LEN_OF_PORT_STRING, "Console");*/
        sprintf(port_str_p, "Console");
    }
    else
    {
        /* There have a known problem in snprintf, so that
         * use sprintf to replace snprintf
         */
        /* vty count from 0 ~ */
        /*snprintf(port_str_p, LIBTACACS_MAX_LEN_OF_PORT_STRING, "Vty-%ld",
            ifindex - 1);*/
        sprintf(port_str_p, "Vty-%ld", (long)ifindex - 1);
    }

    port_str_p[TACACS_LIB_MAX_LEN_OF_PORT_STRING] = '\0';
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TAC_UTILS_GetNasPort
 *---------------------------------------------------------------------------
 * PURPOSE  : Get NAS port description from caller type
 * INPUT    : caller_type - Caller type
 *            line_no     - (optional) Connected line number
 * OUTPUT   : nas_port    - nas port description
 * RETURN   : TRUE/Succeeded, FALSE/Failed
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
BOOL_T TAC_UTILS_GetNasPort(
    TACACS_SessType_T sess_type,
    UI32_T line_no,
    char *nas_port,
    UI32_T nas_port_size)
{
    if (NULL == nas_port)
    {
        return FALSE;
    }

    nas_port[0] = '\0';

    if (nas_port_size < TACACS_TYPE_MAX_LEN_OF_NAS_PORT+1)
    {
        return FALSE;
    }

    switch (sess_type)
    {
        case TACACS_SESS_TYPE_CONSOLE:
            strncpy(nas_port, "Console", TACACS_TYPE_MAX_LEN_OF_NAS_PORT);
            break;

        case TACACS_SESS_TYPE_TELNET:
        case TACACS_SESS_TYPE_SSH:
            snprintf(nas_port, TACACS_TYPE_MAX_LEN_OF_NAS_PORT, "Vty-%lu", (unsigned long)line_no);
            break;

        case TACACS_SESS_TYPE_HTTP:
            strncpy(nas_port, "HTTP", TACACS_TYPE_MAX_LEN_OF_NAS_PORT);
            break;

        case TACACS_SESS_TYPE_HTTPS:
            strncpy(nas_port, "HTTPS", TACACS_TYPE_MAX_LEN_OF_NAS_PORT);
            break;

        default:
            return FALSE;
    }

    nas_port[TACACS_TYPE_MAX_LEN_OF_NAS_PORT] = '\0';
    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TAC_UTILS_GetRemAddr
 *---------------------------------------------------------------------------
 * PURPOSE  : Get Remote address from caller type
 * INPUT    : caller_type - Caller type
 *            caller_addr - (optional) Caller address
 * OUTPUT   : rem_addr    - remote address
 * RETURN   : TRUE/Succeeded, FALSE/Failed
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
BOOL_T TAC_UTILS_GetRemAddr(
    TACACS_SessType_T sess_type,
    const L_INET_AddrIp_T *caller_addr,
    char *rem_addr,
    UI32_T rem_addr_size)
{
    if (NULL == caller_addr || NULL == rem_addr)
    {
        return FALSE;
    }

    rem_addr[0] = '\0';

    if (rem_addr_size < TACACS_TYPE_MAX_LEN_OF_REM_ADDR+1)
    {
        return FALSE;
    }

    switch (sess_type)
    {
        case TACACS_SESS_TYPE_CONSOLE:
            strcpy(rem_addr, "async");
            break;

        case TACACS_SESS_TYPE_TELNET:
        case TACACS_SESS_TYPE_SSH:
        case TACACS_SESS_TYPE_HTTP:
        case TACACS_SESS_TYPE_HTTPS:
            L_INET_InaddrToString((L_INET_Addr_T *) caller_addr, rem_addr, rem_addr_size);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_ConvertAuthMethod
 *---------------------------------------------------------------------------
 * PURPOSE  : convert TACACS_Authentic_T to auth_method
 * INPUT    : auth_by_whom
 * OUTPUT   : None
 * RETURN   : auth_method
 * NOTE     : aute_method defined in TACACS+ draft could be:
 *                  TAC_PLUS_AUTHEN_METH_RADIUS;
 *                  TAC_PLUS_AUTHEN_METH_TACACSPLUS;
 *                  TAC_PLUS_AUTHEN_METH_LOCAL;
 *                  TAC_PLUS_AUTHEN_METH_NOT_SET;
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_LIB_ConvertAuthMethod(TACACS_Authentic_T auth_by_whom)
{
    switch (auth_by_whom)
    {
        case TACACS_AUTHEN_BY_RADIUS:
            return TAC_PLUS_AUTHEN_METH_RADIUS;
        case TACACS_AUTHEN_BY_TACACS_PLUS:
            return TAC_PLUS_AUTHEN_METH_TACACSPLUS;
        case TACACS_AUTHEN_BY_LOCAL:
            return TAC_PLUS_AUTHEN_METH_LOCAL;
        default:
            break;
    }

    return TAC_PLUS_AUTHEN_METH_NOT_SET;
}

#endif /* SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE */

unsigned int call_time(void)
{
 return time(NULL);
}

/*
   this function translate tacacs server authenticaton reply status
   to string
*/
char*
tac_print_authen_status(int status) {

   switch(status) {
   case 1:
      return("TAC_PLUS_AUTHEN_STATUS_PASS");
      break;
   case 2:
      return("TAC_PLUS_AUTHEN_STATUS_FAIL");
      break;
   case 3:
      return("TAC_PLUS_AUTHEN_STATUS_GETDATA");
      break;
   case 4:
      return("TAC_PLUS_AUTHEN_STATUS_GETUSER");
      break;
   case 5:
      return("TAC_PLUS_AUTHEN_STATUS_GETPASS");
      break;
   case 6:
      return("TAC_PLUS_AUTHEN_STATUS_RESTART");
      break;
   case 7:
      return("TAC_PLUS_AUTHEN_STATUS_ERROR");
      break;
   case 0x21:
      return("TAC_PLUS_AUTHEN_STATUS_FOLLOW");
      break;
   default:
      return("Unknown status");
      break;
  }
  return(NULL);
}


/* translate authorization status to string */
char*
tac_print_author_status(int status) {
      switch(status) {
       case TAC_PLUS_AUTHOR_STATUS_PASS_ADD:
	  return("TAC_PLUS_AUTHOR_STATUS_PASS_ADD");
	  break;
       case TAC_PLUS_AUTHOR_STATUS_PASS_REPL:
	  return("TAC_PLUS_AUTHOR_STATUS_PASS_REPL");
	  break;
       case TAC_PLUS_AUTHOR_STATUS_FAIL:
	  return("TAC_PLUS_AUTHOR_STATUS_FAIL");
	  break;
       case TAC_PLUS_AUTHOR_STATUS_ERROR:
	  return("TAC_PLUS_AUTHOR_STATUS_ERROR");
	  break;
       case TAC_PLUS_AUTHOR_STATUS_FOLLOW:
	  return("TAC_PLUS_AUTHOR_STATUS_FOLLOW");
	  break;
       default:
	  return("Unknown");
	  break;
      }
      return(NULL);
}


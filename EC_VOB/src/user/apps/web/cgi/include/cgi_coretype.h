/*****************************************************************************
;
;   (C) Unpublished Work of Accton Technology,  Corp.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF ACCTON TECHNOLOGY CORP.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) ACCTON EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN ACCTON WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF ACCTON.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project : Common Plateform
;    Creator : Simon
;    File    : coretype.h
;    Abstract: define the global types of the core routines
;
;Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;
;*****************************************************************************/

#ifndef CGI_CORETYPE_H
#define CGI_CORETYPE_H

/*---------------------------
 * INCLUDE FILE DECLARATIONS
 *---------------------------*/
#include "sys_cpnt.h"
#include "sys_adpt.h" // FIXME: remote this dependency
#include "sys_type.h"

#define CGI_MAX_TOKEN 	            64                                                     /* can use max token length */
#define CGI_MAX_STRPARAM            135                                                    /* can use max STRPARAM length */
#define CGI_MAX_FUNCNEXT_INDEX 	    2560                                                  /* FUNCNEXT can use max index length */
#define CGI_MAX_FUNCNEXT_FUNCNAME   64                                                     /* FUNCNEXT can use max function name length */
#define CGI_MAX_VARIABLE_STR        1024                                                   /* variable string max length */
#define CGI_MAX_ESCAPE_STR          CGI_MAX_VARIABLE_STR * 3                               /* variable escape string max length */
#define CGI_MAX_DISPLAY_STR         CGI_MAX_ESCAPE_STR + CGI_MAX_FUNCNEXT_FUNCNAME + 32    /* FUNCNEXT use to display function and variable escape string */

#define CGI_MACHEXLEN              SYS_ADPT_MAC_ADDR_LEN         /* mac hex length */
#define CGI_MACSTRLEN              18                            /* mac string length */
#define CGI_IPLEN                  4                             /* ip length */
#define CGI_IPSTRLEN               16                            /* ip string length */
#define CGI_UI64STRLEN             21                            /* UI64 string length */

#define CGI_CORE_NOTFOUND (-100)

#define CGI_CORETYPE_MAX_UI8_VALUE  0xff
#define CGI_CORETYPE_MAX_UI16_VALUE 0xffff
#define CGI_CORETYPE_MAX_UI32_VALUE 0xffffffff

/* spk mercury */
typedef	unsigned long	CGI_SIZE_T;


/************************************/
/*  Global structure types define   */
/************************************/
#define STRUCT	__packed__ struct

typedef struct CGI_GETVAR_OID_S
{
    UI32_T 	num_components;		/* original:int */
    UI32_T  *component_list;	/* original:WORD */
} CGI_GETVAR_OID_T;

typedef struct CGI_GETVAR_OCTET_S
{
    UI8_T 	*octet_P;
    UI32_T  len;				/* original:int */
} CGI_GETVAR_OCTET_T;

typedef union CGI_GETVAR_VAL_S
{
    UI64_T      dval;
    UI32_T      ival;			/* original:WORD */
    CGI_GETVAR_OCTET_T  octet;
    CGI_GETVAR_OID_T    oid;
} GI_GETVAR_VAL_T;   // unused ?

typedef struct CGI_GETVAL_S {
    int    sock;
    const UI32_T *index_arr;
    char  *str_buf;
    const char  *str_param;
} CGI_GETVAL_T;

enum
{
    CGI_TYPE_TRACE_ID_CGI_BUILD_HEADER=0,
    CGI_TYPE_TRACE_ID_CGI_QUERY_PARSE,
    CGI_TYPE_TRACE_ID_CGI_REAL_SENDOPTIONRANDOM,
    CGI_TYPE_TRACE_ID_CGI_REAL_SENDROWTRRANDOM,
    CGI_TYPE_TRACE_ID_CGI_REAL_SENDLINE,
    CGI_TYPE_TRACE_ID_CGI_REAL_SENDVAL,
    CGI_TYPE_TRACE_ID_CGI_REAL_LARGEBUFFER,
    CGI_TYPE_TRACE_ID_CGI_CORE_GET_NATIVE,
    CGI_TYPE_TRACE_ID_CGI_CORE_GET_UNIFIED,
    CGI_TYPE_TRACE_ID_CGI_CORE_SET_NATIVE,
    CGI_TYPE_TRACE_ID_CGI_SUBMIT_ALLOCATE_TMPBUF,
    CGI_TYPE_TRACE_ID_CGI_UNAUTHORISED,
    CGI_TYPE_TRACE_ID_CGI_SENDHEADER,
    CGI_TYPE_TRACE_ID_CGI_NOTFOUND,
    CGI_TYPE_TRACE_ID_CGI_REDIRECT_HTTPS,
    CGI_TYPE_TRACE_ID_CGI_MULTIPART_BDRY,
    CGI_TYPE_TRACE_ID_CGI_MULTIPART_RECV,
    CGI_TYPE_TRACE_ID_CGI_MULTI_FILEUPLOAD,
    CGI_TYPE_TRACE_ID_CGI_QUERY_SETQUERYSTRINGENV_1,
    CGI_TYPE_TRACE_ID_CGI_QUERY_SETQUERYSTRINGENV_2,
    CGI_TYPE_TRACE_ID_CGI_QUERY_PARSEVARIABLEPARAMETER_1,
    CGI_TYPE_TRACE_ID_CGI_QUERY_PARSEVARIABLEPARAMETER_2,
    CGI_TYPE_TRACE_ID_CGI_LIB_JSONPARSESTRING
};

enum
{
    CGI_TYPE_HTTP_UPGRADE_NOT_COPY = 0,
    CGI_TYPE_HTTP_UPGRADE_COPYING
};

typedef enum CGI_TYPE_Lport_Type_E
{
    CGI_TYPE_LPORT_UNKNOWN_PORT = 0,
    CGI_TYPE_LPORT_NORMAL_PORT,
    CGI_TYPE_LPORT_TRUNK_PORT,
    CGI_TYPE_LPORT_TRUNK_PORT_MEMBER,
    CGI_TYPE_LPORT_MGMT_PORT,
} CGI_TYPE_Lport_Type_T;

/************************************************
 * Error code is returned when errors occured.  *
 ************************************************/
enum   CORE_ERROR_CODE_E
{
    CORE_NO_ERROR = 0,
    CORE_NO_SUCH_NAME     = 2,
    CORE_BAD_VALUE        = 3,
    CORE_NULL_BUFFER,
    CORE_BUFFER_LEN_ERROR,
    CORE_BAD_IP_ADDR,
    CORE_BAD_NET_MASK,
    CORE_BAD_IP_NET,
    CORE_BAD_IP_HOST_ID,
    CORE_BAD_GATEWAY_IP,
    CORE_BAD_IPX_NET,
    CORE_BAD_IPX_NODE,
    CORE_BAD_MAC_ADDR,
    CORE_BAD_INDEX,
    CORE_BAD_PORT_NO,
    CORE_RESOURCE_LOCK,
    CORE_NO_SUCH_MAC_ADDR,
    CORE_END_OF_ADDR_TABLE,
    CORE_GEN_ERROR,
    CORE_TIMEOUT,
    CORE_SSI_FAIL,
    CORE_OUT_OF_MEMORY
};

typedef enum CORE_ERROR_CODE_E CGI_CORETYPE_T;

#endif  /* CGI_CORETYPE_H */

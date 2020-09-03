/* ------------------------------------------------------------------------
 * FILE NAME - DOS_TYPE.H
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Wakka             24/01/2011      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2011
 * ------------------------------------------------------------------------
 */
#ifndef DOS_TYPE_H
#define DOS_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define DOS_TYPE_IPCMSG_KEY     SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY

/* MACRO FUNCTION DECLARATIONS
 */
#define DOS_TYPE_LST_ENUM(a)    a,
#define DOS_TYPE_LST_NAME(a)    #a,

#define DOS_TYPE_E_LST(_)       \
    _(DOS_TYPE_E_OK)            \
    _(DOS_TYPE_E_INVALID)       \
    _(DOS_TYPE_E_NOT_READY)     \
    _(DOS_TYPE_E_NOT_SUPPORT)   \
    _(DOS_TYPE_E_UNKNOWN)

/* Definitions of fields
 *
 * valid range of data
 *   STATUS    : see DOS_TYPE_Status_T
 *   RATELIMIT : SYS_ADPT_DOS_MIN_RATELIMIT ~ SYS_ADPT_DOS_MAX_RATELIMIT
 */
#define DOS_TYPE_FLD_LST(_)     \
    _(DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_STATUS           /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_RATELIMIT        /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_LAND_STATUS                   /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_SMURF_STATUS                  /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_STATUS           /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_RATELIMIT        /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_TCP_NULL_SCAN_STATUS          /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_TCP_SCAN_STATUS               /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_TCP_SYN_FIN_SCAN_STATUS       /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_TCP_UDP_PORT_ZERO_STATUS      /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_TCP_XMAS_SCAN_STATUS          /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_STATUS           /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_RATELIMIT        /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_WIN_NUKE_STATUS               /* GLOBAL UI32   RW */) \
    _(DOS_TYPE_FLD_SYSTEM_WIN_NUKE_RATELIMIT            /* GLOBAL UI32   RW */)

#define DOS_TYPE_DBG_LST(_)       \
    _(DOS_TYPE_DBG_TRACE)         \

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    DOS_TYPE_DBG_LST(DOS_TYPE_LST_ENUM)
    DOS_TYPE_DBG_NUM,
} DOS_TYPE_DbgFlag_T;

typedef enum
{
    DOS_TYPE_E_LST(DOS_TYPE_LST_ENUM)
    DOS_TYPE_E_NUM,
} DOS_TYPE_Error_T;

typedef enum
{
    DOS_TYPE_FLD_LST(DOS_TYPE_LST_ENUM)
    DOS_TYPE_FLD_NUM,
} DOS_TYPE_FieldId_T;

typedef enum
{
    DOS_TYPE_FTYPE_UI32,
    DOS_TYPE_FTYPE_UNKNOWN,
} DOS_TYPE_FieldType_T;

typedef union
{
    UI32_T u32;
} DOS_TYPE_FieldDataBuf_T;

typedef union
{
    void *p;
    UI32_T *u32_p;
} DOS_TYPE_FieldDataPtr_T;

typedef enum {
    DOS_TYPE_STATUS_ENABLED     = 1,
    DOS_TYPE_STATUS_DISABLED    = 2,
} DOS_TYPE_Status_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif /* DOS_TYPE_H */


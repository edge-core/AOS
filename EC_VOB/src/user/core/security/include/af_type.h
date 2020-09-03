/* ------------------------------------------------------------------------
 * FILE NAME - AF_TYPE_H
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Ezio             14/03/2013      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2013
 * ------------------------------------------------------------------------
 */
#ifndef _AF_TYPE_H
#define _AF_TYPE_H

#include "sys_type.h"
#include "sys_bld.h"

#define AF_TYPE_IPCMSG_KEY     SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY

typedef enum
{
    AF_TYPE_SUCCESS,
    AF_TYPE_E_UNKNOWN,
    AF_TYPE_E_PARAMETER,
    AF_TYPE_E_UNKOWPKT,
    AF_TYPE_E_NOT_READY,
    AF_TYPE_E_IPC_CMD_INVALID,
    AF_TYPE_E_SET_SWCTRL,
} AF_TYPE_ErrorCode_T;

typedef enum
{
    AF_TYPE_DEFAULT = 0,
    AF_TYPE_DISCARD,

    AF_TYPE_STATUS_MIN = 0,
    AF_TYPE_STATUS_MAX = AF_TYPE_DISCARD,
} AF_TYPE_STATUS_T;

#endif /* #ifndef _AF_TYPE_H */


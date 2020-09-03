/* ------------------------------------------------------------------------
 * FILE NAME - DOS_MGR.C
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
/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "swctrl_pmgr.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_DOS == TRUE)
#include "dos_type.h"
#include "dos_mgr.h"
#include "dos_om.h"

/* NAMING CONSTANT DECLARARTIONS
 */

/* TYPE DEFINITIONS
 */

/* MACRO DEFINITIONS
 */
#ifndef SYSFUN_USE_CSC
#define SYSFUN_USE_CSC(r)
#define SYSFUN_RELEASE_CSC()
#endif

#define DOS_USE_CSC(r)      SYSFUN_USE_CSC(r)
#define DOS_RELEASE_CSC()   SYSFUN_RELEASE_CSC()

#define DOS_MGR_LOCK()
#define DOS_MGR_UNLOCK()

#define DOS_MGR_DBG_TRACE_MSG(fmt, ...) \
    if (DOS_OM_IsDebugFlagOn(DOS_TYPE_DBG_TRACE)) \
        BACKDOOR_MGR_Printf("%s:%d: " fmt "\r\n", __func__, __LINE__, ##__VA_ARGS__);   \
    else

/* LOCAL FUNCTIONS DECLARATIONS
 */
static BOOL_T DOS_MGR_IsFieldSupported(DOS_TYPE_FieldId_T field_id);
static BOOL_T DOS_MGR_CheckDataValidity(DOS_TYPE_FieldId_T field_id, void *data_p);
static BOOL_T DOS_MGR_CompareData(DOS_TYPE_FieldId_T field_id, void *data1_p, void *data2_p);
static BOOL_T DOS_MGR_GetDefaultDataByField(DOS_TYPE_FieldId_T field_id, void *data_p);
static BOOL_T DOS_MGR_SetDataByField_Internal(DOS_TYPE_FieldId_T field_id, void *data_p);
static BOOL_T DOS_MGR_ConfigSystem(BOOL_T use_default);

/* LOCAL VARIABLES DECLARATIONS
 */
SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_MGR_InitiateSystemResources(void)
{
    DOS_OM_Init();
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - DOS_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *------------------------------------------------------------------------*/
void DOS_MGR_Create_InterCSC_Relation(void)
{
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: Set component mode to Transition.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 *--------------------------------------------------------------------------
 */
void DOS_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter transition mode.
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();

    DOS_OM_Reset();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_MGR_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE();

    DOS_MGR_ConfigSystem(TRUE);
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE: This function will initialize the port_p OM of the module ports
 *          when the option module is inserted.
 * INPUT  : starting_port_ifindex -- the ifindex of the first module port_p
 *                                   inserted
 *          number_of_port        -- the number of ports on the inserted
 *                                   module
 *          use_default           -- the flag indicating the default
 *                                   configuration is used without further
 *                                   provision applied; TRUE if a new module
 *                                   different from the original one is
 *                                   inserted
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void DOS_MGR_HandleHotInsertion(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port,
    BOOL_T in_use_default)
{
    DOS_MGR_ConfigSystem(FALSE);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE: This function will clear the port_p OM of the module ports when
 *          the option module is removed.
 * INPUT  : starting_port_ifindex -- the ifindex of the first module port_p
 *                                   removed
 *          number_of_port        -- the number of ports on the removed
 *                                   module
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void DOS_MGR_HandleHotRemoval(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port)
{
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE: Handle the ipc request message.
 * INPUT  : msgbuf_p -- input request ipc message buffer
 * OUTPUT : msgbuf_p -- output response ipc message buffer
 * RETURN : TRUE  - there is a  response required to be sent
 *          FALSE - there is no response required to be sent
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DOS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *msgbuf_p)
{
    DOS_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (DOS_MGR_IpcMsg_T *)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.result = DOS_TYPE_E_NOT_READY;
        msgbuf_p->msg_size = DOS_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding MGR function
     */
    switch (msg_p->type.cmd)
    {
        case DOS_MGR_IPC_SET_DATA_BY_FIELD:
            msgbuf_p->msg_size = DOS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            msg_p->type.result = DOS_MGR_SetDataByField(
                msg_p->data.fld_data.field_id,
                (msg_p->data.fld_data.use_dflt ? NULL : &msg_p->data.fld_data.data));
            break;
        case DOS_MGR_IPC_GET_DATA_BY_FIELD:
            msgbuf_p->msg_size = DOS_MGR_GET_MSG_SIZE(fld_data);
            msg_p->type.result = DOS_MGR_GetDataByField(
                msg_p->data.fld_data.field_id,
                &msg_p->data.fld_data.data);
            break;
        case DOS_MGR_IPC_GET_RUNNING_DATA_BY_FIELD:
            msgbuf_p->msg_size = DOS_MGR_GET_MSG_SIZE(fld_data);
            msg_p->type.result = DOS_MGR_GetRunningDataByField(
                msg_p->data.fld_data.field_id,
                &msg_p->data.fld_data.data);
            break;
        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.result = DOS_TYPE_E_INVALID;
            msgbuf_p->msg_size = DOS_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_SetDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set data by field
 * INPUT  : field_id -- DOS_TYPE_FieldId_T
 *          data_p   -- pointer to buffer that contains value to set
 *                      use can also use DOS_TYPE_FieldDataBuf_T as data buffer
 *                      specifies NULL to set to default value
 * OUTPUT : None.
 * RETURN : DOS_TYPE_Error_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
DOS_TYPE_Error_T DOS_MGR_SetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p)
{
    DOS_TYPE_Error_T ret = DOS_TYPE_E_OK;
    DOS_TYPE_FieldDataBuf_T tmp_data, om_data;

    DOS_USE_CSC(DOS_TYPE_E_NOT_READY);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DOS_TYPE_E_NOT_READY;
        goto exit;
    }

    if (data_p == NULL)
    {
        if (!DOS_MGR_GetDefaultDataByField(field_id, &tmp_data))
        {
            ret = DOS_TYPE_E_UNKNOWN;
            goto exit;
        }
        data_p = &tmp_data;
    }

    if (!DOS_MGR_IsFieldSupported(field_id))
    {
        ret = DOS_TYPE_E_NOT_SUPPORT;
        goto exit;
    }

    if (!DOS_MGR_CheckDataValidity(field_id, data_p))
    {
        ret = DOS_TYPE_E_INVALID;
        goto exit;
    }

    if (!DOS_OM_GetDataByField(field_id, &om_data))
    {
        ret = DOS_TYPE_E_UNKNOWN;
        goto exit;
    }

    if (DOS_MGR_CompareData(field_id, data_p, &om_data))
    {
        goto exit;
    }

    if (!DOS_MGR_SetDataByField_Internal(field_id, data_p))
    {
        ret = DOS_TYPE_E_UNKNOWN;
        goto exit;
    }

exit:
    DOS_RELEASE_CSC();
    DOS_MGR_DBG_TRACE_MSG("field_id:%d ret:%d", field_id, ret);
    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_GetRunningDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get data by field
 * INPUT  : field_id - DOS_TYPE_FieldId_T
 *          data_p   -- pointer to buffer that contains value of specified field
 *                      use can also use DOS_TYPE_FieldDataBuf_T as data buffer
 * RETURN : DOS_TYPE_Error_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
DOS_TYPE_Error_T DOS_MGR_GetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p)
{
    DOS_TYPE_Error_T ret = DOS_TYPE_E_OK;
    DOS_TYPE_FieldDataPtr_T data;

    data.p = data_p;

    DOS_USE_CSC(DOS_TYPE_E_NOT_READY);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DOS_TYPE_E_NOT_READY;
        goto exit;
    }

    if (data_p == NULL)
    {
        ret = DOS_TYPE_E_INVALID;
        goto exit;
    }

    if (!DOS_MGR_IsFieldSupported(field_id))
    {
        ret = DOS_TYPE_E_NOT_SUPPORT;
        goto exit;
    }

    if (!DOS_OM_GetDataByField(field_id, data_p))
    {
        ret = DOS_TYPE_E_UNKNOWN;
        goto exit;
    }

exit:
    DOS_RELEASE_CSC();
    DOS_MGR_DBG_TRACE_MSG("field_id:%d ret:%d", field_id, ret);
    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_GetRunningDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get data by field
 * INPUT  : field_id - DOS_TYPE_FieldId_T
 *          data_p   -- pointer to buffer that contains value of specified field
 *                      use can also use DOS_TYPE_FieldDataBuf_T as data buffer
 * RETURN : SYS_TYPE_Get_Running_Cfg_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T DOS_MGR_GetRunningDataByField(DOS_TYPE_FieldId_T field_id, void *data_p)
{
    SYS_TYPE_Get_Running_Cfg_T ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    DOS_TYPE_FieldDataBuf_T dflt_data;

    BOOL_T is_default = TRUE;

    DOS_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        goto exit;
    }

    if (data_p == NULL)
    {
        goto exit;
    }

    if (!DOS_MGR_IsFieldSupported(field_id))
    {
        goto exit;
    }

    if (!DOS_MGR_GetDefaultDataByField(field_id, &dflt_data) ||
        !DOS_OM_GetDataByField(field_id, data_p))
    {
        goto exit;
    }

    is_default = DOS_MGR_CompareData(field_id, data_p, &dflt_data);

    ret = is_default ? SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

exit:
    DOS_RELEASE_CSC();
    DOS_MGR_DBG_TRACE_MSG("field_id:%d ret:%d", field_id, ret);
    return ret;
}

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_IsFieldSupported
 *-----------------------------------------------------------------------------
 * PURPOSE: To check if the type of DOS protection is supported.
 * INPUT  : field_id -- DOS_TYPE_FieldId_T
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T DOS_MGR_IsFieldSupported(DOS_TYPE_FieldId_T field_id)
{
    BOOL_T ret;

    switch (field_id)
    {
        #if (SYS_CPNT_DOS_ECHO_CHARGEN == TRUE)
        case DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_RATELIMIT:
        #endif

        #if (SYS_CPNT_DOS_LAND == TRUE)
        case DOS_TYPE_FLD_SYSTEM_LAND_STATUS:
        #endif

        #if (SYS_CPNT_DOS_SMURF == TRUE)
        case DOS_TYPE_FLD_SYSTEM_SMURF_STATUS:
        #endif

        #if (SYS_CPNT_DOS_TCP_FLOODING == TRUE)
        case DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_RATELIMIT:
        #endif

        #if (SYS_CPNT_DOS_TCP_NULL_SCAN == TRUE)
        case DOS_TYPE_FLD_SYSTEM_TCP_NULL_SCAN_STATUS:
        #endif

        #if (SYS_CPNT_DOS_TCP_SCAN == TRUE)
        case DOS_TYPE_FLD_SYSTEM_TCP_SCAN_STATUS:
        #endif

        #if (SYS_CPNT_DOS_TCP_SYN_FIN_SCAN == TRUE)
        case DOS_TYPE_FLD_SYSTEM_TCP_SYN_FIN_SCAN_STATUS:
        #endif

        #if (SYS_CPNT_DOS_TCP_UDP_PORT_ZERO == TRUE)
        case DOS_TYPE_FLD_SYSTEM_TCP_UDP_PORT_ZERO_STATUS:
        #endif

        #if (SYS_CPNT_DOS_TCP_XMAS_SCAN == TRUE)
        case DOS_TYPE_FLD_SYSTEM_TCP_XMAS_SCAN_STATUS:
        #endif

        #if (SYS_CPNT_DOS_UDP_FLOODING == TRUE)
        case DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_STATUS:
        case DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_RATELIMIT:
        #endif

        #if (SYS_CPNT_DOS_WIN_NUKE == TRUE)
        case DOS_TYPE_FLD_SYSTEM_WIN_NUKE_STATUS:
        case DOS_TYPE_FLD_SYSTEM_WIN_NUKE_RATELIMIT:
        #endif

            ret = TRUE;
            break;

        default:
            ret = FALSE;
    } /* end of switch (field_id) */

    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_CheckDataValidity
 *-----------------------------------------------------------------------------
 * PURPOSE: To check data validity.
 * INPUT  : field_id -- DOS_TYPE_FieldId_T
 *          data_p   -- value to set
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T DOS_MGR_CheckDataValidity(DOS_TYPE_FieldId_T field_id, void *data_p)
{
    DOS_TYPE_FieldDataPtr_T data;

    BOOL_T ret = TRUE;

    data.p = data_p;

    switch (field_id)
    {
        case DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_LAND_STATUS:
        case DOS_TYPE_FLD_SYSTEM_SMURF_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_NULL_SCAN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_SCAN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_SYN_FIN_SCAN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_UDP_PORT_ZERO_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_XMAS_SCAN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_STATUS:
        case DOS_TYPE_FLD_SYSTEM_WIN_NUKE_STATUS:
            ret =
                *data.u32_p == DOS_TYPE_STATUS_ENABLED ||
                *data.u32_p == DOS_TYPE_STATUS_DISABLED;
            break;

        case DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_RATELIMIT:
        case DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_RATELIMIT:
        case DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_RATELIMIT:
        case DOS_TYPE_FLD_SYSTEM_WIN_NUKE_RATELIMIT:
            ret =
                *data.u32_p >= SYS_ADPT_DOS_MIN_RATELIMIT &&
                *data.u32_p <= SYS_ADPT_DOS_MAX_RATELIMIT;
            break;

        default:
            ;
    } /* end of switch (field_id) */

    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_CompareData
 *-----------------------------------------------------------------------------
 * PURPOSE: To check data validity.
 * INPUT  : field_id -- DOS_TYPE_FieldId_T
 *          data_p   -- value to set
 * OUTPUT : None.
 * RETURN : TRUE if two value are equal
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T DOS_MGR_CompareData(DOS_TYPE_FieldId_T field_id, void *data1_p, void *data2_p)
{
    DOS_TYPE_FieldDataPtr_T data1, data2;
    int diff = 0;

    data1.p = data1_p;
    data2.p = data2_p;

    switch (DOS_OM_GetDataTypeByField(field_id))
    {
        case DOS_TYPE_FTYPE_UI32:
            diff = *data1.u32_p - *data2.u32_p;
            break;

        default:
            ;
    }

    return !diff;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_GetDefaultDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get default data by field
 * INPUT  : field_id - DOS_TYPE_FieldId_T
 * OUTPUT : data_p   - value of the field
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T DOS_MGR_GetDefaultDataByField(DOS_TYPE_FieldId_T field_id, void *data_p)
{
    DOS_TYPE_FieldDataPtr_T data;

    BOOL_T ret = TRUE;

    if (data_p == NULL)
    {
        return FALSE;
    }

    data.p = data_p;

    switch (field_id)
    {
        case DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_STATUS:
            *data.u32_p = SYS_DFLT_DOS_ECHO_CHARGEN_STATUS;
            break;
        case DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_RATELIMIT:
            *data.u32_p = SYS_DFLT_DOS_ECHO_CHARGEN_RATE_LIMIT;
            break;
        case DOS_TYPE_FLD_SYSTEM_LAND_STATUS:
            *data.u32_p = SYS_DFLT_DOS_LAND_STATUS;
            break;
        case DOS_TYPE_FLD_SYSTEM_SMURF_STATUS:
            *data.u32_p = SYS_DFLT_DOS_SMURF_STATUS;
            break;
        case DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_STATUS:
            *data.u32_p = SYS_DFLT_DOS_TCP_FLOODING_STATUS;
            break;
        case DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_RATELIMIT:
            *data.u32_p = SYS_DFLT_DOS_TCP_FLOODING_RATE_LIMIT;
            break;
        case DOS_TYPE_FLD_SYSTEM_TCP_NULL_SCAN_STATUS:
            *data.u32_p = SYS_DFLT_DOS_TCP_NULL_SCAN_STATUS;
            break;
        case DOS_TYPE_FLD_SYSTEM_TCP_SCAN_STATUS:
            *data.u32_p = SYS_DFLT_DOS_TCP_SCAN_STATUS;
            break;
        case DOS_TYPE_FLD_SYSTEM_TCP_SYN_FIN_SCAN_STATUS:
            *data.u32_p = SYS_DFLT_DOS_TCP_SYN_FIN_SCAN_STATUS;
            break;
        case DOS_TYPE_FLD_SYSTEM_TCP_UDP_PORT_ZERO_STATUS:
            *data.u32_p = SYS_DFLT_DOS_TCP_UDP_PORT_ZERO_STATUS;
            break;
        case DOS_TYPE_FLD_SYSTEM_TCP_XMAS_SCAN_STATUS:
            *data.u32_p = SYS_DFLT_DOS_TCP_XMAS_SCAN_STATUS;
            break;
        case DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_STATUS:
            *data.u32_p = SYS_DFLT_DOS_UDP_FLOODING_STATUS;
            break;
        case DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_RATELIMIT:
            *data.u32_p = SYS_DFLT_DOS_UDP_FLOODING_RATE_LIMIT;
            break;
        case DOS_TYPE_FLD_SYSTEM_WIN_NUKE_STATUS:
            *data.u32_p = SYS_DFLT_DOS_WIN_NUKE_STATUS;
            break;
        case DOS_TYPE_FLD_SYSTEM_WIN_NUKE_RATELIMIT:
            *data.u32_p = SYS_DFLT_DOS_WIN_NUKE_RATE_LIMIT;
            break;
        default:
            ret = FALSE;
    } /* end of switch (field_id) */

    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_SetDataByField_Internal
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set data by field
 * INPUT  : field_id -- DOS_TYPE_FieldId_T
 *          data_p   -- value to set
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T DOS_MGR_SetDataByField_Internal(DOS_TYPE_FieldId_T field_id, void *data_p)
{
    enum {
        FILTER,
        RATE_LIMIT,
    };

    static struct {
        UI32_T method;
        UI32_T type;
    }
    field_config_info[] =
    {
        [DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_STATUS]       = { RATE_LIMIT, SWCTRL_DOS_RATELIMIT_ECHO_CHARGEN },
        [DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_RATELIMIT]    = { RATE_LIMIT, SWCTRL_DOS_RATELIMIT_ECHO_CHARGEN },
        [DOS_TYPE_FLD_SYSTEM_LAND_STATUS]               = { FILTER,     SWCTRL_DOS_FILTER_LAND },
        [DOS_TYPE_FLD_SYSTEM_SMURF_STATUS]              = { FILTER,     SWCTRL_DOS_FILTER_SMURF },
        [DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_STATUS]       = { RATE_LIMIT, SWCTRL_DOS_RATELIMIT_TCP_FLOODING },
        [DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_RATELIMIT]    = { RATE_LIMIT, SWCTRL_DOS_RATELIMIT_TCP_FLOODING },
        [DOS_TYPE_FLD_SYSTEM_TCP_NULL_SCAN_STATUS]      = { FILTER,     SWCTRL_DOS_FILTER_TCP_NULL_SCAN },
        [DOS_TYPE_FLD_SYSTEM_TCP_SCAN_STATUS]           = { FILTER,     SWCTRL_DOS_FILTER_TCP_SCAN },
        [DOS_TYPE_FLD_SYSTEM_TCP_SYN_FIN_SCAN_STATUS]   = { FILTER,     SWCTRL_DOS_FILTER_TCP_SYN_FIN_SCAN },
        [DOS_TYPE_FLD_SYSTEM_TCP_UDP_PORT_ZERO_STATUS]  = { FILTER,     SWCTRL_DOS_FILTER_TCP_UDP_PORT_ZERO },
        [DOS_TYPE_FLD_SYSTEM_TCP_XMAS_SCAN_STATUS]      = { FILTER,     SWCTRL_DOS_FILTER_TCP_XMAS_SCAN },
        [DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_STATUS]       = { RATE_LIMIT, SWCTRL_DOS_RATELIMIT_UDP_FLOODING },
        [DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_RATELIMIT]    = { RATE_LIMIT, SWCTRL_DOS_RATELIMIT_UDP_FLOODING },
        [DOS_TYPE_FLD_SYSTEM_WIN_NUKE_STATUS]           = { RATE_LIMIT, SWCTRL_DOS_RATELIMIT_WIN_NUKE },
        [DOS_TYPE_FLD_SYSTEM_WIN_NUKE_RATELIMIT]        = { RATE_LIMIT, SWCTRL_DOS_RATELIMIT_WIN_NUKE },
    };

    DOS_TYPE_FieldDataPtr_T data;

    BOOL_T ret = TRUE;
    UI32_T tmp, arg;

    /* prepare arguments to config driver
     */
    data.p = data_p;

    switch (field_id)
    {
        case DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_STATUS:
        case DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_STATUS:
        case DOS_TYPE_FLD_SYSTEM_WIN_NUKE_STATUS:
            ret = DOS_OM_GetDataByField(field_id + 1, &tmp); /* get rate */
            arg = *data.u32_p == DOS_TYPE_STATUS_ENABLED ? tmp : 0;
            break;

        case DOS_TYPE_FLD_SYSTEM_ECHO_CHARGEN_RATELIMIT:
        case DOS_TYPE_FLD_SYSTEM_TCP_FLOODING_RATELIMIT:
        case DOS_TYPE_FLD_SYSTEM_UDP_FLOODING_RATELIMIT:
        case DOS_TYPE_FLD_SYSTEM_WIN_NUKE_RATELIMIT:
            ret = DOS_OM_GetDataByField(field_id - 1, &tmp); /* get status */
            arg = tmp == DOS_TYPE_STATUS_ENABLED ? *data.u32_p : 0;
            break;

        case DOS_TYPE_FLD_SYSTEM_LAND_STATUS:
        case DOS_TYPE_FLD_SYSTEM_SMURF_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_NULL_SCAN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_SCAN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_SYN_FIN_SCAN_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_UDP_PORT_ZERO_STATUS:
        case DOS_TYPE_FLD_SYSTEM_TCP_XMAS_SCAN_STATUS:
            arg = *data.u32_p == DOS_TYPE_STATUS_ENABLED;
            break;

        default:
            ret = FALSE;
    } /* end of switch (field_id) */

    if (!ret)
    {
        goto exit;
    }

    /* config driver
     */
    switch (field_config_info[field_id].method)
    {
        case FILTER:
            ret = SWCTRL_PMGR_SetDosProtectionFilter(field_config_info[field_id].type, arg);
            break;

        case RATE_LIMIT:
            ret = SWCTRL_PMGR_SetDosProtectionRateLimit(field_config_info[field_id].type, arg);
            break;

        default:
            ret = FALSE;
    }

    if (!ret)
    {
        goto exit;
    }

    /* update OM
     */
    ret = DOS_OM_SetDataByField(field_id, data_p);

exit:
    DOS_MGR_DBG_TRACE_MSG("field_id:%d ret:%d", field_id, ret);
    return ret;
}


/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_ConfigSystem
 *-----------------------------------------------------------------------------
 * PURPOSE: Perform DOS protection config.
 * INPUT  : use_default -- set with default value
 * OUTPUT : NOne.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T DOS_MGR_ConfigSystem(BOOL_T use_default)
{
    DOS_TYPE_FieldId_T field_id;
    DOS_TYPE_FieldDataBuf_T data;
    BOOL_T ret = TRUE;

    for (field_id = 0; field_id < DOS_TYPE_FLD_NUM; field_id++)
    {
        BOOL_T tmp_ret = TRUE;

        if (!DOS_MGR_IsFieldSupported(field_id))
        {
            continue;
        }

        if (use_default)
        {
            tmp_ret = DOS_MGR_GetDefaultDataByField(field_id, &data);
        }
        else
        {
            tmp_ret = DOS_OM_GetDataByField(field_id, &data);
        }

        if (tmp_ret)
        {
            tmp_ret = DOS_MGR_SetDataByField_Internal(field_id, &data);
        }

        ret = ret && tmp_ret;
    } /* end of for (fld_id) */

    return ret;
}

#endif /* (SYS_CPNT_DOS == TRUE) */



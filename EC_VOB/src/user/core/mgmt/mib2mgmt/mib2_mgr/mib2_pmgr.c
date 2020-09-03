/* -------------------------------------------------------------------------
 * FILE NAME - MIB2_PMGR.C
 * -------------------------------------------------------------------------
 * Purpose: For other CSCS to ipc MGR
 *          Written by:     Eli
 *          Date:           06/13/2007
 *
 * Modification History:
 * -------------------------------------------------------------------------
 * Copyright(C)                              ACCTON Technology Corp., 1998
 * -------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sysfun.h"
#include "sys_module.h"
#include "sysfun.h"
#include "l_mm.h"
#include "sys_bld.h"
#include "mib2_mgr.h"

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : TACACS_PMGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for TACACS_PMGR.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void MIB2_PMGR_Init(void)
{
    /* Given that TACACS PMGR requests are handled in TACACSGROUP of L2_L4_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
        SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
    }
}
/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_PMGR_GetSysObjectID
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system authoritative identification
 *           of this device is successfully retrieved. Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_oid    - system decsription of this device.
 *           length     - length of system object id.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. This value is allocated within the SMI enterprises subtree (1.3.6.1.4.1).
 *           2. This field is read-only.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_PMGR_GetSysObjectID(UI32_T *sys_oid, UI32_T *length)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_MGR_IPCMsg_T *)0)->data.sysoid_length)
        +sizeof(((MIB2_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = MIB2_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p=(MIB2_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_MGR_IPCCMD_GETSYSOBJECTID;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    memcpy(sys_oid,msg_data_p->data.sysoid_length.sys_oid,
        sizeof(msg_data_p->data.sysoid_length.sys_oid));
    *length=msg_data_p->data.sysoid_length.length;

    return msg_data_p->type.result_bool;
} /* end of MIB2_PMGR_GetSysObjectID() */

/* ---------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_PMGR_SetSysContact
 * ---------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system contact info of this device
 *           is successfully configured. Otherwise, false is returned.
 * INPUT   : sys_contact        - system contact info of this device
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Contact is a Octet string.
 *              The max length of System Contact infomation could be up to 255 bytes.
 *           2. The Octet string is NOT a Zero-ASCII string as defined by C.
 *              The Octet string is an ASCII string represented as a byte-array format
 *              with string length.
 *           3. The max length of System Contact supported by this device will be
 *              OEM customer dependent and shall be defined in the OEM_ADPT.H.
 * ---------------------------------------------------------------------------------------*/
BOOL_T  MIB2_PMGR_SetSysContact(UI8_T *sys_contact)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_MGR_IPCMsg_T *)0)->data.contact)
        +sizeof(((MIB2_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = msg_buf_size;
    msg_data_p=(MIB2_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_MGR_IPCCMD_SETSYSCONTACT;
    /*respond size
     */
    resp_data_size=MIB2_MGR_MSGBUF_TYPE_SIZE ;
    /*assign input parameter
     */
    memcpy(msg_data_p->data.contact,sys_contact,
        sizeof(msg_data_p->data.contact));
    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }
    /*assign output parameter
     */

    return msg_data_p->type.result_bool;
} /* end of MIB2_PMGR_SetSysContact() */
/*
  EPR_ID:ES3628BT-FLF-ZZ-00057
  Problem: CLI: The behavior of hostname command is NOT correct.
  Root Cause: use error command line.
  Solution: 1. use "hostname" command is modification of "system name".
            2. Add "prompt" CLI command.
*/
#if 0 /* shumin.wang delete command "snmp-server sysname" */
/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_PMGR_SetSysName
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system name is successfully configured.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_name - An administratively assigned name.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 *           2. This is the node's qualify domain name.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_PMGR_SetSysName(UI8_T *sys_name)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_MGR_IPCMsg_T *)0)->data.sys_name)
        +sizeof(((MIB2_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(MIB2_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_MGR_IPCCMD_SETSYSNAME;

    /*respond size
     */
    resp_data_size=MIB2_MGR_MSGBUF_TYPE_SIZE ;

    /*assign input parameter
     */
    memcpy(msg_data_p->data.sys_name,sys_name,
        sizeof(msg_data_p->data.sys_name));

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */

    return msg_data_p->type.result_bool;
} /* end of MIB2_PMGR_SetSysName() */
#endif /* shumin.wang delete command "snmp-server sysname" */
/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_PMGR_SetHostName
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the host name is successfully configured.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : host_name - An administratively assigned name.
 * RETURN  : TRUE/FALSE
 * NOTE    : Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_PMGR_SetHostName(UI8_T *host_name)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_MGR_IPCMsg_T *)0)->data.sys_name)
        +sizeof(((MIB2_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(MIB2_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_MGR_IPCCMD_SETHOSTNAME;

    /*respond size
     */
    resp_data_size=MIB2_MGR_MSGBUF_TYPE_SIZE ;

    /*assign input parameter
     */
    memcpy(msg_data_p->data.sys_name,host_name,
        sizeof(msg_data_p->data.sys_name));

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */

    return msg_data_p->type.result_bool;
} /* end of MIB2_PMGR_SetHostName() */

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_PMGR_SetSysLocation
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system location can be successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_location   - Physical location of the node.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_PMGR_SetSysLocation(UI8_T *sys_location)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_MGR_IPCMsg_T *)0)->data.location)
        +sizeof(((MIB2_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(MIB2_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_MGR_IPCCMD_SETSYSLOCATION;

    /*respond size
     */
    resp_data_size=MIB2_MGR_MSGBUF_TYPE_SIZE ;

    /*assign input parameter
     */
    memcpy(msg_data_p->data.location,sys_location,
        sizeof(msg_data_p->data.location));

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */

    return msg_data_p->type.result_bool;
} /* end of MIB2_PMGR_SetSysLocation() */

/* For the Interfaces group related manage function will be provided by
 * another separate package, IF_MGR.C
 */

/* The management info for IP, ICMP, TCP, UDP, and SNMP groups will depend
 * on the TCP/IP ptotocol stack. Phase-II TCP/IP and Routing protocol stack
 * is used in the new software plaform to support all L2/L3/L4 product family.
 */


/* RUNNING CONFIG API
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MIB2_PMGR_GetRunningSysName
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default system name is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: system_name      - system name in byte (ASCII char) array
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *        4. Based on RFC1213, system name is defined as Octet string
 *           (ASCII char array) with length up to 255 bytes.
 *           Octet string is not an ASCII Zero string as defined in C.
 * ---------------------------------------------------------------------*/
UI32_T  MIB2_PMGR_GetRunningSysName(UI8_T *sys_name)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_MGR_IPCMsg_T *)0)->data.sys_name)
        +sizeof(((MIB2_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = MIB2_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p=(MIB2_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_MGR_IPCCMD_GETRUNNINGSYSNAME;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output parameter
     */
    /* shumin.wang modified for ES4827G-FLF-ZZ-00119 */
    memcpy(sys_name,msg_data_p->data.sys_name,
        sizeof(msg_data_p->data.sys_name));

    return msg_data_p->type.result_ui32;
} /* MIB2_PMGR_GetRunningSysName() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MIB2_PMGR_GetRunningSysContact
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default system contact info is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: system_contact   - system contact info in byte (ASCII char) array
 *         length           - length of system contact info
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system contact info.
 *        3. Caller has to prepare buffer for storing system contact info
 *        4. Based on RFC1213, system contact info is defined as Octet string
 *           (ASCII char array) with length up to 255 bytes.
 *           Octet string is not an ASCII Zero string as defined in C.
 * ---------------------------------------------------------------------*/
UI32_T  MIB2_PMGR_GetRunningSysContact(UI8_T *sys_contact)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_MGR_IPCMsg_T *)0)->data.contact)
        +sizeof(((MIB2_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = MIB2_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p=(MIB2_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_MGR_IPCCMD_GETRUNNINGSYSCONTACT;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output parameter
     */
    memcpy(sys_contact,msg_data_p->data.contact,
        sizeof(msg_data_p->data.contact));

    return msg_data_p->type.result_ui32;
} /* end of MIB2_PMGR_GetRunningSysContact() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MIB2_PMGR_GetRunningSysLocation
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default system location info is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: system_contact   - system location info in byte (ASCII char) array
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system location info.
 *        3. Caller has to prepare buffer for storing system location info
 *        4. Based on RFC1213, system location info is defined as Octet string
 *           (ASCII char array) with length up to 255 bytes.
 *           Octet string is not an ASCII Zero string as defined in C.
 * ---------------------------------------------------------------------*/
UI32_T  MIB2_PMGR_GetRunningSysLocation(UI8_T *sys_location)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_MGR_IPCMsg_T *)0)->data.location)
        +sizeof(((MIB2_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = MIB2_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p=(MIB2_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_MGR_IPCCMD_GETRUNNINGSYSLOCATION;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output parameter
     */
    memcpy(sys_location,msg_data_p->data.location,
        sizeof(msg_data_p->data.location));

    return msg_data_p->type.result_ui32;
} /* end of MIB2_PMGR_GetRunningSysLocation() */


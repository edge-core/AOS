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
#include "mib2_om.h"

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : MIB2_POM_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for MIB2_POM_Init.
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
void MIB2_POM_Init(void)
{
    /* Given that TACACS PMGR requests are handled in TACACSGROUP of L2_L4_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_SYS_MGMT_PROC_OM_IPCMSGQ_KEY,
        SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
    }
}
/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_POM_GetSysDescr
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system description of this device
 *           is successfully retrieved. Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_descrption     - system decsription of this device.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 *           2. The Octet string is NOT a Zero-ASCII string as defined by C.
 *              The Octet string is an ASCII string represented as a byte-array format
 *              with string length.
 *           3. The max length of System Decsription supported by this device will be
 *              OEM customer dependent and shall be defined in the OEM_ADPT.H.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_POM_GetSysDescr(UI8_T *sys_description)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_OM_IPCMsg_T *)0)->data.description)
        +sizeof(((MIB2_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;
	
    memset(space_msg,0,sizeof(UI8_T)*SYSFUN_SIZE_OF_MSG(msg_buf_size));
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = MIB2_OM_MSGBUF_TYPE_SIZE;

    msg_data_p=(MIB2_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_OM_IPCCMD_GETSYSDESCR;

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

    strncpy((char *)sys_description, (char *)msg_data_p->data.description, MAXSIZE_sysDescr);
     sys_description[MAXSIZE_sysDescr] = '\0';

    return msg_data_p->type.result_bool;
}/* End of  MIB2_POM_GetSysDescr() */

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_POM_GetSysContact
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the contact information is successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_contact    - identification and contact info of the contact person.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_POM_GetSysContact(UI8_T *sys_contact)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_OM_IPCMsg_T *)0)->data.contact)
        +sizeof(((MIB2_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;
	
    memset(space_msg,0,sizeof(UI8_T)*SYSFUN_SIZE_OF_MSG(msg_buf_size));
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = MIB2_OM_MSGBUF_TYPE_SIZE;

    msg_data_p=(MIB2_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_OM_IPCCMD_GETSYSCONTACT;

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
    memcpy(sys_contact,msg_data_p->data.contact,
        sizeof(msg_data_p->data.contact));

    return msg_data_p->type.result_bool;
} /* end of MIB2_POM_GetSysContact() */

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_POM_GetSysName
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system name is successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_name       - An administratively assigned name.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 *              The max length of System Decsription could be up to 255 bytes.
 *           2. This is the node's qualify domain name.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_POM_GetSysName(UI8_T *sys_name)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_OM_IPCMsg_T *)0)->data.name)
        +sizeof(((MIB2_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    memset(space_msg,0,SYSFUN_SIZE_OF_MSG(msg_buf_size));
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = MIB2_OM_MSGBUF_TYPE_SIZE;

    msg_data_p=(MIB2_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_OM_IPCCMD_GETSYSNAME;

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
   strncpy((char *)sys_name, (char *)msg_data_p->data.name, MAXSIZE_sysName);
   sys_name[MAXSIZE_sysName] = '\0';

    return msg_data_p->type.result_bool;
} /* end of MIB2_POM_GetSysName() */

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_POM_GetSysLocation
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system location can be successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_location   - Physical location of the node.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_POM_GetSysLocation(UI8_T *sys_location)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_OM_IPCMsg_T *)0)->data.location)
        +sizeof(((MIB2_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    memset(space_msg,0,SYSFUN_SIZE_OF_MSG(msg_buf_size));
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = MIB2_OM_MSGBUF_TYPE_SIZE;

    msg_data_p=(MIB2_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_OM_IPCCMD_GETSYSLOCATION;

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
    memcpy(sys_location,msg_data_p->data.location,
        sizeof(msg_data_p->data.location));

    return msg_data_p->type.result_bool;
} /* end of  MIB2_POM_GetSysLocation() */

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_POM_GetSysServices
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the type of service provided by the system can be
 *           successfully retrieved.  Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_services   - indicates the set of services current system provides.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. sys_services is the sum of set of services provided by the system.
 *           2. This value is interpreted as a 7-bit code such that each bit of the code
 *              correspondes to a layer in the TCP/IP or OSI architecture whith the LSB
 *              corresponds to layer 1.
 *           3. The max value is 127.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_POM_GetSysServices(UI8_T *sys_services)
{
    const UI32_T msg_buf_size=(sizeof(((MIB2_OM_IPCMsg_T *)0)->data.service)
        +sizeof(((MIB2_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    MIB2_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    memset(space_msg,0,SYSFUN_SIZE_OF_MSG(msg_buf_size));
    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_MIB2MGMT;
    /*request size
     */
    msg_p->msg_size = MIB2_OM_MSGBUF_TYPE_SIZE;

    msg_data_p=(MIB2_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MIB2_OM_IPCCMD_GETSYSSERVICES;

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

    /*assign input parameter
     */
    *sys_services=msg_data_p->data.service;

    return msg_data_p->type.result_bool;
} /* end of MIB2_POM_GetSysServices() */


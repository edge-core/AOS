/* -------------------------------------------------------------------------
 * FILE NAME - MIB2_OM.C
 * -------------------------------------------------------------------------
 * Purpose: This package provides the sevices to manage the RFC1213 MIB
 * Notes: 1. This package shall be a reusable package for all the L2/L3 switchs.
 *        2. For the Interfaces group related manage function will be provided by
 *           another separate package, IF_OM.C
 *           The Interfaces group defined by RFC1213 MIB has been obsoleted by
 *           the new RFC2863.
 *        3. The the Address Translation group (at group) will be obsoleted in the
 *           and is not be supported by all the L2/L3 switchs.
 *        4. The EGP Neighbor group (egp group) will be supported only when the
 *           L3 switches supports EGP protocol.
 *        5. The management info for IP, ICMP, TCP, UDP, and SNMP groups will depend
 *           on the TCP/IP ptotocol stack. In Accton, pSOS PNA+ protocol stack is used
 *           for all Foxfire L2 switch family. Phase-II TCP/IP and Routing protocol
 *           stack is used in the new software plaform to support all L2/L3/L4 product
 *           family start from 2002.
 *
 *        6. The write operation for ifStackTable is not supported in this package.
 *
 *
 *
 *          Written by:     Amy
 *          Date:           10/29/2001
 *
 * Modification History:
 *   By              Date     Ver.   Modification Description
 *   --------------- -------- -----  ---------------------------------------
 *    Amytu         9-25-2002         EnterTransitionMode for stacking
 * -------------------------------------------------------------------------
 * Copyright(C)                              ACCTON Technology Corp., 1998
 * -------------------------------------------------------------------------
 */



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include <stdlib.h>
#include "sys_dflt.h"
#include <stdio.h>
#include "sysfun.h"
#include "mib2_mgr.h"
#include <string.h>
#include "leaf_1213.h"
#include "leaf_sys.h"
#include "sys_bld.h"
#include "mib2_om.h"

/* NAMING CONSTANT DECLARARTIONS
 */

#define  MIB2_OM_EnterCriticalSection(mib2_om_sem_id)  mib2_om_orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(mib2_om_sem_id)
#define  MIB2_OM_LeaveCriticalSection(mib2_om_sem_id)  SYSFUN_OM_LEAVE_CRITICAL_SECTION(mib2_om_sem_id,mib2_om_orig_priority)

#define MIB2_OM_USE_CSC(a)
#define MIB2_OM_RELEASE_CSC()

//#define  MIB2_OM_SIZE_OBJECT_ID    sizeof(SYS_ADPT_SYS_OID_STR)
/*
#if (SYS_CPNT_LLDP == TRUE)
#define MIB2_OM_NotifySysNameChanged() LLDP_POM_NotifySysNameChanged()
#else
#define MIB2_OM_NotifySysNameChanged()
#endif
*/
/* TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATION
 */


/* LOCAL VARIABLE DECLARATION
 */


MIB2_MGR_SYSTEM_T   mib2_om_system_info;

static UI32_T       mib2_om_sem_id;
static UI32_T       mib2_om_orig_priority;
/* Allen Cheng: Deleted
static UI32_T       mib2_om_operation_mode;
*/

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_OM_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE  : Enable vlan operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void  MIB2_OM_InitiateSystemResources(void)
{
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_MIB2_OM, &mib2_om_sem_id) != SYSFUN_OK)
        return;
    memset(&mib2_om_system_info, 0, sizeof(MIB2_MGR_SYSTEM_T));

/* Allen Cheng: Deleted
    mib2_om_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
*/
    return;
} /* end of MIB2_OM_InitiateSystemResources() */


/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_GetSysDescr
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
BOOL_T  MIB2_OM_GetSysDescr(UI8_T *sys_description)
{

    MIB2_OM_EnterCriticalSection(mib2_om_sem_id);
    strcpy((char *)sys_description, (char *)mib2_om_system_info.description);
    MIB2_OM_LeaveCriticalSection(mib2_om_sem_id);
    return TRUE;

} /* End of  MIB2_OM_GetSysDescr() */


/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_GetSysContact
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the contact information is successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_contact    - identification and contact info of the contact person.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_OM_GetSysContact(UI8_T *sys_contact)
{
    if (sys_contact == NULL)
    {
        return FALSE;
    } /* End of if */

    MIB2_OM_EnterCriticalSection(mib2_om_sem_id);
    strcpy((char *)sys_contact, (char *)mib2_om_system_info.contact);
    MIB2_OM_LeaveCriticalSection(mib2_om_sem_id);
    return TRUE;
} /* end of MIB2_OM_GetSysContact() */

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_GetSysName
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
BOOL_T  MIB2_OM_GetSysName(UI8_T *sys_name)
{
    /* BODY */
    if (sys_name == NULL)
    {
        return FALSE;
    } /* End of if */

    MIB2_OM_EnterCriticalSection(mib2_om_sem_id);
    strcpy((char *)sys_name, (char *)mib2_om_system_info.name);
    MIB2_OM_LeaveCriticalSection(mib2_om_sem_id);
    return TRUE;

} /* end of MIB2_OM_GetSysName() */


/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_GetSysLocation
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system location can be successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_location   - Physical location of the node.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_OM_GetSysLocation(UI8_T *sys_location)
{
    /* BODY */
    if (sys_location == NULL)
    {
        return FALSE;
    } /* End of if */

    MIB2_OM_EnterCriticalSection(mib2_om_sem_id);
    strcpy((char *)sys_location, (char *)mib2_om_system_info.location);
    MIB2_OM_LeaveCriticalSection(mib2_om_sem_id);
    return TRUE;

} /* end of  MIB2_OM_GetSysLocation() */

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_GetSysServices
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
BOOL_T  MIB2_OM_GetSysServices(UI8_T *sys_services)
{
    if (sys_services == NULL)
    {
        return FALSE;
    } /* End of if */

    MIB2_OM_EnterCriticalSection(mib2_om_sem_id);
    *sys_services = mib2_om_system_info.service;
    MIB2_OM_LeaveCriticalSection(mib2_om_sem_id);
    return TRUE;

} /* end of MIB2_OM_GetSysServices() */


/* ---------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_InitSysInfo
 * ---------------------------------------------------------------------------------------
 * FUNCTION: set system info to 0
 * INPUT   :   
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 
 * ---------------------------------------------------------------------------------------*/
BOOL_T  MIB2_OM_InitSysInfo(void)
{

    MIB2_OM_EnterCriticalSection(mib2_om_sem_id);
    memset(&mib2_om_system_info,0,sizeof(mib2_om_system_info));
    MIB2_OM_LeaveCriticalSection(mib2_om_sem_id);
    return TRUE;

} /* end of MIB2_OM_SetSysContact() */

/* ---------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_SetSysDescr
 * ---------------------------------------------------------------------------------------
 * FUNCTION: set system description
 * INPUT   : description  
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 
 * ---------------------------------------------------------------------------------------*/
BOOL_T  MIB2_OM_SetSysDescr(UI8_T *description)
{
    if (description == NULL)
    {
        return FALSE;
    } /* End of if */

    MIB2_OM_EnterCriticalSection(mib2_om_sem_id);
    strcpy((char *)mib2_om_system_info.description, (char *)description);
    MIB2_OM_LeaveCriticalSection(mib2_om_sem_id);
    return TRUE;

} /* end of MIB2_OM_SetSysContact() */

/* ---------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_SetSysService
 * ---------------------------------------------------------------------------------------
 * FUNCTION: set system service
 * INPUT   : service        - system contact info of this device
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 
 * ---------------------------------------------------------------------------------------*/
BOOL_T  MIB2_OM_SetSysService(UI32_T service)
{
    MIB2_OM_EnterCriticalSection(mib2_om_sem_id);
    mib2_om_system_info.service=service;
    MIB2_OM_LeaveCriticalSection(mib2_om_sem_id);
    return TRUE;

} /* end of MIB2_OM_SetSysContact() */

/* ---------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_SetSysContact
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
BOOL_T  MIB2_OM_SetSysContact(UI8_T *sys_contact)
{
    if (sys_contact == NULL)
    {
        return FALSE;
    } /* End of if */

    MIB2_OM_EnterCriticalSection(mib2_om_sem_id);
    strcpy((char *)mib2_om_system_info.contact, (char *)sys_contact);
    MIB2_OM_LeaveCriticalSection(mib2_om_sem_id);
    return TRUE;

} /* end of MIB2_OM_SetSysContact() */


/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_SetSysName
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system name is successfully configured.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_name - An administratively assigned name.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 *           2. This is the node's qualify domain name.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_OM_SetSysName(UI8_T *sys_name)
{
    if (sys_name == NULL)
    {

        return FALSE;
    } /* End of if */
    MIB2_OM_EnterCriticalSection(mib2_om_sem_id);
    strcpy((char *)mib2_om_system_info.name, (char *)sys_name);
    MIB2_OM_LeaveCriticalSection(mib2_om_sem_id);
    return TRUE;

} /* end of MIB2_OM_SetSysName() */



/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_SetSysLocation
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system location can be successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_location   - Physical location of the node.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_OM_SetSysLocation(UI8_T *sys_location)
{
    if (sys_location == NULL)
    {
        return FALSE;
    } /* End of if */
    MIB2_OM_EnterCriticalSection(mib2_om_sem_id);
    strcpy((char *)mib2_om_system_info.location, (char *)sys_location);
    MIB2_OM_LeaveCriticalSection(mib2_om_sem_id);
    return TRUE;

} /* end of MIB2_OM_SetSysLocation() */


/* LOCAL SUBPROGRAM DEFINITION
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME :  MIB2_OM_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T   MIB2_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    MIB2_OM_IPCMsg_T *msg_data_p;
    UI32_T cmd;

    if(ipcmsg_p==NULL)
        return FALSE;

    msg_data_p = ( MIB2_OM_IPCMsg_T*)ipcmsg_p->msg_buf;
    cmd = msg_data_p->type.cmd;

    switch(cmd)
    {

        case MIB2_OM_IPCCMD_GETSYSDESCR:
            ipcmsg_p->msg_size= MIB2_OM_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.description);
            msg_data_p->type.result_bool= MIB2_OM_GetSysDescr(
                msg_data_p->data.description);
            break;


        case MIB2_OM_IPCCMD_GETSYSCONTACT:
            ipcmsg_p->msg_size= MIB2_OM_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.contact);
            msg_data_p->type.result_bool= MIB2_OM_GetSysContact(
                msg_data_p->data.contact);
            break;

        case MIB2_OM_IPCCMD_GETSYSNAME:
            ipcmsg_p->msg_size= MIB2_OM_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.name);
            msg_data_p->type.result_bool= MIB2_OM_GetSysName(
                msg_data_p->data.name);
            break;

        case MIB2_OM_IPCCMD_GETSYSLOCATION:
            ipcmsg_p->msg_size= MIB2_OM_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.location);
            msg_data_p->type.result_bool= MIB2_OM_GetSysLocation(
                msg_data_p->data.location);
            break;

        case MIB2_OM_IPCCMD_GETSYSSERVICES:
            ipcmsg_p->msg_size= MIB2_OM_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.service);
            msg_data_p->type.result_bool= MIB2_OM_GetSysServices(
                &msg_data_p->data.service);
            break;

        default:
            ipcmsg_p->msg_size= MIB2_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32=0;
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            return TRUE;
    }

    //exit:

        /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
         */
        if(cmd< MIB2_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    return TRUE;
}


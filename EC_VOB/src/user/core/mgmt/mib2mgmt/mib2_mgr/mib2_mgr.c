/* -------------------------------------------------------------------------
 * FILE NAME - MIB2_MGR.C
 * -------------------------------------------------------------------------
 * Purpose: This package provides the sevices to manage the RFC1213 MIB
 * Notes: 1. This package shall be a reusable package for all the L2/L3 switchs.
 *        2. For the Interfaces group related manage function will be provided by
 *           another separate package, IF_MGR.C
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
#include "mib2_om.h"
#include <string.h>
#include "leaf_1213.h"
#include "sys_mgr.h"//for single image
#include "leaf_sys.h"
#include "sys_bld.h"

#if defined(STRAWMAN) || defined(STRAWMANHD)
#include "stktplg_mgr.h"
#endif

#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_pmgr.h"
#endif

#define MIB2_MGR_USE_CSC(a)
#define MIB2_MGR_RELEASE_CSC()

//#define  MIB2_MGR_SIZE_OBJECT_ID    sizeof(SYS_ADPT_SYS_OID_STR)
/*
#if (SYS_CPNT_LLDP == TRUE)
#define MIB2_MGR_NotifySysNameChanged() LLDP_PMGR_NotifySysNameChanged()
#else
#define MIB2_MGR_NotifySysNameChanged()
#endif
*/
/* TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATION
 */
static BOOL_T MIB2_MGR_StringToObejctID(UI32_T *oid_P, UI8_T *text_p, UI32_T *length);

/* LOCAL VARIABLE DECLARATION
 */

/* Allen Cheng: Deleted
static UI32_T       mib2_mgr_operation_mode;
*/
SYSFUN_DECLARE_CSC
/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE  : Enable vlan operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void  MIB2_MGR_InitiateSystemResources(void)
{
    MIB2_OM_InitiateSystemResources();
/* Allen Cheng: Deleted
    mib2_mgr_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
*/
    return;
} /* end of MIB2_MGR_InitiateSystemResources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void MIB2_MGR_Create_InterCSC_Relation(void)
{
    return;
} /* end of MIB2_MGR_Create_InterCSC_Relation */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Enable mib2_mgr operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void  MIB2_MGR_EnterMasterMode(void)
{
    UI8_T descr_p[MAXSIZE_sysDescr+1];

#if 0    
    /* Amy Add 6-05-2002 for strawman
     */
#if defined(STRAWMAN) || defined(STRAWMANHD)

    UI8_T       strawman_sys_name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN+1];
    STKTPLG_MGR_Switch_Info_T       switch_info;

    /* BODY */

	strcpy(strawman_sys_name, "switch%d_%s");
#endif
#endif

    /* System Contact, Location and Name will be provided after provision complete.
       Values of these three field do not need to be initialized in EnterMasterMode().
     */
/* Allen Cheng: Deleted
    mib2_mgr_operation_mode = SYS_TYPE_STACKING_MASTER_MODE;
*/
    MIB2_OM_InitSysInfo();

    if (!SYS_MGR_GetSysDescr(SYS_VAL_LOCAL_UNIT_ID, descr_p))
    {
		printf("MIB2_MGR_EnterMasterMode:SYS_MGR_GetSysDescr return false\n");
    }

    MIB2_OM_SetSysDescr(descr_p);
    MIB2_OM_SetSysService(SYS_ADPT_SYS_SERVICES);
    MIB2_OM_SetSysContact((UI8_T *)SYS_DFLT_SYS_CONTACT);
    MIB2_OM_SetSysName((UI8_T *)SYS_DFLT_SYS_NAME);
    MIB2_OM_SetSysLocation((UI8_T *)SYS_DFLT_SYS_LOCATION);
    
#if 0
    /* Amy Add 6-05-2002 for strawman
     */
#if defined(STRAWMAN) || defined(STRAWMANHD)

    memset(&switch_info, 0 , sizeof(switch_info));
    switch_info.sw_unit_index = 1;
    if (STKTPLG_MGR_GetSwitchInfo(&switch_info) != TRUE)
    {
        return;
    }
    sprintf(mib2_mgr_system_info.name, strawman_sys_name, (UI16_T *) switch_info.sw_identifier, (UI8_T *) switch_info.sw_chassis_service_tag);

#endif
#endif

    SYSFUN_ENTER_MASTER_MODE();

    return;
} /* end of MIB2_MGR_EnterMasterMode()*/


/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set mib2_mgr into slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void MIB2_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
/* Allen Cheng: Deleted
    mib2_mgr_operation_mode = SYS_TYPE_STACKING_SLAVE_MODE;
*/
    return;
} /* end of MIB2_MGR_EnterSlaveMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set mib2_mgr into transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void MIB2_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
/* Allen Cheng: Deleted
    mib2_mgr_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
*/

    MIB2_OM_InitSysInfo();
    return;

} /* end of MIB2_MGR_EnterTransitionMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  MIB2_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();

} /* end of MIB2_MGR_SetTransitionMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_ProvisionComplete
 *--------------------------------------------------------------------------
 * PURPOSE  : mib2mgmt_init will call this function when provision completed
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void MIB2_MGR_ProvisionComplete(void)
{
	return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This functions returns the current operation mode of this component
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T  MIB2_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
} /* end of MIB2_MGR_GetOperationMode() */

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_MGR_GetSysObjectID
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
BOOL_T  MIB2_MGR_GetSysObjectID(UI32_T *sys_oid, UI32_T *length)
{
    /* BODY */
    UI8_T oid_str[SYS_ADPT_MAX_OID_STRING_LEN + 1];
    BOOL_T result=TRUE;
    MIB2_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MIB2_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

	if (SYS_MGR_GetSysObjectID(SYS_VAL_LOCAL_UNIT_ID, oid_str))
	{
	    if (!MIB2_MGR_StringToObejctID(sys_oid, oid_str, length))
                result=FALSE;
	}
	else
	{
                result=FALSE;
		printf("MIB2_MGR_GetSysObjectID:SYS_MGR_GetSysObjectID return false\n");
	}

    MIB2_MGR_RELEASE_CSC();
    return result;
} /* end of MIB2_MGR_GetSysObjectID() */

/* ---------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_MGR_SetSysContact
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
BOOL_T  MIB2_MGR_SetSysContact(UI8_T *sys_contact)
{
    /* BODY */

    MIB2_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MIB2_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    if (sys_contact == NULL)
    {
        MIB2_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */
    MIB2_OM_SetSysContact(sys_contact);
    
    MIB2_MGR_RELEASE_CSC();
    return TRUE;

} /* end of MIB2_MGR_SetSysContact() */


/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_MGR_SetSysName
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system name is successfully configured.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_name - An administratively assigned name.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 *           2. This is the node's qualify domain name.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_MGR_SetSysName(UI8_T *sys_name)
{
    /* BODY */

    MIB2_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MIB2_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    if (sys_name == NULL)
    {
        MIB2_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */
    
    //SYS_MGR_SetPromptString(sys_name);
    MIB2_OM_SetSysName(sys_name);
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_PMGR_NotifySysNameChanged();
#endif
    MIB2_MGR_RELEASE_CSC();
    return TRUE;

} /* end of MIB2_MGR_SetSysName() */



/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_MGR_SetSysLocation
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system location can be successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_location   - Physical location of the node.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_MGR_SetSysLocation(UI8_T *sys_location)
{
    /* BODY */

    MIB2_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MIB2_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */
    if (sys_location == NULL)
    {
        MIB2_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */
    
    MIB2_OM_SetSysLocation(sys_location);
    
    MIB2_MGR_RELEASE_CSC();
    return TRUE;

} /* end of MIB2_MGR_SetSysLocation() */



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
 * ROUTINE NAME  - MIB2_MGR_GetRunningSysName
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
UI32_T  MIB2_MGR_GetRunningSysName(UI8_T *sys_name)
{
    UI8_T name[MAXSIZE_sysName + 1];

     MIB2_OM_GetSysName(name);
     
#if defined(STRAWMAN) || defined(STRAWMANHD)
    UI32_T unit_id;
    STKTPLG_MGR_Switch_Info_T       switch_info;
    UI8_T SysNameBuf[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1];

    memset(&switch_info, 0, sizeof(switch_info));

    STKTPLG_MGR_GetMyUnitID(&unit_id);
    switch_info.sw_unit_index = unit_id;

    if (STKTPLG_MGR_GetSwitchInfo(&switch_info) != TRUE)
    {
        MIB2_MGR_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    sprintf(SysNameBuf, "switch%lu_%s", switch_info.sw_identifier, switch_info.sw_chassis_service_tag);

#endif
    /* BODY */

    MIB2_MGR_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MIB2_MGR_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    } /* End of if */

#if defined(STRAWMAN) || defined(STRAWMANHD)
    if (!strcmp(name, SysNameBuf))
#else
    if (name[0] == 0)
#endif
    {
        MIB2_MGR_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        strcpy((char *)sys_name, (char *)name);

    MIB2_MGR_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

} /* MIB2_MGR_GetRunningSysName() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MIB2_MGR_GetRunningSysContact
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
UI32_T  MIB2_MGR_GetRunningSysContact(UI8_T *sys_contact)
{
    /* BODY */
    UI8_T contact[MAXSIZE_sysContact + 1];
    
    MIB2_OM_GetSysContact(contact);

    MIB2_MGR_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MIB2_MGR_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    } /* End of if */


    if (contact[0] == 0)
    {
        MIB2_MGR_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    } /* End of if */
    else
        strcpy((char *)sys_contact, (char *)contact);

    MIB2_MGR_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

} /* end of MIB2_MGR_GetRunningSysContact() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MIB2_MGR_GetRunningSysLocation
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
UI32_T  MIB2_MGR_GetRunningSysLocation(UI8_T *location)
{
    /* BODY */
    UI8_T sys_location[MAXSIZE_sysLocation + 1];
    MIB2_OM_GetSysLocation(sys_location);

    MIB2_MGR_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MIB2_MGR_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    } /* End of if */

    if (sys_location[0] == 0)
    {
        MIB2_MGR_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    } /* End of if */
    else
        strcpy((char *)location, (char *)sys_location );

    MIB2_MGR_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

} /* end of MIB2_MGR_GetRunningSysLocation() */


/* LOCAL SUBPROGRAM DEFINITION
 */

static BOOL_T MIB2_MGR_StringToObejctID(UI32_T *oid_P, UI8_T *text_p, UI32_T *obj_length)
{
    UI8_T    *tp;
    UI32_T  no;

    if ( oid_P == NULL )
        return FALSE;

   for ( tp=text_p, no = 0; ;tp++)
   {
      if ( *tp == '.' || *tp==0)
      {
         oid_P[no++] =  atoi((char *)text_p);
         if ( *tp==0 ) break;
         text_p= tp+1;
      }
   }
   *obj_length = no;
    return TRUE;
} /* end of MIB2_MGR_StringToObejctID() */


/*------------------------------------------------------------------------------
 * ROUTINE NAME :  MIB2_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca mgr.
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
BOOL_T   MIB2_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    MIB2_MGR_IPCMsg_T *msg_data_p;
    UI32_T cmd;

    if(ipcmsg_p==NULL)
        return FALSE;

    msg_data_p = ( MIB2_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;
    cmd = msg_data_p->type.cmd;
    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        /*EPR:NULL
         *Problem:When slave enter transition mode,if msg_size have not 
         *        any value,will cause sender receive reply overflow.
         *Solution: use a default msg_size to reply the sender.
         *Fixed by:DanXie
         *Modify file:mib2_mgr.c
         *Approved by:Hardsun
         */
        ipcmsg_p->msg_size= MIB2_MGR_MSGBUF_TYPE_SIZE;
        msg_data_p->type.result_ui32 = FALSE;
        goto exit;
    }

    switch(cmd)
    {

        case MIB2_MGR_IPCCMD_GETSYSOBJECTID:
            ipcmsg_p->msg_size= MIB2_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.sysoid_length);
            msg_data_p->type.result_bool= MIB2_MGR_GetSysObjectID(
                msg_data_p->data.sysoid_length.sys_oid,
                &msg_data_p->data.sysoid_length.length);
            break;

        case MIB2_MGR_IPCCMD_SETSYSCONTACT:
            ipcmsg_p->msg_size= MIB2_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool= MIB2_MGR_SetSysContact(
                msg_data_p->data.contact);
            break;
/*
  EPR_ID:ES3628BT-FLF-ZZ-00057
  Problem: CLI: The behavior of hostname command is NOT correct.
  Root Cause: use error command line.
  Solution: 1. use "hostname" command is modification of "system name".
            2. Add "prompt" CLI command.
*/
#if 0
        case MIB2_MGR_IPCCMD_SETSYSNAME:
            ipcmsg_p->msg_size= MIB2_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool= MIB2_MGR_SetSysName(
                msg_data_p->data.sys_name);
            break;
#endif /* shumin.wang delete command "snmp-server sysname" */
        case MIB2_MGR_IPCCMD_SETHOSTNAME:
            ipcmsg_p->msg_size= MIB2_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool= MIB2_MGR_SetSysName(
                msg_data_p->data.sys_name);
            break;

        case MIB2_MGR_IPCCMD_SETSYSLOCATION:
            ipcmsg_p->msg_size= MIB2_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool= MIB2_MGR_SetSysLocation(
                msg_data_p->data.location);
            break;

        case MIB2_MGR_IPCCMD_GETRUNNINGSYSNAME:
            /* shumin.wang modified for ES4827G-FLF-ZZ-00119 */
            ipcmsg_p->msg_size= MIB2_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.sys_name);
            msg_data_p->type.result_ui32= MIB2_MGR_GetRunningSysName(
                msg_data_p->data.sys_name);
            break;

        case MIB2_MGR_IPCCMD_GETRUNNINGSYSCONTACT:
            ipcmsg_p->msg_size= MIB2_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.contact);
            msg_data_p->type.result_ui32= MIB2_MGR_GetRunningSysContact(
                msg_data_p->data.contact);
            break;

        case MIB2_MGR_IPCCMD_GETRUNNINGSYSLOCATION:
            ipcmsg_p->msg_size= MIB2_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.location);
            msg_data_p->type.result_ui32= MIB2_MGR_GetRunningSysLocation(
                msg_data_p->data.location);
            break;


        default:
            ipcmsg_p->msg_size= MIB2_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32=0;
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            return TRUE;
    }

    exit:

        /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
         */
        if(cmd< MIB2_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    return TRUE;
}


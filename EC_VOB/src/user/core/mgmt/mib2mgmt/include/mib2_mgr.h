/* ------------------------------------------------------------------------- 
 * FILE NAME - MIB2_MGR.H                                                     
 * ------------------------------------------------------------------------- 
 * Purpose: This package provides the sevices to manage the RFC1213 MIB                                   
 * Notes: 1. This package shall be a reusable package for all the L2/L3 switchs.
 *        2. For the Interfaces group related manage function will be provided by 
 *           another separate package, IF_MGR.C
 *           The Interfaces group defined by RFC1213 MIB has been obsoleted by
 *           the new RFC2863.
 *        3. The Address Translation group (at group) will be obsoleted in MIB-II
 *           and is not be supported by all the L2/L3 switchs.
 *        4. The EGP Neighbor group (egp group) will be supported only when the 
 *           L3 switches supports EGP protocol.
 *        5. The management info for IP, ICMP, TCP, UDP, and SNMP groups will depend
 *           on the TCP/IP ptotocol stack. In Accton, pSOS PNA+ protocol stack is used
 *           for all Foxfire L2 switch family. Phase-II TCP/IP and Routing protocol
 *           stack is used in the new software plaform to support all L2/L3/L4 product
 *           family start from 2002.
 *            
 *
 *
 *
 *                                                                           
 *          Written by:     Amy                                              
 *          Date:           10/29/2001                                            
 *                                                                           
 * Modification History:                                                     
 *   By              Date     Ver.   Modification Description                
 *   --------------- -------- -----  --------------------------------------- 
 *   Amytu          11-14-2001          Remove length field          
 *                  9-25-2002           EnterTransitionMode for stacking                                                            
 * ------------------------------------------------------------------------- 
 * Copyright(C)                              ACCTON Technology Corp., 1998   
 * -------------------------------------------------------------------------
 */

#ifndef MIB2_MGR_H
#define MIB2_MGR_H



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "leaf_1907.h"
/* definitions of command which will be used in ipc message
 */

#define MIB2_MGR_MAX_SYS_OID_COUNTER 32

enum
{
    MIB2_MGR_IPCCMD_GETSYSOBJECTID,
    MIB2_MGR_IPCCMD_SETSYSCONTACT,
/*
  EPR_ID:ES3628BT-FLF-ZZ-00057
  Problem: CLI: The behavior of hostname command is NOT correct.
  Root Cause: use error command line.
  Solution: 1. use "hostname" command is modification of "system name".
            2. Add "prompt" CLI command.
*/
    //MIB2_MGR_IPCCMD_SETSYSNAME,
    MIB2_MGR_IPCCMD_SETHOSTNAME,
    MIB2_MGR_IPCCMD_SETSYSLOCATION,
    MIB2_MGR_IPCCMD_GETRUNNINGSYSNAME,
    MIB2_MGR_IPCCMD_GETRUNNINGSYSCONTACT,
    MIB2_MGR_IPCCMD_GETRUNNINGSYSLOCATION,
    MIB2_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC
};

/* TYPE DECLARATIONS
 */
typedef struct MIB2_MGR_SYSTEM_S
{
    UI8_T   description[MAXSIZE_sysDescr+1];
    UI8_T	location[MAXSIZE_sysLocation + 1];
    UI8_T	name[MAXSIZE_sysName + 1];
    UI8_T	contact[MAXSIZE_sysContact + 1];
    UI8_T   service;

//    UI32_T   object_id[MIB2_MGR_SIZE_OBJECT_ID +1];
} MIB2_MGR_SYSTEM_T;

/*use to the definition of IPC message buffer*/
typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/ 
        UI32_T result_i32;  /*respond i32 return*/ 
    }type;
    
    union
    {

        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T i32_v;
        I16_T i16_v;
        UI8_T ip4_v[4];
        
        UI8_T   description[MAXSIZE_sysDescr+1];
        UI8_T	location[MAXSIZE_sysLocation + 1];
        UI8_T	name[MAXSIZE_sysName + 1];
		UI8_T   sys_name[MAXSIZE_sysName + 1];
        UI8_T	contact[MAXSIZE_sysContact + 1];
        UI8_T   service;

        struct{
            UI32_T sys_oid[MIB2_MGR_MAX_SYS_OID_COUNTER];
            UI32_T length;
        } sysoid_length;

    } data; /* contains the supplemntal data for the corresponding cmd */
}MIB2_MGR_IPCMsg_T;

#define MIB2_MGR_MSGBUF_TYPE_SIZE     sizeof(((MIB2_MGR_IPCMsg_T *)0)->type)

/* TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T   day;
    UI32_T   hour;
    UI32_T   minute;
    UI32_T   second;
    UI32_T   milisecond;

} MIB2_MGR_System_Time_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* ----------------------------------------------------------------------------------------- 
 * ROUTINE NAME - MIB2_MGR_InitiateSystemResources                                               
 * ----------------------------------------------------------------------------------------- 
 * FUNCTION: This function returns true if the system authoritative identification
 *           of this device is successfully retrieved. Otherwise, false is returned.              
 * INPUT   : None                           
 * OUTPUT  : sys_oid    - system decsription of this device.
 *           length     - the length of system decsription in byte.
 * RETURN  : TRUE/FALSE                   
 * NOTE    : 1. This value is allocated within the SMI enterprises subtree (1.3.6.1.4.1).
 *           2. This field is read-only.
 * -----------------------------------------------------------------------------------------*/
void  MIB2_MGR_InitiateSystemResources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set mib2_mgr into master mode.  
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set mib2_mgr into slave mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2_MGR_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set mib2_mgr into transition mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2_MGR_EnterTransitionMode(void);

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
void  MIB2_MGR_SetTransitionMode(void);

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
void MIB2_MGR_ProvisionComplete(void);

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
SYS_TYPE_Stacking_Mode_T  MIB2_MGR_GetOperationMode(void);

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
BOOL_T  MIB2_MGR_GetSysObjectID(UI32_T *sys_oid, UI32_T *length);

/* For the Interfaces group related manage function will be provided by 
 * another separate package, IF_MGR.C
 */
 
/* The management info for IP, ICMP, TCP, UDP, and SNMP groups will depend
 * on the TCP/IP ptotocol stack. Phase-II TCP/IP and Routing protocol stack 
 * is used in the new software plaform to support all L2/L3/L4 product family.
 */



/* Get Running Config API
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
UI32_T  MIB2_MGR_GetRunningSysName(UI8_T *sys_name);


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
UI32_T  MIB2_MGR_GetRunningSysLocation(UI8_T *sys_location);


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
UI32_T  MIB2_MGR_GetRunningSysContact(UI8_T *sys_contact);

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
BOOL_T   MIB2_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);
#endif /* End of MIB2_MGR.H */


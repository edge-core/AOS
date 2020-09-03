/* ------------------------------------------------------------------------- 
 * FILE NAME - MIB2_OM.H                                                     
 * ------------------------------------------------------------------------- 
 * Purpose: This package provides the sevices to manage the RFC1213 MIB                                   
 * Notes: 1. This package shall be a reusable package for all the L2/L3 switchs.
 *        2. For the Interfaces group related manage function will be provided by 
 *           another separate package, IF_OM.C
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

#ifndef MIB2_OM_H
#define MIB2_OM_H



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "leaf_1907.h"
#include "mib2_mgr.h"

enum
{
    MIB2_OM_IPCCMD_GETSYSDESCR,
    MIB2_OM_IPCCMD_GETSYSCONTACT,
    MIB2_OM_IPCCMD_GETSYSNAME,
    MIB2_OM_IPCCMD_GETSYSLOCATION,
    MIB2_OM_IPCCMD_GETSYSSERVICES,
    MIB2_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC
};

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
        UI8_T	contact[MAXSIZE_sysContact + 1];
        UI8_T   service;

        struct{
            UI32_T sys_oid[MIB2_MGR_MAX_SYS_OID_COUNTER];
            UI32_T length;
        } sysoid_length;

    } data; /* contains the supplemntal data for the corresponding cmd */
}MIB2_OM_IPCMsg_T;

#define MIB2_OM_MSGBUF_TYPE_SIZE     sizeof(((MIB2_OM_IPCMsg_T *)0)->type)

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_OM_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE  : Enable vlan operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void  MIB2_OM_InitiateSystemResources(void);

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
BOOL_T  MIB2_OM_GetSysDescr(UI8_T *sys_description);

/* ---------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_SetSysDescr
 * ---------------------------------------------------------------------------------------
 * FUNCTION: set system description
 * INPUT   : description  
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 
 * ---------------------------------------------------------------------------------------*/
BOOL_T  MIB2_OM_SetSysDescr(UI8_T *description);

/* ---------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_SetSysService
 * ---------------------------------------------------------------------------------------
 * FUNCTION: set system service
 * INPUT   : service        - system contact info of this device
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 
 * ---------------------------------------------------------------------------------------*/
BOOL_T  MIB2_OM_SetSysService(UI32_T service);

/* ---------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_InitSysInfo
 * ---------------------------------------------------------------------------------------
 * FUNCTION: set system info to 0
 * INPUT   :   
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 
 * ---------------------------------------------------------------------------------------*/
BOOL_T  MIB2_OM_InitSysInfo(void);

/* ---------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_OM_SetSysUpTime
 * ---------------------------------------------------------------------------------------
 * FUNCTION: set system time
 * INPUT   : description  
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 
 * ---------------------------------------------------------------------------------------*/
BOOL_T  MIB2_OM_SetSysUpTime(UI32_T sys_up_time);

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
BOOL_T  MIB2_OM_GetSysContact(UI8_T *sys_contact);

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
BOOL_T  MIB2_OM_GetSysName(UI8_T *sys_name);

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
BOOL_T  MIB2_OM_GetSysLocation(UI8_T *sys_location);


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
BOOL_T  MIB2_OM_SetSysContact(UI8_T *sys_contact);


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
BOOL_T  MIB2_OM_SetSysName(UI8_T *sys_name);


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
BOOL_T  MIB2_OM_SetSysLocation(UI8_T *sys_location);


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
BOOL_T   MIB2_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/* TYPE DECLARATIONS
 */
#endif /* End of MIB2_OM.H */


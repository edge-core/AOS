/* MODULE NAME:  udphelper_mgr.h
 * PURPOSE:
 *     This module provides APIs for UDPHELPER CSC to use.
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    03/31/2009 - Lin.Li, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */


#ifndef _UDPHELPER_MGR_H_
#define _UDPHELPER_MGR_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "udphelper_om.h"
#include "udphelper_type.h"


/* NAME CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */
#define UDPHELPER_MGR_MSGBUF_TYPE_SIZE sizeof(union UDPHELPER_MGR_IPCMsg_Type_U)
#define UDPHELPER_MGR_GET_MSG_SIZE(field_name)                       \
                (UDPHELPER_MGR_MSGBUF_TYPE_SIZE +                        \
                sizeof(((UDPHELPER_MGR_IPCMsg_T*)0)->data.field_name))
    
    
    

/* DATA TYPE DECLARATIONS
 */


/* structure for the request/response ipc message in csc pmgr and mgr
 */
 
typedef struct UDPHELPER_MGR_IPCMsg_S
{
    union UDPHELPER_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/ 
        UI32_T result_i32;  /*respond i32 return*/
        SYS_TYPE_Get_Running_Cfg_T result_running_cfg; /* For running config API */
    } type;

    union
    {
        BOOL_T  bool_v;
        UI8_T   ui8_v;
        I8_T    i8_v;
        UI32_T  ui32_v;
        UI16_T  ui16_v;
        I32_T   i32_v;
        I16_T   i16_v;
        UI8_T   ip4_v[SYS_ADPT_IPV4_ADDR_LEN];
        int     int_v;
        
        struct
        {    
            UI32_T ifindex;
            L_INET_AddrIp_T addr;
        } arg5; 
        /* Note: this arg5 cannot be deleted althouth it isn't used now 
                  because we use it to expand the receive buffer's size */
        struct 
        {
            UI32_T array[1024];
        }arg6;
    } data; /* contains the supplemntal data for the corresponding cmd */
} UDPHELPER_MGR_IPCMsg_T;

typedef enum UDPHELPER_MGR_IPCCMD_E
{
    UDPHELPER_MGR_IPCCMD_SET_STATUS,
    UDPHELPER_MGR_IPCCMD_UDPPORTSET,
    UDPHELPER_MGR_IPCCMD_UDPPORTUNSET,
    UDPHELPER_MGR_IPCCMD_HELPERSET,
    UDPHELPER_MGR_IPCCMD_HELPERUNSET,
    UDPHELPER_MGR_IPCCMD_L3IF_CREATE,
    UDPHELPER_MGR_IPCCMD_L3IF_DELETE,
    UDPHELPER_MGR_IPCCMD_RIF_CREATE,
    UDPHELPER_MGR_IPCCMD_RIF_DELETE,
    UDPHELPER_MGR_IPCCMD_GET_STATUS,
    UDPHELPER_MGR_IPCCMD_GETNEXT_FORWARD_PORT,
    UDPHELPER_MGR_IPCCMD_GETNEXT_HELPER,
    UDPHELPER_MGR_IPCCMD_GET_HELPER,
    UDPHELPER_MGR_IPCCMD_GET_FORWARD_PORT,        
}UDPHELPER_MGR_IPCCMD_T;


/* EXPORTED FUNCTION PROTOTYPE
 */ 

/* -------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_Initiate_System_Resources
 * -------------------------------------------------------------------------
 * PURPOSE: Initialize process resources for UDPHELEPER_MGR
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void UDPHELPER_MGR_Initiate_System_Resources(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_EnterMasterMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set udphelper_mgr into master mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void UDPHELPER_MGR_EnterMasterMode(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_ProvisionComplete
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set udphelper_mgr provision complete.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void UDPHELPER_MGR_ProvisionComplete(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_EnterSlaveMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set udphelper_mgr into slave mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void UDPHELPER_MGR_EnterSlaveMode(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_EnterTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set udphelper_mgr into transition mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void UDPHELPER_MGR_EnterTransitionMode(void); 

/*------------------------------------------------------------------------------
 * Function : UDPHELPER_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void UDPHELPER_MGR_SetTransitionMode(void); 

/*--------------------------------------------------------------------------
 * FUNCTION NAME - UDPHELPER_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void UDPHELPER_MGR_Create_InterCSC_Relation(void);

/* FUNCTION NAME : UDPHELPER_MGR_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for UDPHELPER mgr.
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
 *
 */
BOOL_T UDPHELPER_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_HandleHotInsertion
 * PURPOSE: Hot swap init function for insertion
 * INPUT:   starting_port_ifindex
 *          number_of_port
 *          use_default
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-------------------------------------------------------------------------*/
void UDPHELPER_MGR_HandleHotInsertion(UI32_T starting_port_ifindex,
                                    UI32_T number_of_port,
                                    BOOL_T use_default);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_MGR_HandleHotRemoval
 * PURPOSE: Hot swap init function for removal
 * INPUT:   starting_port_ifindex
 *          number_of_port
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 *------------------------------------------------------------------------*/
void UDPHELPER_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);
#endif /*_UDPHELPER_MGR_H_*/


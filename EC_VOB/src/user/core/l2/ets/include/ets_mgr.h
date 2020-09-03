/* MODULE NAME: ets_mgr.h
 * PURPOSE:
 *   Declarations of MGR APIs for ETS
 *   (IEEE Std 802.1Qaz - Enhanced Transmission Selection).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   11/23/2012 - Roy Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef _ETS_MGR_H_
#define _ETS_MGR_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "ets_type.h"
#include "ets_om.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define ETS_MGR_IPCMSG_TYPE_SIZE   sizeof(ETS_MGR_IpcMsg_Type_T)

/* command used in IPC message
 */
enum
{
    ETS_MGR_IPCCMD_GET_MODE,
    ETS_MGR_IPCCMD_SET_MODE,
    ETS_MGR_IPCCMD_GET_TC_TSA,
    ETS_MGR_IPCCMD_SET_TC_TSA_BY_USER,
    ETS_MGR_IPCCMD_GET_PRIO_ASSIGNMENT,
    ETS_MGR_IPCCMD_SET_PRIO_ASSIGNMENT_BY_USER,
    ETS_MGR_IPCCMD_GET_TC_WEIGHT,
    ETS_MGR_IPCCMD_SET_TC_WEIGHT_BY_USER,
    ETS_MGR_IPCCMD_GETRUNNING_PORT_MODE,
    ETS_MGR_IPCCMD_GETRUNNING_TC_WEIGHT,
    ETS_MGR_IPCCMD_GETRUNNING_TC_TSA,
    ETS_MGR_IPCCMD_GETRUNNING_PRIO_ASSIGNMENT,
    ETS_MGR_IPCCMD_GET_PORT_ENTRY,
    ETS_MGR_IPCCMD_GET_NEXT_PORT_ENTRY,
    ETS_MGR_IPCCMD_GET_MAX_NUMBER_OF_TC,
    ETS_MGR_IPCCMD_GET_OPER_MODE,
};

/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in ETS_MGR_IpcMsg_T.data
 */
#define ETS_MGR_GET_MSG_SIZE(field_name)                       \
            (ETS_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((ETS_MGR_IpcMsg_T *)0)->data.field_name))

#define ETS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
            sizeof(ETS_MGR_IpcMsg_Type_T)

#define ETS_MGR_MSG_CMD(msg_p)    (((ETS_MGR_IpcMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define ETS_MGR_MSG_RETVAL(msg_p) (((ETS_MGR_IpcMsg_T *)(msg_p)->msg_buf)->type.result)
#define ETS_MGR_MSG_DATA(msg_p)   ((void *)&((ETS_MGR_IpcMsg_T *)(msg_p)->msg_buf)->data)

#define ETS_MGR_MSGBUF_TYPE_SIZE    sizeof(((ETS_MGR_IpcMsg_T *)0)->type)
/* DATA TYPE DECLARATIONS
 */

/* Message declarations for IPC.
 */
typedef union
{
    UI32_T  cmd;        /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result_ui32;   /* for response */
    BOOL_T  ret_bool;   /* for response */

    SYS_TYPE_Get_Running_Cfg_T ret_running_cfg;
} ETS_MGR_IpcMsg_Type_T;

typedef union
{
        BOOL_T                          bool_data;
        UI32_T                          ui32_data;
        UI8_T                           ui8_data;

        struct
        {
            UI32_T ifindex;
            UI32_T cosq2group[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE];
            BOOL_T cosq2group_is_valid;
        }cos_group_mapping;
        struct
        {
            UI32_T ifindex;
            UI32_T method;
            UI32_T weights[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];
            BOOL_T weights_is_valid;
        }cos_group_scheduling;
        struct
        {
            UI32_T lport;
            ETS_TYPE_MODE_T mode;
        }ets_lport_mode;

        struct
        {
            UI32_T lport;
            UI32_T prio;
            UI32_T tc;
            ETS_TYPE_DB_T oper_cfg;
        }ets_lport_prio_tc_oper_cfg;

        struct
        {
            UI32_T lport;
            UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];
            ETS_TYPE_DB_T oper_cfg;
        }ets_lport_weight_oper_cfg;
        struct
        {
            UI32_T lport;
            UI32_T tc;
            ETS_TYPE_TSA_T tsa;
            ETS_TYPE_DB_T oper_cfg;
        }ets_lport_tc_tsa_oper_cfg;
        struct
        {
            UI32_T lport;
            ETS_TYPE_PortEntry_T entry;
            ETS_TYPE_DB_T oper_cfg;
        }ets_lport_entry_oper_cfg;
} ETS_MGR_IpcMsg_Data_T;

typedef struct
{
    ETS_MGR_IpcMsg_Type_T type;
    ETS_MGR_IpcMsg_Data_T data;
} ETS_MGR_IpcMsg_T;



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


 #if (ETS_TYPE_BUILD_LINUX == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME: ETS_OM_InitiateSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will init system resource
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void ETS_OM_InitiateSystemResources(void);

/*---------------------------------------------------------------------------------
 * FUNCTION - ETS_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for ETS OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void ETS_OM_AttachSystemResources(void);

#endif /* #if (ETS_TYPE_BUILD_LINUX == TRUE) */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_AddFstTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the port joins the trunk
 *          as the 1st member
 * INPUT  : trunk_ifindex  - specify which trunk to join.
 *          member_ifindex - specify which member port being add to trunk.
 * OUTPUT : None
 * RETURN : None
 * NOTES  : member_ifindex is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void ETS_MGR_AddFstTrkMbr_CallBack();
/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_AddTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the 2nd or the following
 *          trunk member is removed from the trunk
 * INPUT  : trunk_ifindex  - specify which trunk to join to
 *          member_ifindex - specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void ETS_MGR_AddTrkMbr_CallBack();

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_DelLstTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the last trunk member
 *          is removed from the trunk
 * INPUT  : trunk_ifindex   -- specify which trunk to join to
 *          member_ifindex  -- specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void ETS_MGR_DelLstTrkMbr_CallBack();
/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_DelTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the 2nd or the following
 *          trunk member is removed from the trunk
 * INPUT  : trunk_ifindex   -- specify which trunk to join to
 *          member_ifindex  -- specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void ETS_MGR_DelTrkMbr_CallBack();

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_CosConfigChanged_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from COS when cos config is changed
 *          on the lport
 * INPUT  : lport             -- which lport event occurred on
 *          new_is_cos_global -- TRUE if cos is global
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void ETS_MGR_CosConfigChanged_CallBack(
    UI32_T lport, BOOL_T new_is_cos_global);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce for ETS.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void ETS_MGR_InitiateSystemResources(void);
/*------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *------------------------------------------------------------------------*/
void ETS_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function returns the current operation mode of this component.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T ETS_MGR_GetOperationMode(void);
/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enable ETS operation while in master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void ETS_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Disable the ETS operation while in slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void ETS_MGR_EnterSlaveMode(void);
/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: It's the temporary transition mode between system into master
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void ETS_MGR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function sets the component to temporary transition mode.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 *--------------------------------------------------------------------------
 */
void ETS_MGR_SetTransitionMode(void);

/* ---------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: To do provision complete.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
void ETS_MGR_ProvisionComplete(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_HandleHotInsertion
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
void ETS_MGR_HandleHotInsertion(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port,
    BOOL_T in_use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_HandleHotRemoval
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
void ETS_MGR_HandleHotRemoval();

#if (ETS_TYPE_BUILD_LINUX == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - ETS_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE: Handle the ipc request message for ETS MGR.
 * INPUT  : msgbuf_p -- input request ipc message buffer
 * OUTPUT : msgbuf_p -- output response ipc message buffer
 * RETURN : TRUE  - there is a  response required to be sent
 *          FALSE - there is no response required to be sent
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
BOOL_T ETS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);
#endif /* #if (ETS_TYPE_BUILD_LINUX == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_PortLeaveTrunk_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Handle event: when a logical port is removed from a trunk
 * INPUT   : trunk_ifindex  - which trunk to add in
 *           member_ifindex - which port to be added
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void ETS_MGR_PortLeaveTrunk_CallBack(
    UI32_T trunk_ifindex, UI32_T member_ifindex, BOOL_T is_last);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_PortAddIntoTrunk_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Handle event: when a logical port is added to a trunk
 * INPUT   : trunk_ifindex  - which trunk to add in
 *           member_ifindex - which port to be added
 *           is_firstmem    - Is this port the first member?
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void ETS_MGR_PortAddIntoTrunk_CallBack(
    UI32_T trunk_ifindex, UI32_T member_ifindex, BOOL_T is_firstmem);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetOperConfigEntryByDCBx
 * -------------------------------------------------------------------------
 * FUNCTION: copy ETS OM by entire entry
 * INPUT   : lport - which port to be overwritten
 *           entry - input OM entry
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_CHIP_ERROR: set chip fail.
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : only operational OM is written.
 * -------------------------------------------------------------------------*/
BOOL_T ETS_MGR_SetOperConfigEntryByDCBx(UI32_T lport, ETS_TYPE_PortEntry_T  entry);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetPortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS OM by entire entry
 * INPUT   : lport - which port to be overwritten
 *           oper_cfg - operation or configuration
 * OUTPUT  : entry - input OM entry
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_CHIP_ERROR: set chip fail.
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_MGR_GetPortEntry(UI32_T lport, ETS_TYPE_PortEntry_T  *entry, ETS_TYPE_DB_T oper_cfg);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetNextPortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS OM by entire entry
 * INPUT   : lport_p  - which port to be overwritten
 *           oper_cfg - operation or configuration
 * OUTPUT  : entry_p  - input OM entry
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_CHIP_ERROR: set chip fail.
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_MGR_GetNextPortEntry(
    UI32_T *lport_p, ETS_TYPE_PortEntry_T  *entry_p, ETS_TYPE_DB_T oper_cfg);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetMaxNumberOfTC
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS OM by entire entry
 * INPUT   : none
 * OUTPUT  : max_nbr_of_tc - max number of Traffic class (machine depends)
 * RETURN  : ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_MGR_GetMaxNumberOfTC(UI8_T *max_nbr_of_tc);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_Is_Config_Match_Operation
 * -------------------------------------------------------------------------
 * FUNCTION: check if Config Matches Operation
 * INPUT   : lport  - which port
 * OUTPUT  : None
 * RETURN  : TRUE : config = operation
 *           FALSE: config != operation or input lport error
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_MGR_Is_Config_Match_Operation(UI32_T lport);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_SetWeightByDCBx
 * -------------------------------------------------------------------------
 * FUNCTION: Set BW weighting of all tc in a port
 * INPUT   : lport  - which port
 *           weight - BW weighting of all tc
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Only operational is updated.
 *           Weight is always 0 for non-ETS TC.
 *           Weight is 0~100 for ETS TC.
 *           For chip, weight=0 means Strict Priority, so we work around to be 1 by
 *              update OM and get again by ETS_MGR_GetAllTCWeight().
 * -------------------------------------------------------------------------*/
BOOL_T ETS_MGR_SetWeightByDCBx(UI32_T lport, UI32_T weight [ SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS ]);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_RestoreToConfigByDCBx
 * -------------------------------------------------------------------------
 * FUNCTION: sync operational OM to configuration one.
 * INPUT   : lport  - which port
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    : Restore OM and chip setting to be same as UI configuration.
 * -------------------------------------------------------------------------*/
BOOL_T ETS_MGR_RestoreToConfigByDCBx(UI32_T lport);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS mode on the interface.
 * INPUT   : lport    -- which port to get
 * OUTPUT  : mode     -- current mode
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_MGR_GetMode(UI32_T lport, ETS_TYPE_MODE_T *mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetOperMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS operation mode on the interface.
 * INPUT   : lport    -- which port to get
 * OUTPUT  : mode     -- current operation mode
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_MGR_GetOperMode(UI32_T lport, ETS_TYPE_MODE_T *mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_IsAnyPortOperEnabled
 * -------------------------------------------------------------------------
 * FUNCTION: To check is any port operational enabled
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T ETS_MGR_IsAnyPortOperEnabled(void);

#endif /* End of _ETS_MGR_H */


/* MODULE NAME: pfc_mgr.h
 * PURPOSE:
 *   Declarations of MGR APIs for PFC
 *   (IEEE Std 802.1Qbb, Priority-based Flow Control).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   10/15/12    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef _PFC_MGR_H_
#define _PFC_MGR_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "pfc_type.h"
#include "pfc_om.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define PFC_MGR_IPCMSG_TYPE_SIZE   sizeof(PFC_MGR_IpcMsg_Type_T)

/* command used in IPC message
 */
enum
{
    PFC_MGR_IPC_GET_DATA_BY_FLD,
    PFC_MGR_IPC_GET_RUNNING_DATA_BY_FLD,
    PFC_MGR_IPC_SET_DATA_BY_FLD,
};

/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in PFC_MGR_IpcMsg_T.data
 */
#define PFC_MGR_GET_MSG_SIZE(field_name)                       \
            (PFC_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((PFC_MGR_IpcMsg_T *)0)->data.field_name))

#define PFC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
            sizeof(PFC_MGR_IpcMsg_Type_T)

#define PFC_MGR_MSG_CMD(msg_p)    (((PFC_MGR_IpcMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define PFC_MGR_MSG_RETVAL(msg_p) (((PFC_MGR_IpcMsg_T *)(msg_p)->msg_buf)->type.result)
#define PFC_MGR_MSG_DATA(msg_p)   ((void *)&((PFC_MGR_IpcMsg_T *)(msg_p)->msg_buf)->data)

/* DATA TYPE DECLARATIONS
 */

/* Message declarations for IPC.
 */
typedef struct
{
    UI16_T  lport;
    UI16_T  fld_id;
    UI32_T  ui32_data;
} PFC_MGR_IpcMsg_LportFldData_T;

typedef union
{
    UI32_T  cmd;        /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  ret_ui32;   /* for response */
    BOOL_T  ret_bool;   /* for response */
} PFC_MGR_IpcMsg_Type_T;

typedef union
{
    BOOL_T                          bool_data;
    UI32_T                          ui32_data;
    PFC_MGR_IpcMsg_LportFldData_T   lport_fld_data;
} PFC_MGR_IpcMsg_Data_T;

typedef struct
{
    PFC_MGR_IpcMsg_Type_T type;
    PFC_MGR_IpcMsg_Data_T data;
} PFC_MGR_IpcMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_AddFstTrkMbr_CallBack
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
void PFC_MGR_AddFstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_AddTrkMbr_CallBack
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
void PFC_MGR_AddTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_DelLstTrkMbr_CallBack
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
void PFC_MGR_DelLstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_DelTrkMbr_CallBack
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
void PFC_MGR_DelTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_CosConfigChanged_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from COS when cos config is changed
 *          on the lport
 * INPUT  : lport         -- which lport event occurred on
 *          new_is_cos_ok -- TRUE if cos is ok for PFC
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void PFC_MGR_CosConfigChanged_CallBack(
    UI32_T  lport,
    BOOL_T  new_is_cos_ok);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce for PFC.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_MGR_InitiateSystemResources(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PFC_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *------------------------------------------------------------------------*/
void PFC_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function returns the current operation mode of this component.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T PFC_MGR_GetOperationMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enable PFC operation while in master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Disable the PFC operation while in slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_MGR_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: It's the temporary transition mode between system into master
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PFC_MGR_EnterTransitionMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_HandleHotInsertion
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
void PFC_MGR_HandleHotInsertion(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port,
    BOOL_T in_use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_HandleHotRemoval
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
void PFC_MGR_HandleHotRemoval(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port);

#if (PFC_TYPE_BUILD_LINUX == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME: PFC_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE: Handle the ipc request message for PFC MGR.
 * INPUT  : msgbuf_p -- input request ipc message buffer
 * OUTPUT : msgbuf_p -- output response ipc message buffer
 * RETURN : TRUE  - there is a  response required to be sent
 *          FALSE - there is no response required to be sent
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T PFC_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);
#endif

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function sets the component to temporary transition mode.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 *--------------------------------------------------------------------------
 */
void PFC_MGR_SetTransitionMode(void);

/* ---------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: To do provision complete.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
void PFC_MGR_ProvisionComplete(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_GetRunningDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running data by specified field id and lport.
 * INPUT  : lport    - which lport to get
 *                       0 - global data to get
 *                      >0 - lport  data to get
 *          field_id - field id to get (PFC_TYPE_FieldId_E)
 * OUTPUT : data_p   - pointer to output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE   : 1. all output data are represented in UI32_T
 *--------------------------------------------------------------------------
 */
UI32_T PFC_MGR_GetRunningDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_GetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To get the data by specified field id and lport.
 * INPUT  : lport    - which lport to get
 *                       0 - global data to get
 *                      >0 - lport  data to get
 *          field_id - field id to get (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to output data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : 1. all output data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_MGR_GetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_SetDataByFieldByDCBX
 *-------------------------------------------------------------------------
 * PURPOSE: To set the data by specified field id and lport for DCBX.
 * INPUT  : lport    - which lport to set
 *                      0 - global data to set
 *                     >0 - lport  data to set
 *          field_id - field id to set (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to input data
 * OUTPUT : None
 * RETURN : PFC_TYPE_ReturnCode_E
 * NOTE   : 1. all input data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
UI32_T PFC_MGR_SetDataByFieldByDCBX(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_SetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To set the data by specified field id and lport.
 * INPUT  : lport    - which lport to set
 *                      0 - global data to set
 *                     >0 - lport  data to set
 *          field_id - field id to set (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to input data
 * OUTPUT : None
 * RETURN : PFC_TYPE_ReturnCode_E
 * NOTE   : 1. all input data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
UI32_T PFC_MGR_SetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_SetDataByFieldWithoutXorCheck
 *-------------------------------------------------------------------------
 * PURPOSE: To set the data by specified field id and lport.
 * INPUT  : lport    - which lport to set
 *                      0 - global data to set
 *                     >0 - lport  data to set
 *          field_id - field id to set (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to input data
 * OUTPUT : None
 * RETURN : PFC_TYPE_ReturnCode_E
 * NOTE   : 1. all input data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
UI32_T PFC_MGR_SetDataByFieldWithoutXorCheck(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_RestoreToConfigByDCBX
 *-------------------------------------------------------------------------
 * PURPOSE: Restore operational OM to configuration one.
 * INPUT  : lport - which lport to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : Restore OM and chip setting to be same as UI configuration.
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_MGR_RestoreToConfigByDCBX(UI32_T lport);

#endif /* End of _PFC_MGR_H */


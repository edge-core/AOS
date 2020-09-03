/* MODULE NAME: pppoe_ia_mgr.h
 * PURPOSE:
 *   Declarations of MGR APIs for PPPOE Intermediate Agent.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/17/09    -- Squid Ro, Create
 *   11/26/09    -- Squid Ro, Modify for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 */

#ifndef _PPPOE_IA_MGR_H_
#define _PPPOE_IA_MGR_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "sysfun.h"
#include "pppoe_ia_type.h"
#include "pppoe_ia_om.h"
#include "pppoe_ia_engine.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define PPPOE_IA_MGR_IPCMSG_TYPE_SIZE   sizeof(PPPOE_IA_MGR_IpcMsg_Type_T)

#define PPPOE_IA_MAX(a,b)              ((a)>(b) ? (a) : (b))

/* use the maximum of  ACCESS_NODE_ID, CIRCUIT_ID, REMOTE_ID, GENERIC_ERMSG,
 * for the buffer size of string data
 */
#define PPPOE_IA_MGR_MAX_STR_LEN                                        \
            (PPPOE_IA_MAX(PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN,           \
                PPPOE_IA_MAX(PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN,           \
                    PPPOE_IA_MAX(PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN,  \
                        PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN))) + 1)

/* command used in IPC message
 */
enum
{
    PPPOE_IA_MGR_IPC_CLEAR_PORT_STATISTICS,
    PPPOE_IA_MGR_IPC_GET_ACCESS_NODE_ID,
    PPPOE_IA_MGR_IPC_GET_GENERIC_ERR_MSG,
    PPPOE_IA_MGR_IPC_GET_GLOBAL_ENABLE,
    PPPOE_IA_MGR_IPC_GET_NEXT_PORT_OPRCFG_ENTRY,
    PPPOE_IA_MGR_IPC_GET_NEXT_PORT_STATS_ENTRY,
    PPPOE_IA_MGR_IPC_GET_PORT_BOOL_DATA_BY_FLD,
    PPPOE_IA_MGR_IPC_GET_PORT_UI32_DATA_BY_FLD,
    PPPOE_IA_MGR_IPC_GET_PORT_OPRCFG_ENTRY,
    PPPOE_IA_MGR_IPC_GET_PORT_STATS_ENTRY,
    PPPOE_IA_MGR_IPC_GET_PORT_STR_DATA_BY_FLD,
    PPPOE_IA_MGR_IPC_GET_RUNNING_BOOL_DATA_BY_FLD,
    PPPOE_IA_MGR_IPC_GET_RUNNING_STR_DATA_BY_FLD,
    PPPOE_IA_MGR_IPC_GET_RUNNING_UI32_DATA_BY_FLD,
    PPPOE_IA_MGR_IPC_SET_GLOBAL_ENABLE,
    PPPOE_IA_MGR_IPC_SET_GLOBAL_ADM_STR_DATA_BY_FLD,
    PPPOE_IA_MGR_IPC_SET_PORT_BOOL_DATA_BY_FLD,
    PPPOE_IA_MGR_IPC_SET_PORT_UI32_DATA_BY_FLD,
    PPPOE_IA_MGR_IPC_SET_PORT_ADM_STR_DATA_BY_FLD,
};

/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in PPPOE_IA_MGR_IpcMsg_T.data
 */
#define PPPOE_IA_MGR_GET_MSG_SIZE(field_name)                       \
            (PPPOE_IA_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((PPPOE_IA_MGR_IpcMsg_T *)0)->data.field_name))

#define PPPOE_IA_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
            sizeof(PPPOE_IA_MGR_IpcMsg_Type_T)

#define PPPOE_IA_MGR_MSG_CMD(msg_p)    (((PPPOE_IA_MGR_IpcMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define PPPOE_IA_MGR_MSG_RETVAL(msg_p) (((PPPOE_IA_MGR_IpcMsg_T *)(msg_p)->msg_buf)->type.result)
#define PPPOE_IA_MGR_MSG_DATA(msg_p)   ((void *)&((PPPOE_IA_MGR_IpcMsg_T *)(msg_p)->msg_buf)->data)

/* DATA TYPE DECLARATIONS
 */

/* Message declarations for IPC.
 */
typedef struct
{
    UI32_T  lport;
    UI32_T  fld_id;
    BOOL_T  bool_data;
} PPPOE_IA_MGR_IpcMsg_LPortFldBoolData_T;

typedef struct
{
    UI32_T  lport;
    UI32_T  fld_id;
    UI32_T  ui32_data;
} PPPOE_IA_MGR_IpcMsg_LPortFldUi32Data_T;

typedef struct
{
    UI32_T  lport;
    UI32_T  fld_id;
    BOOL_T  is_oper;
    I32_T   str_len_max;
    UI8_T   str_data[PPPOE_IA_MGR_MAX_STR_LEN];
} PPPOE_IA_MGR_IpcMsg_LPortFldStrData_T;

typedef struct
{
    UI32_T                          lport;
    PPPOE_IA_OM_PortOprCfgEntry_T   pcfg_entry;
} PPPOE_IA_MGR_IpcMsg_LPortOprCfgEntry_T;

typedef struct
{
    UI32_T                      lport;
    PPPOE_IA_OM_PortStsEntry_T  psts_entry;
} PPPOE_IA_MGR_IpcMsg_LPortStsEntry_T;

typedef struct
{
    BOOL_T  bool_data;
    UI8_T   str_data[PPPOE_IA_MGR_MAX_STR_LEN];
} PPPOE_IA_MGR_IpcMsg_BoolStrData_T;

typedef union
{
    UI32_T  cmd;        /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  ret_ui32;   /* for response */
    BOOL_T  ret_bool;   /* for response */
} PPPOE_IA_MGR_IpcMsg_Type_T;

typedef union
{
    BOOL_T                                  bool_data;
    UI32_T                                  ui32_data;
    UI8_T                                   str_data[PPPOE_IA_MGR_MAX_STR_LEN];
    PPPOE_IA_MGR_IpcMsg_LPortFldBoolData_T  lport_fld_bool_data;
    PPPOE_IA_MGR_IpcMsg_LPortFldStrData_T   lport_fld_str_data;
    PPPOE_IA_MGR_IpcMsg_LPortFldUi32Data_T  lport_fld_ui32_data;
    PPPOE_IA_MGR_IpcMsg_LPortOprCfgEntry_T  lport_cfg_data;
    PPPOE_IA_MGR_IpcMsg_LPortStsEntry_T     lport_sts_data;
    PPPOE_IA_MGR_IpcMsg_BoolStrData_T       bool_str_data;
} PPPOE_IA_MGR_IpcMsg_Data_T;

typedef struct
{
    PPPOE_IA_MGR_IpcMsg_Type_T type;
    PPPOE_IA_MGR_IpcMsg_Data_T data;
} PPPOE_IA_MGR_IpcMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - PPPOE_IA_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PPPOE_IA_MGR_Create_InterCSC_Relation(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_AddFstTrkMbr_CallBack
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
void PPPOE_IA_MGR_AddFstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_AddTrkMbr_CallBack
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
void PPPOE_IA_MGR_AddTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_DelLstTrkMbr_CallBack
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
void PPPOE_IA_MGR_DelLstTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_DelTrkMbr_CallBack
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
void PPPOE_IA_MGR_DelTrkMbr_CallBack(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enable PPPoE IA operation while in master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Disable the PPPOE IA operation while in slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: It's the temporary transition mode between system into master
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_ClearPortStatistics
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the statistics entry for the specified ifindex.
 * INPUT  : lport - 1-based ifindex to clear
 *                  (0 to clear all)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_ClearPortStatistics(
    UI32_T  lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetAccessNodeId
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global access node id. (operation string)
 * INPUT  : is_oper   - TRUE to get operation access node id
 * OUTPUT : outbuf_ar - pointer to output buffer
 *                      >= PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1
 * RETURN : TRUE/FALSE
 * NOTE   : a. access node id may be
 *           1. user configured id
 *           2. first ip address if 1 is not available
 *           3. cpu mac if 1 & 2 are not available
 *          b. set API - PPPOE_IA_MGR_SetGlobalAdmStrDataByField
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetAccessNodeId(
    BOOL_T  is_oper,
    UI8_T   outbuf_ar[PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1]);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetGenericErrMsg
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global generic error message. (operation string)
 * INPUT  : is_oper   - TRUE to get operation generic error message
 * OUTPUT : outbuf_ar - pointer to output buffer
 *                      >= PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1
 * RETURN : TRUE/FALSE
 * NOTE   : 1. set API - PPPOE_IA_MGR_SetGlobalAdmStrDataByField
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetGenericErrMsg(
    BOOL_T  is_oper,
    UI8_T   outbuf_ar[PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1]);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetGlobalEnable
 * ------------------------------------------------------------------------
 * PURPOSE: To get global enable status
 * INPUT  : None
 * OUTPUT : is_enable_p - pointer to output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_MGR_SetGlobalEnable
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetGlobalEnable(
    BOOL_T  *is_enable_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetNextPortOprCfgEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get next PPPoE IA port config entry
 * INPUT  : lport_p - pointer to lport used to get next
 * OUTPUT : lport_p - pointer to lport got
 *          pcfg_p  - pointer to config entry got
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetNextPortOprCfgEntry(
    UI32_T                          *lport_p,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetNextPortStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get next PPPoE IA port statistic entry
 * INPUT  : lport_p - pointer to lport used to get next
 * OUTPUT : lport_p - pointer to lport got
 *          psts_p  - pointer to statistic entry got
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetNextPortStatisticsEntry(
    UI32_T                      *lport_p,
    PPPOE_IA_OM_PortStsEntry_T  *psts_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function returns the current operation mode of this component
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T PPPOE_IA_MGR_GetOperationMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetPortStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To get string data for specified ifindex and field id.
 * INPUT  : lport     - 1-based ifindex to get
 *                      0 to get global data
 *                      (e,g. access node id or generic error message)
 *          fld_id    - PPPOE_IA_TYPE_FLDID_E
 *                      field id to get
 *          is_oper   - TRUE to get operation string
 *          str_len_p - length of input buffer
 *                      (including null terminator)
 * OUTPUT : str_p     - pointer to output string data
 *          str_len_p - length of output buffer used
 *                      (not including null terminator)
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetPortStrDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  is_oper,
    I32_T   *str_len_p,
    UI8_T   *str_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetPortBoolDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To get boolean data for specified ifindex and field id.
 * INPUT  : lport  - 1-based ifindex to get
 *          fld_id - PPPOE_IA_TYPE_FLDID_E
 *                   field id to get
 * OUTPUT : val_p  - pointer to output value
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_MGR_SetPortBoolDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetPortBoolDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  *val_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetPortUi32DataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To get ui32 data for specified ifindex and field id.
 * INPUT  : lport  - 1-based ifindex to get
 *          fld_id - PPPOE_IA_TYPE_FLDID_E
 *                   field id to get
 * OUTPUT : val_p  - pointer to output value
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_MGR_SetPortUi32DataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetPortUi32DataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI32_T  *val_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get the PPPoE IA port config entry for specified lport.
 * INPUT  : lport  - lport to get
 * OUTPUT : pcfg_p - pointer to config entry got
 * RETURN : TRUE/FALSE
 * NOTE   : 1. will get the operation string for the port
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetPortConfigEntry(
    UI32_T                          lport,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetPortStatisticsEntry
 *--------------------------------------------------------------------------
 * PURPOSE: To get the port statistics entry for specified lport.
 * INPUT  : lport      - 1-based lport
 * OUTPUT : psts_ent_p - pointer to the output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_GetPortStatisticsEntry(
    UI32_T                      lport,
    PPPOE_IA_OM_PortStsEntry_T  *psts_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetRunningBoolDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running bool data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : bool_flag_p - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_MGR_GetRunningBoolDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  *bool_flag_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetRunningUi32DataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running ui32 data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : ui32_data_p - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_MGR_GetRunningUi32DataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  *ui32_data_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_GetRunningStrDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running string data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 *          str_len_max - maximum length of buffer to receive the string
 *                        (including null terminator)
 * OUTPUT : string_p    - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_MGR_GetRunningStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  str_len_max,
    UI8_T   *string_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_HandleHotInsertion
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
void PPPOE_IA_MGR_HandleHotInsertion(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port,
    BOOL_T in_use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_HandleHotRemoval
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
void PPPOE_IA_MGR_HandleHotRemoval(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: PPPOE_IA_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for PPPOE_IA MGR.
 * INPUT   : msgbuf_p -- input request ipc message buffer
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 * RETURN  : TRUE  - there is a  response required to be sent
 *           FALSE - there is no response required to be sent
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce for PPPOE_IA.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_InitiateSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_ProcessRcvdPDU
 *-------------------------------------------------------------------------
 * PURPOSE: To process the received PPPoE PDU. (PPPoE discover)
 * INPUT  : msg_p - pointer to the message get from the msg queue
 * OUTPUT : None
 * RETURN : None
 * NOTE   : call from pppoe_ia_task
 *-------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_ProcessRcvdPDU(
    PPPOE_IA_TYPE_Msg_T  *pppoe_ia_msg_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetGlobalEnable
 * ------------------------------------------------------------------------
 * PURPOSE: To enable or disable globally.
 * INPUT  : is_enable - TRUE to enable
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_MGR_GetGlobalEnable
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_SetGlobalEnable(
    BOOL_T  is_enable);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetGlobalAdmStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set global string data for specified field id.
 * INPUT  : fld_id    - PPPOE_IA_TYPE_FLDID_E
 *                      field id to set
 *          str_p     - pointer to input string data
 *          str_len   - length of input string data
 *                      (not including null terminator)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_MGR_GetAccessNodeId/
 *                       PPPOE_IA_MGR_GetGenericErrMsg
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_SetGlobalAdmStrDataByField(
    UI32_T  fld_id,
    UI8_T   *str_p,
    UI32_T  str_len);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetPortBoolDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set boolean data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_MGR_GetPortBoolDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_SetPortBoolDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  new_val);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetPortUi32DataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set ui32 data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_MGR_GetPortUi32DataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_SetPortUi32DataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI32_T  new_val);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetPortAdmStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set string data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          str_p   - pointer to input string data
 *          str_len - length of input string data
 *                    (not including null terminator)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. use engine API to apply setting on trunk.
 *          2. get API - PPPOE_IA_MGR_GetPortStrDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_MGR_SetPortAdmStrDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI8_T   *str_p,
    UI32_T  str_len);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_MGR_SetTransitionMode(void);

#endif /* End of _PPPOE_IA_MGR_H */


/* MODULE NAME: pppoe_ia_om.h
 * PURPOSE:
 *   Declarations of OM APIs for PPPOE Intermediate Agent.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/17/09    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2009
 */

#ifndef _PPPOE_IA_OM_H
#define _PPPOE_IA_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "pppoe_ia_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct PPPOE_IA_OM_PortAdmCfgEntry_S
{
    BOOL_T  is_trust;
    BOOL_T  is_strip_vtag;
    BOOL_T  is_enable;
    BOOL_T  is_rid_delim_en;
    UI8_T   rid_delim_ascii;
    UI8_T   circuit_id_len;
    UI8_T   remote_id_len;
    UI8_T   circuit_id[PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN+1];
    UI8_T   remote_id[PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+1];
} PPPOE_IA_OM_PortAdmCfgEntry_T;

typedef struct PPPOE_IA_OM_PortStsEntry_S
{
    UI32_T  all;
    UI32_T  padi;
    UI32_T  pado;
    UI32_T  padr;
    UI32_T  pads;
    UI32_T  padt;
    UI32_T  malform;
    UI32_T  rep_untrust;  /* received response on untrusted port */
    UI32_T  req_untrust;  /* received request  to untrusted port */
} PPPOE_IA_OM_PortStsEntry_T;

typedef struct PPPOE_IA_OM_PortOprCfgEntry_S
{
    PPPOE_IA_OM_PortAdmCfgEntry_T   adm_cfg;
    UI8_T                           opr_ccid_len;
    UI8_T                           opr_rmid_len;
    UI8_T                           opr_ccid[PPPOE_IA_TYPE_MAX_CIRCUIT_ID_LEN+1];
    UI8_T                           opr_rmid[PPPOE_IA_TYPE_MAX_REMOTE_ID_LEN+1];
} PPPOE_IA_OM_PortOprCfgEntry_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_AddPortToPorts
 *--------------------------------------------------------------------------
 * PURPOSE: To add one specified ifindex to the lport lists. (enabled port only)
 * INPUT  : lport - 1-based ifindex to add to the lport lists
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. update the enable/trust/untrust lport lists according to config
 *          2. for clear, the src_ifidx should be removed explicitly first
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_AddPortToPorts(
    UI32_T  lport);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_ClearOM
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the om database.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_OM_ClearOM(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_ClearPortConfig
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the config entry for the specified ifindex.
 * INPUT  : lport - 1-based ifindex to clear
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_OM_ClearPortConfig(
    UI32_T  lport);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_ClearPortStatistics
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the statistics entry for the specified ifindex.
 * INPUT  : lport - 1-based ifindex to clear
 *                  (0 to clear all)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_ClearPortStatistics(
    UI32_T  lport);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_CopyPortConfigTo
 *--------------------------------------------------------------------------
 * PURPOSE: To copy one config entry from specified src_ifidx to dst_ifidx
 * INPUT  : src_ifidx - 1-based source      ifindex to copy config from
 *          dst_ifidx - 1-based destination ifindex to copy config to
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_CopyPortConfigTo(
    UI32_T  src_ifidx,
    UI32_T  dst_ifidx);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_DelPortFromPorts
 *--------------------------------------------------------------------------
 * PURPOSE: To remove one specified ifindex from the lport lists.
 * INPUT  : src_ifidx - 1-based ifindex to remove from the lport lists
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_DelPortFromPorts(
    UI32_T  src_ifidx);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetBoolDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get boolean data by field id from port or global config entry.
 * INPUT  : lport       - lport to get
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : bool_flag_p - pointer to content of output data
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetBoolDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  *bool_flag_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetUi32DataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get boolean data by field id from port or global config entry.
 * INPUT  : lport       - lport to get
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : ui32_data_p - pointer to content of output data
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetUi32DataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  *ui32_data_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetLports
 *--------------------------------------------------------------------------
 * PURPOSE: To get current lport list for specified port type
 * INPUT  : port_type   - PPPOE_IA_TYPE_PTYPE_E
 *                        port type to get
 * OUTPUT : lports      - pointer to content of output lport list
 *          port_num_p  - pointer to output port number
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetLports(
    UI8_T   port_type,
    UI8_T   lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
    UI32_T  *port_num_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetPortAdmCfgEntry
 *--------------------------------------------------------------------------
 * PURPOSE: To get one port config entry for specified lport.
 * INPUT  : lport      - 1-based lport
 * OUTPUT : pcfg_ent_p - pointer to the output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetPortAdmCfgEntry(
    UI32_T                          lport,
    PPPOE_IA_OM_PortAdmCfgEntry_T   *pcfg_ent_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetPortStatisticsEntry
 *--------------------------------------------------------------------------
 * PURPOSE: To get the port statistics entry for specified lport.
 * INPUT  : lport      - 1-based lport
 * OUTPUT : psts_ent_p - pointer to the output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetPortStatisticsEntry(
    UI32_T                      lport,
    PPPOE_IA_OM_PortStsEntry_T  *psts_ent_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetRunningBoolDataByField
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
UI32_T PPPOE_IA_OM_GetRunningBoolDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  *bool_flag_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetRunningUi32DataByField
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
UI32_T PPPOE_IA_OM_GetRunningUi32DataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  *ui32_data_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetRunningStrDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running string data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 *          str_len_max - maximum length of buffer to receive the string
 * OUTPUT : string_p    - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_OM_GetRunningStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  str_len_max,
    UI8_T   *string_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_GetAdmStrDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get string data by field id from port or global config entry.
 * INPUT  : lport     - lport
 *                       0, get from the global config entry
 *                      >0, get from the port config entry
 *          field_id  - PPPOE_IA_TYPE_FLDID_E
 *          str_len_p - length of buffer to receive the string
 * OUTPUT : str_len_p - length of buffer to receive the string
 *          string_p  - pointer to content of output data
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_GetAdmStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    I32_T   *str_len_p,
    UI8_T   *string_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_ResetOnePortToDefault
 *--------------------------------------------------------------------------
 * PURPOSE: To reset the om database of one port to default.
 * INPUT  : lport - 1-based ifindex
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 1. need to reset port list by other API.
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_OM_ResetOnePortToDefault(
    UI32_T  lport);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_ResetToDefault
 *--------------------------------------------------------------------------
 * PURPOSE: To reset the om database to default.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 1. called by PPPOE_IA_MGR_EnterMasterMode
 *          2. should use PPPOE_IA_ENGINE_SetDefaultConfig to
 *             setup the env for this config.
 *--------------------------------------------------------------------------
 */
void PPPOE_IA_OM_ResetToDefault(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_SetUi32DataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To set boolean data by field id to port or global config entry.
 * INPUT  : lport     - lport
 *                       0, set to the global config entry
 *                      >0, set to the port config entry
 *          field_id  - PPPOE_IA_TYPE_FLDID_E
 *          ui32_data - content of input data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_SetUi32DataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  ui32_data);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_SetBoolDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To set boolean data by field id to port or global config entry.
 * INPUT  : lport     - lport
 *                       0, set to the global config entry
 *                      >0, set to the port config entry
 *          field_id  - PPPOE_IA_TYPE_FLDID_E
 *          bool_flag - content of input data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_SetBoolDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  bool_flag);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_SetAdmStrDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To set string data by field id from port or global config entry.
 * INPUT  : lport     - lport
 *                         0, set to the global config entry
 *                        >0, set to the port config entry
 *          field_id  - PPPOE_IA_TYPE_FLDID_E
 *          str_len   - length of input string
 *          string_p  - pointer to content of input string
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_SetAdmStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI8_T   *string_p,
    UI8_T   str_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for LACP objects
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   none
 * NOTE     :   This function is invoked in LACP_TASK_Init.
 *-------------------------------------------------------------------------
 */
void PPPOE_IA_OM_InitSemaphore(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_OM_IncreaseStatisticsByField
 *--------------------------------------------------------------------------
 * PURPOSE: To increase the statistics for specified lport and field id.
 * INPUT  : lport      - 1-based lport to update
 *          ing_fld_id - PPPOE_IA_TYPE_FLDID_E
 *                       ingress type of packet
 *          err_fld_id - PPPOE_IA_TYPE_FLDID_E
 *                       drop reason
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_OM_IncreaseStatisticsByField(
    UI32_T  lport,
    UI32_T  ing_fld_id,
    UI32_T  err_fld_id);

#endif /* End of _PPPOE_IA_OM_H */


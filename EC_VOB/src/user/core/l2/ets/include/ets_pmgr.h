/* MODULE NAME: ets_pmgr.h
 *   Declarations of MGR IPC APIs for ETS
 *   (802.1Qaz - Enhanced Transmission Selection).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   11/23/2012 - Roy Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef _ETS_PMGR_H
#define _ETS_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "ets_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

 /*-----------------------------------------------------------------------------
 * FUNCTION NAME - ETS_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for ETS_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T ETS_PMGR_InitiateProcessResource(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS mode on the interface.
 * INPUT   : lport -- which port to configure
 * OUTPUT  : mode  -- current mode
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetMode(UI32_T lport, ETS_TYPE_MODE_T *mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_SetMode
 * -------------------------------------------------------------------------
 * FUNCTION: Set ETS mode on the interface.
 * INPUT   : lport -- which port to configure.
 *           mode  -- which mode to set.
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_SetMode(UI32_T lport, ETS_TYPE_MODE_T mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetOperMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS operation mode on the interface.
 * INPUT   : lport -- which port to get
 * OUTPUT  : mode  -- current operation mode
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetOperMode(UI32_T lport, ETS_TYPE_MODE_T *mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetTSA
 * -------------------------------------------------------------------------
 * FUNCTION: Get TSA of a traffic class on the interface.
 * INPUT   : lport    -- which port to configure.
 *           tc       -- which traffic class
 *           oper_cfg -- operation or configuration
 * OUTPUT  : tsa      -- outputed TSA
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetTSA(UI32_T lport, UI32_T tc, UI32_T* tsa, ETS_TYPE_DB_T oper_cfg);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_SetTSAByUser
 * -------------------------------------------------------------------------
 * FUNCTION: Set TSA of a traffic class on the interface from UI.
 * INPUT   : lport    -- which port to configure.
 *           tc       -- which traffic class
 *           tsa      -- what kind of  TSA
 * OUTPUT  : NONE
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Both operational and configuration OM are updated.
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_SetTSAByUser(UI32_T lport, UI32_T tc, UI32_T tsa);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetPortPrioAssign
 * -------------------------------------------------------------------------
 * FUNCTION: Set the priotiy to TC mapping tc in a port
 * INPUT   : lport  - which port
 *           prio   - which priority
 * OUTPUT  : tc     - the TC of the prio belongs
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetPortPrioAssign(UI32_T lport, UI32_T prio, UI32_T* tc, ETS_TYPE_DB_T oper_cfg);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_SetPortPrioAssignByUser
 * -------------------------------------------------------------------------
 * FUNCTION: Change mapping of priority to tc in a port
 * INPUT   : lport - which port
 *           prio  - which Cos priority
 *           tc    - which traffic class of this Cos priority belong
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Both operational and configuration OM are copied.
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_SetPortPrioAssignByUser(UI32_T lport, UI32_T prio, UI32_T tc);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Get BW weighting of all tc in a port
 * INPUT   : lport  - which port
 * OUTPUT  : weight - BW weighting of all tc
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetWeight(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS],  ETS_TYPE_DB_T oper_cfg);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_SetWeightByUser
 * -------------------------------------------------------------------------
 * FUNCTION: Set BW weighting of all tc in a port
 * INPUT   : lport  - which port
 *           weight - BW weighting of all tc
 * OUTPUT  : None
 * RETURN  : ETS_TYPE_RETURN_PORTNO_OOR: input lport not valid
 *           ETS_TYPE_RETURN_INPUT_ERR : ouput pointer invalid
 *           ETS_TYPE_RETURN_OK        : success
 * NOTE    : Both operational and configuration OM are updated.
 *           Weight is always 0 for non-ETS TC.
 *           Weight is 0~100 for ETS TC.
 *           If all weights are 0xffffffff, weights will be reset to default.
 *           For chip, weight=0 means Strict Priority, so we work around to be 1 by
 *              update OM and get again by SWCTRL_ETS_MGR_GetAllTCWeight().
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_SetWeightByUser(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetPortEntry
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
UI32_T ETS_PMGR_GetPortEntry(UI32_T lport, ETS_TYPE_PortEntry_T  *entry, ETS_TYPE_DB_T oper_cfg);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetNextPortEntry
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
UI32_T ETS_PMGR_GetNextPortEntry(
    UI32_T *lport_p, ETS_TYPE_PortEntry_T  *entry_p, ETS_TYPE_DB_T oper_cfg);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_PMGR_GetMaxNumberOfTC
 * -------------------------------------------------------------------------
 * FUNCTION: Get max number of ETS  on this machine
 * INPUT   : none
 * OUTPUT  : max_nbr_of_tc - max number of Traffic class (machine depends)
 * RETURN  : ETS_TYPE_RETURN_OK        : success
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T ETS_PMGR_GetMaxNumberOfTC(UI8_T *max_nbr_of_tc);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ETS_MGR_GetRunningPortMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS operation mode on the interface.
 * INPUT   : lport    -- which port to configure.
 * OUTPUT  : mode     -- which mode to set.
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T ETS_PMGR_GetRunningPortMode(UI32_T lport, ETS_TYPE_MODE_T *mode);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ETS_MGR_GetRunningTCTSA
 * -------------------------------------------------------------------------
 * FUNCTION: Get TSA of a traffic class on the interface.
 * INPUT   : lport    -- which port to configure.
 *           tc       -- which traffic class
 * OUTPUT  : tsa      -- outputed TSA
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    : Get the configuration OM instead of operational one.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T ETS_PMGR_GetRunningTCTSA(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T* tsa);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_MGR_GetRunningWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Get BW weighting of all tc in a port
 * INPUT   : lport  - which port
 * OUTPUT  : weight - BW weighting of all tc
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    : Get the configuration OM instead of operational one.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T ETS_PMGR_GetRunningWeight(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS]);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ETS_MGR_GetRunningPrioAssign
 * -------------------------------------------------------------------------
 * FUNCTION: Set the priotiy to TC mapping tc in a port
 * INPUT   : lport  - which port
 *           prio   - which priority
 * OUTPUT  : tc     - the TC of the prio belongs
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL      : error
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : all setting = default.
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS   : something differ from default
 * NOTE    : Get the configuration OM instead of operational one.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T ETS_PMGR_GetRunningPrioAssign(UI32_T lport, UI32_T prio, UI32_T* tc);

#endif /* #ifndef _ETS_PMGR_H */


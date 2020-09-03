/* MODULE NAME: ets_om.h
 * PURPOSE:
 *   Declarations of OM APIs for ETS
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

#ifndef _ETS_OM_H
#define _ETS_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "ets_type.h"

#if (ETS_TYPE_BUILD_LINUX == TRUE)
#include "sysrsc_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */

/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#if (ETS_TYPE_BUILD_LINUX == TRUE)
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

/*-------------------------------------------------------------------------
 * FUNCTION NAME - ETS_OM_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  -  shared memory segment id
 *          seglen_p -  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:   This function is called in SYSRSC_MGR_CreateSharedMemory().
 *-------------------------------------------------------------------------
 */
void ETS_OM_GetShMemInfo(
    SYSRSC_MGR_SEGID_T  *segid_p,
    UI32_T              *seglen_p);

#endif /* #if (ETS_TYPE_BUILD_LINUX == TRUE) */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_OM_ResetToDefault
 * ------------------------------------------------------------------------
 * PURPOSE: To do extra work for default configuration for all port.
 *          (e.g. setup the rules.)
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void ETS_OM_ResetToDefault();
/*--------------------------------------------------------------------------
 * FUNCTION NAME - ETS_OM_ClearOM
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the om database.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void ETS_OM_ClearOM();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_CopyByLPort
 * -------------------------------------------------------------------------
 * FUNCTION: copy ETS OM from 1 port to another
 * INPUT   : des_lport - the destination port to copy to
 *           src_lport - the source port to copy from
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    : Both operational and configuration OM are copied.
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_CopyByLPort(UI32_T des_lport, UI32_T src_lport);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_CopyByLPort
 * -------------------------------------------------------------------------
 * FUNCTION: copy ETS OM by entire entry
 * INPUT   : lport - which port to be overwritten
 *           entry - input OM entry
 *           oper_cfg - operation or configuration
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_SetLPortEntry(UI32_T lport, ETS_TYPE_PortEntry_T *entry, ETS_TYPE_DB_T oper_cfg);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_GetLPortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS OM by entire entry
 * INPUT   : lport - which port to be overwritten
 *           entry - input OM entry
 *           oper_cfg - operation or configuration
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_GetLPortEntry(UI32_T lport,
                        ETS_TYPE_PortEntry_T *entry, ETS_TYPE_DB_T oper_cfg);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_Is_Config_Match_Operation
 * -------------------------------------------------------------------------
 * FUNCTION: check if Config Matches Operation
 * INPUT   : lport  - which port
 * OUTPUT  : None
 * RETURN  : TRUE : config = operation
 *           FALSE: config != operation
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_Is_Config_Match_Operation(UI32_T lport);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_SetTSA
 * -------------------------------------------------------------------------
 * FUNCTION: Change tsa of a tc in a port
 * INPUT   : lport - which port
 *           tc    - which traffic class
 *           tsa   - what kinds of TX selection algorithm
 *           oper_cfg - operation or configuration
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_SetTSA(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T tsa, ETS_TYPE_DB_T oper_cfg);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_GetTSA
 * -------------------------------------------------------------------------
 * FUNCTION: Change tsa of a tc in a port
 * INPUT   : lport - which port
 *           tc    - which traffic class
 *           oper_cfg - operation or configuration
 * OUTPUT  : tsa   - what kinds of TX selection algorithm
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_GetTSA(UI32_T lport, UI32_T tc, ETS_TYPE_TSA_T *tsa, ETS_TYPE_DB_T oper_cfg);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_SetMode
 * -------------------------------------------------------------------------
 * FUNCTION: Set ETS mode on the interface.
 * INPUT   : lport  -- which port to configure.
 *           mode   -- which mode to set.
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_SetMode(UI32_T lport, ETS_TYPE_MODE_T mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_GetMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS mode on the interface.
 * INPUT   : lport -- which port to configure.
 * OUTPUT  : mode  -- which mode
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_GetMode(UI32_T lport, ETS_TYPE_MODE_T *mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_SetOperMode
 * -------------------------------------------------------------------------
 * FUNCTION: Set ETS operation mode on the interface.
 * INPUT   : lport  -- which port to configure.
 *           mode   -- which mode to set.
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_SetOperMode(UI32_T lport, ETS_TYPE_MODE_T mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_GetOperMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS operation mode on the interface.
 * INPUT   : lport -- which port to configure.
 * OUTPUT  : mode  -- which mode
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_GetOperMode(UI32_T lport, ETS_TYPE_MODE_T *mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_SetPortPrioAssign
 * -------------------------------------------------------------------------
 * FUNCTION: Change mapping of priority to tc in a port
 * INPUT   : lport - which port
 *           prio  - which Cos priority
 *           tc    - which traffic class of this Cos priority belong
 *           oper_cfg - operation or configuration
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_SetPortPrioAssign(UI32_T lport, UI32_T prio, UI32_T tc, ETS_TYPE_DB_T oper_cfg);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_GetPortPrioAssign
 * -------------------------------------------------------------------------
 * FUNCTION: Get mapping of priority to tc in a port
 * INPUT   : lport - which port
 *           prio  - which Cos priority
 *           oper_cfg - operation or configuration
 * OUTPUT  : tc    - which traffic class of this Cos priority belong
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_GetPortPrioAssign(UI32_T lport, UI32_T prio, UI32_T* tc, ETS_TYPE_DB_T oper_cfg);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_SetWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Change BW weighting of all tc in a port
 * INPUT   : lport  - which port
 *           weight - BW weighting of all tc
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_SetWeight(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS], ETS_TYPE_DB_T oper_cfg);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_GetWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Get BW weighting of all tc in a port
 * INPUT   : lport  - which port
 * OUTPUT  : weight - BW weighting of all tc
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_GetWeight(UI32_T lport, UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS], ETS_TYPE_DB_T oper_cfg);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_RestoreToConfig
 * -------------------------------------------------------------------------
 * FUNCTION: sync operational OM to configuration one.
 * INPUT   : lport  - which port
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    : the mode is not restored for it always be the same between oper and cfg.
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_RestoreToConfig(UI32_T lport);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_GetIsCosConfigOk
 * -------------------------------------------------------------------------
 * FUNCTION: To get is_cos_config_ok flag
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : is_cos_config_ok flag
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_GetIsCosConfigOk(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_SetIsCosConfigOk
 * -------------------------------------------------------------------------
 * FUNCTION: To set is_cos_config_global flag
 * INPUT   : new_is_cos_ok -
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_SetIsCosConfigOk(
    BOOL_T  new_is_cos_ok);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_IsAnyPortOperEnabled
 * -------------------------------------------------------------------------
 * FUNCTION: To check is any port operational enabled
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_IsAnyPortOperEnabled(void);

#endif /* End of _ETS_OM_H */


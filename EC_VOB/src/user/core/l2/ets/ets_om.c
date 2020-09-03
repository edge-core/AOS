/* MODULE NAME: ets_om.c
 * PURPOSE:
 *   Definitions of OM APIs for ETS
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

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_type.h"
#include "sys_dflt.h"

#if (SYS_CPNT_ETS == TRUE)

#include "sysfun.h"
#include "ets_backdoor.h"
#include "ets_om.h"
#include "l_stdlib.h"
#include "l_rstatus.h"

#if (ETS_TYPE_BUILD_LINUX == TRUE)
#include "sys_bld.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */

#if (ETS_TYPE_BUILD_LINUX == TRUE)
    #define ETS_OM_LOCK()              orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ets_om_sem_id)
    #define ETS_OM_UNLOCK()            SYSFUN_OM_LEAVE_CRITICAL_SECTION(ets_om_sem_id, orig_priority)
    #define ETS_OM_ENTER_CRITICAL_SECTION() SYSFUN_OM_ENTER_CRITICAL_SECTION(ets_om_sem_id)
    #define ETS_OM_LEAVE_CRITICAL_SECTION() SYSFUN_OM_LEAVE_CRITICAL_SECTION(ets_om_sem_id, orig_priority)
#else
    #define ETS_OM_LOCK()              SYSFUN_OM_ENTER_CRITICAL_SECTION()
    #define ETS_OM_UNLOCK()            SYSFUN_OM_LEAVE_CRITICAL_SECTION()
#endif

/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DECLARATIONS
 */
#if (ETS_TYPE_BUILD_LINUX == TRUE)
static UI32_T           ets_om_sem_id;
static UI32_T           orig_priority;
#endif
static ETS_TYPE_INFO_T  ets_oper;
static ETS_TYPE_INFO_T  ets_config;
static BOOL_T           ets_om_is_cos_config_ok; /* global and 1-to-1 */

/* EXPORTED SUBPROGRAM BODIES
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
void ETS_OM_InitiateSystemResources(void)
{
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_ETS_OM, &ets_om_sem_id);
}


/*---------------------------------------------------------------------------------
 * FUNCTION - ETS_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for ETS OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void ETS_OM_AttachSystemResources(void)
{
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_ETS_OM, &ets_om_sem_id);
}


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
void ETS_OM_ResetToDefault()
{
}

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
void ETS_OM_ClearOM()
{
    memset(&ets_oper, 0 ,sizeof(ETS_TYPE_INFO_T));
    memset(&ets_config, 0 ,sizeof(ETS_TYPE_INFO_T));
    ets_om_is_cos_config_ok = TRUE;
}

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
BOOL_T ETS_OM_CopyByLPort(UI32_T des_lport, UI32_T src_lport)
{
    ETS_OM_ENTER_CRITICAL_SECTION();
    ets_oper.mode[des_lport-1]    = ets_oper.mode[src_lport-1];
    ets_config.mode[des_lport-1]  = ets_config.mode[src_lport-1];

    memcpy( &ets_oper.port_entry[des_lport-1], &ets_oper.port_entry[src_lport-1], sizeof(ETS_TYPE_PortEntry_T));
    memcpy( &ets_config.port_entry[des_lport-1], &ets_config.port_entry[src_lport-1], sizeof(ETS_TYPE_PortEntry_T));

    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_SetLPortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Set ETS OM by entire entry
 * INPUT   : lport - which port to be overwritten
 *           entry - input OM entry
 *           oper_cfg - operation or configuration
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_SetLPortEntry(UI32_T lport,
                        ETS_TYPE_PortEntry_T *entry, ETS_TYPE_DB_T oper_cfg)
{
    ETS_TYPE_INFO_T *ets_db = &ets_oper;
    if(oper_cfg==ETS_TYPE_DB_CONFIG)
    {
        ets_db = &ets_config;
    }
    ETS_OM_ENTER_CRITICAL_SECTION();
    memcpy( &ets_db->port_entry[lport-1], entry, sizeof(ETS_TYPE_PortEntry_T));
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

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
                        ETS_TYPE_PortEntry_T *entry, ETS_TYPE_DB_T oper_cfg)
{
    ETS_TYPE_INFO_T *ets_db = &ets_oper;
    if(oper_cfg==ETS_TYPE_DB_CONFIG)
    {
        ets_db = &ets_config;
    }
    ETS_OM_ENTER_CRITICAL_SECTION();
    memcpy(entry, &ets_db->port_entry[lport-1],  sizeof(ETS_TYPE_PortEntry_T));
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

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
BOOL_T ETS_OM_Is_Config_Match_Operation(UI32_T lport)
{
    ETS_TYPE_PortEntry_T *oper_entry = &ets_oper.port_entry[lport-1];
    ETS_TYPE_PortEntry_T *cfg_entry = &ets_config.port_entry[lport-1];
    int ret;


    ETS_OM_ENTER_CRITICAL_SECTION();
    ret = memcmp(oper_entry, cfg_entry,  sizeof(ETS_TYPE_PortEntry_T));
    ETS_OM_LEAVE_CRITICAL_SECTION();

    return ((ret==0)?TRUE:FALSE);
}

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
BOOL_T ETS_OM_SetTSA(UI32_T lport,
                                UI32_T tc, ETS_TYPE_TSA_T tsa, ETS_TYPE_DB_T oper_cfg)
{
    ETS_TYPE_INFO_T *ets_db = &ets_oper;

    if(oper_cfg==ETS_TYPE_DB_CONFIG)
    {
        ets_db = &ets_config;
    }

    ETS_OM_ENTER_CRITICAL_SECTION();
    ets_db->port_entry[lport-1].tsa[tc]= tsa;
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}
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
BOOL_T ETS_OM_GetTSA(UI32_T lport,
                                UI32_T tc, ETS_TYPE_TSA_T *tsa, ETS_TYPE_DB_T oper_cfg)
{
    ETS_TYPE_INFO_T *ets_db = &ets_oper;
    if(oper_cfg==ETS_TYPE_DB_CONFIG)
    {
        ets_db = &ets_config;
    }

    ETS_OM_ENTER_CRITICAL_SECTION();
    *tsa = ets_db->port_entry[lport-1].tsa[tc];
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

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
BOOL_T ETS_OM_SetMode(UI32_T lport, ETS_TYPE_MODE_T mode)
{
    ETS_OM_ENTER_CRITICAL_SECTION();
    ets_config.mode[lport-1]= mode;
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_GetMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS mode on the interface.
 * INPUT   : lport -- which port to configure.
 * OUTPUT  : mode  -- which mode
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_GetMode(UI32_T lport, ETS_TYPE_MODE_T *mode)
{
    ETS_OM_ENTER_CRITICAL_SECTION();
    *mode = ets_config.mode[lport-1];
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

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
BOOL_T ETS_OM_SetOperMode(UI32_T lport, ETS_TYPE_MODE_T mode)
{
    ETS_OM_ENTER_CRITICAL_SECTION();
    ets_oper.mode[lport-1]  = mode;
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_GetOperMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get ETS operation mode on the interface.
 * INPUT   : lport -- which port to configure.
 * OUTPUT  : mode  -- which mode
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_GetOperMode(UI32_T lport, ETS_TYPE_MODE_T *mode)
{
    ETS_OM_ENTER_CRITICAL_SECTION();
    *mode = ets_oper.mode[lport-1] ;
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

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
BOOL_T ETS_OM_SetPortPrioAssign(UI32_T lport,
                                UI32_T prio, UI32_T tc, ETS_TYPE_DB_T oper_cfg)
{
    ETS_TYPE_INFO_T *ets_db = &ets_oper;
    if(oper_cfg==ETS_TYPE_DB_CONFIG)
    {
        ets_db = &ets_config;
    }
    ETS_OM_ENTER_CRITICAL_SECTION();
    ets_db->port_entry[lport-1].priority_assign[prio]=tc;
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

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
BOOL_T ETS_OM_GetPortPrioAssign(UI32_T lport,
                                UI32_T prio, UI32_T* tc, ETS_TYPE_DB_T oper_cfg)
{
    ETS_TYPE_INFO_T *ets_db = &ets_oper;
    if(oper_cfg==ETS_TYPE_DB_CONFIG)
    {
        ets_db = &ets_config;
    }
    ETS_OM_ENTER_CRITICAL_SECTION();
    *tc = ets_db->port_entry[lport-1].priority_assign[prio];
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_SetWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Change BW weighting of all tc in a port
 * INPUT   : lport  - which port
 *           weight - BW weighting of all tc
 *           oper_cfg - operation or configuration
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_SetWeight(UI32_T lport,
                UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS], ETS_TYPE_DB_T oper_cfg)
{
    ETS_TYPE_INFO_T *ets_db = &ets_oper;
    UI32_T i;

    if(oper_cfg==ETS_TYPE_DB_CONFIG)
    {
        ets_db = &ets_config;
    }

    /* update OM.
     */
    ETS_OM_ENTER_CRITICAL_SECTION();
    for(i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
    {
        ets_db->port_entry[lport-1].tc_weight[i] = weight[i];
    }
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_GetWeight
 * -------------------------------------------------------------------------
 * FUNCTION: Get BW weighting of all tc in a port
 * INPUT   : lport  - which port
 * OUTPUT  : weight - BW weighting of all tc
 * RETURN  : Always TRUE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_GetWeight(UI32_T lport,
                UI32_T weight[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS], ETS_TYPE_DB_T oper_cfg)
{
    ETS_TYPE_INFO_T *ets_db = &ets_oper;
    UI32_T i;

    if(oper_cfg==ETS_TYPE_DB_CONFIG)
    {
        ets_db = &ets_config;
    }

    ETS_OM_ENTER_CRITICAL_SECTION();
    for(i=0; i<SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; i++)
    {
        weight[i] = ets_db->port_entry[lport-1].tc_weight[i];
    }
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_GetIsCosConfigOk
 * -------------------------------------------------------------------------
 * FUNCTION: To get is_cos_config_global flag
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : is_cos_config_global flag
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_GetIsCosConfigOk(void)
{
    BOOL_T  ret;

    ETS_OM_ENTER_CRITICAL_SECTION();
    ret = ets_om_is_cos_config_ok;
    ETS_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_SetIsCosConfigOk
 * -------------------------------------------------------------------------
 * FUNCTION: To set is_cos_ok flag
 * INPUT   : new_is_cos_ok -
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_SetIsCosConfigOk(
    BOOL_T  new_is_cos_ok)
{
    /* update OM.
     */
    ETS_OM_ENTER_CRITICAL_SECTION();
    ets_om_is_cos_config_ok = new_is_cos_ok;
    ETS_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_IsAnyPortOperEnabled
 * -------------------------------------------------------------------------
 * FUNCTION: To check is any port operational enabled
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_IsAnyPortOperEnabled(void)
{
    UI32_T  lport;
    BOOL_T  ret = FALSE;

    ETS_OM_ENTER_CRITICAL_SECTION();

    for (lport =0; lport <SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        if (ETS_TYPE_MODE_OFF != ets_oper.mode[lport])
        {
            ret = TRUE;
            break;
        }
    }

    ETS_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_OM_RestoreToConfig
 * -------------------------------------------------------------------------
 * FUNCTION: sync operational OM to configuration one.
 * INPUT   : lport  - which port
 * OUTPUT  : None
 * RETURN  : Always TRUE
 * NOTE    : the mode is not restored for it always be the same between oper and cfg.
 * -------------------------------------------------------------------------*/
BOOL_T ETS_OM_RestoreToConfig(UI32_T lport)
{
    ETS_OM_ENTER_CRITICAL_SECTION();
    memcpy(&ets_oper.port_entry[lport-1], &ets_config.port_entry[lport-1], sizeof(ETS_TYPE_PortEntry_T) );
    ETS_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


/* LOCAL SUBPROGRAM BODIES
 */

#endif /* #if (SYS_CPNT_ETS == TRUE) */


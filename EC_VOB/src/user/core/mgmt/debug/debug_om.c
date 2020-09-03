/* MODULE NAME: DEBUG_OM.C
 * PURPOSE:
 *   Implementation for the DEBUG object manager
 * NOTES:
 *
 * HISTORY:
 *   12/12/2007 -- Kelly Chen, Create
 *   03/10/2008 -- Duo Chen, Modify
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "sysfun.h"
#include "sys_adpt.h"
#include "debug_om.h"
#include "debug_type.h"
#include "sys_bld.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */
#define DEBUG_OM_LOCK(orig_priority)    \
        (orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(debug_om_sem_id))
#define DEBUG_OM_UNLOCK(orig_priority)  \
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(debug_om_sem_id,orig_priority)

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    UI16_T reserved;

    union
    {
        UI16_T full_debug_flag;
        struct
        {
            UI8_T class;
            UI8_T feature;
        }debug_flag;
    }u;

} DEBUG_OM_Flag_T;

typedef struct
{
    DEBUG_OM_Flag_T sess_table[DEBUG_TYPE_MAX_SESSION_NUM];
} DEBUG_OM_FlagSess_T;

typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    DEBUG_OM_FlagSess_T flag_table[DEBUG_TYPE_MAX_CSC_NUM];
} DEBUG_OM_ShmemData_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* FUNCTION NAME: DEBUG_OM_CmpCscFlag
 * PURPOSE: compare flag by csc.
 * INPUT  : matched_way -- the way to match Debug om value when CSCs want to
 *          print.
 *          class_mask -- setting flag value.
 *          feature_mask -- setting flag value, it's null when no use.
 *          om_flag -- om's flag value.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
static BOOL_T
DEBUG_OM_CmpCscFlag(
    DEBUG_TYPE_Matched_T matched_way,
    UI32_T class_mask,
    UI32_T feature_mask,
    UI32_T om_flag
);

/* STATIC VARIABLE DEFINITIONS
 */
static UI32_T debug_om_sem_id;
static DEBUG_OM_ShmemData_T *debug_om_shmem_data_p;


/* FUNCTION NAME: DEBUG_OM_Get_Operating_Mode_On_Shmem
 * PURPOSE: DEBUG_OM_Get_Operating_Mode_On_Shmem
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
SYS_TYPE_Stacking_Mode_T DEBUG_OM_Get_Operating_Mode_On_Shmem(void)
{
	return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(debug_om_shmem_data_p);
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: DEBUG_OM_InitiateSystemResources
 * PURPOSE: Initialize DEBUG database
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
void  DEBUG_OM_InitiateSystemResources(void)
{
 	debug_om_shmem_data_p =
    (DEBUG_OM_ShmemData_T *)SYSRSC_MGR_GetShMem(SYSRSC_MGR_DEBUG_OM_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(debug_om_shmem_data_p);
}

/* FUNCTION NAME: DEBUG_OM_AttachSystemResources
 * PURPOSE: Attach system resource for DEBUG in the context of the
 *          calling process.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
void DEBUG_OM_AttachSystemResources(void)
{
    debug_om_shmem_data_p =
    (DEBUG_OM_ShmemData_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_DEBUG_OM_SHMEM_SEGID);
    if(SYSFUN_OK!=
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_DEBUG_OM, &debug_om_sem_id))
    printf("%s(): SYSFUN_GetSem fails.\n", __FUNCTION__);
}

/* FUNCTION NAME: DEBUG_OM_GetShMemInfo
 * PURPOSE: Provide shared memory information of DEBUG for SYSRSC.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
void DEBUG_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_DEBUG_OM_SHMEM_SEGID;
    *seglen_p = sizeof(*debug_om_shmem_data_p);
}

/* FUNCTION NAME: DEBUG_OM_SetTransitionMode
 * PURPOSE: This function will set the transition mode
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
void DEBUG_OM_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(debug_om_shmem_data_p);
}

/* FUNCTION NAME: DEBUG_OM_EnterTransitionMode
 * PURPOSE: This function will initialize all system resource
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
void DEBUG_OM_EnterTransitionMode(void)
{
    memset(debug_om_shmem_data_p, 0, sizeof(*debug_om_shmem_data_p));
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(debug_om_shmem_data_p);
}

/* FUNCTION NAME: DEBUG_OM_EnterMasterMode
 * PURPOSE: This function will enable address monitor services
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
void DEBUG_OM_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(debug_om_shmem_data_p);
}

/* FUNCTION NAME: DEBUG_OM_EnterSlaveMode
 * PURPOSE: This function will disable address monitor services
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
void DEBUG_OM_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(debug_om_shmem_data_p);
}


/* FUNCTION NAME: DEBUG_OM_Init
 * PURPOSE: Initialize DEBUG database
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : None.
 */
void  DEBUG_OM_Init(void)
{
    memset( debug_om_shmem_data_p->flag_table
        , 0
        , sizeof(debug_om_shmem_data_p->flag_table) );
}

/* FUNCTION NAME: DEBUG_OM_SetModuleFlag
 * PURPOSE: Set debug flag for specified CSC module.
 * INPUT  : csc_id  -- setting CSC name.
 *          session_id -- which session would be printed out.
 *          flag -- setting flag value.
 * OUTPUT : None.
 * RETURN : None.
 * NOTES  : Just set flag to OM table.
 *          Don't need return value because DEBUG_MGR will check input argument
 *          is valid or not.
 */
DEBUG_TYPE_ReturnValue_T
DEBUG_OM_SetModuleFlag(
    DEBUG_TYPE_CscId_E csc_id,
    UI32_T session_id,
    UI32_T flag)
{
    UI32_T orig_priority;

    if(sizeof(DEBUG_OM_Flag_T) != sizeof(flag))
    {
        assert(0);
        return DEBUG_TYPE_RETURN_FAIL;
    }

    if((csc_id >= DEBUG_TYPE_MAX_CSC_NUM) ||
            (session_id >= DEBUG_TYPE_MAX_SESSION_NUM))
    {
        return DEBUG_TYPE_RETURN_INVALID_ARG;
    }

    DEBUG_OM_LOCK(orig_priority);
    memcpy(&(debug_om_shmem_data_p->flag_table[csc_id].sess_table[session_id])
        , &flag,
        sizeof(debug_om_shmem_data_p->flag_table[csc_id].sess_table[session_id]));
    DEBUG_OM_UNLOCK(orig_priority);
    return DEBUG_TYPE_RETURN_OK;
}

/* FUNCTION NAME: DEBUG_OM_GetModuleFlag
 * PURPOSE: Get debug flag for specified CSC module.
 * INPUT  : csc_id  -- setting CSC name.
 *          session_id -- which session would be printed out.
 * OUTPUT : flag -- setting flag value.
 * RETURN : None.
 * NOTES  : Just get flag from OM table.
 *          Don't need return value because DEBUG_MGR will check input argument
 *          is valid or not.
 */
DEBUG_TYPE_ReturnValue_T
DEBUG_OM_GetModuleFlag(
    DEBUG_TYPE_CscId_E csc_id,
    UI32_T session_id,
    UI32_T *flag_p)
{
    UI32_T orig_priority;

    if(sizeof(DEBUG_OM_Flag_T) != sizeof(*flag_p))
    {
        assert(0);
        return DEBUG_TYPE_RETURN_FAIL;
    }

    if((csc_id >= DEBUG_TYPE_MAX_CSC_NUM) ||
        (session_id >= DEBUG_TYPE_MAX_SESSION_NUM))
    {
        return DEBUG_TYPE_RETURN_INVALID_ARG;
    }

    DEBUG_OM_LOCK(orig_priority);
    memcpy(flag_p,
        &(debug_om_shmem_data_p->flag_table[csc_id].sess_table[session_id]),
        sizeof(*flag_p));
    DEBUG_OM_UNLOCK(orig_priority);
    return DEBUG_TYPE_RETURN_OK;
}

/* FUNCTION NAME: DEBUG_MGR_IsPrintable
 * PURPOSE: Detect if flag is on or off by session.
 * INPUT  : csc_id  -- setting CSC name, defined in debug_type.h.
 *          session_id -- which session would be printed out.
 *          matched_way -- the way to match Debug om value when CSCs want to print.
 *          class -- setting flag value.
 *          feature_mask -- setting flag value, it's null when no use.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */

BOOL_T
DEBUG_OM_IsPrintable(
    DEBUG_TYPE_CscId_E csc_id,
    UI32_T session_id,
    DEBUG_TYPE_Matched_T matched_way,
    UI32_T class,
    UI32_T feature)
{
    UI32_T om_flag;

    if ( (csc_id >= DEBUG_TYPE_MAX_CSC_NUM)
        || (session_id >= DEBUG_TYPE_MAX_SESSION_NUM) )
    {
        return FALSE;
    }

    DEBUG_OM_GetModuleFlag(csc_id,session_id, &om_flag);
    if(DEBUG_OM_CmpCscFlag(matched_way, class, feature, om_flag))
    {
        return TRUE;
    }
    return FALSE;
}


/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: DEBUG_MGR_CmpCscFlag
 * PURPOSE: compare flag by csc.
 * INPUT  : matched_way -- the way to match Debug om value when CSCs want to print.
 *          class -- setting flag value.
 *          feature_mask -- setting flag value, it's null when no use.
 *          om_flag -- om's flag value.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */

static BOOL_T
DEBUG_OM_CmpCscFlag(
    DEBUG_TYPE_Matched_T matched_way,
    UI32_T class,
    UI32_T feature_mask,
    UI32_T om_flag)
{
    switch (matched_way)
    {
    case DEBUG_TYPT_MATCH_ALL:
        if(class == (om_flag&class))
        {
            return TRUE;
        }
        return FALSE;

    case DEBUG_TYPE_MATCH_ANY:
        if(0!= (om_flag&class))
        {
            return TRUE;
        }

    case DEBUG_TYPE_MATCH_ALL_ANY:
        if( (class == (om_flag&class)) && (0!= (om_flag&feature_mask)) )
        {
            return TRUE;
        }
        return FALSE;

    case DEBUG_TYPE_MATCH_ANY_ANY:
        if( (0!= (om_flag&class)) && (0!= (om_flag&feature_mask)) )
        {
            return TRUE;
        }

    default:
        return FALSE;
    }
}

/* LOCAL SUBPROGRAM BODIES
 */

/* MODULE NAME - CN_OM.C
 * PURPOSE : Provides the definitions for CN database management.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"
#include "cn_om.h"
#include "cn_om_private.h"
#include "cn_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

#define CN_OM_ENTER_CRITICAL_SECTION() \
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(om_sem_id)

#define CN_OM_LEAVE_CRITICAL_SECTION() \
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(om_sem_id, orig_priority)

/* DATA TYPE DECLARATIONS
 */

typedef struct CN_OM_ShmemData_S
{
    CN_OM_GlobalData_T  global_data;
    UI32_T              active_pri_count;
    CN_OM_PriData_T     pri_list[CN_TYPE_MAX_PRIORITY+1];
    CN_OM_PortPriData_T port_pri_list[CN_TYPE_MAX_PRIORITY+1][SYS_ADPT_TOTAL_NBR_OF_LPORT];
    CN_OM_CpData_T      cp_list[SYS_ADPT_TOTAL_NBR_OF_LPORT][SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT];
} CN_OM_ShmemData_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DEFINITIONS
 */

static CN_OM_ShmemData_T    *shmem_data_p;
static UI32_T   om_sem_id;
static UI32_T   orig_priority;

static BOOL_T   debug_err;
static char     *err_str[] = {
    [CN_TYPE_RETURN_ERROR]                      = "general",
    [CN_TYPE_RETURN_ERROR_MASTER_MODE]          = "master mode",
    [CN_TYPE_RETURN_ERROR_GLOBAL_STATUS_RANGE]  = "global status range",
    [CN_TYPE_RETURN_ERROR_PRIORITY_RANGE]       = "priority range",
    [CN_TYPE_RETURN_ERROR_ACTIVE_RANGE]         = "active range",
    [CN_TYPE_RETURN_ERROR_CNPV_EXISTENCE]       = "CNPV entry existence",
    [CN_TYPE_RETURN_ERROR_CNPV_CAPACITY]        = "CNPV capacity",
    [CN_TYPE_RETURN_ERROR_MODE_RANGE]           = "defense mode range",
    [CN_TYPE_RETURN_ERROR_ALT_PRIORITY_RANGE]   = "alternate priority range",
    [CN_TYPE_RETURN_ERROR_LPORT_RANGE]          = "logical port range",
    [CN_TYPE_RETURN_ERROR_PORT_CNPV_EXISTENCE]  = "port-cnpv entry existence",
    [CN_TYPE_RETURN_ERROR_CP_INDEX_RANGE]       = "CP index range",
    [CN_TYPE_RETURN_ERROR_CP_EXISTENCE]         = "CP entry existence",
    [CN_TYPE_RETURN_ERROR_CP_CAPACITY]          = "CP capacity",
    [CN_TYPE_RETURN_ERROR_NULL_POINTER]         = "null pointer",
    [CN_TYPE_RETURN_ERROR_PFC_CONFLICT]         = "PFC conflict",
    [CN_TYPE_RETURN_ERROR_ETS_CONFLICT]         = "ETS conflict"
    };

static BOOL_T   debug_thread;

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME - CN_OM_InitiateSystemResources
 * PURPOSE : Initiate system resources.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_OM_InitiateSystemResources(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return;
} /* End of CN_OM_InitiateSystemResources */

/* FUNCTION NAME - CN_OM_AttachSystemResources
 * PURPOSE : Attach system resources for CN in the context of the calling
 *           process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_OM_AttachSystemResources(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    shmem_data_p = (CN_OM_ShmemData_T*)SYSRSC_MGR_GetShMem(
                                            SYSRSC_MGR_CN_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_CN_OM, &om_sem_id);
    return;
} /* End of CN_OM_AttachSystemResources */

/* FUNCTION NAME - CN_OM_GetShMemInfo
 * PURPOSE : Provide shared memory information of CN for SYSRSC_MGR.
 * INPUT   : None
 * OUTPUT  : segid_p  - shared memory segment id
 *           seglen_p - length of the shared memroy segment
 * RETURN  : None
 * NOTES   : This function is called in SYSRSC_MGR_CreateSharedMemory().
 */
void CN_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    *segid_p  = SYSRSC_MGR_CN_SHMEM_SEGID;
    *seglen_p = sizeof(CN_OM_ShmemData_T);
    return;
} /* End of CN_OM_GetShMemInfo */

/* FUNCTION NAME - CN_OM_ResetAll
 * PURPOSE : Reset all CN data.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_OM_ResetAll(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    memset(shmem_data_p, 0, sizeof(CN_OM_ShmemData_T));
    CN_OM_LEAVE_CRITICAL_SECTION();
    debug_err = FALSE;
    debug_thread = FALSE;
    return;
} /* End of CN_OM_ResetAll */

/* FUNCTION NAME - CN_OM_DefaultAll
 * PURPOSE : Default all CN data.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_OM_DefaultAll(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  i, j;

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();

    shmem_data_p->global_data.admin_status = SYS_DFLT_CN_GLOBAL_ADMIN_STATUS;
    shmem_data_p->global_data.cnm_tx_priority =
        SYS_DFLT_CN_CNM_TRANSMIT_PRIORITY;

    for (i = 0; i <= CN_TYPE_MAX_PRIORITY; i++)
    {
        shmem_data_p->pri_list[i].active = FALSE;
        shmem_data_p->pri_list[i].defense_mode = SYS_DFLT_CN_CNPV_DEFENSE_MODE;
        shmem_data_p->pri_list[i].admin_alt_priority =
            SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY;
        shmem_data_p->pri_list[i].auto_alt_priority =
            SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY;

        for (j = 0; j < SYS_ADPT_TOTAL_NBR_OF_LPORT; j++)
        {
            shmem_data_p->port_pri_list[i][j].active = FALSE;
            shmem_data_p->port_pri_list[i][j].admin_defense_mode =
                CN_TYPE_DEFENSE_MODE_BY_GLOBAL;
            shmem_data_p->port_pri_list[i][j].oper_defense_mode =
                CN_TYPE_DEFENSE_MODE_DISABLED;
            shmem_data_p->port_pri_list[i][j].admin_alt_priority =
                CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL;
            shmem_data_p->port_pri_list[i][j].oper_alt_priority =
                SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY;
            shmem_data_p->port_pri_list[i][j].tx_ready = FALSE;
        }
    }

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        for (j = 0; j < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; j++)
        {
            shmem_data_p->cp_list[i][j].active = FALSE;
            shmem_data_p->cp_list[i][j].queue = CN_TYPE_MAX_PRIORITY+1;
            shmem_data_p->cp_list[i][j].managed_cnpvs = 0;
            shmem_data_p->cp_list[i][j].set_point = CN_TYPE_DEFAULT_SET_POINT;
            shmem_data_p->cp_list[i][j].feedback_weight =
                CN_TYPE_DEFAULT_FEEDBACK_WEIGHT;
            shmem_data_p->cp_list[i][j].min_sample_base =
                CN_TYPE_DEFAULT_MIN_SAMEPLE_BASE;
        }
    }

    CN_OM_LEAVE_CRITICAL_SECTION();
    return;
} /* End of CN_OM_DefaultAll */

/* FUNCTION NAME - CN_OM_DefaultPort
 * PURPOSE : Default all port-pri data and cp data for a logical port.
 * INPUT   : lport - the specified logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_OM_DefaultPort(UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  i;

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();

    for (i = 0; i <= CN_TYPE_MAX_PRIORITY; i++)
    {
        shmem_data_p->port_pri_list[i][lport-1].active = FALSE;
        shmem_data_p->port_pri_list[i][lport-1].admin_defense_mode =
            CN_TYPE_DEFENSE_MODE_BY_GLOBAL;
        shmem_data_p->port_pri_list[i][lport-1].oper_defense_mode =
            CN_TYPE_DEFENSE_MODE_DISABLED;
        shmem_data_p->port_pri_list[i][lport-1].admin_alt_priority =
            CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL;
        shmem_data_p->port_pri_list[i][lport-1].oper_alt_priority =
            SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY;
        shmem_data_p->port_pri_list[i][lport-1].tx_ready = FALSE;
    }

    for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
    {
        shmem_data_p->cp_list[lport-1][i].active = FALSE;
        shmem_data_p->cp_list[lport-1][i].queue = CN_TYPE_MAX_PRIORITY+1;
        shmem_data_p->cp_list[lport-1][i].managed_cnpvs = 0;
        shmem_data_p->cp_list[lport-1][i].set_point = CN_TYPE_DEFAULT_SET_POINT;
        shmem_data_p->cp_list[lport-1][i].feedback_weight =
            CN_TYPE_DEFAULT_FEEDBACK_WEIGHT;
        shmem_data_p->cp_list[lport-1][i].min_sample_base =
            CN_TYPE_DEFAULT_MIN_SAMEPLE_BASE;
    }

    CN_OM_LEAVE_CRITICAL_SECTION();
    return;
} /* End of CN_OM_DefaultPort */

/* FUNCTION NAME - CN_OM_GetGlobalAdminStatus
 * PURPOSE : Get CN global admin status.
 * INPUT   : None
 * OUTPUT  : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetGlobalAdminStatus(UI32_T *status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (status == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *status = shmem_data_p->global_data.admin_status;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetGlobalAdminStatus */

/* FUNCTION NAME - CN_OM_GetGlobalOperStatus
 * PURPOSE : Get CN global oper status.
 * INPUT   : None
 * OUTPUT  : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetGlobalOperStatus(UI32_T *status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (status == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *status = shmem_data_p->global_data.oper_status;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetGlobalOperStatus */

/* FUNCTION NAME - CN_OM_GetRunningGlobalAdminStatus
 * PURPOSE : Get running CN global admin status.
 * INPUT   : None
 * OUTPUT  : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningGlobalAdminStatus(UI32_T *status)
{

    if (status == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *status = shmem_data_p->global_data.admin_status;
    CN_OM_LEAVE_CRITICAL_SECTION();

    if (*status == SYS_DFLT_CN_GLOBAL_ADMIN_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* End of CN_OM_GetRunningGlobalAdminStatus */

/* FUNCTION NAME - CN_OM_SetGlobalAdminStatus
 * PURPOSE : Set CN global admin status.
 * INPUT   : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetGlobalAdminStatus(UI32_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->global_data.admin_status = status;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetGlobalAdminStatus */

/* FUNCTION NAME - CN_OM_SetGlobalOperStatus
 * PURPOSE : Set CN global oper status.
 * INPUT   : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetGlobalOperStatus(UI32_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->global_data.oper_status = status;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetGlobalOperStatus */

/* FUNCTION NAME - CN_OM_GetCnmTxPriority
 * PURPOSE : Get the priority used for transmitting CNMs.
 * INPUT   : None
 * OUTPUT  : priority - the priority used for CNM transmission
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetCnmTxPriority(UI32_T *priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (priority == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *priority = shmem_data_p->global_data.cnm_tx_priority;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetCnmTxPriority */

/* FUNCTION NAME - CN_OM_GetRunningCnmTxPriority
 * PURPOSE : Get the running priority used for transmitting CNMs.
 * INPUT   : None
 * OUTPUT  : priority - the priority used for CNM transmission
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningCnmTxPriority(UI32_T *priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (priority == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *priority = shmem_data_p->global_data.cnm_tx_priority;
    CN_OM_LEAVE_CRITICAL_SECTION();

    if (*priority == SYS_DFLT_CN_CNM_TRANSMIT_PRIORITY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* End of CN_OM_GetRunningCnmTxPriority */

/* FUNCTION NAME - CN_OM_SetCnmTxPriority
 * PURPOSE : Set the priority used for transmitting CNMs.
 * INPUT   : priority - priority in the range between 0 and 7
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetCnmTxPriority(UI32_T priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->global_data.cnm_tx_priority = priority;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetCnmTxPriority */

/* FUNCTION NAME - CN_OM_GetNextCnpv
 * PURPOSE : Get the next CNPV to a given priority.
 * INPUT   : priority - the starting priority for searching
 * OUTPUT  : priority - the CNPV next to the given priority
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetNextCnpv(UI32_T *priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  i;

    /* BODY
     */

    if (priority == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    if (    (*priority != CN_TYPE_START_PRIORITY_FOR_SEARCHING)
         && (    (*priority < CN_TYPE_MIN_PRIORITY)
              || (*priority > CN_TYPE_MAX_PRIORITY)
            )
       )
    {
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    CN_OM_ENTER_CRITICAL_SECTION();

    if (*priority == CN_TYPE_START_PRIORITY_FOR_SEARCHING)
    {
        i = 0;
    }
    else
    {
        i = *priority + 1;
    }

    for (; i <= CN_TYPE_MAX_PRIORITY; i++)
    {
        if (shmem_data_p->pri_list[i].active == TRUE)
        {
            *priority = i;
            break;
        }
    }

    CN_OM_LEAVE_CRITICAL_SECTION();

    if (i > CN_TYPE_MAX_PRIORITY)
    {
        return CN_TYPE_RETURN_ERROR;
    }
    else
    {
        return CN_TYPE_RETURN_OK;
    }
} /* End of CN_OM_GetNextCnpv */

/* FUNCTION NAME - CN_OM_SetPriStatus
 * PURPOSE : Set a priority to be CNPV or not.
 * INPUT   : priority - the specify priority
 *           active   - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPriStatus(UI32_T priority, BOOL_T active)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (    (active == TRUE)
         && (shmem_data_p->active_pri_count == CN_TYPE_MAX_NBR_OF_CNPV)
       )
    {
        return CN_TYPE_RETURN_ERROR_CNPV_CAPACITY;
    }

    CN_OM_ENTER_CRITICAL_SECTION();

    if (active == TRUE)
    {
        shmem_data_p->active_pri_count++;
    }
    else
    {
        shmem_data_p->active_pri_count--;
    }
    shmem_data_p->pri_list[priority].active = active;

    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetPriStatus */

/* FUNCTION NAME - CN_OM_GetPriDefenseMode
 * PURPOSE : Get the defense mode for a priority.
 * INPUT   : priority - the specified priority
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetPriDefenseMode(UI32_T priority, UI32_T *mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (mode == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *mode = shmem_data_p->pri_list[priority].defense_mode;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetPriDefenseMode */

/* FUNCTION NAME - CN_OM_GetRunningPriDefenseMode
 * PURPOSE : Get the running defense mode for a priority.
 * INPUT   : priority - the specified CNPV
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningPriDefenseMode(UI32_T priority,
                                UI32_T *mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (mode == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *mode = shmem_data_p->pri_list[priority].defense_mode;
    CN_OM_LEAVE_CRITICAL_SECTION();

    if (*mode == SYS_DFLT_CN_CNPV_DEFENSE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* End of CN_OM_GetRunningPriDefenseMode */

/* FUNCTION NAME - CN_OM_SetPriDefenseMode
 * PURPOSE : Set the defense mode for a priority.
 * INPUT   : priority - the specify priority
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPriDefenseMode(UI32_T priority, UI32_T mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->pri_list[priority].defense_mode = mode;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetPriDefenseMode */

/* FUNCTION NAME - CN_OM_GetPriAlternatePriority
 * PURPOSE : Get the admin alternate priority used for a priority.
 * INPUT   : priority - the specified priority
 * OUTPUT  : alt_priority - the alternate priority for a priority
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetPriAlternatePriority(UI32_T priority, UI32_T *alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (alt_priority == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *alt_priority = shmem_data_p->pri_list[priority].admin_alt_priority;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetPriAlternatePriority */

/* FUNCTION NAME - CN_OM_GetRunningPriAlternatePriority
 * PURPOSE : Get the running admin alternate priority used for a priority.
 * INPUT   : priority - the specified priority
 * OUTPUT  : alt_priority - the alternate priority for a priority
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningPriAlternatePriority(UI32_T priority,
                                UI32_T *alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (alt_priority == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *alt_priority = shmem_data_p->pri_list[priority].admin_alt_priority;
    CN_OM_LEAVE_CRITICAL_SECTION();

    if (*alt_priority == SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* End of CN_OM_GetRunningPriAlternatePriority */

/* FUNCTION NAME - CN_OM_SetPriAlternatePriority
 * PURPOSE : Set the alternate priority used for a priority..
 * INPUT   : priority     - the specify priority
 *           alt_priority - the alternate priority for a priority
 *           is_admin - whether it is admin or auto
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPriAlternatePriority(UI32_T priority, UI32_T alt_priority,
            BOOL_T is_admin)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    if (is_admin == TRUE)
    {
        shmem_data_p->pri_list[priority].admin_alt_priority = alt_priority;
    }
    else
    {
        shmem_data_p->pri_list[priority].auto_alt_priority = alt_priority;
    }
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetPriAlternatePriority */

/* FUNCTION NAME - CN_OM_SetPortPriStatus
 * PURPOSE : Set a priority to be CNPV or not on a port.
 * INPUT   : priority - the specify priority
 *           lport    - the specified logical port
 *           active   - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPortPriStatus(UI32_T priority, UI32_T lport, BOOL_T active)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_pri_list[priority][lport-1].active = active;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetPortPriStatus */

/* FUNCTION NAME - CN_OM_GetPortPriDefenseMode
 * PURPOSE : Get the admin defense mode for a priority on a port.
 * INPUT   : priority - the specified priority
 *           lport    - the specified logical port
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetPortPriDefenseMode(UI32_T priority, UI32_T lport, UI32_T *mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (mode == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    if ((lport < 1) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return CN_TYPE_RETURN_ERROR_LPORT_RANGE;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *mode = shmem_data_p->port_pri_list[priority][lport-1].admin_defense_mode;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetPortPriDefenseMode */

/* FUNCTION NAME - CN_OM_GetRunningPortPriDefenseMode
 * PURPOSE : Get the running admin defense mode for a priority on a port.
 * INPUT   : priority - the specified priority
 *           lport    - the specified logical port
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningPortPriDefenseMode(UI32_T priority,
                            UI32_T lport, UI32_T *mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (mode == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if ((lport < 1) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *mode = shmem_data_p->port_pri_list[priority][lport-1].admin_defense_mode;
    CN_OM_LEAVE_CRITICAL_SECTION();

    if (*mode == CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* End of CN_OM_GetRunningPortPriDefenseMode */

/* FUNCTION NAME - CN_OM_SetPortPriDefenseMode
 * PURPOSE : Set the defense mode for a priority on a port.
 * INPUT   : priority - the specify priority
 *           lport    - the specified logical port
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 *           is_admin - whether it is admin or oper
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPortPriDefenseMode(UI32_T priority, UI32_T lport, UI32_T mode,
        BOOL_T is_admin)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    if (is_admin == TRUE)
    {
        shmem_data_p->port_pri_list[priority][lport-1].admin_defense_mode = mode;
    }
    else
    {
        shmem_data_p->port_pri_list[priority][lport-1].oper_defense_mode = mode;
    }
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetPortPriDefenseMode */

/* FUNCTION NAME - CN_OM_GetPortPriAlternatePriority
 * PURPOSE : Get the admin alternate priority used for a priority on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : alt_priority - the alternate priority for a priority on a port
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetPortPriAlternatePriority(UI32_T priority, UI32_T lport,
        UI32_T *alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (alt_priority == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    if ((lport < 1) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return CN_TYPE_RETURN_ERROR_LPORT_RANGE;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *alt_priority =
        shmem_data_p->port_pri_list[priority][lport-1].admin_alt_priority;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetPortPriAlternatePriority */

/* FUNCTION NAME - CN_OM_GetRunningPortPriAlternatePriority
 * PURPOSE : Get the running admin alternate priority used for a priority on a
 *           port.
 * INPUT   : priority - the specified priority
 *           lport    - the specified logical port
 * OUTPUT  : alt_priority - the alternate priority for a priority on a port
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningPortPriAlternatePriority(
                            UI32_T priority, UI32_T lport, UI32_T *alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (alt_priority == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if ((lport < 1) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *alt_priority =
        shmem_data_p->port_pri_list[priority][lport-1].admin_alt_priority;
    CN_OM_LEAVE_CRITICAL_SECTION();

    if (*alt_priority == CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* End of CN_OM_GetRunningPortPriAlternatePriority */

/* FUNCTION NAME - CN_OM_SetPortPriAlternatePriority
 * PURPOSE : Set the alternate priority used for a priority on a port.
 * INPUT   : priority     - the specify priority
 *           lport        - the specified logical port
 *           alt_priority - the specified alternate priority
 *           is_admin     - whether it is admin or oper
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPortPriAlternatePriority(UI32_T priority, UI32_T lport,
        UI32_T alt_priority, BOOL_T is_admin)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    if (is_admin == TRUE)
    {
        shmem_data_p->port_pri_list[priority][lport-1].admin_alt_priority =
            alt_priority;
    }
    else
    {
        shmem_data_p->port_pri_list[priority][lport-1].oper_alt_priority =
            alt_priority;
    }
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetPortPriAlternatePriority */

/* FUNCTION NAME - CN_OM_GetTxReady
 * PURPOSE : Get the ready value for CN TLV Ready Indicators.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : ready - whether priority remap defense has been disabled
 * RETURN  : None
 * NOTES   : IEEE 802.1Qau-2010 32.4.8
 */
UI32_T CN_OM_GetTxReady(UI32_T priority, UI32_T lport, BOOL_T *ready)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (ready == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    if ((lport < 1) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return CN_TYPE_RETURN_ERROR_LPORT_RANGE;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    *ready = shmem_data_p->port_pri_list[priority][lport-1].tx_ready;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetTxReady */

/* FUNCTION NAME - CN_OM_SetTxReady
 * PURPOSE : Set the ready value for CN TLV Ready Indicators.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 *           ready    - whether priority remap defense has been disabled
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : IEEE 802.1Qau-2010 32.4.8
 */
UI32_T CN_OM_SetTxReady(UI32_T priority, UI32_T lport, BOOL_T ready)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_pri_list[priority][lport-1].tx_ready = ready;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetTxReady */

/* FUNCTION NAME - CN_OM_SetCpStatus
 * PURPOSE : Set the status of a CP.
 * INPUT   : lport  - the specified logical port
 *           index  - the index of the CP
 *           active - to be active or not
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetCpStatus(UI32_T lport, UI32_T index, BOOL_T active)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->cp_list[lport-1][index].active = active;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetCpStatus */

/* FUNCTION NAME - CN_OM_SetCpMappedQueue
 * PURPOSE : Set the mapped queue for a CP.
 * INPUT   : lport - the specified logical port
 *           index - the index of the CP
 *           queue - queue value to be mapped with the CP
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetCpMappedQueue(UI32_T lport, UI32_T index, UI8_T queue)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->cp_list[lport-1][index].queue = queue;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetCpMappedQueue */

/* FUNCTION NAME - CN_OM_SetCpManagedCnpvs
 * PURPOSE : Set the bitmap of CNPVs managed by a CP.
 * INPUT   : lport - the specified logical port
 *           index - the index of the CP
 *           cnpvs - the bitmap of CNPVs managed by a CP
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetCpManagedCnpvs(UI32_T lport, UI32_T index, UI8_T cnpvs)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    CN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->cp_list[lport-1][index].managed_cnpvs = cnpvs;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_SetCpManagedCnpvs */

/* FUNCTION NAME - CN_OM_IsCnpv
 * PURPOSE : Check whether a priority is a CNPV.
 * INPUT   : priority - the priority to be checked
 * OUTPUT  : None
 * RETURN  : TRUE  - the priority is a CNPV
 *           FALSE - the priority is not a CNPV
 * NOTES   : None
 */
BOOL_T CN_OM_IsCnpv(UI32_T priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    BOOL_T result;

    /* BODY
     */

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return FALSE;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    result = shmem_data_p->pri_list[priority].active;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of CN_OM_IsCnpv */

/* FUNCTION NAME - CN_OM_IsCp
 * PURPOSE : Check whether it is a CP given port and index.
 * INPUT   : lport - the lport where the CP is located
 *           index - the index of the CP
 * OUTPUT  : None
 * RETURN  : TRUE  - it is a CP
 *           FALSE - it is not a CP
 * NOTES   : None
 */
BOOL_T CN_OM_IsCp(UI32_T lport, UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    BOOL_T result;

    /* BODY
     */

    if ((lport < 1) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return CN_TYPE_RETURN_ERROR_LPORT_RANGE;
    }

    if (index >= SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT)
    {
        return CN_TYPE_RETURN_ERROR_CP_INDEX_RANGE;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    result = shmem_data_p->cp_list[lport-1][index].active;
    CN_OM_LEAVE_CRITICAL_SECTION();
    return result;
} /* End of CN_OM_IsCp */

/* FUNCTION NAME - CN_OM_GetGlobalData
 * PURPOSE : Get the global data.
 * INPUT   : None
 * OUTPUT  : buffer - buffer to contain the global data
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Used by CN backdoor
 */
UI32_T CN_OM_GetGlobalData(CN_OM_GlobalData_T *buffer)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (buffer == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    memcpy(buffer, &shmem_data_p->global_data, sizeof(CN_OM_GlobalData_T));
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetGlobalData */

/* FUNCTION NAME - CN_OM_GetPriData
 * PURPOSE : Get data for a priority.
 * INPUT   : priority - the specified priority
 * OUTPUT  : buffer - buffer to contain the data for a priority
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Used by CN backdoor
 */
UI32_T CN_OM_GetPriData(UI32_T priority, CN_OM_PriData_T *buffer)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (buffer == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    memcpy(buffer, &shmem_data_p->pri_list[priority], sizeof(CN_OM_PriData_T));
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetPriData */

/* FUNCTION NAME - CN_OM_GetPortPriData
 * PURPOSE : Get the data for a priority on a port.
 * INPUT   : priority - the specified priority
 *           lport    - the specified logical port
 * OUTPUT  : buffer - buffer to contain the data for a priority on a port
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Used by CN backdoor
 */
UI32_T CN_OM_GetPortPriData(UI32_T priority, UI32_T lport,
        CN_OM_PortPriData_T *buffer)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (buffer == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    if ((lport < 1) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return CN_TYPE_RETURN_ERROR_LPORT_RANGE;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    memcpy(buffer, &shmem_data_p->port_pri_list[priority][lport-1],
        sizeof(CN_OM_PortPriData_T));
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetPortPriData */

/* FUNCTION NAME - CN_OM_GetCpData
 * PURPOSE : Get the data for a CP.
 * INPUT   : lport    - the specified logical port number
 *           cp_index - the specified CP index
 * OUTPUT  : buffer - buffer to contain the data for a CP
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Used by CN backdoor
 */
UI32_T CN_OM_GetCpData(UI32_T lport, UI32_T cp_index, CN_OM_CpData_T *buffer)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (buffer == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    if ((lport < 1) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return CN_TYPE_RETURN_ERROR_LPORT_RANGE;
    }

    if (cp_index >= SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT)
    {
        return CN_TYPE_RETURN_ERROR_CP_INDEX_RANGE;
    }

    CN_OM_ENTER_CRITICAL_SECTION();
    memcpy(buffer, &shmem_data_p->cp_list[lport-1][cp_index],
        sizeof(CN_OM_CpData_T));
    CN_OM_LEAVE_CRITICAL_SECTION();
    return CN_TYPE_RETURN_OK;
} /* End of CN_OM_GetCpData */

/* FUNCTION NAME - CN_OM_GetGlobalDataPtr
 * PURPOSE : Get a pointer to the global data.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : the pointer to the global data
 * NOTES   : Used by CN backdoor
 */
CN_OM_GlobalData_T* CN_OM_GetGlobalDataPtr(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return &shmem_data_p->global_data;
} /* End of CN_OM_GetGlobalDataPtr */

/* FUNCTION NAME - CN_OM_GetPriDataPtr
 * PURPOSE : Get a pointer to the data for a priority.
 * INPUT   : priority - the specified priority
 * OUTPUT  : None
 * RETURN  : the pointer to the data for a priority
 * NOTES   : None
 */
CN_OM_PriData_T* CN_OM_GetPriDataPtr(UI32_T priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return &shmem_data_p->pri_list[priority];
} /* End of CN_OM_GetPriDataPtr */

/* FUNCTION NAME - CN_OM_GetPortPriDataPtr
 * PURPOSE : Get a pointer to the data for a priority on a port.
 * INPUT   : priority - the specified priority
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : the pointer to the data for a priority on a port
 * NOTES   : None
 */
CN_OM_PortPriData_T* CN_OM_GetPortPriDataPtr(UI32_T priority, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return &shmem_data_p->port_pri_list[priority][lport-1];
} /* End of CN_OM_GetPortPriDataPtr */

/* FUNCTION NAME - CN_OM_GetCpDataPtr
 * PURPOSE : Get a pointer to an entry for the specified CP on a port.
 * INPUT   : lport    - the specified logical port
 *           cp_index - the specified CP index
 * OUTPUT  : None
 * RETURN  : the pointer to the data for a CP
 * NOTES   : None
 */
CN_OM_CpData_T* CN_OM_GetCpDataPtr(UI32_T lport, UI32_T cp_index)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return &shmem_data_p->cp_list[lport-1][cp_index];
} /* End of CN_OM_GetCpDataPtr */

void CN_OM_SetErrDebug(BOOL_T flag)
{
    debug_err = flag;
    return;
} /* End of CN_MGR_SetErrDebug */

BOOL_T CN_OM_ErrDebug(void)
{
    return debug_err;
} /* End of CN_MGR_GetErrDebug */

char* CN_OM_ErrStr(UI32_T ret_code)
{
    return err_str[ret_code];
} /* End of CN_OM_ErrStr */

void CN_OM_SetThreadDebug(BOOL_T flag)
{
    debug_thread = flag;
    return;
} /* End of CN_MGR_SetErrDebug */

BOOL_T CN_OM_ThreadDebug(void)
{
    return debug_thread;
} /* End of CN_MGR_GetErrDebug */

/* LOCAL SUBPROGRAM BODIES
 */

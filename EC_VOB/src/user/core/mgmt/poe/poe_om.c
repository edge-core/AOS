/*-----------------------------------------------------------------------------
 * FILE NAME: poe_om.c
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file provides the APIs for POE MGR to read/write the database.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    04/7/2003 - Kelly Hung, Created
 *    12/03/2008 - Eugene Yu, porting POE to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "poe_om.h"
#include "poe_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "leaf_3621.h"
#include <stdio.h>
#include <string.h>
#include "swctrl.h"
#include "swctrl_pom.h"
#include "stktplg_pom.h"
#include "stktplg_om.h"
#include "stktplg_board.h"
#include "poedrv.h"
#include "poe_engine.h"
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
#include "time_range_type.h"
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T POE_OM_GetNextUserPort(UI32_T *group_index, UI32_T *port_index);
static BOOL_T POE_OM_GetNextUnit(UI32_T *group_index_p);


/* STATIC VARIABLE DEFINITIONS
 */
static  POE_OM_PsePort_T              poe_om_pse_table[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT];
static  POE_OM_MainPse_T              poe_om_main_pse_table[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
static  UI32_T                        poe_om_main_pse_ctrl_table;
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
static  BOOL_T                        poe_om_use_local_power[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
#endif
static  POE_OM_PsePort_T              poe_om_pse_table_def;
static  POE_OM_MainPse_T              poe_om_main_pse_table_def;
static  UI32_T                        poe_om_main_pse_ctrl_table_def;
static  UI32_T                        poe_om_sem_id;
//static UI32_T                               original_priority;

/* NAMING CONSTANT DECLARATIONS
 */
#define WATT2MILLIWATT                1000

/* MACRO FUNCTION DECLARATIONS
 */
#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

#define POE_OM_EnterCriticalSection() {SYSFUN_OM_ENTER_CRITICAL_SECTION(poe_om_sem_id);}
#define POE_OM_LeaveCriticalSection() {int x; SYSFUN_OM_LEAVE_CRITICAL_SECTION(poe_om_sem_id, x);}

/* EXPORTED SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - POE_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the semaphore for POE objects
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void POE_OM_InitSemaphore(void)
{
    poe_om_sem_id = 0;
    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO,
            &poe_om_sem_id) != SYSFUN_OK)
    {
        printf("\n%s: get poe om sem id fail.\n", __FUNCTION__);
    }
    return;
}/* End of POE_OM_InitSemaphore */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetDBDefaultValue
 * -------------------------------------------------------------------------
 * FUNCTION: get database's default value
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_GetDBDefaultValue(void)
{
    char *str=PSE_PORT_TYPE_DEF;
    UI16_T len=strlen(str);
    poe_om_pse_table_def.pse_port_admin_enable = PSE_PORT_ADMIN_DEF;
    poe_om_pse_table_def.pse_port_power_pairs_ctrl_ability = PSE_PORT_POWER_PAIRS_CONTROL_ABILITY_DEF;
    poe_om_pse_table_def.pse_port_power_pairs = PSE_PORT_POWER_PAIRS_DEF;
#if 0
    poe_om_pse_table_def.pse_port_power_detection_ctrl = PSE_PORT_POWER_DETECTION_CONTROL_DEF;
#endif
    poe_om_pse_table_def.pse_port_detection_status = PSE_PORT_DETECTION_STATUS_DEF;
    poe_om_pse_table_def.pse_port_power_priority = PSE_PORT_POWER_PRIORITY_DEF;
#if 0
    poe_om_pse_table_def.pse_port_power_maintenance_status = PSE_PORT_POWER_MAINTENANCE_STATUS_DEF;
#endif
    poe_om_pse_table_def.pse_port_mpsabsent_counter = PSE_PORT_MPSABSENT_COUNT_DEF;
    poe_om_pse_table_def.pse_port_over_curr_counter = PSE_PORT_OVERCURRENT_COUNT_DEF;
    memcpy(poe_om_pse_table_def.pse_port_type,str,len+1);
    poe_om_pse_table_def.pse_port_power_classifications = PSE_PORT_POWER_CLASSIFICATION_DEF;
    poe_om_pse_table_def.pse_port_power_max_allocation = PSE_PORT_POWER_MAX_ALLOCATION;
    poe_om_pse_table_def.pse_port_consumption_power = 0;

#ifdef SYS_CPNT_POE_PSE_DOT3AT
    poe_om_pse_table_def.pse_port_force_power_mode = PSE_PORT_POWER_HIGHPOWER_MODE_DEF;
#endif

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
        strncpy((char *) poe_om_pse_table_def.pse_port_time_range_name, SYS_DFLT_PSE_PORT_TIME_RANGE_NAME, sizeof(poe_om_pse_table_def.pse_port_time_range_name));
    poe_om_pse_table_def.pse_port_time_range_index = TIME_RANGE_TYPE_UNDEF_TIME_RANGE;
    poe_om_pse_table_def.pse_port_time_range_status = POE_TYPE_TIMERANGE_STATUS_NONE;
#endif

    poe_om_main_pse_table_def.main_pse_power = MAIN_PSE_POWER_DEF;
    poe_om_main_pse_table_def.main_pse_oper_status = MAIN_PSE_OPER_STATUS_DEF;
    poe_om_main_pse_table_def.main_pse_consumption_power = MAIN_PSE_CONSUMPTION_POWER_DEF;
    poe_om_main_pse_table_def.main_pse_usage_threshold = MAIN_PSE_USAGE_THRESHOLD_DEF;
    poe_om_main_pse_table_def.main_pse_power_max_allocation = MAIN_PSE_POWER_MAX_ALLOCATION;
    poe_om_main_pse_table_def.main_pse_Legacy_Detection = SYS_DFLT_CAPACITOR_DETECTION ;
    poe_om_main_pse_table_def.max_pse_port_number = SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;

#if 1 /* Jason Yang change to reflect default value by SNMP 2004-9-22 */
    poe_om_main_pse_ctrl_table_def = NOTIFICATION_CONTROL_DEF;
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetDBToDefault
 * -------------------------------------------------------------------------
 * FUNCTION: set database to default value
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetDBToDefault(void)
{
    UI32_T unit, port;
    UI32_T board_id;
    STKTPLG_BOARD_BoardInfo_T board_info;

    /* sanity check for consistent definitions among SYS_ADPT_POE_PSE_MIN_PORT_NUMBER,
     * SYS_ADPT_POE_PSE_MAX_PORT_NUMBER and SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT
     */
    if (SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT != (SYS_ADPT_POE_PSE_MAX_PORT_NUMBER-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER+1))
    {
        printf("%s(%d): ERROR! Inconsistent constant definitions:SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT=%d\r\n",
            __FUNCTION__, __LINE__, SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT);
        printf("SYS_ADPT_POE_PSE_MAX_PORT_NUMBER=%d, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER=%d\r\n",
            SYS_ADPT_POE_PSE_MAX_PORT_NUMBER, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER);
        return;
    }

    for (unit = 1; unit <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
    {
        STKTPLG_OM_GetUnitBoardID(unit, &board_id);
        memset(&board_info, 0, sizeof(board_info));
        STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
        for (port = SYS_ADPT_POE_PSE_MIN_PORT_NUMBER; port <= SYS_ADPT_POE_PSE_MAX_PORT_NUMBER; port++)
        {
            memcpy(&poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER], &poe_om_pse_table_def, sizeof(POE_OM_PsePort_T));
            poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_max_allocation = board_info.per_port_power_max_allocation[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER];
        }
        memcpy(&poe_om_main_pse_table[unit-1], &poe_om_main_pse_table_def, sizeof(POE_OM_MainPse_T));
        poe_om_main_pse_table[unit-1].main_pse_power_max_allocation = board_info.main_pse_power_max_allocation;
        poe_om_main_pse_table[unit-1].max_pse_port_number = board_info.max_pse_port_number;
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
        poe_om_use_local_power[unit-1] = TRUE;
#endif
    }
    poe_om_main_pse_ctrl_table = poe_om_main_pse_ctrl_table_def;
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetDBToDefaultForHotInsert
 * -------------------------------------------------------------------------
 * FUNCTION: set database to default value for specific unit when hot insert
 * INPUT   : unit
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetDBToDefaultForHotInsert(UI32_T unit)
{
    UI32_T port, board_id;
    STKTPLG_BOARD_BoardInfo_T board_info;

    if(unit < 1 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return;

    /* sanity check for consistent definitions among SYS_ADPT_POE_PSE_MIN_PORT_NUMBER,
     * SYS_ADPT_POE_PSE_MAX_PORT_NUMBER and SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT
     */
    if (SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT != (SYS_ADPT_POE_PSE_MAX_PORT_NUMBER-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER+1))
    {
        printf("%s(%d): ERROR! Inconsistent constant definitions:SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT=%d\r\n",
            __FUNCTION__, __LINE__, SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT);
        printf("SYS_ADPT_POE_PSE_MAX_PORT_NUMBER=%d, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER=%d\r\n",
            SYS_ADPT_POE_PSE_MAX_PORT_NUMBER, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER);
        return;
    }

    STKTPLG_OM_GetUnitBoardID(unit, &board_id);
    memset(&board_info, 0, sizeof(board_info));
    STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
    for (port = SYS_ADPT_POE_PSE_MIN_PORT_NUMBER; port <= board_info.max_pse_port_number; port++)
    {
        memcpy(&poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER], &poe_om_pse_table_def, sizeof(POE_OM_PsePort_T));
        poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_max_allocation = board_info.per_port_power_max_allocation[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER];
    }
    memcpy(&poe_om_main_pse_table[unit-1], &poe_om_main_pse_table_def, sizeof(POE_OM_MainPse_T));
    poe_om_main_pse_table[unit-1].main_pse_power_max_allocation = board_info.main_pse_power_max_allocation;
    poe_om_main_pse_table[unit-1].max_pse_port_number = board_info.max_pse_port_number;
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
    poe_om_use_local_power[unit-1] = TRUE;
#endif
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_ClearDB
 * -------------------------------------------------------------------------
 * FUNCTION: clear database
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_ClearDB(void)
{
    memset(poe_om_pse_table, 0, (sizeof(POE_OM_PsePort_T)*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT));
    memset(poe_om_main_pse_table, 0, (sizeof(POE_OM_MainPse_T)*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK));
    poe_om_main_pse_ctrl_table = 0;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_UserPortExisting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this user port of poe device is existing
 * INPUT   : unit -- unit ID, port -- port number
 * OUTPUT  : None
 * RETURN  : TRUE: Existing, FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_UserPortExisting(UI32_T group_index, UI32_T port_index)
{
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI32_T board_id;
    UI8_T max_pse_port = SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;

    memset(&board_info, 0x0, sizeof(STKTPLG_BOARD_BoardInfo_T));
    STKTPLG_OM_GetUnitBoardID(group_index, &board_id);
    STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);

    max_pse_port = board_info.max_pse_port_number;

    if(SWCTRL_POM_UserPortExisting(group_index, port_index) && (port_index <= max_pse_port) )
    {
        if(POEDRV_UserPortExisting(group_index, port_index) == TRUE)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextUserPort
 * -------------------------------------------------------------------------
 * FUNCTION: get next poe user port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T POE_OM_GetNextUserPort(UI32_T *group_index_p, UI32_T *port_index_p)
{
    UI8_T  max_pse_port_number = SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;

    if((*port_index_p >= SYS_ADPT_POE_PSE_MAX_PORT_NUMBER) ||
       (FALSE == STKTPLG_OM_IsPoeDevice(*group_index_p)))
    {
        if (!POE_OM_GetNextUnit(group_index_p))
            return FALSE;

        *port_index_p = SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;
    }
    else
    { /* unit support poe */

        POE_OM_EnterCriticalSection();
        max_pse_port_number = poe_om_main_pse_table[*group_index_p - 1].max_pse_port_number;
        POE_OM_LeaveCriticalSection();

        if(*port_index_p < SYS_ADPT_POE_PSE_MIN_PORT_NUMBER)
        {
            *port_index_p = SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;
        }
        else
        {
            /* We assumes that SYS_ADPT_POE_PSE_MIN_PORT_NUMBER are
             * the same for all the board_id.
             *
             * As for SYS_ADPT_POE_PSE_MAX_PORT_NUMBER, they are
             * different in existing projects.
             */
            *port_index_p += 1;
            if(*port_index_p > max_pse_port_number)
            {
                if (!POE_OM_GetNextUnit(group_index_p))
                    return FALSE;

                *port_index_p = SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;
            }
        }
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextUnit
 * -------------------------------------------------------------------------
 * FUNCTION: get next poe unit
 * INPUT   : group_index
 * OUTPUT  : group_index
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T POE_OM_GetNextUnit(UI32_T *group_index_p)
{
    while(STKTPLG_OM_GetNextUnit(group_index_p))
    {
        if(FALSE == STKTPLG_OM_IsPoeDevice(*group_index_p))
            continue;
        else
            return TRUE;
    }

    return FALSE;
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: enables power and detection mechanism for this port.
 * INPUT   : unit
 *           port
 *           value = enable(1) enables power and detection mechanism for this port.
 *                 = disable(2) disables power for this port.
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortAdmin(UI32_T unit, UI32_T port, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_admin_enable = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port admin status on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortAdmin(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_admin_enable;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortAdmin(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortAdmin(*group_index, *port_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPsePortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPsePortAdmin(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (! POE_OM_GetPsePortAdmin(group_index, port_index, value))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (*value == SYS_DFLT_PSE_PORT_ADMIN)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    else
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPsePortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPsePortAdmin(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    return POE_OM_GetRunningPsePortAdmin(*group_index, *port_index, value);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerPairsCtrlAbility
 * -------------------------------------------------------------------------
 * FUNCTION: Describes the capability of controlling the power pairs
 *           functionality to switch pins for sourcing power.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerPairsCtrlAbility(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_pairs_ctrl_ability;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortPowerPairsCtrlAbility
 * -------------------------------------------------------------------------
 * FUNCTION: Describes the capability of controlling the power pairs
 *           functionality to switch pins for sourcing power.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortPowerPairsCtrlAbility(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortPowerPairsCtrlAbility(*group_index, *port_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortPowerPairs
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.
 * INPUT   : unit
 *           port
 *           value = signal(1)means that the signal pairs only are in use.
 *                 = spare(2) means that the spare pairs only are in use.
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : If the value of pethPsePortPowerPairsControl is true,
 *           this object is writable.
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortPowerPairs(UI32_T unit, UI32_T port, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_pairs = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerPairs
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerPairs(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_pairs;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortPowerPairs
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortPowerPairs(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortPowerPairs(*group_index, *port_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPsePortPowerPairs
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPsePortPowerPairs(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (! POE_OM_GetPsePortPowerPairs(group_index, port_index, value))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (*value == PSE_PORT_POWER_PAIRS_DEF)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    else
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPsePortPowerPairs
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPsePortPowerPairs(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    return POE_OM_GetRunningPsePortPowerPairs(*group_index, *port_index, value);
}

#if 0
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortPowerDetectionCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the power detection mechanism of the port.
 * INPUT   : unit
 *           port
 *           value = auto(1)enables the power detection mechanism of the port.
 *                 = test(2)puts the port in a testmode:
 *                   force continuous discovery without applying
 *                   power regardless of whether PD detected.
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortPowerDetectionCtrl(UI32_T unit, UI32_T port, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_pse_table[unit - 1][port - 1].pse_port_power_detection_ctrl = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerDetectionCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the power detection mechanism of the port.
 * INPUT   : unit
 *           port
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_GetPsePortPowerDetectionCtrl(UI32_T unit, UI32_T port, UI32_T *value)
{
    POE_OM_EnterCriticalSection();
    *value = poe_om_pse_table[unit - 1][port - 1].pse_port_power_detection_ctrl;
    POE_OM_LeaveCriticalSection();
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortDetectionStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the operational status of the port PD detection
 * INPUT   : unit
 *           port
 *           value = disabled(1), searching(2), deliveringPower(4),
 *                   fault(5), test(7), denyLowPriority(8)
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortDetectionStatus(UI32_T unit, UI32_T port, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_detection_status = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortDetectionStatus
 * -------------------------------------------------------------------------
 * FUNCTION: the operational status of the port PD detection
 * INPUT   : group_index
             port_index
 * OUTPUT  : value : disabled(1), searching(2), deliveringPower(4), fault(5), test(7), denyLowPriority(8)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortDetectionStatus(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_detection_status;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortDetectionStatus
 * -------------------------------------------------------------------------
 * FUNCTION: the operational status of the port PD detection
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value : disabled(1), searching(2), deliveringPower(4), fault(5), test(7), denyLowPriority(8)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortDetectionStatus(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortDetectionStatus(*group_index, *port_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortPowerConsumption
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the operational status of the port PD detection
 * INPUT   : unit
 *           port
 *           value
 * OUTPUT  : None
 * RETURN  :
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortPowerConsumption(UI32_T unit, UI32_T port, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_consumption_power = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerConsumption
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port the power consumption
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerConsumption(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_consumption_power;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortPowerPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point
 *           of view of a power management algorithm.
 * INPUT   : unit
 *           port
 *           value = critical(1)
 *                 = high(2)
 *                 = low(3)
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : The priority that
 *           is set by this variable could be used by a control mechanism
 *           that prevents over current situations by disconnecting first
 *           ports with lower power priority. Ports that connect devices
 *           critical to the operation of the network - like the E911
 *           telephones ports - should be set to higher priority.
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortPowerPriority(UI32_T unit, UI32_T port, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_priority = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point
 *           of view of a power management algorithm.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value = critical(1)
 *                 = high(2)
 *                 = low(3)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_priority;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortPowerPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point
 *           of view of a power management algorithm.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value = critical(1)
 *                 = high(2)
 *                 = low(3)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortPowerPriority(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortPowerPriority(*group_index, *port_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPsePortPowerPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point
 *           of view of a power management algorithm.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value = critical(1)
 *                 = high(2)
 *                 = low(3)
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (! POE_OM_GetPsePortPowerPriority(group_index, port_index, value))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (*value == SYS_DFLT_PSE_PORT_POWER_PRIORITY)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    else
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPsePortPowerPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point
 *           of view of a power management algorithm.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value = critical(1)
 *                 = high(2)
 *                 = low(3)
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPsePortPowerPriority(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    return POE_OM_GetRunningPsePortPowerPriority(*group_index, *port_index, value);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerMaintenanceStatus
 * -------------------------------------------------------------------------
 * FUNCTION: get PSE power maintenance status
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : The value ok(1) indicates the Power Maintenance
 *           Signature is present and the overcurrent condition has not been
 *           detected.
 *           The value overCurrent (2) indicates an overcurrent condition
 *           has been detected.
 *           The value mPSAbsent(3) indicates that the Power Maintenance
 *           Signature is absent.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerMaintenanceStatus(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_maintenance_status;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortPowerMaintenanceStatus
 * -------------------------------------------------------------------------
 * FUNCTION: get PSE power maintenance status
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : The value ok(1) indicates the Power Maintenance
 *           Signature is present and the overcurrent condition has not been
 *           detected.
 *           The value overCurrent (2) indicates an overcurrent condition
 *           has been detected.
 *           The value mPSAbsent(3) indicates that the Power Maintenance
 *           Signature is absent.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortPowerMaintenanceStatus(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortPowerMaintenanceStatus(*group_index, *port_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortMPSAbsentCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Counts the number of times that the
 *           pethPsePortPowerMaintenanceStatus attribute changes from any
 *           value to the value mPSAbsent(3)
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortMPSAbsentCounter(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
#if (SYS_CPNT_POE_COUNTER_SUPPORT==TRUE)
        POEDRV_GetPortMPSAbsentCounter(group_index, port_index, value);
#else
        POE_OM_EnterCriticalSection();
        *value = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_mpsabsent_counter;
        POE_OM_LeaveCriticalSection();
#endif
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortMPSAbsentCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Counts the number of times that the
 *           pethPsePortPowerMaintenanceStatus attribute changes from any
 *           value to the value mPSAbsent(3)
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortMPSAbsentCounter(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortMPSAbsentCounter(*group_index, *port_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortOverCurrCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Counts the number of times that the aPSEPowerCurrentStatus
 *           attribute changes from any value to the value overCurrent(2).
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortOverCurrCounter(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_over_curr_counter;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortOverCurrCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Counts the number of times that the aPSEPowerCurrentStatus
 *           attribute changes from any value to the value overCurrent(2).
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortOverCurrCounter(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortOverCurrCounter(*group_index, *port_index, value))
        return FALSE;

    return TRUE;
}

#if (SYS_CPNT_POE_COUNTER_SUPPORT==TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortInvalidSignCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Get the port invalid signature counters.
 *
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : counter
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortInvalidSignCounter(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POEDRV_GetPortInvalidSignCounter(group_index, port_index, value);
            return TRUE;
    }
    else
    {
            return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerDeniedCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Get the port power denied counters.
 *
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : counter
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerDeniedCounter(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POEDRV_GetPortPowerDeniedCounter(group_index, port_index, value);
            return TRUE;
    }
    else
    {
            return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortOverloadCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Get the port overload counters.
 *
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : counter
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortOverloadCounter(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POEDRV_GetPortOverloadCounter(group_index, port_index, value);
            return TRUE;
    }
    else
    {
            return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortShortCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Get the port short counters.
 *
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : counter
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortShortCounter(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POEDRV_GetPortShortCounter(group_index, port_index, value);
            return TRUE;
    }
    else
    {
            return FALSE;
    }
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPoeSoftwareVersion
 * -------------------------------------------------------------------------
 * FUNCTION: Get software version from PoE controller
 * INPUT   : unit -- unit ID
 * OUTPUT  : version1 -- version number 1
 *           version2 -- version number 2
 *           build    -- build number
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPoeSoftwareVersion(UI32_T unit, UI8_T *version1, UI8_T *version2, UI8_T *build)
{
    /* Semantic check
     */
    if((unit < 1) || (unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK))
    {
        return FALSE;
    }

    /* Get software version from PoE controller
     */
    return POEDRV_GetPoeSoftwareVersion(unit, version1, version2, build);
} /* End of POE_OM_GetPoeSoftwareVersion() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPortPowerTemperature
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port temperature
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPortPowerTemperature(UI32_T group_index, UI32_T port_index, I32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
        POEDRV_GetPoePortTemperature(group_index, port_index, value);
#elif (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_POWERDSINE)
        value = 0;
#endif
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPortPowerVoltage
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port voltage
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value (V)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPortPowerVoltage(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
        POEDRV_GetPoePortVoltage(group_index, port_index, value);
#elif (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_POWERDSINE)
        value = 0;
#endif
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPortPowerCurrent
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port current
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value (mA)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPortPowerCurrent(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
        POEDRV_GetPoePortCurrent(group_index, port_index, value);
#elif (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_POWERDSINE)
        value = 0;
#endif
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortType
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value
 *           that indicates the type of the device that is connected
 *           to theport. This value can be the result of the mapping
 *           the address of the station connected to the port and of
 *           the value of the pethPdPortType of the respective PD port.
 * INPUT   : unit
 *           port
 *           value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortType(UI32_T unit, UI32_T port, UI8_T *value, UI32_T len)
{
    POE_OM_EnterCriticalSection();
    memcpy(poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_type,value,len);
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortType
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value
 *           that indicates the type of the device that is connected
 *           to theport. This value can be the result of the mapping
 *           the address of the station connected to the port and of
 *           the value of the pethPdPortType of the respective PD port.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortType(UI32_T group_index, UI32_T port_index, UI8_T *value, UI32_T len)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        memcpy(value,poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_type,len);
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortType
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value
 *           that indicates the type of the device that is connected
 *           to theport. This value can be the result of the mapping
 *           the address of the station connected to the port and of
 *           the value of the pethPdPortType of the respective PD port.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortType(UI32_T *group_index, UI32_T *port_index, UI8_T *value, UI32_T len)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortType(*group_index, *port_index, value, len))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPsePortType
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value
 *           that indicates the type of the device that is connected
 *           to theport. This value can be the result of the mapping
 *           the address of the station connected to the port and of
 *           the value of the pethPdPortType of the respective PD port.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPsePortType(UI32_T group_index, UI32_T port_index, UI8_T *value, UI32_T len)
{
    if (! POE_OM_GetPsePortType(group_index, port_index, value, len))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    /*if (*value == PSE_PORT_TYPE_DEF)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    else*/
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPsePortType
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value
 *           that indicates the type of the device that is connected
 *           to theport. This value can be the result of the mapping
 *           the address of the station connected to the port and of
 *           the value of the pethPdPortType of the respective PD port.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPsePortType(UI32_T *group_index, UI32_T *port_index, UI8_T *value, UI32_T len)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    return POE_OM_GetRunningPsePortType(*group_index, *port_index, value, len);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortPowerClassification
 * -------------------------------------------------------------------------
 * FUNCTION: Update the power class of a port
 * INPUT   : unit
 *           port
 *           value
 * OUTPUT  : None
 * RETURN  :
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortPowerClassification(UI32_T unit, UI32_T port, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_classifications = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerClassifications
 * -------------------------------------------------------------------------
 * FUNCTION: Classification is a way to tag different terminals on the
 *           Power over LAN network according to their power consumption.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value : class0(1),
 *                   class1(2),
 *                   class2(3),
 *                   class3(4),
 *                   class4(5)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerClassifications(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_classifications;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortPowerClassifications
 * -------------------------------------------------------------------------
 * FUNCTION: Classification is a way to tag different terminals on the
 *           Power over LAN network according to their power consumption.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value : class0(1),
 *                   class1(2),
 *                   class2(3),
 *                   class3(4),
 *                   class4(5)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortPowerClassifications(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortPowerClassifications(*group_index, *port_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPortPowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Set a specified port the maximum power
 * INPUT   : unit
 *           port
 *           value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
void POE_OM_SetPortPowerMaximumAllocation(UI32_T unit, UI32_T port, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_pse_table[unit-1][port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_max_allocation = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPortPowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port the maximum power
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_max_allocation;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPortPowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port the maximum power
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPortPowerMaximumAllocation(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPortPowerMaximumAllocation(*group_index, *port_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPortPowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port the maximum power
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (! POE_OM_GetPortPowerMaximumAllocation(group_index, port_index, value))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

#if (SYS_CPNT_POE_PORT_MAX_ALLOC_DIFF == TRUE)
{
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI32_T board_id;

    STKTPLG_OM_GetUnitBoardID(group_index, &board_id);
    STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);

    if (*value == board_info.per_port_power_max_allocation[port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER])
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}
#else

    if (*value == SYS_HWCFG_MAX_POWER_INLINE_ALLOCATION)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPortPowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port the maximum power
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPortPowerMaximumAllocation(UI32_T *group_index, UI32_T *port_index, UI32_T *value)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    return POE_OM_GetRunningPortPowerMaximumAllocation(*group_index, *port_index, value);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetMainPsePower
 * -------------------------------------------------------------------------
 * FUNCTION: set the nominal power of the PSE expressed in Watts.
 * INPUT   : unit
 *           value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetMainPsePower(UI32_T unit, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_main_pse_table[unit - 1].main_pse_power = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetMainPsePower
 * -------------------------------------------------------------------------
 * FUNCTION: The nominal power of the PSE expressed in Watts.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetMainPsePower(UI32_T group_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_main_pse_table[group_index - 1].main_pse_power;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainPsePower
 * -------------------------------------------------------------------------
 * FUNCTION: The nominal power of the PSE expressed in Watts.
 * INPUT   : group_index
 * OUTPUT  : group_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainPsePower(UI32_T *group_index, UI32_T *value)
{
    if (! POE_OM_GetNextUnit(group_index))
        return FALSE;

    if (! POE_OM_GetMainPsePower(*group_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetMainPseOperStatus
 * -------------------------------------------------------------------------
 * FUNCTION: set the operational status of the main PSE.
 * INPUT   : unit
 *           value = on(1)
 *                 = off(2)
 *                 = faulty(3)
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetMainPseOperStatus(UI32_T unit, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_main_pse_table[unit - 1].main_pse_oper_status = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetMainPseOperStatus
 * -------------------------------------------------------------------------
 * FUNCTION: The operational status of the main PSE.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetMainPseOperStatus(UI32_T group_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_main_pse_table[group_index - 1].main_pse_oper_status;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainPseOperStatus
 * -------------------------------------------------------------------------
 * FUNCTION: The operational status of the main PSE.
 * INPUT   : group_index
 * OUTPUT  : group_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainPseOperStatus(UI32_T *group_index, UI32_T *value)
{
    if (! POE_OM_GetNextUnit(group_index))
        return FALSE;

    if (! POE_OM_GetMainPseOperStatus(*group_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetMainPsePower
 * -------------------------------------------------------------------------
 * FUNCTION: set the available power of the PSE expressed in Watts.
 * INPUT   : unit
 *           value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetMainPseConsumptionPower(UI32_T unit, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_main_pse_table[unit - 1].main_pse_consumption_power = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetMainPseConsumptionPower
 * -------------------------------------------------------------------------
 * FUNCTION: Measured usage power expressed in Watts.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetMainPseConsumptionPower(UI32_T group_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER))
    {
#ifdef SYS_CPNT_POE_STYLE_3COM_HUAWEI
        UI32_T nPort;
        UI32_T power = 0;
#endif

        POE_OM_EnterCriticalSection();
#ifdef SYS_CPNT_POE_STYLE_3COM_HUAWEI
        /* Ed_huang 2006.11.16, according to POE chip's limitation, global power consumption can only be
           gathered in Watt, 3COM need us the modify this value to be consistent with per port consumption(in mWatt) */
        for (nPort = SYS_ADPT_POE_PSE_MIN_PORT_NUMBER; nPort <= SYS_ADPT_POE_PSE_MAX_PORT_NUMBER; nPort++)
        {
            power += poe_om_pse_table[group_index-1][nPort-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_consumption_power;
        }
        *value = power;
#else
        *value = poe_om_main_pse_table[group_index - 1].main_pse_consumption_power;
#endif
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainPseConsumptionPower
 * -------------------------------------------------------------------------
 * FUNCTION: Measured usage power expressed in Watts.
 * INPUT   : group_index
 * OUTPUT  : group_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainPseConsumptionPower(UI32_T *group_index, UI32_T *value)
{
    if (! POE_OM_GetNextUnit(group_index))
        return FALSE;

    if (! POE_OM_GetMainPseConsumptionPower(*group_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetMainPseUsageThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: set the usage threshold expressed in percents for
 *           comparing the measured power and initiating
 *           an alarm if the threshold is exceeded.
 * INPUT   : unit
 *           value = (1..99)
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetMainPseUsageThreshold(UI32_T unit, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_main_pse_table[unit - 1].main_pse_usage_threshold = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetMainPseUsageThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: get the usage threshold expressed in percents for
 *           comparing the measured power and initiating
 *           an alarm if the threshold is exceeded.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetMainPseUsageThreshold(UI32_T group_index, UI32_T *value)

{
    if (POE_OM_UserPortExisting(group_index, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_main_pse_table[group_index - 1].main_pse_usage_threshold;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainPseUsageThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: get the usage threshold expressed in percents for
 *           comparing the measured power and initiating
 *           an alarm if the threshold is exceeded.
 * INPUT   : group_index
 * OUTPUT  : group_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainPseUsageThreshold(UI32_T *group_index, UI32_T *value)
{
    if (! POE_OM_GetNextUnit(group_index))
        return FALSE;

    if (! POE_OM_GetMainPseUsageThreshold(*group_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningMainPseUsageThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: get the usage threshold expressed in percents for
 *           comparing the measured power and initiating
 *           an alarm if the threshold is exceeded.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningMainPseUsageThreshold(UI32_T group_index, UI32_T *value)
{
    if (! POE_OM_GetMainPseUsageThreshold(group_index, value))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (*value == MAIN_PSE_USAGE_THRESHOLD_DEF)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    else
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningMainPseUsageThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: get the usage threshold expressed in percents for
 *           comparing the measured power and initiating
 *           an alarm if the threshold is exceeded.
 * INPUT   : group_index
 * OUTPUT  : group_index
 *           value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningMainPseUsageThreshold(UI32_T *group_index, UI32_T *value)
{
    if (! POE_OM_GetNextUnit(group_index))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    return POE_OM_GetRunningMainPseUsageThreshold(*group_index, value);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetMainpowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get the power, available for Power
 *           Management on PoE.
 * INPUT   : unit
 *           value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 36 to 800 watts.
 * -------------------------------------------------------------------------*/
void POE_OM_SetMainpowerMaximumAllocation(UI32_T unit, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_main_pse_table[unit - 1].main_pse_power_max_allocation= value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetMainpowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get the power, available for Power
 *           Management on PoE.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 36 to 800 watts.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetMainpowerMaximumAllocation(UI32_T group_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER))
    {
        POE_OM_EnterCriticalSection();
#ifdef SYS_CPNT_POE_STYLE_3COM_HUAWEI
        /* Ed_huang 2006.11.16, according to POE chip's limitation, global power consumption can only be
           gathered in Watt, 3COM need us the modify this value to be consistent with per port consumption(in mWatt) */
        *value = poe_om_main_pse_table[group_index - 1].main_pse_power * WATT2MILLIWATT;
#else
        *value = poe_om_main_pse_table[group_index - 1].main_pse_power_max_allocation;
#endif
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainpowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to get the power, available for Power
 *           Management on PoE.
 * INPUT   : group_index
 * OUTPUT  : group_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 36 to 800 watts.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainpowerMaximumAllocation(UI32_T *group_index, UI32_T *value)
{
    if (! POE_OM_GetNextUnit(group_index))
        return FALSE;

    if (! POE_OM_GetMainpowerMaximumAllocation(*group_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningMainpowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to get the power, available for Power
 *           Management on PoE.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : Power can be set from 36 to 800 watts.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningMainpowerMaximumAllocation(UI32_T group_index, UI32_T *value)
{
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI32_T board_id;

    if (! POE_OM_GetMainpowerMaximumAllocation(group_index, value))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    STKTPLG_OM_GetUnitBoardID(group_index, &board_id);
    memset(&board_info, 0x0, sizeof(STKTPLG_BOARD_BoardInfo_T));
    STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);

    //if (*value == MAIN_PSE_POWER_MAX_ALLOCATION)
    if (*value == board_info.main_pse_power_max_allocation)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningMainpowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to get the power, available for Power
 *           Management on PoE.
 * INPUT   : group_index
 * OUTPUT  : group_index
 *           value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : Power can be set from 36 to 800 watts.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningMainpowerMaximumAllocation(UI32_T *group_index, UI32_T *value)
{
    if (! POE_OM_GetNextUnit(group_index))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    return POE_OM_GetRunningMainpowerMaximumAllocation(*group_index, value);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetNotificationCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: set notification control
 * INPUT   : unit
 *           value
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetNotificationCtrl(UI32_T unit, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_main_pse_ctrl_table = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPseNotificationCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPseNotificationCtrl(UI32_T group_index, UI32_T *value)

{
    if (POE_OM_UserPortExisting(group_index, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_main_pse_ctrl_table;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextNotificationCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
 * OUTPUT  : group_index
 *           value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextNotificationCtrl(UI32_T *group_index, UI32_T *value)
{
    if (! POE_OM_GetNextUnit(group_index))
        return FALSE;

    if (! POE_OM_GetPseNotificationCtrl(*group_index, value))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningNotificationCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningNotificationCtrl(UI32_T group_index, UI32_T *value)
{
    if (! POE_OM_GetPseNotificationCtrl(group_index, value))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (*value == NOTIFICATION_CONTROL_DEF)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    else
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningNotificationCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningNotificationCtrl(UI32_T *group_index, UI32_T *value)
{
    if (! POE_OM_GetNextUnit(group_index))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    return POE_OM_GetRunningNotificationCtrl(*group_index, value);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetLegacyDetection
 * -------------------------------------------------------------------------
 * FUNCTION: Set Legacy Detection
 * INPUT   : unit
 *           value (1 for Enable, 0 for disable)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetLegacyDetection(UI32_T unit, UI32_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_main_pse_table[unit - 1].main_pse_Legacy_Detection = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetLegacyDetection
 * -------------------------------------------------------------------------
 * FUNCTION: Get Legacy Detection
 * INPUT   : group_index
 * OUTPUT  : value (1 for Enable, 0 for disable)
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetLegacyDetection(UI32_T group_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER))
    {
        POE_OM_EnterCriticalSection();
        *value = poe_om_main_pse_table[group_index - 1].main_pse_Legacy_Detection;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextLegacyDetection
 * -------------------------------------------------------------------------
 * FUNCTION: Get Legacy Detection
 * INPUT   : group_index
 * OUTPUT  : group_index, value (1 for Enable, 0 for disable)
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextLegacyDetection(UI32_T *group_index, UI32_T *value)
{
    if (!POE_OM_GetNextUnit(group_index))
    {
        return FALSE;
    }

    if (!POE_OM_GetLegacyDetection(*group_index, value))
    {
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningLegacyDetection
 * -------------------------------------------------------------------------
 * FUNCTION: Get Legacy Detection
 * INPUT   : unit
 * OUTPUT  : value (1 for Enable, 0 for disable)
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningLegacyDetection(UI32_T group_index, UI32_T *value)
{
    if (! POE_OM_GetLegacyDetection(group_index, value))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (*value == SYS_DFLT_CAPACITOR_DETECTION)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    else
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningLegacyDetection
 * -------------------------------------------------------------------------
 * FUNCTION: Get Legacy Detection
 * INPUT   : unit
 * OUTPUT  : value (1 for Enable, 0 for disable)
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningLegacyDetection(UI32_T *group_index, UI32_T *value)
{
    if (! POE_OM_GetNextUnit(group_index))
        return FALSE;

    return POE_OM_GetRunningLegacyDetection(*group_index, value);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will display and control the power
 *           characteristics power Ethernet ports on a Power Source
 *           Entity (PSE) device.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : entry (POE_MGR_PETH_PSE_PORT_T)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortEntry(UI32_T group_index, UI32_T port_index, POE_OM_PsePort_T *entry)
{
#if 1
if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        memcpy(entry,&poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER], sizeof(POE_OM_PsePort_T));
        POE_OM_LeaveCriticalSection();

#if (SYS_CPNT_POE_COUNTER_SUPPORT==TRUE)
        POEDRV_GetPortMPSAbsentCounter(group_index, port_index, &entry->pse_port_mpsabsent_counter);
        POEDRV_GetPortShortCounter(group_index, port_index, &entry->pse_port_short_counter);
        POEDRV_GetPortInvalidSignCounter(group_index, port_index, &entry->pse_port_invalid_signature_counter);
        POEDRV_GetPortPowerDeniedCounter(group_index, port_index, &entry->pse_port_power_denied_counter);
        POEDRV_GetPortOverloadCounter(group_index, port_index, &entry->pse_port_overload_counter);
#endif

        return TRUE;
    }
    else
    {
        return FALSE;
    }

#else
    memset(entry, 0, sizeof(POE_OM_PsePort_T));

    if (! POE_OM_GetPsePortAdmin(group_index, port_index, &entry->pse_port_admin_enable))
        return FALSE;
    if (! POE_OM_GetPsePortPowerPairsCtrlAbility(group_index, port_index, &entry->pse_port_power_pairs_ctrl_ability))
        return FALSE;
    if (! POE_OM_GetPsePortPowerPairs(group_index, port_index, &entry->pse_port_power_pairs))
        return FALSE;
    if (! POE_OM_GetPsePortDetectionStatus(group_index, port_index, &entry->pse_port_detection_status))
        return FALSE;
    if (! POE_OM_GetPsePortPowerPriority(group_index, port_index, &entry->pse_port_power_priority))
        return FALSE;
    if (! POE_OM_GetPsePortPowerMaintenanceStatus(group_index, port_index, &entry->pse_port_power_maintenance_status))
        return FALSE;
    if (! POE_OM_GetPsePortMPSAbsentCounter(group_index, port_index, &entry->pse_port_mpsabsent_counter))
        return FALSE;
#if (SYS_CPNT_POE_COUNTER_SUPPORT==TRUE)
    if (! POE_OM_GetPsePortShortCounter(group_index, port_index, &entry->pse_port_short_counter))
        return FALSE;
    if (! POE_OM_GetPsePortInvalidSignCounter(group_index, port_index, &entry->pse_port_invalid_signature_counter))
        return FALSE;
    if (! POE_OM_GetPsePortPowerDeniedCounter(group_index, port_index, &entry->pse_port_power_denied_counter))
        return FALSE;
    if (! POE_OM_GetPsePortOverloadCounter(group_index, port_index, &entry->pse_port_overload_counter))
        return FALSE;
#endif
    if (! POE_OM_GetPsePortOverCurrCounter(group_index, port_index, &entry->pse_port_over_curr_counter))
        return FALSE;
    if (! POE_OM_GetPsePortType(group_index, port_index,entry->pse_port_type , MAXSIZE_pethPsePortType+1))
        return FALSE;
    if (! POE_OM_GetPsePortPowerClassifications(group_index, port_index, &entry->pse_port_power_classifications))
        return FALSE;
    if (! POE_OM_GetPortPowerMaximumAllocation(group_index, port_index, &entry->pse_port_power_max_allocation))
        return FALSE;

#ifdef SYS_CPNT_POE_PSE_DOT3AT
    if (! POE_OM_GetPortManualHighPowerMode(group_index,port_index,&entry->pse_port_force_power_mode))
        return FALSE;
#endif

    return TRUE;
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will display and control next power
 *           characteristics power Ethernet ports on a Power Source
 *           Entity (PSE) device.
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           entry (POE_MGR_PETH_PSE_PORT_T)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortEntry(UI32_T *group_index, UI32_T *port_index, POE_OM_PsePort_T *entry)
{
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortEntry(*group_index, *port_index, entry))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPethMainPseEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will display and control the Main power
 *           on a PSE  device. Example - an Ethernet switch midspan device can
 *             control an Ethnternet port and the Main Power supply unit's.
 * INPUT   : group_index
 * OUTPUT  : entry (POE_MGR_PETH_MAIN_PSE_T)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPethMainPseEntry(UI32_T group_index, POE_OM_MainPse_T *entry)
{
#if 1
    if (POE_OM_UserPortExisting(group_index, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER))
    {
        POE_OM_EnterCriticalSection();
        memcpy(entry,&poe_om_main_pse_table[group_index - 1], sizeof(POE_OM_MainPse_T));
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }


#else

    memset(entry, 0, sizeof(POE_OM_MainPse_T));

DBG_PRINT();
    if (! POE_OM_GetMainPsePower(group_index, &entry->main_pse_power))
        return FALSE;
DBG_PRINT();
    if (! POE_OM_GetMainPseOperStatus(group_index, &entry->main_pse_oper_status))
        return FALSE;
DBG_PRINT();
    if (! POE_OM_GetMainPseConsumptionPower(group_index, &entry->main_pse_consumption_power))
        return FALSE;
DBG_PRINT();
    if (! POE_OM_GetMainPseUsageThreshold(group_index, &entry->main_pse_usage_threshold))
        return FALSE;
DBG_PRINT();
    if (! POE_OM_GetMainpowerMaximumAllocation(group_index, &entry->main_pse_power_max_allocation))
        return FALSE;

DBG_PRINT();
    return TRUE;
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainPseEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will display and control next Main power
 *           on a PSE  device. Example - an Ethernet switch midspan device can
 *             control an Ethnternet port and the Main Power supply unit's.
 * INPUT   : group_index
 * OUTPUT  : entry (POE_MGR_PETH_MAIN_PSE_T)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainPseEntry(UI32_T *group_index, POE_OM_MainPse_T *entry)
{
DBG_PRINT();
    if (! POE_OM_GetNextUnit(group_index))
        return FALSE;

DBG_PRINT();
    if (! POE_OM_GetPethMainPseEntry(*group_index, entry))
        return FALSE;

DBG_PRINT();
    return TRUE;
}

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortTimeRangeIndex
 * -------------------------------------------------------------------------
 * FUNCTION: Set POE port binding time range index on this port
 * INPUT   : group_index
 *           port_index
 *           index - time range index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortTimeRangeIndex(UI32_T group_index, UI32_T port_index, UI32_T index)
{
DBG_PRINT();
    POE_OM_EnterCriticalSection();
    poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_time_range_index = index;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortTimeRangeIndex
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range index on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : index - time range index
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortTimeRangeIndex(UI32_T group_index, UI32_T port_index, UI32_T *index)
{
DBG_PRINT();
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *index = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_time_range_index;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Set POE port binding time range name on this port
 * INPUT   : group_index
 *           port_index
 *           time_range - time range name
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortTimeRangeName(UI32_T group_index, UI32_T port_index, UI8_T* time_range)
{
    DBG_PRINT();
    POE_OM_EnterCriticalSection();
    strncpy((char *) (poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_time_range_name), (char *) time_range, sizeof(poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_time_range_name));
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range name on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : time_range - time range name
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortTimeRangeName(UI32_T group_index, UI32_T port_index, UI8_T *time_range)
{
DBG_PRINT();
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        strncpy((char *) time_range, (char *) (poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_time_range_name), sizeof(poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_time_range_name));
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range name on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           time_range - time range name
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortTimeRangeName(UI32_T *group_index, UI32_T *port_index, UI8_T *time_range)
{
DBG_PRINT();
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortTimeRangeName(*group_index, *port_index, time_range))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get running POE port binding time range name on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : time_range
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPsePortTimeRangeName(UI32_T group_index, UI32_T port_index, UI8_T *time_range)
{
DBG_PRINT();
    if (! POE_OM_GetPsePortTimeRangeName(group_index, port_index, time_range))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (strcmp((char*)time_range, SYS_DFLT_PSE_PORT_TIME_RANGE_NAME) == 0)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    else
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get running POE port binding time range name on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           time_range
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPsePortTimeRangeName(UI32_T *group_index, UI32_T *port_index, UI8_T *time_range)
{
DBG_PRINT();
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    return POE_OM_GetRunningPsePortTimeRangeName(*group_index, *port_index, time_range);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortTimeRangeStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set POE port binding time range status on this port
 * INPUT   : group_index
 *           port_index
 *           status - time range status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortTimeRangeStatus(UI32_T group_index, UI32_T port_index, UI32_T status)
{
DBG_PRINT();
    POE_OM_EnterCriticalSection();
    poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_time_range_status = status;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortTimeRangeStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range status on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : status - time range status
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortTimeRangeStatus(UI32_T group_index, UI32_T port_index, UI32_T *status)
{
DBG_PRINT();
    if (POE_OM_UserPortExisting(group_index, port_index))
    {
        POE_OM_EnterCriticalSection();
        *status = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_time_range_status;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortTimeRangeStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range status on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           status - time range status
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortTimeRangeStatus(UI32_T *group_index, UI32_T *port_index, UI32_T *status)
{
DBG_PRINT();
    if (! POE_OM_GetNextUserPort(group_index, port_index))
        return FALSE;

    if (! POE_OM_GetPsePortTimeRangeStatus(*group_index, *port_index, status))
        return FALSE;

    return TRUE;
}

#endif

#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetUseLocalPower
 * -------------------------------------------------------------------------
 * FUNCTION: Set Legacy Detection
 * INPUT   : unit
 *           value (TRUE => Local power, FALSE => RPS)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetUseLocalPower(UI32_T unit, BOOL_T value)
{
    POE_OM_EnterCriticalSection();
    poe_om_use_local_power[unit-1] = value;
    POE_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetUseLocalPower
 * -------------------------------------------------------------------------
 * FUNCTION: Set Legacy Detection
 * INPUT   : unit
 *           value (TRUE => Local power, FALSE => RPS)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetUseLocalPower(UI32_T unit, BOOL_T *value)
{
    if (POE_OM_UserPortExisting(unit, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER))
    {
        POE_OM_EnterCriticalSection();
            *value = poe_om_use_local_power[unit-1];
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif

/* Ed_huang 2006.7.19, modeifiy for 3COM style poe implementation */
#ifdef SYS_CPNT_POE_STYLE_3COM_HUAWEI
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetAllHighPriorityPsePortPowerMaxAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get total maximum power of the high
             priority port
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetAllHighPriorityPsePortPowerMaxAllocation(UI32_T group_index, UI32_T *value)
{
    if (POE_OM_UserPortExisting(group_index, SYS_ADPT_POE_PSE_MIN_PORT_NUMBER))
    {
        UI32_T nPort;
        UI32_T power = 0;


        POE_OM_EnterCriticalSection();
        for (nPort = SYS_ADPT_POE_PSE_MIN_PORT_NUMBER; nPort <= SYS_ADPT_POE_PSE_MAX_PORT_NUMBER; nPort++)
        {
            if(poe_om_pse_table[unit-1][nPort-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_priority
                    == VAL_pethPsePortPowerPriority_critical)
            {
                power += poe_om_pse_table[unit-1][nPort-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_max_allocation;
            }
        }
        *value = power;
        POE_OM_LeaveCriticalSection();

        return TRUE;
    }
    else
    {
        return FALSE
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetExistedHighPriorityPowerMaxAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get maximum power of the high
             priority port except the specific port.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
void POE_OM_GetExistedHighPriorityPowerMaxAllocation(UI32_T unit, UI32_T port_index, UI32_T *value)
{
    UI32_T nPort;
    UI32_T power = 0;

    POE_OM_EnterCriticalSection();
    for (nPort = SYS_ADPT_POE_PSE_MIN_PORT_NUMBER; nPort <= SYS_ADPT_POE_PSE_MAX_PORT_NUMBER; nPort++)
    {
        if(port_index == nPort)
        {
            continue;
        }
        if(poe_om_pse_table[unit-1][nPort-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_priority
                == VAL_pethPsePortPowerPriority_critical)
        {
            power += poe_om_pse_table[unit-1][nPort-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_power_max_allocation;
        }
    }
    *value = power;

    POE_OM_LeaveCriticalSection();
}
#endif

#ifdef SYS_CPNT_POE_PSE_DOT3AT
#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - POE_OM_SetPortManualHighPowerMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get current manual high power mode
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
void POE_OM_SetPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T mode)
{
    POE_OM_EnterCriticalSection();
    poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_force_power_mode = mode;
    POE_OM_LeaveCriticalSection();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_OM_GetPortManualHighPowerMode
 *-------------------------------------------------------------------------
 * FUNCTION : This function is used to Get current manual high power mode
 * INPUT    : group_index
 *            port_index
 * OUTPUT   : mode : 1 - force high power, 0 - normal
 * RETURN   : TURE or FALSE
 * NOTE     :
 *-------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T *mode)
{
    if (POE_OM_UserPortExisting(group_index, port_index) == TRUE)
    {
        POE_OM_EnterCriticalSection();
        *mode = poe_om_pse_table[group_index-1][port_index-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].pse_port_force_power_mode;
        POE_OM_LeaveCriticalSection();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} /* end POE_OM_GetPortManualHighPowerMode() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPortManualHighPowerMode
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the high-power of the port from the point
             of view of a power management algorithm.
 * INPUT   : group_index
             port_index
 * OUTPUT  : value = normal(0)
                   = high-power(1)

 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T *value)
{
    if (!POE_OM_GetPortManualHighPowerMode(group_index, port_index, value))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    /* normal mode is default mode
     * need to modified to use naming constant..
     */
    if (*value == PSE_PORT_POWER_HIGHPOWER_MODE_DEF)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    else
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}
#endif
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_OM_GetPortDot3atPowerInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the poe infomation for LLDP to transmition frame
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_OM_GetPortDot3atPowerInfo(UI32_T group_index, UI32_T port_index, POE_TYPE_Dot3atPowerInfo_T *info)
{

    if (POE_ENGINE_GetPortDot3atPowerInfo(group_index, port_index, info)==TRUE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}/* End of POE_MGR_GetDot3atPowerMode */
#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - POE_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for POE OM.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T POE_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    POE_OM_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (POE_OM_IpcMsg_T*)msgbuf_p->msg_buf;

    /* dispatch IPC message and call the corresponding POE_OM function
     */
    switch (msg_p->type.cmd)
    {
        case POE_OM_IPC_GETPSEPORTADMIN:
            msg_p->type.ret_ui32 =
                POE_OM_GetPsePortAdmin(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETPSEPORTPOWERPRIORITY:
            msg_p->type.ret_ui32 =
                POE_OM_GetPsePortPowerPriority(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETPSEPORTDECTECTIONSTATUS:
            msg_p->type.ret_ui32 =
                POE_OM_GetPsePortDetectionStatus(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETPORTPOWERMAXIMUMALLOCATION:
            msg_p->type.ret_ui32 =
                POE_OM_GetPortPowerMaximumAllocation(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETPORTPOWERCONSUMPTION:
            msg_p->type.ret_ui32 =
                POE_OM_GetPsePortPowerConsumption(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
        case POE_OM_IPC_GETPORTMANUALHIGHPOWERMODE:
            msg_p->type.ret_ui32 =
                POE_OM_GetPortManualHighPowerMode(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;
#endif

        case POE_OM_IPC_GETMAINPOWERMAXIMUMALLOCATION:
            msg_p->type.ret_ui32 =
                POE_OM_GetMainpowerMaximumAllocation(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        case POE_OM_IPC_GETPOESOFTWAREVERSION:
            msg_p->type.ret_ui32 =
                POE_OM_GetPoeSoftwareVersion(msg_p->data.arg_grp_ui32_ui8_ui8_ui8.arg_ui32, &msg_p->data.arg_grp_ui32_ui8_ui8_ui8.arg_ui8_1, &msg_p->data.arg_grp_ui32_ui8_ui8_ui8.arg_ui8_2, &msg_p->data.arg_grp_ui32_ui8_ui8_ui8.arg_ui8_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui8_ui8_ui8);
            break;

        case POE_OM_IPC_GETNEXTLEGACYDETECTION:
            msg_p->type.ret_ui32 =
                POE_OM_GetNextLegacyDetection(&msg_p->data.arg_grp_ui32_ui32.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        case POE_OM_IPC_GETNEXTPSEPORTADMIN:
            msg_p->type.ret_ui32 =
                POE_OM_GetNextPsePortAdmin(&msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETNEXTMAINPSEENTRY:
            msg_p->type.ret_ui32 =
                POE_OM_GetNextMainPseEntry(&msg_p->data.arg_grp_ui32_mainpse.arg_ui32, &msg_p->data.arg_grp_ui32_mainpse.arg_entry);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_mainpse);
            break;

        case POE_OM_IPC_GETPETHMAINPSEENTRY:
            msg_p->type.ret_ui32 =
                POE_OM_GetPethMainPseEntry(msg_p->data.arg_grp_ui32_mainpse.arg_ui32, &msg_p->data.arg_grp_ui32_mainpse.arg_entry);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_mainpse);
            break;

        case POE_OM_IPC_GETPSEPORTENTRY:
            msg_p->type.ret_ui32 =
                POE_OM_GetPsePortEntry(msg_p->data.arg_grp_ui32_ui32_pseport.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_pseport.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_pseport.arg_entry);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_pseport);
            break;

        case POE_OM_IPC_GETRUNNINGPSEPORTADMIN:
            msg_p->type.ret_ui32 =
                POE_OM_GetRunningPsePortAdmin(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETRUNNINGPSEPORTPOWERPRIORITY:
            msg_p->type.ret_ui32 =
                POE_OM_GetRunningPsePortPowerPriority(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETRUNNINGPORTPOWERMAXIMUMALLOCATION:
            msg_p->type.ret_ui32 =
                POE_OM_GetRunningPortPowerMaximumAllocation(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETRUNNINGMAINPOWERMAXIMUMALLOCATION:
            msg_p->type.ret_ui32 =
                POE_OM_GetRunningMainpowerMaximumAllocation(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        case POE_OM_IPC_GETRUNNINGLEGACYDETECTION:
            msg_p->type.ret_ui32 =
                POE_OM_GetRunningLegacyDetection(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        case POE_OM_IPC_GETMAINPSEOPERSTATUS:
            msg_p->type.ret_ui32 =
                POE_OM_GetMainPseOperStatus(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        case POE_OM_IPC_GETPSEPORTPOWERPAIRSCTRLABILITY:
            msg_p->type.ret_ui32 =
                POE_OM_GetPsePortPowerPairsCtrlAbility(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETPSEPORTPOWERPAIRS:
            msg_p->type.ret_ui32 =
                POE_OM_GetPsePortPowerPairs(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETPSEPORTPOWERCLASSIFICATIONS:
            msg_p->type.ret_ui32 =
                POE_OM_GetPsePortPowerClassifications(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETPORTPOWERCURRENT:
            msg_p->type.ret_ui32 =
                POE_OM_GetPortPowerCurrent(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETPORTPOWERVOLTAGE:
            msg_p->type.ret_ui32 =
                POE_OM_GetPortPowerVoltage(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETNEXTPSEPORTENTRY:
            msg_p->type.ret_ui32 =
                POE_OM_GetNextPsePortEntry(&msg_p->data.arg_grp_ui32_ui32_pseport.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32_pseport.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_pseport.arg_entry);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_pseport);
            break;

        case POE_OM_IPC_GETPSENOTIFICATIONCONTROL:
            msg_p->type.ret_ui32 =
                POE_OM_GetPseNotificationCtrl(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        case POE_OM_IPC_GETNEXTNOTIFICATIONCONTROL:
            msg_p->type.ret_ui32 =
                POE_OM_GetNextNotificationCtrl(&msg_p->data.arg_grp_ui32_ui32.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        case POE_OM_IPC_GETDOT3ATPORTPOWERINFO:
            msg_p->type.ret_ui32 =
                POE_OM_GetPortDot3atPowerInfo(msg_p->data.arg_grp_ui32_ui32_3atInfo.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_3atInfo.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_3atInfo.arg_entry);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_3atInfo);
            break;

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
        case POE_OM_IPC_GETRUNNINGPORTMANUALHIGHPOWERMODE:
            msg_p->type.ret_ui32 =
                POE_OM_GetRunningPortManualHighPowerMode(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;
#endif

#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
        case POE_OM_IPC_GETUSELOCALPOWER:
            msg_p->type.ret_ui32 =
                POE_OM_GetUseLocalPower(msg_p->data.arg_grp_ui32_bool.arg_ui32, &msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_bool);
            break;
#endif

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
        case POE_OM_IPC_GETPSEPORTTIMERANGENAME:
            msg_p->type.ret_ui32 =
                POE_OM_GetPsePortTimeRangeName(msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2, msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui8);
            break;

        case POE_OM_IPC_GETNEXTPSEPORTTIMERANGENAME:
            msg_p->type.ret_ui32 =
                POE_OM_GetNextPsePortTimeRangeName(&msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2, msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui8);
            break;

        case POE_OM_IPC_GETRUNNINGPSEPORTTIMERANGENAME:
            msg_p->type.ret_ui32 =
                POE_OM_GetRunningPsePortTimeRangeName(msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2, msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui8);
            break;

        case POE_OM_IPC_GETNEXTRUNNINGPSEPORTTIMERANGENAME:
            msg_p->type.ret_ui32 =
                POE_OM_GetNextRunningPsePortTimeRangeName(&msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui32_2, msg_p->data.arg_grp_ui32_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui8);
            break;

        case POE_OM_IPC_GETPSEPORTTIMERANGESTATUS:
            msg_p->type.ret_ui32 =
                POE_OM_GetPsePortTimeRangeStatus(msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;

        case POE_OM_IPC_GETNEXTPSEPORTTIMERANGESTATUS:
            msg_p->type.ret_ui32 =
                POE_OM_GetNextPsePortTimeRangeStatus(&msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2, &msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            msgbuf_p->msg_size = POE_OM_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
            break;
#endif

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_ui32 = POE_TYPE_RETURN_ERROR;
            msgbuf_p->msg_size = POE_OM_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of POE_OM_HandleIPCReqMsg */


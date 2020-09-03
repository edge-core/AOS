/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_om.c
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    This file provides the APIs for POEDRV OM to read/write the database.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    07/01/2009 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sys_time.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"
#include "sysrsc_mgr.h"
#include "sysfun.h"
#include "stktplg_pom.h"
#include "leaf_3621.h"
#include "backdoor_mgr.h"
#include "poedrv_type.h"
#include "stktplg_om.h"
#include "stktplg_board.h"
//#include "poedrv_control.h"


/* NAME CONSTANT DECLARATIONS
 */
//#define POEDRV_OM_ENTER_CRITICAL_SECTION    SYSFUN_ENTER_CRITICAL_SECTION(poedrv_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
//#define POEDRV_OM_LEAVE_CRITICAL_SECTION    SYSFUN_LEAVE_CRITICAL_SECTION(poedrv_om_sem_id);
#define POEDRV_OM_EnterCriticalSection() SYSFUN_TakeSem(poedrv_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define POEDRV_OM_LeaveCriticalSection() SYSFUN_GiveSem(poedrv_om_sem_id)

#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

#if 0
static UI32_T POE_PORT_L2P_MATRIX[SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT+1] = 
//{1,0,3,2,5,4,7,6,9,8,11,10,13,12,15,14,17,16,19,18,21,20,23,22};
//{11,10,9,8,7,6,5,4,3,2,1,0,23,22,21,20,19,18,17,16,15,14,13,12};
{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
static UI32_T POE_PORT_P2L_MATRIX[SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT+1] = 
//{2,1,4,3,6,5,8,7,10,9,12,11,14,13,16,15,18,17,20,19,22,21,24,23};
//{12,11,10,9,8,7,6,5,4,3,2,1,24,23,22,21,20,19,18,17,16,15,14,13};
{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
#endif
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
static POEDRV_TYPE_ShmemData_T *shmem_data_p;

static UI32_T poedrv_om_sem_id;          /* PoE driver semaphore ID */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 

/* FUNCTION NAME : POEDRV_OM_Logical2PhyDevicePortID
 * PURPOSE: This function initializes all releated variables and restarts
 *          the PoE driver.
 * INPUT:   port->logical port num;phy_port->address of physical port num
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
BOOL_T	POEDRV_OM_Logical2PhyDevicePortID(UI32_T port,UI32_T *phy_port)
{
    if(port >= SYS_ADPT_POE_PSE_MIN_PORT_NUMBER && port <= SYS_ADPT_POE_PSE_MAX_PORT_NUMBER)
    {
        POEDRV_OM_EnterCriticalSection();

        if(shmem_data_p->poedrv_port_L2P_matrix[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER] != 0xFF)
        {
            *phy_port = shmem_data_p->poedrv_port_L2P_matrix[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER];

            POEDRV_OM_LeaveCriticalSection();

            return TRUE;
        }
        else
        {
            POEDRV_OM_LeaveCriticalSection();
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

/* FUNCTION NAME : POEDRV_OM_Physical2LogicalPort
 * PURPOSE: This function initializes all releated variables and restarts
 *          the PoE driver.
 * INPUT:   port->phisical port num; lport->address of logical port num
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
BOOL_T POEDRV_OM_Physical2LogicalPort(UI32_T port, UI32_T *lport)
{
    if(port >=0 && port < POEDRV_NO_OF_POE_PORTS)
    {
        POEDRV_OM_EnterCriticalSection();

    	if(shmem_data_p->poedrv_port_P2L_matrix[port] != 0xFF)        /* Add by Andy  1104-05 */
    	{
            *lport = shmem_data_p->poedrv_port_P2L_matrix[port];

			POEDRV_OM_LeaveCriticalSection();

            return TRUE;
        }
        else
        {
            POEDRV_OM_LeaveCriticalSection();

            return FALSE;
        }
    }
    else
        return FALSE;
}


void POEDRV_OM_Reset(void)
{
//Eugene temp,    STKTPLG_BOARD_BoardInfo_T  *board_info_p;
//Eugene temp,    UI32_T board_id;
    UI32_T port = 0, my_unit = 0xFF, total_unit = 0xFF;
    UI32_T board_id = 0xFF;
    STKTPLG_BOARD_BoardInfo_T board_info;


    DBG_PRINT();

#ifdef INCLUDE_DIAG
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_my_unit_id = 1;
    shmem_data_p->poedrv_num_of_units = 1;
#else
    STKTPLG_OM_GetMyUnitID(&my_unit);
    STKTPLG_OM_GetNumberOfUnit(&total_unit);
    STKTPLG_OM_GetUnitBoardID(my_unit, &board_id);
    STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);

    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_my_unit_id = my_unit;
    shmem_data_p->poedrv_mainpower_info.unit_id = (UI8_T) my_unit;
    shmem_data_p->poedrv_num_of_units = total_unit;
#endif

#if 0 /* Eugene temp, check if can call stktplg_board api directly */
    /* Get board_id */
    if ( STKTPLG_OM_GetUnitBoardID(my_unit, &board_id) )
    {
        /* Get POEDRV board_info and set relative vlaue */
        if ( STKTPLG_BOARD_GetBoardInformation( board_id, &board_info_p))
        {
            main_pse_power_max_allocation =  board_info_p->main_pse_power_max_allocation;

            memcpy(poedrv_port_L2P_matrix,&board_info_p->Logical2PhysicalPort,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

            memcpy(poedrv_port_P2L_matrix,&board_info_p->Physical2LogicalPort,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

            memcpy(per_port_power_max_allocation,&board_info_p->per_port_power_max_allocation,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

        }
        else
            SYSFUN_Debug_Printf("\n\r*** Can not get related board information.***");
    }
    else
        SYSFUN_Debug_Printf("\n\r*** Fail in getting board id!***");
#endif


    memset(&shmem_data_p->poedrv_mainpower_info, 0, sizeof(POEDRV_Mainpower_Info_T));
    memset(shmem_data_p->poedrv_port_info, 0, sizeof(POEDRV_Port_Info_T)*(POEDRV_NO_OF_POE_PORTS));

#if 0
    for (port=0;port<POEDRV_NO_OF_POE_PORTS;port++)
        memset(shmem_data_p->poedrv_port_counter[port], 0, sizeof(UI32_T)*POEDRV_MAX_COUNTER_TYPE);
    memset(shmem_data_p->poedrv_pingpong_info, 0, sizeof(POEDRV_PINGPONG_DETECTION_T)*POEDRV_NO_OF_POE_PORTS);
#endif

    for (port=0;port<POEDRV_NO_OF_POE_PORTS;port++)
    {
        shmem_data_p->poedrv_port_state[port] = POEDRV_PORT_IS_OFF;

        shmem_data_p->poedrv_port_L2P_matrix[port] = 0xFF;
        shmem_data_p->poedrv_port_P2L_matrix[port] = 0xFF;
        shmem_data_p->per_port_power_max_allocation[port] = SYS_DFLT_PSE_PORT_POWER_MAX_ALLOCATION;
    }

    memcpy(shmem_data_p->poedrv_port_L2P_matrix,board_info.Logical2PhysicalPort,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);
    memcpy(shmem_data_p->poedrv_port_P2L_matrix,board_info.Physical2LogicalPort,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);
//    memcpy(shmem_data_p->poedrv_port_L2P_matrix,POE_PORT_L2P_MATRIX,sizeof(UI32_T)*SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT);
//    memcpy(shmem_data_p->poedrv_port_P2L_matrix,POE_PORT_P2L_MATRIX, sizeof(UI32_T)*SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT);

    shmem_data_p->main_pse_power_max_allocation = POEDRV_TYPE_PSE_POWER_MAX_ALLOCATION_LOCAL;

    shmem_data_p->poedrv_min_port_number = POEDRV_TYPE_PSE_MIN_PORT_NUMBER;
    shmem_data_p->poedrv_max_port_number = POEDRV_TYPE_PSE_MAX_PORT_NUMBER;
    shmem_data_p->poedrv_poe_image_version_1 = 0xFF;
    shmem_data_p->poedrv_poe_image_version_2 = 0xFF;
    shmem_data_p->poedrv_poe_image_build= 0xFF;
	
    /* Initialize Main PSE with default value
     */
    shmem_data_p->poedrv_mainpower_info.unit_id              = (UI8_T) shmem_data_p->poedrv_my_unit_id;
    shmem_data_p->poedrv_mainpower_info.board_id             = board_id;
    shmem_data_p->poedrv_mainpower_info.main_pse_power       = MIN_pethMainPsePower;
    shmem_data_p->poedrv_mainpower_info.main_pse_oper_status = VAL_pethMainPseOperStatus_faulty;
    shmem_data_p->poedrv_mainpower_info.main_pse_consumption = 0;
    shmem_data_p->poedrv_mainpower_info.legacy_detection_enable = 0;

    /* Initialize port PSE with default value
     */
    for ( port=0; port<POEDRV_NO_OF_POE_PORTS; port++)
    {
        shmem_data_p->poedrv_port_info[port].detection_status  = VAL_pethPsePortDetectionStatus_disabled;
        shmem_data_p->poedrv_port_info[port].power_class       = VAL_pethPsePortPowerClassifications_class0;
        shmem_data_p->poedrv_port_info[port].power_consumption = 0;
        shmem_data_p->poedrv_port_info[port].is_overload       = FALSE;
        shmem_data_p->poedrv_port_info[port].admin_status      = FALSE;
        shmem_data_p->poedrv_port_info[port].poe_linkup        = FALSE;
        shmem_data_p->poedrv_port_info[port].poe_active        = FALSE;
        shmem_data_p->poedrv_port_info[port].is_port_failure   = FALSE;
        /* Aaron 2007-10-05, patch the code for non-802.3af LED incorrect issue, like Cisco 7910, 7920 and 7960
         * Use unused default value to prevent the default value  same as non-802.3af final status
         * Original default value is 0x00 (same as non-802.3af final status and cause PoE LED not correct
         * when connect one non-802.3af device to switch and bootup again.
         */
        shmem_data_p->poedrv_port_info[port].actual_status     = 0xFF;
    }

    shmem_data_p->poedrv_provision_complete = FALSE;
    shmem_data_p->poedrv_hw_enable = FALSE;
    shmem_data_p->is_stop_polling = TRUE;

    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_ResetByBoardID(void)
{
    UI32_T unit_id = 0xFF, board_id = 0xFF;
    STKTPLG_BOARD_BoardInfo_T board_info;


    memset(&board_info, 0, sizeof(board_info));
    if (STKTPLG_OM_GetMyUnitID(&unit_id) == FALSE)
    {
        printf("\r\n%s(%d): Get Unit ID Fail.", __FUNCTION__, __LINE__);
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }
    else if (STKTPLG_OM_GetUnitBoardID(unit_id, &board_id) == FALSE)
    {
        printf("\r\n%s(%d): Get Board ID Fail.", __FUNCTION__, __LINE__);
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }
    else if (STKTPLG_BOARD_GetBoardInformation(board_id, &board_info) == FALSE)
    {
        printf("\r\n%s(%d): Get Board Information Fail.", __FUNCTION__, __LINE__);
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    POEDRV_OM_EnterCriticalSection();
    memcpy(shmem_data_p->poedrv_port_L2P_matrix, board_info.Logical2PhysicalPort, sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);
    memcpy(shmem_data_p->poedrv_port_P2L_matrix, board_info.Physical2LogicalPort, sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

    memcpy(shmem_data_p->per_port_power_max_allocation, board_info.per_port_power_max_allocation, sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);
    shmem_data_p->main_pse_power_max_allocation = board_info.main_pse_power_max_allocation;
    shmem_data_p->poedrv_mainpower_info.main_pse_power = shmem_data_p->main_pse_power_max_allocation;
    shmem_data_p->poedrv_min_port_number = board_info.min_pse_port_number;
    shmem_data_p->poedrv_max_port_number = board_info.max_pse_port_number;

    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_Init(void)
{
    UI32_T ret_value;

    shmem_data_p = (POEDRV_TYPE_ShmemData_T *)SYSRSC_MGR_GetShMem(SYSRSC_MGR_POEDRV_SHMEM_SEGID);

    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);

    if ((ret_value = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_POEDRV_OM, &poedrv_om_sem_id))!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_GetSem return != SYSFUN_OK value=%lu\n",__FUNCTION__,ret_value);
    }
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for POEDRV_OM
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_AttachSystemResources(void)
{
    UI32_T ret_value;

    shmem_data_p = (POEDRV_TYPE_ShmemData_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_POEDRV_SHMEM_SEGID);

    if ((ret_value = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_POEDRV_OM, &poedrv_om_sem_id))!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_GetSem return != SYSFUN_OK value=%lu\n",__FUNCTION__,ret_value);
    }

}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_EnterMasterMode (void)
{
DBG_PRINT();
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_EnterSlaveMode (void)
{
DBG_PRINT();
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Sets to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_SetTransitionMode (void)
{
DBG_PRINT();
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_EnterTransitionMode (void)
{
DBG_PRINT();
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_GetOperatingMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Get operating mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
UI32_T POEDRV_OM_GetOperatingMode()
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p);
}

void POEDRV_OM_SetProvisionComplete(BOOL_T status)
{

    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_provision_complete = status;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetProvisionComplete(BOOL_T *status)
{
    POEDRV_OM_EnterCriticalSection();
    *status = shmem_data_p->poedrv_provision_complete;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetHwEnable(BOOL_T status)
{

    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_hw_enable = status;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetHwEnable(BOOL_T *status)
{
    POEDRV_OM_EnterCriticalSection();
    *status = shmem_data_p->poedrv_hw_enable;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMyUnitID(UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_my_unit_id = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMyUnitID(UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_my_unit_id;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetNumOfUnits(UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_num_of_units = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetNumOfUnits(UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_num_of_units;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerMaxAllocation(UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->main_pse_power_max_allocation;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPOEPortNumber(UI32_T *min, UI32_T *max)
{
    POEDRV_OM_EnterCriticalSection();
    *min = shmem_data_p->poedrv_min_port_number;
    *max = shmem_data_p->poedrv_max_port_number;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetImageVersion(UI8_T value_1, UI8_T value_2)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_poe_image_version_1 = value_1;
    shmem_data_p->poedrv_poe_image_version_2 = value_2;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetImageVersion(UI8_T *value_1, UI8_T *value_2)
{
    POEDRV_OM_EnterCriticalSection();
    *value_1 = shmem_data_p->poedrv_poe_image_version_1;
    *value_2 = shmem_data_p->poedrv_poe_image_version_2;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetImageBuild(UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_poe_image_build = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetImageBuild(UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_poe_image_build;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMainPowerInfoUnitID(UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_mainpower_info.unit_id = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerInfoUnitID(UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_mainpower_info.unit_id;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMainPowerInfoBoardID(UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_mainpower_info.board_id = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerInfoBoardID(UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_mainpower_info.board_id;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMainPowerInfoMainOperStatus(UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_mainpower_info.main_pse_oper_status = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerInfoMainOperStatus(UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_mainpower_info.main_pse_oper_status;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMainPowerInfoMainPower(UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_mainpower_info.main_pse_power = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerInfoMainPower(UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_mainpower_info.main_pse_power;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMainPowerInfoMainConsumption(UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_mainpower_info.main_pse_consumption = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerInfoMainConsumption(UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_mainpower_info.main_pse_consumption;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetMainPowerInfoLegacyDectionEnable(UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_mainpower_info.legacy_detection_enable = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetMainPowerInfoLegacyDectionEnable(UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_mainpower_info.legacy_detection_enable;
    POEDRV_OM_LeaveCriticalSection();
}



void POEDRV_OM_SetPortInfoPowerConsumption(UI32_T port, UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].power_consumption = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoPowerConsumption(UI32_T port, UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].power_consumption;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoDetectionStatus(UI32_T port, UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].detection_status = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoDetectionStatus(UI32_T port, UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].detection_status;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoPowerClass(UI32_T port, UI32_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].power_class = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoPowerClass(UI32_T port, UI32_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].power_class;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoIsOverload(UI32_T port, BOOL_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].is_overload = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoIsOverload(UI32_T port, BOOL_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].is_overload;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoActualStatus(UI32_T port, UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].actual_status = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoActualStatus(UI32_T port, UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].actual_status;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoLinkUp(UI32_T port, UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].poe_linkup = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoLinkUp(UI32_T port, UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].poe_linkup;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoActive(UI32_T port, UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].poe_active = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoActive(UI32_T port, UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].poe_active;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoIsPortFailure(UI32_T port, BOOL_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].is_port_failure = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoIsPortFailure(UI32_T port, BOOL_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].is_port_failure;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_SetPortInfoAdminStatus(UI32_T port, UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].admin_status = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortInfoAdminStatus(UI32_T port, UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_info[port-SYS_ADPT_POE_PSE_MIN_PORT_NUMBER].admin_status;
    POEDRV_OM_LeaveCriticalSection();
}


void POEDRV_OM_SetPortState(UI32_T port, UI8_T value)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->poedrv_port_state[port-1] = value;
    POEDRV_OM_LeaveCriticalSection();
}

void POEDRV_OM_GetPortState(UI32_T port, UI8_T *value)
{
    POEDRV_OM_EnterCriticalSection();
    *value = shmem_data_p->poedrv_port_state[port-1];
    POEDRV_OM_LeaveCriticalSection();
}

/* FUNCTION NAME: POEDRV_OM_IsStopMonitorFlagOn
 * PURPOSE:       This function is used to get the status of query function in PoE.
 * INPUT:         None.
 * OUTPUT:        None.
 * RETURN:        TRUE  -- Stop to poll.
 *                FALSE -- In polling.
 * NOTES:
 */
BOOL_T POEDRV_OM_IsStopMonitorFlagOn(void)
{
    BOOL_T is_stop_polling = FALSE;


    POEDRV_OM_EnterCriticalSection();
    is_stop_polling = shmem_data_p->is_stop_polling;
    POEDRV_OM_LeaveCriticalSection();

    return is_stop_polling;
}

/* FUNCTION NAME: POEDRV_OM_SetStopMonitorFlag
 * PURPOSE:       This function is used to set the flag of poe monitor function.
 * INPUT:         state: TRUE  -- Stop to poll.
 *                       FALSE -- In polling.
 * OUTPUT:        None.
 * RETURN:        None.
 *
 * NOTES:
 */
void POEDRV_OM_SetStopMonitorFlag(BOOL_T state)
{
    POEDRV_OM_EnterCriticalSection();
    shmem_data_p->is_stop_polling = state;
    POEDRV_OM_LeaveCriticalSection();
}


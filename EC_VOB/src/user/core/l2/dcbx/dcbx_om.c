/*-----------------------------------------------------------------------------
 * Module Name: dcbx_om.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementation for the DCBX object manager
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    9/20/2012 - Ricky Lin, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2012
 *-----------------------------------------------------------------------------
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"

#include "l_cvrt.h"
#include "l_hisam.h"
#include "l_lst.h"
#include "l_mpool.h"
#include "l_sort_lst.h"
#include "sysfun.h"
#include "dcbx_om.h"
#include "dcbx_om_private.h"
#include "dcbx_type.h"
#include "swctrl.h"


#define RX_DEBUG_PRINT  0

/* system operation entry */
static DCBX_OM_SystemOperEntry_T   DCBX_OM_SysOper;

/* port configuration entry*/
static DCBX_OM_PortConfigEntry_T   DCBX_OM_PortConfig[SYS_ADPT_TOTAL_NBR_OF_LPORT];

/* semaphore id */
static UI32_T                      DCBX_OM_SemId;

static  UI32_T                     original_priority;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetPortConfigEntryPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the specified port configuration
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : DCBX_OM_PortConfigEntry_T*
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
DCBX_OM_PortConfigEntry_T* DCBX_OM_GetPortConfigEntryPtr(UI32_T lport)
{
    return &DCBX_OM_PortConfig[lport-1];
}/* End of DCBX_OM_GetPortConfigEntryPtr */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetSysOper
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the system operation
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : DCBX_OM_SystemOperEntry_T*
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
DCBX_OM_SystemOperEntry_T* DCBX_OM_GetSysOper(void)
{
    return &DCBX_OM_SysOper;
}/* End of DCBX_OM_GetSysOper */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_Init
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void DCBX_OM_Init(void)
{

}/* End of DCBX_OM_Init */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the semaphore for DCBX objects
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void DCBX_OM_InitSemaphore(void)
{
    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO,
            &DCBX_OM_SemId) != SYSFUN_OK)
    {
        printf("\n%s: get dcbx om sem id fail.\n", __FUNCTION__);
    }

    return;
}/* End of DCBX_OM_InitSemaphore */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_ResetAll
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset the database
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void DCBX_OM_ResetAll()
{
    UI32_T  index;

    memset(&DCBX_OM_SysOper, 0, sizeof(DCBX_OM_SystemOperEntry_T));

    for(index = 0; index < SYS_ADPT_TOTAL_NBR_OF_LPORT; index++)
    {
        memset(&DCBX_OM_PortConfig[index], 0, sizeof(DCBX_OM_PortConfigEntry_T));
    }
    return ;
}/* End of DCBX_OM_ResetAll*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_ResetPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset all dynamic status of each port and the clean the database
 * INPUT    : lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void DCBX_OM_ResetPort(UI32_T   lport)
{
    DCBX_OM_PortConfig[lport - 1].lport_num = lport;
//    DCBX_OM_PortConfig[lport - 1].port_status = DCBX_TYPE_DEFAULT_PORT_STATUS;
//    DCBX_OM_PortConfig[lport - 1].port_mode = DCBX_TYPE_DEFAULT_PORT_MODE;
    DCBX_OM_PortConfig[lport - 1].ets_sm_state = DCBX_TYPE_ASYM_DISABLE_STATE;
    DCBX_OM_PortConfig[lport - 1].pfc_sm_state = DCBX_TYPE_SYM_DISABLE_STATE;
    DCBX_OM_PortConfig[lport - 1].is_peer_detected = FALSE;
    return ;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_EnterCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical section before access om
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   The task priority of the caller is raised to
 *              SYS_BLD_RAISE_TO_HIGH_PRIORITY.
 *-------------------------------------------------------------------------
 */
void  DCBX_OM_EnterCriticalSection(void)
{
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(DCBX_OM_SemId);/* pgr0695 */
    return;
}/* End of DCBX_OM_EnterCriticalSection */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_LeaveCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical section after access om
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   The task priority of the caller is set to the original
 *              task priority.
 *-------------------------------------------------------------------------
 */
void  DCBX_OM_LeaveCriticalSection(void)
{
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(DCBX_OM_SemId, original_priority);
}/* End of DCBX_OM_LeaveCriticalSection */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_SetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port status
 * INPUT    : lport
 *            port_status
 * OUTPUT   : None
 * RETUEN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_SetPortStatus(UI32_T lport, BOOL_T port_status)
{
    UI32_T                  result;
    DCBX_OM_PortConfigEntry_T    *om_port_config;

    result = DCBX_TYPE_RETURN_ERROR;
    om_port_config = DCBX_OM_GetPortConfigEntryPtr(lport);

    om_port_config->port_status = port_status;
    result = DCBX_TYPE_RETURN_OK;

    return result;
}/* End of DCBX_OM_SetPortStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port status
 * INPUT    : lport
 * OUTPUT   : port_status_p
 * RETUEN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_GetPortStatus(UI32_T lport, BOOL_T *port_status_p)
{
    UI32_T                  result;
    DCBX_OM_PortConfigEntry_T    *om_port_config;

    result = DCBX_TYPE_RETURN_ERROR;
    if (port_status_p == NULL)
    {
        return result;
    }

    om_port_config = DCBX_OM_GetPortConfigEntryPtr(lport);

    if (om_port_config == NULL)
    {
        return result;
    }

    *port_status_p = om_port_config->port_status;
    result = DCBX_TYPE_RETURN_OK;

    return result;
}/* End of DCBX_OM_GetPortStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetRunningPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : port_status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_GetRunningPortStatus(UI32_T lport, BOOL_T *port_status)
{
    UI32_T                  result;
    DCBX_OM_PortConfigEntry_T    *om_port_config;

    result              = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    om_port_config = DCBX_OM_GetPortConfigEntryPtr(lport);

    *port_status = om_port_config->port_status;
    if(om_port_config->port_status != DCBX_TYPE_DEFAULT_PORT_STATUS)
    {
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
}/* End of DCBX_OM_GetRunningPortStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_SetPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port mode
 * INPUT    : lport
 *            port_mode
 * OUTPUT   : None
 * RETUEN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_SetPortMode(UI32_T lport, UI32_T port_mode)
{
    UI32_T                  result;
    DCBX_OM_PortConfigEntry_T    *om_port_config;

    result              = DCBX_TYPE_RETURN_ERROR;
    om_port_config = DCBX_OM_GetPortConfigEntryPtr(lport);

    om_port_config->port_mode = port_mode;
    result = DCBX_TYPE_RETURN_OK;

    return result;
}/* End of DCBX_OM_SetPortMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port mode
 * INPUT    : lport
 * OUTPUT   : port_mode_p
 * RETUEN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_GetPortMode(UI32_T lport, UI32_T *port_mode_p)
{
    UI32_T                  result;
    DCBX_OM_PortConfigEntry_T    *om_port_config;

    result              = DCBX_TYPE_RETURN_ERROR;
    if (port_mode_p == NULL)
    {
        return result;
    }

    om_port_config = DCBX_OM_GetPortConfigEntryPtr(lport);

    if (om_port_config == NULL)
    {
        return result;
    }

    *port_mode_p = om_port_config->port_mode;
    result = DCBX_TYPE_RETURN_OK;

    return result;
}/* End of DCBX_OM_GetPortMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetRunningPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : port_mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_GetRunningPortMode(UI32_T lport, UI32_T *port_mode)
{
    UI32_T                  result;
    DCBX_OM_PortConfigEntry_T    *om_port_config;

    result              = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    om_port_config = DCBX_OM_GetPortConfigEntryPtr(lport);

    *port_mode = om_port_config->port_mode;
    if(om_port_config->port_mode != DCBX_TYPE_DEFAULT_PORT_MODE)
    {
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
}/* End of DCBX_OM_GetRunningPortMode */




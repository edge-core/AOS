/* Module Name: NICDRV_INIT.C
 * Purpose: 
 *  This module is in first task of NIC Driver, bring up whole nic driver.
 *  In the NIC driver, at least it will include lan driver. If driver is
 *  written by Accton ourselves, there will be another xxxnic driver and
 *  we need to bring up that nic driver as well.
 *  The bring up init file will include:
 *      1. NIC and LAN driver resource initialization
 *      2. NIC and LAN driver tasks creation if we need to
 *      3. Curretnly, we don't need any task neither LAN nor NIC.
 *
 * Notes: 
 *
 * History:                                                               
 *          Date      -- Modifier,        Reason
 * ------------------------------------------------------------------------
 *  V1.0  2001.09.25  -- Jason Hsue,      Creation
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "lan.h"
#include "sys_bld.h"
#include "l_threadgrp.h"
#include "dev_nicdrv_om.h"


/* FUNCTION NAME: NICDRV_INIT_Initiate_System_Resource
 *----------------------------------------------------------------------------------
 * PURPOSE: init NIC driver and LAN driver
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void NICDRV_INIT_InitiateSystemResources(void)
{
    DEV_NICDRV_Init(SYS_BLD_MAX_LAN_RX_BUF_SIZE_PER_PACKET, (void*)NULL, (void*)NULL);
    LAN_InitiateSystemResources();
}

void NICDRV_INIT_AttachSystemResources(void)
{
    DEV_NICDRV_OM_AttachSystemResources();
    LAN_AttachSystemResources();
}

void NICDRV_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    LAN_GetShMemInfo(segid_p, seglen_p);
}


/* FUNCTION NAME: NICDRV_INIT_Create_InterCSC_Relation
 *----------------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void NICDRV_INIT_Create_InterCSC_Relation(void)
{
    LAN_Create_InterCSC_Relation();
}

/* FUNCTION NAME: LAN_EnterTransitionMode
 *---------------------------------------------------------------------
 * PURPOSE: Enable the LAN activities as transition mode
 *---------------------------------------------------------------------
 * INPUT:   None
 * RETURN:  None.
 *---------------------------------------------------------------------
 * NOTE:	
 */
void NICDRV_INIT_EnterTransitionMode(void)
{
    DEV_NICDRV_EnterTransitionMode();
    LAN_EnterTransitionMode();    
}
 
/* FUNCTION NAME: NICDRV_INIT_SetTransitionMode
 *---------------------------------------------------------------------
 * PURPOSE: the LAN set into transition mode
 *---------------------------------------------------------------------
 * INPUT:   None
 * RETURN:  None.
 *---------------------------------------------------------------------
 * NOTE:
 */
void NICDRV_INIT_SetTransitionMode(void)
{
    DEV_NICDRV_SetTransitionMode();
    LAN_SetTransitionMode();    
}

/* FUNCTION NAME: NICDRV_INIT_EnterMasterMode
 *---------------------------------------------------------------------
 * PURPOSE: Enable the LAN activities as master mode
 *---------------------------------------------------------------------
 * INPUT:   None
 * RETURN:  None.
 *---------------------------------------------------------------------
 * NOTE:	
 */
void NICDRV_INIT_EnterMasterMode(void)
{
   DEV_NICDRV_EnterMasterMode();
   LAN_EnterMasterMode();     
}

/* FUNCTION NAME: NICDRV_INIT_EnterSlaveMode
 *---------------------------------------------------------------------
 * PURPOSE: Enable the LAN activities as slave mode
 *---------------------------------------------------------------------
 * INPUT:   None
 * RETURN:  None.
 *---------------------------------------------------------------------
 * NOTE:	
 */
void NICDRV_INIT_EnterSlaveMode(void)
{
    DEV_NICDRV_EnterSlaveMode();
    LAN_EnterSlaveMode();
}

/* FUNCTION NAME: NICDRV_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: create NIC task 
 *----------------------------------------------------------------------------------
 * INPUT:   tg_handle   --  thread group handle
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void NICDRV_INIT_Create_Tasks(void)
{
    DEV_NICDRV_CreateTask();
    LAN_CreateTask();
}

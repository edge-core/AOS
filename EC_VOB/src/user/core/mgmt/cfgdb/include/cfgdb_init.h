/*-----------------------------------------------------------------------------
 * Module Name: CFGDB_INIT.H
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES: None.
 * 
 *-----------------------------------------------------------------------------
 * HISTORY:
 *      03/21/2003  - Ryan,          Created
 *      04/08/2003 -- Vincent,       Modify
 *      04/30/2003 -- Charles Cheng, Take over
 *-----------------------------------------------------------------------------
 *  (C) Unpublished Work of Accton Technology,  Corp.  All Rights Reserved.
 *
 *      THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
 *      PROPRIETARY AND TRADESECRET INFORMATION OF ACCTON TECHNOLOGY CORP.
 *      ACCESS TO THIS WORK IS RESTRICTED TO (I) ACCTON EMPLOYEES WHO HAVE A
 *      NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
 *      AND (II) ENTITIES OTHER THAN ACCTON WHO HAVE ENTERED INTO APPROPRIATE
 *      LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
 *      PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
 *      ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
 *      TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF ACCTON.
 *      ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
 *      SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
 *-----------------------------------------------------------------------------
 */
#ifndef CFGDB_INIT_H
#define CFGDB_INIT_H

#include "sys_type.h"
#include "cfgdb_mgr.h"
#include "sysrsc_mgr.h"

#define CFGDB_INIT_BUFFER_SIZE      sizeof(CFGDB_MGR_Shmem_Data_T)

/* INCLUDE FILE DECLARATIONS
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_INIT_InitiateSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE  : Init CFGDB data.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void CFGDB_INIT_InitiateSystemResources(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for CFGDB in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void CFGDB_INIT_AttachSystemResources(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_INIT_GetShMemInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *---------------------------------------------------------------------------------*/
void CFGDB_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : called by root.c
 *-------------------------------------------------------------------------
 */
void CFGDB_INIT_Create_InterCSC_Relation(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_INIT_Create_Tasks
 *-------------------------------------------------------------------------
 * PURPOSE  : Create CFGDB task.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void CFGDB_INIT_Create_Tasks(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFGDB_INIT_EnterMasterMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter slave mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFGDB_INIT_EnterSlaveMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFGDB_INIT_EnterTransitionMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : Set transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFGDB_INIT_SetTransitionMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFGDB_INIT_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE : Notify that CLI has provision completed.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFGDB_INIT_ProvisionComplete(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_INIT_HandleHotInsertion
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.

 * -------------------------------------------------------------------------*/
void CFGDB_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: CFGDB_INIT_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void CFGDB_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif

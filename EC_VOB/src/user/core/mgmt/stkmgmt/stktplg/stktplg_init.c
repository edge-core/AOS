/* Module Name: STKTPLG_INIT.C
 * Purpose: This file contains the information of stack topology:
 *
 *          Master Mode:
 *
 *          1. topology discovery function to find the topology of working
 *             chassis.
 *          2. check if topology is changing, if changing, informing stack
 *             control module to re-initialize the system.
 *          3. collect information of every unit.
 *          4. because this module has all the information of the chassis,
 *             it should respond for the translation of different
 *             addressing mode.
 *          5. try to restore the information stored in the original unit.
 *             that means we should recreate the relation between logical
 *             unit id (addressing by user) to physical unit id
 *             (addressing by unique serial number of every unit).
 *          6. keep the relation between the interface number and the
 *             physical unit id since power on.
 * 
 *          Slave Mode:
 *
 *          1. support unit id for upper layer application.
 *          2. if we are backup master, synchronizing the interface number
 *             used information.
 *
 * Notes:   1. The purpose part is copied from MC2, need to modify 
 *             when coding finish.
 * History:                                                               
 *    07/04/01       -- Aaron Chuang, Create
 * Copyright(C)      Accton Corporation, 1999, 2000   				
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "sys_type.h"

#include "stktplg_backdoor.h"
#include "stktplg_om.h"
#include "stktplg_mgr.h"
#include "stktplg_task.h"
#include "stktplg_engine.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef STKTPLG_INIT_DEBUG
#define DBG(x...) printf(x)
#else
#define DBG(x...) 
#endif
/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : STKTPLG_INIT_InitiateProcessResources
 * PURPOSE: This function initializes STKTPLG which inhabits in the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_INIT_InitiateProcessResources(void)
{
    DBG("%s: check - 1\n",__FUNCTION__);
    if(FALSE==STKTPLG_OM_InitiateProcessResources())
    {
        printf("\r\n%s:STKTPLG_OM_AttachProcessResources fail.", __FUNCTION__);
        return FALSE;
    }
    DBG("%s: check - 2\n",__FUNCTION__);
    if(FALSE==STKTPLG_MGR_InitiateProcessResources())
    {
        printf("\r\n%s:STKTPLG_MGR_InitiateProcessResources fail.", __FUNCTION__);
        return FALSE;
    }
    DBG("%s: check - 3\n",__FUNCTION__);
    if(FALSE==STKTPLG_TASK_InitiateProcessResources())
    {
        printf("\r\n%s:STKTPLG_TASK_InitiateProcessResources fail.", __FUNCTION__);
        return FALSE;
    }

    if(FALSE==STKTPLG_ENGINE_InitiateProcessResources())
    {
        printf("\r\n%s:STKTPLG_ENGINE_InitiateProcessResources fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : STKTPLG_INIT_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration
 *          operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_INIT_Create_InterCSC_Relation(void)
{
    STKTPLG_BACKDOOR_Create_InterCSC_Relation();
} /* End of STKTPLG_INIT_Create_InterCSC_Relation() */

/* LOCAL SUBPROGRAM BODIES
 */


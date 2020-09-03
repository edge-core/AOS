/* MODULE NAME:  dying_gasp.h
 * PURPOSE:
 *     Dying gasp is a feature that will send trap just
 *     before power failure event occurs.
 *
 * NOTES:
 *     To simplify the call path, this CSC will be put
 *     in the snmp_proc process and the task will be
 *     spawned by SNMP CSC group. However, this CSC has no
 *     resource contention with SNMP CSC group, so no
 *     thread group protection will be used in this CSC.
 *
 * CREATOR:      Charlie Chen
 * HISTORY
 *    7/29/2010 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Corporation, 2010
 */
#ifndef DYING_GASP_H
#define DYING_GASP_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - DYING_GASP_InitiateProcessResource
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all process resource.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DYING_GASP_InitiateProcessResource(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DYING_GASP_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DYING_GASP_Create_InterCSC_Relation(void);

/* FUNCTION NAME : DYING_GASP_CreateTask
 * PURPOSE:
 *      Spawn task for dying gasp.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      Highest real time priority will be
 *      assigned to this task.
 */
void DYING_GASP_CreateTask(void);

#endif    /* End of DYING_GASP_H */

/* Module Name: STKCTRL_TASK.H
 * Purpose: This file contains the functions for control the stack modules
 *          when the system state is changed.
 *          Other detail description will be added when this module is
 *          finished.
 * Notes:   None.
 * History:
 *    07/03/01       -- Aaron Chuang, Create
 * Copyright(C)      Accton Corporation, 1999, 2000   				
 */
 
 
#ifndef STKCTRL_TASK_H
#define STKCTRL_TASK_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

typedef enum STKCTRL_TASK_Event_E
{
    /* no event
     */
    STKCTRL_TASK_EVENT_NONE                =   0x0000L,
    /* event coming from stack topology module,
     * it will inform stack control what system state currently
     * is.
     */ 
    STKCTRL_TASK_EVENT_STATE_CHANGE        =   0x0001L,
    /* when CLI completes mainboard configuration provision,
     * it will send this event to stack control.
     * so stack control will trigger module hot swap insertion and then
     * CLI to do module provision.
     */
    STKCTRL_TASK_EVENT_PROVISION_COMPLETE  =   0x0002L,
    /* when user issues "copy XX running", CLI will inform this event
     * to stack control. And stack control will re-initialized whole 
     * stack.
     */
    STKCTRL_TASK_EVENT_RUNNING_CONFIG      =   0x0004L,
    /* event coming from stack topology module,
     * it will inform stack control what module state change and
     * the ask stack topology which module state change and change to
     * which state.
     */
    STKCTRL_TASK_EVENT_MOULE_STATE_CHANGE  =   0x0008L,
    /* when CLI completes module configuration provision,
     * it will send this event to stack control.
     * so stack control will inform every module that
     * configuration provision is completed.
     */
    STKCTRL_TASK_EVENT_MOULE_PROVISION_COMPLETE  =   0x0010L,
    /* this event will let stack ctrl task to call system driver 
     * to reload system.
     */
    STKCTRL_TASK_EVENT_RELOAD_SYSTEM            = 0x0020L,
    /* this event will let stack ctrl task to call system driver 
     * to warm start system.
     */
    STKCTRL_TASK_EVENT_WARM_START_SYSTEM        = 0x0040L,
    /* this event will let stack ctrl task to call system driver 
     * to cold start system.
     */
    STKCTRL_TASK_EVENT_COLD_START_SYSTEM        = 0x0080L,
    /* this event will let stack ctrl task to renumber the unit id.
     */
    STKCTRL_TASK_EVENT_UNIT_ID_RENUMBER         = 0x0100L,    

 #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    /* event coming from stack topology module,
     * it will inform stack control that add/remove a unit 
     */
    STKCTRL_TASK_EVENT_UNIT_HOT_INSERT_REMOVE   = 0x0200L,
 #endif
} STKCTRL_TASK_Event_T;

 #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
#define STKCTRL_TASK_EVENT_ALL  (STKCTRL_TASK_EVENT_STATE_CHANGE | \
                                 STKCTRL_TASK_EVENT_RUNNING_CONFIG | \
                                 STKCTRL_TASK_EVENT_MOULE_STATE_CHANGE | \
                                 STKCTRL_TASK_EVENT_MOULE_PROVISION_COMPLETE | \
                                 STKCTRL_TASK_EVENT_PROVISION_COMPLETE | \
                                 STKCTRL_TASK_EVENT_RELOAD_SYSTEM | \
                                 STKCTRL_TASK_EVENT_WARM_START_SYSTEM | \
                                 STKCTRL_TASK_EVENT_COLD_START_SYSTEM | \
                                 STKCTRL_TASK_EVENT_UNIT_ID_RENUMBER | STKCTRL_TASK_EVENT_UNIT_HOT_INSERT_REMOVE)
#else
#define STKCTRL_TASK_EVENT_ALL  (STKCTRL_TASK_EVENT_STATE_CHANGE | \
                                 STKCTRL_TASK_EVENT_RUNNING_CONFIG | \
                                 STKCTRL_TASK_EVENT_MOULE_STATE_CHANGE | \
                                 STKCTRL_TASK_EVENT_MOULE_PROVISION_COMPLETE | \
                                 STKCTRL_TASK_EVENT_PROVISION_COMPLETE | \
                                 STKCTRL_TASK_EVENT_RELOAD_SYSTEM | \
                                 STKCTRL_TASK_EVENT_WARM_START_SYSTEM | \
                                 STKCTRL_TASK_EVENT_COLD_START_SYSTEM | \
                                 STKCTRL_TASK_EVENT_UNIT_ID_RENUMBER)
#endif
/* EXPORTED SUBPROGRAM DECLARATIONS
 */ 

/* FUNCTION NAME: STKCTRL_TASK_InitiateProcessResources
 * PURPOSE: This function is used to initialize the stack control module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T STKCTRL_TASK_InitiateProcessResources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - STKCTRL_TASK_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void STKCTRL_TASK_Create_InterCSC_Relation(void);

/* FUNCTION NAME: STKCTRL_TASK_HandleEvents
 * PURPOSE: This procedure handles the events for the topology control task.
 * INPUT:   events_p  --  events to be handled
 * OUTPUT:  events_p  --  events remains after this function is called
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_HandleEvents(UI32_T *events_p,UI32_T *unit);

/* FUNCTION NAME: STKCTRL_TASK_EnterTransitionMode_CallBack
 * PURPOSE: Hook function to receive enter transition event from CLI module.
 * INPUT:   None.
 * OUTPUT:  events_p  -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_EnterTransitionMode_CallBack(UI32_T *events_p);

/* FUNCTION NAME: STKCTRL_TASK_ProvisionComplete_CallBack
 * PURPOSE: Hook function to receive provision complete event from CLI module.
 * INPUT:   None.
 * OUTPUT:  events_p  -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_ProvisionComplete_CallBack(UI32_T *events_p);

/* FUNCTION NAME: STKCTRL_TASK_ModuleProvisionComplete_CallBack
 * PURPOSE: Hook function to receive module provision complete event from CLI module.
 * INPUT:   None.
 * OUTPUT:  events_p  -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_ModuleProvisionComplete_CallBack(UI32_T *events_p,UI32_T *unit);

/* FUNCTION NAME: STKCTRL_TASK_StackState_CallBack
 * PURPOSE: Hook function to receive event from STK_TPLG module.
 * INPUT:   msg     -- topology change msg(TPOGY_CHANGE, TPOGY_LOSE)
 * OUTPUT:  events  -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_StackState_CallBack(UI32_T stktplg_msg, UI32_T *events_p);

/* FUNCTION NAME: STKCTRL_TASK_ModuleStateChanged_CallBack
 * PURPOSE: Hook function to receive event from STK_TPLG module.
 * INPUT:   unit_id     -- The module state of which unit was changed.
 * OUTPUT:  events_p    -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_ModuleStateChanged_CallBack(UI32_T *unit_id, UI32_T *events_p);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME: STKCTRL_TASK_UnitHotInsertRemove_CallBack
 * PURPOSE: Hook function to receive event from STK_TPLG module.
 * INPUT:   msg     -- unit hot insert/remove
 * OUTPUT:  events  -- events remains after this function is called.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKCTRL_TASK_UnitHotInsertRemove_CallBack(UI32_T *events_p,UI32_T *unit);
#endif

/* events for stack topology task
 */
 
#define STKTPLG_TASK_EVENT_HELLO_TIMER   0x0004L
#define STKTPLG_TASK_EVENT_HBT0_TIMER    0x0008L
#define STKTPLG_TASK_EVENT_HBT1_TIMER    0x0010L
#define STKTPLG_TASK_EVENT_TCN           0x0020L


void STKCTRL_TASK_BD_ShowTranstionPerformace();
void STKCTRL_TASK_BD_ShowSlavePerformace();
void STKCTRL_TASK_BD_ShowMasterPerformace();
void STKCTRL_TASK_BD_ShowProvisionPerformace();
void STKCTRL_TASK_BD_ShowHotAddPerformace();
void STKCTRL_TASK_BD_ShowHotRemovePerformace();
void STKCTRL_TASK_BD_ClearPerformaceDB();
void STKCTRL_TASK_BD_SetPerformaceMaxTime(UI8_T type,UI16_T index,UI32_T time);


#endif /* STKCTRL_TASK_H */

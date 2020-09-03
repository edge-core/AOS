/* MODULE NAME:  dbsync_txt_init.h
* PURPOSE:
*   DBSYNC_TXT initiation and DBSYNC_TXT task creation
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2003-2-10       -- poli , created.
*
* Copyright(C)      Accton Corporation, 2002
*/

#ifndef DBSYNC_TXT_INIT_H

#define DBSYNC_TXT_INIT_H



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

/* FUNCTION NAME:  DBSYNC_TXT_INIT_Initiate_System_Resources
 * PURPOSE:
 *          This function initaites the system resources, such as queue, semaphore,
 *          and events used by this subsystem. All the call back functions shall be
 *          registered during subsystem initiation.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          1. This function must be invoked before any tasks in this subsystem can be created.
 *          2. This function must be invoked before any services in this subsystem can be executed.
 */
void DBSYNC_TXT_INIT_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DBSYNC_TXT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DBSYNC_TXT_INIT_Create_InterCSC_Relation(void);

/* FUNCTION NAME:  DBSYNC_TXT_INIT_Create_Tasks
 * PURPOSE:
 *          This function creates all the task of this subsystem.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          1. This function shall not be invoked before DBSYNC_TXT_INIT_Initiate_System_Resources() is performed.
 */
void DBSYNC_TXT_INIT_Create_Tasks(void);



/* FUNCTION NAME:  DBSYNC_TXT_INIT_EnterMasterMode
 * PURPOSE:
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the DBSYNC_TXT subsystem will enter the
 *          Master Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          1. If "System Boot Configruation File" does not exist, the system database and
 *             switch will be initiated to the factory default value.
 *          2. SSHD will handle network requests only when this subsystem
 *             is in the Master Operation mode
 */
void DBSYNC_TXT_INIT_EnterMasterMode(void);



/* FUNCTION NAME:  DBSYNC_TXT_INIT_EnterSlaveMode
 * PURPOSE:
 *          This function forces this subsystem enter the Slave Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          In Slave Operation mode, any network requests
 *          will be ignored.
 */
void DBSYNC_TXT_INIT_EnterSlaveMode(void);



/* FUNCTION NAME:  DBSYNC_TXT_INIT_EnterTransitionMode
 * PURPOSE:
 *          This function forces this subsystem enter the Teansition Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          In Transition Operation mode, any network requests
 *          will be ignored.
 */
void DBSYNC_TXT_INIT_EnterTransitionMode(void);



/* FUNCTION NAME : DBSYNC_TXT_INIT_SetTransitionMode
 * PURPOSE:
 *      This call will set sshd_mgr into transition mode to prevent
 *      calling request.
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
 *      None.
 */
void DBSYNC_TXT_INIT_SetTransitionMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DBSYNC_TXT_INIT_HandleHotInsertion
 * ------------------------------------------------------------------------
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
 * ------------------------------------------------------------------------
 */
void DBSYNC_TXT_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DBSYNC_TXT_INIT_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void DBSYNC_TXT_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);



#endif /* #ifndef DBSYNC_TXT_INIT_H */




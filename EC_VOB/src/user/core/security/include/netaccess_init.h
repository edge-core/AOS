/* Project Name: New Feature
 * File_Name : netaccess_init.h
 * Purpose     : NetAccess initiation and NetAccess task creation
 *
 * 2006/01/27    : Ricky Lin     Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (ACP_V3_Enhancement)
 */

#ifndef NETACCESS_INIT_H
#define NETACCESS_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_INIT_InitiateSystemResources
 *---------------------------------------------------------------------------
 * PURPOSE: This function initaites the system resources, such as queue, semaphore,
 *          and events used by this subsystem. All the call back functions shall be
 *          registered during subsystem initiation.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 1. This function must be invoked before any tasks in this subsystem can be created.
 *       2. This function must be invoked before any services in this subsystem can be executed.
 *---------------------------------------------------------------------------
 */
void NETACCESS_INIT_InitiateSystemResources(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_INIT_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTE:    None.
 *---------------------------------------------------------------------------
 */
void NETACCESS_INIT_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_INIT_CreateTask
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 1. This function shall not be invoked before
 *          NETACCESS_INIT_InitiateSystemResources() is performed
 *---------------------------------------------------------------------------
 */
void NETACCESS_INIT_CreateTasks(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE: This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the 1X subsystem will enter the
 *          Master Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 1. All the other subsystems must enter the master mode before this function
 *          can be invoked.
 *       2. If "System Boot Configruation File" does not exist, the system database and
 *          switch will be initiated to the factory default value.
 *-------------------------------------------------------------------------
 */
void NETACCESS_INIT_EnterMasterMode(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Transition Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any requests issued from applications will be ignored.
 *-------------------------------------------------------------------------
 */
void NETACCESS_INIT_EnterTransitionMode(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any requests issued from applications will be ignored.
 *-------------------------------------------------------------------------
 */
void NETACCESS_INIT_EnterSlaveMode(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This call will set dot1x_mgr into transition mode to prevent
 *      calling request.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE:   : None
 *-------------------------------------------------------------------------
 */
void NETACCESS_INIT_SetTransitionMode(void);

 /*-------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_INIT_HandleHotInsertion
 *-------------------------------------------------------------------------
 * Purpose: This function will initialize the port OM of the module ports
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
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 *-------------------------------------------------------------------------
 */
void NETACCESS_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

 /*-------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_INIT_HandleHotRemoval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 *-------------------------------------------------------------------------
 */
void NETACCESS_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
#endif /*NETACCESS_INIT_H*/



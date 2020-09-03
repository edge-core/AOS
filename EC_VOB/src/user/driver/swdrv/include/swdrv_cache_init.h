#ifndef SWDRV_CACHE_INIT_H
#define SWDRV_CACHE_INIT_H

#include "sys_type.h"

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_INIT_Initiate_System_Resources
 * -------------------------------------------------------------------------
 * FUNCTION: This function will init resource SWDRV_CACHE.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_CACHE_INIT_Initiate_System_Resources ();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_INIT_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_CACHE_INIT_Create_InterCSC_Relation(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_INIT_Create_Tasks
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create task in SWDRV_CACHE.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_CACHE_INIT_Create_Tasks();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_INIT_EnterMasterMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function make SWDRV_CACHE to enter master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_CACHE_INIT_EnterMasterMode();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_INIT_EnterSlaveMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function make SWDRV_CACHE to enter slave mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_CACHE_INIT_EnterSlaveMode();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_INIT_EnterTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function make SWDRV_CACHE to enter transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_CACHE_INIT_EnterTransitionMode();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_INIT_SetTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function set transitionm mode flag in SWDRV_CACHE.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRV_CACHE_INIT_SetTransitionMode();

#endif /* end of SWDRV_CACHE_INIT_H */

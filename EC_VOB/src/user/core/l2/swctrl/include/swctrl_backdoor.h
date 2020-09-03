/* Module Name: SWCTRL_BACKDOOR.H
 * Purpose: 

 * Notes: 
 *        ( Something must be known or noticed by developer     )
 * History:                                                               
 *       Date        Modifier        Reason
 *       2002/9/23   Charles Cheng   Create this file -- obsolete swctrl_cmn.h
 *
 * Copyright(C)      Accton Corporation, 1999, 2000   				
 */


#ifndef _SWCTRL_BACKDOOR_H_
#define _SWCTRL_BACKDOOR_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"

enum SWCTRL_BACKDOOR_DEBUG_FLAG_E
{
       SWCTRL_BACKDOOR_DEBUG_FLAG_NONE                       = BIT_0,
       SWCTRL_BACKDOOR_DEBUG_FLAG_CALLBACK_NOTIFY            = BIT_1,
       SWCTRL_BACKDOOR_DEBUG_FLAG_PORT_OPER_STATUS_CHANGED   = BIT_2,
       SWCTRL_BACKDOOR_DEBUG_FLAG_RXTCN                      = BIT_3,
       SWCTRL_BACKDOOR_DEBUG_FLAG_AMTR                       = BIT_4,
       SWCTRL_BACKDOOR_DEBUG_FLAG_ERRMSG                     = BIT_5,
       SWCTRL_BACKDOOR_DEBUG_FLAG_DBGMSG                     = BIT_6,
       SWCTRL_BACKDOOR_DEBUG_FLAG_MDIX                       = BIT_7,
       SWCTRL_BACKDOOR_DEBUG_FLAG_ABIL                       = BIT_8,
       SWCTRL_BACKDOOR_DEBUG_FLAG_SFP_DDM_TRAP               = BIT_9,

       SWCTRL_BACKDOOR_DEBUG_FLAG_ALL                        = BIT_31
};

enum SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_E
{
       SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_NONE                                 = 0,           
       SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_PACKET_COUNTER                   = 1,   
       SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_BROADCAST_EVENT                  = 2,   
       SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_BROADCAST_TRANSFER_STATE_MACHINE = 4,   
       SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_MULTICAST_EVENT                  = 8,   
       SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_MULTICAST_TRANSFER_STATE_MACHINE = 16,     
       SWCTRL_BACKDOOR_DEBUG_FLAG_ATC_ALL                        = 32
};

/* NAMING CONSTANT DECLARATIONS
 */

/* TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/****************************************************************************/
/* Switch Initialization													*/
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_Init
 * -------------------------------------------------------------------------
 * FUNCTION: This function allocates and initiates the system resource for
 *           Switch Control module
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Only called by swctrl_init.c
 * -------------------------------------------------------------------------*/
void SWCTRL_BACKDOOR_Init(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Only called by swctrl_init.c
 * -------------------------------------------------------------------------*/
void SWCTRL_BACKDOOR_Create_InterCSC_Relation(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_IsDebugFlagOn
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check the debug flag is on or not 
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Only called by swctrl.c
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_BACKDOOR_IsDebugFlagOn(UI32_T flag);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BACKDOOR_IsDebugFlagATCOn
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check the debug flag of ATC is on or not 
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Only called by swctrl.c
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_BACKDOOR_IsDebugFlagATCOn(UI32_T flag);

#endif /* _SWCTRL_BACKDOOR_H_ */

/*-------------------------------------------------------------------------
 * Module Name  :   vlan_backdoor.h
 *-------------------------------------------------------------------------
 * Purpose      :   This file is the header file of vlan_backdoor.c
 *-------------------------------------------------------------------------
 * Notes:
 * History:
 *    03/11/2001 -  Allen Cheng, created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-------------------------------------------------------------------------
 */

#ifndef _VLAN_BACKDOOR_H_
#define _VLAN_BACKDOOR_H_

#include "sys_type.h"

enum VLAN_DEBUG_FLAG_E
{
    VLAN_DEBUG_FLAG_NONE        = 0x00000000L,
    VLAN_DEBUG_FLAG_ERRMSG      = 0x00000001L,
    VLAN_DEBUG_FLAG_TRUNK       = 0x00000002L,
    VLAN_DEBUG_FLAG_VLAN        = 0x00000004L,
    VLAN_DEBUG_FLAG_NOTIFY      = 0x00000008L,
    VLAN_DEBUG_FLAG_DBGMSG4     = 0x00000010L,
    VLAN_DEBUG_FLAG_ALL         = 0xFFFFFFFFL
};


BOOL_T  VLAN_Debug(UI32_T flag);
void    VLAN_SetDebugFlag(UI32_T flag);
void    VLAN_GetDebugFlag(UI32_T *flag);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - VLAN_BACKDOOR_Init
 * ------------------------------------------------------------------------
 * FUNCTION : This function ititiates the backdoor function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void VLAN_BACKDOOR_Init(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - VLAN_BACKDOOR_Create_InterCSC_Relation
 * ------------------------------------------------------------------------
 * FUNCTION : This function initializes all function pointer registration operations.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void VLAN_BACKDOOR_Create_InterCSC_Relation(void);

/* ------------------------------------------------------------------------
 * ROUTINE NAME - VLAN_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the main routine of the backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void VLAN_BACKDOOR_Main(void);


#endif /* _VLAN_BACKDOOR_H_ */

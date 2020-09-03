/* MODULE NAME: netaccess_backdoor.h
 * PURPOSE:
 *  Implement netaccess backdoor
 *
 * NOTES:
 *
 * History:
 *    2007/11/27 : Squid Ro      Create this file
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef NETACCESS_BACKDOOR_H
#define NETACCESS_BACKDOOR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_Backdoor_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : security backdoor callback function
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void NETACCESS_Backdoor_CallBack(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_Backdoor_Register_SubsysBackdoorFunc
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
void NETACCESS_Backdoor_Register_SubsysBackdoorFunc(void);

BOOL_T NETACCESS_BACKDOOR_Register(const char *show_name, UI32_T *reg_no_p);
BOOL_T NETACCESS_BACKDOOR_IsOn(UI32_T reg_no);

#endif /* NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE */

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
#endif /* End of NETACCESS_BACKDOOR_H */

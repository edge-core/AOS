/*-----------------------------------------------------------------------------
 * Module Name: portauthsrvc_mgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE: A header file for the definition of Data Structure and export functions of portauthsrv_mgr.c
 *-----------------------------------------------------------------------------
 * NOTES:
 * 1. This header file is created for PortAuthSrv.c to do Auto VLAN and QoS Assignment.
 * 2. All the funcion prototypes are implements in portauthsrv_mgr.c.
 * 3. All those files which want to implement Auto VLAN and QoS Assignment should include this hearder file.
 * 3. portauthsrv_mgr.c will not call any upper layer API.
 * 4. portauthsrv_mgr.c will call API of VLAN to set auto vlans to physical ports.
 * 5. portauthsrv_mgr.c will call API of Differv to set auto QoS to physical ports.
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    11/08/2004 - Rene Wang, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2004
 *-----------------------------------------------------------------------------
 */

#ifndef PORTAUTHSRVC_MGR_H
#define PORTAUTHSRVC_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include "sys_adpt.h"
#include "security_backdoor.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define PORTAUTHSRVC_SUPPORT_ACCTON_BACKDOOR      (FALSE && SECURITY_SUPPORT_ACCTON_BACKDOOR) /* support backdoor functions */

#define PORTAUTHSRVC_DEFAULT_VLAN_ASSIGNMENT       "-0Default0-" /* implies no returned vlan attribute so use default setting */


/* DATA TYPE DECLARATIONS
 */
typedef struct PortAuthSrvAssign_S
{
    UI32_T  unit;
    UI32_T  port;
    UI8_T   vlan_assignment[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
    UI8_T   qos_assignment[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
    struct PortAuthSrvAssign_S    *next;

} PortAuthSrvAssign_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
void PORTAUTHSRVC_MGR_InitiateSystemResource();

/* Dynamic VLAN Assignment Functions
 */
BOOL_T PORTAUTHSRVC_MGR_Vlan_IsValidVlanListString(UI32_T ifindex, const char *str);
BOOL_T PORTAUTHSRVC_MGR_Vlan_StringToMd5(const char *str, UI8_T digit[16]);
BOOL_T PORTAUTHSRVC_MGR_Vlan_SetToOper(UI32_T ifindex, const char *str);
BOOL_T PORTAUTHSRVC_MGR_Vlan_SetToAdmin(UI32_T ifindex);

/* Dynamic Qos Assignment Functions
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Qos_IsValidProfileString
 * ---------------------------------------------------------------------
 * PURPOSE: Validates the QoS profile string.
 * INPUT  : ifindex -- lport number
 *          str     -- QoS profile string
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Qos_IsValidProfileString(UI32_T ifindex, const char *str);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Qos_StringToMd5
 * ---------------------------------------------------------------------
 * PURPOSE: Get a MD5 digit from a QoS profile string.
 * INPUT  : str     -- QoS profile string
 * OUTPUT : digit   -- MD5 digit
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Qos_StringToMd5(const char *str, unsigned char digit[16]);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Qos_SetToOper
 * ---------------------------------------------------------------------
 * PURPOSE: Applies QoS profile.
 * INPUT  : ifindex -- lport number
 *          str     -- QoS profile string
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  :
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Qos_SetToOper(UI32_T ifindex, const char *str);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PORTAUTHSRVC_MGR_Qos_SetToAdmin
 * ---------------------------------------------------------------------
 * PURPOSE: Restores QoS profile.
 * INPUT  : ifindex -- lport number
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  :
 * ---------------------------------------------------------------------
 */
BOOL_T PORTAUTHSRVC_MGR_Qos_SetToAdmin(UI32_T ifindex);

void PORTAUTHSRVC_MGR_Unit_Test_Main();

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
#endif


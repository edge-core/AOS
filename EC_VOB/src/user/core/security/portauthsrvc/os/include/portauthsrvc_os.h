#ifndef __PORTAUTHSRVC_OS_H__
#define __PORTAUTHSRVC_OS_H__

#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include "sys_dflt.h"
#include "sys_adpt.h"

#include "vlan_om.h"

typedef BOOL_T (*PORTAUTHSRVC_OS_Vlan_Exec_T)(UI32_T ifindex, UI32_T pvid, VLAN_OM_VLIST_T *tag_lst, VLAN_OM_VLIST_T *untag_lst);
typedef BOOL_T (*PORTAUTHSRVC_OS_Qos_Exec_T)(UI32_T ifindex, const char *key, const char *val);

typedef struct
{
    PORTAUTHSRVC_OS_Vlan_Exec_T check;
    PORTAUTHSRVC_OS_Vlan_Exec_T commit;
    PORTAUTHSRVC_OS_Vlan_Exec_T restore;
}PORTAUTHSRVC_OS_Vlan_Command_T;

typedef struct
{
    char *cmd_str;
    char *val_str;
    PORTAUTHSRVC_OS_Qos_Exec_T check;
    PORTAUTHSRVC_OS_Qos_Exec_T commit;
    PORTAUTHSRVC_OS_Qos_Exec_T restore;
}PORTAUTHSRVC_OS_Qos_Command_T;

void PORTAUTHSRVC_OS_Init();

PORTAUTHSRVC_OS_Vlan_Command_T* PORTAUTHSRVC_OS_Vlan_Command();

PORTAUTHSRVC_OS_Qos_Command_T* PORTAUTHSRVC_OS_Qos_Command(
    int idx
    );

UI32_T PORTAUTHSRVC_OS_Qos_CommandNumber();

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
#endif

#ifndef _PSEC_OM_H_
#define _PSEC_OM_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "leaf_es3626a.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */
#define PSEC_OM_GET_MSGBUFSIZE(type_name) \
    ((uintptr_t)&((PSEC_OM_IPCMsg_T *)0)->data + sizeof(type_name))

#define PSEC_OM_MSG_CMD(msg_p)    (((PSEC_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define PSEC_OM_MSG_RETVAL(msg_p) (((PSEC_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.result)
#define PSEC_OM_MSG_DATA(msg_p)   ((void *)&((PSEC_OM_IPCMsg_T *)(msg_p)->msg_buf)->data)

/* DATA TYPE DECLARATIONS
 */
typedef union
{
    UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T result; /* for response */
} PSEC_OM_IPCMsg_Type_T;

typedef union
{
    UI32_T ifindex;
    UI32_T mac_count;
} PSEC_OM_IPCMsg_GetPortSecurityMacCount_T;

typedef union
{
    PSEC_OM_IPCMsg_GetPortSecurityMacCount_T get_port_security_mac_count_data;
} PSEC_OM_IPCMsg_Data_T;

typedef struct
{
    PSEC_OM_IPCMsg_Type_T type;
    PSEC_OM_IPCMsg_Data_T data;
} PSEC_OM_IPCMsg_T;

enum
{
    PSEC_OM_IPC_GET_MAX_MAC_COUNT
};

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
void PSEC_OM_InitiateSystemResources();

BOOL_T PSEC_OM_GetPortSecurityStatus(UI32_T lport, UI32_T *status_p);

BOOL_T PSEC_OM_SetPortSecurityStatus(UI32_T lport, UI32_T status);

BOOL_T PSEC_OM_GetPortSecurityActionActive(UI32_T lport, UI32_T *state_p);

BOOL_T PSEC_OM_SetPortSecurityActionActive(UI32_T lport, UI32_T state);

BOOL_T PSEC_OM_GetIntrusionAction(UI32_T lport, UI32_T *action_p);

BOOL_T PSEC_OM_SetIntrusionAction(UI32_T lport, UI32_T action);

BOOL_T PSEC_OM_GetMaxMacCount(UI32_T lport, UI32_T *mac_count_p);

BOOL_T PSEC_OM_SetMaxMacCount(UI32_T lport, UI32_T mac_count);

BOOL_T PSEC_OM_GetLastIntrusionTime(UI32_T lport, UI32_T *seconds);

BOOL_T PSEC_OM_SetLastIntrusionTime(UI32_T lport, UI32_T seconds);

BOOL_T PSEC_OM_GetLastIntrusionMac(UI32_T lport, UI8_T *mac);

BOOL_T PSEC_OM_SetLastIntrusionMac(UI32_T lport, UI8_T *mac);

const char *PSEC_OM_StrPortSecurityStatus(UI32_T psec_status);

BOOL_T PSEC_OM_GetTrapTime(UI32_T lport, UI32_T *seconds);

BOOL_T PSEC_OM_SetTrapTime(UI32_T lport, UI32_T seconds);

const char *PSEC_OM_StrIntrusionAction(UI32_T action);

BOOL_T PSEC_OM_IsEnabled(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_OM_HandleIPCReqMsg
 * ------------------------------------------------------------------------
 * PURPOSE  :   Handle the ipc request message for PSEC OM.
 * INPUT    :   msgbuf_p     -- input request ipc message buffer
 * OUTPUT   :   msgbuf_p     -- output response ipc message buffer
 * RETURN   :   TRUE, success; else return FALSE.
 * NOTE     :   none
 *-------------------------------------------------------------------------
 */
BOOL_T PSEC_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

#endif

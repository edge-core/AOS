/* MODULE NAME:  telnet_om.h
 * PURPOSE:
 * OM for telnet.
 *
 * NOTES:
 *    Only the CSC that owns the OM can do write operation through OM.
 *    Other CSC can ONLY DO READ OPERATION through OM.
 *
 * REASON:
 * Description:
 * HISTORY
 *    06/03/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef TELNET_OM_H
#define TELNET_OM_H

/* INCLUDE FILE DECLARATIONS
 */
//rich #include "sys_types.h"
#include "telnet_mgr.h"
#include "sysfun.h"
/* NAMING CONSTANT DECLARATIONS
 */

#define TELNET_OM_MSGBUF_TYPE_SIZE sizeof(union TELNET_OM_IPCMSG_Type_U)

/* definitions of command in TELNET which will be used in POM operation
 */
enum
{
    TELNET_OM_IPC_GET_TNPD_PORT=1,
    TELNET_OM_IPC_GET_TNPD_STATUS=2,
};

/* MACRO FUNCTION DECLARATIONS
 */
#define TELNET_OM_EnterCriticalSection(sem_id)    SYSFUN_OM_ENTER_CRITICAL_SECTION(sem_id)
#define TELNET_OM_LeaveCriticalSection(sem_id, orig_priority)    SYSFUN_OM_LEAVE_CRITICAL_SECTION(sem_id, orig_priority)

/* Macro function for calculation of ipc msg_buf size based on structure name
 * used in TELNET_OM_IPCMsg_T.data
 */
#define TELNET_OM_GET_MSGBUFSIZE(field_name)                       \
            (TELNET_OM_MSGBUF_TYPE_SIZE +                        \
            sizeof(((TELNET_OM_IPCMsg_T*)0)->data.field_name))



/* DATA TYPE DECLARATIONS
 */
/* structure for the request/response ipc message in TELNET pom and om
 */
typedef struct
{
    union TELNET_OM_IPCMSG_Type_U
    {
        UI32_T cmd;    /* for sending IPC request. */
        BOOL_T result; /* for response */
    } type;

    union
    {
        /* the structure which is used when requesting cmd==TELNET_OM_IPC_GetTnpdPort
         */
        struct TELNET_OM_IPCMSG_PORT_DATA_S
        {
            UI32_T port;
        } port_data;

        /* the structure which is used when responding cmd:TELNET_OM_IPC_GetTnpdStatus
         */
        struct TELNET_OM_IPCMSG_STATUS_DATA_S
        {
            TELNET_State_T state;
        } status_data;

    } data; /* contains the supplemntal data for the corresponding cmd */
} TELNET_OM_IPCMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_InitateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in this process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE  -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_InitateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_GetTnpdPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will get telnet port from OM.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    *port_p -- tcp port for telnet.
 *
 * RETURN:
 *    TRUE/FALSE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_GetTnpdPort(UI32_T *port_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_SetTnpdPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will set telnet port to OM.
 * INPUT:
 *    port -- telnet port
 *
 * OUTPUT:
 *    None
 *
 * RETURN:
 *    TRUE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_SetTnpdPort(UI32_T port);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_GetTnpdStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will get telnet status from OM.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    *state_p -- TELNET_STATE_ENABLED / TELNET_STATE_DISABLED
 *
 * RETURN:
 *    TRUE/FALSE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_GetTnpdStatus(TELNET_State_T *state_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_SetTnpdMaxSession
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will set telnet max session to OM.
 *
 * INPUT: 
 *    maxSession -- max session number
 *
 * OUTPUT:
 *    None
 *
 * RETURN:
 *    TRUE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_SetTnpdMaxSession(UI32_T maxSession);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_GetTnpdMaxSession
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will get telnet max session from OM.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    max session number
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T TELNET_OM_GetTnpdMaxSession();

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_SetTnpdStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will set telnet status to OM.
 * INPUT:
 *    state.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_SetTnpdStatus(TELNET_State_T state);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for TELNET om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  --  There is a response need to send.
 *    FALSE --  No response need to send.
 *
 * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif    /* End of TELNET_OM_H */


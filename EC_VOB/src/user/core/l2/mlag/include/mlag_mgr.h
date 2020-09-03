/* =============================================================================
 * MODULE NAME : MLAG_MGR.H
 * PURPOSE     : Provide declarations for MLAG operational functions.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

#ifndef MLAG_MGR_H
#define MLAG_MGR_H

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include "sys_type.h"
#include "sys_adpt.h"
#include "l_mm_type.h"
#include "sysfun.h"
#include "swctrl.h"
#include "mlag_type.h"

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#define MLAG_MGR_IPCMSG_TYPE_SIZE sizeof(union MLAG_MGR_IpcMsg_Type_U)

/* command used in IPC message */
enum
{
    MLAG_MGR_IPC_SETGLOBALSTATUS,
    MLAG_MGR_IPC_SETDOMAIN,
    MLAG_MGR_IPC_REMOVEDOMAIN,
    MLAG_MGR_IPC_SETMLAG,
    MLAG_MGR_IPC_REMOVEMLAG,
};

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in MLAG_MGR_IpcMsg_T.data
 */
#define MLAG_MGR_GET_MSG_SIZE(field_name)               \
    (MLAG_MGR_IPCMSG_TYPE_SIZE +                        \
    sizeof(((MLAG_MGR_IpcMsg_T*)0)->data.field_name))

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* IPC message structure
 */
typedef struct
{
    union MLAG_MGR_IpcMsg_Type_U
    {
        UI32_T  cmd;
        BOOL_T  ret_bool;
        UI32_T  ret_ui32;
    } type; /* the intended action or return value */

    union
    {
        UI32_T  arg_ui32;
        char    arg_str[MLAG_TYPE_MAX_DOMAIN_ID_LEN+1];

        struct
        {
            char    arg_str[MLAG_TYPE_MAX_DOMAIN_ID_LEN+1];
            UI32_T  arg_ui32;
        } arg_str_ui32;
        struct
        {
            UI32_T  arg_ui32_1;
            UI32_T  arg_ui32_2;
            char    arg_str[MLAG_TYPE_MAX_DOMAIN_ID_LEN+1];
        } arg_ui32_ui32_str;
    } data; /* the argument(s) for the function corresponding to cmd */
} MLAG_MGR_IpcMsg_T;

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM SPECIFICATIONS
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_MGR_InitiateProcessResources
 * PURPOSE : Initiate process resources for the CSC.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_InitiateProcessResources();

/* FUNCTION NAME - MLAG_MGR_CreateInterCscRelation
 * PURPOSE : Create inter-CSC relations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_CreateInterCscRelation();

/* FUNCTION NAME - MLAG_MGR_SetTransitionMode
 * PURPOSE : Process when system is set to be transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_SetTransitionMode();

/* FUNCTION NAME - MLAG_MGR_EnterTransitionMode
 * PURPOSE : Process when system enters transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_EnterTransitionMode();

/* FUNCTION NAME - MLAG_MGR_EnterMasterMode
 * PURPOSE : Process when system enters master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_EnterMasterMode();

/* FUNCTION NAME - MLAG_MGR_EnterSlaveMode
 * PURPOSE : Process when system enters slave mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_EnterSlaveMode();

/* FUNCTION NAME - MLAG_MGR_ProvisionComplete
 * PURPOSE : Process when the CSC is informed of provision complete.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_ProvisionComplete();

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME - MLAG_MGR_HandleHotInsertion
 * PURPOSE : Process when optional module is inserted.
 * INPUT   : starting_port_ifindex -- the first port on the module
 *           number_of_port        -- the number of ports on the module
 *           use_default           -- whether to use default configuration
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_HandleHotInsertion(UI32_T starting_port_ifindex,
    UI32_T number_of_port, BOOL_T use_default);

/* FUNCTION NAME - MLAG_MGR_HandleHotRemoval
 * PURPOSE : Process when optional module is removed.
 * INPUT   : starting_port_ifindex -- the first port on the module
 *           number_of_port        -- the number of ports on the module
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_HandleHotRemoval(UI32_T starting_port_ifindex,
    UI32_T number_of_port);
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

/* FUNCTION NAME - MLAG_MGR_PortOperUp_CallBack
 * PURPOSE : Process callback when port oper status is up.
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_PortOperUp_CallBack(UI32_T ifindex);

/* FUNCTION NAME - MLAG_MGR_PortNotOperUp_CallBack
 * PURPOSE : Process callback when port oper status is not up.
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_PortNotOperUp_CallBack(UI32_T ifindex);

/* FUNCTION NAME - MLAG_MGR_PortEffectiveOperStatusChanged_CallBack
 * PURPOSE : Process callback when port effective oper status is changed.
 * INPUT   : ifindex        -- which logical port
 *           pre_status     -- status before change
 *           current_status -- status after change
 *           level          -- SWCTRL_OperDormantLevel_T
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :  Handle MLAG dormant only
 */
void MLAG_MGR_PortEffectiveOperStatusChanged_CallBack(UI32_T ifindex,
    UI32_T pre_status, UI32_T current_status, UI32_T level);

/* FUNCTION NAME - MLAG_MGR_MacUpdate_CallBack
 * PURPOSE : Process callback when a MAC update happens.
 * INPUT   : ifindex -- port on whcih MAC address is updated
 *           vid     -- VLAN ID
 *           mac_p   -- MAC address
 *           added   -- MAC address is added or removed
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_MacUpdate_CallBack(UI32_T ifindex, UI32_T vid, UI8_T *mac_p,
    BOOL_T added);

/* FUNCTION NAME - MLAG_MGR_ProcessReceivedPacket
 * PURPOSE : Process when packet is received.
 * INPUT   : mref_handle_p -- memory reference of received packet
 *           src_mac_p     -- source MAC address
 *           tag_info      -- raw tagged info of the packet
 *           src_unit      -- source unit
 *           src_port      -- source port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_ProcessReceivedPacket(L_MM_Mref_Handle_T *mref_handle_p,
                                    UI8_T              *src_mac_p,
                                    UI16_T             tag_info,
                                    UI32_T             src_unit,
                                    UI32_T             src_port);

/* FUNCTION NAME - MLAG_MGR_ProcessTimerEvent
 * PURPOSE : Process when timer event is received.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_ProcessTimerEvent();

/* FUNCTION NAME - MLAG_MGR_SetGlobalStatus
 * PURPOSE : Set global status of the feature.
 * INPUT   : status -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                     MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_MGR_SetGlobalStatus(UI32_T status);

/* FUNCTION NAME - MLAG_MGR_SetDomain
 * PURPOSE : Set a MLAG domain.
 * INPUT   : domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 *           lport       -- peer link
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Replace if the domain ID has existed
 */
UI32_T MLAG_MGR_SetDomain(char *domain_id_p, UI32_T lport);

/* FUNCTION NAME - MLAG_MGR_RemoveDomain
 * PURPOSE : Remove a MLAG domain.
 * INPUT   : domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Success if the domain ID does not exist
 */
UI32_T MLAG_MGR_RemoveDomain(char *domain_id_p);

/* FUNCTION NAME - MLAG_MGR_SetMlag
 * PURPOSE : Set a MLAG.
 * INPUT   : mlag_id     -- MLAG ID
 *           lport       -- MLAG member
 *           domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Replace if the MLAG ID has existed
 */
UI32_T MLAG_MGR_SetMlag(UI32_T mlag_id, UI32_T lport, char *domain_id_p);

/* FUNCTION NAME - MLAG_MGR_RemoveMlag
 * PURPOSE : Remove a MLAG.
 * INPUT   : mlag_id -- MLAG ID
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_MGR_RemoveMlag(UI32_T mlag_id);

/* FUNCTION NAME - MLAG_MGR_HandleIPCReqMsg
 * PURPOSE : Handle the IPC request message.
 * INPUT   : msgbuf_p -- IPC request message buffer
 * OUTPUT  : msgbuf_p -- IPC response message buffer
 * RETURN  : TRUE  -- there is a response required to be sent
 *           FALSE -- there is no response required to be sent
 * NOTE    : None
 */
BOOL_T MLAG_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

#endif /* End of MLAG_MGR_H */

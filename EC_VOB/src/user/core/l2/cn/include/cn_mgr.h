/* MODULE NAME - CN_MGR.H
 * PURPOSE : Provides the declarations for CN functional management.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef CN_MGR_H
#define CN_MGR_H

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define CN_MGR_IPCMSG_TYPE_SIZE sizeof(union CN_MGR_IpcMsg_Type_U)

/* command used in IPC message */
enum
{
    CN_MGR_IPC_SETGLOBALADMINSTATUS,
    CN_MGR_IPC_SETCNMTXPRIORITY,
    CN_MGR_IPC_SETCNPV,
    CN_MGR_IPC_SETCNPVDEFENSEMODE,
    CN_MGR_IPC_SETCNPVALTERNATEPRIORITY,
    CN_MGR_IPC_SETPORTCNPVDEFENSEMODE,
    CN_MGR_IPC_SETPORTCNPVALTERNATEPRIORITY,
};

/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in VLAN_MGR_IpcMsg_T.data
 */
#define CN_MGR_GET_MSG_SIZE(field_name)    \
    (CN_MGR_IPCMSG_TYPE_SIZE + sizeof(((CN_MGR_IpcMsg_T*)0)->data.field_name))

/* DATA TYPE DECLARATIONS
 */

/* IPC message structure */
typedef struct
{
	union CN_MGR_IpcMsg_Type_U
	{
		UI32_T cmd;
        UI32_T ret_ui32;
	} type; /* the intended action or return value */

	union
	{
	    UI32_T arg_ui32;

        struct
        {
            UI32_T arg_ui32;
            BOOL_T arg_bool;
        } arg_grp_ui32_bool;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
        } arg_grp_ui32_ui32;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI32_T arg_ui32_3;
        } arg_grp_ui32_ui32_ui32;
	} data; /* the argument(s) for the function corresponding to cmd */
} CN_MGR_IpcMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME - CN_MGR_InitiateProcessResources
 * PURPOSE : Initiate process resources.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_InitiateProcessResources(void);

/* FUNCTION NAME - CN_MGR_CreateInterCscRelation
 * PURPOSE : Create inter-CSC relations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_CreateInterCscRelation(void);

/* FUNCTION NAME - CN_MGR_SetTransitionMode
 * PURPOSE : Process when setting CN to transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_SetTransitionMode(void);

/* FUNCTION NAME - CN_MGR_EnterTransitionMode
 * PURPOSE : Process when CN enters transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_EnterTransitionMode(void);

/* FUNCTION NAME - CN_MGR_EnterMasterMode
 * PURPOSE : Process when CN enters master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_EnterMasterMode(void);

/* FUNCTION NAME - CN_MGR_EnterSlaveMode
 * PURPOSE : Process EnterSlaveMode for LBD MGR.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_EnterSlaveMode(void);

/* FUNCTION NAME - CN_MGR_ProvisionComplete
 * PURPOSE : Process when CN is informed of provision complete.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_ProvisionComplete(void);

/* FUNCTION NAME - CN_MGR_TrunkMemberAdd1st_CallBack
 * PURPOSE : Process callback when the first member is added to a trunk.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_TrunkMemberAdd1st_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/* FUNCTION NAME - CN_MGR_TrunkMemberAdd_CallBack
 * PURPOSE : Process callback when a member except the first is added to a
 *           trunk.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_TrunkMemberAdd_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/* FUNCTION NAME - CN_MGR_TrunkMemberDelete_CallBack
 * PURPOSE : Process callback when a member except the last is deleted from a
 *           trunk.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_TrunkMemberDelete_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/* FUNCTION NAME - CN_MGR_TrunkMemberDeleteLst_CallBack
 * PURPOSE : Process callback when the last member is deleted from a trunk.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_TrunkMemberDeleteLst_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/* FUNCTION NAME - CN_MGR_RemoteChange_CallBack
 * PURPOSE : Process callback when the remote CN data is changed.
 * INPUT   : lport            - the logical port which receives the CN TLV
 *           neighbor_num     - the number of neighbors
 *           cnpv_indicators  - the CNPV indicators in the received CN TLV
 *           ready_indicators - the Ready indicators in the received CN TLV
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_RemoteChange_CallBack(UI32_T lport, UI32_T neighbor_num,
    UI32_T cnpv_indicators, UI32_T ready_indicators);

/* FUNCTION NAME - CN_MGR_SetGlobalAdminStatus
 * PURPOSE : Set CN global admin status.
 * INPUT   : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetGlobalAdminStatus(UI32_T status);

/* FUNCTION NAME - CN_MGR_SetGlobalOperStatus
 * PURPOSE : Set CN global oper status.
 * INPUT   : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetGlobalOperStatus(UI32_T status);

/* FUNCTION NAME - CN_MGR_SetCnmTxPriority
 * PURPOSE : Set the priority used for transmitting CNMs.
 * INPUT   : priority - the specified priority
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetCnmTxPriority(UI32_T priority);

/* FUNCTION NAME - CN_MGR_SetCnpv
 * PURPOSE : Set a priority to be CNPV or not.
 * INPUT   : priority - the specified priority
 *           active   - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetCnpv(UI32_T priority, BOOL_T active);

/* FUNCTION NAME - CN_MGR_SetCnpvDefenseMode
 * PURPOSE : Set the defense mode for a CNPV.
 * INPUT   : priority - the specified CNPV
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetCnpvDefenseMode(UI32_T priority, UI32_T mode);

/* FUNCTION NAME - CN_MGR_SetCnpvAlternatePriority
 * PURPOSE : Set the alternate priority used for a CNPV.
 * INPUT   : priority     - the specified CNPV
 *           alt_priority - the specified alternate priority
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetCnpvAlternatePriority(UI32_T priority, UI32_T alt_priority);

/* FUNCTION NAME - CN_MGR_SetPortCnpvDefenseMode
 * PURPOSE : Set the defense mode for a CNPV on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetPortCnpvDefenseMode(UI32_T priority, UI32_T lport, UI32_T mode);

/* FUNCTION NAME - CN_MGR_SetPortCnpvAlternatePriority
 * PURPOSE : Set the alternate priority used for a CNPV on a port.
 * INPUT   : priority     - the specified CNPV
 *           lport        - the specified logical port
 *           alt_priority - the specified alternate priority
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetPortCnpvAlternatePriority(UI32_T priority, UI32_T lport, UI32_T alt_priority);

/* FUNCTION NAME - CN_MGR_HandleIPCReqMsg
 * PURPOSE : Handle the IPC request message for CN MGR.
 * INPUT   : msgbuf_p - input request ipc message buffer
 * OUTPUT  : msgbuf_p - output response ipc message buffer
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 * NOTES   : None
 */
BOOL_T CN_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

#endif /* End of CN_MGR_H */

 /* MODULE NAME: cli_api_vxlan.h
 * PURPOSE:
 *   Provides the definitions for VXLAN APIs.
 * NOTES:
 *   None
 *
 * HISTORY:
 *   04/21/2015 -- Kelly Chen, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#ifndef CLI_API_VXLAN_H
#define CLI_API_VXLAN_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME - CLI_API_VXLAN_UdpDstPort
 * PURPOSE : Set UDP port number.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_UdpDstPort(UI16_T cmd_idx, char *arg[],
                                CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_VXLAN_Vni
 * PURPOSE : Configure VNI
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_Vni(UI16_T cmd_idx, char *arg[],
                                CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_VXLAN_FloodRVTEP
 * PURPOSE : looding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_FloodRVTEP(UI16_T cmd_idx, char *arg[],
                                CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_VXLAN_FloodMulticast
 * PURPOSE : looding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_FloodMulticast(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_VXLAN_VlanVniMap
 * PURPOSE : Configure VLAN and VNI mapping relationship.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_VlanVniMap(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P);


/* FUNCTION NAME - CLI_API_VXLAN_SrcInterface
 * PURPOSE : Specify source interface of local VTEP.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_SrcInterface(UI16_T cmd_idx, char *arg[],
                           CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_VXLAN_ShowUdpDstPort
 * PURPOSE : Show VxLAN UDP port number.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowUdpDstPort(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_VXLAN_ShowVtep
 * PURPOSE : Show VxLAN tunnel created situation.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowVtep(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_VXLAN_ShowFlood
 * PURPOSE : Show VxLAN flooding configuration.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowFlood(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_VXLAN_ShowAccessPort
 * PURPOSE : Show VXLAN access port configuration.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowAccessPort(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P);


/* FUNCTION NAME - CLI_API_VXLAN_ShowVlanVniMap
 * PURPOSE : Show VLAN and VNI mapping relationship.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowVlanVniMap(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_VXLAN_ShowSrcIf
 * PURPOSE : Show source interface of local VTEP.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_VXLAN_ShowSrcIf(UI16_T cmd_idx, char *arg[],
                                    CLI_TASK_WorkingArea_T *ctrl_P);
#endif /* End of CLI_API_VXLAN_H */

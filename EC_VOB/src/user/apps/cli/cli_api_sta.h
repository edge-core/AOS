/* -------------------------------------------------------------------------------------
 * FILE NAME:  CLI_API_STA.H
 * -------------------------------------------------------------------------------------
 * PURPOSE:This file is the action function of 802.1w command
 * NOTE:
 *
 *
 *
 * HISTORY:
 * Modifier         Date                Description
 * -------------------------------------------------------------------------------------
 * pttch           6-21-2002             First Created
 *
 * -------------------------------------------------------------------------------------
 * Copyright(C)                 Accton Technology Corp. 2002
 * -------------------------------------------------------------------------------------*/



/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_MaxHops
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "max-hops" in MSTP configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_MaxHops(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Mst
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "mst" in MSTP configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Mst(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Name
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "name" in MSTP configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Name(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Revision
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "revision" in MSTP configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Revision(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_ForwartTime
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree forward-time"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_ForwartTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_HelloTime
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree hello-time" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_HelloTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_MaxAge
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree max-age" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_MaxAge(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Mode
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree mode" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Mode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_MstConfiguration
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree mst-configuration"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_MstConfiguration(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Pathcost_Method
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree pathcost method" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Pathcost_Method(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Priority
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree priority" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_TransmissionLimit
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree transmission-limit"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_TransmissionLimit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Cost_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree cost"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Cost_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_EdgePort_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree edgeport"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_EdgePort_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_LinkType_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree link-type"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_LinkType_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Mst_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree mst"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Mst_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_PortPriority_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree port-priority"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_PortPriority_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Portfast_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree portfast"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Portfast_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_ProtocolMigration
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree prototol migration"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_ProtocolMigration(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Cost_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree cost"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Cost_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_EdgePort_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree edgeport"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_EdgePort_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_LinkType_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree link-type"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_LinkType_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Mst_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree mst"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Mst_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_PortPriority_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree port-priority"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_PortPriority_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Portfast_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree portfast"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Portfast_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_ProtocolMigration_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree prototol migration"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_ProtocolMigration_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Spanningtree
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show spanningtree"
 *            in priviledge configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Spanningtree(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SPANNINGTREE_PER_VLAN
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree"
 *            in vlan database mode just noly for PVST
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_SPANNINGTREE_PER_VLAN(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Port_MacAddr_Learning_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "mac-learning"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Port_MacAddr_Learning_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Port_MacAddr_Learning_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "mac-learning"
 *            in interface portchannel configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Port_MacAddr_Learning_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_MacAddressTable_AgingTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
UI32_T CLI_API_MacAddressTable_HashLookupDepth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif
UI32_T CLI_API_MacAddressTable_MacLearning(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_MacAddressTable_Secure(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_MacAddressTable_Static(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Clear_MacAddressTable_Dynamic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_MacAddressTable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_CollisionMacAddressTable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Clear_CollisionMacAddressTable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_MacAddressTable_AgingTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
UI32_T CLI_API_Show_MacAddressTable_HashLookupDepth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif
UI32_T CLI_API_Show_MacAddressTable_Secure(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_RootGuard_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree root-guard"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_RootGuard_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_RootGuard_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree root-guard"
 *            in interface portchannel configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_RootGuard_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_BpduGuard_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdu-guard"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_BpduGuard_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_BpduGuard_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdu-guard"
 *            in interface portchannel configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_BpduGuard_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SpanningTree_BpduGuard_AutoRecovery_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdu-guard auto-recovery"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_SpanningTree_BpduGuard_AutoRecovery_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SpanningTree_BpduGuard_AutoRecovery_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdu-guard auto-recovery"
 *            in interface portchannel configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_SpanningTree_BpduGuard_AutoRecovery_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_BpduFilter_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdug-filtering"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_BpduFilter_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_BpduFilter_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdug-filtering"
 *            in interface portchannel configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_BpduFilter_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_CiscoPrestandard
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanning-tree cisco-prestandard-compatibility"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_CiscoPrestandard(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_tc_prop_stop_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "Spanning-tree tc-prop-stop"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_tc_prop_stop_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_tc_prop_stop_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "Spanning-tree tc-prop-stop"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_tc_prop_stop_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_SetTcPropGroup
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "Spanning-tree tc-prop group"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_SetTcPropGroup(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Spanningtree_TcProp
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "Spanning-tree tc-prop"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Spanningtree_TcProp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

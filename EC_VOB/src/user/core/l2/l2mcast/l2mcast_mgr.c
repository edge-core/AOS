/*-----------------------------------------------------------------------------
 * Module   : l2mcast_mgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Process VLAN/SWCTRL callback message.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 12/03/2001 - Lyn Yeh, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sysfun.h"
#include "syslog_pmgr.h"
#include "vlan_lib.h"
#include "igv3snp_defines.h"
#include "igv3snp_mgr.h"
#include "l2mcast_mgr.h"
#if(SYS_CPNT_MLDSNP == TRUE)
#include "mldsnp_mgr.h"
#endif
/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
SYSFUN_DECLARE_CSC

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : This function initiates the IGMP snooping module.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_InitiateProcessResources(void)
{
     return;

}   /* End of L2MCAST_MGR_InitiateProcessResources() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_Create_InterCSC_Relation(void)
{
    return;
} /* End of L2MCAST_MGR_Create_InterCSC_Relation() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function sets the transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();

}   /* End of L2MCAST_MGR_SetTransitionMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function enters master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_EnterMasterMode(void)
{
    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();

}   /* End of L2MCAST_MGR_EnterMasterMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function enters the transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_EnterTransitionMode(void)
{
    /* set mgr in transition mode */
    SYSFUN_ENTER_TRANSITION_MODE();

}   /* End of L2MCAST_MGR_EnterTransitionMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function sets the transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_EnterSlaveMode(void)
{
    /* set mgr in slave mode */
    SYSFUN_ENTER_SLAVE_MODE();

}   /* End of L2MCAST_MGR_EnterSlaveMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_ReportSyslogMessage
 *-------------------------------------------------------------------------
 * PURPOSE : This function is an API to use syslog to report error message.
 * INPUT    : error_type    - the type of error, defined in l2mcast_mgr.h
 *            error_msg     - defined in syslog_mgr.h
 *           function_name - the specific function which error is occured.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_ReportSyslogMessage(UI8_T error_type, UI8_T error_msg, UI8_T *function_name)
{
    SYSLOG_OM_RecordOwnerInfo_T       owner_info;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    else
    {
        owner_info.level = error_type;
        owner_info.module_no = SYS_MODULE_L2MCAST;

        switch(error_type)
        {
            case SYSLOG_LEVEL_ERR:
                owner_info.function_no = L2MCAST_MGR_L2MCAST_MGR_ROWSTATUS_FUNCTION_NUMBER;
                owner_info.error_no = L2MCAST_MGR_L2MCAST_MGR_ROWSTATUS_ERROR_NUMBER;
                SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, SWITCH_TO_DEFAULT_MESSAGE_INDEX, function_name, 0, 0);
                break;

            case SYSLOG_LEVEL_CRIT:
                /* This level include Memory Allocate and Free failure and Create Task Failure
                 */
                owner_info.function_no = L2MCAST_MGR_L2MCAST_TASK_TASK_FUNCTION_NUMBER;
                owner_info.error_no = L2MCAST_MGR_L2MCAST_TASK_ERROR_NUMBER;
                SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, error_msg, function_name, 0, 0);
                break;

            default:
                break;
        }
    }

    return;
}   /* End of L2MCAST_MGR_ReportSyslogMessage() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE : The function notify us provision is completed.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_ProvisionComplete(void)
{
    return;
}   /* End of L2MCAST_MGR_ProvisionComplete() */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void L2MCAST_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{

    IGMPSNP_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
#if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
#endif
    return;
} /* end of L2MCAST_MGR_HandleHotInsertion() */


/* -----------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void L2MCAST_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    IGMPSNP_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
#if (SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
#endif
    return;
} /* end of L2MCAST_MGR_HandleHotRemoval() */



/* Callback functions
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_LPortOperUp
 *-------------------------------------------------------------------------
 * PURPOSE : SWCTRL uses this funtion to notify IGMPSNP that the port
 *           operation mode is up.
 * INPUT   : lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_LPortOperUp(UI32_T lport_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    else
    {
        IGMPSNP_MGR_LPortOperUp(lport_ifindex);
    }

    return;
}   /* End of L2MCAST_MGR_LPortOperUp() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_LPortNotOperUp
 *-------------------------------------------------------------------------
 * PURPOSE : SWCTRL uses this funtion to notify IGMPSNP that the port
 *           operation mode is not up.
 * INPUT   : lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_LPortNotOperUp(UI32_T lport_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    else
    {
        IGMPSNP_MGR_LPortNotOperUp(lport_ifindex);
        #if (SYS_CPNT_MLDSNP == TRUE)
        MLDSNP_MGR_LportNotOperUpCallBack(lport_ifindex);
        #endif
    }

    return;
}   /* End of L2MCAST_MGR_LPortNotOperUp() */

void L2MCAST_MGR_LPortEnterForwardingCallbackHandler(UI32_T xstp_id, UI32_T lport_ifindex)
{
    if(xstp_id!=0)
    {
    #if (SYS_CPNT_L2MCAST == TRUE)
        IGMPSNP_MGR_LportEnterForwarding(xstp_id, lport_ifindex);
    
    #endif
    }
    else
    {
        IGMPSNP_MGR_LPortOperUp(lport_ifindex);
    }
}

void L2MCAST_MGR_LPortLeaveForwardingCallbackHandler(UI32_T xstp_id, UI32_T lport_ifindex)
{
#if (SYS_CPNT_L2MCAST == TRUE)	
    if(xstp_id!=0)
    {
      IGMPSNP_MGR_LPortLeaveForwarding(xstp_id, lport_ifindex);
  
    }
    else
    {
        IGMPSNP_MGR_LPortNotOperUp(lport_ifindex);
        #if (SYS_CPNT_MLDSNP == TRUE)
        MLDSNP_MGR_LportNotOperUpCallBack(lport_ifindex);
        #endif
    }
#endif
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_TrunkMemberAdd1st
 *-------------------------------------------------------------------------
 * PURPOSE : SWCTRL uses this funtion to notify IGMPSNP that the first
 *           trunk member is added.
 * INPUT   : trunk_ifindex - trunk interface index
 *           lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_TrunkMemberAdd1st(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    else
    {
        IGMPSNP_MGR_TrunkMemberAdd1st(trunk_ifindex, lport_ifindex);
        #if (SYS_CPNT_MLDSNP == TRUE)
        MLDSNP_MGR_TrunkMemberAdd1sCallback(trunk_ifindex, lport_ifindex);
        #endif   
    }

    return;
}   /* End of L2MCAST_MGR_TrunkMemberAdd1st() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_TrunkMemberAdd
 *-------------------------------------------------------------------------
 * PURPOSE : SWCTRL uses this funtion to notify IGMPSNP that the
 *           2nd/3rd/4th trunk member is added.
 * INPUT   : trunk_ifindex - trunk interface index
 *           lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_TrunkMemberAdd(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return;

    IGMPSNP_MGR_TrunkMemberAdd(trunk_ifindex, lport_ifindex);

    #if (SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_MGR_TrunkMemberAddCallback(trunk_ifindex, lport_ifindex);
    #endif
    
    return;
}   /* End of L2MCAST_MGR_TrunkMemberAdd() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_TrunkMemberDelete
 *-------------------------------------------------------------------------
 * PURPOSE : Enter master mode
 * INPUT   : SWCTRL uses this funtion to notify IGMPSNP that the last
 *           2nd/3rd/4th trunk member is deleted.
 * OUTPUT  : trunk_ifindex - trunk interface index
 *           lport_ifindex - lport interface index
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_TrunkMemberDelete(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return;

    IGMPSNP_MGR_TrunkMemberDelete(trunk_ifindex, lport_ifindex);

    #if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_MGR_TrunkMemberDeleteCallback(trunk_ifindex, lport_ifindex);
    #endif

    return;
}   /* End of L2MCAST_MGR_TrunkMemberDelete() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_TrunkMemberDeleteLast
 *-------------------------------------------------------------------------
 * PURPOSE : SWCTRL uses this funtion to notify IGMPSNP that last trunk
 *           member is deleted.
 * INPUT   : trunk_ifindex - trunk interface index
 *           lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_TrunkMemberDeleteLst(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    else
    {
        IGMPSNP_MGR_TrunkMemberDeleteLst(trunk_ifindex, lport_ifindex);

        #if(SYS_CPNT_MLDSNP == TRUE)
        MLDSNP_MGR_TrunkMemberLstDeleteCallback(trunk_ifindex, lport_ifindex);
        #endif
    }

    return;
}   /* End of L2MCAST_MGR_TrunkMemberDeleteLast() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_VlanCreate
 *-------------------------------------------------------------------------
 * PURPOSE : VLAN uses this funtion to notify MVR that the vlan is
 *           created.
 * INPUT   : vlan_ifindex - vlan interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_VlanCreate(UI32_T vlan_ifindex, UI32_T vlan_status)
{
    UI32_T vid;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
       return;
    }
    else
    {
        if(vlan_ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
        	return; 

        VLAN_IFINDEX_CONVERTTO_VID(vlan_ifindex, vid);

        IGMPSNP_MGR_VlanCreate(vid);
        #if(SYS_CPNT_MLDSNP == TRUE)
         MLDSNP_MGR_VlanCreatedCallBack(vlan_ifindex, vlan_status);
        #endif
    }

    return;

} /* End of L2MCAST_MGR_VlanCreate() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_VlanDestory
 *-------------------------------------------------------------------------
 * PURPOSE : VLAN uses this funtion to notify IGMPSNP that the vlan is
 *           destroyed.
 * INPUT   : vlan_ifindex - vlan interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_VlanDestroy(UI32_T vlan_ifindex, UI32_T vlan_status)
{
    UI32_T vid;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    else
    {
        if(vlan_ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
        	return; 
        
        VLAN_IFINDEX_CONVERTTO_VID(vlan_ifindex, vid);

        IGMPSNP_MGR_VlanDestory(vid);
        #if(SYS_CPNT_MLDSNP == TRUE)
        MLDSNP_MGR_VlanDestroyCallBack(vlan_ifindex, vlan_status);
        #endif
    }

    return;
}   /* End of L2MCAST_MGR_VlanDestory() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_VlanMemberAdd
 *-------------------------------------------------------------------------
 * PURPOSE : VLAN uses this funtion to notify IGMPSNP that the vlan member
 *           is added.
 * INPUT   : vlan_ifindex  - vlan interface index
 *           lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :vlan_status -- VLAN_TYPE_VLAN_STATUS_NONE
 *                         VLAN_TYPE_VLAN_STATUS_CONFIG
 *                         VLAN_TYPE_VLAN_STATUS_GVRP
 *                         VLAN_TYPE_VLAN_STATUS_AUTO
 *                         VLAN_TYPE_VLAN_STATUS_VOICE
 *                         VLAN_TYPE_VLAN_STATUS_MVR
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_VlanMemberAdd(UI32_T vlan_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return;

    if(vlan_ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
    	return; 


    IGMPSNP_MGR_VlanMemberAdd(vlan_ifindex, lport_ifindex, vlan_status);

    return;
}   /* End of L2MCAST_MGR_VlanMemberAdd() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_VlanMemberDelete
 *-------------------------------------------------------------------------
 * PURPOSE : VLAN uses this funtion to notify IGMPSNP that the vlan member
 *           is deleted.
 * INPUT   : vlan_ifindex  - vlan interface index
 *           lport_ifindex - lport interface index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_VlanMemberDelete(UI32_T vlan_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return;

    if(vlan_ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
    	return; 

    IGMPSNP_MGR_VlanMemberDelete(vlan_ifindex, lport_ifindex, vlan_status);

    #if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_MGR_VlanMemberDeletedCallBack(vlan_ifindex, lport_ifindex);
    #endif

    return;
}   /* End of L2MCAST_MGR_VlanMemberDelete() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_VlanMemberChanged
 *-------------------------------------------------------------------------
 * PURPOSE : VLAN uses this funtion to notify IGMPSNP and MVR that the vlan member
 *           is changed.
 * INPUT   : vlan_ifindex  - vlan interface index
 *           lport_ifindex - lport interface index
 *           vlan_status - member add type
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :vlan_status -- VLAN_TYPE_VLAN_STATUS_NONE
 *                         VLAN_TYPE_VLAN_STATUS_CONFIG
 *                         VLAN_TYPE_VLAN_STATUS_GVRP
 *                         VLAN_TYPE_VLAN_STATUS_AUTO
 *                         VLAN_TYPE_VLAN_STATUS_VOICE
 *                         VLAN_TYPE_VLAN_STATUS_MVR
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_VlanMemberChanged(UI32_T vlan_ifindex, UI32_T lport_ifidx, UI32_T vlan_status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
       return;

    if(vlan_ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
    	return; 
    
    IGMPSNP_MGR_VlanMemberAdd(vlan_ifindex, lport_ifidx, vlan_status);

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_VlanDestory
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to process XSTP topology change
 * INPUT   : 
 *           lport_ifindex - lport interface index
 *           type          - XSTP type (STP/RSTP/MSTP)
 *           is_rt_br      - if the local switch is the RootBridge
 *           tc_time       - topo recover time. (Network Topo will be convergence after this time.)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_XSTPTopoChange(BOOL_T is_mstp_mode,UI32_T xstid,UI32_T lport,BOOL_T is_root,UI32_T tc_timer/*,UI8_T *vlan_bit_map*/)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return;

    IGMPSNP_MGR_XSTPTopoChange(is_mstp_mode, xstid, lport, is_root, tc_timer/*, vlan_bit_map*/);

    return;
}   

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_VlanPortModeChange
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to vlan port mode change
 * INPUT   : 
 *           lport           - lport interface index
 *           vlan_port_mode  - VLAN port mode
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_VlanPortModeChange(UI32_T lport, UI32_T vlan_port_mode)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
        return;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_ReceiveIgmpsnpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE : This function process receiver igmp packet
 * INPUT   : 
 *           lport           - lport interface index
 *           vlan_port_mode  - VLAN port mode
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_ReceiveIgmpsnpPacketCallback(L_MM_Mref_Handle_T	*mref_handle_p,
                                                UI8_T 	        dst_mac[6],
                                                UI8_T 	        src_mac[6],
                                                UI16_T 	        tag_info,
                                                UI16_T           type,
                                                UI32_T           pkt_length,
                                                UI32_T 	        lport)
{
    IGMPSNP_MGR_ProcessMulticastPkt(mref_handle_p, dst_mac, src_mac, tag_info, type, pkt_length, lport);

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_ReceiveMldsnpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE : This function process receiver mld packet
 * INPUT   : 
 *           lport           - lport interface index
 *           vlan_port_mode  - VLAN port mode
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_MGR_ReceiveMldsnpPacketCallback(L_MM_Mref_Handle_T * mref_handle_p, 
                                             UI8_T dst_mac [ 6 ], 
                                             UI8_T src_mac [ 6 ], 
                                             UI16_T tag_info, 
                                             UI16_T type, 
                                             UI32_T pkt_length, 
                                             UI32_T ip_ext_opt_len, 
                                             UI32_T lport)
{

#if (SYS_CPNT_MLDSNP == TRUE)
        MLDSNP_MGR_ProcessRcvdMldPdu(mref_handle_p, dst_mac, src_mac, tag_info, type, pkt_length, ip_ext_opt_len, lport);
#endif
#if(SYS_CPNT_MLDSNP == FALSE)
    L_MM_Mref_Release(&mref_handle_p);
#endif

}

#if(SYS_CPNT_IGMPAUTH == TRUE)            
void L2MCAST_MGR_RadiusIgmpAuthCallback(UI32_T result, UI32_T port_ifindex, UI32_T src_ip, UI32_T dst_ip, UI32_T vid, UI8_T *src_mac, UI8_T msg_type)
{
    IGMPSNP_MGR_ProcessRadiusIgmpAuth(result, port_ifindex, (struct igs_in4_addr *)&src_ip, (struct igs_in4_addr *)&dst_ip, vid, src_mac, msg_type);
}
#endif

/* End of this file */

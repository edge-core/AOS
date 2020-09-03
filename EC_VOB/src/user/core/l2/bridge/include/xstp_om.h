/*-----------------------------------------------------------------------------
 * Module Name: xstp_om.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the XSTP
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    05/30/2001 - Allen Cheng, Created
 *    06/12/2002 - Kelly Chen, Added
 *    02-09-2004 - Kelly Chen, Revise the implementations of 802.1w/1s according to the IEEE 802.1D/D3
 *    05/21/2007 - Timon Chang, Move privately used resources to xstp_om_private.h
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef XSTP_OM_H
#define XSTP_OM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sysfun.h"
#include "xstp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* For SNMP */
#define XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP               0xFFFFFFFF
#define XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_1K              1
#define XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_2K              2
#define XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_3K              3
#define XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_4K              4

#define XSTP_OM_IPCMSG_TYPE_SIZE sizeof(union XSTP_OM_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
    XSTP_OM_IPC_GETFORCEVERSION = 1,
    XSTP_OM_IPC_GETMAXHOPCOUNT,
    XSTP_OM_IPC_GETMAXINSTANCENUMBER,
    XSTP_OM_IPC_GETNUMOFACTIVETREE,
    XSTP_OM_IPC_GETPATHCOSTMETHOD_EX,
    XSTP_OM_IPC_GETREGIONNAME,
    XSTP_OM_IPC_GETREGIONREVISION,
    XSTP_OM_IPC_GETSPANNINGTREESTATUS,
    XSTP_OM_IPC_GETTRAPFLAGTC,
    XSTP_OM_IPC_GETTRAPFLAGNEWROOT,
    XSTP_OM_IPC_GETLPORTPATHCOST,
    XSTP_OM_IPC_GETMSTIDFROMMSTCONFIGURATIONTABLEBYVLAN,
    XSTP_OM_IPC_GETNEXTXSTPMEMBERFROMMSTCONFIGURATIONTABLE,
    XSTP_OM_IPC_ISMSTFULLMEMBERTOPOLOGY,
    XSTP_OM_IPC_GETCURRENTCFGINSTANCENUMBER,
    XSTP_OM_IPC_GETINSTANCEENTRYID,
    XSTP_OM_IPC_GETPORTSTATEBYVLAN,
    XSTP_OM_IPC_GETPORTSTATEBYINSTANCE,
    XSTP_OM_IPC_ISPORTFORWARDINGSTATEBYVLAN,
    XSTP_OM_IPC_ISPORTFORWARDINGSTATEBYINSTANCE,
    XSTP_OM_IPC_ISPORTBLOCKINGSTATEBYVLAN,
    XSTP_OM_IPC_ISPORTBLOCKINGSTATEBYINSTANCE,
    XSTP_OM_IPC_GETNEXTEXISTINGMSTIDBYLPORT,
    XSTP_OM_IPC_GETNEXTEXISTINGMEMBERVIDBYMSTID,
    XSTP_OM_IPC_GETMSTINSTANCEINDEXBYMSTID,
    XSTP_OM_IPC_GETRUNNINGSYSTEMSPANNINGTREESTATUS,
    XSTP_OM_IPC_GETSYSTEMSPANNINGTREESTATUS,
    XSTP_OM_IPC_GETRUNNINGSYSTEMSPANNINGTREEVERSION,
    XSTP_OM_IPC_GETSYSTEMSPANNINGTREEVERSION,
    XSTP_OM_IPC_GETRUNNINGFORWARDDELAY,
    XSTP_OM_IPC_GETFORWARDDELAY,
    XSTP_OM_IPC_GETRUNNINGHELLOTIME,
    XSTP_OM_IPC_GETHELLOTIME,
    XSTP_OM_IPC_GETRUNNINGMAXAGE,
    XSTP_OM_IPC_GETMAXAGE,
    XSTP_OM_IPC_GETRUNNINGPATHCOSTMETHOD,
    XSTP_OM_IPC_GETRUNNINGTRANSMISSIONLIMIT,
    XSTP_OM_IPC_GETTRANSMISSIONLIMIT,
    XSTP_OM_IPC_GETRUNNINGMSTPRIORITY,
    XSTP_OM_IPC_GETMSTPRIORITY,
    XSTP_OM_IPC_GETRUNNINGMSTPORTPRIORITY,
    XSTP_OM_IPC_GETMSTPORTPRIORITY,
    XSTP_OM_IPC_GETRUNNINGPORTLINKTYPEMODE,
    XSTP_OM_IPC_GETPORTLINKTYPEMODE,
    XSTP_OM_IPC_GETRUNNINGPORTPROTOCOLMIGRATION,
    XSTP_OM_IPC_GETPORTPROTOCOLMIGRATION,
    XSTP_OM_IPC_GETRUNNINGPORTADMINEDGEPORT,
    XSTP_OM_IPC_GETPORTADMINEDGEPORT,
    XSTP_OM_IPC_GETDOT1DMSTPORTENTRY,
    XSTP_OM_IPC_GETNEXTDOT1DMSTPORTENTRY,
    XSTP_OM_IPC_GETNEXTPORTMEMBERBYINSTANCE,
    XSTP_OM_IPC_GETDOT1DMSTPORTENTRYX,
    XSTP_OM_IPC_GETNEXTDOT1DMSTPORTENTRYX,
    XSTP_OM_IPC_GETDOT1DMSTEXTPORTENTRY,
    XSTP_OM_IPC_GETNEXTDOT1DMSTEXTPORTENTRY,
    XSTP_OM_IPC_GETRUNNINGMSTPREVISIONLEVEL,
    XSTP_OM_IPC_GETMSTPREVISIONLEVEL,
    XSTP_OM_IPC_GETRUNNINGMSTPMAXHOP,
    XSTP_OM_IPC_GETMSTPMAXHOP,
    XSTP_OM_IPC_GETMSTPCONFIGURATIONENTRY,
    XSTP_OM_IPC_GETMSTPINSTANCEVLANMAPPED,
    XSTP_OM_IPC_GETMSTPINSTANCEVLANMAPPEDFORMSB,
    XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANMAPPED,
    XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANMAPPEDFORMSB,
    XSTP_OM_IPC_GETMSTPINSTANCEVLANCONFIGURATION,
    XSTP_OM_IPC_GETMSTPINSTANCEVLANCONFIGURATIONFORMSB,
    XSTP_OM_IPC_GETRUNNINGMSTPINSTANCEVLANCONFIGURATION,
    XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANCONFIGURATION,
    XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANCONFIGURATIONFORMSB,
    XSTP_OM_IPC_ISMSTINSTANCEEXISTING,
    XSTP_OM_IPC_GETNEXTEXISTEDINSTANCE,
    XSTP_OM_IPC_ISMSTINSTANCEEXISTINGINMSTCONFIGTABLE,
    XSTP_OM_IPC_GETNEXTEXISTEDINSTANCEFORMSTCONFIGTABLE,
    XSTP_OM_IPC_GETMSTPORTROLE,
    XSTP_OM_IPC_GETMSTPORTSTATE,
    XSTP_OM_IPC_GETNEXTVLANMEMBERBYINSTANCE,
    XSTP_OM_IPC_GETNEXTMSTIDFROMMSTCONFIGURATIONTABLEBYVLAN,
    XSTP_OM_IPC_ISMEMBERPORTOFINSTANCEEX,
    XSTP_OM_IPC_GETCONFIGDIGEST,
    XSTP_OM_IPC_GETMSTPROWSTATUS,
    XSTP_OM_IPC_GETNEXTMSTPROWSTATUS,
    XSTP_OM_IPC_GETPORTADMINPATHCOST,
    XSTP_OM_IPC_GETPORTOPERPATHCOST,
    XSTP_OM_IPC_GETMSTPORTADMINPATHCOST,
    XSTP_OM_IPC_GETMSTPORTOPERPATHCOST,
    XSTP_OM_IPC_GETRUNNINGPORTSPANNINGTREESTATUS,
    XSTP_OM_IPC_GETPORTSPANNINGTREESTATUS,
    XSTP_OM_IPC_GETDESIGNATEDROOT,
    XSTP_OM_IPC_GETBRIDGEIDCOMPONENT,
    XSTP_OM_IPC_GETPORTDESIGNATEDROOT,
    XSTP_OM_IPC_GETPORTDESIGNATEDBRIDGE,
    XSTP_OM_IPC_GETPORTDESIGNATEDPORT,
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
    XSTP_OM_IPC_GETPORTROOTGUARDSTATUS,
    XSTP_OM_IPC_GETRUNNINGPORTROOTGUARDSTATUS,
#endif
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    XSTP_OM_IPC_GETPORTBPDUGUARDSTATUS,
    XSTP_OM_IPC_GETRUNNINGPORTBPDUGUARDSTATUS,
    XSTP_OM_IPC_GETPORTBPDUGUARDAUTORECOVERY,
    XSTP_OM_IPC_GETRUNNINGPORTBPDUGUARDAUTORECOVERY,
    XSTP_OM_IPC_GETPORTBPDUGUARDAUTORECOVERYINTERVAL,
    XSTP_OM_IPC_GETRUNNINGPORTBPDUGUARDAUTORECOVERYINTERVAL,
#endif
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    XSTP_OM_IPC_GETPORTBPDUFILTERSTATUS,
    XSTP_OM_IPC_GETRUNNINGPORTBPDUFILTERSTATUS,
#endif
#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
    XSTP_OM_IPC_GETCISCOPRESTANDARDCOMPATIBILITY,
    XSTP_OM_IPC_GETRUNNINGCISCOPRESTANDARDCOMPATIBILITY,
#endif
    XSTP_OM_IPC_GETRUNNINGSPANNINGPORTFLOODING,
    XSTP_OM_IPC_GETRUNNINGSPANNINGTREEFLOODINGBEHAVIOR,
    XSTP_OM_IPC_GETSPANNINGTREEFLOODINGBEHAVIOR,
    XSTP_OM_IPC_GETSPANNINGPORTFLOODING,
    XSTP_OM_IPC_GETPORTTCPROPSTOP,
    XSTP_OM_IPC_GETRUNNINGPORTTCPROPSTOP,
#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
    XSTP_OM_IPC_GETTCPROPGROUPPORTBITMAP,
    XSTP_OM_IPC_GETTCPROPNEXTGROUPPORTBITMAP,
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in XSTP_OM_IpcMsg_T.data
 */
#define XSTP_OM_GET_MSG_SIZE(field_name)                        \
            (XSTP_OM_IPCMSG_TYPE_SIZE +                         \
            sizeof(((XSTP_OM_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

/* IPC message structure
 */
typedef struct
{
    union XSTP_OM_IpcMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T ret_bool;
        UI8_T  ret_ui8;
        UI32_T ret_ui32;
    } type;

    union
    {
        BOOL_T               arg_bool;
        UI8_T                arg_ui8;
        UI32_T               arg_ui32;
        char                 arg_ar1[XSTP_TYPE_REGION_NAME_MAX_LENGTH+1];
        UI8_T                arg_ar2[16];
        XSTP_MGR_MstpEntry_T arg_mstentry;
        struct
        {
            UI32_T arg1;
            UI32_T arg2;
        } arg_grp1;
        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI32_T arg3;
        } arg_grp2;
        struct
        {
            UI32_T                       arg1;
            XSTP_MGR_Dot1dStpPortEntry_T arg2;
        } arg_grp3;
        struct
        {
            UI32_T                          arg1;
            UI32_T                          arg2;
            XSTP_MGR_Dot1dStpExtPortEntry_T arg3;
        } arg_grp4;
        struct
        {
            UI32_T                       arg1;
            XSTP_MGR_MstpInstanceEntry_T arg2;
        } arg_grp5;
        struct
        {
            UI32_T                       arg1;
            XSTP_MGR_BridgeIdComponent_T arg2;
        } arg_grp6;
        struct
        {
            UI32_T                       arg1;
            UI32_T                       arg2;
            XSTP_MGR_BridgeIdComponent_T arg3;
        } arg_grp7;
        struct
        {
            UI32_T                     arg1;
            UI32_T                     arg2;
            XSTP_MGR_PortIdComponent_T arg3;
        } arg_grp8;
#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
        struct
        {
            UI32_T                        arg1;
            UI8_T                         arg_ar1[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            BOOL_T                        arg_bool;
        } arg_grp9;
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/
    } data;
} XSTP_OM_IpcMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* ===================================================================== */
/* System Information function
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetForceVersion
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the system force version
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : force version
 *-------------------------------------------------------------------------
 */
UI8_T   XSTP_OM_GetForceVersion(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMaxHopCount
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the max_hop_count
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : max_hop_count
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetMaxHopCount(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMaxInstanceNumber
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the max_instance_number
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : max_instance_number
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetMaxInstanceNumber(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNumOfActiveTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the num_of_active_tree
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : num_of_active_tree
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetNumOfActiveTree(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPathCostMethod
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the path_cost_method
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : path_cost_method
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetPathCostMethod(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRegionName
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the region name
 * INPUT    : None
 * OUTPUT   : str       -- region name
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_GetRegionName(char *str);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRegionRevision
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the region_revision
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : region_revision
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetRegionRevision(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the spanning tree status
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : force version
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetSpanningTreeStatus(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTrapFlagTc
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the trap_flag_tc
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : trap_flag_tc
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetTrapFlagTc(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTrapFlagNewRoot
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the trap_flag_new_root
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : trap_flag_new_root
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetTrapFlagNewRoot(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTrapRxFlagTc
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the trap_flag_tc
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : trap_flag_tc
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetTrapRxFlagTc(void);

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetLportPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the path cost of the specified logical port
 * INPUT    :   lport       : lport
 * OUTPUT   :   path_cost   : path cost
 * RETURN   :   XSTP_TYPE_RETURN_OK/XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetLportPathCost(UI32_T lport, UI32_T *path_cost);
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstidFromMstConfigurationTableByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mstid value form mst configuration table for a specified
 *            vlan.
 * INPUT    : vid       -- vlan number
 *            mstid     -- mstid value point
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_GetMstidFromMstConfigurationTableByVlan(UI32_T vid,
                                                     UI32_T *mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextXstpMemberFromMstConfigurationTable
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next XSTP member form mst config table for a
 *            specified instance
 * INPUT    : mstid     -- this instance value
 *            vid       -- vlan id pointer
 * OUTPUT   : vid       -- next vlan id pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextXstpMemberFromMstConfigurationTable(UI32_T mstid,
                                                           UI32_T *vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMstFullMemberTopology
 * ------------------------------------------------------------------------
 * PURPOSE  : This function returns TRUE if mst_topology_method is full member topology.
 *            Otherwise, return FALSE.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 *-------------------------------------------------------------------------
 */
BOOL_T   XSTP_OM_IsMstFullMemberTopology(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetCurrentCfgInstanceNumber
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the current instance number created by user.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : num_of_cfg_msti
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetCurrentCfgInstanceNumber(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetInstanceEntryId
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the entry_id for the specified xstid
 * INPUT    : xstid -- MST instance ID
 * OUTPUT   : None
 * RETUEN   : entry_id (stg_id) for the specified xstid
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI8_T   XSTP_OM_GetInstanceEntryId(UI32_T xstid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for XSTP OM.
 *
 * INPUT   : msgbuf_p - input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p - output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : 1. The size of msgbuf_p->msg_buf must be large enough to carry
 *              any response messages.
 *-----------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/*=============================================================================
 * Move from xstp_svc.h
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   U32_T  *state   -- the pointer of state value
 *                                 VAL_dot1dStpPortState_blocking
 *                                 VAL_dot1dStpPortState_listening
 *                                 VAL_dot1dStpPortState_learning
 *                                 VAL_dot1dStpPortState_forwarding
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetPortStateByVlan(UI32_T vid, UI32_T lport, UI32_T *state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T Xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   U32_T  *state   -- the pointer of state value
 *                                 VAL_dot1dStpPortState_blocking
 *                                 VAL_dot1dStpPortState_listening
 *                                 VAL_dot1dStpPortState_learning
 *                                 VAL_dot1dStpPortState_forwarding
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetPortStateByInstance(UI32_T xstid, UI32_T lport, UI32_T *state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsPortForwardingStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in VAL_dot1dStpPortState_forwarding,
 *              else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_IsPortForwardingStateByVlan(UI32_T vid, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsPortForwardingStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in VAL_dot1dStpPortState_forwarding,
 *              else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_IsPortForwardingStateByInstance(UI32_T xstid, UI32_T lport);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsPortBlockingStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in Blocking, else FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsPortBlockingStateByVlan(UI32_T vid, UI32_T lport);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsPortBlockingStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in Blocking, else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsPortBlockingStateByInstance(UI32_T xstid, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextExistingMstidByLport
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the next mst instance id with a specified lport.
 * INPUT    :   UI32_T lport    -- lport number
 * OUTPUT   :   UI32_T *xstid   -- mst instance id
 * OUTPUT   :   UI32_T *xstid   -- next mst instance id
 * RETURN   :   TRUE if the next mstid is existing, else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextExistingMstidByLport(UI32_T lport, UI32_T *xstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextExistingMemberVidByMstid
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the next member vlan id with a specified mst instance id.
 * INPUT    :   UI32_T xstid    -- mst instance id
 * OUTPUT   :   UI32_T *vid     -- vlan id
 * OUTPUT   :   UI32_T *vid     -- next member vlan id
 * RETURN   :   TRUE if the next member vlan is existing, else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextExistingMemberVidByMstid(UI32_T xstid, UI32_T *vid);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstInstanceIndexByMstid
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the entry_id for the specified xstid
 * INPUT    : xstid -- MST instance ID
 * OUTPUT   : mstudx -- MST instance INDEX
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T   XSTP_OM_GetMstInstanceIndexByMstid(UI32_T xstid, UI32_T *mstidx);


/*=============================================================================
 * Move from xstp_mgr.h
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningSystemSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global spanning tree status.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningSystemSpanningTreeStatus(UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetSystemSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global spanning tree status.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetSystemSpanningTreeStatus(UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningSystemSpanningTreeVersion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree mode.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *mode          -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningSystemSpanningTreeVersion(UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetSystemSpanningTreeVersion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree mode.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *mode          -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetSystemSpanningTreeVersion(UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the forward_delay time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *forward_delay -- pointer of the forward_delay value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningForwardDelay(UI32_T *forward_delay);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the forward_delay time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *forward_delay -- pointer of the forward_delay value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetForwardDelay(UI32_T *forward_delay);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the hello_time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *hello_time      -- pointer of the hello_time value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningHelloTime(UI32_T *hello_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the hello_time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *hello_time      -- pointer of the hello_time value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetHelloTime(UI32_T *hello_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the max_age information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *max_age         -- pointer of the max_age value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMaxAge(UI32_T *max_age);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the max_age information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *max_age         -- pointer of the max_age value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMaxAge(UI32_T *max_age);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPathCostMethod
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the default path cost calculation method.
 * INPUT    :   UI32_T  *pathcost_method  -- pointer of the method value
 * OUTPUT   :   None
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPathCostMethod(UI32_T *pathcost_method);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPathCostMethod_Ex
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the default path cost calculation method.
 * INPUT    :   UI32_T  *pathcost_method  -- pointer of the method value
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPathCostMethod_Ex(UI32_T *pathcost_method);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the transmission limit count vlaue.
 * INPUT    :   None
 * OUTPUT   :   UI32_T  *tx_hold_count  -- pointer of the TXHoldCount value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningTransmissionLimit(UI32_T *tx_hold_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the transmission limit count vlaue.
 * INPUT    :   None
 * OUTPUT   :   UI32_T  *tx_hold_count  -- pointer of the TXHoldCount value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetTransmissionLimit(UI32_T *tx_hold_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge priority information.
 * INPUT    :   None
 * OUTPUT   :   UI16_T  mstid            -- instance value
 *              UI32_T  *priority        -- pointer of the priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMstPriority(UI32_T mstid,
                                      UI32_T *priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge priority information.
 * INPUT    :   None
 * OUTPUT   :   UI16_T  mstid            -- instance value
 *              UI32_T  *priority        -- pointer of the priority value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstPriority(UI32_T mstid,
                               UI32_T *priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port priority for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *priority        -- pointer of the priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * REF      :   RFC-1493/dot1dStpPortEntry 2
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMstPortPriority(UI32_T lport,
                                          UI32_T mstid,
                                          UI32_T *priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port priority for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *priority        -- pointer of the priority value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * REF      :   RFC-1493/dot1dStpPortEntry 2
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstPortPriority(UI32_T lport,
                                   UI32_T mstid,
                                   UI32_T *priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortLinkTypeMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port link_type mode of the port for the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetRunningPortLinkTypeMode(UI32_T lport,
                                            UI32_T mstid,
                                            UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortLinkTypeMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port link_type mode of the port for the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetPortLinkTypeMode(UI32_T lport,
                                     UI32_T mstid,
                                     UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortProtocolMigration
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get protocol_migration status for a port in the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetRunningPortProtocolMigration(UI32_T lport,
                                                 UI32_T mstid,
                                                 UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortProtocolMigration
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get protocol_migration status for a port in the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetPortProtocolMigration(UI32_T lport,
                                          UI32_T mstid,
                                          UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortAdminEdgePort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get edge_port status for a port for in specified spanning
 *              tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetRunningPortAdminEdgePort(UI32_T lport,
                                             UI32_T mstid,
                                             UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortAdminEdgePort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get edge_port status for a port for in specified spanning
 *              tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetPortAdminEdgePort(UI32_T lport,
                                      UI32_T mstid,
                                      UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetDot1dMstPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 *              UI32_T mstid                -- instance value
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetDot1dMstPortEntry(UI32_T mstid,
                                     XSTP_MGR_Dot1dStpPortEntry_T *port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_OM_GetNextDot1dMstPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 *              mstid                       -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   If next available port entry is available, the
 *              port_entry->dot1d_stp_port will be updated and the entry
 *              info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextDot1dMstPortEntry(UI32_T *mstid,
                                         XSTP_MGR_Dot1dStpPortEntry_T *port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_OM_GetNextPortMemberByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              lport                       -- lport value
 * OUTPUT   :   lport                       -- next lport
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextPortMemberByInstance(UI32_T mstid, UI32_T *lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetDot1dMstPortEntryX
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 *              UI32_T mstid                -- instance value
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. State is backing for a port with no link.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetDot1dMstPortEntryX(UI32_T mstid,
                                      XSTP_MGR_Dot1dStpPortEntry_T *port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_OM_GetNextDot1dMstPortEntryX
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 *              mstid                       -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. If next available port entry is available, the
 *                 port_entry->dot1d_stp_port will be updated and the entry
 *                 info will be retrieved from the table.
 *              2. State is backing for a port with no link.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextDot1dMstPortEntryX(UI32_T *mstid,
                                          XSTP_MGR_Dot1dStpPortEntry_T *port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetDot1dMstExtPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *ext_port_entry                 -- pointer of the specified
 *                                                 port ext_entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetDot1dMstExtPortEntry(UI32_T mstid,
                                        UI32_T lport,
                                        XSTP_MGR_Dot1dStpExtPortEntry_T *ext_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_OM_GetNextDot1dMstExtPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available ext port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   *lport                          -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *ext_port_entry                 -- pointer of the specified
 *                                                 port ext_entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   If next available port ext_entry is available, the
 *              ext_port_entry->dot1d_stp_port will be updated and the
 *              entry info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextDot1dMstExtPortEntry(UI32_T mstid,
                                            UI32_T *lport,
                                            XSTP_MGR_Dot1dStpExtPortEntry_T *ext_port_entry);

/*-------------------------------------------------------------------------
 * The following Functions only provide for MSTP
 *-------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstpRevisionLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP revision level value.
 * INPUT    :
 * OUTPUT   :   U32_T *revision     -- pointer of the revision value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMstpRevisionLevel(UI32_T *revision);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpRevisionLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP revision level value.
 * INPUT    :
 * OUTPUT   :   U32_T *revision     -- pointer of the revision value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpRevisionLevel(UI32_T *revision);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstpMaxHop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP Max_Hop count.
 * INPUT    :
 * OUTPUT   :   U32_T *hop_count              -- pointer of max_hop count
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMstpMaxHop(UI32_T *hop_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpMaxHop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP Max_Hop count.
 * INPUT    :
 * OUTPUT   :   U32_T *hop_count              -- pointer of max_hop count
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpMaxHop(UI32_T *hop_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpConfigurationEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the configuration entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :
 * OUTPUT   :   *mstp_entry      -- pointer of the configuration entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpConfigurationEntry(XSTP_MGR_MstpEntry_T  *mstp_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpInstanceVlanMapped
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of map VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Mapping_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpInstanceVlanMapped(UI32_T mstid,
                                          XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpInstanceVlanMappedForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of map VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Mapping_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpInstanceVlanMappedForMSB(UI32_T mstid,
                                                XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_NextGetMstpInstanceVlanMapped
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Mapping_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextMstpInstanceVlanMapped(UI32_T *mstid,
                                              XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextMstpInstanceVlanMappedForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Mapping_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextMstpInstanceVlanMappedForMSB(UI32_T *mstid,
                                                    XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Configuration_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpInstanceVlanConfiguration(UI32_T mstid,
                                                 XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpInstanceVlanConfigurationForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Configuration_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpInstanceVlanConfigurationForMSB(UI32_T mstid,
                                                       XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
  * RETURN   :  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   From Configuration_Table.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMstpInstanceVlanConfiguration(UI32_T mstid,
                                                        XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Configuration_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextMstpInstanceVlanConfiguration(UI32_T *mstid,
                                                     XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextMstpInstanceVlanConfigurationForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Configuration_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextMstpInstanceVlanConfigurationForMSB(UI32_T *mstid,
                                                           XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMstInstanceExisting
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funcion returns true if the mst instance exist for mst
 *              mapping table (active).Otherwise, returns false.
 * INPUT    :   UI32_T mstid             -- the instance id
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsMstInstanceExisting(UI32_T mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextExistedInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next existed MST instance(active) for mst mapping table.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextExistedInstance(UI32_T *mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMstInstanceExistingInMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funcion returns true if the mst instance exist for mst
 *              config table(inactive).Otherwise, returns false.
 * INPUT    :   UI32_T mstid             -- the instance id
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsMstInstanceExistingInMstConfigTable(UI32_T mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextExistedInstanceForMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next existed MST instance (inactive) for mst config
 *            table.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextExistedInstanceForMstConfigTable(UI32_T *mstid);

 /*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port role in a specified spanning tree.
 * INPUT    :   UI32_T lport                 -- lport number
 *              UI32_T mstid                 -- instance value
 * OUTPUT   :   UI32_T  *role                -- the pointer of role value
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- get successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid  out of range
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetMstPortRole(UI32_T lport,
                               UI32_T mstid,
                               UI32_T *role);

 /*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state in a specified spanning tree.
 * INPUT    :   UI32_T lport                 -- lport number
 *              UI32_T mstid                 -- instance value
 * OUTPUT   :   U32_T  *state                -- the pointer of state value
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- get successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid  out of range
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetMstPortState(UI32_T lport,
                                UI32_T mstid,
                                UI32_T *state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextVlanMemberByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next vlan member for a specified instance
 * INPUT    : mstid                 -- instance value
 *            vid       -- vlan id pointer
 * OUTPUT   : vid       -- next vlan id pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextVlanMemberByInstance(UI32_T mstid,
                                             UI32_T *vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstidFromMstConfigurationTableByVlanEx
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mstid value form mst configuration table for a specified
 *            vlan.
 * INPUT    : vid       -- vlan number
 *            mstid     -- mstid value point
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstidFromMstConfigurationTableByVlanEx(UI32_T vid, UI32_T *mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextMstidFromMstConfigurationTableByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mstid value form mst configuration table for a specified
 *            vlan.
 * INPUT    : vid       -- vlan number
 *            mstid     -- mstid value point
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextMstidFromMstConfigurationTableByVlan(UI32_T *vid, UI32_T *mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMemberPortOfInstanceEx
 *-------------------------------------------------------------------------
 * PURPOSE  : Check whether the specified lport is the member of this
 *            spanning tree instance
 * INPUT    : mstid     -- mstid value
 *            lport     -- lport
 * OUTPUT   : None
 * RETUEN   : TRUE if the specified vlan is the member of this instance, else
 *            FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_IsMemberPortOfInstanceEx(UI32_T mstid, UI32_T lport);

#ifdef  XSTP_TYPE_PROTOCOL_MSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetConfigDigest
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the configuration digest
 * INPUT    :   None
 * OUTPUT   :   config_digest           -- pointer of a 16 octet buffer for
 *                                         the configuration digest
 * RETURN   :   TRUE/FALSE
 * NOTES    :   Ref to the description in 13.7, IEEE 802.1s-2002
 * ------------------------------------------------------------------------
 */
BOOL_T    XSTP_OM_GetConfigDigest(UI8_T *config_digest);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpRowStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the entry can be
 *            get successfully.  Otherwise, return false.
 * INPUT    : mstid       -- instance value
 * OUTPUT   : row_status  -- VAL_dot1qVlanStaticRowStatus_active
 *                           VAL_dot1qVlanStaticRowStatus_notInService
 *                           VAL_dot1qVlanStaticRowStatus_notReady
 *                           VAL_dot1qVlanStaticRowStatus_createAndGo
 *                           VAL_dot1qVlanStaticRowStatus_createAndWait
 *                           VAL_dot1qVlanStaticRowStatus_destroy
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T XSTP_OM_GetMstpRowStatus(UI32_T mstid, UI32_T *row_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextMstpRowStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the entry can be
 *            get successfully.  Otherwise, return false.
 * INPUT    : mstid       -- instance value
 * OUTPUT   : row_status  -- VAL_dot1qVlanStaticRowStatus_active
 *                           VAL_dot1qVlanStaticRowStatus_notInService
 *                           VAL_dot1qVlanStaticRowStatus_notReady
 *                           VAL_dot1qVlanStaticRowStatus_createAndGo
 *                           VAL_dot1qVlanStaticRowStatus_createAndWait
 *                           VAL_dot1qVlanStaticRowStatus_destroy
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T XSTP_OM_GetNextMstpRowStatus(UI32_T *mstid, UI32_T *row_status);

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortAdminPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the admin path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *admin_path_cost -- admin path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   1. If the default Path Cost is being used, return '0'.
 *              2. It is equal to external_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPortAdminPathCost(UI32_T lport, UI32_T *admin_path_cost);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortOperPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the oper path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *oper_path_cost  -- oper path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   It is equal to external_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPortOperPathCost(UI32_T lport, UI32_T *oper_path_cost);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortAdminPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the admin path_cost of the port for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *admin_path_cost -- admin path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   1. If the default Path Cost is being used, return '0'.
 *              2. It is equal to internal_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetMstPortAdminPathCost(UI32_T lport, UI32_T mstid, UI32_T *admin_path_cost);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortOperPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the oper path_cost of the port for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *oper_path_cost  -- oper path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   It is equal to internal_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetMstPortOperPathCost(UI32_T lport, UI32_T mstid, UI32_T *oper_path_cost);

/* per_port spanning tree : begin */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree status of the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortSpanningTreeStatus(UI32_T lport, UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree status of the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortSpanningTreeStatus(UI32_T lport, UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetDesignatedRoot
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated root for specified instance.
 * INPUT    :   mstid                    -- instance value
 * OUTPUT   :   *designated_root         -- pointer of the specified
 *                                          designated_root
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetDesignatedRoot(UI32_T mstid,
                                  XSTP_MGR_BridgeIdComponent_T *designated_root);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetBridgeIdComponent
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge_id_component for specified instance.
 * INPUT    :   mstid                    -- instance value
 * OUTPUT   :   *designated_root         -- pointer of the specified
 *                                          bridge_id_component
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetBridgeIdComponent(UI32_T mstid,
                                     XSTP_MGR_BridgeIdComponent_T *bridge_id_component);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortDesignatedRoot
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated root for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              mstid                           -- instance value
 * OUTPUT   :   *designated_root                -- pointer of the specified
 *                                                 designated_root
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortDesignatedRoot(UI32_T lport,
                                      UI32_T mstid,
                                      XSTP_MGR_BridgeIdComponent_T *designated_root);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortDesignatedBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated bridge for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *designated_bridge              -- pointer of the specified
 *                                                 port designated_bridge
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortDesignatedBridge(UI32_T lport,
                                        UI32_T mstid,
                                        XSTP_MGR_BridgeIdComponent_T *designated_bridge);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortDesignatedPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated port for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *designated_port                -- pointer of the specified
 *                                                 designated_port
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortDesignatedPort(UI32_T lport,
                                      UI32_T mstid,
                                      XSTP_MGR_PortIdComponent_T *designated_port);

/* per_port spanning tree : end */

BOOL_T  XSTP_OM_GetMappedInstanceByVlan(UI32_T vid, UI32_T *mstid);

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get root guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- status value.
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetPortRootGuardStatus(UI32_T lport, UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get running root guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- status value.
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :   None.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortRootGuardStatus(UI32_T lport, UI32_T *status);
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortBpduGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU guard status on the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPortBpduGuardStatus(UI32_T lport, UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortBpduGuardStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running BPDU Guard status on the specified port.
 * INPUT    : lport     -- the logical port number
 * OUTPUT   : status    -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : (interface function)
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortBpduGuardStatus(UI32_T lport, UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortBPDUGuardAutoRecovery
 * ------------------------------------------------------------------------
 * PURPOSE : Get the BPDU guard auto recovery status on the specified port.
 * INPUT   : lport -- lport number
 * OUTPUT  : status -- the status value
 * RETURN  : XSTP_TYPE_RETURN_CODE_E
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPortBPDUGuardAutoRecovery(UI32_T lport, UI32_T  *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortBPDUGuardAutoRecovery
 *-------------------------------------------------------------------------
 * PURPOSE : Get running BPDU guard auto recovery status for the specified
 *           port.
 * INPUT   : lport -- the logical port number
 * OUTPUT  : status -- the status value
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortBPDUGuardAutoRecovery(UI32_T lport,
                                                   UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortBPDUGuardAutoRecoveryInterval
 * ------------------------------------------------------------------------
 * PURPOSE : Get the BPDU guard auto recovery interval on the specified
 *           port.
 * INPUT   : lport -- lport number
 * OUTPUT  : interval -- the interval value
 * RETURN  : XSTP_TYPE_RETURN_CODE_E
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortBPDUGuardAutoRecoveryInterval(UI32_T lport,
                                                    UI32_T *interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortBPDUGuardAutoRecoveryInterval
 *-------------------------------------------------------------------------
 * PURPOSE : Get running BPDU guard auto recovery interval for the specified
 *           port.
 * INPUT   : lport -- the logical port number
 * OUTPUT  : interval -- the status value
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortBPDUGuardAutoRecoveryInterval(UI32_T lport,
                                                           UI32_T *interval);
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortBpduFilterStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU filter status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPortBpduFilterStatus(UI32_T lport, UI32_T *status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortBpduFilterStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get per port variable bpdu_filter_status value.
 * INPUT    : lport     -- the logical port number
 * OUTPUT   : status    -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : (interface function)
 *------------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortBpduFilterStatus(UI32_T lport, UI32_T *status);
#endif /* #if (SYS_CPNT_STP_BPDU_FILTER == TRUE) */

#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetCiscoPrestandardCompatibility
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the cisco prestandard compatibility status
 * INPUT    :   UI32_T status           -- the status value
 * OUTPUT   :   UI32_T status           -- the status value
 * RETURN   :   None
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_GetCiscoPrestandardCompatibility(UI32_T *status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningCiscoPrestandardCompatibility
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the cisco prestandard compatibility status.
 * INPUT    : UI32_T status           -- the status value
 * OUTPUT   : UI32_T status           -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    :
 *------------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningCiscoPrestandardCompatibility(UI32_T *status);

#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */

#if (SYS_CPNT_EAPS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetEthRingPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   To get the port role for eth ring protocol.
 * INPUT    :   lport       -- lport number (1-based)
 * OUTPUT   :   port_role_p -- pointer to content of port role
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetEthRingPortRole(
    UI32_T  lport,
    UI32_T  *port_role_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetEthRingPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   To set the port role for eth ring protocol.
 * INPUT    :   lport     -- lport number (1-based)
 *              port_role -- port role to set
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_SetEthRingPortRole(
    UI32_T  lport,
    UI32_T  port_role);

#endif /* End of #if (SYS_CPNT_EAPS == TRUE) */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetEthRingPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : To get the ring port status.
 * INPUT    : lport    -- lport number to check
 *            vid      -- VLAN id to check
 * OUTPUT   : is_blk_p -- pointer to content of blocking status
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetEthRingPortStatus(
    UI32_T  lport,
    UI32_T  vid,
    BOOL_T  *is_blk_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the port tc prop status
 * INPUT    : logical port number
 *            enable_status
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void XSTP_OM_SetPortTcPropStop(UI32_T lport, BOOL_T enable_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  : Is port in tc prop stop
 * INPUT    : logical port number
 *            enable_status
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsPortTcPropStop(UI32_T lport);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  : Get port tc prop stop running status
 * INPUT    : logical port number
 *            enable_status
 * OUTPUT   : None
 * OUTPUT   : None
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS - need to store configuration
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - configure no change
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortTcPropStop(UI32_T lport, UI32_T *enable_status_p);

#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_AddTcPropGroupPortList
 * ------------------------------------------------------------------------
 * PURPOSE  : Add ports to a TC propagation control group.
 * INPUT    : group_id     -- group ID
 *            portbitmap   -- group member ports
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_AddTcPropGroupPortList(UI32_T group_id,
                                UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_DelTcPropGroupPortList
 * ------------------------------------------------------------------------
 * PURPOSE  : Remove ports from a TC propagation control group.
 * INPUT    : group_id     -- group ID
 *            portbitmap   -- group member ports
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_DelTcPropGroupPortList(UI32_T group_id,
                                UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTcPropGroupPortbitmap
 * ------------------------------------------------------------------------
 * PURPOSE  : Get port list for specified group ID.
 * INPUT    : group_id     -- group ID
 * OUTPUT   : portbitmap   -- group member ports
 *            has_port_p   -- have port or not
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetTcPropGroupPortbitmap(UI32_T group_id,
                                UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                BOOL_T *has_port_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTcProplNextGroupPortbitmap
 * ------------------------------------------------------------------------
 * PURPOSE  : Get next group ID and port list.
 * INPUT    : group_id_p   -- group ID pointer
 * OUTPUT   : group_id_p   -- group ID pointer
 *            portbitmap   -- group member ports
 *            has_port_p   -- have port or not
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetTcPropNextGroupPortbitmap(UI32_T *group_id_p,
                                     UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                     BOOL_T *has_port_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPropGropIdByPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Get group ID for specified port.
 * INPUT    : lport   -- logical port number
 * OUTPUT   : None
 * RETURN   : group ID
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPropGropIdByPort(UI32_T lport);

#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/

BOOL_T XSTP_OM_GetPortTcInfo(UI32_T xstid, UI32_T lport, BOOL_T *is_mstp_mode,
    BOOL_T *is_root, UI32_T *tc_timer);

UI32_T XSTP_OM_GetMstidByEntryId(UI32_T entry_id);

#endif /* XSTP_OM_H */

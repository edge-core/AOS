/*-------------------------------------------------------------------------
 * Module Name: rspan_om.h
 *-------------------------------------------------------------------------
 * PURPOSE: Definitions for the RSPAN
 *-------------------------------------------------------------------------
 * NOTES:
 *
 *-------------------------------------------------------------------------
 * HISTORY:
 *    07/23/2007 - Tien Kuo, Created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-------------------------------------------------------------------------
 */
#ifndef _RSPAN_OM_H
#define _RSPAN_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_type.h"
#include "l_cvrt.h"
#include "leaf_es3626a.h"
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define RSPAN_OM_IS_MEMBER     L_CVRT_IS_MEMBER_OF_PORTLIST
#define RSPAN_OM_ADD_MEMBER    L_CVRT_ADD_MEMBER_TO_PORTLIST
#define RSPAN_OM_DEL_MEMBER    L_CVRT_DEL_MEMBER_FROM_PORTLIST
#define RSPAN_BACK_DOOR             TRUE

/* DATA TYPE DECLARATIONS
 */
/*-------------------------------------------------------------------------
 * The following structures apply to RSPAN
 *-------------------------------------------------------------------------
 */
typedef struct
{

    UI8_T   session_id;           /* The session number for RSPAN (1,2) or Local Port Monitor (3). */
    UI8_T   src_tx[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    				              /* RSPAN or Local Port Monitor source ports in tx mode. */
    UI8_T   src_rx[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    			                  /* RSPAN or Local Port Monitor source ports in rx mode. */
    UI8_T   dst;                  /* RSPAN or Local Port Monitor destination port. */
    UI8_T   dst_in;               /* For intermediate case, special record dst port. */
    UI8_T   is_tagged;            /* For RSPAN destination mode. */
    UI8_T   switch_role;          /* The role of the switch - source or intermediate or destination. */
    UI8_T   uplink[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]; /* It's also snmp port list pointer. */
    UI32_T  remote_vid;
    UI8_T   snmpEntryStatus ;     /* For snmp rspan entry. 2007.11.26. */

} RSPAN_OM_SessionEntry_T;

typedef struct
{

    UI8_T is_rspan_vid ;

} RSPAN_OM_RSPAN_VLAN_T;
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for BUFFERMGMT in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void RSPAN_OM_AttachSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - RSPAN_OM_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the RSPAN size for share memory
 * INPUT    : *segid_p
 *            *seglen_p
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void RSPAN_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_InitDataBase
 *-------------------------------------------------------------------------
 * PURPOSE : Initialize RSPAN OM database
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void RSPAN_OM_InitDataBase(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void RSPAN_OM_EnterMasterMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void RSPAN_OM_EnterSlaveMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Set transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void RSPAN_OM_SetTransitionMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void RSPAN_OM_EnterTransitionMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_GetOperatingMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Get operating mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
UI32_T RSPAN_OM_GetOperatingMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_SetRspanVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set RSPAN VLAN entry in RSPAN database.
 * INPUT    : vid         -- The new created vlan id
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
void RSPAN_OM_SetRspanVlanEntry(UI32_T vid);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_DeleteRspanVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will delete RSPAN VLAN entry in RSPAN database.
 * INPUT    : vid         -- The specific RSPAN vlan id
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
void RSPAN_OM_DeleteRspanVlanEntry(UI32_T vid);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsRspanVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will find the specific RSPAN VLAN entry from RSPAN database.
 * INPUT    : vid         -- The specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE        -- RSPAN VLAN is found
 *            FALSE       -- RSPAN VLAN isn't found
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsRspanVlan(UI32_T vid);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_SetRspanSessionEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set RSPAN session entry in RSPAN_OM.
 * INPUT    : The whole data structure to store the value
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_SetRspanSessionEntry( RSPAN_OM_SessionEntry_T *rspan_entry );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_DeleteRspanSessionEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete RSPAN session entry in RSPAN_OM.
 * INPUT    : The whole data structure to delete the value
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is deleted successfully.
 *            FALSE       -- The configuration isn't deleted successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_DeleteRspanSessionEntry( RSPAN_OM_SessionEntry_T *rspan_entry );

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_GetRspanSessionEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the RSPAN entry of the specific session from RSPAN database.
 * INPUT    : session_id  -- The specific session id.
 *            *rspan_entry -- The RSPAN entry pointer.
 * OUTPUT   : *rspan_entry -- The whole data structure with the specific entry.
 * RETURN   : TRUE         -- The configuration is set successfully.
 *            FALSE        -- The configuration isn't set successfully.
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_GetRspanSessionEntry(UI8_T session_id, RSPAN_OM_SessionEntry_T *rspan_entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_SettingValidation
 *--------------------------------------------------------------------------
 * PURPOSE  : Check if items of a session entry are valid to set the database.
 * INPUT    : The pointer of the session structure.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The entry is valid for setting the database.
 *            FALSE       -- The entry is not valid.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_SettingValidation ( RSPAN_OM_SessionEntry_T *is_rspan_entry_valid , UI8_T target_port) ;

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsSessionEntryCompleted
 *--------------------------------------------------------------------------
 * PURPOSE  : Check if items of a session entry are ready to set the chip.
 * INPUT    : The pointer of the session structure.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The entry is ready for setting the chip.
 *            FALSE       -- The entry is not completed.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsSessionEntryCompleted( RSPAN_OM_SessionEntry_T *rspan_entry );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_RemoveSessionId
 *--------------------------------------------------------------------------
 * PURPOSE  : Remove a session id when all items of this entry are empty.
 * INPUT    : session_id : The specific session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The session id is removed.
 *            FALSE       -- The session id is not removed  or not found.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_RemoveSessionId( UI8_T session_id );

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_GetSessionEntryCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the number of RSPAN entries from RSPAN database.
 * INPUT    : *session_cnt -- The total session numbers.
 * OUTPUT   : *session_cnt -- The total session numbers.
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void RSPAN_OM_GetSessionEntryCounter(UI8_T *session_cnt) ;

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IncreaseSrcUplinkCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : Increase source uplink counters.
 * INPUT    : session_id : The specific session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- Increase source uplink couners successfully.
 *            FALSE       -- Doesn't increase source uplink couners successfully.
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IncreaseSrcUplinkCounter (UI8_T session_id) ;

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_DecreaseSrcUplinkCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : Decrease source uplink counters.
 * INPUT    : session_id : The specific session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- Decrease source uplink couners successfully.
 *            FALSE       -- Doesn't decrease source uplink couners successfully.
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_DecreaseSrcUplinkCounter (UI8_T session_id);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_RemoveSessionIdEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Remove a session id and all items of this session id.
 * INPUT    : session_id : The specific session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The session id and entry infor. are removed.
 *            FALSE       -- The session id and entry infor. are not removed
 *                           because session id is not found.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_RemoveSessionIdEntry( UI8_T session_id );

/* 802.1X and port security will need this. 2007/09/20 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsRspanUplinkPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN mirrored port
 *           FALSE: The ifindex is not a RSPAN mirrored port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsRspanUplinkPort(UI32_T ifindex) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_SetRspanUplinkPortUsage
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the status of the usage of uplink ports
 *           by session.
 * INPUT   : session id
 *           ret        -- the current status needs to set in OM.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This data is used for AMTR to check if it needs to enable/disable
 *           port learning of RSPAN uplink ports.
 * -------------------------------------------------------------------------
 */
void RSPAN_OM_SetRspanUplinkPortUsage ( UI8_T session_id, BOOL_T ret ) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsRspanUplinkPortUsed
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN mirrored port and ready to perform
 *                  RSPAN.
 *           FALSE: The ifindex is not a RSPAN mirrored port or not ready to
 *                  perform RSPAN.
 * NOTE    : This API is used for AMTR to check if it needs to enable/disable
 *           port learning of RSPAN uplink ports.
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsRspanUplinkPortUsed ( UI32_T ifindex ) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_ValidateRspanPortRelation
 * -------------------------------------------------------------------------
 * FUNCTION: This function tests if the port of this port list isn't used by
 *           the other session or other field.
 * INPUT   : session_id     -- RSPAN session ID
 *           targetPort     -- the port which user configures for rspan field
 *           port_list_type -- LEAF_rspanSrcTxPorts  2
 *                             LEAF_rspanSrcRxPorts  3
 *                             LEAF_rspanRemotePorts 7
 * OUTPUT  : None
 * RETURN  : TRUE : The port is valid to set in this session.
 *           FALSE: The port isn't valid to set in this session.
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_ValidateRspanPortRelation ( UI8_T session_id, UI8_T targetPort , UI8_T port_list_type ) ;

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsLocalMirrorPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to test whether the ufindex is a mirrored
 *            source or destination port.
 * INPUT    : ifindex -- this interface index
 * OUTPUT   : None
 * RETURN   : TRUE : The ifindex is a mirrored source or destination port
 *            FALSE: The ifindex is a mirrored source or destination port
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsLocalMirrorPort( UI32_T ifindex );

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_ValidateRspanPortRelation
 * -------------------------------------------------------------------------
 * FUNCTION: This function tests if the port of this port list isn't used by
 *           the other session or other field.
 * INPUT   : session_id     -- RSPAN session ID
 *           ifindex     -- the port which user configures for rspan field
 *           port_list_type -- LEAF_rspanSrcTxPorts  2
 *                             LEAF_rspanSrcRxPorts  3
 *                             LEAF_rspanRemotePorts 7
 * OUTPUT  : None
 * RETURN  : TRUE : The port is valid to set in this session.
 *           FALSE: The port isn't valid to set in this session.
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_ValidateRspanPortRelation ( UI8_T session_id, UI8_T ifindex , UI8_T port_list_type ) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsSwitchRoleValid
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether if the switch role is vaild to set
 *           in the OM from SNMP.
 * INPUT   : session id
 *           switch_role -- VAL_rspanSwitchRole_source       1L
 *                          VAL_rspanSwitchRole_intermediate 2L
 *                          VAL_rspanSwitchRole_destination  3L
 * OUTPUT  : None
 * RETURN  : TRUE        -- The configuration is set successfully.
 *           FALSE       -- The configuration isn't set successfully.
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsSwitchRoleValid ( UI8_T session_id, UI8_T switch_role ) ;

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsRspanMirrorToPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN
 *           destination port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN destination port
 *           FALSE: The ifindex is not a RSPAN destination port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsRspanMirrorToPort( UI32_T ifindex ) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsRspanMirroredPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN mirrored port
 *           FALSE: The ifindex is not a RSPAN mirrored port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsRspanMirroredPort( UI32_T ifindex ) ;

#endif /*#if (SYS_CPNT_SYSCTRL_XOR == TRUE)*/

#if(RSPAN_BACK_DOOR == TRUE)
/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_GetLocalSessionEntryForBackdoor
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the RSPAN entry of the specific session from RSPAN database.
 * INPUT    : *rspan_entry -- The Local entry pointer.
 * OUTPUT   : *rspan_entry -- The whole data structure with the specific entry.
 * RETURN   : TRUE         -- The configuration is set successfully.
 *            FALSE        -- The configuration isn't set successfully.
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_GetLocalSessionEntryForBackdoor ( RSPAN_OM_SessionEntry_T *rspan_entry) ;
#endif /*#if(RSPAN_BACK_DOOR == TRUE) */

#endif  /* _RSPAN_OM_H */

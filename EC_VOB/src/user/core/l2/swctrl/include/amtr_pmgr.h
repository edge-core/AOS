/*-----------------------------------------------------------------------------
 * FILE NAME: AMTR_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for AMTR MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/14     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef AMTR_PMGR_H
#define AMTR_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "amtr_mgr.h"
#include "amtr_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : AMTR_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for AMTR_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_InitiateProcessResource(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetRunningAgingTime
 *-----------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          non-default ageing time can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None
 * OUTPUT:  *aging_time   - the non-default ageing time
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default ageing time.
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetRunningAgingTime(UI32_T *aging_time);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_SetAddrEntryForMlag
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will set address table entry
 * INPUT :   addr_entry -> vid - VLAN ID
 *           addr_entry -> mac - MAC address
 *           addr_entry -> ifindex - interface index
 *           addr_entry -> action -Forwarding/Discard by SA match/Discard by DA match/Trap to CPU only/Forwarding and trap to CPU
 *           addr_entry -> source -Config/Learn/Internal/Self
 *           addr_entry -> life_time -permanent/Other/Delete on Reset/Delete on Timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T status           - Success or not
 * NOTE    : 1. parameters:
 *              action:     AMTRDRV_OM_ADDRESS_ACTION_FORWARDING
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYSAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYDAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_TRAPTOCPUONLY
 *                          AMTRDRV_OM_ADDRESS_ACTION_FORWARDINGANDTRAPTOCPU
 *
 *              source:     AMTRDRV_OM_ADDRESS_SOURCE_LEARN
 *                          AMTRDRV_OM_ADDRESS_SOURCE_CONFIG
 *                          AMTRDRV_OM_ADDRESS_SOURCE_INTERNAL
 *                          AMTRDRV_OM_ADDRESS_SOURCE_SELF
 *
 *              life_time:  AMTRDRV_OM_ADDRESS_LIFETIME_OTHER
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_INVALID
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_PERMANENT
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONRESET
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONTIMEOUT
 *           2.Set CPU mac don't care the ifinedx, but can't be "0".
 *             In AMTR, ifindex ==0 means just set to OM, don't program chip.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_SetAddrEntryForMlag(AMTR_TYPE_AddrEntry_T *addr_entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_SetAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will set address table entry
 * INPUT :   addr_entry -> vid		 - VLAN ID
 *           addr_entry -> mac		 - MAC address
 *           addr_entry -> ifindex	 - interface index
 *           addr_entry -> action	 -Forwarding/Discard by SA match/Discard by DA match/Trap to CPU only/Forwarding and trap to CPU
 *		     addr_entry -> source	 -Config/Learn/Internal/Self
 *		     addr_entry -> life_time -permanent/Other/Delete on Reset/Delete on Timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T status           - Success or not
 * NOTE    : 1. parameters:
 *              action:     AMTRDRV_OM_ADDRESS_ACTION_FORWARDING
 *   				        AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYSAMATCH
 *             		        AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYDAMATCH
 *               	        AMTRDRV_OM_ADDRESS_ACTION_TRAPTOCPUONLY
 *          		        AMTRDRV_OM_ADDRESS_ACTION_FORWARDINGANDTRAPTOCPU
 *
 *		        source:     AMTRDRV_OM_ADDRESS_SOURCE_LEARN
 *				            AMTRDRV_OM_ADDRESS_SOURCE_CONFIG
 *				            AMTRDRV_OM_ADDRESS_SOURCE_INTERNAL
 *				            AMTRDRV_OM_ADDRESS_SOURCE_SELF
 *
 *		        life_time:  AMTRDRV_OM_ADDRESS_LIFETIME_OTHER
 *			   	            AMTRDRV_OM_ADDRESS_LIFETIME_INVALID
 *				            AMTRDRV_OM_ADDRESS_LIFETIME_PERMANENT
 *				            AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONRESET
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONTIMEOUT
 *	         2.Set CPU mac don't care the ifinedx, but can't be "0".
 *	         In AMTR, ifindex ==0 means just set to OM, don't program chip.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_SetAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddr
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will clear specific address table entry
 * INPUT   : UI16_T  vid   - vlan ID
 *           UI8_T  *Mac   - Specified MAC to be cleared
 *
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE:     This API can't delete CPU MAC!
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddr(UI32_T vid, UI8_T *mac);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrForMlag
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will clear specific address table entry
 * INPUT   : UI16_T  vid   - vlan ID
 *           UI8_T  *Mac   - Specified MAC to be cleared
 *
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE:     This API can't delete CPU MAC!
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrForMlag(UI32_T vid, UI8_T *mac);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByLifeTime
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete entries by life time
 * INPUT   : AMTR_TYPE_AddressLifeTime_T life_time
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByLifeTime(AMTR_TYPE_AddressLifeTime_T life_time);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByLifeTimeAndLPort
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time and Port.
 * INPUT   : UI32_T ifindex - interface index
 *			 AMTR_MGR_LiftTimeMode_T life_time - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByLifeTimeAndLPort(UI32_T ifindex, AMTR_TYPE_AddressLifeTime_T life_time);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrBySourceAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by specific source and Lport.
 * INPUT   : UI32_T ifindex               - interface index
 *		     AMTR_MGR_SourceMode_T source - learnt, config, intrenal, self
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_DeleteAddrBySourceAndLPort(UI32_T ifindex, AMTR_TYPE_AddressSource_T source);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port.
 * INPUT   : UI32_T ifindex                     - interface index
 *           UI32_T vid                         - vlan id
 *           AMTR_MGR_LiftTimeMode_T life_time  - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status                      - True : successs, False : failed
 * NOTE    : When an instance has too many vlan trigger tc,
 *           it will generate many events. Here changes to asynchronize call
 *           and use start_vid and end_vid
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port
 *           synchronously.
 * INPUT   : UI32_T ifindex - interface index
 *           UI16_T start_vid - the starting vid
 *           UI16_T end_vid   - the end vid
 *           UI32_T vid     - vlan id
 *           AMTR_MGR_LiftTimeMode_T life_time - delete on timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : 1. This function will not return until the delete operation is done
 *           2. Only support life_time as AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByLifeTimeAndMstIDAndLPort
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port.
 * INPUT   : UI32_T ifindex                     - interface index
 *           UI32_T vid                         - vlan id
 *			 AMTR_MGR_LiftTimeMode_T life_time  - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status                      - True : successs, False : failed
 * NOTE    : when a instance has too many vlan trigger tc, 
 *           it will generate may enven. Here change to asynchronize call
 *           and use start_vid and end_vid
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByLifeTimeAndMstIDAndLPort(UI32_T ifindex, UI32_T mst_id, AMTR_TYPE_AddressLifeTime_T life_time);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetExactAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get exact address table entry (vid+mac)
 * INPUT   : addr_entry->vid    - VLAN ID     (key)
 *           addr_entry->mac    - MAC address (key)
 * OUTPUT  : addr_entry         - address entry info
 * RETURN  : TRUE  if a valid address exists and retrieved from macList
 *           FALSE if no address can be found
 * NOTE    : 1. using key generated from (vlanID,mac address)
 *		     2. search the entry from Hash table(driver layer)
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetExactAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetExactAddrEntryFromChip                                
 *------------------------------------------------------------------------
 * FUNCTION: This function will get exact address table entry (mac+vid)            
 * INPUT   : addr_entry->mac - mac address
 *           addr_entry->vid - vlan id
 * OUTPUT  : addr_entry      - addresss entry info
 * RETURN  : TRUE  if a valid address exists and retrieved from macList   
 *           FALSE if no address can be found                             
 * NOTE    : 1.This API is only support IML_MGR to get exact mac address from chip 
 *             We don't support MIB to get under_create entry since l_port =0 
 *             will return false
 *           2.This API only support MV key or VM key not IVM key                                                       
 *------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_GetExactAddrEntryFromChip(AMTR_TYPE_AddrEntry_T *addr_entry);


/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextMVAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get next address table entry (mac+vid)
 * INPUT   : addr_entry->mac    - MAC address (primary key)
 *           addr_entry->vid    - VLAN ID     (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if mac[0]~mac[5] == \0 => get the first entry
 *		     2. search the entry from Hisam table(core layer)
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextMVAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextVMAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get next address table entry (vid+mac)
 * INPUT   : addr_entry->vid    - VLAN ID        (primary key)
 *           addr_entry->mac    - MAC address    (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if vid == 0 => get the first entry
 *              2. using key generated from (vlanID,mac address)
 *		    3. search the entry from Hisam table(core layer)
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextVMAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextIfIndexAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get next address table entry (ifindex+vid+mac)
 * INPUT   : addr_entry->l_port - interface index   (primary key)
 *           addr_entry->vid    - vlan id           (key)
 *           addr_entry->mac    - MAC address       (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. l_port is a physical port or trunk port
 *		     2. search the entry from Hisam table(core layer)
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextIfIndexAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextRunningStaticAddrEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           static address can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL is returned.
 * INPUT   : addr_entry->mac    - MAC address (primary key)
 *           addr_entry->vid    - VLAN ID     (key)
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if mac[0]~mac[5] == \0 => get the first entry
 *           2. the function shall be only invoked by cli
 *		     3. search the entry from Hash table
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetNextRunningStaticAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry);

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetInterventionEntry
 *-----------------------------------------------------------------------------
 * Purpose  : This routine will add a CPU MAC to the chip.  Packets with this
 *            MAC in the specified VLAN will trap to CPU.
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address
 * OUTPUT   : None
 * RETURN   : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTE     : This API doesn't forbid that CPU has only one MAC.  It will only
 *            depend on spec.
 *-----------------------------------------------------------------------------
 */
AMTR_TYPE_Ret_T  AMTR_PMGR_SetInterventionEntry(UI32_T vid, UI8_T *mac);

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_DeleteInterventionEntry
 *-----------------------------------------------------------------------------
 * Purpose  : This function will delete a intervention mac address from
 *            address table
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_DeleteInterventionEntry(UI32_T vid, UI8_T *mac);

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_CreateMulticastAddrTblEntry
 *-----------------------------------------------------------------------------
 * Purpose  : This function will create a multicast address table entry
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - multicast mac address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_CreateMulticastAddrTblEntry(UI32_T vid, UI8_T *mac);

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_DestroyMulticastAddrTblEntry
 *-----------------------------------------------------------------------------
 * Purpose  : This function will destroy a multicast address table entry
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - multicast mac address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_DestroyMulticastAddrTblEntry(UI32_T vid, UI8_T* mac);

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetMulticastPortMember
 *-----------------------------------------------------------------------------
 * Purpose  : This function sets the port member(s) of a given multicast address
 * INPUT    : UI32_T vid
 *            UI8_T *mac                                                                                - multicast MAC address
 *            UI8_T ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]   - the member ports of the MAC
 *            UI8_T tagged[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] - tagged/untagged member
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetMulticastPortMember(UI32_T vid,
                                         UI8_T *mac,
                                         UI8_T ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                         UI8_T tagged[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_GetAgingStatus
 *-----------------------------------------------------------------------------
 * Purpose  : This function Get the Address Ageing Status
 * INPUT    : None
 * OUTPUT   : UI32_T *status - VAL_amtrMacAddrAgingStatus_enabled
 *                             VAL_amtrMacAddrAgingStatus_disabled
 * RETURN   : BOOL_T         - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_GetAgingStatus(UI32_T *status);

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_GetRunningAgingStatus
 *-----------------------------------------------------------------------------
 * Purpose  : This function Get the Address Ageing Status
 * INPUT    : None
 * OUTPUT   : UI32_T *status -  VAL_amtrMacAddrAgingStatus_enabled
 *                              VAL_amtrMacAddrAgingStatus_disabled
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T -
 *                              1. SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *                              2. SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *                              3. SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T  AMTR_PMGR_GetRunningAgingStatus(UI32_T *status);

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetAgingStatus
 *-----------------------------------------------------------------------------
 * Purpose  : This function Get the Address Ageing Status
 * INPUT    : None
 * OUTPUT   : UI32_T status  - VAL_amtrMacAddrAgingStatus_enabled
 *                             VAL_amtrMacAddrAgingStatus_disabled
 * RETURN   : BOOL_T         - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetAgingStatus(UI32_T status);

/*---------------------------------------------------------------------- */
/* The dot1dTp group (dot1dBridge 4 ) -- dot1dTp 2 & dot1dTp 3 */
/*
 *         dot1dTpAgingTime
 *             SYNTAX   INTEGER (10..1000000)
 *             ::= { dot1dTp 2 }
 *----------------------------------------------------------------------
 *         dot1dTpFdbTable OBJECT-TYPE
 *             SYNTAX  SEQUENCE OF Dot1dTpFdbEntry
 *             ::= { dot1dTp 3 }
 *         Dot1dTpFdbEntry
 *             INDEX   { dot1dTpFdbAddress }
 *             ::= { dot1dTpFdbTable 1 }
 *         Dot1dTpFdbEntry ::=
 *             SEQUENCE {
 *                 dot1dTpFdbAddress    MacAddress,
 *                 dot1dTpFdbPort       INTEGER,
 *                 dot1dTpFdbStatus     INTEGER
 *             }
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetDot1dTpAgingTime
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified aging time
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   aging_time                  -- aging time
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTp 2
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1dTpAgingTime(UI32_T *aging_time);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1dTpAgingTime
 *-----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified aging time
 *              can be successfully set. Otherwise, false is returned.
 * INPUT    :   aging_time                  -- aging time
 * OUTPUT   :   NOne
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. RFC1493/dot1dTp 2
 *              2. aging time is in [10..1000000] seconds
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_SetDot1dTpAgingTime(UI32_T aging_time);

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetHashLookupDepthFromConfig
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified hash lookup depth
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   lookup_depth_p   -- hash lookup depth
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetHashLookupDepthFromConfig(UI32_T *lookup_depth_p);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetHashLookupDepthFromChip
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified hash lookup depth
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   lookup_depth_p   -- hash lookup depth
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetHashLookupDepthFromChip(UI32_T *lookup_depth_p);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetHashLookupDepth
 *-----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified max hash lookup length
 *              can be successfully set. Otherwise, false is returned.
 * INPUT    :   lookup_depth     -- hash lookup depth
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_SetHashLookupDepth(UI32_T lookup_depth);
#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetDot1dTpFdbEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified forwarding database
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   tp_fdb_entry -- Forwarding Database for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpFdbTable 1
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1dTpFdbEntry(AMTR_MGR_Dot1dTpFdbEntry_T *tp_fdb_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetNextDot1dTpFdbEntry
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified forwarding database
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   tp_fdb_entry -- Forwarding Database for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpFdbTable 1
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextDot1dTpFdbEntry(AMTR_MGR_Dot1dTpFdbEntry_T *tp_fdb_entry);

/*---------------------------------------------------------------------- */
/* The Static (Destination-Address Filtering) Database (dot1dBridge 5) */
/*
 *         Dot1dStaticEntry
 *             INDEX   { dot1dStaticAddress, dot1dStaticReceivePort }
 *             ::= { dot1dStaticTable 1 }
 *
 *         Dot1dStaticEntry ::=
 *             SEQUENCE {
 *                 dot1dStaticAddress           MacAddress,
 *                 dot1dStaticReceivePort       INTEGER,
 *                 dot1dStaticAllowedToGoTo     OCTET STRING,
 *                 dot1dStaticStatus            INTEGER
 *             }
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetDot1dStaticEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   static_entry->dot1d_static_address
 * OUTPUT   :   static_entry                  -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dStaticTable 1
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1dStaticEntry(AMTR_MGR_Dot1dStaticEntry_T *static_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetNextDot1dStaticEntry
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   static_entry->dot1d_static_address
 * OUTPUT   :   static_entry                  -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dStaticTable 1
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextDot1dStaticEntry(AMTR_MGR_Dot1dStaticEntry_T *static_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1dStaticAddress
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set the static address information.
 * INPUT    :   UI8_T old_mac -- the original mac address (key)
 *              UI8_T new_mac -- the new mac to replace original mac
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 1
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1dStaticAddress(UI8_T *old_mac, UI8_T *new_mac);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1dStaticReceivePort
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set the static receive port information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI32_T receive_port     -- the receive port number
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 2
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1dStaticReceivePort(UI8_T *mac, UI32_T receive_port);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1dStaticAllowedToGoTo
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set the static allowed to go to information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI8_T allow_to_go_to    -- the set of ports
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 3
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1dStaticAllowedToGoTo(UI8_T *mac, UI8_T *allowed_to_go_to);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1dStaticStatus
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set/Create the static status information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI32_T status           -- VAL_dot1dStaticStatus_other
 *                                         VAL_dot1dStaticStatus_invalid
 *                                         VAL_dot1dStaticStatus_permanent
 *                                         VAL_dot1dStaticStatus_deleteOnReset
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 4
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1dStaticStatus(UI8_T *mac, UI32_T status);

/*---------------------------------------------------------------------- */
/* the current Filtering Database Table (the dot1qTp group 1) */
/*
 *       INDEX   { dot1qFdbId }
 *       Dot1qFdbEntry ::=
 *       SEQUENCE {
 *           dot1qFdbId             Unsigned32,
 *           dot1qFdbDynamicCount   Counter32
 *       }
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetDot1qFdbEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get the filtering database entry info
 * INPUT   : dot1q_fdb_entry->dot1q_fdb_id  - The identity of this Filtering Database
 * OUTPUT  : dot1q_fdb_entry                - The  Filtering Database entry
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 1
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1qFdbEntry(AMTR_MGR_Dot1qFdbEntry_T *dot1q_fdb_entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextDot1qFdbEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get the next filtering database entry info
 * INPUT   : dot1q_fdb_entry->dot1q_fdb_id  - The identity of this Filtering Database
 * OUTPUT  : dot1q_fdb_entry                - The  Filtering Database entry
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 1
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextDot1qFdbEntry(AMTR_MGR_Dot1qFdbEntry_T *dot1q_fdb_entry);

/*---------------------------------------------------------------------- */
/* (the dot1qTp group 2) */
/*
 *      INDEX   { dot1qFdbId, dot1qTpFdbAddress }
 *      Dot1qTpFdbEntry ::=
 *          SEQUENCE {
 *              dot1qTpFdbAddress  MacAddress,
 *              dot1qTpFdbPort     INTEGER,
 *              dot1qTpFdbStatus   INTEGER
 *          }
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetDot1qTpFdbEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get the dot1qTpFdbEntry info
 * INPUT   : dot1q_tp_fdb_entry->dot1q_fdb_id  - vlan id
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_address - mac address
 * OUTPUT  : dot1q_tp_fdb_entry                - The dot1qTpFdbEntry info
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 2
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_status == addr_entry->source
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1qTpFdbEntry(UI32_T dot1q_fdb_id, AMTR_MGR_Dot1qTpFdbEntry_T *dot1q_tp_fdb_entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetNextDot1qTpFdbEntry
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will get the next dot1qTpFdbEntry info
 * INPUT   : dot1q_tp_fdb_entry->dot1q_fdb_id  - vlan id
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_address - mac address
 * OUTPUT  : dot1q_tp_fdb_entry             - The dot1qTpFdbEntry info
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 2
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_status == addr_entry->source
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextDot1qTpFdbEntry(UI32_T *dot1q_fdb_id, AMTR_MGR_Dot1qTpFdbEntry_T *dot1q_tp_fdb_entry);

/*---------------------------------------------------------------------- */
/* The Static (Destination-Address Filtering) Database (dot1qStatic 1) */
/*
 *             INDEX   { dot1qFdbId, dot1qStaticUnicastAddress, dot1qStaticUnicastReceivePort }
 *             ::= { dot1qStaticUnicastTable 1 }
 *
 *         Dot1qStaticUnicastEntry ::=
 *             SEQUENCE {
 *                 dot1qStaticUnicastAddress           MacAddress,
 *                 dot1qStaticUnicastReceivePort       INTEGER,
 *                 dot1qStaticUnicastAllowedToGoTo     OCTET STRING,
 *                 dot1qStaticUnicastStatus            INTEGER
 *             }
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetDot1qStaticUnicastEntry
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   dot1q_fdb_id                -- vlan id
 *              static_unitcast_entry->dot1q_static_unicast_address
 * OUTPUT   :   static_unitcast_entry       -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674q/dot1qStaticUnicastTable 1
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetDot1qStaticUnicastEntry(UI32_T dot1q_fdb_id,
                                            AMTR_MGR_Dot1qStaticUnicastEntry_T *static_unitcast_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_GetNextDot1qStaticUnicastEntry
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   dot1q_fdb_id                -- vlan id
 *              static_unitcast_entry->dot1q_static_unicast_address
 * OUTPUT   :   static_unitcast_entry       -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674q/dot1qStaticUnicastTable 1
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetNextDot1qStaticUnicastEntry(UI32_T *dot1q_fdb_id,
                                                AMTR_MGR_Dot1qStaticUnicastEntry_T *static_unitcast_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1qStaticUnicastAllowedToGoTo
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set the static allowed to go to information.
 * INPUT    :   UI32_T vid              -- vlan id
 *              UI8_T mac               -- the mac address (key)
 *              UI8_T allow_to_go_to    -- the set of ports
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC2674q/dot1qStaticUnicastEntry 3
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1qStaticUnicastAllowedToGoTo(UI32_T vid, UI8_T *mac, UI8_T *allowed_to_go_to);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_PMGR_SetDot1qStaticUnicastStatus
 * ----------------------------------------------------------------------------
 * PURPOSE  :   Set/Create the static status information.
 * INPUT    :   UI32_T vid              -- vlan id
 *              UI8_T mac               -- the mac address (key)
 *              UI32_T status           -- VAL_dot1qStaticUnicastStatus_other
 *                                         VAL_dot1qStaticUnicastStatus_invalid
 *                                         VAL_dot1qStaticUnicastStatus_permanent
 *                                         VAL_dot1qStaticUnicastStatus_deleteOnReset
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC2674q/dot1qStaticUnicastEntry 4
 * ----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetDot1qStaticUnicastStatus(UI32_T vid, UI8_T *mac, UI32_T status);

/*-----------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetLearningMode
 *-----------------------------------------------------------------------------
 * Purpose  : This function will set the learning mode for whole system
 * INPUT    : UI32_T learning_mode    - VAL_dot1qConstraintType_independent (IVL)
 *                                      VAL_dot1qConstraintType_shared (SVL)
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : RFC2674q
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetLearningMode(UI32_T learning_mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_IsPortSecurityEnabled
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return Ture if the port security is enabled
 * INPUT   : ifindex        -- which port to get
 * OUTPUT  : none
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T  AMTR_PMGR_IsPortSecurityEnabled(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_Notify_IntrusionMac
 * -------------------------------------------------------------------------
 * FUNCTION: When detecting intrusion mac, AMTR will notify other CSCs by this function.
 * INPUT   : vid    -- which vlan id
 *           mac    -- mac address
 *           ifindex-- which port
 *           is_age -- learned / aged
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_Notify_IntrusionMac(UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_Notify_SecurityPortMove
 * -------------------------------------------------------------------------
 * FUNCTION: When port move, AMTR will notify other CSCs by this function.
 * INPUT   : ifindex          -- port whcih the mac is learnt now
 *           vid              -- which vlan id
 *           mac              -- mac address
 *           original_ifindex -- original port which the mac was learnt before
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_Notify_SecurityPortMove( UI32_T ifindex,
                                               UI32_T vid,
                                               UI8_T  *mac,
                                               UI32_T original_ifindex);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByVidAndLifeTime
 *-----------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time and vid.
 * INPUT   : UI32_T vid - vlan id
 *			 AMTR_MGR_LiftTimeMode_T life_time - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : when a instance has too many vlan trigger tc, 
 *           it will generate may enven. Here change to asynchronize call
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_DeleteAddrByVidAndLifeTime(UI32_T vid, AMTR_TYPE_AddressLifeTime_T life_time);

/*------------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetCpuMac
 *------------------------------------------------------------------------------
 * Purpose  : This routine will add a CPU MAC to the chip.  Packets with this
 *            MAC in the specified VLAN will trap to CPU.
 * INPUT    : vid        -- vlan ID
 *            mac        -- MAC address
 *            is_routing -- whether the routing feature is supported and enabled
 * OUTPUT  : None
 * RETURN  : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
AMTR_TYPE_Ret_T  AMTR_PMGR_SetCpuMac(UI32_T vid, UI8_T *mac, BOOL_T is_routing);

/*------------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetMacNotifyGlobalStatus
 *------------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap global status
 * INPUT   : None
 * OUTPUT  : is_enabled - global status to set
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetMacNotifyGlobalStatus(BOOL_T is_enabled);

/*------------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetMacNotifyInterval
 *------------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap interval
 * INPUT   : None
 * OUTPUT  : interval - interval to set (in seconds)
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetMacNotifyInterval(UI32_T  interval);

/*------------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap port status
 * INPUT   : ifidx      - lport ifindex
 * OUTPUT  : is_enabled - port status to set
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetRunningMacNotifyInterval
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default interval can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : None
 * OUTPUT  : interval_p   - the non-default interval
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default interval.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetRunningMacNotifyInterval(UI32_T  *interval_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetRunningMacNotifyGlobalStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default global status can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : None
 * OUTPUT  : is_enabled_p   - the non-default global status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default global status.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetRunningMacNotifyGlobalStatus(BOOL_T  *is_enabled_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_GetRunningMacNotifyPortStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default port status can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : ifidx          - lport ifindex
 * OUTPUT  : is_enabled_p   - the non-default port status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default port status.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetRunningMacNotifyPortStatus(
    UI32_T  ifidx,
    BOOL_T  *is_enabled_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteCpuMac
 *-----------------------------------------------------------------------------
 * FUNCTION: This routine will delete the CPU MAC of DUT 
 * INPUT   : vid   -- vlan id
 *           mac   -- MAC address
 * OUTPUT  : None
 * RETURN  : True : successs, False : failed
 * NOTE    : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_DeleteCpuMac(UI32_T vid, UI8_T *mac);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_PMGR_ClearCollisionVlanMacTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Remove all entries in the collision mac table.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_ClearCollisionVlanMacTable(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_PMGR_GetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get vlan learning status of specified vlan
 * INPUT    : vid
 * OUTPUT   : *learning_p
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_GetVlanLearningStatus(UI32_T vid, BOOL_T *learning_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_PMGR_GetRunningVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get running vlan learning status of specified vlan
 * INPUT    : vid
 * OUTPUT   : *learning_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_PMGR_GetRunningVlanLearningStatus(UI32_T vid, BOOL_T *learning_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_PMGR_SetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Enable/disable vlan learning of specified vlan
 * INPUT    : vid
 *            learning
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_PMGR_SetVlanLearningStatus(UI32_T vid, BOOL_T learning);

/*------------------------------------------------------------------------------
 * Function : AMTR_PMGR_SetMlagMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * FUNCTION: To set the MLAG mac notify port status
 * INPUT   : ifidx      - lport ifindex
 * OUTPUT  : is_enabled - port status to set
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  AMTR_PMGR_SetMlagMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_AuthenticatePacket
 * -------------------------------------------------------------------------
 * FUNCTION: This API will handle the packet by authenticated result
 * INPUT   : src_mac     --  source mac address
 *           vid         --  VLAN id
 *           lport       --  logical port ifindex
 *           auth_result --  authentication result
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *--------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_AuthenticatePacket(
    UI8_T *src_mac,
    UI32_T vid,
    UI32_T lport,
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T auth_result
);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_PMGR_DeleteAddrByVidAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by vid and Port.
 * INPUT   : UI32_T ifindex - interface index
 *           UI32_T vid - VLAN ID
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_PMGR_DeleteAddrByVidAndLPort(UI32_T lport, UI32_T vid);

#endif /* #ifndef AMTR_PMGR_H */

/*------------------------------------------------------------------------------
 * Module Name: DEV_AMTRDRV.H
 *------------------------------------------------------------------------------
 * Purpose    : This file is the driver of the address monitor,
 *              provides APIs to manipulate ARL table of specific device.
 * Notes      :
 *
 * Copyright(C)      Accton Corporation, 1999 - 2003
 *-----------------------------------------------------------------------------
 * Modification History:                                                    
 *   By              Date     Ver.   Modification Description               
 *   ------------  --------   -----  ------------------------------------------
 *   Ted           01/24/2002        Clarify the API                        
 *                 02/20/2002        Add constants for                      
 *                                   DEV_AMTRDRV_Register_NA_CallBack()     
 *                                                                          
 *   Jason Hsue    01/23/2003        Modify some return values and more APIs
 *   Jason Hsue    03/20/2003        1. Add two more APIs -- 
 *                                          DEV_AMTRDRV_DeleteAllDynamicAddr()
 *                                          DEV_AMTRDRV_DeleteAllAddr()
 *                                   2. Add 3 more constants DEV_AMTRDRV_ADDRESS_ACCESS_SUCCESS...
 *-----------------------------------------------------------------------------*/



#ifndef _DEV_AMTRDRV_H
#define _DEV_AMTRDRV_H

#include "swdrv_type.h"

#include <sys_type.h>

/*--------------------
 * EXPORTED CONSTANTS
 *-------------------*/

/*--------------------
 * EXPORTED ROUTINES
 *-------------------*/

/*  1-30-2002
*/
#define DEV_AMTRDRV_INTERV_MODE_SA          1
#define DEV_AMTRDRV_INTERV_MODE_DA          2
#define DEV_AMTRDRV_INTERV_MODE_EITHER      3

/*  NA message types (Per Prestera spec.)
*/
#define DEV_AMTRDRV_NA_TYPE_NA              1   /* new address */
#define DEV_AMTRDRV_NA_TYPE_SA              2   /* security address */
#define DEV_AMTRDRV_NA_TYPE_AA              3   /* aged address */

#define DEV_AMTRDRV_ADDRESS_ACCESS_SUCCESS          0
#define DEV_AMTRDRV_ADDRESS_ACCESS_END_OF_TABLE     1
#define DEV_AMTRDRV_ADDRESS_ACCESS_FAIL             2
#define DEV_AMTRDRV_ADDRESS_ACCESS_BREAK            3 /* too many non NA entries is read, so break */

#define DEV_AMTRDRV_FIRST_DEVICE 1
#define DEV_AMTRDRV_EXCLUDE_1ST_DEVICES 2
#define DEV_AMTRDRV_ALL_DEVICE 3 /*1|2=3*/

typedef enum
{
    DEV_AMTRDRV_SUCCESS       ,
    DEV_AMTRDRV_FAIL          ,        
    DEV_AMTRDRV_COLLISION     ,     
}DEV_AMTRDRV_Ret_T; 


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_Init()
 *------------------------------------------------------------------------------
 * Purpose  : Initialization amtr table
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : 1. For Hardware learn, it should prepare dma allocate buffer.
 *-----------------------------------------------------------------------------*/
void DEV_AMTRDRV_Init();

/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_SetAgeingTime
 *------------------------------------------------------------------------------
 * Purpose  : This function will set aging time of the whole system
 * INPUT    : UI32_T value  - Aging time to set
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - TRUE : success , FALSE : fialed
 * NOTES    : 1. aging time is in [10..1000000] seconds
 *            2. seting aging time to 0 disable the age timer
 *-----------------------------------------------------------------------------*/
BOOL_T DEV_AMTRDRV_SetAgeingTime(UI32_T value);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_ResetToFirstAddrTblEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will get the first address table entry from ARL
 * INPUT    : UI32_T  unit      - Unit number
 * OUTPUT   : None
 * RETURN   : UI32_T            - True - successs, False - failed
 *
 * NOTES    : 1. Return value should be UI32 or boolean.
 *	          2. Skip stacking port.
 *            3. For Intel, we don't need this API
 *
 * <01242002> 1. This API is used for directly access the L2 table from chip.
 *            2. For L2 synchronized architecture like Prestera,
 *               the input parameter "unit" can be ignored.
 *               For non-synchronized architecture like BCM Strata or XGS,
 *               If each unit consists of multiple chips, this reset function should
 *               point to the first entry of the first chip.
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_ResetToFirstAddrTblEntry(UI32_T unit);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_GetNextAddrTblEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will get the next address table entry from ARL
 * INPUT    : UI32_T *unit              - Unit number       <01242002>
 * OUTPUT   : UI32_T *vid               - VLAN ID
 *            UI8_T  *mac               - MAC address
 *            UI32_T *port              - Port number
 *            UI32_T *trunk_id          - Trunk ID
 *            UI32_T *is_trunk          - is trunk or not
 *            BOOL_T *is_static         - TRUE:static, FALSE:dynamic
 * RETURN   : UI32_T
 *                   0: DEV_AMTRDRV_ADDRESS_ACCESS_SUCESS
 *                   1: DEV_AMTRDRV_ADDRESS_ACCESS_END_OF_TABLE
 *                   2: DEV_AMTRDRV_ADDRESS_ACCESS_FAIL
 *                   3: DEV_AMTRDRV_ADDRESS_ACCESS_BREAK
 *
 * NOTES    :
 *
 * <01242002> 1. This API is used for directly access the L2 table from chip.
 *            2. This API implies that the driver has to keep an internal index
 *               to point to next available entry from L2 table.
 *            3. For L2 synchonized architecture like Prestera,
 *               the input parameter "unit" can be ignored;
 *               But the data type should be changed to allow output, "UI32_T *unit".
 *               For non-synchronized architecture like BCM Strata or XGS,
 *               If each unit consists of multiple chips,
 *               this function should automatically get entry from next chip if pointer
 *               reaches the end of the L2 table of current chip.
 *
 *-----------------------------------------------------------------------------*/
UI32_T  DEV_AMTRDRV_GetNextAddrTblEntry(SWDRV_TYPE_L2_Address_Info_T *addr_table_entry);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_GetNextNAddrTblEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will get the next N address table entry from ARL
 * INPUT    : UI32_T unit               - Unit number
 *            UI32_T get_number         - N
 * OUTPUT   : DEV_SWDRV_TYPE_L2_Address_Info_T *address_table
 *                                      - to store N entries
 *            UI32_T *processed_number  - how many has been got
 * RETURN   : UI32_T
 *                   0: DEV_AMTRDRV_L2_ADDRESS_ACCESS_SUCESS
 *                   1: DEV_AMTRDRV_L2_ADDRESS_ACCESS_END_OF_TABLE
 *                   2: DEV_AMTRDRV_L2_ADDRESS_ACCESS_FAIL
 *
 * NOTES    : 1. Called by amtr
 *
 * <01242002> Get number of "get_number" entries from chip L2 table.
 *
 *-----------------------------------------------------------------------------*/
UI32_T  DEV_AMTRDRV_GetNextNAddrTblEntry(SWDRV_TYPE_L2_Address_Info_T *address_table,
                                        UI32_T number_of_address,
                                        UI32_T *actual_number_of_address_return);

/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_GetNewAddr
 *------------------------------------------------------------------------------
 * Purpose  : This function will get amonut of new address from new address buf
 * INPUT    : void CallBackFun    - callback to amtr to maintain mac address
 *                                  table
 *            UI32_T limit_number - fixed number entry per get
 *            BOOL_T *hw_arl_sync_required  - hardware ARL synchronous required
 * OUTPUT   : hw_ARL_sync_requested
 * RETURN   : the updated accessing address number
 * NOTES    : 1. The function will get a address ( new learning / aging out) from DMA
 *               and pass the address via the CallBackFun().
 *            2. The function will block the system. Therefore, decided the limit
 *               number carefully.
 *            3. every time lower driver found we need to sync, it always tells and
 *               clear the flag after return.
 *            4. For Intel, hw_ARL_sync_requested will always return FLASE
 *
 * <01242002> 1. The original Prestera driver will keep its own sorted L2 table,
 *               but we'll do the L2 shaow maintenance by ourselves.
 *               This API will not be implemented by Prestera TAPI.
 *               The Prestera's internal L2 table may still exist for Galtis debug purpose.
 *            2. We will provide a CallBackFun to Prestera TAPI. See the next API.
 *
 *-----------------------------------------------------------------------------*/
UI32_T DEV_AMTRDRV_GetNewAddr(void CallBackFun (UI32_T vid,
    										    UI8_T  *mac,
    										    UI32_T unit,
    										    UI32_T port,
    										    UI32_T trunk_id,
    										    BOOL_T is_trunk,
    										    BOOL_T is_aged),
   						      UI32_T limit_number,
   						      BOOL_T *hw_arl_sync_required);

/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_Register_NA_CallBack
 *------------------------------------------------------------------------------
 * Purpose  : This function will register a callback function into Prestera TAPI for NA processing.
 * INPUT    : void CallBackFun    - callback to amtr to maintain mac address
 *                                  table
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    :
 *
 * <01242002> 1. It is an asynchronous callback whenever there's a NA event from the chip.
 *               The parameter "NA_type" shall depend on the Prestera features. (TBD)
 *            3. Since the NA may be very frequent, we have to tune the Prestera NA
 *                  interrupt processing to lower priority.
 *
 *-----------------------------------------------------------------------------*/
void DEV_AMTRDRV_Register_NA_CallBack(void CallBackFun (UI32_T vid,
                                                    UI8_T  *mac,
                                                    UI32_T unit,
                                                    UI32_T port,
                                                    UI32_T trunk_id,
                                                    BOOL_T is_trunk,
                                                    UI32_T NA_type));   /* <01242002> */


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_SetAddrTblEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will set address table entry in ARL
 * INPUT    : UI32_T vid                - VLAN ID
 *            UI8_T *mac                - MAC address
 *            UI32_T unit               - Unit number
 *            UI32_T *port   - Port number
 *            UI32_T *is_trunk          - is trunk or not
 *            BOOL_T is_static          - TRUE:static, FALSE:dynamic
 * OUTPUT   : None
 * RETURN   : BOOL_T Status             - True : successs, False : failed
 * NOTE     : Called by amtr, portsec, ui_dbg
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_SetAddrTblEntry(SWDRV_TYPE_L2_Address_Info_T *address_table);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DeleteAddr
 *------------------------------------------------------------------------------
 * Purpose  : This function will delete address table entries in ARL
 * INPUT    : UI32_T  vid   - vlan ID
 *            UI8_T  *mac   - Specified MAC to be cleared
 *            UI32_T unit   - unit number
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : Called by amtr, portsec, ui_dbg, mflt_mgr
 *
 * <01242002> 1. Prestera can ignore the parameter "unit".
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_DeleteAddr(UI32_T vid, UI8_T *mac);

/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DeleteAddrByPort
 *------------------------------------------------------------------------------
 * Purpose  : This function will delete addresses by port
 * INPUT    : UI32_T  unit                 -- unit
 *            UI32_T  port                 -- port
 *            UI32_T  trunk_id             -- trunk_id 
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : 1.Called by amtr,
 *            2.if trunk_id =0 means normal port else we will consider it as trunk port
 *-----------------------------------------------------------------------------*/
BOOL_T DEV_AMTRDRV_DeleteAddrByPort(UI32_T unit,UI32_T port,UI32_T trunk_id);
/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DeleteDynamicAddrByPortAndVlan
 *------------------------------------------------------------------------------
 * Purpose  : This function will delete addresses by port vlan
 * INPUT    : UI32_T  unit                 -- unit
 *            UI32_T  port                 -- port
 *            UI32_T  trunk_id             -- trunk_id 
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 *-----------------------------------------------------------------------------*/

BOOL_T DEV_AMTRDRV_DeleteDynamicAddrByPortAndVlan(UI32_T unit,UI32_T port,UI32_T trunk_id,UI32_T vlan_id);
/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DeleteDynamicAddrByPort
 *------------------------------------------------------------------------------
 * Purpose  : This function will delete addresses by port vlan
 * INPUT    : UI32_T  unit                 -- unit
 *            UI32_T  port                 -- port
 *            UI32_T  trunk_id             -- trunk_id 
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 *-----------------------------------------------------------------------------*/
BOOL_T DEV_AMTRDRV_DeleteDynamicAddrByPort(UI32_T unit,UI32_T port,UI32_T trunk_id);
/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DeleteAllDynamicAddr
 *------------------------------------------------------------------------------
 * Purpose  : Deletes all mac address entries learnt in the H/W table
 *            in local machine.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : Called by amtr_drv
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_DeleteAllDynamicAddr(void);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DeleteAllAddr
 *------------------------------------------------------------------------------
 * Purpose  : Deletes all dynamic mac address entries learnt in the H/W table
 *            in local machine
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : Called by amtr_drv
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_DeleteAllAddr(void);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_SetInterventionEntry
 *------------------------------------------------------------------------------
 * Purpose  : This routine will add a CPU MAC to the chip.  Packets with this
 *            MAC in the specified VLAN will trap to CPU.
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address
 *              mode        -               <01302002>
 * OUTPUT   : None
 * RETURN   : DEV_AMTRDRV_Ret_T   DEV_AMTRDRV_SUCCESS / DEV_AMTRDRV_FAIL / DEV_AMTRDRV_COLLISION
 * NOTE     : This API doesn't forbid that CPU has only one MAC.  It will only
 *            depend on spec.
 *
 * <01302002> 1. mode could be SA, DA, or either SA or DA.
 *                  DEV_AMTRDRV_INTERV_MODE_SA
 *                  DEV_AMTRDRV_INTERV_MODE_DA
 *                  DEV_AMTRDRV_INTERV_MODE_EITHER
 *            2. If chipset doesn't support "mode", it can ignore this parameter.
 *            3. CPU/Host MAC also use this API.
 *            4. This API is not just only for CPU MAC, but also for those packet
 *               which should be trapped to CPU.
 *
 *------------------------------------------------------------------------------*/
DEV_AMTRDRV_Ret_T  DEV_AMTRDRV_SetInterventionEntry(UI32_T vid, UI8_T *mac,UI8_T deviceGroup, UI32_T mode);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DeleteInterventionEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will delete a intervention mac address from
 *            address table
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_DeleteInterventionEntry(UI32_T vid, UI8_T *mac);


/*******************************************************************************
 * DEV_AMTRDRV_AddVlanMac
 *
 * Purpose: Add one MAC address that belonging to one VLAN.
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *       2. For Intel solution, there is only ONE MAC for the whole routing system.
 *       3. If IP Routing is disabled, the router MAC should just delete L3 bit.
 *          And the behavior of CPU mac will be just the same as L2 Intervention.
 *******************************************************************************
 */
 BOOL_T DEV_AMTRDRV_AddVlanMac(UI32_T vid, UI8_T *vlan_mac);


/*******************************************************************************
 * DEV_AMTRDRV_DeleteVlanMac
 *
 * Purpose: Remove one MAC address that belonging to one vlan interface.
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *       2. For Intel solution, there is only ONE MAC for the whole routing system.
 *          This function is non-applicable for Intel solution
 *******************************************************************************
 */
BOOL_T DEV_AMTRDRV_DeleteVlanMac(UI32_T vid, UI8_T *vlan_mac);

/*IGMP SNOOPING*/
/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_UpdateMulticastAddrTblEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will create a multicast address table for each device
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - multicast mac address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *
 * <01242002> This API implies the Prestera CPSS will create the specific entry
 *              in L2 table and the L2 Multicast table.
 *
 *-----------------------------------------------------------------------------*/
BOOL_T DEV_AMTRDRV_UpdateMulticastAddrTblEntry(UI32_T vid, UI8_T *mac, UI16_T fwd_priority);

/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_CreateMulticastAddrTblEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will create a multicast address table for each device
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - multicast mac address
 *            UI32_T fwd_priority- forwarding priority
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *
 * <01242002> This API implies the Prestera TAPI will create the specific entry
 *              in L2 table and the L2 Multicast table.
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_CreateMulticastAddrTblEntry(UI32_T vid, UI8_T *mac, UI16_T fwd_priority);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DestroyMulticastAddrTblEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will destroy a multicast address table for each device
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - multicast mac address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     :
 *
 * <01242002> This API implies the Prestera TAPI will remove the specific entry
 *              from the L2 table and the L2 Multicast table.
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_DestroyMulticastAddrTblEntry(UI32_T vid, UI8_T* mac);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_AddMulticastPortMember
 *------------------------------------------------------------------------------
 * Purpose  : This function will set multicast address table entry in ARL
 * INPUT    : UI32_T vid    - VLAN ID
 *            UI8_T *mac    - multicast MAC address
 *            UI32_T unit   - Unit number
 *            UI32_T port   - Port number
 *            BOOL_T tagged - TRUE: tagged / FALSE: untagged
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : 1. per device dependent
 *           2. For Intel, the last argument tagged will be ignored.
 *           3. If chip does not follow VLAN egress rule, argument tagged is needed.
 *           4. If egress rule is changed, we need to modify this ....
 *
 * <01242002> 1. For Prestera, the input parameter "tagged" could be ignored.
 *               Since the chip will lookup the tagging format from VLAN table for egress.
 *
 *-----------------------------------------------------------------------------*/
BOOL_T DEV_AMTRDRV_AddMulticastPortMember(UI32_T vid, UI8_T *mac, UI32_T unit, UI32_T port, BOOL_T tagged);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DeleteMulticastPortMember
 *------------------------------------------------------------------------------
 * Purpose  : This function will remove multicast port member in specified group in ARL
 * INPUT    : UI32_T unit   - Unit number
              UI32_T port   - Port number
 *            UI32_T vid    - VLAN ID
 *            UI8_T *mac    - multicast MAC address
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : 1. per device dependent
 *-----------------------------------------------------------------------------*/
BOOL_T DEV_AMTRDRV_DeleteMulticastPortMember(UI32_T unit, UI32_T port, UI32_T vid, UI8_T *mac);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_SetMulticastPortList
 *------------------------------------------------------------------------------
 * Purpose  : This function will set multicast address table entry in ARL
 * INPUT    : UI32_T vid    - VLAN ID
 *            UI8_T *mac    - multicast MAC address
 *            UI32_T unit   - Unit number
 *            UI32_T port   - Port number
 *            BOOL_T tagged - TRUE: tagged / FALSE: untagged
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : 1. per device dependent
 *           2. For Intel, the last argument tagged will be ignored.
 *           3. If chip does not follow VLAN egress rule, argument tagged is needed.
 *           4. If egress rule is changed, we need to modify this ....
 *
 * <01242002> 1. For Prestera, the input parameter "tagged" could be ignored.
 *               Since the chip will lookup the tagging format from VLAN table for egress.
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_SetMulticastPortList(UI32_T vid, UI8_T *mac, UI8_T *port_list, UI8_T *untagged_port_list);

/* MIKE add to patch IML to get address information from chip directly 08/25/05 */
/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_ARLFirstChipLookUp
 *------------------------------------------------------------------------------
 * Purpose  : This function will use given a MAC address and VLAN ID, check if 
 *            the entry is present in the L2 table
 * INPUT    : UI32_T  vid   - vlan ID
 *            UI8_T  *mac   - Specified MAC to be cleared
 * OUTPUT   : UI32_T *unit              - unit
 *            UI32_T *port              - Port number 
 *            UI32_T *trunk_id          - Trunk ID
 *            BOOL_T *is_trunk          - is trunk or not
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : 
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_ARLFirstChipLookUp(UI32_T vid, 
                                       UI8_T  *mac,
                                       UI32_T *unit, 
                                       UI32_T *port, 
                                       UI32_T *trunk_id, 
                                       BOOL_T *is_trunk);
                                       
/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_ARLLookUp
 *------------------------------------------------------------------------------
 * Purpose  : This function will use given a MAC address and VLAN ID, check if 
 *            the entry is present in the L2 table
 * INPUT    : UI32_T  vid   - vlan ID
 *            UI8_T  *mac   - Specified MAC to be cleared
 * OUTPUT   : UI32_T *port              - Port number 
 *            UI32_T *trunk_id          - Trunk ID
 *            BOOL_T *is_trunk          - is trunk or not
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : 
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_ARLLookUp(UI32_T vid, 
                              UI8_T *mac, 
                              UI32_T *port, 
                              UI32_T *trunk_id, 
                              BOOL_T *is_trunk);

/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DisableBroadcastTrapToCpu
 *------------------------------------------------------------------------------
 * Purpose  : This function will enable broadcast that trap to CPU 
 * INPUT    : I32_T device_id - Device ID
 *            UI32_T  vid     - vlan ID
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : 
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_EnableBroadcastTrapToCpu(I32_T device_id, UI32_T vid);

/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DisableBroadcastTrapToCpu
 *------------------------------------------------------------------------------
 * Purpose  : This function will disable broadcast that trap to CPU 
 * INPUT    : I32_T device_id - Device ID
 *            UI32_T  vid     - vlan ID
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : 
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_DisableBroadcastTrapToCpu(I32_T device_id, UI32_T vid);


/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_ReadAndClearLocalHitBit
 *------------------------------------------------------------------------------
 * Purpose  : this function return corresponding ARL entry's hitbit (0 or 1) in local unit, and set    hitbit=0
 * INPUT    : 
 *            UI8_T *mac    - MAC address
 *            UI32_T vid    - VLAN ID
 *	         UI32_T port[]   - Port number. When port[0]==0, it means chekc every device on local unit.
 *            UI32_T trunk_member_count. If >1 -> member is unit[0]/port[0], unit[1]/port[1],..
 * OUTPUT   : None
 * RETURN   : BOOL_T  hitbit value true:1 / false:0
 * NOTES    : 
 *-----------------------------------------------------------------------------*/
BOOL_T  DEV_AMTRDRV_ReadAndClearLocalHitBit(UI8_T* hitbit_value,UI8_T*  mac, UI32_T vid,UI32_T unit,UI32_T* port, UI32_T trunk_member_count_in_localunit);

/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_SetAddrTblEntryByDevice
 *------------------------------------------------------------------------------
 * Purpose  : This function set address entry to specified device
 * INPUT    : UI8_T deviceGroup    
 *            DEV_AMTRDRV_FIRST_DEVICE : set 1st device only
 *            DEV_AMTRDRV_EXCLUDE_1ST_DEVICES : set all chips except 1st one.
 *            DEV_AMTRDRV_ALL_DEVICE : set all device
 *            SWDRV_TYPE_L2_Address_Info_T     address_p
 * OUTPUT   : None
 * RETURN   : DEV_AMTRDRV_Ret_T   DEV_AMTRDRV_SUCCESS / DEV_AMTRDRV_FAIL / DEV_AMTRDRV_COLLISION
 * NOTES    : used for trunk port
 *-----------------------------------------------------------------------------*/
DEV_AMTRDRV_Ret_T  DEV_AMTRDRV_SetAddrTblEntryByDevice (UI8_T deviceGroup,SWDRV_TYPE_L2_Address_Info_T *address_p);

/*------------------------------------------------------------------------------
 * Function : DEV_AMTRDRV_DeleteAddrByDevice
 *------------------------------------------------------------------------------
 * Purpose  : This function will delete address table entries from specific chip(s)
 * INPUT    :   UI8_T deviceGroup
 *                  DEV_AMTRDRV_FIRST_DEVICE : del 1st device only
 *			        DEV_AMTRDRV_EXCLUDE_1ST_DEVICES : del all chips except 1st one.
 *			        DEV_AMTRDRV_ALL_DEVICE : del all device 
 *              UI32_T  vid   - vlan ID
 *              UI8_T  *mac   - Specified MAC to be cleared
 *              UI32_T unit   - unit number
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : Called by amtr, portsec, ui_dbg, mflt_mgr
 *
 * <01242002> 1. Prestera can ignore the parameter "unit".
 *
 *-----------------------------------------------------------------------------*/
BOOL_T DEV_AMTRDRV_DeleteAddrByDevice(UI8_T deviceGroup,UI32_T vid, UI8_T *mac);

/*End Simon_Shih*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DEV_AMTRDRV_DisableHardwareLearning
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will disable hardware learning
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T DEV_AMTRDRV_DisableHardwareLearning();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DEV_AMTRDRV_SetMCastEntryForInternalPacket
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set a multi-cast entry for internal packet.
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   multi-cast entry to 5690: 
 *              mac  - 0x1ffffffffff
 *              vid  - SYS_ADPT_VLAN_INTERNAL_MANAGEMENT_VLAN
 *              port - CMIC Port only
 * ------------------------------------------------------------------------
 */
void DEV_AMTRDRV_SetMCastEntryForInternalPacket(void);

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - DEV_AMTRDRV_GetHashLookupDepth
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will get hash lookup depth from chip
 * INPUT    :   None
 * OUTPUT   :   lookup_depth_p  -- hash lookup depth
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T DEV_AMTRDRV_GetHashLookupDepth(UI32_T *lookup_depth_p);
#endif

void DEV_AMTRDRV_Create_InterCSC_Relation(void);

BOOL_T DEV_AMTRDRV_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);


#endif  /* end of DEV_AMTRDRV_H */


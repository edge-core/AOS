/*-----------------------------------------------------------------------------
 * MODULE NAME: AMTR_OM_PRIVATE.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The declarations of AMTR OM which are only used by AMTR.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/14     --- Timon, Create and move something from amtr_om.h
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef AMTR_OM_PRIVATE_H
#define AMTR_OM_PRIVATE_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "amtr_type.h"
#include "amtr_mgr.h"
#include "sys_cpnt.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* key index for HISAM table
 */
#define AMTR_MV_KIDX                             0                   /* mac + vid key index              */
#define AMTR_VM_KIDX                             1                   /* vid + mac key index              */
#define AMTR_IVM_KIDX                            2                   /* ifindex + vid + mac key     */

/* Field length definitions for HISAM table
 */
#define AMTR_VID_LEN                             2
#define AMTR_IFINDEX_LEN                         2
#define AMTR_MVKEY_LEN                           (AMTR_TYPE_MAC_LEN + AMTR_VID_LEN)
#define AMTR_VMKEY_LEN                           (AMTR_MVKEY_LEN)
#define AMTR_IVMKEY_LEN                          (AMTR_MVKEY_LEN + AMTR_IFINDEX_LEN)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_Init
 * -------------------------------------------------------------------------
 * PURPOSE:  This function initialize AMTR_OM(Hisam table).
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_Init(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamSetRecord
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set a record to Hisam Table
 * INPUT:    AMTR_TYPE_AddrEntry_T addr_entry -- address entry
 * OUTPUT:   none.
 * RETURN:  none
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamSetRecord(AMTR_TYPE_AddrEntry_T *addr_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecord
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete one record from Hisam Table
 * INPUT:    UI8_T *key     -- search key
 * OUTPUT:   None
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecord(UI32_T vid, UI8_T *mac);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecord
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete all record from Hisam Table
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTES:    Hisam TAble re-initial
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteAllRecord(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLifeTime
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by life time from Hisam Table
 * INPUT:    AMTR_TYPE_AddressLifeTime_T life_time    -- condition
 * OUTPUT:   UI32_T *action_counter                   -- number of deletion
 * RETURN:   None
 * NOTES:    1. action_counter: This API count how many records are deleted from Hisam TAble.
 *           2. AMTR Task will count sync number by action_counter.
 *              This job (sync Hash to Hisam) can't be spent too much time.
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLifeTime(AMTR_TYPE_AddressLifeTime_T life_time, UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordBySource
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by source from Hisam Table
 * INPUT:    AMTR_TYPE_AddressSource_T source    -- condition
 * OUTPUT:   UI32_T *action_counter              -- number of deletion
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordBySource(AMTR_TYPE_AddressSource_T source,UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPort
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port from Hisam Table
 * INPUT:    UI32_T ifindex                -- condition
 * OUTPUT:   UI32_T *action_counter        -- number of deletion
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPort(UI32_T ifindex,UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndLifeTime
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+life_time from Hisam Table
 * INPUT:    UI32_T ifindex                            -- condition1
 *           AMTR_TYPE_AddressLifeTime_T life_time     -- condition2
 * OUTPUT:   UI32_T *action_counter                    -- number of deletion
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndLifeTime(UI32_T ifindex,AMTR_TYPE_AddressLifeTime_T life_time, UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndSource
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+source from Hisam Table
 * INPUT:    UI32_T ifindex                          -- condition1
 *           AMTR_TYPE_AddressSource_T source        -- condition2
 * OUTPUT:   UI32_T *action_counter                  -- number of deletion
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndSource(UI32_T ifindex,AMTR_TYPE_AddressSource_T source, UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByVid
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by vid from Hisam Table
 * INPUT:    UI32_T vid                -- condition
 * OUTPUT:   UI32_T *action_counter    -- number of deletion
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByVid(UI32_T vid,UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByVidAndLifeTime
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by vid+life_time from Hisam Table
 * INPUT:    UI32_T vid                                -- condition1
 *           AMTR_TYPE_AddressLifeTime_T life_time     -- condition2
 * OUTPUT:   UI32_T *action_counter                    -- number of deletion
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByVidAndLifeTime(UI32_T vid,AMTR_TYPE_AddressLifeTime_T life_time, UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByVidAndSource
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by vid+Source from Hisam Table
 * INPUT:    UI32_T vid                              -- condition1
 *           AMTR_TYPE_AddressSource_T source        -- condition2
 * OUTPUT:   UI32_T *action_counter                  -- number of deletion
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByVidAndSource(UI32_T vid,AMTR_TYPE_AddressSource_T source, UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndVid
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+vid from Hisam Table
 * INPUT:    UI32_T port                -- condition1
 *           UI32_T vid                 -- condition2
 * OUTPUT:   UI32_T *action_counter     -- number of deletion
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndVid(UI32_T ifindex,UI32_T vid, UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndVidAndLifeTime
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+vid+life_time from Hisam Table
 * INPUT:    UI32_T port                            -- condition1
 *           UI32_T vid                             -- condition2
 *           AMTR_TYPE_AddressLifeTime_T life_time  -- condition3
 * OUTPUT:   UI32_T *action_counter                 -- number of deletion
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndVidAndLifeTime(UI32_T ifindex,UI32_T vid,AMTR_TYPE_AddressLifeTime_T life_time, UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndVidlistAndLifeTime
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+vid+life_time from Hisam Table
 * INPUT:    UI32_T port                            -- condition1
 *           UI32_T vid                             -- condition2
 *           AMTR_TYPE_AddressLifeTime_T life_time  -- condition3
 * OUTPUT:   UI32_T *action_counter                 -- number of deletion
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/

void AMTR_OM_HisamDeleteRecordByLPortAndVidlistAndLifeTime(UI32_T ifindex,UI16_T *vlan_p,AMTR_TYPE_AddressLifeTime_T life_time, UI32_T *action_counter);


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndVidAndSource
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+vid+source from Hisam Table
 * INPUT:    UI32_T port                          -- condition1
 *           UI32_T vid                           -- condition2
 *           AMTR_TYPE_AddressSource_T source     -- condition3
 * OUTPUT:   UI32_T *action_counter               -- number of deletion
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndVidAndSource(UI32_T ifindex,UI32_T vid,AMTR_TYPE_AddressSource_T source, UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamDeleteRecordByLPortAndVidExceptCertainAddr
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will delete records by port+vid from Hisam Table
 * INPUT  :  UI32_T port                 -- condition1
 *           UI32_T vid                  -- condition2
 *           UI8_T mac_list_p[][SYS_ADPT_MAC_ADDR_LEN]
 *           UI8_T mask_list_p[][SYS_ADPT_MAC_ADDR_LEN]
 *           UI32_T number_of_entry_in_list
 * OUTPUT :  UI32_T *action_counter      -- number of deletion
 * RETURN :  None
 * NOTES  :
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamDeleteRecordByLPortAndVidExceptCertainAddr(UI32_T ifindex,
                                                             UI32_T vid,
                                                             UI8_T mac_list_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                             UI8_T mask_list_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                             UI32_T number_of_entry_in_list,
                                                             UI32_T *action_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_HisamUpdateRecordLifeTimeByLPort
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will update record's life time by port in Hisam Table
 * INPUT:    UI32_T ifindex
 *           AMTR_TYPE_AddressLifeTime_T life_time
 * OUTPUT:   None
 * RETURN:   None
 * NOTES:    Only learnt entry will be updated.
 * -------------------------------------------------------------------------*/
void AMTR_OM_HisamUpdateRecordLifeTimeByLPort(UI32_T ifindex,AMTR_TYPE_AddressLifeTime_T life_time);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMVKey
 *------------------------------------------------------------------------------
 * PURPOSE: Generate the key needed to access HISAM entries in MAC -> VLAN order
 * INPUT  : UI16_T vid  - vlan id
 *          UI8_T  *mac - logical unit
 * OUTPUT : UI8_T  *key - required key
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
void AMTR_OM_SetMVKey (UI8_T key[AMTR_MVKEY_LEN], UI16_T vid, UI8_T *mac);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetVMKey
 *------------------------------------------------------------------------------
 * PURPOSE: Generate the key needed to access HISAM entries in MAC -> VLAN order
 * INPUT  : UI16_T vid  - vlan id
 *          UI8_T  *mac - logical unit
 * OUTPUT : UI8_T  *key - required key
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
void AMTR_OM_SetVMKey (UI8_T key[AMTR_VMKEY_LEN], UI16_T vid, UI8_T *mac);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetIVMKey
 *------------------------------------------------------------------------------
 * PURPOSE: Generate the key needed to access HISAM entries in MAC -> VLAN order
 * INPUT  : UI16_T  vid     - vlan id
 *          UI8_T   *mac    - mac address
 *          UI8_T   port    - u_port number
 *          UI8_T   trunk_id- trunk id
 * OUTPUT : UI8_T  *key - required key
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
void AMTR_OM_SetIVMKey(UI8_T key[AMTR_IVMKEY_LEN], UI16_T vid, UI8_T *mac, UI16_T ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_SetEditAddrEntryBuf
 *-------------------------------------------------------------------------
 * PURPOSE: This funciton will specify the buffer of event entry for
 *          tracking which entries were deleted
 * INPUT  : edit_entry_buf -- the buffer to store event entry
 *          max_count      -- the number of event entry in the buffer
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : None
 *-------------------------------------------------------------------------*/
BOOL_T AMTR_OM_SetEditAddrEntryBuf(AMTR_TYPE_EventEntry_T *edit_entry_buf, UI32_T max_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_GetPortInfoPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will return pointer to amtr port info.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This API is only allowed to be called in AMTR internally.
 *-------------------------------------------------------------------------
 */
AMTR_MGR_PortInfo_T* AMTR_OM_GetPortInfoPtr(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set AMTR operating mode as Transition mode
 *            on shared memory OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This API is only allowed to be called in AMTR internally.
 *-------------------------------------------------------------------------
 */
void AMTR_OM_SetTransitionMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : This function performs AMTR enter Transition mode
 *            on shared memory OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This API is only allowed to be called in AMTR internally.
 *-------------------------------------------------------------------------
 */
void AMTR_OM_EnterTransitionMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set AMTR operating mode as Master mode
 *            on shared memory OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This API is only allowed to be called in AMTR internally.
 *-------------------------------------------------------------------------
 */
void AMTR_OM_EnterMasterMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set AMTR operating mode as Slave mode
 *            on shared memory OM.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This API is only allowed to be called in AMTR internally.
 *-------------------------------------------------------------------------
 */
void AMTR_OM_EnterSlaveMode(void);

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_MacNotifyCopyAndClearQueue
 *------------------------------------------------------------------------------
 * PURPOSE  : To process the mac-notification entries in the queue
 * INPUT    : None
 * OUTPUT   : rec_p
 *          : ntfy_cnt_p
 *          : used_head_p
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void AMTR_OM_MacNotifyCopyAndClearQueue(AMTR_MGR_MacNotifyRec_T *rec_p, UI32_T *ntfy_cnt_p, UI32_T *used_head_p);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_MacNotifyAddNewEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : To add a new entry to the queue for further processing
 * INPUT    : ifidx   - lport ifindex
 *            vid     - vlan id
 *            src_mac - source mac
 *            is_add  - add/remove operation
 *            need_sem - need to take sem or not
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_MacNotifyAddNewEntry(
    UI32_T  ifidx,
    UI16_T  vid,
    UI8_T   *src_mac_p,
    BOOL_T  is_add,
    BOOL_T  need_sem);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMacNotifyGlobalStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : To set mac-notification-trap global status
 * INPUT    : is_enabled - global status to set
 * OUTPUT   : None
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_SetMacNotifyGlobalStatus(BOOL_T  is_enabled);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMacNotifyInterval
 *------------------------------------------------------------------------------
 * PURPOSE  : To set mac-notification-trap interval
 * INPUT    : interval - interval to set (in ticks)
 * OUTPUT   : None
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_SetMacNotifyInterval(UI32_T  interval);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : To get mac-notification-trap port status
 * INPUT    : ifidx
 *            is_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_SetMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_MacNotifyAddFstTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the port joins the trunk
 *          as the 1st member
 * INPUT  : trk_ifidx - specify which trunk to join.
 *          mbr_ifidx - specify which member port being add to trunk.
 * OUTPUT : None
 * RETURN : None
 * NOTES  : mbr_ifidx is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void AMTR_OM_MacNotifyAddFstTrkMbr(UI32_T  trk_ifidx, UI32_T  mbr_ifidx);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_MacNotifyDelTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the trunk member is
 *          removed from the trunk
 * INPUT  : trk_ifidx - specify which trunk to remove from
 *          mbr_ifidx - specify which member port being removed from trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void AMTR_OM_MacNotifyDelTrkMbr(UI32_T  trk_ifidx, UI32_T  mbr_ifidx);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_MacNotifyDelLstTrkMbr
 * ------------------------------------------------------------------------
 * PURPOSE: Service the callback from SWCTRL when the last trunk member
 *          is removed from the trunk
 * INPUT  : trk_ifidx - specify which trunk to join to
 *          mbr_ifidx - specify which member port being add to trunk
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
void AMTR_OM_MacNotifyDelLstTrkMbr(UI32_T  trk_ifidx, UI32_T  mbr_ifidx);
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Enable/disable vlan learning of specified vlan
 * INPUT    : vid
 *            learning
 * OUTPUT   : None
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_SetVlanLearningStatus(UI32_T vid, BOOL_T learning);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_SetMlagMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : To set MLAG mac notify port status
 * INPUT    : ifidx
 *            is_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_SetMlagMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_MlagMacNotifyAddNewEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : To add a new entry to the queue for further processing
 * INPUT    : ifidx   - lport ifindex
 *            vid     - vlan id
 *            src_mac - source mac
 *            is_add  - add/remove operation
 *            need_sem - need to take sem or not
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_MlagMacNotifyAddNewEntry(
    UI32_T  ifidx,
    UI16_T  vid,
    UI8_T   *src_mac_p,
    BOOL_T  is_add,
    BOOL_T  need_sem);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_MlagMacNotifyCopyAndClearQueue
 *------------------------------------------------------------------------------
 * PURPOSE  : To process the MLAG mac notify entries in the queue
 * INPUT    : None
 * OUTPUT   : rec_p
 *          : ntfy_cnt_p
 *          : used_head_p
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void AMTR_OM_MlagMacNotifyCopyAndClearQueue(AMTR_MGR_MacNotifyRec_T *rec_p, UI32_T *ntfy_cnt_p, UI32_T *used_head_p);

#if (SYS_CPNT_OVSVTEP == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_OvsCopyAndClearQueue
 *------------------------------------------------------------------------------
 * PURPOSE  : To process the OVSVTEP mac notify entries in the queue
 * INPUT    : None
 * OUTPUT   : rec_p
 *          : ntfy_cnt_p
 *          : used_head_p
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void AMTR_OM_OvsCopyAndClearQueue(AMTR_MGR_MacNotifyRec_T *rec_p, UI32_T *ntfy_cnt_p, UI32_T *used_head_p);

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_OvsAddNewEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : To add a new entry to the queue for further processing
 * INPUT    : ifidx     - lport ifindex
 *            vid       - vlan id
 *            src_mac_p - source mac
 *            is_add    - add/remove operation
 *            need_sem  - need to take sem or not
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_OvsAddNewEntry(
    UI32_T  ifidx,
    UI16_T  vid,
    UI8_T   *src_mac_p,
    BOOL_T  is_add,
    BOOL_T  need_sem);

#endif

#endif /* #ifndef AMTR_OM_PRIVATE_H */


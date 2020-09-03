/*-----------------------------------------------------------------------------
 * Module Name: lldp_om.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementation for the LLDP object manager
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    11/17/2005 - Gordon Kao, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2005
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "l_cvrt.h"
#include "l_hisam.h"
#include "l_mpool.h"
#include "l_sort_lst.h"
#include "l_link_lst.h"
#include "sysfun.h"
#include "lldp_om.h"
#include "lldp_om_private.h"
#include "lldp_type.h"
#include "lldp_uty.h"
#include "swctrl.h"
#include "sys_time.h"


#define RX_DEBUG_PRINT  0

/* MACRO FUNCTIONS DECLARACTION
 */
#define LLDP_OM_SET_ETS_NOTIFY_ENTRY(is_del, not_ent_p, rem_data_p) \
            {                                                       \
                memset(not_ent_p, 0, sizeof(*not_ent_p));           \
                not_ent_p->time_mark = rem_data_p->time_mark;       \
                not_ent_p->lport     = lport;                       \
                not_ent_p->index     = rem_data_p->index;           \
                not_ent_p->is_delete = is_del;                      \
                if (TRUE != is_del)                                 \
                {                                                   \
                    if (rem_data_p->dcbx_rem_ets_config_entry != NULL)  \
                    {   \
                        not_ent_p->rem_config_willing = rem_data_p->dcbx_rem_ets_config_entry->rem_willing; \
                        not_ent_p->rem_config_cbs = rem_data_p->dcbx_rem_ets_config_entry->rem_cbs; \
                        not_ent_p->rem_config_max_tc = rem_data_p->dcbx_rem_ets_config_entry->rem_max_tc;   \
                        memcpy(not_ent_p->rem_config_pri_assign_table, rem_data_p->dcbx_rem_ets_config_entry->rem_pri_assign_table, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);    \
                        memcpy(not_ent_p->rem_config_tc_bandwidth_table, rem_data_p->dcbx_rem_ets_config_entry->rem_tc_bandwidth_table, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);  \
                        memcpy(not_ent_p->rem_config_tsa_assign_table, rem_data_p->dcbx_rem_ets_config_entry->rem_tsa_assign_table, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);    \
                    }   \
                    if (rem_data_p->dcbx_rem_ets_recommend_entry != NULL)   \
                    {   \
                        not_ent_p->rem_recommend_rcvd = TRUE;   \
                        memcpy(not_ent_p->rem_recommend_pri_assign_table, rem_data_p->dcbx_rem_ets_recommend_entry->rem_pri_assign_table, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);  \
                        memcpy(not_ent_p->rem_recommend_tc_bandwidth_table, rem_data_p->dcbx_rem_ets_recommend_entry->rem_tc_bandwidth_table, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);    \
                        memcpy(not_ent_p->rem_recommend_tsa_assign_table, rem_data_p->dcbx_rem_ets_recommend_entry->rem_tsa_assign_table, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);  \
                    } \
                }   \
            }

#define LLDP_OM_SET_PFC_NOTIFY_ENTRY(is_del, not_ent_p, rem_data_p) \
            {                                                       \
                memset(not_ent_p, 0, sizeof(*not_ent_p));           \
                not_ent_p->time_mark = rem_data_p->time_mark;       \
                not_ent_p->lport     = lport;                       \
                not_ent_p->index     = rem_data_p->index;           \
                not_ent_p->is_delete = is_del;                      \
                if (TRUE != is_del)                                 \
                {                                                   \
                    memcpy(not_ent_p->rem_mac, rem_data_p->dcbx_rem_pfc_config_entry->rem_mac, 6);    \
                    not_ent_p->rem_willing = rem_data_p->dcbx_rem_pfc_config_entry->rem_willing;      \
                    not_ent_p->rem_mbc = rem_data_p->dcbx_rem_pfc_config_entry->rem_mbc;              \
                    not_ent_p->rem_cap = rem_data_p->dcbx_rem_pfc_config_entry->rem_cap;              \
                    not_ent_p->rem_enable = rem_data_p->dcbx_rem_pfc_config_entry->rem_enable;        \
                }   \
            }

/* system configuration entry */
static LLDP_OM_SysConfigEntry_T             LLDP_OM_SysConfig;

/* port configuration entry*/
static LLDP_OM_PortConfigEntry_T            LLDP_OM_PortConfig[SYS_ADPT_TOTAL_NBR_OF_LPORT];

/* system statistic entry*/
static LLDP_OM_Statistics_T                 LLDP_OM_Statistics;

/* port statistic entry */
static LLDP_OM_PortStatistics_T             LLDP_OM_PortStatistics[SYS_ADPT_TOTAL_NBR_OF_LPORT];


/* Pool*/
static L_MPOOL_HANDLE                       LLDP_OM_RemDataPool;
static UI32_T                               LLDP_OM_RemDataPoolFreeNum;
static UI32_T                               LLDP_OM_RemDataPoolAllocNum;

/* total number of remote data */
static UI32_T                               LLDP_OM_RemDataTotal;

/* remote data index */
static UI32_T                               LLDP_OM_RemDataIndex;

/* Hisam table */
static L_HISAM_KeyDef_T                     LLDP_OM_Keydef;
static L_HISAM_Desc_T                       LLDP_OM_HisamDesc;

/* Sort list */
static L_SORT_LST_List_T                    LLDP_OM_LocManAddrConfig;
static L_SORT_LST_List_T                    LLDP_OM_RemDataList[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static L_SORT_LST_List_T                    LLDP_OM_RemDataAllList;

/* Age out sequence */
static LLDP_OM_RemData_T                    *LLDP_OM_AgeSeqBeg;
static LLDP_OM_RemData_T                    *LLDP_OM_AgeSeqEnd;

/* semaphore id */
static UI32_T                               LLDP_OM_SemId;

static  UI32_T                              original_priority;

/* notify entry list */
static L_LINK_LST_List_T                    LLDP_OM_NotifyTelephoneTypeList;
static L_SORT_LST_List_T                    LLDP_OM_NotifyTelephoneAddrChangedList;
static L_SORT_LST_List_T                    LLDP_OM_NotifyMedRemTableChangedList;
#if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2))
static L_SORT_LST_List_T                    LLDP_OM_NotifyDot3atInfoReceivedList;
#endif
#if(SYS_CPNT_DCBX == TRUE)
static L_LINK_LST_List_T                    LLDP_OM_NotifyDcbxRemEtsChangedList;
static L_LINK_LST_List_T                    LLDP_OM_NotifyDcbxRemPfcChangedList;
static LLDP_DCBX_TIMER_T                    LLDP_OM_DcbxTimer[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
#if (SYS_CPNT_CN == TRUE)
static L_LINK_LST_List_T                    LLDP_OM_NotifyCnRemoteChangeList;
#endif
static L_LINK_LST_List_T                    LLDP_OM_NotifyRemChangeList;

static BOOL_T   tlv_change_detect[LLDP_OM_MAX_DECTECT_NUM];

#if(SYS_CPNT_DCBX == TRUE)
static BOOL_T LLDP_OM_CompNotifyDcbxRemEtsEntry(void *inlist_element, void *input_element);
static BOOL_T LLDP_OM_CompNotifyDcbxRemPfcEntry(void *inlist_element, void *input_element);
static void LLDP_OM_InitDcbxTimer(void);
static BOOL_T LLDP_OM_IsMultiPeerDetected(UI32_T lport);
#endif
static int LLDP_OM_SortCompRemAppPriorityEntry(void* inlist_element, void* input_element);
static void LLDP_OM_FreeRemAppPriorityList(L_SORT_LST_List_T *rem_app_priority_list);

static void LLDP_OM_FreeRemProtoVlanList();
static void LLDP_OM_FreeRemVlanNameList();
static void LLDP_OM_FreeRemProtocolList();
static void LLDP_OM_LocalFreeRemData(LLDP_OM_RemData_T *free_data);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompRemData
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote data sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompRemData(void* inlist_element, void* input_element)
{
    LLDP_OM_RemData_T *element1, *element2;
    element1 = *((LLDP_OM_RemData_T **)(inlist_element));
    element2 = *((LLDP_OM_RemData_T **)(input_element));
    return (element1->index - element2->index);
}/* End of LLDP_OM_SortCompRemData */

#if 0
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompRemDataKey
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote data sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompRemDataKey(void *inlist_element, void *input_element)
{
    LLDP_OM_RemData_T    *element1, *element2;
    int result;
    element1 = *((LLDP_OM_RemData_T **)(inlist_element));
    element2 = *((LLDP_OM_RemData_T **)(input_element));

    if(element1->rem_chassis_id_len != element2->rem_chassis_id_len)
        return (element1->rem_chassis_id_len - element2->rem_chassis_id_len);

    if((result = memcmp(element1->rem_chassis_id, element2->rem_chassis_id, element1->rem_chassis_id_len))!= 0)
        return result;

    if(element1->rem_port_id_len != element2->rem_port_id_len)
        return (element1->rem_port_id_len - element2->rem_port_id_len);

    if((result = memcmp(element1->rem_port_id, element2->rem_port_id, element1->rem_port_id_len))!= 0)
        return result;

    return 0;
}
#endif /* #if 0 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompRemDataMib
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote data sort list comparing the index
 *            defined in the standard MIB
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompRemDataMib(void *inlist_element, void *input_element)
{
    LLDP_OM_RemData_T    *element1, *element2;

    element1 = *((LLDP_OM_RemData_T **)(inlist_element));
    element2 = *((LLDP_OM_RemData_T **)(input_element));

    if (element1->time_mark != element2->time_mark)
        return (element1->time_mark - element2->time_mark);

    if (element1->lport != element2->lport)
        return (element1->lport - element2->lport);

    return (element1->index - element2->index);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompRemManAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote management address sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompRemManAddrEntry(void* inlist_element, void* input_element)
{
    LLDP_OM_RemManAddrEntry_T *element1, *element2;
    int i;
    element1 = *((LLDP_OM_RemManAddrEntry_T**)(inlist_element));
    element2 = *((LLDP_OM_RemManAddrEntry_T**)(input_element));
    if(element1->rem_man_addr_subtype != element2->rem_man_addr_subtype)
        return (element1->rem_man_addr_subtype - element2->rem_man_addr_subtype);
    for(i = 0; i < LLDP_TYPE_MAX_MANAGEMENT_ADDR_LENGTH; i++)
    {
        if(element1->rem_man_addr[i] != element2->rem_man_addr[i])
            return element1->rem_man_addr[i] - element2->rem_man_addr[i];
    }

    return 0;
}/* End of LLDP_OM_SortCompRemManAddrEntry*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompLocManAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote management address sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompLocManAddrEntry(void* inlist_element, void* input_element)
{
    LLDP_OM_ConfigManAddrEntry_T *element1, *element2;
    int i;


    element1 = *((LLDP_OM_ConfigManAddrEntry_T**)(inlist_element));
    element2 = *((LLDP_OM_ConfigManAddrEntry_T**)(input_element));
    if(element1->loc_man_addr_subtype != element2->loc_man_addr_subtype)
        return (element1->loc_man_addr_subtype - element2->loc_man_addr_subtype);
    for(i = 0; i < LLDP_TYPE_MAX_MANAGEMENT_ADDR_LENGTH; i++)
    {
        if(element1->loc_man_addr[i] != element2->loc_man_addr[i])
            return element1->loc_man_addr[i] - element2->loc_man_addr[i];
    }

    return 0;
}/* End of LLDP_OM_SortCompLocManAddrEntry*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompRemProtoVlanEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote protocol vlan list to sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompRemProtoVlanEntry(void* inlist_element, void* input_element)
{
    LLDP_OM_Xdot1RemProtoVlanEntry_T *element1, *element2;

    element1 = *(LLDP_OM_Xdot1RemProtoVlanEntry_T **)inlist_element;
    element2 = *(LLDP_OM_Xdot1RemProtoVlanEntry_T **)input_element;
    return (element1->rem_proto_vlan_id - element2->rem_proto_vlan_id);

}/* End of LLDP_OM_SortCompRemProtoVlanEntry */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompRemVlanNameEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote vlan name list to sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompRemVlanNameEntry(void* inlist_element, void* input_element)
{
    LLDP_OM_Xdot1RemVlanNameEntry_T *element1, *element2;

    element1 = *(LLDP_OM_Xdot1RemVlanNameEntry_T **)inlist_element;
    element2 = *(LLDP_OM_Xdot1RemVlanNameEntry_T **)input_element;

    return (element1->rem_vlan_id - element2->rem_vlan_id);

}/* End of LLDP_OM_SortCompRemVlanNameEntry */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompRemProtocolEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote protocol list to sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompRemProtocolEntry(void* inlist_element, void* input_element)
{
    LLDP_OM_Xdot1ProtocolEntry_T *element1, *element2;

    element1 = *(LLDP_OM_Xdot1ProtocolEntry_T **)inlist_element;
    element2 = *(LLDP_OM_Xdot1ProtocolEntry_T **)input_element;

    return (element1->rem_protocol_index - element2->rem_protocol_index);

}/* End of LLDP_OM_SortCompRemProtocolEntry */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompCivicAddrCaEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the civic addr ca sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompCivicAddrCaEntry(void* inlist_element, void* input_element)
{
    LLDP_OM_XMedLocationCivicAddrCaTlv_T *element1, *element2;

    element1 = *(LLDP_OM_XMedLocationCivicAddrCaTlv_T **)inlist_element;
    element2 = *(LLDP_OM_XMedLocationCivicAddrCaTlv_T **)input_element;

    return (element1->ca_type - element2->ca_type);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompNotifyEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the notify entry list to sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompNotifyEntry(void *inlist_element, void *input_element)
{
    LLDP_TYPE_NotifyTelephone_T   *element1, *element2;

    element1 = *(LLDP_TYPE_NotifyTelephone_T **)inlist_element;
    element2 = *(LLDP_TYPE_NotifyTelephone_T **)input_element;

    if (element1->lport != element2->lport)
    {
        return (element1->lport - element2->lport);
    }

    if (element1->tel_exist != element2->tel_exist)
    {
        return (element1->tel_exist - element2->tel_exist);
    }

    return (element1->time_mark - element2->time_mark);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CompTelNotifyEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the notify entry list to check existence
 * INPUT    : inlist_element        -- the element in the list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T LLDP_OM_CompTelNotifyEntry(void *inlist_element, void *input_element)
{
    LLDP_TYPE_NotifyTelephone_T   *element1, *element2;

    element1 = (LLDP_TYPE_NotifyTelephone_T *)inlist_element;
    element2 = (LLDP_TYPE_NotifyTelephone_T *)input_element;

    if(element1->time_mark != element2->time_mark
       || element1->lport != element2->lport
       || element1->tel_exist != element2->tel_exist
      )
    {
        return FALSE;
    }

    return TRUE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompNotifyMedRemDataEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the notify entry list to sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompNotifyMedRemDataEntry(void *inlist_element, void *input_element)
{
    LLDP_TYPE_NotifyMedRemData_T   *element1, *element2;

    element1 = *(LLDP_TYPE_NotifyMedRemData_T **)inlist_element;
    element2 = *(LLDP_TYPE_NotifyMedRemData_T **)input_element;

    if(element1->rem_time_mark!= element2->rem_time_mark)
        return (element1->rem_time_mark - element2->rem_time_mark);
    if(element1->rem_local_port_num!= element2->rem_local_port_num)
        return (element1->rem_local_port_num - element2->rem_local_port_num);
    if(element1->rem_index!= element2->rem_index)
        return (element1->rem_index - element2->rem_index);

    return 0;
}

#if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2))
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompNotifyDot3atInfoEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the notify entry list to sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompNotifyDot3atInfoEntry(void *inlist_element, void *input_element)
{
    LLDP_TYPE_NotifyDot3atInfo_T   *element1, *element2;

    element1 = *(LLDP_TYPE_NotifyDot3atInfo_T **)inlist_element;
    element2 = *(LLDP_TYPE_NotifyDot3atInfo_T **)input_element;

    if (element1->time_mark!= element2->time_mark)
        return (element1->time_mark - element2->time_mark);
    else
        return (element1->lport - element2->lport);
}
#endif /* #if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)) */

#if (SYS_CPNT_CN == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CompNotifyCnRemoteChangeEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the notify entry list to check existence
 * INPUT    : inlist_element        -- the element in the list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T LLDP_OM_CompNotifyCnRemoteChangeEntry(void *inlist_element, void *input_element)
{
    LLDP_TYPE_NotifyCnRemoteChange_T   *element1, *element2;

    element1 = (LLDP_TYPE_NotifyCnRemoteChange_T *)inlist_element;
    element2 = (LLDP_TYPE_NotifyCnRemoteChange_T *)input_element;

    if (    (element1->time_mark != element2->time_mark)
         || (element1->lport != element2->lport)
       )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#endif /* #if (SYS_CPNT_CN == TRUE) */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CompNotifyRemChangeEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the notify entry list to check existence
 * INPUT    : inlist_element        -- the element in the list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T LLDP_OM_CompNotifyRemChangeEntry(void *inlist_element, void *input_element)
{
    LLDP_TYPE_NotifyRemChange_T   *element1, *element2;

    element1 = (LLDP_TYPE_NotifyRemChange_T *)inlist_element;
    element2 = (LLDP_TYPE_NotifyRemChange_T *)input_element;

    if (    (element1->time_mark != element2->time_mark)
         || (element1->lport != element2->lport)
       )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SetLocManAddrConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Set local management address configuration entry
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_SetLocManAddrConfig(LLDP_OM_ConfigManAddrEntry_T* input_config)
{
    BOOL_T  result;
    LLDP_OM_ConfigManAddrEntry_T    *new_config = NULL;

    result = FALSE;
    new_config = (LLDP_OM_ConfigManAddrEntry_T *)malloc(sizeof(LLDP_OM_ConfigManAddrEntry_T));
    if(new_config == NULL)
    {
        #if RX_DEBUG_PRINT
        puts("allocate fail");
        #endif
        return result;
    }

    memset(new_config, 0, sizeof(LLDP_OM_ConfigManAddrEntry_T));
    memcpy(new_config, input_config, sizeof(LLDP_OM_ConfigManAddrEntry_T));
    result = L_SORT_LST_Set(&LLDP_OM_LocManAddrConfig,&new_config);

    if(!result)
        free(new_config);

    return result;
}/* End of LLDP_OM_SetLocManAddrConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetLocManAddrConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get local management address configuration entry pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetLocManAddrConfigPtr(LLDP_OM_ConfigManAddrEntry_T** loc_man_addr_config)
{
    return L_SORT_LST_Get(&LLDP_OM_LocManAddrConfig, loc_man_addr_config);
}/* End of LLDP_OM_GetLocManAddrConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextLocManAddrConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next local management address configuration entry pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetNextLocManAddrConfigPtr(LLDP_OM_ConfigManAddrEntry_T** loc_man_addr_config)
{
    return L_SORT_LST_Get_Next(&LLDP_OM_LocManAddrConfig, loc_man_addr_config);

}/* End of LLDP_OM_GetNextLocManAddrConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_DeleteLocManAddrConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete local management address configuration entry
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_DeleteLocManAddrConfig(LLDP_OM_ConfigManAddrEntry_T* del_config)
{
    BOOL_T result;

    result = L_SORT_LST_Delete(&LLDP_OM_LocManAddrConfig, &del_config);

    if(result)
        free(del_config);

    return result;

}/* End of LLDP_OM_DeleteLocManAddrConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRemDataPoolFreeNum
 *-------------------------------------------------------------------------
 * PURPOSE  : Get number of free remote-data entry
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : UI32_T    -- free entry number
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRemDataPoolFreeNum()
{
    return LLDP_OM_RemDataPoolFreeNum;
}/* End of LLDP_OM_GetRemSysPoolFreeNum */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetPortRemDataNum
 *-------------------------------------------------------------------------
 * PURPOSE  : Get number of free remote-data entry in port list
 * INPUT    : lport
 * OUTPUT   : None
 * RETUEN   : UI32_T    -- free entry number
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetPortRemDataNum(UI32_T lport)
{
    return LLDP_OM_RemDataList[lport-1].nbr_of_element;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_FreeRemManAddrList
 *-------------------------------------------------------------------------
 * PURPOSE  : Free the remote-management-address entry from the
 *            remote-management-address sort list to the memory pool
 * INPUT    : list      -- remote man address sort list
 *            num       -- number of entry in the sort list
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static void LLDP_OM_FreeRemManAddrList(L_SORT_LST_List_T* list)
{
    LLDP_OM_RemManAddrEntry_T *tmp;

    while(list->nbr_of_element != 0)
    {
        L_SORT_LST_Remove_1st(list, &tmp);
        free(tmp);
    }
}/* End of  LLDP_OM_FreeRemManAddrList */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_AllocRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Allocate a remote data entry from the memory pool
 * INPUT    : None
 * OUTPUT   : void *rem_data
 * RETUEN   : LLDP_TYPE_RETURN_ERROR
 *            LLDP_TYPE_RETURN_OK
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_AllocRemData(LLDP_OM_RemData_T **rem_data)
{
    if(!L_MPOOL_AllocateBlock(LLDP_OM_RemDataPool, (void **)rem_data))
        return LLDP_TYPE_RETURN_ERROR;
    memset(*rem_data, 0, sizeof(LLDP_OM_RemData_T));
    (*rem_data)->self = *rem_data;
    LLDP_OM_RemDataPoolAllocNum++;
    LLDP_OM_RemDataPoolFreeNum--;

    return LLDP_TYPE_RETURN_OK;


}/* End of LLDP_OM_AllocRemData*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_FreeRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Free a remote data entry to the memory pool
 * INPUT    : free_data        -- remote data entry to be freed
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_FreeRemData(LLDP_OM_RemData_T *free_data)
{
    LLDP_OM_LocalFreeRemData(free_data);
    L_MPOOL_FreeBlock(LLDP_OM_RemDataPool, free_data);
    LLDP_OM_RemDataPoolAllocNum--;
    LLDP_OM_RemDataPoolFreeNum++;


}/* End of LLDP_OM_FreeRemData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_AgeSeqInsert
 *-------------------------------------------------------------------------
 * PURPOSE  : Insert a remote data to the age sequence
 * INPUT    : new_rem_data      -- input remote data
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_AgeSeqInsert(LLDP_OM_RemData_T *new_rem_data)
{
    LLDP_OM_RemData_T   *tmp;
    UI32_T  current_time;

    SYS_TIME_GetSystemUpTimeByTick(&current_time);
    new_rem_data->age_out_time = new_rem_data->rx_info_ttl * LLDP_TYPE_TIME_UNIT + current_time;

    if(LLDP_OM_AgeSeqBeg == NULL)
    {
        #if RX_DEBUG_PRINT
        {
            puts("Age Seq Insert first record");
        }
        #endif
        new_rem_data->age_seq_prev = NULL;
        new_rem_data->age_seq_next = NULL;
        LLDP_OM_AgeSeqBeg = new_rem_data;
        LLDP_OM_AgeSeqEnd = new_rem_data;

        return ;
    }
    tmp = LLDP_OM_AgeSeqBeg;
    while(1)
    {
        if(tmp->age_out_time > new_rem_data->age_out_time )
        {
            new_rem_data->age_seq_prev = tmp->age_seq_prev;
            new_rem_data->age_seq_next = tmp;
            tmp->age_seq_prev = new_rem_data;
            /* insert to head of the Age Seq list */
            if(new_rem_data->age_seq_prev == NULL)
            {
                LLDP_OM_AgeSeqBeg = new_rem_data;
            }
            else /* insert to middle of the Age Seq */
            {
                new_rem_data->age_seq_prev->age_seq_next = new_rem_data;
            }
            break;
        }
        /* insert to the end of the Age Seq*/
        if(tmp->age_seq_next == NULL)
        {
            new_rem_data->age_seq_prev = tmp;
            new_rem_data->age_seq_next = tmp->age_seq_next;
            tmp->age_seq_next = new_rem_data;
            LLDP_OM_AgeSeqEnd = new_rem_data;
            break;
        }
        tmp = tmp->age_seq_next;
    }
    #if RX_DEBUG_PRINT
    {
        puts("Age Seq Insert");
    }
    #endif
    return;

}/* End of LLDP_OM_AgeSeqInsert */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_AgeSeqDelete
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete a remote data from the age sequence
 * INPUT    : rem_data      -- remote data to be deleted
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_AgeSeqDelete(LLDP_OM_RemData_T *del_rem_data)
{
    if(del_rem_data == LLDP_OM_AgeSeqBeg)
    {
        LLDP_OM_AgeSeqBeg = del_rem_data->age_seq_next;
        if (LLDP_OM_AgeSeqBeg != NULL)
        {
            LLDP_OM_AgeSeqBeg->age_seq_prev = NULL;
        }
        return;
    }

    if(del_rem_data == LLDP_OM_AgeSeqEnd)
    {
        LLDP_OM_AgeSeqEnd = del_rem_data->age_seq_prev;
        if (LLDP_OM_AgeSeqEnd != NULL)
        {
            LLDP_OM_AgeSeqEnd->age_seq_next = NULL;
        }
        return;
    }

    del_rem_data->age_seq_prev->age_seq_next = del_rem_data->age_seq_next;
    del_rem_data->age_seq_next->age_seq_prev = del_rem_data->age_seq_prev;
    return;
}/* End of LLDP_OM_AgeSeqDelete */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InsertRemData
 * ------------------------------------------------------------------------
 * PURPOSE  : Insert remote data to DB
 * INPUT    : remote data       -- remote data to be inserted
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_InsertRemData(LLDP_OM_RemData_T *rem_data)
{
    /* 3.2 rearrange rem_data into age_seq */
    rem_data->index = ++LLDP_OM_RemDataIndex;

    if(!L_SORT_LST_Set(&LLDP_OM_RemDataList[ (rem_data->lport-1) ], &rem_data))
    {
        #if RX_DEBUG_PRINT
        puts("Insert to port sort list fail");
        #endif
        return FALSE;
    }

    /* 4. insert rem_data into RemDataAll list */
    if(!L_SORT_LST_Set(&LLDP_OM_RemDataAllList, &rem_data))
    {
        return FALSE;
    }

    LLDP_OM_AgeSeqInsert(rem_data);

    #if 0
    LLDP_OM_HisamInsert(rem_data);
    #endif

    LLDP_OM_Statistics.rem_table_inserts++;
    SYS_TIME_GetSystemUpTimeByTick(&LLDP_OM_Statistics.rem_table_last_change_time);
    LLDP_OM_RemDataTotal++;

    /* 5. notify insert */
    if((rem_data->rem_sys_entry->rem_sys_cap_enabled & (1 << VAL_lldpRemSysCapEnabled_telephone)) &&
       (rem_data->rem_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_MAC_ADDR))
    {
        {
            LLDP_OM_RemManAddrEntry_T *rem_man_addr;
            LLDP_TYPE_NotifyTelephone_T   notify_entry;

            memset(&notify_entry, 0, sizeof(LLDP_TYPE_NotifyTelephone_T));
            notify_entry.lport = rem_data->lport;
            notify_entry.time_mark = rem_data->time_mark;
            memcpy(notify_entry.mac_addr, rem_data->rem_port_id, SYS_ADPT_MAC_ADDR_LEN);
            if(L_SORT_LST_Get_1st(&rem_data->rem_man_addr_list, &rem_man_addr))
            {
                notify_entry.network_addr_subtype = rem_man_addr->rem_man_addr_subtype;
                notify_entry.network_addr_len = rem_man_addr->rem_man_addr_len;
                memcpy(notify_entry.network_addr, rem_man_addr->rem_man_addr, notify_entry.network_addr_len);
                notify_entry.network_addr_ifindex = rem_man_addr->rem_man_addr_if_id;
            }
            notify_entry.tel_exist = TRUE;
            L_LINK_LST_Set(&LLDP_OM_NotifyTelephoneTypeList, &notify_entry, L_LINK_LST_APPEND);
            LLDP_OM_PortStatistics[rem_data->lport - 1].rx_telephone_total++;
        }
    }

#if (LLDP_TYPE_MED == TRUE)
    if(rem_data->lldp_med_device_type != 0)
    {
        LLDP_OM_PortConfig[rem_data->lport - 1].fast_start_count = LLDP_OM_SysConfig.fast_start_repeat_count;
        LLDP_OM_PortConfig[rem_data->lport - 1].something_changed_local = TRUE;
        LLDP_OM_PortConfig[rem_data->lport - 1].med_device_exist = TRUE;
    }

    if((rem_data->lldp_med_device_type != 0) &&
       (LLDP_OM_PortConfig[rem_data->lport - 1].lldp_med_notification == VAL_lldpXMedPortConfigNotifEnable_true))
    {
        LLDP_TYPE_NotifyMedRemData_T *notify_entry = 0;

        notify_entry = (LLDP_TYPE_NotifyMedRemData_T *)malloc(sizeof(LLDP_TYPE_NotifyMedRemData_T));
        if(notify_entry)
        {
            memset(notify_entry, 0, sizeof(LLDP_TYPE_NotifyMedRemData_T));
            notify_entry->rem_time_mark = rem_data->time_mark;
            notify_entry->rem_local_port_num = rem_data->lport;
            notify_entry->rem_index = rem_data->index;
            notify_entry->rem_chassis_id_subtype = rem_data->rem_chassis_id_subtype;
            memcpy(notify_entry->rem_chassis_id, rem_data->rem_chassis_id, rem_data->rem_chassis_id_len);
            notify_entry->rem_chassis_id_len = rem_data->rem_chassis_id_len;
            notify_entry->rem_device_class = rem_data->lldp_med_device_type;
            if(!L_SORT_LST_Set(&LLDP_OM_NotifyMedRemTableChangedList, &notify_entry))
            {
                free(notify_entry);
            }
        }
    }
#endif /* #if (LLDP_TYPE_MED == TRUE) */

#if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2))
    if (rem_data->xdot3_rem_power_entry != NULL)
    {
        LLDP_TYPE_NotifyDot3atInfo_T *notify_entry = NULL;

        notify_entry = (LLDP_TYPE_NotifyDot3atInfo_T *)malloc(sizeof(LLDP_TYPE_NotifyDot3atInfo_T));
        if (notify_entry != NULL)
        {
            memset(notify_entry, 0, sizeof(LLDP_TYPE_NotifyDot3atInfo_T));
            notify_entry->time_mark = rem_data->time_mark;
            notify_entry->lport = rem_data->lport;
            notify_entry->power_type = rem_data->xdot3_rem_power_entry->rem_power_type;
            notify_entry->power_source = rem_data->xdot3_rem_power_entry->rem_power_source;
            notify_entry->power_priority = rem_data->xdot3_rem_power_entry->rem_power_priority;
            notify_entry->pd_requested_power = rem_data->xdot3_rem_power_entry->rem_pd_requested_power;
            notify_entry->pse_allocated_power = rem_data->xdot3_rem_power_entry->rem_pse_allocated_power;
            if (L_SORT_LST_Set(&LLDP_OM_NotifyDot3atInfoReceivedList, &notify_entry) == FALSE)
            {
                free(notify_entry);
            }
        }
    }
#endif /* #if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)) */

#if (SYS_CPNT_DCBX == TRUE)
    if( (rem_data->dcbx_rem_ets_config_entry != NULL) || (rem_data->dcbx_rem_ets_recommend_entry != NULL) ||
        (rem_data->dcbx_rem_pfc_config_entry != NULL) || (rem_data->dcbx_rem_app_pri_list.nbr_of_element != 0))
    {
        LLDP_OM_PortConfig[rem_data->lport - 1].rx_dcbx_peer_num++;
        if (    (LLDP_OM_PortConfig[rem_data->lport - 1].rx_dcbx_peer_num > 1)
             && (LLDP_OM_DcbxTimer[rem_data->lport - 1].enabled == FALSE)
           )
        {   /* start timer for multi-peer detection */
            LLDP_OM_DcbxTimer[rem_data->lport - 1].enabled = TRUE;
            LLDP_OM_DcbxTimer[rem_data->lport - 1].time = 0;
        }

        if (    (rem_data->rx_info_ttl > LLDP_OM_DcbxTimer[rem_data->lport - 1].limit)
             && (LLDP_OM_IsMultiPeerDetected(rem_data->lport) == FALSE)
           )
        {
            LLDP_OM_DcbxTimer[rem_data->lport - 1].limit = rem_data->rx_info_ttl;
        }
    }

    if (    (    (rem_data->dcbx_rem_ets_config_entry != NULL)
              || (rem_data->dcbx_rem_ets_recommend_entry != NULL)
            )
         && (LLDP_OM_IsMultiPeerDetected(rem_data->lport) == FALSE)
       )
    {
        LLDP_TYPE_NotifyDcbxEtsInfo_T notify_entry;

        memset(&notify_entry, 0, sizeof(LLDP_TYPE_NotifyDcbxEtsInfo_T));
        notify_entry.time_mark = rem_data->time_mark;
        notify_entry.lport = rem_data->lport;
        notify_entry.index = rem_data->index;
        notify_entry.is_delete = FALSE;
        notify_entry.rem_recommend_rcvd = (rem_data->dcbx_rem_ets_recommend_entry != NULL) ? TRUE : FALSE;
        if (rem_data->dcbx_rem_ets_config_entry != NULL)
        {
            notify_entry.rem_config_willing = rem_data->dcbx_rem_ets_config_entry->rem_willing;
            notify_entry.rem_config_cbs = rem_data->dcbx_rem_ets_config_entry->rem_cbs;
            notify_entry.rem_config_max_tc = rem_data->dcbx_rem_ets_config_entry->rem_max_tc;
            memcpy(notify_entry.rem_config_pri_assign_table, rem_data->dcbx_rem_ets_config_entry->rem_pri_assign_table, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
            memcpy(notify_entry.rem_config_tc_bandwidth_table, rem_data->dcbx_rem_ets_config_entry->rem_tc_bandwidth_table, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
            memcpy(notify_entry.rem_config_tsa_assign_table, rem_data->dcbx_rem_ets_config_entry->rem_tsa_assign_table, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);
        }
        if (rem_data->dcbx_rem_ets_recommend_entry != NULL)
        {
            memcpy(notify_entry.rem_recommend_pri_assign_table, rem_data->dcbx_rem_ets_recommend_entry->rem_pri_assign_table, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
            memcpy(notify_entry.rem_recommend_tc_bandwidth_table, rem_data->dcbx_rem_ets_recommend_entry->rem_tc_bandwidth_table, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
            memcpy(notify_entry.rem_recommend_tsa_assign_table, rem_data->dcbx_rem_ets_recommend_entry->rem_tsa_assign_table, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);
        }
        L_LINK_LST_Set(&LLDP_OM_NotifyDcbxRemEtsChangedList, &notify_entry, L_LINK_LST_APPEND);
    }

    if (    (rem_data->dcbx_rem_pfc_config_entry != NULL)
         && (LLDP_OM_IsMultiPeerDetected(rem_data->lport) == FALSE)
       )
    {
        LLDP_TYPE_NotifyDcbxPfcInfo_T notify_entry;

        memset(&notify_entry, 0, sizeof(LLDP_TYPE_NotifyDcbxPfcInfo_T));
        notify_entry.time_mark = rem_data->time_mark;
        notify_entry.lport = rem_data->lport;
        notify_entry.index = rem_data->index;
        notify_entry.is_delete = FALSE;
        memcpy(notify_entry.rem_mac, rem_data->dcbx_rem_pfc_config_entry->rem_mac, 6);
        notify_entry.rem_willing = rem_data->dcbx_rem_pfc_config_entry->rem_willing;
        notify_entry.rem_mbc = rem_data->dcbx_rem_pfc_config_entry->rem_mbc;
        notify_entry.rem_cap = rem_data->dcbx_rem_pfc_config_entry->rem_cap;
        notify_entry.rem_enable = rem_data->dcbx_rem_pfc_config_entry->rem_enable;
        L_LINK_LST_Set(&LLDP_OM_NotifyDcbxRemPfcChangedList, &notify_entry, L_LINK_LST_APPEND);
    }
#endif /* #if (SYS_CPNT_DCBX == TRUE) */

#if (SYS_CPNT_CN == TRUE)
    if (tlv_change_detect[LLDP_OM_DETECT_CHANGE_CN_TLV] == TRUE)
    {
        LLDP_TYPE_NotifyCnRemoteChange_T notify_entry;

        memset(&notify_entry, 0, sizeof(LLDP_TYPE_NotifyCnRemoteChange_T));
        notify_entry.time_mark = rem_data->time_mark;
        notify_entry.lport = rem_data->lport;
        notify_entry.neighbor_num = LLDP_OM_RemDataList[rem_data->lport-1].nbr_of_element;
        if (rem_data->xdot1_rem_cn_entry == NULL)
        {
            notify_entry.cnpv_indicators = 0;
            notify_entry.ready_indicators = 0;
        }
        else
        {
            notify_entry.cnpv_indicators = rem_data->xdot1_rem_cn_entry->cnpv_indicators;
            notify_entry.ready_indicators = rem_data->xdot1_rem_cn_entry->ready_indicators;
        }
        L_LINK_LST_Set(&LLDP_OM_NotifyCnRemoteChangeList, &notify_entry, L_LINK_LST_APPEND);
        tlv_change_detect[LLDP_OM_DETECT_CHANGE_CN_TLV] = FALSE;
    }
#endif /* #if (SYS_CPNT_CN == TRUE) */

    {
        LLDP_TYPE_NotifyRemChange_T notify_entry;

        memset(&notify_entry, 0, sizeof(LLDP_TYPE_NotifyRemChange_T));
        notify_entry.time_mark = rem_data->time_mark;
        notify_entry.lport = rem_data->lport;
        L_LINK_LST_Set(&LLDP_OM_NotifyRemChangeList, &notify_entry, L_LINK_LST_APPEND);
    }

    return TRUE;
}/* End of LLDP_OM_InsertRemoteData */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_DeleteRemData_EX
 * ------------------------------------------------------------------------
 * PURPOSE  : Delete remote data from the DB
 * INPUT    : new_rem_data_p -- new remote data
 *            del_rem_data_p -- old remote data to be deleted
 *            reason         -- why to delete remote data
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_DeleteRemData_EX(
        LLDP_OM_RemData_T *new_rem_data_p,
        LLDP_OM_RemData_T *del_rem_data_p,
        UI32_T             reason)
{
    LLDP_OM_RemData_T   *tmp_rem_data_p, *found_data_p;
    UI32_T              lport, current_time;

    lport = del_rem_data_p->lport;

    /* 1. remove rem_data from age_seq */
    LLDP_OM_AgeSeqDelete(del_rem_data_p);

    /* 2. remove it from the port list */
    if (L_SORT_LST_Delete(&LLDP_OM_RemDataList[(lport-1)], &del_rem_data_p))
    {
        #if RX_DEBUG_PRINT
        printf("Delete rem data->rem_index:%ld\n", del_rem_data_p->index);
        #endif
    }
    else
    {
        #if RX_DEBUG_PRINT
        puts("Delete rem data from port list fail");
        #endif
    }

    /* 3. remove it from the total list */
    if (!L_SORT_LST_Delete(&LLDP_OM_RemDataAllList, &del_rem_data_p))
    {
        #if RX_DEBUG_PRINT
        puts("Delete rem data from port list fail");
        #endif
    }

    if (reason != LLDP_OM_DELETE_REASON_UPDATE)
    {
        LLDP_TYPE_NotifyRemChange_T notify_entry;

        memset(&notify_entry, 0, sizeof(LLDP_TYPE_NotifyRemChange_T));
        notify_entry.time_mark = del_rem_data_p->time_mark;
        notify_entry.lport = lport;
        L_LINK_LST_Set(&LLDP_OM_NotifyRemChangeList, &notify_entry, L_LINK_LST_APPEND);

        /* notify for ADD */
        if (del_rem_data_p->rem_sys_entry->rem_sys_cap_enabled & (1 << VAL_lldpRemSysCapEnabled_telephone))
        {
            LLDP_TYPE_NotifyTelephone_T notify_entry;
            LLDP_OM_RemManAddrEntry_T *rem_man_addr;

            memset(&notify_entry, 0, sizeof(LLDP_TYPE_NotifyTelephone_T));
            notify_entry.lport = lport;
            SYS_TIME_GetSystemUpTimeByTick(&current_time);
            notify_entry.time_mark = current_time;
            if(del_rem_data_p->rem_chassis_id_subtype == LLDP_TYPE_CHASSIS_ID_SUBTYPE_MAC_ADDR)
                memcpy(notify_entry.mac_addr, del_rem_data_p->rem_chassis_id, 6);
            if(L_SORT_LST_Get_1st(&del_rem_data_p->rem_man_addr_list, &rem_man_addr))
            {
                notify_entry.network_addr_subtype = rem_man_addr->rem_man_addr_subtype;
                notify_entry.network_addr_len = rem_man_addr->rem_man_addr_len;
                memcpy(notify_entry.network_addr, rem_man_addr->rem_man_addr, notify_entry.network_addr_len);
                notify_entry.network_addr_ifindex = rem_man_addr->rem_man_addr_if_id;
            }

            notify_entry.tel_exist = FALSE;
            L_LINK_LST_Set(&LLDP_OM_NotifyTelephoneTypeList, &notify_entry, L_LINK_LST_APPEND);
            LLDP_OM_PortStatistics[lport - 1].rx_telephone_total--;
        }

#if (LLDP_TYPE_MED == TRUE)
        if (    (del_rem_data_p->lldp_med_device_type != 0)
             && (LLDP_OM_PortConfig[lport - 1].lldp_med_notification == VAL_lldpXMedPortConfigNotifEnable_true)
           )
        {
            LLDP_TYPE_NotifyMedRemData_T *notify_entry = 0;

            notify_entry = (LLDP_TYPE_NotifyMedRemData_T *)malloc(sizeof(LLDP_TYPE_NotifyMedRemData_T));
            if (notify_entry != NULL)
            {
                memset(notify_entry, 0, sizeof(LLDP_TYPE_NotifyMedRemData_T));
                notify_entry->rem_time_mark = del_rem_data_p->time_mark;
                notify_entry->rem_local_port_num = lport;
                notify_entry->rem_index = del_rem_data_p->index;
                notify_entry->rem_chassis_id_subtype = del_rem_data_p->rem_chassis_id_subtype;
                memcpy(notify_entry->rem_chassis_id, del_rem_data_p->rem_chassis_id, del_rem_data_p->rem_chassis_id_len);
                notify_entry->rem_chassis_id_len = del_rem_data_p->rem_chassis_id_len;
                notify_entry->rem_device_class = del_rem_data_p->lldp_med_device_type;
                if (L_SORT_LST_Set(&LLDP_OM_NotifyMedRemTableChangedList, &notify_entry) == FALSE)
                {
                    free(notify_entry);
                }
            }
        }
#endif /* #if (LLDP_TYPE_MED == TRUE) */
    }

#if (SYS_CPNT_DCBX == TRUE)
    if (    (del_rem_data_p->dcbx_rem_ets_config_entry != NULL)
         || (del_rem_data_p->dcbx_rem_ets_recommend_entry != NULL)
         || (del_rem_data_p->dcbx_rem_pfc_config_entry != NULL)
         || (del_rem_data_p->dcbx_rem_app_pri_list.nbr_of_element != 0)
       )
    {
        LLDP_TYPE_NotifyDcbxEtsInfo_T notify_ets_entry;
        LLDP_TYPE_NotifyDcbxPfcInfo_T notify_pfc_entry;

        LLDP_OM_PortConfig[lport-1].rx_dcbx_peer_num--;

        if (reason != LLDP_OM_DELETE_REASON_UPDATE)
        {
            found_data_p = NULL;
            if (L_SORT_LST_Get_1st(&LLDP_OM_RemDataList[lport-1], &tmp_rem_data_p)
                    == TRUE)
            {
                if (    (tmp_rem_data_p->dcbx_rem_ets_config_entry != NULL)
                     || (tmp_rem_data_p->dcbx_rem_ets_recommend_entry != NULL)
                     || (tmp_rem_data_p->dcbx_rem_pfc_config_entry != NULL)
                   )
                {
                    found_data_p = tmp_rem_data_p;
                }

                while (L_SORT_LST_Get_Next(&LLDP_OM_RemDataList[lport-1],
                        &tmp_rem_data_p) == TRUE)
                {
                    if (    (tmp_rem_data_p->dcbx_rem_ets_config_entry != NULL)
                         || (tmp_rem_data_p->dcbx_rem_ets_recommend_entry != NULL)
                         || (tmp_rem_data_p->dcbx_rem_pfc_config_entry != NULL)
                       )
                    {
                        found_data_p = tmp_rem_data_p;
                    }
                }
            }

            if (LLDP_OM_PortConfig[lport-1].rx_dcbx_peer_num == 0)
            {
                if (    (del_rem_data_p->dcbx_rem_ets_config_entry != NULL)
                     || (del_rem_data_p->dcbx_rem_ets_recommend_entry != NULL)
                   )
                {
                    LLDP_OM_SET_ETS_NOTIFY_ENTRY(TRUE, (&notify_ets_entry), del_rem_data_p);
                    L_LINK_LST_Set(&LLDP_OM_NotifyDcbxRemEtsChangedList, &notify_ets_entry, L_LINK_LST_APPEND);
                }

                if (del_rem_data_p->dcbx_rem_pfc_config_entry != NULL)
                {
                    LLDP_OM_SET_PFC_NOTIFY_ENTRY(TRUE, (&notify_pfc_entry), del_rem_data_p);
                    L_LINK_LST_Set(&LLDP_OM_NotifyDcbxRemPfcChangedList, &notify_pfc_entry, L_LINK_LST_APPEND);
                }
            }
            else if (LLDP_OM_PortConfig[lport-1].rx_dcbx_peer_num == 1)
            {
                /* multi-peer condition disappear */
                LLDP_OM_DcbxTimer[lport-1].enabled = FALSE;
                if (found_data_p != NULL)
                {
                    LLDP_OM_DcbxTimer[lport-1].limit = found_data_p->rx_info_ttl;
                }
            }

            if (    (LLDP_OM_IsMultiPeerDetected(lport) == FALSE)
                 && (found_data_p != NULL)
               )
            {
                if (    (found_data_p->dcbx_rem_ets_config_entry != NULL)
                     || (found_data_p->dcbx_rem_ets_recommend_entry != NULL)
                   )
                {
                    LLDP_OM_SET_ETS_NOTIFY_ENTRY(FALSE, (&notify_ets_entry), found_data_p);
                    L_LINK_LST_Set(&LLDP_OM_NotifyDcbxRemEtsChangedList, &notify_ets_entry, L_LINK_LST_APPEND);
                }

                if (found_data_p->dcbx_rem_pfc_config_entry != NULL)
                {
                    LLDP_OM_SET_PFC_NOTIFY_ENTRY(FALSE, (&notify_pfc_entry), found_data_p);
                    L_LINK_LST_Set(&LLDP_OM_NotifyDcbxRemPfcChangedList, &notify_pfc_entry, L_LINK_LST_APPEND);
                }
            }
        }
        else
        {   /* reason == LLDP_OM_DELETE_REASON_UPDATE
             * notify delete message if new remote data doesn't contain pfc/ets
             * data
             */
            if (  (NULL != new_rem_data_p)
                &&(LLDP_OM_IsMultiPeerDetected(lport) == FALSE)
               )
            {
                if (new_rem_data_p->dcbx_rem_ets_config_entry == NULL)
                {
                    LLDP_OM_SET_ETS_NOTIFY_ENTRY(TRUE, (&notify_ets_entry), del_rem_data_p);
                    L_LINK_LST_Set(&LLDP_OM_NotifyDcbxRemEtsChangedList, &notify_ets_entry, L_LINK_LST_APPEND);
                }

                if (new_rem_data_p->dcbx_rem_pfc_config_entry == NULL)
                {
                    LLDP_OM_SET_PFC_NOTIFY_ENTRY(TRUE, (&notify_pfc_entry), del_rem_data_p);
                    L_LINK_LST_Set(&LLDP_OM_NotifyDcbxRemPfcChangedList, &notify_pfc_entry, L_LINK_LST_APPEND);
                }
            }
        }
    }
#endif /* #if (SYS_CPNT_DCBX == TRUE) */

#if (SYS_CPNT_CN == TRUE)
    if (    (del_rem_data_p->xdot1_rem_cn_entry != NULL)
         && (reason != LLDP_OM_DELETE_REASON_UPDATE)
       )
    {
        LLDP_TYPE_NotifyCnRemoteChange_T notify_entry;

        memset(&notify_entry, 0, sizeof(LLDP_TYPE_NotifyCnRemoteChange_T));
        notify_entry.time_mark = del_rem_data_p->time_mark;
        notify_entry.lport = lport;
        notify_entry.neighbor_num = LLDP_OM_RemDataList[lport-1].nbr_of_element;
        notify_entry.cnpv_indicators = 0;
        notify_entry.ready_indicators = 0;
        L_LINK_LST_Set(&LLDP_OM_NotifyCnRemoteChangeList, &notify_entry, L_LINK_LST_APPEND);
    }
#endif

    /* 4. Free remote data */
    LLDP_OM_FreeRemData(del_rem_data_p);

    LLDP_OM_Statistics.rem_table_deletes++;
    SYS_TIME_GetSystemUpTimeByTick(&LLDP_OM_Statistics.rem_table_last_change_time);
    LLDP_OM_RemDataTotal--;

    /* check if need to notify */
    if(LLDP_OM_PortConfig[(lport - 1)].notification_enable == VAL_lldpPortConfigNotificationEnable_true)
    {
        LLDP_OM_SysConfig.something_changed_remote = TRUE;
    }

    /* check if there is any LLDP-MED device in the port */
    LLDP_OM_PortConfig[lport - 1].med_device_exist = FALSE;
    if(L_SORT_LST_Get_1st(&LLDP_OM_RemDataList[lport - 1], &tmp_rem_data_p))
    {
        do
        {
            if(tmp_rem_data_p->lldp_med_cap_sup != 0)
            {
                LLDP_OM_PortConfig[lport - 1].med_device_exist = TRUE;
            }
        }
        while(!LLDP_OM_PortConfig[lport - 1].med_device_exist &&
              L_SORT_LST_Get_Next(&LLDP_OM_RemDataList[lport - 1], &tmp_rem_data_p));
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_DeleteRemData
 * ------------------------------------------------------------------------
 * PURPOSE  : Delete remote data from the DB
 * INPUT    : rem_data -- remote data to be deleted
 *            reason   -- why to delete remote data
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_DeleteRemData(LLDP_OM_RemData_T *delete_rem_data, UI32_T reason)
{
    LLDP_OM_DeleteRemData_EX(NULL, delete_rem_data, reason);
}
/* End of LLDP_OM_DeleteRemData */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_DeleteRemDataByPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Delete remote data from DB by a specified port
 * INPUT    : lport         -- a specified port
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_DeleteRemDataByPort(UI32_T lport)
{
    LLDP_OM_RemData_T *tmp_rem_data;

    if ((lport < 1) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return;
    }

    while(LLDP_OM_RemDataList[lport-1].nbr_of_element != 0)
    {
        L_SORT_LST_Get_1st(&LLDP_OM_RemDataList[lport-1], &tmp_rem_data);
        LLDP_OM_DeleteRemData(tmp_rem_data, LLDP_OM_DELETE_REASON_PERPORTDISABLED);
    }

    return ;
}/* End of LLDP_OM_DeleteRemDataByPort */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_RemDataExist
 * ------------------------------------------------------------------------
 * PURPOSE  : Check whether the rem_data is in hisam table
 * INPUT    : rem_data          -- remote data to be checked (it will use the
 *                                 chassis_id and port_id field of the remote data)
 * OUTPUT   : void *ptr
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_RemDataExist(LLDP_OM_RemData_T *rem_data, LLDP_OM_RemData_T **ptr)
{

    /* Get the record in hash as old_rem_data */
    #if 1
    {
        LLDP_OM_RemData_T   *rem_data_in_list;
        BOOL_T  result = FALSE;

        if(!L_SORT_LST_Get_1st(&LLDP_OM_RemDataAllList, &rem_data_in_list))
        {
            return FALSE;
        }
        do
        {
            /* compare chassis id */
            if((rem_data_in_list->rem_chassis_id_len == rem_data->rem_chassis_id_len) &&
               (rem_data_in_list->rem_chassis_id_subtype == rem_data->rem_chassis_id_subtype) &&
               (memcmp(rem_data_in_list->rem_chassis_id, rem_data->rem_chassis_id, rem_data_in_list->rem_chassis_id_len) == 0) &&
               (memcmp(rem_data_in_list->rem_port_id,rem_data->rem_port_id,rem_data_in_list->rem_port_id_len)==0)&&
               (rem_data_in_list->rem_port_id_len == rem_data->rem_port_id_len) &&
               (rem_data_in_list->rem_port_id_subtype == rem_data->rem_port_id_subtype))
            {
                *ptr = rem_data_in_list;
                return TRUE;
            }

        }while(L_SORT_LST_Get_Next(&LLDP_OM_RemDataAllList, &rem_data_in_list));

        return result;
    }
    #endif
    #if 0
    {
        static LLDP_OM_RemData_T    tmp;
        memset(&tmp, 0, sizeof(LLDP_OM_RemData_T));

        /* what tmp will return if msap_id is not in hash_table */
        if(L_HISAM_GetRecord(&LLDP_OM_HisamDesc, 0, (UI8_T*)rem_data, (UI8_T *)&tmp))
        {
            *ptr = (LLDP_OM_RemData_T *)tmp.self;
            #if RX_DEBUG_PRINT
            puts("Get Recrod");
            #endif
            return TRUE;
        }
        else
        {
            #if RX_DEBUG_PRINT
            puts("Record isn't existed");
            #endif
            return FALSE;
        }
    }
    #endif

}/* End of LLDP_OM_RemDataExist */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetSysConfigEntryPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the system configuration entry
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : LLDP_OM_SysConfigEntry_T *
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LLDP_OM_SysConfigEntry_T* LLDP_OM_GetSysConfigEntryPtr()
{
    return &LLDP_OM_SysConfig;
}/* End of LLDP_OM_GetSysConfigEntryPtr */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetPortConfigEntryPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the specified port configuration
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : LLDP_OM_PortConfigEntry_T*
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LLDP_OM_PortConfigEntry_T* LLDP_OM_GetPortConfigEntryPtr(UI32_T lport)
{
    return &LLDP_OM_PortConfig[lport-1];
}/* End of LLDP_OM_GetPortConfigEntryPtr */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetStatisticsPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the global statistics information
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : LLDP_OM_Statistics_T*
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LLDP_OM_Statistics_T* LLDP_OM_GetStatisticsPtr()
{
    return &LLDP_OM_Statistics;
}/* End of LLDP_OM_GetStatisticsPtr */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetPortStatisticsEntryPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the specified port statistics information
 * INPUT    : lport         -- a specified port
 * OUTPUT   : None
 * RETUEN   : LLDP_OM_PortStatistics_T*
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LLDP_OM_PortStatistics_T* LLDP_OM_GetPortStatisticsEntryPtr(UI32_T lport)
{
    return &LLDP_OM_PortStatistics[lport - 1];
}/* End of LLDP_OM_GetPortStatisticsEntryPtr */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRemData
 * ------------------------------------------------------------------------
 * PURPOSE  : Find next remote data
 * INPUT    : lport             -- a specified port
 *            index             -- a specified index
 * OUTPUT   : get_data
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetRemData(UI32_T lport, UI32_T index, LLDP_OM_RemData_T **get_data)
{
    L_SORT_LST_List_T   *list;
    LLDP_OM_RemData_T   *rem_data;

    if ((lport == 0) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
        return FALSE;

    list = &LLDP_OM_RemDataList[(lport - 1)];

    if(L_SORT_LST_Get_1st(list, &rem_data))
    {
        do{
            if(index == rem_data->index)
            {
                *get_data = rem_data;
                return TRUE;
            }
        }while(L_SORT_LST_Get_Next(list, &rem_data));
    }
    return FALSE;
}/*End of LLDP_OM_GetRemData*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextRemDataByPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Find next remote data on a port
 * INPUT    : lport                 -- a specified port (fix)
 *            (*rem_data_pp)->index -- a specified index
 * OUTPUT   : get_data
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetNextRemDataByPort(UI32_T lport, LLDP_OM_RemData_T **rem_data_pp)
{
    if (LLDP_OM_RemDataTotal == 0 || lport == 0 || lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        return FALSE;
    }

    return L_SORT_LST_Get_Next(&LLDP_OM_RemDataList[lport - 1], rem_data_pp);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRemDataWithTimeMark
 * ------------------------------------------------------------------------
 * PURPOSE  : Find remote data with timemark, lport and index
 * INPUT    : (*rem_data_pp)->time_mark  -- a specified timestamp
 *            (*rem_data_pp)->lport      -- a specified port
 *            (*rem_data_pp)->index      -- a specified index
 * OUTPUT   : rem_data_pp
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetRemDataWithTimeMark(LLDP_OM_RemData_T **rem_data_pp)
{
    return L_SORT_LST_Get(&LLDP_OM_RemDataAllList, rem_data_pp);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextRemDataWithTimeMark
 * ------------------------------------------------------------------------
 * PURPOSE  : Find next remote data
 * INPUT    : (*rem_data_pp)->time_mark  -- a specified timestamp
 *            (*rem_data_pp)->lport      -- a specified port
 *            (*rem_data_pp)->index      -- a specified index
 * OUTPUT   : rem_data_pp
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetNextRemDataWithTimeMark(LLDP_OM_RemData_T **rem_data_pp)
{
    return L_SORT_LST_Get_Next(&LLDP_OM_RemDataAllList, rem_data_pp);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InitAllPool
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize all memory pool
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_InitAllPool(void)
{
    LLDP_OM_RemDataPool     = L_MPOOL_CreateBlockPool(sizeof(LLDP_OM_RemData_T), LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA);
    LLDP_OM_RemDataPoolFreeNum = LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA;
    LLDP_OM_RemDataPoolAllocNum = 0;

}/* End of LLDP_OM_InitAllPool */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InitHisam
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the hisam table
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_InitHisam(void)
{
    /* initialize key definition */
    LLDP_OM_Keydef.field_number = 2;                                /* just one key: msap_id*/
    LLDP_OM_Keydef.offset[0] = 0;                                   /* the msap_id's pos is at the start of the record*/
    LLDP_OM_Keydef.length[0] = LLDP_TYPE_MAX_CHASSIS_ID_LENGTH;     /* length of chassis id */
    LLDP_OM_Keydef.offset[1] = LLDP_TYPE_MAX_CHASSIS_ID_LENGTH;
    LLDP_OM_Keydef.length[1] = LLDP_TYPE_MAX_PORT_ID_LENGTH;

    /* initialize hash handle */
    LLDP_OM_HisamDesc.total_record_nbr = LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA;
    LLDP_OM_HisamDesc.total_index_nbr = 32;
    LLDP_OM_HisamDesc.total_hash_nbr = 256;
    LLDP_OM_HisamDesc.record_length = sizeof(LLDP_OM_RemData_T);
    LLDP_OM_HisamDesc.hash_depth = 4;
    LLDP_OM_HisamDesc.N1 = 4;
    LLDP_OM_HisamDesc.N2 = 32;

    /* create hash table */
    L_HISAM_Create(&LLDP_OM_HisamDesc, 1/*number of key*/, &LLDP_OM_Keydef);
}/* End of LLDP_OM_InitHisam */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateRemManAddrList
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the remote management address sort list
 * INPUT    : list          -- the list to be initialized
 *            count         -- the max number of entry in the sort list
 * OUTPUT   : TRUE/FALSE
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateRemManAddrList(L_SORT_LST_List_T *list, UI32_T count)
{
    return L_SORT_LST_Create(list,
                             count,
                             sizeof(UI8_T*),
                             LLDP_OM_SortCompRemManAddrEntry);
}/* End of LLDP_OM_CreateRemManAddrList */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InitRemDataSortList
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the remote data sort list
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void LLDP_OM_InitRemDataSortList()
{
    UI32_T  index;

    for(index = 0; index < SYS_ADPT_TOTAL_NBR_OF_LPORT; index++)
    {
        L_SORT_LST_Create(&LLDP_OM_RemDataList[index],
                          LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT,
                          sizeof(LLDP_OM_RemData_T *),
                          LLDP_OM_SortCompRemData);
    }
}/* End of LLDP_OM_InitRemDataSortList*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InitRemDataAllList
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the remote data sort list
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void LLDP_OM_InitRemDataAllList()
{
    L_SORT_LST_Create(&LLDP_OM_RemDataAllList,
                      LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA,
                      sizeof(LLDP_OM_RemData_T *),
                      LLDP_OM_SortCompRemDataMib);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_Init
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_Init(void)
{
    LLDP_OM_InitAllPool();
    #if 0
    LLDP_OM_InitHisam();
    #endif
    LLDP_OM_InitRemDataSortList();
    LLDP_OM_InitRemDataAllList();


    /* Create local management address sort list */
    L_SORT_LST_Create(&LLDP_OM_LocManAddrConfig,
                      LLDP_TYPE_MAX_NUM_OF_LOC_MAN_ADDR_ENTRY,
                      sizeof(LLDP_OM_ConfigManAddrEntry_T*),
                      LLDP_OM_SortCompLocManAddrEntry);

    /* Create notify list */
    L_LINK_LST_Create(&LLDP_OM_NotifyTelephoneTypeList,
                      LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA,
                      sizeof(LLDP_TYPE_NotifyTelephone_T),
                      LLDP_OM_CompTelNotifyEntry,
                      FALSE);

    L_SORT_LST_Create(&LLDP_OM_NotifyTelephoneAddrChangedList,
                      LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA,
                      sizeof(LLDP_TYPE_NotifyTelephone_T *),
                      LLDP_OM_SortCompNotifyEntry);

    L_SORT_LST_Create(&LLDP_OM_NotifyMedRemTableChangedList,
                      LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA,
                      sizeof(LLDP_TYPE_NotifyMedRemData_T *),
                      LLDP_OM_SortCompNotifyMedRemDataEntry);

#if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2))
    L_SORT_LST_Create(&LLDP_OM_NotifyDot3atInfoReceivedList,
                      LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA,
                      sizeof(LLDP_TYPE_NotifyDot3atInfo_T *),
                      LLDP_OM_SortCompNotifyDot3atInfoEntry);
#endif

#if(SYS_CPNT_DCBX == TRUE)
    L_LINK_LST_Create(&LLDP_OM_NotifyDcbxRemEtsChangedList,
                      LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA,
                      sizeof(LLDP_TYPE_NotifyDcbxEtsInfo_T),
                      LLDP_OM_CompNotifyDcbxRemEtsEntry,
                      FALSE);

    L_LINK_LST_Create(&LLDP_OM_NotifyDcbxRemPfcChangedList,
                      LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA,
                      sizeof(LLDP_TYPE_NotifyDcbxPfcInfo_T),
                      LLDP_OM_CompNotifyDcbxRemPfcEntry,
                      FALSE);

    LLDP_OM_InitDcbxTimer();
#endif

#if (SYS_CPNT_CN == TRUE)
    L_LINK_LST_Create(&LLDP_OM_NotifyCnRemoteChangeList,
                      LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA,
                      sizeof(LLDP_TYPE_NotifyCnRemoteChange_T),
                      LLDP_OM_CompNotifyCnRemoteChangeEntry,
                      FALSE);
#endif

    L_LINK_LST_Create(&LLDP_OM_NotifyRemChangeList,
                      LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA,
                      sizeof(LLDP_TYPE_NotifyRemChange_T),
                      LLDP_OM_CompNotifyRemChangeEntry,
                      FALSE);

    /* initial other statistic data storage */
    memset(LLDP_OM_PortStatistics, 0, sizeof(LLDP_OM_PortStatistics_T) * LLDP_TYPE_MAX_NUM_OF_LPORT);
    memset(&LLDP_OM_Statistics, 0, sizeof(LLDP_OM_Statistics_T));

    LLDP_OM_RemDataTotal = 0;
    LLDP_OM_RemDataIndex = 0;
    /* initial config varialbe */
}/* End of LLDP_OM_Init */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the semaphore for LLDP objects
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_InitSemaphore(void)
{
    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO,
            &LLDP_OM_SemId) != SYSFUN_OK)
    {
        printf("\n%s: get lldp om sem id fail.\n", __FUNCTION__);
    }

    return;
}/* End of LLDP_OM_InitSemaphore */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_ResetAll
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset all dynamic status of each port and the clean the database
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_ResetAll()
{
    /* Delete all rem data*/
    UI32_T  index, current_time;


    while(LLDP_OM_RemDataTotal != 0)
    {
        LLDP_OM_DeleteRemData(LLDP_OM_AgeSeqBeg, LLDP_OM_DELETE_REASON_GLOBALDISABLED);
    }

    /* reset system statistics */
    memset(&LLDP_OM_Statistics, 0, sizeof(LLDP_OM_Statistics_T));

    /* reset all port status and statistics */
    for(index = 0; index < SYS_ADPT_TOTAL_NBR_OF_LPORT; index++)
    {
        LLDP_OM_PortConfig[index].reinit_flag = FALSE;
        LLDP_OM_PortConfig[index].something_changed_local = FALSE;
        LLDP_OM_PortConfig[index].something_changed_remote = FALSE;
        LLDP_OM_PortConfig[index].transfer_timer = 0;
        LLDP_OM_PortConfig[index].tx_delay_timer = 0;
        SYS_TIME_GetSystemUpTimeByTick(&current_time);
        LLDP_OM_PortConfig[index].reinit_delay_timer = current_time + (LLDP_OM_SysConfig.reinit_delay * LLDP_TYPE_TIME_UNIT);/*xiongyu 20081211*/
        LLDP_OM_PortConfig[index].notify_timer = 0;
        LLDP_OM_PortConfig[index].fast_start_count = 0;
        LLDP_OM_PortConfig[index].med_device_exist = FALSE;

        memset(&LLDP_OM_PortStatistics[index], 0, sizeof(LLDP_OM_PortStatistics_T));
    }
    LLDP_OM_RemDataIndex = 0;
    return ;
}/* End of LLDP_OM_ResetAll*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_ResetPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset all dynamic status of each port and the clean the database
 * INPUT    : lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_ResetPort(UI32_T   lport)
{
    UI32_T current_time;

    /* reset all port status and statistics */
    LLDP_OM_PortConfig[lport - 1].reinit_flag = FALSE;
    LLDP_OM_PortConfig[lport - 1].something_changed_local = FALSE;
    LLDP_OM_PortConfig[lport - 1].something_changed_remote = FALSE;
    LLDP_OM_PortConfig[lport - 1].transfer_timer = 0;
    LLDP_OM_PortConfig[lport - 1].tx_delay_timer = 0;
    SYS_TIME_GetSystemUpTimeByTick(&current_time);
    LLDP_OM_PortConfig[lport - 1].reinit_delay_timer = current_time + (LLDP_OM_SysConfig.reinit_delay * LLDP_TYPE_TIME_UNIT);/*xiongyu 20081211*/
    LLDP_OM_PortConfig[lport - 1].notify_timer = 0;
    LLDP_OM_PortConfig[lport - 1].med_device_exist = FALSE;
    LLDP_OM_PortConfig[lport - 1].fast_start_count = 0;

    memset(&LLDP_OM_PortStatistics[lport - 1], 0, sizeof(LLDP_OM_PortStatistics_T));
    return ;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_ResetPortStatistics
 *-------------------------------------------------------------------------
 * PURPOSE  : Reset port statistics
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_ResetPortStatistics(UI32_T lport)
{
    memset(&LLDP_OM_PortStatistics[lport - 1], 0, sizeof(LLDP_OM_PortStatistics_T));
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_EnterCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical section before access om
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   The task priority of the caller is raised to
 *              SYS_BLD_RAISE_TO_HIGH_PRIORITY.
 *-------------------------------------------------------------------------
 */
void  LLDP_OM_EnterCriticalSection(void)
{
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
    return;
}/* End of LLDP_OM_EnterCriticalSection */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_LeaveCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical section after access om
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   The task priority of the caller is set to the original
 *              task priority.
 *-------------------------------------------------------------------------
 */
void  LLDP_OM_LeaveCriticalSection(void)
{
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
}/* End of LLDP_OM_LeaveCriticalSection */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_AgeOutRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Age out remote data when the timer expired
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_AgeOutRemData(void)
{
    LLDP_OM_RemData_T  *tmp;
    UI32_T time;

    if(LLDP_OM_RemDataTotal == 0)
    {
        return;
    }
    tmp = LLDP_OM_AgeSeqBeg;
    SYS_TIME_GetSystemUpTimeByTick(&time);
    while(tmp != NULL)
    {
        if(tmp->age_out_time <= time)
        {
            LLDP_OM_RemData_T *deltmp;

            deltmp = tmp;
            tmp = tmp->age_seq_next;

            LLDP_OM_PortStatistics[deltmp->lport-1].rx_ageouts_total++;

            #if RX_DEBUG_PRINT
            {
                printf("Age Out Rem Data->rem_index:%ld\n", deltmp->index);
            }
            #endif

            /* remove it */
            LLDP_OM_DeleteRemData(deltmp, LLDP_OM_DELETE_REASON_AGEOUT);

            LLDP_OM_Statistics.rem_table_ageouts++;
        }
        else
        {
            break;
        }
    }
}/* End of LLDP_OM_AgeDelete */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetTotalNumOfRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get total number of remote data in the DB
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : UI32_T
 * NOTE     : This function is used by backdoor
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetTotalNumOfRemData()
{
    return LLDP_OM_RemDataTotal;
}/* End of LLDP_OM_GetTotalNumOfRemData*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateRemProtoVlanList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote protocol vlan list
 * INPUT    : rem_proto_vlan_list
 * OUTPUT   : rem_proto_vlan_list
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateRemProtoVlanList(L_SORT_LST_List_T *rem_proto_vlan_list)
{
    return L_SORT_LST_Create(rem_proto_vlan_list,                       /* list */
                             LLDP_TYPE_MAX_REM_VLAN_NUM,                /* max number*/
                             sizeof(LLDP_OM_Xdot1RemProtoVlanEntry_T *),  /* element size */
                             LLDP_OM_SortCompRemProtoVlanEntry);        /* compare function */

}/* End of LLDP_OM_CreateRemProtoVlanList */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_FreeRemProtoVlanList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote protocol vlan list
 * INPUT    : rem_proto_vlan_list
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void LLDP_OM_FreeRemProtoVlanList(L_SORT_LST_List_T *rem_proto_vlan_list)
{
    LLDP_OM_Xdot1RemProtoVlanEntry_T    *rem_prot_vlan_entry_p;
    while(rem_proto_vlan_list->nbr_of_element != 0)
    {
        L_SORT_LST_Remove_1st(rem_proto_vlan_list, &rem_prot_vlan_entry_p);
        free(rem_prot_vlan_entry_p);
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateRemVlanNameList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote vlan name list
 * INPUT    : rem_vlan_name_list
 * OUTPUT   : rem_vlan_name_list
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateRemVlanNameList(L_SORT_LST_List_T *rem_vlan_name_list)
{
    return L_SORT_LST_Create(rem_vlan_name_list,                        /* list */
                             LLDP_TYPE_MAX_REM_VLAN_NUM,                /* max number*/
                             sizeof(LLDP_OM_Xdot1RemVlanNameEntry_T *), /* element size */
                             LLDP_OM_SortCompRemVlanNameEntry);         /* compare function */
}/* End of LLDP_OM_CreateRemVlanNameList */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateRemVlanNameList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote vlan name list
 * INPUT    : rem_vlan_name_list
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void LLDP_OM_FreeRemVlanNameList(L_SORT_LST_List_T *rem_vlan_name_list)
{
    LLDP_OM_Xdot1RemVlanNameEntry_T *rem_vlan_name_entry_p;
    while(rem_vlan_name_list->nbr_of_element != 0)
    {
        L_SORT_LST_Remove_1st(rem_vlan_name_list, &rem_vlan_name_entry_p);
        free(rem_vlan_name_entry_p);
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateRemProtocolList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote protocol list
 * INPUT    : rem_proto_list
 * OUTPUT   : rem_proto_list
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateRemProtocolList(L_SORT_LST_List_T *rem_proto_list)
{
    return L_SORT_LST_Create(rem_proto_list,                            /* list */
                             LLDP_TYPE_MAX_PROTOCOL_IDENT_NUM,          /* max number*/
                             sizeof(LLDP_OM_Xdot1ProtocolEntry_T *),    /* element size */
                             LLDP_OM_SortCompRemProtocolEntry);         /* compare function */
}/* End of LLDP_OM_CreateRemProtocolList */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_FreeRemProtocolList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote protocol list
 * INPUT    : rem_proto_list
 * OUTPUT   : rem_proto_list
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void LLDP_OM_FreeRemProtocolList(L_SORT_LST_List_T *rem_protocol_list)
{
    LLDP_OM_Xdot1ProtocolEntry_T    *rem_protocol_entry_p;
    while(rem_protocol_list->nbr_of_element != 0)
    {
        L_SORT_LST_Remove_1st(rem_protocol_list, &rem_protocol_entry_p);
        free(rem_protocol_entry_p);
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateCivicAddrCaList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote protocol list
 * INPUT    : civic_addr_ca_list
 * OUTPUT   : civic_addr_ca_list
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateCivicAddrCaList(L_SORT_LST_List_T *civic_addr_ca_list)
{
    return L_SORT_LST_Create(civic_addr_ca_list,                        /* list */
                             LLDP_TYPE_MAX_NUM_OF_CIVIC_ADDR_ELEMENT,   /* max number*/
                             sizeof(LLDP_OM_XMedLocationCivicAddrCaTlv_T *),    /* element size */
                             LLDP_OM_SortCompCivicAddrCaEntry);         /* compare function */
}/* End of LLDP_OM_CreateCivicAddrCaList */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_FreeCivicAddrCaList
 * ------------------------------------------------------------------------
 * PURPOSE  : Free Civic addr ca list
 * INPUT    : civic_addr_ca_list
 * OUTPUT   : civic_addr_ca_list
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void LLDP_OM_FreeCivicAddrCaList(L_SORT_LST_List_T *civic_addr_ca_list)
{
    LLDP_OM_XMedLocationCivicAddrCaTlv_T *civic_addr_ca_entry;
    while(civic_addr_ca_list->nbr_of_element != 0)
    {
        L_SORT_LST_Remove_1st(civic_addr_ca_list, &civic_addr_ca_entry);
        free(civic_addr_ca_entry);
    }
}/* End of LLDP_OM_FreeCivicAddrCaList */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyMedRemTableChangedList
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_SORT_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_SORT_LST_List_T *LLDP_OM_GetNotifyMedRemTableChangedList()
{
    return &LLDP_OM_NotifyMedRemTableChangedList;
}

#if (SYS_CPNT_ADD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyTelephoneTypeListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_SORT_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_LINK_LST_List_T *LLDP_OM_GetNotifyTelephoneTypeListPtr()
{
    return &LLDP_OM_NotifyTelephoneTypeList;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyTelephoneAddrChangedListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_SORT_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_SORT_LST_List_T *LLDP_OM_GetNotifyTelephoneAddrChangedListPtr()
{
    return &LLDP_OM_NotifyTelephoneAddrChangedList;
}
#endif

#if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2))
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyDot3atInfoReceivedListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_SORT_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_SORT_LST_List_T *LLDP_OM_GetNotifyDot3atInfoReceivedListPtr(void)
{
    return &LLDP_OM_NotifyDot3atInfoReceivedList;
}
#endif /* #if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)) */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CreateRemAppPriorityList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote vlan name list
 * INPUT    : rem_app_priority_list
 * OUTPUT   : rem_app_priority_list
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_CreateRemAppPriorityList(L_SORT_LST_List_T *rem_app_priority_list)
{
    return L_SORT_LST_Create(rem_app_priority_list,                        /* list */
                             LLDP_TYPE_MAX_REM_APP_PRIORITY_ENTRY_NUM,                /* max number*/
                             sizeof(LLDP_OM_XDcbxRemAppPriorityEntry_T *), /* element size */
                             LLDP_OM_SortCompRemAppPriorityEntry);         /* compare function */
}/* End of LLDP_OM_CreateRemAppPriorityList */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_SortCompRemAppPriorityEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote app priority list to sort list
 * INPUT    : inlist_element        -- the element in the sort list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int LLDP_OM_SortCompRemAppPriorityEntry(void* inlist_element, void* input_element)
{
    LLDP_OM_XDcbxRemAppPriorityEntry_T *element1, *element2;

    element1 = *(LLDP_OM_XDcbxRemAppPriorityEntry_T **)inlist_element;
    element2 = *(LLDP_OM_XDcbxRemAppPriorityEntry_T **)input_element;

    return (element1->rem_protocol_id - element2->rem_protocol_id);

}/* End of LLDP_OM_SortCompRemAppPriorityEntry */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_FreeRemAppPriorityList
 * ------------------------------------------------------------------------
 * PURPOSE  : Create remote app priority list
 * INPUT    : rem_app_priority_list
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void LLDP_OM_FreeRemAppPriorityList(L_SORT_LST_List_T *rem_app_priority_list)
{
    LLDP_OM_XDcbxRemAppPriorityEntry_T *rem_app_priority_entry_p;
    while(rem_app_priority_list->nbr_of_element != 0)
    {
        L_SORT_LST_Remove_1st(rem_app_priority_list, &rem_app_priority_entry_p);
        free(rem_app_priority_entry_p);
    }
}

#if(SYS_CPNT_DCBX == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CompNotifyDcbxRemEtsEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the notify entry list to link list
 * INPUT    : inlist_element        -- the element in the link list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T LLDP_OM_CompNotifyDcbxRemEtsEntry(void *inlist_element, void *input_element)
{
    LLDP_TYPE_NotifyDcbxEtsInfo_T   *element1, *element2;

    element1 = (LLDP_TYPE_NotifyDcbxEtsInfo_T *)inlist_element;
    element2 = (LLDP_TYPE_NotifyDcbxEtsInfo_T *)input_element;

    if(element1->time_mark != element2->time_mark)
        return FALSE;
    if(element1->lport != element2->lport)
        return FALSE;
    if(element1->lport != element2->index)
        return FALSE;

    return TRUE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_CompNotifyDcbxRemPfcEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : a function used in the notify entry list to link list
 * INPUT    : inlist_element        -- the element in the link list
 *            input_element         -- the element to compare
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T LLDP_OM_CompNotifyDcbxRemPfcEntry(void *inlist_element, void *input_element)
{
    LLDP_TYPE_NotifyDcbxPfcInfo_T   *element1, *element2;

    element1 = (LLDP_TYPE_NotifyDcbxPfcInfo_T *)inlist_element;
    element2 = (LLDP_TYPE_NotifyDcbxPfcInfo_T *)input_element;

    if(element1->time_mark != element2->time_mark)
        return FALSE;
    if(element1->lport != element2->lport)
        return FALSE;
    if(element1->lport != element2->index)
        return FALSE;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyDcbxRemEtsChangedListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_LINK_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_LINK_LST_List_T *LLDP_OM_GetNotifyDcbxRemEtsChangedListPtr(void)
{
    return &LLDP_OM_NotifyDcbxRemEtsChangedList;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyDcbxRemPfcChangedListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_LINK_LST_List_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_LINK_LST_List_T *LLDP_OM_GetNotifyDcbxRemPfcChangedListPtr(void)
{
    return &LLDP_OM_NotifyDcbxRemPfcChangedList;
}

BOOL_T LLDP_OM_IncreaseDcbxTimer(UI32_T lport)
{
    if (LLDP_OM_DcbxTimer[lport - 1].enabled == FALSE)
    {
        return FALSE;
    }

    if (LLDP_OM_DcbxTimer[lport - 1].time <  LLDP_OM_DcbxTimer[lport - 1].limit)
    {
        LLDP_OM_DcbxTimer[lport - 1].time++;
        if (LLDP_OM_DcbxTimer[lport - 1].time == LLDP_OM_DcbxTimer[lport - 1].limit)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static void LLDP_OM_InitDcbxTimer(void)
{
    UI32_T index = 0;

    for(index = 0; index < SYS_ADPT_TOTAL_NBR_OF_LPORT; index++)
    {
        LLDP_OM_DcbxTimer[index].enabled = FALSE;
    }

    return;
}

static BOOL_T LLDP_OM_IsMultiPeerDetected(UI32_T lport)
{
    if (LLDP_OM_DcbxTimer[lport - 1].enabled == FALSE)
    {
        return FALSE;
    }

    if (LLDP_OM_DcbxTimer[lport - 1].time == LLDP_OM_DcbxTimer[lport - 1].limit)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InsertNotifyDcbxRemEtsChangedList
 * ------------------------------------------------------------------------
 * PURPOSE  : Insert remote data to NotifyDcbxRemEtsChangedList
 * INPUT    : lport -- lport to be inserted
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_InsertNotifyDcbxRemEtsChangedList(UI32_T lport)
{
    BOOL_T              ret = FALSE;
    LLDP_OM_RemData_T   rem_data, *rem_data_p;

    rem_data_p = &rem_data;
    rem_data.lport = lport;
    rem_data.index = 0;

    if (  (FALSE == LLDP_OM_IsMultiPeerDetected(rem_data.lport))
        &&(TRUE  == LLDP_OM_GetNextRemDataByPort(rem_data.lport, &rem_data_p))
        &&(  (NULL != rem_data_p->dcbx_rem_ets_config_entry)
           ||(NULL != rem_data_p->dcbx_rem_ets_recommend_entry)
          )
       )
    {
        LLDP_TYPE_NotifyDcbxEtsInfo_T notify_entry;

        memset(&notify_entry, 0, sizeof(LLDP_TYPE_NotifyDcbxEtsInfo_T));
        notify_entry.time_mark = rem_data_p->time_mark;
        notify_entry.lport = rem_data_p->lport;
        notify_entry.index = rem_data_p->index;
        notify_entry.is_delete = FALSE;
        notify_entry.rem_recommend_rcvd = (rem_data_p->dcbx_rem_ets_recommend_entry != NULL) ? TRUE : FALSE;
        if (rem_data_p->dcbx_rem_ets_config_entry != NULL)
        {
            notify_entry.rem_config_willing = rem_data_p->dcbx_rem_ets_config_entry->rem_willing;
            notify_entry.rem_config_cbs = rem_data_p->dcbx_rem_ets_config_entry->rem_cbs;
            notify_entry.rem_config_max_tc = rem_data_p->dcbx_rem_ets_config_entry->rem_max_tc;
            memcpy(notify_entry.rem_config_pri_assign_table, rem_data_p->dcbx_rem_ets_config_entry->rem_pri_assign_table, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
            memcpy(notify_entry.rem_config_tc_bandwidth_table, rem_data_p->dcbx_rem_ets_config_entry->rem_tc_bandwidth_table, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
            memcpy(notify_entry.rem_config_tsa_assign_table, rem_data_p->dcbx_rem_ets_config_entry->rem_tsa_assign_table, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);
        }
        if (rem_data_p->dcbx_rem_ets_recommend_entry != NULL)
        {
            memcpy(notify_entry.rem_recommend_pri_assign_table, rem_data_p->dcbx_rem_ets_recommend_entry->rem_pri_assign_table, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
            memcpy(notify_entry.rem_recommend_tc_bandwidth_table, rem_data_p->dcbx_rem_ets_recommend_entry->rem_tc_bandwidth_table, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
            memcpy(notify_entry.rem_recommend_tsa_assign_table, rem_data_p->dcbx_rem_ets_recommend_entry->rem_tsa_assign_table, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);
        }
        L_LINK_LST_Set(&LLDP_OM_NotifyDcbxRemEtsChangedList, &notify_entry, L_LINK_LST_APPEND);
        ret = TRUE;
    }

    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_InsertNotifyDcbxRemPfcChangedList
 * ------------------------------------------------------------------------
 * PURPOSE  : Insert remote data to NotifyDcbxRemPfcChangedList
 * INPUT    : lport -- lport to be inserted
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_InsertNotifyDcbxRemPfcChangedList(UI32_T lport)
{
    BOOL_T              ret = FALSE;
    LLDP_OM_RemData_T   rem_data, *rem_data_p;

    rem_data_p = &rem_data;
    rem_data.lport = lport;
    rem_data.index = 0;

    if (  (FALSE == LLDP_OM_IsMultiPeerDetected(rem_data.lport))
        &&(TRUE  == LLDP_OM_GetNextRemDataByPort(rem_data.lport, &rem_data_p))
        &&(NULL != rem_data_p->dcbx_rem_pfc_config_entry)
       )
    {
        LLDP_TYPE_NotifyDcbxPfcInfo_T notify_entry;

        memset(&notify_entry, 0, sizeof(LLDP_TYPE_NotifyDcbxPfcInfo_T));
        notify_entry.time_mark = rem_data_p->time_mark;
        notify_entry.lport = rem_data_p->lport;
        notify_entry.index = rem_data_p->index;
        notify_entry.is_delete = FALSE;
        memcpy(notify_entry.rem_mac, rem_data_p->dcbx_rem_pfc_config_entry->rem_mac, 6);
        notify_entry.rem_willing = rem_data_p->dcbx_rem_pfc_config_entry->rem_willing;
        notify_entry.rem_mbc = rem_data_p->dcbx_rem_pfc_config_entry->rem_mbc;
        notify_entry.rem_cap = rem_data_p->dcbx_rem_pfc_config_entry->rem_cap;
        notify_entry.rem_enable = rem_data_p->dcbx_rem_pfc_config_entry->rem_enable;

        L_LINK_LST_Set(&LLDP_OM_NotifyDcbxRemPfcChangedList, &notify_entry, L_LINK_LST_APPEND);
        ret = TRUE;
    }

    return ret;
}
#endif /* #if(SYS_CPNT_DCBX == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyCnRemoteChangeListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_LINK_LST_List_T*
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_LINK_LST_List_T* LLDP_OM_GetNotifyCnRemoteChangeListPtr(void)
{
    return &LLDP_OM_NotifyCnRemoteChangeList;
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNotifyRemChangeListPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get notify list pointer
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : L_LINK_LST_List_T*
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_LINK_LST_List_T* LLDP_OM_GetNotifyRemChangeListPtr(void)
{
    return &LLDP_OM_NotifyRemChangeList;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetPortDeviceList
 *-------------------------------------------------------------------------
 * PURPOSE  : Get remote data list of port
 * INPUT    : lport
 * OUTPUT   : None
 * RETUEN   : SYS_TYPE_CallBack_T *
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
L_SORT_LST_List_T *LLDP_OM_GetPortDeviceList(UI32_T lport)
{
    return &LLDP_OM_RemDataList[lport - 1];
}

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetSmallestTimeMarkOfRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get neighbor type call back list
 * INPUT    : None
 * OUTPUT   : smallest_time_mark
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_GetSmallestTimeMarkOfRemData(UI32_T *smallest_time_mark)
{
    if(LLDP_OM_RemDataTotal == 0)
        *smallest_time_mark = SYSFUN_GetSysTick();
    else
    {
        LLDP_OM_RemData_T   *rem_data;

        rem_data = LLDP_OM_AgeSeqBeg;

        while(rem_data)
        {
            if(*smallest_time_mark > rem_data->time_mark)
                *smallest_time_mark = rem_data->time_mark - 1;

            rem_data = rem_data->age_seq_next;
        }
    }

}
#endif


/*=============================================================================
 * Moved from lldp_mgr.c
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP global admin status
 * INPUT    : None
 * OUTPUT   : admin_status
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetSysAdminStatus(UI32_T *admin_status)
{
    LLDP_OM_SysConfigEntry_T    *sys_config;

    sys_config = LLDP_OM_GetSysConfigEntryPtr();
    *admin_status = sys_config->global_admin_status;

    return LLDP_TYPE_RETURN_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP global admin status
 * INPUT    : None
 * OUTPUT   : admin_status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningSysAdminStatus(UI32_T *admin_status)
{
    UI32_T                  result;
    LLDP_OM_SysConfigEntry_T    *sys_config;

    result = LLDP_TYPE_RETURN_ERROR;

    sys_config = LLDP_OM_GetSysConfigEntryPtr();

    if(sys_config->global_admin_status != LLDP_TYPE_DEFAULT_SYS_ADMIN_STATUS)
    {
        *admin_status = sys_config->global_admin_status;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get configuration entry info. for system.
 * INPUT    : LLDP_MGR_SysConfigEntry_T  *config_entry
 * OUTPUT   : LLDP_MGR_SysConfigEntry_T  *config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetSysConfigEntry(LLDP_MGR_SysConfigEntry_T *sys_config_entry)
{
    LLDP_OM_SysConfigEntry_T  *om_sys_config_ptr;

    /* get system config entry pointer from om */
    om_sys_config_ptr = LLDP_OM_GetSysConfigEntryPtr();

    sys_config_entry->message_tx_interval       = om_sys_config_ptr->msg_tx_interval;
    sys_config_entry->message_tx_hold_multiplier= om_sys_config_ptr->msg_tx_hold_mul;
    sys_config_entry->reinit_delay              = om_sys_config_ptr->reinit_delay;
    sys_config_entry->tx_delay                  = om_sys_config_ptr->tx_delay;
    sys_config_entry->notification_interval     = om_sys_config_ptr->notification_interval;

    return LLDP_TYPE_RETURN_OK;
}/* End of LLDP_OM_GetSysConfigEntry*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRemoteSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.2.2/9.5.2.3/9.5.3.3/9.5.5.2/
 *            9.5.6.2/9.5.7.2/9.5.8.1/9.5.8.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetRemoteSystemData(LLDP_MGR_RemoteSystemData_T *system_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;

    result              = LLDP_TYPE_RETURN_ERROR;

    if( LLDP_OM_GetRemData(system_entry->rem_local_port_num,
                          system_entry->rem_index,
                          &rem_data) )
    {
        /* get remote data from om and fill in system entry */
        if(system_entry->rem_time_mark <= rem_data->time_mark)
        {
            system_entry->rem_index                 = rem_data->index;
            system_entry->rem_chassis_id_subtype    = rem_data->rem_chassis_id_subtype;
            system_entry->rem_chassis_id_len        = rem_data->rem_chassis_id_len;
            memcpy(system_entry->rem_chassis_id, rem_data->rem_chassis_id, system_entry->rem_chassis_id_len);

            system_entry->rem_port_id_subtype       = rem_data->rem_port_id_subtype;
            system_entry->rem_port_id_len           = rem_data->rem_port_id_len;
            memcpy(system_entry->rem_port_id, rem_data->rem_port_id, system_entry->rem_port_id_len);

            system_entry->rem_port_desc_len         = rem_data->rem_sys_entry->rem_port_desc_len;
            memcpy(system_entry->rem_port_desc, rem_data->rem_sys_entry->rem_port_desc, system_entry->rem_port_desc_len);
            system_entry->rem_port_desc[system_entry->rem_port_desc_len] = 0;

            system_entry->rem_sys_name_len          = rem_data->rem_sys_entry->rem_sys_name_len;
            memcpy(system_entry->rem_sys_name, rem_data->rem_sys_entry->rem_sys_name, system_entry->rem_sys_name_len);
            system_entry->rem_sys_name[system_entry->rem_sys_name_len] = 0;

            system_entry->rem_sys_desc_len          = rem_data->rem_sys_entry->rem_sys_desc_len;
            memcpy(system_entry->rem_sys_desc, rem_data->rem_sys_entry->rem_sys_desc, system_entry->rem_sys_desc_len);
            system_entry->rem_sys_desc[system_entry->rem_sys_desc_len] = 0;

            system_entry->rem_sys_cap_supported = rem_data->rem_sys_entry->rem_sys_cap_supported;
            system_entry->rem_sys_cap_enabled   = rem_data->rem_sys_entry->rem_sys_cap_enabled;

#if 0
            /* Flip to snmp order */
            ((UI8_T*)(&system_entry->rem_sys_cap_supported))[0] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_supported)[1]);
            ((UI8_T*)(&system_entry->rem_sys_cap_supported))[1] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_supported)[0]);

            ((UI8_T*)(&system_entry->rem_sys_cap_enabled))[0] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_enabled)[1]);
            ((UI8_T*)(&system_entry->rem_sys_cap_enabled))[1] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_enabled)[0]);
#endif

            result = LLDP_TYPE_RETURN_OK;
        }
    }

    return result;
}/* End of LLDP_OM_GetRemoteSystemData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextRemManAddrByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : This function is used by CLI/WEB
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetNextRemManAddrByIndex(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;
    LLDP_OM_RemManAddrEntry_T *rem_man_addr_entry, *rem_man_addr_entry_tmp;

    result              = LLDP_TYPE_RETURN_ERROR;
    /* get remote data by index */
    if(LLDP_OM_GetRemData(man_addr_entry->rem_local_port_num, man_addr_entry->rem_index, &rem_data))
    {
        rem_man_addr_entry = (LLDP_OM_RemManAddrEntry_T *)malloc(sizeof(LLDP_OM_RemManAddrEntry_T));
        rem_man_addr_entry_tmp = rem_man_addr_entry;

        rem_man_addr_entry->rem_man_addr_subtype = man_addr_entry->rem_man_addr_subtype;
        memcpy(rem_man_addr_entry->rem_man_addr, man_addr_entry->rem_man_addr, LLDP_TYPE_MAX_MANAGEMENT_ADDR_LENGTH);
        if(L_SORT_LST_Get_Next(&rem_data->rem_man_addr_list, &rem_man_addr_entry))
        {
            man_addr_entry->rem_man_addr_subtype = rem_man_addr_entry->rem_man_addr_subtype;
            memcpy(man_addr_entry->rem_man_addr, rem_man_addr_entry->rem_man_addr, rem_man_addr_entry->rem_man_addr_len);
            man_addr_entry->rem_man_addr_len = rem_man_addr_entry->rem_man_addr_len;
            man_addr_entry->rem_man_addr_if_subtype = rem_man_addr_entry->rem_man_addr_if_subtype;
            man_addr_entry->rem_man_addr_if_id = rem_man_addr_entry->rem_man_addr_if_id;
            result = LLDP_TYPE_RETURN_OK;
        }
        free(rem_man_addr_entry_tmp);
    }

    return result;
}/* End of LLDP_OM_GetNextRemManAddrByIndex*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningMsgTxHoldMul
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T  *msg_tx_hold
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningMsgTxHoldMul(UI32_T  *msg_tx_hold)
{
    UI32_T                  result;

    LLDP_OM_SysConfigEntry_T    *om_sys_config;

    result              = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    om_sys_config = LLDP_OM_GetSysConfigEntryPtr();

    if(om_sys_config->msg_tx_hold_mul != LLDP_TYPE_DEFAULT_TX_HOLD_MUL)
    {
        *msg_tx_hold = om_sys_config->msg_tx_hold_mul;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
}/* End of LLDP_OM_GetRunningMsgTxHoldMul */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningMsgTxInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T  *msg_tx_interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningMsgTxInterval(UI32_T  *msg_tx_interval)
{
    UI32_T                  result;

    LLDP_OM_SysConfigEntry_T    *om_sys_config;

    result              = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    om_sys_config = LLDP_OM_GetSysConfigEntryPtr();

    if(om_sys_config->msg_tx_interval != LLDP_TYPE_DEFAULT_TX_INTERVAL)
    {
        *msg_tx_interval = om_sys_config->msg_tx_interval;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
}/* End of LLDP_OM_GetRunningMsgTxInterval*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningReinitDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T *reinit_delay
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningReinitDelay(UI32_T *reinit_delay)
{
    UI32_T                  result;

    LLDP_OM_SysConfigEntry_T    *om_sys_config;

    result              = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    om_sys_config = LLDP_OM_GetSysConfigEntryPtr();

    if(om_sys_config->reinit_delay != LLDP_TYPE_DEFAULT_REINIT_DELAY)
    {
        *reinit_delay = om_sys_config->reinit_delay;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
}/* End of LLDP_OM_GetRunningReinitDelayTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningTxDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T  *tx_delay_time
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningTxDelay(UI32_T  *tx_delay_time)
{
    UI32_T                  result;

    LLDP_OM_SysConfigEntry_T    *om_sys_config;

    result              = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    om_sys_config = LLDP_OM_GetSysConfigEntryPtr();

    if(om_sys_config->tx_delay != LLDP_TYPE_DEFAULT_TX_DELAY)
    {
        *tx_delay_time = om_sys_config->tx_delay;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
}/* End of LLDP_OM_GetRunningTxDelayTime*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningNotifyInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T *notify_interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningNotifyInterval(UI32_T *notify_interval)
{
    UI32_T                  result;

    LLDP_OM_SysConfigEntry_T    *om_sys_config;

    result              = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    om_sys_config = LLDP_OM_GetSysConfigEntryPtr();

    if(om_sys_config->notification_interval != LLDP_TYPE_DEFAULT_NOTIFY_INTERVAL)
    {
        *notify_interval = om_sys_config->notification_interval;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
}/* End of LLDP_OM_GetRunningNotifyInterval*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningPortAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : basic_tlv_enable
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningPortAdminStatus(UI32_T lport, UI8_T *admin_status)
{
    UI32_T                  result;

    LLDP_OM_PortConfigEntry_T    *om_port_config;

    result              = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    om_port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

    if(om_port_config->admin_status != LLDP_TYPE_DEFAULT_PORT_ADMIN_STATUS)
    {
        *admin_status = om_port_config->admin_status;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningPortNotify
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningPortNotificationEnable(UI32_T lport, BOOL_T *notify_enable)
{
    UI32_T                  result;

    LLDP_OM_PortConfigEntry_T    *om_port_config;

    result              = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    om_port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

    if(om_port_config->notification_enable != LLDP_TYPE_DEFAULT_PORT_NOTIFY)
    {
        *notify_enable = om_port_config->notification_enable;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
}/* End of LLDP_OM_GetRunningPortNotify*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningPortBasicTlvTransfer
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : basic_tlvs_tx_flag, basic_tlvs_change_flag
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningPortBasicTlvTransfer(UI32_T lport, UI8_T *basic_tlvs_tx_flag, UI8_T *basic_tlvs_change_flag)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *om_port_config;

    om_port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
    if (om_port_config->basic_tlvs_tx_flag != LLDP_TYPE_DEFAULT_PORT_BASIC_TLV_TRANSFER_FLAG)
    {
        *basic_tlvs_tx_flag = om_port_config->basic_tlvs_tx_flag;
        *basic_tlvs_change_flag = om_port_config->basic_tlvs_tx_flag ^ LLDP_TYPE_DEFAULT_PORT_BASIC_TLV_TRANSFER_FLAG;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return result;
}/* End of LLDP_OM_GetRunningPortBasicTlvTransfer*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningPortManAddrTlvTransfer
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningPortManAddrTlvTransfer(UI32_T lport, UI8_T *man_addr_tlv_enable)
{
    UI32_T                  result;
    LLDP_OM_PortConfigEntry_T   *port_config;

    result              = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
    if(port_config->man_addr_transfer_flag == LLDP_TYPE_DEFAULT_MAN_ADDR_TLV_TRANSFER_FLAG)
    {
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        *man_addr_tlv_enable = port_config->man_addr_transfer_flag;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return result;
}/* End of LLDP_OM_GetRunningPortManAddrTlvTransfer*/

#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get current 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetRunningXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry)
{
    UI32_T                  result;

    LLDP_OM_PortConfigEntry_T   *port_config;

    result = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    port_config = LLDP_OM_GetPortConfigEntryPtr(xdot1_config_entry->lport);

    if (LLDP_TYPE_DEFAULT_XDOT1_PORT_VLAN_TX != port_config->xdot1_port_vlan_tx_enable)
    {
        xdot1_config_entry->port_vlan_tx_enable = port_config->xdot1_port_vlan_tx_enable;
        xdot1_config_entry->port_vlan_tx_changed = TRUE;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        xdot1_config_entry->port_vlan_tx_changed = FALSE;
    }

    if (LLDP_TYPE_DEFAULT_XDOT1_PROTOCOL_TX != port_config->xdot1_protocol_tx_enable)
    {
        xdot1_config_entry->protocol_tx_enable = port_config->xdot1_protocol_tx_enable;
        xdot1_config_entry->protocol_tx_changed = TRUE;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        xdot1_config_entry->protocol_tx_changed = FALSE;
    }

    if (LLDP_TYPE_DEFAULT_XDOT1_PROTO_VLAN_TX != port_config->xdot1_proto_vlan_tx_enable)
    {
        xdot1_config_entry->proto_vlan_tx_enable = port_config->xdot1_proto_vlan_tx_enable;
        xdot1_config_entry->proto_vlan_tx_changed = TRUE;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        xdot1_config_entry->proto_vlan_tx_changed = FALSE;
    }

    if (LLDP_TYPE_DEFAULT_XDOT1_VLAN_NAME_TX != port_config->xdot1_vlan_name_tx_enable)
    {
        xdot1_config_entry->vlan_name_tx_enable = port_config->xdot1_vlan_name_tx_enable;
        xdot1_config_entry->vlan_name_tx_changed = TRUE;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        xdot1_config_entry->vlan_name_tx_changed = FALSE;
    }

    if (LLDP_TYPE_DEFAULT_XDOT1_CN_TX != port_config->xdot1_cn_tx_enable)
    {
        xdot1_config_entry->cn_tx_enable = port_config->xdot1_cn_tx_enable;
        xdot1_config_entry->cn_tx_changed = TRUE;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        xdot1_config_entry->cn_tx_changed = FALSE;
    }

    return result;
}/* End of LLDP_OM_GetRunningXdot1ConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot1RemEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot1RemEntry(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;

    result = LLDP_TYPE_RETURN_ERROR;
    if(LLDP_OM_GetRemData(xdot1_rem_entry->rem_local_port_num, xdot1_rem_entry->rem_index, &rem_data))
    {
        if(xdot1_rem_entry->rem_time_mark <= rem_data->time_mark &&
           rem_data->xdot1_rem_port_vlan_id)
        {
            xdot1_rem_entry->rem_port_vlan_id = rem_data->xdot1_rem_port_vlan_id;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    return result;
}/* End of LLDP_OM_GetXdot1RemEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot1RemProtoVlanEntry(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;

    result = LLDP_TYPE_RETURN_ERROR;
    if(LLDP_OM_GetRemData(xdot1_rem_proto_vlan_entry->rem_local_port_num, xdot1_rem_proto_vlan_entry->rem_index, &rem_data))
    {
        if(xdot1_rem_proto_vlan_entry->rem_time_mark <= rem_data->time_mark)
        {
            LLDP_OM_Xdot1RemProtoVlanEntry_T    *rem_proto_vlan_entry, *rem_proto_vlan_entry_tmp;

            rem_proto_vlan_entry = (LLDP_OM_Xdot1RemProtoVlanEntry_T *)malloc(sizeof(LLDP_OM_Xdot1RemProtoVlanEntry_T));
            rem_proto_vlan_entry_tmp = rem_proto_vlan_entry;

            rem_proto_vlan_entry->rem_proto_vlan_id = xdot1_rem_proto_vlan_entry->rem_proto_vlan_id;
            if(L_SORT_LST_Get(&rem_data->xdot1_rem_proto_vlan_list, &rem_proto_vlan_entry))
            {
                xdot1_rem_proto_vlan_entry->rem_proto_vlan_supported = rem_proto_vlan_entry->rem_proto_vlan_supported;
                xdot1_rem_proto_vlan_entry->rem_proto_vlan_enabled = rem_proto_vlan_entry->rem_proto_vlan_enabled;
                result = LLDP_TYPE_RETURN_OK;
            }
            free(rem_proto_vlan_entry_tmp);
        }
    }

    return result;
}/* End of LLDP_OM_GetXdot1RemProtoVlanEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextXdot1RemProtoVlanEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetNextXdot1RemProtoVlanEntryByIndex(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;
    LLDP_OM_Xdot1RemProtoVlanEntry_T    *rem_proto_vlan_entry, *rem_proto_vlan_entry_tmp;

    result = LLDP_TYPE_RETURN_ERROR;

    if (LLDP_OM_GetRemData(xdot1_rem_proto_vlan_entry->rem_local_port_num, xdot1_rem_proto_vlan_entry->rem_index, &rem_data))
    {
        rem_proto_vlan_entry = (LLDP_OM_Xdot1RemProtoVlanEntry_T *)malloc(sizeof(LLDP_OM_Xdot1RemProtoVlanEntry_T));
        rem_proto_vlan_entry_tmp = rem_proto_vlan_entry;
        rem_proto_vlan_entry->rem_proto_vlan_id = xdot1_rem_proto_vlan_entry->rem_proto_vlan_id;

        if (    (    (rem_proto_vlan_entry->rem_proto_vlan_id == LLDP_TYPE_FIRST_PPVID)
                  && (L_SORT_LST_Get_1st(&rem_data->xdot1_rem_proto_vlan_list, &rem_proto_vlan_entry))
                )
             || (L_SORT_LST_Get_Next(&rem_data->xdot1_rem_proto_vlan_list, &rem_proto_vlan_entry))
           )
        {
            xdot1_rem_proto_vlan_entry->rem_local_port_num = rem_data->lport;
            xdot1_rem_proto_vlan_entry->rem_index = rem_data->index;
            xdot1_rem_proto_vlan_entry->rem_proto_vlan_id = rem_proto_vlan_entry->rem_proto_vlan_id;
            xdot1_rem_proto_vlan_entry->rem_proto_vlan_supported = rem_proto_vlan_entry->rem_proto_vlan_supported;
            xdot1_rem_proto_vlan_entry->rem_proto_vlan_enabled = rem_proto_vlan_entry->rem_proto_vlan_enabled;
            result = LLDP_TYPE_RETURN_OK;
        }
        free(rem_proto_vlan_entry_tmp);
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot1RemVlanNameEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot1RemVlanNameEntry(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;
    LLDP_OM_Xdot1RemVlanNameEntry_T *rem_vlan_name_entry, *rem_vlan_name_entry_tmp;

    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_OM_GetRemData(xdot1_rem_vlan_name_entry->rem_local_port_num,
                          xdot1_rem_vlan_name_entry->rem_index,
                          &rem_data))
    {
        if(xdot1_rem_vlan_name_entry->rem_time_mark <= rem_data->time_mark )
        {
            rem_vlan_name_entry = (LLDP_OM_Xdot1RemVlanNameEntry_T *)malloc(sizeof(LLDP_OM_Xdot1RemVlanNameEntry_T));
            rem_vlan_name_entry_tmp = rem_vlan_name_entry;

            rem_vlan_name_entry->rem_vlan_id = xdot1_rem_vlan_name_entry->rem_vlan_id;
            if(L_SORT_LST_Get(&rem_data->xdot1_rem_vlan_name_list, &rem_vlan_name_entry))
            {
                xdot1_rem_vlan_name_entry->rem_vlan_name_len = rem_vlan_name_entry->rem_vlan_name_len;
                memcpy(xdot1_rem_vlan_name_entry->rem_vlan_name, rem_vlan_name_entry->rem_vlan_name, rem_vlan_name_entry->rem_vlan_name_len);
                xdot1_rem_vlan_name_entry->rem_vlan_name[xdot1_rem_vlan_name_entry->rem_vlan_name_len]= 0;
                result = LLDP_TYPE_RETURN_OK;
            }
            free(rem_vlan_name_entry_tmp);
        }
    }

    return result;
}/* End of LLDP_OM_GetXdot1RemVlanNameEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextXdot1RemVlanNameEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetNextXdot1RemVlanNameEntryByIndex(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;
    LLDP_OM_Xdot1RemVlanNameEntry_T *rem_vlan_name_entry, *rem_vlan_name_entry_tmp;

    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_OM_GetRemData(xdot1_rem_vlan_name_entry->rem_local_port_num,
                          xdot1_rem_vlan_name_entry->rem_index,
                          &rem_data))
    {
        rem_vlan_name_entry = (LLDP_OM_Xdot1RemVlanNameEntry_T *)malloc(sizeof(LLDP_OM_Xdot1RemVlanNameEntry_T));
        rem_vlan_name_entry_tmp = rem_vlan_name_entry;
        rem_vlan_name_entry->rem_vlan_id = xdot1_rem_vlan_name_entry->rem_vlan_id;

        if(L_SORT_LST_Get_Next(&rem_data->xdot1_rem_vlan_name_list, &rem_vlan_name_entry))
        {
            xdot1_rem_vlan_name_entry->rem_vlan_id = rem_vlan_name_entry->rem_vlan_id;
            xdot1_rem_vlan_name_entry->rem_vlan_name_len = rem_vlan_name_entry->rem_vlan_name_len;
            memcpy(xdot1_rem_vlan_name_entry->rem_vlan_name, rem_vlan_name_entry->rem_vlan_name, rem_vlan_name_entry->rem_vlan_name_len);
            xdot1_rem_vlan_name_entry->rem_vlan_name[xdot1_rem_vlan_name_entry->rem_vlan_name_len]= 0;
            result = LLDP_TYPE_RETURN_OK;
        }
        free(rem_vlan_name_entry_tmp);
    }

    return result;
}/* End of LLDP_OM_GetNextXdot1RemVlanNameEntryByIndex */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot1RemProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot1RemProtocolEntry(LLDP_MGR_Xdot1RemProtocolEntry_T *xdot1_rem_protocol_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;
    LLDP_OM_Xdot1ProtocolEntry_T    *rem_proto_entry, *rem_proto_entry_tmp;

    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_OM_GetRemData(xdot1_rem_protocol_entry->rem_local_port_num,
                          xdot1_rem_protocol_entry->rem_index,
                          &rem_data))
    {
        if(xdot1_rem_protocol_entry->rem_time_mark <= rem_data->time_mark)
        {
            rem_proto_entry = (LLDP_OM_Xdot1ProtocolEntry_T *)malloc(sizeof(LLDP_OM_Xdot1ProtocolEntry_T));
            rem_proto_entry_tmp = rem_proto_entry;

            rem_proto_entry->rem_protocol_index = xdot1_rem_protocol_entry->rem_protocol_index;
            if(L_SORT_LST_Get(&rem_data->xdot1_rem_protocol_list, &rem_proto_entry))
            {
                memcpy(xdot1_rem_protocol_entry->rem_protocol_id, rem_proto_entry->rem_protocol_id, rem_proto_entry->rem_protocol_id_len);
                xdot1_rem_protocol_entry->rem_protocol_id_len = rem_proto_entry->rem_protocol_id_len;
                result = LLDP_TYPE_RETURN_OK;
            }
            free(rem_proto_entry_tmp);
        }
    }

    return result;
}/* End of LLDP_OM_GetXdot1RemProtocolEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextXdot1RemProtocolEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetNextXdot1RemProtocolEntryByIndex(LLDP_MGR_Xdot1RemProtocolEntry_T *xdot1_rem_protocol_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;
    LLDP_OM_Xdot1ProtocolEntry_T    *rem_proto_entry, *rem_proto_entry_tmp;

    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_OM_GetRemData(xdot1_rem_protocol_entry->rem_local_port_num,
                          xdot1_rem_protocol_entry->rem_index,
                          &rem_data))
    {
        rem_proto_entry = (LLDP_OM_Xdot1ProtocolEntry_T *)malloc(sizeof(LLDP_OM_Xdot1ProtocolEntry_T));
        rem_proto_entry_tmp = rem_proto_entry;
        rem_proto_entry->rem_protocol_index = xdot1_rem_protocol_entry->rem_protocol_index;

        if(L_SORT_LST_Get_Next(&rem_data->xdot1_rem_protocol_list, &rem_proto_entry))
        {
            xdot1_rem_protocol_entry->rem_protocol_index = rem_proto_entry->rem_protocol_index;
            memcpy(xdot1_rem_protocol_entry->rem_protocol_id, rem_proto_entry->rem_protocol_id, rem_proto_entry->rem_protocol_id_len);
            xdot1_rem_protocol_entry->rem_protocol_id_len = rem_proto_entry->rem_protocol_id_len;
            result = LLDP_TYPE_RETURN_OK;
        }
        free(rem_proto_entry_tmp);
    }

    return result;
}/* End of LLDP_OM_GetNextXdot1RemProtocolEntryByIndex */

#if (SYS_CPNT_CN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot1RemProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote congestion notification entry
 * INPUT    : xdot1_rem_cn_entry
 * OUTPUT   : xdot1_rem_cn_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot1RemCnEntry(LLDP_MGR_Xdot1RemCnEntry_T *xdot1_rem_cn_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;

    result = LLDP_TYPE_RETURN_ERROR;
    if (LLDP_OM_GetRemData(xdot1_rem_cn_entry->rem_local_port_num,
                          xdot1_rem_cn_entry->rem_index,
                          &rem_data))
    {
        if ((xdot1_rem_cn_entry->rem_time_mark <= rem_data->time_mark) &&
            (rem_data->xdot1_rem_cn_entry != NULL))
        {
            xdot1_rem_cn_entry->rem_cnpv_indicators = rem_data->xdot1_rem_cn_entry->cnpv_indicators;
            xdot1_rem_cn_entry->rem_ready_indicators = rem_data->xdot1_rem_cn_entry->ready_indicators;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    return result;
}/* End of LLDP_OM_GetXdot1RemCnEntry */
#endif /* #if (SYS_CPNT_CN == TRUE) */

#endif /* #if (LLDP_TYPE_EXT_802DOT1 == TRUE) */

#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetRunningXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry_p)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config_p;

    result = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    port_config_p = LLDP_OM_GetPortConfigEntryPtr(xdot3_port_config_entry_p->lport);

    xdot3_port_config_entry_p->tlvs_tx_changed = port_config_p->xdot3_tlvs_tx_flag ^ LLDP_TYPE_DEFAULT_XDOT3_PORT_CONFIG;
    if (xdot3_port_config_entry_p->tlvs_tx_changed != 0)
    {
        xdot3_port_config_entry_p->tlvs_tx_enable = port_config_p->xdot3_tlvs_tx_flag;

        if (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_MAC_PHY_TLV_TX)
        {
            xdot3_port_config_entry_p->mac_phy_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->mac_phy_tlv_enabled = FALSE;
        }

        if (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_POWER_VIA_MDI_TX)
        {
            xdot3_port_config_entry_p->power_via_mdi_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->power_via_mdi_tlv_enabled = FALSE;
        }

        if (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_LINK_AGG_TX)
        {
            xdot3_port_config_entry_p->link_agg_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->link_agg_tlv_enabled = FALSE;
        }

        if (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_MAX_FRAME_SIZE_TLV)
        {
            xdot3_port_config_entry_p->max_frame_size_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->max_frame_size_tlv_enabled = FALSE;
        }

        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return result;
}/* End of LLDP_OM_GetRunningXdot3PortConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data_p;

    result = LLDP_TYPE_RETURN_ERROR;
    if(LLDP_OM_GetRemData(xdot3_rem_port_entry->rem_local_port_num, xdot3_rem_port_entry->rem_index, &rem_data_p))
    {
        if(xdot3_rem_port_entry->rem_time_mark <= rem_data_p->time_mark &&
           rem_data_p->xdot3_rem_port_entry)
        {
            xdot3_rem_port_entry->rem_port_auto_neg_adv_cap = rem_data_p->xdot3_rem_port_entry->rem_port_auto_neg_adv_cap;
            xdot3_rem_port_entry->rem_port_auto_neg_enable = rem_data_p->xdot3_rem_port_entry->rem_auto_neg_enable;
            xdot3_rem_port_entry->rem_port_auto_neg_supported = rem_data_p->xdot3_rem_port_entry->rem_auto_neg_support;
            xdot3_rem_port_entry->rem_port_oper_mau_type = rem_data_p->xdot3_rem_port_entry->rem_port_oper_mau_type;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot3RemPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote power entry
 * INPUT    : xdot3_rem_power_entry
 * OUTPUT   : xdot3_rem_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot3RemPowerEntry(LLDP_MGR_Xdot3RemPowerEntry_T *xdot3_rem_power_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;

    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_OM_GetRemData(xdot3_rem_power_entry->rem_local_port_num, xdot3_rem_power_entry->rem_index, &rem_data))
    {
        if(xdot3_rem_power_entry->rem_time_mark <= rem_data->time_mark &&
           rem_data->xdot3_rem_power_entry)
        {
            xdot3_rem_power_entry->rem_power_class = rem_data->xdot3_rem_power_entry->rem_power_class;
            xdot3_rem_power_entry->rem_power_mdi_enabled = rem_data->xdot3_rem_power_entry->rem_power_mdi_enabled;
            xdot3_rem_power_entry->rem_power_mdi_supported = rem_data->xdot3_rem_power_entry->rem_power_mdi_supported;
            xdot3_rem_power_entry->rem_power_pairs = rem_data->xdot3_rem_power_entry->rem_power_pairs;
            xdot3_rem_power_entry->rem_power_pair_controlable = rem_data->xdot3_rem_power_entry->rem_power_pair_controlable;
            xdot3_rem_power_entry->rem_power_port_class = rem_data->xdot3_rem_power_entry->rem_power_port_class;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;

    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_OM_GetRemData(xdot3_rem_link_agg_entry->rem_local_port_num, xdot3_rem_link_agg_entry->rem_index, &rem_data))
    {
        if(xdot3_rem_link_agg_entry->rem_time_mark <= rem_data->time_mark &&
           rem_data->xdot3_rem_link_agg_entry)
        {
            xdot3_rem_link_agg_entry->rem_link_agg_port_id = rem_data->xdot3_rem_link_agg_entry->rem_link_agg_port_id;
            xdot3_rem_link_agg_entry->rem_link_agg_status = rem_data->xdot3_rem_link_agg_entry->rem_link_agg_status;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;

    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_OM_GetRemData(xdot3_rem_max_frame_size_entry->rem_local_port_num, xdot3_rem_max_frame_size_entry->rem_index, &rem_data))
    {
        if(xdot3_rem_max_frame_size_entry->rem_time_mark <= rem_data->time_mark &&
           rem_data->xdot3_rem_max_frame_size)
        {
            xdot3_rem_max_frame_size_entry->rem_max_frame_size = rem_data->xdot3_rem_max_frame_size;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    return result;
}
#endif /* #if (LLDP_TYPE_EXT_802DOT3 == TRUE) */

#if (LLDP_TYPE_MED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED configuration entry
 * INPUT    : None
 * OUTPUT   : xmed_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetXMedConfigEntry(LLDP_MGR_XMedConfig_T *xmed_config_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_SysConfigEntry_T    *sys_config;

    sys_config = LLDP_OM_GetSysConfigEntryPtr();

    xmed_config_entry->lldp_xmed_loc_device_class = 4;  /* Device type: Network connectivity device(4) */
    xmed_config_entry->lldp_xmed_fast_start_repeat_count = sys_config->fast_start_repeat_count;
    result = TRUE;

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningXMedFastStartRepeatCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED running cfg fast start repeat count
 * INPUT    : None
 * OUTPUT   : repeat_count
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningXMedFastStartRepeatCount(UI32_T  *repeat_count)
{
    UI32_T                  result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    LLDP_OM_SysConfigEntry_T    *sys_config;

    sys_config = LLDP_OM_GetSysConfigEntryPtr();

    if(sys_config->fast_start_repeat_count != LLDP_TYPE_DEFAULT_FAST_START_REPEAT_COUNT)
    {
        *repeat_count = sys_config->fast_start_repeat_count;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemCapEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device capability entry
 * INPUT    : None
 * OUTPUT   : rem_cap_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemCapEntry(LLDP_MGR_XMedRemCapEntry_T *rem_cap_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;

    if(LLDP_OM_GetRemData(rem_cap_entry->rem_local_port_num, rem_cap_entry->rem_index, &rem_data))
    {
        if(rem_data->time_mark >= rem_cap_entry->rem_time_mark)
        {
            if (rem_data->lldp_med_device_type != 0)
            {
                rem_cap_entry->rem_device_class = rem_data->lldp_med_device_type;
                rem_cap_entry->rem_cap_supported = rem_data->lldp_med_cap_sup;
                rem_cap_entry->rem_cap_current = rem_data->lldp_med_cap_enabled;
                result = TRUE;
            }
        }
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemMediaPolicyEntry(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       rem_data, *rem_data_p;
    UI8_T                   app_type;

    if (rem_med_policy_entry->rem_app_type < VAL_lldpXMedRemMediaPolicyAppType_voice ||
        rem_med_policy_entry->rem_app_type > LLDP_TYPE_MED_MAX_NETWORK_POLITY_TYPE)
    {
        return FALSE;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = rem_med_policy_entry->rem_time_mark;
    rem_data.lport     = rem_med_policy_entry->rem_local_port_num;
    rem_data.index     = rem_med_policy_entry->rem_index;

    if (LLDP_OM_GetRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->med_rem_network_policy)
        {
            app_type = rem_med_policy_entry->rem_app_type;
            if (rem_data_p->med_rem_network_policy->app_type[app_type-1].valid)
            {
                rem_med_policy_entry->rem_dscp     = rem_data_p->med_rem_network_policy->app_type[app_type-1].dscp;
                rem_med_policy_entry->rem_priority = rem_data_p->med_rem_network_policy->app_type[app_type-1].priority;
                rem_med_policy_entry->rem_tagged   = (rem_data_p->med_rem_network_policy->app_type[app_type-1].tagged)? VAL_lldpXMedRemMediaPolicyTagged_true : VAL_lldpXMedRemMediaPolicyTagged_false;
                rem_med_policy_entry->rem_unknown  = (rem_data_p->med_rem_network_policy->app_type[app_type-1].unknown)? VAL_lldpXMedRemMediaPolicyUnknown_true : VAL_lldpXMedRemMediaPolicyUnknown_false;
                rem_med_policy_entry->rem_vid      = rem_data_p->med_rem_network_policy->app_type[app_type-1].vid;
                result = TRUE;
            }
        }
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device inventory entry
 * INPUT    : None
 * OUTPUT   : rem_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemInventoryEntry(LLDP_MGR_XMedRemInventoryEntry_T *rem_inventory_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;

    if(LLDP_OM_GetRemData(rem_inventory_entry->rem_local_port_num,
                          rem_inventory_entry->rem_index,
                          &rem_data))
    {
        if(rem_data->time_mark >= rem_inventory_entry->rem_time_mark)
        {
            if(rem_data->med_rem_inventory)
            {
                rem_inventory_entry->rem_asset_id_len = rem_data->med_rem_inventory->asset_id_len;
                memcpy(rem_inventory_entry->rem_asset_id,
                       rem_data->med_rem_inventory->asset_id,
                       rem_inventory_entry->rem_asset_id_len);

                rem_inventory_entry->rem_firmware_rev_len = rem_data->med_rem_inventory->firmware_revision_len;
                memcpy(rem_inventory_entry->rem_firmware_rev,
                       rem_data->med_rem_inventory->firmware_revision,
                       rem_inventory_entry->rem_firmware_rev_len);

                rem_inventory_entry->rem_hardware_rev_len = rem_data->med_rem_inventory->hardware_revision_len;
                memcpy(rem_inventory_entry->rem_hardware_rev,
                       rem_data->med_rem_inventory->hardware_revision,
                       rem_inventory_entry->rem_hardware_rev_len);

                rem_inventory_entry->rem_mfg_name_len = rem_data->med_rem_inventory->manufaturer_name_len;
                memcpy(rem_inventory_entry->rem_mfg_name,
                       rem_data->med_rem_inventory->manufaturer_name,
                       rem_inventory_entry->rem_mfg_name_len);

                rem_inventory_entry->rem_model_name_len = rem_data->med_rem_inventory->model_name_len;
                memcpy(rem_inventory_entry->rem_model_name,
                       rem_data->med_rem_inventory->model_name,
                       rem_inventory_entry->rem_model_name_len);

                rem_inventory_entry->rem_serial_num_len = rem_data->med_rem_inventory->serial_num_len;
                memcpy(rem_inventory_entry->rem_serial_num,
                       rem_data->med_rem_inventory->serial_num,
                       rem_inventory_entry->rem_serial_num_len);

                rem_inventory_entry->rem_software_rev_len = rem_data->med_rem_inventory->software_revision_len;
                memcpy(rem_inventory_entry->rem_software_rev,
                       rem_data->med_rem_inventory->software_revision,
                       rem_inventory_entry->rem_software_rev_len);

                result = TRUE;
            }
        }
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used only for SNMP.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemLocationEntry(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;

    if(LLDP_OM_GetRemData(rem_location_entry->rem_local_port_num,
                          rem_location_entry->rem_index,
                          &rem_data))
    {
        if(rem_data->time_mark >= rem_location_entry->rem_time_mark && rem_data->med_rem_location)
        {
            if (rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_coordinateBased))
            {
#if 0 // TODO
                rem_location_entry->rem_location_subtype = VAL_lldpXMedRemLocationSubtype_coordinateBased;
                result = TRUE;
#endif
            }
            else if (rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_civicAddress))
            {
                LLDP_OM_XMedLocationCivicAddrCaTlv_T *ca_entry;
                UI8_T   *ca;

                rem_location_entry->rem_location_subtype = VAL_lldpXMedRemLocationSubtype_civicAddress;
                rem_location_entry->rem_location_info[1] = rem_data->med_rem_location->civic_addr.what;
                memcpy(&rem_location_entry->rem_location_info[2],
                rem_data->med_rem_location->civic_addr.country_code, 2);

                ca = &rem_location_entry->rem_location_info[4];

                if (rem_data->med_rem_location->civic_addr.ca_list.nbr_of_element != 0 &&
                    L_SORT_LST_Get_1st(&rem_data->med_rem_location->civic_addr.ca_list, &ca_entry))
                {
                    do
                    {
                        ca[0] = ca_entry->ca_type;
                        ca[1] = ca_entry->ca_length;
                        memcpy(&ca[2], ca_entry->ca_value, ca[1]);
                        ca += (2 + ca[1]);
                    } while (L_SORT_LST_Get_Next(&rem_data->med_rem_location->civic_addr.ca_list, &ca_entry));
                }

                rem_location_entry->rem_location_info_len = ca - rem_location_entry->rem_location_info;
                rem_location_entry->rem_location_info[0] = rem_location_entry->rem_location_info_len - 1;
                result = TRUE;
            }
            else if (rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_elin))
            {
                rem_location_entry->rem_location_subtype = VAL_lldpXMedRemLocationSubtype_elin;
                rem_location_entry->rem_location_info_len = rem_data->med_rem_location->elin_addr.elin_len;
                memcpy(rem_location_entry->rem_location_info, rem_data->med_rem_location->elin_addr.elin, rem_data->med_rem_location->elin_addr.elin_len);
                result = TRUE;
            }
        }
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe entry
 * INPUT    : None
 * OUTPUT   : rem_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemPoeEntry(LLDP_MGR_XMedRemPoeEntry_T *rem_poe_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;

    if(LLDP_OM_GetRemData(rem_poe_entry->rem_local_port_num,
                          rem_poe_entry->rem_index,
                          &rem_data))
    {
        if(rem_data->time_mark >= rem_poe_entry->rem_time_mark && rem_data->med_rem_ext_power)
        {
            rem_poe_entry->rem_poe_device_type = rem_data->med_rem_ext_power->power_type;

            result = TRUE;
        }
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemPoePseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pse_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemPoePseEntry(LLDP_MGR_XMedRemPoePseEntry_T *rem_pse_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;

    if(LLDP_OM_GetRemData(rem_pse_entry->rem_local_port_num,
                          rem_pse_entry->rem_index,
                          &rem_data))
    {
        if(rem_data->time_mark >= rem_pse_entry->rem_time_mark)
        {
            if(rem_data->med_rem_ext_power && rem_data->med_rem_ext_power->power_type == 2)
            {
                rem_pse_entry->rem_pse_power_av = rem_data->med_rem_ext_power->power_value;
                rem_pse_entry->rem_pse_power_priority = rem_data->med_rem_ext_power->power_priority;
                rem_pse_entry->rem_pse_power_source = rem_data->med_rem_ext_power->power_source;
                result = TRUE;
            }
        }
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemPoePdEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pd_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemPoePdEntry(LLDP_MGR_XMedRemPoePdEntry_T *rem_pd_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;

    if(LLDP_OM_GetRemData(rem_pd_entry->rem_local_port_num,
                          rem_pd_entry->rem_index,
                          &rem_data))
    {
        if(rem_data->time_mark >= rem_pd_entry->rem_time_mark)
        {
            if(rem_data->med_rem_ext_power && rem_data->med_rem_ext_power->power_type == 3)
            {
                rem_pd_entry->rem_pd_power_req= rem_data->med_rem_ext_power->power_value;
                rem_pd_entry->rem_pd_power_priority = rem_data->med_rem_ext_power->power_priority;
                rem_pd_entry->rem_pd_power_source = rem_data->med_rem_ext_power->power_source;
                result = TRUE;
            }
        }
    }

    return result;
}
#endif /* #if (LLDP_TYPE_MED == TRUE) */

#if (SYS_CPNT_ADD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextTelephoneDevice
 *-------------------------------------------------------------------------
 * PURPOSE  : To get next telephone device
 * INPUT    : lport, rem_dev_index
 * OUTPUT   : mac_addr, network_addr_subtype, network_addr, network_addr_len
 *            network_addr_ifindex
 * RETURN   : TRUE/FALSE
 * NOTE     : To get first entry, rem_dev_index is set to zero.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetNextTelephoneDevice(UI32_T lport,
                                        UI32_T *rem_dev_index,
                                        UI8_T *mac_addr,
                                        UI8_T *network_addr_subtype,
                                        UI8_T *network_addr,
                                        UI8_T *network_addr_len,
                                        UI32_T *network_addr_ifindex)
{
    BOOL_T                      result = FALSE;
    LLDP_OM_RemData_T           *rem_data, *rem_data_tmp;
    LLDP_OM_RemManAddrEntry_T   *rem_man_addr;
    L_SORT_LST_List_T           *device_port_list;

    rem_data = (LLDP_OM_RemData_T *)malloc(sizeof(LLDP_OM_RemData_T));
    rem_data_tmp = rem_data;
    rem_data->lport = lport;
    rem_data->index = *rem_dev_index;

    device_port_list = LLDP_OM_GetPortDeviceList(lport);

    while(!result && L_SORT_LST_Get_Next(device_port_list, &rem_data))
    {
        if((rem_data->rem_sys_entry->rem_sys_cap_enabled & (1 << VAL_lldpRemSysCapEnabled_telephone)) &&
           (rem_data->rem_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_MAC_ADDR))
        {
            *rem_dev_index = rem_data->index;
            memcpy(mac_addr, rem_data->rem_port_id, SYS_ADPT_MAC_ADDR_LEN);
            if(L_SORT_LST_Get_1st(&rem_data->rem_man_addr_list, &rem_man_addr))
            {
                memcpy(network_addr, rem_man_addr->rem_man_addr, rem_man_addr->rem_man_addr_len);
                *network_addr_len = rem_man_addr->rem_man_addr_len;
                *network_addr_subtype = rem_man_addr->rem_man_addr_subtype;
                *network_addr_ifindex = rem_man_addr->rem_man_addr_if_id;
            }
            result = TRUE;
        }
    }

    free(rem_data_tmp);

    return result;
}
#endif /* #if (SYS_CPNT_ADD == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetAllRemIndexByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get all remote index for a port
 * INPUT    : lport
 * OUTPUT   : remote_index
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetAllRemIndexByPort(UI32_T lport, UI32_T remote_index[LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT])
{
    LLDP_OM_RemData_T   *rem_data;
    UI32_T              i;

    if ((lport < 1) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return FALSE;
    }

    memset(remote_index, 0, sizeof(UI32_T)*LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT);

    if (L_SORT_LST_Get_1st(&LLDP_OM_RemDataList[lport-1], &rem_data) == FALSE)
    {
        return FALSE;
    }

    i = 0;
    do
    {
        remote_index[i] = rem_data->index;
        i++;
    } while (L_SORT_LST_Get_Next(&LLDP_OM_RemDataList[lport-1], &rem_data) == TRUE);

    return TRUE;
}


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for LLDP OM.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    LLDP_OM_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;

    /* dispatch IPC message and call the corresponding LLDP_OM function
     */
    switch (msg_p->type.cmd)
    {
        case LLDP_OM_IPC_GETSYSADMINSTATUS:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetSysAdminStatus(&msg_p->data.arg_ui32);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case LLDP_OM_IPC_GETRUNNINGSYSADMINSTATUS:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetRunningSysAdminStatus(&msg_p->data.arg_ui32);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case LLDP_OM_IPC_GETSYSCONFIGENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetSysConfigEntry(&msg_p->data.arg_sys_config_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_sys_config_entry);
            break;

        case LLDP_OM_IPC_GETREMOTESYSTEMDATA:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetRemoteSystemData(&msg_p->data.arg_remote_system_data);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_remote_system_data);
            break;

        case LLDP_OM_IPC_GETNEXTREMMANADDRBYINDEX:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetNextRemManAddrByIndex(&msg_p->data.arg_remote_mgmt_addr_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_remote_mgmt_addr_entry);
            break;

        case LLDP_OM_IPC_GETRUNNINGMSGTXHOLDMUL:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetRunningMsgTxHoldMul(&msg_p->data.arg_ui32);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case LLDP_OM_IPC_GETRUNNINGMSGTXINTERVAL:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetRunningMsgTxInterval(&msg_p->data.arg_ui32);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case LLDP_OM_IPC_GETRUNNINGNOTIFYINTERVAL:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetRunningNotifyInterval(&msg_p->data.arg_ui32);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case LLDP_OM_IPC_GETRUNNINGREINITDELAY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetRunningReinitDelay(&msg_p->data.arg_ui32);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case LLDP_OM_IPC_GETRUNNINGTXDELAY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetRunningTxDelay(&msg_p->data.arg_ui32);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case LLDP_OM_IPC_GETRUNNINGPORTADMINSTATUS:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 = LLDP_OM_GetRunningPortAdminStatus(
                msg_p->data.arg_grp_ui32_ui8.arg_ui32,
                &msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_grp_ui32_ui8);
            break;

        case LLDP_OM_IPC_GETRUNNINGPORTBASICTLVTRANSFER:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 = LLDP_OM_GetRunningPortBasicTlvTransfer(
                msg_p->data.arg_grp_ui32_ui8_ui8.arg_ui32,
                &msg_p->data.arg_grp_ui32_ui8_ui8.arg_ui8_1,
                &msg_p->data.arg_grp_ui32_ui8_ui8.arg_ui8_2);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_grp_ui32_ui8_ui8);
            break;

        case LLDP_OM_IPC_GETRUNNINGPORTMANADDRTLVTRANSFER:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 = LLDP_OM_GetRunningPortManAddrTlvTransfer(
                msg_p->data.arg_grp_ui32_ui8.arg_ui32,
                &msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_grp_ui32_ui8);
            break;

        case LLDP_OM_IPC_GETRUNNINGPORTNOTIFICATIONENABLE:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 = LLDP_OM_GetRunningPortNotificationEnable(
                msg_p->data.arg_grp_ui32_ui8.arg_ui32,
                &msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_grp_ui32_ui8);
            break;

#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
        case LLDP_OM_IPC_GETRUNNINGXDOT1CONFIGENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetRunningXdot1ConfigEntry(&msg_p->data.arg_xdot1_config_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_config_entry);
            break;

        case LLDP_OM_IPC_GETXDOT1REMENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetXdot1RemEntry(&msg_p->data.arg_xdot1_rem_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_entry);
            break;

        case LLDP_OM_IPC_GETXDOT1REMPROTOVLANENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetXdot1RemProtoVlanEntry(&msg_p->data.arg_xdot1_rem_proto_vlan_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_proto_vlan_entry);
            break;

        case LLDP_OM_IPC_GETNEXTXDOT1REMPROTOVLANENTRYBYINDEX:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetNextXdot1RemProtoVlanEntryByIndex(&msg_p->data.arg_xdot1_rem_proto_vlan_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_proto_vlan_entry);
            break;

        case LLDP_OM_IPC_GETXDOT1REMVLANNAMEENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetXdot1RemVlanNameEntry(&msg_p->data.arg_xdot1_rem_vlan_name_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_vlan_name_entry);
            break;

        case LLDP_OM_IPC_GETNEXTXDOT1REMVLANNAMEENTRYBYINDEX:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetNextXdot1RemVlanNameEntryByIndex(&msg_p->data.arg_xdot1_rem_vlan_name_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_vlan_name_entry);
            break;

        case LLDP_OM_IPC_GETXDOT1REMPROTOCOLENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetXdot1RemProtocolEntry(&msg_p->data.arg_xdot1_rem_protocol_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_protocol_entry);
            break;

        case LLDP_OM_IPC_GETNEXTXDOT1REMPROTOCOLENTRYBYINDEX:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetNextXdot1RemProtocolEntryByIndex(&msg_p->data.arg_xdot1_rem_protocol_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_protocol_entry);
            break;

#if (SYS_CPNT_CN == TRUE)
        case LLDP_OM_IPC_GETXDOT1REMCNENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetXdot1RemCnEntry(&msg_p->data.arg_xdot1_rem_cn_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_cn_entry);
            break;
#endif /* #if (SYS_CPNT_CN == TRUE) */
#endif /* #if (LLDP_TYPE_EXT_802DOT1 == TRUE) */

#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
        case LLDP_OM_IPC_GETRUNNINGXDOT3PORTCONFIGENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetRunningXdot3PortConfigEntry(&msg_p->data.arg_xdot3_port_config_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot3_port_config_entry);
            break;

        case LLDP_OM_IPC_GETXDOT3REMPORTENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetXdot3RemPortEntry(&msg_p->data.arg_xdot3_rem_port_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot3_rem_port_entry);
            break;

        case LLDP_OM_IPC_GETXDOT3REMPOWERENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetXdot3RemPowerEntry(&msg_p->data.arg_xdot3_rem_power_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot3_rem_power_entry);
            break;

        case LLDP_OM_IPC_GETXDOT3REMLINKAGGENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetXdot3RemLinkAggEntry(&msg_p->data.arg_xdot3_rem_link_agg_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot3_rem_link_agg_entry);
            break;

        case LLDP_OM_IPC_GETXDOT3REMMAXFRAMESIZEENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetXdot3RemMaxFrameSizeEntry(&msg_p->data.arg_xdot3_rem_max_frame_size_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot3_rem_max_frame_size_entry);
            break;
#endif /* #if (LLDP_TYPE_EXT_802DOT3 == TRUE) */

#if (LLDP_TYPE_MED == TRUE)
        case LLDP_OM_IPC_GETXMEDCONFIGENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_bool =
                LLDP_OM_GetXMedConfigEntry(&msg_p->data.arg_xmed_config_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_config_entry);
            break;

        case LLDP_OM_IPC_GETRUNNINGXMEDFASTSTARTREPEATCOUNT:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_ui32 =
                LLDP_OM_GetRunningXMedFastStartRepeatCount(&msg_p->data.arg_ui32);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case LLDP_OM_IPC_GETXMEDREMCAPENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_bool =
                LLDP_OM_GetXMedRemCapEntry(&msg_p->data.arg_xmed_rem_cap_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_cap_entry);
            break;

        case LLDP_OM_IPC_GETXMEDREMMEDIAPOLICYENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_bool =
                LLDP_OM_GetXMedRemMediaPolicyEntry(&msg_p->data.arg_xmed_rem_med_policy_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_med_policy_entry);
            break;

        case LLDP_OM_IPC_GETXMEDREMINVENTORYENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_bool =
                LLDP_OM_GetXMedRemInventoryEntry(&msg_p->data.arg_xmed_rem_inventory_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_inventory_entry);
            break;

        case LLDP_OM_IPC_GETXMEDREMLOCATIONENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_bool =
                LLDP_OM_GetXMedRemLocationEntry(&msg_p->data.arg_xmed_rem_location_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_location_entry);
            break;

        case LLDP_OM_IPC_GETXMEDREMPOEENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_bool =
                LLDP_OM_GetXMedRemPoeEntry(&msg_p->data.arg_xmed_rem_poe_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_poe_entry);
            break;

        case LLDP_OM_IPC_GETXMEDREMPOEPSEENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_bool =
                LLDP_OM_GetXMedRemPoePseEntry(&msg_p->data.arg_xmed_rem_pse_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_pse_entry);
            break;

        case LLDP_OM_IPC_GETXMEDREMPOEPDENTRY:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_bool =
                LLDP_OM_GetXMedRemPoePdEntry(&msg_p->data.arg_xmed_rem_pd_entry);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_pd_entry);
            break;
#endif /* #if (LLDP_TYPE_MED == TRUE) */

        case LLDP_OM_IPC_GETALLREMINDEXBYPORT:
            /* +++ Enter critical region +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(LLDP_OM_SemId);
            msg_p->type.ret_bool = LLDP_OM_GetAllRemIndexByPort(
                msg_p->data.arg_grp_ui32_index.arg_ui32,
                msg_p->data.arg_grp_ui32_index.arg_index);
            /* Leave critical region*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(LLDP_OM_SemId, original_priority);
            msgbuf_p->msg_size = LLDP_OM_GET_MSG_SIZE(arg_grp_ui32_index);
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_ui32 = LLDP_TYPE_RETURN_ERROR;
            msgbuf_p->msg_size = LLDP_OM_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of VLAN_OM_HandleIPCReqMsg */

static void LLDP_OM_LocalFreeRemData(LLDP_OM_RemData_T *free_data)
{
    /* remote system entry */
    if (free_data->rem_sys_entry != NULL)
    {
        if (free_data->rem_sys_entry->rem_port_desc != NULL)
            free(free_data->rem_sys_entry->rem_port_desc);
        if (free_data->rem_sys_entry->rem_sys_name != NULL)
            free(free_data->rem_sys_entry->rem_sys_name);
        if (free_data->rem_sys_entry->rem_sys_desc != NULL)
            free(free_data->rem_sys_entry->rem_sys_desc);
        if (free_data->rem_sys_entry != NULL)
            free(free_data->rem_sys_entry);
    }

    /* remote management address list */
    LLDP_OM_FreeRemManAddrList(&free_data->rem_man_addr_list);

    /* xdot1 information */
    LLDP_OM_FreeRemProtocolList(&free_data->xdot1_rem_protocol_list);
    LLDP_OM_FreeRemVlanNameList(&free_data->xdot1_rem_vlan_name_list);
    LLDP_OM_FreeRemProtoVlanList(&free_data->xdot1_rem_proto_vlan_list);

    /* xdot3 information */
    if (free_data->xdot3_rem_link_agg_entry)
        free(free_data->xdot3_rem_link_agg_entry);
    if (free_data->xdot3_rem_port_entry)
        free(free_data->xdot3_rem_port_entry);
    if (free_data->xdot3_rem_power_entry)
        free(free_data->xdot3_rem_power_entry);

    /* MED information */
    if (free_data->med_rem_network_policy)
        free(free_data->med_rem_network_policy);
    if (free_data->med_rem_inventory)
        free(free_data->med_rem_inventory);
    if (free_data->med_rem_location)
    {
        if (free_data->med_rem_location->location_type_valid &
            BIT_VALUE(VAL_lldpXMedRemLocationSubtype_civicAddress))
        {
            LLDP_OM_FreeCivicAddrCaList(&free_data->med_rem_location->civic_addr.ca_list);
        }
        free(free_data->med_rem_location);
    }
    if (free_data->med_rem_ext_power)
        free(free_data->med_rem_ext_power);

    /* Free DCBX information */
    if(free_data->dcbx_rem_ets_config_entry)
        free(free_data->dcbx_rem_ets_config_entry);
    if(free_data->dcbx_rem_ets_recommend_entry)
        free(free_data->dcbx_rem_ets_recommend_entry);
    if(free_data->dcbx_rem_pfc_config_entry)
        free(free_data->dcbx_rem_pfc_config_entry);
    LLDP_OM_FreeRemAppPriorityList(&free_data->dcbx_rem_app_pri_list);

    return;
} /* End of LLDP_OM_LocalFreeRemData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_ReleaseConvertMem
 *-------------------------------------------------------------------------
 * PURPOSE  : Release memory dynamically allocated when converting remote
 *            message
 * INPUT    : free_data - pointer to the memory to be released
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_OM_ReleaseConvertMem(LLDP_OM_RemData_T *free_data)
{
    LLDP_OM_LocalFreeRemData(free_data);
} /* End of LLDP_OM_ReleaseConvertMem */

BOOL_T LLDP_OM_GetTlvChangeDetect(UI32_T tlv)
{
    return tlv_change_detect[tlv];
}

void LLDP_OM_SetTlvChangeDetect(UI32_T tlv, BOOL_T flag)
{
    tlv_change_detect[tlv] = flag;
}

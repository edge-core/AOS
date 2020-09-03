/* FUNCTION NAME: add_utest.h
 * PURPOSE:
 *	1. ADD unit test
 * NOTES:
 *
 * REASON:
 *    DESCRIPTION:
 *    CREATOR:	Junying
 *    Date       2006/04/26
 *
 * Copyright(C)      Accton Corporation, 2006
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "add_utest.h"

#if (ADD_DO_UNIT_TEST == TRUE)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "add_type.h"
#include "add_om.h"
#include "add_mgr.h"
#include "l_cvrt.h"
#include "vlan_mgr.h"
#include "amtr_mgr.h"
#include "cfgdb_mgr.h"
#include "leaf_es3626a.h"
#include "vlan_type.h"

#if 0
static UI32_T ADD_UTEST_GetPortVlanMemberStatus(UI32_T vid, UI32_T lport);
static void ADD_UTEST_ShowMacAddress();
static BOOL_T ADD_UTEST_FindAddress(UI32_T vid, UI8_T *mac, UI32_T lport);
#endif

static BOOL_T ADD_UTEST_TestSysCpnt();
static BOOL_T ADD_UTEST_TestOmInit();
static BOOL_T ADD_UTEST_TestOmVoiceVlanId();
static BOOL_T ADD_UTEST_TestOmVoiceVlanTimeout();
static BOOL_T ADD_UTEST_TestOmOui();
static BOOL_T ADD_UTEST_TestOmVoiceVlanPortMode();
static BOOL_T ADD_UTEST_TestOmVoiceVlanPortSecurityState();
static BOOL_T ADD_UTEST_TestOmVoiceVlanPortDisjoinWhen();
static BOOL_T ADD_UTEST_TestOmVoiceVlanPortJoinState();

static BOOL_T ADD_UTEST_TestEprs();

#if 0
static BOOL_T ADD_UTEST_TestFoo();
static BOOL_T ADD_UTEST_ChangePortMode();
static BOOL_T ADD_UTEST_TestAmtrNewAddr();
static BOOL_T ADD_UTEST_TestAmtrAgedAddr();
static BOOL_T ADD_UTEST_TestTimerMsgProcess();
static BOOL_T ADD_UTEST_TestOui();
static BOOL_T ADD_UTEST_TestTimeout();
static BOOL_T ADD_UTEST_TestAutoJoinOnRebooted();
#endif

/*---------------------------------------------------------------------------
 * Routine Name : ADD_UTEST_Main
 *---------------------------------------------------------------------------
 * Function : excute unit test main function
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void ADD_UTEST_Main()
{
}

/*---------------------------------------------------------------------------
 * Routine Name : ADD_UTEST_RunVoiceVlanUTest
 *---------------------------------------------------------------------------
 * Function : excute unit test main function
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
BOOL_T ADD_UTEST_RunVoiceVlanUTest()
{
    BOOL_T ret;
//    BOOL_T delete_self = FALSE;

    ret &= ADD_UTEST_TestSysCpnt();
    ret &= ADD_UTEST_TestOmInit();
    ret &= ADD_UTEST_TestOmVoiceVlanId();
    ret &= ADD_UTEST_TestOmVoiceVlanTimeout();
    ret &= ADD_UTEST_TestOmOui();
    ret &= ADD_UTEST_TestOmVoiceVlanPortMode();
    ret &= ADD_UTEST_TestOmVoiceVlanPortSecurityState();
    ret &= ADD_UTEST_TestOmVoiceVlanPortDisjoinWhen();
    ret &= ADD_UTEST_TestOmVoiceVlanPortJoinState();

    ret &= ADD_UTEST_TestEprs();
/***************************************************
    if(VLAN_MGR_IsVlanExisted(2) == FALSE)
    {
        if(VLAN_MGR_CreateVlan(2, VAL_dot1qVlanStatus_permanent) == FALSE)
        {
            printf("\r\nerror to create vlan 2");
        }
        else
        {
            delete_self = TRUE;
        }
    }

    if(VLAN_MGR_SetVlanPortMode(1, VAL_vlanPortMode_hybrid) == FALSE)
    {
        printf("\r\nerror to change port 1 to hybrid mode");
    }
    if(VLAN_MGR_SetVlanPortMode(2, VAL_vlanPortMode_hybrid) == FALSE)
    {
        printf("\r\nerror to change port 2 to hybrid mode");
    }
    SYSFUN_Sleep(20);

    ret &= ADD_UTEST_TestFoo();
    ret &= ADD_UTEST_ChangePortMode();
    ret &= ADD_UTEST_TestAmtrNewAddr();
    ret &= ADD_UTEST_TestAmtrAgedAddr();
    ret &= ADD_UTEST_TestTimerMsgProcess();
    ret &= ADD_UTEST_TestOui();
    ret &= ADD_UTEST_TestTimeout();
    ret &= ADD_UTEST_TestAutoJoinOnRebooted();

    if(delete_self == TRUE)
    {
        VLAN_MGR_DeleteVlan(2, VAL_dot1qVlanStatus_permanent);
    }
*************************************************************/
    return ret;
}

/* LOCAL SUBPROGRAM BODIES
 */
 #if 0
static UI32_T ADD_UTEST_GetPortVlanMemberStatus(UI32_T vid, UI32_T lport)
{
    UI32_T time_mark = 0;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
    UI32_T vid_ifindex;
    UI8_T  arBitList[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};

    VLAN_MGR_GetDot1qVlanCurrentEntry(time_mark, vid, &vlan_info);

    L_CVRT_convert_portList_2_portMap(vlan_info.dot1q_vlan_static_egress_ports, arBitList , SYS_ADPT_TOTAL_NBR_OF_LPORT, 0x31);
    VLAN_MGR_ConvertToIfindex(vid, &vid_ifindex);
    if (VLAN_MGR_IsPortVlanMember(vid_ifindex, lport))
    {
        if(arBitList[lport-1] == '1')
            return VAL_dot1qVlanStatus_permanent; /* static */
        else
            return VLAN_TYPE_VLAN_STATUS_VOICE; /* dynamic */
    }
    return 0;
}

static void ADD_UTEST_ShowMacAddress()
{
    AMTR_MGR_AddrEntry_T addr_entry;
    UI8_T tmp_buff[20] = {0};

    printf("\r\nvid mac-address       port state");
    memset(&addr_entry, 0, sizeof(AMTR_MGR_AddrEntry_T));
    addr_entry.l_port = 1;
    addr_entry.vid = 1;
    while(AMTR_MGR_GetNextVMAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS)==TRUE)
    {
       switch(addr_entry.attribute)
       {
       case AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT:
          strcpy(tmp_buff, " Learned");
          break;

       case AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT:
          strcpy(tmp_buff, " Permanent");
          break;

       case AMTR_MGR_ENTRY_ATTRIBUTE_DELETE_ON_RESET:
          strcpy(tmp_buff, " Delete-on-reset");
          break;

       case AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT_PSEC:
          strcpy(tmp_buff, " Permanent-PSEC");
          break;

       case AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT_PSEC:
       case AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT_PSEC_STATIC:
          strcpy(tmp_buff, " Learned-PSEC");
          break;

       case AMTR_MGR_ENTRY_ATTRIBUTE_COMMUNITY_VLAN:
          strcpy(tmp_buff, " Permanent-PVLAN");
          break;

       default:
          strcpy(tmp_buff, " Other");
          break;
       }

        printf("\r\n%-3ld %02X-%02X-%02X-%02X-%02X-%02X %-4ld %s",
            addr_entry.vid,
            addr_entry.mac[0],addr_entry.mac[1],addr_entry.mac[2],
            addr_entry.mac[3],addr_entry.mac[4],addr_entry.mac[5],
            addr_entry.l_port,
            tmp_buff);
    }
}

static BOOL_T ADD_UTEST_FindAddress(UI32_T vid, UI8_T *mac, UI32_T lport)
{
    AMTR_MGR_AddrEntry_T addr_entry;

    SYSFUN_Sleep(10);/* Make sure the AMTR do all thing. */
    memset(&addr_entry, 0, sizeof(AMTR_MGR_AddrEntry_T));
    addr_entry.l_port = 1;
    addr_entry.vid = 1;
    while(AMTR_MGR_GetNextVMAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS)==TRUE)
    {
        if(memcmp(addr_entry.mac, mac, 6) == 0 &&
            addr_entry.vid == vid && addr_entry.l_port == lport)
            return TRUE;
    }
    return FALSE;
}
#endif

static BOOL_T ADD_UTEST_TestSysCpnt()
{
    BOOL_T ret = TRUE;

#if (SYS_CPNT_CFGDB == TRUE)
#else
    printf("\r\n[%s:%d] CFGDB disabled", __FUNCTION__, __LINE__);
    ret &= FALSE;
#endif

#if (SYS_CPNT_LLDP == TRUE)
#else
    printf("\r\n[%s:%d] LLDP disable", __FUNCTION__, __LINE__);
#endif

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
#else
    printf("\r\n[%s:%d] XOR disable", __FUNCTION__, __LINE__);
#endif

#if (SYS_HWCFG_SUPPORT_FILTER_MAC == TRUE)
#else
    printf("\r\n[%s:%d] SYS_HWCFG_SUPPORT_FILTER_MAC no defined", __FUNCTION__, __LINE__);
    ret &= FALSE;
#endif

    if(TRUE == ret)
    {
        printf(".");
    }

    return ret;
}

static BOOL_T ADD_UTEST_TestOmInit()
{
    char title[] = "ADD_OM_Initialize";
    BOOL_T ret = TRUE;
    I32_T  vid;
    UI32_T timeout;
    UI32_T lport;
    UI32_T per_mode;
    UI32_T per_security;
    UI32_T per_disjoin_when;
    UI8_T  per_join_state;

    if(ADD_OM_Initialize() != TRUE)
    {
        printf("\r\n%d:Test %s Fail(initialize om)", __LINE__, title);
        ret = FALSE;
    }

//    ADD_OM_GetVoiceVlanState(&state);
//    if(state != SYS_DFLT_ADD_VOICE_VLAN_STATE)
//    {
//        printf("\r\n%d:Test %s Fail(incorrect default voice vlan state)", __LINE__, title);
//        ret = FALSE;
//    }

    ADD_OM_GetVoiceVlanId(&vid);
    if(vid != SYS_DFLT_ADD_VOICE_VLAN_ID)
    {
        printf("\r\n%d:Test %s Fail(incorrect default voice vlan id)", __LINE__, title);
        ret = FALSE;
    }

    ADD_OM_GetVoiceVlanAgingTime(&timeout);
    if(timeout != SYS_DFLT_ADD_VOICE_VLAN_TIMEOUT_MINUTE)
    {
        printf("\r\n%d:Test %s Fail(incorrect default voice vlan timeout)", __LINE__, title);
        ret = FALSE;
    }


    for(lport=1; lport<=SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
    {
        ADD_OM_GetVoiceVlanPortMode(lport, &per_mode);
        if(per_mode != SYS_DFLT_ADD_VOICE_VLAN_PORT_MODE)
        {
            printf("\r\n%d:Test %s Fail(incorrect default port%ld mode)", __LINE__, title, lport);
            ret = FALSE;
        }

        ADD_OM_GetVoiceVlanPortSecurityState(lport, &per_security);
        if(per_security != SYS_DFLT_ADD_VOICE_VLAN_PORT_SECURITY_STATE)
        {
            printf("\r\n%d:Test %s Fail(incorrect default port%ld security state)", __LINE__, title, lport);
            ret = FALSE;
        }

        ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &per_disjoin_when);
        if(per_disjoin_when != 0)
        {
            printf("\r\n%d:Test %s Fail(incorrect default port%ld disjoin when)", __LINE__, title, lport);
            ret = FALSE;
        }

        ADD_OM_GetVoiceVlanPortJoinState(lport, &per_join_state);
        if(per_join_state != ADD_TYPE_DISCOVERY_PROTOCOL_NONE)
        {
            printf("\r\n%d:Test %s Fail(incorrect default port%ld join state)", __LINE__, title, lport);
            ret = FALSE;
        }
    }
    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

static BOOL_T ADD_UTEST_TestOmVoiceVlanId()
{
    char title[] = "ADD_OM_Set/GetVoiceVlanId";
    BOOL_T ret = TRUE;
    I32_T  prev_id, id;

    ADD_OM_GetVoiceVlanId(&prev_id);

    ADD_OM_SetVoiceVlanId(1);
    if(ADD_OM_GetVoiceVlanId(0) != FALSE)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanId(&id);
    if(id != 1)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    ADD_OM_SetVoiceVlanId(prev_id);

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

static BOOL_T ADD_UTEST_TestOmVoiceVlanTimeout()
{
    char title[] = "ADD_OM_Set/GetVoiceVlanTimeout";
    BOOL_T ret = TRUE;
    UI32_T prev_timeout, timeout;

    ADD_OM_GetVoiceVlanAgingTime(&prev_timeout);

    ADD_OM_SetVoiceVlanAgingTime(10);
    if(ADD_OM_GetVoiceVlanAgingTime(0) != FALSE)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanAgingTime(&timeout);
    if(timeout != 10)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    ADD_OM_SetVoiceVlanAgingTime(prev_timeout);

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

static BOOL_T ADD_UTEST_TestOmOui()
{
    BOOL_T ret = TRUE;
    ADD_MGR_VoiceVlanOui_T prev_oui[SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI];
    int cnt_prev_oui;

    UI8_T addr_0000E8_000001[] = {0x00, 0x00, 0xE8, 0x00, 0x00, 0x01};
    UI8_T addr_0000E8_010001[] = {0x00, 0x00, 0xE8, 0x01, 0x00, 0x01};
    UI8_T addr_FFFFFF_000000[] = {0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00};
    UI8_T addr_FFFFFF_FF0000[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00};
    UI8_T tmp_oui[6], tmp_mask[6], tmp_desc[30+1];
    int i;

    memset(prev_oui, 0, sizeof(prev_oui));
    cnt_prev_oui = 0;

    i = 0;
    memset(tmp_oui, 255, sizeof(tmp_oui));
    while(ADD_OM_GetNextOui(tmp_oui, tmp_mask, tmp_desc)==TRUE)
    {
        memcpy(prev_oui[i].oui,         tmp_oui,  6);
        memcpy(prev_oui[i].mask,        tmp_mask, 6);
        memcpy(prev_oui[i].description, tmp_desc, 30+1);
        ++i;
    }

    cnt_prev_oui = i;

    for(--i;i>=0; --i)
    {
        ADD_OM_RemoveOui(prev_oui[i].oui, prev_oui[i].mask);
    }

    if(ADD_OM_AddOui(addr_0000E8_000001, addr_FFFFFF_000000, NULL) != TRUE)
    {
        printf("\r\n[%s:%d] Test fails", __FUNCTION__, __LINE__);
        ret = FALSE;
    }

    /*add duplicate test*/
    if(ADD_OM_AddOui(addr_0000E8_000001, addr_FFFFFF_000000, NULL) != FALSE)
    {
        printf("\r\n[%s:%d] Test fails", __FUNCTION__, __LINE__);
        ret = FALSE;
    }

    /*add duplicate test*/
    if(ADD_OM_AddOui(addr_0000E8_010001, addr_FFFFFF_FF0000, NULL) != FALSE)
    {
        printf("\r\n[%s:%d] Test fails", __FUNCTION__, __LINE__);
        ret = FALSE;
    }

    ADD_OM_RemoveOui(addr_0000E8_000001, addr_FFFFFF_000000);

    if(ADD_OM_AddOui(addr_0000E8_010001, addr_FFFFFF_FF0000, NULL) != TRUE)
    {
        printf("\r\n[%s:%d] Test fails", __FUNCTION__, __LINE__);
        ret = FALSE;
    }

    ADD_OM_RemoveOui(addr_0000E8_010001, addr_FFFFFF_FF0000);

    for(i=0; i<SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI; ++i)
    {
        addr_0000E8_010001[3] = i;
        if(ADD_OM_AddOui(addr_0000E8_010001, addr_FFFFFF_FF0000, NULL) != TRUE)
        {
            printf("\r\n[%s:%d] Test fails", __FUNCTION__, __LINE__);
        }
    }

    addr_0000E8_010001[3] = SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI;
    if(ADD_OM_AddOui(addr_0000E8_010001, addr_FFFFFF_FF0000, NULL) != FALSE)
    {
        printf("\r\n[%s:%d] Test fails", __FUNCTION__, __LINE__);
    }

    /*GetNext test*/
    i = 0;
    memset(tmp_oui, 255, sizeof(tmp_oui));
    while(ADD_OM_GetNextOui(tmp_oui, tmp_mask, tmp_desc)==TRUE)
    {
        ++i;
    }

    if(i != SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI-1)
    {
            printf("\r\n[%s:%d] Test fails", __FUNCTION__, __LINE__);
    }

    for(i=SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI-1; i>=0; --i)
    {
        addr_0000E8_010001[3] = i;
        if(ADD_OM_RemoveOui(addr_0000E8_010001, addr_FFFFFF_FF0000) != TRUE)
        {
            printf("\r\n[%s:%d] Test fails", __FUNCTION__, __LINE__);
        }
    }

    /* setdown */
    for(i=0; i<cnt_prev_oui; ++i)
    {
        ADD_OM_AddOui(prev_oui[i].oui, prev_oui[i].mask, prev_oui[i].description);
    }
    /*ADD_OM_ShowOui();*/

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

static BOOL_T ADD_UTEST_TestOmVoiceVlanPortMode()
{
    BOOL_T ret = TRUE;
    UI32_T prev_per_mode, per_mode;
    UI32_T lport;

    lport = 0;
    if(ADD_OM_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none) != FALSE)
    {
        printf("\r\n[%s:%d] Test fails lport(%ld)", __FUNCTION__, __LINE__, lport);
    }
    if(ADD_OM_GetVoiceVlanPortMode(lport, &per_mode) != FALSE)
    {
        printf("\r\n[%s:%d] Test fails lport(%ld)", __FUNCTION__, __LINE__, lport);
    }

    lport = SYS_ADPT_TOTAL_NBR_OF_LPORT+1;
    if(ADD_OM_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none) != FALSE)
    {
        printf("\r\n[%s:%d] Test fails lport(%ld)", __FUNCTION__, __LINE__, lport);
    }
    if(ADD_OM_GetVoiceVlanPortMode(lport, &per_mode) != FALSE)
    {
        printf("\r\n[%s:%d] Test fails lport(%ld)", __FUNCTION__, __LINE__, lport);
    }

    for(lport=1; lport<=SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
    {
        ADD_OM_GetVoiceVlanPortMode(lport, &prev_per_mode);

        ADD_OM_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
        if(ADD_OM_GetVoiceVlanPortMode(lport, 0) != FALSE)
        {
            printf("\r\n[%s:%d] Test fails lport(%ld)", __FUNCTION__, __LINE__, lport);
        }
        ADD_OM_GetVoiceVlanPortMode(lport, &per_mode);
        if(per_mode != VAL_voiceVlanPortMode_none)
        {
            printf("\r\n[%s:%d] Test fails lport(%ld)", __FUNCTION__, __LINE__, lport);
            ret = FALSE;
        }

        ADD_OM_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_manual);
        ADD_OM_GetVoiceVlanPortMode(lport, &per_mode);
        if(per_mode != VAL_voiceVlanPortMode_manual)
        {
            printf("\r\n[%s:%d] Test fails lport(%ld)", __FUNCTION__, __LINE__, lport);
            ret = FALSE;
        }

        ADD_OM_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_auto);
        ADD_OM_GetVoiceVlanPortMode(lport, &per_mode);
        if(per_mode != VAL_voiceVlanPortMode_auto)
        {
            printf("\r\n[%s:%d] Test fails lport(%ld)", __FUNCTION__, __LINE__, lport);
            ret = FALSE;
        }

        if(ADD_OM_SetVoiceVlanPortMode(lport, 0) != FALSE)
        {
            printf("\r\n[%s:%d] Test fails lport(%ld)", __FUNCTION__, __LINE__, lport);
            ret = FALSE;
        }

        if(ADD_OM_SetVoiceVlanPortMode(lport, 4) != FALSE)
        {
            printf("\r\n[%s:%d] Test fails lport(%ld)", __FUNCTION__, __LINE__, lport);
            ret = FALSE;
        }

        ADD_OM_SetVoiceVlanPortMode(lport, prev_per_mode);
    }

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

static BOOL_T ADD_UTEST_TestOmVoiceVlanPortSecurityState()
{
    char title[] = "ADD_OM_Set/GetVoiceVlanPortSecurityState";
    BOOL_T ret = TRUE;
    UI32_T prev_per_security_state, per_security_state;
    UI32_T lport;

    lport = 0;
    if(ADD_OM_SetVoiceVlanPortSecurityState(lport, VAL_voiceVlanPortSecurity_enabled) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }
    if(ADD_OM_GetVoiceVlanPortSecurityState(lport, &per_security_state) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }

    lport = SYS_ADPT_TOTAL_NBR_OF_LPORT+1;
    if(ADD_OM_SetVoiceVlanPortSecurityState(lport, VAL_voiceVlanPortSecurity_enabled) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }
    if(ADD_OM_GetVoiceVlanPortSecurityState(lport, &per_security_state) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }

    for(lport=1; lport<=SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
    {
        ADD_OM_GetVoiceVlanPortSecurityState(lport, &prev_per_security_state);

        ADD_OM_SetVoiceVlanPortSecurityState(lport, VAL_voiceVlanPortSecurity_enabled);
        if(ADD_OM_GetVoiceVlanPortSecurityState(lport, 0) != FALSE)
        {
            printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
        }
        ADD_OM_GetVoiceVlanPortSecurityState(lport, &per_security_state);
        if(per_security_state != VAL_voiceVlanPortSecurity_enabled)
        {
            printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
            ret = FALSE;
        }

        ADD_OM_SetVoiceVlanPortSecurityState(lport, VAL_voiceVlanPortSecurity_disabled);
        ADD_OM_GetVoiceVlanPortSecurityState(lport, &per_security_state);
        if(per_security_state != VAL_voiceVlanPortSecurity_disabled)
        {
            printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
            ret = FALSE;
        }

        ADD_OM_SetVoiceVlanPortSecurityState(lport, prev_per_security_state);
    }

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

static BOOL_T ADD_UTEST_TestOmVoiceVlanPortDisjoinWhen()
{
    char title[] = "ADD_OM_Set/GetVoiceVlanPortDisjoinWhen";
    BOOL_T ret = TRUE;
    UI32_T prev_per_disjoin_when, per_disjoin_when;
    UI32_T lport;

    lport = 0;
    if(ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, 5) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }
    if(ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &per_disjoin_when) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }

    lport = SYS_ADPT_TOTAL_NBR_OF_LPORT+1;
    if(ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, 5) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }
    if(ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &per_disjoin_when) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }

    for(lport=1; lport<=SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
    {
        ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &prev_per_disjoin_when);

        ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, 5);
        if(ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, 0) != FALSE)
        {
            printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
        }
        ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &per_disjoin_when);
        if(per_disjoin_when != 5)
        {
            printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
            ret = FALSE;
        }

        ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, prev_per_disjoin_when);
    }

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

BOOL_T ADD_UTEST_TestOmVoiceVlanPortJoinState()
{
    char title[] = "ADD_OM_Set/GetVoiceVlanPortDisjoinWhen";
    BOOL_T ret = TRUE;
    UI8_T  prev_per_join_state, per_join_state;
    UI32_T lport;

    lport = 0;
    if(ADD_OM_SetVoiceVlanPortJoinState(lport, ADD_TYPE_DISCOVERY_PROTOCOL_ALL) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }
    if(ADD_OM_GetVoiceVlanPortJoinState(lport, &per_join_state) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }

    lport = SYS_ADPT_TOTAL_NBR_OF_LPORT+1;
    if(ADD_OM_SetVoiceVlanPortJoinState(lport, ADD_TYPE_DISCOVERY_PROTOCOL_ALL) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }
    if(ADD_OM_GetVoiceVlanPortJoinState(lport, &per_join_state) != FALSE)
    {
        printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
    }

    for(lport=1; lport<=SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
    {
        ADD_OM_GetVoiceVlanPortJoinState(lport, &prev_per_join_state);

        ADD_OM_ClearVoiceVlanPortJoinState(lport,ADD_TYPE_DISCOVERY_PROTOCOL_ALL);
        ADD_OM_GetVoiceVlanPortJoinState(lport, &per_join_state);
        if(per_join_state != ADD_TYPE_DISCOVERY_PROTOCOL_NONE)
        {
            printf("\r\n%d:Test %s Fail(port%ld)", __LINE__, title, lport);
        }

        ADD_OM_SetVoiceVlanPortJoinState(lport, ADD_TYPE_DISCOVERY_PROTOCOL_OUI);
        ADD_OM_GetVoiceVlanPortJoinState(lport, &per_join_state);
        if(per_join_state != ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
        {
            printf("\r\n%d:Test %s Fail(set port%ld join state)", __LINE__, title, lport);
            ret = FALSE;
        }

        ADD_OM_SetVoiceVlanPortJoinState(lport, ADD_TYPE_DISCOVERY_PROTOCOL_LLDP);
        ADD_OM_GetVoiceVlanPortJoinState(lport, &per_join_state);
        if(per_join_state != (ADD_TYPE_DISCOVERY_PROTOCOL_OUI | ADD_TYPE_DISCOVERY_PROTOCOL_LLDP))
        {
            printf("\r\n%d:Test %s Fail(set port%ld join state)", __LINE__, title, lport);
            ret = FALSE;
        }

        ADD_OM_ClearVoiceVlanPortJoinState(lport,ADD_TYPE_DISCOVERY_PROTOCOL_OUI);
        ADD_OM_GetVoiceVlanPortJoinState(lport, &per_join_state);
        if(per_join_state != ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
        {
            printf("\r\n%d:Test %s Fail(set port%ld join state)", __LINE__, title, lport);
            ret = FALSE;
        }

        ADD_OM_SetVoiceVlanPortJoinState(lport, prev_per_join_state);
    }
    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

static BOOL_T ADD_UTEST_TestEprs()
{
    BOOL_T ret = TRUE;
    /* ES3524MO-PoE-FLF-26B-00148
     * Dynamic vlan can join to voice vlan
     */
    {
        UI32_T vid = 1;
        UI32_T vid_ifindex;
        VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
        for(; vid<4095; ++vid)
        {
            if(VLAN_MGR_IsVlanExisted(vid) == FALSE)
            {
                if(!VLAN_MGR_CreateVlan(vid, VAL_dot1qVlanStatus_dynamicGvrp))
                {
                    continue;
                }

                if(!VLAN_MGR_ConvertToIfindex(vid, &vid_ifindex)                                   ||
                   !VLAN_MGR_SetDot1qVlanStaticRowStatus(vid, VAL_dot1qVlanStaticRowStatus_active) ||
                   !ADD_MGR_SetVoiceVlanEnabledId(vid))
                {
                    VLAN_MGR_DeleteVlan(vid, VAL_dot1qVlanStatus_dynamicGvrp);
                    continue;
                }

                vlan_info.dot1q_vlan_index = vid_ifindex;
                VLAN_OM_GetVlanEntry(&vlan_info);
                if(vlan_info.dot1q_vlan_status != VAL_dot1qVlanStatus_permanent)
                {
                    printf("\r\nTest ES3524MO-PoE-FLF-26B-00148 Fails");
                    ret &= FALSE;
                }

                VLAN_MGR_DeleteVlan(vid, VAL_dot1qVlanStatus_dynamicGvrp);
                break;
            }
        }
        if(vid == 4095)
        {
            printf("\r\nTest ES3524MO-PoE-FLF-26B-00148 Fails");
            ret &= FALSE;
        }
    }
    return ret;
}

#if 0
static BOOL_T ADD_UTEST_TestFoo()
{
    char title[] = "foo";
    BOOL_T ret = TRUE;
    UI32_T lport = 1;
    UI32_T voice_vlan_id = 2;
    UI8_T addr_00E0_BB00_0001[] = {0x00, 0xE0, 0xBB, 0x00, 0x00, 0x01};
    UI8_T addr_00E0_BB00_0002[] = {0x00, 0xE0, 0xBB, 0x00, 0x00, 0x02};
    UI8_T addr_00E0_BB00_0003[] = {0x00, 0xE0, 0xBB, 0x00, 0x00, 0x03};
    UI8_T addr_00E0_BB00_0004[] = {0x00, 0xE0, 0xBB, 0x00, 0x00, 0x04};
//    UI8_T addr_00E0_XX00_0001[] = {0x00, 0xE0, 0xBB, 0x00, 0x00, 0x01};
//    UI8_T addr_0000_00FF_FFFF[] = {0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF};
//    UI8_T desc[50] = {0};
    AMTR_MGR_AddrEntry_T addr_entry;
    AMTR_MGR_AddrEntry_T addr_entry2;
    UI32_T count = 0;
//    UI32_T cnt_of_oui = 0;
//    UI32_T cnt_of_static_addr = 0;

/*    UI8_T mac[6]={0};
    UI32_T start, stop;
    UI32_T i;

    for(i=1; i<20; ++i)
    {
        addr_00E0_XX00_0001[2] = i;
        ADD_MGR_AddOui(addr_00E0_XX00_0001, addr_0000_00FF_FFFF, desc);
    }
    while(ADD_MGR_GetNextOui(addr_00E0_XX00_0001, addr_0000_00FF_FFFF, desc))
        ++cnt_of_oui;

    for(i=1; i<=(16*1024); ++i)
    {
        memcpy(mac+2, &i, sizeof(i));
        AMTR_MGR_SetStaticAddrEntry(1, mac, lport,AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);
    }
    AMTR_MGR_GetNumberOfStaticAddrEntry(&cnt_of_static_addr);

    start = SYSFUN_GetSysTick();
    printf("\r\nStart(%ld) CntOfOui=%ld CntOfMacTbl=%ld", start, cnt_of_oui, cnt_of_static_addr);
    memset(&addr_entry, 0, sizeof(AMTR_MGR_AddrEntry_T));
    while(AMTR_MGR_GetNextVMAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS)==TRUE)
    {
        ADD_OM_IsOui(addr_entry.mac);
    }
    stop = SYSFUN_GetSysTick();
    printf("\r\nStop(%ld), Diff=%ld tick(10ms)", stop, stop-start);*/

    /* test Foo setup */
    //printf("\r\nvlan status=%d", ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport));
    //VLAN_MGR_AddEgressPortMember(voice_vlan_id,  lport, VAL_dot1qVlanStatus_permanent);
    //printf("\r\nvlan status=%d", ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport));
    //VLAN_MGR_DeleteEgressPortMember(voice_vlan_id,  lport, VAL_dot1qVlanStatus_permanent);
    //printf("\r\nvlan status=%d", ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport));

    //VLAN_MGR_AddEgressPortMember(voice_vlan_id,  lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    //printf("\r\nvlan status=%d", ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport));
    //VLAN_MGR_DeleteEgressPortMember(voice_vlan_id,  lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    //printf("\r\nvlan status=%d", ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport));

    AMTR_MGR_DeleteAddr_ByLPort(lport);
    SYSFUN_Sleep(100);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setup fail)", __LINE__, title);
        ret = FALSE;
    }
    ADD_MGR_SetVoiceVlanEnabledId(-1);
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);

    ADD_MGR_SetVoiceVlanEnabledId(voice_vlan_id);

    VLAN_MGR_AddEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    //printf("\r\nvlan status=%d", ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport));
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_auto);
    //printf("\r\nvlan status=%d", ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport));
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_manual);

    //printf("\r\nvlan status=%d", ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport));
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VAL_dot1qVlanStatus_permanent)
    {
        printf("\r\n%d:Test %s Fail(static vlan be modified by voice method)", __LINE__, title);
        ret = FALSE;
    }

    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    //printf("\r\nvlan status=%d", ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport));
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VAL_dot1qVlanStatus_permanent)
    {
        printf("\r\n%d:Test %s Fail(static vlan be modified by voice method)", __LINE__, title);
        ret = FALSE;
    }

    VLAN_MGR_DeleteEgressPortMember(2, lport, VAL_dot1qVlanStatus_permanent);
    if(ADD_UTEST_GetPortVlanMemberStatus(2, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    VLAN_MGR_AddEgressPortMember(2, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(2, lport) != VLAN_TYPE_VLAN_STATUS_VOICE)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    VLAN_MGR_DeleteEgressPortMember(2, lport, VLAN_TYPE_VLAN_STATUS_VOICE);


    /* for amtr loop (learned count) */
    AMTR_MGR_SetStaticAddrEntry(1, addr_00E0_BB00_0001, 1, AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);
    AMTR_MGR_SetStaticAddrEntry(1, addr_00E0_BB00_0002, 1, AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);
    AMTR_MGR_SetStaticAddrEntry(1, addr_00E0_BB00_0003, 1, AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);
    AMTR_MGR_SetStaticAddrEntry(1, addr_00E0_BB00_0004, 1, AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);

    count = 0;
    memset(&addr_entry, 0, sizeof(AMTR_MGR_AddrEntry_T));
    while(AMTR_MGR_GetNextMVAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS)==TRUE)
    {
        AMTR_MGR_DeleteAddr(addr_entry.vid, addr_entry.mac);

        memset(&addr_entry2, 0, sizeof(AMTR_MGR_AddrEntry_T));
        while(AMTR_MGR_GetNextMVAddrEntry(&addr_entry2, AMTR_MGR_GET_ALL_ADDRESS)==TRUE)
        {
            ++count;
            /*printf("\r\n vid=%ld mac=%02X%02X-%02X%02X-%02X%02X port=%ld",
                addr_entry2.vid,
                addr_entry2.mac[0],addr_entry2.mac[1],addr_entry2.mac[2],
                addr_entry2.mac[3],addr_entry2.mac[4],addr_entry2.mac[5],
                addr_entry2.l_port);*/
        }
        /*printf("\r\n");*/
    }
    if(count != 10)
    {
        printf("\r\n%d:Test %s Fail(amtr loop)", __LINE__, title);
    }

    /* test Foo setdown */
    AMTR_MGR_DeleteAddr_ByLPort(lport);
    SYSFUN_Sleep(20);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setup fail)", __LINE__, title);
        ret = FALSE;
    }
    ADD_MGR_SetVoiceVlanEnabledId(-1);
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

static BOOL_T ADD_UTEST_ChangePortMode()
{
    char title[] = "Change the port mode";
    BOOL_T ret = TRUE;
    UI32_T voice_vlan_id = 2;
    UI32_T lport = 1;
    UI8_T addr_00E0_BB00_0001[] = {0x00, 0xE0, 0xBB, 0x00, 0x00, 0x01};

    /* test AMTR New Address setup */
    AMTR_MGR_DeleteAddr_ByLPort(lport);
    SYSFUN_Sleep(20);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setup fail)", __LINE__, title);
        ret = FALSE;
    }
    ADD_MGR_SetVoiceVlanEnabledId(-1);
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);

    AMTR_MGR_SetStaticAddrEntry(voice_vlan_id, addr_00E0_BB00_0001, lport, AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);
    AMTR_MGR_SetStaticAddrEntry(1, addr_00E0_BB00_0001, lport, AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);

    ADD_MGR_SetVoiceVlanEnabledId(voice_vlan_id);
    ADD_OM_SetVoiceVlanPortRuleBitmap(lport, ADD_TYPE_DISCOVERY_PROTOCOL_OUI);

    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_auto);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VLAN_TYPE_VLAN_STATUS_VOICE)
    {
        printf("\r\n%d:Test %s Fail(error for process na) vid=%ld,mac=%02X:%02X:%02X:%02X:%02X:%02X,lport=%ld", __LINE__, title,
            1L,
            addr_00E0_BB00_0001[0],addr_00E0_BB00_0001[1],addr_00E0_BB00_0001[2],
            addr_00E0_BB00_0001[3],addr_00E0_BB00_0001[4],addr_00E0_BB00_0001[5],
            lport);
        ret = FALSE;
    }

    AMTR_MGR_DeleteAddr_ByLPort(lport);
    SYSFUN_Sleep(20);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setup fail)", __LINE__, title);
        ret = FALSE;
    }
    ADD_MGR_SetVoiceVlanEnabledId(-1);
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

/* ASSUMPTION: lport 1 is hybird mode
               Vlan 2 is exited
 */
static BOOL_T ADD_UTEST_TestAmtrNewAddr()
{
    char title[] = "AMTR New Address";
    BOOL_T ret = TRUE;
    UI32_T voice_vlan_id = 2;
    UI32_T lport = 1;
    UI32_T disjoin_when;
    UI8_T addr_00E0_AA00_0001[] = {0x00, 0xE0, 0xAA, 0x00, 0x00, 0x01};
    UI8_T addr_00E0_BB00_0001[] = {0x00, 0xE0, 0xBB, 0x00, 0x00, 0x01};

    /* test AMTR New Address setup */
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setup fail)", __LINE__, title);
        ret = FALSE;
    }
    ADD_MGR_SetVoiceVlanEnabledId(-1);
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);


    /* Sample oui match test */
    /*ADD_OM_InitOui();*/
    if(ADD_OM_IsOui(addr_00E0_AA00_0001) != FALSE)
    {
        printf("\r\n%d:Test %s Fail, mac=%02X:%02X:%02X:%02X:%02X:%02X", __LINE__, title,
            addr_00E0_AA00_0001[0],addr_00E0_AA00_0001[1],addr_00E0_AA00_0001[2],
            addr_00E0_AA00_0001[3],addr_00E0_AA00_0001[4],addr_00E0_AA00_0001[5]);
        ret = FALSE;
    }
    if(ADD_OM_IsOui(addr_00E0_BB00_0001) != TRUE)
    {
        printf("\r\n%d:Test %s Fail, mac=%02X:%02X:%02X:%02X:%02X:%02X", __LINE__, title,
            addr_00E0_BB00_0001[0],addr_00E0_BB00_0001[1],addr_00E0_BB00_0001[2],
            addr_00E0_BB00_0001[3],addr_00E0_BB00_0001[4],addr_00E0_BB00_0001[5]);
        ret = FALSE;
    }
    /* End of Sample oui match test */

    ADD_MGR_SetVoiceVlanEnabledId(voice_vlan_id);
    ADD_OM_SetVoiceVlanPortRuleBitmap(lport, ADD_TYPE_DISCOVERY_PROTOCOL_OUI);

    /* for auto mode
     * 1. attach a nonphone to this port
     * 2. attach a phone to this port
     */
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_auto);
    ADD_MGR_AMTR_EditAddrNotify_CallBack(lport, addr_00E0_AA00_0001, lport, FALSE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(error for process na) vid=%ld,mac=%02X:%02X:%02X:%02X:%02X:%02X,lport=%ld", __LINE__, title,
            1L,
            addr_00E0_AA00_0001[0],addr_00E0_AA00_0001[1],addr_00E0_AA00_0001[2],
            addr_00E0_AA00_0001[3],addr_00E0_AA00_0001[4],addr_00E0_AA00_0001[5],
            lport);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(incorrect lport%ld disjoin timer state)", __LINE__, title,
            lport);
        ret = FALSE;
    }

    ADD_MGR_AMTR_EditAddrNotify_CallBack(lport, addr_00E0_BB00_0001, lport, FALSE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VLAN_TYPE_VLAN_STATUS_VOICE)//VAL_dot1qVlanStatus_voice)
    {
        printf("\r\n%d:Test %s Fail(error for process na) vid=%ld,mac=%02X:%02X:%02X:%02X:%02X:%02X,lport=%ld", __LINE__, title,
            1L,
            addr_00E0_BB00_0001[0],addr_00E0_BB00_0001[1],addr_00E0_BB00_0001[2],
            addr_00E0_BB00_0001[3],addr_00E0_BB00_0001[4],addr_00E0_BB00_0001[5],
            lport);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(incorrect lport%ld disjoin timer state)", __LINE__, title,
            lport);
        ret = FALSE;
    }

    /* disable voice vlan feature for lport 1 */
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(incorrect lport%ld disjoin timer state)", __LINE__, title,
            lport);
        ret = FALSE;
    }

    /* for manual mode
     * 1. attach a phone to this port
     */
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_manual);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(error on changing port mode to manual mode)", __LINE__, title);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(incorrect lport%ld disjoin timer state)", __LINE__, title,
            lport);
        ret = FALSE;
    }

    /*ADD_UTEST_ShowMacAddress();*/

    ADD_MGR_AMTR_EditAddrNotify_CallBack(1, addr_00E0_BB00_0001, lport, FALSE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(error for process na) vid=%ld,mac=%02X:%02X:%02X:%02X:%02X:%02X,lport=%ld", __LINE__, title,
            1L,
            addr_00E0_BB00_0001[0],addr_00E0_BB00_0001[1],addr_00E0_BB00_0001[2],
            addr_00E0_BB00_0001[3],addr_00E0_BB00_0001[4],addr_00E0_BB00_0001[5],
            lport);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(incorrect lport%ld disjoin timer state)", __LINE__, title,
            lport);
        ret = FALSE;
    }

    /* for normal mode
     * 1. attach a phone to this port
     */
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    ADD_MGR_AMTR_EditAddrNotify_CallBack(lport, addr_00E0_BB00_0001, lport, FALSE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(error for process na) vid=%ld,mac=%02X:%02X:%02X:%02X:%02X:%02X,lport=%ld", __LINE__, title,
            1L,
            addr_00E0_BB00_0001[0],addr_00E0_BB00_0001[1],addr_00E0_BB00_0001[2],
            addr_00E0_BB00_0001[3],addr_00E0_BB00_0001[4],addr_00E0_BB00_0001[5],
            lport);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(incorrect lport%ld disjoin timer state)", __LINE__, title,
            lport);
        ret = FALSE;
    }

    /* voice vlan cannot change the static vlan */
    VLAN_MGR_AddEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VAL_dot1qVlanStatus_permanent)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_auto);
    ADD_MGR_AMTR_EditAddrNotify_CallBack(lport, addr_00E0_AA00_0001, lport, FALSE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VAL_dot1qVlanStatus_permanent)
    {
        printf("\r\n%d:Test %s Fail(error for process na) vid=%ld,mac=%02X:%02X:%02X:%02X:%02X:%02X,lport=%ld", __LINE__, title,
            1L,
            addr_00E0_AA00_0001[0],addr_00E0_AA00_0001[1],addr_00E0_AA00_0001[2],
            addr_00E0_AA00_0001[3],addr_00E0_AA00_0001[4],addr_00E0_AA00_0001[5],
            lport);
        ret = FALSE;
    }

    ADD_MGR_AMTR_EditAddrNotify_CallBack(lport, addr_00E0_BB00_0001, lport, FALSE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VAL_dot1qVlanStatus_permanent)
    {
        printf("\r\n%d:Test %s Fail(existed static vlan be modify by voice method)", __LINE__, title);
        ret = FALSE;
    }

    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_manual);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VAL_dot1qVlanStatus_permanent)
    {
        printf("\r\n%d:Test %s Fail(existed static vlan be modify by voice method)", __LINE__, title);
        ret = FALSE;
    }

    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VAL_dot1qVlanStatus_permanent)
    {
        printf("\r\n%d:Test %s Fail(delete static vlan from voice method)", __LINE__, title);
        ret = FALSE;
    }

    /* test AMTR New Address setdown */
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setdown)", __LINE__, title);
        ret = FALSE;
    }

    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(incorrect lport%ld disjoin timer state)", __LINE__, title,
            lport);
        ret = FALSE;
    }


    /* test AMTR New Address setdown */
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setup fail)", __LINE__, title);
        ret = FALSE;
    }
    ADD_MGR_SetVoiceVlanEnabledId(-1);
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);


    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

/* ASSUMPTION: lport 1 is hybird mode
               Vlan 2 is exited
 */
static BOOL_T ADD_UTEST_TestAmtrAgedAddr()
{
    char title[] = "AMTR Aged Address";
    BOOL_T ret = TRUE;
    UI32_T voice_vlan_id = 2;
    UI32_T lport = 1;
    UI32_T disjoin_when;
    UI8_T addr_00E0_AA00_0001[] = {0x00, 0xE0, 0xAA, 0x00, 0x00, 0x01};
    UI8_T addr_00E0_BB00_0001[] = {0x00, 0xE0, 0xBB, 0x00, 0x00, 0x01};

    /* test AMTR Aging Address setup */
    AMTR_MGR_DeleteAddr_ByLPort(lport);
    SYSFUN_Sleep(20);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setup fail)", __LINE__, title);
        ret = FALSE;
    }
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    ADD_MGR_SetVoiceVlanEnabledId(-1);


    /* Sample oui match test */
    /*ADD_OM_InitOui();*/
    if(ADD_OM_IsOui(addr_00E0_AA00_0001) != FALSE)
    {
        printf("\r\n%d:Test %s Fail, mac=%02X:%02X:%02X:%02X:%02X:%02X", __LINE__, title,
            addr_00E0_AA00_0001[0],addr_00E0_AA00_0001[1],addr_00E0_AA00_0001[2],
            addr_00E0_AA00_0001[3],addr_00E0_AA00_0001[4],addr_00E0_AA00_0001[5]);
        ret = FALSE;
    }
    if(ADD_OM_IsOui(addr_00E0_BB00_0001) != TRUE)
    {
        printf("\r\n%d:Test %s Fail, mac=%02X:%02X:%02X:%02X:%02X:%02X", __LINE__, title,
            addr_00E0_BB00_0001[0],addr_00E0_BB00_0001[1],addr_00E0_BB00_0001[2],
            addr_00E0_BB00_0001[3],addr_00E0_BB00_0001[4],addr_00E0_BB00_0001[5]);
        ret = FALSE;
    }
    /* End of Sample oui match test */


    ADD_MGR_SetVoiceVlanEnabledId(voice_vlan_id);
    ADD_OM_SetVoiceVlanPortRuleBitmap(lport, ADD_TYPE_DISCOVERY_PROTOCOL_OUI);
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_auto);

    AMTR_MGR_SetStaticAddrEntry(1, addr_00E0_BB00_0001, lport, AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);
    AMTR_MGR_SetStaticAddrEntry(voice_vlan_id, addr_00E0_BB00_0001, lport, AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);

//    ADD_UTEST_ShowMacAddress();
//    ADD_MGR_AMTR_EditAddrNotify_CallBack(1, addr_00E0_BB00_0001, lport, FALSE);
//    ADD_MGR_AMTR_EditAddrNotify_CallBack(voice_vlan_id, addr_00E0_BB00_0001, lport, FALSE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VLAN_TYPE_VLAN_STATUS_VOICE)//VAL_dot1qVlanStatus_voice)
    {
        printf("\r\n%d:Test %s Fail(error for process aa) vid=%ld,mac=%02X:%02X:%02X:%02X:%02X:%02X,lport=%ld", __LINE__, title,
            1L,
            addr_00E0_BB00_0001[0],addr_00E0_BB00_0001[1],addr_00E0_BB00_0001[2],
            addr_00E0_BB00_0001[3],addr_00E0_BB00_0001[4],addr_00E0_BB00_0001[5],
            lport);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(error disjoin timer state) lport=%ld", __LINE__, title,
            lport);
        ret = FALSE;
    }

    AMTR_MGR_DeleteAddr(1, addr_00E0_BB00_0001);
//    ADD_UTEST_ShowMacAddress();
//    ADD_MGR_AMTR_EditAddrNotify_CallBack(1, addr_00E0_BB00_0001, lport, TRUE);
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(error disjoin timer state) lport=%ld", __LINE__, title,
            lport);
        ret = FALSE;
    }

    AMTR_MGR_DeleteAddr(voice_vlan_id, addr_00E0_BB00_0001);
//    ADD_UTEST_ShowMacAddress();
//    ADD_MGR_AMTR_EditAddrNotify_CallBack(voice_vlan_id, addr_00E0_BB00_0001, lport, TRUE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VLAN_TYPE_VLAN_STATUS_VOICE)//VAL_dot1qVlanStatus_voice)
    {
        printf("\r\n%d:Test %s Fail(error for process aa) vid=%ld,mac=%02X:%02X:%02X:%02X:%02X:%02X,lport=%ld", __LINE__, title,
            1L,
            addr_00E0_BB00_0001[0],addr_00E0_BB00_0001[1],addr_00E0_BB00_0001[2],
            addr_00E0_BB00_0001[3],addr_00E0_BB00_0001[4],addr_00E0_BB00_0001[5],
            lport);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when == 0)
    {
        printf("\r\n%d:Test %s Fail(error disjoin timer state) lport=%ld", __LINE__, title,
            lport);
        ret = FALSE;
    }

    AMTR_MGR_SetStaticAddrEntry(voice_vlan_id, addr_00E0_BB00_0001, lport, AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);
//    ADD_MGR_AMTR_EditAddrNotify_CallBack(voice_vlan_id, addr_00E0_BB00_0001, lport, FALSE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VLAN_TYPE_VLAN_STATUS_VOICE)//VAL_dot1qVlanStatus_voice)
    {
        printf("\r\n%d:Test %s Fail(error for process aa) vid=%ld,mac=%02X:%02X:%02X:%02X:%02X:%02X,lport=%ld", __LINE__, title,
            1L,
            addr_00E0_BB00_0001[0],addr_00E0_BB00_0001[1],addr_00E0_BB00_0001[2],
            addr_00E0_BB00_0001[3],addr_00E0_BB00_0001[4],addr_00E0_BB00_0001[5],
            lport);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(error disjoin timer state) lport=%ld", __LINE__, title,
            lport);
        ret = FALSE;
    }


    /* test AMTR Aging Address setdown */
    AMTR_MGR_DeleteAddr_ByLPort(lport);
    SYSFUN_Sleep(20);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setdown)", __LINE__, title);
        ret = FALSE;
    }

    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(error disjoin timer state) lport=%ld", __LINE__, title,
            lport);
        ret = FALSE;
    }

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

static BOOL_T ADD_UTEST_TestOui()
{
    char title[] = "OUI";
    BOOL_T ret = TRUE;
    UI32_T voice_vlan_id = 2;
    UI32_T lport = 1;
    UI8_T addr_00EE_BB00_0001[] = {0x00, 0xEE, 0xBB, 0x00, 0x00, 0x01};
    UI8_T addr_00EE_BB00_0000[] = {0x00, 0xEE, 0xBB, 0x00, 0x00, 0x00};
    UI8_T addr_00E0_AA00_0000[] = {0x00, 0xE0, 0xAA, 0x00, 0x00, 0x00};
    UI8_T addr_00EE_AA00_0001[] = {0x00, 0xEE, 0xAA, 0x00, 0x00, 0x01};
    UI8_T addr_FFFF_FF00_0000[] = {0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00};
    UI8_T addr_FFFF_FFFF_FFFF[] ={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    UI8_T desc[] = "XXX";
    UI32_T disjoin_when;

    /* test oui setup */
    AMTR_MGR_DeleteAddr_ByLPort(lport);
    SYSFUN_Sleep(20);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setdown)", __LINE__, title);
        ret = FALSE;
    }

    /*ADD_OM_InitOui();*/
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(error disjoin timer state) lport=%ld", __LINE__, title,
            lport);
        ret = FALSE;
    }


    if(ADD_OM_AddOui(addr_00E0_AA00_0000, addr_FFFF_FF00_0000, desc) != TRUE)
    {
        printf("\r\n%d:Test %s Fail, oui=%02X:%02X:%02X:%02X:%02X:%02X, mask=%02X:%02X:%02X:%02X:%02X:%02X",
            __LINE__, title,
            addr_00E0_AA00_0000[0],addr_00E0_AA00_0000[1],addr_00E0_AA00_0000[2],
            addr_00E0_AA00_0000[3],addr_00E0_AA00_0000[4],addr_00E0_AA00_0000[5],
            addr_FFFF_FF00_0000[0],addr_FFFF_FF00_0000[1],addr_FFFF_FF00_0000[2],
            addr_FFFF_FF00_0000[3],addr_FFFF_FF00_0000[4],addr_FFFF_FF00_0000[5]);
        ret = FALSE;
    }

    if(ADD_OM_AddOui(addr_00EE_BB00_0000, addr_FFFF_FF00_0000, desc) != TRUE)
    {
        printf("\r\n%d:Test %s Fail, oui=%02X:%02X:%02X:%02X:%02X:%02X, mask=%02X:%02X:%02X:%02X:%02X:%02X",
            __LINE__, title,
            addr_00EE_BB00_0000[0],addr_00EE_BB00_0000[1],addr_00EE_BB00_0000[2],
            addr_00EE_BB00_0000[3],addr_00EE_BB00_0000[4],addr_00EE_BB00_0000[5],
            addr_FFFF_FF00_0000[0],addr_FFFF_FF00_0000[1],addr_FFFF_FF00_0000[2],
            addr_FFFF_FF00_0000[3],addr_FFFF_FF00_0000[4],addr_FFFF_FF00_0000[5]);
        ret = FALSE;
    }

    /* test for add a duplicable oui. */
    if(ADD_OM_AddOui(addr_00EE_BB00_0000, addr_FFFF_FF00_0000, desc) != FALSE)
    {
        printf("\r\n%d:Test %s Fail, oui=%02X:%02X:%02X:%02X:%02X:%02X, mask=%02X:%02X:%02X:%02X:%02X:%02X",
            __LINE__, title,
            addr_00EE_BB00_0000[0],addr_00EE_BB00_0000[1],addr_00EE_BB00_0000[2],
            addr_00EE_BB00_0000[3],addr_00EE_BB00_0000[4],addr_00EE_BB00_0000[5],
            addr_FFFF_FF00_0000[0],addr_FFFF_FF00_0000[1],addr_FFFF_FF00_0000[2],
            addr_FFFF_FF00_0000[3],addr_FFFF_FF00_0000[4],addr_FFFF_FF00_0000[5]);
        ret = FALSE;
    }

    /* test for add a duplicable oui. */
    if(ADD_OM_AddOui(addr_00EE_BB00_0001, addr_FFFF_FFFF_FFFF, desc) != FALSE)
    {
        printf("\r\n%d:Test %s Fail, oui=%02X:%02X:%02X:%02X:%02X:%02X, mask=%02X:%02X:%02X:%02X:%02X:%02X",
            __LINE__, title,
            addr_00EE_BB00_0001[0],addr_00EE_BB00_0001[1],addr_00EE_BB00_0001[2],
            addr_00EE_BB00_0001[3],addr_00EE_BB00_0001[4],addr_00EE_BB00_0001[5],
            addr_FFFF_FFFF_FFFF[0],addr_FFFF_FFFF_FFFF[1],addr_FFFF_FFFF_FFFF[2],
            addr_FFFF_FFFF_FFFF[3],addr_FFFF_FFFF_FFFF[4],addr_FFFF_FFFF_FFFF[5]);
        ret = FALSE;
    }

    /* test for search a exited oui. */
    if(ADD_OM_IsOui(addr_00EE_BB00_0001) != TRUE)
    {
        printf("\r\n%d:Test %s Fail, mac=%02X:%02X:%02X:%02X:%02X:%02X", __LINE__, title,
            addr_00EE_BB00_0001[0],addr_00EE_BB00_0001[1],addr_00EE_BB00_0001[2],
            addr_00EE_BB00_0001[3],addr_00EE_BB00_0001[4],addr_00EE_BB00_0001[5]);
        ret = FALSE;
    }

    /* test for search a exited oui. */
    if(ADD_OM_IsOui(addr_00EE_AA00_0001) != FALSE)
    {
        printf("\r\n%d:Test %s Fail, mac=%02X:%02X:%02X:%02X:%02X:%02X", __LINE__, title,
            addr_00EE_AA00_0001[0],addr_00EE_AA00_0001[1],addr_00EE_AA00_0001[2],
            addr_00EE_AA00_0001[3],addr_00EE_AA00_0001[4],addr_00EE_AA00_0001[5]);
        ret = FALSE;
    }

    ADD_MGR_SetVoiceVlanEnabledId(voice_vlan_id);
//    ADD_MGR_SetVoiceVlanState(TRUE);

#if 0
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_auto);
    AMTR_MGR_SetStaticAddrEntry(1, addr_00EE_BB00_0001, lport);
    AMTR_MGR_SetStaticAddrEntry(voice_vlan_id, addr_00EE_BB00_0001, lport);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VLAN_TYPE_VLAN_STATUS_VOICE)//VAL_dot1qVlanStatus_voice)
    {
        printf("\r\n%d:Test %s Fail, mac=%02X:%02X:%02X:%02X:%02X:%02X", __LINE__, title,
            addr_00EE_BB00_0001[0],addr_00EE_BB00_0001[1],addr_00EE_BB00_0001[2],
            addr_00EE_BB00_0001[3],addr_00EE_BB00_0001[4],addr_00EE_BB00_0001[5]);
        ret = FALSE;
    }
no test for remove oui ...................................................
    /* test for remove an oui */
    ADD_OM_ShowOui();
    ADD_UTEST_ShowMacAddress();

    ADD_MGR_RemoveOui(addr_00EE_BB00_0000, addr_FFFF_FF00_0000);

    ADD_OM_ShowOui();
    ADD_UTEST_ShowMacAddress();

    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when == 0) //<-------------------- start disjoin timer
    {
        printf("\r\n%d:Test %s Fail, oui=%02X:%02X:%02X:%02X:%02X:%02X, mask=%02X:%02X:%02X:%02X:%02X:%02X", __LINE__, title,
            addr_00EE_BB00_0000[0],addr_00EE_BB00_0000[1],addr_00EE_BB00_0000[2],
            addr_00EE_BB00_0000[3],addr_00EE_BB00_0000[4],addr_00EE_BB00_0000[5],
            addr_FFFF_FF00_0000[0],addr_FFFF_FF00_0000[1],addr_FFFF_FF00_0000[2],
            addr_FFFF_FF00_0000[3],addr_FFFF_FF00_0000[4],addr_FFFF_FF00_0000[5]);
        ret = FALSE;
    }
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VLAN_TYPE_VLAN_STATUS_VOICE)//VAL_dot1qVlanStatus_voice)
    {
        printf("\r\n%d:Test %s Fail, mac=%02X:%02X:%02X:%02X:%02X:%02X", __LINE__, title,
            addr_00EE_BB00_0001[0],addr_00EE_BB00_0001[1],addr_00EE_BB00_0001[2],
            addr_00EE_BB00_0001[3],addr_00EE_BB00_0001[4],addr_00EE_BB00_0001[5]);
        ret = FALSE;
    }
#endif

    ADD_MGR_RemoveOui(addr_00EE_BB00_0000, addr_FFFF_FF00_0000);

    /* test for add an oui */
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    /*ADD_OM_ShowOui();*/
    AMTR_MGR_SetStaticAddrEntry(1, addr_00EE_BB00_0001, lport, AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_auto);

    ADD_MGR_AddOui(addr_00EE_BB00_0000, addr_FFFF_FF00_0000, desc);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VLAN_TYPE_VLAN_STATUS_VOICE)//VAL_dot1qVlanStatus_voice)
    {
        printf("\r\n%d:Test %s Fail, mac=%02X:%02X:%02X:%02X:%02X:%02X", __LINE__, title,
            addr_00EE_BB00_0001[0],addr_00EE_BB00_0001[1],addr_00EE_BB00_0001[2],
            addr_00EE_BB00_0001[3],addr_00EE_BB00_0001[4],addr_00EE_BB00_0001[5]);
        ret = FALSE;
    }

    /* test oui setdown */
    AMTR_MGR_DeleteAddr_ByLPort(lport);
    SYSFUN_Sleep(20);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setdown)", __LINE__, title);
        ret = FALSE;
    }

    /*ADD_OM_InitOui();*/
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(error disjoin timer state) lport=%ld", __LINE__, title,
            lport);
        ret = FALSE;
    }

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}

static BOOL_T ADD_UTEST_TestTimeout()
{
    char title[] = "Set Timeout";
    BOOL_T ret = TRUE;
    UI32_T day, hour, minute;
    UI32_T timeout;

    /* test setup */
    ADD_MGR_GetVoiceVlanAgingTime(&timeout);

    if(ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinute(0,1,0) != TRUE)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinute(&day,&hour,&minute);
    if((day != 0) || (hour != 1) || (minute != 0))
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    if(ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinute(0,0,4) != FALSE)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    if(ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinute(30,0,1) != FALSE)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    /* testdown */
    ADD_MGR_SetVoiceVlanAgingTime(timeout);

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}


/* ASSUMPTION: lport 1 is hybird mode
               Vlan 2 is exited
 */
static BOOL_T ADD_UTEST_TestTimerMsgProcess()
{
    char title[] = "Timer Message Process";
    BOOL_T ret = TRUE;
    UI32_T voice_vlan_id = 2;
    UI32_T lport = 1;
    UI32_T disjoin_when;
    UI8_T  per_join_state;
    UI8_T addr_00E0_BB00_0001[] = {0x00, 0xE0, 0xBB, 0x00, 0x00, 0x01};
    int i;

#define _ADD_OM_DEBUG

    /* test timer message process setup */
    AMTR_MGR_DeleteAddr_ByLPort(lport);
    SYSFUN_Sleep(20);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setup fail)", __LINE__, title);
        ret = FALSE;
    }
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    ADD_MGR_SetVoiceVlanEnabledId(-1);


    ADD_MGR_SetVoiceVlanEnabledId(voice_vlan_id);
    ADD_OM_SetVoiceVlanId(voice_vlan_id);
    ADD_OM_SetVoiceVlanPortRuleBitmap(lport, ADD_TYPE_DISCOVERY_PROTOCOL_OUI);

    //ADD_UTEST_ShowMacAddress();

    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_auto);
    AMTR_MGR_SetStaticAddrEntry(1, addr_00E0_BB00_0001, lport, AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT);

    //ADD_UTEST_ShowMacAddress();

    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != VLAN_TYPE_VLAN_STATUS_VOICE)
    {
        printf("\r\n%d:Test %s Fail(error process na)", __LINE__, title);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when != 0)
    {
        printf("\r\n%d:Test %s Fail(error disjoin timer state) lport=%ld", __LINE__, title,
            lport);
        ret = FALSE;
    }

    ADD_OM_SetVoiceVlanAgingTime(5);
    AMTR_MGR_DeleteAddr(1, addr_00E0_BB00_0001);
    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
    if(disjoin_when == 0)
    {
        printf("\r\n%d:Test %s Fail(error disjoin timer state) lport=%ld", __LINE__, title,
            lport);
        ret = FALSE;
    }

    ADD_OM_GetVoiceVlanPortDisjoinWhen(lport, &disjoin_when);
#ifdef _ADD_OM_DEBUG
    if(disjoin_when != 5)
#else
    if(disjoin_when != (5*60))
#endif
    {
        printf("\r\n%d:Test %s Fail(error disjoin when value)", __LINE__, title);
        ret = FALSE;
    }

#ifdef _ADD_OM_DEBUG
    for(i = 0; i<5; ++i)
#else
    for(i = 0; i<(5*60); ++i)
#endif
    {
        ADD_MGR_ProcessTimeoutMessage(lport);
    }

    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0L)
    {
        printf("\r\n%d:Test %s Fail(error process na)", __LINE__, title);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanPortJoinState(lport, &per_join_state);
    if(per_join_state != ADD_TYPE_DISCOVERY_PROTOCOL_NONE)
    {
        printf("\r\n%d:Test %s Fail(error join state)", __LINE__, title);
        ret = FALSE;
    }


    /* test timer message process setdownload */
    AMTR_MGR_DeleteAddr_ByLPort(lport);
    SYSFUN_Sleep(20);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(voice_vlan_id, lport) != 0)
    {
        printf("\r\n%d:Test %s Fail(setup fail)", __LINE__, title);
        ret = FALSE;
    }
    ADD_MGR_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
    ADD_MGR_SetVoiceVlanEnabledId(-1);


    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}


/* ASSUMPTION: lport 1 and lport 2 are hybird mode
               Vlan 2 is exited
 */
BOOL_T ADD_UTEST_TestAutoJoinOnRebooted()
{
    char title[] = "Auto Join to Voice VLAN on Rebooted";
    BOOL_T ret = TRUE;
    UI8_T default_join_state[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};
    UI32_T lport1 = 1;
    UI32_T lport2 = 2;
    UI8_T join_state;
    UI32_T lport;

    default_join_state[0] = default_join_state[1] = ADD_TYPE_DISCOVERY_PROTOCOL_OUI;
    for(lport = 1; lport < SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
    {
        ADD_OM_SetVoiceVlanPortJoinState(lport, default_join_state[lport-1]);
    }

    VLAN_MGR_DeleteEgressPortMember(2, lport1, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(2, lport1, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(2, lport1) != 0)
    {
        printf("\r\n%d:Test %s Fail(lport%ld dont recover to default status)", __LINE__, title, lport1);
        ret = FALSE;
    }
    VLAN_MGR_DeleteEgressPortMember(2, lport2, VAL_dot1qVlanStatus_permanent);
    VLAN_MGR_DeleteEgressPortMember(2, lport2, VLAN_TYPE_VLAN_STATUS_VOICE);
    if(ADD_UTEST_GetPortVlanMemberStatus(2, lport2) != 0)
    {
        printf("\r\n%d:Test %s Fail(lport%ld dont recover to default status)", __LINE__, title, lport2);
        ret = FALSE;
    }

    ADD_MGR_SetVoiceVlanEnabledId(2);
//    ADD_MGR_SetVoiceVlanState(TRUE);
    ADD_MGR_SetVoiceVlanPortMode(lport1, VAL_voiceVlanPortMode_auto);
    if(ADD_UTEST_GetPortVlanMemberStatus(2, lport1) != VLAN_TYPE_VLAN_STATUS_VOICE)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    ADD_MGR_SetVoiceVlanPortMode(lport2, VAL_voiceVlanPortMode_manual);
    if(ADD_UTEST_GetPortVlanMemberStatus(2, lport2) != 0)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }
    ADD_OM_GetVoiceVlanPortJoinState(lport2, &join_state);
    if(join_state != 0)
    {
        printf("\r\n%d:Test %s Fail", __LINE__, title);
        ret = FALSE;
    }

    /* testdown */
    ADD_MGR_SetVoiceVlanPortMode(lport1, VAL_voiceVlanPortMode_none);
    ADD_MGR_SetVoiceVlanPortMode(lport2, VAL_voiceVlanPortMode_none);

    if(ret == TRUE)
    {
        printf(".");
    }
    return ret;
}
#endif

#endif /* ADD_DO_UNIT_TEST == TRUE */

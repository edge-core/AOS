/* =============================================================================
 * MODULE NAME : MLAG_MGR.C
 * PURPOSE     : Provide definitions for MLAG operational functions.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "leaf_2863.h"
#include "sys_adpt.h"
#include "l_md5.h"
#include "l_mm.h"
#include "l_mm_type.h"
#include "l_stdlib.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "l2mux_mgr.h"
#include "amtr_mgr.h"
#include "amtr_pmgr.h"
#include "amtr_type.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "vlan_om.h"
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
#include "sysctrl_xor_mgr.h"
#endif
#include "mlag_backdoor.h"
#include "mlag_mgr.h"
#include "mlag_om.h"
#include "mlag_om_private.h"
#include "mlag_type.h"

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#define MLAG_MGR_TIMEOUT_VALUE  30 /* seconds */

#define MLAG_MGR_MAX_PDU_SIZE           1500
#define MLAG_MGR_PROTOCOL_ID_SIZE       5
#define MLAG_MGR_FDB_PAYLOAD_THRESHOLD  (MLAG_MGR_MAX_PDU_SIZE - \
                                        sizeof(MLAG_MGR_FdbEntry_T))

#define MLAG_MGR_SNAP_DSAP          0xAA
#define MLAG_MGR_SNAP_SSAP          0xAA
#define MLAG_MGR_SNAP_CONTROL       0x03
#define MLAG_MGR_SNAP_OUI_BYTE1     SYS_ADPT_SNAP_OUI_BYTE1
#define MLAG_MGR_SNAP_OUI_BYTE2     SYS_ADPT_SNAP_OUI_BYTE2
#define MLAG_MGR_SNAP_OUI_BYTE3     SYS_ADPT_SNAP_OUI_BYTE3
#define MLAG_MGR_SNAP_TYPE_BYTE1    0x00
#define MLAG_MGR_SNAP_TYPE_BYTE2    0x04

#define MLAG_MGR_SUBTYPE_LINK   0x01
#define MLAG_MGR_SUBTYPE_FDB    0x02

#define MLAG_MGR_TX_COS     3

enum
{
    MLAG_MGR_DISAGREE,
    MLAG_MGR_AGREE
};

enum
{
    MLAG_MGR_STATE_AGED,
    MLAG_MGR_STATE_LEARNED
};

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#define MLAG_MGR_DEBUG_PRINTF(flag, format_string, ...)         \
{                                                               \
    if (MLAG_OM_GetDebugFlag(flag) == TRUE)                     \
    {                                                           \
        BACKDOOR_MGR_Printf("\r\n%s(%d): "format_string"\r\n",  \
            __func__, __LINE__, ##__VA_ARGS__);                 \
    }                                                           \
}

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
#define MLAG_MGR_XOR_UNLOCK_AND_RETURN(ret_val) \
{                                               \
    SYSCTRL_XOR_MGR_ReleaseSemaphore();         \
    return ret_val;                             \
}
#else
#define MLAG_MGR_XOR_UNLOCK_AND_RETURN(ret_val) { return ret_val; }
#endif

/* ptr  : pointer to common header (MLAG_MGR_CommonHeader_T*)
 * len  : PDU length
 * type : PDU subtype
 * id   : domain ID
 */
#define MLAG_MGR_CONSTRUCT_COMMON_HEADER(ptr, len, type, id)    \
{                                                               \
    ptr->dsap = MLAG_MGR_SNAP_DSAP;                             \
    len += sizeof(ptr->dsap);                                   \
                                                                \
    ptr->ssap = MLAG_MGR_SNAP_SSAP;                             \
    len += sizeof(ptr->ssap);                                   \
                                                                \
    ptr->control = MLAG_MGR_SNAP_CONTROL;                       \
    len += sizeof(ptr->control);                                \
                                                                \
    ptr->protocol_id[0] = MLAG_MGR_SNAP_OUI_BYTE1;              \
    ptr->protocol_id[1] = MLAG_MGR_SNAP_OUI_BYTE2;              \
    ptr->protocol_id[2] = MLAG_MGR_SNAP_OUI_BYTE3;              \
    ptr->protocol_id[3] = MLAG_MGR_SNAP_TYPE_BYTE1;             \
    ptr->protocol_id[4] = MLAG_MGR_SNAP_TYPE_BYTE2;             \
    len += MLAG_MGR_PROTOCOL_ID_SIZE;                           \
                                                                \
    ptr->subtype = type;                                        \
    len += sizeof(ptr->subtype);                                \
                                                                \
    memcpy(ptr->domain_id, id, MLAG_TYPE_MAX_DOMAIN_ID_LEN);    \
    len += MLAG_TYPE_MAX_DOMAIN_ID_LEN;                         \
                                                                \
    memset(ptr->digest, 0, MLAG_OM_DIGEST_SIZE);                \
    len += MLAG_OM_DIGEST_SIZE;                                 \
                                                                \
    ptr->num_of_entries = 0;                                    \
    len += sizeof(ptr->num_of_entries);                         \
}

/* state: 2 bits
 * agree: 1 bit
 * reserved: 1 bit
 * mlag_id: 12 bits
 */
#define MLAG_MGR_LINK_PACK_STATE(__r__, __v__) \
    (__r__ = __r__ | ((__v__ & 0x3) << 14))
#define MLAG_MGR_LINK_PACK_AGREE(__r__, __v__) \
    (__r__ = __r__ | (__v__ & 0x1) << 13)
#define MLAG_MGR_LINK_PACK_MLAG_ID(__r__, __v__) \
    (__r__ = __r__ | ((__v__ & 0xfff)))
#define MLAG_MGR_LINK_GET_STATE(__v__)      ((__v__ & 0xc000) >> 14)
#define MLAG_MGR_LINK_GET_AGREE(__v__)      ((__v__ & 0x2000) >> 13)
#define MLAG_MGR_LINK_GET_MLAG_ID(__v__)    (__v__ & 0x0fff)

/* state: 1 bit
 * reserved: 3 bits
 * mlag_id: 12 bits
 * vlan_id: 16 bits
 */
#define MLAG_MGR_FDB_PACK_STATE(__r__, __v__) \
    (__r__ = __r__ | ((__v__ & 0x1) << 31))
#define MLAG_MGR_FDB_PACK_MLAG_ID(__r__, __v__) \
    (__r__ = __r__ | ((__v__ & 0xfff) << 16))
#define MLAG_MGR_FDB_PACK_VLAN(__r__, __v__) \
    (__r__ = __r__ | (__v__ & 0xffff))
#define MLAG_MGR_FDB_GET_STATE(__v__)   ((__v__ & 0x80000000) >> 31)
#define MLAG_MGR_FDB_GET_MLAG_ID(__v__) ((__v__ & 0x0fff0000) >> 16)
#define MLAG_MGR_FDB_GET_VLAN(__v__)    (__v__ & 0x0000ffff)


/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/************************* pack(1) beg *************************/
#pragma pack(1)

typedef struct
{
    UI8_T   dsap;
    UI8_T   ssap;
    UI8_T   control;
    UI8_T   protocol_id[MLAG_MGR_PROTOCOL_ID_SIZE];
    UI8_T   subtype;
    char    domain_id[MLAG_TYPE_MAX_DOMAIN_ID_LEN];
    UI8_T   digest[MLAG_OM_DIGEST_SIZE];
    UI8_T   num_of_entries;
} MLAG_MGR_CommonHeader_T;

/* state: 2 bits
 * agree: 1 bit
 * reserved: 1 bit
 * mlag_id: 12 bits
 */
typedef struct
{
    UI16_T  state_agree_gid;
} MLAG_MGR_LinkEntry_T;

/* state: 1 bit
 * reserved: 3 bits
 * mlag_id: 12 bits
 * vlan_id: 16 bits
 */
typedef struct
{
    UI32_T  state_gid_vid;
    UI8_T   mac[SYS_ADPT_MAC_ADDR_LEN];
} MLAG_MGR_FdbEntry_T;

#pragma pack()
/************************* pack(1) end *************************/

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM DECLARATIONS
 * -----------------------------------------------------------------------------
 */

static UI32_T MLAG_MGR_SendLinkPacketForDomain(MLAG_TYPE_DomainEntry_T *entry_p);
static UI32_T MLAG_MGR_SendLinkPacket(MLAG_TYPE_MlagEntry_T *entry_p);
static UI32_T MLAG_MGR_SendFdbPacketForDomain(MLAG_TYPE_DomainEntry_T *entry_p);
static UI32_T MLAG_MGR_SendFdbPacketForMlag(MLAG_TYPE_MlagEntry_T *entry_p);
static UI32_T MLAG_MGR_SendFdbPacket(MLAG_OM_FdbEntry_T *entry_p, BOOL_T added);
static UI32_T MLAG_MGR_SendPacket(UI32_T lport, UI8_T *data_p, UI32_T data_len);
static UI32_T MLAG_MGR_ActivateDomain(MLAG_TYPE_DomainEntry_T *entry_p);
static UI32_T MLAG_MGR_InactivateDomain(MLAG_TYPE_DomainEntry_T *entry_p);
static UI32_T MLAG_MGR_HandleConnection(MLAG_TYPE_DomainEntry_T *entry_p);
static UI32_T MLAG_MGR_HandleDisconnection(MLAG_TYPE_DomainEntry_T *entry_p);
static UI32_T MLAG_MGR_RemoveRemoteFdbForMlag(MLAG_TYPE_MlagEntry_T *entry_p);
static BOOL_T MLAG_MGR_IsAlphaNumbericString(char *str_p);

/* -----------------------------------------------------------------------------
 * STATIC VARIABLE DEFINITIONS
 * -----------------------------------------------------------------------------
 */

SYSFUN_DECLARE_CSC

static BOOL_T is_provision_complete;

static UI8_T cpu_mac_ar[SYS_ADPT_MAC_ADDR_LEN];
static UI8_T digest_key[] =
    {   0x13, 0xAC, 0x06, 0xA6, 0x2E, 0x47, 0xFD, 0x51,
        0xF9, 0x5D, 0x2B, 0xA2, 0x43, 0xCD, 0x03, 0x46
    };

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_MGR_InitiateProcessResources
 * PURPOSE : Initiate process resources for the CSC.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_InitiateProcessResources()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_OM_InitiateProcessResources();
    return;
} /* End of MLAG_MGR_InitiateProcessResources */

/* FUNCTION NAME - MLAG_MGR_CreateInterCscRelation
 * PURPOSE : Create inter-CSC relations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_CreateInterCscRelation()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("mlag",
        SYS_BLD_MLAG_GROUP_IPCMSGQ_KEY, MLAG_BACKDOOR_Main);
    return;
} /* End of MLAG_MGR_CreateInterCscRelation */

/* FUNCTION NAME - MLAG_MGR_SetTransitionMode
 * PURPOSE : Process when system is set to be transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_SetTransitionMode()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    SYSFUN_SET_TRANSITION_MODE();
    return;
} /* End of MLAG_MGR_SetTransitionMode */

/* FUNCTION NAME - MLAG_MGR_EnterTransitionMode
 * PURPOSE : Process when system enters transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_EnterTransitionMode()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    SYSFUN_ENTER_TRANSITION_MODE();
    is_provision_complete = FALSE;
    MLAG_OM_ClearAll();
    return;
} /* End of MLAG_MGR_EnterTransitionMode */

/* FUNCTION NAME - MLAG_MGR_EnterMasterMode
 * PURPOSE : Process when system enters master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_EnterMasterMode()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  status;

    /* BODY
     */

    MLAG_OM_DefaultAll();

    if (MLAG_OM_GetGlobalStatus(&status) != MLAG_TYPE_RETURN_OK)
    {
        printf("\r\n %s: failed to get global status \r\n", __func__);
        return;
    }
    if (status == MLAG_TYPE_GLOBAL_STATUS_ENABLED)
    {
        if (SWCTRL_PMGR_SetPktTrapStatus(SWCTRL_PKTTYPE_ORG_SPECIFIC3,
                SWCTRL_ORG_SPECIFIC3_TRAP_BY_MLAG, TRUE, TRUE) == FALSE)
        {
            printf("\r\n %s: failed to set packet trap status \r\n", __func__);
            return;
        }
    }

    SYSFUN_ENTER_MASTER_MODE();
    return;
} /* End of MLAG_MGR_EnterMasterMode */

/* FUNCTION NAME - MLAG_MGR_EnterSlaveMode
 * PURPOSE : Process when system enters slave mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_EnterSlaveMode()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    SYSFUN_ENTER_SLAVE_MODE();
    return;
} /* End of MLAG_MGR_EnterSlaveMode */

/* FUNCTION NAME - MLAG_MGR_ProvisionComplete
 * PURPOSE : Process when the CSC is informed of provision complete.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_ProvisionComplete()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    memset(cpu_mac_ar, 0, SYS_ADPT_MAC_ADDR_LEN);
    if (SWCTRL_GetCpuMac(cpu_mac_ar) == FALSE)
    {
        printf("\r\n %s: failed to get CPU MAC \r\n", __func__);
        return;
    }

    is_provision_complete = TRUE;
    return;
} /* End of MLAG_MGR_ProvisionComplete */

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME - MLAG_MGR_HandleHotInsertion
 * PURPOSE : Process when optional module is inserted.
 * INPUT   : starting_port_ifindex -- the first port on the module
 *           number_of_port        -- the number of ports on the module
 *           use_default           -- whether to use default configuration
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_HandleHotInsertion(UI32_T starting_port_ifindex,
    UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return;
} /* End of MLAG_MGR_HandleHotInsertion */

/* FUNCTION NAME - MLAG_MGR_HandleHotRemoval
 * PURPOSE : Process when optional module is removed.
 * INPUT   : starting_port_ifindex -- the first port on the module
 *           number_of_port        -- the number of ports on the module
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_HandleHotRemoval(UI32_T starting_port_ifindex,
    UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;
    UI32_T                  lport, i;

    /* BODY
     */

    for (i = 0; i < number_of_port; i++)
    {
        lport = starting_port_ifindex + i;

        memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
        domain_entry.peer_link = lport;
        if (MLAG_OM_GetDomainEntryByPort(&domain_entry) == MLAG_TYPE_RETURN_OK)
        {
            if (MLAG_OM_RemoveDomainEntry(&domain_entry) != MLAG_TYPE_RETURN_OK)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to remove "
                    "domain entry %s", domain_entry.domain_id);
                return;
            }
        }
        else
        {
            memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
            mlag_entry.local_member = lport;
            if (MLAG_OM_GetMlagEntryByPort(&mlag_entry) == MLAG_TYPE_RETURN_OK)
            {
                if (MLAG_OM_RemoveMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                        "remove mlag entry %lu", (unsigned long)mlag_entry.mlag_id);
                    return;
                }
            }
        }
    }

    return;
} /* End of MLAG_MGR_HandleHotRemoval */
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

/* FUNCTION NAME - MLAG_MGR_PortOperUp_CallBack
 * PURPOSE : Process callback when port oper status is up.
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_PortOperUp_CallBack(UI32_T ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;
    AMTR_TYPE_AddrEntry_T   addr_entry;
    MLAG_OM_FdbEntry_T      om_entry;
    UI32_T                  global_status;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "ifindex=%lu", (unsigned long)ifindex);

    if (is_provision_complete == FALSE)
    {
        return;
    }

    if (MLAG_OM_GetGlobalStatus(&global_status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get global "
            "status");
        return;
    }
    if (global_status == MLAG_TYPE_GLOBAL_STATUS_DISABLED)
    {
        return;
    }

    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    domain_entry.peer_link = ifindex;
    if (MLAG_OM_GetDomainEntryByPort(&domain_entry) == MLAG_TYPE_RETURN_OK)
    {
        if (MLAG_MGR_HandleConnection(&domain_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to handle "
                "connection");
            return;
        }
    }
    else
    {
        memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
        mlag_entry.local_member = ifindex;
        if (MLAG_OM_GetMlagEntryByPort(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            return;
        }

        mlag_entry.local_state = MLAG_TYPE_STATE_UP;
        if (MLAG_OM_SetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mlag "
                "entry");
            return;
        }

        if (mlag_entry.remote_state == MLAG_TYPE_STATE_INEXISTENT)
        {
            return;
        }

        memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
        strncpy(domain_entry.domain_id, mlag_entry.domain_id,
            MLAG_TYPE_MAX_DOMAIN_ID_LEN);
        if (MLAG_OM_GetDomainEntry(&domain_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get domain "
                "entry %s", domain_entry.domain_id);
            return;
        }
        if ((mlag_entry.remote_state == MLAG_TYPE_STATE_DOWN) &&
            (SWCTRL_PMGR_SetPortEgressBlock(domain_entry.peer_link,
                mlag_entry.local_member, FALSE) == FALSE))
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set egress "
                "block (%lu, %lu, FALSE)", (unsigned long)domain_entry.peer_link,
                (unsigned long)mlag_entry.local_member);
            return;
        }

        if (MLAG_MGR_SendLinkPacket(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send LINK "
                "packet");
            return;
        }

        memset(&om_entry, 0, sizeof(MLAG_OM_FdbEntry_T));
        om_entry.mlag_id = mlag_entry.mlag_id;
        while (MLAG_OM_GetNextRemoteFdbEntryByMlag(&om_entry) ==
                MLAG_TYPE_RETURN_OK)
        {
            memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
            addr_entry.vid = om_entry.vid;
            memcpy(addr_entry.mac, om_entry.mac, SYS_ADPT_MAC_ADDR_LEN);
            addr_entry.ifindex = ifindex;
            addr_entry.action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
            addr_entry.source = AMTR_TYPE_ADDRESS_SOURCE_MLAG;
            addr_entry.life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
            if (AMTR_PMGR_SetAddrEntryForMlag(&addr_entry) == FALSE)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set addr "
                    "entry for mlad %lu", (unsigned long)mlag_entry.mlag_id);
                return;
            }
        }
    }

    return;
} /* End of MLAG_MGR_PortOperUp_CallBack */

/* FUNCTION NAME - MLAG_MGR_PortNotOperUp_CallBack
 * PURPOSE : Process callback when port oper status is not up.
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_PortNotOperUp_CallBack(UI32_T ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T   plist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;
    MLAG_OM_FdbEntry_T      om_entry;
    AMTR_TYPE_AddrEntry_T   addr_entry;
    UI32_T                  global_status;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "ifindex=%lu", (unsigned long)ifindex);

    if (is_provision_complete == FALSE)
    {
        return;
    }

    if (MLAG_OM_GetGlobalStatus(&global_status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get global "
            "status");
        return;
    }
    if (global_status == MLAG_TYPE_GLOBAL_STATUS_DISABLED)
    {
        return;
    }

    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    domain_entry.peer_link = ifindex;
    if (MLAG_OM_GetDomainEntryByPort(&domain_entry) == MLAG_TYPE_RETURN_OK)
    {
        if (MLAG_MGR_HandleDisconnection(&domain_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to handle "
                "disconnection");
            return;
        }

        memset(plist, 0xff, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        if (SWCTRL_PMGR_SetPortEgressBlockEx(domain_entry.peer_link, NULL,
                plist) == FALSE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set egress "
                "block ex (%lu, NULL, plist)", (unsigned long)domain_entry.peer_link);
            return;
        }
    }
    else
    {
        memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
        mlag_entry.local_member = ifindex;
        if (MLAG_OM_GetMlagEntryByPort(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            return;
        }

        mlag_entry.local_state = MLAG_TYPE_STATE_DOWN;
        if (MLAG_OM_SetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mlag "
                "entry");
            return;
        }

        if (mlag_entry.remote_state == MLAG_TYPE_STATE_INEXISTENT)
        {
            return;
        }

        memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
        strncpy(domain_entry.domain_id, mlag_entry.domain_id,
            MLAG_TYPE_MAX_DOMAIN_ID_LEN);
        if (MLAG_OM_GetDomainEntry(&domain_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get domain "
                "entry");
            return;
        }

        if (SWCTRL_PMGR_SetPortEgressBlock(domain_entry.peer_link,
                mlag_entry.local_member, TRUE) == FALSE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set egress "
                "block (%lu, %lu, TRUE)", (unsigned long)domain_entry.peer_link,
                (unsigned long)mlag_entry.local_member);
            return;
        }

        if (MLAG_MGR_SendLinkPacket(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send LINK "
                "packet");
            return;
        }

        memset(&om_entry, 0, sizeof(MLAG_OM_FdbEntry_T));
        om_entry.mlag_id = mlag_entry.mlag_id;
        while (MLAG_OM_GetNextRemoteFdbEntryByMlag(&om_entry) ==
                MLAG_TYPE_RETURN_OK)
        {
            memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
            addr_entry.vid = om_entry.vid;
            memcpy(addr_entry.mac, om_entry.mac, SYS_ADPT_MAC_ADDR_LEN);
            addr_entry.ifindex = domain_entry.peer_link;
            addr_entry.action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
            addr_entry.source = AMTR_TYPE_ADDRESS_SOURCE_MLAG;
            addr_entry.life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
            if (AMTR_PMGR_SetAddrEntryForMlag(&addr_entry) == FALSE)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set addr "
                    "entry");
                return;
            }
        }
    }

    return;
} /* End of MLAG_MGR_PortNotOperUp_CallBack */

/* FUNCTION NAME - MLAG_MGR_PortEffectiveOperStatusChanged_CallBack
 * PURPOSE : Process callback when port effective oper status is changed.
 * INPUT   : ifindex        -- which logical port
 *           pre_status     -- status before change
 *           current_status -- status after change
 *           level          -- SWCTRL_OperDormantLevel_T
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Handle MLAG dormant only
 */
void MLAG_MGR_PortEffectiveOperStatusChanged_CallBack(UI32_T ifindex,
    UI32_T pre_status, UI32_T current_status, UI32_T level)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   entry;
    UI32_T                  global_status;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "ifindex=%lu, pre_status=%lu, "
        "current_status=%lu, level=%lu)", (unsigned long)ifindex, (unsigned long)pre_status, (unsigned long)current_status,
        (unsigned long)level);

    if ((level != SWCTRL_OPER_DORMANT_LV_MLAG) ||
        (current_status != VAL_ifOperStatus_dormant))
    {
        return;
    }

    if (is_provision_complete == FALSE)
    {
        return;
    }

    if (MLAG_OM_GetGlobalStatus(&global_status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get global "
            "status");
        return;
    }
    if (global_status == MLAG_TYPE_GLOBAL_STATUS_DISABLED)
    {
        return;
    }

    entry.local_member = ifindex;
    if (MLAG_OM_GetMlagEntryByPort(&entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "lport %lu is not MLAG local"
            "member", (unsigned long)ifindex);
        return;
    }

    if (entry.remote_state == MLAG_TYPE_STATE_INEXISTENT)
    {
        return;
    }

    entry.local_state = MLAG_TYPE_STATE_DORMANT;
    if (MLAG_OM_SetMlagEntry(&entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mlag entry");
        return;
    }

    if (MLAG_MGR_SendLinkPacket(&entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send LINK "
            "packet");
        return;
    }

    if (MLAG_OM_SetDigest(entry.domain_id, MLAG_OM_DIGEST_LINK, NULL)
            != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set LINK digest");
        return;
    }

    return;
} /* End of MLAG_MGR_PortEffectiveOperStatusChanged_CallBack */

/* FUNCTION NAME - MLAG_MGR_MacUpdate_CallBack
 * PURPOSE : Process callback when a MAC update happens.
 * INPUT   : ifindex -- port on which MAC address is updated
 *           vid     -- VLAN ID
 *           mac_p   -- MAC address
 *           added   -- MAC address is added or removed
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_MacUpdate_CallBack(UI32_T ifindex, UI32_T vid, UI8_T *mac_p,
    BOOL_T added)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_OM_FdbEntry_T      om_entry;
    UI32_T                  global_status;

    /* BODY
     */

    if (SWCTRL_LogicalPortExisting(ifindex) == FALSE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "ifindex does not exist");
        return;
    }
    if (VLAN_OM_IsVlanExisted(vid) == FALSE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "VLAN does not exist");
        return;
    }
    if (mac_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "NULL pointer");
        return;
    }

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "ifindex=%lu, vid=%lu, "
        "mac=%02X-%02X-%02X-%02X-%02X-%02X, added=%hu", (unsigned long)ifindex,
        (unsigned long)vid, mac_p[0], mac_p[1], mac_p[2], mac_p[3], mac_p[4],
        mac_p[5], added);

    if (is_provision_complete == FALSE)
    {
        return;
    }

    if (MLAG_OM_GetGlobalStatus(&global_status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get global "
            "status");
        return;
    }
    if (global_status == MLAG_TYPE_GLOBAL_STATUS_DISABLED)
    {
        return;
    }

    mlag_entry.local_member = ifindex;
    if (MLAG_OM_GetMlagEntryByPort(&mlag_entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "lport %lu is not MLAG local"
            " member", (unsigned long)ifindex);
        return;
    }

    memcpy(om_entry.mac, mac_p, SYS_ADPT_MAC_ADDR_LEN);
    om_entry.vid = vid;
    if (MLAG_OM_GetRemoteFdbEntry(&om_entry) == MLAG_TYPE_RETURN_OK)
    {
        if (MLAG_OM_RemoveRemoteFdbEntry(&om_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to remove "
                "rmeote fdb entry");
            return;
        }
    }
    else
    {
        if (mlag_entry.local_state != MLAG_TYPE_STATE_UP)
        {
            return;
        }
        if (mlag_entry.remote_state == MLAG_TYPE_STATE_INEXISTENT)
        {
            return;
        }

        om_entry.mlag_id = mlag_entry.mlag_id;
        if (MLAG_MGR_SendFdbPacket(&om_entry, added) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send FDB "
                "packet");
            return;
        }
    }

    return;
} /* End of MLAG_MGR_MacUpdate_CallBack */

/* FUNCTION NAME - MLAG_MGR_ProcessReceivedPacket
 * PURPOSE : Process when packet is received.
 * INPUT   : mref_handle_p -- memory reference of received packet
 *           src_mac_p     -- source MAC address
 *           tag_info      -- raw tagged info of the packet
 *           src_unit      -- source unit
 *           src_port      -- source port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_ProcessReceivedPacket(L_MM_Mref_Handle_T *mref_handle_p,
                                    UI8_T              *src_mac_p,
                                    UI16_T             tag_info,
                                    UI32_T             src_unit,
                                    UI32_T             src_port)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;
    UI32_T                  global_status;
    UI32_T                  rx_lport;
    UI8_T                   *pdu_p;
    UI32_T                  pdu_len;
    MLAG_MGR_CommonHeader_T *common_p;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "tag_info=%u, src_unit=%lu, "
        "src_port=%lu", tag_info, (unsigned long)src_unit, (unsigned long)src_port);

    if (is_provision_complete == FALSE)
    {
        return;
    }

    if (MLAG_OM_GetGlobalStatus(&global_status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get global "
            "status");
        return;
    }
    if (global_status == MLAG_TYPE_GLOBAL_STATUS_DISABLED)
    {
        return;
    }

    if (SWCTRL_UserPortToLogicalPort(src_unit, src_port, &rx_lport)
            == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "rx port %lu is unknown "
            "port", (unsigned long)rx_lport);
        return;
    }

    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    domain_entry.peer_link = rx_lport;
    if (MLAG_OM_GetDomainEntryByPort(&domain_entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get domain entry"
            " by port %lu", (unsigned long)rx_lport);
        return;
    }

    pdu_p = (UI8_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if (pdu_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get PDU "
            "position");
        return;
    }

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_RX, "RX from lport %lu (%lu)",
        (unsigned long)rx_lport, (unsigned long)SYSFUN_GetSysTick());

    common_p = (MLAG_MGR_CommonHeader_T*)pdu_p;
    if (strncmp(common_p->domain_id, domain_entry.domain_id,
            MLAG_TYPE_MAX_DOMAIN_ID_LEN) != 0)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "not same domain ID");
        return;
    }

    if (MLAG_OM_GetTimer(common_p->domain_id, MLAG_OM_TIMER_RX) == 0)
    {
        if (MLAG_MGR_HandleConnection(&domain_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to handle "
                "connection for domain %s", common_p->domain_id);
            return;
        }
    }
    else if (MLAG_OM_SetTimer(common_p->domain_id, MLAG_OM_TIMER_RX,
                MLAG_MGR_TIMEOUT_VALUE*2) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set rx timer for "
            "domain %s", common_p->domain_id);
        return;
    }

    switch (common_p->subtype)
    {
    case MLAG_MGR_SUBTYPE_LINK:
    {
        MLAG_MGR_LinkEntry_T    link_entry, *link_entry_p;
        UI32_T                  agree, state;

        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_RX, "RX LINK packet");

        if (MLAG_OM_SameDigest(common_p->domain_id, MLAG_OM_DIGEST_LINK,
                common_p->digest) == TRUE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_RX, "same digest");
            return;
        }

        link_entry_p = (MLAG_MGR_LinkEntry_T*)(common_p + 1);
        while (common_p->num_of_entries > 0)
        {
            memset(&link_entry, 0, sizeof(MLAG_MGR_LinkEntry_T));
            link_entry.state_agree_gid = L_STDLIB_Ntoh16(
                                            link_entry_p->state_agree_gid);

            memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
            mlag_entry.mlag_id = link_entry.state_agree_gid & 0x0fff;
            if (MLAG_OM_GetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get mlag "
                    "entry");
                common_p->num_of_entries--;
                link_entry_p++;
                continue;
            }
            if (strncmp(mlag_entry.domain_id, common_p->domain_id,
                    MLAG_TYPE_MAX_DOMAIN_ID_LEN) != 0)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "domain ID is "
                    "mismatched");
                common_p->num_of_entries--;
                link_entry_p++;
                continue;
            }

            agree = (link_entry.state_agree_gid >> 13) & 0x1;
            if ((mlag_entry.local_state == MLAG_TYPE_STATE_DORMANT) &&
                (agree == MLAG_MGR_AGREE))
            {
                if (SWCTRL_PMGR_TriggerPortOperDormantEvent(
                        mlag_entry.local_member, SWCTRL_OPER_DORMANT_LV_MLAG,
                        SWCTRL_OPER_DORMANT_EV_LEAVE) == FALSE)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                        "trigger dormant event (%lu, LV_MLAG, EV_LEAVE)",
                        (unsigned long)mlag_entry.local_member);
                    return;
                }
            }

            state = (link_entry.state_agree_gid >> 14) & 0x3;
            if (mlag_entry.remote_state == state)
            {
                common_p->num_of_entries--;
                link_entry_p++;
                continue;
            }

            switch (state)
            {
            case MLAG_TYPE_STATE_INEXISTENT:
                if (mlag_entry.local_state == MLAG_TYPE_STATE_UP)
                {
                    if (SWCTRL_PMGR_SetPortEgressBlock(rx_lport,
                            mlag_entry.local_member, TRUE) == FALSE)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "set egress block (%lu, %lu, TRUE)",
                            (unsigned long)rx_lport,
                            (unsigned long)mlag_entry.local_member);
                        return;
                    }
                }
                if (AMTR_PMGR_SetMlagMacNotifyPortStatus(
                        mlag_entry.local_member, FALSE) == FALSE)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set "
                        "mac notify (%lu, FALSE)",
                        (unsigned long)mlag_entry.local_member);
                    return;
                }
                if (SWCTRL_PMGR_SetPortOperDormantStatus(
                        mlag_entry.local_member, SWCTRL_OPER_DORMANT_LV_MLAG,
                        FALSE, FALSE) == FALSE)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set "
                        "dormant status (%lu, LV_MLAG, FALSE, FALSE)",
                        (unsigned long)mlag_entry.local_member);
                    return;
                }
                break;

            case MLAG_TYPE_STATE_DOWN:
                if (mlag_entry.remote_state == MLAG_TYPE_STATE_INEXISTENT)
                {
                    if (SWCTRL_PMGR_SetPortOperDormantStatus(
                            mlag_entry.local_member,
                            SWCTRL_OPER_DORMANT_LV_MLAG, TRUE, TRUE) == FALSE)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "set dormant status (%lu, LV_MLAG, TRUE, TRUE)",
                            (unsigned long)mlag_entry.local_member);
                        return;
                    }
                    if (AMTR_PMGR_SetMlagMacNotifyPortStatus(
                            mlag_entry.local_member, TRUE) == FALSE)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "set mac notify (%lu, TRUE)",
                            (unsigned long)mlag_entry.local_member);
                        return;
                    }
                    /* let peer know my existence more quickly */
                    if (MLAG_MGR_SendLinkPacket(&mlag_entry) !=
                            MLAG_TYPE_RETURN_OK)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "send LINK packet");
                        return;
                    }
                }
                if (mlag_entry.local_state == MLAG_TYPE_STATE_UP)
                {
                    if (SWCTRL_PMGR_SetPortEgressBlock(rx_lport,
                            mlag_entry.local_member, FALSE) == FALSE)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "set egress block (%lu, %lu, FALSE)",
                            (unsigned long)rx_lport,
                            (unsigned long)mlag_entry.local_member);
                        return;
                    }
                    if (MLAG_MGR_SendFdbPacketForMlag(&mlag_entry) !=
                            MLAG_TYPE_RETURN_OK)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "send FDB packet for MLAG %lu",
                            (unsigned long)mlag_entry.mlag_id);
                        return;
                    }
                }
                if (mlag_entry.remote_state == MLAG_TYPE_STATE_UP)
                {
                    if (MLAG_MGR_RemoveRemoteFdbForMlag(&mlag_entry) !=
                            MLAG_TYPE_RETURN_OK)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "remove remote FDB for MLAG %lu",
                            (unsigned long)mlag_entry.mlag_id);
                        return;
                    }
                }
                break;

            case MLAG_TYPE_STATE_DORMANT:
                if (mlag_entry.local_state == MLAG_TYPE_STATE_UP)
                {
                    if (SWCTRL_PMGR_SetPortEgressBlock(rx_lport,
                            mlag_entry.local_member, TRUE) == FALSE)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "set egress block (%lu, %lu, TRUE)",
                            (unsigned long)rx_lport,
                            (unsigned long)mlag_entry.local_member);
                        return;
                    }
                }
                mlag_entry.remote_state = MLAG_TYPE_STATE_DORMANT;
                if (MLAG_MGR_SendLinkPacket(&mlag_entry) != MLAG_TYPE_RETURN_OK)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send "
                        "LINK packet");
                    return;
                }
                break;

            case MLAG_TYPE_STATE_UP:
                if (mlag_entry.remote_state == MLAG_TYPE_STATE_INEXISTENT)
                {
                    if (SWCTRL_PMGR_SetPortOperDormantStatus(
                            mlag_entry.local_member,
                            SWCTRL_OPER_DORMANT_LV_MLAG, TRUE, TRUE) == FALSE)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "set dormant status (%lu, LV_MLAG, TRUE, TRUE)",
                            (unsigned long)mlag_entry.local_member);
                        return;
                    }
                    if (AMTR_PMGR_SetMlagMacNotifyPortStatus(
                            mlag_entry.local_member, TRUE) == FALSE)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "set mac notify (%lu, TRUE)",
                            (unsigned long)mlag_entry.local_member);
                        return;
                    }
                    /* let peer know my existence more quickly */
                    if (MLAG_MGR_SendLinkPacket(&mlag_entry) !=
                            MLAG_TYPE_RETURN_OK)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "send LINK packet");
                        return;
                    }
                }
                if (mlag_entry.local_state == MLAG_TYPE_STATE_UP)
                {
                    if (MLAG_MGR_SendFdbPacketForMlag(&mlag_entry) !=
                            MLAG_TYPE_RETURN_OK)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "send FDB packet for MLAG %lu",
                            (unsigned long)mlag_entry.mlag_id);
                        return;
                    }
                }
                break;

            default:
                common_p->num_of_entries--;
                link_entry_p++;
                continue;
            } /* end switch link_entry.bit_data.state */

            mlag_entry.remote_state = state;
            if (MLAG_OM_SetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mlag "
                    "entry for MLAG %lu", (unsigned long)mlag_entry.mlag_id);
                return;
            }

            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_RX, "MLAG ID:%lu, Agree:%lu, "
                "State:%lu", (unsigned long)mlag_entry.mlag_id,
                (unsigned long)agree, (unsigned long)state);

            common_p->num_of_entries--;
            link_entry_p++;
        } /* end while num_of_entries > 0 */

        if (MLAG_OM_SetDigest(common_p->domain_id, MLAG_OM_DIGEST_LINK,
                common_p->digest) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set link "
                "digest for domain %s", common_p->domain_id);
            return;
        }
    }
        break;

    case MLAG_MGR_SUBTYPE_FDB:
    {
        AMTR_TYPE_AddrEntry_T   addr_entry;
        MLAG_OM_FdbEntry_T      om_entry;
        MLAG_MGR_FdbEntry_T     fdb_entry, *fdb_entry_p;
        UI32_T                  vlan_id;

        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_RX, "RX FDB packet");

        if (MLAG_OM_SameDigest(common_p->domain_id, MLAG_OM_DIGEST_FDB,
                common_p->digest) == TRUE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_RX, "same digest");
            return;
        }

        fdb_entry_p = (MLAG_MGR_FdbEntry_T*)(common_p + 1);
        while (common_p->num_of_entries > 0)
        {
            memset(&fdb_entry, 0, sizeof(MLAG_MGR_FdbEntry_T));
            fdb_entry.state_gid_vid = L_STDLIB_Ntoh32(
                                        fdb_entry_p->state_gid_vid);
            memcpy(fdb_entry.mac, fdb_entry_p->mac, SYS_ADPT_MAC_ADDR_LEN);

            memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
            mlag_entry.mlag_id =
                MLAG_MGR_FDB_GET_MLAG_ID(fdb_entry.state_gid_vid);
            if (MLAG_OM_GetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get mlag "
                    "entry");
                common_p->num_of_entries--;
                fdb_entry_p++;
                continue;
            }
            if (strncmp(mlag_entry.domain_id, common_p->domain_id,
                    MLAG_TYPE_MAX_DOMAIN_ID_LEN) != 0)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "domain ID is "
                    "mismatched");
                common_p->num_of_entries--;
                fdb_entry_p++;
                continue;
            }
            if ((mlag_entry.local_state != MLAG_TYPE_STATE_UP) &&
                (mlag_entry.local_state != MLAG_TYPE_STATE_DOWN))
            {
                common_p->num_of_entries--;
                fdb_entry_p++;
                continue;
            }

            vlan_id = MLAG_MGR_FDB_GET_VLAN(fdb_entry.state_gid_vid);
            if (VLAN_OM_IsVlanExisted(vlan_id) == FALSE)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "VLAN %lu does not "
                    "exist", (unsigned long)vlan_id);
                common_p->num_of_entries--;
                fdb_entry_p++;
                continue;
            }

            if (MLAG_MGR_FDB_GET_STATE(fdb_entry.state_gid_vid) ==
                    MLAG_MGR_STATE_LEARNED)
            {
                memset(&om_entry, 0, sizeof(MLAG_OM_FdbEntry_T));
                memcpy(om_entry.mac, fdb_entry.mac, SYS_ADPT_MAC_ADDR_LEN);
                om_entry.vid = vlan_id;
                if (MLAG_OM_GetRemoteFdbEntry(&om_entry) == MLAG_TYPE_RETURN_OK)
                {
                    if (om_entry.mlag_id == mlag_entry.mlag_id)
                    {
                        common_p->num_of_entries--;
                        fdb_entry_p++;
                        continue;
                    }
                }

                if (MLAG_OM_GetFreeRemoteFdbCount() == 0)
                {
                    common_p->num_of_entries--;
                    fdb_entry_p++;
                    continue;
                }

                memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
                addr_entry.vid = om_entry.vid;
                memcpy(addr_entry.mac, om_entry.mac, SYS_ADPT_MAC_ADDR_LEN);
                if (AMTR_PMGR_GetExactAddrEntry(&addr_entry) == TRUE)
                {
                    common_p->num_of_entries--;
                    fdb_entry_p++;
                    continue;
                }

                if (mlag_entry.local_state == MLAG_TYPE_STATE_UP)
                {
                    addr_entry.ifindex = mlag_entry.local_member;
                }
                else /* mlag_entry.local_state == MLAG_TYPE_STATE_DOWN */
                {
                    addr_entry.ifindex = rx_lport;
                }
                addr_entry.action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
                addr_entry.source = AMTR_TYPE_ADDRESS_SOURCE_MLAG;
                addr_entry.life_time =
                    AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
                if (AMTR_PMGR_SetAddrEntryForMlag(&addr_entry) == FALSE)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set "
                        "addr entry");
                    common_p->num_of_entries--;
                    fdb_entry_p++;
                    continue;
                }

                om_entry.mlag_id = mlag_entry.mlag_id;
                if (MLAG_OM_SetRemoteFdbEntry(&om_entry) != MLAG_TYPE_RETURN_OK)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set "
                        "remote fdb entry");
                    common_p->num_of_entries--;
                    fdb_entry_p++;
                    continue;
                }

                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_RX, "\r\n[Learned] MAC:"
                    "%02X-%02X-%02X-%02X-%02X-%02X, VID:%lu, MLAG ID:%lu",
                    om_entry.mac[0], om_entry.mac[1], om_entry.mac[2],
                    om_entry.mac[3], om_entry.mac[4], om_entry.mac[5],
                    (unsigned long)om_entry.vid, (unsigned long)om_entry.mlag_id);
            } /* end if state is learned */
            else if (MLAG_MGR_FDB_GET_STATE(fdb_entry.state_gid_vid) ==
                        MLAG_MGR_STATE_AGED)
            {
                memset(&om_entry, 0, sizeof(MLAG_OM_FdbEntry_T));
                memcpy(om_entry.mac, fdb_entry.mac, SYS_ADPT_MAC_ADDR_LEN);
                om_entry.vid = vlan_id;
                if (MLAG_OM_GetRemoteFdbEntry(&om_entry) != MLAG_TYPE_RETURN_OK)
                {
                    common_p->num_of_entries--;
                    fdb_entry_p++;
                    continue;
                }
                if (om_entry.mlag_id != mlag_entry.mlag_id)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "MLAG ID is "
                        "mismatched");
                    common_p->num_of_entries--;
                    fdb_entry_p++;
                    continue;
                }

                memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
                addr_entry.vid = om_entry.vid;
                memcpy(addr_entry.mac, om_entry.mac, SYS_ADPT_MAC_ADDR_LEN);
                if (AMTR_PMGR_GetExactAddrEntry(&addr_entry) == FALSE)
                {
                    common_p->num_of_entries--;
                    fdb_entry_p++;
                    continue;
                }

                if (    (    (mlag_entry.local_state == MLAG_TYPE_STATE_UP)
                          && (addr_entry.ifindex == mlag_entry.local_member)
                        )
                     || (    (mlag_entry.local_state == MLAG_TYPE_STATE_DOWN)
                          && (addr_entry.ifindex == rx_lport)
                        )
                   )
                {
                    if (AMTR_PMGR_DeleteAddrForMlag(om_entry.vid, om_entry.mac)
                            == FALSE)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "delete addr entry");
                        common_p->num_of_entries--;
                        fdb_entry_p++;
                        continue;
                    }
                }

                if (MLAG_OM_RemoveRemoteFdbEntry(&om_entry) !=
                        MLAG_TYPE_RETURN_OK)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                        "remove remote fdb entry");
                    common_p->num_of_entries--;
                    fdb_entry_p++;
                    continue;
                }

                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_RX, "\r\n[Aged] MAC:"
                    "%02X-%02X-%02X-%02X-%02X-%02X, VID:%lu, MLAG ID:%lu",
                    om_entry.mac[0], om_entry.mac[1], om_entry.mac[2],
                    om_entry.mac[3], om_entry.mac[4], om_entry.mac[5],
                    (unsigned long)om_entry.vid, (unsigned long)om_entry.mlag_id);
            } /* end else if state is aged */

            common_p->num_of_entries--;
            fdb_entry_p++;
        } /* end while num_of_entries > 0 */

        if (MLAG_OM_SetDigest(common_p->domain_id, MLAG_OM_DIGEST_FDB,
                common_p->digest) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set fdb "
                "digest for domain %s", common_p->domain_id);
            return;
        }
    }
        break;

    default:
        return;
    }

    return;
} /* End of MLAG_MGR_ProcessReceivedPacket */

/* FUNCTION NAME - MLAG_MGR_ProcessTimerEvent
 * PURPOSE : Process when timer event is received.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_MGR_ProcessTimerEvent()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T   plist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    MLAG_TYPE_DomainEntry_T entry;
    UI32_T                  global_status;
    UI32_T                  timer;

    /* BODY
     */

    if (is_provision_complete == FALSE)
    {
        return;
    }

    if (MLAG_OM_GetGlobalStatus(&global_status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get global "
            "status");
        return;
    }
    if (global_status == MLAG_TYPE_GLOBAL_STATUS_DISABLED)
    {
        return;
    }

    memset(&entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    while (MLAG_OM_GetNextDomainEntry(&entry) == MLAG_TYPE_RETURN_OK)
    {
        timer = MLAG_OM_GetTimer(entry.domain_id, MLAG_OM_TIMER_RX);
        if (timer > 0)
        {
            timer--;
            if (timer == 0)
            {
                if (MLAG_MGR_HandleDisconnection(&entry) != MLAG_TYPE_RETURN_OK)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                        "handle disconnection");
                    return;
                }

                memset(plist, 0xff,
                    SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                if (SWCTRL_PMGR_SetPortEgressBlockEx(entry.peer_link, NULL,
                        plist) == FALSE)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set "
                        "egress block ex (%lu, NULL, plist)", (unsigned long)entry.peer_link);
                    return;
                }

                continue;
            }
            if (MLAG_OM_SetTimer(entry.domain_id, MLAG_OM_TIMER_RX, timer) !=
                    MLAG_TYPE_RETURN_OK)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set rx "
                    "timer");
                return;
            }
        }

        timer = MLAG_OM_GetTimer(entry.domain_id, MLAG_OM_TIMER_TX);
        if (timer > 0)
        {
            timer--;
            if (timer == 0)
            {
                if (MLAG_MGR_SendLinkPacketForDomain(&entry) !=
                        MLAG_TYPE_RETURN_OK)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send "
                        "LINK packet");
                    return;
                }
                timer = MLAG_MGR_TIMEOUT_VALUE;
            }
            if (MLAG_OM_SetTimer(entry.domain_id, MLAG_OM_TIMER_TX, timer) !=
                    MLAG_TYPE_RETURN_OK)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set tx "
                    "timer");
                return;
            }
        }

        timer = MLAG_OM_GetTimer(entry.domain_id, MLAG_OM_TIMER_FDB);
        if (timer > 0)
        {
            timer--;
            if (timer == 0)
            {
                if (MLAG_MGR_SendFdbPacketForDomain(&entry) !=
                        MLAG_TYPE_RETURN_OK)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send "
                        "FDB packet for domain %s", entry.domain_id);
                    return;
                }
                timer = MLAG_MGR_TIMEOUT_VALUE;
            }
            if (MLAG_OM_SetTimer(entry.domain_id, MLAG_OM_TIMER_FDB, timer) !=
                    MLAG_TYPE_RETURN_OK)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set fdb "
                    "timer for domain %s", entry.domain_id);
                return;
            }
        }
    }

    return;
} /* End of MLAG_MGR_ProcessTimerEvent */

/* FUNCTION NAME - MLAG_MGR_SetGlobalStatus
 * PURPOSE : Set global status of the feature.
 * INPUT   : status -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                     MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_MGR_SetGlobalStatus(UI32_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_DomainEntry_T entry;
    UI32_T                  current_status;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "status=%lu", (unsigned long)status);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "not master mode");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if ((status != MLAG_TYPE_GLOBAL_STATUS_ENABLED) &&
        (status != MLAG_TYPE_GLOBAL_STATUS_DISABLED))
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "invalid status value");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_OM_GetGlobalStatus(&current_status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get current "
            "status");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (current_status == status)
    {
        return MLAG_TYPE_RETURN_OK;
    }

    memset(&entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    if (status == MLAG_TYPE_GLOBAL_STATUS_ENABLED)
    {
        if (SWCTRL_PMGR_SetPktTrapStatus(SWCTRL_PKTTYPE_ORG_SPECIFIC3,
                SWCTRL_ORG_SPECIFIC3_TRAP_BY_MLAG, TRUE, TRUE) == FALSE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to enable "
                "packet trap status");
            return MLAG_TYPE_RETURN_ERROR;
        }

        while (MLAG_OM_GetNextDomainEntry(&entry) == MLAG_TYPE_RETURN_OK)
        {
            if (MLAG_MGR_ActivateDomain(&entry) != MLAG_TYPE_RETURN_OK)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to activate "
                    "domain %s", entry.domain_id);
                return MLAG_TYPE_RETURN_ERROR;
            }
        }
    }
    else /* status == MLAG_TYPE_GLOBAL_STATUS_DISABLED */
    {
        if (SWCTRL_PMGR_SetPktTrapStatus(SWCTRL_PKTTYPE_ORG_SPECIFIC3,
                SWCTRL_ORG_SPECIFIC3_TRAP_BY_MLAG, FALSE, TRUE) == FALSE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to disable "
                "packet trap status");
            return MLAG_TYPE_RETURN_ERROR;
        }

        while (MLAG_OM_GetNextDomainEntry(&entry) == MLAG_TYPE_RETURN_OK)
        {
            if (MLAG_MGR_InactivateDomain(&entry) != MLAG_TYPE_RETURN_OK)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                    "inactivate domain %s", entry.domain_id);
                return MLAG_TYPE_RETURN_ERROR;
            }
        }
    }

    if (MLAG_OM_SetGlobalStatus(status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set global "
            "status");
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_MGR_SetGlobalStatus */

/* FUNCTION NAME - MLAG_MGR_SetDomain
 * PURPOSE : Set a MLAG domain.
 * INPUT   : domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 *           lport       -- peer link
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_MGR_SetDomain(char *domain_id_p, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;
    UI32_T                  status;
    BOOL_T                  existent;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "lport=%lu", (unsigned long)lport);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "not master mode");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (domain_id_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "NULL pointer");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if ((strlen(domain_id_p) < MLAG_TYPE_MIN_DOMAIN_ID_LEN) ||
        (strlen(domain_id_p) > MLAG_TYPE_MAX_DOMAIN_ID_LEN))
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "invalid domain ID length");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_MGR_IsAlphaNumbericString(domain_id_p) == FALSE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "domain ID is not "
            "alphanumeric string");
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    mlag_entry.local_member = lport;
    if (MLAG_OM_GetMlagEntryByPort(&mlag_entry) == MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "lport %lu is an member",
            (unsigned long)lport);
        return MLAG_TYPE_RETURN_ERROR;
    }

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    SYSCTRL_XOR_MGR_GetSemaphore();
    if (SYSCTRL_XOR_MGR_PermitBeingSetToMlagPort(lport) == FALSE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "lport %lu is not permitted "
            "to be set as peer link", (unsigned long)lport);
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
        return MLAG_TYPE_RETURN_ERROR;
    }
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    strncpy(domain_entry.domain_id, domain_id_p, MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    if (MLAG_OM_GetDomainEntry(&domain_entry) == MLAG_TYPE_RETURN_OK)
    {
        if (domain_entry.peer_link == lport)
        {
            MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_OK);
        }
        existent = TRUE;
    }
    else
    {
        if (MLAG_OM_AddPrivateEntry(domain_id_p) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to add private "
                "entry");
            MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
        }
        existent = FALSE;
    }

    if (MLAG_OM_GetGlobalStatus(&status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get global "
            "status");
        MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
    }
    if (status == MLAG_TYPE_GLOBAL_STATUS_ENABLED)
    {
        if (existent == TRUE)
        {
            if (SWCTRL_PMGR_SetPortEgressBlockEx(domain_entry.peer_link, NULL,
                    NULL) == FALSE)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set "
                    "egress block ex (%lu, NULL, NULL)",
                    (unsigned long)domain_entry.peer_link);
                MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
            }
            if (SWCTRL_PMGR_SetPortMACLearningStatus(domain_entry.peer_link,
                    TRUE) == FALSE)
            {
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mac "
                    "learning (%lu, TRUE)",
                    (unsigned long)domain_entry.peer_link);
                MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
            }
        }

        domain_entry.peer_link = lport;
        if (MLAG_MGR_ActivateDomain(&domain_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to activate "
                "domain");
            MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
        }
    } /* end if global status is enabled */

    domain_entry.peer_link = lport;
    if (MLAG_OM_SetDomainEntry(&domain_entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set domain "
            "entry");
        MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
    }

    MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_OK);
} /* End of MLAG_MGR_SetDomain */

/* FUNCTION NAME - MLAG_MGR_RemoveDomain
 * PURPOSE : Remove a MLAG domain.
 * INPUT   : domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_MGR_RemoveDomain(char *domain_id_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_DomainEntry_T entry;
    UI32_T                  status;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "");

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "not master mode");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (domain_id_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "NULL pointer");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if ((strlen(domain_id_p) < MLAG_TYPE_MIN_DOMAIN_ID_LEN) ||
        (strlen(domain_id_p) > MLAG_TYPE_MAX_DOMAIN_ID_LEN))
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "invalid domain ID length");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_MGR_IsAlphaNumbericString(domain_id_p) == FALSE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "domain ID is not "
            "alphanumeric string");
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    strncpy(entry.domain_id, domain_id_p, MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    if (MLAG_OM_GetDomainEntry(&entry) != MLAG_TYPE_RETURN_OK)
    {
        return MLAG_TYPE_RETURN_OK;
    }

    if (MLAG_OM_GetGlobalStatus(&status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get global "
            "status");
        return MLAG_TYPE_RETURN_ERROR;
    }
    if (status == MLAG_TYPE_GLOBAL_STATUS_ENABLED)
    {
        if (MLAG_MGR_InactivateDomain(&entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to inactivate "
                "domain");
            return MLAG_TYPE_RETURN_ERROR;
        }
    }

    if (MLAG_OM_DeletePrivateEntry(domain_id_p) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to delete private "
            "entry");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_OM_RemoveDomainEntry(&entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to remove domain "
            "entry");
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_MGR_RemoveDomain */

/* FUNCTION NAME - MLAG_MGR_SetMlag
 * PURPOSE : Set a MLAG.
 * INPUT   : mlag_id     -- MLAG ID
 *           lport       -- MLAG member
 *           domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_MGR_SetMlag(UI32_T mlag_id, UI32_T lport, char *domain_id_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;
    UI32_T                  status;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "mlag_id=%lu, lport=%lu",
        (unsigned long)mlag_id, (unsigned long)lport);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "not master mode");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if ((mlag_id < MLAG_TYPE_MIN_MLAG_ID) ||
        (mlag_id > MLAG_TYPE_MAX_MLAG_ID))
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "invalid MLAG ID");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (domain_id_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "NULL pointer");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if ((strlen(domain_id_p) < MLAG_TYPE_MIN_DOMAIN_ID_LEN) ||
        (strlen(domain_id_p) > MLAG_TYPE_MAX_DOMAIN_ID_LEN))
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "invalid domain ID length");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_MGR_IsAlphaNumbericString(domain_id_p) == FALSE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "domain ID is not "
            "alphanumeric string");
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    mlag_entry.local_member = lport;
    if (MLAG_OM_GetMlagEntryByPort(&mlag_entry) == MLAG_TYPE_RETURN_OK)
    {
        if (mlag_entry.mlag_id != mlag_id)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "lport %lu is already "
                "in other MLAG", (unsigned long)lport);
            return MLAG_TYPE_RETURN_ERROR;
        }
    }
    else
    {
        domain_entry.peer_link = lport;
        if (MLAG_OM_GetDomainEntryByPort(&domain_entry) == MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "lport %lu is a peer "
                "link", (unsigned long)lport);
            return MLAG_TYPE_RETURN_ERROR;
        }
    }

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    SYSCTRL_XOR_MGR_GetSemaphore();
    if (SYSCTRL_XOR_MGR_PermitBeingSetToMlagPort(lport) == FALSE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "lport %lu is not permitted "
            "to be set as MLAG member", (unsigned long)lport);
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
        return MLAG_TYPE_RETURN_ERROR;
    }
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

    if (MLAG_OM_GetGlobalStatus(&status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get global "
            "status");
        MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
    }
    if (status == MLAG_TYPE_GLOBAL_STATUS_ENABLED)
    {
        mlag_entry.mlag_id = mlag_id;
        if (MLAG_OM_GetMlagEntry(&mlag_entry) == MLAG_TYPE_RETURN_OK)
        {
            if (mlag_entry.remote_state != MLAG_TYPE_STATE_INEXISTENT)
            {
                memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
                strncpy(domain_entry.domain_id, mlag_entry.domain_id,
                    MLAG_TYPE_MAX_DOMAIN_ID_LEN);
                if (MLAG_OM_GetDomainEntry(&domain_entry) !=
                        MLAG_TYPE_RETURN_OK)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get "
                        "domain entry");
                    MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
                }

                if (mlag_entry.local_member != lport)
                {
                    if (SWCTRL_PMGR_SetPortEgressBlock(domain_entry.peer_link,
                            mlag_entry.local_member, TRUE) == FALSE)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "set egress block (%lu, %lu, TRUE)",
                            (unsigned long)domain_entry.peer_link, (unsigned long)mlag_entry.local_member);
                        MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
                    }
                    if (AMTR_PMGR_SetMlagMacNotifyPortStatus(
                            mlag_entry.local_member, FALSE) == FALSE)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "set mac notify (%lu, FALSE)",
                            (unsigned long)mlag_entry.local_member);
                        MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
                    }
                    if (SWCTRL_PMGR_SetPortOperDormantStatus(
                            mlag_entry.local_member,
                            SWCTRL_OPER_DORMANT_LV_MLAG, FALSE, FALSE) == FALSE)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "set dormant status (%lu, LV_MLAG, FALSE, FALSE)",
                            (unsigned long)mlag_entry.local_member);
                        MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
                    }
                }
                if (strncmp(mlag_entry.domain_id, domain_id_p,
                        MLAG_TYPE_MAX_DOMAIN_ID_LEN) != 0)
                {
                    mlag_entry.local_state = MLAG_TYPE_STATE_INEXISTENT;
                    if (MLAG_MGR_SendLinkPacket(&mlag_entry) !=
                            MLAG_TYPE_RETURN_OK)
                    {
                        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to "
                            "send LINK packet");
                        MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
                    }
                }
            } /* end if remote state is not inexistent */
        } /* end if get mlag entry is OK */

        memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
        strncpy(domain_entry.domain_id, domain_id_p,
            MLAG_TYPE_MAX_DOMAIN_ID_LEN);

        if (MLAG_OM_GetDomainEntry(&domain_entry) == MLAG_TYPE_RETURN_OK)
        {
            if (SWCTRL_IsPortOperationUp(lport) == TRUE)
            {
                mlag_entry.local_state = MLAG_TYPE_STATE_UP;
            }
            else
            {
                mlag_entry.local_state = MLAG_TYPE_STATE_DOWN;
            }

            if (SWCTRL_IsPortOperationUp(domain_entry.peer_link) == TRUE)
            {
                strncpy(mlag_entry.domain_id, domain_id_p,
                    MLAG_TYPE_MAX_DOMAIN_ID_LEN);
                mlag_entry.local_member = lport;
                if (MLAG_MGR_SendLinkPacket(&mlag_entry) != MLAG_TYPE_RETURN_OK)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to sned "
                        "LINK packet");
                    MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
                }
                if (MLAG_OM_SetDigest(domain_id_p, MLAG_OM_DIGEST_LINK, NULL) !=
                        MLAG_TYPE_RETURN_OK)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set "
                        "LINK digest");
                    MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
                }
                if (MLAG_OM_SetDigest(domain_id_p, MLAG_OM_DIGEST_FDB, NULL) !=
                        MLAG_TYPE_RETURN_OK)
                {
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set "
                        "FDB digest");
                    MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
                }
            }
        } /* end if get domain entry is OK */
    } /* end if global status is enabled */

    mlag_entry.mlag_id = mlag_id;
    strncpy(mlag_entry.domain_id, domain_id_p, MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    mlag_entry.local_member = lport;
    if (MLAG_OM_SetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mlag entry");
        MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_ERROR);
    }

    MLAG_MGR_XOR_UNLOCK_AND_RETURN(MLAG_TYPE_RETURN_OK);
} /* End of MLAG_MGR_SetMlag */

/* FUNCTION NAME - MLAG_MGR_RemoveMlag
 * PURPOSE : Remove a MLAG.
 * INPUT   : mlag_id -- MLAG ID
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_MGR_RemoveMlag(UI32_T mlag_id)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;
    UI32_T                  status;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "malg_id=%lu", (unsigned long)mlag_id);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "not master mode");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if ((mlag_id < MLAG_TYPE_MIN_MLAG_ID) ||
        (mlag_id > MLAG_TYPE_MAX_MLAG_ID))
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "invalid MLAG ID");
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    mlag_entry.mlag_id = mlag_id;
    if (MLAG_OM_GetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
    {
        return MLAG_TYPE_RETURN_OK;
    }

    if (MLAG_OM_GetGlobalStatus(&status) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get global "
            "status");
        return MLAG_TYPE_RETURN_ERROR;
    }
    if ((status == MLAG_TYPE_GLOBAL_STATUS_ENABLED) &&
        (mlag_entry.remote_state != MLAG_TYPE_STATE_INEXISTENT))
    {
        memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
        strncpy(domain_entry.domain_id, mlag_entry.domain_id,
            MLAG_TYPE_MAX_DOMAIN_ID_LEN);
        if (MLAG_OM_GetDomainEntry(&domain_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get domain "
                "entry");
            return MLAG_TYPE_RETURN_ERROR;
        }

        if (SWCTRL_PMGR_SetPortEgressBlock(domain_entry.peer_link,
                mlag_entry.local_member, TRUE) == FALSE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set egress "
                "block (%lu, %lu, TRUE)", (unsigned long)domain_entry.peer_link,
                (unsigned long)mlag_entry.local_member);
            return MLAG_TYPE_RETURN_ERROR;
        }
        if (AMTR_PMGR_SetMlagMacNotifyPortStatus(mlag_entry.local_member, FALSE)
                == FALSE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mac "
                "notify (%lu, FALSE)", (unsigned long)mlag_entry.local_member);
            return MLAG_TYPE_RETURN_ERROR;
        }
        if (SWCTRL_PMGR_SetPortOperDormantStatus(mlag_entry.local_member,
                SWCTRL_OPER_DORMANT_LV_MLAG, FALSE, FALSE) == FALSE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set dormant "
                "status (%lu, LV_MLAG, FALSE, FALSE)", (unsigned long)mlag_entry.local_member);
            return MLAG_TYPE_RETURN_ERROR;
        }

        mlag_entry.local_state = MLAG_TYPE_STATE_INEXISTENT;
        if (MLAG_MGR_SendLinkPacket(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send LINK "
                "packet");
            return MLAG_TYPE_RETURN_ERROR;
        }

        if (MLAG_MGR_RemoveRemoteFdbForMlag(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to remove "
                "remote FDB for mlag %lu", (unsigned long)mlag_id);
            return MLAG_TYPE_RETURN_ERROR;
        }
    } /* end if global status is enabled & remote state is not inexistent */

    if (MLAG_OM_RemoveMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to remove mlag "
            "entry");
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_MGR_RemoveMlag */

/* FUNCTION NAME - MLAG_MGR_HandleIPCReqMsg
 * PURPOSE : Handle the IPC request message.
 * INPUT   : msgbuf_p -- IPC request message buffer
 * OUTPUT  : msgbuf_p -- IPC response message buffer
 * RETURN  : TRUE  -- there is a response required to be sent
 *           FALSE -- there is no response required to be sent
 * NOTE    : None
 */
BOOL_T MLAG_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_MGR_IpcMsg_T *msg_p;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "");

    if (msgbuf_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "NULL pointer");
        return FALSE;
    }

    msg_p = (MLAG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* IPC request will fail if operating mode is not master mode */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = MLAG_MGR_IPCMSG_TYPE_SIZE;
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "not master mode");
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding MGR function
     */
    switch (msg_p->type.cmd)
    {
        case MLAG_MGR_IPC_SETGLOBALSTATUS:
            msg_p->type.ret_ui32 = MLAG_MGR_SetGlobalStatus(
                                    msg_p->data.arg_ui32);
            msgbuf_p->msg_size = MLAG_MGR_IPCMSG_TYPE_SIZE;
            break;

        case MLAG_MGR_IPC_SETDOMAIN:
            msg_p->type.ret_ui32 = MLAG_MGR_SetDomain(
                                    msg_p->data.arg_str_ui32.arg_str,
                                    msg_p->data.arg_str_ui32.arg_ui32);
            msgbuf_p->msg_size = MLAG_MGR_IPCMSG_TYPE_SIZE;
            break;

        case MLAG_MGR_IPC_REMOVEDOMAIN:
            msg_p->type.ret_ui32 = MLAG_MGR_RemoveDomain(msg_p->data.arg_str);
            msgbuf_p->msg_size = MLAG_MGR_IPCMSG_TYPE_SIZE;
            break;

        case MLAG_MGR_IPC_SETMLAG:
            msg_p->type.ret_ui32 = MLAG_MGR_SetMlag(
                                    msg_p->data.arg_ui32_ui32_str.arg_ui32_1,
                                    msg_p->data.arg_ui32_ui32_str.arg_ui32_2,
                                    msg_p->data.arg_ui32_ui32_str.arg_str);
            msgbuf_p->msg_size = MLAG_MGR_IPCMSG_TYPE_SIZE;
            break;

        case MLAG_MGR_IPC_REMOVEMLAG:
            msg_p->type.ret_ui32 = MLAG_MGR_RemoveMlag(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = MLAG_MGR_IPCMSG_TYPE_SIZE;
            break;

        default:
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "invalid IPC command");
            msg_p->type.ret_bool = FALSE;
            msgbuf_p->msg_size = MLAG_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of MLAG_MGR_HandleIPCReqMsg */

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_MGR_SendLinkPacketForDomain
 * PURPOSE : Send a LINK packet for all MLAGs in a MLAG domain.
 * INPUT   : entry_p -- pointer to a domain entry
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : If entry_p is NULL, send for all MLAG domains
 */
static UI32_T MLAG_MGR_SendLinkPacketForDomain(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    UI8_T                   *data_p;
    UI32_T                  pdu_len;
    MLAG_MGR_CommonHeader_T *common_p;
    MLAG_MGR_LinkEntry_T    *link_entry_p;
    MLAG_MGR_LinkEntry_T    link_entry;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "");

    if (SWCTRL_IsPortOperationUp(entry_p->peer_link) == FALSE)
    {
        return MLAG_TYPE_RETURN_OK;
    }

    data_p = (UI8_T*)malloc(sizeof(MLAG_MGR_CommonHeader_T) +
                MLAG_TYPE_MAX_NBR_OF_MLAG * sizeof(MLAG_MGR_LinkEntry_T));
    if (data_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to malloc");
        return MLAG_TYPE_RETURN_ERROR;
    }

    pdu_len = 0;
    common_p = (MLAG_MGR_CommonHeader_T*)data_p;
    MLAG_MGR_CONSTRUCT_COMMON_HEADER(common_p, pdu_len, MLAG_MGR_SUBTYPE_LINK,
        entry_p->domain_id);

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    strncpy(mlag_entry.domain_id, entry_p->domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    while (MLAG_OM_GetNextMlagEntryByDomain(&mlag_entry) == MLAG_TYPE_RETURN_OK)
    {
        memset(&link_entry, 0, sizeof(MLAG_MGR_LinkEntry_T));
        link_entry.state_agree_gid = mlag_entry.local_state << 14;
        if (mlag_entry.remote_state == MLAG_TYPE_STATE_DORMANT)
        {
            link_entry.state_agree_gid |= MLAG_MGR_AGREE << 13;
        }
        else
        {
            link_entry.state_agree_gid |= MLAG_MGR_DISAGREE << 13;
        }
        link_entry.state_agree_gid |= mlag_entry.mlag_id;

        link_entry_p = (MLAG_MGR_LinkEntry_T*)(data_p + pdu_len);
        link_entry_p->state_agree_gid = L_STDLIB_Hton16(
                                            link_entry.state_agree_gid);

        pdu_len += sizeof(MLAG_MGR_LinkEntry_T);;
        common_p->num_of_entries++;

        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_TX, "MLAG ID:%lu, Agree:%u, "
            "State:%lu", (unsigned long)mlag_entry.mlag_id, (mlag_entry.remote_state ==
            MLAG_TYPE_STATE_DORMANT) ? MLAG_MGR_AGREE : MLAG_MGR_DISAGREE,
            (unsigned long)mlag_entry.local_state);
    } /* end while get next mlag entry */

    if (common_p->num_of_entries == 0)
    {
        free(data_p);
        return MLAG_TYPE_RETURN_OK;
    }

    L_MD5_HMAC_MD5(data_p, pdu_len, digest_key, sizeof(digest_key),
        common_p->digest);

    if (MLAG_MGR_SendPacket(entry_p->peer_link, data_p, pdu_len)
            == MLAG_TYPE_RETURN_OK)
    {
        free(data_p);
        return MLAG_TYPE_RETURN_OK;
    }
    else
    {
        free(data_p);
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send packet");
        return MLAG_TYPE_RETURN_ERROR;
    }
} /* End of MLAG_MGR_SendLinkPacketForDomain */

/* FUNCTION NAME - MLAG_MGR_SendLinkPacket
 * PURPOSE : Send a LINK packet for a MLAG.
 * INPUT   : entry_p -- pointer to a MLAG entry
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
static UI32_T MLAG_MGR_SendLinkPacket(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_DomainEntry_T domain_entry;
    UI8_T                   *data_p;
    UI32_T                  pdu_len;
    MLAG_MGR_CommonHeader_T *common_p;
    MLAG_MGR_LinkEntry_T    *link_entry_p;
    MLAG_MGR_LinkEntry_T    link_entry;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "");

    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    strncpy(domain_entry.domain_id, entry_p->domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    if (MLAG_OM_GetDomainEntry(&domain_entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get domain "
            "entry");
        return MLAG_TYPE_RETURN_ERROR;
    }
    if (SWCTRL_IsPortOperationUp(domain_entry.peer_link) == FALSE)
    {
        return MLAG_TYPE_RETURN_OK;
    }

    data_p = (UI8_T*)malloc(sizeof(MLAG_MGR_CommonHeader_T) +
                sizeof(MLAG_MGR_LinkEntry_T));
    if (data_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "NULL pointer");
        return MLAG_TYPE_RETURN_ERROR;
    }

    pdu_len = 0;
    common_p = (MLAG_MGR_CommonHeader_T*)data_p;
    MLAG_MGR_CONSTRUCT_COMMON_HEADER(common_p, pdu_len, MLAG_MGR_SUBTYPE_LINK,
        domain_entry.domain_id);

    memset(&link_entry, 0, sizeof(MLAG_MGR_LinkEntry_T));
    link_entry.state_agree_gid = entry_p->local_state << 14;
    if (entry_p->remote_state == MLAG_TYPE_STATE_DORMANT)
    {
        link_entry.state_agree_gid |= MLAG_MGR_AGREE << 13;
    }
    else
    {
        link_entry.state_agree_gid |= MLAG_MGR_DISAGREE << 13;
    }
    link_entry.state_agree_gid |= entry_p->mlag_id;

    link_entry_p = (MLAG_MGR_LinkEntry_T*)(data_p + pdu_len);
    link_entry_p->state_agree_gid = L_STDLIB_Hton16(link_entry.state_agree_gid);
    pdu_len += sizeof(MLAG_MGR_LinkEntry_T);;
    common_p->num_of_entries++;

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_TX, "MLAG ID:%lu, Agree:%u, State:%lu",
        (unsigned long)entry_p->mlag_id, (entry_p->remote_state == MLAG_TYPE_STATE_DORMANT) ?
        MLAG_MGR_AGREE : MLAG_MGR_DISAGREE, (unsigned long)entry_p->local_state);

    L_MD5_HMAC_MD5(data_p, pdu_len, digest_key, sizeof(digest_key),
        common_p->digest);

    if (MLAG_MGR_SendPacket(domain_entry.peer_link, data_p, pdu_len)
            == MLAG_TYPE_RETURN_OK)
    {
        free(data_p);
        return MLAG_TYPE_RETURN_OK;
    }
    else
    {
        free(data_p);
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send packet");
        return MLAG_TYPE_RETURN_ERROR;
    }
} /* End of MLAG_MGR_SendLinkPacket */

/* FUNCTION NAME - MLAG_MGR_SendFdbPacketForDomain
 * PURPOSE : Send FDB packet(s) for all MLAGs in a MLAG domain.
 * INPUT   : entry_p -- pointer to a domain entry
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
static UI32_T MLAG_MGR_SendFdbPacketForDomain(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    AMTR_TYPE_AddrEntry_T   addr_entry;
    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_OM_FdbEntry_T      om_entry;
    MLAG_MGR_FdbEntry_T     fdb_entry;
    MLAG_MGR_FdbEntry_T     *fdb_entry_p;
    UI8_T                   *data_p;
    UI32_T                  pdu_len;
    MLAG_MGR_CommonHeader_T *common_p;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "");

    if (SWCTRL_IsPortOperationUp(entry_p->peer_link) == FALSE)
    {
        return MLAG_TYPE_RETURN_OK;
    }

    data_p = (UI8_T*)malloc(MLAG_MGR_MAX_PDU_SIZE);
    if (data_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to malloc");
        return MLAG_TYPE_RETURN_ERROR;
    }

    pdu_len = 0;
    common_p = (MLAG_MGR_CommonHeader_T*)data_p;
    MLAG_MGR_CONSTRUCT_COMMON_HEADER(common_p, pdu_len, MLAG_MGR_SUBTYPE_FDB,
        entry_p->domain_id);

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    strncpy(mlag_entry.domain_id, entry_p->domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    while (MLAG_OM_GetNextMlagEntryByDomain(&mlag_entry) == MLAG_TYPE_RETURN_OK)
    {
        if (mlag_entry.local_state != MLAG_TYPE_STATE_UP)
        {
            continue;
        }

        memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
        addr_entry.ifindex = mlag_entry.local_member;
        while (AMTR_PMGR_GetNextIfIndexAddrEntry(&addr_entry,
                AMTR_MGR_GET_DYNAMIC_ADDRESS) == TRUE)
        {
            if (addr_entry.ifindex != mlag_entry.local_member)
            {
                break;
            }

            memset(&om_entry, 0, sizeof(MLAG_OM_FdbEntry_T));
            om_entry.vid = addr_entry.vid;
            memcpy(om_entry.mac, addr_entry.mac, SYS_ADPT_MAC_ADDR_LEN);
            if ((MLAG_OM_GetRemoteFdbEntry(&om_entry) == MLAG_TYPE_RETURN_OK) &&
                (om_entry.mlag_id == mlag_entry.mlag_id))
            {
                continue;
            }

            memset(&fdb_entry, 0, sizeof(MLAG_MGR_FdbEntry_T));
            MLAG_MGR_FDB_PACK_STATE(fdb_entry.state_gid_vid,MLAG_MGR_STATE_LEARNED);
            MLAG_MGR_FDB_PACK_MLAG_ID(fdb_entry.state_gid_vid,mlag_entry.mlag_id);
            MLAG_MGR_FDB_PACK_VLAN(fdb_entry.state_gid_vid,addr_entry.vid);

            fdb_entry_p = (MLAG_MGR_FdbEntry_T*)(data_p + pdu_len);
            fdb_entry_p->state_gid_vid = L_STDLIB_Hton32(
                                            fdb_entry.state_gid_vid);
            memcpy(fdb_entry_p->mac, addr_entry.mac, SYS_ADPT_MAC_ADDR_LEN);
            pdu_len += sizeof(MLAG_MGR_FdbEntry_T);
            common_p->num_of_entries++;

            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_TX, "\r\n[Learned] MAC:%02X-"
                "%02X-%02X-%02X-%02X-%02X, VID:%u, MLAG ID:%lu",
                addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2],
                addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5],
                addr_entry.vid, (unsigned long)mlag_entry.mlag_id);

            if (pdu_len > MLAG_MGR_FDB_PAYLOAD_THRESHOLD)
            {
                L_MD5_HMAC_MD5(data_p, pdu_len, digest_key, sizeof(digest_key),
                    common_p->digest);
                if (MLAG_MGR_SendPacket(entry_p->peer_link, data_p, pdu_len)
                        != MLAG_TYPE_RETURN_OK)
                {
                    free(data_p);
                    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send"
                        "packet");
                    return MLAG_TYPE_RETURN_ERROR;
                }
                pdu_len -= common_p->num_of_entries*sizeof(MLAG_MGR_FdbEntry_T);
                common_p->num_of_entries = 0;
            }
        } /* end while get next ifindex address entry */
    } /* end while get next mlag entry */

    if (common_p->num_of_entries == 0)
    {
        free(data_p);
        return MLAG_TYPE_RETURN_OK;
    }

    L_MD5_HMAC_MD5(data_p, pdu_len, digest_key, sizeof(digest_key),
        common_p->digest);

    if (MLAG_MGR_SendPacket(entry_p->peer_link, data_p, pdu_len)
            == MLAG_TYPE_RETURN_OK)
    {
        free(data_p);
        return MLAG_TYPE_RETURN_OK;
    }
    else
    {
        free(data_p);
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send packet");
        return MLAG_TYPE_RETURN_ERROR;
    }
} /* End of MLAG_MGR_SendFdbPacketForDomain */

/* FUNCTION NAME - MLAG_MGR_SendFdbPacketForMlag
 * PURPOSE : Send FDB packet(s) for a MLAG.
 * INPUT   : entry_p -- pointer to a MLAG entry
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : If local member is not oper-up, do nothing.
 */
static UI32_T MLAG_MGR_SendFdbPacketForMlag(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_DomainEntry_T domain_entry;
    AMTR_TYPE_AddrEntry_T   addr_entry;
    MLAG_OM_FdbEntry_T      om_entry;
    MLAG_MGR_FdbEntry_T     fdb_entry;
    MLAG_MGR_FdbEntry_T     *fdb_entry_p;
    UI8_T                   *data_p;
    UI32_T                  pdu_len;
    MLAG_MGR_CommonHeader_T *common_p;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "");

    if (entry_p->local_state != MLAG_TYPE_STATE_UP)
    {
        return MLAG_TYPE_RETURN_OK;
    }

    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    strncpy(domain_entry.domain_id, entry_p->domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    if (MLAG_OM_GetDomainEntry(&domain_entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get domain "
            "entry");
        return MLAG_TYPE_RETURN_ERROR;
    }
    if (SWCTRL_IsPortOperationUp(domain_entry.peer_link) == FALSE)
    {
        return MLAG_TYPE_RETURN_OK;
    }

    data_p = (UI8_T*)malloc(MLAG_MGR_MAX_PDU_SIZE);
    if (data_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to malloc");
        return MLAG_TYPE_RETURN_ERROR;
    }

    pdu_len = 0;
    common_p = (MLAG_MGR_CommonHeader_T*)data_p;
    MLAG_MGR_CONSTRUCT_COMMON_HEADER(common_p, pdu_len, MLAG_MGR_SUBTYPE_FDB,
        domain_entry.domain_id);

    memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
    addr_entry.ifindex = entry_p->local_member;
    while (AMTR_PMGR_GetNextIfIndexAddrEntry(&addr_entry,
            AMTR_MGR_GET_DYNAMIC_ADDRESS) == TRUE)
    {
        if (addr_entry.ifindex != entry_p->local_member)
        {
            break;
        }

        memset(&om_entry, 0, sizeof(MLAG_OM_FdbEntry_T));
        om_entry.vid = addr_entry.vid;
        memcpy(om_entry.mac, addr_entry.mac, SYS_ADPT_MAC_ADDR_LEN);
        if ((MLAG_OM_GetRemoteFdbEntry(&om_entry) == MLAG_TYPE_RETURN_OK) &&
            (om_entry.mlag_id == entry_p->mlag_id))
        {
            continue;
        }

        memset(&fdb_entry, 0, sizeof(MLAG_MGR_FdbEntry_T));
        MLAG_MGR_FDB_PACK_STATE(fdb_entry.state_gid_vid,MLAG_MGR_STATE_LEARNED);
        MLAG_MGR_FDB_PACK_MLAG_ID(fdb_entry.state_gid_vid,entry_p->mlag_id);
        MLAG_MGR_FDB_PACK_VLAN(fdb_entry.state_gid_vid,addr_entry.vid);

        fdb_entry_p = (MLAG_MGR_FdbEntry_T*)(data_p + pdu_len);
        fdb_entry_p->state_gid_vid = L_STDLIB_Hton32(fdb_entry.state_gid_vid);
        memcpy(fdb_entry_p->mac, addr_entry.mac, SYS_ADPT_MAC_ADDR_LEN);
        pdu_len += sizeof(MLAG_MGR_FdbEntry_T);
        common_p->num_of_entries++;

        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_TX, "\r\n[Learned] MAC:%02X-%02X-"
            "%02X-%02X-%02X-%02X, VID:%u, MLAG ID:%lu", addr_entry.mac[0],
            addr_entry.mac[1], addr_entry.mac[2], addr_entry.mac[3],
            addr_entry.mac[4], addr_entry.mac[5], addr_entry.vid,
            (unsigned long)entry_p->mlag_id);

        if (pdu_len > MLAG_MGR_FDB_PAYLOAD_THRESHOLD)
        {
            L_MD5_HMAC_MD5(data_p, pdu_len, digest_key,
                sizeof(digest_key), common_p->digest);
            if (MLAG_MGR_SendPacket(domain_entry.peer_link,
                data_p, pdu_len) != MLAG_TYPE_RETURN_OK)
            {
                free(data_p);
                MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send "
                    "packet");
                return MLAG_TYPE_RETURN_ERROR;
            }
            pdu_len -= common_p->num_of_entries *
                        sizeof(MLAG_MGR_FdbEntry_T);
            common_p->num_of_entries = 0;
        }
    } /* end while get next ifindex address entry */

    if (common_p->num_of_entries == 0)
    {
        free(data_p);
        return MLAG_TYPE_RETURN_OK;
    }

    L_MD5_HMAC_MD5(data_p, pdu_len, digest_key, sizeof(digest_key),
        common_p->digest);

    if (MLAG_MGR_SendPacket(domain_entry.peer_link, data_p, pdu_len)
            == MLAG_TYPE_RETURN_OK)
    {
        free(data_p);
        return MLAG_TYPE_RETURN_OK;
    }
    else
    {
        free(data_p);
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send packet");
        return MLAG_TYPE_RETURN_ERROR;
    }
} /* End of MLAG_MGR_SendFdbPacketForMlag */

/* FUNCTION NAME - MLAG_MGR_SendFdbPacket
 * PURPOSE : Send a FDB packet for a FDB entry.
 * INPUT   : entry_p -- pointer to a FDB entry
 *           added   -- if TRUE, action is learn; else, release
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
static UI32_T MLAG_MGR_SendFdbPacket(MLAG_OM_FdbEntry_T *entry_p, BOOL_T added)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;
    MLAG_MGR_FdbEntry_T     fdb_entry;
    MLAG_MGR_FdbEntry_T     *fdb_entry_p;
    UI8_T                   *data_p;
    UI32_T                  pdu_len;
    MLAG_MGR_CommonHeader_T *common_p;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "added=%hu", added);

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    mlag_entry.mlag_id = entry_p->mlag_id;
    if (MLAG_OM_GetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get mlag entry");
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    strncpy(domain_entry.domain_id, mlag_entry.domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    if (MLAG_OM_GetDomainEntry(&domain_entry) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get domain "
            "entry");
        return MLAG_TYPE_RETURN_ERROR;
    }
    if (SWCTRL_IsPortOperationUp(domain_entry.peer_link) == FALSE)
    {
        return MLAG_TYPE_RETURN_OK;
    }

    data_p = (UI8_T*)malloc(sizeof(MLAG_MGR_CommonHeader_T) +
                sizeof(MLAG_MGR_FdbEntry_T));
    if (data_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to malloc");
        return MLAG_TYPE_RETURN_ERROR;
    }

    pdu_len = 0;
    common_p = (MLAG_MGR_CommonHeader_T*)data_p;
    MLAG_MGR_CONSTRUCT_COMMON_HEADER(common_p, pdu_len, MLAG_MGR_SUBTYPE_FDB,
        domain_entry.domain_id);

    memset(&fdb_entry, 0, sizeof(MLAG_MGR_FdbEntry_T));
    if (added == TRUE)
    {
        MLAG_MGR_FDB_PACK_MLAG_ID(fdb_entry.state_gid_vid , MLAG_MGR_STATE_LEARNED);
    }
    else
    {
        MLAG_MGR_FDB_PACK_MLAG_ID(fdb_entry.state_gid_vid, MLAG_MGR_STATE_AGED);
    }

    MLAG_MGR_FDB_PACK_MLAG_ID(fdb_entry.state_gid_vid, entry_p->mlag_id);
    MLAG_MGR_FDB_PACK_VLAN(fdb_entry.state_gid_vid, entry_p->vid);

    fdb_entry_p = (MLAG_MGR_FdbEntry_T*)(data_p + pdu_len);
    fdb_entry_p->state_gid_vid = L_STDLIB_Hton32(fdb_entry.state_gid_vid);
    memcpy(fdb_entry_p->mac, entry_p->mac, SYS_ADPT_MAC_ADDR_LEN);
    pdu_len += sizeof(MLAG_MGR_FdbEntry_T);
    common_p->num_of_entries++;

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_TX, "\r\n[%s] MAC:%02X-%02X-%02X-%02X-"
        "%02X-%02X, VID:%lu, MLAG ID:%lu", (added == TRUE) ? "Learned" : "Aged",
        entry_p->mac[0], entry_p->mac[1], entry_p->mac[2], entry_p->mac[3],
        entry_p->mac[4], entry_p->mac[5], (unsigned long)entry_p->vid, (unsigned long)entry_p->mlag_id);

    L_MD5_HMAC_MD5(data_p, pdu_len, digest_key, sizeof(digest_key),
        common_p->digest);

    if (MLAG_MGR_SendPacket(domain_entry.peer_link, data_p, pdu_len)
            == MLAG_TYPE_RETURN_OK)
    {
        free(data_p);
        return MLAG_TYPE_RETURN_OK;
    }
    else
    {
        free(data_p);
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send packet");
        return MLAG_TYPE_RETURN_ERROR;
    }
} /* End of MLAG_MGR_SendFdbPacket */

/* FUNCTION NAME - MLAG_MGR_SendPacket
 * PURPOSE : Send a packet.
 * INPUT   : lport    -- logical port from which packet is sent
 *           data_p   -- data buffer
 *           data_len -- size of data buffer
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
static UI32_T MLAG_MGR_SendPacket(UI32_T lport, UI8_T *data_p, UI32_T data_len)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               dst_mac_ar[] = SYS_DFLT_ORG_SPEC3_DA;
    L_MM_Mref_Handle_T  *mref_handle_p;
    UI8_T               *pdu_p;
    UI32_T              tmp_len;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "lport=%lu, data_len=%lu",
        (unsigned long)lport, (unsigned long)data_len);

    mref_handle_p = L_MM_AllocateTxBuffer(data_len,
                        L_MM_USER_ID2(SYS_MODULE_MLAG, 0));
    if (mref_handle_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to allocate tx "
            "buffer");
        return MLAG_TYPE_RETURN_ERROR;
    }

    pdu_p = (UI8_T*)L_MM_Mref_GetPdu(mref_handle_p, &tmp_len);
    if (pdu_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to get pdu address");
        return MLAG_TYPE_RETURN_ERROR;
    }

    memcpy(pdu_p, data_p, data_len);
    mref_handle_p->current_usr_id = SYS_MODULE_MLAG;
    mref_handle_p->next_usr_id = SYS_MODULE_LAN;
    L2MUX_MGR_SendBPDU(mref_handle_p,
                       dst_mac_ar,
                       cpu_mac_ar,
                       data_len,
                       1,
                       data_len,
                       lport,
                       FALSE,
                       MLAG_MGR_TX_COS,
                       FALSE);

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_TX, "TX to lport %lu (%lu)",
        (unsigned long)lport, (unsigned long)SYSFUN_GetSysTick());

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_MGR_SendPacket */

/* FUNCTION NAME - MLAG_MGR_ActivateDomain
 * PURPOSE : Activate a MLAG domain.
 * INPUT   : entry_p -- pointer to a domain entry
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
static UI32_T MLAG_MGR_ActivateDomain(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T   plist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    MLAG_TYPE_MlagEntry_T   mlag_entry;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "");

    if (entry_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "NULL pointer");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (SWCTRL_PMGR_SetPortMACLearningStatus(entry_p->peer_link, FALSE) ==
            FALSE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mac learning "
            "(%lu, FALSE)", (unsigned long)entry_p->peer_link);
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(plist, 0xff, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    if (SWCTRL_PMGR_SetPortEgressBlockEx(entry_p->peer_link, NULL, plist)
            == FALSE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set egress block "
            "ex (%lu, NULL, plist)", (unsigned long)entry_p->peer_link);
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    strncpy(mlag_entry.domain_id, entry_p->domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    while (MLAG_OM_GetNextMlagEntryByDomain(&mlag_entry) == MLAG_TYPE_RETURN_OK)
    {
        if (SWCTRL_IsPortOperationUp(mlag_entry.local_member) == TRUE)
        {
            mlag_entry.local_state = MLAG_TYPE_STATE_UP;
        }
        else
        {
            mlag_entry.local_state = MLAG_TYPE_STATE_DOWN;
        }

        if (MLAG_OM_SetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mlag "
                "entry");
            return MLAG_TYPE_RETURN_ERROR;
        }
    }

    if ((SWCTRL_IsPortOperationUp(entry_p->peer_link) == TRUE) &&
        (MLAG_MGR_HandleConnection(entry_p) != MLAG_TYPE_RETURN_OK))
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to handle "
            "connection for domain %s", entry_p->domain_id);
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_MGR_ActivateDomain */

/* FUNCTION NAME - MLAG_MGR_InactivateDomain
 * PURPOSE : Inactivate a MLAG domain.
 * INPUT   : domain_id_p -- MLAG domain ID
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
static UI32_T MLAG_MGR_InactivateDomain(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "");

    if (entry_p == NULL)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "NULL pointer");
        return MLAG_TYPE_RETURN_ERROR;
    }

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    strncpy(mlag_entry.domain_id, entry_p->domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    while (MLAG_OM_GetNextMlagEntryByDomain(&mlag_entry) == MLAG_TYPE_RETURN_OK)
    {
        mlag_entry.local_state = MLAG_TYPE_STATE_INEXISTENT;
        if (MLAG_OM_SetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mlag "
                "entry %lu", (unsigned long)mlag_entry.mlag_id);
            return MLAG_TYPE_RETURN_ERROR;
        }
    }

    if (SWCTRL_IsPortOperationUp(entry_p->peer_link) == TRUE)
    {
        if (MLAG_MGR_SendLinkPacketForDomain(entry_p) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send LINK "
                "packet for domain %s", entry_p->domain_id);
            return MLAG_TYPE_RETURN_ERROR;
        }

        if (MLAG_MGR_HandleDisconnection(entry_p) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to handle "
                "disconnection for domain %s", entry_p->domain_id);
            return MLAG_TYPE_RETURN_ERROR;
        }
    }

    if (SWCTRL_PMGR_SetPortEgressBlockEx(entry_p->peer_link, NULL, NULL)
            == FALSE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set egress block "
            "ex (%lu, NULL, NULL)", (unsigned long)entry_p->peer_link);
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (SWCTRL_PMGR_SetPortMACLearningStatus(entry_p->peer_link, TRUE) == FALSE)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mac learning "
            "(%lu, TRUE)", (unsigned long)entry_p->peer_link);
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_MGR_InactivateDomain */

/* FUNCTION NAME - MLAG_MGR_HandleReconnection
 * PURPOSE : Handle connection with peer.
 * INPUT   : entry_p -- pointer to a domain entry
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
static UI32_T MLAG_MGR_HandleConnection(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "");

    if (MLAG_MGR_SendLinkPacketForDomain(entry_p) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to send LINK packet "
            "for domain %s", entry_p->domain_id);
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_OM_SetTimer(entry_p->domain_id, MLAG_OM_TIMER_TX,
            MLAG_MGR_TIMEOUT_VALUE) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set tx timer for "
            "domain %s", entry_p->domain_id);
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_OM_SetTimer(entry_p->domain_id, MLAG_OM_TIMER_FDB,
            MLAG_MGR_TIMEOUT_VALUE) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set fdb timer "
            "for domain %s", entry_p->domain_id);
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_OM_SetTimer(entry_p->domain_id, MLAG_OM_TIMER_RX,
            MLAG_MGR_TIMEOUT_VALUE*2) != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set rx timer for "
            "domain %s", entry_p->domain_id);
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_MGR_HandleConnection */

/* FUNCTION NAME - MLAG_MGR_HandleDisconnection
 * PURPOSE : Handle disconnection with peer.
 * INPUT   : entry_p -- pointer to a domain entry
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
static UI32_T MLAG_MGR_HandleDisconnection(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    MLAG_TYPE_MlagEntry_T   mlag_entry;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "");

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    strncpy(mlag_entry.domain_id, entry_p->domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    while (MLAG_OM_GetNextMlagEntryByDomain(&mlag_entry) == MLAG_TYPE_RETURN_OK)
    {
        if (AMTR_PMGR_SetMlagMacNotifyPortStatus(mlag_entry.local_member, FALSE)
                == FALSE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mac "
                "notify (%lu, FALSE)", (unsigned long)mlag_entry.local_member);
            return MLAG_TYPE_RETURN_ERROR;
        }

        if (SWCTRL_PMGR_SetPortOperDormantStatus(mlag_entry.local_member,
                SWCTRL_OPER_DORMANT_LV_MLAG, FALSE, FALSE) == FALSE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set dormant "
                "status (%lu, LV_MLAG, FALSE, FALSE)", (unsigned long)mlag_entry.local_member);
            return MLAG_TYPE_RETURN_ERROR;
        }

        mlag_entry.remote_state = MLAG_TYPE_STATE_INEXISTENT;
        if (MLAG_OM_SetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set mlag "
                "entry");
            return MLAG_TYPE_RETURN_ERROR;
        }

        if (MLAG_MGR_RemoveRemoteFdbForMlag(&mlag_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to remove "
                "remote FDB for MLAG %lu", (unsigned long)mlag_entry.mlag_id);
            return MLAG_TYPE_RETURN_ERROR;
        }
    }

    if (MLAG_OM_SetTimer(entry_p->domain_id, MLAG_OM_TIMER_TX, 0)
            != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set tx timer");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_OM_SetTimer(entry_p->domain_id, MLAG_OM_TIMER_FDB, 0)
            != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set fdb timer");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_OM_SetTimer(entry_p->domain_id, MLAG_OM_TIMER_RX, 0)
            != MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set rx timer");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_OM_SetDigest(entry_p->domain_id, MLAG_OM_DIGEST_LINK, NULL) !=
            MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set LINK digest");
        return MLAG_TYPE_RETURN_ERROR;
    }

    if (MLAG_OM_SetDigest(entry_p->domain_id, MLAG_OM_DIGEST_FDB, NULL) !=
            MLAG_TYPE_RETURN_OK)
    {
        MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to set FDB digest");
        return MLAG_TYPE_RETURN_ERROR;
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_MGR_HandleDisconnection */

/* FUNCTION NAME - MLAG_MGR_RemoveRemoteFdbForMlag
 * PURPOSE : Remove remote fdb for a MLAG.
 * INPUT   : entry_p -- pointer to a MLAG entry
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Both OM and chip are updated
 */
static UI32_T MLAG_MGR_RemoveRemoteFdbForMlag(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    AMTR_TYPE_AddrEntry_T   addr_entry;
    MLAG_OM_FdbEntry_T      om_entry;

    /* BODY
     */

    MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_THREAD, "");

    memset(&om_entry, 0, sizeof(MLAG_OM_FdbEntry_T));
    om_entry.mlag_id = entry_p->mlag_id;
    while (MLAG_OM_GetNextRemoteFdbEntryByMlag(&om_entry) ==
            MLAG_TYPE_RETURN_OK)
    {
        memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
        addr_entry.vid = om_entry.vid;
        memcpy(addr_entry.mac, om_entry.mac, SYS_ADPT_MAC_ADDR_LEN);

        if (AMTR_PMGR_DeleteAddrForMlag(om_entry.vid, om_entry.mac) == FALSE)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to delete addr"
                " for mlag %lu", (unsigned long)entry_p->mlag_id);
            return MLAG_TYPE_RETURN_ERROR;
        }
        if (MLAG_OM_RemoveRemoteFdbEntry(&om_entry) != MLAG_TYPE_RETURN_OK)
        {
            MLAG_MGR_DEBUG_PRINTF(MLAG_OM_DEBUG_ERROR, "failed to remove remote"
                " fdb entry");
            return MLAG_TYPE_RETURN_ERROR;
        }
    }

    return MLAG_TYPE_RETURN_OK;
} /* End of MLAG_MGR_RemoveRemoteFdbForMlag */

/* FUNCTION NAME - MLAG_MGR_IsAlphaNumbericString
 * PURPOSE : Check whether a string is alphanumeric string.
 * INPUT   : str_p -- string to be checked
 * OUTPUT  : None
 * RETURN  : TRUE  -- the string is alphanumeric string
 *           FALSE -- the string is not alphanumeric string
 * NOTE    : None
 */
static BOOL_T MLAG_MGR_IsAlphaNumbericString(char *str_p)
{
    int c;

    for (c = *str_p; c != 0; c = *(str_p++))
    {
        if ((c < 48) ||
            ((c > 57) && (c < 65)) ||
            ((c > 90) && (c < 97)) ||
            (c > 122))
        {
            return FALSE;
        }
    }

    return TRUE;
} /* End of MLAG_MGR_IsAlphaNumbericString */

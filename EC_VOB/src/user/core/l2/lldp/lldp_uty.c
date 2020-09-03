/*-----------------------------------------------------------------------------
 * Module Name: lldp_uty.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementations for the LLDP utilities
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    03/06/2006 - Gordon Kao, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_module.h"
#include "sys_type.h"
#include "leaf_1213.h"
#include "leaf_2737.h"
#include "leaf_es3626a.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "l_sort_lst.h"
#include "l_stdlib.h"
#include "sysfun.h"
#if (SYS_CPNT_ADD == TRUE)
#include "add_om.h"
#endif
#include "if_pmgr.h"
#include "lan.h"
#include "lldp_om_private.h"
#include "lldp_engine.h"
#include "lldp_type.h"
#include "lldp_backdoor.h"
#include "mib2_pom.h"
#include "netcfg_type.h"
#include "netcfg_pom_ip.h"  /*added by Jinhua Wei ,to remove warning ,becaued the relative function's head file not included */
#if (SYS_CPNT_POE == TRUE)
#include "poe_pom.h"
#endif
#if (SYS_CPNT_PRI_MGMT_PORT_BASE == TRUE)
#include "pri_pmgr.h"
#endif
#include "stktplg_om.h"
#include "swctrl.h"
#include "sys_time.h"
#include "trk_lib.h"
#include "trk_pmgr.h"
#include "vlan_lib.h"
#include "vlan_om.h"
#include "vlan_pmgr.h"
#include "vlan_type.h"
#if (SYS_CPNT_DCBX == TRUE)
#include "dcbx_pmgr.h"
#endif
#if (SYS_CPNT_PFC == TRUE)
#include "pfc_pmgr.h"
#endif
#if (SYS_CPNT_ETS == TRUE)
#include "ets_pmgr.h"
#endif
#if (SYS_CPNT_CN == TRUE)
#include "cn_om.h"
#include "cn_type.h"
#endif


typedef struct
{
    UI32_T  protocol_ident_len;
    char    *protocol_ident;
} LLDP_UTY_ProtocolIdent_T;

static UI8_T  LLDP_MultiCastAddr[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E};
static UI8_T  LLDP_Org802dot1Oui[3] = {0x00, 0x80, 0xC2};
static UI8_T  LLDP_Org802dot3Oui[3] = {0x00, 0x12, 0x0F};
static UI8_T  LLDP_OrgTiaOui[3] = {0x00, 0x12, 0xBB};
static UI16_T  LLDP_EtherType = 0x88CC;


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructMandatoryTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct the first three mandatory TLVs
 * INPUT    : sys_config
 *            port_config
 *            pdu
 *            pdu_len
 * OUTPUT   : pdu
 *            pdu_len
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_UTY_ConstructMandatoryTLV(LLDP_OM_SysConfigEntry_T *sys_config,
                                           LLDP_OM_PortConfigEntry_T *port_config,
                                           UI8_T *pdu,
                                           UI16_T *pdu_len)
{
    UI8_T   *tlv;
    tlv = pdu + *pdu_len;
    /* 1. insert chassis id tlv */
    {
        tlv[0] = (UI8_T)LLDP_TYPE_CHASSIS_ID_TLV << 1 ;     /* tlv type = chassis_id */
        tlv[1] = 1 + 6;                                     /* tlv len = len(chassis_id_subtype) + len(chassis_id)*/
        tlv[2] = LLDP_TYPE_CHASSIS_ID_SUBTYPE_MAC_ADDR;     /* chassis_id_subtype = mac addr */
        SWCTRL_GetCpuMac(&tlv[3]);
        #if TX_DEBUG_PRINT
        {
            int i;
            printf("chassis_id_tlv:");
            for(i = 0; i < 9; i++ )
            {
                printf("%02X", tlv[i]);
            }
            puts("");
        }
        #endif
        *pdu_len += 9;
        tlv += 9;
    }
    /* 2. insert port id tlv */
    {
        tlv[0] = (UI8_T)LLDP_TYPE_PORT_ID_TLV   << 1;       /* tlv type = chassis_id */
        tlv[1] = 1 + 6;                                     /* tlv len = len(port_id_subtype) + len(port_id) */
        tlv[2] = LLDP_TYPE_PORT_ID_SUBTYPE_MAC_ADDR;        /* port_id_subtype = mac addr */
        SWCTRL_GetPortMac(port_config->lport_num, &tlv[3]);
        #if TX_DEBUG_PRINT
        {
            int i;
            printf("port_id_tlv:");
            for(i = 0; i < 9; i++ )
            {
                printf("%02X", tlv[i]);
            }
            puts("");
        }
        #endif
        *pdu_len += 9;
        tlv+= 9;
    }
    /* 3. insert Time to Live tlv */
    {
        UI16_T  tmp;
        UI32_T  ttl;
        tlv[0] = (UI8_T)LLDP_TYPE_TIME_TO_LIVE_TLV << 1;
        tlv[1] = 2;
        ttl = sys_config->msg_tx_interval * sys_config->msg_tx_hold_mul;
        if(ttl > 65535)
            ttl = 65535;
        tmp = L_STDLIB_Hton16((UI16_T)ttl);
        memcpy(&tlv[2], &tmp, 2);
        #if TX_DEBUG_PRINT
        {
            int i;
            printf("TTL_tlv:");
            for(i = 0; i < 4; i++)
                printf("%02X", tlv[i]);
            puts("");
        }
        #endif
        *pdu_len += 4;
        tlv += 4;
    }

}/* End of LLDP_UTY_ConstructMandatoryTLV */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructBasicTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct basic tlvs (port description tlv, system name tlv,
 *            system description tlv, system capabilities tlv)
 * INPUT    : sys_config
 *            port_config
 *            pdu           -- start address of LLDPDU packet
 *            pdu_len       -- total length of LLDPDU packet
 * OUTPUT   : pdu
 *            pdu_len
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_UTY_ConstructBasicTLV(LLDP_OM_SysConfigEntry_T *sys_config,
                                       LLDP_OM_PortConfigEntry_T *port_config,
                                       UI8_T *pdu,
                                       UI16_T *pdu_len)
{
    UI8_T   *tlv;
    UI8_T   basic_tlv;

    tlv = pdu + *pdu_len;
    basic_tlv = port_config->basic_tlvs_tx_flag;

    /* port description tlv*/
    if((basic_tlv & LLDP_TYPE_TX_PORT_DESC_TLV) != 0 && !port_config->med_device_exist)
    {
        IF_MGR_IfEntry_T    if_entry;
        UI32_T              port_desc_len = 0;

        memset(&if_entry, 0, sizeof(IF_MGR_IfEntry_T));
        tlv[0] = (UI8_T)LLDP_TYPE_PORT_DESC_TLV << 1;
        if_entry.if_index = port_config->lport_num;

        /* Get data from if_mgr */
        if(IF_PMGR_GetIfEntry(&if_entry))
        {
            port_desc_len = strlen((char *)if_entry.if_descr);

            tlv[1] = port_desc_len;
            memcpy(&tlv[2], if_entry.if_descr, port_desc_len);
        }
        else
        {
            tlv[1] = 0;
        }
        #if TX_DEBUG_PRINT
        {
            int i;
            printf("port_desc_tlv:");
            for(i = 0; i < 2 + tlv[1]; i++)
            {
                printf("%02X", tlv[i]);
            }
            puts("");
        }
        #endif
        *pdu_len += (2 + tlv[1]);
        tlv += (2 + tlv[1]);
    }
    /* system name tlv*/
    if((basic_tlv & LLDP_TYPE_TX_SYS_NAME_TLV) != 0 && !port_config->med_device_exist)
    {
        tlv[0] = (UI8_T)LLDP_TYPE_SYS_NAME_TLV << 1;

        /* Get data from mib2_mgr */
        if(MIB2_POM_GetSysName(&tlv[2]))
        {
            UI32_T  sys_name_len;
            sys_name_len = strlen((char *)&tlv[2]);
            tlv[1] = sys_name_len;
        }
        else
        {
            tlv[1] = 0;
        }
        #if TX_DEBUG_PRINT
        {
            int i;
            printf("sys_name_tlv:");
            for(i = 0; i < (2 + tlv[1]); i++)
            {
                printf("%02X", tlv[i]);
            }
            puts("");
        }
        #endif
        *pdu_len += (2 + tlv[1]);
        tlv += (2 + tlv[1]);
    }
    /* system desc tlv */
    if((basic_tlv & LLDP_TYPE_TX_SYS_DESC_TLV) != 0 && !port_config->med_device_exist)
    {
        tlv[0] = (UI8_T)LLDP_TYPE_SYS_DESC_TLV << 1;

        /* Get data from mib2_mgr */
        if(MIB2_POM_GetSysDescr(&tlv[2]))
        {
            UI32_T  sys_desc_len;
            sys_desc_len = strlen((char *)&tlv[2]);
            tlv[1] = sys_desc_len;
        }
        else
        {
            tlv[1] = 0;
        }
        #if TX_DEBUG_PRINT
        {
            int i;
            printf("sys_desc:");
            for(i = 0; i < (2 + tlv[1]); i++)
            {
                printf("%02X", tlv[i]);
            }
            puts("");
        }
        #endif
        *pdu_len += (2 + tlv[1]);
        tlv += (2 + tlv[1]);

    }

    /* system capabilities */
    if((basic_tlv & LLDP_TYPE_TX_SYS_CAP_TLV) != 0 || port_config->med_device_exist)
    {
        UI16_T  system_cap_supported = 0;
        UI16_T  system_cap_enabled = 0;
        int     ip_forwarding;

        UI16_T  tmp;
        tlv[0] = LLDP_TYPE_SYS_CAP_TLV << 1;
        tlv[1] = 4;

        system_cap_supported = SYS_ADPT_LLDP_SYSTEM_CAPABILITIES;

#if 0 /* Vai Wang, comment out */
        NETCFG_PMGR_GetIpForwarding(&ip_forwarding);
#endif
        ip_forwarding = VAL_ipForwarding_forwarding;
        system_cap_enabled = LLDP_TYPE_LOC_SYS_CAP_ENA_BRIDGE;

        if(ip_forwarding == VAL_ipForwarding_forwarding)
        {
            system_cap_enabled |= LLDP_TYPE_LOC_SYS_CAP_ENA_ROUTER;
        }

        system_cap_enabled &= system_cap_supported;

        tmp = L_STDLIB_Hton16(system_cap_supported);
        memcpy(&tlv[2], &tmp, 2);
        tmp = L_STDLIB_Hton16(system_cap_enabled);
        memcpy(&tlv[4], &tmp, 2);

        #if TX_DEBUG_PRINT
        {
            int i;
            printf("system cap:");
            for(i = 0; i < (2 + 4); i++)
            {
                printf("%02X",tlv[i]);
            }
            puts("");
        }
        #endif
        *pdu_len += (2 + tlv[1]);
        tlv += (2 + tlv[1]);
    }

}/* End of LLDP_UTY_ConstructBasicTLV */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructManAddrTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct management address tlvs
 * INPUT    : sys_config
 *            port_config
 *            pdu           -- start address of the LLDPDU packet
 *            pdu_len       -- total length of the LLDPDU packet
 * OUTPUT   : pdu
 *            pdu_len
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_UTY_ConstructManAddrTLV(LLDP_OM_SysConfigEntry_T *sys_config,
                                       LLDP_OM_PortConfigEntry_T *port_config,
                                       UI8_T *pdu,
                                       UI16_T *pdu_len)
{
    LLDP_OM_ConfigManAddrEntry_T    man_addr_config;
    NETCFG_TYPE_InetRifConfig_T     rif_config;
    UI32_T                          vid = 0, vlan_ifindex;
    UI32_T                          tmp;
    UI8_T                           *tlv;
    BOOL_T                          ip_exist;

    /* if the transmit flag is not bit on, return immediately*/
    if(!port_config->man_addr_transfer_flag || port_config->med_device_exist)
        return;

    tlv = pdu + *pdu_len;

    memset(&man_addr_config, 0, sizeof(LLDP_OM_ConfigManAddrEntry_T));

    ip_exist = FALSE;

    /* obtained all management addresses of vlans which are joined by the lport
     */
    while(VLAN_OM_GetNextVlanId(0, &vid))
    {
        VLAN_VID_CONVERTTO_IFINDEX(vid, vlan_ifindex);

        /* check whether the port is vlan's member */
        if(VLAN_OM_IsPortVlanMember(vlan_ifindex, port_config->lport_num))
        {
            if ((*pdu_len + 169) > 1498)
            {
                break;
            }

            /* if there is no ip associated with the vlan, then get next vlan */
            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.ifindex = vlan_ifindex;
            if(NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config) != NETCFG_TYPE_OK)
            {
                continue;
            }
            ip_exist = TRUE;
            tlv[0] = LLDP_TYPE_MAN_ADDR_TLV << 1;
            tlv[1] = 1 + 1 + rif_config.addr.addrlen + 1 + 4 + 1;   /* tlv_len*/
            tlv[2] = 1 + rif_config.addr.addrlen;                   /* man_addr_str_len*/
            tlv[3] = 1;                                             /* man addr subtype: IPv4 (1)*/

            memcpy(&tlv[4], rif_config.addr.addr, sizeof(UI32_T));  /* ip_addr */
            tlv[2 + tlv[2] + 1] = 2;                                /* if_subtype: if_index(2) */

            tmp = L_STDLIB_Hton32(vlan_ifindex);
            memcpy(&tlv[2 + tlv[2] + 2], &tmp, sizeof(UI32_T));     /* if_index*/
            tlv[2 + tlv[2] + 2 + 4] = 0;                            /* oid_length*/
            #if TX_DEBUG_PRINT
            {
                int i;
                printf("man_addr_tlv:");
                for(i = 0; i < (2 + tlv[1]); i++)
                {
                    printf("%02X",tlv[i]);
                }
                puts("");
            }
            #endif

            *pdu_len += (2 + tlv[1]);
            tlv += (2 + tlv[1]);
        }
    }

    /* if the management address does not exist,
     * return mac address.
     */
    if(!ip_exist)
    {
        tlv[0] = LLDP_TYPE_MAN_ADDR_TLV << 1;
        tlv[1] = 14;                /* tlv_len*/
        tlv[2] = 7;                 /* man_addr_str_len: mac addr len */
        tlv[3] = 6;                 /* man addr subtype: cananical form */
        SWCTRL_GetCpuMac(&tlv[4]);
        tlv[2 + tlv[2] + 1] = 1;    /* if_subtype: unknown(1)*/
        memset(&tlv[2 + tlv[2] + 2], 0, 4); /* if_number, must be 0 */
        #if TX_DEBUG_PRINT
        {
            int i;
            printf("man_addr_tlv:");
            for(i = 0; i < (2 + tlv[1]); i++)
            {
                printf("%02X",tlv[i]);
            }
            puts("");
        }
        #endif
        tlv[2 + tlv[2] + 2 + 4] = 0;    /* oid_len*/
        *pdu_len += (2 + tlv[1]);
        tlv += (2 + tlv[1]);

    }
}/* End of LLDP_UTY_ConstructManAddrTLV */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructEndOfPduTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct 'End of LLDPDU' TLV
 * INPUT    : pdu       -- start address of the LLDPDU packet
 *            pdu_len   -- the length of the LLDPDU packet
 * OUTPUT   : pdu
 *            pdu_len
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_UTY_ConstructEndOfPduTLV(UI8_T *pdu, UI16_T *pdu_len)
{
    UI8_T *tlv;

    tlv = pdu + *pdu_len;
    tlv[0] = LLDP_TYPE_END_OF_LLDPDU_TLV << 1;
    tlv[1] = 0;
    *pdu_len += 2;
    tlv += 2;
}/* End of LLDP_UTY_ConstructEndOfPduTLV */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructXdot1Tlvs
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct 802.1 extensions tlvs
 * INPUT    : port_config
 *            pdu           -- start address of the LLDPDU packet
 *            pdu_len       -- the length of the LLDPDU packet
 * OUTPUT   : pdu
 *            pdu_len
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_UTY_ConstructXdot1Tlvs(LLDP_OM_PortConfigEntry_T *port_config, UI8_T *pdu, UI16_T *pdu_len)
{
    if(port_config->med_device_exist)
        return;
    /* Port Vlan ID TLV*/
    if(port_config->xdot1_port_vlan_tx_enable == VAL_lldpXdot1ConfigPortVlanTxEnable_true &&
       ((*pdu_len + 8) <= 1498))
    {
        UI8_T   *tlv;
        UI16_T  tmp;
        VLAN_OM_Dot1qPortVlanEntry_T    port_vlan_entry;

        tlv = pdu + *pdu_len;
        tlv[0] = 127 << 1;
        tlv[1] = 6;
        memcpy(&tlv[2], LLDP_Org802dot1Oui, 3);
        tlv[5] = 1;

        /* Get dot1q PVID */
#if 0
        if(VLAN_PMGR_GetDot1qPortVlanEntry(port_config->lport_num, &port_vlan_entry))
#else
        if(VLAN_OM_GetDot1qPortVlanEntry(port_config->lport_num, &port_vlan_entry))
#endif
        {
            tmp = L_STDLIB_Hton16((UI16_T)port_vlan_entry.dot1q_pvid_index);
            memcpy(&tlv[6], &tmp, 2);
        }
        *pdu_len += (2 + tlv[1]);
    }

    /* Port and Protocol Vlan ID TLV */
    if(port_config->xdot1_proto_vlan_tx_enable == VAL_lldpXdot1ConfigProtoVlanTxEnable_true)
    {
        UI8_T   *tlv;
        UI16_T  tmp;
        UI16_T  ppvid;

        while ((*pdu_len + 9) <= 1498)
        {
            ppvid = 0;

            tlv = pdu + *pdu_len;
            tlv[0] = 127 << 1;
            tlv[1] = 7;
            memcpy(&tlv[2], LLDP_Org802dot1Oui, 3);
            tlv[5] = 2;

            /* not supported and not enabled */
            tlv[6] = 0x00;
            /* Std 802.1AB-2005, F.3.2 port and protocol VLAN ID (PPVID), page 109.
             * If the port is not capable of supporting port and protocol VLANs and/or
             * the port is not enabled with any port and protocol VLAN,
             * the PPVID number should be zero.
             */
            tmp = L_STDLIB_Hton16(ppvid);
            memcpy(&tlv[7], &tmp, 2);
            *pdu_len += (2 + tlv[1]);

            if (ppvid == 0)
            {
                break;
            }

        } /* end while */
    }

    /* VLAN Name TLV*/
    if(port_config->xdot1_vlan_name_tx_enable == VAL_lldpXdot1ConfigVlanNameTxEnable_true)
    {
        UI8_T  *tlv;
        UI32_T  vid = 0, vlan_ifindex;
        UI16_T  tmp;
        VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_entry;

        while(VLAN_OM_GetNextVlanId(0, &vid) && ((*pdu_len + 41) <= 1498))
        {
            VLAN_VID_CONVERTTO_IFINDEX(vid, vlan_ifindex);
            if(VLAN_OM_IsPortVlanMember(vlan_ifindex, port_config->lport_num))
            {
                tlv = pdu + *pdu_len;
                tlv[0] = 127 << 1;                          /* TLV_type */
                memcpy(&tlv[2], LLDP_Org802dot1Oui, 3);     /* 802.1 OUI */
                tlv[5] = 3;                                 /* 802.1 subtype */
                tmp = L_STDLIB_Hton16((UI16_T)vid);
                memcpy(&tlv[6], &tmp, sizeof(UI16_T));      /* VLAN ID */
                if(VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_entry))
                {
                    tlv[8] = strlen(vlan_entry.dot1q_vlan_static_name);    /* VLAN name length */
                    memcpy(&tlv[9], vlan_entry.dot1q_vlan_static_name, tlv[8]);    /* VLAN Name */
                }
                #if ORG_DEBUG
                {
                    int i;
                    printf("<LLDP_UTY_ConstructOrgSpecTLV>Construct VLAN name TLV !!!\n");
                    printf("<LLDP_UTY_ConstructOrgSpecTLV>tlv_type:%d\n", (tlv[0] >> 1));
                    printf("<LLDP_UTY_ConstructOrgSpecTLV>tlv_len:%d\n", tlv[1]);
                    printf("<LLDP_UTY_ConstructOrgSpecTLV>OUI:");
                    for(i = 0; i < 3; i++) printf("%02X", tlv[2+i]);
                    puts("");
                    printf("<LLDP_UTY_ConstructOrgSpecTLV>tlv_subtype:%d\n", tlv[5]);
                    printf("<LLDP_UTY_ConstructOrgSpecTLV>vid:");
                    for(i = 0; i < 2; i++) printf("%02X", tlv[6+i]);
                    puts("");
                    printf("<LLDP_UTY_ConstructOrgSpecTLV>vlan_name_len:%d\n", tlv[8]);
                    printf("<LLDP_UTY_ConstructOrgSpecTLV>vlan_name:");
                    for(i = 0; i < tlv[8]; i++) printf("%c", tlv[9+i]);
                    puts("");
                }
                #endif
                tlv[1] = 3 + 1 + 2 + 1 + tlv[8];                            /* IEEE 802.1AB p109 */
                *pdu_len += (2 + tlv[1]);
            }
        }
    }

    /* Protocol Identity TLV */
    if(port_config->xdot1_protocol_tx_enable == VAL_lldpXdot1ConfigProtocolTxEnable_true &&
       ((*pdu_len + 9) <= 1498))
    {
        int i = 0;
        UI8_T   *tlv;

        LLDP_UTY_ProtocolIdent_T    protocol[] =
        {
            #if 0
            {8, "\x00\x26\x42\x42\x03\x00\x00\x00"},  /* STP */
            {3, "\x88\x8E\x01"},                      /* 802.1x */
            #endif
            {2, "\x88\xCC"},                          /* LLDP */
            {0, "\x00"}
        };

        while(protocol[i].protocol_ident_len != 0)
        {
            tlv= pdu + *pdu_len;
            tlv[0] = 127 << 1;                          /* tlv type */
            memcpy(&tlv[2], LLDP_Org802dot1Oui, 3);     /* 802.1 subtype */
            tlv[5] = 4;                                 /* protocol ident */
            tlv[6] = protocol[i].protocol_ident_len;
            memcpy(&tlv[7], protocol[i].protocol_ident, tlv[6]);
            tlv[1] = 3 + 1 + 1 + tlv[6];
            *pdu_len += (2 + tlv[1]);
            i++;
        }

    }

#if (SYS_CPNT_CN == TRUE)
    /* Congestion Notification TLV */
    if((port_config->xdot1_cn_tx_enable == TRUE) && ((*pdu_len + 8) <= 1498))
    {
        UI8_T   *tlv;
        UI32_T  i;
        BOOL_T  error, ready;
        UI8_T   cnpv_indicators, ready_indicators;

        cnpv_indicators = ready_indicators = 0;
        error = FALSE;
        for (i = 0; i < 8; i++)
        {
            if (CN_OM_IsCnpv(i) == TRUE)
            {
                cnpv_indicators |= 1 << i;
            }

            if (CN_OM_GetTxReady(i, port_config->lport_num, &ready)
                != CN_TYPE_RETURN_OK)
            {
                error = TRUE;
                break;
            }
            if (ready == TRUE)
            {
                ready_indicators |= 1 << i;
            }
        }

        if ((error == FALSE) && (cnpv_indicators != 0))
        {
            tlv = pdu + *pdu_len;
            tlv[0] = 127 << 1;                      /* organization specific */
            tlv[1] = 6;                             /* TLV length */
            memcpy(&tlv[2], LLDP_Org802dot1Oui, 3); /* IEEE 802.1 OUI */
            tlv[5] = 8;                             /* congestion notification */
            tlv[6] = cnpv_indicators;
            tlv[7] = ready_indicators;
            *pdu_len += (2 + tlv[1]);
        }
    }
#endif /* #if (SYS_CPNT_CN == TRUE) */
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructXdcbxTlvs
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct DCBX tlvs
 * INPUT    : port_config
 *            pdu           -- start address of the LLDPDU packet
 *            pdu_len       -- the length of the LLDPDU packet
 * OUTPUT   : pdu
 *            pdu_len
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_UTY_ConstructXdcbxTlvs(LLDP_OM_PortConfigEntry_T *port_config, UI8_T *pdu, UI16_T *pdu_len)
{
#if(SYS_CPNT_ETS == TRUE)
    BOOL_T ets_willing = FALSE;
    UI8_T max_nbr_of_tc = 0;
    ETS_TYPE_PortEntry_T  entry;
    UI8_T   index = 0;
    ETS_TYPE_MODE_T ets_mode = 0;
    UI32_T dcbx_mode = 0;

    DCBX_PMGR_GetPortMode(port_config->lport_num, &dcbx_mode);
    ETS_PMGR_GetMode(port_config->lport_num, &ets_mode);
    ETS_PMGR_GetMaxNumberOfTC(&max_nbr_of_tc);
    ETS_PMGR_GetPortEntry(port_config->lport_num, &entry, ETS_TYPE_DB_OPER);

    /* ETS Config TLV*/
    if((port_config->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_ETS_CON_TLV_TX) && ((*pdu_len + 27) <= 1498))
    {
        UI8_T   *tlv;

        tlv = pdu + *pdu_len;
        memset(tlv, 0, 27);
        tlv[0] = 127 << 1;
        tlv[1] = 25;
        memcpy(&tlv[2], LLDP_Org802dot1Oui, 3);
        tlv[5] = 9;/* 802.1 subtype */
        if(((dcbx_mode == DCBX_TYPE_PORT_MODE_CFGSRC) || (dcbx_mode == DCBX_TYPE_PORT_MODE_AUTOUP)) &&
            (ets_mode == ETS_TYPE_MODE_AUTO))
        {
            tlv[6] |= (1<<7);/* willing bit,decide by DCBX mode and ETS mode */
        }
        else
        {
            tlv[6] &= ~(1<<7);
        }

        tlv[6] &= ~(1<<6);/* CBS */
        tlv[6] &= ~(7<<3);/* Reserved */
        tlv[6] &= ~(7<<0);/* init Max TCs */
        tlv[6] |= (0x7 & max_nbr_of_tc);
        for(index = 0; index < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; index++)
        {
            tlv[7 + (index + 2)/2 - 1] |= ((0xf & entry.priority_assign[index]) <<(((index + 1)%2)*4));/* Priority Assignment Table */
        }
        for(index = 0; index < SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; index++)
        {
            tlv[11 + index] |= (0xff & entry.tc_weight[index]);/* TC Bandwidth Table */
        }
        for(index = 0; index < SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; index++)
        {
            tlv[19 + index] |= (0xff & entry.tsa[index]);/* TSA Assignment Table */
        }

        *pdu_len += (2 + tlv[1]);

        ets_willing = (tlv[6] & (1<<7))?TRUE:FALSE;
    }

    /* ETS recommendation TLV */
    if((port_config->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_ETS_RECOM_TLV_TX) && ((*pdu_len + 27) <= 1498) && (ets_willing == FALSE))
    {
        UI8_T   *tlv;

        tlv = pdu + *pdu_len;
        memset(tlv, 0, 27);
        tlv[0] = 127 << 1;
        tlv[1] = 25;
        memcpy(&tlv[2], LLDP_Org802dot1Oui, 3);
        tlv[5] = 10;/* 802.1 subtype */
        tlv[6] = 0;/* Reserved */
        for(index = 0; index < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; index++)
        {
            tlv[7 + (index + 2)/2 - 1] |= ((0xf & entry.priority_assign[index]) <<(((index + 1)%2)*4));/* Priority Assignment Table */
        }
        for(index = 0; index < SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; index++)
        {
            tlv[11 + index] |= (0xff & entry.tc_weight[index]);/* TC Bandwidth Table */
        }
        for(index = 0; index < SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; index++)
        {
            tlv[19 + index] |= (0xff & entry.tsa[index]);/* TSA Assignment Table */
        }

        *pdu_len += (2 + tlv[1]);
    }
#endif /* #if(SYS_CPNT_ETS == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
    /* PFC config TLV*/
    if((port_config->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_PFC_CON_TLV_TX) && ((*pdu_len + 8) <= 1498))
    {
        UI8_T   *tlv;
        UI32_T   pfc_enable = 0;
        UI32_T dcbx_mode = 0;
        UI32_T pfc_mode = 0;
        UI32_T pfc_cap = 0;

        tlv = pdu + *pdu_len;
        memset(tlv, 0, 8);
        tlv[0] = 127 << 1;
        tlv[1] = 6;
        memcpy(&tlv[2], LLDP_Org802dot1Oui, 3);
        tlv[5] = 11;/* 802.1 subtype */
        DCBX_PMGR_GetPortMode(port_config->lport_num, &dcbx_mode);
        PFC_PMGR_GetDataByField(port_config->lport_num, PFC_TYPE_FLDE_PORT_MODE_ADM, &pfc_mode);

        if(((dcbx_mode == DCBX_TYPE_PORT_MODE_CFGSRC) || (dcbx_mode == DCBX_TYPE_PORT_MODE_AUTOUP)) &&
            (pfc_mode == PFC_TYPE_PMODE_AUTO))
        {
            tlv[6] |= (1<<7);/* willing bit,decide by DCBX mode and PFC mode */
        }
        else
        {
            tlv[6] &= ~(1<<7);
        }
        tlv[6] |= (1<<6);/* MBC,fix to 1,not capable */
        tlv[6] &= ~(3<<4);/* Reserved */
        tlv[6] &= ~(15<<0);/* init PFC cap field */
        if(PFC_PMGR_GetDataByField(0, PFC_TYPE_FLDE_GLOBAL_MAX_TC_CAP, &pfc_cap))
        {
            tlv[6] |= (0xf & pfc_cap);/* PFC cap */
        }

        tlv[7] = 0;/* init PFC Enable field, get PFC oper value */
        if(PFC_PMGR_GetDataByField(port_config->lport_num, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &pfc_enable))
        {
            tlv[7] |= (0xff & pfc_enable);/* PFC Enable */
        }

        *pdu_len += (2 + tlv[1]);
    }
#endif /* #if (SYS_CPNT_PFC == TRUE) */

#if 0
    /* Application priority TLV,not support now */
    if(port_config->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_APP_PRI_TLV_TX)
    {
    }
#endif
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructXdot3Tlvs
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct 802.3 extensions tlvs
 * INPUT    : port_config
 *            pdu       -- start address of the LLDPDU packet
 *            pdu_len   -- the length of the LLDPDU packet
 * OUTPUT   : pdu
 *            pdu_len
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_UTY_ConstructXdot3Tlvs(LLDP_OM_PortConfigEntry_T *port_config, UI8_T *pdu, UI16_T *pdu_len)
{
    UI8_T   *tlv;

    /* MAC/PHY configurations/status TLV*/
    if((*pdu_len + 11 <= 1498) &&
       ((port_config->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_MAC_PHY_TLV_TX) != 0 ||
        port_config->med_device_exist))
    {

#if (SYS_CPNT_MAU_MIB == TRUE)
        BOOL_T  flag = TRUE;
        SWCTRL_IfMauEntry_T         if_mau_entry;
        SWCTRL_IfMauAutoNegEntry_T  if_mau_auto_neg_entry;

        /* keys are ifMauIfIndex and ifMauIndex*/
        if_mau_entry.ifMauIfIndex = port_config->lport_num;
        if_mau_entry.ifMauIndex = 1;
        if_mau_auto_neg_entry.ifMauIfIndex = port_config->lport_num;
        if_mau_auto_neg_entry.ifMauIndex = 1;
        /*EPR:ES3628BT-FLF-ZZ-00460
         *Problem: LLDP: DUT can't send dot3-tlv mac tlv on trunk port.
         *Root Cause:The code did not care the interface is trunk and when
         *           first port add to trunk,did not regiester the callback
         *           for LLDP to handle the case!
         *Solution: If the interface is trunk,use the first trunk member
         *          dot3 config to send!Add the first port add to trunk callback,and
         *          port add to trunk callback regiester function!
         *Modify file: lldp_uty.c,lldp_mgr.c,sys_callback_mgr.c,gvrp_group.c
         *Fixed by:DanXie
         */
        if(SWCTRL_LogicalPortIsTrunkPort(port_config->lport_num))
        {
            UI32_T  i;
            SWCTRL_TrunkPortExtInfo_T    trunk_ext_p_info;
            SWCTRL_GetTrunkPortExtInfo (port_config->lport_num, &trunk_ext_p_info);

            for(i = 0; i < trunk_ext_p_info.member_number; i++)
            {
                if_mau_entry.ifMauIfIndex = (UI32_T) trunk_ext_p_info.member_list[i].port;
                if_mau_auto_neg_entry.ifMauIfIndex = (UI32_T) trunk_ext_p_info.member_list[i].port;
                break;
            } /* End of for () */
        }

        if(!SWCTRL_GetIfMauEntry(&if_mau_entry))
        {
            #if ORG_DEBUG
            puts("<LLDP_UTY_ConstructXdot3Tlvs>SWCTRL_GetIfMauEntry error");
            #endif
            flag = FALSE;

        }
        if(!SWCTRL_GetIfMauAutoNegEntry(&if_mau_auto_neg_entry))
        {
            #if ORG_DEBUG
            puts("<LLDP_UTY_ConstructXdot3Tlvs>SWCTRL_GetIfMauAutoEntry error");
            #endif
            flag = FALSE;
        }
        if(flag)
        {
            UI16_T  tmp2;
            tlv = pdu + *pdu_len;
            tlv[0] = 127 << 1;                      /* tlv_type: orgnaizational tlv */
            tlv[1] = 9;                             /* tlv_len: 9 */
            memcpy(&tlv[2], LLDP_Org802dot3Oui, 3); /* 802.3 OUI */
            tlv[5] = 1;                             /* 802.3 subtype */
            tlv[6] |= (if_mau_entry.ifMauAutoNegSupported == 1)? 1 : 0 ;
            tlv[6] |= (if_mau_auto_neg_entry.ifMauAutoNegAdminStatus == 1)? (1 << 1) : 0;

            #if ORG_DEBUG
            printf("<LLDP_UTY_ConstructOrgSpecTLV>mau_auto_neg_cap_bits:%X", if_mau_auto_neg_entry.ifMauAutoNegCapAdvertisedBits);
            #endif

            tmp2 = (if_mau_auto_neg_entry.ifMauAutoNegCapAdvertisedBits >> 16);
            tmp2 = L_STDLIB_Hton16(tmp2);
            memcpy(&tlv[7], &tmp2, 2);

            #if 0
            tmp = L_STDLIB_Hton16(((UI16_T *)&if_mau_auto_neg_entry.ifMauAutoNegCapAdvertisedBits)[0]);
            memcpy(&tlv[7], &tmp, 2);
            #endif
            tmp2 = if_mau_entry.ifMauType;
            tmp2 = L_STDLIB_Hton16(tmp2);
            memcpy(&tlv[9], &tmp2, 2);
            *pdu_len += (2 + tlv[1]);

        }
#else
        Port_Info_T      port_info;

        memset(&port_info, 0, sizeof(Port_Info_T));
        if(SWCTRL_GetPortInfo(port_config->lport_num, &port_info))
        {
            UI16_T  tmp2;

            tlv = pdu + *pdu_len;
            tlv[0] = 127 << 1;                      /* tlv_type: orgnaizational tlv */
            tlv[1] = 9;                             /* tlv_len: 9 */
            memcpy(&tlv[2], LLDP_Org802dot3Oui, 3); /* 802.3 OUI */
            tlv[5] = 1;                             /* 802.3 subtype */
            tlv[6] = 1;                             /* Auto neg always support */
            tlv[6] |= (port_info.autoneg_state == VAL_portAutonegotiation_enabled)? (1 << 1) : 0;
            tmp2 = 0; /* not support MAU, use 0 for auto-negotiation advertised capability */
            memcpy(&tlv[7], &tmp2, 2);
            tmp2 = 0; /* not support MAU, use 0 for MAU type */
            memcpy(&tlv[9], &tmp2, 2);

            *pdu_len += (2 + tlv[1]);
        }
#endif
    }


    /* Power Via MDI */
#if (SYS_CPNT_POE == TRUE)
    if(
    #if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
          (*pdu_len + 14 <= 1498) &&
    #else
          (*pdu_len + 9 <= 1498) &&
    #endif
        ((port_config->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_POWER_VIA_MDI_TX) != 0 &&
        !port_config->med_device_exist))
    {
        /* transform the lport to the unit/port type */
        if(!SWCTRL_LogicalPortIsTrunkPort(port_config->lport_num))
        {
            UI32_T  unit, port, trunk_id;
            UI32_T  tmp;
            SWCTRL_LogicalPortToUserPort(port_config->lport_num, &unit, &port, &trunk_id);
    #if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
            POE_TYPE_Dot3atPowerInfo_T dot3at_power_info;
    #endif

            if (STKTPLG_OM_IsPoeDevice(unit) == TRUE)
            {
                tlv = pdu + *pdu_len;
                tlv[0] = 127 << 1;      /* tlv_type: orgnaizational tlv  */
                tlv[1] = 7;             /* tlv_len: 7*/
                memcpy(&tlv[2], LLDP_Org802dot3Oui, 3); /* 802.3 OUI */
                tlv[5] = 2;             /* 802.3 subtype */

                POE_POM_GetMainPseOperStatus(unit, &tmp);
                tlv[6] = 0x01 + 0x02 + ((tmp == 1) ? 0x04 : 0x00);
                POE_POM_GetPsePortPowerPairsCtrlAbility(unit, port, &tmp);
                tlv[6] += (tmp == 1) ? 0x08 : 0x00;
                POE_POM_GetPsePortPowerPairs(unit, port, &tmp);
                tlv[7] = (UI8_T)tmp;
                POE_POM_GetPsePortPowerClassifications(unit, port, &tmp);
                tlv[8] = (UI8_T)tmp;

    #if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                if (POE_POM_GetPortDot3atPowerInfo(unit, port, &dot3at_power_info) == TRUE)
                {
                    UI16_T tmp2;

                    tlv[1] = 12;             /* tlv_len: 12 */
                    tlv[9] = (dot3at_power_info.power_type |
                              dot3at_power_info.power_source |
                              dot3at_power_info.power_priority);
                    tmp2 = L_STDLIB_Hton16(dot3at_power_info.pd_requested_power);
                    memcpy(&tlv[10], &tmp2, 2);
                    tmp2 = L_STDLIB_Hton16(dot3at_power_info.pse_allocated_power);
                    memcpy(&tlv[12], &tmp2, 2);
                }
    #endif /* #if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2) */

                *pdu_len += (2 + tlv[1]);
            }
        }
    }
#endif

    /* Link Aggregation */
    if((*pdu_len + 11 <= 1498) &&
       ((port_config->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_LINK_AGG_TX) != 0 &&
        !port_config->med_device_exist))
    {
        tlv = pdu + *pdu_len;
        tlv[0] = 127 << 1;      /* tlv_type: orgnaizational tlv  */
        tlv[1] = 9;             /* tlv_len: 9 */
        memcpy(&tlv[2], LLDP_Org802dot3Oui, 3); /* 802.3 OUI */
        tlv[5] = 3;

        if(SWCTRL_LogicalPortIsTrunkPort(port_config->lport_num))
        {
            UI32_T  tmp;
            tlv[6] = 3;         /* aggregation capability=yes & status=yes */
            tmp = L_STDLIB_Hton32(port_config->lport_num);      /* aggregated id */
            memcpy(&tlv[7], &tmp, sizeof(UI32_T));
        }
        else
        {
            tlv[6] = 1;         /* aggregation capability=yes */
        }
        *pdu_len += (2 + tlv[1]);
    }

    /* Max frame size */
    if( (*pdu_len + 8 <= 1498) &&
        ((port_config->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_MAX_FRAME_SIZE_TLV) != 0 &&
        !port_config->med_device_exist))
    {
        UI32_T  untagged_size, tagged_size;
        UI16_T  tmp;

        tlv = pdu + *pdu_len;
        tlv[0] = 127 << 1;      /* tlv_type: orgnaizational tlv  */
        tlv[1] = 6;             /* tlv_len: 9 */
        memcpy(&tlv[2], LLDP_Org802dot3Oui, 3); /* 802.3 OUI */
        tlv[5] = 4;
        if (SWCTRL_GetPortMaxFrameSize(port_config->lport_num, &untagged_size,
                &tagged_size) == FALSE)
        {
            return;
        }
#if (SYS_CPNT_VLAN == TRUE)
        tmp = (UI16_T)tagged_size;
#else
        tmp = (UI16_T)untagged_size;
#endif
        tmp = L_STDLIB_Hton16(tmp);
        memcpy(&tlv[6], &tmp, sizeof(UI16_T));
        *pdu_len += (2 + tlv[1]);
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructMedTlvs
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct 802.3 extensions tlvs
 * INPUT    : port_config
 *            pdu       -- start address of the LLDPDU packet
 *            pdu_len   -- the length of the LLDPDU packet
 * OUTPUT   : pdu
 *            pdu_len
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_UTY_ConstructMedTlvs(LLDP_OM_PortConfigEntry_T *port_config, UI8_T *pdu, UI16_T *pdu_len)
{
    UI8_T   *tlv;

    if (!LLDP_BACKDOOR_GetDebugFlag(LLDP_BACKDOOR_DEBUG_MED) &&
        !port_config->med_device_exist)
        return;

    /* LLDP-MED capability */
    if((*pdu_len + 9 <= 1498) && (port_config->lldp_med_tx_enable & LLDP_TYPE_MED_CAP_TX))
    {
        UI16_T  cap;

        tlv = pdu + *pdu_len;

        tlv[0] = 127 << 1;          /* tlv type: org tlv */
        tlv[1] = 7;                 /* tlv len */
        memcpy(&tlv[2], LLDP_OrgTiaOui, 3); /* TIA OUI */
        tlv[5] = 1;                 /* MED subtype: MED cap TLV */

        cap = L_STDLIB_Hton16(SYS_ADPT_LLDP_MED_CAPABILITY);
        memcpy(&tlv[6], &cap, 2);

        tlv[8] = 4;                 /* MED device type: network connectivity*/

        *pdu_len += (2 + tlv[1]);
    }

    /* LLDP-MED network policy */
    if((*pdu_len + 10 <= 1498) && (port_config->lldp_med_tx_enable & LLDP_TYPE_MED_NETWORK_POLICY_TX))
    {
#if (SYS_CPNT_ADD == TRUE)
        /* currently only voice application is supported, if voice vlan is not
         * supported, there is no need to send this TLV.
         */
        I32_T   vid = 0;
        UI32_T  vlan_ifindex;
        UI32_T  mode = 0;
        UI16_T  tmp;
        UI8_T   cos;
        UI32_T  dscp;

        if ((ADD_OM_GetVoiceVlanId(&vid) == TRUE) &&
            (vid != VAL_voiceVlanEnabledId_disabled) &&
            (ADD_OM_GetVoiceVlanPortMode(port_config->lport_num, &mode) == TRUE) &&
            (mode != VAL_voiceVlanPortMode_none))
        {
            tlv = pdu + *pdu_len;
            tlv[0] = 127 << 1; /* TLV type: 127 */
            tlv[1] = 8; /* TLV length: 8 */
            memcpy(&tlv[2], LLDP_OrgTiaOui, 3); /* TIA OUI */
            tlv[5] = 2; /* subtype: 2(Network Policy) */
            tlv[6] = 1; /* Application type: voice */

            tmp = (UI16_T)vid;

            /* follow static configuration first ; otherwise, default tagged */
            VLAN_VID_CONVERTTO_IFINDEX(vid, vlan_ifindex);
            if (VLAN_OM_IsVlanUntagPortListMember(vlan_ifindex, port_config->lport_num) == FALSE)
            {
                tmp |= 0x2000; /* U:0, T:1, X:0 */
            }

            tmp = tmp << 1;

            if (ADD_OM_GetVoiceVlanPortPriority(port_config->lport_num, &cos) == FALSE)
            {
                cos = 0;
            }

            /* get the highest bit of 3bit-L2-priority and placed it to the
             * lowest bit of previous byte
             */
            if ((cos & 4) != 0)
                tmp += 1;

            tmp = L_STDLIB_Hton16(tmp);
            memcpy(&tlv[7], &tmp, 2);

            /* get the remaining two bits of 3bit-L2-priority and placed it to
             * the last byte of the tlv
             */
            tlv[9] = (cos << 6);

            /* not support modification yet, use default */
            dscp = 0;
            tlv[9] += (UI8_T)dscp;

            *pdu_len += (2 + tlv[1]);
        }
#endif /* #if (SYS_CPNT_ADD == TRUE) */
    }

    /* LLDP-MED location ident tlv */
    if(port_config->lldp_med_tx_enable & LLDP_TYPE_MED_LOCATION_IDENT_TX)
    {
        if (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_coordinateBased))
        {
            /* wakka TDDO: need implement */
        }
        if (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress))
        {
            LLDP_OM_XMedLocationCivicAddrCaTlv_T *ca_entry;
            UI8_T   *ca;

            tlv = pdu + *pdu_len;
            tlv[0] = 127 << 1;                  /* tlv type: org tlv */
            memcpy(&tlv[2], LLDP_OrgTiaOui, 3); /* TIA OUI */
            tlv[5] = 3;                         /* MED subtype: Location ID */
            tlv[6] = 2;                         /* Location data format */
            tlv[7] = 3;                         /* LCI length */
            tlv[8] = port_config->lldp_med_location.civic_addr.what;      /* what */
            memcpy(&tlv[9], port_config->lldp_med_location.civic_addr.country_code, 2); /* country code */

            ca = &tlv[11];

            /* if there is any ca entry in ca_list */
            if (port_config->lldp_med_location.civic_addr.ca_list.nbr_of_element != 0 &&
                L_SORT_LST_Get_1st(&port_config->lldp_med_location.civic_addr.ca_list, &ca_entry))
            {
                /* insert into packet */
                do
                {
                    if((tlv[7] + ca_entry->ca_length) > 255)
                        break;
                    ca[0] = ca_entry->ca_type;
                    ca[1] = ca_entry->ca_length;
                    memcpy(&ca[2], ca_entry->ca_value, ca[1]);
                    tlv[7] += (2 + ca[1]);
                    ca += (2 + ca[1]);
                }while(L_SORT_LST_Get_Next(&port_config->lldp_med_location.civic_addr.ca_list, &ca_entry));
            }

            tlv[1] = 5 + 1 + tlv[7];                /* tlv length */

            if (*pdu_len + (2 + tlv[1]) <= 1498)
            *pdu_len += (2 + tlv[1]);
        }
        if (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_elin))
        {
            tlv = pdu + *pdu_len;
            tlv[0] = 127 << 1;                  /* tlv type: org tlv */
            memcpy(&tlv[2], LLDP_OrgTiaOui, 3); /* TIA OUI */
            tlv[5] = 3;                         /* MED subtype: Location ID */
            tlv[6] = 3;                         /* Location data format */
            memcpy(&tlv[7],
            port_config->lldp_med_location.elin_addr.elin,
            port_config->lldp_med_location.elin_addr.elin_len);
            tlv[1] = 5 + port_config->lldp_med_location.elin_addr.elin_len;
            *pdu_len += ( 2 + tlv[1]);
        }
    }
#if (SYS_CPNT_POE == TRUE)
    /* LLDP-MED Extended power via mdi */
    /* extended pse*/
    if((*pdu_len + 9 <= 1498) && (port_config->lldp_med_tx_enable & LLDP_TYPE_MED_EXT_PSE_TX))
    {
        if(!SWCTRL_LogicalPortIsTrunkPort(port_config->lport_num))
        {
            UI32_T  unit, port, trunk_id;
            UI16_T  tmp;
            SWCTRL_LogicalPortToUserPort(port_config->lport_num, &unit, &port, &trunk_id);

            tlv = pdu + *pdu_len;
            tlv[0] = 127 << 1;          /* tlv type: org */
            tlv[1] = 7;                 /* tlv len */
            memcpy(&tlv[2], LLDP_OrgTiaOui, 3); /* TIA OUI */
            tlv[5] = 4;                 /* MED subtype */

            {
                POE_OM_PsePort_T   entry;

                POE_POM_GetPsePortEntry(unit, port, &entry);
                tlv[6] = 0x10 + (((UI8_T)entry.pse_port_power_priority) & 0x0F);   /* power type = PSE, power source = primary, power priority */
                tmp = L_STDLIB_Hton16((UI16_T)entry.pse_port_power_max_allocation);
                memcpy(&tlv[7], &tmp, 2);   /* power value */
            }

            *pdu_len += (2 + tlv[1]);
        }
    }
#endif

    #if 0
    /* extended pd*/
    if(port_config->lldp_med_tx_enable & LLDP_TYPE_MED_EXT_PD_TX)
    {
        UI16_T tmp;

        tlv = pdu + *pdu_len;
        tlv[0] = 127 << 1;          /* tlv type: org */
        tlv[1] = 7;                 /* tlv len */
        memcpy(&tlv[2], LLDP_OrgTiaOui, 3); /* TIA OUI */
        tlv[5] = 4;                 /* MED subtype */
        tlv[6] = 0x61;
        tmp = L_STDLIB_Hton16(1023);
        memcpy(&tlv[7], &tmp, 2);

        *pdu_len += (2 + tlv[1]);
    }
    #endif

    /* LLDP-MED inventory tlvs */
    if(port_config->lldp_med_tx_enable & LLDP_TYPE_MED_INVENTORY_TX)
    {
        UI32_T  tmp_len;
        int                     i;
        STKTPLG_OM_EntPhysicalEntry_T  *ent_physical_entry_for_container_p = NULL;
        STKTPLG_OM_EntPhysicalEntry_T  *ent_physical_entry_for_module_p = NULL;
        STKTPLG_OM_EntPhysicalEntry_T  ent_physical_entry[2];

        memset(&ent_physical_entry, 0, sizeof(ent_physical_entry));

        for (i = 0;
            i < sizeof(ent_physical_entry)/sizeof(*ent_physical_entry) &&
                STKTPLG_OM_GetNextEntPhysicalEntry(&ent_physical_entry[i]); )
        {
            if (ent_physical_entry_for_container_p == NULL &&
                ent_physical_entry[i].ent_physical_class == VAL_entPhysicalClass_container)
            {
                ent_physical_entry_for_container_p = &ent_physical_entry[i++];
                continue;
            }

            if (ent_physical_entry_for_module_p == NULL &&
                ent_physical_entry[i].ent_physical_class == VAL_entPhysicalClass_module)
            {
                ent_physical_entry_for_module_p = &ent_physical_entry[i++];
                continue;
            }
        }

        /* Hardware revision */
        if(*pdu_len + 38 <= 1498 && ent_physical_entry_for_container_p)
        {
            if((tmp_len = strlen((char *)ent_physical_entry_for_container_p->ent_physical_hardware_rev)) > LLDP_TYPE_MAX_HARDWARE_REV_LEN)
                tmp_len = LLDP_TYPE_MAX_HARDWARE_REV_LEN;
            if(tmp_len != 0)
            {
                tlv = pdu + *pdu_len;
                tlv[0] = 127 << 1;
                tlv[1] = 4 + tmp_len;
                memcpy(&tlv[2], LLDP_OrgTiaOui, 3);
                tlv[5] = 5;
                memcpy(&tlv[6], ent_physical_entry_for_container_p->ent_physical_hardware_rev, tmp_len);
                *pdu_len += (2 + tlv[1]);
            }
        }

        /* Firmware revision */
        if(*pdu_len + 38 <= 1498 && ent_physical_entry_for_module_p)
        {
            if((tmp_len = strlen((char *)ent_physical_entry_for_module_p->ent_physical_firmware_rev)) > LLDP_TYPE_MAX_FIRMWARE_REV_LEN)
                tmp_len = LLDP_TYPE_MAX_FIRMWARE_REV_LEN;
            if(tmp_len != 0)
            {
                tlv = pdu + *pdu_len;
                tlv[0] = 127 << 1;
                tlv[1] = 4 + tmp_len;
                memcpy(&tlv[2], LLDP_OrgTiaOui, 3);
                tlv[5] = 6;
                memcpy(&tlv[6], ent_physical_entry_for_module_p->ent_physical_firmware_rev, tmp_len);
                *pdu_len += (2 + tlv[1]);
            }
        }

        /* Software revision */
        if(*pdu_len + 38 <= 1498 && ent_physical_entry_for_module_p)
        {
            if((tmp_len = strlen((char *)ent_physical_entry_for_module_p->ent_physical_software_rev)) > LLDP_TYPE_MAX_SOFTWARE_REV_LEN)
                tmp_len = LLDP_TYPE_MAX_SOFTWARE_REV_LEN;
            if(tmp_len != 0)
            {
                tlv = pdu + *pdu_len;
                tlv[0] = 127 << 1;
                tlv[1] = 4 + tmp_len;
                memcpy(&tlv[2], LLDP_OrgTiaOui, 3);
                tlv[5] = 7;
                memcpy(&tlv[6], ent_physical_entry_for_module_p->ent_physical_software_rev, tmp_len);
                *pdu_len += (2 + tlv[1]);
            }
        }

        /* Serial num */
        if(*pdu_len + 38 <= 1498 && ent_physical_entry_for_container_p)
        {
            if((tmp_len = strlen((char *)ent_physical_entry_for_container_p->ent_physical_entry_rw.ent_physical_serial_num)) > LLDP_TYPE_MAX_SERIAL_NUM_LEN)
                tmp_len = LLDP_TYPE_MAX_SERIAL_NUM_LEN;
            if(tmp_len != 0)
            {
                tlv = pdu + *pdu_len;
                tlv[0] = 127 << 1;
                tlv[1] = 4 + tmp_len;
                memcpy(&tlv[2], LLDP_OrgTiaOui, 3);
                tlv[5] = 8;
                memcpy(&tlv[6], ent_physical_entry_for_container_p->ent_physical_entry_rw.ent_physical_serial_num, tmp_len);
                *pdu_len += (2 + tlv[1]);
            }
        }

        /* Manufacturer name */
        if(*pdu_len + 38 <= 1498 && ent_physical_entry_for_container_p)
        {
            if((tmp_len = strlen((char *)ent_physical_entry_for_container_p->ent_physical_mfg_name)) > LLDP_TYPE_MAX_MFG_NAME_LEN)
                tmp_len = LLDP_TYPE_MAX_MFG_NAME_LEN;
            if(tmp_len != 0)
            {
                tlv = pdu + *pdu_len;
                tlv[0] = 127 << 1;
                tlv[1] = 4 + tmp_len;
                memcpy(&tlv[2], LLDP_OrgTiaOui, 3);
                tlv[5] = 9;
                memcpy(&tlv[6], ent_physical_entry_for_container_p->ent_physical_mfg_name, tmp_len);
                *pdu_len += (2 + tlv[1]);
            }
        }

        /* Model name */
        if(*pdu_len + 38 <= 1498 && ent_physical_entry_for_container_p)
        {
            if((tmp_len = strlen((char *)ent_physical_entry_for_container_p->ent_physical_model_name)) > LLDP_TYPE_MAX_MODEL_NAME_LEN)
                tmp_len = LLDP_TYPE_MAX_MODEL_NAME_LEN;
            if(tmp_len != 0)
            {
                tlv = pdu + *pdu_len;
                tlv[0] = 127 << 1;
                tlv[1] = 4 + tmp_len;
                memcpy(&tlv[2], LLDP_OrgTiaOui, 3);
                tlv[5] = 10;
                memcpy(&tlv[6], ent_physical_entry_for_container_p->ent_physical_model_name, tmp_len);
                *pdu_len += (2 + tlv[1]);
            }
        }

        /* Asset ID */
        if(*pdu_len + 38 <= 1498 && ent_physical_entry_for_container_p)
        {
            if((tmp_len = strlen((char *)ent_physical_entry_for_container_p->ent_physical_entry_rw.ent_physical_asset_id)) > LLDP_TYPE_MAX_ASSET_ID_LEN)
                tmp_len = LLDP_TYPE_MAX_ASSET_ID_LEN;
            if(tmp_len != 0)
            {
                tlv = pdu + *pdu_len;
                tlv[0] = 127 << 1;
                tlv[1] = 4 + tmp_len;
                memcpy(&tlv[2], LLDP_OrgTiaOui, 3);
                tlv[5] = 11;
                memcpy(&tlv[6], ent_physical_entry_for_container_p->ent_physical_entry_rw.ent_physical_asset_id, tmp_len);
                *pdu_len += (2 + tlv[1]);
            }
        }
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructOrgSpecTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct the organizational specific TLV
 * INPUT    : port_config
 *            pdu           -- start address of the LLDPDU packet
 *            pdu_len       -- the length of the LLDPDU packet
 * OUTPUT   : pdu
 *            pdu_len
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_UTY_ConstructOrgSpecTLV(LLDP_OM_PortConfigEntry_T *port_config, UI8_T *pdu, UI16_T *pdu_len)
{
    LLDP_UTY_ConstructXdot1Tlvs(port_config, pdu, pdu_len);
    LLDP_UTY_ConstructXdcbxTlvs(port_config, pdu, pdu_len);
    LLDP_UTY_ConstructXdot3Tlvs(port_config, pdu, pdu_len);
    LLDP_UTY_ConstructMedTlvs(port_config, pdu, pdu_len);

}/* End of LLDP_UTY_ConstructOrgSpecTLV*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_SendLLDPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Send the LLDPDU to lan
 * INPUT    : UI32_T    lport
 *            UI8_T     *pdu        -- start address of the LLDPDU packet
 *            UI16_T    pdu_len;    -- the length of the LLDPDU packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_UTY_SendLLDPDU(UI32_T lport, UI8_T *pdu, UI16_T pdu_len)
{
    UI32_T      link_status;
    Port_Info_T port_info;
    UI8_T       my_mac_addr[6];


    L_MM_Mref_Handle_T *mref;
    UI8_T       *pdu_pos = 0;
    UI32_T      tmp_len;
    UI32_T      unit = 0, port = 0, trunk_id = 0;

    if((mref = L_MM_AllocateTxBuffer((pdu_len + 18), L_MM_USER_ID2(SYS_MODULE_LLDP,0))) == NULL)
    {
        return ;
    }

    pdu_pos = (UI8_T *)L_MM_Mref_GetPdu(mref, &tmp_len);
    if(pdu_pos == NULL)
    {
        return;
    }

    memcpy(pdu_pos, pdu, pdu_len);

    SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    /* === Check link === */
    if(!SWCTRL_GetPortInfo(lport, &port_info))
    {
        L_MM_Mref_Release(&mref);

        #if DEBUG_PRINT
        printf("<LLDP_UTY_SendLLDPDU>Can not get lport%ld port info\n", lport);
        #endif
        return ;
    }

    link_status = port_info.link_status;

    /* if the link is down, free packet and return */
    if(link_status == SWCTRL_LINK_DOWN)
    {

        L_MM_Mref_Release(&mref);

        #if DEBUG_PRINT
        {
            printf("<LLDP_UTY_SendLLDPDU>Link status of lport%d is SWCTRL_LINK_DOWN\n", lport);
        }
        #endif
        return ;
    }
    #if PRINT_PACKET
    {
        UI32_T index;
        printf("Send LLDPDU from port %ld:", lport);
        for(index = 0; index < pdu_len; index++)
        {
            printf("%02X", pdu[index]);
        }
        puts("");
    }
    #endif

    /* Get port mac*/
    if(!SWCTRL_GetPortMac(lport, my_mac_addr))
    {
        L_MM_Mref_Release(&mref);
        return;
    }

    /* send packet */
    mref->next_usr_id = SYS_MODULE_LAN;

    LAN_SendPacket( mref,               /* Reference to memory */
                    LLDP_MultiCastAddr,
                    my_mac_addr,
                    LLDP_EtherType,
                    SYS_TYPE_IGNORE_VID_CHECK,
                    pdu_len,
                    unit,
                    port,
                    FALSE,  /* untagged */
                    LLDP_TYPE_LLDPDU_TX_COS); /* highest priority */
}/* End of LLDP_UTY_SendLLDPDU*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_RecogLLDPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Check the type of the packet
 * INPUT    : LLDP_TYPE_MSG_T *msg
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref. to the description in 10.3.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_UTY_RecogLLDPDU(LLDP_TYPE_MSG_T *msg)
{
    if(0x88CC != msg->type)
    {
        return FALSE;
    }
    return TRUE;
}/* End of LLDP_UTY_RecogLLDPDU */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ExtractTLV
 *-------------------------------------------------------------------------
 * PURPOSE  : Extract TLVs from incoming LLDPDU
 * INPUT    : pdu (pos of the first tlv)
 * OUTPUT   : TLV
 * RETURN   : Next TLV pos
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static UI8_T* LLDP_UTY_ExtractTLV(UI8_T *pdu, LLDP_TYPE_TLV_T *tlv)
{
    int i = 0;
    UI16_T  len1, len2;

    tlv->type = ((char)(*pdu) & 0xFE ) >> 1;    /* get TLV type */

    len1 = (*pdu & 0x01) * 256;                 /* get TLV length */
    len2 = *(pdu + 1);
    tlv->len = len1 + len2;

    for (i = 0; i < tlv->len; i++)              /* get TLV value */
        tlv->value[i] = pdu[2 + i];

    pdu = pdu + 2 + tlv->len;                   /* shift to the next TLV */
    return pdu;
}/* End of LLDP_UTY_ExtractTLV */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_InsertElementToList
 *-------------------------------------------------------------------------
 * PURPOSE  : To insert a new element to list
 * INPUT    : list_p        - pointer to list descriptor
 *            new_element_p - pointer to element to insert
 * OUTPUT   : None
 * RETURN   : TRUE  - element is inserted into list.
 *            FALSE - element is not inserted into list.
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T LLDP_UTY_InsertElementToList(
    L_SORT_LST_List_T   *list_p,
    void                *new_element_p)
{
    void    *old_element_p;
    BOOL_T  is_free_old = FALSE;

    old_element_p = new_element_p;

    /* element already exists in the list
     */
    if (L_SORT_LST_Get(list_p, &old_element_p))
    {
        is_free_old = TRUE;
    }

    /* replace with new element
     */
    if (!L_SORT_LST_Set(list_p, &new_element_p))
    {
        return FALSE;
    }

    if (TRUE == is_free_old)
    {
        free(old_element_p);
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConvertMsgToRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Record tlvs to LLDP_OM_RemData_T type structure
 * INPUT    : LLDP_TYPE_MSG_T* msg          -- incoming message
 *            LLDP_OM_RemData_T *rem_data   -- prepared rem_data
 *            tlv_type_count                -- the count of each type tlv
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_UTY_ConvertMsgToRemData(LLDP_TYPE_MSG_T* msg, LLDP_OM_RemData_T *rem_data, UI32_T *tlv_type_count)
{
    unsigned char       *next_tlv, *pdu, *ori_pdu;
    static LLDP_TYPE_TLV_T     tlv;
    LLDP_OM_PortStatistics_T    *port_statistics;
    LLDP_OM_PortConfigEntry_T   *port_config;
    LLDP_OM_SysConfigEntry_T    *sys_config;
    UI32_T  pdu_len = 0, current_time;

    SYS_TIME_GetSystemUpTimeByTick(&current_time);
    rem_data->time_mark = current_time;

    rem_data->lport = msg->lport;
    port_statistics = LLDP_OM_GetPortStatisticsEntryPtr(rem_data->lport);
    port_config = LLDP_OM_GetPortConfigEntryPtr(rem_data->lport);
    sys_config = LLDP_OM_GetSysConfigEntryPtr();

    /* extract each tlv in LLDPDU*/
    #if RX_DEBUG_PRINT
    {
        printf("Covert/Store packet to DB:\n");
    }
    #endif

    pdu = (UI8_T *)L_MM_Mref_GetPdu(msg->mem_ref, &pdu_len);
    if(pdu_len == 0)
        return;

    rem_data->rem_sys_entry = (LLDP_OM_RemSysEntry_T *)malloc(sizeof(LLDP_OM_RemSysEntry_T));
    if (rem_data->rem_sys_entry == NULL)
    {
        return;
    }
    memset(rem_data->rem_sys_entry, 0, sizeof(LLDP_OM_RemSysEntry_T));

    ori_pdu = pdu;

    do
    {
        memset(&tlv, 0, sizeof(LLDP_TYPE_TLV_T));
        next_tlv = LLDP_UTY_ExtractTLV(pdu, &tlv);

        if (next_tlv > (ori_pdu+pdu_len)) /* means current tlv already outside pdu_len */
        {
            port_statistics->rx_tlvs_discarded_total++;
            break;
        }

        switch(tlv.type)
        {
            case LLDP_TYPE_CHASSIS_ID_TLV:
                {
                    if (tlv.len >= 2 && tlv.len <= 256) /* rem_chassis_id_len is 1~255 */
                    {
                        rem_data->rem_chassis_id_subtype = tlv.value[0];
                        rem_data->rem_chassis_id_len = tlv.len - 1;
                        if(rem_data->rem_chassis_id_len > LLDP_TYPE_MAX_CHASSIS_ID_LENGTH)
                        {
                            rem_data->rem_chassis_id_len = LLDP_TYPE_MAX_CHASSIS_ID_LENGTH;
                        }
                        memcpy(rem_data->rem_chassis_id, (UI8_T*)&(tlv.value[1]), rem_data->rem_chassis_id_len);
                        #if RX_DEBUG_PRINT
                        {
                            int i;
                            printf("CHASSIS_ID_TLV-> subtype:%d, length:%ld, chass_id: ",
                                    rem_data->rem_chassis_id_subtype,
                                    rem_data->rem_chassis_id_len);
                            for(i = 0; i < rem_data->rem_chassis_id_len; i++)
                                printf("%02X", rem_data->rem_chassis_id[i]);
                            puts("");
                        }
                        #endif
                    }
                    else
                    {
                        port_statistics->rx_tlvs_discarded_total++;
                        break;
                    }
                }
                break;

            case LLDP_TYPE_PORT_ID_TLV:
                {
                    if (tlv.len >= 2 && tlv.len <= 256) /* rem_port_id_len is 1~255 */
                    {
                        rem_data->rem_port_id_subtype = tlv.value[0];
                        rem_data->rem_port_id_len = tlv.len - 1;
                        if(rem_data->rem_port_id_len > LLDP_TYPE_MAX_PORT_ID_LENGTH)
                        {
                            rem_data->rem_port_id_len = LLDP_TYPE_MAX_PORT_ID_LENGTH;
                        }
                        memcpy(rem_data->rem_port_id, (UI8_T*)&(tlv.value[1]), rem_data->rem_port_id_len);
                        #if RX_DEBUG_PRINT
                        {
                            int i;
                            printf("PORT_ID_TLV-> subtype:%d, length:%ld, port_id:",
                                    rem_data->rem_port_id_subtype,
                                    rem_data->rem_port_id_len);
                            for(i = 0; i < rem_data->rem_port_id_len; i++)
                                printf("%02X", rem_data->rem_port_id[i]);
                            puts("");
                        }
                        #endif
                    }
                    else
                    {
                        port_statistics->rx_tlvs_discarded_total++;
                        break;
                    }
                }
                break;

            case LLDP_TYPE_TIME_TO_LIVE_TLV:
                {
                    UI16_T  tmp;

                    if (tlv.len == 2) /* TTL TLV length must equal 2 */
                    {
                        memcpy(&tmp, tlv.value, 2);
                        rem_data->rx_info_ttl = L_STDLIB_Ntoh16(tmp);

                        #if RX_DEBUG_PRINT
                        {
                            printf("TTL_TLV-> ttl:%d\n", rem_data->rx_info_ttl);
                        }
                        #endif
                    }
                    else
                    {
                        port_statistics->rx_tlvs_discarded_total++;
                        break;
                    }
                }
                break;
            case LLDP_TYPE_PORT_DESC_TLV:
                {
                    if(tlv.len <= 255 && rem_data->rem_sys_entry->rem_port_desc == NULL)
                    {
                        rem_data->rem_sys_entry->rem_port_desc_len = tlv.len;
                        if((rem_data->rem_sys_entry->rem_port_desc = (UI8_T *)malloc(tlv.len)) == 0)
                        {
                            break;
                        }
                        memcpy(rem_data->rem_sys_entry->rem_port_desc, tlv.value, tlv.len);
                        #if RX_DEBUG_PRINT
                        {
                            printf("PORT_DESC_TLV-> length:%ld, port_desc:%s\n",
                                    rem_data->rem_sys_entry->rem_port_desc_len,
                                    rem_data->rem_sys_entry->rem_port_desc);
                        }
                        #endif
                    }
                    else
                    {
                        port_statistics->rx_tlvs_discarded_total++;
                    }
                }
                break;
            case LLDP_TYPE_SYS_NAME_TLV:
                {
                    if(tlv.len <= 255 && rem_data->rem_sys_entry->rem_sys_name == NULL)
                    {
                        rem_data->rem_sys_entry->rem_sys_name_len = tlv.len;
                        if((rem_data->rem_sys_entry->rem_sys_name = (UI8_T *)malloc(tlv.len)) == 0)
                        {
                            break;
                        }
                        memcpy(rem_data->rem_sys_entry->rem_sys_name, tlv.value, tlv.len);
                        #if RX_DEBUG_PRINT
                        {
                            printf("SYS_NAME_TLV-> length:%ld, sys_name:%s\n",
                                    rem_data->rem_sys_entry->rem_sys_name_len,
                                    rem_data->rem_sys_entry->rem_sys_name);
                        }
                        #endif
                    }
                    else
                    {
                        port_statistics->rx_tlvs_discarded_total++;
                    }
                }
                break;

            case LLDP_TYPE_SYS_DESC_TLV:
                {
                    if(tlv.len <= 255 && rem_data->rem_sys_entry->rem_sys_desc == NULL)
                    {
                        rem_data->rem_sys_entry->rem_sys_desc_len = tlv.len;
                        if((rem_data->rem_sys_entry->rem_sys_desc = (UI8_T *)malloc(tlv.len)) == 0 )
                        {
                            break;
                        }
                        memcpy(rem_data->rem_sys_entry->rem_sys_desc, tlv.value, tlv.len);
                        #if RX_DEBUG_PRINT
                        {
                            printf("SYS_DESC_TLV-> length:%ld, sys_desc:%s\n",
                                    rem_data->rem_sys_entry->rem_sys_desc_len,
                                    rem_data->rem_sys_entry->rem_sys_desc);
                        }
                        #endif
                    }
                    else
                    {
                        port_statistics->rx_tlvs_discarded_total++;
                    }
                }
                break;

            case LLDP_TYPE_SYS_CAP_TLV:
                {
                    if(tlv.len == 4)
                    {
                        UI16_T tmp;

                        memcpy(&tmp, &tlv.value[0], 2);
                        rem_data->rem_sys_entry->rem_sys_cap_supported = L_STDLIB_Ntoh16(tmp);
                        memcpy(&tmp, &tlv.value[2], 2);
                        rem_data->rem_sys_entry->rem_sys_cap_enabled = L_STDLIB_Ntoh16(tmp);
                        #if RX_DEBUG_PRINT
                        {
                            printf("SYS_CAP_TLV-> cap_sup: 0x%04X, cap_enable:0x%04X",
                                    rem_data->rem_sys_entry->rem_sys_cap_supported,
                                    rem_data->rem_sys_entry->rem_sys_cap_enabled);
                        }
                        #endif
                    }
                    else
                    {
                        port_statistics->rx_tlvs_discarded_total++;
                    }
                }
                break;
            case LLDP_TYPE_MAN_ADDR_TLV:
                {
                    BOOL_T  no_oid = FALSE;
                    if(tlv.len >= 9 && tlv.len <= 167)
                    {
                        LLDP_OM_RemManAddrEntry_T *rem_man_addr = 0;
                        UI32_T  tmp;

                        int rem_addr_len = tlv.value[0];


                        {
                            /* prevent illegal addressing operation by
                             * calculate remote man address field len
                             */
                            int tmplen = tlv.len;
                            tmplen--;               /* subtract len(man_addr_subtype_field) */
                            tmplen -= rem_addr_len; /* subtact len(man_addr_field) */
                            if(tmplen < 0 || (rem_addr_len > 32 || rem_addr_len < 2) )
                            {
                                /* debug print */
                                #if RX_DEBUG_PRINT
                                {
                                    puts("Rem man addr error 1");
                                }
                                #endif
                                port_statistics->rx_tlvs_discarded_total++;
                                break;
                            }
                            tmplen -= 5;        /* subtract len(if_subtype_field + if_number_field) */

                            if(tmplen < 1)
                            {
                                #if RX_DEBUG_PRINT
                                {
                                    puts("Rem man addr contain no IF entry");
                                }
                                #endif

                                /* debug print */
                                port_statistics->rx_tlvs_discarded_total++;
                                break;
                            }
                            tmplen -= 1;        /* subtract len(OID_string_len_field)*/

                            if(tmplen <=0)
                            {
                                #if RX_DEBUG_PRINT
                                {
                                    puts("Rem man addr contain no OID entry");
                                }
                                #endif
                                /* debug print */
                                no_oid = TRUE;
                            }
                        }

                        /* allocate rem man addr sort list if needed*/
                        if(tlv_type_count[LLDP_TYPE_MAN_ADDR_TLV] != 0 &&
                        rem_data->rem_man_addr_list.max_element_count != tlv_type_count[LLDP_TYPE_MAN_ADDR_TLV] )
                        {
                            if(!LLDP_OM_CreateRemManAddrList(&rem_data->rem_man_addr_list, tlv_type_count[8]))
                            {
                                #if RX_DEBUG_PRINT
                                {
                                    puts("Alloc rem man addr sort list error");
                                }
                                #endif
                                break;
                            }
                        }

                        /* fill it*/
                        rem_man_addr = (LLDP_OM_RemManAddrEntry_T *)malloc(sizeof(LLDP_OM_RemManAddrEntry_T));
                        if (rem_man_addr == NULL)
                        {
                            break;
                        }

                        memset(rem_man_addr, 0, sizeof(LLDP_OM_RemManAddrEntry_T));

                        rem_man_addr->rem_man_addr_len = rem_addr_len;
                        rem_man_addr->rem_man_addr_subtype = tlv.value[1];
                        memcpy(rem_man_addr->rem_man_addr, &tlv.value[2], rem_addr_len - 1);
                        rem_man_addr->rem_man_addr_if_subtype = tlv.value[rem_addr_len + 1];
                        memcpy(&tmp, &tlv.value[rem_addr_len + 2], 4);
                        rem_man_addr->rem_man_addr_if_id = L_STDLIB_Ntoh32(tmp);
                        if(!no_oid)
                        {
                            rem_man_addr->rem_man_addr_oid_len = tlv.value[rem_addr_len+ 6];

                            if (rem_man_addr->rem_man_addr_oid_len > LLDP_TYPE_MAX_MANAGEMENT_ADDR_OID_LENGTH)
                                rem_man_addr->rem_man_addr_oid_len = LLDP_TYPE_MAX_MANAGEMENT_ADDR_OID_LENGTH;

                            memcpy(rem_man_addr->rem_man_addr_oid, &tlv.value[rem_addr_len+ 7], rem_man_addr->rem_man_addr_oid_len);
                        }
                        #if RX_DEBUG_PRINT
                        {
                            int i;
                            printf("REM_MAN_ADDR_TLV-> addr_length:%ld, subtype:%d, addr:",
                                    rem_man_addr->rem_man_addr_len,
                                    rem_man_addr->rem_man_addr_subtype);
                            for(i = 0; i < rem_man_addr->rem_man_addr_len - 1; i++)
                                printf("%02X", rem_man_addr->rem_man_addr[i]);

                            printf(", if_subtype:%d, if_id:%ld",
                                   rem_man_addr->rem_man_addr_if_subtype,
                                   rem_man_addr->rem_man_addr_if_id);
                            if(!no_oid)
                            {
                                printf(", oid_length:%ld, oid:", rem_man_addr->rem_man_addr_oid_len);
                                for(i = 0; i < rem_man_addr->rem_man_addr_oid_len; i++)
                                    printf("%02X", rem_man_addr->rem_man_addr_oid[i]);
                            }
                            puts("");
                        }
                        #endif
                        /* insert into list */

                        if (FALSE == LLDP_UTY_InsertElementToList(
                                        &rem_data->rem_man_addr_list, rem_man_addr))
                        {
                            #if RX_DEBUG_PRINT
                                puts("Rem man addr insert fail !!");
                            #endif
                            free(rem_man_addr);
                        }

                    }
                    else
                    {
                        #if RX_DEBUG_PRINT
                        {
                            puts("Rem man addr error 0");
                        }
                        #endif
                        port_statistics->rx_tlvs_discarded_total++;
                    }
                }
                break;
            /* under type are not support for basic LLDP*/
            case LLDP_TYPE_ORG_SPEC_TLV:
                {
                    if (tlv.len<4 || tlv.len>511) /* Organizationally-specific TLV length range */
                    {
                        port_statistics->rx_tlvs_discarded_total++;
                        break;
                    }

                    /* 802.1 */
                    if(tlv.value[0] == 0x00 && tlv.value[1] == 0x80 && tlv.value[2] == 0xC2)
                    {
                        switch(tlv.value[3])
                        {
                            case LLDP_TYPE_DOT1_PORT_VLAN_ID_TLV:
                                if(tlv.len == 6)
                                {
                                    UI16_T  tmp;
                                    memcpy(&tmp, &tlv.value[4], 2);
                                    rem_data->xdot1_rem_port_vlan_id = L_STDLIB_Ntoh16(tmp);
                                    #if RX_DEBUG_PRINT
                                    {
                                        printf("PORT_VLAN_ID_TLV: port_vlan_id=%ld\n", rem_data->xdot1_rem_port_vlan_id);
                                    }
                                    #endif
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_DOT1_PORT_AND_PROTO_VLAN_ID_TLV:
                                if(tlv.len == 7)
                                {
                                    UI16_T tmp;
                                    LLDP_OM_Xdot1RemProtoVlanEntry_T *new_proto_vlan_entry;

                                    memcpy(&tmp, &tlv.value[5], 2);

                                    /* refer to LLDP_UTY_ConstructXdot1Tlvs,
                                     *   Std 802.1AB-2005, F.3.2, etc...
                                     */
                                    if (tmp <= 4094)
                                    {
                                        if (    ((tlv.value[4] & (1 << 1)) == 0)
                                             && ((tlv.value[4] & (1 << 2)) != 0)
                                           )
                                        {
                                            /* not supported but enabled */
                                            port_statistics->rx_tlvs_discarded_total++;
                                            break;
                                        }

                                        new_proto_vlan_entry = (LLDP_OM_Xdot1RemProtoVlanEntry_T *)malloc(sizeof(LLDP_OM_Xdot1RemProtoVlanEntry_T));       /* Don't forget to free if deleted */
                                        if (new_proto_vlan_entry == NULL)
                                        {
                                            break;
                                        }
                                        memset(new_proto_vlan_entry, 0, sizeof(LLDP_OM_Xdot1RemProtoVlanEntry_T));

                                        new_proto_vlan_entry->rem_proto_vlan_id = L_STDLIB_Ntoh16(tmp);
                                        new_proto_vlan_entry->rem_proto_vlan_supported = ( (tlv.value[4] & (1 << 1) ) != 0)? 1 : 2;
                                        new_proto_vlan_entry->rem_proto_vlan_enabled = ( (tlv.value[4] & (1 << 2) ) != 0)? 1 : 2;

                                        if(rem_data->xdot1_rem_proto_vlan_list.max_element_count == 0)
                                        {
                                            if(!LLDP_OM_CreateRemProtoVlanList(&rem_data->xdot1_rem_proto_vlan_list))
                                            {
                                                free(new_proto_vlan_entry);
                                                break;
                                            }
                                        }
                                        if (FALSE == LLDP_UTY_InsertElementToList(
                                                        &rem_data->xdot1_rem_proto_vlan_list, new_proto_vlan_entry))
                                        {
                                            free(new_proto_vlan_entry);
                                        }
                                        #if RX_DEBUG_PRINT
                                        {
                                            printf("PORT_PROTO_VLAN_ID: rem_proto_vlan_id=%ld, rem_proto_vlan_support=%d, rem_proto_vlan_enable=%d\n",
                                                    new_proto_vlan_entry->rem_proto_vlan_id, new_proto_vlan_entry->rem_proto_vlan_supported, new_proto_vlan_entry->rem_proto_vlan_enabled);
                                        }
                                        #endif
                                    }
                                    else
                                    {
                                        port_statistics->rx_tlvs_discarded_total++;
                                    }
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_DOT1_VLAN_NAME_TLV:
                                if(tlv.len <= 39 && tlv.len >= 7)
                                {
                                    UI16_T tmp;
                                    LLDP_OM_Xdot1RemVlanNameEntry_T *new_rem_vlan_name_entry;

                                    new_rem_vlan_name_entry = (LLDP_OM_Xdot1RemVlanNameEntry_T *)malloc(sizeof(LLDP_OM_Xdot1RemVlanNameEntry_T));
                                    if (new_rem_vlan_name_entry == NULL)
                                    {
                                        break;
                                    }

                                    memset(new_rem_vlan_name_entry, 0, sizeof(LLDP_OM_Xdot1RemVlanNameEntry_T));

                                    memcpy(&tmp, &tlv.value[4], 2);

                                    new_rem_vlan_name_entry->rem_vlan_id = L_STDLIB_Ntoh16(tmp);
                                    new_rem_vlan_name_entry->rem_vlan_name_len = tlv.value[6];
                                    memcpy(new_rem_vlan_name_entry->rem_vlan_name, &tlv.value[7], tlv.value[6]);

                                    if(rem_data->xdot1_rem_vlan_name_list.max_element_count == 0)
                                    {
                                        if(!LLDP_OM_CreateRemVlanNameList(&rem_data->xdot1_rem_vlan_name_list))
                                        {
                                            free(new_rem_vlan_name_entry);
                                            break;
                                        }
                                    }
                                    if (FALSE == LLDP_UTY_InsertElementToList(
                                                    &rem_data->xdot1_rem_vlan_name_list, new_rem_vlan_name_entry))
                                    {
                                        free(new_rem_vlan_name_entry);
                                    }
                                    #if RX_DEBUG_PRINT
                                    {
                                        int i;
                                        printf("VLAN_NAME_TLV:rem_vlan_id=%ld, rem_vlan_name_len=%ld,", new_rem_vlan_name_entry->rem_vlan_id, new_rem_vlan_name_entry->rem_vlan_name_len);
                                        for(i = 0; i < new_rem_vlan_name_entry->rem_vlan_name_len; i++)
                                        {
                                            printf("%c", new_rem_vlan_name_entry->rem_vlan_name[i]);
                                        }
                                        puts("");
                                    }
                                    #endif
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_DOT1_PTOTO_IDENT_TLV:
                                if(tlv.len <= 260 && tlv.len >= 5)
                                {
                                    LLDP_OM_Xdot1ProtocolEntry_T    *new_rem_protocol_entry;

                                    new_rem_protocol_entry = (LLDP_OM_Xdot1ProtocolEntry_T *)malloc(sizeof(LLDP_OM_Xdot1ProtocolEntry_T));
                                    if (new_rem_protocol_entry == NULL)
                                    {
                                        break;
                                    }

                                    memset(new_rem_protocol_entry, 0, sizeof(LLDP_OM_Xdot1ProtocolEntry_T));

                                    new_rem_protocol_entry->rem_protocol_id_len = tlv.value[4];
                                    memcpy(new_rem_protocol_entry->rem_protocol_id, &tlv.value[5], tlv.value[4]);

                                    if(rem_data->xdot1_rem_protocol_list.max_element_count == 0)
                                    {
                                        if(!LLDP_OM_CreateRemProtocolList(&rem_data->xdot1_rem_protocol_list))
                                        {
                                            free(new_rem_protocol_entry);
                                            break;
                                        }
                                    }

                                    new_rem_protocol_entry->rem_protocol_index = rem_data->xdot1_rem_protocol_list.nbr_of_element + 1;

                                    if (FALSE == LLDP_UTY_InsertElementToList(
                                                    &rem_data->xdot1_rem_protocol_list, new_rem_protocol_entry))
                                    {
                                        free(new_rem_protocol_entry);
                                    }
                                    #if RX_DEBUG_PRINT
                                    {
                                        int i;
                                        printf("PTOTO_IDENT_TLV:rem_protocol_index=%ld, rem_protocol_id_len=%ld,", new_rem_protocol_entry->rem_protocol_index, new_rem_protocol_entry->rem_protocol_id_len);
                                        for(i = 0; i < new_rem_protocol_entry->rem_protocol_id_len; i++)
                                        {
                                            printf("%c", new_rem_protocol_entry->rem_protocol_id[i]);
                                        }
                                        puts("");
                                    }
                                    #endif
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;

                                }
                                break;
                            case LLDP_TYPE_DOT1_CN_TLV:
                                if((tlv.len == 6) && (rem_data->xdot1_rem_cn_entry == NULL))
                                {
                                    rem_data->xdot1_rem_cn_entry = (LLDP_OM_Xdot1RemCnEntry_T *)malloc(sizeof(LLDP_OM_Xdot1RemCnEntry_T));
                                    if (rem_data->xdot1_rem_cn_entry == NULL)
                                    {
                                        break;
                                    }

                                    memset(rem_data->xdot1_rem_cn_entry, 0, sizeof(LLDP_OM_Xdot1RemCnEntry_T));
                                    rem_data->xdot1_rem_cn_entry->cnpv_indicators = tlv.value[4];
                                    rem_data->xdot1_rem_cn_entry->ready_indicators = tlv.value[5];
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_DCBX_ETS_CONFIG_TLV:
                                if(tlv.len == 25)
                                {
                                    rem_data->dcbx_rem_ets_config_entry = (LLDP_OM_XDcbxRemEtsConfigEntry_T *)malloc(sizeof(LLDP_OM_XDcbxRemEtsConfigEntry_T));
                                    if (rem_data->dcbx_rem_ets_config_entry == NULL)
                                    {
                                        break;
                                    }

                                    memset(rem_data->dcbx_rem_ets_config_entry, 0, sizeof(LLDP_OM_XDcbxRemEtsConfigEntry_T));
                                    rem_data->dcbx_rem_ets_config_entry->rem_willing =  ((tlv.value[4] & (1<<7)) != 0)? TRUE : FALSE;
                                    rem_data->dcbx_rem_ets_config_entry->rem_cbs =  ((tlv.value[4] & (1<<6)) != 0)? TRUE : FALSE;
                                    rem_data->dcbx_rem_ets_config_entry->rem_max_tc =  (tlv.value[4] & (7 << 0));
                                    memcpy(rem_data->dcbx_rem_ets_config_entry->rem_pri_assign_table, &tlv.value[5], LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
                                    memcpy(rem_data->dcbx_rem_ets_config_entry->rem_tc_bandwidth_table, &tlv.value[9], LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
                                    memcpy(rem_data->dcbx_rem_ets_config_entry->rem_tsa_assign_table, &tlv.value[17], LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);

                                    #if RX_DEBUG_PRINT
                                    {
                                        UI8_T index = 0;
                                        printf("ETS_CONFIG_TLV: willing=%d\n", rem_data->dcbx_rem_ets_config_entry->rem_willing);
                                        printf("ETS_CONFIG_TLV: cbs=%d\n", rem_data->dcbx_rem_ets_config_entry->rem_cbs);
                                        printf("ETS_CONFIG_TLV: max_tc=%d\n", rem_data->dcbx_rem_ets_config_entry->rem_max_tc);
                                        printf("ETS_CONFIG_TLV: pri_assign_table=");
                                        for(index=0;index<4;index++)
                                            printf("%02X", rem_data->dcbx_rem_ets_config_entry->rem_pri_assign_table[index]);
                                        printf("\nETS_CONFIG_TLV: tc_bandwidth_table=");
                                        for(index=0;index<8;index++)
                                            printf("%02X", rem_data->dcbx_rem_ets_config_entry->rem_tc_bandwidth_table[index]);
                                        printf("\nETS_CONFIG_TLV: tsa_assign_table=");
                                        for(index=0;index<8;index++)
                                            printf("%02X\n", rem_data->dcbx_rem_ets_config_entry->rem_tsa_assign_table[index]);
                                        printf("\n");
                                    }
                                    #endif
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_DCBX_ETS_RECOMMEND_TLV:
                                if(tlv.len == 25)
                                {
                                    rem_data->dcbx_rem_ets_recommend_entry = (LLDP_OM_XDcbxRemEtsRecommendEntry_T *)malloc(sizeof(LLDP_OM_XDcbxRemEtsRecommendEntry_T));
                                    if (rem_data->dcbx_rem_ets_recommend_entry == NULL)
                                    {
                                        break;
                                    }

                                    memset(rem_data->dcbx_rem_ets_recommend_entry, 0, sizeof(LLDP_OM_XDcbxRemEtsRecommendEntry_T));
                                    memcpy(rem_data->dcbx_rem_ets_recommend_entry->rem_pri_assign_table, &tlv.value[5], LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
                                    memcpy(rem_data->dcbx_rem_ets_recommend_entry->rem_tc_bandwidth_table, &tlv.value[9], LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
                                    memcpy(rem_data->dcbx_rem_ets_recommend_entry->rem_tsa_assign_table, &tlv.value[17], LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);

                                    #if RX_DEBUG_PRINT
                                    {
                                        UI8_T index = 0;
                                        printf("ETS_RECOMMEND_TLV: pri_assign_table=");
                                        for(index=0;index<4;index++)
                                            printf("%02X", rem_data->dcbx_rem_ets_recommend_entry->rem_pri_assign_table[index]);
                                        printf("\nETS_RECOMMEND_TLV: tc_bandwidth_table=");
                                        for(index=0;index<8;index++)
                                            printf("%02X", rem_data->dcbx_rem_ets_recommend_entry->rem_tc_bandwidth_table[index]);
                                        printf("\nETS_RECOMMEND_TLV: tsa_assign_table=");
                                        for(index=0;index<8;index++)
                                            printf("%02X\n", rem_data->dcbx_rem_ets_recommend_entry->rem_tsa_assign_table[index]);
                                        printf("\n");
                                    }
                                    #endif
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_DCBX_PFC_CONFIG_TLV:
                                if(tlv.len == 6)
                                {
                                    rem_data->dcbx_rem_pfc_config_entry = (LLDP_OM_XDcbxRemPfcConfigEntry_T *)malloc(sizeof(LLDP_OM_XDcbxRemPfcConfigEntry_T));
                                    if (rem_data->dcbx_rem_pfc_config_entry == NULL)
                                    {
                                        break;
                                    }

                                    memset(rem_data->dcbx_rem_pfc_config_entry, 0, sizeof(LLDP_OM_XDcbxRemPfcConfigEntry_T));
                                    memcpy(rem_data->dcbx_rem_pfc_config_entry->rem_mac, msg->saddr, 6);
                                    rem_data->dcbx_rem_pfc_config_entry->rem_willing =  ((tlv.value[4] & (1<<7)) != 0)? TRUE : FALSE;
                                    rem_data->dcbx_rem_pfc_config_entry->rem_mbc =  ((tlv.value[4] & (1<<6)) != 0)? TRUE : FALSE;
                                    rem_data->dcbx_rem_pfc_config_entry->rem_cap =  (tlv.value[4] & (15 << 0));
                                    rem_data->dcbx_rem_pfc_config_entry->rem_enable =  tlv.value[5];
                                    #if RX_DEBUG_PRINT
                                    {
                                        printf("PFC_CONFIG_TLV: willing=%d\n", rem_data->dcbx_rem_pfc_config_entry->rem_willing);
                                        printf("PFC_CONFIG_TLV: mbc=%d\n", rem_data->dcbx_rem_pfc_config_entry->rem_mbc);
                                        printf("PFC_CONFIG_TLV: pfc_cap=%d\n", rem_data->dcbx_rem_pfc_config_entry->rem_pfc_cap);
                                        printf("PFC_CONFIG_TLV: pfc_enable=%d\n", rem_data->dcbx_rem_pfc_config_entry->rem_pfc_enable);
                                    }
                                    #endif
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_DCBX_APP_PRI_TLV:
                                if((tlv.len >= 8) && (((tlv.len -5)%3) == 0))
                                {
                                    UI16_T app_pri_len;
                                    UI8_T *tlv_p;

                                    app_pri_len = tlv.len - 5;
                                    tlv_p = &tlv.value[5];
                                    while(app_pri_len > 0)
                                    {
                                        UI16_T tmp;
                                        LLDP_OM_XDcbxRemAppPriorityEntry_T *new_rem_app_pri_entry;

                                        new_rem_app_pri_entry = (LLDP_OM_XDcbxRemAppPriorityEntry_T *)malloc(sizeof(LLDP_OM_XDcbxRemAppPriorityEntry_T));
                                        if (new_rem_app_pri_entry == NULL)
                                        {
                                            break;
                                        }

                                        memset(new_rem_app_pri_entry, 0, sizeof(LLDP_OM_XDcbxRemAppPriorityEntry_T));

                                        new_rem_app_pri_entry->rem_priority = ((tlv_p[0]>>5) & 7);
                                        new_rem_app_pri_entry->rem_sel = (tlv_p[0] & 7);
                                        memcpy(&tmp, &tlv_p[1], 2);
                                        new_rem_app_pri_entry->rem_protocol_id = L_STDLIB_Ntoh16(tmp);

                                        if(rem_data->dcbx_rem_app_pri_list.max_element_count == 0)
                                        {
                                            if(!LLDP_OM_CreateRemAppPriorityList(&rem_data->dcbx_rem_app_pri_list))
                                            {
                                                free(new_rem_app_pri_entry);
                                                break;
                                            }
                                        }
                                        if (FALSE == LLDP_UTY_InsertElementToList(
                                                        &rem_data->dcbx_rem_app_pri_list, new_rem_app_pri_entry))
                                        {
                                            free(new_rem_app_pri_entry);
                                        }
                                        #if RX_DEBUG_PRINT
                                        {
                                            printf("APP_PRIORITY_TLV:rem_priority=%d, rem_sel=%d, rem_protocol_id=%ld\n",
                                                            new_rem_app_pri_entry->rem_priority,
                                                            new_rem_app_pri_entry->rem_sel,
                                                            new_rem_app_pri_entry->rem_protocol_id);
                                        }
                                        #endif
                                        app_pri_len -= 3;
                                        tlv_p += 3;
                                    }
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            default:
                                port_statistics->rx_tlvs_unrecognized_total++;
                                break;
                        }
                    }

                    /* 802.3 */
                    if(tlv.value[0] == 0x00 && tlv.value[1] == 0x12 && tlv.value[2] == 0x0F)
                    {
                        switch(tlv.value[3])
                        {
                            case LLDP_TYPE_DOT3_MAC_PHY_TLV:
                                if(tlv.len == 9 && rem_data->xdot3_rem_port_entry == NULL)
                                {
                                    UI16_T tmp;

                                    rem_data->xdot3_rem_port_entry = (LLDP_OM_Xdot3RemPortEntry_T *)malloc(sizeof(LLDP_OM_Xdot3RemPortEntry_T));
                                    if (rem_data->xdot3_rem_port_entry == NULL)
                                    {
                                        break;
                                    }

                                    memset(rem_data->xdot3_rem_port_entry, 0, sizeof(LLDP_OM_Xdot3RemPortEntry_T));

                                    rem_data->xdot3_rem_port_entry->rem_auto_neg_support = ((tlv.value[4] & 1) != 0)? 1 : 2;
                                    rem_data->xdot3_rem_port_entry->rem_auto_neg_enable = ((tlv.value[4] & 2) != 0)? 1 : 2;

                                    memcpy(&(rem_data->xdot3_rem_port_entry->rem_port_auto_neg_adv_cap), &tlv.value[5], 2);

                                    memcpy(&tmp, &tlv.value[7], 2);
                                    rem_data->xdot3_rem_port_entry->rem_port_oper_mau_type = L_STDLIB_Ntoh16(tmp);

                                    #if RX_DEBUG_PRINT
                                    printf("MAC_PHY: auto_neg_support=%d, auto_neg_enable=%d, auto_neg_adv_cap=%0X, mau_type=%d\n",
                                           rem_data->xdot3_rem_port_entry->rem_auto_neg_support,
                                           rem_data->xdot3_rem_port_entry->rem_auto_neg_enable,
                                           rem_data->xdot3_rem_port_entry->rem_port_auto_neg_adv_cap,
                                           rem_data->xdot3_rem_port_entry->rem_port_oper_mau_type);
                                    #endif
                                }
                                else
                                {

                                    port_statistics->rx_tlvs_discarded_total++;

                                }
                                break;

                            case LLDP_TYPE_DOT3_POWER_VIA_MDI_TLV:
                                if (( tlv.len == 7 || tlv.len == 12 )&&(rem_data->xdot3_rem_power_entry == NULL))
                                {
                                    rem_data->xdot3_rem_power_entry = (LLDP_OM_Xdot3RemPowerEntry_T *)malloc(sizeof(LLDP_OM_Xdot3RemPowerEntry_T));
                                    if (rem_data->xdot3_rem_power_entry == NULL)
                                    {
                                        break;
                                    }

                                    memset(rem_data->xdot3_rem_power_entry, 0, sizeof(LLDP_OM_Xdot3RemPowerEntry_T));
                                    rem_data->xdot3_rem_power_entry->rem_power_port_class = ((tlv.value[4] & 0x01) != 0) ? 1 : 2;  /* pse: 1, pd:2 */
                                    rem_data->xdot3_rem_power_entry->rem_power_mdi_supported = ((tlv.value[4] & 0x02) != 0) ? 1 : 2;
                                    rem_data->xdot3_rem_power_entry->rem_power_mdi_enabled = ((tlv.value[4] & 0x04) != 0) ? 1 : 2;
                                    rem_data->xdot3_rem_power_entry->rem_power_pair_controlable = ((tlv.value[4] & 0x08) != 0) ? 1 : 2;
                                    rem_data->xdot3_rem_power_entry->rem_power_pairs = tlv.value[5];
                                    rem_data->xdot3_rem_power_entry->rem_power_class = tlv.value[6];
    #if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
                                    if (tlv.len == 12)
                                    {
                                        UI16_T tmp;
                                        rem_data->xdot3_rem_power_entry->rem_power_type = (tlv.value[7] & 0xc0);
                                        rem_data->xdot3_rem_power_entry->rem_power_source = (tlv.value[7] & 0x30);
                                        rem_data->xdot3_rem_power_entry->rem_power_priority = (tlv.value[7] & 0x03);
                                        memcpy(&tmp, &tlv.value[8], 2);
                                        rem_data->xdot3_rem_power_entry->rem_pd_requested_power = L_STDLIB_Ntoh16(tmp);
                                        memcpy(&tmp, &tlv.value[10], 2);
                                        rem_data->xdot3_rem_power_entry->rem_pse_allocated_power = L_STDLIB_Ntoh16(tmp);
                                    }
    #endif
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;

                                }
                                break;

                            case LLDP_TYPE_DOT3_LINK_AGGREGATE_TLV:
                                if(tlv.len == 9 && rem_data->xdot3_rem_link_agg_entry == NULL)
                                {
                                    UI32_T  tmp;

                                    rem_data->xdot3_rem_link_agg_entry = (LLDP_OM_Xdot3RemLinkAggEntry_T *)malloc(sizeof(LLDP_OM_Xdot3RemLinkAggEntry_T));
                                    if (rem_data->xdot3_rem_link_agg_entry == NULL)
                                    {
                                        break;
                                    }

                                    memset(rem_data->xdot3_rem_link_agg_entry, 0, sizeof(LLDP_OM_Xdot3RemLinkAggEntry_T));
                                    rem_data->xdot3_rem_link_agg_entry->rem_link_agg_status = tlv.value[4];
                                    memcpy(&tmp, &tlv.value[5], sizeof(UI32_T));
                                    rem_data->xdot3_rem_link_agg_entry->rem_link_agg_port_id = L_STDLIB_Ntoh32(tmp);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_DOT3_MAX_FRAME_SIZE_TLV:
                                if(tlv.len == 6)
                                {
                                    UI16_T  tmp;
                                    memcpy(&tmp, &tlv.value[4], sizeof(UI16_T));
                                    rem_data->xdot3_rem_max_frame_size = L_STDLIB_Ntoh16(tmp);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            default:
                                port_statistics->rx_tlvs_unrecognized_total++;
                                break;
                        }
                    }
#if (LLDP_TYPE_MED == TRUE)
                    /* TIA LLDP-MED */
                    if(tlv.value[0] == 0x00 && tlv.value[1] == 0x12 && tlv.value[2] == 0xBB)
                    {
                        switch(tlv.value[3])
                        {
                            case LLDP_TYPE_MED_CAP_TLV:
                                if(tlv.len == 7)
                                {
                                    UI16_T  tmp;
                                    memcpy(&tmp, &tlv.value[4], 2);
                                    rem_data->lldp_med_cap_sup = L_STDLIB_Ntoh16(tmp);
                                    rem_data->lldp_med_device_type = tlv.value[6];
                                    if(!port_config->med_device_exist)
                                    {
                                        port_config->fast_start_count = sys_config->fast_start_repeat_count;
                                        port_config->something_changed_local = TRUE;
                                        port_config->med_device_exist = TRUE;
                                    }
                                    rem_data->lldp_med_cap_enabled |= (1 << 0);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_MED_NETWORK_POLICY_TLV:
                                if(tlv.len == 8)
                                {
                                    if (rem_data->med_rem_network_policy == NULL)
                                    {
                                        rem_data->med_rem_network_policy = (LLDP_OM_XMedRemNetworkPolicyEntry_T *)malloc(sizeof(LLDP_OM_XMedRemNetworkPolicyEntry_T));
                                        if (rem_data->med_rem_network_policy != NULL)
                                            memset(rem_data->med_rem_network_policy, 0, sizeof(LLDP_OM_XMedRemNetworkPolicyEntry_T));
                                    }

                                    if(rem_data->med_rem_network_policy != NULL)
                                    {
                                        UI32_T app_type;

                                        app_type = tlv.value[4];
                                        if (app_type >= VAL_lldpXMedRemMediaPolicyAppType_voice &&
                                            app_type <= LLDP_TYPE_MED_MAX_NETWORK_POLITY_TYPE &&
                                            !(rem_data->med_rem_network_policy->app_type[app_type-1].valid))
                                        {
                                            rem_data->med_rem_network_policy->app_type[app_type-1].valid    = TRUE;
                                            rem_data->med_rem_network_policy->app_type[app_type-1].unknown  = ((tlv.value[5] & 0x80) != 0) ? TRUE : FALSE;
                                            rem_data->med_rem_network_policy->app_type[app_type-1].tagged   = ((tlv.value[5] & 0x40) != 0) ? TRUE : FALSE;
                                            rem_data->med_rem_network_policy->app_type[app_type-1].reserved = ((tlv.value[5] & 0x20) != 0) ? TRUE : FALSE;
                                            rem_data->med_rem_network_policy->app_type[app_type-1].vid      = (tlv.value[5] & 0x1f) * 128 + ((tlv.value[6] & 0xFE) >> 1);
                                            rem_data->med_rem_network_policy->app_type[app_type-1].priority = (tlv.value[6] & 0x01) * 4 + ((tlv.value[7] & 0xC0) >> 6);
                                            rem_data->med_rem_network_policy->app_type[app_type-1].dscp     = (tlv.value[7] & 0x3F);
                                        }
                                    }
                                    rem_data->lldp_med_cap_enabled |= (1 << 1);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_MED_LOCATION_IDENT_TLV:
                                if(tlv.len <= 260)
                                {
                                    UI8_T  location_id_subtype = tlv.value[4];
                                    UI16_T location_id_len = tlv.len - 5;
                                    UI8_T  *location_id = &tlv.value[5];

                                    switch(location_id_subtype)
                                    {
                                        case 0:
                                            break;
                                        case 1:
#if 0 // wakka TODO
                                            if (rem_data->med_rem_location != NULL ||
                                                (rem_data->med_rem_location = (LLDP_OM_XMedRemLocationEntry_T *)calloc(sizeof(LLDP_OM_XMedRemLocationEntry_T))) != 0)
                                            {
                                                rem_data->med_rem_location->location_type_valid |= BIT_VALUE(VAL_lldpXMedLocLocationSubtype_coordinateBased);

                                                memset(&rem_data->med_rem_location->coord_addr, 0, sizeof(LLDP_OM_XMedLocationCoordinated_T));
                                            }
#endif
                                            break;
                                        case 2:
                                            if (rem_data->med_rem_location != NULL ||
                                                (rem_data->med_rem_location = (LLDP_OM_XMedRemLocationEntry_T *)calloc(1, sizeof(LLDP_OM_XMedRemLocationEntry_T))) != 0)
                                            {
                                                UI8_T   lci_len;
                                                LLDP_OM_XMedLocationCivicAddrCaTlv_T *ca_entry;
                                                UI8_T   *ca;

                                                if ((rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_civicAddress)) &&
                                                    port_config->lldp_med_location.civic_addr.ca_list.nbr_of_element != 0)
                                                {
                                                    LLDP_OM_FreeCivicAddrCaList(&rem_data->med_rem_location->civic_addr.ca_list);
                                                }
                                                else
                                                {
                                                    rem_data->med_rem_location->location_type_valid |= BIT_VALUE(VAL_lldpXMedRemLocationSubtype_civicAddress);
                                                }

                                                memset(&rem_data->med_rem_location->civic_addr, 0, sizeof(LLDP_OM_XMedLocationCivicAddr_T));
                                                rem_data->med_rem_location->civic_addr.what = tlv.value[6];
                                                memcpy(rem_data->med_rem_location->civic_addr.country_code, &tlv.value[7], 2);

                                                lci_len = location_id[0];
                                                lci_len -= 3;

                                                ca = &location_id[4];

                                                /* extract ca */
                                                while(lci_len > 0)
                                                {
                                                    ca_entry = 0;
                                                    /* create ca list*/

                                                    if(rem_data->med_rem_location->civic_addr.ca_list.nbr_of_element == 0)
                                                        LLDP_OM_CreateCivicAddrCaList(&rem_data->med_rem_location->civic_addr.ca_list);

                                                    /* allocate a ca_entry*/
                                                    ca_entry = (LLDP_OM_XMedLocationCivicAddrCaTlv_T *)malloc(sizeof(LLDP_OM_XMedLocationCivicAddrCaTlv_T));
                                                    if(ca_entry == 0)
                                                        continue;

                                                    memset(ca_entry, 0, sizeof(LLDP_OM_XMedLocationCivicAddrCaTlv_T));
                                                    ca_entry->ca_type = ca[0];
                                                    ca_entry->ca_length = ca[1];
                                                    memcpy(ca_entry->ca_value, &ca[2], ca[1]);

                                                    /* insert to the ca_list */
                                                    if (FALSE == LLDP_UTY_InsertElementToList(
                                                                    &rem_data->med_rem_location->civic_addr.ca_list, ca_entry))
                                                    {
                                                        free(ca_entry);
                                                    }
                                                    lci_len -= (2 + ca[1]);
                                                    ca += (2 + ca[1]);
                                                }
                                            }
                                            break;
                                        case 3:
                                            if (rem_data->med_rem_location != NULL ||
                                                (rem_data->med_rem_location = (LLDP_OM_XMedRemLocationEntry_T *)calloc(1, sizeof(LLDP_OM_XMedRemLocationEntry_T))) != 0)
                                            {
                                                rem_data->med_rem_location->location_type_valid |= BIT_VALUE(VAL_lldpXMedRemLocationSubtype_elin);
                                                memset(&rem_data->med_rem_location->elin_addr, 0, sizeof(LLDP_OM_XMedLocationElin_T));
                                                rem_data->med_rem_location->elin_addr.elin_len = location_id_len;
                                                memcpy(rem_data->med_rem_location->elin_addr.elin,
                                                       location_id,
                                                       location_id_len);
                                            }
                                        case 4:
                                        default:
                                            port_statistics->rx_tlvs_unrecognized_total++;
                                            break;
                                    }
                                    rem_data->lldp_med_cap_enabled |= (1 << 2);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_MED_EXT_POWER_VIA_MDI_TLV:
                                if(tlv.len == 7 && rem_data->med_rem_ext_power == NULL)
                                {
                                    if((rem_data->med_rem_ext_power = (LLDP_OM_XMedRemExtPowerEntry_T *)malloc(sizeof(LLDP_OM_XMedRemExtPowerEntry_T))) != 0)
                                    {
                                        UI16_T  tmp;

                                        memset(rem_data->med_rem_ext_power, 0, sizeof(LLDP_OM_XMedRemExtPowerEntry_T));
                                        rem_data->med_rem_ext_power->power_type = (tlv.value[4] & 0x80) ? VAL_lldpXMedRemXPoEDeviceType_unknown :
                                                                                  ((tlv.value[4] & 0x40) ? VAL_lldpXMedRemXPoEDeviceType_pdDevice : VAL_lldpXMedRemXPoEDeviceType_pseDevice);
                                        rem_data->med_rem_ext_power->power_source = ((tlv.value[4] & 0x30) >> 4) + 1;
                                        rem_data->med_rem_ext_power->power_priority = (tlv.value[4] & 0x0F) + 1;
                                        memcpy(&tmp, &tlv.value[5], 2);
                                        rem_data->med_rem_ext_power->power_value = L_STDLIB_Ntoh16(tmp);

                                        if(rem_data->med_rem_ext_power->power_type == VAL_lldpXMedRemXPoEDeviceType_pdDevice)
                                            rem_data->lldp_med_cap_enabled |= (1 << 4);
                                        else if(rem_data->med_rem_ext_power->power_type == VAL_lldpXMedRemXPoEDeviceType_pseDevice)
                                            rem_data->lldp_med_cap_enabled |= (1 << 3);
                                    }
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_MED_INV_HARDWARE_REVISION_TLV:
                                if(tlv.len <= 36)
                                {
                                    if(rem_data->med_rem_inventory == NULL)
                                    {
                                        rem_data->med_rem_inventory = (LLDP_OM_XMedRemInventoryEntry_T *)malloc(sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                        if(rem_data->med_rem_inventory != NULL)
                                            memset(rem_data->med_rem_inventory, 0, sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                    }
                                    if(rem_data->med_rem_inventory != NULL)
                                    {
                                        rem_data->med_rem_inventory->hardware_revision_len = tlv.len - 4;
                                        memcpy(rem_data->med_rem_inventory->hardware_revision, &tlv.value[4], rem_data->med_rem_inventory->hardware_revision_len);
                                    }
                                    rem_data->lldp_med_cap_enabled |= (1 << 5);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_MED_INV_FIRMWARE_REVISION_TLV:
                                if(tlv.len <= 36)
                                {
                                    if(rem_data->med_rem_inventory == NULL)
                                    {
                                        rem_data->med_rem_inventory = (LLDP_OM_XMedRemInventoryEntry_T *)malloc(sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                        if(rem_data->med_rem_inventory != NULL)
                                            memset(rem_data->med_rem_inventory, 0, sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                    }
                                    if(rem_data->med_rem_inventory != NULL)
                                    {
                                        rem_data->med_rem_inventory->firmware_revision_len = tlv.len - 4;
                                        memcpy(rem_data->med_rem_inventory->firmware_revision, &tlv.value[4], rem_data->med_rem_inventory->firmware_revision_len);
                                    }
                                    rem_data->lldp_med_cap_enabled |= (1 << 5);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_MED_INV_SOFTWARE_REVISION_TLV:
                                if(tlv.len <= 36)
                                {
                                    if(rem_data->med_rem_inventory == NULL)
                                    {
                                        rem_data->med_rem_inventory = (LLDP_OM_XMedRemInventoryEntry_T *)malloc(sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                        if(rem_data->med_rem_inventory != NULL)
                                            memset(rem_data->med_rem_inventory, 0, sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                    }
                                    if(rem_data->med_rem_inventory != NULL)
                                    {
                                        rem_data->med_rem_inventory->software_revision_len = tlv.len - 4;
                                        memcpy(rem_data->med_rem_inventory->software_revision, &tlv.value[4], rem_data->med_rem_inventory->software_revision_len);
                                    }
                                    rem_data->lldp_med_cap_enabled |= (1 << 5);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_MED_INV_SERIAL_NUM_TLV:
                                if(tlv.len <= 36)
                                {
                                    if(rem_data->med_rem_inventory == NULL)
                                    {
                                        rem_data->med_rem_inventory = (LLDP_OM_XMedRemInventoryEntry_T *)malloc(sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                        if(rem_data->med_rem_inventory != NULL)
                                            memset(rem_data->med_rem_inventory, 0, sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                    }
                                    if(rem_data->med_rem_inventory != NULL)
                                    {
                                        rem_data->med_rem_inventory->serial_num_len = tlv.len - 4;
                                        memcpy(rem_data->med_rem_inventory->serial_num, &tlv.value[4], rem_data->med_rem_inventory->serial_num_len);
                                    }
                                    rem_data->lldp_med_cap_enabled |= (1 << 5);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_MED_INV_MANUFACTURER_NAME_TLV:
                                if(tlv.len <= 36)
                                {
                                    if(rem_data->med_rem_inventory == NULL)
                                    {
                                        rem_data->med_rem_inventory = (LLDP_OM_XMedRemInventoryEntry_T *)malloc(sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                        if(rem_data->med_rem_inventory != NULL)
                                            memset(rem_data->med_rem_inventory, 0, sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                    }
                                    if(rem_data->med_rem_inventory != NULL)
                                    {
                                        rem_data->med_rem_inventory->manufaturer_name_len = tlv.len - 4;
                                        memcpy(rem_data->med_rem_inventory->manufaturer_name, &tlv.value[4], rem_data->med_rem_inventory->manufaturer_name_len);
                                    }
                                    rem_data->lldp_med_cap_enabled |= (1 << 5);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_MED_INV_MODEL_NAME_TLV:
                                if(tlv.len <= 36)
                                {
                                    if(rem_data->med_rem_inventory == NULL)
                                    {
                                        rem_data->med_rem_inventory = (LLDP_OM_XMedRemInventoryEntry_T *)malloc(sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                        if(rem_data->med_rem_inventory != NULL)
                                            memset(rem_data->med_rem_inventory, 0, sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                    }
                                    if(rem_data->med_rem_inventory != NULL)
                                    {
                                        rem_data->med_rem_inventory->model_name_len = tlv.len - 4;
                                        memcpy(rem_data->med_rem_inventory->model_name, &tlv.value[4], rem_data->med_rem_inventory->model_name_len);
                                    }
                                    rem_data->lldp_med_cap_enabled |= (1 << 5);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            case LLDP_TYPE_MED_ASSET_ID_TLV:
                                if(tlv.len <= 36)
                                {
                                    if(rem_data->med_rem_inventory == NULL)
                                    {
                                        rem_data->med_rem_inventory = (LLDP_OM_XMedRemInventoryEntry_T *)malloc(sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                        if(rem_data->med_rem_inventory != NULL)
                                            memset(rem_data->med_rem_inventory, 0, sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                                    }
                                    if(rem_data->med_rem_inventory != NULL)
                                    {
                                        rem_data->med_rem_inventory->asset_id_len = tlv.len - 4;
                                        memcpy(rem_data->med_rem_inventory->asset_id, &tlv.value[4], rem_data->med_rem_inventory->asset_id_len);
                                    }
                                    rem_data->lldp_med_cap_enabled |= (1 << 5);
                                }
                                else
                                {
                                    port_statistics->rx_tlvs_discarded_total++;
                                }
                                break;
                            default:
                                port_statistics->rx_tlvs_unrecognized_total++;
                                break;
                        }
                    }
#endif
                }
                break;
            case LLDP_TYPE_END_OF_LLDPDU_TLV:
                break;
            default:
                port_statistics->rx_tlvs_unrecognized_total++;
                break;
        }
        pdu = next_tlv;

    }while(tlv.type != LLDP_TYPE_END_OF_LLDPDU_TLV);


}/* End of LLDP_UTY_ConvertMsgToRemData */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ValidateLLDPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Validate LLDPDU
 * INPUT    : msg
 * OUTPUT   : UI32_T    *tlv_type_count -- number of each tlv type
 * RETURN   : None
 * NOTE     : Ref. to the description in 10.3.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_UTY_ValidateLLDPDU(LLDP_TYPE_MSG_T *msg, UI32_T *tlv_type_count)
{
    UI8_T           type = 0;
    UI16_T          len1 = 0;
    UI16_T          len2 = 0;
    UI16_T          len = 0;
    UI16_T          lport;
    UI32_T          pdu_len = 0;
    UI32_T          len_count = 0;
    unsigned char   *pdu;

    LLDP_OM_PortStatistics_T    *port_statistic;


    pdu = (UI8_T *)L_MM_Mref_GetPdu(msg->mem_ref, &pdu_len);

    lport = msg->lport;

    port_statistic = LLDP_OM_GetPortStatisticsEntryPtr(lport);

    {/* validate Chassis ID TLV*/
        type = ((char)(*pdu) & 0xFE ) >> 1;               /* get TLV type */
        len1 = (UI16_T) ((char)(*pdu) & 0x01) * 256;      /* get TLV length */
        len2 = (UI16_T) ((char)*(pdu + 1));
        len = len1 + len2;
        pdu = pdu + 2 + len;                            /* shift to the next TLV*/
        len_count = len_count + 2 + len;
        tlv_type_count[type]++;

        /* validate type and length */
        if(type != LLDP_TYPE_CHASSIS_ID_TLV)
        {
            port_statistic->rx_frames_discarded_total++;
            port_statistic->rx_frames_errors++;
            return LLDP_TYPE_VALIDATE_TLV_ERROR;
        }
        if(len < 2 || len > 256)
        {
            port_statistic->rx_frames_discarded_total++;
            port_statistic->rx_frames_errors++;
            return LLDP_TYPE_VALIDATE_TLV_ERROR;
        }
    }

    {/* validate Port ID TLV */
        type = ((*pdu) & 0xFE ) >> 1;               /* get TLV type */
        len1 = (UI16_T) ((*pdu) & 0x01) * 256;      /* get TLV length */
        len2 = (UI16_T) (*(pdu + 1));
        len = len1 + len2;
        pdu = pdu + 2 + len;                            /* shift to the next TLV*/
        len_count = len_count + 2 + len;
        tlv_type_count[type]++;

        /* validate type and length */
        if(type != LLDP_TYPE_PORT_ID_TLV)
        {
            port_statistic->rx_frames_discarded_total++;
            port_statistic->rx_frames_errors++;
            return LLDP_TYPE_VALIDATE_TLV_ERROR;
        }
        if(len < 2 || len > 256)
        {
            port_statistic->rx_frames_discarded_total++;
            port_statistic->rx_frames_errors++;
            return LLDP_TYPE_VALIDATE_TLV_ERROR;
        }
    }

    {/* validate Time To Live TLV */
        type = ((*pdu) & 0xFE ) >> 1;               /* get TLV type */
        len1 = (UI16_T) ((*pdu) & 0x01) * 256;      /* get TLV length */
        len2 = (UI16_T) (*(pdu + 1));
        len = len1 + len2;
        pdu = pdu + 2 + len;                        /* shift to the next TLV*/
        len_count = len_count + 2 + len;
        tlv_type_count[type]++;

        /* validate type and length */
        if(type != LLDP_TYPE_TIME_TO_LIVE_TLV)
        {
            port_statistic->rx_frames_discarded_total++;
            port_statistic->rx_frames_errors++;
            return LLDP_TYPE_VALIDATE_TLV_ERROR;
        }
        if(len != 2) /* just allow TTL field length = 2*/
        {
            port_statistic->rx_frames_discarded_total++;
            port_statistic->rx_frames_errors++;
            return LLDP_TYPE_VALIDATE_TLV_ERROR;
        }
    }

    while(1)
    {
        type = ((*pdu) & 0xFE ) >> 1;               /* get TLV type */
        len1 = (UI16_T) ((*pdu) & 0x01) * 256;      /* get TLV length */
        len2 = (UI16_T) (*(pdu + 1));
        len = len1 + len2;
        pdu = pdu + 2 + len;                            /* shift to the next TLV*/
        len_count = len_count + 2 + len;

        if(type >= 9 && type < 127)         /* reserved TLV */
            type = LLDP_TYPE_UNKNOWN_TLV;
        if(type == 127)                     /* organizatinoal specified TLV*/
            type = LLDP_TYPE_ORG_SPEC_TLV;

        tlv_type_count[type]++;

        if(len_count > pdu_len)
        {
            port_statistic->rx_frames_discarded_total++;
            port_statistic->rx_tlvs_discarded_total++;
            return LLDP_TYPE_VALIDATE_TLV_ERROR;
        }

        if(type == LLDP_TYPE_END_OF_LLDPDU_TLV)
        {
            if(len != 0 ) return LLDP_TYPE_VALIDATE_TLV_ERROR;
            break;
        }
    }

    if(tlv_type_count[LLDP_TYPE_CHASSIS_ID_TLV] > 1 ||
       tlv_type_count[LLDP_TYPE_PORT_ID_TLV] > 1    ||
       tlv_type_count[LLDP_TYPE_TIME_TO_LIVE_TLV] > 1 )
    {
        port_statistic->rx_frames_discarded_total++;
        port_statistic->rx_frames_errors++;
        return LLDP_TYPE_VALIDATE_TLV_ERROR;
    }

    return LLDP_TYPE_VALIDATE_TLV_OK;
}/* End of LLDP_UTY_ValidateLLDPDU */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_CompareRemManAddrList
 *-------------------------------------------------------------------------
 * PURPOSE  : Compare the remote man addr sort list
 * INPUT    : list1     -- remote address sort list 1
 *            list2     -- remote address sort list 2
 * OUTPUT   : None
 * RETUEN   : LLDP_TYPE_REM_MAN_ADDR_EQUAL
 *            LLDP_TYPE_REM_MAN_ADDR_DIFF
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static UI32_T LLDP_UTY_CompareRemManAddrList(L_SORT_LST_List_T *list1, L_SORT_LST_List_T *list2)
{
    UI32_T  result;
    BOOL_T  flag1, flag2;
    LLDP_OM_RemManAddrEntry_T   *input1, *input2;


    result = LLDP_TYPE_REM_MAN_ADDR_DIFF;

    flag1 = L_SORT_LST_Get_1st(list1, &input1);
    flag2 = L_SORT_LST_Get_1st(list2, &input2);
    while(1)
    {
        if(flag1 ^ flag2)
        {
            return result;
        }

        if(flag1)
        {
            if(input1->rem_man_addr_subtype != input2->rem_man_addr_subtype)
            {
                return result;
            }
            if(input1->rem_man_addr_len != input2->rem_man_addr_len)
            {
                return result;
            }
            if(memcmp(input1->rem_man_addr, input2->rem_man_addr, input1->rem_man_addr_len) != 0)
            {
                return result;
            }
            if(input1->rem_man_addr_if_subtype != input2->rem_man_addr_if_subtype)
            {
                return result;
            }
            if(input1->rem_man_addr_if_id != input2->rem_man_addr_if_id)
            {
                return result;
            }
            if(input1->rem_man_addr_oid_len != input2->rem_man_addr_oid_len)
            {
                return result;
            }
            if(memcmp(input1->rem_man_addr_oid, input2->rem_man_addr_oid, input1->rem_man_addr_oid_len) != 0)
            {
                return result;
            }
        }
        else
        {
            return LLDP_TYPE_REM_MAN_ADDR_EQUAL;
        }

        flag1 = L_SORT_LST_Get_Next(list1, &input1);
        flag2 = L_SORT_LST_Get_Next(list2, &input2);
    }
}/* End of LLDP_UTY_CompareRemManAddrList */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_CompareRemSys
 *-------------------------------------------------------------------------
 * PURPOSE  : Compare remote system information
 * INPUT    : input1        -- remote system entry 1
 *            input2        -- remote system entry 2
 * OUTPUT   : None
 * RETUEN   : LLDP_TYPE_REM_SYS_EQUAL
 *            LLDP_TYPE_REM_SYS_DIFF
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static UI32_T LLDP_UTY_CompareRemSys(LLDP_OM_RemSysEntry_T *input1, LLDP_OM_RemSysEntry_T *input2)
{
    UI32_T  result;

    result = LLDP_TYPE_REM_SYS_DIFF;
    if(input1 == NULL && input2 == NULL)
    {
        return LLDP_TYPE_REM_SYS_EQUAL;
    }
    else if((input1 == NULL && input2 != NULL) || (input1 != NULL && input2 == NULL))
    {
        return result;
    }

    if(input1->rem_port_desc_len != input2->rem_port_desc_len)
    {
        return result;
    }
    if(memcmp(input1->rem_port_desc, input2->rem_port_desc, input1->rem_port_desc_len))
    {
        return result;
    }
    if(input1->rem_sys_name_len != input2->rem_sys_name_len)
    {
        return result;
    }
    if(memcmp(input1->rem_sys_name, input2->rem_sys_name, input1->rem_sys_name_len))
    {
        return result;
    }
    if(input1->rem_sys_desc_len != input2->rem_sys_desc_len)
    {
        return result;
    }
    if(memcmp(input1->rem_sys_desc, input2->rem_sys_desc, input1->rem_sys_desc_len))
    {
        return result;
    }
    if(input1->rem_sys_cap_supported != input2->rem_sys_cap_supported)
    {
        return result;
    }
    if(input1->rem_sys_cap_enabled != input2->rem_sys_cap_enabled)
    {
        return result;
    }

    result = LLDP_TYPE_REM_SYS_EQUAL;
    return result;
}/* End of LLDP_UTY_CompareRemSys*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_CompareXdot1Entry
 * ------------------------------------------------------------------------
 * PURPOSE  : Check the difference between two rem_data
 * INPUT    : rem_data1         -- remote data entry 1
 *            rem_data2         -- remote data entry 2
 * OUTPUT   : None
 * RETUEN   : LLDP_TYPE_REMOTE_DATA_DIFF
 *            LLDP_TYPE_REMOTE_DATA_EQUAL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static UI32_T LLDP_UTY_CompareXdot1Entry(LLDP_OM_RemData_T *rem_data1, LLDP_OM_RemData_T *rem_data2)
{
    LLDP_OM_Xdot1RemProtoVlanEntry_T    *rem_proto_vlan_entry1, *rem_proto_vlan_entry2;
    LLDP_OM_Xdot1RemVlanNameEntry_T     *rem_vlan_name_entry1, *rem_vlan_name_entry2;
    LLDP_OM_Xdot1ProtocolEntry_T        *rem_protocol_entry1, *rem_protocol_entry2;
    UI32_T      result;
    BOOL_T      flag1, flag2;

    result = LLDP_TYPE_REMOTE_DATA_DIFF;

    /* check remote port vlan id */
    if(rem_data1->xdot1_rem_port_vlan_id != rem_data2->xdot1_rem_port_vlan_id)
    {
        return result;
    }

    /* check remote proto vlan list */
    if(rem_data1->xdot1_rem_proto_vlan_list.nbr_of_element != rem_data2->xdot1_rem_proto_vlan_list.nbr_of_element)
    {
        return result;
    }

    flag1 = L_SORT_LST_Get_1st(&rem_data1->xdot1_rem_proto_vlan_list, &rem_proto_vlan_entry1);
    flag2 = L_SORT_LST_Get_1st(&rem_data2->xdot1_rem_proto_vlan_list, &rem_proto_vlan_entry2);

    while(1)
    {
        if(flag1 ^ flag2)
        {
            return result;
        }
        if(flag1)
        {
            if(rem_proto_vlan_entry1->rem_proto_vlan_id != rem_proto_vlan_entry2->rem_proto_vlan_id)
            {
                return result;
            }
            if(rem_proto_vlan_entry1->rem_proto_vlan_supported != rem_proto_vlan_entry2->rem_proto_vlan_supported)
            {
                return result;
            }
            if(rem_proto_vlan_entry1->rem_proto_vlan_enabled != rem_proto_vlan_entry2->rem_proto_vlan_enabled)
            {
                return result;
            }
        }
        else
        {
            break;
        }
        flag1 = L_SORT_LST_Get_Next(&rem_data1->xdot1_rem_proto_vlan_list, &rem_proto_vlan_entry1);
        flag2 = L_SORT_LST_Get_Next(&rem_data2->xdot1_rem_proto_vlan_list, &rem_proto_vlan_entry2);
    }

    /* check remote vlan name list */
    if(rem_data1->xdot1_rem_vlan_name_list.nbr_of_element != rem_data2->xdot1_rem_vlan_name_list.nbr_of_element)
    {
        return result;
    }

    flag1 = L_SORT_LST_Get_1st(&rem_data1->xdot1_rem_vlan_name_list, &rem_vlan_name_entry1);
    flag2 = L_SORT_LST_Get_1st(&rem_data2->xdot1_rem_vlan_name_list, &rem_vlan_name_entry2);

    while(1)
    {
        if(flag1 ^ flag2)
        {
            return result;
        }
        if(flag1)
        {
            if(rem_vlan_name_entry1->rem_vlan_id != rem_vlan_name_entry2->rem_vlan_id)
            {
                return result;
            }
            if(rem_vlan_name_entry1->rem_vlan_name_len != rem_vlan_name_entry2->rem_vlan_name_len)
            {
                return result;
            }
            if(memcmp(rem_vlan_name_entry1->rem_vlan_name, rem_vlan_name_entry2->rem_vlan_name, rem_vlan_name_entry1->rem_vlan_name_len) != 0)
            {
                return result;
            }
        }
        else
        {
            break;
        }
        flag1 = L_SORT_LST_Get_Next(&rem_data1->xdot1_rem_vlan_name_list, &rem_vlan_name_entry1);
        flag2 = L_SORT_LST_Get_Next(&rem_data2->xdot1_rem_vlan_name_list, &rem_vlan_name_entry2);
    }

    /* check remote protocol list */
    if(rem_data1->xdot1_rem_protocol_list.nbr_of_element != rem_data2->xdot1_rem_protocol_list.nbr_of_element)
    {
        return result;
    }

    flag1 = L_SORT_LST_Get_1st(&rem_data1->xdot1_rem_protocol_list, &rem_protocol_entry1);
    flag2 = L_SORT_LST_Get_1st(&rem_data2->xdot1_rem_protocol_list, &rem_protocol_entry2);

    while(1)
    {
        if(flag1 ^ flag2)
        {
            return result;
        }
        if(flag1)
        {
            if(rem_protocol_entry1->rem_protocol_index!= rem_protocol_entry2->rem_protocol_index)
            {
                return result;
            }
            if(rem_protocol_entry1->rem_protocol_id_len != rem_protocol_entry2->rem_protocol_id_len)
            {
                return result;
            }
            if(memcmp(rem_protocol_entry1->rem_protocol_id, rem_protocol_entry2->rem_protocol_id, rem_protocol_entry1->rem_protocol_id_len) != 0)
            {
                return result;
            }
        }
        else
        {
            break;
        }
        flag1 = L_SORT_LST_Get_Next(&rem_data1->xdot1_rem_protocol_list, &rem_protocol_entry1);
        flag2 = L_SORT_LST_Get_Next(&rem_data2->xdot1_rem_protocol_list, &rem_protocol_entry2);
    }

    if (rem_data1->xdot1_rem_cn_entry != NULL && rem_data2->xdot1_rem_cn_entry != NULL)
    {
        if (memcmp(rem_data1->xdot1_rem_cn_entry, rem_data2->xdot1_rem_cn_entry,
                sizeof(LLDP_OM_Xdot1RemCnEntry_T)) != 0)
        {
#if (SYS_CPNT_CN == TRUE)
            LLDP_OM_SetTlvChangeDetect(LLDP_OM_DETECT_CHANGE_CN_TLV, TRUE);
#endif
            return result;
        }
    }
    else if (rem_data1->xdot1_rem_cn_entry != rem_data2->xdot1_rem_cn_entry)
    {
#if (SYS_CPNT_CN == TRUE)
        LLDP_OM_SetTlvChangeDetect(LLDP_OM_DETECT_CHANGE_CN_TLV, TRUE);
#endif
        return result;
    }

    return LLDP_TYPE_REMOTE_DATA_EQUAL;
}/* End of LLDP_UTY_CompareXdot1Entry */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_CompareXdot3Entry
 * ------------------------------------------------------------------------
 * PURPOSE  : Check the difference between two rem_data
 * INPUT    : rem_data1         -- remote data entry 1
 *            rem_data2         -- remote data entry 2
 * OUTPUT   : None
 * RETUEN   : LLDP_TYPE_REMOTE_DATA_DIFF
 *            LLDP_TYPE_REMOTE_DATA_EQUAL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static UI32_T LLDP_UTY_CompareXdot3Entry(LLDP_OM_RemData_T *rem_data1, LLDP_OM_RemData_T *rem_data2)
{
    UI32_T  result;

    result = LLDP_TYPE_REMOTE_DATA_DIFF;

    if(!(rem_data1->xdot3_rem_link_agg_entry == 0 && rem_data2->xdot3_rem_link_agg_entry == 0))
    {
        if(rem_data1->xdot3_rem_link_agg_entry != 0 && rem_data2->xdot3_rem_link_agg_entry != 0)
        {
            if(memcmp(rem_data1->xdot3_rem_link_agg_entry,
                      rem_data2->xdot3_rem_link_agg_entry,
                      sizeof(LLDP_OM_Xdot3RemLinkAggEntry_T)) != 0)
                return result;
        }
        else
        {
            return result;
        }
    }

    if(rem_data1->xdot3_rem_max_frame_size != rem_data2->xdot3_rem_max_frame_size)
        return result;

    if(!(rem_data1->xdot3_rem_port_entry == 0 && rem_data2->xdot3_rem_port_entry == 0))
    {
        if(rem_data1->xdot3_rem_port_entry != 0 && rem_data2->xdot3_rem_port_entry != 0)
        {
            if(memcmp(rem_data1->xdot3_rem_port_entry,
                      rem_data2->xdot3_rem_port_entry,
                      sizeof(LLDP_OM_Xdot3RemPortEntry_T)) != 0)
                return result;
        }
        else
        {
            return result;
        }
    }

    if(!(rem_data1->xdot3_rem_power_entry == 0 && rem_data2->xdot3_rem_power_entry == 0))
    {
        if(rem_data1->xdot3_rem_power_entry != 0 && rem_data2->xdot3_rem_power_entry != 0)
        {
            if(memcmp(rem_data1->xdot3_rem_power_entry,
                      rem_data2->xdot3_rem_power_entry,
                      sizeof(LLDP_OM_Xdot3RemPowerEntry_T)) != 0)
                return result;
        }
        else
        {
            return result;
        }
    }

    return LLDP_TYPE_REMOTE_DATA_EQUAL;
}


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_CompareMedEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : Check the difference between two rem_data
 * INPUT    : rem_data1         -- remote data entry 1
 *            rem_data2         -- remote data entry 2
 * OUTPUT   : None
 * RETUEN   : LLDP_TYPE_REMOTE_DATA_DIFF
 *            LLDP_TYPE_REMOTE_DATA_EQUAL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static UI32_T LLDP_UTY_CompareMedEntry(LLDP_OM_RemData_T *rem_data1, LLDP_OM_RemData_T *rem_data2)
{
    UI32_T  result;

    result = LLDP_TYPE_REMOTE_DATA_DIFF;
    if(rem_data1->lldp_med_cap_enabled != rem_data2->lldp_med_cap_enabled)
        return result;
    if(rem_data1->lldp_med_cap_sup != rem_data2->lldp_med_cap_sup)
        return result;
    if(rem_data1->lldp_med_device_type != rem_data2->lldp_med_device_type)
        return result;

    if(!(rem_data1->med_rem_ext_power == 0 && rem_data2->med_rem_ext_power == 0))
    {
        if(rem_data1->med_rem_ext_power != 0 && rem_data2->med_rem_ext_power != 0)
        {
            if(memcmp(rem_data1->med_rem_ext_power,
                      rem_data2->med_rem_ext_power,
                      sizeof(LLDP_OM_XMedRemExtPowerEntry_T)) != 0)
            {
                return result;
            }
        }
        else
        {
            return result;
        }
    }

    if(!(rem_data1->med_rem_network_policy == 0 && rem_data2->med_rem_network_policy == 0))
    {
        if(rem_data1->med_rem_network_policy != 0 && rem_data2->med_rem_network_policy != 0)
        {
            if(memcmp(rem_data1->med_rem_network_policy,
                      rem_data2->med_rem_network_policy,
                      sizeof(LLDP_OM_XMedRemNetworkPolicyEntry_T)) != 0)
            {
                return result;
            }
        }
        else
        {
            return result;
        }
    }

    if(!(rem_data1->med_rem_location == 0 && rem_data2->med_rem_location == 0))
    {
        if (rem_data1->med_rem_location != 0 && rem_data2->med_rem_location != 0 &&
            rem_data1->med_rem_location->location_type_valid == rem_data2->med_rem_location->location_type_valid)
        {
            if (rem_data1->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_coordinateBased))
            {
                if (memcmp(&rem_data1->med_rem_location->coord_addr,
                          &rem_data2->med_rem_location->coord_addr,
                          sizeof(LLDP_OM_XMedLocationCoordinated_T)) != 0)
                {
                    return result;
                }
            }
            if (rem_data1->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_civicAddress))
            {
                LLDP_OM_XMedLocationCivicAddrCaTlv_T    *ca_entry1, *ca_entry2;
                BOOL_T  flag1, flag2;

                if (rem_data1->med_rem_location->civic_addr.what != rem_data2->med_rem_location->civic_addr.what ||
                    rem_data1->med_rem_location->civic_addr.country_code[0] != rem_data2->med_rem_location->civic_addr.country_code[0] ||
                    rem_data1->med_rem_location->civic_addr.country_code[1] != rem_data2->med_rem_location->civic_addr.country_code[1])
                {
                    return result;
                }

                flag1 = L_SORT_LST_Get_1st(&rem_data1->med_rem_location->civic_addr.ca_list, &ca_entry1);
                flag2 = L_SORT_LST_Get_1st(&rem_data2->med_rem_location->civic_addr.ca_list, &ca_entry2);
                while(1)
                {
                    if(flag1 != flag2)
                        return result;
                    if(flag1)
                    {
                        if(memcmp(ca_entry1, ca_entry2, sizeof(LLDP_OM_XMedLocationCivicAddrCaTlv_T)) != 0)
                        {
                            return result;
                        }
                    }
                    else
                    {
                        break;
                    }

                    flag1 = L_SORT_LST_Get_Next(&rem_data1->med_rem_location->civic_addr.ca_list, &ca_entry1);
                    flag2 = L_SORT_LST_Get_Next(&rem_data1->med_rem_location->civic_addr.ca_list, &ca_entry2);
                }
            }
            if (rem_data1->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_elin))
            {
                if (memcmp(&rem_data1->med_rem_location->elin_addr,
                          &rem_data2->med_rem_location->elin_addr,
                          sizeof(LLDP_OM_XMedLocationElin_T)) != 0)
                {
                    return result;
                }
            }
        }
        else
        {
            return result;
        }
    }

    if(!(rem_data1->med_rem_inventory == 0 && rem_data2->med_rem_inventory == 0))
    {
        if(rem_data1->med_rem_inventory != 0 && rem_data2->med_rem_inventory != 0)
        {
            if(memcmp(rem_data1->med_rem_inventory,
                      rem_data2->med_rem_inventory,
                      sizeof(LLDP_OM_XMedRemInventoryEntry_T)) != 0)
            {
                return result;
            }
        }
        else
        {
            return result;
        }
    }

    return LLDP_TYPE_REMOTE_DATA_EQUAL;

}

#if(SYS_CPNT_DCBX == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_CompareXdcbxEntry
 * ------------------------------------------------------------------------
 * PURPOSE  : Check the difference between two rem_data
 * INPUT    : rem_data1         -- remote data entry 1
 *            rem_data2         -- remote data entry 2
 * OUTPUT   : None
 * RETUEN   : LLDP_TYPE_REMOTE_DATA_DIFF
 *            LLDP_TYPE_REMOTE_DATA_EQUAL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static UI32_T LLDP_UTY_CompareXdcbxEntry(LLDP_OM_RemData_T *rem_data1, LLDP_OM_RemData_T *rem_data2)
{
    UI32_T  result;
    BOOL_T flag1, flag2;
    LLDP_OM_XDcbxRemAppPriorityEntry_T rem_app_pri_entry1, rem_app_pri_entry2;

    result = LLDP_TYPE_REMOTE_DATA_EQUAL;

    /* check ETS config TLV */
    if(!(rem_data1->dcbx_rem_ets_config_entry == 0 && rem_data2->dcbx_rem_ets_config_entry == 0))
    {
        if(rem_data1->dcbx_rem_ets_config_entry != 0 && rem_data2->dcbx_rem_ets_config_entry != 0)
        {
            if(memcmp(rem_data1->dcbx_rem_ets_config_entry,
                      rem_data2->dcbx_rem_ets_config_entry,
                      sizeof(LLDP_OM_XDcbxRemEtsConfigEntry_T)) != 0)
            {
                result = LLDP_TYPE_REMOTE_DATA_DIFF;
            }
        }
        else
        {
            result = LLDP_TYPE_REMOTE_DATA_DIFF;
        }
    }

    /* check ETS recommend TLV */
    if(!(rem_data1->dcbx_rem_ets_recommend_entry == 0 && rem_data2->dcbx_rem_ets_recommend_entry == 0))
    {
        if(rem_data1->dcbx_rem_ets_recommend_entry != 0 && rem_data2->dcbx_rem_ets_recommend_entry != 0)
        {
            if(memcmp(rem_data1->dcbx_rem_ets_recommend_entry,
                      rem_data2->dcbx_rem_ets_recommend_entry,
                      sizeof(LLDP_OM_XDcbxRemEtsRecommendEntry_T)) != 0)
            {
                result = LLDP_TYPE_REMOTE_DATA_DIFF;
            }
        }
        else
        {
            result = LLDP_TYPE_REMOTE_DATA_DIFF;
        }
    }


    /* check PFC config TLV */
    if(!(rem_data1->dcbx_rem_pfc_config_entry == 0 && rem_data2->dcbx_rem_pfc_config_entry == 0))
    {
        if(rem_data1->dcbx_rem_pfc_config_entry != 0 && rem_data2->dcbx_rem_pfc_config_entry != 0)
        {
            if(memcmp(rem_data1->dcbx_rem_pfc_config_entry,
                      rem_data2->dcbx_rem_pfc_config_entry,
                      sizeof(LLDP_OM_XDcbxRemPfcConfigEntry_T)) != 0)
            {
                result = LLDP_TYPE_REMOTE_DATA_DIFF;
            }
        }
        else
        {
            result = LLDP_TYPE_REMOTE_DATA_DIFF;
        }
    }

       /* check remote application priority list */
    if(rem_data1->dcbx_rem_app_pri_list.nbr_of_element != rem_data2->dcbx_rem_app_pri_list.nbr_of_element)
    {
        result = LLDP_TYPE_REMOTE_DATA_DIFF;
        return result;
    }

    flag1 = L_SORT_LST_Get_1st(&rem_data1->dcbx_rem_app_pri_list, &rem_app_pri_entry1);
    flag2 = L_SORT_LST_Get_1st(&rem_data2->dcbx_rem_app_pri_list, &rem_app_pri_entry2);

    while(1)
    {
        if(flag1 ^ flag2)
        {
            result = LLDP_TYPE_REMOTE_DATA_DIFF;
            return result;
        }
        if(flag1)
        {
            if(rem_app_pri_entry1.rem_protocol_id != rem_app_pri_entry2.rem_protocol_id)
            {
                result = LLDP_TYPE_REMOTE_DATA_DIFF;
                return result;
            }
            if(rem_app_pri_entry1.rem_sel != rem_app_pri_entry2.rem_sel)
            {
                result = LLDP_TYPE_REMOTE_DATA_DIFF;
                return result;
            }
            if(rem_app_pri_entry1.rem_priority != rem_app_pri_entry2.rem_priority)
            {
                result = LLDP_TYPE_REMOTE_DATA_DIFF;
                return result;
            }
        }
        else
        {
            break;
        }
        flag1 = L_SORT_LST_Get_Next(&rem_data1->dcbx_rem_app_pri_list, &rem_app_pri_entry1);
        flag2 = L_SORT_LST_Get_Next(&rem_data2->dcbx_rem_app_pri_list, &rem_app_pri_entry2);
    }

    return result;
}

#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_DiffRemData
 * ------------------------------------------------------------------------
 * PURPOSE  : Check the difference between two rem_data
 * INPUT    : rem_data1         -- remote data entry 1
 *            rem_data2         -- remote data entry 2
 * OUTPUT   : None
 * RETUEN   : LLDP_TYPE_REMOTE_DATA_DIFFERENT
 *            LLDP_TYPE_REMOTE_DATA_EQUAL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static UI32_T LLDP_UTY_DiffRemData(LLDP_OM_RemData_T *rem_data1, LLDP_OM_RemData_T *rem_data2)
{
    UI32_T  result;
    LLDP_OM_RemSysEntry_T       *systmp1, *systmp2;
    L_SORT_LST_List_T   *man_list1, *man_list2;

    systmp1 = rem_data1->rem_sys_entry;
    systmp2 = rem_data2->rem_sys_entry;
    man_list1 = &rem_data1->rem_man_addr_list;
    man_list2 = &rem_data2->rem_man_addr_list;

    result = LLDP_TYPE_REMOTE_DATA_DIFF;


    /* Compare remote data content */
    if(rem_data1->lport != rem_data2->lport)
    {
        return result;
    }

    if(LLDP_UTY_CompareRemManAddrList(man_list1, man_list2) != LLDP_TYPE_REM_MAN_ADDR_EQUAL)
    {
#if 0
        /* if the device is telephone and its management address is changed */
        if((systmp2->rem_sys_cap_enabled & (1 << VAL_lldpRemSysCapEnabled_telephone)) != 0)
        {
            LLDP_TYPE_NotifyTelephone_T   *notify_entry;
            LLDP_OM_RemManAddrEntry_T     *orig_rem_man_addr, *curr_rem_man_addr;
            L_SORT_LST_List_T             *notify_list;

            notify_entry = (LLDP_TYPE_NotifyTelephone_T *)malloc(sizeof(LLDP_TYPE_NotifyTelephone_T));
            memset(notify_entry, 0, sizeof(LLDP_TYPE_NotifyTelephone_T));

            notify_entry->lport = rem_data2->lport;
            notify_entry->time_mark = rem_data2->time_mark;
            memcpy(notify_entry->mac, rem_data2->rem_chassis_id, 6);

            if(L_SORT_LST_Get_1st(man_list1, &curr_rem_man_addr))
            {
                notify_entry->curr_addr_subtype = curr_rem_man_addr->rem_man_addr_subtype;
                notify_entry->curr_addr_len = curr_rem_man_addr->rem_man_addr_len;
                memcpy(notify_entry->curr_addr, curr_rem_man_addr->rem_man_addr, curr_rem_man_addr->rem_man_addr_len);
            }

            if(L_SORT_LST_Get_1st(man_list2, &orig_rem_man_addr))
            {
                notify_entry->orig_addr_subtype = orig_rem_man_addr->rem_man_addr_subtype;
                notify_entry->orig_addr_len = orig_rem_man_addr->rem_man_addr_len;
                memcpy(notify_entry->orig_addr, orig_rem_man_addr->rem_man_addr, curr_rem_man_addr->rem_man_addr_len);
            }

            notify_list = LLDP_OM_GetNotifyTelephoneAddrChangedListPtr();
            if(!L_SORT_LST_Set(notify_list, &notify_entry))
            {
                free(notify_entry);
            }

        }
#endif
        return result;
    }

    if(LLDP_UTY_CompareRemSys(systmp1, systmp2) != LLDP_TYPE_REM_SYS_EQUAL)
    {
        return result;
    }

    if(LLDP_UTY_CompareXdot1Entry(rem_data1, rem_data2) != LLDP_TYPE_REMOTE_DATA_EQUAL)
    {
        return result;
    }

    if(LLDP_UTY_CompareXdot3Entry(rem_data1, rem_data2) != LLDP_TYPE_REMOTE_DATA_EQUAL)
    {
        return result;
    }

    if(LLDP_UTY_CompareMedEntry(rem_data1, rem_data2) != LLDP_TYPE_REMOTE_DATA_EQUAL)
    {
        return result;
    }

#if(SYS_CPNT_DCBX == TRUE)
    if(LLDP_UTY_CompareXdcbxEntry(rem_data1, rem_data2) != LLDP_TYPE_REMOTE_DATA_EQUAL)
    {
        return result;
    }
#endif

    return LLDP_TYPE_REMOTE_DATA_EQUAL;
}/* End of LLDP_UTY_DiffRemData */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_UpdateRemDataTTL
 * ------------------------------------------------------------------------
 * PURPOSE  : Refresh the age time in the remote data
 * INPUT    : rem_data          -- remote data to be updated
 * OUTPUT   : None
 * RETUEN   : LLDP_TYPE_OM_UPDATE_TTL_OK
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T LLDP_UTY_UpdateRemDataTTL(LLDP_OM_RemData_T *rem_data)
{
    LLDP_OM_AgeSeqDelete(rem_data);
    LLDP_OM_AgeSeqInsert(rem_data);
    return LLDP_TYPE_RETURN_OK;
}/* End of LLDP_OM_UpdateRemoteDataTTL */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_InsertRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : determine if received lldpdu should be insert into om
 * INPUT    : LLDP_TYPE_MSG_T   *msg        -- incoming message
 *            UI32_T            *tlv_count  -- number of each tlv type
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_ERROR /
 *            LLDP_TYPE_RETURN_TOO_MANY_NEIGHBOR /
 *            LLDP_TYPE_RETURN_OK
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_UTY_InsertRemData(LLDP_TYPE_MSG_T *msg, UI32_T *tlv_type_count)
{
    LLDP_OM_RemData_T        *tmp_rem_data;
    LLDP_OM_RemData_T        *old_rem_data;
    LLDP_OM_RemData_T        *new_rem_data;
    LLDP_OM_PortStatistics_T *port_stat;
    LLDP_OM_Statistics_T     *sys_stat;

    sys_stat = LLDP_OM_GetStatisticsPtr();
    port_stat = LLDP_OM_GetPortStatisticsEntryPtr(msg->lport);

    tmp_rem_data = (LLDP_OM_RemData_T*)malloc(sizeof(LLDP_OM_RemData_T));
    if (tmp_rem_data == NULL)
    {
        sys_stat->rem_table_drops++;
        port_stat->rx_frames_discarded_total++;
        return LLDP_TYPE_RETURN_ERROR;
    }

    memset(tmp_rem_data, 0, sizeof(LLDP_OM_RemData_T));
    LLDP_UTY_ConvertMsgToRemData(msg, tmp_rem_data, tlv_type_count);
    port_stat->rx_frames_total++;

    if (LLDP_OM_RemDataExist(tmp_rem_data, &old_rem_data))
    {
        #if RX_DEBUG_PRINT
        {
            printf("Rem data in DB");
        }
        #endif

        if (tmp_rem_data->rx_info_ttl == 0)
        {
            LLDP_OM_DeleteRemData(old_rem_data, LLDP_OM_DELETE_REASON_RXSHUTDOWN);
            LLDP_OM_ReleaseConvertMem(tmp_rem_data);
            free(tmp_rem_data);
            return LLDP_TYPE_RETURN_OK;
        }

        if (LLDP_UTY_DiffRemData(tmp_rem_data, old_rem_data) == LLDP_TYPE_REMOTE_DATA_DIFF)
        {
            #if RX_DEBUG_PRINT
            {
                printf("Data different");
            }
            #endif

            /* delete old */
            LLDP_OM_DeleteRemData_EX(tmp_rem_data, old_rem_data, LLDP_OM_DELETE_REASON_UPDATE);

            /* insert new */
            if (LLDP_OM_AllocRemData(&new_rem_data) != LLDP_TYPE_RETURN_OK)
            {
                #if RX_DEBUG_PRINT
                {
                    printf("Alloc rem data error");
                }
                #endif

                sys_stat->rem_table_drops++;
                port_stat->rx_frames_discarded_total++;
                LLDP_OM_ReleaseConvertMem(tmp_rem_data);
                free(tmp_rem_data);
                return LLDP_TYPE_RETURN_ERROR;
            }

            memcpy(new_rem_data, tmp_rem_data, sizeof(LLDP_OM_RemData_T));

            if (LLDP_OM_InsertRemData(new_rem_data))
            {
                LLDP_ENGINE_SomethingChangedRemote(new_rem_data->lport);
            }
            else
            {
#if (SYS_CPNT_CN == TRUE)
                LLDP_OM_SetTlvChangeDetect(LLDP_OM_DETECT_CHANGE_CN_TLV, FALSE);
#endif
                LLDP_OM_FreeRemData(new_rem_data);
            }
        }
        else
        {
            #if RX_DEBUG_PRINT
            {
                printf("Data identical");
            }
            #endif

            /* update TTL */
            old_rem_data->rx_info_ttl = tmp_rem_data->rx_info_ttl;
            LLDP_UTY_UpdateRemDataTTL(old_rem_data);

            LLDP_OM_ReleaseConvertMem(tmp_rem_data);
        }
    }
    else
    {
        #if RX_DEBUG_PRINT
        {
            printf("Rem data is a new record");
        }
        #endif

        if (tmp_rem_data->rx_info_ttl == 0)
        {
            LLDP_OM_ReleaseConvertMem(tmp_rem_data);
            free(tmp_rem_data);
            return LLDP_TYPE_RETURN_OK;
        }

        /* check if there is sufficient space */
        if (LLDP_OM_GetRemDataPoolFreeNum() == 0)
        {
            #if RX_DEBUG_PRINT
            {
                printf("Remote Data pool empty");
            }
            #endif

            sys_stat->rem_table_drops++;
            port_stat->rx_frames_discarded_total++;
            LLDP_OM_ReleaseConvertMem(tmp_rem_data);
            free(tmp_rem_data);
            return LLDP_TYPE_RETURN_TOO_MANY_NEIGHBOR;
        }

        /* check if there is sufficient space in port list */
        if (LLDP_OM_GetPortRemDataNum(msg->lport) ==
            LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT)
        {
            sys_stat->rem_table_drops++;
            port_stat->rx_frames_discarded_total++;
            LLDP_OM_ReleaseConvertMem(tmp_rem_data);
            free(tmp_rem_data);
            return LLDP_TYPE_RETURN_TOO_MANY_NEIGHBOR;
        }

        if (LLDP_OM_AllocRemData(&new_rem_data) != LLDP_TYPE_RETURN_OK)
        {
            #if RX_DEBUG_PRINT
            {
                printf("Alloc rem data error");
            }
            #endif

            sys_stat->rem_table_drops++;
            port_stat->rx_frames_discarded_total++;
            LLDP_OM_ReleaseConvertMem(tmp_rem_data);
            free(tmp_rem_data);
            return LLDP_TYPE_RETURN_ERROR;
        }

        memcpy(new_rem_data, tmp_rem_data, sizeof(LLDP_OM_RemData_T));

#if (SYS_CPNT_CN == TRUE)
        if (new_rem_data->xdot1_rem_cn_entry != NULL)
        {
            LLDP_OM_SetTlvChangeDetect(LLDP_OM_DETECT_CHANGE_CN_TLV, TRUE);
        }
#endif

        if (LLDP_OM_InsertRemData(new_rem_data))
        {
            LLDP_ENGINE_SomethingChangedRemote(new_rem_data->lport);
        }
        else
        {
            LLDP_OM_FreeRemData(new_rem_data);
        }
    }

    free(tmp_rem_data);
    return LLDP_TYPE_RETURN_OK;
}/* End of LLDP_UTY_InsertRemData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructNormalLLDPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct normal LLDPDU
 * INPUT    : sys_config
 *            port_config
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_UTY_ConstructNormalLLDPDU(LLDP_OM_SysConfigEntry_T *sys_config, LLDP_OM_PortConfigEntry_T *port_config)
{
    static UI8_T   pdu[1500];      /* max payload length of L2 frame */
    UI16_T  pdu_len = 0;        /* use to count all payload size */

    memset(pdu, 0, 1500);
    LLDP_UTY_ConstructMandatoryTLV(sys_config, port_config, pdu, &pdu_len);
    LLDP_UTY_ConstructBasicTLV(sys_config, port_config, pdu, &pdu_len);
    LLDP_UTY_ConstructManAddrTLV(sys_config, port_config, pdu, &pdu_len);
    LLDP_UTY_ConstructOrgSpecTLV(port_config, pdu, &pdu_len);
    LLDP_UTY_ConstructEndOfPduTLV(pdu, &pdu_len);

    /* Send packet */
    LLDP_UTY_SendLLDPDU(port_config->lport_num, pdu, pdu_len);


}/* End of LLDP_UTY_ConstructNormalLLDPDU */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_ConstructShutdownLLDPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct shutdown LLDPDU
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_UTY_ConstructShutdownLLDPDU(UI32_T lport)
{
    UI8_T   pdu[24] = {0};
    UI16_T  pdu_len = 0;
    UI8_T   *tlv;

    tlv = &pdu[0];
    /* 1. insert chassis id tlv */
    {
        tlv[0] = LLDP_TYPE_CHASSIS_ID_TLV << 1 ;   /* tlv type = chassis_id */
        tlv[1] = 1 + 6;     /* tlv len = len(chassis_id_subtype) + len(chassis_id)*/
        tlv[2] = 4;         /* chassis_id_subtype = mac addr */
        SWCTRL_GetCpuMac(&tlv[3]);
        pdu_len += 9;
        tlv += 9;
    }

    /* 2. insert port id tlv */
    {
        tlv[0] = LLDP_TYPE_PORT_ID_TLV << 1;    /* tlv type = chassis_id */
        tlv[1] = 1 + 6;     /* tlv len = len(port_id_subtype) + len(port_id) */
        tlv[2] = 3;         /* port_id_subtype = mac addr */
        SWCTRL_GetPortMac(lport, &tlv[3]);
        pdu_len +=9;
        tlv+= 9;
    }

    /* 3. insert Time to Live tlv */
    {
        tlv[0] = LLDP_TYPE_TIME_TO_LIVE_TLV << 1;
        tlv[1] = 2;
        /* may have alignment protblem */
        *(UI16_T *)(&tlv[2]) = 0;
        pdu_len += 4;
        tlv += 4;
    }
    /* 4. insert end of LLDPDU tlv */
    {
        tlv[0] = LLDP_TYPE_END_OF_LLDPDU_TLV << 1;
        tlv[1] = 0;
        pdu_len += 2;
    }

    /* 5. send packet*/
    LLDP_UTY_SendLLDPDU(lport, pdu, pdu_len);

}/* End of LLDP_UTY_ConstructShutdownLLDPDU */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_GetNextLogicalPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next logical port
 * INPUT    : lport
 * OUTPUT   : lport
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_UTY_GetNextLogicalPort(UI32_T *lport)
{
    SWCTRL_PortEntry_T port_entry;
    UI32_T unit = 0, port = 0, trunk_id = 0;

    memset(&port_entry, 0, sizeof(port_entry));
    port_entry.port_index = *lport;

    while(SWCTRL_GetNextPortEntry(&port_entry))
    {
        if(SWCTRL_LPORT_TRUNK_PORT ==
                SWCTRL_LogicalPortToUserPort(port_entry.port_index, &unit, &port, &trunk_id))
        {
            return FALSE;
        }
        *lport = port_entry.port_index;
        return TRUE;
    }

#if 0
    UI32_T  trunk_id = 0;

    while(SWCTRL_GetNextLogicalPort(lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if(SWCTRL_LogicalPortIsTrunkPort(*lport))
        {
            TRK_OM_IfindexToTrunkId(*lport, &trunk_id);
            if(TRK_PMGR_GetTrunkMemberCounts(trunk_id) == 0)
                continue;
        }

        return TRUE;

    }
#endif
    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_UTY_LogicalPortExisting
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next logical port
 * INPUT    : lport
 * OUTPUT   : lport
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_UTY_LogicalPortExisting(UI32_T lport)
{
    SWCTRL_PortEntry_T port_entry;
    UI32_T unit = 0, port = 0, trunk_id = 0;

    memset(&port_entry, 0, sizeof(port_entry));
    port_entry.port_index = lport;

    if(SWCTRL_GetPortEntry(&port_entry))
    {
        if(SWCTRL_LPORT_TRUNK_PORT ==
                SWCTRL_LogicalPortToUserPort(port_entry.port_index, &unit, &port, &trunk_id))
        {
            return FALSE;
        }

        return TRUE;
    }
#if 0
    UI32_T  trunk_id = 0;

    if(SWCTRL_LogicalPortExisting(lport))
    {
        if(SWCTRL_LogicalPortIsTrunkPort(lport))
        {
            TRK_OM_IfindexToTrunkId(lport, &trunk_id);
            if(TRK_PMGR_GetTrunkMemberCounts(trunk_id) == 0)
                return FALSE;
        }

        return TRUE;
    }
#endif
    return FALSE;
}


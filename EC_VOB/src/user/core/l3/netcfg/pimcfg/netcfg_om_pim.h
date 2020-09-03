/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_OM_PIM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/05/18     --- hongliang.yang Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
#ifndef NETCFG_OM_PIM_H
#define NETCFG_OM_PIM_H

#include <sys_type.h>
#include "sysfun.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "netcfg_type.h"
#include "l_radix.h"

struct netcfg_om_pim_prefix
{
  UI8_T family;
  UI8_T prefixlen;
  UI8_T pad1;
  UI8_T pad2;
  UI32_T group;
  UI32_T src;
  UI8_T  flags;
};

struct netcfg_om_pim_crp_grprng
{
  /* Group Range prefix */
  struct prefix p;

  struct netcfg_om_pim_prefix *crp;

  /* Back pointer to Route-Table node */
  struct route_node *rn;
};


/* RP configuration: RP address and group list */
struct netcfg_om_pim_rp_conf
{
  struct netcfg_om_pim_rp_conf *next;
  struct netcfg_om_pim_rp_conf *prev;

  u_int8_t config;
#define NETCFG_OM_PIM_ST_RP_CONF_DEFAULT (1 << 0)
#define NETCFG_OM_PIM_ST_RP_CONF_ACL     (1 << 1)

  UI32_T rp_addr;

  /* Group list access list */
  char *grp_acl;
};

/* C-RP configuration  */
struct netcfg_om_pim_crp
{
  struct netcfg_om_pim_crp *next;
  struct netcfg_om_pim_crp *prev;

  /* C-RP interface name.  */
  char *crp_ifname;

  /* CRP configuration.  */
  u_char config;
#define NETCFG_OM_PIM_CRP_CONFIG_PRIORITY               (1 << 0)
#define NETCFG_OM_PIM_CRP_CONFIG_INTERVAL               (1 << 1)
#define NETCFG_OM_PIM_CRP_CONFIG_ACL                    (1 << 2)
#define NETCFG_OM_PIM_CRP_CONFIG_HOLDTIME               (1 << 3) /* Only from SNMP */
#define NETCFG_OM_PIM_CRP_CONFIG_GROUP                  (1 << 4) 
#define NETCFG_OM_PIM_CRP_CONFIG_DEFAULT_RANGE          (1 << 5)

  struct route_table *grp_range;
  UI32_T num_grp_range;

};

struct netcfg_om_pim_bsr
{
      /* BSR configuration.  */
      u_char configs;
#define NETCFG_OM_PIM_BSR_CONFIG_CANDIDATE              (1 << 0)
#define NETCFG_OM_PIM_BSR_CONFIG_PRIORITY               (1 << 1)
#define NETCFG_OM_PIM_BSR_CONFIG_HASH_MASKLEN           (1 << 2)
#define NETCFG_OM_PIM_BSR_CONFIG_CRP_CISCO_PRIFIX       (1 << 3)

    char *ifname;              
    /* My configured priority.  */
    u_char my_priority;
    /* My configured hash mask length.  */
    u_char my_hash_masklen;
    struct netcfg_om_pim_crp *crp_head;
    struct netcfg_om_pim_crp *crp_tail;

};

typedef struct NETCFG_OM_PIM_Instance_S
{
      UI32_T configs;
#define NETCFG_OM_PIM_CONFIG_JP_TIMER               (1 << 0)
#define NETCFG_OM_PIM_CONFIG_IGNORE_RP_SET_PRIORITY (1 << 1)
#define NETCFG_OM_PIM_CONFIG_SPT_SWITCH             (1 << 2)
#define NETCFG_OM_PIM_CONFIG_REG_SRC_ADDR           (1 << 3)
#define NETCFG_OM_PIM_CONFIG_REG_SRC_INTF           (1 << 4)
#define NETCFG_OM_PIM_CONFIG_REG_RATE_LIMIT         (1 << 5)
#define NETCFG_OM_PIM_CONFIG_REG_RP_REACH           (1 << 6)
#define NETCFG_OM_PIM_CONFIG_RP_REG_KAT             (1 << 7)
#define NETCFG_OM_PIM_CONFIG_REG_SUPP               (1 << 8)
#define NETCFG_OM_PIM_CONFIG_RP_REG_FILTER          (1 << 9)
#define NETCFG_OM_PIM_CONFIG_CISCO_REG_CKSUM        (1 << 10)
#define NETCFG_OM_PIM_CONFIG_MRT                    (1 << 11)
#define NETCFG_OM_PIM_CONFIG_SSM_DEFAULT            (1 << 12)
#define NETCFG_OM_PIM_CONFIG_SSM_ACL                (1 << 13)

    UI16_T   jp_periodic; /* used for jt_timer */
    UI8_T   *spt_switch;
    UI16_T   reg_rate_limit;
    UI16_T   rp_reg_kat;
    UI16_T   reg_suppression;
    UI8_T   *rp_reg_filter;
    UI8_T   *reg_checksum_filter;
    struct netcfg_om_pim_bsr bsr;    
    /* List of static RP configuration */
    struct netcfg_om_pim_rp_conf *st_rp_head;
    struct netcfg_om_pim_rp_conf *st_rp_tail;

}NETCFG_OM_PIM_Instance_T;

void NETCFG_OM_PIM_Init(void);
BOOL_T NETCFG_OM_PIM_UnSetPimRpCandidateGroupAddr(UI32_T vr_id,
                                                                   UI32_T instance,
                                                                   UI8_T *ifname,
                                                                   UI32_T groupAddr, 
                                                                   UI32_T maskAddr);
BOOL_T NETCFG_OM_PIM_SetPimRpCandidateGroupAddr(UI32_T vr_id,
                                                                   UI32_T instance,
                                                                   UI8_T *ifname,
                                                                   UI32_T groupAddr, 
                                                                   UI32_T maskAddr);
BOOL_T NETCFG_OM_PIM_UnSetPimRpCandidate( UI32_T vr_id,
                                                           UI32_T instance,
                                                           UI8_T* ifname );
BOOL_T NETCFG_OM_PIM_SetPimRpCandidate( UI32_T vr_id,
                                                       UI32_T instance,
                                                       UI8_T* ifname );
BOOL_T NETCFG_OM_PIM_GetRpCandidateGroupAddr(UI32_T vr_id,
                                                              UI32_T instance,
                                                              UI8_T* ifname,
                                                              UI32_T groupAddr,
                                                              UI32_T maskAddr);
BOOL_T NETCFG_OM_PIM_GetRpCandidate(UI32_T vr_id,
                                                  UI32_T instance,
                                                  UI8_T* ifname,
                                                  struct netcfg_om_pim_crp *candidate_rp);
UI32_T NETCFG_OM_PIM_SignalIpAddrDelete(UI32_T vr_id, 
                                                     UI32_T vrf_id, 
                                                     UI32_T ifindex,
                                                     UI32_T ip_addr, 
                                                     UI32_T ip_mask);
UI32_T NETCFG_OM_PIM_SignalIpAddrAdd(UI32_T vr_id, 
                                                   UI32_T vrf_id,
                                                   UI32_T ifindex,
                                                   UI32_T ip_addr,
                                                   UI32_T ip_mask);
BOOL_T NETCFG_OM_PIM_UpdateInstanceEntry(UI32_T vr_id, UI32_T instance, NETCFG_OM_PIM_Instance_T *entry);
BOOL_T NETCFG_OM_PIM_UpdateInterfaceEntry(UI32_T vr_id, NETCFG_TYPE_PIM_If_T *entry);
BOOL_T NETCFG_OM_PIM_GetInterfaceEntry(UI32_T vr_id,NETCFG_TYPE_PIM_If_T *entry);
BOOL_T NETCFG_OM_PIM_AddInterface(UI32_T vr_id,UI32_T ifindex);
void NETCFG_OM_PIM_DeleteInterface(UI32_T vr_id,UI32_T ifindex);

#endif/*NETCFG_OM_PIM_H*/

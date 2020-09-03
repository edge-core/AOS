/* MODULE NAME: vxlan_om.c
 * PURPOSE:
 *   Defines some database APIs for VXLAN.
 * NOTES:
 *   None.
 *
 * HISTORY:
 *    mm/dd/yy
 *    04/21/15 -- Kelly, Create
 *
 * Copyright(C) Accton Corporation, 2015
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "l_hisam.h"
#include "l_sort_lst.h"
#include "l_inet.h"
#include "vxlan_om.h"
#include "vxlan_type.h"
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* Field length definitions for HISAM table
 */
#define RVTEP_NODE_LEN                      sizeof(VXLAN_OM_RVtep_T)
#define RVTEP_KEY_LEN                       (4 + 4) /* VNI, IP */
#define VNI_NODE_LEN                        sizeof(VXLAN_OM_VNI_T)


/* constant definitions for HISAM
 */
#define INDEX_NBR                           100                 /* table is divided to 100 blocks  */
#define HISAM_N1                            30                  /* table balance threshold */
#define HISAM_N2                            135                 /* table balance threshold */
#define HASH_DEPTH                          4
#define M_RVTEP_NODE_NBR                    SYS_ADPT_VXLAN_MAX_NBR_OF_MC_FLOOD_ENTRY
#define M_RVTEP_HASH_NBR                    (M_RVTEP_NODE_NBR / 10)
#define U_RVTEP_NODE_NBR                    SYS_ADPT_VXLAN_MAX_NBR_OF_UC_FLOOD_ENTRY
#define U_RVTEP_HASH_NBR                    (U_RVTEP_NODE_NBR / 10)
#define U_RVTEP_NUMBER_OF_KEYS              1
#define M_RVTEP_NUMBER_OF_KEYS              1
#define VNI_NODE_NBR                        SYS_ADPT_VXLAN_MAX_NBR_VNI
#define VNI_HASH_NBR                        (VNI_NODE_NBR / 10)
#define VNI_NUMBER_OF_KEYS                  2

/* key index for HISAM table
 */
#define U_RVTEP_VNI_IP_KIDX                 0 /* VNI+IP for UC RVTEP */
#define M_RVTEP_VNI_KIDX                    0 /* VNI for MC RVTEP */
#define VNI_KIDX                            0 /* VNI for VLAN-VNI */
#define VNI_VFI_KIDX                        1 /* VFI for VLAN-VNI */

/* MACRO FUNCTION DECLARATIONS
 */
#define VXLAN_OM_ENTER_CRITICAL_SECTION()                                                               \
{                                                                                                       \
    SYSFUN_TakeSem(vxlan_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);                                       \
}
#define VXLAN_OM_LEAVE_CRITICAL_SECTION()                                                               \
{                                                                                                       \
    SYSFUN_GiveSem(vxlan_om_sem_id);                                                                    \
}
#define VXLAN_OM_DEBUG_PRINTF_RVTEP(__flag__, __rvtep_p__)                                              \
{                                                                                                       \
    if(VXLAN_OM_GetDebug(__flag__) == TRUE)                                                             \
    {                                                                                                   \
        BACKDOOR_MGR_Printf("\r\n[%s%d] vni[%lu], vfi[0x%lx] \r\n"                                      \
               " d_ip[%d.%d.%d.%d], s_ip[%d.%d.%d.%d]  nexthop_cnt[%lu] \r\n"                    \
               " vid[%u], lport[%lu], uc_vxlan_port[%lx], mc_vxlan_port[%lx] \r\n"                      \
               " mac[%02X:%02X:%02X:%02X:%02X:%02X]\r\n",                                               \
        __FUNCTION__, __LINE__,                                                                         \
        (unsigned long)__rvtep_p__->vni, (unsigned long)__rvtep_p__->vfi,                               \
        __rvtep_p__->ip.addr[0], __rvtep_p__->ip.addr[1],                                               \
        __rvtep_p__->ip.addr[2], __rvtep_p__->ip.addr[3],                                               \
        __rvtep_p__->s_ip.addr[0], __rvtep_p__->s_ip.addr[1],                                           \
        __rvtep_p__->s_ip.addr[2], __rvtep_p__->s_ip.addr[3],                                           \
        (unsigned long)__rvtep_p__->nexthop_cnt,                                                                       \
        __rvtep_p__->vid, (unsigned long)__rvtep_p__->lport,                                            \
        (unsigned long)__rvtep_p__->uc_vxlan_port, (unsigned long)__rvtep_p__->mc_vxlan_port,           \
        __rvtep_p__->mac_ar[0], __rvtep_p__->mac_ar[1],                                                 \
        __rvtep_p__->mac_ar[2], __rvtep_p__->mac_ar[3],                                                 \
        __rvtep_p__->mac_ar[4], __rvtep_p__->mac_ar[5]);                                                \
    }                                                                                                   \
}

#define VXLAN_OM_DEBUG_PRINTF(__flag__, format_string, ...)             \
{                                                                       \
    if (VXLAN_OM_GetDebug(__flag__) == TRUE)                            \
    {                                                                   \
        BACKDOOR_MGR_Printf("\r\n%s(%d): "format_string"\r\n",          \
            __FUNCTION__, __LINE__, ##__VA_ARGS__);                     \
    }                                                                   \
}


/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI16_T  udp_dst_port;
    UI32_T  src_ifindex;
} VXLAN_OM_Shmem_Data_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void VXLAN_OM_Init();
static void VXLAN_OM_HisamDeleteAllRecord(void);
static int VXLAN_OM_CompareVlanVniMap(void *element1, void *element2);
static void VXLAN_OM_SetRvtepKey (UI8_T *key, UI32_T vni, UI8_T *ip);

/* STATIC VARIABLE DEFINITIONS
 */
static UI32_T   debug_flag = VXLAN_TYPE_DEBUG_FLAG_NONE;
static UI32_T   vxlan_om_sem_id;
static VXLAN_OM_Shmem_Data_T   *shmem_data_p;
static L_HISAM_Desc_T           m_rvtep_hisam_desc;
static L_HISAM_Desc_T           u_rvtep_hisam_desc;
static L_HISAM_Desc_T           vni_hisam_desc;
/* access_vxlan_port_ar[0][x] means per-port vxlan access port
 * access_vxlan_port_ar[0][0] is not used
 */
static UI32_T access_vxlan_port_ar[VXLAN_TYPE_VLAN_ID_MAX + 1][SYS_ADPT_TOTAL_NBR_OF_LPORT + 1] = {{0}}; /* UC VTEP */
static UI32_T l3_if_ar[VXLAN_TYPE_VLAN_ID_MAX] = {0};
static UI16_T l3_if_use_count_ar[VXLAN_TYPE_VLAN_ID_MAX] = {0}; //count access ports that use the L3 intf
static UI32_T vlan_vni_ar[VXLAN_TYPE_VLAN_ID_MAX] = {0};
static UI32_T port_vlan_vni_ar[SYS_ADPT_TOTAL_NBR_OF_LPORT + 1][VXLAN_TYPE_VLAN_ID_MAX + 1] = {{0}};

/* For UC remote VTEP, primary key is VNI+IP.
 */
const static L_HISAM_KeyDef_T uc_rvtep_key_def_table[U_RVTEP_NUMBER_OF_KEYS] =
                {
                    /* vni, ip */
                    {   2,                         /* field number */
                        {0, 8, 0, 0, 0, 0, 0, 0},  /* offset */
                        {4, 4, 0, 0, 0, 0, 0, 0}   /* len */
                    }
                };

const static L_HISAM_KeyType_T uc_rvtep_key_type_table[U_RVTEP_NUMBER_OF_KEYS][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
                {
                    /* vni,  ip*/
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER}
                };

/* For MC remote VTEP.
 * Each VNI can be assigned to only one MC remote VTEP, can use vni as primary key.
 */
const static L_HISAM_KeyDef_T mc_rvtep_key_def_table[M_RVTEP_NUMBER_OF_KEYS] =
                {
                    /* vni */
                    {   1,                         /* field number */
                        {0, 0, 0, 0, 0, 0, 0, 0},  /* offset */
                        {4, 0, 0, 0, 0, 0, 0, 0}   /* len */
                    }
                };

const static L_HISAM_KeyType_T mc_rvtep_key_type_table[M_RVTEP_NUMBER_OF_KEYS][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
                {
                    /* vni */
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER}
                };

const static L_HISAM_KeyDef_T vni_key_def_table[VNI_NUMBER_OF_KEYS] =
                {
                    /* vni */
                    {   1,                         /* field number */
                        {0, 0, 0, 0, 0, 0, 0, 0},  /* offset */
                        {4, 0, 0, 0, 0, 0, 0, 0}   /* len */
                    },
                    /* vfi */
                    {   1,                         /* field number */
                        {4, 0, 0, 0, 0, 0, 0, 0},  /* offset */
                        {4, 0, 0, 0, 0, 0, 0, 0}   /* len */
                    }
                };

const static L_HISAM_KeyType_T vni_key_type_table[VNI_NUMBER_OF_KEYS][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
                {
                    /* vni */
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER},
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER}
                };

static UI32_T vlan_member_count_ar[VXLAN_TYPE_VLAN_ID_MAX + 1] = {0}; /* vlan_member_count_ar[0] is not used */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: VXLAN_OM_InitiateSystemResources
 * PURPOSE : initiate VXLAN OM.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_InitiateSystemResources(void)
{
    shmem_data_p = (VXLAN_OM_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_VXLAN_SHMEM_SEGID);
    VXLAN_OM_Init();
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_VXLAN_OM, &vxlan_om_sem_id);

    return;
} /* End of VXLAN_OM_InitiateSystemResources */


/* FUNCTION NAME: VXLAN_OM_AttachSystemResources
 * PURPOSE : Attch system resource for VXLAN OM  in the context of the calling process.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_AttachSystemResources(void)
{
    shmem_data_p = (VXLAN_OM_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_VXLAN_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_VXLAN_OM, &vxlan_om_sem_id);
    return;
} /* End of VXLAN_OM_AttachSystemResources */

/* FUNCTION NAME: VXLAN_OM_GetShMemInfo
 * PURPOSE : Provide shared memory information for SYSRSC.
 * INPUT   : None.
 * OUTPUT  : segid_p   -- shared memory segment ID.
 *           senglen_p -- length of the shared memory segment.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_VXLAN_SHMEM_SEGID;
    *seglen_p = sizeof(VXLAN_OM_Shmem_Data_T);
    return;
} /* End of VXLAN_OM_GetShMemInfo */

/* FUNCTION NAME: VXLAN_OM_ClearAll
 * PURPOSE : Clear all static memory used by VXLAN OM.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_ClearAll(void)
{
    VXLAN_OM_ENTER_CRITICAL_SECTION();

    VXLAN_OM_HisamDeleteAllRecord();
    memset(shmem_data_p, 0, sizeof(VXLAN_OM_Shmem_Data_T));
    memset(access_vxlan_port_ar, 0, sizeof(access_vxlan_port_ar));
    memset(l3_if_ar, 0, sizeof(l3_if_ar));
    memset(l3_if_use_count_ar, 0, sizeof(l3_if_use_count_ar));
    memset(vlan_vni_ar, 0, sizeof(vlan_vni_ar));
    memset(port_vlan_vni_ar, 0, sizeof(port_vlan_vni_ar));
    memset(vlan_member_count_ar, 0, sizeof(vlan_member_count_ar));

    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    return;
} /* End of VXLAN_OM_ClearAll */

/* FUNCTION NAME: VXLAN_OM_SetAllDefault
 * PURPOSE : Set default values for all data used by VXLAN OM.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_SetAllDefault(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
    VXLAN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->udp_dst_port = VXLAN_TYPE_DFLT_UDP_DST_PORT;
    shmem_data_p->src_ifindex = 0;
    VXLAN_OM_LEAVE_CRITICAL_SECTION();
    return;
} /* End of VXLAN_OM_SetAllDefault */

/* FUNCTION NAME: VXLAN_OM_SetUdpDstPort
 * PURPOSE : Set UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_SetUdpDstPort(UI16_T port_no)
{
    shmem_data_p->udp_dst_port = port_no;
    return VXLAN_TYPE_RETURN_OK;
}

/* FUNCTION NAME: VXLAN_OM_GetUdpDstPort
 * PURPOSE : Get UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : port_no -- UDP port number
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetUdpDstPort(UI16_T *port_no)
{
    VXLAN_OM_ENTER_CRITICAL_SECTION();
    *port_no = shmem_data_p->udp_dst_port;
    VXLAN_OM_LEAVE_CRITICAL_SECTION();
    return VXLAN_TYPE_RETURN_OK;
}

/* FUNCTION NAME: VXLAN_OM_GetRunningUdpDstPort
 * PURPOSE : Get UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : port_no -- UDP port number
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None.
 */
SYS_TYPE_Get_Running_Cfg_T VXLAN_OM_GetRunningUdpDstPort(UI16_T *port_no)
{
    VXLAN_OM_ENTER_CRITICAL_SECTION();
    *port_no = shmem_data_p->udp_dst_port;
    VXLAN_OM_LEAVE_CRITICAL_SECTION();
    if (*port_no == VXLAN_TYPE_DFLT_UDP_DST_PORT)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* FUNCTION NAME: VXLAN_OM_AddFloodRVtep
 * PURPOSE : Flooding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : rvtep_entry_p
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_AddFloodRVtep(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI32_T  hisam_ret;

    VXLAN_OM_DEBUG_PRINTF_RVTEP(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, rvtep_entry_p);

    if (rvtep_entry_p != NULL)
    {
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        hisam_ret = L_HISAM_SetRecord (&u_rvtep_hisam_desc, (UI8_T *)rvtep_entry_p, TRUE);
        if ((hisam_ret == L_HISAM_INSERT) || (hisam_ret == L_HISAM_REPLACE))
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }
    return ret;
}

/* FUNCTION NAME: VXLAN_OM_DelFloodRVtep
 * PURPOSE : Delete remote VTEP with specified IP.
 * INPUT   : rvtep_entry_p
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_DelFloodRVtep(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI8_T key[RVTEP_KEY_LEN];

    VXLAN_OM_DEBUG_PRINTF_RVTEP(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, rvtep_entry_p);

    if (rvtep_entry_p != NULL)
    {
        VXLAN_OM_SetRvtepKey(key, rvtep_entry_p->vni, (rvtep_entry_p->ip.addr));
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        if (L_HISAM_DeleteRecord(&u_rvtep_hisam_desc, key) == TRUE)
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }
    return ret;
}

/* FUNCTION NAME: VXLAN_OM_GetFloodRVtepByVniIp
 * PURPOSE : Get remote VTEP with specified VNI+IP.
 * INPUT   : rvtep_entry_p->vni
 *           rvtep_entry_p->ip
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetFloodRVtepByVniIp(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI8_T   key[RVTEP_KEY_LEN];

    if (rvtep_entry_p != NULL)
    {
        VXLAN_OM_SetRvtepKey(key, rvtep_entry_p->vni, (rvtep_entry_p->ip.addr));
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        if (L_HISAM_GetRecord(&u_rvtep_hisam_desc, U_RVTEP_VNI_IP_KIDX, key, (UI8_T *)rvtep_entry_p) == TRUE)
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }

    VXLAN_OM_DEBUG_PRINTF_RVTEP(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, rvtep_entry_p);

    return ret;
}

/* FUNCTION NAME: VXLAN_OM_GetNextFloodRVtepByVni
 * PURPOSE : Get next remote VTEP with VNI.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetNextFloodRVtepByVni(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI32_T  vni = rvtep_entry_p->vni;
    UI8_T   key[RVTEP_KEY_LEN];
    UI8_T   ip[4];

    if (rvtep_entry_p != NULL)
    {
        VXLAN_OM_SetRvtepKey(key, vni, (rvtep_entry_p->ip.addr));
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        if (L_HISAM_GetNextRecord(&u_rvtep_hisam_desc, U_RVTEP_VNI_IP_KIDX, key, (UI8_T *)rvtep_entry_p) == TRUE)
        {
            if (rvtep_entry_p->vni == vni)
            {
                ret = VXLAN_TYPE_RETURN_OK;
            }
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }

    VXLAN_OM_DEBUG_PRINTF_RVTEP(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, rvtep_entry_p);

    return ret;
}

/* FUNCTION NAME: VXLAN_OM_GetNextFloodRVtep
 * PURPOSE : Get next remote VTEP with VNI+IP.
 * INPUT   : rvtep_entry_p->vni
 *           rvtep_entry_p->ip
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetNextFloodRVtep(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI8_T   key[RVTEP_KEY_LEN];

    if (rvtep_entry_p != NULL)
    {
        VXLAN_OM_SetRvtepKey(key, rvtep_entry_p->vni, (rvtep_entry_p->ip.addr));
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        if (L_HISAM_GetNextRecord(&u_rvtep_hisam_desc, U_RVTEP_VNI_IP_KIDX, key, (UI8_T *)rvtep_entry_p) == TRUE)
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }

    VXLAN_OM_DEBUG_PRINTF_RVTEP(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, rvtep_entry_p);

    return ret;
}


/* FUNCTION NAME: VXLAN_OM_AddFloodMulticast
 * PURPOSE : Flooding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : rvtep_entry_p
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_AddFloodMulticast(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI32_T  hisam_ret;

    VXLAN_OM_DEBUG_PRINTF_RVTEP(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, rvtep_entry_p);

    if (rvtep_entry_p != NULL)
    {
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        hisam_ret = L_HISAM_SetRecord (&m_rvtep_hisam_desc, (UI8_T *)rvtep_entry_p, TRUE);
        if ((hisam_ret == L_HISAM_INSERT) || (hisam_ret == L_HISAM_REPLACE))
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }
    return ret;
}

/* FUNCTION NAME: VXLAN_OM_DelFloodMulticast
 * PURPOSE : Delete remote VTEP with multicast group.
 * INPUT   : rvtep_entry_p
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_DelFloodMulticast(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI8_T   key[RVTEP_KEY_LEN];

    VXLAN_OM_DEBUG_PRINTF_RVTEP(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, rvtep_entry_p);

    if (rvtep_entry_p != NULL)
    {
        VXLAN_OM_SetRvtepKey(key, rvtep_entry_p->vni, (rvtep_entry_p->ip.addr));
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        if (L_HISAM_DeleteRecord(&m_rvtep_hisam_desc, key) == TRUE)
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }
    return ret;
}

/* FUNCTION NAME: VXLAN_OM_GetFloodMulticastByVni
 * PURPOSE : Get remote VTEP with multicast group.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetFloodMulticastByVni(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI8_T key[4];

    if (rvtep_entry_p != NULL)
    {
        memcpy(key, &(rvtep_entry_p->vni), 4);
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        if (L_HISAM_GetRecord(&m_rvtep_hisam_desc, M_RVTEP_VNI_KIDX, key, (UI8_T *)rvtep_entry_p) == TRUE)
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }

    VXLAN_OM_DEBUG_PRINTF_RVTEP(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, rvtep_entry_p);

    return ret;
}

/* FUNCTION NAME: VXLAN_OM_GetNextFloodMulticast
 * PURPOSE : Get Next remote VTEP.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetNextFloodMulticast(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI8_T key[4];

    if (rvtep_entry_p != NULL)
    {
        memcpy(&key, &(rvtep_entry_p->vni), 4);
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        if (L_HISAM_GetNextRecord(&m_rvtep_hisam_desc, M_RVTEP_VNI_KIDX, key, (UI8_T *)rvtep_entry_p) == TRUE)
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }

    VXLAN_OM_DEBUG_PRINTF_RVTEP(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, rvtep_entry_p);

    return ret;
}

/* FUNCTION NAME: VXLAN_OM_SetVlanVniMap
 * PURPOSE : Configure VLAN and VNI mapping relationship.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : For delete, vni = 0;
 */
UI32_T VXLAN_OM_SetVlanVniMap(UI16_T vid, UI32_T vni)
{
    VXLAN_OM_ENTER_CRITICAL_SECTION();
    vlan_vni_ar[vid-1] = vni;
    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vid[%u], vni[0x%lx]\r\n", vid, (unsigned long)vni);

    return VXLAN_TYPE_RETURN_OK;
}

/* FUNCTION NAME: VXLAN_OM_DelVlanVniMap
 * PURPOSE : Configure VLAN and VNI mapping relationship.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : vid = 0 means per-port
 */
UI32_T VXLAN_OM_DelVlanVniMap(UI16_T vid, UI32_T vni)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    
    VXLAN_OM_ENTER_CRITICAL_SECTION();
    if (vni == vlan_vni_ar[vid-1])
    {
        vlan_vni_ar[vid-1] = 0;
        ret = VXLAN_TYPE_RETURN_OK;
    }
    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vid[%u], vni[0x%lx]\r\n", vid, (unsigned long)vni);

    return ret;
}

/* FUNCTION NAME: VXLAN_OM_SetPortVlanVniMap
 * PURPOSE : Configure Port (and VLAN) to VNI mapping relationship.
 * INPUT   : lport -- port ifindex
 *           vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : 1. For delete, vni = 0;
 *           2. vid = 0 means per-port
 */
UI32_T VXLAN_OM_SetPortVlanVniMap(UI32_T lport, UI16_T vid, UI32_T vni)
{
    VXLAN_OM_ENTER_CRITICAL_SECTION();
    port_vlan_vni_ar[lport][vid] = vni;
    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "lport[%lu], vid[%u], vni[0x%lx]\r\n", (unsigned long)lport, vid, (unsigned long)vni);

    return VXLAN_TYPE_RETURN_OK;
}

/* FUNCTION NAME: VXLAN_OM_DelPortVlanVniMap
 * PURPOSE : Configure port (and VLAN) to VNI mapping relationship.
 * INPUT   : lport -- port ifindex
 *           vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_DelPortVlanVniMap(UI32_T lport, UI16_T vid, UI32_T vni)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    if (vni == port_vlan_vni_ar[lport][vid])
    {
        port_vlan_vni_ar[lport][vid] = 0;
        ret = VXLAN_TYPE_RETURN_OK;
    }
    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "lport[%lu], vid[%u], vni[0x%lx]\r\n", (unsigned long)lport, vid, (unsigned long)vni);

    return ret;
}

/* FUNCTION NAME: VXLAN_OM_GetNextPortVlanVniMapByVni
 * PURPOSE : Get next port (and VLAN) to VNI mapping relationship.
 * INPUT   : vni -- VXLAN ID
 *           lport_p -- port ifindex
 *           vid_p -- VLAN ID
 * OUTPUT  : lport_p -- port ifindex
 *           vid_p -- VLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetNextPortVlanVniMapByVni(UI32_T vni, UI32_T *lport_p, UI16_T *vid_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI32_T  lport;
    UI16_T  vid, start_vid;

    start_vid = *vid_p + 1;
    VXLAN_OM_ENTER_CRITICAL_SECTION();
    for (lport = *lport_p; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        for (vid = start_vid; vid <= VXLAN_TYPE_VLAN_ID_MAX; vid++)
        {
            if (vni == port_vlan_vni_ar[lport][vid])
            {
                *lport_p = lport;
                *vid_p = vid;
                ret = VXLAN_TYPE_RETURN_OK;
                goto out;
            }
        }
        start_vid = 0;
    }
out:
    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "lport[%lu], vid[%u], vni[0x%lx]\r\n", (unsigned long)*lport_p, *vid_p, (unsigned long)vni);

    return ret;
}

/* FUNCTION NAME - VXLAN_OM_GetVlanVniMapEntry
 * PURPOSE : Get VLAN-VNI mapping.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_OM_GetVlanVniMapEntry(UI16_T vid, UI32_T *vni)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    *vni = vlan_vni_ar[vid-1];
    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    if (0 != *vni)
        ret = VXLAN_TYPE_RETURN_OK;

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vid[%u], vni[0x%lx] \r\n",
        vid, (unsigned long)*vni);

    return ret;
} /* End of VXLAN_OM_GetVlanVniMapEntry */

/* FUNCTION NAME - VXLAN_OM_GetPortVlanVniMapEntry
 * PURPOSE : Get Port+VLAN to VNI mapping.
 * INPUT   : lport -- port ifindex
 *           vid -- VLAN ID
 * OUTPUT  : vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_OM_GetPortVlanVniMapEntry(UI32_T lport, UI16_T vid, UI32_T *vni)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    *vni = port_vlan_vni_ar[lport][vid];
    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    if (0 != *vni)
        ret = VXLAN_TYPE_RETURN_OK;

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "lport[%lu], vid[%u], vni[0x%lx] \r\n",
        (unsigned long)lport, vid, (unsigned long)*vni);

    return ret;
}

/* FUNCTION NAME - VXLAN_OM_GetNextVlanVniMapEntry
 * PURPOSE : Get next VLAN-VNI mapping.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : vid -- VLAN ID
 *           vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : vid=0 to get the first entry
 */
UI32_T VXLAN_OM_GetNextVlanVniMapEntry(UI16_T *vid, UI32_T *vni)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI16_T  index;

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    for (index = *vid; index < VXLAN_TYPE_VLAN_ID_MAX; index++)
    {
        if (0 != vlan_vni_ar[index])
        {
            *vid = index+1;
            *vni = vlan_vni_ar[index];
            ret = VXLAN_TYPE_RETURN_OK;
            break;
        }
    }
    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vid[%u], vni[0x%lx]  \r\n",
        *vid, (unsigned long)*vni);

    return ret;

} /* End of VXLAN_OM_GetNextVlanVniMapEntry */

/* FUNCTION NAME - VXLAN_OM_SetVniEntry
 * PURPOSE : Set a VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_OM_SetVniEntry(VXLAN_OM_VNI_T *vni_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI32_T  hisam_ret;

    if (vni_entry_p != NULL)
    {
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        hisam_ret = L_HISAM_SetRecord (&vni_hisam_desc, (UI8_T *)vni_entry_p, TRUE);
        if ((hisam_ret == L_HISAM_INSERT) || (hisam_ret == L_HISAM_REPLACE))
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }
    return ret;
} /* End of VXLAN_OM_SetVniEntry */

/* FUNCTION NAME - VXLAN_OM_DelVniEntry
 * PURPOSE : Set a VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_OM_DelVniEntry(VXLAN_OM_VNI_T *vni_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI8_T   key[4];

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vni[0x%lx], vfi[0x%lx], bcast_group[0x%lx]  \r\n",
        (unsigned long)vni_entry_p->vni, (unsigned long)vni_entry_p->vfi, (unsigned long)vni_entry_p->bcast_group);

    if (vni_entry_p != NULL)
    {
        memcpy(key, &(vni_entry_p->vni), 4);
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        if (L_HISAM_DeleteRecord(&vni_hisam_desc, key) == TRUE)
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }
    return ret;
} /* End of VXLAN_OM_DelVniEntry */

/* FUNCTION NAME - VXLAN_OM_GetVniEntry
 * PURPOSE : Get a VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_OM_GetVniEntry(VXLAN_OM_VNI_T *vni_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI8_T key[4];

    if (vni_entry_p != NULL)
    {
        memcpy(key, &(vni_entry_p->vni), 4);
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        if (L_HISAM_GetRecord(&vni_hisam_desc, VNI_KIDX, key, (UI8_T *)vni_entry_p) == TRUE)
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vni[0x%lx], vfi[0x%lx], bcast_group[0x%lx]  \r\n",
        (unsigned long)vni_entry_p->vni, (unsigned long)vni_entry_p->vfi, (unsigned long)vni_entry_p->bcast_group);
    return ret;

} /* End of VXLAN_OM_GetVniEntry */

/* FUNCTION NAME - VXLAN_OM_GetNextVniEntry
 * PURPOSE : Get a next VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : vni=0 to get the first entry
 */
UI32_T VXLAN_OM_GetNextVniEntry(VXLAN_OM_VNI_T *vni_entry_p)
{
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI8_T key[4];

    if (vni_entry_p != NULL)
    {
        memcpy(key, &(vni_entry_p->vni), 4);
        VXLAN_OM_ENTER_CRITICAL_SECTION();
        if (L_HISAM_GetNextRecord(&vni_hisam_desc, VNI_KIDX, key, (UI8_T *)vni_entry_p) == TRUE)
        {
            ret = VXLAN_TYPE_RETURN_OK;
        }
        VXLAN_OM_LEAVE_CRITICAL_SECTION();
    }

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vni[0x%lx], vfi[0x%lx], bcast_group[0x%lx]  \r\n",
        (unsigned long)vni_entry_p->vni, (unsigned long)vni_entry_p->vfi, (unsigned long)vni_entry_p->bcast_group);
    return ret;
} /* End of VXLAN_OM_GetNextVniEntry */

/* FUNCTION NAME: VXLAN_OM_SetAccessVxlanPort
 * PURPOSE : Add/ Delete access VTEP.
 * INPUT   : vid         -- VLAN ID.
 *           lport       -- port number.
 *           vxlan_port  -- output vxlan port.
 *           is_add      -- TRUE is create, FALSE is delete.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : If is_add = FALSE, shall input vxlan_port = 0.
 */
void VXLAN_OM_SetAccessVxlanPort(UI16_T vid, UI32_T lport, UI32_T vxlan_port, BOOL_T is_add)
{
    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vid[%u], lport[%lu], uc_vxlan_port[%lx] %s \r\n",
        vid, (unsigned long)lport, (unsigned long)vxlan_port, ((is_add == TRUE) ? "Add" : "Del"));

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    access_vxlan_port_ar[vid][lport] = vxlan_port;

    if (is_add == TRUE)
    {
        vlan_member_count_ar[vid]++;
    }
    else
    {
        if (vlan_member_count_ar[vid] > 0)
        {
            vlan_member_count_ar[vid]--;
        }
    }
    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vid[%u], member_count[%lu] \r\n",
        vid, (unsigned long)(vlan_member_count_ar[vid]));

    VXLAN_OM_LEAVE_CRITICAL_SECTION();
    return;
} /* End of VXLAN_OM_SetAccessVxlanPort */

/* FUNCTION NAME: VXLAN_OM_GetVniByVfi
 * PURPOSE : Get VNI value from VFI
 * INPUT   : vfi         -- virtual forwarding instance
 * OUTPUT  : None.
 * RETURN  : vni, returns -1 when error
 * NOTES   : None.
 */
I32_T VXLAN_OM_GetVniByVfi(UI32_T vfi)
{
    VXLAN_OM_VNI_T vni_entry;
    UI8_T key[4];
    I32_T ret=-1;

    memcpy(key, &vfi, 4);

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    if (L_HISAM_GetRecord(&vni_hisam_desc, VNI_VFI_KIDX, key, (UI8_T *)&vni_entry) == TRUE)
    {
        ret = vni_entry.vni;
    }

    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}

/* FUNCTION NAME: VXLAN_OM_GetAccessVxlanPort
 * PURPOSE : Get access VXLAN port.
 * INPUT   : vid         -- VLAN ID.
 *           lport       -- port number.
 * OUTPUT  : vxlan_port  -- output vxlan port.
 * RETURN  : None.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetAccessVxlanPort(UI16_T vid, UI32_T lport)
{
    UI32_T vxlan_port;

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    vxlan_port = access_vxlan_port_ar[vid][lport];
    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vid[%u], lport[%lu], uc_vxlan_port[%lx] \r\n",
        vid, (unsigned long)lport, (unsigned long)vxlan_port);

    return vxlan_port;
} /* End of VXLAN_OM_GetAccessVxlanPort */

/* FUNCTION NAME: VXLAN_OM_GetNextAccessVxlanPort
 * PURPOSE : Get next access VXLAN port.
 * INPUT   : vid_p         -- VLAN ID.
 *           lport_p       -- port number.
 *           vxlan_port_p  -- output vxlan port.
 * OUTPUT  : TRUE/FALSE
 * RETURN  : None.
 * NOTES   : None.
 */
BOOL_T VXLAN_OM_GetNextAccessVxlanPort(UI16_T *vid_p, UI32_T *lport_p, UI32_T *vxlan_port_p)
{
    UI32_T vxlan_port = 0;
    UI32_T lport = *lport_p;
    UI16_T vid = *vid_p;
    UI32_T i, j;

    if ((0 != vid) || (0 != lport))
    {
        lport += 1;
    }

    if (SYS_ADPT_TOTAL_NBR_OF_LPORT < lport)
    {
        lport = 0;
        vid += 1;
    }

    if (VXLAN_TYPE_VLAN_ID_MAX < vid)
    {
        return FALSE;
    }
//printf("                      input vid %lu port %lu,", *vid_p, *lport_p);
    VXLAN_OM_ENTER_CRITICAL_SECTION();
    for (i = vid; i <= VXLAN_TYPE_VLAN_ID_MAX; i++)
    {
        for (j = lport; j <=SYS_ADPT_TOTAL_NBR_OF_LPORT; j++)
        {
            vxlan_port = access_vxlan_port_ar[i][j];

            if(0 != vxlan_port)
            {//printf(" get %lu %lu 0x%x.\r\n", i, j, vxlan_port);
                *vid_p = i;
                *lport_p = j;
                *vxlan_port_p = vxlan_port;
                VXLAN_OM_LEAVE_CRITICAL_SECTION();
                return TRUE;
            }
        }

        lport = 0;
    }
    VXLAN_OM_LEAVE_CRITICAL_SECTION();
//printf(" false on om %lu %lu.\r\n", i, j);
    return FALSE;
} /* End of VXLAN_OM_GetAccessVxlanPort */

/* FUNCTION NAME: VXLAN_OM_GetVlanNlportOfAccessPort
 * PURPOSE : Get VID and port from access VXLAN port.
 * INPUT   : vxlan_port    -- vxlan port.
 * OUTPUT  : vid_p         -- VLAN ID.
 *           lport_p       -- port number.
 * RETURN  : TRUE/FALSE
 * NOTES   : None.
 */
BOOL_T VXLAN_OM_GetVlanNlportOfAccessPort(UI32_T vxlan_port, UI16_T *vid_p, UI32_T *lport_p)
{
    UI32_T i, j;
    
    if ((NULL == vid_p) || (NULL == lport_p))
    {
        return FALSE;
    }
    
    VXLAN_OM_ENTER_CRITICAL_SECTION();
    for (i = 0; i <= VXLAN_TYPE_VLAN_ID_MAX; i++)
    {
        for (j = 0; j <=SYS_ADPT_TOTAL_NBR_OF_LPORT; j++)
        {
            if (vxlan_port == access_vxlan_port_ar[i][j])
            {
                *vid_p = i;
                *lport_p = j;
                
                VXLAN_OM_LEAVE_CRITICAL_SECTION();
                return TRUE;
            }    
        }
    }
    
    VXLAN_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
} /* End of VXLAN_OM_GetAccessVxlanPort */

/* FUNCTION NAME: VXLAN_OM_SetL3If
 * PURPOSE : Create a L3 interface for specified VLAN
 * INPUT   : vid         -- VLAN ID.
 *           l3_if       -- L3 interface value.
 *           is_add      -- TRUE is create, FALSE is delete.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : If is_add = FALSE, shall input l3_if = 0.
 */
void VXLAN_OM_SetL3If(UI16_T vid, UI32_T l3_if, BOOL_T is_add)
{
    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vid[%u], l3_if[%lu] %s \r\n",
        vid, (unsigned long)l3_if, ((is_add == TRUE) ? "Add" : "Del"));

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    l3_if_ar[vid-1] = l3_if;
    VXLAN_OM_LEAVE_CRITICAL_SECTION();
    return;
} /* End of VXLAN_OM_SetL3If */

/* FUNCTION NAME: VXLAN_OM_GetL3If
 * PURPOSE : Get a L3 interface for specified VLAN
 * INPUT   : vid         -- VLAN ID.
 * OUTPUT  : l3_if       -- L3 interface value.
 * RETURN  : None.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetL3If(UI16_T vid)
{
    UI32_T l3_if;

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    l3_if = l3_if_ar[vid-1];
    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vid[%u], l3_if[%lu]\r\n", vid, (unsigned long)l3_if);

    return l3_if;
} /* End of VXLAN_OM_GetL3If */

/* FUNCTION NAME: VXLAN_OM_SetL3IfUseCount
 * PURPOSE : Set count of a L3 interface for specified VLAN
 * INPUT   : vid         -- VLAN ID.
 *           is_add      -- TRUE is add, FALSE is minus.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_SetL3IfUseCount(UI16_T vid, BOOL_T is_add)
{
    VXLAN_OM_ENTER_CRITICAL_SECTION();

    if (TRUE == is_add)
    {
        l3_if_use_count_ar[vid-1] += 1;
    }
    else if (0 < l3_if_use_count_ar[vid-1])
    {
        l3_if_use_count_ar[vid-1] -= 1;
    }

    VXLAN_OM_LEAVE_CRITICAL_SECTION();
    return;
} /* End of VXLAN_OM_SetL3IfUseCount */

/* FUNCTION NAME: VXLAN_OM_GetL3IfUseCount
 * PURPOSE : Get use count of a L3 interface for specified VLAN
 * INPUT   : vid         -- VLAN ID.
 * OUTPUT  : None.
 * RETURN  : use count   -- use count
 * NOTES   : None.
 */
UI16_T VXLAN_OM_GetL3IfUseCount(UI16_T vid)
{
    UI16_T count;

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    count = l3_if_use_count_ar[vid-1];
    VXLAN_OM_LEAVE_CRITICAL_SECTION();
    return count;
} /* End of VXLAN_OM_GetL3IfUseCount */


/* FUNCTION NAME - VXLAN_OM_HandleIPCReqMsg
 * PURPOSE : Handle the IPC request message.
 * INPUT   : msgbuf_p -- IPC request message buffer
 * OUTPUT  : msgbuf_p -- IPC response message buffer
 * RETURN  : TRUE  -- there is a response required to be sent
 *           FALSE -- there is no response required to be sent
 * NOTE    : None
 */
BOOL_T VXLAN_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */
    VXLAN_OM_IpcMsg_T *msg_p;

    /* BODY
     */

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (VXLAN_OM_IpcMsg_T*)msgbuf_p->msg_buf;

    /* dispatch IPC message and call the corresponding OM function
     */
    switch (msg_p->type.cmd)
    {
        case VXLAN_OM_IPC_GETUDPDSTPORT:
            msg_p->type.ret_ui32 = VXLAN_OM_GetUdpDstPort(&msg_p->data.arg_ui16);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_ui16);
            break;
        case VXLAN_OM_IPC_GETRUNNINGUDPDSTPORT:
            msg_p->type.ret_ui32 = VXLAN_OM_GetRunningUdpDstPort(&msg_p->data.arg_ui16);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_ui16);
            break;
        case VXLAN_OM_IPC_GETFLOODRVTEPBYVNIIP:
            msg_p->type.ret_ui32 = VXLAN_OM_GetFloodRVtepByVniIp(&msg_p->data.arg_rvtep);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_rvtep);
            break;
        case VXLAN_OM_IPC_GETNEXTFLOODRVTEPBYVNI:
            msg_p->type.ret_ui32 = VXLAN_OM_GetNextFloodRVtepByVni(&msg_p->data.arg_rvtep);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_rvtep);
            break;
        case VXLAN_OM_IPC_GETNEXTFLOODRVTEP:
            msg_p->type.ret_ui32 = VXLAN_OM_GetNextFloodRVtep(&msg_p->data.arg_rvtep);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_rvtep);
            break;
        case VXLAN_OM_IPC_GETFLOODMULTICASTBYVNI:
            msg_p->type.ret_ui32 = VXLAN_OM_GetFloodMulticastByVni(&msg_p->data.arg_rvtep);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_rvtep);
            break;
        case VXLAN_OM_IPC_GETNEXTFLOODMULTICAST:
            msg_p->type.ret_ui32 = VXLAN_OM_GetNextFloodMulticast(&msg_p->data.arg_rvtep);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_rvtep);
            break;
        case VXLAN_OM_IPC_GETNEXTPORTVLANVNIMAPBYVNI:
            msg_p->type.ret_ui32 = VXLAN_OM_GetNextPortVlanVniMapByVni(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1,
                                        &msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2,
                                        &msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_grp_ui16_ui32_ui32);
            break;
        case VXLAN_OM_IPC_GETVLANVNIMAPENTRY:
            msg_p->type.ret_ui32 = VXLAN_OM_GetVlanVniMapEntry(msg_p->data.arg_grp_ui16_ui32.arg_ui16,
                &(msg_p->data.arg_grp_ui16_ui32.arg_ui32));
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_grp_ui16_ui32);
            break;
        case VXLAN_OM_IPC_GETNEXTVLANVNIMAPENTRY:
            msg_p->type.ret_ui32 = VXLAN_OM_GetNextVlanVniMapEntry(&(msg_p->data.arg_grp_ui16_ui32.arg_ui16),
                &(msg_p->data.arg_grp_ui16_ui32.arg_ui32));
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_grp_ui16_ui32);
            break;
        case VXLAN_OM_IPC_GETVNIBYVFI:
            msg_p->type.ret_ui32 = VXLAN_OM_GetVniByVfi(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_ui32);
            break;
        case VXLAN_OM_IPC_GETNEXTVNIENTRY:
            msg_p->type.ret_ui32 = VXLAN_OM_GetNextVniEntry(&msg_p->data.arg_vni_entry);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_vni_entry);
            break;
        case VXLAN_OM_IPC_GETSRCIF:
            msg_p->type.ret_ui32 = VXLAN_OM_GetSrcIf(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_ui32);
            break;
        case VXLAN_OM_IPC_GETRUNNINGSRCIF:
            msg_p->type.ret_ui32 = VXLAN_OM_GetRunningSrcIf(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_ui32);
            break;
        case VXLAN_OM_IPC_GETVLANANDVNIBYVFI:
            msg_p->type.ret_ui32 = VXLAN_OM_GetVlanAndVniByVfi(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1,
                &(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16),
                &(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2));
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_grp_ui16_ui32_ui32);
            break;
        case VXLAN_OM_IPC_GETLPORTOFACCESSPORT:
            msg_p->type.ret_ui32 = VXLAN_OM_GetLportOfAccessPort(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1,
                msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16,
                &(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2));
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_grp_ui16_ui32_ui32);
            break;
        case VXLAN_OM_IPC_GETNEXTACCESSPORT:
            msg_p->type.ret_bool = VXLAN_OM_GetNextAccessVxlanPort(&(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16),
                &(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1),
                &(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2));
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_grp_ui16_ui32_ui32);
            break;
        case VXLAN_OM_IPC_GETPORTVLANVNIMAPENTRY:
            msg_p->type.ret_ui32 = VXLAN_OM_GetPortVlanVniMapEntry(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1,
                msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16,
                &(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2));
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_grp_ui16_ui32_ui32);
            break; 
        case VXLAN_OM_IPC_GETACCESSVXLANPORT:
            msg_p->type.ret_ui32 = VXLAN_OM_GetAccessVxlanPort(msg_p->data.arg_grp_ui16_ui32.arg_ui16,
                msg_p->data.arg_grp_ui16_ui32.arg_ui32);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_grp_ui16_ui32);
            break;  
        case VXLAN_OM_IPC_GETVNIENTRY:
            msg_p->type.ret_ui32 = VXLAN_OM_GetVniEntry(&msg_p->data.arg_vni_entry);
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_vni_entry);
            break;     
        case VXLAN_OM_IPC_GETVLANNLPORTOFACCESSPORT:
            msg_p->type.ret_ui32 = VXLAN_OM_GetVlanNlportOfAccessPort(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1,
                &(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16),
                &(msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2));
            msgbuf_p->msg_size = VXLAN_OM_GET_MSG_SIZE(arg_grp_ui16_ui32_ui32);
            break;        
        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msg_p->type.ret_ui32 = VXLAN_TYPE_RETURN_ERROR;
            msgbuf_p->msg_size = VXLAN_OM_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of VXLAN_OM_HandleIPCReqMsg */

/* FUNCTION NAME: VXLAN_OM_SetDebug
 * PURPOSE : Set debug flag.
 * INPUT   : flag.
 *           is_on
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_OM_SetDebug(UI32_T flag, BOOL_T is_on)
{
    if (TRUE == is_on)
        debug_flag = debug_flag | flag;
    else
        debug_flag = debug_flag &(~flag);
    return;
} /* End of VXLAN_OM_SetDebug */

/* FUNCTION NAME: VXLAN_OM_GetDebug
 * PURPOSE : Get debug flag.
 * INPUT   : flag.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
BOOL_T VXLAN_OM_GetDebug(UI32_T flag)
{
    return ( (debug_flag & flag) != 0);
} /* End of VXLAN_OM_GetDebug */

/* FUNCTION NAME: VXLAN_OM_GetTotalRVtepNumber
 * PURPOSE : Get the total number of remote VTEP entries
 * INPUT   : is_uc   -- Ture: for unicast case.
 *                   -- FALSE: for multicast case.
 * OUTPUT  : None
 * RETURN  : used number.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetTotalRVtepNumber(BOOL_T is_uc)
{
    UI32_T free_buffer = 0;
    UI32_T used_buffer = 0;

    if (is_uc)
    {
        free_buffer = L_HISAM_GetFreeBufNo(&u_rvtep_hisam_desc);
        used_buffer += u_rvtep_hisam_desc.total_record_nbr - free_buffer;
    }
    else
    {
        free_buffer = L_HISAM_GetFreeBufNo(&m_rvtep_hisam_desc);
        used_buffer += m_rvtep_hisam_desc.total_record_nbr - free_buffer;
    }

    return(used_buffer);
}

/* FUNCTION NAME: VXLAN_OM_SetSrcIf
 * PURPOSE : Set source interface ifindex of VTEP.
 * INPUT   : ifindex -- interface index
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_SetSrcIf(UI32_T ifindex)
{
    shmem_data_p->src_ifindex = ifindex;
    return VXLAN_TYPE_RETURN_OK;
}

/* FUNCTION NAME: VXLAN_OM_GetSrcIf
 * PURPOSE : Get source interface ifindex of VTEP.
 * INPUT   : None
 * OUTPUT  : ifindex_p -- interface index
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetSrcIf(UI32_T *ifindex_p)
{
    *ifindex_p = shmem_data_p->src_ifindex;
    return VXLAN_TYPE_RETURN_OK;
}

/* FUNCTION NAME: VXLAN_OM_GetRunningSrcIf
 * PURPOSE : Get source interface ifindex of VTEP.
 * INPUT   : None
 * OUTPUT  : ifindex_p -- interface index
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None.
 */
SYS_TYPE_Get_Running_Cfg_T VXLAN_OM_GetRunningSrcIf(UI32_T *ifindex_p)
{
    VXLAN_OM_ENTER_CRITICAL_SECTION();
    *ifindex_p = shmem_data_p->src_ifindex;
    VXLAN_OM_LEAVE_CRITICAL_SECTION();
    if (*ifindex_p == 0)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* FUNCTION NAME: VXLAN_OM_GetVlanAndVniByVfi
 * PURPOSE : Get VLAN and VNI value from VFI
 * INPUT   : vfi         -- virtual forwarding instance
 * OUTPUT  : vid         -- vlan ID
 *           vni         -- vxlan ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetVlanAndVniByVfi(UI32_T vfi, UI16_T *vid, UI32_T *vni)
{
    VXLAN_OM_VNI_T vni_entry;
    UI32_T ret=VXLAN_TYPE_RETURN_ERROR;
    UI16_T  index;
    UI8_T key[4];

    memcpy(key, &vfi, 4);

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    if (L_HISAM_GetRecord(&vni_hisam_desc, VNI_VFI_KIDX, key, (UI8_T *)&vni_entry) == TRUE) 
    {
        for (index = 0; index < VXLAN_TYPE_VLAN_ID_MAX; index++)
        {
            if (vni_entry.vni == vlan_vni_ar[index])
            {
                *vid = index+1;
                *vni = vni_entry.vni;
                ret = VXLAN_TYPE_RETURN_OK;
            }
        }
    }
    VXLAN_OM_LEAVE_CRITICAL_SECTION();
    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vfi[0x%lx], vid[%u], vni[0x%lx]  \r\n",
        (unsigned long)vfi, *vid, (unsigned long)*vni);
    return ret;
}

/* FUNCTION NAME: VXLAN_OM_GetLportOfAccessPort
 * PURPOSE : Get logical port number of access port on specified VLAN.
 * INPUT   : vxlan_port  -- vxlan port nunmber of access port
 *           vid         -- vlan ID
 * OUTPUT  : lport       -- logical port number
 *           vni         -- vxlan ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetLportOfAccessPort(UI32_T vxlan_port, UI16_T vid, UI32_T *lport)
{
    UI32_T ret=VXLAN_TYPE_RETURN_ERROR;
    UI16_T  index;
    VXLAN_OM_ENTER_CRITICAL_SECTION();
    for (index = 1; index <= SYS_ADPT_TOTAL_NBR_OF_LPORT; index++)
    {
        if (vxlan_port == access_vxlan_port_ar[vid][index])
        {
            *lport = index;
            ret = VXLAN_TYPE_RETURN_OK;
        }
    }
    VXLAN_OM_LEAVE_CRITICAL_SECTION();
    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vxlan_port[0x%lx], vid[%u], lport[%lu]  \r\n",
        vxlan_port, vid, *lport);
    return ret;

}

/* FUNCTION NAME: VXLAN_OM_GetVlanMemberCounter
 * PURPOSE : Get member counter for specified VLAN.
 * INPUT   : vid         -- VLAN ID.
 * OUTPUT  : member counter  -- output vlan member counter.
 * RETURN  : None.
 * NOTES   : None.
 */
UI32_T VXLAN_OM_GetVlanMemberCounter(UI16_T vid)
{
    UI32_T member_counter;

    VXLAN_OM_ENTER_CRITICAL_SECTION();
    member_counter = vlan_member_count_ar[vid];
    VXLAN_OM_LEAVE_CRITICAL_SECTION();

    VXLAN_OM_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "vid[%u], member_counter[%lu]\r\n",
        vid, (unsigned long)member_counter);

    return member_counter;
} /* End of VXLAN_OM_GetAccessVxlanPort */

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: VXLAN_OM_Init
 * PURPOSE : Allocate database memory.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_OM_Init()
{
    m_rvtep_hisam_desc.hash_depth         = HASH_DEPTH;
    m_rvtep_hisam_desc.N1                 = HISAM_N1;
    m_rvtep_hisam_desc.N2                 = HISAM_N2;
    m_rvtep_hisam_desc.record_length      = RVTEP_NODE_LEN;
    m_rvtep_hisam_desc.total_hash_nbr     = M_RVTEP_HASH_NBR;
    m_rvtep_hisam_desc.total_index_nbr    = INDEX_NBR;
    m_rvtep_hisam_desc.total_record_nbr   = M_RVTEP_NODE_NBR;

    if (!L_HISAM_CreateV2(&m_rvtep_hisam_desc, M_RVTEP_NUMBER_OF_KEYS /* number_of _key */, mc_rvtep_key_def_table, mc_rvtep_key_type_table))
    {
        printf(" VXLAN_OM: Create VNI HISAM Error !\n");
        while(1);
    }

    u_rvtep_hisam_desc.hash_depth         = HASH_DEPTH;
    u_rvtep_hisam_desc.N1                 = HISAM_N1;
    u_rvtep_hisam_desc.N2                 = HISAM_N2;
    u_rvtep_hisam_desc.record_length      = RVTEP_NODE_LEN;
    u_rvtep_hisam_desc.total_hash_nbr     = U_RVTEP_HASH_NBR;
    u_rvtep_hisam_desc.total_index_nbr    = INDEX_NBR;
    u_rvtep_hisam_desc.total_record_nbr   = U_RVTEP_NODE_NBR;

    if (!L_HISAM_CreateV2(&u_rvtep_hisam_desc, U_RVTEP_NUMBER_OF_KEYS /* number_of _key */, uc_rvtep_key_def_table, uc_rvtep_key_type_table))
    {
        printf(" VXLAN_OM: Create RVTEP HISAM Error !\n");
        while(1);
    }

    vni_hisam_desc.hash_depth         = HASH_DEPTH;
    vni_hisam_desc.N1                 = HISAM_N1;
    vni_hisam_desc.N2                 = HISAM_N2;
    vni_hisam_desc.record_length      = VNI_NODE_LEN;
    vni_hisam_desc.total_hash_nbr     = VNI_HASH_NBR;
    vni_hisam_desc.total_index_nbr    = INDEX_NBR;
    vni_hisam_desc.total_record_nbr   = VNI_NODE_NBR;

    if (!L_HISAM_CreateV2(&vni_hisam_desc, VNI_NUMBER_OF_KEYS /* number_of _key */, vni_key_def_table, vni_key_type_table))
    {
        printf(" VXLAN_OM: Create VLAN-VNI HISAM Error !\n");
        while(1);
    }

    shmem_data_p->udp_dst_port = 0;
    return;
}


 /* FUNCTION NAME: VXLAN_OM_HisamDeleteAllRecord
 * PURPOSE : This function will delete all record from Hisam Table
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : Hisam TAble re-initial.
 */
static void VXLAN_OM_HisamDeleteAllRecord(void)
{
    L_HISAM_DeleteAllRecord(&m_rvtep_hisam_desc);
    L_HISAM_DeleteAllRecord(&u_rvtep_hisam_desc);
    L_HISAM_DeleteAllRecord(&vni_hisam_desc);
}


/* FUNCTION NAME: VXLAN_OM_SetRvtepKey
 * PURPOSE : Generate the key needed to access HISAM entries in VNI -> IP order
 * INPUT   : vni
 *           ip
 * OUTPUT  : UI8_T  *key - required key
 * RETURN  : result of compare
 * NOTES   : None.
 */
static void VXLAN_OM_SetRvtepKey (UI8_T *key, UI32_T vni, UI8_T *ip)
{
    memcpy(key, &vni, 4);
    memcpy(key+4, ip, 4);
}

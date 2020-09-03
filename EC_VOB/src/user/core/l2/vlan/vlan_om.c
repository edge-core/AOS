/* -----------------------------------------------------------------------------
 * FILE NAME: VLAN_OM.c
 *
 * PURPOSE: This package serves as a database to store Q-bridge MIB defined
 *          information.
 * NOTES:   1. The two primary functions of this file is to maintain
 *             dot1qVlanStaticTable and dot1qPortVlanTable as defined in Q-bridge
 *             MIB.
 *          2. The vlan_om only contain two tables. One is vlan_table which contains all
 *             information of each VLAN (ex: creation time, VID, egress ports,
 *             untagged ports¡Ketc), and another is vlan_port_table which contains all
 *             information of each port (ex: PVID, acceptable_frame_type,
 *             ingress filtering...etc). The most content of these two tables is defined
 *             in RFC2674.
 * MODIFICATION HISOTRY:
 * MODIFIER               DATE         DESCRIPTION
 * ------------------------------------------------------------------------------
 * cpyang       6-19-2001      First created
 * amytu        8-01-2001      Revised VLAN_OM based on Q-bridge MIB definition.
 * ------------------------------------------------------------------------------
 * Copyright(C)        Accton Techonology Corporation 2001
* ------------------------------------------------------------------------------*/


/* INCLUDE FILE DECLARATIONS
 */

#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "leaf_2674q.h"
#include "leaf_es3626a.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_mm.h"
#include "l_proto.h"
#include "l_cvrt.h"
#include "sysfun.h"
#include "vlan_lib.h"
#include "vlan_om.h"
#include "vlan_om_private.h"
#include "vlan_type.h"
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATATYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void VLAN_OM_ClearAllVlanTable(void);
static void VLAN_OM_ClearAllVlanPortTable(void);

/* Local Utilities */
static  BOOL_T  VLAN_OM_ConvertIndexToPosition(UI32_T index, UI16_T *byte, UI8_T *bit, BOOL_T ascent);
static  BOOL_T  VLAN_OM_ConvertIndexFmPosition(UI32_T *index, UI16_T byte, UI8_T bit, BOOL_T ascent);
static  BOOL_T  VLAN_OM_GetNextOnBit(UI8_T *array, UI16_T length, UI16_T *index, UI8_T *bit, BOOL_T ascent);

static BOOL_T VLAN_OM_GetMgmtIpStateOfVlan(UI32_T vid, BOOL_T* mgmt_state, UI8_T* ip_state);

static BOOL_T VLAN_OM_GetVlanEntry_Local(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info);
#if (SYS_CPNT_MAC_VLAN == TRUE)
static BOOL_T VLAN_OM_IsMacAddressNull(UI8_T *mac_address);
static BOOL_T VLAN_OM_GetIndexOfNullMacVlanEntry(UI32_T *index);
static BOOL_T VLAN_OM_GetIndexOfMacAddress(UI8_T *mac_address, UI8_T *mask, UI32_T *index);
#endif /*end of #if (SYS_CPNT_MAC_VLAN == TRUE)*/

/* STATIC VARIABLE DECLARATIONS
*/
/* The vlan_table array contains a limited number of VLAN entries. */
static VLAN_Shmem_Data_T *shmem_data_p;
static UI32_T             vlan_om_sem_id;

#if (SYS_CPNT_MAC_VLAN == TRUE)
const UI8_T    null_mac[] = {0,0,0,0,0,0};
const UI8_T    broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#endif


#define VLAN_OM_ENTER_CRITICAL_SECTION(sem_id) \
   do{            \
    SYSFUN_TakeSem(sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER) ; \
    }while(0)

#define VLAN_OM_LEAVE_CRITICAL_SECTION(sem_id) \
    do{            \
        SYSFUN_GiveSem(sem_id);\
     }while(0)


#if 0

static VLAN_OM_Dot1qVlanCurrentEntry_T      vlan_table[SYS_ADPT_MAX_NBR_OF_VLAN + 1];
/* The index of the vlan_entry_index array indicate the vid, and the value of vlan_entry_index[vid]
   indicate the pointer of the vid's corresponding VLAN entry which is stored in vlan_table. */
static VLAN_OM_Dot1qVlanCurrentEntry_T      *vlan_entry_index[SYS_ADPT_MAX_VLAN_ID];

static L_PT_Descriptor_T                    vlan_table_desc;
static VLAN_OM_Vlan_Port_Info_T             vlan_port_table[SYS_ADPT_RS232_1_IF_INDEX_NUMBER];
/* The total numbers of current existent VLAN. */
static UI32_T   vlan_om_current_cfg_vlan=0;
/* The global gvrp status. It's difference from the gvrp status on a port.
   Enable gvrp on a specific port must enable global gvrp status first. */
static UI32_T   global_dot1q_gvrp_status;
/* The number of times a VLAN entry has been deleted from the dot1qVlanCurrentTable (for any reason).
   If an entry is deleted, then inserted, and then deleted, this counter will be incremented by 2. */
static UI32_T   num_vlan_deletes=0;
/* The value of time when the vlan_table or vlan_port_table was modified. */
static UI32_T   last_update_time;
/* Specify whether IVL or SVL is the default constraint.
    VAL_dot1qConstraintTypeDefault_independent
    VAL_dot1qConstraintTypeDefault_shared
*/
static UI32_T   dot1q_constraint_type_default;

static UI32_T   default_vlan_id;

static UI32_T   vlan_om_sem_id;
static UI32_T   original_priority;

static UI32_T   if_table_last_change;
static UI32_T   if_stack_last_change;
static UI32_T   management_vlan_id;

#else
#define vlan_table    shmem_data_p->vlan_shared_table
#define vlan_entry_offset       shmem_data_p->vlan_shared_entry_offset
#define vlan_table_desc  shmem_data_p->vlan_shared_table_desc
#define vlan_port_table                     shmem_data_p->vlan_shared_port_table
#define VLAN_OM_ENTRY_ADDR(idx)     ((VLAN_OM_Dot1qVlanCurrentEntry_T*)L_CVRT_GET_PTR(&vlan_table_desc, vlan_entry_offset[idx]))
#define VLAN_OM_ENTRY_OFFSET(ptr)   L_CVRT_GET_OFFSET(&vlan_table_desc, ptr)

/* The total numbers of current existent VLAN. */
#define vlan_om_current_cfg_vlan  shmem_data_p->vlan_shard_om_current_cfg_vlan
/* The global gvrp status. It's difference from the gvrp status on a port.
   Enable gvrp on a specific port must enable global gvrp status first. */
#define global_dot1q_gvrp_status shmem_data_p->vlan_shared_global_dot1q_gvrp_status
/* The number of times a VLAN entry has been deleted from the dot1qVlanCurrentTable (for any reason).
   If an entry is deleted, then inserted, and then deleted, this counter will be incremented by 2. */
#define num_vlan_deletes shmem_data_p->vlan_shard_num_vlan_deletes
/* The value of time when the vlan_table or vlan_port_table was modified. */
#define last_update_time  shmem_data_p->vlan_shard_last_update_time
#define dot1q_constraint_type_default shmem_data_p->vlan_shard_dot1q_constraint_type_default
#define default_vlan_id  shmem_data_p->vlan_shard_default_vlan_id
#define if_table_last_change  shmem_data_p->vlan_shard_if_table_last_change
#define if_stack_last_change  shmem_data_p->vlan_shard_if_stack_last_change
#define management_vlan_id  shmem_data_p->vlan_shard_management_vlan_id
#endif

#if (SYS_CPNT_MAC_VLAN == TRUE)
/* The MAC-to-VLAN mapping table */
#define mac_vlan_table shmem_data_p->mac_vlan_table
#define mac_vlan_entry_count shmem_data_p->mac_vlan_entry_count

#define IS_NULL_MAC(mac)        (memcmp((mac), null_mac, SYS_ADPT_MAC_ADDR_LEN) == 0)
#define IS_MULTICAST_MAC(mac)   ((mac)[0] & 0x01)
#define IS_BROADCAST_MAC(mac)   (memcmp((mac), broadcast_mac, SYS_ADPT_MAC_ADDR_LEN) == 0)
#endif /*end of #if (SYS_CPNT_MAC_VLAN == TRUE)*/

/*---------------------------
 * EXPORTED SUBPROGRAM BODIES
 *---------------------------*/
/*---------------------------------------------------------------------------------
 * FUNCTION : void VLAN_SHOM_InitiateSystemResources(void)
*---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource for Vlan OM
* INPUT    : none
* OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void VLAN_OM_InitiateSystemResources(void)
{
    shmem_data_p = (VLAN_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_VLAN_SHMEM_SEGID);
    memset(shmem_data_p,0,sizeof(VLAN_Shmem_Data_T));
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_VLAN_OM, &vlan_om_sem_id);
}

/*---------------------------------------------------------------------------------
* FUNCTION : VLAN_SHOM_AttachSystemResources
*---------------------------------------------------------------------------------
* PURPOSE  : Attach system resource for vlan OM in the context of the calling process.
* INPUT    : None
* OUTPUT   : None
* RETURN   : None
* NOTES    : None
 *---------------------------------------------------------------------------------*/
void VLAN_OM_AttachSystemResources(void)
{
    shmem_data_p = (VLAN_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_VLAN_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_VLAN_OM, &vlan_om_sem_id);
}
void VLAN_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p  = SYSRSC_MGR_VLAN_SHMEM_SEGID;
    *seglen_p = sizeof(VLAN_Shmem_Data_T);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_Init
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Initialize vlan_table and vlan_port_table
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-----------------------------------------------------------------------------------*/
void VLAN_OM_Init()
{
    memset(vlan_entry_offset, 0, sizeof(vlan_entry_offset));
    memset(vlan_table, 0, sizeof(vlan_table));
    memset(vlan_port_table, 0, sizeof(vlan_port_table));

    vlan_table_desc.buffer_offset = L_CVRT_GET_OFFSET(&vlan_table_desc,vlan_table);
    vlan_table_desc.buffer_len = sizeof(vlan_table);
    vlan_table_desc.partition_len = sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T);
    L_PT_ShMem_Create(&vlan_table_desc);

    vlan_om_current_cfg_vlan = 0;
    num_vlan_deletes = 0;
    last_update_time = 0;

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_VLAN_OM, &vlan_om_sem_id) != SYSFUN_OK)
    {
        printf("\n%s:get om sem id fail.\n", __FUNCTION__);
    }
} /* end of VLAN_OM_Init() */


/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ClearDatabase
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function is to clear the vlan table and vlan port table entryies.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. This function shall be invoked when system enters transition mode.
 *            2. All the entries in database will be purged.
 *-----------------------------------------------------------------------------------*/
void VLAN_OM_ClearDatabase()
{
    VLAN_OM_ClearAllVlanTable();
    VLAN_OM_ClearAllVlanPortTable();

    vlan_om_current_cfg_vlan = 0;
    num_vlan_deletes = 0;
    last_update_time = 0;

} /* end of VLAN_OM_ClearDatabase() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_DeleteVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if vlan is succussfully deleted.
 *            Otherwise, returns false.
 * INPUT    : vid -- specify which vlan to be deleted.
 * OUTPUT   : NONE
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_DeleteVlanEntry(UI32_T vid)
{
    BOOL_T  ret = FALSE;
    UI32_T  time ;

    if(!IS_VLAN_ID_VAILD(vid))
      return ret;

    time =  SYSFUN_GetSysTick();

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);
    if (vlan_entry_offset[vid-1]){

        L_PT_ShMem_Free(&vlan_table_desc, (void *)VLAN_OM_ENTRY_ADDR(vid - 1));
        vlan_entry_offset[vid - 1] = 0;

        num_vlan_deletes++;
        vlan_om_current_cfg_vlan--;
        last_update_time = time;
        ret =  TRUE;
    }
    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret;
} /* end of VLAN_OM_DeleteVlanEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan info is
              available.  Otherwise, return false.
 * INPUT    : vlan_info->dot1q_vlan_index  -- specify which vlan information to be retrieved
 * OUTPUT   : returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetVlanEntry(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info)
{
    BOOL_T  ret = FALSE;
    UI32_T  vid;

    if ((NULL != vlan_info)&&IS_VLAN_IFINDEX_VAILD(vlan_info->dot1q_vlan_index)){

        VLAN_IFINDEX_CONVERTTO_VID(vlan_info->dot1q_vlan_index, vid);

        VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);
        if (vlan_entry_offset[vid-1]){

            memcpy(vlan_info, VLAN_OM_ENTRY_ADDR(vid-1), sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
            ret =  TRUE;
        }
        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
    }


    return ret;
} /* end of VLAN_OM_GetVlanEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next vlan info entry is available.
 *            Otherwise, false is returned.
 * INPUT    : vlan_info->dot1q_vlan_index  -- specify which vlan information to be retrieved
 * OUTPUT   : return next available vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetNextVlanEntry(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info)
{
    BOOL_T  ret = FALSE;
    UI32_T  entry_index;

    if (NULL != vlan_info){

        VLAN_IFINDEX_CONVERTTO_VID(vlan_info->dot1q_vlan_index, entry_index);

        VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);
        for(; entry_index < SYS_ADPT_MAX_VLAN_ID; entry_index++){
            if (vlan_entry_offset[entry_index]){
                memcpy(vlan_info, VLAN_OM_ENTRY_ADDR(entry_index), sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
                ret =  TRUE;
                break;
            }
        }
        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
    }

    return ret;
}/* end of VLAN_OM_GetNextVlanEntry() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if vlan entry has been modified
 *            successfully. otherwise, false is return.
 * INPUT    : vlan_info->dot1q_vlan_index -- specify which vlan info to be modified.
 * OUTPUT   : vlan info has been updated.
 * RETURN   : TRUE / FALSE.
 * NOTES    : 1. If the specified entry does not exist, a new entry will be created.
 *            2. VLAN_OM shall use this function to create a new VLAN entry.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_SetVlanEntry(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info)
{
    BOOL_T  ret = FALSE;
    UI32_T  vid;
    UI32_T  time ;
    VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry_p;

    if ((NULL != vlan_info)&&IS_VLAN_IFINDEX_VAILD(vlan_info->dot1q_vlan_index)){

        VLAN_IFINDEX_CONVERTTO_VID(vlan_info->dot1q_vlan_index, vid);
        time =  SYSFUN_GetSysTick();
        VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

        if (vlan_entry_offset[vid-1]){
            memcpy(VLAN_OM_ENTRY_ADDR(vid-1), vlan_info, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
            last_update_time = time;
            ret =  TRUE;
        }else if (NULL != (vlan_entry_p = (VLAN_OM_Dot1qVlanCurrentEntry_T *)L_PT_ShMem_Allocate(&vlan_table_desc))){
            vlan_entry_offset[vid-1] = VLAN_OM_ENTRY_OFFSET(vlan_entry_p);
            memcpy(vlan_entry_p, vlan_info, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
            vlan_om_current_cfg_vlan++;
            last_update_time = time;
            ret =  TRUE;
        }

        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
    }

    return ret;
} /* end of VLAN_OM_SetVlanEntry() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_DeleteVlanPortEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the vlan port entry has been deleted.
 *            otherwise, returns false;
 * INPUT    : lport_ifindex -- specify which port information to be deleted
 * OUTPUT   : none
 * RETURN   : TRUE / fALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_DeleteVlanPortEntry(UI32_T lport_ifindex)
{
    UI32_T time =  SYSFUN_GetSysTick();

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    memset(&vlan_port_table[lport_ifindex-1], 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
    last_update_time = time;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return TRUE;
} /* end of VLAN_OM_DeleteVlanPortEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanPortEntryByIfindex
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan port entry is
              available. Otherwise, false is returned
 * INPUT    : vlan_port_info->lport_ifidx  -- specify which port information to
 *            be retrieved
 * OUTPUT   : returns the specific vlan port info.
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetVlanPortEntryByIfindex(UI32_T lport_ifindex,VLAN_OM_VlanPortEntry_T *vlan_port_entry)
{
    VLAN_OM_Vlan_Port_Info_T        port_info;

    port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&port_info))
      return FALSE;

    memcpy(vlan_port_entry, &port_info.vlan_port_entry, sizeof(VLAN_OM_VlanPortEntry_T));

    return TRUE;
} /* end of VLAN_OM_GetVlanPortEntryByIfindex() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanPortPvid
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan port entry is
              available. Otherwise, false is returned
 * INPUT    : lport_ifindex  -- specify which port information to be retrieved
 * OUTPUT   : dot1q_pvid_index -- the pvid information of the lport_ifindex.
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetVlanPortPvid(UI32_T lport_ifindex, UI32_T *dot1q_pvid_index)
{
    VLAN_OM_Vlan_Port_Info_T  vlan_port_info;

    memset(&vlan_port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
    vlan_port_info.lport_ifindex = lport_ifindex;
    if(!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
        return FALSE;

    *dot1q_pvid_index = vlan_port_info.port_item.dot1q_pvid_index;

    return TRUE;
} /* end of VLAN_OM_GetVlanPortPvid() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanPortEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan port entry is
              available. Otherwise, false is returned
 * INPUT    : vlan_port_info->lport_ifidx  -- specify which port information to
 *            be retrieved
 * OUTPUT   : returns the specific vlan port info.
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetVlanPortEntry(VLAN_OM_Vlan_Port_Info_T *vlan_port_info)
{
    if ((vlan_port_info == NULL)
       || (vlan_port_info->lport_ifindex < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER)
       ||(vlan_port_info->lport_ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT))
        return FALSE;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    memcpy(vlan_port_info, &vlan_port_table[(vlan_port_info->lport_ifindex)-1], sizeof(VLAN_OM_Vlan_Port_Info_T));

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return TRUE;
} /* end of VLAN_OM_GetVlanPortEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanPortEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next vlan port entry is available.
 *            Otherwise, false is returned.
 * INPUT    : vlan_port_info->lport_ifidx  -- specify which port information
 *            to be retrieved
 * OUTPUT   : next available VLAN port entry
 * RETURN   : the start address of the next available vlan port entry.
 *            otherwise, NULL.
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetNextVlanPortEntry(VLAN_OM_Vlan_Port_Info_T *vlan_port_info)
{
    if (vlan_port_info == NULL)
        return FALSE;

    if(vlan_port_info->lport_ifindex < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER)
        vlan_port_info->lport_ifindex = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER;
    else
        vlan_port_info->lport_ifindex++;

    if (vlan_port_info->lport_ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        return FALSE;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    memcpy(vlan_port_info, &vlan_port_table[(vlan_port_info->lport_ifindex)-1], sizeof(VLAN_OM_Vlan_Port_Info_T));

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return TRUE;
} /* end of VLAN_OM_GetNextVlanPortEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetVlanPortEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the new VLAN port info is successfully
 *            updated to the specified entry. Otherwise, false is returned.
 * INPUT    : *vlan_port_info -- The specific vlan port info to be modified
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. If the specified entry does not exist, a new entry will be created.
 *            2. VLAN_OM shall use this function to create a new VLAN port entry.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_SetVlanPortEntry(VLAN_OM_Vlan_Port_Info_T *vlan_port_info)
{
    UI32_T  time = SYSFUN_GetSysTick();

    if (vlan_port_info->lport_ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        return FALSE;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    memcpy(&vlan_port_table[(vlan_port_info->lport_ifindex)-1], vlan_port_info, sizeof(VLAN_OM_Vlan_Port_Info_T));
    last_update_time = time;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return TRUE;
} /* end of VLAN_OM_SetVlanPortEntry() */

#define VLAN_OM_ASCENT  FALSE   /* Local definition only for VLAN_OM_GetNextVlanMember */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanMember
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next Vlan member
 * INPUT    : vlan_ifindex  -- vlan ifindex
 *            lport         -- specified lport number
 *            type          -- one of the following type
 *                             VLAN_OM_VlanMemberType_CurrentUntagged   (0)
 *                             VLAN_OM_VlanMemberType_CurrentEgress     (1)
 *                             VLAN_OM_VlanMemberType_StaticEgress      (2)
 *                             VLAN_OM_VlanMemberType_ForbiddenEgress   (3)
 * OUTPUT   : lport         -- the next lport number of the specified lport
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : VLAN_OM_ASCENT is FALSE defined locally
 * ------------------------------------------------------------------------
 */
BOOL_T  VLAN_OM_GetNextVlanMember(UI32_T vlan_ifindex, UI32_T *lport, VLAN_OM_VlanMemberType_T type)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;
    BOOL_T  result = FALSE;
    UI8_T*  port_list;
    UI32_T  vid;

    if(!IS_VLAN_IFINDEX_VAILD(vlan_ifindex))
        return result;

    VLAN_IFINDEX_CONVERTTO_VID(vlan_ifindex, vid);

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if (vlan_entry_offset[vid-1]){
        memcpy(&vlan_entry, VLAN_OM_ENTRY_ADDR(vid-1), sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
        result = TRUE;
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    if (result){
        UI16_T  byte;
        UI8_T   bit;

        switch (type){

            case VLAN_OM_VlanMemberType_CurrentUntagged:
                port_list   = vlan_entry.dot1q_vlan_current_untagged_ports;
            break;

            case VLAN_OM_VlanMemberType_CurrentEgress:
                port_list   = vlan_entry.dot1q_vlan_current_egress_ports;
            break;

            case VLAN_OM_VlanMemberType_StaticEgress:
                port_list   = vlan_entry.dot1q_vlan_static_egress_ports;
             break;

            case VLAN_OM_VlanMemberType_ForbiddenEgress:
                port_list   = vlan_entry.dot1q_vlan_forbidden_egress_ports;
            break;

            default:
                port_list   = 0;
                break;
        }

        if (*lport){

            if (!VLAN_OM_ConvertIndexToPosition(*lport, &byte, &bit, VLAN_OM_ASCENT)){

                byte    = SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST;
                bit     = 8;
            }
        }else{

            byte    = SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST;
            bit     = 8;
        }

        if(port_list
            &&VLAN_OM_GetNextOnBit(port_list,
                                     SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST,
                                     &byte,
                                     &bit,
                                     VLAN_OM_ASCENT)){

            result  = VLAN_OM_ConvertIndexFmPosition(lport, byte, bit, VLAN_OM_ASCENT);
        }else{

            result  = FALSE;
        }
    }

    return result;
} /* End of VLAN_OM_GetNextVlanMember */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetMaxSupportVlanID
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the max vlan id number supported by the device.
 * INPUT    : None.
 * OUTPUT   : max_support_vid
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetMaxSupportVlanID(UI32_T *max_support_vid)
{

    *max_support_vid = SYS_DFLT_DOT1QMAXVLANID;

    return TRUE;
} /* end of VLAN_OM_GetMaxSupportVlanID() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetMaxSupportVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the maximum number of vlan supported
 *            by the system if returned.  Otherwise, false is returned.
 * INPUT    : None.
 * OUTPUT   : max_support_vlan
 * RETURN   : TRUE/FALSE
 * NOTES    : This value is defined in SYS_ADPT.H
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetMaxSupportVlan(UI32_T *max_support_vlan)
{

    *max_support_vlan = SYS_ADPT_MAX_NBR_OF_VLAN;

    return TRUE;
} /* end of VLAN_OM_GetMaxSupportVlan() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetCurrentNumbOfVlanConfigured
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the total number of vlans currently
 *             configured in the device is availabe.  Otherwise, false is returned.
 * INPUT    : None.
 * OUTPUT   : current_cfg_vlan
 * RETURN   : TRUE/FALSE
 * NOTES    : The total number of vlans currently configured in the device will include
 *            the static configured vlan and dynamic GVRP configured vlan.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetCurrentConfigVlan(UI32_T *current_cfg_vlan)
{

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    *current_cfg_vlan = (UI32_T) vlan_om_current_cfg_vlan;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return TRUE;
} /* end of VLAN_OM_GetCurrentConfigVlan() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetnumVlanDeletes
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the total number of times the specific
 *            vlan deleted from the device is available.  Otherwise, return false.
 * INPUT    : None
 * OUTPUT   : Number of vlan deleted from the system.
 * RETURN   : TRUE \ FALSE
 * NOTES    : The total number of times specific vlans has been deleted from the
 *            device will include the static and dynamic GVRP vlan delete.
 *--------------------------------------------------------------------------*/
UI32_T VLAN_OM_GetnumVlanDeletes(void)
{
    UI32_T ret_val;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    ret_val = num_vlan_deletes;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret_val;
} /* end of VLAN_OM_GetnumVlanDeletes() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qConstraintTypeDefault
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the constrain type for this device
 *            can be retrieve successfully.  Otherwise, return false.
 * INPUT    : None
 * OUTPUT   : VAL_dot1qConstraintTypeDefault_independent\
 *            VAL_dot1qConstraintTypeDefault_shared
 * RETURN   : TRUE \ FALSE
 * NOTES    : System_default is IVL.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetDot1qConstraintTypeDefault(UI32_T *constrain_type)
{

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    *constrain_type = dot1q_constraint_type_default;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return TRUE;
} /* end of VLAN_OM_GetDot1qConstraintTypeDefault() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetDot1qConstraintTypeDefault
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the constrain type for this device
 *            can be set successfully.  Otherwise, return false.
 * INPUT    : VAL_dot1qConstraintTypeDefault_independent\
 *            VAL_dot1qConstraintTypeDefault_shared
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : System_default is IVL.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_SetDot1qConstraintTypeDefault(UI32_T constrain_type)
{

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    dot1q_constraint_type_default = constrain_type;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return TRUE;
} /* end of VLAN_OM_SetDot1qConstraintTypeDefault() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_LastUpdateTime
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the time tick the last time vlan_om is
 *            updated.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : timetick.
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T  VLAN_OM_LastUpdateTime()
{
    UI32_T ret_val;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    ret_val = last_update_time;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret_val;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanDeleteFrequency
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the gvrp status for the device
 *            can be retrieve successfully.  Otherwise, return false.
 * INPUT    : None
 * OUTPUT   : gvrp_status - VAL_dot1qGvrpStatus_enabled \
 *                          VAL_dot1qGvrpStatus_disabled
 * RETURN   : TRUE \ FALSE
 * NOTES    : The administrative status requested by management for GVRP
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetGlobalGvrpStatus(UI32_T *gvrp_status)
{

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    *gvrp_status = global_dot1q_gvrp_status;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return TRUE;
} /* end of VLAN_OM_GetGlobalGvrpStatus() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetGlobalGvrpStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the gvrp status for the device
 *            can be set successfully.  Otherwise, return false.
 * INPUT    : gvrp_status - VAL_dot1qGvrpStatus_enabled \
 *                          VAL_dot1qGvrpStatus_disabled
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : The administrative status requested by management for GVRP
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_SetGlobalGvrpStatus(UI32_T gvrp_status)
{

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    global_dot1q_gvrp_status = gvrp_status;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return TRUE;
} /* end of VLAN_OM_SetGlobalGvrpStatus() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetGlobalDefaultVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default VLAN for this device
 *            can be set successfully.  Otherwise, return false.
 * INPUT    : vid
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_OM_SetGlobalDefaultVlan(UI32_T vid)
{

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    default_vlan_id = vid;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetGlobalDefaultVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default VLAN for this device
 *            can be get successfully.  Otherwise, return false.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : default vlan id
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VLAN_OM_GetGlobalDefaultVlan()
{
    UI32_T ret_val;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    ret_val = default_vlan_id;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret_val;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNext_Vlan_With_PortJoined
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next vlan info entry for the
 *            specified port is available. Otherwise, false is returned.
 * INPUT    : lport                        -- specify the lport ifindex
 *            vlan_info->dot1q_vlan_index  -- specify which vlan information to be retrieved
 * OUTPUT   : return next available vlan info
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetNext_Vlan_With_PortJoined(UI32_T lport, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info)
{
    BOOL_T  ret = FALSE;
    UI32_T  vid;
    UI32_T  entry_index;

    if (NULL != vlan_info)
    {
        VLAN_IFINDEX_CONVERTTO_VID(vlan_info->dot1q_vlan_index, vid);

        VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

        for (entry_index = vid; entry_index < SYS_ADPT_MAX_VLAN_ID; entry_index++)
        {
            if (vlan_entry_offset[entry_index])
            {
                if (VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_current_egress_ports, lport))
                {
                    memcpy(vlan_info, VLAN_OM_ENTRY_ADDR(entry_index), sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
                    ret =  TRUE;
                    break;
                }
            }
        }

        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
    }

    return ret;
}

/*=============================================================================
 * Moved from vlan_mgr.h
 *=============================================================================
 */

/* Dot1qBase group
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qVlanVersionNumber
 *-----------------------------------------------------------------------------
 * PURPOSE  : It is used to identify the vlan version of the system.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The vlan version number
 * NOTES    : 1. Please refer to IEEE 802.1Q/D11 Section 12.10.1.1 {dot1qBase 1}
 *               for detailed information.
 *            2. value define in mib.2674
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetDot1qVlanVersionNumber(void)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    return 2;
#else
    return LEAF_dot1qVlanVersionNumber;
#endif
} /* end of VLAN_OM_GetDot1qVlanVersionNumber()*/

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qMaxVlanId
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funciton returns the maximum (total) number of IEEE 802.1Q
 *            VLANs that this device supports.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The maximum number of VLAN supported by the system.
 * NOTES    : 1. Please refer to IEEE 802.1Q/D11 Section 12.10.1.1"{dot1qBase 3}
 *               for detailed information.
 *            2. It returns the "SYS_ADPT_MAX_NBR_OF_VLAN" defined of system.
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetDot1qMaxVlanId(void)
{
    UI32_T  max_van_id;

    /* BODY */

    if (VLAN_OM_GetMaxSupportVlanID(&max_van_id) == FALSE)
        return VLAN_MGR_RETURN_ZERO;
    else
        return max_van_id;
} /* End of VLAN_OM_GetDot1qMaxVlanId() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qNumVlans
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funciton returns the number of IEEE 802.1Q VLANs are
 *            currently configured in the system.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The current numbers of VLAN configured
 * NOTES    : 1. Please refer to IEEE 802.1Q/D11 Section 12.7.1.1" {dot1qBase 4}
 *               for detailed information.
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetDot1qNumVlans(void)
{
    UI32_T  curr_config_vlan;

    /* BODY */
    if (!VLAN_OM_GetCurrentConfigVlan(&curr_config_vlan))
        return VLAN_MGR_RETURN_ZERO;
    else
        return curr_config_vlan;

} /* end of VLAN_OM_GetDot1qNumVlans() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qMaxSupportedVlans
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funciton returns the maximum IEEE 802.1Q VLAN ID that
 *            this device supported.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The maximum number of VLAN ID
 * NOTES    : 1. Please refer to IEEE 802.1Q/D11 Section 9.3.2.3 {dot1qBase 2}
 *               for detailed information.
 *            2. It returns the "SYS_ADPT_MAX_VLAN_ID" defined of system.
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetDot1qMaxSupportedVlans(void)
{
    UI32_T  max_supported_vlan;

    /* BODY */

    if (!VLAN_OM_GetMaxSupportVlan(&max_supported_vlan))
        return VLAN_MGR_RETURN_ZERO;
    else
        return max_supported_vlan;
} /* End of VLAN_OM_GetDot1qMaxSupportedVlans */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_CurrentConfiguredMaxVlanId
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funciton returns the largest vlan id currently existed in
 *            the database.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The largest vlan id value
 * NOTES    : For CLI use
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_CurrentConfiguredMaxVlanId(void)
{
    UI32_T  max_vid = 0,entry_index;

    /* BODY */

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    for(entry_index = 0; entry_index < SYS_ADPT_MAX_VLAN_ID; entry_index++){
        if (vlan_entry_offset[entry_index])
             max_vid = entry_index;
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);


    return max_vid;
} /* end of VLAN_OM_CurrentConfiguredMaxVlanId() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qGvrpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the GVRP status of the bridge can be
 *            successfully retrieved.  Otherwise, return false.
 * INPUT    : none
 * OUTPUT   : gvrp_status - VAL_dot1qGvrpStatus_enabled \ VAL_dot1qGvrpStatus_disabled
 * RETURN   : TRUE \ FALSE
 * NOTES    : The administrative status requested by management for GVRP.  The
 *            value enabled(1) indicates that GVRP should be enabled on this
 *            device, on all ports for which it has not been specifically disabled.
 *            When disabled(2), GVRP is disabled on all ports and all GVRP packets
 *            will be forwarded transparently.  This object affects all GVRP
 *            Applicant and Registrar state machines.  A transition from disabled(2)
 *            to enabled(1) will cause a reset of all GVRP state machines on all ports
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetDot1qGvrpStatus(UI32_T *gvrp_status)
{
    BOOL_T  ret;

    ret = VLAN_OM_GetGlobalGvrpStatus(gvrp_status);

    return ret;
} /* end of VLAN_OM_GetDot1qGvrpStatus() */

/* Dot1qVlanCurrentTable
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qVlanNumDeletes
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns the number of vlan that has been deleted
 *            from vlan database.
 * INPUT    : None
 * OUTPUT   : Number of vlan that has been deleted.
 * RETURN   : The numbers of a VLAN entry has been deleted
 * NOTES    : 1. Please refer to IEEE 802.1Q/D11 {dot1qVlan 1} for detailed
 *               information.
 *            2. If an entry s deleted, then inserted, and then deleted, this
 *               counter will be incremented by 2
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetDot1qVlanNumDeletes(void)
{
    UI32_T      ret;
    /* BODY */
    ret =  VLAN_OM_GetnumVlanDeletes();

    return ret;
} /* End of VLAN_OM_GetDot1qVlanNumDeletes() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qVlanStaticEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific static vlan entry is
              available.  Otherwise, return false.
 * INPUT    : vid       -- the specific vlan id.
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : True/FALSE
 * NOTES    : The vid parameter is as the primary search key.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetDot1qVlanStaticEntry(UI32_T vid, VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    /* BODY */
    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
        return FALSE;


    VLAN_VID_CONVERTTO_IFINDEX(vid,vlan_info.dot1q_vlan_index);

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if (!VLAN_OM_GetVlanEntry_Local(&vlan_info)){

        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

        return FALSE;
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    /* This funciton only retrieves vlan entry that is in permanent status in database.
     */
    if (vlan_info.dot1q_vlan_status != VAL_dot1qVlanStatus_permanent)
         return FALSE;

    vlan_entry->dot1q_vlan_index = vid;

    memset(vlan_entry->dot1q_vlan_static_name, 0, sizeof(vlan_entry->dot1q_vlan_static_name));
    memcpy(vlan_entry->dot1q_vlan_static_name, &vlan_info.dot1q_vlan_static_name, strlen(vlan_info.dot1q_vlan_static_name));
    memcpy(vlan_entry->dot1q_vlan_static_egress_ports, &vlan_info.dot1q_vlan_static_egress_ports,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memcpy(vlan_entry->dot1q_vlan_static_untagged_ports, &vlan_info.dot1q_vlan_static_untagged_ports,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memcpy(vlan_entry->dot1q_vlan_forbidden_egress_ports, &vlan_info.dot1q_vlan_forbidden_egress_ports,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    vlan_entry->dot1q_vlan_static_row_status = vlan_info.dot1q_vlan_static_row_status;

#if (SYS_CPNT_RSPAN == TRUE)
    if (vlan_info.rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        memset(vlan_entry->dot1q_vlan_static_egress_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        memset(vlan_entry->dot1q_vlan_static_untagged_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        memset(vlan_entry->dot1q_vlan_forbidden_egress_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    }
#endif

    return TRUE;
} /* end of VLAN_OM_GetDot1qVlanStaticEntry() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextDot1qVlanStaticEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available static vlan entry is
              available.  Otherwise, return false.
 * INPUT    : vid       --  the specific vlan id.
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : The vid parameter is as the primary search key.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextDot1qVlanStaticEntry(UI32_T *vid, VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry)
{
    UI32_T      entry_index;
    BOOL_T      record_found = FALSE;


    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    for(entry_index = *vid; entry_index < SYS_ADPT_MAX_VLAN_ID; entry_index++)
    {
        if (    vlan_entry_offset[entry_index]
             && (VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_status == VAL_dot1qVlanStatus_permanent)
           )
        {
            vlan_entry->dot1q_vlan_index = VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_index;

            memcpy(vlan_entry->dot1q_vlan_static_name, VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_static_name, strlen(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_static_name));
            memcpy(vlan_entry->dot1q_vlan_static_egress_ports, VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_static_egress_ports,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
            memcpy(vlan_entry->dot1q_vlan_static_untagged_ports, VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_static_untagged_ports,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
            memcpy(vlan_entry->dot1q_vlan_forbidden_egress_ports, VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_forbidden_egress_ports,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

            vlan_entry->dot1q_vlan_static_row_status = VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_static_row_status;
            record_found =  TRUE;

#if (SYS_CPNT_RSPAN == TRUE)
            if (VLAN_OM_ENTRY_ADDR(entry_index)->rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
            {
                memset(vlan_entry->dot1q_vlan_static_egress_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                memset(vlan_entry->dot1q_vlan_static_untagged_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                memset(vlan_entry->dot1q_vlan_forbidden_egress_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
            }
#endif

            break;
        }
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    if(record_found == TRUE)
        VLAN_IFINDEX_CONVERTTO_VID(vlan_entry->dot1q_vlan_index, *vid);

    return record_found;
} /* end of VLAN_OM_GetNextDot1qVlanStaticEntry() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qVlanCurrentEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : vlan_om will return the specific vlan entry only in the case if
 *            dot1q_vlan_time_mark of the entry is greater than or equal to the
 *            input time_mark.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetDot1qVlanCurrentEntry(UI32_T time_mark, UI32_T vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry)
{

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
        return FALSE;

    VLAN_VID_CONVERTTO_IFINDEX(vid, vlan_entry->dot1q_vlan_index);

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if (!VLAN_OM_GetVlanEntry_Local(vlan_entry)){

        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
        return FALSE;

    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    if (vlan_entry->dot1q_vlan_time_mark < time_mark)
        return FALSE;

    vlan_entry->dot1q_vlan_index = (UI16_T)vid;

#if (SYS_CPNT_RSPAN == TRUE)
    if (vlan_entry->rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        memset(vlan_entry->dot1q_vlan_current_egress_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        memset(vlan_entry->dot1q_vlan_current_untagged_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        memset(vlan_entry->dot1q_vlan_static_egress_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        memset(vlan_entry->dot1q_vlan_static_untagged_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    }
#endif

    return TRUE;
} /* end of VLAN_OM_GetDot1qVlanCurrentEntry() */

/*----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextDot1qVlanCurrentEntry_forUI
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vid      -- the next vlan id
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : vlan_om will return the specific vlan entry only in the case if
 *            dot1q_vlan_time_mark of the next available entry is greater than or
 *            equal to the input time_mark.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetNextDot1qVlanCurrentEntry_forUI(UI32_T time_mark, UI32_T *vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry)
{

    if (VLAN_OM_GetNextDot1qVlanCurrentEntry(time_mark,vid, vlan_entry)){

        if (*vid > SYS_DFLT_DOT1QMAXVLANID){
            return FALSE;
        }else{
            return TRUE;
        }

    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextDot1qVlanCurrentEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vid      -- the next vlan id
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : vlan_om will return the specific vlan entry only in the case if
 *            dot1q_vlan_time_mark of the next available entry is greater than or
 *            equal to the input time_mark.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextDot1qVlanCurrentEntry(UI32_T time_mark, UI32_T *vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry)
{
    UI32_T  entry_index;
    BOOL_T  ret = FALSE;

    if(vlan_entry == NULL)
        return FALSE;

    /* BODY */
    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    for (entry_index = *vid; entry_index < SYS_ADPT_MAX_VLAN_ID; entry_index++)
    {
        if (    vlan_entry_offset[entry_index]
             && (VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_time_mark >= time_mark)
           )
        {
            memcpy(vlan_entry, VLAN_OM_ENTRY_ADDR(entry_index), sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
            *vid = entry_index + 1;
            ret =  TRUE;

#if (SYS_CPNT_RSPAN == TRUE)
            if (VLAN_OM_ENTRY_ADDR(entry_index)->rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
            {
                memset(vlan_entry->dot1q_vlan_current_egress_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                memset(vlan_entry->dot1q_vlan_current_untagged_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                memset(vlan_entry->dot1q_vlan_static_egress_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                memset(vlan_entry->dot1q_vlan_static_untagged_ports, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
            }
#endif

            break;
        }
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret;
} /* end of VLAN_OM_GetNextDot1qVlanCurrentEntry() */

/*----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextDot1qVlanCurrentEntrySortByTimemark
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vid      -- the next vlan id
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : vlan_om will return the specific vlan entry only in the case if
 *            dot1q_vlan_time_mark of the next available entry is greater than or
 *            equal to the input time_mark.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetNextDot1qVlanCurrentEntrySortByTimemark(UI32_T time_mark, UI32_T *vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry)
{
    UI32_T  entry_index;
    UI32_T vid_ifindex, short_timemark=0xffffffffL;
    BOOL_T find = FALSE;

    /* BODY */

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    VLAN_VID_CONVERTTO_IFINDEX(*vid, vid_ifindex);

    for (entry_index = 0; entry_index < SYS_ADPT_MAX_VLAN_ID; entry_index++)
    {
        if(!vlan_entry_offset[entry_index])
          continue;

        if (vid_ifindex == VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_index)
            continue;

        if ( ((VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_time_mark > time_mark)
              &&( VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_time_mark - time_mark < short_timemark)
             )
             ||
             (VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_time_mark == time_mark
              && VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_index > vid_ifindex
              && (!find /*same time_mark already found*/|| short_timemark != 0 /*ever update by not same time_mark*/)
             )
           )
        {
            short_timemark = VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_time_mark - time_mark;
            find   = TRUE;
            memcpy(vlan_entry, VLAN_OM_ENTRY_ADDR(entry_index), sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
            *vid = entry_index + 1;
        }
    }

    if(!find)
    {
      VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
      return FALSE;
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
    return TRUE;
} /* end of VLAN_OM_GetNextDot1qVlanCurrentEntrySortByTimemark() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qNextFreeLocalVlanIndex
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available value for dot1qVlanIndex
 *            of a local VLAN entry in dot1qVlanStaticTable can be retrieve successfully.
 *            Otherwise, return FALSE.
 * INPUT    : none
 * OUTPUT   : *vid      -- the next vlan id
 * RETURN   : TRUE\ FALSE
 * NOTES    : The vid parameter is as the primary search key.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetDot1qNextFreeLocalVlanIndex(UI32_T *vid)
{
    /* BODY */
    /* not support to create local-use VLAN */
    *vid = 0;

    return TRUE;
} /* end of VLAN_OM_GetDot1qNextFreeLocalVlanIndex() */

/* Management VLAN API
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetManagementVlanId
 *-----------------------------------------------------------------------------
 * PURPOSE : Set the global management VLAN
 * INPUT   : vid - management vlan id
 * OUTPUT  : None
 * RETURN  : TRUE \ FALSE
 * NOTES   : none
 *-----------------------------------------------------------------------------
 */
void VLAN_OM_SetManagementVlanId(UI32_T vid)
{
    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    management_vlan_id = vid;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetManagementVlanId
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the global management VLAN
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : vid - management vlan id
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetManagementVlanId(void)
{
    UI32_T ret_val;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    ret_val = management_vlan_id;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret_val;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetManagementVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : get the global management VLAN
 * INPUT    : none
 * OUTPUT   : vid - management vlan id
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. this function is paired with SetGlobalManagementVlan()
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetManagementVlan(UI32_T *vid)
{
    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    *vid = management_vlan_id;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return TRUE;
} /* End of VLAN_OM_GetManagementVlan() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextIpInterface
 *-----------------------------------------------------------------------------
 * PURPOSE  : Get the nex VLAN which is labeled as IP interface
 * INPUT    : vid
 * OUTPUT   : vid - management vlan id
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. This function is coupled with VLAN_OM_SetIpInterface()
 *               and VLAN_OM_LeaveManagementVlan()
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextIpInterface(UI32_T *vid)
{
    BOOL_T mgmt_state;
    UI8_T  ip_state;
    UI32_T find_vid;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    for (find_vid = *vid+1; find_vid <= SYS_ADPT_MAX_VLAN_ID; find_vid++){

        if((VLAN_OM_GetMgmtIpStateOfVlan(find_vid, &mgmt_state, &ip_state) == TRUE)
             && (ip_state != VLAN_MGR_IP_STATE_NONE)){

            *vid = find_vid;

            break;
        }
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    if (find_vid > SYS_ADPT_MAX_VLAN_ID){

        return FALSE;
    }else{

        return TRUE;
    }
} /* End of VLAN_OM_GetNextIpInterface() */

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetL3IPv6VlanState
 * ----------------------------------------------------------------------------
 * PURPOSE  : This function the state returns true if the vlan is l3 ipv6 vlan
 *            Otherwise, return FALSE.
 * INPUT    : vid - vlan id
 * OUTPUT   : state-is l3 ipv6 vlan or not.
 * RETURN   : TRUE \ FALSE
 * NOTES    : To be phased out, please use VLAN_OM_GetVlanMgmtIpState() instead
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetL3IPv6VlanState(UI32_T vid, BOOL_T *state)
{
    BOOL_T mgmt_state;
    UI8_T  ip_state;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if (VLAN_OM_GetMgmtIpStateOfVlan(vid, &mgmt_state, &ip_state) == FALSE){

        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
        return FALSE;
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    if (ip_state == VLAN_MGR_IP_STATE_IPV6){
        *state = TRUE;
    }else{
        *state = FALSE;
    }

    return TRUE;
} /* End of VLAN_OM_GetL3IPv6VlanState() */

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanMgmtIpState
 * ----------------------------------------------------------------------------
 * PURPOSE  : Get management vlan state and ip interface state of a vlan.
 * INPUT    : vid - the identifier of a vlan
 * OUTPUT   : mgmt_state - whether is management vlan
 *            ip_state   - whether has ip interface and which version
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetVlanMgmtIpState(UI32_T vid, BOOL_T* mgmt_state, UI8_T* ip_state)
{
    BOOL_T ret = FALSE;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if (vlan_entry_offset[vid-1]){
        *ip_state = VLAN_OM_ENTRY_ADDR(vid-1)->vlan_ip_state;
         ret = TRUE ;
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    if (vid == management_vlan_id)
        *mgmt_state = TRUE;
    else
        *mgmt_state = FALSE;

    return ret;
} /* End of VLAN_OM_GetVlanMgmtIpState() */

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanMgmtIpState
 * ----------------------------------------------------------------------------
 * PURPOSE  : Get management vlan state and ip interface state of next vlan.
 * INPUT    : vid - the identifier of a vlan
 * OUTPUT   : vid        - the identifier of the next existed vlan
 *            mgmt_state - whether is management vlan
 *            ip_state   - whether has ip interface and which version
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextVlanMgmtIpState(UI32_T* vid, BOOL_T* mgmt_state, UI8_T* ip_state)
{
    BOOL_T ret = FALSE;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    for(*vid = (*vid)+1; (*vid) <= SYS_ADPT_MAX_VLAN_ID; (*vid)++){

        if (vlan_entry_offset[*vid-1]){
            *ip_state = VLAN_OM_ENTRY_ADDR(*vid-1)->vlan_ip_state;
             ret = TRUE ;

             break;
        }

    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    if (*vid == management_vlan_id)
        *mgmt_state = TRUE;
    else
        *mgmt_state = FALSE;

    return ret;
} /* End of VLAN_OM_GetNextVlanMgmtIpState() */


/*  Miselleneous API
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanId
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next vlan id whose create_time is
 *            greater then time_mark is available.  Otherwise, return false.
 * INPUT    : time_mark -- time mark of serching key
 * OUTPUT   : *vid       -- vlan id
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. Time mark as the searching key.
 *            2. The condition for dynamic vlan existed is that if there is
 *               at least one port register within the member set.
 *            3. The condition for static vlan existed is that if the status
 *               is in "Row_Status_Active".
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextVlanId(UI32_T time_mark, UI32_T *vid)
{
    BOOL_T                  vlan_info_found = FALSE;

    /* BODY */

    if (*vid >= SYS_ADPT_MAX_VLAN_ID)
        return FALSE;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    for( ; *vid < SYS_ADPT_MAX_VLAN_ID; (*vid)++){

        if(vlan_entry_offset[*vid]
                &&(VLAN_OM_ENTRY_ADDR(*vid)->dot1q_vlan_time_mark >= time_mark)){

            vlan_info_found = TRUE;

            ++(*vid);
            break;
        }
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return vlan_info_found;
} /* VLAN_OM_GetNextVlanId() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsVlanExisted
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan exist in the database.
 *            Otherwise, return false.
 * INPUT    : vid     -- specified vlan id
 * OUTPUT   : none
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. The condition for dynamic vlan existed is that if there is
 *               one port regist within the port list.
 *            2. When a vlan is existed, it must either dynamic "Register" or
 *               static "Row_Status_Active" to identify active.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsVlanExisted(UI32_T vid)
{
    BOOL_T ret;

    if (!IS_VLAN_ID_VAILD(vid))
        return FALSE;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if ((vlan_entry_offset[vid-1]))
        ret = TRUE;
    else
        ret = FALSE;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret;
} /* End of VLAN_OM_IsVlanExisted() */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - UI8_T VLAN_OM_GetVlanStatus

 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan status in the database.
 * INPUT    : vid     -- specified vlan id
 * OUTPUT   : none
 * RETURN   :
 *-----------------------------------------------------------------------------
 */
UI8_T VLAN_OM_GetVlanStatus(UI32_T vid)
{
    UI8_T   vlan_operation_status = VAL_ifOperStatus_down;

    if (!IS_VLAN_ID_VAILD(vid))
       return vlan_operation_status ;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if (vlan_entry_offset[vid-1]){
       vlan_operation_status = VLAN_OM_ENTRY_ADDR(vid-1)->if_entry.vlan_operation_status;
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return vlan_operation_status;
}
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qVlanRowStatus
 *--------------------------------------------------------------------------
 * PURPOSE	:
 * INPUT		: vid  -- specified vlan id
 * OUTPUT 	: none
 * RETURN 	:
 * NOTES		: none
 *--------------------------------------------------------------------------*/
UI32_T VLAN_OM_GetDot1qVlanRowStatus(UI32_T vid)
{
	UI32_T vlan_row_status = VAL_dot1qVlanStaticRowStatus_notInService;

	if( (vid == 0) || (vid > SYS_ADPT_MAX_VLAN_ID) )
		return vlan_row_status;

	VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

	if( vlan_entry_offset[vid - 1] ) {
		vlan_row_status = VLAN_OM_ENTRY_ADDR(vid - 1)->dot1q_vlan_static_row_status;
	}

	VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

	return vlan_row_status;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsPortVlanMember_forUI
 *--------------------------------------------------------------------------
 * PURPOSE  : This funcion returns true if current port is in vlan's member
 *            list (egress_port).  Otherwise, returns false.
 * INPUT    : dot1q_vlan_index   -- vlan index number
              lport_ifindex -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_IsPortVlanMember_forUI(UI32_T dot1q_vlan_index, UI32_T lport_ifindex)
{
    UI32_T  vid;

    VLAN_IFINDEX_CONVERTTO_VID(dot1q_vlan_index, vid);

    if (!IS_VLAN_ID_VAILD(vid))
        return FALSE;

    return VLAN_OM_IsPortVlanMember(dot1q_vlan_index, lport_ifindex);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsPortVlanMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funcion returns true if current port is in vlan's member
 *            list (egress_port).  Otherwise, returns false.
 * INPUT    : dot1q_vlan_index   -- vlan index number
              lport_ifindex -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsPortVlanMember(UI32_T dot1q_vlan_index, UI32_T lport_ifindex)
{
    BOOL_T  ret = FALSE;
    UI32_T  vid;
    /* BODY*/

    if((lport_ifindex == 0)
        ||(lport_ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        ||(!IS_VLAN_IFINDEX_VAILD(dot1q_vlan_index)))
        return FALSE;

    VLAN_IFINDEX_CONVERTTO_VID(dot1q_vlan_index, vid);

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if (vlan_entry_offset[vid-1] == 0)
        ret = FALSE;
#if (SYS_CPNT_RSPAN == TRUE)
    else if (VLAN_OM_ENTRY_ADDR(vid-1)->rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
        ret = FALSE;
#endif
    else if (VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(vid-1)->dot1q_vlan_current_egress_ports, lport_ifindex))
        ret =  TRUE;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret;
} /* end VLAN_OM_IsPortVlanMember() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetIfTableLastUpdateTimePtr
 *-----------------------------------------------------------------------------
 * PURPOSE  : Get the pointer to the last time when vlan_om is updated by management
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : pointer to Time tick
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
UI32_T* VLAN_OM_GetIfTableLastUpdateTimePtr()
{
    return &if_table_last_change;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IfTableLastUpdateTime
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns the last time vlan_om is updated by management
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : Time tick
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_IfTableLastUpdateTime()
{
    UI32_T ret_val;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    ret_val = if_table_last_change;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret_val;
} /* end of VLAN_OM_IfTableLastUpdateTime() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetIfStackLastUpdateTime
 *-----------------------------------------------------------------------------
 * PURPOSE  : Set the value of the last time when vlan egress port list is modified
 * INPUT    : time_mark - the time value
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
void VLAN_OM_SetIfStackLastUpdateTime(UI32_T time_mark)
{

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if_stack_last_change = time_mark;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetIfStackLastUpdateTimePtr
 *-----------------------------------------------------------------------------
 * PURPOSE  : Get the pointer to the last time when vlan egress port list is modified
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : pointer to Time tick
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
UI32_T* VLAN_OM_GetIfStackLastUpdateTimePtr()
{
    return &if_stack_last_change;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IfStackLastUpdateTime
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns the last time vlan egress port list is modified.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : Time tick
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_IfStackLastUpdateTime()
{
    UI32_T ret_val;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    ret_val = if_stack_last_change;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret_val;
} /* end of VLAN_OM_IfStackLastUpdateTime() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsVlanUntagPortListMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if lport_ifindex is in vlan_info's
 *            dot1q_vlan_current_untagged_ports, otherwise, return false.
 * INPUT    : vlan_info -- typedef vlan struct that holds vlan information
              lport -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsVlanUntagPortListMember(UI32_T vid_ifindex, UI32_T lport_ifindex)
{
    BOOL_T   ret = FALSE;
    UI32_T   vid;
    /* BODY*/

    if((lport_ifindex == 0)
        ||  (lport_ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        ||(!IS_VLAN_IFINDEX_VAILD(vid_ifindex)))
        return FALSE;

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if (vlan_entry_offset[vid-1]
           &&VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(vid-1)->dot1q_vlan_current_untagged_ports, lport_ifindex))
        ret =  TRUE;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret;

} /* end VLAN_OM_IsVlanUntagPortListMember() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsVlanForbiddenPortMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if lport_ifindex is in vlan entry's
 *            dot1q_vlan_forbidden_egress_ports.  Otherwise, returns false.
 * INPUT    : vlan_info -- typedef vlan struct that holds vlan information
              lport -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsVlanForbiddenPortListMember(UI32_T vid_ifindex, UI32_T lport_ifindex)
{
    BOOL_T   ret = FALSE;
    UI32_T   vid;
    /* BODY*/

    if((lport_ifindex == 0)
        ||  (lport_ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        ||(!IS_VLAN_IFINDEX_VAILD(vid_ifindex)))
        return FALSE;

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if(vlan_entry_offset[vid-1]
        &&VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(vid-1)->dot1q_vlan_forbidden_egress_ports, lport_ifindex))
        ret =  TRUE;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret;
} /* end VLAN_OM_IsVlanForbiddenPortListMember() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetUncertifiedStaticEgressPorts
 *-----------------------------------------------------------------------------
 * PURPOSE  : Get the static egress ports which are not cerified by Radius server
 * INPUT    : vid           -- the search key
 * OUTPUT   : egress_ports  -- the port bit map
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetUncertifiedStaticEgressPorts(UI32_T vid, UI8_T *egress_ports)
{
    UI32_T  ifindex;
    BOOL_T  ret = FALSE;

    if (!IS_VLAN_ID_VAILD(vid))
        return FALSE;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if (vlan_entry_offset[vid-1])
    {
        memcpy(egress_ports, VLAN_OM_ENTRY_ADDR(vid-1)->dot1q_vlan_static_egress_ports, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        ret =  TRUE;
    }

    if (ret) {
        for(ifindex = 1; ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ifindex++){

            if (VLAN_OM_IS_MEMBER(egress_ports, ifindex))
            {
                if (vlan_port_table[ifindex-1].port_item.auto_vlan_mode)
                {
                    VLAN_OM_DEL_MEMBER(egress_ports, ifindex);
                }
            }
        }
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret;
} /* End of VLAN_OM_GetUncertifiedStaticEgressPorts() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanId_ByLport
 *-----------------------------------------------------------------------------
 * PURPOSE  : To know the specified port belongs to which VLANs
 * INPUT    : lport_ifindex     -- specify the lport
 *            member_type       -- VLAN_MGR_UNTAGGED_ONLY
 *                                 VLAN_MGR_TAGGED_ONLY
 *                                 VLAN_MGR_BOTH
 *            member_status     -- VLAN_MGR_PERMANENT_ONLY
 *                                 VLAN_MGR_DYNAMIC_ONLY
 *                                 VLAN_MGR_BOTH
 *            vid               -- the specific vlan id.
 * OUTPUT   : *vid              -- the next vlan id
 *            *is_tagged        -- only meaningful when member_type is VLAN_MGR_BOTH
 *                                 TRUE[tagged member]
 *                                 FALSE[untagged member]
 *            *is_static        -- only meaningful when member_status is VLAN_MGR_BOTH
 *                                 TRUE[static member]
 *                                 FALSE[dynamic member]
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T  VLAN_OM_GetNextVlanId_ByLport (UI32_T lport_ifindex, UI32_T member_type,
                                       UI32_T member_status, UI32_T *vid,
                                       BOOL_T *is_tagged, BOOL_T *is_static)
{
    UI32_T  entry_index;
    BOOL_T  is_found = FALSE;

    if ((lport_ifindex == 0) || (lport_ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT))
        return FALSE;

    if (*vid >= SYS_ADPT_MAX_VLAN_ID)
        return FALSE;

    if (    VLAN_MGR_UNTAGGED_ONLY != member_type
         && VLAN_MGR_TAGGED_ONLY != member_type
         && VLAN_MGR_BOTH != member_type
       )
        return FALSE;

    if (    VLAN_MGR_PERMANENT_ONLY != member_status
         && VLAN_MGR_DYNAMIC_ONLY != member_status
         && VLAN_MGR_BOTH != member_status
       )
        return FALSE;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    for (entry_index = *vid; (entry_index < SYS_ADPT_MAX_VLAN_ID)&&(is_found == FALSE); entry_index++)
    {
        if (vlan_entry_offset[entry_index])
        {
            switch (member_status)
            {
            case VLAN_MGR_PERMANENT_ONLY:
                if (VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_static_egress_ports, lport_ifindex))
                {
                    if (    VLAN_MGR_TAGGED_ONLY != member_type
                         && VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_static_untagged_ports, lport_ifindex)
                       )
                    {
                        *is_tagged = FALSE;
                        *is_static = TRUE;
                        is_found = TRUE;
                    }
                    else if (    VLAN_MGR_UNTAGGED_ONLY != member_type
                              && !VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_static_untagged_ports, lport_ifindex)
                            )
                    {
                        *is_tagged = TRUE;
                        *is_static = TRUE;
                        is_found = TRUE;
                    }
                }
                if (is_found)
                    goto lable;

                break;

            case VLAN_MGR_DYNAMIC_ONLY:
                if (    VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_current_egress_ports, lport_ifindex)
                     && !VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_static_egress_ports, lport_ifindex)
                   )
                {
                    if (    VLAN_MGR_TAGGED_ONLY != member_type
                         && VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_current_untagged_ports, lport_ifindex)
                       )
                    {
                        *is_tagged = FALSE;
                        *is_static = FALSE;
                        is_found = TRUE;
                    }
                    else if (    VLAN_MGR_UNTAGGED_ONLY != member_type
                              && !VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_current_untagged_ports, lport_ifindex)
                            )
                    {
                        *is_tagged = TRUE;
                        *is_static = FALSE;
                        is_found = TRUE;
                    }
                }

                if (is_found)
                    goto lable;

                break;

            case VLAN_MGR_BOTH:
                if (VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_current_egress_ports, lport_ifindex))
                {
                    if (    VLAN_MGR_TAGGED_ONLY != member_type
                         && VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_current_untagged_ports, lport_ifindex)
                       )
                    {
                        *is_tagged = FALSE;
                        is_found = TRUE;
                    }
                    else if (    VLAN_MGR_UNTAGGED_ONLY != member_type
                              && !VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_current_untagged_ports, lport_ifindex)
                            )
                    {
                        *is_tagged = TRUE;
                        is_found = TRUE;
                    }

                    if (is_found)
                    {
                        if (VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_static_untagged_ports, lport_ifindex))
                        {
                            *is_static = TRUE;
                        }
                        else
                        {
                            *is_static = FALSE;
                        }
                        goto lable;
                    }
                }
                break;
            }
        }
    }

 lable:
    if (is_found)
        //VLAN_IFINDEX_CONVERTTO_VID(vlan_entry_index[entry_index]->dot1q_vlan_index, *vid);
        (*vid = ((VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_index < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)? 0 : (VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_index -  SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1)));

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return is_found;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetGlobalDefaultVlan_Ex
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default VLAN for this device
 *            can be get successfully.  Otherwise, return false.
 * INPUT    : vid
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetGlobalDefaultVlan_Ex(UI32_T *vid)
{
    *vid = VLAN_OM_GetGlobalDefaultVlan();
    return TRUE;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetRunningGlobalDefaultVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default VLAN for this device
 *            can be get successfully.  Otherwise, return false.
 * INPUT    : none
 * OUTPUT   : vid
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : if the return value is SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *            there shall be "no vlan [SYS_DFLT_1Q_PORT_VLAN_PVID]" command
 *            in the running config
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetRunningGlobalDefaultVlan(UI32_T *vid)
{
    *vid = VLAN_OM_GetGlobalDefaultVlan();

    if (*vid == VLAN_MGR_DOT1Q_DEFAULT_PVID){
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNext_Vlan_With_PortJoined
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next vlan info entry for the
 *            specified port is available. Otherwise, false is returned.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 *            lport     -- the third search key
 * OUTPUT   : *vid      -- the next vlan id
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : 1. vlan_om will return the specific vlan entry only in the case if
 *               dot1q_vlan_time_mark of the next available entry is greater than or
 *               equal to the input time_mark.
 *            2. if the port only joins VLAN statically but currently, this function
 *               will not return vlan info for this vlan
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextDot1qVlanCurrentEntry_With_PortJoined(UI32_T time_mark, UI32_T *vid, UI32_T lport, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry)
{
    UI32_T entry_index;
    BOOL_T ret = FALSE;

    /* BODY */

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    for (entry_index = *vid; entry_index < SYS_ADPT_MAX_VLAN_ID; entry_index++)
    {
        if (vlan_entry_offset[entry_index])
        {
            if (    (VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_time_mark >= time_mark)
                 && (VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(entry_index)->dot1q_vlan_current_egress_ports, lport))
               )
            {
                memcpy(vlan_entry, VLAN_OM_ENTRY_ADDR(entry_index), sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
                VLAN_IFINDEX_CONVERTTO_VID(vlan_entry->dot1q_vlan_index, *vid);
                ret = TRUE;
                break;
            }
        }
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
    return ret;
}

VLAN_OM_VLIST_T *VLAN_OM_AddVlanList(VLAN_OM_VLIST_T *vlan_list, UI32_T vid)
{
    VLAN_OM_VLIST_T    *tmp_list = vlan_list;
    VLAN_OM_VLIST_T    *prev = NULL;
    VLAN_OM_VLIST_T    *new_vlist;
    BOOL_T             found = FALSE;

    while (tmp_list != NULL){

        if (tmp_list->vlan_id == vid){

            found = TRUE;
            break;
        }

        prev = tmp_list;
        tmp_list = tmp_list->next;
    } /* End of while */

    if (!found){

        new_vlist = (VLAN_OM_VLIST_T*)L_MM_Malloc(sizeof(VLAN_OM_VLIST_T), L_MM_USER_ID2(SYS_MODULE_VLAN, VLAN_TYPE_TRACE_ID_VLAN_MGR_ADDVLANLIST));
        if (new_vlist){

            memset(new_vlist, 0, sizeof(VLAN_OM_VLIST_T));
            new_vlist->vlan_id = vid;
            new_vlist->next = NULL;
            if (prev == NULL){

                vlan_list = new_vlist;
            }else{

                prev->next = new_vlist;
            }
        } /* End of if (new_vlist) */
    } /* End of if (not found) */

    return vlan_list;
}

#if (SYS_CPNT_MAC_VLAN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ClearMacVlanDatabase
 *-------------------------------------------------------------------------
 * PURPOSE  : Clear the MAC VLAN OM
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void VLAN_OM_ClearMacVlanDatabase(void)
{
    /*Set each field of mac_vlan_table to 0*/
    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);
    memset(&mac_vlan_table, 0, sizeof(mac_vlan_table));

    mac_vlan_entry_count =0;
    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the MAC VLAN entry
 * INPUT    : mac_address       - only allow unitcast address
 *            mask              - mask
 * OUTPUT   : mac_vlan_entry    - the MAC VLAN entry
 * RETURN   : TRUE/FALSE        - TRUE if successful;FALSE if failed
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetMacVlanEntry(VLAN_TYPE_MacVlanEntry_T *mac_vlan_entry)
{
    /*Get the entry whose mac_address is equivalent to input mac_address*/
    UI32_T i;

    if(mac_vlan_entry == NULL)
    {
        return FALSE;
    }

    /*if input mac_address is a null address, multicast address, or broadcast address, return FALSE*/
    if( IS_NULL_MAC(mac_vlan_entry->mac_address)
     || IS_MULTICAST_MAC(mac_vlan_entry->mac_address)
     || IS_BROADCAST_MAC(mac_vlan_entry->mac_address)
      )
    {
        return FALSE;
    }

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);
    for(i = 0; i < SYS_ADPT_MAX_NBR_OF_MAC_VLAN_ENTRY; i++)
    {
        if(memcmp(mac_vlan_entry->mac_address, mac_vlan_table[i].mac_address, SYS_ADPT_MAC_ADDR_LEN) == 0
           &&memcmp(mac_vlan_entry->mask, mac_vlan_table[i].mask, SYS_ADPT_MAC_ADDR_LEN) == 0)
        {
            *mac_vlan_entry  = mac_vlan_table[i];
            VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
            return TRUE;
        }
    }
    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the next MAC VLAN entry
 * INPUT    : mac_address       - only allow unitcast address
 *            mask              - mask
 * OUTPUT   : mac_vlan_entry    - the MAC VLAN entry
 * RETURN   : TRUE/FALSE        - TRUE if successful;FALSE if failed
 * NOTES    : It will sort by mask
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextMacVlanEntry(VLAN_TYPE_MacVlanEntry_T *mac_vlan_entry)
{
    UI8_T  found_mask[SYS_ADPT_MAC_ADDR_LEN], found_mac[SYS_ADPT_MAC_ADDR_LEN];
    I32_T  found_index = -1;
    UI32_T  i;

    if(mac_vlan_entry == NULL)
    {
        return FALSE;
    }

    /*if input mac_address is a multicast address, or broadcast address, return FALSE*/
    if( IS_MULTICAST_MAC(mac_vlan_entry->mac_address)
     || IS_BROADCAST_MAC(mac_vlan_entry->mac_address)
      )
    {
        return FALSE;
    }

    memset(found_mask, 0, SYS_ADPT_MAC_ADDR_LEN);
    memset(found_mac, 0xff, SYS_ADPT_MAC_ADDR_LEN);

    if(VLAN_OM_IsMacAddressNull(mac_vlan_entry->mac_address))
    	memset(mac_vlan_entry->mask, 0xff, SYS_ADPT_MAC_ADDR_LEN);

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);
    /*find the next entry whose mac_address is bigger than input mac_address*/
    for(i = 0; i < SYS_ADPT_MAX_NBR_OF_MAC_VLAN_ENTRY; i++)
    {
        /*if the entry is null, continue the loop*/
        if(VLAN_OM_IsMacAddressNull(mac_vlan_table[i].mask))
            continue;

        if(memcmp(mac_vlan_table[i].mask, mac_vlan_entry->mask, SYS_ADPT_MAC_ADDR_LEN)<0)
        {
            /*If this mask is shorter than current, then it should be a candidate,
                compare with previous found again */
            if (memcmp(mac_vlan_table[i].mask, found_mask, SYS_ADPT_MAC_ADDR_LEN)>0)
            {
              /* Mask is larger than previous found, replace found*/
              memcpy(found_mask, mac_vlan_table[i].mask, SYS_ADPT_MAC_ADDR_LEN);
              memcpy(found_mac, mac_vlan_table[i].mac_address, SYS_ADPT_MAC_ADDR_LEN);
              found_index = i;
            }
            else if (memcmp(mac_vlan_table[i].mask, found_mask, SYS_ADPT_MAC_ADDR_LEN)==0)
            {
                /* Mask is same as previous found, then compare mac-address again */
                if (memcmp(mac_vlan_table[i].mac_address, found_mac,SYS_ADPT_MAC_ADDR_LEN)<0)
                {
                    /* If the mask equal to previous and mac smaller than previous, then replace previous */
                    memcpy(found_mask, mac_vlan_table[i].mask, SYS_ADPT_MAC_ADDR_LEN);
                    memcpy(found_mac, mac_vlan_table[i].mac_address, SYS_ADPT_MAC_ADDR_LEN);
                    found_index = i;
                }

            }
        }
        else
        if (memcmp(mac_vlan_table[i].mask, mac_vlan_entry->mask, SYS_ADPT_MAC_ADDR_LEN) == 0)
        {
    	    /*For the mask is same as current*/
            if (memcmp(mac_vlan_table[i].mac_address, mac_vlan_entry->mac_address,SYS_ADPT_MAC_ADDR_LEN)>0)
            {
                /* Mac is larger than current, this is our candidate, then check if better than previous
                   Because the selected previous one mask should be equal or smaller than current
                   So we just check these two case*/
                if(memcmp(mac_vlan_table[i].mask, found_mask, SYS_ADPT_MAC_ADDR_LEN) >0)
                {
                    /* Mask larger than previous, this is better, then replace previous */
                    memcpy(found_mask, mac_vlan_table[i].mask, SYS_ADPT_MAC_ADDR_LEN);
                    memcpy(found_mac, mac_vlan_table[i].mac_address, SYS_ADPT_MAC_ADDR_LEN);
                    found_index = i;
                }
                else
                {
                    /* Here the mask should be equal to previous one */
                    if (memcmp(mac_vlan_table[i].mac_address, found_mac,SYS_ADPT_MAC_ADDR_LEN)<0)
                    {
                       /* If the mask equal to previous and mac smaller than previous, then replace previous */
                       memcpy(found_mask, mac_vlan_table[i].mask, SYS_ADPT_MAC_ADDR_LEN);
                       memcpy(found_mac, mac_vlan_table[i].mac_address, SYS_ADPT_MAC_ADDR_LEN);
                       found_index = i;
                    }
                }
            }
        }
    }

    if(found_index >= 0)
    {
        *mac_vlan_entry  = mac_vlan_table[found_index];
        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
        return TRUE;
    }
    else
    {
        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
        return FALSE;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextMacVlanEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the next MAC VLAN entry
 * INPUT    : next_index_p      - om index value
 * OUTPUT   : mac_vlan_entry    - the MAC VLAN entry
 *            *next_index_p     - current array index
 * RETURN   : TRUE/FALSE        - TRUE if successful;FALSE if failed
 * NOTES    : -1 to get first
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextMacVlanEntryByIndex(I32_T *next_index_p, VLAN_TYPE_MacVlanEntry_T *mac_vlan_entry)
{
    UI32_T  i;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    /*find the next entry whose mac_address is bigger than input mac_address*/
    for(i = *next_index_p+1; i < SYS_ADPT_MAX_NBR_OF_MAC_VLAN_ENTRY; i++)
    {
        /*if the entry is null, continue the loop*/
        if(FALSE == VLAN_OM_IsMacAddressNull(mac_vlan_table[i].mac_address))
            break;
    }

    if(i < SYS_ADPT_MAX_NBR_OF_MAC_VLAN_ENTRY)
    {
        *mac_vlan_entry = mac_vlan_table[i];
        *next_index_p = i;

        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
        return TRUE;
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the MAC VLAN entry
 * INPUT    : mac_address   - only allow unitcast address
 *            vid           - the VLAN ID
 *                            the valid value is 1 ~ SYS_DFLT_DOT1QMAXVLANID
 *            mask          - mask
 *            priority      - the priority
 *                            the valid value is 0 ~ 7
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : if SYS_CPNT_MAC_VLAN_WITH_PRIORITY == FALSE, it's recommanded
 *            that set input priority to 0.
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_SetMacVlanEntry(UI8_T *mac_address, UI8_T *mask, UI16_T vid, UI8_T priority)
{
    UI32_T new_index;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if(VLAN_OM_GetIndexOfNullMacVlanEntry(&new_index) == FALSE)
    {
        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
        return FALSE;  //no enough space to add new entry
    }

    memcpy(mac_vlan_table[new_index].mac_address, mac_address, SYS_ADPT_MAC_ADDR_LEN);
    mac_vlan_table[new_index].vid = vid;
    mac_vlan_table[new_index].priority = priority;
    memcpy(mac_vlan_table[new_index].mask, mask, SYS_ADPT_MAC_ADDR_LEN);
    mac_vlan_table[new_index].status = VAL_macVlanStatus_valid;
    mac_vlan_entry_count++;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_DeleteMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete Mac Vlan entry
 * INPUT    : mac_address   - only allow unitcast address
 *            mask          - mask
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_DeleteMacVlanEntry(UI8_T *mac_address, UI8_T *mask)
{
    UI32_T index;

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if(!VLAN_OM_GetIndexOfMacAddress(mac_address, mask, &index))
    {
        VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
        return TRUE;  //no such entry to delete
    }

    memset(&mac_vlan_table[index], 0, sizeof(VLAN_TYPE_MacVlanEntry_T));
    mac_vlan_entry_count --;
    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsMacVlanTableFull
 *-------------------------------------------------------------------------
 * PURPOSE  : Verify om is not space to store
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsMacVlanTableFull()
{
    UI32_T index;

    if(TRUE == VLAN_OM_GetIndexOfNullMacVlanEntry(&index))
    	return FALSE;

    return TRUE;
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - VLAN_OM_GetNextRunningMacVlanEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : Get next RUNNING MAC VLAN entry.
 * INPUT    : mac_address       - only allow unitcast address
 *                                use 0 to get the first entry
 * OUTPUT   : mac_vlan_entry    - the MAC VLAN entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL.
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T VLAN_OM_GetNextRunningMacVlanEntry(VLAN_TYPE_MacVlanEntry_T *mac_vlan_entry)
{
    if(TRUE == VLAN_OM_GetNextMacVlanEntry(mac_vlan_entry))
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

#if (SYS_CPNT_MAC_VLAN_IMPLEMENTED_BY_RULE== TRUE )
/* ----------------------------------------------------------------------------------
 * FUNCTION : VLAN_OM_CorrectIngressVidFromMacBasedVlan
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Correct the vid information
 * INPUT    :
 *            src_mac     -- Source address
 *            tag_info    -- raw tagged info of the packet
 * OUTPUT   : tag_info    -- raw tagged info of the packet
 * RETURN   : TRUE  -- change vid
 *            FALSE -- not mac vlan configure for this src mac
 * NOTE     :
 * ----------------------------------------------------------------------------------*/
BOOL_T VLAN_OM_CorrectIngressVidFromMacBasedVlan(
	                                             UI8_T *src_mac,
                                                 UI16_T *tag_info)
{
    UI32_T i, j, k, match_mask_len=0, cur_mask_len=0;
    BOOL_T replace = FALSE;

    if(mac_vlan_entry_count  == 0)
        return FALSE;
    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    for(i = 0; i < SYS_ADPT_MAX_NBR_OF_MAC_VLAN_ENTRY; i++)
    {
        if(mac_vlan_table[i].status == VAL_macVlanStatus_valid)
        {
            for(j=0; j<SYS_ADPT_MAC_ADDR_LEN; j++)
            {
                if((src_mac[j]&mac_vlan_table[i].mask[j]) != mac_vlan_table[i].mac_address[j])
                {
                    break;
                }
            }

             if(j == SYS_ADPT_MAC_ADDR_LEN)
             {
                cur_mask_len=0;

                for(j=0; j<SYS_ADPT_MAC_ADDR_LEN; j++)
                {
                    if(mac_vlan_table[i].mask[j] == 0xff)
                    {
                        cur_mask_len+=8;
                        continue;
                    }

                    for(k=0; k<8 ; k++)
                    {
                        if(mac_vlan_table[i].mask[j]&(0x80>>k))
                            cur_mask_len++;
                    }
                }

                if(cur_mask_len > match_mask_len)
                {
                    replace = TRUE;
                    match_mask_len = cur_mask_len;
                    *tag_info= (*tag_info&0xf000) | mac_vlan_table[i].vid;
                }
            }
        }
    }
    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return replace;
}
#endif /*SYS_CPNT_MAC_VLAN_IMPLEMENTED_BY_RULE== TRUE*/
#endif /*end of #if (SYS_CPNT_MAC_VLAN == TRUE)*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_EnterCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical section before a task invokes the spanning
 *              tree objects.
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   Utilize SYSFUN_OM_ENTER_CRITICAL_SECTION().
 *-------------------------------------------------------------------------
 */
void VLAN_OM_EnterCriticalSection(void)
{
    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_LeaveCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical section after a task invokes the spanning
 *              tree objects.
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   Utilize SYSFUN_OM_LEAVE_CRITICAL_SECTION().
 *-------------------------------------------------------------------------
 */
void VLAN_OM_LeaveCriticalSection(void)
{
    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for VLAN OM.
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
BOOL_T VLAN_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    VLAN_OM_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
       return FALSE;

    msg_p = (VLAN_OM_IpcMsg_T*)msgbuf_p->msg_buf;

    /* dispatch IPC message and call the corresponding VLAN_OM function
     */
    switch (msg_p->type.cmd){

        case VLAN_OM_IPC_GETVLANENTRY:
            msg_p->type.ret_bool =
                VLAN_OM_GetVlanEntry(&msg_p->data.arg_current_entry);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_current_entry);
            break;

        case VLAN_OM_IPC_GETNEXTVLANENTRY:
            msg_p->type.ret_bool =
                VLAN_OM_GetNextVlanEntry(&msg_p->data.arg_current_entry);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_current_entry);
            break;

        case VLAN_OM_IPC_GETVLANPORTENTRY:
            msg_p->type.ret_bool =
                VLAN_OM_GetVlanPortEntry(&msg_p->data.arg_port_info);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_port_info);
            break;

        case VLAN_OM_IPC_GETDOT1QCONSTRAINTTYPEDEFAULT:
        	msg_p->type.ret_bool =
                VLAN_OM_GetDot1qConstraintTypeDefault(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case VLAN_OM_IPC_GETDOT1QVLANVERSIONNUMBER:
        	msg_p->type.ret_ui32 = VLAN_OM_GetDot1qVlanVersionNumber();
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_GETDOT1QMAXVLANID:
            msg_p->type.ret_ui32 = VLAN_OM_GetDot1qMaxVlanId();
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_GETDOT1QMAXSUPPORTEDVLANS:
            msg_p->type.ret_ui32 = VLAN_OM_GetDot1qMaxSupportedVlans();
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_GETDOT1QNUMVLANS:
            msg_p->type.ret_ui32 = VLAN_OM_GetDot1qNumVlans();
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_GETDOT1QVLANNUMDELETES:
            msg_p->type.ret_ui32 = VLAN_OM_GetDot1qVlanNumDeletes();
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_GETDOT1QVLANSTATICENTRY:
            msg_p->type.ret_bool = VLAN_OM_GetDot1qVlanStaticEntry(
        	    msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case VLAN_OM_IPC_GETNEXTDOT1QVLANSTATICENTRY:
            msg_p->type.ret_bool = VLAN_OM_GetNextDot1qVlanStaticEntry(
        	    &msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case VLAN_OM_IPC_GETDOT1QVLANCURRENTENTRY:
            msg_p->type.ret_bool = VLAN_OM_GetDot1qVlanCurrentEntry(
        	    msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
        	    &msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case VLAN_OM_IPC_GETNEXTDOT1QVLANCURRENTRY_FORUI:
        	msg_p->type.ret_bool = VLAN_OM_GetNextDot1qVlanCurrentEntry_forUI(
        	    msg_p->data.arg_grp2.arg1, &msg_p->data.arg_grp2.arg2,
        	    &msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case VLAN_OM_IPC_GETNEXTDOT1QVLANCURRENTENTRY:
            msg_p->type.ret_bool = VLAN_OM_GetNextDot1qVlanCurrentEntry(
        	    msg_p->data.arg_grp2.arg1, &msg_p->data.arg_grp2.arg2,
        	    &msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case VLAN_OM_IPC_GETDOT1QNEXTFREELOCALVLANINDEX:
            msg_p->type.ret_bool =
                VLAN_OM_GetDot1qNextFreeLocalVlanIndex(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case VLAN_OM_IPC_GETMANAGEMENTVLAN:
            msg_p->type.ret_bool =
                VLAN_OM_GetManagementVlan(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case VLAN_OM_IPC_GETNEXTVLANID:
            msg_p->type.ret_bool = VLAN_OM_GetNextVlanId(
        	    msg_p->data.arg_grp3.arg1, &msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp3);
            break;

        case VLAN_OM_IPC_ISVLANEXISTED:
            msg_p->type.ret_bool =
                VLAN_OM_IsVlanExisted(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_ISPORTVLANMEMBER_FORUI:
        	msg_p->type.ret_bool = VLAN_OM_IsPortVlanMember_forUI(
        	    msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_ISPORTVLANMEMBER:
            msg_p->type.ret_bool = VLAN_OM_IsPortVlanMember(
        	    msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_ISVLANUNTAGPORTLISTMEMBER:
            msg_p->type.ret_bool = VLAN_OM_IsVlanUntagPortListMember(
        	    msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_ISVLANFORBIDDENPORTLISTMEMBER:
            msg_p->type.ret_bool = VLAN_OM_IsVlanForbiddenPortListMember(
        	    msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_CONVERTTOIFINDEX:
            msg_p->type.ret_bool = VLAN_VID_CONVERTTO_IFINDEX(
        	    msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp3);
            break;

        case VLAN_OM_IPC_CONVERTFROMIFINDEX:
            msg_p->type.ret_bool = VLAN_IFINDEX_CONVERTTO_VID(
        	    msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp3);
            break;

        case VLAN_OM_IPC_IFTABLELASTUPDATETIME:
            msg_p->type.ret_ui32 = VLAN_OM_IfTableLastUpdateTime();
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_IFSTACKLASTUPDATETIME:
            msg_p->type.ret_ui32 = VLAN_OM_IfStackLastUpdateTime();
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_OM_IPC_GETUNCERTIFIEDSTATICEGRESSPORTS:
            msg_p->type.ret_bool = VLAN_OM_GetUncertifiedStaticEgressPorts(
        	    msg_p->data.arg_grp4.arg1, msg_p->data.arg_grp4.arg2);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp4);
            break;

        case VLAN_OM_IPC_GETNEXTVLANID_BYLPORT:
            msg_p->type.ret_bool = VLAN_OM_GetNextVlanId_ByLport(
        	    msg_p->data.arg_grp5.arg1, msg_p->data.arg_grp5.arg2,
        	    msg_p->data.arg_grp5.arg3, &msg_p->data.arg_grp5.arg4,
        	    &msg_p->data.arg_grp5.arg5, &msg_p->data.arg_grp5.arg6);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp5);
            break;

        case VLAN_OM_IPC_GETGLOBALDEFAULTVLANEX:
            msg_p->type.ret_bool =
                VLAN_OM_GetGlobalDefaultVlan_Ex(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case VLAN_OM_IPC_GETRUNNINGGLOBALDEFAULTVLAN:
            msg_p->type.ret_ui32 =
                VLAN_OM_GetRunningGlobalDefaultVlan(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case VLAN_OM_IPC_GETNEXTDOT1QVLANCURRENTENTRY_WITH_PORTJOINED:
            msg_p->type.ret_bool = VLAN_OM_GetNextDot1qVlanCurrentEntry_With_PortJoined(
        	    msg_p->data.arg_grp6.arg1, &msg_p->data.arg_grp6.arg2,
        	    msg_p->data.arg_grp6.arg3, &msg_p->data.arg_grp6.arg4);
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp6);
            break;
        case VLAN_OM_IPC_GETVLANUPSTATE:
            msg_p->type.ret_bool =
                VLAN_OM_IsVlanUp(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
            break;
        case VLAN_OM_IPC_GETVLANPORTPVID:
            msg_p->type.ret_bool = VLAN_OM_GetVlanPortPvid(
                msg_p->data.arg_grp3.arg1,&(msg_p->data.arg_grp3.arg2));
            msgbuf_p->msg_size = VLAN_OM_GET_MSG_SIZE(arg_grp3);
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msgbuf_p->msg_size = VLAN_OM_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of VLAN_OM_HandleIPCReqMsg */


/* LOCAL SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ClearAllVlanTable
 *--------------------------------------------------------------------------
 * PURPOSE  : This function clears all the records in vlan table.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. Called by the enter transition mode.
 *--------------------------------------------------------------------------*/
static void VLAN_OM_ClearAllVlanTable(void)
{
    UI32_T  entry_index;

    for(entry_index = 0; entry_index < SYS_ADPT_MAX_VLAN_ID; entry_index++){
        if (vlan_entry_offset[entry_index]){
            L_PT_ShMem_Free(&vlan_table_desc, (void *)VLAN_OM_ENTRY_ADDR(entry_index));
            vlan_entry_offset[entry_index] = 0;
        }
    }

    return;
} /* end of VLAN_OM_ClearAllVlanTable() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ClearAllPortTable
 *--------------------------------------------------------------------------
 * PURPOSE  : This function clears all the records in port table.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. Called by the enter transition mode.
 *--------------------------------------------------------------------------*/
static void VLAN_OM_ClearAllVlanPortTable(void)
{
    memset(vlan_port_table, 0, sizeof(vlan_port_table));
    return;

} /* end of VLAN_OM_ClearAllPortTable() */


/* ===================================================================== */
/* Utilities */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ConvertIndexToPosition
 * ------------------------------------------------------------------------
 * PURPOSE  : Convert the index to the bit position
 * INPUT    : index     -- the order indicating the sequence of the bit
 *            ascent    -- TRUE if the LSB of the byte is the first bit,
 *                         else FALSE if MSB
 * OUTPUT   : byte      -- the entry indicator of the array
 *                          range: [0..(length-1)]
 *            bit       -- the bit indicator of the specified array entry
 *                          range: [0..7]
 * RETURN   : TRUE if OK, or FALSE if there is an error occurred.
 * NOTE     : 1. Ascent of the VLAN is FALSE
 *            2. Example:
 *                  ascent  :   FALSE       TRUE
 *                  index   :   [byte, bit] [byte, bit]
 *                  0       :   [-, -]      [-, -]
 *                  1       :   [0, 7]      [0, 0]
 *                  2       :   [0, 6]      [0, 1]
 * ------------------------------------------------------------------------
 */
static  BOOL_T  VLAN_OM_ConvertIndexToPosition(UI32_T index, UI16_T *byte, UI8_T *bit, BOOL_T ascent)
{
    UI8_T   pos_bit;
    BOOL_T  result;

    if (index){
        *byte   = (UI16_T)(index - 1) / 8;
        pos_bit = (UI8_T) (index - 1) % 8;
        *bit    = (ascent)?(pos_bit):(7 - pos_bit);
        result  = TRUE;

    }else{
        *byte   = 0;
        *bit    = 0;
        result  = FALSE;

    }

    return result;
} /* End of VLAN_OM_ConvertIndexToPosition */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ConvertIndexFmPosition
 * ------------------------------------------------------------------------
 * PURPOSE  : Convert the index from the bit position
 * INPUT    : byte      -- the entry indicator of the array
 *                          range: [0..(length-1)]
 *            bit       -- the bit indicator of the specified array entry
 *                          range: [0..7]
 *            ascent    -- TRUE if the LSB of the byte is the first bit,
 *                         else FALSE if MSB
 * OUTPUT   : index     -- the order indicating the sequence of the bit
 * RETURN   : TRUE if OK, or FALSE if there is an error occurred.
 * NOTE     : 1. Ascent of the VLAN is FALSE
 *            2. Example:
 *                  ascent  :   FALSE       TRUE
 *                  index   :   [byte, bit] [byte, bit]
 *                  1       :   [0, 7]      [0, 0]
 *                  2       :   [0, 6]      [0, 1]
 *                  60      :   [7, 4]      [7, 3]
 * ------------------------------------------------------------------------
 */
static  BOOL_T  VLAN_OM_ConvertIndexFmPosition(UI32_T *index, UI16_T byte, UI8_T bit, BOOL_T ascent)
{
    UI8_T   pos_bit;
    BOOL_T  result;

    if (bit < 8){

        pos_bit = (ascent)?(bit):(7 - bit);
        *index  = byte * 8 + pos_bit + 1;
        result  = TRUE;

    }else{
        result  = FALSE;
    }

    return result;
} /* End of VLAN_OM_ConvertIndexFmPosition */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextOnBit
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next On Bit
 * INPUT    : array     -- pointer to the bitmap
 *            length    -- length of the array
 *            index     -- the entry indicator of the array
 *                          range: [0..(length-1)]
 *            bit       -- the bit indicator of the specified array entry
 *                          range: [0..7]
 *            ascent    -- TRUE if the LSB of the array entry is the first
 *                         bit, else FALSE if MSB
 * OUTPUT   : index     -- the entry indicator of the array indicating the
 *                         next on_bit
 *            bit       -- the bit indicator of the specified array entry
 *                         indicating the next on_bit
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : if ( (index == length) && (bit == 8) ) then the first on_bit
 *            of the whole array is returned.
 *
 * ------------------------------------------------------------------------
 */
static  BOOL_T  VLAN_OM_GetNextOnBit(UI8_T *array, UI16_T length, UI16_T *index, UI8_T *bit, BOOL_T ascent)
{
    UI8_T   bmap_byte, bmap_mask;
    UI16_T  this_byte_index;
    UI8_T   this_bit_index, non_zero_bit;
    BOOL_T  found;

    /* (*bit)           : external bit sequence
     * this_bit_index   : internal bit sequence
     */
    /* Convert external bit to internal bit */
    if ( (*index) == length ){

        this_byte_index = 0;
        this_bit_index  = 0;

    }else{

        this_byte_index = *index;
        this_bit_index  = ascent?((*bit)+1):(8-(*bit));
    }

    found   = FALSE;
    while ( (!found) && (this_byte_index < length)){
        bmap_byte   = array[this_byte_index];
        if (ascent){
            bmap_mask   = 0xFF << this_bit_index;
        }else{
            bmap_mask   = ~( 0xFF << (8-this_bit_index) );
        }
        bmap_byte   &= bmap_mask;

        if (bmap_byte){
            found   = TRUE;
            non_zero_bit    = ascent?0:7;

            while ( !(bmap_byte & (0x01<<non_zero_bit) ) ){
                if (ascent){
                    non_zero_bit++;
                }else{
                    non_zero_bit--;
                }
            }

        }else{
            this_byte_index++;
            this_bit_index  = 0;
        }
    } /* End of while */

    /* non_zero_bit : external bit sequence */
    if (found){
        *index  = this_byte_index;
        *bit    = non_zero_bit;
        return  TRUE;

    }else{
        *index  = length;
        *bit    = 0;
        return  FALSE;
    }
} /* End of VLAN_OM_GetNextOnBit */


/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetMgmtIpStateOfVlan
 * ----------------------------------------------------------------------------
 * PURPOSE  : Get management state and ip interface state of a vlan.
 * INPUT    : vid - the identifier of vlan
 * OUTPUT   : mgmt_state - whether is management vlan
 *            ip_state   - whether has ip interface and which version
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
static BOOL_T VLAN_OM_GetMgmtIpStateOfVlan(UI32_T vid, BOOL_T* mgmt_state, UI8_T* ip_state)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T  vlan_info;

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
        return FALSE;

    VLAN_VID_CONVERTTO_IFINDEX(vid, vlan_info.dot1q_vlan_index);

    if (!VLAN_OM_GetVlanEntry_Local(&vlan_info))
        return FALSE;

    *ip_state = vlan_info.vlan_ip_state;

    if (vid == management_vlan_id){
        *mgmt_state = TRUE;
    }else{
        *mgmt_state = FALSE;
    }

    return TRUE;
} /* End of VLAN_OM_GetMgmtIpStateOfVlan() */

static BOOL_T VLAN_OM_GetVlanEntry_Local(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info)
{
    UI32_T  vid;

    if (vlan_info == NULL)
    {
        return FALSE;
    }

    VLAN_IFINDEX_CONVERTTO_VID(vlan_info->dot1q_vlan_index,vid);

    if (vid == 0)
    {
        return FALSE;
    }

    if (vlan_entry_offset[vid-1])
    {
        memcpy(vlan_info, VLAN_OM_ENTRY_ADDR(vid-1), sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL_T VLAN_OM_IsVlanUp(UI32_T vid)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T 	vlan_info;

    if (!IS_VLAN_ID_VAILD(vid))
        return FALSE;

    VLAN_VID_CONVERTTO_IFINDEX(vid, vlan_info.dot1q_vlan_index);

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
        return FALSE;

    if (((UI32_T)vlan_info.if_entry.vlan_operation_status) == VAL_ifOperStatus_up)
        return TRUE;
    else
        return FALSE;

}

BOOL_T VLAN_OM_GetDot1qPortVlanEntry(UI32_T lport_ifindex, VLAN_OM_Dot1qPortVlanEntry_T *vlan_port_entry)
{
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;

    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
        return FALSE;

    VLAN_IFINDEX_CONVERTTO_VID(vlan_port_info.port_item.dot1q_pvid_index,vlan_port_info.port_item.dot1q_pvid_index );

    memcpy(vlan_port_entry, &vlan_port_info.port_item, sizeof(VLAN_OM_Dot1qPortVlanEntry_T));

    return TRUE;

}/* end of VLAN_MGR_GetDot1qPortVlanEntry()*/

#if (SYS_CPNT_MAC_VLAN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsMacAddressNull
 *-------------------------------------------------------------------------
 * PURPOSE  : Check if a MAC Address is equal to 0
 * INPUT    : mac_address
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static BOOL_T VLAN_OM_IsMacAddressNull(UI8_T *mac_address)
{
    UI8_T null_mac[SYS_ADPT_MAC_ADDR_LEN] = {0,0,0,0,0,0};

    return (memcmp(mac_address, null_mac, SYS_ADPT_MAC_ADDR_LEN) == 0);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetIndexOfNullMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the index of 1st null entry
 * INPUT    : None
 * OUTPUT   : index
 * RETURN   : TRUE if found;FALSE if not found
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static BOOL_T VLAN_OM_GetIndexOfNullMacVlanEntry(UI32_T *index)
{
    UI32_T i;

    for(i = 0; i < SYS_ADPT_MAX_NBR_OF_MAC_VLAN_ENTRY; i++)
    {
        if(VLAN_OM_IsMacAddressNull(mac_vlan_table[i].mac_address))
        {
            *index = i;
            return TRUE;
        }
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetIndexOfMacAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the index of the MAC address in mac_vlan_table
 * INPUT    : mac_address
 * OUTPUT   : index
 * RETURN   : TRUE if found;FALSE if not found
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static BOOL_T VLAN_OM_GetIndexOfMacAddress(UI8_T *mac_address, UI8_T *mask, UI32_T *index)
{
    UI32_T i;

    for(i = 0; i < SYS_ADPT_MAX_NBR_OF_MAC_VLAN_ENTRY; i++)
    {
        if(memcmp(mac_vlan_table[i].mac_address, mac_address, SYS_ADPT_MAC_ADDR_LEN) == 0
           &&memcmp(mac_vlan_table[i].mask, mask, SYS_ADPT_MAC_ADDR_LEN) == 0)
        {
            *index = i;
            return TRUE;
        }
    }

    return FALSE;
}

#endif /*end of #if (SYS_CPNT_MAC_VLAN == TRUE)*/

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsPortVlanStaticMember
 *--------------------------------------------------------------------------
 * PURPOSE  : Check if a port is a static member of a VLAN.
 * INPUT    : vid   -- vlan index number
              lport -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE  -- is static member
 *            FALSE -- is not static member
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsPortVlanStaticMember(UI32_T vid, UI32_T lport)
{
    BOOL_T                              ret;

    if(!IS_VLAN_ID_VAILD(vid))
        return FALSE;

    if ((lport == 0) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return FALSE;
    }

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    if (vlan_entry_offset[vid-1])
    {
        ret = VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(vid-1)->dot1q_vlan_static_egress_ports, lport);
    }
    else
        ret = FALSE;

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret;
} /* end VLAN_OM_IsPortVlanStaticMember() */

BOOL_T VLAN_OM_GetPortAddedVlanBitmap(UI32_T lport,
                                      UI8_T tagged_bitmap[(SYS_ADPT_MAX_VLAN_ID/8)+1],
                                      UI8_T untagged_bitmap[(SYS_ADPT_MAX_VLAN_ID/8)+1])
{
    UI16_T next_vid;

    if ((lport == 0) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return FALSE;
    }

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);

    for(next_vid = 0; next_vid < SYS_ADPT_MAX_VLAN_ID; next_vid++)
    {
      if (vlan_entry_offset[next_vid] == 0)
        continue;

      if(VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(next_vid)->dot1q_vlan_current_egress_ports, lport))
      	VLAN_OM_ADD_MEMBER(tagged_bitmap, (next_vid+1));

      if(VLAN_OM_IS_MEMBER(VLAN_OM_ENTRY_ADDR(next_vid)->dot1q_vlan_current_untagged_ports, lport))
      	VLAN_OM_ADD_MEMBER(untagged_bitmap, (next_vid+1));
    }

    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return TRUE;
}

BOOL_T VLAN_OM_IsPortVlanInactiveMember(UI32_T vid, UI32_T lport)
{
    BOOL_T  ret;

    if(!IS_VLAN_ID_VAILD(vid))
    {
        return FALSE;
    }

    if ((lport == 0) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return FALSE;
    }

    VLAN_OM_ENTER_CRITICAL_SECTION(vlan_om_sem_id);
    {
        ret = FALSE;
    }
    VLAN_OM_LEAVE_CRITICAL_SECTION(vlan_om_sem_id);

    return ret;
} /* VLAN_OM_IsPortVlanInactiveMember */

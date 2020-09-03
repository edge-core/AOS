/* MODULE NAME: mldsnp_om.C
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*    {2. Related documents or hardware information}
* NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitations}
*    {4. Any performance limitations}
*    {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007     Macauley_Cheng Create
*
* Copyright(C)      Accton Corporation, 2007
*/

/* INCLUDE FILE DECLARATIONS
*/
#include "sys_cpnt.h"
#if (SYS_CPNT_MLDSNP == TRUE)
#include "mldsnp_om.h"
#include "mldsnp_pom.h"
#include "l_cvrt.h"
#include "sysrsc_mgr.h"
#include "vlan_lib.h"
#include "l_linklist.h"
/* NAMING CONSTANT DECLARATIONS
*/
/*define key field lenth*/
#define VID_LEN                            (sizeof(((MLDSNP_OM_HisamEntry_T *)NULL)->vid))
#define GROUP_LEN                          MLDSNP_TYPE_IPV6_DST_IP_LEN
#define SRC_IP_LEN                         MLDSNP_TYPE_IPV6_SRC_IP_LEN
#define VID_GROUP_SRCIP_LEN                VID_LEN+GROUP_LEN+SRC_IP_LEN

#define GROUP_ENTRY_NODE_LEN               sizeof(MLDSNP_OM_HisamEntry_T)
#define GROUP_ENTRY_NUMBER_OF_KEYS         1


/* constant definitions for HISAM
*/
#define INDEX_NBR                           10                 /* table is divided to 100 blocks  */
#define HISAM_N1                            3                  /* table balance threshold */
#define HISAM_N2                            13                 /* table balance threshold */
#define HASH_DEPTH                          4

#define GROUP_ENTRY_NODE_NBR                SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY
#define GROUP_ENTRY_HASH_NBR                (GROUP_ENTRY_NODE_NBR / 10)


#define HAVE_DYNAMIC_PORT   BIT_1
#define HAVE_STATIC_PORT     BIT_2
#define HAVE_UNKNOWN_PORT   BIT_3

/* MACRO FUNCTION DECLARATIONS
*/
#define MLDSNP_OM_LOCK()  {/*if(semaphore_flag>0) printf("%s.%d, \t\t\tLOCK\n", __FUNCTION__, __LINE__); semaphore_flag++; */SYSFUN_TakeSem(mldsnp_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);}
#define MLDSNP_OM_UNLOCK() {/*semaphore_flag--; if(semaphore_flag>0 || semaphore_flag<0)printf("%s.%d, \t\t\tUNLOCK\n", __FUNCTION__, __LINE__);semaphore_flag=0;*/SYSFUN_GiveSem(mldsnp_om_sem_id); }

/* DATA TYPE DECLARATIONS
*/


const static L_HISAM_KeyDef_T group_entry_key_def_table[GROUP_ENTRY_NUMBER_OF_KEYS] =
{
    /* vid, group, src, */
    {   3,                          /* field number */
        {0, VID_LEN, GROUP_LEN + VID_LEN, 0, 0, 0, 0, 0},  /* offset */
        {VID_LEN, GROUP_LEN, SRC_IP_LEN, 0, 0, 0, 0, 0}   /* len */
    }
};

const static L_HISAM_KeyType_T group_entry_key_type_table[GROUP_ENTRY_NUMBER_OF_KEYS][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
{
    /* vid, group, src, */
    {L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER,
        L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER}
};


/* LOCAL SUBPROGRAM DECLARATIONS
*/
static void MLDSNP_OM_CreateHisameTaable();
static BOOL_T MLDSNP_OM_HisamSetRecord(
    L_HISAM_Desc_T *desc_p,
    UI8_T *rec_p);
static BOOL_T MLDSNP_OM_HisamGetRecord(
    L_HISAM_Desc_T *desc_p,
    UI32_T kidx,
    UI8_T *key_p,
    UI8_T *rec_p);
static BOOL_T MLDSNP_OM_HisamDeleteRecord(
    L_HISAM_Desc_T *desc_p,
    UI8_T *key_p);
static BOOL_T MLDSNP_OM_HisamGetNextRecord(
    L_HISAM_Desc_T *desc_p,
    UI32_T kidx,
    UI8_T *key_p,
    UI8_T *rec_p);
static void MLDSNP_OM_SetVidGroupSrcKey(
    UI8_T *key_p,
    UI32_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap);
static BOOL_T MLDSNP_OM_AddPortIntoHisamPortList(
    MLDSNP_OM_HisamEntry_T *hisam_entry_p,
    MLDSNP_OM_PortInfo_T   *port_info_p);
static BOOL_T MLDSNP_OM_LocalPortIncStat(UI32_T lport, UI32_T vid, MLDSNP_TYPE_Statistics_T type, BOOL_T inc);
static BOOL_T MLDSNP_OM_LocalVlanIncStat(UI32_T vid, MLDSNP_TYPE_Statistics_T type, BOOL_T inc);

/* STATIC VARIABLE DEFINITIONS
*/
/*
 *  Database storage define
 */
static MLDSNP_OM_Cfg_T            mldsnp_om_cfg_db_g;
static MLDSNP_OM_StaticPortJoin_T static_join_group_entry[SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP + 1];
static L_HISAM_Desc_T             group_entry_hisam_desc_g;
static L_SORT_LST_List_T          mldsnp_om_vlan_list_g;
static UI32_T                     mldsnp_om_sem_id;
//static I32_T                      semaphore_flag; /*for debug lock and unlock*/
static UI8_T                      v1_host_preset_portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
MLDSNP_Timer_T                    *mldsnp_om_querier_timer_p;             /*store quereire timer pointer*/
MLDSNP_Timer_T                    *mldsnp_om_mrd_solicitation_timer_p;    /*store mrd solicitation timer pointer*/
MLDSNP_Timer_T                    *mldsnp_om_unsolicite_timer_p;          /*store unsolicitation timer pointer*/

/*if these constant variable have assign value, it will have exception when compile time can't identify it*/
const UI8_T mldsnp_om_null_src_ip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN]   ={0};
const UI8_T mldsnp_om_null_group_ip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN] ={0};

#if ( SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
#define SIZE_OF_MLD_FILTER_POOL            sizeof(MLDSNP_OM_FilterInfo_T)
#define SIZE_OF_MLD_PROFILE_POOL           (sizeof(MLDSNP_OM_ProfileInfo_T) * SYS_ADPT_MLD_PROFILE_TOTAL_NBR)
/*a port only can assign one profile*/
#define SIZE_OF_MLD_PORT_PROFILE_POOL      (sizeof(UI32_T) * SYS_ADPT_TOTAL_NBR_OF_LPORT)
#define SIZE_OF_MLD_PROFILE_GROUP_POOL     (sizeof(MLDSNP_OM_ProfileGroupEntry_T) * SYS_ADPT_MLD_TOTAL_PROFILE_GROUPS)
#define SIZE_OF_MLD_THROTTLE_POOL          (sizeof(MLDSNP_OM_Throttle_T) *SYS_ADPT_MLD_THROTTLE_TOTAL_NBR_OF_PORT)

static MLDSNP_OM_ProfileInfo_T         *free_profile_ptr;
static MLDSNP_OM_FilterInfo_T          mld_filter_db;
static UI32_T                          mld_port_profile_id[SYS_ADPT_MLD_PROFILE_TOTAL_NBR];
static MLDSNP_OM_ProfileGroupEntry_T   *free_profile_group_ptr;
static MLDSNP_OM_Throttle_T            mld_throttle_db[SYS_ADPT_MLD_THROTTLE_TOTAL_NBR_OF_PORT];

static MLDSNP_OM_ProfileInfo_T         mld_profile_pool[SYS_ADPT_MLD_PROFILE_TOTAL_NBR];
static MLDSNP_OM_ProfileGroupEntry_T   mld_profile_group_pool[SYS_ADPT_MLD_TOTAL_PROFILE_GROUPS];
#endif

static MLDSNP_OM_Counter_T port_statis[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static MLDSNP_OM_Counter_T vlan_statis[SYS_ADPT_MAX_VLAN_ID];

/* LOCAL SUBPROGRAM BODIES
*/
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_OM_CreateHisameTaable
 * ------------------------------------------------------------------------
 * PURPOSE  : this function will create the om hisam table for group entries
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void MLDSNP_OM_CreateHisameTaable()
{
    group_entry_hisam_desc_g.hash_depth         = HASH_DEPTH;
    group_entry_hisam_desc_g.N1                 = HISAM_N1;
    group_entry_hisam_desc_g.N2                 = HISAM_N2;
    group_entry_hisam_desc_g.record_length      = GROUP_ENTRY_NODE_LEN;
    group_entry_hisam_desc_g.total_hash_nbr     = GROUP_ENTRY_HASH_NBR;
    group_entry_hisam_desc_g.total_index_nbr    = INDEX_NBR;
    group_entry_hisam_desc_g.total_record_nbr   = GROUP_ENTRY_NODE_NBR;

    if (!L_HISAM_CreateV2(&group_entry_hisam_desc_g, GROUP_ENTRY_NUMBER_OF_KEYS, group_entry_key_def_table, group_entry_key_type_table))
    {
        printf(" Create HISAM Error !\n");
        while (1);
    }
}


/* ------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_HisamSetRecord
 * ------------------------------------------------------------------------
 * PURPOSE  : the function set the record to the hisam table
 * INPUT    : *desc_p  -  the hisam descriptor
 *            *rec_p   - the record will be put into hisame table
 * OUTPUT   : None
 * RETUEN   : TRUE   - sucess
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_OM_HisamSetRecord(
    L_HISAM_Desc_T *desc_p,
    UI8_T          *rec_p)
{
    UI32_T return_value;

    return_value = L_HISAM_SetRecord(desc_p, rec_p, TRUE);

    if ((L_HISAM_INSERT != return_value) && (L_HISAM_REPLACE != return_value))
    {
        return FALSE;
    }
    return TRUE;;
}/*end of MLDSNP_OM_HisamSetRecord*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_HisamSetRecord
 * ------------------------------------------------------------------------
 * PURPOSE  : the function get the record from the hisam table
 * INPUT    : *desc_p  -  the hisam descriptor
 *            *rec_p   - the record will be put into hisame table
 * OUTPUT   : None
 * RETUEN   : TRUE   - sucess
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_OM_HisamGetRecord(
    L_HISAM_Desc_T *desc_p,
    UI32_T         kidx,
    UI8_T          *key_p,
    UI8_T          *rec_p)
{
    BOOL_T return_value;

    return_value = L_HISAM_GetRecord(desc_p, kidx, key_p, rec_p);

    return return_value;
}/*end of MLDSNP_OM_HisamGetRecord*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_HisamGetNextRecord
 * ------------------------------------------------------------------------
 * PURPOSE  : the function get the next record from the hisam table
 * INPUT    : *desc_p  -  the hisam desc_priptor
 *            *rec_p   - the record will be put into hisame table
 * OUTPUT   : None
 * RETUEN   : TRUE   - sucess
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_OM_HisamGetNextRecord(
    L_HISAM_Desc_T  *desc_p,
    UI32_T          kidx,
    UI8_T           *key_p,
    UI8_T           *rec_p)
{
    BOOL_T return_value;

    return_value = L_HISAM_GetNextRecord(desc_p, kidx, key_p, rec_p);

    return return_value;
}/*end of MLDSNP_OM_HisamGetNextRecord*/

/* ------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_HisamDeleteRecord
 * ------------------------------------------------------------------------
 * PURPOSE  : the function delet the record from the hisam table
 * INPUT    : *desc_p  -  the hisam desc_priptor
 *            *rec_p   - the record will be put into hisame table
 * OUTPUT   : None
 * RETUEN   : TRUE   - sucess
 *            FALSE  - fail
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_OM_HisamDeleteRecord(
    L_HISAM_Desc_T *desc_p,
    UI8_T          *key_p)
{
    BOOL_T return_value;

    return_value = L_HISAM_DeleteRecord(desc_p, key_p);

    return return_value;
}/*end of MLDSNP_OM_HisamDeleteRecord*/

/*------------------------------------------------------------------------------
 * Function : MLDSNP_OM_SetVidGroupSrcKey
 *------------------------------------------------------------------------------
 * Purpose: Generate the key to access HISAM entries
 * INPUT  : vid       - the vlan id
 *          gip       - the group ip
 *          *sip_ap   - the source ip
 * OUTPUT : *key_p    - required key
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
static void MLDSNP_OM_SetVidGroupSrcKey(
    UI8_T  *key_p,
    UI32_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap)
{

    memcpy(key_p, &vid, VID_LEN);

    if (NULL != gip_ap)
        memcpy(key_p + VID_LEN, gip_ap, GROUP_LEN);
    else
        memcpy(key_p + VID_LEN, mldsnp_om_null_group_ip_a, GROUP_LEN);


    if (NULL != sip_ap)
        memcpy(key_p + VID_LEN + GROUP_LEN, sip_ap, SRC_IP_LEN);
    else
        memcpy(key_p + VID_LEN + GROUP_LEN, mldsnp_om_null_src_ip_a, SRC_IP_LEN);

    return;
}/*end of MLDSNP_OM_SetVidGroupSrcKey*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_AddPortIntoHisamPortList
*------------------------------------------------------------------------------
* Purpose:  This function add the port into group entry's port list
* INPUT  : *hisam_entry_p  - the group entry
*          port_info_p     -  the port info
*
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_OM_AddPortIntoHisamPortList(
    MLDSNP_OM_HisamEntry_T *hisam_entry_p,
    MLDSNP_OM_PortInfo_T   *port_info_p)
{
    if (NULL == hisam_entry_p)
    {
        return FALSE;
    }

    if (0 == hisam_entry_p->register_port_list.nbr_of_element)
    {
        L_SORT_LST_Create(&hisam_entry_p->register_port_list,
                          MLDSNP_TYPE_MAX_NUM_OF_LPORT,
                          sizeof(MLDSNP_OM_PortInfo_T),
                          MLDSNP_OM_SortCompPortInfoPortNo);
    }


    if (FALSE == L_SORT_LST_Set(&hisam_entry_p->register_port_list, port_info_p))
    {
        return FALSE;
    }

    if (FALSE == MLDSNP_OM_HisamSetRecord(&group_entry_hisam_desc_g, (UI8_T *)hisam_entry_p))
    {
        return FALSE;
    }

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    mld_throttle_db[port_info_p->port_no - 1].current_group_count++;
#endif

    MLDSNP_OM_LocalPortIncStat(port_info_p->port_no, 0, MLDSNP_TYPE_STAT_GRROUPS, TRUE);

    return TRUE;
}/*End of MLDSNP_OM_AddPortIntoHisamPortList*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_PutRegisteredPortsToPortbimap
*------------------------------------------------------------------------------
* Purpose:  This function search the register port list and add the registered port into port bitmap
* INPUT  : *hisam_entry_p  - the group entry
*          port_no     - the port info
*
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
static void MLDSNP_OM_PutRegisteredPortsToPortbimap(
    L_SORT_LST_List_T *report_port_list_p,
    MLDSNP_OM_GroupInfo_T *group_info_p,
    UI8_T *flag_p)
{
    MLDSNP_OM_PortInfo_T port_info;

    port_info.port_no = 0;

    if (NULL == report_port_list_p)
        return;

    while (TRUE == L_SORT_LST_Get_Next(report_port_list_p, &port_info))
    {
        if (port_info.join_type == MLDSNP_TYPE_JOIN_DYNAMIC)
        {
            *flag_p |= HAVE_DYNAMIC_PORT;
            MLDSNP_TYPE_AddPortIntoPortBitMap(port_info.port_no, group_info_p->dynamic_port_bitmap);
        }
        else if (port_info.join_type == MLDSNP_TYPE_JOIN_STATIC)
        {
            *flag_p |= HAVE_STATIC_PORT;
            MLDSNP_TYPE_AddPortIntoPortBitMap(port_info.port_no, group_info_p->static_port_bitmap);
        }
        else if (port_info.join_type == MLDSNP_TYPE_JOIN_UNKNOWN)
        {
            *flag_p |= HAVE_UNKNOWN_PORT;
            MLDSNP_TYPE_AddPortIntoPortBitMap(port_info.port_no, group_info_p->unknown_port_bitmap);
        }
        else
        {
            printf("join_type is wrong\r\n");
        }
    }

}/*End of MLDSNP_OM_PutRegisteredPortsToPortbimap*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetVlanRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the vlan's router list
* INPUT  : vid               - the vlan id
* OUTPUT : router_port_list  - the router port info
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is only provide to UI to access
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_OM_GetVlanRouterPortlist(
    UI16_T                     vid,
    MLDSNP_OM_RouterPortList_T *router_port_list)
{
    MLDSNP_OM_RouterPortInfo_T router_port_info;
    MLDSNP_OM_VlanInfo_T vlan_info;
    BOOL_T ret = FALSE;

    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        return FALSE;
    }

    memset(router_port_list, 0, sizeof(MLDSNP_OM_RouterPortList_T));
    router_port_list->vid    = vid;
    router_port_info.port_no = 0;

    while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &router_port_info))
    {
        if (MLDSNP_TYPE_JOIN_DYNAMIC == router_port_info.attribute)
        {
            MLDSNP_TYPE_AddPortIntoPortBitMap(router_port_info.port_no, router_port_list->router_port_bitmap);
        }
        else if (MLDSNP_TYPE_JOIN_STATIC == router_port_info.attribute)
        {
            MLDSNP_TYPE_AddPortIntoPortBitMap(router_port_info.port_no, router_port_list->static_router_port_bitmap);
            MLDSNP_TYPE_AddPortIntoPortBitMap(router_port_info.port_no, router_port_list->router_port_bitmap);
        }
        ret = TRUE;
    }
    return ret;
}/*End of MLDSNP_OM_GetVlanRouterPortlist*/

static I32_T
MLDSNP_OM_Ipv6AddrCmp(void *ip1, void *ip2)
{
    return IPV6_ADDR_CMP(ip1, ip2);
}

#if 0
#endif

/*---------------------------------------------------------------------------------
 * FUNCTION : MLDSNP_OM_LinkListNodeFree
 *---------------------------------------------------------------------------------
 * PURPOSE  : Free the memalloc when insert a node to linklist
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void MLDSNP_OM_LinkListNodeFree(void *p)
{
    L_MM_Free(p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION : CFM_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for CFM OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void MLDSNP_OM_AttachSystemResources(void)
{
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_MLDSNP, &mldsnp_om_sem_id);
    return;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SortCompPortInfoPortNo
*------------------------------------------------------------------------------
* Purpose:
 * PURPOSE  : a function used in the remote management address sort list
 * INPUT    : *list_node_p        - the node in the sort list
 *            *comp_node_p        - the node to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
*------------------------------------------------------------------------------*/
int MLDSNP_OM_SortCompPortInfoPortNo(
    void* list_node_p,
    void* comp_node_p)
{
    MLDSNP_OM_PortInfo_T *node_list_p = NULL, *node_comp_p = NULL;

    node_list_p = (MLDSNP_OM_PortInfo_T *) list_node_p;
    node_comp_p = (MLDSNP_OM_PortInfo_T *) comp_node_p;

    return node_list_p->port_no - node_comp_p->port_no;
}/*End of MLDSNP_OM_SortCompPortInfoPortNo*/

/*------------------------------------------------------------------------------
 * Function : MLDSNP_OM_SortCompRouterPortInfoPortNo
 *------------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote management address sort list
 * INPUT    : *list_node_p        - the node in the sort list
 *            *comp_node_p        - the node to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
*------------------------------------------------------------------------------*/
int MLDSNP_OM_SortCompRouterPortInfoPortNo(
    void* list_node_p,
    void* comp_node_p)
{
    MLDSNP_OM_RouterPortInfo_T *node_list_p = NULL, *node_comp_p = NULL;

    node_list_p = (MLDSNP_OM_RouterPortInfo_T *) list_node_p;
    node_comp_p = (MLDSNP_OM_RouterPortInfo_T *) comp_node_p;

    return node_list_p->port_no - node_comp_p->port_no;
}/*End of MLDSNP_OM_SortCompRouterPortInfoPortNo*/

/*------------------------------------------------------------------------------
 * Function : MLDSNP_OM_SortCompVlanId
 *------------------------------------------------------------------------------
 * PURPOSE  : a function used in the remote management address sort list
 * INPUT    : *list_node_p        - the node in the sort list
 *            *comp_node_p        - the node to compare
 * OUTPUT   : None
 * RETUEN   : >0, =0, <0
 * NOTES    : None
*------------------------------------------------------------------------------*/
int MLDSNP_OM_SortCompVlanId(
    void* list_node_p,
    void* comp_node_p)
{
    MLDSNP_OM_VlanInfo_T*node_list_p = NULL, *node_comp_p = NULL;

    node_list_p = (MLDSNP_OM_VlanInfo_T *) list_node_p;
    node_comp_p = (MLDSNP_OM_VlanInfo_T *) comp_node_p;

    return node_list_p->vid - node_comp_p->vid;
}/*End of MLDSNP_OM_SortCompVlanId*/


/* EXPORTED SUBPROGRAM BODIES
*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteAllPortFromPortList
*------------------------------------------------------------------------------
* Purpose:This function add the port into group entry's port list
* INPUT  : *hisam_entry_p  - the group entry
* OUTPUT : None
* RETURN : TRUE - success
*       FALSE- fail
* NOTES  : this function won't take care link list.
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteAllPortFromPortList(
    MLDSNP_OM_HisamEntry_T *hisam_entry_p)
{
    MLDSNP_OM_PortInfo_T return_port_info;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};

    MLDSNP_OM_LOCK();
    /*delete all port link in hisam entry*/
    if (FALSE == L_SORT_LST_Delete_All(&hisam_entry_p->register_port_list))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    return_port_info.port_no = 0;
    while (L_SORT_LST_Get_Next(&hisam_entry_p->register_port_list, &return_port_info))
    {
        MLDSNP_OM_LocalPortIncStat(return_port_info.port_no, 0, MLDSNP_TYPE_STAT_GRROUPS, FALSE);
    }

    /*delete the hisam entry because of no port*/
    MLDSNP_OM_SetVidGroupSrcKey(key_a, hisam_entry_p->vid, hisam_entry_p->gip_a, hisam_entry_p->sip_a);

    if (FALSE == MLDSNP_OM_HisamDeleteRecord(&group_entry_hisam_desc_g, key_a))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }
    MLDSNP_OM_LocalVlanIncStat(hisam_entry_p->vid, MLDSNP_TYPE_STAT_GRROUPS, FALSE);
    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_DeleteAllPortFromPortList*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_Init
*------------------------------------------------------------------------------
* Purpose: This function initial the OM
* INPUT  : None
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : clear om can't use this function
*------------------------------------------------------------------------------*/
void MLDSNP_OM_Init()
{
    /*initial vlan*/
    L_SORT_LST_Create(&mldsnp_om_vlan_list_g,
                      SYS_ADPT_MAX_VLAN_ID,
                      sizeof(MLDSNP_OM_VlanInfo_T),
                      MLDSNP_OM_SortCompVlanId);

    MLDSNP_OM_CreateHisameTaable();
    MLDSNP_OM_AttachSystemResources();

    return;
}
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_EnterTransitionMode
*------------------------------------------------------------------------------
* Purpose: This function clear all the om record
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  : clear om record
*------------------------------------------------------------------------------*/
void MLDSNP_OM_EnterTransitionMode()
{
    /*clear hisam*/
    MLDSNP_OM_HisamEntry_T hisam_entry;
    MLDSNP_OM_PortInfo_T return_port_info;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    UI8_T gip_a[GROUP_LEN] = {0};
    UI8_T sip_a[SRC_IP_LEN] = {0};

    MLDSNP_OM_SetVidGroupSrcKey(key_a, 0, gip_a, sip_a);

    while (TRUE == MLDSNP_OM_HisamGetNextRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        MLDSNP_OM_SetVidGroupSrcKey(key_a, hisam_entry.vid, hisam_entry.gip_a, hisam_entry.sip_a);

        return_port_info.port_no = 0;
        while (L_SORT_LST_Get_Next(&hisam_entry.register_port_list, &return_port_info))
        {
            L_list_delete_all_node(&return_port_info.host_sip_lst);
        }
    }

    MLDSNP_OM_RemoveAllHisamEntry();

    /*clear vlan*/
    {
        MLDSNP_OM_VlanInfo_T vlan_info;

        vlan_info.vid = 0;

        while (TRUE == L_SORT_LST_Get_Next(&mldsnp_om_vlan_list_g, &vlan_info))
        {
            L_SORT_LST_Delete_All(&vlan_info.router_port_list);
        }

        L_SORT_LST_Delete_All(&mldsnp_om_vlan_list_g);
    }

    mldsnp_om_querier_timer_p          = NULL;
    mldsnp_om_mrd_solicitation_timer_p = NULL;
    mldsnp_om_unsolicite_timer_p       = NULL;

    memset(port_statis, 0, sizeof(port_statis));
    memset(vlan_statis, 0, sizeof(vlan_statis));

    return;
}/*End of MLDSNP_OM_EnterTransitionMode*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_EnterMasterMode
*------------------------------------------------------------------------------
* Purpose: This function set the default value
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  : None
*------------------------------------------------------------------------------*/
void MLDSNP_OM_EnterMasterMode()
{
    UI32_T i;

    mldsnp_om_cfg_db_g.mldsnp_status  = MLDSNP_TYPE_DFLT_MLDSNP_STATUS;
    mldsnp_om_cfg_db_g.version        = MLDSNP_TYPE_DFLT_VER;

    mldsnp_om_cfg_db_g.querier_status = MLDSNP_TYPE_DFLT_QUERIER_STATUS;
    mldsnp_om_cfg_db_g.query_interval = MLDSNP_TYPE_DFLT_QUERY_INTERVAL;

    MLDSNP_OM_GetListenerInterval(&mldsnp_om_cfg_db_g.listener_interval);

    mldsnp_om_cfg_db_g.robust_value                = MLDSNP_TYEP_DFLT_ROBUST_VALUE;
    mldsnp_om_cfg_db_g.query_response_interval     = MLDSNP_TYPE_DFLT_MAX_RESP_INTERVAL;
    mldsnp_om_cfg_db_g.last_listner_query_interval = MLDSNP_TYPE_DFLT_LAST_LISTENER_QUERY_INTERVAL;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    mldsnp_om_cfg_db_g.proxy_reporting             = SYS_DFLT_MLDSNP_PROXY_REPORTING;
    mldsnp_om_cfg_db_g.unsolicited_report_interval = SYS_DFLT_MLDSNP_UNSOLICIT_REPORT_INTERVAL;
#endif
    mldsnp_om_cfg_db_g.router_exp_time             = MLDSNP_TYPE_DFLT_ROUTER_EXP_TIME;
#if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    mldsnp_om_cfg_db_g.unknown_flood_behavior      = MLDSNP_TYPE_DFLT_FLOOD_BEHAVIOR;
#else
    {
      for(i=0; i< SYS_ADPT_MAX_VLAN_ID; i++)
       mldsnp_om_cfg_db_g.unknown_flood_behavior[i]= MLDSNP_TYPE_DFLT_FLOOD_BEHAVIOR;
    }
#endif
    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)
#if(SYS_DFLT_MLD_SNOOP_QUERY_DROP== VAL_mldSnoopQueryDrop_enable)
        SET_FLAG(mldsnp_om_cfg_db_g.conf[i], MLDSNP_OM_QUERY_DROP_ENABLED);
#endif
#endif
#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP == TRUE)
#if(SYS_DFLT_MLD_SNOOP_MULTICAST_DATA_DROP == VAL_mldSnoopMulticastDataDrop_enable)
        SET_FLAG(mldsnp_om_cfg_db_g.conf[i], MLDSNP_OM_MULTICAST_DATA_DROP_ENABLED);
#endif
#endif
    }
#if ( SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    /* Set the database to factoy default,
     * and then File system to load the command line to here.
     */
    memset(&mld_filter_db, 0, SIZE_OF_MLD_FILTER_POOL);
    memset(mld_profile_pool, 0, SIZE_OF_MLD_PROFILE_POOL);
    memset(mld_profile_group_pool, 0, SIZE_OF_MLD_PROFILE_GROUP_POOL);
    memset(mld_port_profile_id, 0, SIZE_OF_MLD_PORT_PROFILE_POOL);

    free_profile_ptr = NULL;
    free_profile_group_ptr = NULL;

    mld_filter_db.filter_status = SYS_DFLT_MLD_FILTER_STATUS;
    mld_filter_db.profile_entry_p = NULL;

    /* Initialize profile pool. Put all entries in free pool
     */
    free_profile_ptr = &mld_profile_pool[0];
    for (i = 1; i < SYS_ADPT_MLD_PROFILE_TOTAL_NBR; i++)
    {
        mld_profile_pool[i - 1].nextPtr = &mld_profile_pool[i];
    }

    mld_profile_pool[SYS_ADPT_MLD_PROFILE_TOTAL_NBR - 1].nextPtr = NULL;

    /* Initialize profile group pool. Put all entries in free pool
     */
    free_profile_group_ptr = &mld_profile_group_pool[0];
    for (i = 1; i < SYS_ADPT_MLD_TOTAL_PROFILE_GROUPS; i++)
    {
        mld_profile_group_pool[i - 1].next = &mld_profile_group_pool[i];
    }

    mld_profile_group_pool[SYS_ADPT_MLD_TOTAL_PROFILE_GROUPS - 1].next = NULL;

    memset(mld_throttle_db, 0, SIZE_OF_MLD_THROTTLE_POOL);

    for (i = 0; i < SYS_ADPT_MLD_THROTTLE_TOTAL_NBR_OF_PORT; i++)
    {
        mld_throttle_db[i].throttle_status     = VAL_mldSnoopThrottlePortRunningStatus_false;
        mld_throttle_db[i].action              = SYS_DFLT_MLD_PROFILE_ACCESS_MODE;
        mld_throttle_db[i].max_group_number    = SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY;
    }
#endif
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
    {
        UI16_T i;
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE )
        for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
        {
            mldsnp_om_cfg_db_g.port_mld_pkt_lmt_conf[i] = SYS_DFLT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT;
        }
#endif
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
        for (i = 0; i < SYS_ADPT_MAX_VLAN_ID; i++)
        {
            mldsnp_om_cfg_db_g.vlan_mld_pkt_lmt_conf[i] = SYS_DFLT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN;
        }
#endif
    }
#endif

    memset(v1_host_preset_portbitmap, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    {
        /*because default vlan 1 won't notify mldsnp, so create the default vlan info here*/
        MLDSNP_OM_VlanInfo_T vlan_info;
        MLDSNP_OM_InitVlanInfo(SYS_DFLT_1Q_PORT_VLAN_PVID, &vlan_info);
        L_SORT_LST_Set(&mldsnp_om_vlan_list_g, &vlan_info);
    }
}/*End of MLDSNP_OM_EnterMasterMode*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_InitVlanInfo
*------------------------------------------------------------------------------
* Purpose: This function initial the vlan info
* INPUT  : vid          - the vlan id
*          *vid_info_p  - the vlan info pointer
* OUTPUT : vid_info_p   - the initialize hisam entry
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_InitVlanInfo(UI16_T               vid,
                            MLDSNP_OM_VlanInfo_T *vlan_info_p)
{
    memset(vlan_info_p, 0, sizeof(MLDSNP_OM_VlanInfo_T));

    vlan_info_p->vid                           = vid;
    vlan_info_p->immediate_leave_status        = MLDSNP_TYPE_DFLT_IMMEDIATE_STATUS;
    vlan_info_p->immediate_leave_byhost_status = MLDSNP_TYPE_DFLT_IMMEDIATE_BYHOST_STATUS;
    vlan_info_p->other_querier_present_timer_p = NULL;
    vlan_info_p->query_oper_interval           = mldsnp_om_cfg_db_g.query_interval;
    vlan_info_p->robust_oper_value             = mldsnp_om_cfg_db_g.robust_value;

    vlan_info_p->querier_runing_status = mldsnp_om_cfg_db_g.querier_status;

    L_SORT_LST_Create(&vlan_info_p->router_port_list, SYS_ADPT_MLDSNP_MAX_NBR_OF_ROUTER_PORT,
                      sizeof(MLDSNP_OM_RouterPortInfo_T), MLDSNP_OM_SortCompRouterPortInfoPortNo);

    return;
}/*End of MLDSNP_OM_InitVlanInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_InitialportInfo
*------------------------------------------------------------------------------
* Purpose: This function initial the port information
* INPUT  : lport        - the logical port
*          port_info_p  - the port info pointer
*          jon_type     - how the port join the group
*          register_tiem- the time initial this port
*          list_type    - put this port into which list type
* OUTPUT : *port_info_p - the initialize port info
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_InitialportInfo(UI16_T                 lport,
                               MLDSNP_TYPE_JoinType_T join_type,
                               MLDSNP_TYPE_ListType_T list_type,
                               UI32_T register_time,
                               MLDSNP_OM_PortInfo_T   *port_info_p)
{
    memset(port_info_p, 0, sizeof(MLDSNP_OM_PortInfo_T));

    port_info_p->port_no              = lport;
    port_info_p->join_type            = join_type;
    port_info_p->last_report_time     = register_time;
    port_info_p->register_time        = register_time;
    port_info_p->specific_query_count = 0;
    port_info_p->list_type            = list_type;
    port_info_p->filter_timer_p       = NULL;
    port_info_p->ver1_host_present_timer_p = NULL;

    port_info_p->host_sip_lst.cmp = MLDSNP_OM_Ipv6AddrCmp;
    port_info_p->host_sip_lst.del = MLDSNP_OM_LinkListNodeFree;


    return;
}/*End of MLDSNP_OM_InitialportInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_InitialHisamEntry
*------------------------------------------------------------------------------
* Purpose: This function initial the hiam entry
* INPUT  : vid             - the vlan id
*          *gip_ap         - the group ip
*          *sip_ap         - the source ip
* OUTPUT : *hisam_entry_p  - the initialize hisam entry
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_InitialHisamEntry(UI16_T                 vid,
                                 UI8_T                  *gip_ap,
                                 UI8_T                  *sip_ap,
                                 MLDSNP_OM_HisamEntry_T *hisam_entry_p)
{
    memset(hisam_entry_p, 0, sizeof(MLDSNP_OM_HisamEntry_T));
    hisam_entry_p->vid                     = vid;
    hisam_entry_p->last_fwd_to_router_time = 0;

    if (NULL != gip_ap)
        memcpy(hisam_entry_p->gip_a, gip_ap, GROUP_LEN);
    else
        memcpy(hisam_entry_p->gip_a, mldsnp_om_null_group_ip_a, GROUP_LEN);

    if (NULL != sip_ap)
        memcpy(hisam_entry_p->sip_a, sip_ap, SRC_IP_LEN);
    else
        memcpy(hisam_entry_p->sip_a, mldsnp_om_null_src_ip_a, SRC_IP_LEN);

    return;
}/*End of MLDSNP_OM_InitialHisamEntry*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetHisamEntryInfo
*------------------------------------------------------------------------------
* Purpose: This function set the info in hisam entry
* INPUT  : *entry_info   - the infomation store in hisam
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetHisamEntryInfo(
    MLDSNP_OM_HisamEntry_T *entry_info)
{
    if (NULL == entry_info)
    {
        return FALSE;
    }

    MLDSNP_OM_LOCK();

    if (FALSE == MLDSNP_OM_HisamSetRecord(&group_entry_hisam_desc_g, (UI8_T *)entry_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_SetHisamEntryInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteHisamEntryInfo
*------------------------------------------------------------------------------
* Purpose: This function set the info in hisam entry
* INPUT  :  *entry_info   - the infomation store in hisam
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : please make sure all register port's timer has been clear/freed
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteHisamEntryInfo(
    MLDSNP_OM_HisamEntry_T *entry_info)
{
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};

    if (NULL == entry_info)
    {
        return FALSE;
    }

    MLDSNP_OM_LOCK();
    /*delete all register port*/
    L_SORT_LST_Delete_All(&entry_info->register_port_list);

    MLDSNP_OM_SetVidGroupSrcKey(key_a, entry_info->vid, entry_info->gip_a, entry_info->sip_a);

    if (FALSE == MLDSNP_OM_HisamDeleteRecord(&group_entry_hisam_desc_g, key_a))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_SetHisamEntryInfo*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_AddPortInfo
*------------------------------------------------------------------------------
* Purpose: this function add the port info
*          if the group entry doen't exist, this entry will be created first.
* INPUT  : vid           - the vlan id
*          *gip_ap       - the group ip address
*          *sip_ap       - the source ip address
*          port_info_p   - the port info
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : if the hisam entry is not existed, the function will create the hisam entry
*
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_AddPortInfo(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    MLDSNP_OM_PortInfo_T *port_info_p)
{
    MLDSNP_OM_HisamEntry_T   hisam_entry;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, sip_ap);

    if (0 == port_info_p->port_no)
    {
        return TRUE;
    }

    if (vid == 0)
    {
        return FALSE;
    }

    MLDSNP_OM_LOCK();

    if (FALSE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        MLDSNP_OM_InitialHisamEntry(vid, gip_ap, sip_ap, &hisam_entry);

        if (FALSE == MLDSNP_OM_HisamSetRecord(&group_entry_hisam_desc_g, (UI8_T *)&hisam_entry))
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }

        if (FALSE == MLDSNP_OM_AddPortIntoHisamPortList(&hisam_entry, port_info_p))
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }
        MLDSNP_OM_LocalVlanIncStat(vid, MLDSNP_TYPE_STAT_GRROUPS, TRUE);

        MLDSNP_OM_UNLOCK();
        return TRUE;
    }

    if (FALSE == MLDSNP_OM_AddPortIntoHisamPortList(&hisam_entry, port_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End if MLDSNP_OM_AddPortInfo*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeletePortInfo
*------------------------------------------------------------------------------
* Purpose: This function delete the port from hisam
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*          lport    - the logical port
* OUTPUT : None
* RETURN : TRUE    - success
*          FALSE   - fail
* NOTES  : if there is no port in the hisam entry, this hisam entry will be deleted
*         this function won't take care the pointer in portinfo, use this function shall make sure
*         all timer in port info already been stopped.
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_DeletePortInfo(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI16_T lport)
{
    MLDSNP_OM_HisamEntry_T   hisam_entry;
    MLDSNP_OM_PortInfo_T     port_info;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, sip_ap);
    MLDSNP_OM_LOCK();

    if (FALSE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *) &hisam_entry))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    port_info.port_no = lport;
    if (FALSE == L_SORT_LST_Get(&hisam_entry.register_port_list, &port_info))
    {
        MLDSNP_OM_UNLOCK();
        return TRUE;
    }

    if (FALSE == L_SORT_LST_Delete(&hisam_entry.register_port_list, &port_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    L_list_delete_all_node(&port_info.host_sip_lst);

    /* if there is no node in the sort list, then delete the sort list*/
    if (0 == hisam_entry.register_port_list.nbr_of_element)
    {
        if (FALSE == MLDSNP_OM_HisamDeleteRecord(&group_entry_hisam_desc_g, key_a))
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
        mld_throttle_db[lport - 1].current_group_count--;
#endif

        MLDSNP_OM_LocalPortIncStat(lport, vid, MLDSNP_TYPE_STAT_GRROUPS, FALSE);
    }
    else
    {
        if (FALSE == MLDSNP_OM_HisamSetRecord(&group_entry_hisam_desc_g, (UI8_T *)&hisam_entry))
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }
        MLDSNP_OM_LocalPortIncStat(lport, 0, MLDSNP_TYPE_STAT_GRROUPS, FALSE);
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End if MLDSNP_OM_DeletePortInfo*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the group's port list
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT : *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : 1. This function is only rovide to UI to access
*          2. It will only collect (vid, group, *), so it port join (vid, group, s1) and (vid, group, s2)
*             it will be put into (vid, group, 0)
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetGroupPortlist(
    UI16_T                vid,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    MLDSNP_OM_GroupInfo_T *group_info_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;

    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    UI8_T flag = 0;

    memset(group_info_p, 0, sizeof(MLDSNP_OM_GroupInfo_T));

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, NULL);
    MLDSNP_OM_LOCK();
    /*(vid, gip, 0)*/
    if (FALSE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    group_info_p->vid = hisam_entry.vid;
    memcpy(group_info_p->gip_a, hisam_entry.gip_a, GROUP_LEN);
    memset(group_info_p->sip_a, 0, SRC_IP_LEN);

    MLDSNP_OM_PutRegisteredPortsToPortbimap(&hisam_entry.register_port_list, group_info_p, &flag);

    /*all (vid, gip, srcs)*/
    while (TRUE == MLDSNP_OM_HisamGetNextRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        if (hisam_entry.vid != vid
                || memcmp(gip_ap, hisam_entry.gip_a, GROUP_LEN))
            break;

        MLDSNP_OM_PutRegisteredPortsToPortbimap(&hisam_entry.register_port_list, group_info_p, &flag);

        MLDSNP_OM_SetVidGroupSrcKey(key_a, hisam_entry.vid, hisam_entry.gip_a, hisam_entry.sip_a);
    }
    MLDSNP_OM_UNLOCK();
#if 1 /*shall we show the router port? although we will forward data to router port automatically*/
  #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == mldsnp_om_cfg_db_g.unknown_flood_behavior)
  #else
    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == mldsnp_om_cfg_db_g.unknown_flood_behavior[vid-1])
  #endif
    {
        MLDSNP_OM_RouterPortInfo_T nxt_r_port_info;
        MLDSNP_OM_VlanInfo_T vlan_info;

        if (TRUE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
        {
            nxt_r_port_info.port_no = 0;
            while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &nxt_r_port_info))
            {
                if (flag&HAVE_DYNAMIC_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->dynamic_port_bitmap);
                if (flag&HAVE_STATIC_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->static_port_bitmap);
                if (flag&HAVE_UNKNOWN_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->unknown_port_bitmap);
            }
        }
    }
#endif

    return TRUE;
}/*End if MLDSNP_OM_GetGroupPortlist*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the next group's port list
* INPUT  : vid_p    - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT :  vid_p   - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*          *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*          2. It will only collect (vid, group, *), so it port join (vid, group, s1) and (vid, group, s2)
*             it will be put into (vid, group, 0)
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetNextGroupPortlist(
    UI16_T                *vid_p,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    MLDSNP_OM_GroupInfo_T *group_info_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    UI16_T lock_vid                  = *vid_p;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    UI8_T *lock_gip_ap               = gip_ap; /*if find first then lock this gip, because only search this gip*/
    UI8_T ff_sip_a[SRC_IP_LEN]       = {0xff, 0xff, 0xff, 0xff,
                                        0xff, 0xff, 0xff, 0xff,
                                        0xff, 0xff, 0xff, 0xff,
                                        0xff, 0xff, 0xff, 0xff
                                       };
    UI8_T flag                       = 0;
    BOOL_T find_first                = FALSE;

    memset(group_info_p, 0, sizeof(MLDSNP_OM_GroupInfo_T));
    memset(group_info_p->dynamic_port_bitmap, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(group_info_p->static_port_bitmap,  0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(group_info_p->unknown_port_bitmap, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    MLDSNP_OM_SetVidGroupSrcKey(key_a, *vid_p, gip_ap, ff_sip_a);

    MLDSNP_OM_LOCK();
    while (TRUE == MLDSNP_OM_HisamGetNextRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        if (TRUE == find_first
                && (memcmp(lock_gip_ap, hisam_entry.gip_a, GROUP_LEN)
                    || lock_vid != hisam_entry.vid))
            break;

        if (FALSE == find_first)
        {
            find_first = TRUE;
            /*lock_vid = hisam_entry.vid;*/
            memcpy(lock_gip_ap, hisam_entry.gip_a, GROUP_LEN);

            group_info_p->vid = hisam_entry.vid;
            memcpy(group_info_p->gip_a, hisam_entry.gip_a, GROUP_LEN);
            memcpy(gip_ap, hisam_entry.gip_a, GROUP_LEN);
            memcpy(sip_ap, ff_sip_a, SRC_IP_LEN);
            *vid_p = hisam_entry.vid;
        }

        MLDSNP_OM_PutRegisteredPortsToPortbimap(&hisam_entry.register_port_list, group_info_p, &flag);

        MLDSNP_OM_SetVidGroupSrcKey(key_a, hisam_entry.vid, hisam_entry.gip_a, hisam_entry.sip_a);
    }
    MLDSNP_OM_UNLOCK();

#if 1 /*shall we shall the router port? although we will forward data to router port automatically*/
    if (TRUE == find_first &&
  #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
      (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == mldsnp_om_cfg_db_g.unknown_flood_behavior)
  #else
      (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == mldsnp_om_cfg_db_g.unknown_flood_behavior[group_info_p->vid-1])
  #endif
       )
    {
        MLDSNP_OM_RouterPortInfo_T nxt_r_port_info;
        MLDSNP_OM_VlanInfo_T       vlan_info;

        if (TRUE == MLDSNP_OM_GetVlanInfo(group_info_p->vid, &vlan_info))
        {
            nxt_r_port_info.port_no = 0;
            while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &nxt_r_port_info))
            {
                if (flag&HAVE_DYNAMIC_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->dynamic_port_bitmap);
                if (flag&HAVE_STATIC_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->static_port_bitmap);
                if (flag&HAVE_UNKNOWN_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->unknown_port_bitmap);
            }
        }
    }
#endif

    return find_first;
}/*End if MLDSNP_OM_GetNextGroupPortlist*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetGroupSourcePortlist
*------------------------------------------------------------------------------
* Purpose: This function get the group and source  port list
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT : *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : 1. This function is only rovide to UI to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetGroupSourcePortlist(
    UI16_T                vid,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    MLDSNP_OM_GroupInfo_T *group_info_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    UI8_T flag = 0;

    memset(group_info_p, 0, sizeof(MLDSNP_OM_GroupInfo_T));
    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, sip_ap);

    MLDSNP_OM_LOCK();

    if (FALSE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    group_info_p->vid = hisam_entry.vid;
    memcpy(group_info_p->gip_a, hisam_entry.gip_a, GROUP_LEN);
    memcpy(group_info_p->sip_a, hisam_entry.sip_a, SRC_IP_LEN);

    MLDSNP_OM_PutRegisteredPortsToPortbimap(&hisam_entry.register_port_list, group_info_p, &flag);
    MLDSNP_OM_UNLOCK();
#if 1 /*shall we shall the router port? although we will forward data to router port automatically*/
  #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == mldsnp_om_cfg_db_g.unknown_flood_behavior)
  #else
    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == mldsnp_om_cfg_db_g.unknown_flood_behavior[vid-1])
  #endif
    {
        MLDSNP_OM_RouterPortInfo_T nxt_r_port_info;
        MLDSNP_OM_VlanInfo_T       vlan_info;

        if (TRUE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
        {
            nxt_r_port_info.port_no = 0;
            while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &nxt_r_port_info))
            {
                if (flag&HAVE_DYNAMIC_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->dynamic_port_bitmap);
                if (flag&HAVE_STATIC_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->static_port_bitmap);
                if (flag&HAVE_UNKNOWN_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->unknown_port_bitmap);
            }
        }
    }
#endif
    return TRUE;
}/*End if MLDSNP_OM_GetGroupSourcePortlist*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextGroupSourcePortlist
*------------------------------------------------------------------------------
* Purpose: This function get the next group and source  port list
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT : *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : 1. This function is only rovide to UI to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetNextGroupSourcePortlist(
    UI16_T                vid,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    MLDSNP_OM_GroupInfo_T *group_info_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    UI8_T flag = 0;

    memset(group_info_p, 0, sizeof(MLDSNP_OM_GroupInfo_T));

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, sip_ap);

    MLDSNP_OM_LOCK();
    if (FALSE == MLDSNP_OM_HisamGetNextRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    group_info_p->vid = hisam_entry.vid;
    memcpy(group_info_p->gip_a, hisam_entry.gip_a, GROUP_LEN);
    memcpy(group_info_p->sip_a, hisam_entry.sip_a, SRC_IP_LEN);

    MLDSNP_OM_PutRegisteredPortsToPortbimap(&hisam_entry.register_port_list, group_info_p, &flag);
    MLDSNP_OM_UNLOCK();
#if 1 /*shall we shall the router port? although we will forward data to router port automatically*/
  #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == mldsnp_om_cfg_db_g.unknown_flood_behavior)
  #else
    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT == mldsnp_om_cfg_db_g.unknown_flood_behavior[vid-1])
  #endif
    {
        MLDSNP_OM_RouterPortInfo_T nxt_r_port_info;
        MLDSNP_OM_VlanInfo_T       vlan_info;

        if (TRUE == MLDSNP_OM_GetVlanInfo(group_info_p->vid, &vlan_info))
        {
            nxt_r_port_info.port_no = 0;
            while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &nxt_r_port_info))
            {
                if (flag&HAVE_DYNAMIC_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->dynamic_port_bitmap);
                if (flag&HAVE_STATIC_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->static_port_bitmap);
                if (flag&HAVE_UNKNOWN_PORT)
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, group_info_p->unknown_port_bitmap);
            }
        }
    }
#endif
    return TRUE;
}/*End if MLDSNP_OM_GetNextGroupSourcePortlist*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextPortGroupSourceList
*------------------------------------------------------------------------------
* Purpose: This function get the port joined all group
* INPUT  : port_src_lst_p->vid    - the next vlan id
*          port_src_lst_p->port  - the port to get
* OUTPUT :  port_src_lst_p - the next vid, gip and source list
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*              get the port joined group and source list,
*              (port, vid, group) is the key
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetNextPortGroupSourceList(
    MLDSNP_OM_PortSourceListInfo_T *port_src_lst_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    MLDSNP_OM_PortInfo_T   port_info;
    UI16_T lock_vid                  = port_src_lst_p->vid;/*if find first then lock this vid, because only search this vid*/
    UI8_T *lock_gip_ap               = port_src_lst_p->gip_a; /*if find first then lock this gip, because only search this gip*/
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    UI8_T ff_sip_a[SRC_IP_LEN]       = {0xff, 0xff, 0xff, 0xff,
                                        0xff, 0xff, 0xff, 0xff,
                                        0xff, 0xff, 0xff, 0xff,
                                        0xff, 0xff, 0xff, 0xff
                                       };
    BOOL_T find_first = FALSE; /*find_first used to identify this is first find entry*/

    port_src_lst_p->num_of_ex = 0;
    memset(port_src_lst_p->exclude_list, 0, sizeof(port_src_lst_p->exclude_list));
    port_src_lst_p->num_of_req = 0;
    memset(port_src_lst_p->request_list, 0, sizeof(port_src_lst_p->request_list));

    MLDSNP_OM_SetVidGroupSrcKey(key_a, lock_vid, lock_gip_ap, ff_sip_a);

    MLDSNP_OM_LOCK();
    while (TRUE == MLDSNP_OM_HisamGetNextRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        if (TRUE == find_first
                && (memcmp(lock_gip_ap, hisam_entry.gip_a, GROUP_LEN)
                    || lock_vid != hisam_entry.vid))
            break;

        port_info.port_no = port_src_lst_p->port;
        if (TRUE == L_SORT_LST_Get(&hisam_entry.register_port_list, &port_info))
        {
            if (FALSE == find_first)
            {
                /*lock the vid and gip*/
                lock_vid = hisam_entry.vid;
                memcpy(lock_gip_ap, hisam_entry.gip_a, GROUP_LEN);
                /*assign the vlaue*/
                port_src_lst_p->vid      = hisam_entry.vid;
                memcpy(port_src_lst_p->gip_a, hisam_entry.gip_a, GROUP_LEN);
                port_src_lst_p->cur_mode = ((0 == memcmp(hisam_entry.sip_a, mldsnp_om_null_src_ip_a, SRC_IP_LEN)
                                             && NULL != port_info.filter_timer_p) ?
                                            MLDSNP_TYPE_IS_EXCLUDE_MODE : MLDSNP_TYPE_IS_INCLUDE_MODE);
                MLDSNP_TIMER_QueryTimer(port_info.filter_timer_p, &port_src_lst_p->filter_time_elapse);
                port_src_lst_p->num_of_req = 0;
                port_src_lst_p->num_of_ex  = 0;
                port_src_lst_p->join_type  = port_info.join_type;
                find_first                 = TRUE;
            }

            /*needn't to record the null sip_a*/
            if (memcmp(hisam_entry.sip_a, mldsnp_om_null_src_ip_a, SRC_IP_LEN))
            {
                if (MLDSNP_TYPE_LIST_INCLUDE == port_info.list_type
                        || MLDSNP_TYPE_LIST_REQUEST == port_info.list_type)
                {
                    memcpy(port_src_lst_p->request_list[port_src_lst_p->num_of_req], hisam_entry.sip_a, SRC_IP_LEN);
                    port_src_lst_p->num_of_req ++;
                }
                else
                {
                    memcpy(port_src_lst_p->exclude_list[port_src_lst_p->num_of_ex], hisam_entry.sip_a, SRC_IP_LEN);
                    port_src_lst_p->num_of_ex ++;
                }
            }
        }

        MLDSNP_OM_SetVidGroupSrcKey(key_a, hisam_entry.vid, hisam_entry.gip_a, hisam_entry.sip_a);
    }
    MLDSNP_OM_UNLOCK();
    return find_first; /*if ever find entry, return true*/
}/*End of MLDSNP_POM_GetNextPortGroupSourceList*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetPortGroupSourceList
*------------------------------------------------------------------------------
* Purpose: This function get the port joined all group
* INPUT  : port_src_lst_p->vid    - the next vlan id
*          port_src_lst_p->port  - the port to get
* OUTPUT :  port_src_lst_p - the next vid, gip and source list
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*              get the port joined group and source list,
*              (port, vid, group) is the key
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetPortGroupSourceList(
    MLDSNP_OM_PortSourceListInfo_T *port_src_lst_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    MLDSNP_OM_PortInfo_T   port_info;
    UI16_T lock_vid                  = port_src_lst_p->vid;/*if find first then lock this vid, because only search this vid*/
    UI8_T *lock_gip_ap               = port_src_lst_p->gip_a; /*if find first then lock this gip, because only search this gip*/
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    UI8_T zero_sip_a[SRC_IP_LEN]       = {0};
    BOOL_T find_first = FALSE; /*find_first used to identify this is first find entry*/

    port_src_lst_p->num_of_ex = 0;
    memset(port_src_lst_p->exclude_list, 0, sizeof(port_src_lst_p->exclude_list));
    port_src_lst_p->num_of_req = 0;
    memset(port_src_lst_p->request_list, 0, sizeof(port_src_lst_p->request_list));

    MLDSNP_OM_SetVidGroupSrcKey(key_a, lock_vid, lock_gip_ap, zero_sip_a);

    MLDSNP_OM_LOCK();
    while (TRUE == MLDSNP_OM_HisamGetNextRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        if (TRUE == find_first
                && (memcmp(lock_gip_ap, hisam_entry.gip_a, GROUP_LEN)
                    || lock_vid != hisam_entry.vid))
            break;

        port_info.port_no = port_src_lst_p->port;
        if (TRUE == L_SORT_LST_Get(&hisam_entry.register_port_list, &port_info))
        {
            if (FALSE == find_first)
            {
                /*lock the vid and gip*/
                lock_vid = hisam_entry.vid;
                memcpy(lock_gip_ap, hisam_entry.gip_a, GROUP_LEN);
                /*assign the vlaue*/
                port_src_lst_p->vid      = hisam_entry.vid;
                memcpy(port_src_lst_p->gip_a, hisam_entry.gip_a, GROUP_LEN);
                port_src_lst_p->cur_mode = ((0 == memcmp(hisam_entry.sip_a, mldsnp_om_null_src_ip_a, SRC_IP_LEN)
                                             && NULL != port_info.filter_timer_p) ?
                                            MLDSNP_TYPE_IS_EXCLUDE_MODE : MLDSNP_TYPE_IS_INCLUDE_MODE);
                MLDSNP_TIMER_QueryTimer(port_info.filter_timer_p, &port_src_lst_p->filter_time_elapse);
                port_src_lst_p->num_of_req = 0;
                port_src_lst_p->num_of_ex  = 0;
                port_src_lst_p->join_type  = port_info.join_type;
                find_first                 = TRUE;
            }

            /*needn't to record the null sip_a*/
            if (memcmp(hisam_entry.sip_a, mldsnp_om_null_src_ip_a, SRC_IP_LEN))
            {
                if (MLDSNP_TYPE_LIST_INCLUDE == port_info.list_type
                        || MLDSNP_TYPE_LIST_REQUEST == port_info.list_type)
                {
                    memcpy(port_src_lst_p->request_list[port_src_lst_p->num_of_req], hisam_entry.sip_a, SRC_IP_LEN);
                    port_src_lst_p->num_of_req ++;
                }
                else
                {
                    memcpy(port_src_lst_p->exclude_list[port_src_lst_p->num_of_ex], hisam_entry.sip_a, SRC_IP_LEN);
                    port_src_lst_p->num_of_ex ++;
                }
            }
        }

        MLDSNP_OM_SetVidGroupSrcKey(key_a, hisam_entry.vid, hisam_entry.gip_a, hisam_entry.sip_a);
    }
    MLDSNP_OM_UNLOCK();
    return find_first; /*if ever find entry, return true*/
}/*End of MLDSNP_POM_GetNextPortGroupSourceList*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextPortGroupSource
*------------------------------------------------------------------------------
* Purpose: This function get the port jonied (S,G)
* INPUT  : port_source_linfo_p->vid    - the next vlan id
*          port_source_linfo_p->port   - the port to get
*          port_source_linfo_p->gip_a  - current group
*          port_source_linfo_p->sip_a  - current group
* OUTPUT :  port_src_lst_p - the next vid, gip, sip
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetNextPortGroupSource(
    MLDSNP_OM_PortSourceInfo_T *port_src_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    MLDSNP_OM_PortInfo_T   port_info;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    UI32_T time_now = SYS_TIME_GetSystemTicksBy10ms();

    MLDSNP_OM_SetVidGroupSrcKey(key_a, port_src_p->vid, port_src_p->gip_a, port_src_p->sip_a);

    MLDSNP_OM_LOCK();
    while (TRUE == MLDSNP_OM_HisamGetNextRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        port_info.port_no = port_src_p->port;

        if (FALSE == L_SORT_LST_Get(&hisam_entry.register_port_list, &port_info))
        {
            MLDSNP_OM_SetVidGroupSrcKey(key_a, hisam_entry.vid, hisam_entry.gip_a, hisam_entry.sip_a);
            continue;
        }

        port_src_p->vid = hisam_entry.vid;
        IPV6_ADDR_COPY(port_src_p->gip_a, hisam_entry.gip_a);
        IPV6_ADDR_COPY(port_src_p->sip_a, hisam_entry.sip_a);
        port_src_p->join_type  = port_info.join_type;
        port_src_p->unreply_q_count = port_info.specific_query_count;

        MLDSNP_TIMER_QueryTimer(port_info.filter_timer_p, &port_src_p->expire);
        if (time_now < port_info.register_time)
        {
            port_src_p->up_time = ((0xffffffff - port_info.register_time) + time_now) / 100; /*to sec*/
        }
        else
            port_src_p->up_time = (time_now - port_info.register_time) / 100; /*to sec*/

        MLDSNP_OM_UNLOCK();
        return TRUE;
    }

    MLDSNP_OM_UNLOCK();
    return FALSE;
}/*End of MLDSNP_OM_GetNextPortGroupSource*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextPortGroupSourceHost
*------------------------------------------------------------------------------
* Purpose: This function get the port joined (S,G, host ip)
* INPUT  : port_source_linfo_p->vid    - the next vlan id
*          port_source_linfo_p->port   - the port to get
*          port_source_linfo_p->gip_a  - current group
*          port_source_linfo_p->sip_a  - current group
*          port_source_linfo_p->host_a  - current group
* OUTPUT :  port_src_lst_p - the next vid, gip, sip
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetNextPortGroupSourceHost(
    MLDSNP_OM_PortHostInfo_T *port_host)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    MLDSNP_OM_PortInfo_T   port_info;
    struct L_listnode *node;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    UI8_T *tmp_p;
    BOOL_T find = FALSE;

    MLDSNP_OM_SetVidGroupSrcKey(key_a, port_host->vid, port_host->gip_a, port_host->sip_a);

    MLDSNP_OM_LOCK();
    if (TRUE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        port_info.port_no = port_host->port;

        if (TRUE == L_SORT_LST_Get(&hisam_entry.register_port_list, &port_info))
        {
            port_host->vid = hisam_entry.vid;
            IPV6_ADDR_COPY(port_host->gip_a, hisam_entry.gip_a);
            IPV6_ADDR_COPY(port_host->sip_a, hisam_entry.sip_a);

            for (node = port_info.host_sip_lst.head; node; L_NEXTNODE(node))
            {
                if ((tmp_p = L_GETDATA(node)) == NULL)
                    continue;

                if (find)
                {
                    IPV6_ADDR_COPY(port_host->host_a, tmp_p);
                    MLDSNP_OM_UNLOCK();
                    return TRUE;
                }

                if (IPV6_ADDR_SAME(tmp_p, port_host->host_a))
                {
                    find = TRUE;
                    continue;
                }
            }
        }
    }

    /*get next group*/
    while (TRUE == MLDSNP_OM_HisamGetNextRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        port_info.port_no = port_host->port;

        if (FALSE == L_SORT_LST_Get(&hisam_entry.register_port_list, &port_info))
        {
            MLDSNP_OM_SetVidGroupSrcKey(key_a, hisam_entry.vid, hisam_entry.gip_a, hisam_entry.sip_a);
            continue;
        }

        port_host->vid = hisam_entry.vid;
        IPV6_ADDR_COPY(port_host->gip_a, hisam_entry.gip_a);
        IPV6_ADDR_COPY(port_host->sip_a, hisam_entry.sip_a);

        for (node = port_info.host_sip_lst.head; node; L_NEXTNODE(node))
        {
            if ((tmp_p = L_GETDATA(node)) == NULL)
                continue;
            /*get first*/
            IPV6_ADDR_COPY(port_host->host_a, tmp_p);
            MLDSNP_OM_UNLOCK();
            return TRUE;
        }

        MLDSNP_OM_SetVidGroupSrcKey(key_a, hisam_entry.vid, hisam_entry.gip_a, hisam_entry.sip_a);
    }

    MLDSNP_OM_UNLOCK();
    return FALSE;
}/*End of MLDSNP_OM_GetNextPortGroupSource*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetHisamEntryInfo
*------------------------------------------------------------------------------
* Purpose: This function get the info in hisam entry
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip
*          *sip_ap  - the source ip
*          key_idx  - use which key to get
* OUTPUT : *entry_info_p   - the infomation store in hisam
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetHisamEntryInfo(
    UI16_T                      vid,
    UI8_T                       *gip_ap,
    UI8_T                       *sip_ap,
    MLDSNP_TYPE_HisamKeyIndex_T key_idx,
    MLDSNP_OM_HisamEntry_T      *entry_info_p)
{
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, sip_ap);

    MLDSNP_OM_LOCK();
    if (FALSE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, key_idx, key_a, (UI8_T *)entry_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetHisamEntryInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextHisamEntryInfo
*------------------------------------------------------------------------------
* Purpose: This function get the info in hisam entry
* INPUT  : *vid_p   - the vlan id
*          *gip_ap  - the group ip
*          *sip_ap  - the source ip
*          key_idx  - use which key to get
* OUTPUT :  *vid_p  - the next vlan id
*          *gip_ap  - the next group ip
*          *sip_ap  - the next source ip
*          *entry_info_p  - the infomation store in hisam
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextHisamEntryInfo(
    UI16_T                      *vid_p,
    UI8_T                       *gip_ap,
    UI8_T                       *sip_ap,
    MLDSNP_TYPE_HisamKeyIndex_T key_idx,
    MLDSNP_OM_HisamEntry_T      *entry_info_p)
{
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};

    MLDSNP_OM_SetVidGroupSrcKey(key_a, *vid_p, gip_ap, sip_ap);

    MLDSNP_OM_LOCK();
    if (FALSE == MLDSNP_OM_HisamGetNextRecord(&group_entry_hisam_desc_g, key_idx, key_a, (UI8_T *)entry_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    *vid_p = entry_info_p->vid;
    memcpy(gip_ap, entry_info_p->gip_a, GROUP_LEN);
    memcpy(sip_ap, entry_info_p->sip_a, SRC_IP_LEN);

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetNextHisamEntryInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_RemoveAllGroupsInVlan
*------------------------------------------------------------------------------
* Purpose: This function will remove the specified vlan's all group
* INPUT  : vid     - the vlan id
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :this function won't take care the pointer in the entry
*              please make sure you clear the pointer in this hisam entry
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_RemoveAllGroupsInVlan(
    UI16_T vid)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    UI8_T gip_a[GROUP_LEN]           = {0};
    UI8_T sip_a[SRC_IP_LEN]          = {0};

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_a, sip_a);

    MLDSNP_OM_LOCK();

    /*delete (vid, gip, 0)*/
    if (TRUE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        /*delete all port link in hisam entry*/
        if (FALSE == L_SORT_LST_Delete_All(&hisam_entry.register_port_list))
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }
        MLDSNP_OM_HisamDeleteRecord(&group_entry_hisam_desc_g, key_a);
        MLDSNP_OM_LocalVlanIncStat(vid, MLDSNP_TYPE_STAT_GRROUPS, FALSE);
    }

    /*delete (vid, gip, sip)*/
    while (TRUE == MLDSNP_OM_HisamGetNextRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        if (hisam_entry.vid != vid)
            break;

        MLDSNP_OM_SetVidGroupSrcKey(key_a, hisam_entry.vid, hisam_entry.gip_a, hisam_entry.sip_a);
        /*delete all port link in hisam entry*/
        if (FALSE == L_SORT_LST_Delete_All(&hisam_entry.register_port_list))
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }

        MLDSNP_OM_HisamDeleteRecord(&group_entry_hisam_desc_g, key_a);
        MLDSNP_OM_LocalVlanIncStat(vid, MLDSNP_TYPE_STAT_GRROUPS, FALSE);
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End if MLDSNP_OM_RemoveAllGroupsInVlan*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_RemoveAllHisamEntry
*------------------------------------------------------------------------------
* Purpose: This function will remove all hisam entry
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :this function won't take care the pointer in the entry
*              please make sure you clear the pointer in this hisam entry
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_RemoveAllHisamEntry()
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    UI8_T gip_a[GROUP_LEN] = {0};
    UI8_T sip_a[SRC_IP_LEN] = {0};

    MLDSNP_OM_SetVidGroupSrcKey(key_a, 0, gip_a, sip_a);

    MLDSNP_OM_LOCK();

    while (TRUE == MLDSNP_OM_HisamGetNextRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        MLDSNP_OM_SetVidGroupSrcKey(key_a, hisam_entry.vid, hisam_entry.gip_a, hisam_entry.sip_a);

        /*delete all port link in hisam entry*/
        if (FALSE == L_SORT_LST_Delete_All(&hisam_entry.register_port_list))
        {
            continue;
        }
        MLDSNP_OM_HisamDeleteRecord(&group_entry_hisam_desc_g, key_a);
        MLDSNP_OM_LocalVlanIncStat(hisam_entry.vid, MLDSNP_TYPE_STAT_GRROUPS, FALSE);
    }
    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End if MLDSNP_OM_RemoveAllHisamEntry*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_UpdatePortInfo
*------------------------------------------------------------------------------
* Purpose: This function is used to update the port info
* INPUT  : vid                   - the vlan id
*          *gip_ap               - the group ip
*          *sip_ap               - the source ip
*          *update_port_info_p   - the update port info
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  if the PortInfo not exit, it will return fail
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_UpdatePortInfo(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    MLDSNP_OM_PortInfo_T *update_port_info_p)
{
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    MLDSNP_OM_HisamEntry_T hisam_entry;

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, sip_ap);

    MLDSNP_OM_LOCK();
    if (FALSE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        MLDSNP_OM_UNLOCK();    
        return FALSE;
    }

    if (FALSE == L_SORT_LST_Set(&hisam_entry.register_port_list, update_port_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }
#if 0
    if(FALSE == MLDSNP_OM_HisamSetRecord(&group_entry_hisam_desc_g, (UI8_T *)&hisam_entry))
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }
#endif
    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_UpdatePortInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_UpdatePortInfoFromHisam
*------------------------------------------------------------------------------
* Purpose: This function is used to update the port info
* INPUT  : *entry_info_p         - the hisam entry info
*          *update_port_info_p   - the update port info
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  if the PortInfo not exit, it will return fail
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_UpdatePortInfoFromHisam(
    MLDSNP_OM_HisamEntry_T *entry_info_p,
    MLDSNP_OM_PortInfo_T   *update_port_info_p)
{
    if (NULL == entry_info_p || NULL == update_port_info_p)
    {
        return FALSE;
    }

    MLDSNP_OM_LOCK();
    if (FALSE == L_SORT_LST_Set(&entry_info_p->register_port_list, update_port_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }
#if 0
    if(FALSE == MLDSNP_OM_HisamSetRecord(&group_entry_hisam_desc_g, (UI8_T *)entry_info_p))
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }
#endif
    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_UpdatePortInfo*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetPortInfo
*------------------------------------------------------------------------------
* Purpose: This function get the port info
* INPUT  : vid                - the vlan id
*          *gip_ap            - the group ip
*          *sip_ap            - the source ip
*          lport              - the logical port
* OUTPUT : retrun_port_info_p - the port info
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetPortInfo(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    UI16_T               lport,
    MLDSNP_OM_PortInfo_T *retrun_port_info_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, sip_ap);

    MLDSNP_OM_LOCK();
    if (FALSE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    retrun_port_info_p->port_no = lport;
    if (FALSE == L_SORT_LST_Get(&hisam_entry.register_port_list, retrun_port_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetPortInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_AddPortHostIp
*------------------------------------------------------------------------------
* Purpose: This function  add the host ip to port
* INPUT  : vid                - the vlan id
*          *gip_ap            - the group ip
*          *sip_ap            - the source ip
*          port_info_p        - the port info
*          lport              - the logical port
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_AddPortHostIp(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    UI8_T                *host_src_ip,
    MLDSNP_OM_PortInfo_T *port_info_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    struct L_listnode *node;
    void *tmp = NULL, *p = L_MM_Malloc(SYS_ADPT_IPV6_ADDR_LEN, \
                                       L_MM_USER_ID2(SYS_MODULE_MLDSNP, MLDSNP_TYPE_TRACE_HOST_IP));
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};
    BOOL_T find = FALSE;

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, sip_ap);

    if (!p)
    {
        return TRUE;
    }

    MLDSNP_OM_LOCK();

    if (FALSE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        L_MM_Free(p);
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    IPV6_ADDR_COPY(p, host_src_ip);

    for (node = port_info_p->host_sip_lst.head; node; L_NEXTNODE(node))
    {
        if ((tmp = L_GETDATA(node)) == NULL)
            continue;

        if (IPV6_ADDR_SAME(tmp, p))
        {
            find = TRUE;
            L_MM_Free(p);
            break;
        }
    }
    if (!find)
    {
        //limit the record host size
        if(port_info_p->host_sip_lst.count >= SYS_ADPT_MLDSNP_MAX_NBR_OF_RECORD_HOST_IP)
        {
            L_MM_Free(p);
            MLDSNP_OM_UNLOCK();
            return TRUE;
        }

        L_listnode_add(&port_info_p->host_sip_lst, (void *)(p));
    }

    L_SORT_LST_Set(&hisam_entry.register_port_list, port_info_p);

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetPortInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeletePortHostIp
*------------------------------------------------------------------------------
* Purpose: This function delet the host ip from port
* INPUT  : vid                - the vlan id
*          *gip_ap            - the group ip
*          *sip_ap            - the source ip
*          port_info_p        - the port info
*          port_info_p        - the port info
*          lport              - the logical port
* OUTPUT : None
* RETURN : host ocunt on this port
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_DeletePortHostIp(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    UI8_T                *host_src_ip,
    MLDSNP_OM_PortInfo_T *port_info_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    struct L_listnode *node;
    void *tmp = NULL;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, sip_ap);

    MLDSNP_OM_LOCK();

    if (FALSE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        MLDSNP_OM_UNLOCK();
        return 0;
    }

    for (node = port_info_p->host_sip_lst.head; node; L_NEXTNODE(node))
    {
        if ((tmp = L_GETDATA(node)) == NULL)
            continue;

        if (IPV6_ADDR_SAME(tmp, host_src_ip))
        {
            if (node->prev)
                node->prev->next = node->next;
            else
                port_info_p->host_sip_lst.head = node->next;

            if (node->next)
                node->next->prev = node->prev;
            else
                port_info_p->host_sip_lst.tail = node->prev;

            port_info_p->host_sip_lst.count--;
            L_MM_Free(tmp);
            free(node);
            break;
        }
    }

    L_SORT_LST_Set(&hisam_entry.register_port_list, port_info_p);

    MLDSNP_OM_UNLOCK();

    return port_info_p->host_sip_lst.count;
}/*End of MLDSNP_OM_GetPortInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetFirstPortInfo
*------------------------------------------------------------------------------
* Purpose: This function get the first port info in the hisam entry
* INPUT  : vid                - the vlan id
*          *gip_ap            - the group ip
*          *sip_ap            - the source ip
* OUTPUT : retrun_port_info_p - the port info
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetFirstPortInfo(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    MLDSNP_OM_PortInfo_T *retrun_port_info_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, sip_ap);

    MLDSNP_OM_LOCK();
    if (FALSE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    if (FALSE == L_SORT_LST_Get_1st(&hisam_entry.register_port_list, retrun_port_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }
    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetFirstPortInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextPortInfo
*------------------------------------------------------------------------------
* Purpose: This function get the next port info
* INPUT  : vid                 - the vlan id
*          *gip                - the group ip
*          *sip                - the source ip
*          *lport_p            - the input port number to get next port number
* OUTPUT : return_port_info_p  - the next group info
*          *lport              - the next port
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  return fail mean in this group entry has no port exist.
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextPortInfo(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    UI16_T               *lport_p,
    MLDSNP_OM_PortInfo_T *return_port_info_p)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    UI8_T key_a[VID_GROUP_SRCIP_LEN] = {0};

    MLDSNP_OM_SetVidGroupSrcKey(key_a, vid, gip_ap, sip_ap);

    MLDSNP_OM_LOCK();
    if (FALSE == MLDSNP_OM_HisamGetRecord(&group_entry_hisam_desc_g, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, key_a, (UI8_T *)&hisam_entry))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    return_port_info_p->port_no = *lport_p;
    if (FALSE == L_SORT_LST_Get_Next(&hisam_entry.register_port_list, return_port_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    *lport_p = return_port_info_p->port_no;

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetNextPortInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextVlanRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the vlan's router list
* INPUT  : *vid_p           - the vlan id
* OUTPUT : router_port_list - the router port info
*         *vid_p            - the next vid
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is only provide to UI to access
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextVlanRouterPortlist(
    UI16_T                     *vid_p,
    MLDSNP_OM_RouterPortList_T *router_port_list)
{
    MLDSNP_OM_RouterPortInfo_T router_port_info;
    MLDSNP_OM_VlanInfo_T       vlan_info;

    MLDSNP_OM_LOCK();

    vlan_info.vid = *vid_p;

    if (FALSE == L_SORT_LST_Get_Next(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    memset(router_port_list, 0, sizeof(MLDSNP_OM_RouterPortList_T));
    router_port_list->vid = *vid_p;

    router_port_info.port_no = 0;

    while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &router_port_info))
    {
        if (MLDSNP_TYPE_JOIN_DYNAMIC == router_port_info.attribute)
        {
            MLDSNP_TYPE_AddPortIntoPortBitMap(router_port_info.port_no, router_port_list->router_port_bitmap);
        }
        else if (MLDSNP_TYPE_JOIN_STATIC == router_port_info.attribute)
        {
            MLDSNP_TYPE_AddPortIntoPortBitMap(router_port_info.port_no, router_port_list->static_router_port_bitmap);
            MLDSNP_TYPE_AddPortIntoPortBitMap(router_port_info.port_no, router_port_list->router_port_bitmap);
        }
    }

    *vid_p = vlan_info.vid;
    router_port_list->vid = vlan_info.vid;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetNextVlanRouterPortlist*/

BOOL_T MLDSNP_OM_GetRouterExpire(UI16_T vid, UI32_T lport, UI32_T *expire_p)
{
    MLDSNP_OM_RouterPortInfo_T router_port_info;
    MLDSNP_OM_VlanInfo_T       vlan_info;

    MLDSNP_OM_LOCK();

    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    router_port_info.port_no = lport;
    if (FALSE == L_SORT_LST_Get(&vlan_info.router_port_list, &router_port_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_TIMER_QueryTimer(router_port_info.router_timer_p, expire_p);

    MLDSNP_OM_UNLOCK();
    return TRUE;
}


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetVlanRouterPortCount
*------------------------------------------------------------------------------
* Purpose: This function get the vlan's router port count
* INPUT  : vid  - the vlan id
*
* OUTPUT : None
* RETURN : the count of router port
* NOTES  :
*------------------------------------------------------------------------------*/
UI16_T MLDSNP_OM_GetVlanRouterPortCount(
    UI16_T vid)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();

    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return 0;
    }

    MLDSNP_OM_UNLOCK();
    return vlan_info.router_port_list.nbr_of_element;
}/*End of MLDSNP_OM_GetVlanRouterPortCount*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRouterPortInfo
*------------------------------------------------------------------------------
* Purpose: This function get the router port info
* INPUT  : vid    - the vlan id
*          r_port - the router port no
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : return_router_port_info_p  - the router port info
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetRouterPortInfo(
    UI16_T                     vid,
    UI16_T                     r_port,
    MLDSNP_OM_RouterPortInfo_T *return_router_port_info_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    return_router_port_info_p->port_no = r_port;
    if (FALSE == L_SORT_LST_Get(&vlan_info.router_port_list, return_router_port_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetRouterPortInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextRouterPortInfo
*------------------------------------------------------------------------------
* Purpose: This function get the router port info
* INPUT  : vid     - the vlan id
*          r_port  - the port number to get next
* OUTPUT : return_router_port_info_p  - the router port info
*          *r_port                    - the next router port
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextRouterPortInfo(
    UI16_T                     vid,
    UI16_T                     *r_port,
    MLDSNP_OM_RouterPortInfo_T *return_router_port_info_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    return_router_port_info_p->port_no = *r_port;

    if (FALSE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, return_router_port_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    *r_port = return_router_port_info_p->port_no;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetNextRouterPortInfo*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_AddRouterPort
*------------------------------------------------------------------------------
* Purpose: This function add the router port into vlan
* INPUT  : vid                - the vlan id
*          *router_port_info_p- the router port info
*
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_AddRouterPort(
    UI16_T                     vid,
    MLDSNP_OM_RouterPortInfo_T *router_port_info_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    if (FALSE == L_SORT_LST_Set(&vlan_info.router_port_list, router_port_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    if (FALSE == L_SORT_LST_Set(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }
    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_AddRouterPort*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteRouterPort
*------------------------------------------------------------------------------
* Purpose: This function remove the router port from vlan
* INPUT  : vid                - the vlan id
*          *router_port_info_p- the router port info
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteRouterPort(
    UI16_T                     vid,
    MLDSNP_OM_RouterPortInfo_T *router_port_info_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    if (FALSE == L_SORT_LST_Delete(&vlan_info.router_port_list, router_port_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    if (FALSE == L_SORT_LST_Set(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }
    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_DeleteRouterPort*/

/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_UpdateRouterPortInfo
*------------------------------------------------------------------------------
* Purpose: This function update the router port info
* INPUT  : vid                - the vlan id
*          *router_port_info_p- the router port info
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_UpdateRouterPortInfo(
    UI16_T                     vid,
    MLDSNP_OM_RouterPortInfo_T *router_port_info_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    if (FALSE == L_SORT_LST_Set(&vlan_info.router_port_list, router_port_info_p))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    if (FALSE == L_SORT_LST_Set(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_UpdateRouterPortInfo*/

/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_IsRouterPort
*------------------------------------------------------------------------------
* Purpose: This function check this port is router port
* INPUT  : vid   - the vlan id to check
*          lport - the logical port to check
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_IsRouterPort(
    UI16_T vid,
    UI16_T lport)
{
    MLDSNP_OM_VlanInfo_T vlan_info;
    MLDSNP_OM_RouterPortInfo_T router_port_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }
    router_port_info.port_no = lport;
    if (FALSE == L_SORT_LST_Get(&vlan_info.router_port_list, &router_port_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_IsRouterPort*/


/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_SetVlanInfo
*------------------------------------------------------------------------------
* Purpose: This function set the vlan info
* INPUT  :*add_vlan_info_p - the vlan info which will be update or insert to list
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetVlanInfo(MLDSNP_OM_VlanInfo_T *add_vlan_info_p)
{
    BOOL_T ret = FALSE;

    MLDSNP_OM_LOCK();

    ret = L_SORT_LST_Set(&mldsnp_om_vlan_list_g, add_vlan_info_p);

    MLDSNP_OM_UNLOCK();
    return ret;
}/*End of MLDSNP_OM_SetVlanInfo*/
/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_GetVlanInfo
*------------------------------------------------------------------------------
* Purpose: This function get the vlan info
* INPUT  :vid - the vlan info which will be get
* OUTPUT : *vlan_info_p
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetVlanInfo(
    UI16_T vid,
    MLDSNP_OM_VlanInfo_T *vlan_info_p)
{
    BOOL_T ret = FALSE;

    MLDSNP_OM_LOCK();

    vlan_info_p->vid = vid;
    ret = L_SORT_LST_Get(&mldsnp_om_vlan_list_g, vlan_info_p);

    MLDSNP_OM_UNLOCK();

    return ret;
}/*End of MLDSNP_OM_GetVlanInfo*/
/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_GetNextVlanInfo
*------------------------------------------------------------------------------
* Purpose: This function get the next vlan info
* INPUT  :*vid_p - the vlan info which will be get
* OUTPUT : *vlan_info_p
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextVlanInfo(
    UI16_T               *vid_p,
    MLDSNP_OM_VlanInfo_T *vlan_info_p)
{
    BOOL_T ret = FALSE;

    MLDSNP_OM_LOCK();

    vlan_info_p->vid = *vid_p;
    ret              = L_SORT_LST_Get_Next(&mldsnp_om_vlan_list_g, vlan_info_p);
    *vid_p           = vlan_info_p->vid;

    MLDSNP_OM_UNLOCK();
    return ret;
}/*End of MLDSNP_OM_GetNextVlanInfo*/
/*------------------------------------------------------------------------------
* Function :  MLDSNP_OM_DeleteVlanInfo
*------------------------------------------------------------------------------
* Purpose: This function delete the vlan info
* INPUT  :*vlan_info_p.vid - the vlan info which will be deleted
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteVlanInfo(
    MLDSNP_OM_VlanInfo_T *vlan_info_p)
{
    BOOL_T ret = FALSE;

    MLDSNP_OM_LOCK();

    ret = L_SORT_LST_Delete(&mldsnp_om_vlan_list_g, vlan_info_p);

    MLDSNP_OM_UNLOCK();

    return ret;
}/*End of MLDSNP_OM_DeleteVlanInfo*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRunningGlobalConf
*------------------------------------------------------------------------------
* Purpose: This function get the running configuration
* INPUT  : None
* OUTPUT : *mldsnp_global_conf_p - the global configuration
* RETUEN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*           SYS_TYPE_GET_RUNNING_CFG_FAIL    - get failure
*           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetRunningGlobalConf(
    MLDSNP_OM_RunningCfg_T *mldsnp_global_conf_p)
{
    MLDSNP_OM_LOCK();
    memset(mldsnp_global_conf_p, 0, sizeof(MLDSNP_OM_RunningCfg_T));

    if (MLDSNP_TYPE_DFLT_MLDSNP_STATUS != mldsnp_om_cfg_db_g.mldsnp_status)
    {
        mldsnp_global_conf_p->mldsnp_status         = mldsnp_om_cfg_db_g.mldsnp_status;
        mldsnp_global_conf_p->mldsnp_status_changed = TRUE;
    }

    if (MLDSNP_TYPE_DFLT_VER != mldsnp_om_cfg_db_g.version)
    {
        mldsnp_global_conf_p->version         = mldsnp_om_cfg_db_g.version;
        mldsnp_global_conf_p->version_changed = TRUE;
    }

    if (MLDSNP_TYPE_DFLT_QUERIER_STATUS != mldsnp_om_cfg_db_g.querier_status)
    {
        mldsnp_global_conf_p->querier_status         = mldsnp_om_cfg_db_g.querier_status;
        mldsnp_global_conf_p->querier_status_changed = TRUE;
    }

    if (MLDSNP_TYEP_DFLT_ROBUST_VALUE != mldsnp_om_cfg_db_g.robust_value)
    {
        mldsnp_global_conf_p->robust_value = mldsnp_om_cfg_db_g.robust_value;
        mldsnp_global_conf_p->robust_value_changed = TRUE;
    }

    if (MLDSNP_TYPE_DFLT_QUERY_INTERVAL != mldsnp_om_cfg_db_g.query_interval)
    {
        mldsnp_global_conf_p->query_interval         = mldsnp_om_cfg_db_g.query_interval;
        mldsnp_global_conf_p->query_interval_changed = TRUE;
    }

    if (MLDSNP_TYPE_DFLT_MAX_RESP_INTERVAL != mldsnp_om_cfg_db_g.query_response_interval)
    {
        mldsnp_global_conf_p->query_response_interval         = mldsnp_om_cfg_db_g.query_response_interval;
        mldsnp_global_conf_p->query_response_interval_changed = TRUE;
    }

    if (MLDSNP_TYPE_DFLT_ROUTER_EXP_TIME != mldsnp_om_cfg_db_g.router_exp_time)
    {
        mldsnp_global_conf_p->router_exp_time         = mldsnp_om_cfg_db_g.router_exp_time;
        mldsnp_global_conf_p->router_exp_time_changed = TRUE;
    }
#if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    if (MLDSNP_TYPE_DFLT_FLOOD_BEHAVIOR != mldsnp_om_cfg_db_g.unknown_flood_behavior)
    {
        mldsnp_global_conf_p->unknown_flood_behavior         = mldsnp_om_cfg_db_g.unknown_flood_behavior;
        mldsnp_global_conf_p->unknown_flood_behaviro_changed = TRUE;
    }
#endif
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    mldsnp_global_conf_p->proxy_reporting           = mldsnp_om_cfg_db_g.proxy_reporting;
    mldsnp_global_conf_p->unsolicit_report_interval = mldsnp_om_cfg_db_g.unsolicited_report_interval;
#endif
    MLDSNP_OM_UNLOCK();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/*End of MLDSNP_OM_GetRunningGlobalConf*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRunningRouterPortList
*------------------------------------------------------------------------------
* Purpose: This function get  the run static router port bit list
* INPUT  : *vid - the vlan id to get next vlan id
* OUTPUT :  *router_port_bitmap_p  - the router port bit map pointer
* RETUEN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*           SYS_TYPE_GET_RUNNING_CFG_FAIL    - get failure
*           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetRunningRouterPortList(
    UI16_T vid,
    UI8_T  *router_port_bitmap_p)
{
    MLDSNP_OM_RouterPortList_T router_port_list;

    MLDSNP_OM_LOCK();
    if (FALSE == MLDSNP_OM_GetVlanRouterPortlist(vid, &router_port_list))
    {
        MLDSNP_OM_UNLOCK();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    memcpy(router_port_bitmap_p, router_port_list.static_router_port_bitmap, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    MLDSNP_OM_UNLOCK();
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/*End of MLDSNP_OM_GetRunningRouterPortList*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRunningVlanImmediateStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate running status
* INPUT  : vid  - the vlan id
* OUTPUT : *status_p   - the immediate status
* RETUEN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*           SYS_TYPE_GET_RUNNING_CFG_FAIL    - get failure
*           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetRunningVlanImmediateStatus(
    UI16_T                        vid,
    MLDSNP_TYPE_ImmediateStatus_T *imme_status_p,
    MLDSNP_TYPE_ImmediateByHostStatus_T *imme_byhost_status_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

     *imme_status_p = vlan_info.immediate_leave_status;
     *imme_byhost_status_p = vlan_info.immediate_leave_byhost_status;

    if ((SYS_DFLT_MLDSNP_IMMEIDATE_STATUS != vlan_info.immediate_leave_status)
        || (SYS_DFLT_MLDSNP_IMMEIDATE_BYHOST_STATUS != vlan_info.immediate_leave_byhost_status))
    {
        MLDSNP_OM_UNLOCK();
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    MLDSNP_OM_UNLOCK();
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}/*End of MLDSNP_OM_GetRunningVlanImmediateStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetQuerierRunningStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier running status of the input vlan
* INPUT  : vid    - the vlan id
*          status - the querier status
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetQuerierRunningStatus(
    UI16_T                      vid,
    MLDSNP_TYPE_QuerierStatus_T status)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    vlan_info.querier_runing_status = status;

    if (FALSE == L_SORT_LST_Set(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_SetQuerierRunningStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQuerierRunningStatus
*------------------------------------------------------------------------------
* Purpose: This function get the querier running status of the input vlan
* INPUT  : vid        - the vlan id
* OUTPUT : *status_p  - the querier running status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetQuerierRunningStatus(
    UI16_T                      vid,
    MLDSNP_TYPE_QuerierStatus_T *status_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;
    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    *status_p = vlan_info.querier_runing_status;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetQuerierRunningStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_InitialAllVlanQuerierRunningStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier running status of the input vlan
* INPUT  : runining_status - the initial querier running status
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_InitialAllVlanQuerierRunningStatus(
    MLDSNP_TYPE_QuerierStatus_T runining_status)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = 0;

    while (TRUE == L_SORT_LST_Get_Next(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        if (NULL == vlan_info.other_querier_present_timer_p)
        { /*if there is no other querier in this vlan exist*/
            vlan_info.querier_runing_status = runining_status;
            if ((runining_status == MLDSNP_TYPE_QUERIER_ENABLED)
                && (vlan_info.querier_uptime == 0))
            {
                vlan_info.querier_uptime =  SYSFUN_GetSysTick();
                vlan_info.query_oper_interval = mldsnp_om_cfg_db_g.query_interval;
                vlan_info.robust_oper_value = mldsnp_om_cfg_db_g.robust_value;
            }
            else if (runining_status == MLDSNP_TYPE_QUERIER_DISABLED)
            {
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                if (mldsnp_om_cfg_db_g.proxy_reporting == MLDSNP_TYPE_PROXY_REPORTING_DISABLE)
#endif
                    vlan_info.querier_uptime = 0;
            }
            L_SORT_LST_Set(&mldsnp_om_vlan_list_g, &vlan_info);
        }
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_InitialAllVlanQuerierRunningStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetGlobalConf
*------------------------------------------------------------------------------
* Purpose: This function get the global configuration
* INPUT  : None
*
* OUTPUT : *mldsnp_global_conf_p - the global configuration
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetGlobalConf(
    MLDSNP_OM_Cfg_T *mldsnp_global_conf_p)
{
    MLDSNP_OM_LOCK();

    memcpy(mldsnp_global_conf_p, &mldsnp_om_cfg_db_g, sizeof(MLDSNP_OM_Cfg_T));

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetGlobalConf*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetMldStatus
*------------------------------------------------------------------------------
* Purpose: This function get the Mld status
* INPUT  : None
* OUTPUT : *mldsnp_status_p - the mldsnp status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetMldStatus(
    MLDSNP_TYPE_MLDSNP_STATUS_T *mldsnp_status_p)
{
    MLDSNP_OM_LOCK();

    *mldsnp_status_p = mldsnp_om_cfg_db_g.mldsnp_status;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetMldStatus*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetMldStatus
*------------------------------------------------------------------------------
* Purpose: This function set the Mld status
* INPUT  : mldsnp_status - the mldsnp status
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetMldStatus(
    MLDSNP_TYPE_MLDSNP_STATUS_T mldsnp_status)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_cfg_db_g.mldsnp_status = mldsnp_status;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetMldGtatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetMldSnpVer
*------------------------------------------------------------------------------
* Purpose: This function get the mldsno version
* INPUT  : None
* OUTPUT : *ver_p - the version
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetMldSnpVer(
    UI16_T *ver_p)
{
    MLDSNP_OM_LOCK();

    *ver_p = mldsnp_om_cfg_db_g.version;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetMldSnpVer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetMldSnpVer
*------------------------------------------------------------------------------
* Purpose: This function set the mldsno version
* INPUT  : ver
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetMldSnpVer(
    MLDSNP_TYPE_Version_T ver)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_cfg_db_g.version = ver;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetMldSnpVer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_StoreQuerierTimer
*------------------------------------------------------------------------------
* Purpose: This function store the querier timer pointer
* INPUT  : *timer_p  - the querier timer pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_StoreQuerierTimer(
    MLDSNP_Timer_T *timer_p)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_querier_timer_p = timer_p;

    MLDSNP_OM_UNLOCK();

    return;
}/*End of MLDSNP_OM_StoreQuerierTimer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQuerierTimer
*------------------------------------------------------------------------------
* Purpose: This function get the querier timer pointer
* INPUT  : **timer_p - the querier timer pointer pointer
* OUTPUT : **timer_p - the qurerier timer pointer
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_GetQuerierTimer(
    MLDSNP_Timer_T **timer_p)
{
    MLDSNP_OM_LOCK();

    *timer_p = mldsnp_om_querier_timer_p;

    MLDSNP_OM_UNLOCK();

    return;
}/*End of MLDSNP_OM_GetQuerierTimer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_StoreMrdSolicitationTimer
*------------------------------------------------------------------------------
* Purpose: This function store the MRD solicitation timer pointer
* INPUT  : *timer_p  - the querier timer pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_StoreMrdSolicitationTimer(
    MLDSNP_Timer_T *timer_p)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_mrd_solicitation_timer_p = timer_p;

    MLDSNP_OM_UNLOCK();

    return;
}/*End of MLDSNP_OM_StoreQuerierTimer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetMrdSolicitationTimer
*------------------------------------------------------------------------------
* Purpose: This function get the mrd solicitation timer pointer
* INPUT  : **timer_p - the querier timer pointer pointer
* OUTPUT : **timer_p - the qurerier timer pointer
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_GetMrdSolicitationTimer(
    MLDSNP_Timer_T **timer_p)
{
    MLDSNP_OM_LOCK();

    *timer_p = mldsnp_om_mrd_solicitation_timer_p;

    MLDSNP_OM_UNLOCK();

    return;
}/*End of MLDSNP_OM_GetMrdSolicitationTimer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_StoreVlanOtherQuerierPresentTimer
*------------------------------------------------------------------------------
* Purpose: This function store the vlan's querier timer pointer
* INPUT  : vid       - the vlan id
*          *timer_p  - the querier timer pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  The vlan's querier timer used to indicate other querier present timer
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_StoreVlanOtherQuerierPresentTimer(
    UI16_T    vid,
    MLDSNP_Timer_T *timer_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    vlan_info.other_querier_present_timer_p = timer_p;

    if (FALSE == L_SORT_LST_Set(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }
    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_StoreVlanOtherQuerierPresentTimer*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetVlanOtherQuerierPresentTimer
*------------------------------------------------------------------------------
* Purpose: This function get the querier timer pointer
* INPUT  : **timer_p - the querier timer pointer pointer
* OUTPUT : **timer_p - the qurerier timer pointer
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetVlanOtherQuerierPresentTimer(UI16_T    vid,
        MLDSNP_Timer_T **timer_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;
    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    *timer_p = vlan_info.other_querier_present_timer_p;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetQuerierTimer*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQuerierStauts
*------------------------------------------------------------------------------
* Purpose: This function get the querier status
* INPUT  : None
* OUTPUT : *querier_status_p - the querier status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetQuerierStauts(
    MLDSNP_TYPE_QuerierStatus_T *querier_status_p)
{
    MLDSNP_OM_LOCK();

    *querier_status_p = mldsnp_om_cfg_db_g.querier_status;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetQuerierStauts*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetQuerierStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier status
* INPUT  : querier_status - the querier status
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetQuerierStatus(
    MLDSNP_TYPE_QuerierStatus_T querier_status)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_cfg_db_g.querier_status = querier_status;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetQuerierStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetMroutStatus
*------------------------------------------------------------------------------
* Purpose: This function set the mroute status
* INPUT  : is_enabled - mroute is enabled or not
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetMroutStatus(BOOL_T is_enabled)
{
    if (is_enabled)
        mldsnp_om_cfg_db_g.flag |= (MLDSNP_OM_MROUTE_ENABLED);
    else
        mldsnp_om_cfg_db_g.flag &= (~MLDSNP_OM_MROUTE_ENABLED);

    return TRUE;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_IsMrouteEnabled
*------------------------------------------------------------------------------
* Purpose: This function check the mroute enabled
* INPUT  : is_enabled - mroute is enabled or not
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_IsMrouteEnabled()
{
    return mldsnp_om_cfg_db_g.flag & (MLDSNP_OM_MROUTE_ENABLED);
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRobustnessValue
*------------------------------------------------------------------------------
* Purpose: This function get the robust ess value
* INPUT  : None
* OUTPUT : *value_p - the robustness value
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetRobustnessValue(UI16_T *value_p)
{
    MLDSNP_OM_LOCK();

    *value_p = mldsnp_om_cfg_db_g.robust_value;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetRobustnessValue*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRobustnessOperValue
*------------------------------------------------------------------------------
* Purpose: This function get the robustness operate value
* INPUT  : None
* OUTPUT : *value_p - the robustness value
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetRobustnessOperValue(
    UI16_T vid,
    UI16_T *value_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();

    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
        *value_p = mldsnp_om_cfg_db_g.robust_value;
    else
        *value_p = vlan_info.robust_oper_value;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetRobustnessOperValue*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetRobustnessValue
*------------------------------------------------------------------------------
* Purpose: This function set the robust ess value
* INPUT  : value - the robustness value
* OUTPUT :None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetRobustnessValue(
    UI16_T value)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_cfg_db_g.robust_value = value;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetRobustnessValue*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion get the query interval
* INPUT  : None
* OUTPUT : *interval_p - the interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.2, 9.2
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetQueryInterval(
    UI16_T *interval_p)
{
    MLDSNP_OM_LOCK();

    *interval_p = mldsnp_om_cfg_db_g.query_interval;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetQueryInterval*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion get the query interval
* INPUT  : None
* OUTPUT : *interval_p - the interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.2, 9.2
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetQueryOperInterval(
    UI16_T vid,
    UI16_T *interval_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();

    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
        *interval_p = mldsnp_om_cfg_db_g.query_interval;
    else
        *interval_p = vlan_info.query_oper_interval;

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetQueryInterval*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion set the query interval
* INPUT  :  interval - the interval in seconds
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.2, 9.2
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetQueryInterval(
    UI16_T interval)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_cfg_db_g.query_interval = interval;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetQueryInterval*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetQueryResponseInterval
*------------------------------------------------------------------------------
* Purpose: This function get the query response interval
* INPUT  : None
* OUTPUT : *interval_p  - the query response interval
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.3, 9.3
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetQueryResponseInterval(
    UI16_T *interval_p)
{
    MLDSNP_OM_LOCK();

    *interval_p = mldsnp_om_cfg_db_g.query_response_interval;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetQueryResponseInterval*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetQueryRresponseInterval
*------------------------------------------------------------------------------
* Purpose: This function get the query response interval
* INPUT  : interval - the query repsonse interval in seconds
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.3, 9.3
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetQueryRresponseInterval(
    UI16_T interval)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_cfg_db_g.query_response_interval = interval;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetQueryResponseInterval*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetListenerInterval
*------------------------------------------------------------------------------
* Purpose: This function get the listner interval
* INPUT  : None
* OUTPUT : *interval_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.4 in RFC2710,  9.4 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetListenerInterval(
    UI16_T *interval_p)
{
    MLDSNP_OM_LOCK();

    *interval_p = mldsnp_om_cfg_db_g.robust_value * mldsnp_om_cfg_db_g.query_interval
                  + mldsnp_om_cfg_db_g.query_response_interval;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetListenerInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetStartupQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function get the querier startup query interval
* INPUT  : None
* OUTPUT : *interval_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.6 in RFC2710,  9.6 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetStartupQueryInterval(
    UI16_T *interval_p)
{
    MLDSNP_OM_LOCK();

    *interval_p = mldsnp_om_cfg_db_g.query_interval / 4;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetStartupQueryInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetStartupQueryCount
*------------------------------------------------------------------------------
* Purpose: This function get the querier startup query count
* INPUT  : None
* OUTPUT : *query_count_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.7 in RFC2710,  9.7 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetStartupQueryCount(
    UI16_T *query_count_p)
{
    MLDSNP_OM_LOCK();

    *query_count_p = mldsnp_om_cfg_db_g.robust_value;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetStartupQueryInterval*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetStartupQuerySentCount
*------------------------------------------------------------------------------
* Purpose: This function set the queris querier startup has been sent
* INPUT  : None
* OUTPUT : *query_count_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.7 in RFC2710,  9.7 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetStartupQuerySentCount(
    UI16_T query_count_p)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_cfg_db_g.querier_start_sent_count = query_count_p;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetStartupQuerySentCount*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetStartupQuerySentCount
*------------------------------------------------------------------------------
* Purpose: This function get the query querier startup has been sent
* INPUT  : None
* OUTPUT : *query_count_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.7 in RFC2710,  9.7 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetStartupQuerySentCount(
    UI16_T *query_count_p)
{
    MLDSNP_OM_LOCK();

    *query_count_p = mldsnp_om_cfg_db_g.querier_start_sent_count;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetStartupQueryInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetOtherQueryPresentInterval
*------------------------------------------------------------------------------
* Purpose: This function get the other querier present interval
* INPUT  : None
* OUTPUT :None
* RETURN : other querier present interval
* NOTES  :  7.5, 9.5 robust * query_interval + query_rsponse_interval
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetOtherQueryPresentInterval()
{
    UI32_T tmp = 0;

    MLDSNP_OM_LOCK();

    tmp = mldsnp_om_cfg_db_g.robust_value * mldsnp_om_cfg_db_g.query_interval
          + mldsnp_om_cfg_db_g.query_response_interval / 2;

    MLDSNP_OM_UNLOCK();

    return tmp;
}/*End of MLDSNP_OM_GetOtherQueryPresentInterval*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetOldVerQuerierPresentTimeOut
*------------------------------------------------------------------------------
* Purpose: This function get the old version querier present time out value
* INPUT  : vid - the vlan id

* OUTPUT : *time_out_p   - the time out in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :9.12 in RFC3180
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetOldVerQuerierPresentTimeOut(
    UI16_T *time_out_p)
{
    MLDSNP_OM_LOCK();

    *time_out_p = mldsnp_om_cfg_db_g.robust_value * mldsnp_om_cfg_db_g.query_interval
                  + mldsnp_om_cfg_db_g.query_response_interval;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetOldVerQuerierPresentTimeOut*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetMALI
*------------------------------------------------------------------------------
* Purpose: This function get MALI value
* INPUT  :  vid - the vlan id
* OUTPUT : None
* RETURN : MALI vaule
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetMALI(
    UI16_T vid)
{
    MLDSNP_OM_VlanInfo_T vlan_info;
    UI32_T mali = 0;

    MLDSNP_OM_LOCK();

    vlan_info.vid = vid;
    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
        mali = mldsnp_om_cfg_db_g.robust_value * mldsnp_om_cfg_db_g.query_interval
               + mldsnp_om_cfg_db_g.query_response_interval;
    else if (FALSE == vlan_info.s_flag)
        mali = mldsnp_om_cfg_db_g.robust_value * mldsnp_om_cfg_db_g.query_interval
               + mldsnp_om_cfg_db_g.query_response_interval;
    else
        mali = vlan_info.robust_oper_value * vlan_info.query_oper_interval
               + mldsnp_om_cfg_db_g.query_response_interval;

    MLDSNP_OM_UNLOCK();
    return mali;
}/*End of MLDSNP_OM_GetMALI*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetLastListenerQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function Get the last listener query interval
* INPUT  : None
* OUTPUT : *interval_p  - the interval in second
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.8, 9.8
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetLastListenerQueryInterval(
    UI16_T *interval_p)
{
    MLDSNP_OM_LOCK();

    *interval_p = mldsnp_om_cfg_db_g.last_listner_query_interval;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetLastListenerQueryInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetLastListnerQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function set the last listener query interval
* INPUT  : interval  - the interval in second
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.8, 9.8
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetLastListnerQueryInterval(
    UI16_T interval)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_cfg_db_g.last_listner_query_interval = interval;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetLastListnerQueryInterval*/

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetUnsolicitTimer
*------------------------------------------------------------------------------
* Purpose: This function store the querier timer pointer
* INPUT  : *timer_p  - the querier timer pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_SetUnsolicitTimer(
    MLDSNP_Timer_T *timer_p)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_unsolicite_timer_p = timer_p;

    MLDSNP_OM_UNLOCK();

    return;
}/*End of MLDSNP_OM_StoreQuerierTimer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetUnsolicitTimer
*------------------------------------------------------------------------------
* Purpose: This function get the querier timer pointer
* INPUT  : **timer_p - the querier timer pointer pointer
* OUTPUT : **timer_p - the qurerier timer pointer
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_GetUnsolicitTimer(
    MLDSNP_Timer_T **timer_p)
{
    MLDSNP_OM_LOCK();

    *timer_p = mldsnp_om_unsolicite_timer_p;

    MLDSNP_OM_UNLOCK();

    return;
}/*End of MLDSNP_OM_GetQuerierTimer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetProxyReporting
*------------------------------------------------------------------------------
* Purpose: This function Get the proxy switching
* INPUT  : None
* OUTPUT : *proxy_staus_p  - the proxy reporting status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetProxyReporting(MLDSNP_TYPE_ProxyReporting_T *proxy_status_p)
{
    MLDSNP_OM_LOCK();

    *proxy_status_p = mldsnp_om_cfg_db_g.proxy_reporting;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetProxyReporting
*------------------------------------------------------------------------------
* Purpose: This function Set the proxy reporting status
* INPUT  : proxy_staus_p  - the proxy reporting status
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetProxyReporting(MLDSNP_TYPE_ProxyReporting_T proxy_staus)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_cfg_db_g.proxy_reporting = proxy_staus;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetUnsolicitedReportInterval
*------------------------------------------------------------------------------
* Purpose: This function Get the unsolicitedReportInterval
* INPUT  : None
* OUTPUT : *interval_p  - the inteval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetUnsolicitedReportInterval(
    UI32_T *interval_p)
{
    MLDSNP_OM_LOCK();

    *interval_p = mldsnp_om_cfg_db_g.unsolicited_report_interval;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetUnsolicitedReportInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetUnsolicitedReportInterval
*------------------------------------------------------------------------------
* Purpose: This function set the unsolicitedReportInterval
* INPUT  : interval  - the inteval in seconds
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetUnsolicitedReportInterval(
    UI16_T interval)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_cfg_db_g.unsolicited_report_interval = interval;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetUnsolicitedReportInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetLastReportHostAddress
*------------------------------------------------------------------------------
* Purpose: This function set last reporter host IP address
* INPUT  : vid   - which vlan
*          src_ap- source ip address
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetLastReportHostAddress(UI16_T vid, UI8_T *src_ap)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    vlan_info.vid = vid;

    MLDSNP_OM_LOCK();

    if (TRUE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        IPV6_ADDR_COPY(vlan_info.last_reporter, src_ap);
        L_SORT_LST_Set(&mldsnp_om_vlan_list_g, &vlan_info);
    }

    MLDSNP_OM_UNLOCK();

    return TRUE;
}
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetLastReportHostAddress
*------------------------------------------------------------------------------
* Purpose: This function get last reporter host IP address
* INPUT  : vid   - which vlan
* OUTPUT : src_ap- source ip address
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetLastReportHostAddress(UI16_T vid, UI8_T *src_ap)
{
    MLDSNP_OM_VlanInfo_T vlan_info;
    BOOL_T ret = FALSE;

    vlan_info.vid = vid;

    MLDSNP_OM_LOCK();

    if (TRUE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        IPV6_ADDR_COPY(src_ap, vlan_info.last_reporter);
        ret = TRUE;
    }

    MLDSNP_OM_UNLOCK();

    return ret;
}

#endif

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRouterExpireTime
*------------------------------------------------------------------------------
* Purpose: This function get the router expire time
* INPUT  :
* OUTPUT : exp_time_p  - the expire time
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetRouterExpireTime(
    UI16_T *exp_time_p)
{
    MLDSNP_OM_LOCK();

    *exp_time_p = mldsnp_om_cfg_db_g.router_exp_time;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetRouterExpireTime*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetRouterExpireTime
*------------------------------------------------------------------------------
* Purpose: This function set the router expire time
* INPUT  : exp_time  - the expire time
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetRouterExpireTime(
    UI16_T exp_time)
{
    MLDSNP_OM_LOCK();

    mldsnp_om_cfg_db_g.router_exp_time = exp_time;

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetRouterExpireTime*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave status
* INPUT  : vid                       - the vlan id
* OUTPUT : immediate_leave_status_p - the immediate leave status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetImmediateLeaveStatus(
    UI16_T                        vid,
    MLDSNP_TYPE_ImmediateStatus_T *immediate_leave_status_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    *immediate_leave_status_p = vlan_info.immediate_leave_status;

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetImmediateLeaveStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetImmediateLeaveByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave by-host-ip status
* INPUT  : vid                      - the vlan id
*
* OUTPUT : immediate_leave_byhost_status_p - the immediate leave status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetImmediateLeaveByHostStatus(
    UI16_T                        vid,
    MLDSNP_TYPE_ImmediateByHostStatus_T *immediate_leave_byhost_status_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    *immediate_leave_byhost_status_p = vlan_info.immediate_leave_byhost_status;

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetImmediateLeaveByHostStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave status
* INPUT  : *vid_p                   - the vlan id
* OUTPUT : immediate_leave_status_p - the immediate leave status
*          *vid_p                   - the next vid
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextImmediateLeaveStatus(
    UI16_T                        *vid_p,
    MLDSNP_TYPE_ImmediateStatus_T *immediate_leave_status_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;
    MLDSNP_OM_LOCK();
    vlan_info.vid = *vid_p;

    if (FALSE == L_SORT_LST_Get_Next(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    *vid_p = vlan_info.vid;
    *immediate_leave_status_p = vlan_info.immediate_leave_status;

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetNextImmediateLeaveStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextImmediateLeaveByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave by-host-ip status
* INPUT  : *vid_p                   - the vlan id
* OUTPUT : immediate_leave_byhost_status_p - the immediate leave by-host-ip status
*          *vid_p                   - the next vid
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextImmediateLeaveByHostStatus(
    UI16_T                        *vid_p,
    MLDSNP_TYPE_ImmediateByHostStatus_T *immediate_leave_byhost_status_p)
{
    MLDSNP_OM_VlanInfo_T vlan_info;
    MLDSNP_OM_LOCK();
    vlan_info.vid = *vid_p;

    if (FALSE == L_SORT_LST_Get_Next(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    *vid_p = vlan_info.vid;
    *immediate_leave_byhost_status_p = vlan_info.immediate_leave_byhost_status;

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_GetNextImmediateLeaveByHostStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave status
* INPUT  : vid                    - the vlan id
*          immediate_leave_status - the returned router port info
*
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetImmediateLeaveStatus(
    UI16_T                        vid,
    MLDSNP_TYPE_ImmediateStatus_T immediate_leave_status)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    vlan_info.immediate_leave_status = immediate_leave_status;
    vlan_info.immediate_leave_byhost_status = MLDSNP_TYPE_IMMEDIATE_BYHOST_DISABLED;

    if (FALSE == L_SORT_LST_Set(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_SetImmediateLeaveStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetImmediateLeaveByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave by-host-ip status
* INPUT  : vid                           - the vlan id
*          immediate_leave_byhost_status - the immediate leave by-host-ip status
*
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetImmediateLeaveByHostStatus(
    UI16_T                        vid,
    MLDSNP_TYPE_ImmediateByHostStatus_T immediate_leave_byhost_status)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();
    vlan_info.vid = vid;

    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    vlan_info.immediate_leave_status = immediate_leave_byhost_status;
    vlan_info.immediate_leave_byhost_status = immediate_leave_byhost_status;

    if (FALSE == L_SORT_LST_Set(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return FALSE;
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_SetImmediateLeaveByHostStatus*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function Set the unknown multicast data flood behavior
* INPUT  :  flood_behavior - the returned router port info
*          vlan_id         - which vlan
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_SetUnknownFloodBehavior(
    UI32_T vlan_id,
    MLDSNP_TYPE_UnknownBehavior_T flood_behavior)
{
    MLDSNP_OM_LOCK();

    #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    mldsnp_om_cfg_db_g.unknown_flood_behavior = flood_behavior;
    #else
    if(vlan_id == 0|| vlan_id > SYS_ADPT_MAX_VLAN_ID)
    {
      MLDSNP_OM_UNLOCK();
      return FALSE;
    }

    mldsnp_om_cfg_db_g.unknown_flood_behavior[vlan_id-1] = flood_behavior;
    #endif
    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetUnknownFloodBehavior*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function get the unknown multicast data flood behavior
* INPUT  :  flood_behavior_p - the returned router port info
*          vlan_id         - which vlan
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetUnknownFloodBehavior(
    UI32_T vlan_id,
    MLDSNP_TYPE_UnknownBehavior_T *flood_behavior_p)
{
    MLDSNP_OM_LOCK();
    #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    *flood_behavior_p = mldsnp_om_cfg_db_g.unknown_flood_behavior;
    #else

    if(vlan_id == 0|| vlan_id > SYS_ADPT_MAX_VLAN_ID)
    {
      MLDSNP_OM_UNLOCK();
      return FALSE;
    }

    *flood_behavior_p = mldsnp_om_cfg_db_g.unknown_flood_behavior[vlan_id-1];
    #endif
    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_GetUnknownFloodBehavior*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetRunningUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function get the unknown multicast data flood behavior
* INPUT  : flood_behavior_p - the returned router port info
*          vlan_id         - which vlan
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetRunningUnknownFloodBehavior(
    UI32_T vlan_id,
    MLDSNP_TYPE_UnknownBehavior_T *flood_behavior_p)
{
    MLDSNP_OM_LOCK();
    #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    *flood_behavior_p = mldsnp_om_cfg_db_g.unknown_flood_behavior;
    #else
    if(vlan_id == 0|| vlan_id > SYS_ADPT_MAX_VLAN_ID)
    {
      MLDSNP_OM_UNLOCK();
      return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    *flood_behavior_p = mldsnp_om_cfg_db_g.unknown_flood_behavior[vlan_id-1];
    #endif
    MLDSNP_OM_UNLOCK();

    if(*flood_behavior_p == SYS_DFLT_MLDSNP_UNKNOWN_MULTICAST_MOD)
      return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/*End of MLDSNP_OM_GetRunningUnknownFloodBehavior*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_SetS_QRV_QQIC
*------------------------------------------------------------------------------
* Purpose: This function set the querier S flag value
* INPUT  :  flood_behavior_p - the returned router port info
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_SetS_QRV_QQIC(
    UI16_T vid,
    BOOL_T s_on,
    UI16_T robust_value,
    UI16_T query_interval)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_OM_LOCK();

    vlan_info.vid = vid;
    if (FALSE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return;
    }

    vlan_info.s_flag              = s_on;
    vlan_info.robust_oper_value   = robust_value;
    vlan_info.query_oper_interval = query_interval;

    if (FALSE == L_SORT_LST_Set(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        MLDSNP_OM_UNLOCK();
        return;
    }

    MLDSNP_OM_UNLOCK();
    return;
}/*End of MLDSNP_OM_SetS_QRV_QQIC*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetV1HostPresentPortbitmap
*------------------------------------------------------------------------------
* Purpose: This function get the v1 host present portbitmap
* INPUT  :  None
* OUTPUT : portbitmap - the portbitmap array to put the result
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_GetV1HostPresentPortbitmap(
    UI8_T *portbitmap_p)
{
    MLDSNP_OM_LOCK();

    memcpy(portbitmap_p, v1_host_preset_portbitmap, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    MLDSNP_OM_UNLOCK();

    return;
}
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_AddV1HostPresentPort
*------------------------------------------------------------------------------
* Purpose: This function add the v1 host present port
* INPUT  : lport - the port to add to portbitmap
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_AddV1HostPresentPort(
    UI16_T lport)
{
    MLDSNP_OM_LOCK();

    MLDSNP_TYPE_AddPortIntoPortBitMap(lport, v1_host_preset_portbitmap);

    MLDSNP_OM_UNLOCK();

    return;
}/*End of MLDSNP_OM_AddV1HostPresentPort*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteV1HostPresentPort
*------------------------------------------------------------------------------
* Purpose: This function delete the v1 host present port
* INPUT  :  lport - the port to add to portbitmap
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_OM_DeleteV1HostPresentPort(
    UI16_T lport)
{
    MLDSNP_OM_LOCK();

    MLDSNP_TYPE_DeletePortFromPorBitMap(lport, v1_host_preset_portbitmap);

    MLDSNP_OM_UNLOCK();

    return;
}/*End of MLDSNP_OM_DeleteV1HostPresentPort*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function get next static join group entry
* INPUT  : next_id  - the next used index in array
*          out_vid - vlan id
*          out_gip_ap - group array pointer
*          out_sip_ap - source array pointer
*          out_lport   - registered port
*          out_rec_type - exclude or include
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_GetNextStaticPortJoinGroup(
    UI16_T *next_id_p,
    UI16_T *out_vid_p,
    UI8_T *out_gip_ap,
    UI8_T *out_sip_ap,
    UI16_T *out_lport,
    MLDSNP_TYPE_RecordType_T *out_rec_type_p)
{
    UI16_T i = *next_id_p + 1;

    MLDSNP_OM_LOCK();
    for (; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
    {
        if (TRUE == static_join_group_entry[i].is_used)
        {
            *next_id_p      = i;
            *out_vid_p      = static_join_group_entry[i].vid;
            *out_lport      = static_join_group_entry[i].lport;
            *out_rec_type_p = static_join_group_entry[i].list_type;
            memcpy(out_gip_ap, static_join_group_entry[i].gip_a, GROUP_LEN);
            memcpy(out_sip_ap, static_join_group_entry[i].sip_a, SRC_IP_LEN);

            MLDSNP_OM_UNLOCK();
            return TRUE;
        }
    }

    MLDSNP_OM_UNLOCK();
    return FALSE;
}/*End of MLDSNP_OM_GetNextStaticPortJoinGroup*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextRunningStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function get next static join group entry
* INPUT  : next_id  - the next used index in array
*          out_vid - vlan id
*          out_gip_ap - group array pointer
*          out_sip_ap - source array pointer
*          out_lport   - registered port
*          out_rec_type - exclude or include
* OUTPUT : None
* RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- success
*          SYS_TYPE_GET_RUNNING_CFG_FAIL -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_OM_GetNextRunningStaticPortJoinGroup(
    UI16_T *next_id_p,
    UI16_T *out_vid_p,
    UI8_T *out_gip_ap,
    UI8_T *out_sip_ap,
    UI16_T *out_lport,
    MLDSNP_TYPE_RecordType_T *out_rec_type_p)
{
    UI16_T i = *next_id_p + 1;

    MLDSNP_OM_LOCK();
    for (; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
    {
        if (TRUE == static_join_group_entry[i].is_used)
        {
            *next_id_p      = i;
            *out_vid_p      = static_join_group_entry[i].vid;
            *out_lport      = static_join_group_entry[i].lport;
            *out_rec_type_p = static_join_group_entry[i].list_type;
            memcpy(out_gip_ap, static_join_group_entry[i].gip_a, GROUP_LEN);
            memcpy(out_sip_ap, static_join_group_entry[i].sip_a, SRC_IP_LEN);

            MLDSNP_OM_UNLOCK();
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }
    MLDSNP_OM_UNLOCK();
    return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}/*End of MLDSNP_OM_GetNextStaticPortJoinGroup*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetStaticGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the static join group port bit list
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT : *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetStaticGroupPortlist(
    UI16_T                vid,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    UI8_T                 *static_portlist)
{
    UI16_T i = 1;
    BOOL_T found = FALSE;
    memset(static_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    MLDSNP_OM_LOCK();

    for (; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
    {
        if (TRUE == static_join_group_entry[i].is_used
                &&  static_join_group_entry[i].vid == vid
                && (!memcmp(static_join_group_entry[i].gip_a, gip_ap, GROUP_LEN)))
        {
            MLDSNP_TYPE_AddPortIntoPortBitMap(static_join_group_entry[i].lport, static_portlist);
            found = TRUE;
        }
    }

    MLDSNP_OM_UNLOCK();
    return found;
}/*End if MLDSNP_OM_GetGroupPortlist*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_GetNextGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the next static join group port bit list
* INPUT  : vid_p    - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT :  vid_p   - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*          *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_OM_GetNextStaticGroupPortlist(
    UI16_T                *vid_p,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    UI8_T                 *static_portlist)
{
    UI16_T i, tmp_vid = SYS_ADPT_MAX_NBR_OF_VLAN;
    BOOL_T found = FALSE;
    UI8_T   tmp_gip_a[GROUP_LEN];
    memset(tmp_gip_a, 0xff, GROUP_LEN);
    memset(static_portlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    MLDSNP_OM_LOCK();

    /* find if there are other groups in the same vlan */
    for (i = 1; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
    {
        if (TRUE == static_join_group_entry[i].is_used
            &&  static_join_group_entry[i].vid == *vid_p)
        {
            if (memcmp(static_join_group_entry[i].gip_a, tmp_gip_a, GROUP_LEN)<0
            && memcmp(static_join_group_entry[i].gip_a, gip_ap, GROUP_LEN)>0)
            {
                found = TRUE;
                tmp_vid = static_join_group_entry[i].vid;
                memcpy(tmp_gip_a, static_join_group_entry[i].gip_a, GROUP_LEN);
            }
        }
    }

    /* if there are no more group in the same vlan, find the next vlan.
         reset gip so that we can start a new search in the new vlan. */
    if (found == FALSE)
    {
        memset(gip_ap, 0, GROUP_LEN);
        for (i = 1; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
        {
            /* find the next vlan */
            if (TRUE == static_join_group_entry[i].is_used
                &&  static_join_group_entry[i].vid < tmp_vid
                &&  static_join_group_entry[i].vid > *vid_p)
            {
                found = TRUE;
                tmp_vid = static_join_group_entry[i].vid;
                memcpy(tmp_gip_a, static_join_group_entry[i].gip_a, GROUP_LEN);
            }
            /* find if the smallest group in this vlan */
            else if (TRUE == static_join_group_entry[i].is_used
                        &&  static_join_group_entry[i].vid == tmp_vid)
            {
                if (memcmp(static_join_group_entry[i].gip_a, tmp_gip_a, GROUP_LEN)<0
                      && memcmp(static_join_group_entry[i].gip_a, gip_ap, GROUP_LEN)>0)
                {
                    found = TRUE;
                    memcpy(tmp_gip_a, static_join_group_entry[i].gip_a, GROUP_LEN);
                }
            }
        }
    }

    /* after find the group entry, get the portlist */
    if (found)
    {
        for (i = 1; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
        {
            if (TRUE == static_join_group_entry[i].is_used
                    &&  static_join_group_entry[i].vid == tmp_vid
                    && (!memcmp(static_join_group_entry[i].gip_a, tmp_gip_a, GROUP_LEN)))
            {
                MLDSNP_TYPE_AddPortIntoPortBitMap(static_join_group_entry[i].lport, static_portlist);
            }
        }

        *vid_p = tmp_vid;
        memcpy(gip_ap, tmp_gip_a, GROUP_LEN);
    }

    MLDSNP_OM_UNLOCK();
    return found;

}/*End if MLDSNP_OM_GetNextGroupPortlist*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_AddStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function add the static join group entry
* INPUT  : vid - vlan id
*          gip_ap - group array pointer
*          sip_ap - source array pointer
*          lport   - registered port
*          rec_type - exclude or include
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_AddStaticPortJoinGroup(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    UI16_T lport,
    MLDSNP_TYPE_RecordType_T rec_type)
{
    UI16_T i = 1; /*add from firt*/

    MLDSNP_OM_LOCK();

    for (; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
    {
        if (FALSE == static_join_group_entry[i].is_used)
        {
            static_join_group_entry[i].vid       = vid;
            static_join_group_entry[i].lport     = lport;
            static_join_group_entry[i].list_type = rec_type;
            memcpy(static_join_group_entry[i].gip_a, gip_ap, GROUP_LEN);
            memcpy(static_join_group_entry[i].sip_a, sip_ap, SRC_IP_LEN);
            static_join_group_entry[i].is_used   = TRUE;

            MLDSNP_OM_UNLOCK();
            return TRUE;
        }
    }

    MLDSNP_OM_UNLOCK();

    return FALSE;
}/*End of MLDSNP_OM_AddStaticPortJoinGroup*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_IsExistStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function check the group is already joined before
* INPUT  : vid - vlan id
*          gip_ap - group array pointer
*          sip_ap - source array pointer
*          lport   - registered port
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_IsExistStaticPortJoinGroup(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    UI16_T lport)
{
    UI16_T i = 1; /*add from firt*/

    MLDSNP_OM_LOCK();

    for (; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
    {
        if (TRUE == static_join_group_entry[i].is_used
                &&  static_join_group_entry[i].vid == vid
                && (!memcmp(static_join_group_entry[i].gip_a, gip_ap, GROUP_LEN))
                && (!memcmp(static_join_group_entry[i].sip_a, sip_ap, SRC_IP_LEN))
                && static_join_group_entry[i].lport == lport)
        {
            MLDSNP_OM_UNLOCK();
            return TRUE;
        }
    }

    MLDSNP_OM_UNLOCK();
    return FALSE;
}/*End of MLDSNP_OM_AddStaticPortJoinGroup*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function add the static join group entry
* INPUT  : vid - vlan id
*          gip_ap - group array pointer
*          sip_ap - source array pointer
*          lport   - registered port
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteStaticPortJoinGroup(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    UI16_T lport)
{
    UI16_T i = 1;

    MLDSNP_OM_LOCK();
    for (; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
    {
        if (TRUE == static_join_group_entry[i].is_used
                &&  static_join_group_entry[i].vid == vid
                && (!memcmp(static_join_group_entry[i].gip_a, gip_ap, GROUP_LEN))
                && (!memcmp(static_join_group_entry[i].sip_a, sip_ap, SRC_IP_LEN))
                && static_join_group_entry[i].lport == lport)
        {
            memset(&static_join_group_entry[i], 0, sizeof(MLDSNP_OM_StaticPortJoin_T));
            static_join_group_entry[i].is_used = FALSE;
            break;
        }
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}/*End of MLDSNP_OM_DeleteStaticPortJoinGroup*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_ReplaceStaticJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function replace the static join group between port and port-channel
* INPUT  : from_ifindex - the port to be replaced
*          to_ifindex - replace by which port
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_ReplaceStaticJoinGroup(
    UI16_T id,
    UI32_T  from_ifindex,
    UI32_T  to_ifindex )
{
     MLDSNP_OM_LOCK();

     if (TRUE == static_join_group_entry[id].is_used
         && static_join_group_entry[id].lport == from_ifindex)
    {
        static_join_group_entry[id].lport = to_ifindex;
        MLDSNP_OM_UNLOCK();
        return TRUE;
    }

    MLDSNP_OM_UNLOCK();    
    return FALSE;

}/*End of MLDSNP_OM_AddStaticPortJoinGroup*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteAllStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function delete all the static group joined on that port
* INPUT  : lport   - registered port
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteAllStaticPortJoinGroup(
    UI16_T lport)
{
    UI16_T i = 1;

    MLDSNP_OM_LOCK();
    for (; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
    {
        if (TRUE == static_join_group_entry[i].is_used
                && static_join_group_entry[i].lport == lport)
        {
            memset(&static_join_group_entry[i], 0, sizeof(MLDSNP_OM_StaticPortJoin_T));
            static_join_group_entry[i].is_used = FALSE;
        }
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteAllStaticPortJoinGroupInVlan
*------------------------------------------------------------------------------
* Purpose: This function delete all the static group joined on that port in that vlan
* INPUT  : vid - vlan id
*          lport   - registered port
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteAllStaticPortJoinGroupInVlan(
    UI16_T vid,
    UI16_T lport)
{
    UI16_T i = 1;

    MLDSNP_OM_LOCK();
    for (; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
    {
        if (TRUE == static_join_group_entry[i].is_used
                &&  static_join_group_entry[i].vid == vid
                && static_join_group_entry[i].lport == lport)
        {
            memset(&static_join_group_entry[i], 0, sizeof(MLDSNP_OM_StaticPortJoin_T));
            static_join_group_entry[i].is_used = FALSE;
        }
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_OM_DeleteAllVlanStaticPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function delete all the static group joined in that vlan
* INPUT  : vid - vlan id
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_OM_DeleteAllStaticJoinGroupInVlan(
    UI16_T vid)
{
    UI16_T i = 1;

    MLDSNP_OM_LOCK();
    for (; i <= SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP; i++)
    {
        if (TRUE == static_join_group_entry[i].is_used
                &&  static_join_group_entry[i].vid == vid)
        {
            memset(&static_join_group_entry[i], 0, sizeof(MLDSNP_OM_StaticPortJoin_T));
            static_join_group_entry[i].is_used = FALSE;
        }
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}

#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set query guard status
 * INPUT   : status  - the enabled or diabled  status
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetQueryDropStatus(UI32_T lport, BOOL_T status)
{
    MLDSNP_OM_LOCK();

    if (status)
        MLDSNP_TYPE_SET_FLAG(mldsnp_om_cfg_db_g.conf[lport-1], MLDSNP_OM_QUERY_DROP_ENABLED);
    else
        MLDSNP_TYPE_UNSET_FLAG(mldsnp_om_cfg_db_g.conf[lport-1], MLDSNP_OM_QUERY_DROP_ENABLED);
    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetQueryDropStatus*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get the query guard status
 * INPUT   : None
 * OUTPUT  : status  - the enabled or diabled status
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetQueryDropStatus(UI32_T lport, BOOL_T  *status)
{
    if (lport == 0 || lport > SYS_ADPT_TOTAL_NBR_OF_LPORT || status == NULL)
        return FALSE;

    MLDSNP_OM_LOCK();

    *status = MLDSNP_TYPE_CHECK_FLAG(mldsnp_om_cfg_db_g.conf[lport-1], MLDSNP_OM_QUERY_DROP_ENABLED) ? TRUE : FALSE;

    MLDSNP_OM_UNLOCK();

    return TRUE;

}/*End of MLDSNP_OM_GetQueryDropStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get the query guard status
 * INPUT   : None
 * OUTPUT  :
 * RETURN  : TRUE  - query guard enble
 *           FALSE - query guard disable
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsQueryDropEnable(UI32_T lport)
{
    BOOL_T ret;

    if (lport == 0 || lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        return FALSE;

    MLDSNP_OM_LOCK();

    ret = MLDSNP_TYPE_CHECK_FLAG(mldsnp_om_cfg_db_g.conf[lport-1], MLDSNP_OM_QUERY_DROP_ENABLED);
    MLDSNP_OM_UNLOCK();

    return ret;
}/*End of MLDSNP_OM_GetQueryDropStatus*/

#endif


#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set multicast data drop status
 * INPUT   : status  - the enabled or diabled  status
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetMulticastDataDropStatus(UI32_T lport, BOOL_T status)
{
    MLDSNP_OM_LOCK();

    if (status)
        MLDSNP_TYPE_SET_FLAG(mldsnp_om_cfg_db_g.conf[lport-1], MLDSNP_OM_MULTICAST_DATA_DROP_ENABLED);
    else
        MLDSNP_TYPE_UNSET_FLAG(mldsnp_om_cfg_db_g.conf[lport-1], MLDSNP_OM_MULTICAST_DATA_DROP_ENABLED);

    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetMulticastDataDropStatus*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get the multicast data drop status
 * INPUT   : None
 * OUTPUT  : status  - the enabled or diabled status
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetMulticastDataDropStatus(UI32_T lport, BOOL_T  *status)
{
    if (lport == 0 || lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        return FALSE;

    MLDSNP_OM_LOCK();

    *status = MLDSNP_TYPE_CHECK_FLAG(mldsnp_om_cfg_db_g.conf[lport-1], MLDSNP_OM_MULTICAST_DATA_DROP_ENABLED) ? TRUE : FALSE;
    MLDSNP_OM_UNLOCK();

    return TRUE;

}/*End of MLDSNP_OM_GetMulticastDataDropStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get the multicast data drop status
 * INPUT   : None
 * OUTPUT  : status  - the enabled or diabled status
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsMulticastDataDropEnable(UI32_T lport)
{
    BOOL_T ret;
    if (lport == 0 || lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        return FALSE;

    MLDSNP_OM_LOCK();

    ret = MLDSNP_TYPE_CHECK_FLAG(mldsnp_om_cfg_db_g.conf[lport-1], MLDSNP_OM_MULTICAST_DATA_DROP_ENABLED);
    MLDSNP_OM_UNLOCK();

    return ret;
}/*End of MLDSNP_OM_GetMulticastDataDropStatus*/
#endif

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE )
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetPortMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : ifindex       - which port to get
 *           limit_per_sec - per second limit value
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetPortMldReportLimitPerSec(UI32_T lport, UI16_T limit_per_sec)
{
    MLDSNP_OM_LOCK();
    mldsnp_om_cfg_db_g.port_mld_pkt_lmt_conf[lport-1] = limit_per_sec;
    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetPortMldReportLimitPerSec*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetPortMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : ifindex       - which port or vlan to get
 * OUTPUT  : limit_per_sec - per second limit value
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI16_T MLDSNP_OM_GetPortMldReportLimitPerSec(UI32_T lport)
{
    UI16_T ret = 0;

    if (lport < 1 || lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        return ret;

    MLDSNP_OM_LOCK();
    ret = mldsnp_om_cfg_db_g.port_mld_pkt_lmt_conf[lport-1];;
    MLDSNP_OM_UNLOCK();

    return ret;
}/*End of MLDSNP_OM_GetPortMldReportLimitPerSec*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_IsPortMldReportRcvdReachLimit
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to count mld report receceived value per second
 * INPUT   : ifindex       - which port to get
 *           limit_per_sec - per second limit value
 * OUTPUT  : None
 * RETURN  : TRUE  - reach limit
 *           FALSE - not reach limit
 * NOTE    : when call this function, it will add one.
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsPortMldReportRcvdReachLimit(UI32_T lport)
{
    UI16_T ret = 0;

    if (lport < 1 || lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        return ret;

    MLDSNP_OM_LOCK();

    if (mldsnp_om_cfg_db_g.port_mld_pkt_lmt_oper[lport-1]
            >= mldsnp_om_cfg_db_g.port_mld_pkt_lmt_conf[lport-1])
    {
        MLDSNP_OM_UNLOCK();
        return TRUE;
    }

    mldsnp_om_cfg_db_g.port_mld_pkt_lmt_oper[lport-1]++;
    MLDSNP_OM_UNLOCK();
    return FALSE;
}/*End of MLDSNP_OM_IsPortMldReportRcvdReachLimit*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_ResetPortMldReportPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to reste mld report limit value per second
 * INPUT   :  None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
void MLDSNP_OM_ResetPortMldReportPerSec()
{
    MLDSNP_OM_LOCK();
    memset(mldsnp_om_cfg_db_g.port_mld_pkt_lmt_oper, 0, sizeof(mldsnp_om_cfg_db_g.port_mld_pkt_lmt_oper));
    MLDSNP_OM_UNLOCK();

    return;
}/*End of MLDSNP_OM_ResetPortMldReportPerSec*/
#endif

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE )
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetVlanMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : vid       - which vlan to get
 *           limit_per_sec - per second limit value
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetVlanMldReportLimitPerSec(UI32_T vid, UI16_T limit_per_sec)
{
    MLDSNP_OM_LOCK();
    mldsnp_om_cfg_db_g.vlan_mld_pkt_lmt_conf[vid-1] = limit_per_sec;
    MLDSNP_OM_UNLOCK();

    return TRUE;
}/*End of MLDSNP_OM_SetVlanMldReportLimitPerSec*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetVlangmpReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : vid       - which vlan to get
 * OUTPUT  :
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI16_T MLDSNP_OM_GetVlanMldReportLimitPerSec(UI32_T vid)
{
    UI16_T ret = 0;

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
        return ret;

    MLDSNP_OM_LOCK();
    ret = mldsnp_om_cfg_db_g.vlan_mld_pkt_lmt_conf[vid-1];
    MLDSNP_OM_UNLOCK();

    return ret;
}/*End of MLDSNP_OM_GetVlangmpReportLimitPerSec*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_IsPortMldReportRcvdReachLimit
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to count mld report receceived value per second
 * INPUT   : vid       - which vlan to test
 * OUTPUT  : None
 * RETURN  : TRUE  - reach limit
 *           FALSE - not reach limit
 * NOTE    : when call this function, it will add one.
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsVlanMldReportRcvdReachLimit(UI32_T vid)
{
    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
        return FALSE;

    MLDSNP_OM_LOCK();

    if (mldsnp_om_cfg_db_g.vlan_mld_pkt_lmt_oper[vid-1]
            >= mldsnp_om_cfg_db_g.vlan_mld_pkt_lmt_conf[vid-1])
    {
        MLDSNP_OM_UNLOCK();
        return TRUE;
    }

    mldsnp_om_cfg_db_g.vlan_mld_pkt_lmt_oper[vid-1]++;

    MLDSNP_OM_UNLOCK();
    return FALSE;
}/*End of MLDSNP_OM_IsPortMldReportRcvdReachLimit*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_ResetPortMldReportPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to reste mld report limit value per second
 * INPUT   :
 * OUTPUT  :
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
void MLDSNP_OM_ResetVlanMldReportPerSec()
{
    MLDSNP_OM_LOCK();
    memset(mldsnp_om_cfg_db_g.vlan_mld_pkt_lmt_oper, 0, sizeof(mldsnp_om_cfg_db_g.vlan_mld_pkt_lmt_oper));
    MLDSNP_OM_UNLOCK();
    return;
}/*End of MLDSNP_OM_ResetPortMldReportPerSec*/
#endif /*End of #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)*/

#if ( SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetMldFilterStatus
 *-------------------------------------------------------------------------
 * PURPOSE : Set Filter statutus
 * INPUT   : mldsnp_filter_status - status
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetMldFilterStatus(UI32_T mldsnp_filter_status)
{
    MLDSNP_OM_LOCK();
    mld_filter_db.filter_status = mldsnp_filter_status;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_IsMldProfileExist
 *-------------------------------------------------------------------------
 * PURPOSE : Check this profile id is exist
 * INPUT   : pid  - profile id
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsMldProfileExist(UI32_T pid)
{
    MLDSNP_OM_ProfileInfo_T *Profile_p;

    MLDSNP_OM_LOCK();

    Profile_p = mld_filter_db.profile_entry_p;

    while (Profile_p != NULL)
    {
        if (Profile_p->pid == pid)
        {/* found */
            MLDSNP_OM_UNLOCK();
            return TRUE;
        }

        if (Profile_p->pid > pid)
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }

        Profile_p = Profile_p->nextPtr;
    }

    MLDSNP_OM_UNLOCK();
    return FALSE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_CreateMldProfileEntry
 *-------------------------------------------------------------------------
 * PURPOSE : Create new profile
 * INPUT   : pid  - profile id
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    : it won't check this pid is exist or not
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_CreateMldProfileEntry(UI32_T pid)
{
    MLDSNP_OM_ProfileInfo_T    *profile_p, *pre_profile_p;
    MLDSNP_OM_ProfileInfo_T    *free_profile_p;

    /* allocate free profile entry */
    if (free_profile_ptr == NULL)
    {
        return FALSE;
    }

    MLDSNP_OM_LOCK();
    free_profile_p = free_profile_ptr;
    free_profile_ptr = free_profile_p->nextPtr;

    profile_p = mld_filter_db.profile_entry_p;
    pre_profile_p = mld_filter_db.profile_entry_p;

    while (profile_p != NULL)
    {
        if (profile_p->pid > pid)
        {/* found */
            break;
        }

        pre_profile_p = profile_p;
        profile_p = profile_p->nextPtr;
    }

    if (pre_profile_p == profile_p)
    {   /* if same, mean the found entry is head */
        mld_filter_db.profile_entry_p = free_profile_p;
    }
    else
    {
        pre_profile_p->nextPtr = free_profile_p;
    }

    free_profile_p->nextPtr = profile_p;
    free_profile_p->pid = pid;
    free_profile_p->access_mode = SYS_DFLT_MLD_PROFILE_ACCESS_MODE;
    free_profile_p->group_entry_p = NULL;
    free_profile_p->total_group_ranges = 0;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_RemoveMldProfileFromAllPort
 *-------------------------------------------------------------------------
 * PURPOSE : unbind all port from pid
 * INPUT   : pid  - which pid to unbind
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_RemoveMldProfileFromAllPort(UI32_T pid)
{
    UI32_T lport;

    MLDSNP_OM_LOCK();
    for (lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        if (mld_port_profile_id[lport - 1] == pid)
        {
            mld_port_profile_id[lport - 1] = SYS_DFLT_MLD_PROFILE_ID_NULL;
        }
    }
    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_DestroyMldProfileEntry
 *-------------------------------------------------------------------------
 * PURPOSE : Destroy profile entry
 * INPUT   : pid  - which pid to destroy
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_DestroyMldProfileEntry(UI32_T pid)
{
    MLDSNP_OM_ProfileInfo_T        *profile_p, *pre_profile_p;
    MLDSNP_OM_ProfileGroupEntry_T  *profile_group_p;

    MLDSNP_OM_LOCK();
    profile_p = mld_filter_db.profile_entry_p;
    pre_profile_p = mld_filter_db.profile_entry_p;

    while (profile_p != NULL)
    {
        if (profile_p->pid == pid)
        {/* found */
            break;
        }

        pre_profile_p = profile_p;
        profile_p = profile_p->nextPtr;
    }

    if (profile_p == pre_profile_p)
    {   /* if same, mean in head */
        mld_filter_db.profile_entry_p = profile_p->nextPtr;
    }
    else
    {
        pre_profile_p->nextPtr = profile_p->nextPtr;
    }

    profile_group_p = profile_p->group_entry_p;

    if (profile_group_p != NULL)
    {
        mld_filter_db.total_entry --;

        while (profile_group_p->next != NULL)
        {
            mld_filter_db.total_entry --;
            profile_group_p = profile_group_p->next;
        }

        profile_group_p->next = free_profile_group_ptr;
        free_profile_group_ptr = profile_p->group_entry_p;
        profile_p->group_entry_p = NULL;
    }

    profile_p->nextPtr = free_profile_ptr;
    free_profile_ptr = profile_p;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetMldProfileAccessMode
 *-------------------------------------------------------------------------
 * PURPOSE : Set profile access mode
 * INPUT   : pid         - which pid to set
 *           access_mode - set which mode
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetMldProfileAccessMode(UI32_T pid, UI32_T access_mode)
{
    MLDSNP_OM_ProfileInfo_T        *Profile_p;

    MLDSNP_OM_LOCK();
    Profile_p = mld_filter_db.profile_entry_p;

    while (Profile_p != NULL)
    {
        if (Profile_p->pid == pid)
        {
            Profile_p->access_mode = access_mode;
            MLDSNP_OM_UNLOCK();
            return TRUE;
        }
        Profile_p = Profile_p->nextPtr;
    }

    MLDSNP_OM_UNLOCK();
    return FALSE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_AddMldProfileGroup
 *-------------------------------------------------------------------------
 * PURPOSE : Add a group range to a profile
 * INPUT   : pid       - which pid to add
 *           start_mip - start ip address
 *           end_mip   - end ip address
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_AddMldProfileGroup(UI32_T pid, UI8_T *start_mip, UI8_T *end_mip)
{
    MLDSNP_OM_ProfileInfo_T        *profile_p;
    MLDSNP_OM_ProfileGroupEntry_T  *profile_group_p, *pre_profile_group_p;
    MLDSNP_OM_ProfileGroupEntry_T  *free_profile_group_p;

    profile_p = mld_filter_db.profile_entry_p;

    if (free_profile_group_ptr == NULL
            || mld_filter_db.total_entry >= SYS_ADPT_MLD_TOTAL_PROFILE_GROUPS)
    {
        return FALSE;
    }

    MLDSNP_OM_LOCK();

    while (profile_p != NULL)
    {
        if (profile_p->pid == pid)
        {
            profile_group_p = pre_profile_group_p = profile_p->group_entry_p;

            while (profile_group_p != NULL)
            {
                if (0 == memcmp(profile_group_p->start_mip, start_mip, SYS_ADPT_IPV6_ADDR_LEN)
                    && 0 == memcmp(profile_group_p->end_mip, end_mip, SYS_ADPT_IPV6_ADDR_LEN))
                {
                    MLDSNP_OM_UNLOCK();   
                    return TRUE; /*duplicat group range*/
                }

                if (memcmp(start_mip, profile_group_p->start_mip, SYS_ADPT_IPV6_ADDR_LEN) < 0)
                    break;
                else if (memcmp(start_mip, profile_group_p->start_mip, SYS_ADPT_IPV6_ADDR_LEN) == 0
                         && memcmp(end_mip, profile_group_p->end_mip, SYS_ADPT_IPV6_ADDR_LEN) < 0)
                    break;

                pre_profile_group_p = profile_group_p;
                profile_group_p = profile_group_p->next;
            }

            if (profile_p->total_group_ranges >= SYS_ADPT_MLD_EACH_PROFILE_MAX_SUPPORT_GROUPS)
            {
                MLDSNP_OM_UNLOCK();   
                return FALSE;
            }

            free_profile_group_p    = free_profile_group_ptr;
            free_profile_group_ptr  = free_profile_group_p->next;
            free_profile_group_p->next = NULL;

            if (pre_profile_group_p == profile_group_p)
            {
                if (pre_profile_group_p != NULL)
                    free_profile_group_p->next = profile_p->group_entry_p;
                profile_p->group_entry_p = free_profile_group_p;
                memcpy(free_profile_group_p->start_mip, start_mip, SYS_ADPT_IPV6_ADDR_LEN);
                memcpy(free_profile_group_p->end_mip, end_mip, SYS_ADPT_IPV6_ADDR_LEN);
            }
            else
            {
                free_profile_group_p->next = pre_profile_group_p->next;
                pre_profile_group_p->next  = free_profile_group_p;
                memcpy(free_profile_group_p->start_mip, start_mip, SYS_ADPT_IPV6_ADDR_LEN);
                memcpy(free_profile_group_p->end_mip, end_mip, SYS_ADPT_IPV6_ADDR_LEN);
            }

            profile_p->total_group_ranges ++;
            mld_filter_db.total_entry ++;
            MLDSNP_OM_UNLOCK();
            return TRUE;
        }/* if (profile_p->pid == pid) */

        if (profile_p->pid > pid)
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }

        profile_p = profile_p->nextPtr;
    }/* while (profile_p != NULL) */

    MLDSNP_OM_UNLOCK();
    return FALSE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_DeleteMldProfileGroup
 *-------------------------------------------------------------------------
 * PURPOSE : Delete mld range from a group
 * INPUT   : pid  - which pid to delete group range
 *           start_mip -  *           start_mip - start ip address
 *           end_mip   - end ip address
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    : the start_mip and end_mip shall match
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_DeleteMldProfileGroup(UI32_T pid, UI8_T *start_mip, UI8_T *end_mip)
{
    MLDSNP_OM_ProfileInfo_T        *profile_p;
    MLDSNP_OM_ProfileGroupEntry_T  *profile_group_p, *pre_profile_group_p;

    MLDSNP_OM_LOCK();
    profile_p = mld_filter_db.profile_entry_p;

    while (profile_p != NULL)
    {
        if (profile_p->pid == pid)
        {
            pre_profile_group_p = profile_group_p = profile_p->group_entry_p;

            if (profile_group_p == NULL)
            {
                MLDSNP_OM_UNLOCK();
                return TRUE;
            }
            while (profile_group_p != NULL)
            {
                if (0 == memcmp(start_mip, profile_group_p->start_mip, SYS_ADPT_IPV6_ADDR_LEN)
                     && 0 == memcmp(end_mip, profile_group_p->end_mip, SYS_ADPT_IPV6_ADDR_LEN))
                {
                    if (pre_profile_group_p != profile_group_p)
                    {
                        pre_profile_group_p->next = profile_group_p->next;
                        profile_group_p->next = free_profile_group_ptr;
                        free_profile_group_ptr = profile_group_p;
                        profile_p->total_group_ranges --;
                        mld_filter_db.total_entry --;
                    }
                    else
                    {
                        profile_p->group_entry_p = profile_group_p->next;
                        profile_group_p->next = free_profile_group_ptr;
                        free_profile_group_ptr = profile_group_p;
                        profile_p->total_group_ranges --;
                        mld_filter_db.total_entry --;
                    }
                    MLDSNP_OM_UNLOCK();
                    return TRUE;
                }

                /*profile_group_p->start_mip is a sorted list, so over it means it can't find*/
                if (memcmp(start_mip, profile_group_p->start_mip, SYS_ADPT_IPV6_ADDR_LEN) < 0)
                {
                    MLDSNP_OM_UNLOCK();
                    return TRUE;
                }
                pre_profile_group_p = profile_group_p;
                profile_group_p = profile_group_p->next;
            }
            MLDSNP_OM_UNLOCK();    
            return TRUE;
        }/* if (profile_p->pid == pid) */

        if (profile_p->pid > pid)
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }

        profile_p = profile_p->nextPtr;

    }/* while (profile_p != NULL) */
    MLDSNP_OM_UNLOCK();
    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_AddMldProfileToPort
 *-------------------------------------------------------------------------
 * PURPOSE : bind a profile to a port
 * INPUT   : lport  - which port to bind
 *           pid    - which pid
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_AddMldProfileToPort(UI32_T lport, UI32_T pid)
{
    if (lport == 0 || lport > SYS_ADPT_MLD_PROFILE_TOTAL_NBR)
        return FALSE;

    MLDSNP_OM_LOCK();
    mld_port_profile_id[lport - 1] = pid;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_RemoveMldProfileFromPort
 *-------------------------------------------------------------------------
 * PURPOSE : Unbind profiles from a port
 * INPUT   : lport - which port to unbind
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_RemoveMldProfileFromPort(UI32_T lport)
{
    if (lport == 0 || lport > SYS_ADPT_MLD_PROFILE_TOTAL_NBR)
        return FALSE;

    MLDSNP_OM_LOCK();
    mld_port_profile_id[lport - 1] = SYS_DFLT_MLD_PROFILE_ID_NULL;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMldThrottlingInfo
 *-------------------------------------------------------------------------
 * PURPOSE : Get throttle information
 * INPUT   : lport   - which port to get
 * OUTPUT  : entry_p - throtle information
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetMldThrottlingInfo(UI32_T lport, MLDSNP_OM_Throttle_T *entry_p)
{
    if (lport == 0 || lport > SYS_ADPT_MLD_PROFILE_TOTAL_NBR || entry_p == NULL)
        return FALSE;

    MLDSNP_OM_LOCK();
    entry_p->throttle_status     = mld_throttle_db[lport - 1].throttle_status;
    entry_p->action              = mld_throttle_db[lport - 1].action;
    entry_p->max_group_number    = mld_throttle_db[lport - 1].max_group_number;
    entry_p->current_group_count = mld_throttle_db[lport - 1].current_group_count;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetPortMldProfileID
 *-------------------------------------------------------------------------
 * PURPOSE : Get port bind to which profle
 * INPUT   : lport - which port to get
 * OUTPUT  : pid_p - which pid
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetPortMldProfileID(UI32_T lport, UI32_T *pid_p)
{
    if (lport == 0 ||  lport > SYS_ADPT_MLD_PROFILE_TOTAL_NBR || pid_p == NULL)
        return FALSE;

    MLDSNP_OM_LOCK();
    *pid_p = mld_port_profile_id[lport - 1];
    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetNextMldProfileGroupbyPid
 *-------------------------------------------------------------------------
 * PURPOSE : Get next profile group range by pid
 * INPUT   : pid   - which pid to get
 * OUTPUT  : start_mip_p - start ip address
 *           end_mip_p   - end ip address
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetNextMldProfileGroupbyPid(UI32_T pid, UI8_T *start_mip_p, UI8_T *end_mip_p)
{
    MLDSNP_OM_ProfileInfo_T        *profile_p;
    MLDSNP_OM_ProfileGroupEntry_T  *profile_group_p;

    MLDSNP_OM_LOCK();
    profile_p = mld_filter_db.profile_entry_p;

    while (profile_p != NULL)
    {
        if (profile_p->pid == pid)
        {
            profile_group_p = profile_p->group_entry_p;
            while (profile_group_p != NULL)
            {
                if (memcmp(start_mip_p, profile_group_p->start_mip, SYS_ADPT_IPV6_ADDR_LEN) < 0)
                {
                    memcpy(start_mip_p, profile_group_p->start_mip, SYS_ADPT_IPV6_ADDR_LEN);
                    memcpy(end_mip_p, profile_group_p->end_mip, SYS_ADPT_IPV6_ADDR_LEN);
                    MLDSNP_OM_UNLOCK();
                    return TRUE;
                }
                else if (memcmp(start_mip_p, profile_group_p->start_mip, SYS_ADPT_IPV6_ADDR_LEN) == 0
                         && memcmp(end_mip_p, profile_group_p->end_mip, SYS_ADPT_IPV6_ADDR_LEN) < 0)
                {
                    memcpy(start_mip_p, profile_group_p->start_mip, SYS_ADPT_IPV6_ADDR_LEN);
                    memcpy(end_mip_p, profile_group_p->end_mip, SYS_ADPT_IPV6_ADDR_LEN);
                    MLDSNP_OM_UNLOCK();
                    return TRUE;
                }

                profile_group_p = profile_group_p->next;
            }
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }
        if (profile_p->pid > pid)
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }
        profile_p = profile_p->nextPtr;
    }
    MLDSNP_OM_UNLOCK();
    return FALSE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetMldThrottlingActionToPort
 *-------------------------------------------------------------------------
 * PURPOSE : Set throttle action to a port
 * INPUT   : lport - which port to set action
 *           acction_mode - which action
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetMldThrottlingActionToPort(UI32_T lport, UI32_T action_mode)
{
    if (lport == 0 || lport > SYS_ADPT_MLD_PROFILE_TOTAL_NBR)
        return FALSE;

    MLDSNP_OM_LOCK();
    mld_throttle_db[lport - 1].action = action_mode;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMldProfileAccessMode
 *-------------------------------------------------------------------------
 * PURPOSE : Get profile access mode
 * INPUT   : pid          - which pid to get
 * OUTPUT  : *access_mode - which access mode
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetMldProfileAccessMode(UI32_T pid, UI32_T *access_mode)
{
    MLDSNP_OM_ProfileInfo_T        *profile_p;

    MLDSNP_OM_LOCK();
    profile_p = mld_filter_db.profile_entry_p;

    while (profile_p != NULL)
    {
        if (profile_p->pid == pid)
        {
            *access_mode = profile_p->access_mode;
            MLDSNP_OM_UNLOCK();
            return TRUE;
        }
        profile_p = profile_p->nextPtr;
    }

    MLDSNP_OM_UNLOCK();
    return FALSE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMldFilterStatus
 *-------------------------------------------------------------------------
 * PURPOSE : Get filter status
 * INPUT   : None
 * OUTPUT  : *mldsnp_filter_status
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetMldFilterStatus(UI32_T *mldsnp_filter_status)
{
    MLDSNP_OM_LOCK();
    *mldsnp_filter_status = mld_filter_db.filter_status;
    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_SetMldThrottlingNumberToPort
 *-------------------------------------------------------------------------
 * PURPOSE : Set port throttle number
 * INPUT   : lport - which port to set
 *           throttling_number - throttle number
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_SetMldThrottlingNumberToPort(UI32_T lport, UI32_T throttling_number)
{
    if (lport == 0 || lport > SYS_ADPT_MLD_PROFILE_TOTAL_NBR)
        return FALSE;

    MLDSNP_OM_LOCK();
    mld_throttle_db[lport - 1].max_group_number = throttling_number;

    if (throttling_number == SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY)
    {
        mld_throttle_db[lport - 1].throttle_status = VAL_mldSnoopThrottlePortRunningStatus_false;
    }
    else
    {
        mld_throttle_db[lport - 1].throttle_status = VAL_mldSnoopThrottlePortRunningStatus_true;
    }
    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetNextMldProfileID
 *-------------------------------------------------------------------------
 * PURPOSE : Get next mld profile id which already created
 * INPUT   : *pid_p - current profile id
 * OUTPUT  : *pid_p - next profile id
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetNextMldProfileID(UI32_T *pid_p)
{
    MLDSNP_OM_ProfileInfo_T        *profile_p;

    MLDSNP_OM_LOCK();
    profile_p = mld_filter_db.profile_entry_p;

    while (profile_p != NULL)
    {
        if (profile_p->pid > *pid_p)
        {
            /* This is sorted link-list. When encounter the first one
               that larger than input one. It is the one
            */
            *pid_p = profile_p->pid;
            MLDSNP_OM_UNLOCK();
            return TRUE;
        }

        profile_p = profile_p->nextPtr;
    }

    MLDSNP_OM_UNLOCK();
    return FALSE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetNextMldProfile
 *-------------------------------------------------------------------------
 * PURPOSE : Get next profile information
 * INPUT   : *pid_p - current profile id
 * OUTPUT  : profile info - next profile information
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    : only for backdoor use
 *-------------------------------------------------------------------------
 */
MLDSNP_OM_ProfileInfo_T * MLDSNP_OM_GetNextMldProfile(UI32_T *pid_p)
{
    MLDSNP_OM_ProfileInfo_T        *profile_p;

    profile_p = mld_filter_db.profile_entry_p;

    while (profile_p != NULL)
    {
        if (profile_p->pid > *pid_p)
        {
            /* This is sorted link-list. When encounter the first one
               that larger than input one. It is the one
            */
            *pid_p = profile_p->pid;
            return profile_p;
        }

        profile_p = profile_p->nextPtr;
    }

    return NULL;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetTotalProfileGroupRangeCount
 *-------------------------------------------------------------------------
 * PURPOSE : Get total profile group's range count
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    : for backdoor use
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_OM_GetTotalProfileGroupRangeCount()
{
    return mld_filter_db.total_entry;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_IsProfileGroup
 *-------------------------------------------------------------------------
 * PURPOSE : Check this group is in this profile configured group range
 * INPUT   : pid    - which profile id
 *           *gip_ap- group address
 * OUTPUT  : *has_group - has this group or not
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_IsProfileGroup(UI32_T pid, UI8_T *gip_ap, BOOL_T *has_group)
{
    MLDSNP_OM_ProfileInfo_T *profile_p;
    MLDSNP_OM_ProfileGroupEntry_T  *profile_group_p;

    MLDSNP_OM_LOCK();
    profile_p = mld_filter_db.profile_entry_p;

    while (profile_p != NULL)
    {
        if (profile_p->pid == pid)
        {   /* found */
            profile_group_p = profile_p->group_entry_p;

            *has_group = profile_group_p == NULL ? FALSE : TRUE;

            while (profile_group_p != NULL)
            {
                if (memcmp(gip_ap , &(profile_group_p->end_mip), SYS_ADPT_IPV6_ADDR_LEN) <= 0)
                {
                    if (memcmp(gip_ap , &(profile_group_p->start_mip), SYS_ADPT_IPV6_ADDR_LEN) >= 0)
                    {
                        MLDSNP_OM_UNLOCK();
                        return TRUE;
                    }
                    goto EXIT;
                }
                profile_group_p = profile_group_p->next;
            }
        }
        if (profile_p->pid > pid)
        {
            goto EXIT;
        }
        profile_p = profile_p->nextPtr;
    }
EXIT:
    MLDSNP_OM_UNLOCK();
    return FALSE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetThrottleStatus
 *-------------------------------------------------------------------------
 * PURPOSE : Get throttle configure status
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : throllt status
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_OM_GetThrottleStatus(UI32_T lport)
{
    UI32_T ret = 0;

    MLDSNP_OM_LOCK();
    ret = mld_throttle_db[lport - 1].throttle_status;
    MLDSNP_OM_UNLOCK();

    return ret;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetMLDThrottlingAction
 *-------------------------------------------------------------------------
 * PURPOSE : Get throttle action
 * INPUT   : lport - which port to get
 * OUTPUT  : action_mode - which action mode
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetMLDThrottlingAction(UI32_T lport, UI32_T *action_mode)
{
    MLDSNP_OM_LOCK();
    *action_mode = mld_throttle_db[lport - 1].action;
    MLDSNP_OM_UNLOCK();

    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetPortMLDThrottlingNumber
 *-------------------------------------------------------------------------
 * PURPOSE : Get port throttle number
 * INPUT   : lport - which port to get
 * OUTPUT  : throttling_number - configured throttle number
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetPortMLDThrottlingNumber(UI32_T lport, UI32_T *throttling_number)
{
    MLDSNP_OM_LOCK();
    *throttling_number = mld_throttle_db[lport - 1].max_group_number;
    MLDSNP_OM_UNLOCK();

    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetPortDynamicGroupCount
 *-------------------------------------------------------------------------
 * PURPOSE : Get this port current learned group count
 * INPUT   : lport - which port to get
 * OUTPUT  : *current_count - current group learned on this port
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetPortDynamicGroupCount(UI32_T lport, UI32_T *current_count)
{
    MLDSNP_OM_LOCK();
    *current_count = mld_throttle_db[lport - 1].current_group_count;
    MLDSNP_OM_UNLOCK();

    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_PortDynamicGroupCountAddOne
 *-------------------------------------------------------------------------
 * PURPOSE : Increate this port learned gorup count
 * INPUT   : lport - which port to increase
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_PortDynamicGroupCountAddOne(UI32_T lport)
{
    MLDSNP_OM_LOCK();
    mld_throttle_db[lport - 1].current_group_count++;
    MLDSNP_OM_UNLOCK();

    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_PortDynamicGroupCountSubtractOne
 *-------------------------------------------------------------------------
 * PURPOSE : decrease this port learned gorup count
 * INPUT   : lport - which port to increase
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_PortDynamicGroupCountSubtractOne(UI32_T lport)
{
    MLDSNP_OM_LOCK();
    mld_throttle_db[lport - 1].current_group_count--;
    MLDSNP_OM_UNLOCK();

    return TRUE;
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_LocalVlanIncStat
 *-------------------------------------------------------------------------
 * PURPOSE : Increase/decrease port statistics
 * INPUT   : vid   - which vlan id
 *           type  - which statistics type
 *           inc   - increase or decrease
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_OM_LocalVlanIncStat(UI32_T vid, MLDSNP_TYPE_Statistics_T type, BOOL_T inc)
{
    if(vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
        goto EXIT_FALSE;

    if (inc)
    {
        switch (type)
        {
            case MLDSNP_TYPE_STAT_GRROUPS:
                vlan_statis[vid-1].num_grecs++;           break;
            case MLDSNP_TYPE_STAT_JOIN_SEND:
                vlan_statis[vid-1].num_joins_send++;      break;
            case MLDSNP_TYPE_STAT_JOINS:
                vlan_statis[vid-1].num_joins++;           break;
            case MLDSNP_TYPE_STAT_JOIN_SUCC:
                vlan_statis[vid-1].num_joins_succ++;      break;
            case MLDSNP_TYPE_STAT_LEAVE_SEND:
                vlan_statis[vid-1].num_leaves_send++;     break;
            case MLDSNP_TYPE_STAT_LEAVE:
                vlan_statis[vid-1].num_leaves++;          break;
            case MLDSNP_TYPE_STAT_GQ_SEND:
                vlan_statis[vid-1].num_gq_send++;         break;
            case MLDSNP_TYPE_STAT_GQ_RCVD:
                vlan_statis[vid-1].num_gq_recv++;         break;
            case MLDSNP_TYPE_STAT_SQ_SEND:
                vlan_statis[vid-1].num_sq_send++;         break;
            case MLDSNP_TYPE_STAT_SQ_RCVD:
                vlan_statis[vid-1].num_sq_recv++;         break;
            case MLDSNP_TYPE_STAT_INVALID:
                vlan_statis[vid-1].num_invalid_mld_recv++;break;
            #if(SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            case MLDSNP_TYPE_STAT_DROP_FILTER:
                vlan_statis[vid-1].num_drop_by_filter++;break;
            #endif
            case MLDSNP_TYPE_STAT_DROP_MROUTER:
                vlan_statis[vid-1].num_drop_by_mroute_port++;break;
            #if(SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN== TRUE)
            case MLDSNP_TYPE_STAT_DROP_RATE_LIMIT:
                vlan_statis[vid-1].num_drop_by_rate_limit++;break;
            #endif
            default:
                goto EXIT_FALSE;
        }
    }
    else
    {
        switch (type)
        {
            case MLDSNP_TYPE_STAT_GRROUPS:
                if (vlan_statis[vid-1].num_grecs > 0)
                    vlan_statis[vid-1].num_grecs--;           break;
            case MLDSNP_TYPE_STAT_JOIN_SEND:
                if (vlan_statis[vid-1].num_joins_send > 0)
                    vlan_statis[vid-1].num_joins_send--;      break;
            case MLDSNP_TYPE_STAT_JOINS:
                if (vlan_statis[vid-1].num_joins > 0)
                    vlan_statis[vid-1].num_joins--;           break;
            case MLDSNP_TYPE_STAT_JOIN_SUCC:
                if (vlan_statis[vid-1].num_joins_succ > 0)
                    vlan_statis[vid-1].num_joins_succ--;      break;
            case MLDSNP_TYPE_STAT_LEAVE_SEND:
                if (vlan_statis[vid-1].num_leaves_send > 0)
                    vlan_statis[vid-1].num_leaves_send--;     break;
            case MLDSNP_TYPE_STAT_LEAVE:
                if (vlan_statis[vid-1].num_leaves > 0)
                    vlan_statis[vid-1].num_leaves--;          break;
            case MLDSNP_TYPE_STAT_GQ_SEND:
                if (vlan_statis[vid-1].num_gq_send > 0)
                    vlan_statis[vid-1].num_gq_send--;         break;
            case MLDSNP_TYPE_STAT_GQ_RCVD:
                if (vlan_statis[vid-1].num_gq_recv > 0)
                    vlan_statis[vid-1].num_gq_recv--;         break;
            case MLDSNP_TYPE_STAT_SQ_SEND:
                if (vlan_statis[vid-1].num_sq_send > 0)
                    vlan_statis[vid-1].num_sq_send--;         break;
            case MLDSNP_TYPE_STAT_SQ_RCVD:
                if (vlan_statis[vid-1].num_sq_recv > 0)
                    vlan_statis[vid-1].num_sq_recv--;         break;
            case MLDSNP_TYPE_STAT_INVALID:
                if (vlan_statis[vid-1].num_invalid_mld_recv > 0)
                    vlan_statis[vid-1].num_invalid_mld_recv++;break;
            #if(SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            case MLDSNP_TYPE_STAT_DROP_FILTER:
                if(vlan_statis[vid-1].num_drop_by_filter>0)
                    vlan_statis[vid-1].num_drop_by_filter--;  break;
            #endif
            case MLDSNP_TYPE_STAT_DROP_MROUTER:
                if(vlan_statis[vid-1].num_drop_by_mroute_port>0)
                    vlan_statis[vid-1].num_drop_by_mroute_port--; break;
            #if(SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN== TRUE)
            case MLDSNP_TYPE_STAT_DROP_RATE_LIMIT:
                if(vlan_statis[vid-1].num_drop_by_rate_limit>0)
                    vlan_statis[vid-1].num_drop_by_rate_limit--;break;
            #endif
            default:
                goto EXIT_FALSE;
        }
    }
    return TRUE;

EXIT_FALSE:
    return FALSE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_LocalPortIncStat
 *-------------------------------------------------------------------------
 * PURPOSE : Increase/decrease port statistics
 * INPUT   : lport - which port to increase
 *           vid   - include vlan or not
 *           type  - which statistics type
 *           inc   - increase or decrease
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    : if vid is not 0, it will increase/decrase vlan, too.
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_OM_LocalPortIncStat(UI32_T lport, UI32_T vid, MLDSNP_TYPE_Statistics_T type, BOOL_T inc)
{
    if (vid > SYS_ADPT_MAX_VLAN_ID)
        goto EXIT_FALSE;

    if (lport < 1 || lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        goto EXIT_FALSE;

    if (vid != 0)
        MLDSNP_OM_LocalVlanIncStat(vid, type, inc);

    if (inc)
    {
        switch (type)
        {
            case MLDSNP_TYPE_STAT_GRROUPS:
                port_statis[lport-1].num_grecs++;           break;
            case MLDSNP_TYPE_STAT_JOIN_SEND:
                port_statis[lport-1].num_joins_send++;      break;
            case MLDSNP_TYPE_STAT_JOINS:
                port_statis[lport-1].num_joins++;           break;
            case MLDSNP_TYPE_STAT_JOIN_SUCC:
                port_statis[lport-1].num_joins_succ++;      break;
            case MLDSNP_TYPE_STAT_LEAVE_SEND:
                port_statis[lport-1].num_leaves_send++;     break;
            case MLDSNP_TYPE_STAT_LEAVE:
                port_statis[lport-1].num_leaves++;          break;
            case MLDSNP_TYPE_STAT_GQ_SEND:
                port_statis[lport-1].num_gq_send++;         break;
            case MLDSNP_TYPE_STAT_GQ_RCVD:
                port_statis[lport-1].num_gq_recv++;         break;
            case MLDSNP_TYPE_STAT_SQ_SEND:
                port_statis[lport-1].num_sq_send++;         break;
            case MLDSNP_TYPE_STAT_SQ_RCVD:
                port_statis[lport-1].num_sq_recv++;         break;
            case MLDSNP_TYPE_STAT_INVALID:
                port_statis[lport-1].num_invalid_mld_recv++;break;
            #if(SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            case MLDSNP_TYPE_STAT_DROP_FILTER:
                port_statis[lport-1].num_drop_by_filter++;break;
            #endif
            case MLDSNP_TYPE_STAT_DROP_MROUTER:
                port_statis[lport-1].num_drop_by_mroute_port++;break;
            #if(SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)
            case MLDSNP_TYPE_STAT_DROP_RATE_LIMIT:
                port_statis[lport-1].num_drop_by_rate_limit++;break;
            #endif
            default:
                goto EXIT_FALSE;
        }
    }
    else
    {
        switch (type)
        {
            case MLDSNP_TYPE_STAT_GRROUPS:
                if (port_statis[lport-1].num_grecs > 0)
                    port_statis[lport-1].num_grecs--;           break;
            case MLDSNP_TYPE_STAT_JOIN_SEND:
                if (port_statis[lport-1].num_joins_send > 0)
                    port_statis[lport-1].num_joins_send--;      break;
            case MLDSNP_TYPE_STAT_JOINS:
                if (port_statis[lport-1].num_joins > 0)
                    port_statis[lport-1].num_joins--;           break;
            case MLDSNP_TYPE_STAT_JOIN_SUCC:
                if (port_statis[lport-1].num_joins_succ > 0)
                    port_statis[lport-1].num_joins_succ--;      break;
            case MLDSNP_TYPE_STAT_LEAVE_SEND:
                if (port_statis[lport-1].num_leaves_send > 0)
                    port_statis[lport-1].num_leaves_send--;     break;
            case MLDSNP_TYPE_STAT_LEAVE:
                if (port_statis[lport-1].num_leaves > 0)
                    port_statis[lport-1].num_leaves--;          break;
            case MLDSNP_TYPE_STAT_GQ_SEND:
                if (port_statis[lport-1].num_gq_send > 0)
                    port_statis[lport-1].num_gq_send--;         break;
            case MLDSNP_TYPE_STAT_GQ_RCVD:
                if (port_statis[lport-1].num_gq_recv > 0)
                    port_statis[lport-1].num_gq_recv--;         break;
            case MLDSNP_TYPE_STAT_SQ_SEND:
                if (port_statis[lport-1].num_sq_send > 0)
                    port_statis[lport-1].num_sq_send--;         break;
            case MLDSNP_TYPE_STAT_SQ_RCVD:
                if (port_statis[lport-1].num_sq_recv > 0)
                    port_statis[lport-1].num_sq_recv--;         break;
            case MLDSNP_TYPE_STAT_INVALID:
                if (port_statis[lport-1].num_invalid_mld_recv > 0)
                    port_statis[lport-1].num_invalid_mld_recv--;break;
            #if(SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            case MLDSNP_TYPE_STAT_DROP_FILTER:
                if(port_statis[lport-1].num_drop_by_filter>0)
                    port_statis[lport-1].num_drop_by_filter--;  break;
            #endif
            case MLDSNP_TYPE_STAT_DROP_MROUTER:
                if(port_statis[lport-1].num_drop_by_mroute_port>0)
                    port_statis[lport-1].num_drop_by_mroute_port--; break;
            #if(SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)
            case MLDSNP_TYPE_STAT_DROP_RATE_LIMIT:
                if(port_statis[lport-1].num_drop_by_rate_limit>0)
                    port_statis[lport-1].num_drop_by_rate_limit--;break;
            #endif
            default:
                goto EXIT_FALSE;
        }
    }
    return TRUE;

EXIT_FALSE:
    return FALSE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_PortIncStat
 *-------------------------------------------------------------------------
 * PURPOSE : Increase/decrease port statistics
 * INPUT   : lport - which port to increase
 *           vid   - include vlan or not
 *           type  - which statistics type
 *           inc   - increase or decrease
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    : if vid is not 0, it will increase/decrase vlan, too.
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_PortIncStat(UI32_T lport, UI32_T vid, MLDSNP_TYPE_Statistics_T type, BOOL_T inc)
{
    BOOL_T ret;

    MLDSNP_OM_LOCK();
    ret = MLDSNP_OM_LocalPortIncStat(lport, vid, type, inc);
    MLDSNP_OM_UNLOCK();

    return ret;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_VlanIncStat
 *-------------------------------------------------------------------------
 * PURPOSE : Increase/decrease port statistics
 * INPUT   : vid   - which vlan id
 *           type  - which statistics type
 *           inc   - increase or decrease
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_VlanIncStat(UI32_T vid, MLDSNP_TYPE_Statistics_T type, BOOL_T inc)
{
    BOOL_T ret;

    if(vid == 0)
        return FALSE;

    MLDSNP_OM_LOCK();
    ret = MLDSNP_OM_LocalVlanIncStat(vid, type, inc);
    MLDSNP_OM_UNLOCK();

    return ret;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_ClearInterfaceStatistics
 *-------------------------------------------------------------------------
 * PURPOSE : Clear all statistics on this port or vlan
 * INPUT   : ifindex - port or vlan id
 *           is_vlan - ifindex mean port or vlan id
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_ClearInterfaceStatistics(UI32_T ifindex, BOOL_T is_vlan)
{
    MLDSNP_OM_LOCK();

    if (is_vlan)
        memset(&vlan_statis[ifindex-1], 0, sizeof(MLDSNP_OM_Counter_T));
    else
        memset(&port_statis[ifindex-1], 0, sizeof(MLDSNP_OM_Counter_T));

    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetInterfaceStatistics
 *-------------------------------------------------------------------------
 * PURPOSE : Get interface statistics
 * INPUT   : ifindex - port or vlan id
 *           is_vlan - is vlan id or port
 * OUTPUT  : statistics_p - statistics
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetInterfaceStatistics(UI32_T ifindex, BOOL_T is_vlan, MLDSNP_OM_InfStat_T *statistics_p)
{
    MLDSNP_OM_LOCK();

    if (is_vlan)
    {
        MLDSNP_OM_VlanInfo_T vlan_info;

        if(ifindex < 1 || ifindex > SYS_ADPT_MAX_VLAN_ID)
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }

        vlan_info.vid = ifindex;

        statistics_p->counter = vlan_statis[ifindex-1];

        if (TRUE == L_SORT_LST_Get(&mldsnp_om_vlan_list_g, &vlan_info))
        {
            IPV6_ADDR_COPY(statistics_p->querier_ip_addr, vlan_info.other_querier_src_ip);
            if(vlan_info.other_querier_uptime!=0)
            {
              UI32_T tmp = SYSFUN_GetSysTick();

              if(tmp > vlan_info.other_querier_uptime)
                  statistics_p->other_query_uptime= (tmp - vlan_info.other_querier_uptime)/100; /*convert ticks to sec*/
              else
                  statistics_p->other_query_uptime = ((0xffffffff - vlan_info.other_querier_uptime) + tmp)/100;
            }
            else
              statistics_p->other_query_uptime=0;
            MLDSNP_TIMER_QueryTimer(vlan_info.other_querier_present_timer_p, &statistics_p->other_query_exptime);

            if(vlan_info.querier_uptime!=0)
            {
              UI32_T tmp2 = SYSFUN_GetSysTick();

              if(tmp2 > vlan_info.querier_uptime)
                  statistics_p->query_uptime= (tmp2 - vlan_info.querier_uptime)/100; /*convert ticks to sec*/
              else
                  statistics_p->query_uptime = ((0xffffffff - vlan_info.querier_uptime) + tmp2)/100;
            }
            else
              statistics_p->query_uptime=0;

        }
        else
        {
            IPV6_ADDR_SET(statistics_p->querier_ip_addr);
        }

        statistics_p->active_group_cnt = vlan_statis[ifindex-1].num_grecs;
        MLDSNP_TIMER_QueryTimer(mldsnp_om_querier_timer_p, &statistics_p->query_exptime);
        #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        if(mldsnp_om_cfg_db_g.proxy_reporting == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
        {
            MLDSNP_TIMER_QueryTimer(mldsnp_om_unsolicite_timer_p, &statistics_p->unsolicit_exptime);
        }
        #endif
    }
    else
    {
        if(ifindex < 1 || ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            MLDSNP_OM_UNLOCK();
            return FALSE;
        }
        statistics_p->counter = port_statis[ifindex-1];
    }

    MLDSNP_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetTotalEntries
 *-------------------------------------------------------------------------
 * PURPOSE : Get interface statistics
 * INPUT   : None
 * OUTPUT  : total_p - total entries
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetTotalEntries(UI32_T *total_p)
{
  MLDSNP_OM_VlanInfo_T vlan_info;
  UI16_T vid=0;

  *total_p=0;

  while(TRUE == MLDSNP_OM_GetNextVlanInfo(&vid, &vlan_info))
  {
    *total_p += vlan_statis[vid-1].num_grecs;
  }

  return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetRunningVlanCfg
 *-------------------------------------------------------------------------
 * PURPOSE : Get vlan configure, only return the VLAN has user configure
 * INPUT   : None
 * OUTPUT  : total_p - total entries
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_GetRunningVlanCfg(UI32_T *nxt_vid_p, MLDSNP_OM_VlanRunningCfg_T *vlan_cfg_p)
{
    MLDSNP_OM_RouterPortInfo_T router_port_info;
    MLDSNP_OM_VlanInfo_T        vlan_info;
    BOOL_T ret = FALSE;

    memset(vlan_cfg_p, 0, sizeof(MLDSNP_OM_VlanRunningCfg_T));

    MLDSNP_OM_LOCK();

    vlan_info.vid = *nxt_vid_p;

    while (TRUE == L_SORT_LST_Get_Next(&mldsnp_om_vlan_list_g, &vlan_info))
    {
        *nxt_vid_p = vlan_info.vid;

        router_port_info.port_no=0;
        while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &router_port_info))
        {
            if (MLDSNP_TYPE_JOIN_STATIC == router_port_info.attribute)
            {
                MLDSNP_TYPE_AddPortIntoPortBitMap(router_port_info.port_no, vlan_cfg_p->router_port_bitmap);
                ret = TRUE;
            }
        }

        #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == TRUE)
        if(*nxt_vid_p >=1 && *nxt_vid_p <= SYS_ADPT_MAX_VLAN_ID)
        {
            vlan_cfg_p->flood_behavior = mldsnp_om_cfg_db_g.unknown_flood_behavior[*nxt_vid_p-1];
        }
        if(vlan_cfg_p->flood_behavior != SYS_DFLT_MLDSNP_UNKNOWN_MULTICAST_MOD)
            ret = TRUE;
        #endif

        vlan_cfg_p->immediate_leave_status        = vlan_info.immediate_leave_status;
        vlan_cfg_p->immediate_leave_byhost_status = vlan_info.immediate_leave_byhost_status;

       if ((SYS_DFLT_MLDSNP_IMMEIDATE_STATUS != vlan_info.immediate_leave_status)
           || (SYS_DFLT_MLDSNP_IMMEIDATE_BYHOST_STATUS != vlan_info.immediate_leave_byhost_status))
       {
           ret = TRUE;
       }

       #if(SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
       vlan_cfg_p->pkt_ratelimit = mldsnp_om_cfg_db_g.vlan_mld_pkt_lmt_conf[vlan_info.vid -1 ];
       if (vlan_cfg_p->pkt_ratelimit != SYS_DFLT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN)
           ret = TRUE;
       #endif

       if(ret == TRUE)
        break;
    }

    MLDSNP_OM_UNLOCK();

    return ret;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : MLDSNP_OM_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
  * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
  * RETURN:
 *    TRUE  --  There is a response need to send.
 *    FALSE --  No response need to send.
  * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *------------------------------------------------------------------------------
 */
BOOL_T MLDSNP_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI32_T cmd;

    if (ipcmsg_p == NULL)
    {
        return TRUE;
    }

    msg_data_p = (MLDSNP_OM_IPCMsg_T*)ipcmsg_p->msg_buf;
    cmd = msg_data_p->type.cmd;

    switch (cmd)
    {
        case MLDSNP_OM_IPCCMD_GETGLOBALCONF:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.mldsnp_global_conf)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetGlobalConf(
                                               &msg_data_p->data.mldsnp_global_conf);
            break;

        case MLDSNP_OM_IPCCMD_GETGROUPPORTLIST:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.group_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetGroupPortlist(
                                               msg_data_p->data.group_portlist.vid,
                                               msg_data_p->data.group_portlist.gip_ar,
                                               msg_data_p->data.group_portlist.sip_ar,
                                               &msg_data_p->data.group_portlist.scgroup_info);
            break;

        case MLDSNP_OM_IPCCMD_GETHISAMENTRYINFO:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.hisam_entry_info)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetHisamEntryInfo(
                                               msg_data_p->data.hisam_entry_info.vid,
                                               msg_data_p->data.hisam_entry_info.gip_ar,
                                               msg_data_p->data.hisam_entry_info.sip_ar,
                                               msg_data_p->data.hisam_entry_info.key_idx,
                                               &msg_data_p->data.hisam_entry_info.entry_info);
            break;

        case MLDSNP_OM_IPCCMD_GETIMMEDIATELEAVESTATUS:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.immediate_leave)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetImmediateLeaveStatus(
                                               msg_data_p->data.immediate_leave.vid,
                                               &msg_data_p->data.immediate_leave.status);
            break;

        case MLDSNP_OM_IPCCMD_GETIMMEDIATELEAVEBYHOSTSTATUS:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.immediate_leave_byhost)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetImmediateLeaveByHostStatus(
                                               msg_data_p->data.immediate_leave_byhost.vid,
                                               &msg_data_p->data.immediate_leave_byhost.status);
            break;

        case MLDSNP_OM_IPCCMD_GETNEXTIMMEDIATELEAVESTATUS:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.immediate_leave)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetNextImmediateLeaveStatus(
                                               &msg_data_p->data.immediate_leave.vid,
                                               &msg_data_p->data.immediate_leave.status);
            break;

       case MLDSNP_OM_IPCCMD_GETNEXTIMMEDIATELEAVEBYHOSTSTATUS:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.immediate_leave_byhost)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetNextImmediateLeaveByHostStatus(
                                               &msg_data_p->data.immediate_leave_byhost.vid,
                                               &msg_data_p->data.immediate_leave_byhost.status);
            break;

        case MLDSNP_OM_IPCCMD_GETLASTLISTENERQUERYINTERVAL:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetLastListenerQueryInterval(
                                               &msg_data_p->data.ui16_v);
            break;

        case MLDSNP_OM_IPCCMD_GETLISTENERINTERVAL:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetListenerInterval(
                                               &msg_data_p->data.ui16_v);
            break;

        case MLDSNP_OM_IPCCMD_GETMLDSNPVER:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetMldSnpVer(
                                               &msg_data_p->data.ui16_v);
            break;

        case MLDSNP_OM_IPCCMD_GETMLDSTATUS:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.mldsnp_status)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetMldStatus(
                                               &msg_data_p->data.mldsnp_status);
            break;

        case MLDSNP_OM_IPCCMD_GETNEXTGROUPPORTLIST:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.group_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetNextGroupPortlist(
                                               &msg_data_p->data.group_portlist.vid,
                                               msg_data_p->data.group_portlist.gip_ar,
                                               msg_data_p->data.group_portlist.sip_ar,
                                               &msg_data_p->data.group_portlist.scgroup_info);
            break;

        case MLDSNP_OM_IPCCMD_GETNEXTPORTGROUPSOURCELIST:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.port_source_list)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetNextPortGroupSourceList(
                                               &msg_data_p->data.port_source_list);
            break;

        case MLDSNP_OM_IPCCMD_GETPORTGROUPSOURCELIST:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.port_source_list)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetPortGroupSourceList(
                                               &msg_data_p->data.port_source_list);
            break;
        case MLDSNP_OM_IPCCMD_GETNEXTPORTGROUPSOURCE:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.port_grp_src)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetNextPortGroupSource(&msg_data_p->data.port_grp_src);
            break;

        case MLDSNP_OM_IPCCMD_GETNEXTPORTGROUPSOURCEHOST:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.port_host)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetNextPortGroupSourceHost(&msg_data_p->data.port_host);
            break;

        case MLDSNP_OM_IPCCMD_GETNEXTRUNNINGPORTJOINSTATICGROUP:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.running_join_static_group)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_OM_GetNextRunningStaticPortJoinGroup(
                                               &msg_data_p->data.running_join_static_group.id,
                                               &msg_data_p->data.running_join_static_group.vid,
                                               msg_data_p->data.running_join_static_group.gip_ar,
                                               msg_data_p->data.running_join_static_group.sip_ar,
                                               &msg_data_p->data.running_join_static_group.port,
                                               &msg_data_p->data.running_join_static_group.rec_type);
            break;
        case MLDSNP_OM_IPCCMD_GETNEXTPORTJOINSTATICGROUP:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.running_join_static_group)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetNextStaticPortJoinGroup(
                                               &msg_data_p->data.running_join_static_group.id,
                                               &msg_data_p->data.running_join_static_group.vid,
                                               msg_data_p->data.running_join_static_group.gip_ar,
                                               msg_data_p->data.running_join_static_group.sip_ar,
                                               &msg_data_p->data.running_join_static_group.port,
                                               &msg_data_p->data.running_join_static_group.rec_type);
            break;

        case MLDSNP_OM_IPCCMD_GETSTATICGROUPPORTLIST:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.group_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetStaticGroupPortlist(
                                               msg_data_p->data.group_portlist.vid,
                                               msg_data_p->data.group_portlist.gip_ar,
                                               msg_data_p->data.group_portlist.sip_ar,
                                               msg_data_p->data.group_portlist.scgroup_info.static_port_bitmap);
            break;

        case MLDSNP_OM_IPCCMD_GETNEXTSTATICGROUPPORTLIST:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.group_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetNextStaticGroupPortlist(
                                               &msg_data_p->data.group_portlist.vid,
                                               msg_data_p->data.group_portlist.gip_ar,
                                               msg_data_p->data.group_portlist.sip_ar,
                                               msg_data_p->data.group_portlist.scgroup_info.static_port_bitmap);
            break;

        case MLDSNP_OM_IPCCMD_GETOLDVERQUERIERPRESENTTIMEOUT:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetOldVerQuerierPresentTimeOut(
                                               &msg_data_p->data.ui16_v);
            break;
        case MLDSNP_OM_IPCCMD_GETOTHERQUERYPRESENTINTERVAL:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.ui32_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetOtherQueryPresentInterval(
                                               &msg_data_p->data.ui32_v);
            break;

        case MLDSNP_OM_IPCCMD_GETQUERIERRUNNINGSTATUS:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.running_queier)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_OM_GetQuerierRunningStatus(
                                               msg_data_p->data.running_queier.vid,
                                               &msg_data_p->data.running_queier.status);
            break;

        case MLDSNP_OM_IPCCMD_GETQUERIERSTATUS:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.querier_status)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetQuerierStauts(
                                               &msg_data_p->data.querier_status);
            break;

        case MLDSNP_OM_IPCCMD_GETQUERYINTERVAL:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetQueryInterval(
                                               &msg_data_p->data.ui16_v);
            break;

        case MLDSNP_OM_IPCCMD_GETQUERYRESPONSEINTERVAL:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetQueryResponseInterval(
                                               &msg_data_p->data.ui16_v);
            break;

        case MLDSNP_OM_IPCCMD_GETROBUSTNESSVALUE:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetRobustnessValue(
                                               &msg_data_p->data.ui16_v);
            break;

        case MLDSNP_OM_IPCCMD_GETROUTEREXPIRETIME:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetRouterExpireTime(
                                               &msg_data_p->data.ui16_v);
            break;

        case MLDSNP_OM_IPCCMD_GETRUNNINGGLOBALCONF:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.mldsnp_running_global_conf)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_OM_GetRunningGlobalConf(
                                               &msg_data_p->data.mldsnp_running_global_conf);
            break;

        case MLDSNP_OM_IPCCMD_GETRUNNINGIMMEDIATELEAVESTATUS:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.immediate_leave)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_OM_GetRunningVlanImmediateStatus(
                                               msg_data_p->data.immediate_leave.vid,
                                               &msg_data_p->data.immediate_leave.status,
                                               &msg_data_p->data.immediate_leave.byhost_status);
            break;

        case MLDSNP_OM_IPCCMD_GETRUNNINGROUTERPORTLIST:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.running_router_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_OM_GetRunningRouterPortList(
                                               msg_data_p->data.running_router_portlist.vid,
                                               msg_data_p->data.running_router_portlist.router_port_bitmap);
            break;

        case MLDSNP_OM_IPCCMD_GETUNKNOWNFLOODBEHAVIOR:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.flood)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetUnknownFloodBehavior(
                                               msg_data_p->data.flood.vlan_id,
                                               &msg_data_p->data.flood.flood_behavior);
            break;

        case MLDSNP_OM_IPCCMD_GETRUNNINGUNKNOWNFLOODBEHAVIOR:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.flood)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_OM_GetRunningUnknownFloodBehavior(
                                               msg_data_p->data.flood.vlan_id,
                                               &msg_data_p->data.flood.flood_behavior);
            break;
        case MLDSNP_OM_IPCCMD_GETVLANROUTERPORTLIST:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.vlan_router_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetVlanRouterPortlist(
                                               msg_data_p->data.vlan_router_portlist.vid,
                                               &msg_data_p->data.vlan_router_portlist.router_port_list);
            break;

        case MLDSNP_OM_IPCCMD_GETNEXTVLANROUTERPORTLIST:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.vlan_router_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetNextVlanRouterPortlist(
                                               &msg_data_p->data.vlan_router_portlist.vid,
                                               &msg_data_p->data.vlan_router_portlist.router_port_list);
            break;
        case MLDSNP_OM_IPCCMD_GETVLANROUTERPORTEXPIRE:
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.u32a1_u32a2_u32_a3)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = MLDSNP_OM_GetRouterExpire(
                                               msg_data_p->data.u32a1_u32a2_u32_a3.u32_a1,
                                               msg_data_p->data.u32a1_u32a2_u32_a3.u32_a2,
                                               &msg_data_p->data.u32a1_u32a2_u32_a3.u32_a3);
            break;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        case MLDSNP_OM_IPCCMD_GET_PROXY_REPORTING:
        {
            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_TYPE_ProxyReporting_T);

            msg_data_p->type.result_bool = MLDSNP_OM_GetProxyReporting(&msg_data_p->data.proxy_reporting);
        }
        break;
        case MLDSNP_OM_IPCCMD_GETUNSOLICITEDREPORTINTERVAL:
        {
            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(UI32_T);

            msg_data_p->type.result_bool = MLDSNP_OM_GetUnsolicitedReportInterval(
                                               &msg_data_p->data.ui32_v);
        }
        break;
#endif
#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP == TRUE)
        case  MLDSNP_OM_IPCCMD_GET_MULTICAST_DATA_DROP_STATUS:
        {
            MLDSNP_OM_IPCMsg_GS2_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            BOOL_T status;
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_GetMulticastDataDropStatus(data_p->value1, &status);
            data_p->value2 = status;
            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T);
        }
        break;

        case  MLDSNP_OM_IPCCMD_GET_RUNNING_MULTICAST_DATA_DROP_STATUS:
        {
            MLDSNP_OM_IPCMsg_GS2_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            BOOL_T status = FALSE;
            MLDSNP_OM_GetMulticastDataDropStatus(data_p->value1, &status);

            if ((SYS_DFLT_MLDSNP_MULTICAST_DATA_DROP == VAL_mldSnoopMulticastDataDrop_disable
                  && status == TRUE)
                  ||
                  (SYS_DFLT_MLDSNP_MULTICAST_DATA_DROP == VAL_mldSnoopMulticastDataDrop_enable
                   && status == FALSE)
               )
            {
                MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
                data_p->value2 = status ? VAL_mldSnoopMulticastDataDrop_enable : VAL_mldSnoopMulticastDataDrop_disable;
            }
            else
                MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T);
        }
        break;
#endif
#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)
        case MLDSNP_OM_IPCCMD_GET_QUERY_GUARD_STATUS:
        {
            MLDSNP_OM_IPCMsg_GS2_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            BOOL_T status = FALSE;
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_GetQueryDropStatus(data_p->value1, &status);
            data_p->value2 = status;
            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T);
        }
        break;

        case MLDSNP_OM_IPCCMD_GET_RUNNING_QUERY_GUARD_STATUS:
        {
            MLDSNP_OM_IPCMsg_GS2_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            BOOL_T status = FALSE;
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_GetQueryDropStatus(data_p->value1, &status);
            if ((SYS_DFLT_MLDSNP_QUERY_DROP == VAL_mldSnoopQueryDrop_disable
                    && status == TRUE)
                    ||
                    (SYS_DFLT_MLDSNP_QUERY_DROP == VAL_mldSnoopQueryDrop_enable
                     && status == FALSE))
            {
                MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
                data_p->value2 = status ? VAL_mldSnoopQueryDrop_enable : VAL_mldSnoopQueryDrop_disable;
            }
            else
                MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T);
        }
        break;
#endif

#if ( SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
        case MLDSNP_OM_IPCCMD_GETNEXT_MLD_PRIFILE_ID:
        {
            MLDSNP_OM_IPCMsg_MldSnoopProfile_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_GetNextMldProfileID(&data_p->mldsnp_Profile_id);
            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfile_T);
            break;
        }

        case MLDSNP_OM_IPCCMD_GET_MLD_THROTTLE_INFO:
        {
            MLDSNP_OM_IPCMsg_MldSnoopThrottleInfo_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_GetMldThrottlingInfo(data_p->port, &data_p->throttling_info);
            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopThrottleInfo_T);
            break;
        }

        case MLDSNP_OM_IPCCMD_EXIST_MLD_CREATE_PRIFILE:
        {
            MLDSNP_OM_IPCMsg_MldSnoopProfile_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_IsMldProfileExist(data_p->mldsnp_Profile_id);
            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfile_T);
            break;
        }

        case MLDSNP_OM_IPCCMD_GET_MLD_PRIFILE_PORT:
        {
            MLDSNP_OM_IPCMsg_MldSnoopProfilePort_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_GetPortMldProfileID(data_p->port, &data_p->mldsnp_Profile_id);
            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfilePort_T);
            break;
        }

        case MLDSNP_OM_IPCCMD_GETNEXT_MLD_PRIFILE_GROUP:
        {
            MLDSNP_OM_IPCMsg_MldSnoopProfileRange_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_GetNextMldProfileGroupbyPid(data_p->mldsnp_Profile_id, data_p->ip_begin, data_p->ip_end);
            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfileRange_T);
            break;
        }

        case MLDSNP_OM_IPCCMD_GET_MLD_PRIFILE_MODE:
        {
            MLDSNP_OM_IPCMsg_MldSnoopProfileMode_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_GetMldProfileAccessMode(data_p->mldsnp_Profile_id, &data_p->profile_mode);
            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfileMode_T);
            break;
        }

        case MLDSNP_OM_IPCCMD_GET_MLD_FILTER_STATUS:
        {
            MLDSNP_OM_IPCMsg_MldSnoopFilter_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_GetMldFilterStatus(&data_p->mldsnp_FilterStatus);
            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopFilter_T);
            break;
        }

#endif
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
        case MLDSNP_OM_IPCCMD_GET_MLD_REPORT_LIMIT_PER_SECOND:
        {
            MLDSNP_OM_IPCMsg_GS2_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = TRUE;

            if (data_p->value1 >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER && data_p->value1 <= (SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + SYS_ADPT_MAX_VLAN_ID))
            {
                UI32_T vid = 0;
                VLAN_IFINDEX_CONVERTTO_VID(data_p->value1, vid);
                data_p->value2 = MLDSNP_OM_GetVlanMldReportLimitPerSec(vid);
            }
            else if (data_p->value1 >= 1 && data_p->value1 <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
                data_p->value2 = MLDSNP_OM_GetPortMldReportLimitPerSec(data_p->value1);
            else
                MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = FALSE;

            ipcmsg_p->msg_size = MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T);
        }
        break;
#endif
        case MLDSNP_OM_IPCCMD_GET_TOTAL_ENTRY:
        {
            MLDSNP_OM_IPCMsg_GS1_T *data_p = MLDSNP_OM_MSG_DATA(ipcmsg_p);

            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_GetTotalEntries(&data_p->value1);
            break;
        }
        case MLDSNP_OM_IPCCMD_GETRUNNINGVLANCFG:
        {
            ipcmsg_p->msg_size = sizeof(msg_data_p->data.vlan_cfg)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE;
            MLDSNP_OM_MSG_RETVAL(ipcmsg_p) = MLDSNP_OM_GetRunningVlanCfg(&msg_data_p->data.vlan_cfg.vlan_id,
                                                                         &msg_data_p->data.vlan_cfg.vlan_cfg);
            break;
        }
        default:
            msg_data_p->type.result_bool = FALSE;
            SYSFUN_Debug_Printf("%s(%d): Invalid cmd.\n", __FUNCTION__, __LINE__);
            return TRUE;

    }

    /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
     */
    if (cmd < MLDSNP_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#endif /*SYS_CPNT_MLDSNP == TRUE*/


/* Module Name:NETCFG_OM_OSPF.C
 * Purpose: To store OSPF configuration information.
 *
 * Notes:
 *      
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *    2008/11/27     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008.
 */

#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_mm.h"
#include "l_pt.h"
#include "sysfun.h"
#include "vlan_lib.h"
#include "netcfg_om_ospf.h"
#include "netcfg_type.h"
#include "netcfg_netdevice.h"
#include "l_ls_prefix.h"

#define MAX_NBR_OF_VR   1
#define MAX_NBR_OF_VRF   1

#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) = (V) | (F)
#define UNSET_FLAG(V,F)      (V) = (V) & ~(F)
#define FLAG_ISSET(V,F)      (((V) & (F)) == (F))
#define LIST_LOOP(L,V,N) \
  if (L) \
    for ((N) = (L)->head; (N); (N) = (N)->next) \
      if (((V) = (N)->data) != NULL)

#define OSPF_AREA_CONF_CHECK(A, T)                                            \
    (CHECK_FLAG ((A)->conf.config, NETCFG_TYPE_OSPF_AREA_CONF_ ## T))

#define OSPF_AREA_CONF_SET(A, T)                                              \
    do {                                                                      \
      if (!OSPF_AREA_CONF_CHECK (A, T))                                       \
        {                                                                     \
          ((A)->conf.config |= NETCFG_TYPE_OSPF_AREA_CONF_ ## T);                         \
          NETCFG_OM_OSPF_AreaLock((A));                                               \
        }                                                                     \
    } while (0)

#define OSPF_AREA_CONF_UNSET(A, T)                                            \
    do {                                                                      \
      if ((A) && OSPF_AREA_CONF_CHECK (A, T))                                 \
        {                                                                     \
          ((A)->conf.config &= ~(NETCFG_TYPE_OSPF_AREA_CONF_ ## T));                      \
          (A) = NETCFG_OM_OSPF_AreaUnlock ((A));                                       \
        }                                                                     \
    } while (0)


#define GETMINVALUE(v1, v2)             (v1 <= v2)? v1:v2
#define PREFIXLEN2MASK(len)             ~((1 << (32 - len)) - 1)

#define IFC_IPV4_CMP(A,B)						\
  (pal_ntoh32 ((A)->s_addr) < pal_ntoh32 ((B)->s_addr) ? -1 :		\
   (pal_ntoh32 ((A)->s_addr) > pal_ntoh32 ((B)->s_addr) ? 1 : 0))

/* Macros. */
#define NETCFG_OM_OSPF_IF_PARAM_CHECK(P, T)                                             \
    ((P) && CHECK_FLAG ((P)->config, NETCFG_OM_OSPF_IF_PARAM_ ## T))
#define NETCFG_OM_OSPF_IF_PARAM_SET(P, T)     ((P)->config |= NETCFG_OM_OSPF_IF_PARAM_ ## T)
#define NETCFG_OM_OSPF_IF_PARAM_UNSET(P, T)   ((P)->config &= ~(NETCFG_OM_OSPF_IF_PARAM_ ## T))
#define NETCFG_OM_OSPF_IF_PARAMS_EMPTY(P)     ((P)->config == 0)

#define NETCFG_OM_OSPF_IF_PARAM_HELLO_INTERVAL_DEFAULT(P)                               \
    (((P) != NULL                                                             \
      && ((P)->type == NETCFG_OM_OSPF_IFTYPE_NBMA                                       \
          || (P)->type == NETCFG_OM_OSPF_IFTYPE_POINTOMULTIPOINT                        \
          || (P)->type == NETCFG_OM_OSPF_IFTYPE_POINTOMULTIPOINT_NBMA))                 \
     ? NETCFG_OM_OSPF_HELLO_INTERVAL_NBMA_DEFAULT : NETCFG_OM_OSPF_HELLO_INTERVAL_DEFAULT)

#define NETCFG_OM_OSPF_IF_PARAM_DEAD_INTERVAL_DEFAULT(P)                                \
    ((((P) != NULL && NETCFG_OM_OSPF_IF_PARAM_CHECK ((P), HELLO_INTERVAL))              \
      ? (P)->hello_interval : NETCFG_OM_OSPF_IF_PARAM_HELLO_INTERVAL_DEFAULT (P)) * 4)

#define NETCFG_OM_OSPF_IF_PARAM_DEAD_INTERVAL_UPDATE(P)                                 \
    do {                                                                      \
        if (!NETCFG_OM_OSPF_IF_PARAM_CHECK (P, DEAD_INTERVAL))                            \
            (P)->dead_interval = NETCFG_OM_OSPF_IF_PARAM_DEAD_INTERVAL_DEFAULT (P);         \
        else if ((P)->dead_interval == NETCFG_OM_OSPF_IF_PARAM_DEAD_INTERVAL_DEFAULT (P)) \
            NETCFG_OM_OSPF_IF_PARAM_UNSET (P, DEAD_INTERVAL);                               \
    } while (0)
/* Add by hongliang */
#define METRIC_TYPE(C)		(CHECK_FLAG ((C)->flags,                      \
                             OSPF_REDIST_METRIC_TYPE) ?       \
                             EXTERNAL_METRIC_TYPE_1 :         \
                             EXTERNAL_METRIC_TYPE_2)
#define METRIC_TYPE_SET(C,T)                                                  \
        do {                                                                      \
          if ((T) == EXTERNAL_METRIC_TYPE_1)                                      \
            SET_FLAG ((C)->flags, OSPF_REDIST_METRIC_TYPE);                       \
          else if ((T) == EXTERNAL_METRIC_TYPE_2)                                 \
            UNSET_FLAG ((C)->flags, OSPF_REDIST_METRIC_TYPE);                     \
        } while (0)
#define METRIC_VALUE(C)		((C)->metric)
#define REDIST_TAG(C)		((C)->tag)
#define DIST_NAME(C)		((C)->distribute_list.name)
#define DIST_LIST(C)		((C)->distribute_list.list)
#define RMAP_NAME(C)		((C)->route_map.name)
#define RMAP_MAP(C)		((C)->route_map.map)
#define REDIST_PROTO_SET(C)                                                   \
                (SET_FLAG ((C)->flags, OSPF_REDIST_ENABLE))
#define REDIST_PROTO_UNSET(C)                                                 \
                (UNSET_FLAG ((C)->flags, OSPF_REDIST_ENABLE))
#define REDIST_PROTO_CHECK(C)                                                 \
                (CHECK_FLAG ((C)->flags, OSPF_REDIST_ENABLE))

typedef struct NETCFG_OM_OSPF_TableIndex_S
{
  unsigned int len;
  unsigned int octets;
  char vars[NETCFG_OM_OSPF_API_INDEX_VARS_MAX];
}NETCFG_OM_OSPF_TableIndex_T;

struct L_ls_table_index netcfg_om_ospf_api_table_def[] =
{
  /* 0 -- dummy. */
  {  0,  0, { L_LS_INDEX_NONE } },
  /* 1 -- dummy ospfGeneralGroup. */
  {  0,  0, { L_LS_INDEX_NONE } },
  /* 2 -- ospfAreaTable. */
  {  4,  4, { L_LS_INDEX_INADDR, L_LS_INDEX_NONE } },
  /* 3 -- ospfStubAreaTable. */
  {  5,  5, { L_LS_INDEX_INADDR, L_LS_INDEX_INT8, L_LS_INDEX_NONE } },
  /* 4 -- ospfLsdbTable. */
  { 13, 13, { L_LS_INDEX_INADDR, L_LS_INDEX_INT8, L_LS_INDEX_INADDR,
	      L_LS_INDEX_INADDR, L_LS_INDEX_NONE } },
  /* 5 -- ospfAreaRangeTable. */
  {  8,  8, { L_LS_INDEX_INADDR, L_LS_INDEX_INADDR, L_LS_INDEX_NONE } },
  /* 6 -- ospfHostTable. */
  {  5,  5, { L_LS_INDEX_INADDR, L_LS_INDEX_INT8, L_LS_INDEX_NONE } },
  /* 7 -- ospfIfTable. */
  {  5,  8, { L_LS_INDEX_INADDR, L_LS_INDEX_INT32, L_LS_INDEX_NONE } },
  /* 8 -- ospfIfMetricTable. */
  {  6,  9, { L_LS_INDEX_INADDR, L_LS_INDEX_INT32, L_LS_INDEX_INT8,
	      L_LS_INDEX_NONE } },
  /* 9 -- ospfVirtIfTable. */
  {  8,  8, { L_LS_INDEX_INADDR, L_LS_INDEX_INADDR, L_LS_INDEX_NONE } },
  /* 10 -- ospfNbrTable. */
  {  5,  8, { L_LS_INDEX_INADDR, L_LS_INDEX_INT32, L_LS_INDEX_NONE } },
  /* 11 -- ospfVirtNbrTable. */
  {  8,  8, { L_LS_INDEX_INADDR, L_LS_INDEX_INADDR, L_LS_INDEX_NONE } },
  /* 12 -- ospfExtLsdbTable. */
  {  9,  9, { L_LS_INDEX_INT8, L_LS_INDEX_INADDR, L_LS_INDEX_INADDR,
	      L_LS_INDEX_NONE } },
  /* 13 -- dummy ospfRouteGroup. */
  {  0,  0, { L_LS_INDEX_NONE } },
  /* 14 -- ospfAreaAggregateTable. */
  { 13, 13, { L_LS_INDEX_INADDR, L_LS_INDEX_INT8, L_LS_INDEX_INADDR,
	      L_LS_INDEX_INADDR, L_LS_INDEX_NONE } },
  /* 15 -- dummy. */
  {  0,  0, { L_LS_INDEX_NONE } },
  /* 16 -- ospfStaticNbrTable. */
  {  4,  4, { L_LS_INDEX_INADDR, L_LS_INDEX_NONE } },
  /* 17 -- ospfLsdbTable - upper. */
  {  5,  5, { L_LS_INDEX_INADDR, L_LS_INDEX_INT8, L_LS_INDEX_NONE } },
  /* 18 -- ospfLsdbTable - lower. */
  {  8,  8, { L_LS_INDEX_INADDR, L_LS_INDEX_INADDR, L_LS_INDEX_NONE } },
  /* 19 -- ospfAreaAggregateTable - lower. */
  {  8,  8, { L_LS_INDEX_INADDR, L_LS_INDEX_INADDR, L_LS_INDEX_NONE } },
};

I32_T NETCFG_OM_OSPF_ProcCmp (void *a, void *b);
static void NETCFG_OM_OSPF_IfMasterInit (NETCFG_OM_OSPF_Master_T *om);
static NETCFG_OM_OSPF_Instance_T * NETCFG_OM_OSPF_ProcLookup (NETCFG_OM_OSPF_Master_T *om, UI32_T proc_id);
static void NETCFG_OM_OSPF_IfParamsFree (NETCFG_OM_OSPF_IfParams_T *oip);
static void NETCFG_OM_OSPF_IfParamsClean (NETCFG_OM_OSPF_IfParams_T *oip);
I32_T NETCFG_OM_OSPF_IfParamsTableCmp(void *v1, void *v2);
static void NETCFG_OM_OSPF_IfParamsTableFree (void *arg);
static void NETCFG_OM_OSPF_CryptKeyDeleteAll (struct L_list *crypt);
static UI32_T NETCFG_OM_OSPF_GetInstance(UI32_T vr_id, UI32_T proc_id, NETCFG_OM_OSPF_Instance_T **top);
static NETCFG_OM_OSPF_IfParams_T *NETCFG_OM_OSPF_IfParamsLookupDefault (NETCFG_OM_OSPF_Master_T *om, char *desc);
static NETCFG_OM_OSPF_IfParams_T *NETCFG_OM_OSPF_IfParamsLookup (NETCFG_OM_OSPF_Master_T *om, char *desc, struct L_ls_prefix *p);
static struct L_ls_table *NETCFG_OM_OSPF_IfParamsTableLookupByName (NETCFG_OM_OSPF_Master_T *om, char *desc);
static UI32_T NETCFG_OM_OSPF_GetOspfInterface(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag,
                                                    struct pal_in4_addr addr, NETCFG_OM_OSPF_IfParams_T **oip);
static NETCFG_OM_OSPF_IfParams_T *NETCFG_OM_OSPF_IfParamsGet (NETCFG_OM_OSPF_Master_T *om, char *desc, struct L_ls_prefix *p);
static NETCFG_OM_OSPF_IfParams_T *NETCFG_OM_OSPF_IfParamsGetDefault (NETCFG_OM_OSPF_Master_T *om, char *desc);
static NETCFG_OM_OSPF_CryptKey_T *NETCFG_OM_OSPF_CryptKeyLookup (struct L_list *crypt, UI8_T key_id);

/*Lin.Li part start*/
static NETCFG_TYPE_OSPF_Network_T *NETCFG_OM_OSPF_NetworkMatch (NETCFG_OM_OSPF_Instance_T *top, struct L_ls_prefix *lp);
//static NETCFG_TYPE_OSPF_Area_T *NETCFG_OM_OSPF_AreaLock (NETCFG_TYPE_OSPF_Area_T *area);
//static NETCFG_TYPE_OSPF_Area_T *NETCFG_OM_OSPF_AreaUnlock (NETCFG_TYPE_OSPF_Area_T *area);
//static void NETCFG_OM_OSPF_AreaDelete (NETCFG_TYPE_OSPF_Area_T *area);
static NETCFG_TYPE_OSPF_Area_T *NETCFG_OM_OSPF_AreaNew (NETCFG_OM_OSPF_Instance_T  *top_t, UI32_T area_id);
static NETCFG_TYPE_OSPF_Area_T *NETCFG_OM_OSPF_AreaGet (NETCFG_OM_OSPF_Instance_T  *top_t, UI32_T area_id);
//static int NETCFG_OM_OSPF_NetworkAreaMatch (NETCFG_TYPE_OSPF_Area_T *area);
//static BOOL_T NETCFG_OM_OSPF_AreaEntryDelete (NETCFG_OM_OSPF_Instance_T  *top_t, NETCFG_TYPE_OSPF_Area_T *area);
static BOOL_T NETCFG_OM_OSPF_AreaEntryInsert (NETCFG_OM_OSPF_Instance_T  *top_t, NETCFG_TYPE_OSPF_Area_T *area);
static void NETCFG_OM_OSPF_NetworkEntryInsert (NETCFG_OM_OSPF_Instance_T  *top_t, NETCFG_TYPE_OSPF_Network_T *network, struct L_ls_prefix *lp);
static NETCFG_TYPE_OSPF_Network_T* NETCFG_OM_OSPF_NetworkLookup (NETCFG_OM_OSPF_Instance_T  *top_t, struct L_ls_prefix *lp);
static void NETCFG_OM_OSPF_NetworkDelete (NETCFG_OM_OSPF_Instance_T  *top_t, NETCFG_TYPE_OSPF_Network_T *network);
static NETCFG_TYPE_OSPF_Area_T *NETCFG_OM_OSPF_AreaEntryLookup (NETCFG_OM_OSPF_Instance_T  *top_t, UI32_T area_id);
//static NETCFG_TYPE_OSPF_Area_Range_T *NETCFG_OM_OSPF_AreaRangeLookup(NETCFG_TYPE_OSPF_Area_T  *area, struct L_ls_prefix *lp);
//static NETCFG_TYPE_OSPF_Area_Range_T *NETCFG_OM_OSPF_AreaRangeMatch (NETCFG_TYPE_OSPF_Area_T  *area, struct L_ls_prefix *lp);
//static NETCFG_TYPE_OSPF_Area_Range_T *NETCFG_OM_OSPF_AreaRangeNew (NETCFG_TYPE_OSPF_Area_T *area);
//static void NETCFG_OM_OSPF_AreaRangeFree (NETCFG_TYPE_OSPF_Area_Range_T *range);
//static void NETCFG_OM_OSPF_AreaRangeAdd (NETCFG_TYPE_OSPF_Area_T *area,NETCFG_TYPE_OSPF_Area_Range_T *range, struct L_ls_prefix *lp);
//static void NETCFG_OM_OSPF_AreaRangeDelete (NETCFG_TYPE_OSPF_Area_T *area,NETCFG_TYPE_OSPF_Area_Range_T *range);
//static void NETCFG_OM_OSPF_AreaRangeDeleteAll (NETCFG_TYPE_OSPF_Area_T *area);
//static UI32_T NETCFG_OM_OSPF_AreaRangeEntryInsert (NETCFG_TYPE_OSPF_Area_T *area, NETCFG_TYPE_OSPF_Area_Range_T *range);
//static UI32_T NETCFG_OM_OSPF_AreaRangeEntryDelete (NETCFG_TYPE_OSPF_Area_T *area, NETCFG_TYPE_OSPF_Area_Range_T *range);
static NETCFG_TYPE_OSPF_Passive_If_T *NETCFG_OM_OSPF_PassiveIfLookUp(UI32_T vr_id, UI32_T proc_id, UI32_T *ifindex, UI32_T *addr);
static I32_T NETCFG_OM_OSPF_PassIfCmp (void *a, void *b);
/*Lin.Li part end*/

static BOOL_T NETCFG_OM_OSPF_MasterInit(UI32_T vr_id);
static BOOL_T NETCFG_OM_OSPF_VrfInit(UI32_T vrf_id);
static BOOL_T NETCFG_OM_OSPF_GetMaster(UI32_T vr_id,NETCFG_OM_OSPF_Master_T **om);
static BOOL_T NETCFG_OM_OSPF_GetVrf(UI32_T vrf_id,NETCFG_OM_OSPF_Vrf_T **ov);

static UI32_T ospf_om_sem_id;   
static NETCFG_OM_OSPF_Master_T *ospf_master[MAX_NBR_OF_VR];/*OSPF database in om*/
static NETCFG_OM_OSPF_Vrf_T *ospf_vrf[MAX_NBR_OF_VRF];/*OSPF database in ov*/

static struct L_ls_prefix LsPrefixIPv4Default;
static struct pal_in4_addr IPv4AddressUnspec;



/* FUNCTION NAME : NETCFG_OM_OSPF_Init
 * PURPOSE:Init NETCFG_OM_OSPF database, create semaphore
 *
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void NETCFG_OM_OSPF_Init(void)
{
    UI32_T i;

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NETCFG_OM, &ospf_om_sem_id) != SYSFUN_OK)
    {
        printf("NETCFG_OM_OSPF_Init : Can't create semaphore\n");
    }

    /*Init ospf master*/
    for(i = 0; i < MAX_NBR_OF_VR; i++)
        ospf_master[i] = NULL;
    if(NETCFG_OM_OSPF_MasterInit(SYS_DFLT_VR_ID) == FALSE)
    {
        /* DEBUG */
        printf("NETCFG_OM_OSPF_Init failed.\n");
        SYSFUN_LogDebugMsg("NETCFG_OM_OSPF_Init : Can't create OSPF List.\n");
    }

    /*Init ospf vrf*/
    for(i = 0; i < MAX_NBR_OF_VRF; i++)
        ospf_vrf[i] = NULL;
    if(NETCFG_OM_OSPF_VrfInit(SYS_DFLT_VRF_ID) == FALSE)
    {
        /* DEBUG */
        printf("NETCFG_OM_OSPF_Init failed.\n");
        SYSFUN_LogDebugMsg("NETCFG_OM_OSPF_Init : Can't create OSPF List.\n");
    }
    
    IPv4AddressUnspec.s_addr = pal_ntoh32 (IPV4_ADDRESS_UNSPEC);
    L_ls_prefix_ipv4_set (&LsPrefixIPv4Default, 0, IPv4AddressUnspec);
}

/* FUNCTION NAME : NETCFG_OM_OSPF_MasterInit
 * PURPOSE:
 *      Init OSPF Master.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
static BOOL_T NETCFG_OM_OSPF_MasterInit(UI32_T vr_id)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_OSPF_Master_T *om = NULL;

    om = (NETCFG_OM_OSPF_Master_T *)malloc(sizeof(NETCFG_OM_OSPF_Master_T));
    if(om != NULL)
    {
        memset(om, 0, sizeof(NETCFG_OM_OSPF_Master_T));
        om->vr_id = vr_id;
        om->ospf = L_list_new ();
        om->ospf->cmp = NETCFG_OM_OSPF_ProcCmp;
        NETCFG_OM_OSPF_IfMasterInit(om);
        
        original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
        ospf_master[vr_id] = om;
        result = TRUE;        
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    }
    return  result;
}

/* Initialize OSPF interface master for system wide configurations. */
static void NETCFG_OM_OSPF_IfMasterInit (NETCFG_OM_OSPF_Master_T *om)
{
  /* Initialize ospf interface parameter list. */
  om->if_params = L_list_new ();
  om->if_params->cmp = NETCFG_OM_OSPF_IfParamsTableCmp;
  om->if_params->del = NETCFG_OM_OSPF_IfParamsTableFree;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_VrfInit
 * PURPOSE:
 *      Init OSPF Vrf.
 *
 * INPUT:
 *      vrf_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
static BOOL_T NETCFG_OM_OSPF_VrfInit(UI32_T vrf_id)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_OSPF_Vrf_T *ov = NULL;


    ov = (NETCFG_OM_OSPF_Vrf_T *)malloc(sizeof(NETCFG_OM_OSPF_Vrf_T));
    if(ov != NULL)
    {
        memset(ov, 0, sizeof(NETCFG_OM_OSPF_Vrf_T));
        ov->vrf_id = vrf_id;
        ov->redist_table = L_ls_table_init (L_LS_IPV4_ROUTE_TABLE_DEPTH, 1);
        ov->ospf = L_list_new ();
        ov->ospf->cmp = NETCFG_OM_OSPF_ProcCmp;

        original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
        ospf_vrf[vrf_id] = ov;
        result = TRUE;        
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    }
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_DeleteAllOspfMasterEntry
 * PURPOSE:
 *          Remove all OSPF master entries.
 *
 * INPUT:  None.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES:  None.
 */
void NETCFG_OM_OSPF_DeleteAllOspfMasterEntry(void)
{
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    #if 0
    for(i=0;i<MAX_NBR_OF_VR;i++)
    {
        if(ospf_master[i] != NULL)
        {
            for(j=0;j<MAX_NBR_OF_VRF_PER_VR;j++)
            {
                if(ospf_master[i]->ospf[j] != NULL) 
                {
                    NETCFG_OM_OSPF_DeleteProcess(i, j);
                }
            }
            L_SORT_LST_Delete_All(&(ospf_master[i]->rip_ifs));
            free(ospf_master[i]);
            ospf_master[i] = NULL;
        }
    }
    #endif
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
}

/* FUNCTION NAME : NETCFG_OM_OSPF_ProcCmp
 * PURPOSE:
 *      Compare two ospf process id.
 *
 * INPUT:
 *      void *a
 *      void *b
 *
 * OUTPUT:
 *     
 *
 * RETURN: 
 *
 * NOTES:
 *     
 */
I32_T NETCFG_OM_OSPF_ProcCmp (void *a, void *b)
{
  NETCFG_OM_OSPF_Instance_T *t1 = (NETCFG_OM_OSPF_Instance_T *)a;
  NETCFG_OM_OSPF_Instance_T *t2 = (NETCFG_OM_OSPF_Instance_T *)b;

  return t1->ospf_id - t2->ospf_id;
}



static void NETCFG_OM_OSPF_CryptKeyDeleteAll (struct L_list *crypt)
{
    struct L_listnode *node, *next;

    for (node = L_LISTHEAD (crypt); node; node = next)
    {
        next = node->next;
        free(node->data);
        L_list_delete_node (crypt, node);
    }
}

int NETCFG_OM_OSPF_IfDisableAllGet (NETCFG_OM_OSPF_Master_T *om, char *ifname)
{
    NETCFG_OM_OSPF_IfParams_T *oip;
  
    oip = NETCFG_OM_OSPF_IfParamsLookupDefault(om, ifname);
    if (oip)
        return NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, DISABLE_ALL);
  
    return 0;
}

I32_T NETCFG_OM_OSPF_IfParamsTableCmp(void *v1, void *v2)
{
  struct L_ls_table *rt1 = v1, *rt2 = v2;

  if (!rt1->desc)
    return 1;

  if (!rt2->desc)
    return 0;

  return strcmp (rt1->desc, rt2->desc);
}

static void NETCFG_OM_OSPF_IfParamsTableFree (void *arg)
{
    NETCFG_OM_OSPF_IfParams_T *oip;
    struct L_ls_table *rt;
    struct L_ls_node *rn;

    rt = arg;
    for (rn = L_ls_table_top (rt); rn; rn = L_ls_route_next (rn))
        if ((oip = RN_INFO (rn, RNI_DEFAULT)))
        {
	        NETCFG_OM_OSPF_IfParamsFree(oip);
	        RN_INFO_UNSET (rn, RNI_DEFAULT);
        }

    if (rt->desc)
        free (rt->desc);

    L_ls_table_finish (rt); 
    rt = NULL;
}

static void NETCFG_OM_OSPF_IfParamsClean (NETCFG_OM_OSPF_IfParams_T *oip)
{
    oip->desc = NULL;
    
    if (oip->auth_crypt)
    {
        NETCFG_OM_OSPF_CryptKeyDeleteAll (oip->auth_crypt);
        L_list_delete (oip->auth_crypt);
    }

    oip->nbrs = NULL;/*implement in the future*/
}

static void NETCFG_OM_OSPF_IfParamsFree (NETCFG_OM_OSPF_IfParams_T *oip)
{
    NETCFG_OM_OSPF_IfParamsClean (oip);
    free(oip);
}

static void NETCFG_OM_OSPF_IfParamsReset (NETCFG_OM_OSPF_IfParams_T *oip)
{
    NETCFG_OM_OSPF_IfParamsClean(oip);
  
    /* Initialize flags.  */
    oip->config = 0;
  
    /* Sanity cleanup.  */
    oip->type = NETCFG_OM_OSPF_IFTYPE_NONE;
    oip->priority = NETCFG_OM_OSPF_ROUTER_PRIORITY_DEFAULT;
    oip->output_cost = NETCFG_OM_OSPF_OUTPUT_COST_DEFAULT;
    oip->hello_interval = NETCFG_OM_OSPF_IF_PARAM_HELLO_INTERVAL_DEFAULT(oip);
    oip->dead_interval = NETCFG_OM_OSPF_IF_PARAM_DEAD_INTERVAL_DEFAULT (oip);
    oip->transmit_delay = NETCFG_OM_OSPF_TRANSMIT_DELAY_DEFAULT;
    oip->retransmit_interval = NETCFG_OM_OSPF_RETRANSMIT_INTERVAL_DEFAULT;
    oip->auth_type = NETCFG_TYPE_OSPF_AUTH_NULL;
    oip->mtu = 0;
    oip->auth_crypt = NULL;
}

static NETCFG_OM_OSPF_IfParams_T *NETCFG_OM_OSPF_IfParamsLookupDefault (NETCFG_OM_OSPF_Master_T *om, char *desc)
{
    return NETCFG_OM_OSPF_IfParamsLookup(om, desc,
				(struct L_ls_prefix *)&LsPrefixIPv4Default);
}

static NETCFG_OM_OSPF_IfParams_T *NETCFG_OM_OSPF_IfParamsLookup (NETCFG_OM_OSPF_Master_T *om, char *desc, struct L_ls_prefix *p)
{
    struct L_ls_table *rt;
    struct L_ls_node *rn;
  
    rt = NETCFG_OM_OSPF_IfParamsTableLookupByName(om, desc);
    if (rt)
    {
        rn = L_ls_node_lookup (rt, p);
        if (rn)
  	    {
  	        L_ls_unlock_node (rn);
  	        return RN_INFO (rn, RNI_DEFAULT);
  	    }
    }
    return NULL;
}

static struct L_ls_table *NETCFG_OM_OSPF_IfParamsTableLookupByName (NETCFG_OM_OSPF_Master_T *om, char *desc)
{
    struct L_listnode *node;
  
    for (node = L_LISTHEAD (om->if_params); node; L_NEXTNODE (node))
    {
        struct L_ls_table *rt = L_GETDATA (node);
        if (rt->desc)
  	        if (strcmp(rt->desc, desc) == 0)
  	            return rt;
    }
    return NULL;
}

static struct L_ls_table *NETCFG_OM_OSPF_IfParamsTableNew (char *desc)
{
    struct L_ls_table *new;
  
    new = L_ls_table_init (L_LS_IPV4_ROUTE_TABLE_DEPTH, 1);
    new->desc = malloc(strlen(desc) + 1);
    strncpy (new->desc, desc, strlen (desc) + 1);
  
    return new;
}

static struct L_ls_table *NETCFG_OM_OSPF_IfParamsTableGet (NETCFG_OM_OSPF_Master_T *om, char *desc)
{
    struct L_ls_table *rt;
  
    rt = NETCFG_OM_OSPF_IfParamsTableLookupByName(om, desc);
    if (rt)
      return rt;
  
    rt = NETCFG_OM_OSPF_IfParamsTableNew(desc);
  
    L_listnode_add_sort (om->if_params, rt);
  
    return rt;
}

static void NETCFG_OM_OSPF_IfParamsTableDelete (NETCFG_OM_OSPF_Master_T *om, char *desc)
{
    struct L_ls_table *rt;
  
    rt = NETCFG_OM_OSPF_IfParamsTableLookupByName(om, desc);
    if (!rt)
        return;
  
    L_listnode_delete (om->if_params, rt);
  
}

static NETCFG_OM_OSPF_IfParams_T *NETCFG_OM_OSPF_IfParamsGet (NETCFG_OM_OSPF_Master_T *om, char *desc, struct L_ls_prefix *p)
{
    struct L_ls_table *rt;
    struct L_ls_node *rn;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;
  
    rt = NETCFG_OM_OSPF_IfParamsTableGet(om, desc);
    rn = L_ls_node_get (rt, p);
    if ((RN_INFO (rn, RNI_DEFAULT)) == NULL)
    {
        oip = (NETCFG_OM_OSPF_IfParams_T *)malloc(sizeof(NETCFG_OM_OSPF_IfParams_T));
        if(oip != NULL)
        {
            memset(oip,0,sizeof(NETCFG_OM_OSPF_IfParams_T));
            oip->vr_id = om->vr_id;
            RN_INFO_SET (rn, RNI_DEFAULT, oip);
            oip->desc = rt->desc;
            NETCFG_OM_OSPF_IfParamsReset(oip);
        }        
        else
        {
            L_ls_unlock_node (rn);
            return NULL;
        }
        
    }
    L_ls_unlock_node (rn);
  
    return RN_INFO (rn, RNI_DEFAULT);
}

static NETCFG_OM_OSPF_IfParams_T *NETCFG_OM_OSPF_IfParamsGetDefault (NETCFG_OM_OSPF_Master_T *om, char *desc)
{
    NETCFG_OM_OSPF_IfParams_T *oip;
  
    oip = NETCFG_OM_OSPF_IfParamsGet(om, desc,
  			    (struct L_ls_prefix *)&LsPrefixIPv4Default);
    
    return oip;
}

static void NETCFG_OM_OSPF_IfParamsDelete (NETCFG_OM_OSPF_Master_T *om, char *desc, struct L_ls_prefix *p)
{
    struct L_ls_table *rt;
    struct L_ls_node *rn;
  
    rt = NETCFG_OM_OSPF_IfParamsTableLookupByName(om, desc);
    if (rt)
    {
        rn = L_ls_node_lookup (rt, p);
        if (rn)
        {
            NETCFG_OM_OSPF_IfParamsFree(RN_INFO (rn, RNI_DEFAULT));
            RN_INFO_UNSET (rn, RNI_DEFAULT);
            L_ls_unlock_node (rn);

            if (rt->top == NULL)
                NETCFG_OM_OSPF_IfParamsTableDelete(om, desc);
        }
    }
}

static void NETCFG_OM_OSPF_IfParamsDeleteDefault (NETCFG_OM_OSPF_Master_T *om, char *desc)
{
    NETCFG_OM_OSPF_IfParamsDelete(om, desc, (struct L_ls_prefix *)&LsPrefixIPv4Default);
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetMaster
 * PURPOSE:
 *      Get a ospf master entry with specific vr_id.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
static BOOL_T NETCFG_OM_OSPF_GetMaster(UI32_T vr_id,NETCFG_OM_OSPF_Master_T **om)
{
    BOOL_T     result = FALSE;

    if(ospf_master[vr_id] != NULL)
    {
        *om = ospf_master[vr_id];
        result = TRUE;
    }
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetVrf
 * PURPOSE:
 *      Get a ospf vrf entry with specific vrf_id.
 *
 * INPUT:
 *      vrf_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
static BOOL_T NETCFG_OM_OSPF_GetVrf(UI32_T vrf_id,NETCFG_OM_OSPF_Vrf_T **ov)
{
    BOOL_T     result = FALSE;

    if(ospf_vrf[vrf_id] != NULL)
    {
        *ov = ospf_vrf[vrf_id];
        result = TRUE;
    }
    return  result;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetInstance
 * PURPOSE:
 *      Get a ospf Instance entry with specific vr_id and proc_id.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
static UI32_T NETCFG_OM_OSPF_GetInstance(UI32_T vr_id, UI32_T proc_id, NETCFG_OM_OSPF_Instance_T **top)
{       
    NETCFG_OM_OSPF_Master_T    *om = NULL;

    
    if(NETCFG_OM_OSPF_GetMaster(vr_id,&om) != TRUE)
    {
        return NETCFG_TYPE_OSPF_MASTER_NOT_EXIST;
    }

    *top = NETCFG_OM_OSPF_ProcLookup(om, proc_id);
    if(*top == NULL)
    {
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetInstanceEntry
 * PURPOSE:
 *      Get a ospf Instance entry with specific vr_id and proc_id.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
UI32_T NETCFG_OM_OSPF_GetInstanceEntry(UI32_T vr_id, UI32_T proc_id, NETCFG_OM_OSPF_Instance_T *top)
{    
    UI32_T original_priority;
    NETCFG_OM_OSPF_Master_T    *om = NULL;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetMaster(vr_id,&om) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OSPF_MASTER_NOT_EXIST;
    }

    top_t = NETCFG_OM_OSPF_ProcLookup(om, proc_id);
    if(top_t == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    memcpy(top, top_t, sizeof(NETCFG_OM_OSPF_Instance_T));
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetOspfInterface
 * PURPOSE:
 *      Get a OSPF interface.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *      oip
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
static UI32_T NETCFG_OM_OSPF_GetOspfInterface(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag,
                                                    struct pal_in4_addr addr, NETCFG_OM_OSPF_IfParams_T **oip)
{    
    NETCFG_OM_OSPF_Master_T    *om = NULL;
    struct L_ls_prefix p;
    char    ifname[NETCFG_NETDEVICE_IFNAME_SIZE + 1] = {0};
    
    NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname);
    
    if(NETCFG_OM_OSPF_GetMaster(vr_id, &om) != TRUE)
    {
        return NETCFG_TYPE_OSPF_MASTER_NOT_EXIST;
    }

    if(addr_flag == TRUE)
    {
        L_ls_prefix_ipv4_set (&p, IPV4_MAX_BITLEN, addr);
     
        *oip = NETCFG_OM_OSPF_IfParamsLookup(om, ifname, &p);
    }
    else
    {
        *oip = NETCFG_OM_OSPF_IfParamsLookupDefault(om, ifname);
    }
    
    if(*oip == NULL)
    {
        return NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST;
    }

    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetOspfInterfaceEntry
 * PURPOSE:
 *      Get a OSPF interface.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *      oip
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
UI32_T NETCFG_OM_OSPF_GetOspfInterfaceEntry(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag,
                                                    struct pal_in4_addr addr, NETCFG_OM_OSPF_IfParams_T *oip)
{    
    UI32_T original_priority;
    NETCFG_OM_OSPF_Master_T    *om = NULL;
    NETCFG_OM_OSPF_IfParams_T *oip_t = NULL;
    struct L_ls_prefix p;
    char    ifname[NETCFG_NETDEVICE_IFNAME_SIZE + 1] = {0};
    
    NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname);
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    if(NETCFG_OM_OSPF_GetMaster(vr_id, &om) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OSPF_MASTER_NOT_EXIST;
    }

    if(addr_flag == TRUE)
    {
        L_ls_prefix_ipv4_set (&p, IPV4_MAX_BITLEN, addr);
        oip_t = NETCFG_OM_OSPF_IfParamsLookup(om, ifname, &p);
    }
    else
    {
        oip_t = NETCFG_OM_OSPF_IfParamsLookupDefault(om, ifname);
    }
    
    if(oip_t == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST;
    }

    memcpy(oip, oip_t, sizeof(NETCFG_OM_OSPF_IfParams_T));
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetCryptKeyEntry
 * PURPOSE:
 *      Get crypt key entry .
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
UI32_T NETCFG_OM_OSPF_GetCryptKeyEntry(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag,
                                                    struct pal_in4_addr addr, NETCFG_OM_OSPF_CryptKey_T *ck)
{    
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;
    NETCFG_OM_OSPF_CryptKey_T * ck_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(oip != NULL)
    {
        if(oip->auth_crypt != NULL)
        {
            ck_t = NETCFG_OM_OSPF_CryptKeyLookup (oip->auth_crypt, key_id);
            if (ck_t != NULL)
            {
                memcpy(ck , ck_t, sizeof(NETCFG_OM_OSPF_CryptKey_T));
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
	            return NETCFG_TYPE_OK;
            }
        }
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST;
    }  
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_FAIL;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetRunningIfEntryByIfindex
* PURPOSE:
*     Get ospf interface config information by ifindex.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_GetRunningIfEntryByIfindex(UI32_T vr_id, NETCFG_TYPE_OSPF_IfConfig_T *entry)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Master_T    *om = NULL;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;
    struct L_ls_prefix p;
    char    ifname[NETCFG_NETDEVICE_IFNAME_SIZE + 1] = {0};
    struct pal_in4_addr address;
    NETCFG_OM_OSPF_CryptKey_T *ck;
    struct L_listnode *node;
    
    NETCFG_NETDEVICE_IfindexToIfname(entry->ifindex, ifname);
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    if(NETCFG_OM_OSPF_GetMaster(vr_id, &om) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OSPF_MASTER_NOT_EXIST;
    }

    if(!entry->default_flag)
    {
        address.s_addr = entry->ip_address;
        L_ls_prefix_ipv4_set (&p, IPV4_MAX_BITLEN, address);
        oip = NETCFG_OM_OSPF_IfParamsLookup(om, ifname, &p);
    }
    else
    {
        oip = NETCFG_OM_OSPF_IfParamsLookupDefault(om, ifname);
    }
    
    if(oip == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST;
    }

    memset(entry, 0, sizeof(NETCFG_TYPE_OSPF_IfConfig_T));
    entry->ip_address = oip->ident.address.u.prefix4.s_addr;
    entry->mask_len = oip->ident.address.prefixlen;	  
    entry->ifindex = oip->ifindex;

    /* Interface disable all.  */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, DISABLE_ALL))
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_DISABLE_ALL);

    /* Interface Network .  */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, NETWORK_TYPE))
    {
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_NETWORK_TYPE);
        entry->network_type = oip->network_type;
    }      

    /* OSPF interface authentication . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, AUTH_TYPE))
    {
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_AUTH_TYPE);
        entry->auth_type = oip->auth_type;
    } 
     
    /* Simple Authentication Password . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, AUTH_SIMPLE))
    {
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_AUTH_SIMPLE);
        memcpy(entry->auth_simple, oip->auth_simple, NETCFG_TYPE_OSPF_AUTH_SIMPLE_SIZE);
    } 
      
    /* Cryptographic Authentication Key . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, AUTH_CRYPT))
    {   
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_AUTH_CRYPT);
        L_LIST_LOOP (oip->auth_crypt, ck, node)
        {
            memcpy(entry->auth_crypt[ck->key_id], ck->auth_key, NETCFG_TYPE_OSPF_AUTH_MD5_SIZE);
        } 
    }

    /* Interface Output Cost . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, OUTPUT_COST))
    {
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_OUTPUT_COST);
        entry->output_cost = oip->output_cost;
    } 
      
    /* Hello Interval . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, HELLO_INTERVAL))
    {
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_HELLO_INTERVAL);
        entry->hello_interval = oip->hello_interval;
    } 
      
    /* Router Dead Interval . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, DEAD_INTERVAL))
    {
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_DEAD_INTERVAL);
        entry->dead_interval = oip->dead_interval;
    } 
      
    /* Router Priority . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, PRIORITY))
    {
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_PRIORITY);
        entry->priority = oip->priority;
    } 
    
#ifdef HAVE_RESTART
    /* Resync timeout. */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, RESYNC_TIMEOUT))
    {
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_RESYNC_TIMEOUT);
        entry->resync_timeout = oip->resync_timeout;
    } 
#endif /* HAVE_RESTART */

    /* MTU . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, MTU))
    {
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_MTU);
        entry->mtu = oip->mtu;
    }      

    /* MTU ignore . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, MTU_IGNORE))
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_MTU_IGNORE);

    /* Retransmit Interval . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, RETRANSMIT_INTERVAL))
    {
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_RETRANSMIT_INTERVAL);
        entry->retransmit_interval = oip->retransmit_interval;
    } 	      

    /* Transmit Delay . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, TRANSMIT_DELAY))
    {
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_TRANSMIT_DELAY);
        entry->transmit_delay = oip->transmit_delay;
    } 
      
    /* Database Filter . */
    if (NETCFG_OM_OSPF_IF_PARAM_CHECK (oip, DATABASE_FILTER))
        SET_FLAG(entry->config, NETCFG_TYPE_OSPF_IF_PARAM_DATABASE_FILTER);
	  
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_CheckCryptKeyExist
 * PURPOSE:
 *      Check if crypt key exist .
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
UI32_T NETCFG_OM_OSPF_CheckCryptKeyExist(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag,
                                                    struct pal_in4_addr addr)
{    
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(oip != NULL)
    {
        if(oip->auth_crypt != NULL)
        {
            if (NETCFG_OM_OSPF_CryptKeyLookup (oip->auth_crypt, key_id) != NULL)
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
	            return NETCFG_TYPE_OSPF_MD5_KEY_EXIST;
            }
        }
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST;
    }  
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

static NETCFG_OM_OSPF_CryptKey_T *NETCFG_OM_OSPF_CryptKeyLookup (struct L_list *crypt, UI8_T key_id)
{
    struct L_listnode *node;
    NETCFG_OM_OSPF_CryptKey_T *ck;
  
    for (node = L_LISTHEAD (crypt); node; L_NEXTNODE (node))
    {
        ck = L_GETDATA (node);
        if (ck->key_id == key_id)
            return ck;
    }
  
    return NULL;
}

static I32_T NETCFG_OM_OSPF_CryptKeyCmp (void *v1, void *v2)
{
    NETCFG_OM_OSPF_CryptKey_T *ck1 = v1, *ck2 = v2;
    return ck1->key_id - ck2->key_id;
}

static void NETCFG_OM_OSPF_CryptKeyDelete (struct L_list *crypt, UI8_T key_id)
{
    NETCFG_OM_OSPF_CryptKey_T *ck;
  
    ck = NETCFG_OM_OSPF_CryptKeyLookup (crypt, key_id);
  
    if (ck)
    {
        L_listnode_delete (crypt, ck);
        free(ck);
    }
}

static NETCFG_OM_OSPF_Instance_T * NETCFG_OM_OSPF_ProcLookup (NETCFG_OM_OSPF_Master_T *om, UI32_T proc_id)
{
  NETCFG_OM_OSPF_Instance_T *top;
  struct L_listnode *node;

  L_LIST_LOOP (om->ospf, top, node)
    if (top->ospf_id == proc_id)
      return top;

  return NULL;
}



u_int8_t NETCFG_OM_OSPF_IfPriorityGet (NETCFG_OM_OSPF_IfParams_T *oip, NETCFG_OM_OSPF_IfParams_T *oip_def)
{
    if (oip && (oip->config & NETCFG_OM_OSPF_IF_PARAM_PRIORITY))
      return oip->priority;
    if (oip_def && (oip_def->config & NETCFG_OM_OSPF_IF_PARAM_PRIORITY))
      return oip_def->priority;
  
    return NETCFG_OM_OSPF_ROUTER_PRIORITY_DEFAULT;
}


static BOOL_T NETCFG_OM_OSPF_IfInit (NETCFG_OM_OSPF_Master_T *om, NETCFG_OM_OSPF_IfParams_T *oip,
	      struct prefix *address, struct prefix *destination)
{  
    /* Set the local IP address.  */
    if (oip->network_type == NETCFG_OM_OSPF_IFTYPE_VIRTUALLINK)
        oip->ident.address.family = AF_INET;
    else
        memcpy(&oip->ident.address, address, sizeof(struct prefix));
  
    /* Set the remote IP address.  */
    if (oip->network_type == NETCFG_OM_OSPF_IFTYPE_VIRTUALLINK)
        oip->destination.family = AF_INET;
    else 
        memcpy(&oip->destination, destination, sizeof(struct prefix));
  
  
    /* Initialize the neighbor table.  */
    oip->nbrs = L_ls_table_init (L_LS_IPV4_ROUTE_TABLE_DEPTH, 1);
   
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_OspfInterfaceAdd
 * PURPOSE:
 *      Add ospf-interface .
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_OspfInterfaceAdd(UI32_T vr_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Master_T    *om = NULL;
    NETCFG_OM_OSPF_Instance_T  *top_t = NULL;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;
    NETCFG_OM_OSPF_IfParams_T *oip_def = NULL;
    NETCFG_TYPE_OSPF_Network_T *network = NULL;
    struct L_listnode *node;
    struct L_ls_prefix p,q;
    struct prefix address;
    struct prefix destination;
    char    ifname[NETCFG_NETDEVICE_IFNAME_SIZE + 1] = {0};
    int prefixlen,i;
    
    NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname);

      /*count subnet length*/
    prefixlen = 0;
    for(i=0;i<32;i++)
    {
        if(ip_mask &(1<<i))
        {    
            prefixlen++;
        }
    }
    address.family = AF_INET;
    address.u.prefix4.s_addr = ip_addr;
    address.prefixlen = prefixlen;
    
    //ifc->flags = msg->flags;

	destination.family = AF_INET;
    destination.prefixlen = prefixlen;
    destination.u.prefix4.s_addr = ip_addr|(~ip_mask);

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetMaster(vr_id,&om) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }
    
    L_ls_prefix_ipv4_set (&p, IPV4_MAX_BITLEN, address.u.prefix4);
    oip = NETCFG_OM_OSPF_IfParamsGet(om, ifname, &p);
    if(oip != NULL)
    {      
        oip->clone = 1;
        /* Set references. */
        
        oip->ifindex = ifindex;
      
        /* Initialize the interface.  */
        NETCFG_OM_OSPF_IfInit(om, oip, &address, &destination);
        
        oip_def = NETCFG_OM_OSPF_IfParamsLookupDefault(om, ifname);
        oip->ident.priority = NETCFG_OM_OSPF_IfPriorityGet(oip, oip_def);

         /* Check 'ip ospf disable all'. */
        if (NETCFG_OM_OSPF_IfDisableAllGet (om, ifname))
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
            return TRUE;
        }

        if (om != NULL)
            L_LIST_LOOP (om->ospf, top_t, node)
            {
	            L_ls_prefix_ipv4_set (&q, address.prefixlen, address.u.prefix4);
	            if ((network = NETCFG_OM_OSPF_NetworkMatch(top_t, &q)))
                {  
                    oip->ospf_id = top_t->ospf_id;
                    oip->ident.router_id = top_t->router_id_config;
                }
            }
        
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return TRUE;   
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }   
 
}

/* FUNCTION NAME : NETCFG_OM_OSPF_OspfInterfaceAdd
 * PURPOSE:
 *      Add default ospf -interface .
 *
 * INPUT:
 *      vr_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_DefaultOspfInterfaceAdd(UI32_T vr_id, UI32_T ifindex)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Master_T    *om = NULL;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;
    char    ifname[NETCFG_NETDEVICE_IFNAME_SIZE + 1] = {0};
    
    NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname);

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetMaster(vr_id,&om) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }
    
    oip = NETCFG_OM_OSPF_IfParamsGetDefault(om, ifname);
    if(oip != NULL)
    {    
        oip->ifindex = ifindex;
        
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return TRUE;    
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }
 
}

BOOL_T NETCFG_OM_OSPF_OspfInterfaceDelete (UI32_T vr_id, UI32_T ifindex, UI32_T ip_addr)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Master_T    *om = NULL;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;
    struct L_ls_prefix p;
    struct pal_in4_addr addr;
    char    ifname[NETCFG_NETDEVICE_IFNAME_SIZE + 1] = {0};
    
    NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname);
    addr.s_addr = ip_addr;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetMaster(vr_id, &om) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }
    
    L_ls_prefix_ipv4_set (&p, IPV4_MAX_BITLEN, addr);
     
    oip = NETCFG_OM_OSPF_IfParamsLookup(om, ifname, &p);
   
    if (oip != NULL)
    {
       NETCFG_OM_OSPF_IfParamsDelete(om, ifname, &p);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

BOOL_T NETCFG_OM_OSPF_DefaultOspfInterfaceDelete(UI32_T vr_id, UI32_T ifindex)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Master_T    *om = NULL;
    char    ifname[NETCFG_NETDEVICE_IFNAME_SIZE + 1] = {0};
    
    NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname);

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetMaster(vr_id,&om) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }
    
    NETCFG_OM_OSPF_IfParamsDeleteDefault(om, ifname);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_PassIfCmp
 * PURPOSE:
 *      Compare two ospf passive if.
 *
 * INPUT:
 *      void *a
 *      void *b
 *
 * OUTPUT:
 *     
 *
 * RETURN: 
 *
 * NOTES:
 *     
 */
static I32_T NETCFG_OM_OSPF_PassIfCmp (void *a, void *b)
{
  NETCFG_TYPE_OSPF_Passive_If_T *element1 = (NETCFG_TYPE_OSPF_Passive_If_T *)a;
  NETCFG_TYPE_OSPF_Passive_If_T *element2 = (NETCFG_TYPE_OSPF_Passive_If_T *)b;

  if (element1->ifindex > element2->ifindex)
  {
      return (1);
  }
  else if (element1->ifindex < element2->ifindex)
  {
      return (-1);
  }
  else
  {
      if (element1->addr > element2->addr)
      {
          return (1);
      }
      else if (element1->addr < element2->addr)
      {
          return (-1);
      }
      else
          return (0);
  }
}

/* FUNCTION NAME : NETCFG_OM_OSPF_InstanceAdd
 * PURPOSE:
 *      Add a OSPF instance entry when router ospf enable .
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *      proc_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_InstanceAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id)
{
    BOOL_T result = FALSE;
    UI32_T original_priority;
    NETCFG_OM_OSPF_Master_T    *om = NULL;
    NETCFG_OM_OSPF_Vrf_T    *ov = NULL;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetMaster(vr_id,&om) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(NETCFG_OM_OSPF_GetVrf(vrf_id,&ov) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    top = (NETCFG_OM_OSPF_Instance_T *)malloc(sizeof(NETCFG_OM_OSPF_Instance_T));

    if(top != NULL)
    {
        memset(top,0,sizeof(NETCFG_OM_OSPF_Instance_T));
        top->ospf_id = proc_id;
        top->vr_id = vr_id;
        top->vrf_id = vrf_id;

        UNSET_FLAG (top->config, NETCFG_OM_OSPF_CONFIG_RFC1583_COMPATIBLE);

        #if 0
        /* Initialize tables related to internal objects. */
        top->area_table = L_ls_table_init (NETCFG_OM_OSPF_AREA_TABLE_DEPTH, 1);
        #endif
        /* Initialize tables ralated to API. */
  
        /* Initilaize tables for configuration. */
        top->networks = L_ls_table_init (L_LS_IPV4_ROUTE_TABLE_DEPTH, 1);

        /* Initialize Routing tables. */

  
        /* Passive interface. */
        top->passive_if = L_list_new ();
        top->passive_if->cmp = NETCFG_OM_OSPF_PassIfCmp;
        /* Initialize summary addresses. */
        top->summary = L_ls_table_init (L_LS_IPV4_ROUTE_TABLE_DEPTH, 1);

        top->default_metric = -1;
        top->ref_bandwidth = OSPF_DEFAULT_REF_BANDWIDTH;
  
        /* LSDB limit init. */
  
        /* maximum area limit */
        top->max_area_limit = NETCFG_OM_OSPF_AREA_LIMIT_DEFAULT;
  
        /* SPF timer value init. */
        top->timer.delay = SYS_DFLT_OSPF_SPF_DELAY_DEFAULT;
        top->timer.hold = SYS_DFLT_OSPF_SPF_HOLDTIME_DEFAULT;
  
        /* Distance table init. */
  
        /* LSA timer init. */

         /* Add the process to the global list.  */
        L_listnode_add_sort (om->ospf, top);
        L_listnode_add (ov->ospf, top);
        
        result = TRUE;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);

    return  result;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_InstanceDelete
 * PURPOSE:
 *      Delete a OSPF instance entry when router ospf disable .
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *      proc_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_InstanceDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Master_T    *om = NULL;
    NETCFG_OM_OSPF_Vrf_T    *ov = NULL;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;
    struct L_ls_node * rn;
    NETCFG_TYPE_OSPF_Network_T *network_entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetMaster(vr_id,&om) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(NETCFG_OM_OSPF_GetVrf(vrf_id,&ov) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    top = NETCFG_OM_OSPF_ProcLookup(om, proc_id);
    if(top == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return TRUE;
    }
    /*there are many other table or structure need destroy, we can add it in the future -- xiongyu */
    /* Clear process tables. */
    
#if 0
    /* Clear Area table. */
    for (rn = L_ls_table_top (top->area_table); rn; rn = L_ls_route_next (rn))
    {
        if (RN_INFO (rn, RNI_DEFAULT))
        {
            NETCFG_OM_OSPF_AreaDelete (RN_INFO (rn, RNI_DEFAULT));
        }
    }
    L_ls_table_finish (top->area_table);		/* area table. */
#endif
    /* Clear config networks. */
    rn = NULL;
    for (rn = L_ls_table_top (top->networks); rn; rn = L_ls_route_next (rn))
    {
        if ((network_entry = RN_INFO (rn, RNI_DEFAULT)))
        {
            free(network_entry);
            RN_INFO_UNSET (rn, RNI_DEFAULT);
        }
    }
    L_ls_table_finish (top->networks);		    



    /* We never reuse this process. */
    L_listnode_delete (om->ospf, top);
    L_listnode_delete (ov->ospf, top);
    free(top);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_IfAuthenticationTypeSet
 * PURPOSE:
 *     Set OSPF interface authentication type.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      type
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfAuthenticationTypeSet(UI32_T vr_id, UI32_T ifindex, UI8_T type, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->auth_type = type;
        NETCFG_OM_OSPF_IF_PARAM_SET (oip, AUTH_TYPE);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_IfAuthenticationTypeUnset
 * PURPOSE:
 *     Unset OSPF interface authentication type.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfAuthenticationTypeUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->auth_type = NETCFG_TYPE_OSPF_AUTH_NULL;
        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, AUTH_TYPE);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_IfAuthenticationKeySet
 * PURPOSE:
 *      Set OSPF interface authentication key.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      auth_key
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfAuthenticationKeySet(UI32_T vr_id, UI32_T ifindex, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        strncpy(oip->auth_simple, auth_key, NETCFG_TYPE_OSPF_AUTH_SIMPLE_SIZE);
        NETCFG_OM_OSPF_IF_PARAM_SET (oip, AUTH_SIMPLE);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_IfAuthenticationKeyUnset
 * PURPOSE:
 *     Unset OSPF interface authentication type.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfAuthenticationKeyUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        memset(oip->auth_simple, 0, NETCFG_TYPE_OSPF_AUTH_SIMPLE_SIZE+1);
        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, AUTH_SIMPLE);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_IfMessageDigestKeySet
 * PURPOSE:
 *      Set OSPF interface message digest key.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      key_id
 *      auth_key
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfMessageDigestKeySet(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;
    NETCFG_OM_OSPF_CryptKey_T *ck = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        if (oip->auth_crypt == NULL)
        {
            oip->auth_crypt = L_list_new ();
            oip->auth_crypt->cmp = NETCFG_OM_OSPF_CryptKeyCmp;  
        }

        ck = (NETCFG_OM_OSPF_CryptKey_T *)malloc(sizeof(NETCFG_OM_OSPF_CryptKey_T));
        if(ck != NULL)
        {
            memset(ck, 0, sizeof(NETCFG_OM_OSPF_CryptKey_T));
            ck->key_id = key_id;
            ck->flags = 0;
            strncpy(ck->auth_key, auth_key, NETCFG_TYPE_OSPF_AUTH_MD5_SIZE);
            L_listnode_add_sort (oip->auth_crypt, ck);
            NETCFG_OM_OSPF_IF_PARAM_SET (oip, AUTH_CRYPT);
        }
        else
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
            return FALSE;
        }
             
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_IfMessageDigestKeyUnset
 * PURPOSE:
 *      Unset OSPF interface message digest key.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      key_id
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfMessageDigestKeyUnset(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;
    NETCFG_OM_OSPF_CryptKey_T *ck = NULL;
    struct L_listnode *node;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }
    
    if (oip->auth_crypt != NULL)
    {

        NETCFG_OM_OSPF_CryptKeyDelete (oip->auth_crypt, key_id);
        if (L_LISTCOUNT(oip->auth_crypt) == 0)
        {
            L_list_delete (oip->auth_crypt);
            oip->auth_crypt = NULL;
            NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, AUTH_CRYPT);
        }
        /* Make sure tail key is not in PASSIVE mode. */
        else
        {
            node = oip->auth_crypt->tail;
            ck = L_GETDATA (node);
            if (CHECK_FLAG (ck->flags, NETCFG_OM_OSPF_AUTH_MD5_KEY_PASSIVE))
                UNSET_FLAG (ck->flags, NETCFG_OM_OSPF_AUTH_MD5_KEY_PASSIVE);
        }
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_IfPrioritySet
 * PURPOSE:
 *      Set OSPF interface priority.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      priority
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI8_T priority, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->priority = priority;
        if (priority != NETCFG_OM_OSPF_ROUTER_PRIORITY_DEFAULT)
	        NETCFG_OM_OSPF_IF_PARAM_SET (oip, PRIORITY);
        else
	        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, PRIORITY);
        
        if (oip->ident.priority != priority)
	        oip->ident.priority = priority;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_IfPriorityUnset
 * PURPOSE:
 *      Unset OSPF interface priority.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->priority = NETCFG_OM_OSPF_ROUTER_PRIORITY_DEFAULT;
        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, PRIORITY);
        oip->ident.priority = NETCFG_OM_OSPF_ROUTER_PRIORITY_DEFAULT;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_IfCostSet
 * PURPOSE:
 *      Set OSPF interface cost.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      cost
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T cost, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->output_cost = cost;
	    NETCFG_OM_OSPF_IF_PARAM_SET(oip, OUTPUT_COST);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_IfCostUnset
 * PURPOSE:
 *      Unset OSPF interface cost.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfCostUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->output_cost = NETCFG_OM_OSPF_OUTPUT_COST_DEFAULT;
        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, OUTPUT_COST);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfDeadIntervalSet
 * PURPOSE:
 *      Set OSPF interface dead interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      interval
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->dead_interval = interval;
        if (interval != NETCFG_OM_OSPF_IF_PARAM_DEAD_INTERVAL_DEFAULT (oip))
  	        NETCFG_OM_OSPF_IF_PARAM_SET (oip, DEAD_INTERVAL);
        else
  	        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, DEAD_INTERVAL);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfDeadIntervalUnset
 * PURPOSE:
 *      Unset OSPF interface dead interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->dead_interval = NETCFG_OM_OSPF_IF_PARAM_DEAD_INTERVAL_DEFAULT (oip);
        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, DEAD_INTERVAL);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfHelloIntervalSet
 * PURPOSE:
 *      Set OSPF interface hello interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      interval
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->hello_interval = interval;
        if (interval != NETCFG_OM_OSPF_IF_PARAM_HELLO_INTERVAL_DEFAULT (oip))
  	        NETCFG_OM_OSPF_IF_PARAM_SET (oip, HELLO_INTERVAL);
        else
  	        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, HELLO_INTERVAL);
        
        NETCFG_OM_OSPF_IF_PARAM_DEAD_INTERVAL_UPDATE (oip);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfHelloIntervalUnset
 * PURPOSE:
 *      Unset OSPF interface hello interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->hello_interval = NETCFG_OM_OSPF_IF_PARAM_HELLO_INTERVAL_DEFAULT (oip);
        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, HELLO_INTERVAL);
        NETCFG_OM_OSPF_IF_PARAM_DEAD_INTERVAL_UPDATE (oip);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfRetransmitIntervalSet
 * PURPOSE:
 *      Set OSPF interface retransmit interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      interval
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->retransmit_interval = interval;
        if (interval != NETCFG_OM_OSPF_RETRANSMIT_INTERVAL_DEFAULT)
  	        NETCFG_OM_OSPF_IF_PARAM_SET (oip, RETRANSMIT_INTERVAL);
        else
  	        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, RETRANSMIT_INTERVAL);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfRetransmitIntervalUnset
 * PURPOSE:
 *      Unset OSPF interface retransmit interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->retransmit_interval = NETCFG_OM_OSPF_RETRANSMIT_INTERVAL_DEFAULT;
        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, RETRANSMIT_INTERVAL);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfTransmitDelaySet
 * PURPOSE:
 *      Set OSPF interface transmit delay.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      delay
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T delay, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->transmit_delay = delay;
        if (delay != NETCFG_OM_OSPF_TRANSMIT_DELAY_DEFAULT)
  	        NETCFG_OM_OSPF_IF_PARAM_SET (oip, TRANSMIT_DELAY);
        else
  	        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, TRANSMIT_DELAY);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfTransmitDelayUnset
 * PURPOSE:
 *      Unset OSPF interface transmit delay.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T ret = 0;
    UI32_T original_priority;
    NETCFG_OM_OSPF_IfParams_T *oip = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetOspfInterface(vr_id, ifindex, addr_flag, addr, &oip);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return FALSE;
    }

    if(oip != NULL)
    {
        oip->retransmit_interval = NETCFG_OM_OSPF_TRANSMIT_DELAY_DEFAULT;
        NETCFG_OM_OSPF_IF_PARAM_UNSET (oip, TRANSMIT_DELAY);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return TRUE;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetRouterId
 * PURPOSE:
 *      Get ospf configured Router ID.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      config_router_id.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
 *     1. if return NETCFG_TYPE_OK, it means you can get configured_router_id
 *     2. if return NETCFG_TYPE_CAN_NOT_GET, it means user not configured router id.
 *
 */
UI32_T NETCFG_OM_OSPF_GetRouterId(UI32_T vr_id, UI32_T proc_id, struct pal_in4_addr *config_router_id)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    if(CHECK_FLAG (top_t->config, NETCFG_OM_OSPF_CONFIG_ROUTER_ID))
    {
        memcpy(config_router_id, &(top_t->router_id_config), sizeof(struct pal_in4_addr));
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return  NETCFG_TYPE_OK;
    }
    else
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return  NETCFG_TYPE_CAN_NOT_GET;
    }
}

/* FUNCTION NAME : NETCFG_OM_OSPF_RouterIdSet
 * PURPOSE:
 *      Set ospf Router ID.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      router_id.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
 *
 * NOTES:
 *     
 *     
 */
UI32_T NETCFG_OM_OSPF_RouterIdSet(UI32_T vr_id, UI32_T proc_id, UI32_T router_id)
{    
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    SET_FLAG (top_t->config, NETCFG_OM_OSPF_CONFIG_ROUTER_ID);
    top_t->router_id_config.s_addr = router_id;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_RouterIdUnset
 * PURPOSE:
 *      Set ospf Router ID to default.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
 *     
 *     
 */
UI32_T NETCFG_OM_OSPF_RouterIdUnset(UI32_T vr_id, UI32_T proc_id)
{    
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    top_t->router_id_config.s_addr = 0;
    UNSET_FLAG (top_t->config, NETCFG_OM_OSPF_CONFIG_ROUTER_ID);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetTimer
 * PURPOSE:
 *      Get ospf timer value.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      timer.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*
*/
UI32_T NETCFG_OM_OSPF_GetTimer(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Timer_T *timer)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    memcpy(timer, &(top_t->timer), sizeof(NETCFG_TYPE_OSPF_Timer_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_TimerSet
 * PURPOSE:
 *      Set ospf timer value.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *      timer
 * OUTPUT:
 *      
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
 *
 * NOTES:
*
*
*/
UI32_T NETCFG_OM_OSPF_TimerSet(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Timer_T timer)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    memcpy(&(top_t->timer), &timer, sizeof(NETCFG_TYPE_OSPF_Timer_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_TimerUnset
 * PURPOSE:
 *      Set ospf timer value to default value.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *      
 * OUTPUT:
 *      
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
 *
 * NOTES:
*
*
*/
UI32_T NETCFG_OM_OSPF_TimerUnset(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    top_t->timer.delay = SYS_DFLT_OSPF_SPF_DELAY_DEFAULT;
    top_t->timer.hold = SYS_DFLT_OSPF_SPF_HOLDTIME_DEFAULT;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetDefaultMetric
 * PURPOSE:
 *      Get ospf default metric value.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      flag,
 *      metric.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetDefaultMetric(UI32_T vr_id, UI32_T proc_id, BOOL_T *flag, UI32_T *metric)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    if(CHECK_FLAG (top_t->config, NETCFG_OM_OSPF_CONFIG_DEFAULT_METRIC))
    {
        *flag = TRUE;
        *metric = top_t->default_metric;
    }
    else
    {
        *flag = FALSE;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultMetricSet
* PURPOSE:
*     Set ospf default metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      metric.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    SET_FLAG (top_t->config, NETCFG_OM_OSPF_CONFIG_DEFAULT_METRIC);
    top_t->default_metric = metric;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultMetricUnset
* PURPOSE:
*     Set ospf default metric to default value.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultMetricUnset(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    UNSET_FLAG (top_t->config, NETCFG_OM_OSPF_CONFIG_DEFAULT_METRIC);
    top_t->default_metric = 0;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetCompatibleRfc1583Status
 * PURPOSE:
 *      Get ospf compatible rfc1583 status.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      status.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetCompatibleRfc1583Status(UI32_T vr_id, UI32_T proc_id, BOOL_T *status)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    if(CHECK_FLAG (top_t->config, NETCFG_OM_OSPF_CONFIG_RFC1583_COMPATIBLE))
    {
        *status = TRUE;
    }
    else
    {
        *status = FALSE;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_CompatibleRfc1583Set
* PURPOSE:
*     Set ospf  compatible rfc1583.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_CompatibleRfc1583Set(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    SET_FLAG (top_t->config, NETCFG_OM_OSPF_CONFIG_RFC1583_COMPATIBLE);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_CompatibleRfc1583Unset
* PURPOSE:
*     Unset ospf  compatible rfc1583.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_CompatibleRfc1583Unset(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    UNSET_FLAG (top_t->config, NETCFG_OM_OSPF_CONFIG_RFC1583_COMPATIBLE);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}
static NETCFG_TYPE_OSPF_Passive_If_T *NETCFG_OM_OSPF_PassiveIfLookUp(UI32_T vr_id, UI32_T proc_id, UI32_T *ifindex, UI32_T *addr)
{    
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;
    NETCFG_TYPE_OSPF_Passive_If_T *passive_entry = NULL;
    struct L_listnode *node;

    if(NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t) != NETCFG_TYPE_OK)
    {
        return NULL;
    }

    LIST_LOOP (top_t->passive_if, passive_entry, node)
      if (passive_entry->ifindex == *ifindex)
        if (passive_entry->addr == *addr)
            return passive_entry;

    return NULL;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetPassiveIf
 * PURPOSE:
 *      Get passive interface status.
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry->ifname,
 *      entry->addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetPassiveIf(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Passive_If_T *entry)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;

    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    if(NETCFG_OM_OSPF_PassiveIfLookUp(vr_id, proc_id, &(entry->ifindex), &(entry->addr)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_GET;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetNextPassiveIf
 * PURPOSE:
 *      Get next passive interface .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetNextPassiveIf(
                UI32_T vr_id, 
                UI32_T proc_id, 
                NETCFG_TYPE_OSPF_Passive_If_T *entry)
{    
    UI32_T original_priority;
    UI32_T ret;
    struct L_listnode *node;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;
    NETCFG_TYPE_OSPF_Passive_If_T *passive_entry = NULL;
    NETCFG_TYPE_OSPF_Passive_If_T *next_passive_entry = NULL;

    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    
    ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t);
    if(ret != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    
    if(entry->ifindex == 0 && entry->addr == 0)
    {
        next_passive_entry = L_listnode_head(top_t->passive_if);
    }
    else
    {
        for (node = top_t->passive_if->head; node; node=node->next)
        { 
            passive_entry = node->data;
            if(passive_entry)
            {
                if(passive_entry->ifindex ==  entry->ifindex)
                {
                    if(passive_entry->addr >  entry->addr)
                    {
                        next_passive_entry = passive_entry;
                        break;
                    }
                }
                else if(passive_entry->ifindex >  entry->ifindex)
                {
                    next_passive_entry = passive_entry;
                    break;
                } 
            }
        }
    }
    
    if (next_passive_entry)
    {
        memcpy(entry, next_passive_entry, sizeof(NETCFG_TYPE_OSPF_Passive_If_T));
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return  NETCFG_TYPE_OK;
    }
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_CAN_NOT_GET;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_PassiveIfSet
* PURPOSE:
*     Set ospf  passive interface.
*
* INPUT:
*      vr_id
*      proc_id,
*      ifindex,
*      addr
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_PassiveIfSet(UI32_T vr_id, UI32_T proc_id, UI32_T ifindex, UI32_T addr)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;
    NETCFG_TYPE_OSPF_Passive_If_T *passive_entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    passive_entry = (NETCFG_TYPE_OSPF_Passive_If_T *)malloc(sizeof(NETCFG_TYPE_OSPF_Passive_If_T));
    if(passive_entry == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }
    passive_entry->ifindex = ifindex;
    passive_entry->addr = addr;
    
    L_listnode_add_sort (top_t->passive_if, passive_entry);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_OSPF_PassiveIfUnset
* PURPOSE:
*     Unset ospf  passive interface.
*
* INPUT:
*      vr_id
*      proc_id,
*      ifindex,
*      addr
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_PassiveIfUnset(UI32_T vr_id, UI32_T proc_id, UI32_T ifindex, UI32_T addr)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;
    NETCFG_TYPE_OSPF_Passive_If_T *passive_entry = NULL;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    passive_entry = NETCFG_OM_OSPF_PassiveIfLookUp(vr_id, proc_id, &ifindex, &addr);

    if(passive_entry == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    
    L_listnode_delete (top_t->passive_if, passive_entry);
    free (passive_entry);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

static NETCFG_TYPE_OSPF_Network_T *NETCFG_OM_OSPF_NetworkMatch (NETCFG_OM_OSPF_Instance_T *top, struct L_ls_prefix *lp)
{
    struct L_ls_node *rn, *rn_tmp;
    
    rn_tmp = L_ls_node_get (top->networks, lp);
    for (rn = rn_tmp; rn; rn = rn->parent)
    {    
        if (RN_INFO (rn, RNI_DEFAULT))
        {
            L_ls_unlock_node (rn_tmp);
            return RN_INFO (rn, RNI_DEFAULT);
        }
    }
    
    L_ls_unlock_node (rn_tmp);
    return NULL;
}
/* FUNCTION NAME : NETCFG_OM_OSPF_GetNetworkEntry
 * PURPOSE:
 *      Get network entry.
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetNetworkEntry(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T *entry)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_TYPE_OSPF_Network_T *network_entry = NULL;
//    struct L_ls_node           *node = NULL;
    NETCFG_OM_OSPF_Instance_T  *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    network_entry = NETCFG_OM_OSPF_NetworkLookup(top_t, entry->lp);
    if(NULL != network_entry)
    {
        memcpy(entry, network_entry, sizeof(NETCFG_TYPE_OSPF_Network_T));
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return  NETCFG_TYPE_OK;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_CAN_NOT_GET;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetNextNetworkEntry
 * PURPOSE:
 *      Get next network .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetNextNetworkEntry(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T *entry)
{    
    UI32_T original_priority;
    UI32_T ret;
    struct L_ls_node           *rn = NULL;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    if ( entry->network == 0 && entry->masklen == 0)
    {
        rn = L_ls_node_lookup_first(top->networks);
    }
    else
    {
        rn = L_ls_node_lookup (top->networks, entry->lp);
        if ( rn == NULL )
            rn = L_ls_node_get(top->networks, entry->lp);
        do
        {
            if ( rn == NULL )
                break;
            /* Find next node which has information */ 
            rn = L_ls_route_next(rn);
        }         
        while(rn && !RN_INFO (rn, RNI_DEFAULT));
    }
    if ( rn && RN_INFO (rn, RNI_DEFAULT) )
    {
        memcpy (entry, RN_INFO (rn, RNI_DEFAULT), sizeof (NETCFG_TYPE_OSPF_Network_T));
        L_ls_unlock_node(rn);
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
       
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_ENTRY_NOT_EXIST;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_NetworkSet
 * PURPOSE:
 *      Set network entry.
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_NetworkSet(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T entry)
{
    UI32_T ret;
    UI32_T original_priority;
//    struct L_ls_node *node = NULL;
    NETCFG_TYPE_OSPF_Network_T  *network_entry = NULL;
    NETCFG_OM_OSPF_Instance_T   *top_t = NULL;
//    NETCFG_TYPE_OSPF_Area_T     *area = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    
#if 0
    area = NETCFG_OM_OSPF_AreaGet(top_t, entry.area_id);
    if (area == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return  NETCFG_TYPE_CAN_NOT_SET;
    }
    NETCFG_OM_OSPF_AreaLock(area);
    
    if (area->conf.format == NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DEFAULT)
    {
        area->conf.format = entry.format;
    }
#endif
    network_entry = (NETCFG_TYPE_OSPF_Network_T *)malloc(sizeof(NETCFG_TYPE_OSPF_Network_T));
    if(network_entry == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return  NETCFG_TYPE_CAN_NOT_SET;
    }
    memset(network_entry, 0, sizeof(NETCFG_TYPE_OSPF_Network_T));
    memcpy(network_entry, &entry, sizeof(NETCFG_TYPE_OSPF_Network_T));

    NETCFG_OM_OSPF_NetworkEntryInsert(top_t, network_entry, entry.lp);

    //ospf_network_run (top, nw);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_NetworkUnset
 * PURPOSE:
 *      Unset network entry.
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_NetworkUnset(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T entry)
{
    UI32_T ret;
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T  *top_t = NULL;
    NETCFG_TYPE_OSPF_Network_T *network_entry = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    network_entry = NETCFG_OM_OSPF_NetworkLookup(top_t, entry.lp);
    if (!network_entry)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return  NETCFG_TYPE_OK;
    }
    
    NETCFG_OM_OSPF_NetworkDelete(top_t, network_entry);
    free (network_entry);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

#if 0
static NETCFG_TYPE_OSPF_Area_T *NETCFG_OM_OSPF_AreaLock (NETCFG_TYPE_OSPF_Area_T *area)
{
  area->lock++;
  return area;
}

static NETCFG_TYPE_OSPF_Area_T *NETCFG_OM_OSPF_AreaUnlock (NETCFG_TYPE_OSPF_Area_T *area)
{
  if (!area)
    return NULL;

  area->lock--;
  assert (area->lock >= 0);

  if (area->lock == 0)
    {
      NETCFG_OM_OSPF_AreaDelete(area);
      return NULL;
    }
  return area;
}
#endif

static NETCFG_TYPE_OSPF_Area_T *NETCFG_OM_OSPF_AreaNew (NETCFG_OM_OSPF_Instance_T  *top_t, UI32_T area_id)
{
    NETCFG_TYPE_OSPF_Area_T *new_area = NULL;
    
    new_area = (NETCFG_TYPE_OSPF_Area_T *)malloc(sizeof(NETCFG_TYPE_OSPF_Area_T));
    memset(new_area, 0, sizeof(NETCFG_TYPE_OSPF_Area_T));

    new_area->area_id = area_id;
    new_area->lock = 0;
    
    new_area->conf.config = 0;
    new_area->conf.format = NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DEFAULT;
    new_area->conf.external_routing = NETCFG_TYPE_OSPF_AREA_DEFAULT;
    //new_area->conf.auth_type = OSPF_AREA_AUTH_NULL;
    //new_area->conf.shortcut = OSPF_SHORTCUT_DEFAULT;
    //new_area->conf.default_cost = SYS_DFLT_OSPF_STUB_DEFAULT_COST;
#if 0
#ifdef HAVE_NSSA
      ospf_area_conf_nssa_reset (conf);
#endif /* HAVE_NSSA */
#endif
    new_area->status = NETCFG_TYPE_ROW_STATUS_NOTREADY;
    new_area->vr_id = top_t->vr_id;
    new_area->ospf_id = top_t->ospf_id;

    return new_area;
}

#if 0
static void NETCFG_OM_OSPF_AreaDelete (NETCFG_TYPE_OSPF_Area_T *area)
{
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;
  
    if(NETCFG_OM_OSPF_GetInstance(area->vr_id,area->ospf_id,&top_t) != NETCFG_TYPE_OK)
        return ;
  
    /* Remove link from instance. */
    if (area->area_id == 0)
        top_t->backbone = NULL;

    if (top_t->area_table)
        NETCFG_OM_OSPF_AreaEntryDelete (top_t, area);

    free(area);
}
#endif

static NETCFG_TYPE_OSPF_Area_T *NETCFG_OM_OSPF_AreaGet (NETCFG_OM_OSPF_Instance_T  *top_t, UI32_T area_id)
{
    NETCFG_TYPE_OSPF_Area_T  *area = NULL;

    area = NETCFG_OM_OSPF_AreaEntryLookup(top_t, area_id);
    if (area == NULL)
    {
        /* Hit area limit. */
        if (area_id != 0)
        {
            if ((top_t->area_table->count[RNI_DEFAULT] - (top_t->backbone != NULL ? 1 : 0)) >= top_t->max_area_limit)
            {
                return NULL;
            }
        }

        area = NETCFG_OM_OSPF_AreaNew(top_t,area_id);
        if (area->area_id == 0)
        {
            top_t->backbone = area;
        }

        NETCFG_OM_OSPF_AreaEntryInsert(top_t, area);
    }

    return area;
}

#if 0
static int NETCFG_OM_OSPF_NetworkAreaMatch (NETCFG_TYPE_OSPF_Area_T *area)
{
    NETCFG_TYPE_OSPF_Network_T *nw;
    struct L_ls_node *rn;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;
  
    if(NETCFG_OM_OSPF_GetInstance(area->vr_id,area->ospf_id,&top_t) != NETCFG_TYPE_OK)
    {
        return 0;
    }
  
    for (rn = L_ls_table_top (top_t->networks); rn; rn = L_ls_route_next (rn))
    {
        if ((nw = RN_INFO (rn, RNI_DEFAULT)))
        {
            if (IPV4_ADDR_SAME (&nw->area_id, &area->area_id))
            {
                L_ls_unlock_node (rn);
                return 1;
            }
        }
    }

    return 0;
}

static BOOL_T NETCFG_OM_OSPF_AreaEntryDelete (NETCFG_OM_OSPF_Instance_T  *top_t, NETCFG_TYPE_OSPF_Area_T *area)
{
    int ret = FALSE;
    struct L_ls_node *rn = NULL;
    struct L_ls_prefix p;
    struct pal_in4_addr area_addr;

    memset (&p, 0, sizeof (struct L_ls_prefix));
    area_addr.s_addr = area->area_id;
    L_ls_prefix_ipv4_set (&p, IPV4_MAX_BITLEN, area_addr);

    rn = L_ls_node_lookup (top_t->area_table, &p);
    if (rn)
    {
        RN_INFO_UNSET (rn, RNI_DEFAULT);
        L_ls_unlock_node (rn);
        ret = TRUE;
    }
    return ret;
}
#endif
    
static NETCFG_TYPE_OSPF_Area_T *NETCFG_OM_OSPF_AreaEntryLookup (NETCFG_OM_OSPF_Instance_T  *top_t, UI32_T area_id)
{
    struct L_ls_node           *node = NULL;
    struct L_ls_prefix         area_p;
    struct pal_in4_addr        area_addr;
    
    memset(&area_p, 0, sizeof(struct L_ls_prefix));
    area_addr.s_addr = area_id;
    L_ls_prefix_ipv4_set (&area_p, IPV4_MAX_BITLEN, area_addr);
    
    node = L_ls_node_lookup(top_t->area_table, &area_p);
    if (node)
    {
        L_ls_unlock_node (node);
        return RN_INFO (node, RNI_DEFAULT);
    }
    return NULL;
}

static BOOL_T NETCFG_OM_OSPF_AreaEntryInsert (NETCFG_OM_OSPF_Instance_T  *top_t, NETCFG_TYPE_OSPF_Area_T *area)
{
    BOOL_T ret = FALSE;
    struct L_ls_node *rn = NULL;
    struct L_ls_prefix p;
    struct pal_in4_addr  area_addr;

    memset (&p, 0, sizeof (struct L_ls_prefix));
    area_addr.s_addr = area->area_id;
    L_ls_prefix_ipv4_set (&p, IPV4_MAX_BITLEN, area_addr);

    rn = L_ls_node_get (top_t->area_table, &p);
    if ((RN_INFO (rn, RNI_DEFAULT)) == NULL)
    {
        RN_INFO_SET (rn, RNI_DEFAULT, area);
        ret = TRUE;
    }
    L_ls_unlock_node (rn);
    return ret;
}

static void NETCFG_OM_OSPF_NetworkEntryInsert (NETCFG_OM_OSPF_Instance_T  *top_t, NETCFG_TYPE_OSPF_Network_T *network, struct L_ls_prefix *lp)
{
    struct L_ls_node *node = NULL;
    node = L_ls_node_get (top_t->networks, lp);
    if ((RN_INFO (node, RNI_DEFAULT)) == NULL)
    {
        network->lp = node->p;
        RN_INFO_SET (node, RNI_DEFAULT, network);
    }
    
    L_ls_unlock_node (node);
}

static NETCFG_TYPE_OSPF_Network_T* NETCFG_OM_OSPF_NetworkLookup (NETCFG_OM_OSPF_Instance_T  *top_t, struct L_ls_prefix *lp)
{
    struct L_ls_node *rn;

    rn = L_ls_node_lookup (top_t->networks, lp);
    if (rn)
    {
        L_ls_unlock_node (rn);
        return RN_INFO (rn, RNI_DEFAULT);
    }
    return NULL;
}

static void NETCFG_OM_OSPF_NetworkDelete (NETCFG_OM_OSPF_Instance_T  *top_t, NETCFG_TYPE_OSPF_Network_T *network)
{
//    NETCFG_TYPE_OSPF_Area_T *area;
    struct L_ls_node *rn;

    rn = L_ls_node_lookup (top_t->networks, network->lp);
    if (rn)
    {
#if 0
        area = NETCFG_OM_OSPF_AreaEntryLookup(top_t, network->area_id);
#endif
        L_ls_unlock_node (rn);
        RN_INFO_UNSET (rn, RNI_DEFAULT);
#if 0
        NETCFG_OM_OSPF_AreaUnlock(area);
#endif

        #if 0/*not sure used in netcfg*/
        /* Update AS-external-LSA.  */
        if (!ospf_area_default_count (top))
        ospf_lsdb_lsa_discard_by_type (top->lsdb,
                    OSPF_AS_EXTERNAL_LSA, 1, 1, 1);
        else
        ospf_redistribute_timer_add_all_proto (top);
        #endif
    }
}
static int
NETCFG_OM_OSPF_Str2DistributeSource (char *str, int *source)
{
  /* Sanity check. */
  if (str == NULL)
    return NETCFG_TYPE_FAIL;

  if (pal_strncmp (str, "k", 1) == 0)
    *source = NETCFG_OM_ROUTE_KERNEL;
  else if (pal_strncmp (str, "c", 1) == 0)
    *source = NETCFG_OM_ROUTE_CONNECT;
  else if (pal_strncmp (str, "s", 1) == 0)
    *source = NETCFG_OM_ROUTE_STATIC;
  else if (pal_strncmp (str, "r", 1) == 0)
    *source = NETCFG_OM_ROUTE_RIP;
  else if (pal_strncmp (str, "b", 1) == 0)
    *source = NETCFG_OM_ROUTE_BGP;
  else if (pal_strncmp (str, "i", 1) == 0)
    *source = NETCFG_OM_ROUTE_ISIS;
  else if (pal_strncmp (str, "d", 1) == 0)
    *source = NETCFG_OM_ROUTE_DEFAULT;
  else
    return NETCFG_TYPE_FAIL;

  return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetNextSummaryAddr
* PURPOSE:
*      Getnext Summary Address.
*
* INPUT:
*      vr_id
*      proc_id,
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetNextSummaryAddr(UI32_T vr_id, UI32_T proc_id, UI32_T *addr, UI32_T *masklen)
{    
    UI32_T original_priority;
    UI32_T ret;
    struct pal_in4_addr in4_addr;
    struct L_ls_prefix lp;
    struct L_ls_node           *rn = NULL;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    in4_addr.s_addr = *addr;
    if ( 0 == *addr && *masklen == 0)
    {
        rn = L_ls_node_lookup_first(top->summary);
    }
    else
    {
        L_ls_prefix_ipv4_set (&lp, *masklen, in4_addr);
        rn = L_ls_node_lookup (top->summary, &lp);
        if ( rn == NULL )
            rn = L_ls_node_get(top->summary, &lp);
        do
        {
            if ( rn == NULL )
                break;
            /* Find next node which has information */ 
            rn = L_ls_route_next(rn);
        }         
        while(rn && !RN_INFO (rn, RNI_DEFAULT));
    }
    if ( rn && RN_INFO (rn, RNI_DEFAULT) )
    {
        memcpy (addr, rn->p->prefix, sizeof (struct pal_in4_addr));
        *masklen = rn->p->prefixlen;
        L_ls_unlock_node(rn);
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
       
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_ENTRY_NOT_EXIST;
}
/* FUNCTION NAME : NETCFG_OM_OSPF_GetAutoCostRefBandwidth
* PURPOSE:
*      Get auto cost reference bandwidth.
*
* INPUT:
*      vr_id
*      proc_id,
*      refbw
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetAutoCostRefBandwidth(UI32_T vr_id, UI32_T proc_id, UI32_T *refbw)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    *refbw = top->ref_bandwidth/1000;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}
/* FUNCTION NAME : NETCFG_OM_OSPF_GetRedistributeConfig
* PURPOSE:
*      Get redistribute configuration inormation.
*
* INPUT:
*      vr_id
*      proc_id,
*      
*
* OUTPUT:
*      redist_config.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetRedistributeConfig(UI32_T vr_id, UI32_T proc_id, char *type, NETCFG_OM_OSPF_REDIST_CONF_T *redist_config)
{    
    UI32_T original_priority;
    UI32_T ret;
    int proto_type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource(type, &proto_type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    *redist_config = top->redist[proto_type];
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}
/* FUNCTION NAME : NETCFG_OM_OSPF_GetDefaultInfoConfig
* PURPOSE:
*      Get default information configuration.
*
* INPUT:
*      vr_id
*      proc_id,
*      
*
* OUTPUT:
*      redist_config.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetDefaultInfoConfig(UI32_T vr_id, UI32_T proc_id, NETCFG_OM_OSPF_REDIST_CONF_T *redist_config)
{    
    UI32_T original_priority;
    UI32_T ret;
    int proto_type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource("default", &proto_type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    *redist_config = top->redist[proto_type];
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetDefaultInfoAlways
* PURPOSE:
*     Get ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_GetDefaultInfoAlways(UI32_T vr_id, UI32_T proc_id, UI32_T *originate)
{
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    /* Get "always" stat */
    *originate = top->default_origin;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_OSPF_GetSummaryAddr
* PURPOSE:
*      Get Summary Address.
*
* INPUT:
*      vr_id
*      proc_id,
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetSummaryAddr(UI32_T vr_id, UI32_T proc_id, UI32_T addr, UI32_T masklen)
{    
    UI32_T original_priority;
    UI32_T ret;
    struct pal_in4_addr in4_addr;
    struct L_ls_prefix lp;
    struct L_ls_node           *rn = NULL;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;
    
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    in4_addr.s_addr = addr;
    L_ls_prefix_ipv4_set (&lp, masklen, in4_addr);
    rn = L_ls_node_lookup (top->summary, &lp);
    if (rn)
    {
        L_ls_unlock_node (rn);        
        if ( RN_INFO (rn, RNI_DEFAULT) )
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
            return NETCFG_TYPE_ENTRY_EXIST;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_ENTRY_NOT_EXIST;
}
/* FUNCTION NAME : NETCFG_OM_OSPF_AddSummaryAddr
* PURPOSE:
*      Add Summary Address.
*
* INPUT:
*      vr_id
*      proc_id,
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_AddSummaryAddr(UI32_T vr_id, UI32_T proc_id, UI32_T addr, UI32_T masklen)
{    
    UI32_T original_priority;
    UI32_T ret;
    struct pal_in4_addr in4_addr;
    struct L_ls_prefix lp;
    struct L_ls_node           *rn = NULL;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;
    NETCFG_OM_OSPF_SummaryAddress_T *summary = NULL;
    
    /* Check if this summary address already exists */
    ret = NETCFG_OM_OSPF_GetSummaryAddr(vr_id, proc_id, addr, masklen);
    if ( ret == NETCFG_TYPE_ENTRY_EXIST )
        return NETCFG_TYPE_ENTRY_EXIST;
    /* Prepare to add this entry */
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    /* malloc summary address entry */
    summary = (NETCFG_OM_OSPF_SummaryAddress_T *)malloc(sizeof (summary));
    if ( !summary )
        goto exit;
    in4_addr.s_addr = addr;
    L_ls_prefix_ipv4_set (&lp, masklen, in4_addr);
    /* Insert this node */
    rn = L_ls_node_get (top->summary, &lp);
    if (RN_INFO (rn, RNI_DEFAULT) == NULL)
    {
        summary->top = top;
        summary->lp = (struct ls_prefix *)rn->p;
        RN_INFO_SET (rn, RNI_DEFAULT, summary);
    }
    L_ls_unlock_node (rn);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
exit:
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_FAIL;
}
/* FUNCTION NAME : NETCFG_OM_OSPF_DelSummaryAddr
* PURPOSE:
*      Delete Summary Address.
*
* INPUT:
*      vr_id
*      proc_id,
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_DelSummaryAddr(UI32_T vr_id, UI32_T proc_id, UI32_T addr, UI32_T masklen)
{    
    UI32_T original_priority;
    UI32_T ret;
    struct pal_in4_addr in4_addr;
    struct L_ls_prefix lp;
    struct L_ls_node           *rn = NULL;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;
    void *info;
  
    /* Check if this summary address already exists */
    ret = NETCFG_OM_OSPF_GetSummaryAddr(vr_id, proc_id, addr, masklen);
    if ( ret != NETCFG_TYPE_ENTRY_EXIST )
        return NETCFG_TYPE_OK;
    /* Prepare to add this entry */
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    in4_addr.s_addr = addr;
    L_ls_prefix_ipv4_set (&lp, masklen, in4_addr);
    rn = L_ls_node_lookup (top->summary, &lp);
    if (rn)
    {
      info = RN_INFO (rn, RNI_DEFAULT);
      RN_INFO_UNSET (rn, RNI_DEFAULT);       
      L_ls_unlock_node (rn);
      free (info);
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}
/* FUNCTION NAME : NETCFG_OM_OSPF_SetAutoCostRefBandwidth
* PURPOSE:
*      Set auto cost reference bandwidth.
*
* INPUT:
*      vr_id
*      proc_id,
*      refbw
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_SetAutoCostRefBandwidth(UI32_T vr_id, UI32_T proc_id, UI32_T refbw)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    /* Check if reference bandwidth is changed. */
    if ((refbw * 1000) == top->ref_bandwidth)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }    
    /* Set */
    top->ref_bandwidth = refbw * 1000;    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}
/* FUNCTION NAME : NETCFG_OM_OSPF_UnsetAutoCostRefBandwidth
* PURPOSE:
*      Unset auto cost reference bandwidth.
*
* INPUT:
*      vr_id
*      proc_id,
*      
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_UnsetAutoCostRefBandwidth(UI32_T vr_id, UI32_T proc_id)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    if (top->ref_bandwidth == OSPF_DEFAULT_REF_BANDWIDTH)
      return NETCFG_TYPE_OK;
    
    top->ref_bandwidth = OSPF_DEFAULT_REF_BANDWIDTH;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeProtoSet
* PURPOSE:
*     Set ospf redistribute.
*
* INPUT:
*      vr_id,
*      proc_id,
*      type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeProtoSet(UI32_T vr_id, UI32_T proc_id, char *type)
{
    UI32_T original_priority;
    UI32_T ret;
    int   proto_type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource(type, &proto_type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[proto_type];
    if (CHECK_FLAG (rc->flags, OSPF_REDIST_ENABLE))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    /* Set flage */
    SET_FLAG (rc->flags, OSPF_REDIST_ENABLE);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeProtoUnset
* PURPOSE:
*     Unset ospf redistribute.
*
* INPUT:
*      vr_id,
*      proc_id,
*      type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeProtoUnset(UI32_T vr_id, UI32_T proc_id, char *type)
{
    UI32_T original_priority;
    UI32_T ret;
    int proto_type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource(type, &proto_type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[proto_type];
    if (!CHECK_FLAG (rc->flags, OSPF_REDIST_ENABLE))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    /* Unset flage */   
    UNSET_FLAG (rc->flags, OSPF_REDIST_ENABLE);
    UNSET_FLAG (rc->flags, OSPF_REDIST_METRIC);
    UNSET_FLAG (rc->flags, OSPF_REDIST_METRIC_TYPE);
    UNSET_FLAG (rc->flags, OSPF_REDIST_TAG);
    UNSET_FLAG (rc->flags, OSPF_REDIST_ROUTE_MAP);
    /* Default value */
    rc->metric = EXTERNAL_METRIC_VALUE_UNSPEC;
    rc->tag = 0;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeMetricTypeSet
* PURPOSE:
*     Set ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric_type
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeMetricTypeSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric_type)
{
    UI32_T original_priority;
    UI32_T ret;
    int type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource(proto_type, &type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[type];
    if (METRIC_TYPE (rc) == metric_type)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    /* Set the flag */
    SET_FLAG (rc->flags, OSPF_REDIST_ENABLE);
    METRIC_TYPE_SET (rc, metric_type);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeMetricTypeUnset
* PURPOSE:
*     Unset ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeMetricTypeUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    UI32_T original_priority;
    UI32_T ret;
    int type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource(proto_type, &type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[type];
    /* Check the proto flag */
    if (!REDIST_PROTO_CHECK (rc) )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    /* Check the metric type flag */
    if (!CHECK_FLAG (rc->flags, OSPF_REDIST_METRIC_TYPE))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    /* Set flag */    
    METRIC_TYPE_SET (rc, EXTERNAL_METRIC_TYPE_2);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeMetricSet
* PURPOSE:
*     Set ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeMetricSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric)
{
    UI32_T original_priority;
    UI32_T ret;
    int type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource(proto_type, &type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[type];
    if (METRIC_VALUE (rc) == metric)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    
    METRIC_VALUE (rc) = metric;
    SET_FLAG (rc->flags, OSPF_REDIST_ENABLE);
    SET_FLAG (rc->flags, OSPF_REDIST_METRIC);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeMetricUnset
* PURPOSE:
*     Unset ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeMetricUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    UI32_T original_priority;
    UI32_T ret;
    int type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource(proto_type, &type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[type];
    /* Check the proto flag */
    if (!REDIST_PROTO_CHECK (rc))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    /* Check the metric type flag */
    if (!CHECK_FLAG (rc->flags, OSPF_REDIST_METRIC))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    /* Unset flag */    
    METRIC_VALUE (rc) = EXTERNAL_METRIC_VALUE_UNSPEC;
    UNSET_FLAG (rc->flags, OSPF_REDIST_METRIC);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeTagSet
* PURPOSE:
*     Set ospf redistribute tag.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      tag
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeTagSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T tag)
{
    UI32_T original_priority;
    UI32_T ret;
    int type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource(proto_type, &type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[type];
    if (REDIST_TAG (rc) == tag)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    
    REDIST_TAG (rc) = tag;
    SET_FLAG (rc->flags, OSPF_REDIST_ENABLE);
    SET_FLAG (rc->flags, OSPF_REDIST_TAG);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeTagUnset
* PURPOSE:
*     Unset ospf redistribute tag.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeTagUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    UI32_T original_priority;
    UI32_T ret;
    int type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource(proto_type, &type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[type];
    /* Check the proto flag */
    if (!REDIST_PROTO_CHECK (rc))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    /* Check the tag flag */
    if (!CHECK_FLAG (rc->flags, OSPF_REDIST_TAG))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }
    /* Unset flag */    
    REDIST_TAG (rc) = 0;
    UNSET_FLAG (rc->flags, OSPF_REDIST_TAG);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeRoutemapSet
* PURPOSE:
*     Set ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      route_map
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, char *route_map)
{
    UI32_T original_priority;
    UI32_T ret, len;
    int type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource(proto_type, &type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[type];
    if (RMAP_NAME (rc))
      free (RMAP_NAME (rc));

    len = strlen(route_map);
    if (len > SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH)
        len = SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH;
    
    RMAP_NAME (rc) = malloc(len);
    strncpy(RMAP_NAME(rc), route_map, len);
    RMAP_NAME(rc)[len - 1] = '\0';
    RMAP_MAP (rc) = 0;    
    SET_FLAG (rc->flags, OSPF_REDIST_ENABLE);
    SET_FLAG (rc->flags, OSPF_REDIST_ROUTE_MAP);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeRoutemapUnset
* PURPOSE:
*     Unset ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeRoutemapUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    UI32_T original_priority;
    UI32_T ret;
    int type;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    ret = NETCFG_OM_OSPF_Str2DistributeSource(proto_type, &type);
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }

    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[type];
    if (RMAP_NAME (rc))
      free(RMAP_NAME (rc));    
    RMAP_NAME (rc) = NULL;
    RMAP_MAP (rc) = NULL;    
    UNSET_FLAG (rc->flags, OSPF_REDIST_ROUTE_MAP);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoMetricTypeSet
* PURPOSE:
*     Set ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric_type
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoMetricTypeSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric_type)
{
    return NETCFG_OM_OSPF_RedistributeMetricTypeSet(vr_id, proc_id, "default", metric_type);
}


/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoMetricTypeUnset
* PURPOSE:
*     Unset ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoMetricTypeUnset(UI32_T vr_id, UI32_T proc_id )
{
    return NETCFG_OM_OSPF_RedistributeMetricTypeUnset(vr_id, proc_id, "default");
}


/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoMetricSet
* PURPOSE:
*     Set ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric)
{
    return NETCFG_OM_OSPF_RedistributeMetricSet(vr_id, proc_id, "default", metric);
}


/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoMetricUnset
* PURPOSE:
*     Unset ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoMetricUnset(UI32_T vr_id, UI32_T proc_id )
{
    return NETCFG_OM_OSPF_RedistributeMetricUnset(vr_id, proc_id, "default");
}


/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoRoutemapSet
* PURPOSE:
*     Set ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      route_map
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *route_map)
{
    return NETCFG_OM_OSPF_RedistributeRoutemapSet(vr_id, proc_id, "default", route_map);
}


/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoRoutemapUnset
* PURPOSE:
*     Unset ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoRoutemapUnset( UI32_T vr_id, UI32_T proc_id )
{
    return NETCFG_OM_OSPF_RedistributeRoutemapUnset(vr_id, proc_id, "default");    
}


/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoAlwaysSet
* PURPOSE:
*     Set ospf default information to "always".
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoAlwaysSet(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[0];
    /* Set always */
    top->default_origin = NETCFG_OM_OSPF_DEFAULT_ORIGINATE_ALWAYS;
    REDIST_PROTO_SET (rc);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoAlwaysUnset
* PURPOSE:
*     Unset ospf default information to "always".
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoAlwaysUnset(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    /* Set always */
    top->default_origin = NETCFG_OM_OSPF_DEFAULT_ORIGINATE_UNSPEC;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}
/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoSet
* PURPOSE:
*     Set ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoSet(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[NETCFG_OM_ROUTE_DEFAULT];
    /* Check this proto */
    if (REDIST_PROTO_CHECK (rc))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }    
    REDIST_PROTO_SET (rc);
    /* Set */
    if (top->default_origin == NETCFG_OM_OSPF_DEFAULT_ORIGINATE_UNSPEC )
        top->default_origin = NETCFG_OM_OSPF_DEFAULT_ORIGINATE_NSM;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoUnset
* PURPOSE:
*     Unset ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoUnset(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T  *top = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    NETCFG_OM_OSPF_REDIST_CONF_T *rc = &top->redist[NETCFG_OM_ROUTE_DEFAULT];
    /* Check this proto */
    if (!REDIST_PROTO_CHECK (rc))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OK;
    }    
    REDIST_PROTO_UNSET (rc);
    /* Unset flage */   
    UNSET_FLAG (rc->flags, OSPF_REDIST_METRIC);
    UNSET_FLAG (rc->flags, OSPF_REDIST_METRIC_TYPE);
    UNSET_FLAG (rc->flags, OSPF_REDIST_ROUTE_MAP);
    /* Default value */
    rc->metric = EXTERNAL_METRIC_VALUE_UNSPEC;
    rc->tag = 0;

    /* Set */
    top->default_origin = NETCFG_OM_OSPF_DEFAULT_ORIGINATE_UNSPEC;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetAreaDefaultCost
 * PURPOSE:
 *      Get ospf area default cost value.
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      area_id
 *
 * OUTPUT:
 *      cost.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetAreaDefaultCost(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T *cost)
{    
    UI32_T original_priority;
    UI32_T ret;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;
    NETCFG_TYPE_OSPF_Area_T   *area = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    
    area = NETCFG_OM_OSPF_AreaEntryLookup(top_t,area_id);
    if (area == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_GET;
    }
    if(area->conf.external_routing == NETCFG_TYPE_OSPF_AREA_DEFAULT)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OSPF_AREA_IS_DEFAULT;
    }
    
    *cost = area->conf.default_cost;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;

}

#if 0
/* FUNCTION NAME : NETCFG_OM_OSPF_AreaDefaultCostSet
* PURPOSE:
*     Set ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      cost.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_AreaDefaultCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T cost)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;
    NETCFG_TYPE_OSPF_Area_T   *area = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }
    
    area = NETCFG_OM_OSPF_AreaEntryLookup(top_t,area_id);
    if (area == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }
    if(area->conf.external_routing == NETCFG_TYPE_OSPF_AREA_DEFAULT)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OSPF_AREA_IS_DEFAULT;
    }
    
    area->conf.default_cost = cost;
    if(cost == SYS_DFLT_OSPF_STUB_DEFAULT_COST)
    {
        OSPF_AREA_CONF_UNSET (area, DEFAULT_COST);
    }
    else
    {
        OSPF_AREA_CONF_SET (area, DEFAULT_COST);
    }

    
    if (area->conf.format == NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DEFAULT)
      area->conf.format = format;
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_AreaDefaultCostUnset
* PURPOSE:
*     Unset ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_AreaDefaultCostUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T *top_t = NULL;
    NETCFG_TYPE_OSPF_Area_T   *area = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }
    
    area = NETCFG_OM_OSPF_AreaEntryLookup(top_t,area_id);
    if (area == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_SET;
    }
    
    if(area->conf.external_routing == NETCFG_TYPE_OSPF_AREA_DEFAULT)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OSPF_AREA_IS_DEFAULT;
    }
    
    area->conf.default_cost = SYS_DFLT_OSPF_STUB_DEFAULT_COST;
    OSPF_AREA_CONF_UNSET (area, DEFAULT_COST);
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

static NETCFG_TYPE_OSPF_Area_Range_T *NETCFG_OM_OSPF_AreaRangeLookup(NETCFG_TYPE_OSPF_Area_T  *area, struct L_ls_prefix *lp)
{
    struct L_ls_node *rn;

    rn = L_ls_node_lookup (area->ranges, lp);
    if (rn)
    {
        L_ls_unlock_node (rn);
        return RN_INFO (rn, RNI_DEFAULT);
    }
    return NULL;
}

static NETCFG_TYPE_OSPF_Area_Range_T *NETCFG_OM_OSPF_AreaRangeMatch (NETCFG_TYPE_OSPF_Area_T  *area, struct L_ls_prefix *lp)
{
    struct L_ls_node *rn;

    rn = L_ls_node_match (area->ranges, lp);
    if (rn)
    {
        L_ls_unlock_node (rn);
        return RN_INFO (rn, RNI_DEFAULT);
    }
    return NULL;
}

static NETCFG_TYPE_OSPF_Area_Range_T *NETCFG_OM_OSPF_AreaRangeNew (NETCFG_TYPE_OSPF_Area_T *area)
{
  NETCFG_TYPE_OSPF_Area_Range_T *range;
  
  range = (NETCFG_TYPE_OSPF_Area_Range_T *)malloc(sizeof(NETCFG_TYPE_OSPF_Area_Range_T));
  memset (range, 0, sizeof (NETCFG_TYPE_OSPF_Area_Range_T));
  range->area = NETCFG_OM_OSPF_AreaLock(area);
  range->status = NETCFG_TYPE_ROW_STATUS_ACTIVE;

  return range;
}

static void NETCFG_OM_OSPF_AreaRangeFree (NETCFG_TYPE_OSPF_Area_Range_T *range)
{
  if (range->subst)
    L_ls_prefix_free (range->subst);

  free(range);
}

static void NETCFG_OM_OSPF_AreaRangeAdd (NETCFG_TYPE_OSPF_Area_T *area,NETCFG_TYPE_OSPF_Area_Range_T *range, struct L_ls_prefix *lp)
{
    //NETCFG_TYPE_OSPF_Area_Range_T *range_alt;
    struct L_ls_node *rn;

    #if 0
    range_alt = NETCFG_OM_OSPF_AreaRangeMatch (area, lp);
    if (range_alt)
    {
        if (range_alt != range)
            OSPF_TIMER_ON (range_alt->t_update, ospf_area_range_timer, range_alt, 1);
    }
    #endif
    rn = L_ls_node_get (area->ranges, lp);
    if (RN_INFO (rn, RNI_DEFAULT) == NULL)
    {
      range->lp = rn->p;
      //ospf_area_aggregate_entry_insert (area, range);
      NETCFG_OM_OSPF_AreaRangeEntryInsert(area, range);

      RN_INFO_SET (rn, RNI_DEFAULT, range);
      
      //OSPF_TIMER_ON (range->t_update, ospf_area_range_timer, range, 1);
    }

    L_ls_unlock_node (rn);
}

static void NETCFG_OM_OSPF_AreaRangeDelete (NETCFG_TYPE_OSPF_Area_T *area,NETCFG_TYPE_OSPF_Area_Range_T *range)
{
  struct L_ls_node *rn;

  rn = L_ls_node_lookup (area->ranges, range->lp);
  if (rn)
    {
      //ospf_area_aggregate_entry_delete (area, range);
      NETCFG_OM_OSPF_AreaRangeEntryDelete(area, range);

      RN_INFO_UNSET (rn, RNI_DEFAULT);

      //ospf_abr_area_range_withdraw (area, range);

      NETCFG_OM_OSPF_AreaRangeFree(range);
      NETCFG_OM_OSPF_AreaUnlock(area);
    }
}

static void NETCFG_OM_OSPF_AreaRangeDeleteAll (NETCFG_TYPE_OSPF_Area_T *area)
{
  NETCFG_TYPE_OSPF_Area_Range_T *range;
  struct L_ls_node *rn;

  for (rn = L_ls_table_top (area->ranges); rn; rn = L_ls_route_next (rn))
    if ((range = RN_INFO (rn, RNI_DEFAULT)))
      {
	//ospf_area_aggregate_entry_delete (area, range);
	NETCFG_OM_OSPF_AreaRangeEntryDelete (area, range);
	NETCFG_OM_OSPF_AreaRangeFree (range);

	RN_INFO_UNSET (rn, RNI_DEFAULT);
      }

  L_ls_table_finish (area->ranges);
  //L_ls_table_finish (area->aggregate_table);

  /* Garbage collection.  */
  //ospf_area_range_entry_clean_inactive (area);
}

static UI32_T NETCFG_OM_OSPF_AreaRangeEntryInsert (NETCFG_TYPE_OSPF_Area_T *area, NETCFG_TYPE_OSPF_Area_Range_T *range)
{
    NETCFG_TYPE_OSPF_Area_Range_T *find;
    struct L_ls_node *rn;
    struct L_ls_prefix8 p;
    
    memset (&p, 0, sizeof (struct L_ls_prefix8));
    p.prefixsize = NETCFG_OM_OSPF_AREA_RANGE_TABLE_DEPTH;
    p.prefixlen = NETCFG_OM_OSPF_AREA_RANGE_TABLE_DEPTH * 8;
    
    /* Set Net and Mask as index.  */
    L_ls_prefix_set_args ((struct L_ls_prefix *)&p,
                netcfg_om_ospf_api_table_def[NETCFG_OM_OSPF_AREA_RANGE_TABLE].vars,
                &area->area_id, range->lp->prefix);
    
    /* Address Range Table is used for SNMP MIB purpose. And the entry 
       is only identify with Area ID and Range net prefix. Due to this 
       limitation, for same Range net prefix but different subnetmask, 
       only install one time.  */
    #if 0
    rn = L_ls_node_get (area->top->area_range_table, (struct L_ls_prefix *)&p);
    
    if ((find = RN_INFO (rn, RNI_DEFAULT)) == NULL)
      RN_INFO_SET (rn, RNI_DEFAULT, range);
    
    L_ls_unlock_node (rn);
    #endif
    return NETCFG_TYPE_OK;

}

static UI32_T NETCFG_OM_OSPF_AreaRangeEntryDelete (NETCFG_TYPE_OSPF_Area_T *area, NETCFG_TYPE_OSPF_Area_Range_T *range)
{
    NETCFG_TYPE_OSPF_Area_Range_T *find;
    struct L_ls_node *rn;
    struct L_ls_prefix8 p;
    
    memset (&p, 0, sizeof (struct L_ls_prefix8));
    p.prefixsize = NETCFG_OM_OSPF_AREA_RANGE_TABLE_DEPTH;
    p.prefixlen = NETCFG_OM_OSPF_AREA_RANGE_TABLE_DEPTH * 8;
    
    /* Set Net and Mask as index.  */
    L_ls_prefix_set_args ((struct L_ls_prefix *)&p,
                netcfg_om_ospf_api_table_def[NETCFG_OM_OSPF_AREA_RANGE_TABLE].vars,
                &area->area_id, range->lp->prefix);
    # if 0
    rn = L_ls_node_lookup (area->top->area_range_table, (struct L_ls_prefix *)&p);
    if (rn)
      {
        if ((find = RN_INFO (rn, RNI_DEFAULT)))
          {
            if (find == range)    
              RN_INFO_UNSET (rn, RNI_DEFAULT);
      }
    
        L_ls_unlock_node (rn);
        return NETCFG_TYPE_OK;
      }
    #endif
    return NETCFG_TYPE_FAIL;
}
#endif

/* FUNCTION NAME : NETCFG_OM_OSPF_AreaStubSet
* PURPOSE:
*     Set ospf area stub.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_AreaStubSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    UI32_T ret;
    UI32_T original_priority;
    //struct L_ls_node *node = NULL;
    NETCFG_OM_OSPF_Instance_T   *top_t = NULL;
    NETCFG_TYPE_OSPF_Area_T     *area = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(vr_id,proc_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    
    area = NETCFG_OM_OSPF_AreaGet(top_t, area_id);
    if (area == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return  NETCFG_TYPE_CAN_NOT_SET;
    }
    
    if (area->vlink_count)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return  NETCFG_TYPE_OSPF_AREA_HAS_VLINK;
    }

    if (area->conf.external_routing == NETCFG_TYPE_OSPF_AREA_NSSA)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_OSPF_AREA_IS_NSSA;
    }
    
    else if (area->conf.external_routing == NETCFG_TYPE_OSPF_AREA_DEFAULT)
    {
        //ospf_area_conf_external_routing_set (area, NETCFG_TYPE_OSPF_AREA_STUB);
    }
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetInstanceStatistics
* PURPOSE:
*     Get ospf instance some parameters.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id.
*
* OUTPUT:
*      entry
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_GetInstancePara(NETCFG_TYPE_OSPF_Instance_Para_T *entry)
{
    UI32_T ret;
    UI32_T original_priority;
    NETCFG_OM_OSPF_Instance_T   *top_t = NULL;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if((ret = NETCFG_OM_OSPF_GetInstance(entry->vr_id,entry->ospf_id,&top_t)) != NETCFG_TYPE_OK)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return ret;
    }
    
    entry->config = top_t->config;
    entry->default_metric = top_t->default_metric;
    entry->router_id_config = top_t->router_id_config.s_addr;
    memcpy(&(entry->timer), &(top_t->timer), sizeof(NETCFG_TYPE_OSPF_Timer_T));
    
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_OM_OSPF_GetInstanceCount
 * PURPOSE:
 *     Get total number of ospf instances
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      count  -- Total instance count
 *
 * RETURN:
 *      RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
 *      None
 */
UI32_T NETCFG_OM_OSPF_GetInstanceCount(UI32_T vr_id, UI32_T *count)
{
    NETCFG_OM_OSPF_Master_T *om = NULL;
    UI32_T original_priority;

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ospf_om_sem_id);
    if(NETCFG_OM_OSPF_GetMaster(vr_id, &om) != TRUE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_GET;
    }

    if (om->ospf == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
        return NETCFG_TYPE_CAN_NOT_GET;
    }

    *count = om->ospf->count;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ospf_om_sem_id, original_priority);
    return  NETCFG_TYPE_OK;
}

 

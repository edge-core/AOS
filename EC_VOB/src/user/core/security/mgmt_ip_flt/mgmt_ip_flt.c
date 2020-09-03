/* Project Name: New Feature
 * File_Name   : mgmt_ip_flt.c
 * Purpose     :
 *
 * 2003/01/14  : Ahten Chen    Create this file
 * 2007/12/27  : Shumin.Wang   Modify this file
 *
 * Copyright(C)      Accton Corporation, 2003
 *
 * Note    : Designed for new platform (Mercury_DC)
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sysfun.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "l_inet.h"
#include "sys_bld.h"
#include "mgmt_ip_flt.h"
#include "trap_mgr.h"
#include "syslog_mgr.h"
#include "syslog_type.h"
#include "sys_module.h"
#include "eh_type.h"
#include "eh_mgr.h"
#include "ip_lib.h"
#include "snmp_pmgr.h"
#include "sys_callback_mgr.h"

/* TYPE DEFINITIONS
 */
typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    UI32_T                      mgmt_ip_flt_status;
    MGMT_IP_FLT_IpFilter_T      snmp_ip_filter  [SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT];
    MGMT_IP_FLT_IpFilter_T      telnet_ip_filter[SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT];
    MGMT_IP_FLT_IpFilter_T      http_ip_filter  [SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT];
    MGMT_IP_FLT_IpFilter_T      global_ip_filter[SYS_ADPT_MAX_NBR_OF_IP_ADDR_FOR_MGMT_IP_FLT];
    MGMT_IP_FLT_BlockCache_T    block_cache[MGMT_IP_FLT_MAX_MODE];
    UI32_T                      ip_filter_counter;
    UI32_T                      mgmt_ip_flt_orig_priority;
} MGMT_IP_FLT_ShmemData_T;

/* MACRO DEFINITIONS
 */

#define  MGMT_IP_FLT_ENTER_CRITICAL_SECTION()     shmem_data_p->mgmt_ip_flt_orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(mgmt_ip_flt_semaphore_id)
#define  MGMT_IP_FLT_LEAVE_CRITICAL_SECTION() SYSFUN_OM_LEAVE_CRITICAL_SECTION(mgmt_ip_flt_semaphore_id,shmem_data_p->mgmt_ip_flt_orig_priority)
#define MGMT_IP_FLT_USE_CSC_WITHOUT_RETURN_VALUE
#define MGMT_IP_FLT_USE_CSC(a)
#define MGMT_IP_FLT_RELEASE_CSC()

/* return value of MGMT_IP_FLT_IpCompare()
 */
#define MGMT_IP_FLT_IP_EQUAL				0x01
#define MGMT_IP_FLT_IP_LARGE				0x02
#define MGMT_IP_FLT_IP_LESS					0x04


#define MGMT_IP_FLT_USE_CSC_CHECK_OPER_MODE(RET_VAL, function_no, format_id)                             \
  MGMT_IP_FLT_USE_CSC(RET_VAL);                                                  \
  if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)  {   \
     MGMT_IP_FLT_RELEASE_CSC();                                                  \
     EH_MGR_Handle_Exception(SYS_MODULE_CLI, function_no, format_id, SYSLOG_LEVEL_INFO);\
     return (RET_VAL);                                                      \
  }

#define MGMT_IP_FLT_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE(function_no, format_id)               \
  MGMT_IP_FLT_USE_CSC_WITHOUT_RETURN_VALUE();                                    \
  if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)  {   \
     MGMT_IP_FLT_RELEASE_CSC();                                                   \
     EH_MGR_Handle_Exception(SYS_MODULE_CLI, function_no, format_id, SYSLOG_LEVEL_INFO);\
    return;                                                                 \
  }

#define MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(RET_VAL)  {               \
    MGMT_IP_FLT_RELEASE_CSC();                                       \
    return (RET_VAL);                                           \
  }

#define MGMT_IP_FLT_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE()  { \
    MGMT_IP_FLT_RELEASE_CSC();                                       \
    return;                                                     \
  }

#define MGMT_IP_FLT_TICKS_1SEC                      100

/* declare variables used for semaphore  */

static UI32_T   mgmt_ip_flt_semaphore_id;
static MGMT_IP_FLT_ShmemData_T *shmem_data_p = 0;

/* declare variables used for transition mode  */
SYSFUN_DECLARE_CSC

/* declare local functions */
#if 0
static void MGMT_IP_FLT_BackDoor_Menu(void);
static void MGMT_IP_FLT_BackDoor_SubMenu(UI32_T mode);
static void MGMT_IP_FLT_BackDoor_ConvertString2Inet(const UI8_T *in_str, L_INET_AddrIp_T *inaddr_p);
#endif  /* #if 0 */
static UI32_T MGMT_IP_FLT_Check_Parameters(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry);
static MGMT_IP_FLT_IpFilter_T *MGMT_IP_FLT_Get_DataBaseByMode(UI32_T mode);
static UI32_T MGMT_IP_FLT_GetIpFilter(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry);
static UI32_T MGMT_IP_FLT_GetNextIpFilter(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry);
static void MGMT_IP_FLT_Rearrange_DataBase(UI32_T mode);
static void MGMT_IP_FLT_Send_Trap(UI32_T mode, const L_INET_AddrIp_T *ip);

#if(SYS_CPNT_MANAGEMENT_IP_FILTER_DEFAULT_DENY ==TRUE)
static BOOL_T MGMT_IP_FLT_IsValidProtocol(UI32_T mode);
#endif

static BOOL_T MGMT_IP_FLT_IsVaildIP(const L_INET_AddrIp_T *ip_addr);
static UI32_T MGMT_IP_FLT_IpCompare(const L_INET_AddrIp_T *ip_addr_a, const L_INET_AddrIp_T *ip_addr_b);

static BOOL_T MGMT_IP_FLT_GetNextBlockCacheEntry(
                                                 UI32_T mode,
                                                 UI32_T *index_p,
                                                 MGMT_IP_FLT_BlockCacheEntry_T *entry_p
                                                 );

static void MGMT_IP_FLT_RemoveAgingBlockCacheEntry(
                                                   UI32_T mode
                                                   );

static BOOL_T MGMT_IP_FLT_FindBlockCache(
                                         UI32_T mode,
                                         const L_INET_AddrIp_T *ip_p
                                         );

static BOOL_T MGMT_IP_FLT_DeleteBlockCacheEntry(
                                                UI32_T mode,
                                                UI32_T index
                                                );

static MGMT_IP_FLT_BlockCache_T* MGMT_IP_FLT_GetBlockCache(
                                                           UI32_T mode
                                                           );

static BOOL_T MGMT_IP_FLT_BlockCache_Add(
                                         MGMT_IP_FLT_BlockCache_T *this,
                                         MGMT_IP_FLT_BlockCacheEntry_T *entry_p
                                         );

static BOOL_T MGMT_IP_FLT_BlockCache_Ary_Add(
                                             MGMT_IP_FLT_BlockCache_T *this,
                                             UI32_T index,
                                             MGMT_IP_FLT_BlockCacheEntry_T *entry_p
                                             );

static BOOL_T MGMT_IP_FLT_BlockCache_Ary_Del(
                                             MGMT_IP_FLT_BlockCache_T *this,
                                             UI32_T index
                                             );

static void MGMT_IP_FLT_DumpBlockCache(
                                       UI32_T mode
                                       );

static void MGMT_IP_FLT_DumpHex(
                                UI8_T *data,
                                UI32_T size
                                );

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_Initiate_System_Resources
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Invoked by root.c()
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_Initiate_System_Resources (void)
{
    MGMT_IP_FLT_Init ();
}

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_Init
 *-----------------------------------------------------------
 * FUNCTION: To initialize the MGR module of IP_FILTER
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------*/
void MGMT_IP_FLT_Init(void)
{
    /* Create semaphore
     */
    if(SYSFUN_OK!=SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_IPFILTER,&mgmt_ip_flt_semaphore_id))
    {

        SYSFUN_Debug_Printf ("\r\n SWCTL: Create semaphore failed. LOCK");
        while (1)
            ;
    }

 	shmem_data_p = (MGMT_IP_FLT_ShmemData_T *)SYSRSC_MGR_GetShMem(SYSRSC_MGR_IPFILTER_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);

    /*
        * EPR_ID:ES3628BT-FLF-ZZ-00304
        * Problem: MgtIPFilter:Configuration can not be cleared after copy startup-config to running-config.
        * Root Cause: No set the configure information to default value when system enter transition mode and init process
        * Solution: set the configure information to default value when system enter transition mode and init process.
        * Modified files:
        *     user\core\security\mgmt_ip_flt\mgmg_ip_flt.c
        */
    memset(shmem_data_p, 0, sizeof (MGMT_IP_FLT_ShmemData_T));
    shmem_data_p->ip_filter_counter = 0;
}
/* FUNCTION NAME: MGMT_IP_FLT_GetShMemInfo
 *----------------------------------------------------------------------------------
 * PURPOSE: Get share memory info
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void MGMT_IP_FLT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_IPFILTER_SHMEM_SEGID;
    *seglen_p = sizeof(MGMT_IP_FLT_ShmemData_T);
}
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: MGMT_IP_FLT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for COS_OM in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void MGMT_IP_FLT_AttachSystemResources(void)
{
    shmem_data_p = (MGMT_IP_FLT_ShmemData_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_IPFILTER_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_IPFILTER, &mgmt_ip_flt_semaphore_id);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_Create_InterCSC_Relation(void)
{
    MGMT_IP_FLT_Create_InterCSC_Relation();
}
/*-----------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_Create_InterCSC_Relation
 *-----------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------*/
void MGMT_IP_FLT_Create_InterCSC_Relation(void)
{
    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_SetTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_EnterTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize all system resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_EnterTransitionMode(void)
{
    /*
        * EPR_ID:ES3628BT-FLF-ZZ-00304
        * Problem: MgtIPFilter:Configuration can not be cleared after copy startup-config to running-config.
        * Root Cause: No set the configure information to default value when system enter transition mode and init process
        * Solution: set the configure information to default value when system enter transition mode and init process.
        * Modified files:
        *     user\core\security\mgmt_ip_flt\mgmg_ip_flt.c
        */
    memset(shmem_data_p, 0, sizeof (MGMT_IP_FLT_ShmemData_T));
    shmem_data_p->ip_filter_counter = 0;
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_EnterMasterMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_EnterMasterMode(void)
{
#if 0
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("ipfilter",
        SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,MGMT_IP_FLT_BackDoor_Menu);
#endif
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
    return;
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_EnterSlaveMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Snmp_GetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the specified IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES: 1. The ip filter receiver can only be accessed by SNMP.
 *        2. The total number of ip filter receivers supported by the system
 *           is defined by sys_adpt.h.
 *        3. By default, there is no ip filter receiver configued in the system.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Snmp_GetIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    return MGMT_IP_FLT_GetIpFilter(MGMT_IP_FLT_SNMP, ip_filter_entry);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Https_GetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the specified IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES: 1. The ip filter receiver can only be accessed by HTTPS.
 *        2. The total number of ip filter receivers supported by the system
 *           is defined by sys_adpt.h.
 *        3. By default, there is no ip filter receiver configued in the system.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Https_GetIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    return MGMT_IP_FLT_GetIpFilter(MGMT_IP_FLT_HTTPS, ip_filter_entry);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Ssh_GetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the specified IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES: 1. The ip filter receiver can only be accessed by SSH.
 *        2. The total number of ip filter receivers supported by the system
 *           is defined by sys_adpt.h.
 *        3. By default, there is no ip filter receiver configued in the system.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Ssh_GetIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    return MGMT_IP_FLT_GetIpFilter(MGMT_IP_FLT_SSH, ip_filter_entry);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Web_GetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the specified IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES: 1. The ip filter receiver can only be accessed by WEB.
 *        2. The total number of ip filter receivers supported by the system
 *           is defined by sys_adpt.h.
 *        3. By default, there is no ip filter receiver configued in the system.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Web_GetIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    return MGMT_IP_FLT_GetIpFilter(MGMT_IP_FLT_WEB, ip_filter_entry);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Telnet_GetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the specified IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES: 1. The ip filter receiver can only be accessed by CLI.
 *        2. The total number of ip filter receivers supported by the system
 *           is defined by sys_adpt.h.
 *        3. By default, there is no ip filter receiver configued in the system.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Telnet_GetIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    return MGMT_IP_FLT_GetIpFilter(MGMT_IP_FLT_TELNET, ip_filter_entry);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_WEB_GetNextIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the next available IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_WEB_GetNextIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    return MGMT_IP_FLT_GetNextIpFilter(MGMT_IP_FLT_WEB, ip_filter_entry);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Snmp_GetNextIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the next available IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Snmp_GetNextIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    return MGMT_IP_FLT_GetNextIpFilter(MGMT_IP_FLT_SNMP, ip_filter_entry);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_TELNET_GetNextIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the next available IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_TELNET_GetNextIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    return MGMT_IP_FLT_GetNextIpFilter(MGMT_IP_FLT_TELNET, ip_filter_entry);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_HTTPS_GetNextIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the next available IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Https_GetNextIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    return MGMT_IP_FLT_GetNextIpFilter(MGMT_IP_FLT_HTTPS, ip_filter_entry);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_SSH_GetNextIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the next available IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Ssh_GetNextIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    return MGMT_IP_FLT_GetNextIpFilter(MGMT_IP_FLT_SSH, ip_filter_entry);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_GetNextRunningIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available entry can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available non-default ip filter.
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL.
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default ip filter.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_GetNextRunningIpFilter(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    UI32_T  i;
    MGMT_IP_FLT_IpFilter_T *local_ip_filter;

    MGMT_IP_FLT_USE_CSC_CHECK_OPER_MODE(MGMT_IP_FLT_NOT_IN_MASTER_MODE, MGMT_IP_FLT_GETNEXTRUNNINGIPFILTER, EH_TYPE_MSG_NOT_IN_MASTER_MODE);

    if ((local_ip_filter = MGMT_IP_FLT_Get_DataBaseByMode(mode)) == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_GETNEXTRUNNINGIPFILTER,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_CRIT, "pointer");
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_FAIL);
    }

    MGMT_IP_FLT_ENTER_CRITICAL_SECTION();

    if (FALSE == MGMT_IP_FLT_IsVaildIP(&ip_filter_entry->start_ipaddress)
        && TRUE == MGMT_IP_FLT_IsVaildIP(&local_ip_filter[0].start_ipaddress))
    {
        memcpy(ip_filter_entry, &(local_ip_filter[0]), sizeof(MGMT_IP_FLT_IpFilter_T));
        MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }
    else
    {
        for (i = 0; i<SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT; i++)
        {
            if (MGMT_IP_FLT_IP_LARGE == MGMT_IP_FLT_IpCompare(
                &local_ip_filter[i].start_ipaddress, &ip_filter_entry->start_ipaddress))
            {
                memcpy(ip_filter_entry, &(local_ip_filter[i]), sizeof(MGMT_IP_FLT_IpFilter_T));
                MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
                MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
            }
        }
    }
    MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
    EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_GETNEXTRUNNINGIPFILTER,
                             EH_TYPE_MSG_IPFLT_IP_RANGE, SYSLOG_LEVEL_INFO, "not exists");
    MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_SetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the status can be successfully
 *          set to the specified ip filter. Otherwise, false is returned.
 *
 * INPUT: 1. mode: the mode for this ip filter(Web/SNMP/Telnet).
 *           - MGMT_IP_FLT_WEB(1).
 *           - MGMT_IP_FLT_SNMP(2).
 *           - MGMT_IP_FLT_TELNET(3).
 *        2. ip_filter_entry->start_ipaddress   - (main key) to specify a unique ip filter address.
 * OUTPUT: None
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES: 1. This function will create a new ip filter to the system if
 *           the specified (start_ipaddress) does not exist,
 *           and total number of ip filter configured is less than
 *           SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT.
 *        2. This function will update an existed ip address if
 *           the specified (start_ipaddress) existed already.
 *        3. start/end address can set 0.
 *        4. if only specify start address, this function will automatically
 *           fill end address.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_SetIpFilter(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    UI32_T  i,check_para_ret;
    MGMT_IP_FLT_IpFilter_T *local_ip_filter;

    MGMT_IP_FLT_USE_CSC_CHECK_OPER_MODE(MGMT_IP_FLT_NOT_IN_MASTER_MODE, MGMT_IP_FLT_SETIPFILTER, EH_TYPE_MSG_NOT_IN_MASTER_MODE);

    check_para_ret = MGMT_IP_FLT_Check_Parameters(mode, ip_filter_entry);
    if (check_para_ret != MGMT_IP_FLT_OK)
    {
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(check_para_ret);
    }

    if ((local_ip_filter = MGMT_IP_FLT_Get_DataBaseByMode(mode)) == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_SETIPFILTER,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_CRIT, "pointer");
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_FAIL);
    }

    MGMT_IP_FLT_ENTER_CRITICAL_SECTION();
    for (i = 0; i<SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT; i++)
    {
        /* if use the same key can update end address
         */
        if (MGMT_IP_FLT_IP_EQUAL == MGMT_IP_FLT_IpCompare(
            &local_ip_filter[i].start_ipaddress, &ip_filter_entry->start_ipaddress))
        {
            if(local_ip_filter[i].start_ipaddress.type != ip_filter_entry->start_ipaddress.type)
            {
                MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
                EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_SETIPFILTER,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_CRIT, "wrong type");
                MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_FAIL);
            }
             if (i == SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT - 1)
             {
                local_ip_filter[i].end_ipaddress = ip_filter_entry->end_ipaddress;
                goto return_ok;
             }
             else
             {
                  /* for endian issue, use memcmp to do comparing.
                   */
                if ( (MGMT_IP_FLT_IP_LESS == MGMT_IP_FLT_IpCompare(
                     &ip_filter_entry->end_ipaddress, &local_ip_filter[i+1].start_ipaddress)) ||
                     FALSE == MGMT_IP_FLT_IsVaildIP(&local_ip_filter[i+1].start_ipaddress) )
                {
                    local_ip_filter[i].end_ipaddress = ip_filter_entry->end_ipaddress;
                    goto return_ok;
                 }
            }
        }

        /* start && end address = 0 means not used entry and this entry is the last empty entry.
         *Because the database is sorted by start address.
         */
        if( FALSE == MGMT_IP_FLT_IsVaildIP(&local_ip_filter[i].start_ipaddress)
            && FALSE == MGMT_IP_FLT_IsVaildIP(&local_ip_filter[i].end_ipaddress) )
        {
            local_ip_filter[i].start_ipaddress = ip_filter_entry->start_ipaddress;
            local_ip_filter[i].end_ipaddress = ip_filter_entry->end_ipaddress;
            goto return_ok;
        }
        else
        {
            /*
             * to make sure the range not overlap
             *            S|------|E        --> database
             *        S|-----|E             -->set value (overlap)
             *              S|--|E          -->set value (overlap)
             *               S|------|E     -->set value (overlap)
             *          S|--------------|E  -->set value (overlap)
            */
            /* for endian issue, use memcmp to do comparing.
             */
            if ((MGMT_IP_FLT_IP_LARGE == MGMT_IP_FLT_IpCompare(
                    &ip_filter_entry->start_ipaddress, &local_ip_filter[i].start_ipaddress) &&
                (MGMT_IP_FLT_IP_LESS |MGMT_IP_FLT_IP_EQUAL) & MGMT_IP_FLT_IpCompare(
                    &ip_filter_entry->start_ipaddress, &local_ip_filter[i].end_ipaddress) )  ||
                (MGMT_IP_FLT_IP_LESS == MGMT_IP_FLT_IpCompare(
                    &ip_filter_entry->start_ipaddress, &local_ip_filter[i].start_ipaddress) &&
                (MGMT_IP_FLT_IP_LARGE |MGMT_IP_FLT_IP_EQUAL) & MGMT_IP_FLT_IpCompare(
                    &ip_filter_entry->end_ipaddress , &local_ip_filter[i].start_ipaddress)  )
               )
            {
                MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
                EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_SETIPFILTER,
                                         EH_TYPE_MSG_IPFLT_IP_RANGE, SYSLOG_LEVEL_INFO, "overlap");
                MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_IP_RANGE_OVERLAP);
            }
        }
    }

    MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
    EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_SETIPFILTER,
                             EH_TYPE_MSG_IPFLT_IP_RANGE, SYSLOG_LEVEL_INFO, "full");
    MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_ENTRY_IS_FULL);
return_ok:
    MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
    MGMT_IP_FLT_Rearrange_DataBase(mode);

    SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged(SYS_MODULE_MGMT_IP_FLT, mode);

    MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_OK);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_DeleteIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the status can be successfully
 *          delete to the specified ip filter. Otherwise, false is returned.
 *
 * INPUT: 1. mode: the mode for this ip filter(Web/SNMP/Telnet).
 *           - MGMT_IP_FLT_WEB(1).
 *           - MGMT_IP_FLT_SNMP(2).
 *           - MGMT_IP_FLT_TELNET(3).
 *        2. ip_filter_entry->start_ipaddress   - (main key) to specify a unique ip filter address.
 * OUTPUT: None
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES: 1. This function will delete an ip filter from system if
 *           the specified (start_ipaddress) exist.
 *        2. False is returned if the specified (start_ipaddress) does not exist.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_DeleteIpFilter(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    UI32_T  i;
    MGMT_IP_FLT_IpFilter_T *local_ip_filter;

    MGMT_IP_FLT_USE_CSC_CHECK_OPER_MODE(MGMT_IP_FLT_NOT_IN_MASTER_MODE, MGMT_IP_FLT_DELETEIPFILTER, EH_TYPE_MSG_NOT_IN_MASTER_MODE);

    if (mode < MGMT_IP_FLT_HTTP || mode > MGMT_IP_FLT_SSH)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_DELETEIPFILTER,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO, "management IP filter mode");
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_MODE_IS_INVALID);
    }

    if (FALSE == MGMT_IP_FLT_IsVaildIP(&ip_filter_entry->start_ipaddress))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_DELETEIPFILTER,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO, "start address");
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_IP_IS_INVALID);
    }

    /* for endian issue, use memcmp to do comparing.
     */
    if (MGMT_IP_FLT_IP_LARGE == MGMT_IP_FLT_IpCompare(
        &ip_filter_entry->start_ipaddress, &ip_filter_entry->end_ipaddress) &&
        TRUE == MGMT_IP_FLT_IsVaildIP(&ip_filter_entry->end_ipaddress))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_DELETEIPFILTER,
                                 EH_TYPE_MSG_IPFLT_IP_RANGE, SYSLOG_LEVEL_INFO, "start address larger than end address");
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_IP_RANGE_START_BIG_THEN_END);
    }

    if ((local_ip_filter = MGMT_IP_FLT_Get_DataBaseByMode(mode)) == NULL)
    {
         EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_DELETEIPFILTER,
                                  EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_CRIT, "pointer");
         MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_FAIL);
    }

    MGMT_IP_FLT_ENTER_CRITICAL_SECTION();

    for (i = 0; i<SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT; i++)
    {
        if (TRUE == MGMT_IP_FLT_IsVaildIP(&local_ip_filter[i].start_ipaddress) &&
            TRUE == MGMT_IP_FLT_IsVaildIP(&local_ip_filter[i].end_ipaddress))
        {
            if ( MGMT_IP_FLT_IP_EQUAL == MGMT_IP_FLT_IpCompare(
                &local_ip_filter[i].start_ipaddress, &ip_filter_entry->start_ipaddress) &&
                (MGMT_IP_FLT_IP_EQUAL == MGMT_IP_FLT_IpCompare(
                &local_ip_filter[i].end_ipaddress, &ip_filter_entry->end_ipaddress) ||
                FALSE == MGMT_IP_FLT_IsVaildIP(&ip_filter_entry->end_ipaddress) )
                )
            {
                memset(&local_ip_filter[i].start_ipaddress, 0x00, sizeof(L_INET_AddrIp_T));
                memset(&local_ip_filter[i].end_ipaddress, 0x00, sizeof(L_INET_AddrIp_T));
                MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
                MGMT_IP_FLT_Rearrange_DataBase(mode);

                SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged(SYS_MODULE_MGMT_IP_FLT, mode);

                MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_OK);
            }
        }
    }
    MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
    EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_DELETEIPFILTER,
                             EH_TYPE_MSG_IPFLT_IP_RANGE, SYSLOG_LEVEL_INFO, "not exists");
    MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_IP_RANGE_NOT_EXISTED);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_AddBlockCache
 * ---------------------------------------------------------------------
 * PURPOSE: Add a IP address to block cache
 *
 * INPUT: 1. mode         : mode (HTTP/SNMP/Telnet).
 *        2. inet_p       : IP address
 *        3. alive_second : alive time(second) in block cache
 * OUTPUT: None
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES: 1. If the cache full, one IP address have shortest alive time
 *           will be removed.
 * ---------------------------------------------------------------------
 */
BOOL_T MGMT_IP_FLT_AddBlockCache(
                                 UI32_T mode,
                                 const L_INET_AddrIp_T *inet_p,
                                 UI32_T alive_second
                                 )
{
    MGMT_IP_FLT_BlockCache_T        *cache_p;
    MGMT_IP_FLT_BlockCacheEntry_T   entry;
    BOOL_T                          ret;

    MGMT_IP_FLT_USE_CSC_CHECK_OPER_MODE(FALSE, MGMT_IP_FLT_ISVALIDIPFILTERADDRESS, EH_TYPE_MSG_NOT_IN_MASTER_MODE);

    cache_p = MGMT_IP_FLT_GetBlockCache(mode);

    if (NULL == cache_p)
    {
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (0 == alive_second)
    {
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(TRUE);
    }

    memset(&entry, 0, sizeof(entry));

    memcpy(&entry.inet, inet_p, sizeof(entry.inet));
    entry.alive_time = (alive_second * MGMT_IP_FLT_TICKS_1SEC) + SYSFUN_GetSysTick();

    ret = MGMT_IP_FLT_BlockCache_Add(cache_p, &entry);

    if (0)
    {
        MGMT_IP_FLT_DumpHex((UI8_T*)&entry.inet, sizeof(entry.inet));
        MGMT_IP_FLT_DumpBlockCache(mode);
    }

    MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(ret);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_IsValidIpFilterAddress
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the ip address is belong valid
 *          ip filter group. Otherwise, false is returned.
 *
 * INPUT: 1. ip_address - ip address
 *        2. mode:
 *           - MGMT_IP_FLT_WEB(1).
 *           - MGMT_IP_FLT_SNMP(2).
 *           - MGMT_IP_FLT_TELNET(3).
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: 1. The function is only used by for checking the ip address
 *           of the manager is valid or not to access the device.
 *        2. 0.0.0.0 is invalid ip address
 *        3. If do not set any ip filter range, all ip can access device
 * ---------------------------------------------------------------------
 */
BOOL_T MGMT_IP_FLT_IsValidIpFilterAddress(UI32_T mode ,const L_INET_AddrIp_T *ip_address)
{
    UI32_T i;
    BOOL_T have_set = FALSE;
    MGMT_IP_FLT_IpFilter_T *ip_entry;

    if(shmem_data_p->mgmt_ip_flt_status == MGMT_IP_FLT_STATUS_DISABLE)
    {
        return TRUE;
    }

    MGMT_IP_FLT_USE_CSC_CHECK_OPER_MODE(FALSE, MGMT_IP_FLT_ISVALIDIPFILTERADDRESS, EH_TYPE_MSG_NOT_IN_MASTER_MODE);

    if (mode < MGMT_IP_FLT_HTTP || mode > MGMT_IP_FLT_SSH)
    {
        MGMT_IP_FLT_Send_Trap(mode, ip_address);
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (FALSE == MGMT_IP_FLT_IsVaildIP(ip_address))
    {
        MGMT_IP_FLT_Send_Trap(mode, ip_address);
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(FALSE);
    }

    /*
     * check the block cache first
     */
    if (TRUE == MGMT_IP_FLT_FindBlockCache(mode, ip_address))
    {
        MGMT_IP_FLT_Send_Trap(mode, ip_address);
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if ((ip_entry = MGMT_IP_FLT_Get_DataBaseByMode(mode)) == NULL)
    {
        MGMT_IP_FLT_Send_Trap(mode, ip_address);
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(FALSE);
    }

    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT; i++)
    {
        if (TRUE == MGMT_IP_FLT_IsVaildIP(&ip_entry[i].start_ipaddress) &&
            TRUE == MGMT_IP_FLT_IsVaildIP(&ip_entry[i].end_ipaddress))
        {
            have_set = TRUE;
        }
        /* for endian issue, use memcmp to do comparing.
         */
        if ( ((MGMT_IP_FLT_IP_EQUAL | MGMT_IP_FLT_IP_LARGE)
            & MGMT_IP_FLT_IpCompare( ip_address, &ip_entry[i].start_ipaddress) ) &&
            ((MGMT_IP_FLT_IP_EQUAL | MGMT_IP_FLT_IP_LESS)
            & MGMT_IP_FLT_IpCompare(ip_address, &ip_entry[i].end_ipaddress) )
	       )
        {
            MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(TRUE);
        }
    }
    if (have_set == TRUE)
    {
        MGMT_IP_FLT_Send_Trap(mode, ip_address);
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(FALSE);
    }
    else
    {
#if(SYS_CPNT_MANAGEMENT_IP_FILTER_DEFAULT_DENY ==TRUE)
        if(MGMT_IP_FLT_IsValidProtocol(mode)==TRUE)
        {
            MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(TRUE);
        }
        else
        {
            MGMT_IP_FLT_Send_Trap(mode, ip_address);
            MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(FALSE);
        }
#else
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(TRUE);
#endif
    }
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_GetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the specified IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. mode of each ip filter is defined as following:
 *           - MGMT_IP_FLT_WEB(1).
 *           - MGMT_IP_FLT_SNMP(2).
 *           - MGMT_IP_FLT_TELNET(3).
 *        2. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES: 1. The ip filter receiver can be accessed by CLI, SNMP and Web.
 *        2. The total number of ip filter receivers supported by the system
 *           is defined by sys_adpt.h.
 *        3. By default, there is no ip filter receiver configued in the system.
 * ---------------------------------------------------------------------
 */
static UI32_T MGMT_IP_FLT_GetIpFilter(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    UI32_T  i;
    MGMT_IP_FLT_IpFilter_T *local_ip_filter;

    MGMT_IP_FLT_USE_CSC_CHECK_OPER_MODE(MGMT_IP_FLT_NOT_IN_MASTER_MODE, MGMT_IP_FLT_GETIPFILTER, EH_TYPE_MSG_NOT_IN_MASTER_MODE);

    if (mode < MGMT_IP_FLT_HTTP || mode > MGMT_IP_FLT_SSH)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_GETIPFILTER,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO, "management IP filter mode");
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_MODE_IS_INVALID);
    }

    if (FALSE == MGMT_IP_FLT_IsVaildIP(&ip_filter_entry->start_ipaddress))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_GETIPFILTER,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO, "start IP address");
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_IP_IS_INVALID);
    }

    if ((local_ip_filter = MGMT_IP_FLT_Get_DataBaseByMode(mode)) == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_GETIPFILTER,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_CRIT, "pointer");
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_FAIL);
    }

    MGMT_IP_FLT_ENTER_CRITICAL_SECTION();

    for (i = 0; i<SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT; i++)
    {
        if (MGMT_IP_FLT_IP_EQUAL == MGMT_IP_FLT_IpCompare(
            &local_ip_filter[i].start_ipaddress, &ip_filter_entry->start_ipaddress))
        {
            ip_filter_entry -> end_ipaddress = local_ip_filter[i].end_ipaddress;
            MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
            MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_OK);
        }
    }
    MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
    EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_GETIPFILTER,
                             EH_TYPE_MSG_IPFLT_IP_RANGE, SYSLOG_LEVEL_INFO, "not exists");
    MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_IP_RANGE_NOT_EXISTED);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_GetNextIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the next available IP filter
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. mode of each ip filter is defined as following:
 *           - MGMT_IP_FLT_WEB(1).
 *           - MGMT_IP_FLT_SNMP(2).
 *           - MGMT_IP_FLT_TELNET(3).
 *        2. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES:Specify start address = 0 to get first entry
 * ---------------------------------------------------------------------
 */
static UI32_T MGMT_IP_FLT_GetNextIpFilter(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
    UI32_T  i;
    MGMT_IP_FLT_IpFilter_T *local_ip_filter;

    MGMT_IP_FLT_USE_CSC_CHECK_OPER_MODE(MGMT_IP_FLT_NOT_IN_MASTER_MODE, MGMT_IP_FLT_GETNEXTIPFILTER, EH_TYPE_MSG_NOT_IN_MASTER_MODE);

    if (mode < MGMT_IP_FLT_HTTP || mode > MGMT_IP_FLT_SSH)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_GETNEXTIPFILTER,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO, "management IP filter mode");
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_MODE_IS_INVALID);
    }

    if ((local_ip_filter = MGMT_IP_FLT_Get_DataBaseByMode(mode)) == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_GETNEXTIPFILTER,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_CRIT, "pointer");
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_FAIL);
    }

    MGMT_IP_FLT_ENTER_CRITICAL_SECTION();

    if (FALSE == MGMT_IP_FLT_IsVaildIP(&ip_filter_entry->start_ipaddress) &&
        TRUE == MGMT_IP_FLT_IsVaildIP(&local_ip_filter[0].start_ipaddress))
    {
        memcpy(ip_filter_entry, &(local_ip_filter[0]), sizeof(MGMT_IP_FLT_IpFilter_T));
        MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
        MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_OK);
    }
    else
    {
        for (i = 0; i<SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT; i++)
        {
            /* for endian issue, use memcmp to do comparing.
             */
            if (MGMT_IP_FLT_IP_LARGE == MGMT_IP_FLT_IpCompare(
                &local_ip_filter[i].start_ipaddress, &ip_filter_entry->start_ipaddress))
            {
                memcpy(ip_filter_entry, &(local_ip_filter[i]), sizeof(MGMT_IP_FLT_IpFilter_T));
                MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
                MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_OK);
            }
        }
    }
    MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
    EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_GETNEXTIPFILTER,
                             EH_TYPE_MSG_IPFLT_IP_RANGE, SYSLOG_LEVEL_INFO, "not exists");
    MGMT_IP_FLT_RETURN_AND_RELEASE_CSC(MGMT_IP_FLT_IP_RANGE_NOT_EXISTED);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_IsVaildIP
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns TRUE if the IP filter is valid.
 *          Otherwise, FALSE is returned.
 * INPUT  : ip_addr: to specify a unique ip filter address.
 * RETURN : TRUE or FALSE
 * NOTES  : none.
 * ---------------------------------------------------------------------
 */
static BOOL_T MGMT_IP_FLT_IsVaildIP(const L_INET_AddrIp_T *ip_addr)
{
    if (NULL == ip_addr)
    {
        return FALSE;
    }
    /* only support v4 and v6
     */
    if( (L_INET_ADDR_TYPE_IPV4 != ip_addr->type) && (L_INET_ADDR_TYPE_IPV6 != ip_addr->type)
        && (L_INET_ADDR_TYPE_IPV4Z != ip_addr->type) && (L_INET_ADDR_TYPE_IPV6Z != ip_addr->type) )
    {
        return FALSE;
    }

    if (L_INET_ADDR_TYPE_IPV4 == ip_addr->type || L_INET_ADDR_TYPE_IPV4Z == ip_addr->type)
    {
        if (FALSE == IP_LIB_IsValidForNetworkInterface((UI8_T *)ip_addr->addr))
        {
            return FALSE;
        }
    }
    else if (L_INET_ADDR_TYPE_IPV6 == ip_addr->type || L_INET_ADDR_TYPE_IPV6Z == ip_addr->type)
    {
        if (IP_LIB_OK != IP_LIB_CheckIPv6PrefixForInterface((UI8_T *)ip_addr->addr, 128))
        {
            return FALSE;
        }
    }

    if (L_INET_ADDR_TYPE_IPV6 == ip_addr->type)
    {
        static UI8_T zero_v6ip[SYS_ADPT_IPV6_ADDR_LEN] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        if (0 == memcmp(zero_v6ip, &ip_addr->addr, sizeof( zero_v6ip)))
        {
            return FALSE;
        }
    }
    return TRUE;
}

static UI32_T MGMT_IP_FLT_IpCompare(const L_INET_AddrIp_T *ip_addr_a, const L_INET_AddrIp_T *ip_addr_b)
{
    int cmp = 0;
    static UI8_T v4_prefix[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF};
    static int v4_prefix_len = sizeof(v4_prefix);
    UI8_T  temp_addr_a[SYS_ADPT_IPV6_ADDR_LEN]={0};
    UI8_T  temp_addr_b[SYS_ADPT_IPV6_ADDR_LEN]={0};

    /* convert to ipv4-map-ipv6
     */
    if( (L_INET_ADDR_TYPE_IPV4 == ip_addr_a->type)
        || (L_INET_ADDR_TYPE_IPV4Z == ip_addr_a->type) )
    {
        memset( temp_addr_a, 0, SYS_ADPT_IPV6_ADDR_LEN);
        memcpy( temp_addr_a, v4_prefix, v4_prefix_len);
        memcpy( &temp_addr_a[v4_prefix_len], &ip_addr_a->addr, SYS_ADPT_IPV4_ADDR_LEN);
    }
    else
    {
        memcpy( temp_addr_a, &ip_addr_a->addr, SYS_ADPT_IPV6_ADDR_LEN);
    }

    if( (L_INET_ADDR_TYPE_IPV4 == ip_addr_b->type)
        ||(L_INET_ADDR_TYPE_IPV4Z == ip_addr_b->type) )
    {
        memset( temp_addr_b, 0, SYS_ADPT_IPV6_ADDR_LEN);
        memcpy( temp_addr_b, v4_prefix, v4_prefix_len);
        memcpy( &temp_addr_b[v4_prefix_len], &ip_addr_b->addr, SYS_ADPT_IPV4_ADDR_LEN);
    }
    else
    {
        memcpy( temp_addr_b, &ip_addr_b->addr, SYS_ADPT_IPV6_ADDR_LEN);
    }

    cmp = memcmp(temp_addr_a, temp_addr_b, SYS_ADPT_IPV6_ADDR_LEN);

    if (0 == cmp)
    {
        return MGMT_IP_FLT_IP_EQUAL;
    }
    else if (0 < cmp)
    {
        return MGMT_IP_FLT_IP_LARGE;
    }
    else
    {
        return MGMT_IP_FLT_IP_LESS;
    }
}


#if 0
static void MGMT_IP_FLT_BackDoor_Menu(void)
{
    UI8_T   ch;
    UI8_T input_buff[2];

    while (1)
    {
        BACKDOOR_MGR_Printf("\r\n 1.WEB mode");
        BACKDOOR_MGR_Printf("\r\n 2.SNMP mode ");
        BACKDOOR_MGR_Printf("\r\n 3.TELNET mode");

        BACKDOOR_MGR_Printf("\r\n x: exit");
        BACKDOOR_MGR_RequestKeyIn(input_buff,2);
        ch=input_buff[0];
        switch(ch)
        {
        case '1':
		    MGMT_IP_FLT_BackDoor_SubMenu(MGMT_IP_FLT_HTTP);
            break;

        case '2':
		    MGMT_IP_FLT_BackDoor_SubMenu(MGMT_IP_FLT_SNMP);
            break;

        case '3':
		    MGMT_IP_FLT_BackDoor_SubMenu(MGMT_IP_FLT_TELNET);
            break;
        case '4':
		    MGMT_IP_FLT_BackDoor_SubMenu(MGMT_IP_FLT_HTTPS);
            break;
         case '5':
		    MGMT_IP_FLT_BackDoor_SubMenu(MGMT_IP_FLT_SSH);
            break;
        case 'x':
        case 'X':
            return;

        default:
            break;
        }
    }
}

static void MGMT_IP_FLT_BackDoor_SubMenu(UI32_T mode)
{
    UI8_T   choose;
    char	in_str[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
    char	ip_str[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
    UI32_T  i = 1;
    UI32_T  max_len=L_INET_MAX_IPADDR_STR_LEN;
    MGMT_IP_FLT_IpFilter_T ip_filter_entry;

    memset(&ip_filter_entry, 0, sizeof(MGMT_IP_FLT_IpFilter_T));

    BACKDOOR_MGR_Printf("\r\nMode = %lu",mode);
    BACKDOOR_MGR_Printf("\r\n1.Add entry");
    BACKDOOR_MGR_Printf("\r\n2.Delete entry");
    BACKDOOR_MGR_Printf("\r\n3.Set entry by snmp");
    BACKDOOR_MGR_Printf("\r\n4.Show entry");
    BACKDOOR_MGR_Printf("\r\n5.Check ip is valid");
    BACKDOOR_MGR_Printf("\r\n6.Get running");
    BACKDOOR_MGR_Printf("\r\nChoose (1-4):");
    BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
    choose=atoi(in_str);

    switch(choose)
    {
    case 1:
        BACKDOOR_MGR_Printf("\r\n Start address:");
        BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
        if (in_str[0] != '\0')
        {
            MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &(ip_filter_entry.start_ipaddress));
        }
        BACKDOOR_MGR_Printf("\r\n End address:");
        BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
        if (in_str[0] != '\0')
        {
            MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &(ip_filter_entry.end_ipaddress));
        }
        if (MGMT_IP_FLT_SetIpFilter(mode,&ip_filter_entry) != MGMT_IP_FLT_OK)
        {
            BACKDOOR_MGR_Printf("\r\nFailed to add this entry\r\n");
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\nSuccess!!!\r\n\r\n");
        }
        break;

    case 2:
        BACKDOOR_MGR_Printf("\r\n Start address:");
        BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
        if (in_str[0] != '\0')
        {
            MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &(ip_filter_entry.start_ipaddress));
        }
        BACKDOOR_MGR_Printf("\r\n End address:");
        BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
        if (in_str[0] != '\0')
        {
            MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &(ip_filter_entry.end_ipaddress));
        }
        if (MGMT_IP_FLT_DeleteIpFilter(mode,&ip_filter_entry) != MGMT_IP_FLT_OK)
        {
            BACKDOOR_MGR_Printf("\r\nFailed to delete this entry\r\n");
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\nSuccess!!!\r\n\r\n");
        }
        break;

    case 3:
    {
        BACKDOOR_MGR_Printf("\r\nStatus(valid/invalid) enter for set entry:");
        BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
        if (in_str[0] == 'v' || in_str[0] == 'V')
        {
            BACKDOOR_MGR_Printf("\r\n Start address(key):");
            BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
            if (in_str[0] != '\0')
            {
                MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &(ip_filter_entry.start_ipaddress));
            }
            memcpy(&ip_filter_entry.end_ipaddress, &ip_filter_entry.start_ipaddress,
                    sizeof(ip_filter_entry.end_ipaddress));
            if (MGMT_IP_FLT_SetIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
                BACKDOOR_MGR_Printf("\r\nFailed to set status\r\n");
            }
            else
            {
                BACKDOOR_MGR_Printf("\r\n Success!!!\r\n\r\n");
            }
            return;
        }
        else if (in_str[0] == 'i' || in_str[0] == 'I')
        {
            ip_filter_entry.end_ipaddress.addrlen = 0;
            BACKDOOR_MGR_Printf("\r\n Start address(key):");
            BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
            if (in_str[0] != '\0')
            {
                MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &(ip_filter_entry.start_ipaddress));
            }
            if (MGMT_IP_FLT_DeleteIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
                BACKDOOR_MGR_Printf("\r\nFailed to set status\r\n");
            }
            else
            {
                BACKDOOR_MGR_Printf("\r\n Success!!!\r\n\r\n");
            }
            return;
        }
        BACKDOOR_MGR_Printf("\r\n Start address(key):");
        BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
        if (in_str[0] != '\0')
        {
            MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &(ip_filter_entry.start_ipaddress));
        }
        BACKDOOR_MGR_Printf("\r\n End address:");
        BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
        if (in_str[0] != '\0')
        {
            MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &(ip_filter_entry.end_ipaddress));
        }
        if (MGMT_IP_FLT_SetIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
        {
            BACKDOOR_MGR_Printf("\r\nFailed to set entry\r\n");
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\n Success!!!\r\n\r\n");
        }
    }
        break;

    case 4:
        BACKDOOR_MGR_Printf("\r\n1.Get existed entry");
        BACKDOOR_MGR_Printf("\r\n2.GetNext entry");
        BACKDOOR_MGR_Printf("\r\nChoose (1-2):");
        BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
        choose=atoi(in_str);
        switch(choose)
        {
        case 1:
            BACKDOOR_MGR_Printf("\r\n Start address:");
            BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
            if (in_str[0] == '\0')
               break;
            MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &(ip_filter_entry.start_ipaddress));

            switch(mode)
            {
            case MGMT_IP_FLT_HTTP:
                if (MGMT_IP_FLT_Web_GetIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
                {
                    BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
                    L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\nStart address = %s",ip_str);
                    L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\nEnd address = %s",ip_str);
                }
                break;

            case MGMT_IP_FLT_SNMP:
                if (MGMT_IP_FLT_Snmp_GetIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
                {
                    BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
                    L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\nStart address = %s",ip_str);
                    L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\nEnd address = %s",ip_str);
                }
                break;

            case MGMT_IP_FLT_TELNET:
                if (MGMT_IP_FLT_Telnet_GetIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
                {
                    BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
                    L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\nStart address = %s",ip_str);
                    L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\nEnd address = %s",ip_str);
                }
                break;

            case MGMT_IP_FLT_HTTPS:
                if (MGMT_IP_FLT_Telnet_GetIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
                {
                    BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
                    L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\nStart address = %s",ip_str);
                    L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\nEnd address = %s",ip_str);
                }
                break;

            case MGMT_IP_FLT_SSH:
                if (MGMT_IP_FLT_Telnet_GetIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
                {
                    BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
                    L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\nStart address = %s",ip_str);
                    L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\nEnd address = %s",ip_str);
                }
                break;
            default:
                break;
            }
            break;

        case 2:
            BACKDOOR_MGR_Printf("\r\n Start address(enter)to get first:");
            BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
            if (in_str[0] != '\0')
            {
                MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &(ip_filter_entry.start_ipaddress));
            }
            else
            {
                memset(&ip_filter_entry, 0,sizeof(MGMT_IP_FLT_IpFilter_T));
            }
            switch(mode)
            {
            case MGMT_IP_FLT_HTTP:
                BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
                while (MGMT_IP_FLT_WEB_GetNextIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
                {
                    L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\n%02lu.Start address = %s",i ,ip_str);
                    L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\n   End address = %s",ip_str);
                    i++;
                }
                break;

            case MGMT_IP_FLT_SNMP:
                BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
                while (MGMT_IP_FLT_Snmp_GetNextIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
                {
                    L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\n%02lu.Start address = %s",i ,ip_str);
                    L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\n   End address = %s",ip_str);
                    i++;
                }
                break;

            case MGMT_IP_FLT_TELNET:
                BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
                while (MGMT_IP_FLT_TELNET_GetNextIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
                {
                    L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\n%02lu.Start address = %s",i ,ip_str);
                    L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\n   End address = %s",ip_str);
                    i++;
                }
                break;
             case MGMT_IP_FLT_HTTPS:
                BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
                while (MGMT_IP_FLT_TELNET_GetNextIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
                {
                    L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\n%02lu.Start address = %s",i ,ip_str);
                    L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\n   End address = %s",ip_str);
                    i++;
                }
                break;

              case MGMT_IP_FLT_SSH:
                BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
                while (MGMT_IP_FLT_TELNET_GetNextIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
                {
                    L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\n%02lu.Start address = %s",i ,ip_str);
                    L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                        ip_str, sizeof(ip_str));
                    BACKDOOR_MGR_Printf("\r\n   End address = %s",ip_str);
                    i++;
                }
                break;
            default:
                break;
            }
            break;

        default:
            break;
        }

        break;

    case 5:
        {
            L_INET_AddrIp_T check_inaddr;

            memset(&check_inaddr, 0, sizeof(check_inaddr));

            BACKDOOR_MGR_Printf("\r\n IP address:");
            BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
            MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &check_inaddr);
            BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);

            if (MGMT_IP_FLT_IsValidIpFilterAddress(mode, &check_inaddr) == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n This ip is valid ip address\r\n");
            }
            else
            {
                BACKDOOR_MGR_Printf("\r\n This ip is invalid ip address\r\n");
            }
        }

        break;

    case 6:
        BACKDOOR_MGR_Printf("\r\n Start address(enter)to get first:");
        BACKDOOR_MGR_RequestKeyIn(in_str,max_len);
        if (in_str[0] != '\0')
        {
            MGMT_IP_FLT_BackDoor_ConvertString2Inet(in_str, &(ip_filter_entry.start_ipaddress));
        }
        else
        {
            memset(&ip_filter_entry, 0,sizeof(MGMT_IP_FLT_IpFilter_T));
        }
        switch(mode)
        {
        case MGMT_IP_FLT_HTTP:
            BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
            while (MGMT_IP_FLT_GetNextRunningIpFilter(MGMT_IP_FLT_WEB, &ip_filter_entry) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
            {
                L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                    ip_str, sizeof(ip_str));
                BACKDOOR_MGR_Printf("\r\n%02lu.Start address = %s",i ,ip_str);
                L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                    ip_str, sizeof(ip_str));
                BACKDOOR_MGR_Printf("\r\n   End address = %s",ip_str);
                i++;
            }
            break;

        case MGMT_IP_FLT_SNMP:
            BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
            while (MGMT_IP_FLT_GetNextRunningIpFilter(MGMT_IP_FLT_SNMP, &ip_filter_entry) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
            {
                L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                    ip_str, sizeof(ip_str));
                BACKDOOR_MGR_Printf("\r\n%02lu.Start address = %s",i ,ip_str);
                L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                    ip_str, sizeof(ip_str));
                BACKDOOR_MGR_Printf("\r\n   End address = %s",ip_str);
                i++;
            }
            break;

        case MGMT_IP_FLT_TELNET:
            BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
            while (MGMT_IP_FLT_GetNextRunningIpFilter(MGMT_IP_FLT_TELNET, &ip_filter_entry) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
            {
                L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                    ip_str, sizeof(ip_str));
                BACKDOOR_MGR_Printf("\r\n%02lu.Start address = %s",i ,ip_str);
                L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                    ip_str, sizeof(ip_str));
                BACKDOOR_MGR_Printf("\r\n   End address = %s",ip_str);
                i++;
            }
            break;
        case MGMT_IP_FLT_HTTPS:
            BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
            while (MGMT_IP_FLT_GetNextRunningIpFilter(MGMT_IP_FLT_TELNET, &ip_filter_entry) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
            {
                L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                    ip_str, sizeof(ip_str));
                BACKDOOR_MGR_Printf("\r\n%02lu.Start address = %s",i ,ip_str);
                L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                    ip_str, sizeof(ip_str));
                BACKDOOR_MGR_Printf("\r\n   End address = %s",ip_str);
                i++;
            }
            break;
        case MGMT_IP_FLT_SSH:
            BACKDOOR_MGR_Printf("\r\nMode = %lu", mode);
            while (MGMT_IP_FLT_GetNextRunningIpFilter(MGMT_IP_FLT_TELNET, &ip_filter_entry) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
            {
                L_INET_InaddrToString(&(ip_filter_entry.start_ipaddress),
                    ip_str, sizeof(ip_str));
                BACKDOOR_MGR_Printf("\r\n%02lu.Start address = %s",i ,ip_str);
                L_INET_InaddrToString(&(ip_filter_entry.end_ipaddress),
                    ip_str, sizeof(ip_str));
                BACKDOOR_MGR_Printf("\r\n   End address = %s",ip_str);
                i++;
            }
            break;
        default:
            break;
        }
        break;


    default:
        break;
    }

}

static void MGMT_IP_FLT_BackDoor_ConvertString2Inet(const UI8_T *in_str, L_INET_AddrIp_T *inaddr_p)
{
    if(FALSE == L_INET_StringToInaddr(in_str, sizeof(L_INET_AddrIp_T), (L_INET_Addr_T*)inaddr_p))
    {
        BACKDOOR_MGR_Printf("\r\nFailed to convert string '%s' to Inert Address\r\n",in_str);
    }
}

#endif
static void MGMT_IP_FLT_Rearrange_DataBase(UI32_T mode)
{
    I32_T i = 0;
    UI32_T j = 0;

    MGMT_IP_FLT_IpFilter_T *ip_entry;
    MGMT_IP_FLT_IpFilter_T temp_entry;
    if ((ip_entry = MGMT_IP_FLT_Get_DataBaseByMode(mode)) == NULL)
    {
        return;
    }
	    MGMT_IP_FLT_ENTER_CRITICAL_SECTION();
#if 0
    /*link the continuous ip ranges*/
    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT - 1; i++)
    {
        if (ip_entry[i].start_ipaddress == 0)
        {
            continue;
        }
        for (j = i; j < SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT ; j++)
        {
            if (ip_entry[j].start_ipaddress == 0)
            {
                continue;
            }
            if ((ip_entry[i].end_ipaddress + 1) == ip_entry[j].start_ipaddress)
            {
                ip_entry[i].end_ipaddress = ip_entry[j].end_ipaddress;
                ip_entry[j].start_ipaddress = 0;
                ip_entry[j].end_ipaddress = 0;
            }
        }
    }
#endif
    /*use bubble sort*/
    for (i = SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT - 1; i > 0; i--)
    {
        for (j = 0; j < i ; j++)
        {
            /*start address = 0 treat as the biggest value, so do not swap*/
            /* for endian issue, use memcmp to do comparing.
             */
            if ( ( MGMT_IP_FLT_IP_LARGE == MGMT_IP_FLT_IpCompare(
                &ip_entry[j].start_ipaddress, &ip_entry[j+1].start_ipaddress) &&
                TRUE == MGMT_IP_FLT_IsVaildIP(&ip_entry[j+1].start_ipaddress) ) ||
                ( FALSE == MGMT_IP_FLT_IsVaildIP(&ip_entry[j].start_ipaddress) &&
                TRUE == MGMT_IP_FLT_IsVaildIP(&ip_entry[j+1].start_ipaddress) )
                )
            {
                memcpy(&temp_entry, &(ip_entry[j]), sizeof(MGMT_IP_FLT_IpFilter_T));
                memcpy(&(ip_entry[j]), &(ip_entry[j+1]), sizeof(MGMT_IP_FLT_IpFilter_T));
                memcpy(&(ip_entry[j+1]), &temp_entry, sizeof(MGMT_IP_FLT_IpFilter_T));
            }
            else
            {

            }
        }
    }
    MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();
    return;
}

static MGMT_IP_FLT_IpFilter_T *MGMT_IP_FLT_Get_DataBaseByMode(UI32_T mode)
{
    MGMT_IP_FLT_IpFilter_T *ip_filter;

    switch(mode)
    {
    case MGMT_IP_FLT_HTTP:
        ip_filter = shmem_data_p->http_ip_filter;
        break;

    case MGMT_IP_FLT_SNMP:
        ip_filter = shmem_data_p->snmp_ip_filter;
        break;

    case MGMT_IP_FLT_TELNET:
        ip_filter = shmem_data_p->telnet_ip_filter;
        break;

    default :
        ip_filter = NULL;
        break;
    }
    return ip_filter;
}


static UI32_T MGMT_IP_FLT_Check_Parameters(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry)
{
#if(SYS_CPNT_MANAGEMENT_IP_FILTER_DEFAULT_DENY==TRUE)
    UI32_T address_mask;
    MGMT_IP_FLT_IpFilter_T after_calculating_ip_filter_entry;
    memset(&after_calculating_ip_filter_entry,0,sizeof(MGMT_IP_FLT_IpFilter_T));
#endif

    if (mode < MGMT_IP_FLT_HTTP || mode > MGMT_IP_FLT_SSH)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_CHECK_PARAMETERS,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO, "management IP filter mode");
        return MGMT_IP_FLT_MODE_IS_INVALID;
    }

    if (FALSE == MGMT_IP_FLT_IsVaildIP(&ip_filter_entry->start_ipaddress) ||
        FALSE == MGMT_IP_FLT_IsVaildIP(&ip_filter_entry->end_ipaddress))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_CHECK_PARAMETERS,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO, "start or end IP address");
       return MGMT_IP_FLT_IP_IS_INVALID;
    }

    /* not support that start address type is different from end address type
     */
    if(ip_filter_entry->start_ipaddress.type != ip_filter_entry->end_ipaddress.type)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_CHECK_PARAMETERS,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO, "start or end IP address");
       return MGMT_IP_FLT_IP_IS_INVALID;
    }

    /* for endian issue, use memcmp to do comparing.
     */
    if (MGMT_IP_FLT_IP_LARGE == MGMT_IP_FLT_IpCompare(
        &ip_filter_entry->start_ipaddress, &ip_filter_entry->end_ipaddress) &&
        TRUE == MGMT_IP_FLT_IsVaildIP(&ip_filter_entry->end_ipaddress))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_CHECK_PARAMETERS,
                                 EH_TYPE_MSG_IPFLT_IP_RANGE, SYSLOG_LEVEL_INFO, "start address larger than end address");
        return MGMT_IP_FLT_IP_RANGE_START_BIG_THEN_END;
    }

#if(SYS_CPNT_MANAGEMENT_IP_FILTER_DEFAULT_DENY==TRUE)
#if 0
    address_mask = ~(ip_filter_entry.start_ipaddress^ip_filter_entry.end_ipaddress);
    if(!IP_LIB_IsValidNetworkMask(address_mask))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_CHECK_PARAMETERS,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO, "end IP address");
        return MGMT_IP_FLT_IP_IS_INVALID;
    }
    else
    {
        MGMT_IP_FLT_ConverAddressToSubnet(ip_filter_entry.start_ipaddress,address_mask,&after_calculating_ip_filter_entry);
        if((ip_filter_entry.start_ipaddress!=after_calculating_ip_filter_entry.start_ipaddress)||(ip_filter_entry.end_ipaddress!=after_calculating_ip_filter_entry.end_ipaddress))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_CLI, MGMT_IP_FLT_CHECK_PARAMETERS,
                                 EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO, "start or end IP address");
            return MGMT_IP_FLT_IP_IS_INVALID;
        }
    }
#endif  /* if 0 */
#endif

    return MGMT_IP_FLT_OK;
}

static void MGMT_IP_FLT_Send_Trap(UI32_T mode, const L_INET_AddrIp_T *ip)
{
    TRAP_EVENT_TrapData_T data;

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
    data.trap_type=TRAP_EVENT_IPFILTER_INET_REJECT_TRAP;
    data.u.ipFilterInet_reject_trap.mode = mode; //1:web; 2:snmp; 3:Telnet
    data.u.ipFilterInet_reject_trap.inet_ip = *ip;//the reject ip
    data.community_specified = FALSE;
    SNMP_PMGR_ReqSendTrapOptional(&data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif
}

#if(SYS_CPNT_MANAGEMENT_IP_FILTER_DEFAULT_DENY ==TRUE)
BOOL_T MGMT_IP_FLT_IsValidProtocol(UI32_T mode)
{
#if 0
    UI32_T  mask_address=0;
    UI32_T  method=0;
    L_INET_AddrIp_T ip_address;
    L_INET_AddrIp_T start_ipaddress,end_ipaddress;
    MGMT_IP_FLT_IpFilter_T ip_filter_entry;

    memset(&ip_filter_entry,0,sizeof(MGMT_IP_FLT_IpFilter_T));

    while(MGMT_IP_FLT_GetNextIpMethodSubnet(&ip_address,&mask_address,&method))
    {
        if(MGMT_IP_FLT_ConverAddressToSubnet(ip_address,mask_address, &ip_filter_entry)==TRUE)
        {
            start_ipaddress = ip_filter_entry.start_ipaddress;
            end_ipaddress = ip_filter_entry.end_ipaddress;

            /* for endian issue, use memcmp to do comparing.
             */
            if ( ((	MGMT_IP_FLT_IP_EQUAL | MGMT_IP_FLT_IP_LARGE) &
                MGMT_IP_FLT_IpCompare(&ip_address, &ip_filter_entry.start_ipaddress) ) &&
                ((	MGMT_IP_FLT_IP_EQUAL | MGMT_IP_FLT_IP_LESS) &
                MGMT_IP_FLT_IpCompare(&ip_address, &ip_filter_entry.end_ipaddress) )
                )
            {
                if(method==mode)
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
    }
#endif  /* if 0 */
    return TRUE;
}
#endif

static BOOL_T MGMT_IP_FLT_GetNextBlockCacheEntry(
                                                 UI32_T mode,
                                                 UI32_T *index_p,
                                                 MGMT_IP_FLT_BlockCacheEntry_T *entry_p
                                                 )
{
    MGMT_IP_FLT_BlockCache_T *cache_p;

    cache_p = MGMT_IP_FLT_GetBlockCache(mode);

    if (NULL == cache_p)
    {
        return FALSE;
    }

    UI32_T BLOCK_CACHE_SIZE = sizeof(cache_p->entries)/sizeof(cache_p->entries[0]);

    if (BLOCK_CACHE_SIZE < *index_p)
    {
        return FALSE;
    }

    if (FALSE == cache_p->entries[*index_p].available)
    {
        return FALSE;
    }

    *entry_p = cache_p->entries[*index_p];

    ++ *index_p;

    return TRUE;
}

static void MGMT_IP_FLT_RemoveAgingBlockCacheEntry(
                                                   UI32_T mode
                                                   )
{
    UI32_T                        i     = 0;
    UI32_T                        old_i;
    MGMT_IP_FLT_BlockCacheEntry_T entry = {0};
    UI32_T                        current_tick = SYSFUN_GetSysTick();

    old_i = i;
    while (MGMT_IP_FLT_GetNextBlockCacheEntry(mode, &i, &entry))
    {
        if (entry.alive_time <= current_tick)
        {
            MGMT_IP_FLT_DeleteBlockCacheEntry(mode, old_i);
            i = 0;
            old_i = i;
        }
        else
        {
            break;
        }
    }
}



static BOOL_T MGMT_IP_FLT_FindBlockCache(
                                         UI32_T mode,
                                         const L_INET_AddrIp_T *ip_p
                                         )
{
    UI32_T                        i = 0;
    MGMT_IP_FLT_BlockCacheEntry_T entry = {0};

    MGMT_IP_FLT_RemoveAgingBlockCacheEntry(mode);

    if (0)
    {
        MGMT_IP_FLT_DumpBlockCache(mode);
        MGMT_IP_FLT_DumpHex((UI8_T*)ip_p, sizeof(L_INET_AddrIp_T));
    }

    while (MGMT_IP_FLT_GetNextBlockCacheEntry(mode, &i, &entry))
    {
        if (0)
        {
            int cmp;

            MGMT_IP_FLT_DumpHex((UI8_T*)&entry.inet, sizeof(L_INET_AddrIp_T));

            cmp = memcmp(&entry.inet, ip_p, sizeof(L_INET_AddrIp_T));
            printf("cmp=%d\n", cmp);
        }

        if (L_INET_CompareInetAddr((L_INET_Addr_T *) ip_p,
            (L_INET_Addr_T *) &entry.inet, 0) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL_T MGMT_IP_FLT_DeleteBlockCacheEntry(
                                                UI32_T mode,
                                                UI32_T index
                                                )
{
    MGMT_IP_FLT_BlockCache_T        *cache_p;
    BOOL_T                          ret;

    cache_p = MGMT_IP_FLT_GetBlockCache(mode);

    if (NULL == cache_p)
    {
        return FALSE;
    }

    MGMT_IP_FLT_ENTER_CRITICAL_SECTION();

    ret = MGMT_IP_FLT_BlockCache_Ary_Del(cache_p, index);

    MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();

    return ret;
}

static MGMT_IP_FLT_BlockCache_T* MGMT_IP_FLT_GetBlockCache(
                                                           UI32_T mode
                                                           )
{
    if (mode < MGMT_IP_FLT_MIN_MODE || MGMT_IP_FLT_MAX_MODE < mode)
    {
        return NULL;
    }

    return &shmem_data_p->block_cache[mode-1];
}

static BOOL_T MGMT_IP_FLT_BlockCache_Add(
                                         MGMT_IP_FLT_BlockCache_T *this,
                                         MGMT_IP_FLT_BlockCacheEntry_T *entry_p
                                         )
{
    UI32_T  i;

    MGMT_IP_FLT_ENTER_CRITICAL_SECTION();

    /* duplicate inet ?
     * remove it before insert
     */
    for (i=0; i < MGMT_IP_FLT_BLOCK_CACHE_SIZE; ++i)
    {
        if (!this->entries[i].available)
            continue;

        if (L_INET_CompareInetAddr((L_INET_Addr_T *) &entry_p->inet,
            (L_INET_Addr_T *) &this->entries[i].inet, 0) == 0)
        {
            MGMT_IP_FLT_BlockCache_Ary_Del(this, i);
            break;
        }
    }

    /* full ?
     * remove the entry which have the shortest alive time
     */
    if (TRUE == this->entries[MGMT_IP_FLT_BLOCK_CACHE_SIZE-1].available)
    {
        MGMT_IP_FLT_BlockCache_Ary_Del(this, 0);
    }

    for (i=0; i < MGMT_IP_FLT_BLOCK_CACHE_SIZE; ++i)
    {
        if (this->entries[i].available)
        {
            if (entry_p->alive_time < this->entries[i].alive_time)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    MGMT_IP_FLT_BlockCache_Ary_Add(this, i, entry_p);

    MGMT_IP_FLT_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

static BOOL_T MGMT_IP_FLT_BlockCache_Ary_Add(
                                             MGMT_IP_FLT_BlockCache_T *this,
                                             UI32_T index,
                                             MGMT_IP_FLT_BlockCacheEntry_T *entry_p
                                             )
{
    UI32_T BLOCK_CACHE_SIZE = sizeof(this->entries)/sizeof(this->entries[0]);
    UI32_T i;

    if (BLOCK_CACHE_SIZE < index)
    {
        return FALSE;
    }

    for (i=BLOCK_CACHE_SIZE-1; index <= i; --i)
    {
        if (0 == i)
            break;

        memcpy(&this->entries[i],
               &this->entries[i-1],
               sizeof(this->entries[0])
               );
    }

    memcpy(&this->entries[index], entry_p, sizeof(this->entries[0]));
    this->entries[index].available = TRUE;
    return TRUE;
}

static BOOL_T MGMT_IP_FLT_BlockCache_Ary_Del(
                                             MGMT_IP_FLT_BlockCache_T *this,
                                             UI32_T index
                                            )
{
    UI32_T BLOCK_CACHE_SIZE = sizeof(this->entries)/sizeof(this->entries[0]);
    UI32_T i;

    if (BLOCK_CACHE_SIZE < index)
    {
        return FALSE;
    }

    if (FALSE == this->entries[index].available)
    {
        return TRUE;
    }

    for (i=index; i < BLOCK_CACHE_SIZE-1; ++i)
    {
        memcpy(&this->entries[i],
                &this->entries[i+1],
                sizeof(this->entries[0])
                );
    }

    memset(&this->entries[BLOCK_CACHE_SIZE-1], 0, sizeof(this->entries[0]));
    this->entries[BLOCK_CACHE_SIZE-1].available = FALSE;
    return TRUE;
}

static void MGMT_IP_FLT_DumpBlockCache(
                                       UI32_T mode
                                       )
{
    UI32_T i=0;
    MGMT_IP_FLT_BlockCacheEntry_T entry = {0};
    char   ip[L_INET_MAX_IPADDR_STR_LEN+1];

    while (MGMT_IP_FLT_GetNextBlockCacheEntry(mode, &i, &entry))
    {
        L_INET_InaddrToString((L_INET_Addr_T*)&entry.inet, ip, sizeof(ip));
        printf("alive_time = %05lu, inet = %s\n", (unsigned long)entry.alive_time, ip);
    }
}

static void MGMT_IP_FLT_DumpHex(
                                UI8_T *data,
                                UI32_T size
                                )
{
    enum{MAX_LEN_OF_LINE = 16};
    UI32_T  row_idx, index;
    UI8_T   line_ar[MAX_LEN_OF_LINE*3 + 2 + 1];
    UI8_T   token_ar[3+1];

    memset(line_ar, 0, sizeof(line_ar));
    for (row_idx = 0, index = 0; size > index; ++row_idx, ++index, ++data)
    {
        if (MAX_LEN_OF_LINE <= row_idx)
            row_idx = 0;

        if (0 == row_idx)
        {
            printf("%s\n", line_ar);
            memset(line_ar, 0, sizeof(line_ar));
        }

        sprintf((char *)token_ar,"%02X ", *data);
        strcat((char *)line_ar, (char *)token_ar);
    }

    printf("%s\n", line_ar);
}


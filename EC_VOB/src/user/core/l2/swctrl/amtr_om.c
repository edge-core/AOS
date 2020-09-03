/*-------------------------------------------------------------------------
 * MODULE NAME: amtr_om.c
 *-------------------------------------------------------------------------
 * PURPOSE: To store and manage AMTR Hisam Table.
 *
 *
 * NOTES:
 *
 * Modification History:
 *      Date                Modifier        Reason
 *      ------------------------------------
 *      03-15-2005    water_huang    create
 *
 * COPYRIGHT(C)         Accton Corporation, 2005
 *------------------------------------------------------------------------*/


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"
#include "amtr_om.h"
#include "amtr_om_private.h"
#include "amtr_type.h"
#include "amtr_mgr.h"

/* NAMING CONSTANT DECLARARTIONS
*/

/* TYPE DEFINITIONS
*/


/* MACRO DEFINITIONS
 */
#define IS_IFINDEX_INVALID(ifindex)    ((ifindex == 0)||(ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT))

#define AMTR_OM_ENTER_CRITICAL_SECTION()                                \
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtr_om_sem_id);

#define AMTR_OM_LEAVE_CRITICAL_SECTION()                              \
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtr_om_sem_id, original_priority);

#define AMTR_OM_ATOM_EXPRESSION(exp) { AMTR_OM_ENTER_CRITICAL_SECTION();\
                                       exp;\
                                       AMTR_OM_LEAVE_CRITICAL_SECTION();\
                                     }

/* LOCAL FUNCTIONS DECLARATIONS
 */

/* LOCAL VARIABLES DECLARATIONS
 */
static UI32_T amtr_om_sem_id;
static UI32_T original_priority;

/* shared memory variables
 */
static AMTR_MGR_Shmem_Data_T *amtr_om_shmem_data_p;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*---------------------------------------------------------------------------------
 * FUNCTION : void AMTR_OM_InitiateSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void AMTR_OM_InitiateSystemResources(void)
{
    amtr_om_shmem_data_p = SYSRSC_MGR_GetShMem(SYSRSC_MGR_AMTR_MGR_SHMEM_SEGID);
    memset(amtr_om_shmem_data_p, 0, sizeof(AMTR_MGR_Shmem_Data_T));
    SYSFUN_INITIATE_CSC_ON_SHMEM(amtr_om_shmem_data_p);

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_AMTR_OM, &amtr_om_sem_id)
            != SYSFUN_OK)
    {
        printf("%s: get om sem id fail.\n", __FUNCTION__);
    }

}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void AMTR_OM_AttachSystemResources(void)
{
    amtr_om_shmem_data_p = SYSRSC_MGR_GetShMem(SYSRSC_MGR_AMTR_MGR_SHMEM_SEGID);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the size for share memory
 * INPUT    : *segid_p
 *            *seglen_p
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void AMTR_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_AMTR_MGR_SHMEM_SEGID;
    *seglen_p = sizeof(AMTR_MGR_Shmem_Data_T);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_OM_GetOperatingMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return AMTR csc operating mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : SYS_TYPE_STACKING_MASTER_MODE
 *           SYS_TYPE_STACKING_SLAVE_MODE
 *           SYS_TYPE_STACKING_TRANSITION_MODE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetOperatingMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(amtr_om_shmem_data_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_OM_GetPortInfoPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will return pointer to amtr port info.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This API is only allowed to be called in AMTR internally.
 *            This API is defined here because AMTR_PMGR_IsPortSecurityEnabled()
 *            will call this function.
 *-------------------------------------------------------------------------
 */

AMTR_MGR_PortInfo_T* AMTR_OM_GetPortInfoPtr(void)
{
    if(amtr_om_shmem_data_p==NULL)
    {
        printf("\r\n%s(): Pointer to shared memory data is NULL.", __FUNCTION__);
        return NULL;
    }

    return amtr_om_shmem_data_p->amtr_port_info;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetCountersPerSystem
 *------------------------------------------------------------------------
 * PURPOSE: This function will get per-system counters such as total dynamic
 *          mac counter and total static mac counter.
 * INPUT  : None
 * OUTPUT : counter_p
 * RETURN : None
 * NOTES  : Total counter means how many entries in the system.
 *------------------------------------------------------------------------*/
void AMTR_OM_GetCountersPerSystem(AMTR_OM_CountersPerSystem_T *counter_p)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();
    counter_p->total_count = amtr_om_shmem_data_p->counters.total_address_count;
    counter_p->static_count = amtr_om_shmem_data_p->counters.total_static_address_count;
    counter_p->dynamic_count = amtr_om_shmem_data_p->counters.total_dynamic_address_count;
    AMTR_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetTotalCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total counter.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : Value of the total counter
 * NOTES  : Total counter means how many entries in the system.
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetTotalCounter(void)
{
    UI32_T total_address_count;

    AMTR_OM_ATOM_EXPRESSION(total_address_count=amtr_om_shmem_data_p->counters.total_address_count)
    return total_address_count;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetTotalDynamicCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total dynamic counter in the system
 * INPUT  : None
 * OUTPUT : None
 * RETURN : Value of the total dynamic counter
 * NOTES  : Total dynamic counter means how many dynamic entries in the
 *          system.
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetTotalDynamicCounter(void)
{
    UI32_T total_dynamic_address_count;

    AMTR_OM_ATOM_EXPRESSION(total_dynamic_address_count=amtr_om_shmem_data_p->counters.total_dynamic_address_count)
    return total_dynamic_address_count;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetTotalStaticCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total static counter in the system.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : Value of the total static counter
 * NOTES  : Total static counter means how many static entries in the system.
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetTotalStaticCounter(void)
{
    UI32_T total_static_address_count;

    AMTR_OM_ATOM_EXPRESSION(total_static_address_count=amtr_om_shmem_data_p->counters.total_static_address_count)
    return total_static_address_count;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetStaticCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get static counter of the specified port.
 * INPUT  : ifindex	- ifindex of the port to get the static counter
 * OUTPUT : None
 * RETURN : Value of the static counter of the specified port.
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetStaticCounterByPort(UI32_T ifindex)
{
    UI32_T address_count_by_port;

    AMTR_OM_ATOM_EXPRESSION(address_count_by_port=amtr_om_shmem_data_p->counters.static_address_count_by_port[ifindex-1])
    return address_count_by_port;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetDynCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get dynamic counter of the specified port.
 * INPUT  : ifindex	- ifindex of the port to get the dynamic counter
 * OUTPUT : None
 * RETURN : Value of the dynamic counter of the specified port
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetDynCounterByPort(UI32_T ifindex)
{
    UI32_T address_count_by_port;

    AMTR_OM_ATOM_EXPRESSION(address_count_by_port=amtr_om_shmem_data_p->counters.dynamic_address_count_by_port[ifindex-1])
    return address_count_by_port;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetDynCounterByVid
 *------------------------------------------------------------------------
 * PURPOSE: This function will get the dynamic counter of the specified vid.
 * INPUT  : vid  -  vlan id
 * OUTPUT : None
 * RETURN : Value of the dynamic counter of the specified vlan id
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetDynCounterByVid(UI32_T vid)
{
    UI32_T address_count_by_vid;

    AMTR_OM_ATOM_EXPRESSION(address_count_by_vid=amtr_om_shmem_data_p->counters.dynamic_address_count_by_vid[vid-1])
    return address_count_by_vid;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetLearntCounterByport
 *------------------------------------------------------------------------
 * PURPOSE: This function will get the learnt counter by the specified port.
 * INPUT  : ifindex	- ifindex of the port to get the learnt counter
 * OUTPUT : None
 * RETURN : Value of the learnt counter of the specified port
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetLearntCounterByport(UI32_T ifindex)
{
    UI32_T address_count_by_port;

    AMTR_OM_ATOM_EXPRESSION(address_count_by_port=amtr_om_shmem_data_p->counters.learnt_address_count_by_port[ifindex-1])
    return address_count_by_port;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetSecurityCounterByport
 *------------------------------------------------------------------------
 * PURPOSE: This function will get the security counter of the specified port.
 * INPUT  : ifindex	- ifindex of the port to get the security counter
 * OUTPUT : None
 * RETURN : Value of the security counter of the specified port
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetSecurityCounterByport(UI32_T ifindex)
{
    UI32_T address_count_by_port;

    AMTR_OM_ATOM_EXPRESSION(address_count_by_port=amtr_om_shmem_data_p->counters.security_address_count_by_port[ifindex-1])
    return address_count_by_port;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_OM_GetConfigCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get the configuration addresses counter of the
 *          specified port.
 * INPUT  : ifindex	- ifindex of the port to get the configuration address
 *          counter
 * OUTPUT : None
 * RETURN : Value of the config counter of the specified port
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTR_OM_GetConfigCounterByPort(UI32_T ifindex)
{
    UI32_T address_count_by_port;

    AMTR_OM_ATOM_EXPRESSION(address_count_by_port=amtr_om_shmem_data_p->counters.config_address_count_by_port[ifindex-1])
    return address_count_by_port;
}

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMacNotifyGlobalStatus
 *------------------------------------------------------------------------------
 * Purpose  : To get mac-notification-trap global status
 * INPUT    : None
 * OUTPUT   : *is_enabled_p
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMacNotifyGlobalStatus(BOOL_T *is_enabled_p)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();
    *is_enabled_p = amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_global;
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMacNotifyInterval
 *------------------------------------------------------------------------------
 * Purpose  : To get mac-notification-trap interval
 * INPUT    : None
 * OUTPUT   : interval_p - pointer to interval to get (in ticks)
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMacNotifyInterval(UI32_T  *interval_p)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();
    *interval_p = amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_interval;
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * Purpose  : To get mac-notification-trap port status
 * INPUT    : ifidx        - lport ifindex
 * OUTPUT   : *is_enabled_p
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMacNotifyPortStatus(UI32_T  ifidx, BOOL_T *is_enabled_p)
{
    BOOL_T  ret = FALSE;

    AMTR_OM_ENTER_CRITICAL_SECTION();
    if ((0 < ifidx) && (ifidx <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        *is_enabled_p = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_en_lports, ifidx)) ? TRUE : FALSE;
        ret = TRUE;
    }
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMacNotifyTimeStamp
 *------------------------------------------------------------------------------
 * Purpose  : To get mac-notification-trap time stamp 
 * INPUT    : None
 * OUTPUT   : time_stamp_p - pointer to time stamp to get
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMacNotifyTimeStamp(UI32_T  *time_stamp_p)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();
    *time_stamp_p = amtr_om_shmem_data_p->amtr_mac_notify.mac_ntfy_time_stamp;
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get vlan learning status of specified vlan
 * INPUT    : vid
 * OUTPUT   : *learning_p
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetVlanLearningStatus(UI32_T vid, BOOL_T *learning_p)
{
    BOOL_T  ret = FALSE;

    if (0 < vid && vid <= SYS_ADPT_MAX_VLAN_ID)
    {
        AMTR_OM_ENTER_CRITICAL_SECTION();
        *learning_p = !(AMTR_MGR_VLAN_LEARNING_TST_BIT(amtr_om_shmem_data_p->amtr_vlan_learning.vlan_learn_dis, vid));
        AMTR_OM_LEAVE_CRITICAL_SECTION();
        ret = TRUE;
    }

    return ret;
}
#if (SYS_CPNT_MLAG == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMlagMacNotifyPortStatus
 *------------------------------------------------------------------------------
 * Purpose  : To get MLAG mac notify port status
 * INPUT    : ifidx        - lport ifindex
 * OUTPUT   : *is_enabled_p
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMlagMacNotifyPortStatus(UI32_T  ifidx, BOOL_T *is_enabled_p)
{
    BOOL_T  ret = FALSE;

    AMTR_OM_ENTER_CRITICAL_SECTION();
    if ((0 < ifidx) && (ifidx <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        *is_enabled_p = (AMTR_MGR_MAC_NTFY_TST_BIT(amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_en_lports, ifidx)) ? TRUE : FALSE;
        ret = TRUE;
    }
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return ret;
}

/*------------------------------------------------------------------------------
 * Function : AMTR_OM_GetMlagMacNotifyTimeStamp
 *------------------------------------------------------------------------------
 * Purpose  : To get MLAG mac notify time stamp 
 * INPUT    : None
 * OUTPUT   : time_stamp_p - pointer to time stamp to get
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_GetMlagMacNotifyTimeStamp(UI32_T  *time_stamp_p)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();
    *time_stamp_p = amtr_om_shmem_data_p->amtr_mlag_mac_notify.mlag_mac_ntfy_time_stamp;
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}
#endif

#if (SYS_CPNT_OVSVTEP == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_OM_OvsGetMacNotifyTimeStamp
 *------------------------------------------------------------------------------
 * Purpose  : To get OVSVTEP mac notify time stamp
 * INPUT    : None
 * OUTPUT   : time_stamp_p - pointer to time stamp to get
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_OM_OvsGetMacNotifyTimeStamp(UI32_T *time_stamp_p)
{
    AMTR_OM_ENTER_CRITICAL_SECTION();
    *time_stamp_p = amtr_om_shmem_data_p->amtr_ovs_mac_notify.ovsvtep_mac_ntfy_time_stamp;
    AMTR_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}
#endif



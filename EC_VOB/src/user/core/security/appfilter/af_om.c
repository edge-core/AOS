/* ------------------------------------------------------------------------
 * FILE NAME - AF_OM.C
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Ezio             14/03/2013      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2013
 * ------------------------------------------------------------------------
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "stdio.h"
#include "string.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"

#if (TRUE == SYS_CPNT_APP_FILTER)
#include "af_om.h"

/* NAMING CONSTANT DECLARARTIONS
 */


/* TYPE DEFINITIONS
 */
typedef struct
{
    AF_TYPE_STATUS_T  status;
} AF_OM_Packet_Cdp;

typedef struct
{
    AF_TYPE_STATUS_T  status;
} AF_OM_Packet_Pvst;

typedef struct
{
    AF_OM_Packet_Cdp   cdp;
    AF_OM_Packet_Pvst  pvst;
} AF_OM_PortEntry_T;

typedef struct
{
    AF_OM_PortEntry_T port_entry[SYS_ADPT_TOTAL_NBR_OF_LPORT];
} AF_OM_Shmem_Data_T;

/* MACRO DEFINITIONS
 */
#ifndef _countof
#define _countof(_Ary) (sizeof(_Ary)/sizeof(*_Ary))
#endif

#define AF_OM_EnterCriticalSection() SYSFUN_TakeSem(af_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define AF_OM_LeaveCriticalSection() SYSFUN_GiveSem(af_om_sem_id)

#define AF_OM_LeaveCriticalSectionReturnState(state)    \
{                                                       \
    AF_OM_LeaveCriticalSection();                       \
    return state;                                       \
}

#define AF_OM_LeaveCriticalSectionReturnVoid()          \
{                                                       \
    AF_OM_LeaveCriticalSection();                       \
    return;                                             \
}


#if _MSC_VER /* VC */
#define AF_OM_DEBUG_PRINTF(fmt, ...)                                        \
    {                                                                       \
        printf("[%s:%d] ", __FUNCTION__, __LINE__);                         \
        printf(fmt, __VA_ARGS__);                                           \
        printf("\r\n");                                                     \
    }
#else
#define AF_OM_DEBUG_PRINTF(fmt, args...)                                   \
    if (TRUE == AF_OM_GetDebugMode())                                      \
    {                                                                      \
        printf("[%s:%d] ", __FUNCTION__, __LINE__);                        \
        printf(fmt, ##args);                                               \
        printf("\r\n");                                                    \
    }
#endif /* COS_VM_DEBUG_PRINTF */


/* LOCAL FUNCTIONS DECLARATIONS
 */

/* LOCAL VARIABLES DECLARATIONS
 */
//static AF_OM_PortEntry_T af_om_port_entry[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static UI32_T  af_om_sem_id;
static UI32_T  debug_flags;

static AF_OM_Shmem_Data_T *af_shmem_data_p;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
 BOOL_T AF_OM_GetDebugMode()
{
    return debug_flags;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_InitiateSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will init system resource
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void AF_OM_InitiateSystemResources(void)
{
    af_shmem_data_p = (AF_OM_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_APP_FILTER_OM_SHMEM_SEGID);
    AF_OM_Init();
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_AttachSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void AF_OM_AttachSystemResources(void)
{
    af_shmem_data_p = (AF_OM_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_APP_FILTER_OM_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_APP_FILTER_OM, &af_om_sem_id);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_GetShMemInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *-----------------------------------------------------------------------------
 */
void AF_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_APP_FILTER_OM_SHMEM_SEGID;
    *seglen_p = sizeof(AF_OM_Shmem_Data_T);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will init OM resouce
 * INPUT  : use_default -- set with default value
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T AF_OM_Init(void)
{
    memset(af_shmem_data_p, 0, sizeof(*af_shmem_data_p));

    return TRUE;
}

static AF_OM_PortEntry_T *
AF_OM_GetPortEntry(UI32_T ifindex)
{
    return (ifindex < _countof(af_shmem_data_p->port_entry)) ?
        &af_shmem_data_p->port_entry[ifindex - 1] :
        NULL;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_SetPortCdpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the traffic status of CDP packet on a port
 * INPUT  : ifindex     -- interface index
 * OUTPUT : status      -- traffic status (AF_TYPE_STATUS_E)
 * RETURN : AF_TYPE_ErrorCode_E
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_OM_SetPortCdpStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status)
{
    AF_OM_PortEntry_T *port_entry_p;

    port_entry_p = AF_OM_GetPortEntry(ifindex);
    if (NULL == port_entry_p)
    {
        AF_OM_DEBUG_PRINTF("Invalid parameter: ifindex %lu", ifindex);
        return AF_TYPE_E_PARAMETER;
    }

    if (status < AF_TYPE_STATUS_MIN ||
        AF_TYPE_STATUS_MAX < status)
    {
        AF_OM_DEBUG_PRINTF("Invalid parameter: status %d", status);
        return AF_TYPE_E_PARAMETER;
    }

    AF_OM_EnterCriticalSection();
    port_entry_p->cdp.status = status;
    AF_OM_LeaveCriticalSectionReturnState(AF_TYPE_SUCCESS);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_GetPortCdptStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the traffic status of CDP packet on a port
 * INPUT  : ifindex     -- interface index
 * OUTPUT : status      -- traffic status (AF_TYPE_STATUS_E)
 * RETURN : AF_TYPE_ErrorCode_E
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_OM_GetPortCdpStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T *status_p)
{
    AF_OM_PortEntry_T *port_entry_p;

    port_entry_p = AF_OM_GetPortEntry(ifindex);
    if (NULL == port_entry_p)
    {
        AF_OM_DEBUG_PRINTF("Invalid parameter: ifindex %lu", ifindex);
        return AF_TYPE_E_PARAMETER;
    }

    if (NULL == status_p)
    {
        AF_OM_DEBUG_PRINTF("Invalid parameter");
        return AF_TYPE_E_PARAMETER;
    }

    AF_OM_EnterCriticalSection();
    *status_p = port_entry_p->cdp.status;
    AF_OM_LeaveCriticalSectionReturnState(AF_TYPE_SUCCESS);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_SetPortPvstStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the traffic status of PVST packet on a port
 * INPUT  : ifindex     -- interface index
 * OUTPUT : status      -- traffic status (AF_TYPE_STATUS_E)
 * RETURN : AF_TYPE_ErrorCode_E
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_OM_SetPortPvstStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status)
{
    AF_OM_PortEntry_T *port_entry_p;

    port_entry_p = AF_OM_GetPortEntry(ifindex);
    if (NULL == port_entry_p)
    {
        AF_OM_DEBUG_PRINTF("Invalid parameter: ifindex %lu", ifindex);
        return AF_TYPE_E_PARAMETER;
    }

    if (status < AF_TYPE_STATUS_MIN ||
        AF_TYPE_STATUS_MAX < status)
    {
        AF_OM_DEBUG_PRINTF("Invalid parameter: status %d", status);
        return AF_TYPE_E_PARAMETER;
    }

    AF_OM_EnterCriticalSection();
    port_entry_p->pvst.status = status;
    AF_OM_LeaveCriticalSectionReturnState(AF_TYPE_SUCCESS);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_OM_GetPortPvstStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the traffic status of PVST packet on a port
 * INPUT  : ifindex     -- interface index
 * OUTPUT : status      -- traffic status (AF_TYPE_STATUS_E)
 * RETURN : AF_TYPE_ErrorCode_E
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_OM_GetPortPvstStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T *status_p)
{
    AF_OM_PortEntry_T *port_entry_p;

    port_entry_p = AF_OM_GetPortEntry(ifindex);
    if (NULL == port_entry_p)
    {
        AF_OM_DEBUG_PRINTF("Invalid parameter: ifindex %lu", ifindex);
        return AF_TYPE_E_PARAMETER;
    }

    if (NULL == status_p)
    {
        AF_OM_DEBUG_PRINTF("Invalid parameter");
        return AF_TYPE_E_PARAMETER;
    }

    AF_OM_EnterCriticalSection();
    *status_p = port_entry_p->pvst.status;
    AF_OM_LeaveCriticalSectionReturnState(AF_TYPE_SUCCESS);
}


#endif /* #if (TRUE == SYS_CPNT_APP_FILTER) */

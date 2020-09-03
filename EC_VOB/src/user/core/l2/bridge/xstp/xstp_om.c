/*-----------------------------------------------------------------------------
 * Module Name: xstp_om.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the XSTP OM
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    05/30/2001 - Allen Cheng, Created
 *    06/12/2002 - Kelly Chen, Added
 *    11/19/2002 - Add two functions to solve stacking issue about
 *                 topology_change and mode_transformation.
 *                 XSTP_OM_CleanDatabase();
 *                 XSTP_OM_SetDefaultValue();
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "sys_adpt.h"
#include "l_stdlib.h"
#include "l_bitmap.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "swctrl.h"
#include "vlan_lib.h"
#include "vlan_om.h"
#include "trk_pmgr.h"
#include "xstp_engine.h" /* temporarily added to let compile ok */
#include "xstp_om.h"
#include "xstp_om_private.h"
#include "xstp_type.h"
#include "sysrsc_mgr.h"
#include "sys_bld.h"
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
#include "l_md5.h"
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

static UI32_T                          original_priority;

static UI32_T  XSTP_OM_StateDebugBmap[8]   = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static UI32_T  XSTP_OM_StateDebugMstBmap[3]= { 0x00, 0x00, 0x00};
static UI32_T  XSTP_OM_DebugFlag           = XSTP_TYPE_DEBUG_FLAG_NONE;

#define XSTP_OM_FORMAT_LONG     0
#define XSTP_OM_FORMAT_SHORT    1
#define XSTP_OM_SPEED_10M       0
#define XSTP_OM_SPEED_100M      1
#define XSTP_OM_SPEED_1G        2
#define XSTP_OM_SPEED_10G       3
#define XSTP_OM_SPEED_25G       4
#define XSTP_OM_SPEED_40G       5
#define XSTP_OM_SPEED_100G      6
#define XSTP_OM_TYPE_HDPLX      0
#define XSTP_OM_TYPE_FDPLX      1
#define XSTP_OM_TYPE_TRUNK      2
#define XSTP_OM_PATH_COST_TM_FACTOR_OFF 0
#define XSTP_OM_PATH_COST_TM_FACTOR_ON  1
#define XSTP_OM_FACTOR(_count_, _def_)                      (((_def_)==0)?(1):((_count_)*(_def_)))
#define XSTP_OM_SHORT_FORMAT(_val_)                         (((_val_)>65535)?(65535):(_val_))

/* XSTP_OM_DefaultCost[FORMAT][SPEED][TYPE] : Table storing the path cost */
static  UI32_T  XSTP_OM_DefaultCost[2][7][3]   =
{
    {       /* Long */
        {   /* 10M */
            SYS_DFLT_XSTP_HDPLX_10M_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_10M_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_10M_PORT_LONG_PATH_COST
        },
        {   /* 100M */
            SYS_DFLT_XSTP_HDPLX_100M_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_100M_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_100M_PORT_LONG_PATH_COST
        },
        {   /* 1G */
            SYS_DFLT_XSTP_HDPLX_1G_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_1G_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_1G_PORT_LONG_PATH_COST
        },
        {   /* 10G */
            SYS_DFLT_XSTP_HDPLX_10G_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_10G_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_10G_PORT_LONG_PATH_COST
        },
        {   /* 25G */
            SYS_DFLT_XSTP_HDPLX_25G_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_25G_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_25G_PORT_LONG_PATH_COST
        },
        {   /* 40G */
            SYS_DFLT_XSTP_HDPLX_40G_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_40G_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_40G_PORT_LONG_PATH_COST
        },
        {   /* 100G */
            SYS_DFLT_XSTP_HDPLX_100G_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_100G_PORT_LONG_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_100G_PORT_LONG_PATH_COST
        }
    },
    {       /* Short */
        {   /* 10M */
            SYS_DFLT_XSTP_HDPLX_10M_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_10M_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_10M_PORT_SHORT_PATH_COST
        },
        {   /* 100M */
            SYS_DFLT_XSTP_HDPLX_100M_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_100M_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_100M_PORT_SHORT_PATH_COST
        },
        {   /* 1G */
            SYS_DFLT_XSTP_HDPLX_1G_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_1G_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_1G_PORT_SHORT_PATH_COST
        },
        {   /* 10G */
            SYS_DFLT_XSTP_HDPLX_10G_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_10G_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_10G_PORT_SHORT_PATH_COST
        },
        {   /* 25G */
            SYS_DFLT_XSTP_HDPLX_25G_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_25G_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_25G_PORT_SHORT_PATH_COST
        },
        {   /* 40G */
            SYS_DFLT_XSTP_HDPLX_40G_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_40G_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_40G_PORT_SHORT_PATH_COST
        },
        {   /* 100G */
            SYS_DFLT_XSTP_HDPLX_100G_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_FDPLX_100G_PORT_SHORT_PATH_COST,
            SYS_DFLT_XSTP_TRUNK_100G_PORT_SHORT_PATH_COST
        }
    }
};  /* End of XSTP_OM_DefaultCost[][][] */


static  void XSTP_OM_InitEdgePortAndLinkType(XSTP_OM_InstanceData_T *om_ptr);
static  void XSTP_OM_InitStaticFlag(XSTP_OM_InstanceData_T *om_ptr);
static  void    XSTP_OM_InitMstiEntryList(void);
static  BOOL_T  XSTP_OM_InsertMstiEntryList(UI32_T insert_index);
static  BOOL_T  XSTP_OM_DeleteMstiEntryList(UI32_T delete_index);
static  BOOL_T  XSTP_OM_GetNextMstiEntryIndex(UI32_T *index);
static void XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local(UI32_T vid, UI32_T *mstid);

/*=============================================================================
 * Move from xstp_mgr.c
 *=============================================================================
 */
static UI32_T XSTP_OM_GetRunningMstForwardDelay(XSTP_OM_InstanceData_T *om_ptr, UI32_T *forward_delay);
static UI32_T XSTP_OM_GetRunningMstHelloTime(XSTP_OM_InstanceData_T *om_ptr, UI32_T *hello_time);
static UI32_T XSTP_OM_GetRunningMstMaxAge(XSTP_OM_InstanceData_T *om_ptr, UI32_T *max_age);
static UI32_T XSTP_OM_GetRunningMstTransmissionLimit(XSTP_OM_InstanceData_T *om_ptr, UI32_T *tx_hold_count);
static BOOL_T XSTP_OM_GetNextPortMemberOfInstance(XSTP_OM_InstanceData_T *om_ptr, UI32_T *lport);
static UI32_T XSTP_OM_GetMstPortRole_(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T *role);
static UI32_T XSTP_OM_GetMstPortState_(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T *state);
static BOOL_T XSTP_OM_GetMstpInstanceVlanMapped_(XSTP_OM_InstanceData_T *om_ptr, XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);
static BOOL_T XSTP_OM_GetMstpInstanceVlanConfiguration_(XSTP_OM_InstanceData_T *om_ptr, XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);
static BOOL_T XSTP_OM_GetDot1dMstPortEntry_(XSTP_OM_InstanceData_T *om_ptr, XSTP_MGR_Dot1dStpPortEntry_T *port_entry);
static BOOL_T XSTP_OM_GetDot1dMstPortEntry_LinkdownAsBlocking_(XSTP_OM_InstanceData_T *om_ptr, XSTP_MGR_Dot1dStpPortEntry_T *port_entry);
static BOOL_T XSTP_OM_GetDot1dMstExtPortEntry_( XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, XSTP_MGR_Dot1dStpExtPortEntry_T *ext_port_entry);
static BOOL_T XSTP_OM_GetNextExistingInstance_(UI32_T *mstid);
static void   XSTP_OM_LSB_To_MSB(UI8_T *data, UI32_T data_len);

static UI32_T   xstp_om_sem_id;
static XSTP_OM_SHARE_T   *shmem_data_p;

#define XSTP_OM_InstanceInfo shmem_data_p->XSTP_OM_InstanceInfo
#define XSTP_OM_Mst_Configuration_Table shmem_data_p->XSTP_OM_Mst_Configuration_Table
#define XSTP_OM_SystemInfo shmem_data_p->XSTP_OM_SystemInfo
#define XSTP_OM_InstanceEntryIndex shmem_data_p->XSTP_OM_InstanceEntryIndex
#define XSTP_OM_Configuration_Digest_Signature_Key shmem_data_p->XSTP_OM_Configuration_Digest_Signature_Key
#define XSTP_OM_BridgeCommonInfo shmem_data_p->XSTP_OM_BridgeCommonInfo
#define XSTP_OM_PortCommonInfo shmem_data_p->XSTP_OM_PortCommonInfo

#if (SYS_CPNT_EAPS == TRUE)
#define XSTP_OM_EthRingProle    shmem_data_p->XSTP_OM_EthRingProle
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_Debug
 * ------------------------------------------------------------------------
 * PURPOSE  :   Check debug flag
 * INPUT    :   flag        -- debug flag
 * OUTPUT   :   None
 * RETURN   :   TRUE if the specified flag bit(s) is/are set, else FALSE
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_Debug(UI32_T flag)
{
    return ( (XSTP_OM_DebugFlag & flag) != 0);
} /* End of XSTP_OM_Debug */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_StateDebugPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Check the state debug port mode
 * INPUT    :   lport     -- lport
 * OUTPUT   :   None
 * RETURN   :   TRUE if the lport is in the state debug mode, else FALSE
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_StateDebugPort(UI32_T lport)
{
    return  (   (lport == 0)
             || (   (   ( XSTP_OM_StateDebugBmap[0x00000007&( (lport-1)>>5)])
                      & ( (UI32_T)0x01<<( (lport-1) & 0x0000001F) )
                    ) != 0
                )
            );
} /* End of XSTP_OM_StateDebugPort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_StateDebugMst
 * ------------------------------------------------------------------------
 * PURPOSE  :   Check the state debug port mode
 * INPUT    :   mstid     -- mstid
 * OUTPUT   :   None
 * RETURN   :   TRUE if the lport is in the state debug mode, else FALSE
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_StateDebugMst(UI32_T mstid)
{
    return  (   (   ( XSTP_OM_StateDebugMstBmap[0x00000007&( (mstid)>>5)])
                  & ( (UI32_T)0x01<<( (mstid) & 0x0000001F) )
                ) != 0
            );
} /* End of XSTP_OM_StateDebugMst */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_StateDebugPortSwitch
 * ------------------------------------------------------------------------
 * PURPOSE  :   Switch the state debug mode
 * INPUT    :   lport     -- lport
 * OUTPUT   :   None
 * RETURN   :   TRUE if the lport is in the state debug mode after switched,
                else FALSE
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_StateDebugPortSwitch(UI32_T lport)
{
    UI32_T  value;
    UI32_T  lport_bitval;

    if (lport == 0)
    {
        return TRUE;
    }
    else
    {
        value           = XSTP_OM_StateDebugBmap[0x00000007&( (lport-1)>>5)];
        lport_bitval    = (UI32_T)0x01<<( (lport-1) & 0x0000001F);
        value           ^= lport_bitval;
        XSTP_OM_StateDebugBmap[0x00000007&( (lport-1)>>5)]  = value;

        return  ( (value & lport_bitval) != 0 );
    }
} /* End of XSTP_OM_StateDebugPortSwitch */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_StateDebugMstSwitch
 * ------------------------------------------------------------------------
 * PURPOSE  :   Switch the state debug mode for specified mstid
 * INPUT    :   mstid     -- mstid
 * OUTPUT   :   None
 * RETURN   :   TRUE if the lport is in the state debug mode after switched,
                else FALSE
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_StateDebugMstSwitch(UI32_T mstid)
{
    UI32_T  value;
    UI32_T  mstid_bitval;

    value           = XSTP_OM_StateDebugMstBmap[0x00000007&( (mstid)>>5)];
    mstid_bitval    = (UI32_T)0x01<<( (mstid) & 0x0000001F);
    value           ^= mstid_bitval;
    XSTP_OM_StateDebugMstBmap[0x00000007&( (mstid)>>5)]  = value;

    return  ( (value & mstid_bitval) != 0 );
} /* End of XSTP_OM_StateDebugMstSwitch */

void    XSTP_OM_SetDebugFlag(UI32_T flag)
{
    XSTP_OM_DebugFlag    = flag;
    return;
}

void    XSTP_OM_GetDebugFlag(UI32_T *flag)
{
    *flag   = XSTP_OM_DebugFlag;
    return;
}
#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for XSTP objects
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   This function is invoked in XSTP_TASK_Init.
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_InitSemaphore(void)
{
    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO,
            &xstp_om_sem_id) != SYSFUN_OK)
        printf("\n%s: get xstp om sem id fail.\n", __FUNCTION__);

    return;
}
#endif
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_EnterCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical section before a task invokes the spanning
 *              tree objects.
 * INPUT    :   xstid       -- the identifier of the mst instance
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_EnterCriticalSection(UI32_T xstid)
{
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
    return;
} /* End of XSTP_OM_EnterCriticalSection */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_LeaveCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical section after a task invokes the spanning
 *              tree objects.
 * INPUT    :   xstid           -- the identifier of the mst instance
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   The task priority of the caller is set to original task
 *              priority.
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_LeaveCriticalSection(UI32_T xstid)
{
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
    return;
} /* XSTP_OM_LeaveCriticalSection */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetInstanceInfoPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the om_ptr for the specified xstid
 * INPUT    : xstid -- MST instance ID
 * OUTPUT   : None
 * RETUEN   : Pointer to the om_ptr for the specified xstid
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
XSTP_OM_InstanceData_T*   XSTP_OM_GetInstanceInfoPtr(UI32_T xstid)
{
    return  (&(XSTP_OM_InstanceInfo[XSTP_OM_InstanceEntryIndex[xstid]]));
} /* End of XSTP_OM_GetInstanceInfoPtr */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextInstanceInfoPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the om_ptr for the next xstid
 * INPUT    : xstid -- MST instance ID
 * OUTPUT   : xstid -- The next MST instance ID, or XSTP_TYPE_INEXISTENT_MSTID
 *                     if at the end of the OM
 * RETUEN   : Pointer to the om_ptr for the next xstid, or the om_ptr for
 *            the XSTP_TYPE_INEXISTENT_MSTID if at the end of the OM
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextInstanceInfoPtr(UI32_T *xstid, XSTP_OM_InstanceData_T **om_pptr)
{
    UI32_T  index;

    if (*xstid >=XSTP_TYPE_INEXISTENT_MSTID)
    {
        *xstid = XSTP_TYPE_INEXISTENT_MSTID;
        return FALSE;
    }
    if (XSTP_OM_InstanceEntryIndex[*xstid] == XSTP_TYPE_MAX_INSTANCE_NUM)
    {
        /* un-allocated instance */
        index       = XSTP_TYPE_CISTID;
        do
        {
            index   = XSTP_OM_InstanceInfo[index].next;
        } while(*xstid > XSTP_OM_InstanceInfo[index].instance_id);
        *om_pptr = &(XSTP_OM_InstanceInfo[index]);
    }
    else
    {
        *om_pptr  = &(XSTP_OM_InstanceInfo[XSTP_OM_InstanceInfo[XSTP_OM_InstanceEntryIndex[*xstid]].next]);
    }
    *xstid  = (*om_pptr)->instance_id;
    return ((*xstid) != XSTP_TYPE_INEXISTENT_MSTID);
} /* End of XSTP_OM_GetNextInstanceInfoPtr */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_Init
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the OM
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_Init(void)
{
    UI32_T                  entry_index;
    /*add by Tony.Lei*/
    shmem_data_p = (XSTP_OM_SHARE_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_XSTP_SHMEM_SEGID);
    /*init error*/
    if(shmem_data_p == NULL)
        while(1);

    if(SYSFUN_OK != SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_XSTP_OM, &xstp_om_sem_id))
        while(1);

    /*end add*/
    XSTP_OM_LongHexStrToVal(XSTP_TYPE_CONFIGURATION_DIGEST_SIGNATURE_KEY,
                            XSTP_OM_Configuration_Digest_Signature_Key,
                            16);
    memset(XSTP_OM_InstanceInfo, 0, sizeof(XSTP_OM_InstanceInfo));
    memset(XSTP_OM_PortCommonInfo, 0, sizeof(XSTP_OM_PortCommonInfo));

    for (entry_index = 0; entry_index <= XSTP_TYPE_MAX_INSTANCE_NUM; entry_index++)
    {
        UI16_T      index;
        XSTP_OM_InstanceInfo[entry_index].bridge_info.common = &XSTP_OM_BridgeCommonInfo;
        for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
        {
            XSTP_OM_InstanceInfo[entry_index].port_info[index].common   = &(XSTP_OM_PortCommonInfo[index]);
        }
        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        if (entry_index == XSTP_TYPE_CISTID)
        {
            XSTP_OM_InstanceInfo[entry_index].bridge_info.cist          = (XSTP_OM_MstBridgeVar_T*)NULL;
            for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
            {
                XSTP_OM_InstanceInfo[entry_index].port_info[index].cist = (XSTP_OM_MstPortVar_T*)NULL;
            }
        }
        else
        {
            XSTP_OM_InstanceInfo[entry_index].bridge_info.cist          = &(XSTP_OM_InstanceInfo[XSTP_TYPE_CISTID].bridge_info);
            for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
            {
                XSTP_OM_InstanceInfo[entry_index].port_info[index].cist = &(XSTP_OM_InstanceInfo[XSTP_TYPE_CISTID].port_info[index]);
            }
        }
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    }
    XSTP_OM_InitMstiEntryList();
    XSTP_OM_CleanDatabase();
    XSTP_OM_SetDefaultValue();
    return;
} /* End of XSTP_OM_Init */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_ClearDatabase
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset/Clear database.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_CleanDatabase(void)
{
    memset(XSTP_OM_Mst_Configuration_Table, 0, XSTP_OM_Mst_Configuration_Table_Size);

#if (SYS_CPNT_EAPS == TRUE)
    memset(XSTP_OM_EthRingProle, 0, sizeof(XSTP_OM_EthRingProle));
#endif

    return;
} /* End of XSTP_OM_CleanDatabase */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetDefaultValue
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the default values.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_SetDefaultValue(void)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  xstid;
    UI32_T                  lport;

    xstid               = XSTP_TYPE_CISTID;
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);

    for (lport = 0; lport < XSTP_TYPE_MAX_NUM_OF_LPORT; lport++)
    {
        pom_ptr = &(om_ptr->port_info[lport]);
#if(SYS_CPNT_STP_ROOT_GUARD == TRUE)
        pom_ptr->common->root_guard_status = XSTP_TYPE_DEFAULT_PORT_ROOT_GUARD_STATUS;
#endif
#if(SYS_CPNT_STP_BPDU_GUARD == TRUE)
        pom_ptr->common->bpdu_guard_status = (XSTP_TYPE_DEFAULT_PORT_BPDU_GUARD_STATUS == XSTP_TYPE_PORT_BPDU_GUARD_ENABLED);
        pom_ptr->common->bpdu_guard_auto_recovery = XSTP_TYPE_DEFAULT_PORT_BPDU_GUARD_AUTO_RECOVERY;
        pom_ptr->common->bpdu_guard_auto_recovery_interval = XSTP_TYPE_DEFAULT_PORT_BPDU_GUARD_AUTO_RECOVERY_INTERVAL;
#endif
#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
        pom_ptr->common->tc_prop_group_id = XSTP_TYPE_TC_PROP_DEFAULT_GROUP_ID;
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/
    }

    do
    {
        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        /* Patch for 13.32 */
        om_ptr->bridge_info.cist_role_updated   = FALSE;
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */

        XSTP_OM_InitStaticFlag(om_ptr);
        XSTP_OM_NullifyInstance(om_ptr);

        if (om_ptr->instance_id == XSTP_TYPE_CISTID)
        {
            /* the variables about edge_port and link_type should not be cleared on disabling the stp or port except in initializing phase.  */
            XSTP_OM_InitEdgePortAndLinkType(om_ptr);
            /* 13.23.8 mst_config_id */
            XSTP_OM_InitMstConfigId(om_ptr);
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));

    XSTP_OM_SystemInfo.spanning_tree_status     = XSTP_TYPE_DEFAULT_SPANNING_TREE_STATUS;
    if (XSTP_TYPE_DEFAULT_STP_VERSION > XSTP_TYPE_PROTOCOL_VERSION_ID)
    {
        XSTP_OM_SystemInfo.force_version        = XSTP_TYPE_PROTOCOL_VERSION_ID;
    }
    else
    {
        XSTP_OM_SystemInfo.force_version        = XSTP_TYPE_DEFAULT_STP_VERSION;
    }
    XSTP_OM_SystemInfo.path_cost_method         = XSTP_TYPE_DEFAULT_PATH_COST_METHOD;

    XSTP_OM_SystemInfo.max_instance_number      = XSTP_TYPE_DEFAULT_INSTANCE_NUM;
    XSTP_OM_SystemInfo.num_of_active_tree       = 0;
    XSTP_OM_SystemInfo.num_of_cfg_msti          = 0;
    XSTP_OM_SystemInfo.max_hop_count            = XSTP_TYPE_DEFAULT_BRIDGE_MAXHOP;
    XSTP_OM_SystemInfo.trap_flag_tc             = FALSE;
    XSTP_OM_SystemInfo.trap_flag_new_root       = FALSE;
    XSTP_OM_SystemInfo.trap_rx_tc               = FALSE;
    XSTP_OM_SystemInfo.tc_rx_port               = 0;
    memset(XSTP_OM_SystemInfo.tc_casee_brdg_mac, 0, sizeof(XSTP_OM_SystemInfo.tc_casee_brdg_mac));
    XSTP_OM_SystemInfo.mst_topology_method      = SYS_CPNT_MST_TOPOLOGY;
#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
    XSTP_OM_SystemInfo.cisco_prestandard        = XSTP_TYPE_DEFAULT_CISCO_PRESTANDARD_COMPATIBILITY;
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */
    return;
} /* End of XSTP_OM_InitDefaultValue */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_NullifyPortOmEngineState
 * ------------------------------------------------------------------------
 * PURPOSE  : Nullify the Engine State of the Port OM
 * INPUT    : xstid     -- spanning tree instance id
 *            lport     -- lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_NullifyPortOmEngineState(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;

    pom_ptr = &(om_ptr->port_info[lport-1]);
    pom_ptr->common->port_enabled   = FALSE;
    pom_ptr->common->link_up        = FALSE;
    pom_ptr->common->port_spanning_tree_status = XSTP_TYPE_DEFAULT_PORT_SPANNING_TREE_STATUS;
    pom_ptr->common->loopback_block = FALSE;
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    pom_ptr->common->bpdu_guard_auto_recovery_while = 0;
#endif

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    pom_ptr->common->bpdu_filter_status = SYS_DFLT_PORT_BPDU_FILTER_STATUS;
#endif

#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
    pom_ptr->common->tc_prop_stop = FALSE;
#endif

    return;
} /* End of XSTP_OM_NullifyPortOmEngineState */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_NullifyInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Nullify the specified instance
 * INPUT    : xstid     -- spanning tree instance id
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_NullifyInstance(XSTP_OM_InstanceData_T *om_ptr)
{
    UI16_T                  index;

    if (om_ptr->instance_exist)
    {
        XSTP_OM_DeleteTreeOm(om_ptr);
    }

    for (index = 0; index < 512; index++)
    {
        if ( (om_ptr->instance_id == 0) && (index == (SYS_DFLT_SWITCH_MANAGEMENT_VLAN/8)) )
        {
            /* Add management vlan into CIST as default */
            om_ptr->instance_vlans_mapped[index]    = 1 << (SYS_DFLT_SWITCH_MANAGEMENT_VLAN%8);
        }
        else
        {
            om_ptr->instance_vlans_mapped[index]    = 0x00;
        }
    }

    om_ptr->instance_remaining_hop_count        = 0;  /* useless ???  */

    XSTP_OM_NullifyBridgeOm(om_ptr);

    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
        XSTP_OM_NullifyPortOm(om_ptr, index+1);

    return;
} /* End of XSTP_OM_NullifyInstance */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_NullifyBridgeOm
 * ------------------------------------------------------------------------
 * PURPOSE  : Nullify the Bridge OM
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_NullifyBridgeOm(XSTP_OM_InstanceData_T *om_ptr)
{
    om_ptr->bridge_info.time_since_topology_change  = (UI32_T)SYSFUN_GetSysTick();
    om_ptr->bridge_info.topology_change_count       = 0;
    om_ptr->bridge_info.trap_flag_tc                = FALSE;
    om_ptr->bridge_info.trap_flag_new_root          = FALSE;


    return;
} /* End of XSTP_OM_NullifyBridgeOm */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_NullifyPortOm
 * ------------------------------------------------------------------------
 * PURPOSE  : Nullify the Port OM
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_NullifyPortOm(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              path_cost;

    pom_ptr             = &(om_ptr->port_info[lport-1]);
    #if 0   /* Allen Cheng, 10/29/2002, is_member is set when create_tree_om */
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
    {
        pom_ptr->is_member                              = TRUE;
    }
    else
    #endif
    {
        pom_ptr->is_member                          = FALSE;
    }

    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
        pom_ptr->common->bpdu                       = NULL;

    pom_ptr->port_forward_transitions           = 0;

    XSTP_OM_GetLportDefaultPathCost(lport, &path_cost);

    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    if (!pom_ptr->static_path_cost)
    {
        pom_ptr->port_path_cost                     = path_cost;
    }
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (!pom_ptr->common->static_external_path_cost)
    {
        pom_ptr->common->external_port_path_cost    = path_cost;
    }
    if (!pom_ptr->static_internal_path_cost)
    {
        pom_ptr->internal_port_path_cost            = path_cost;
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return;
} /* End of XSTP_OM_NullifyPortOm */


#ifdef  XSTP_TYPE_PROTOCOL_RSTP
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmLportConsistency
 * ------------------------------------------------------------------------
 * PURPOSE  : Rename the specified lport if it is a root port or designated
 *            port
 * INPUT    : om_ptr    -- om pointer for this instance
 *            dst_lport -- destinating lport
 *            src_lport -- source lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmLportConsistency(XSTP_OM_InstanceData_T *om_ptr, UI32_T dst_lport, UI32_T src_lport)
{
    XSTP_OM_PortVar_T       *dst_pom_ptr;
    XSTP_OM_PortVar_T       *src_pom_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  index;

    dst_pom_ptr         = &(om_ptr->port_info[dst_lport-1]);
    src_pom_ptr         = &(om_ptr->port_info[src_lport-1]);

    /* port_priority.bridge_port_id */
    dst_pom_ptr->port_priority.bridge_port_id.port_id               = dst_pom_ptr->port_id.port_id;

    /* designated_priority.designated_port_id, designated_priority.bridge_port_id */
    dst_pom_ptr->designated_priority.designated_port_id.port_id     = dst_pom_ptr->port_id.port_id;
    dst_pom_ptr->designated_priority.bridge_port_id.port_id         = dst_pom_ptr->port_id.port_id;

    /* If src_lport is the bridge's root port */
    /* bridge_info.root_port_id */
    /* bridge_info.root_priority.bridge_port_id, bridge_info.root_priority.designated_port_id */
    if (om_ptr->bridge_info.root_port_id.port_id == src_pom_ptr->port_id.port_id)
    {
        om_ptr->bridge_info.root_port_id.port_id                        = dst_pom_ptr->port_id.port_id;
    }
    if (om_ptr->bridge_info.root_priority.bridge_port_id.port_id == src_pom_ptr->port_id.port_id)
    {
        om_ptr->bridge_info.root_priority.bridge_port_id.port_id        = dst_pom_ptr->port_id.port_id;
    }
    if (om_ptr->bridge_info.root_priority.designated_port_id.port_id == src_pom_ptr->port_id.port_id)
    {
        om_ptr->bridge_info.root_priority.designated_port_id.port_id    = dst_pom_ptr->port_id.port_id;
    }

    return;
} /* End of XSTP_OM_MakeOmLportConsistency */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_ConvertPortOm
 * ------------------------------------------------------------------------
 * PURPOSE  : Convert the Port OM due to its lport number is changed
 * INPUT    : om_ptr    -- om pointer for this instance
 *            dst_lport -- destinating lport
 *            src_lport -- source lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_ConvertPortOm(XSTP_OM_InstanceData_T *om_ptr, UI32_T dst_lport, UI32_T src_lport)
{
    XSTP_OM_PortVar_T       *dst_pom_ptr;
    XSTP_OM_PortVar_T       *src_pom_ptr;
    XSTP_OM_PortCommonVar_T *common;
    UI32_T                  path_cost;

    /* Copy Port OM */
    dst_pom_ptr         = &(om_ptr->port_info[dst_lport-1]);
    src_pom_ptr         = &(om_ptr->port_info[src_lport-1]);
    common              = dst_pom_ptr->common;
    memcpy(dst_pom_ptr, src_pom_ptr, sizeof(XSTP_OM_PortVar_T) );
    dst_pom_ptr->common = common;

    memcpy(dst_pom_ptr->common, src_pom_ptr->common, sizeof(XSTP_OM_PortCommonVar_T) );

    /* Convert Per-Port variables */                    /* 17.18 */

    /* 17.16.5 */
    if (!dst_pom_ptr->static_path_cost)
    {
        XSTP_OM_GetLportDefaultPathCost(dst_lport, &path_cost);
        dst_pom_ptr->port_path_cost     = path_cost;
    }
    /* 17.18.16 */
    dst_pom_ptr->port_id.port_id    =   (src_pom_ptr->port_id.port_id & 0xF000) | ( (UI16_T)dst_lport & 0x0FFF);

    return;
} /* End of XSTP_OM_ConvertPortOm */

#endif /* XSTP_TYPE_PROTOCOL_RSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_CreateTreeOm
 * ------------------------------------------------------------------------
 * PURPOSE  : Create a Spanning Tree OM.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : XSTP_TYPE_RETURN_OK   : Create ok
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_CreateTreeOm(XSTP_OM_InstanceData_T *om_ptr)
{
    UI32_T                  lport;
    UI32_T                  vlan_id, vid_ifindex;
    XSTP_OM_PortVar_T       *pom_ptr;

    om_ptr->instance_exist = TRUE;
    om_ptr->row_status     = VAL_dot1qVlanStaticRowStatus_active;
    om_ptr->dirty_bit      = TRUE;
    XSTP_OM_SystemInfo.num_of_active_tree++;

    vlan_id = 0;
    while (XSTP_OM_GetNextXstpMember(om_ptr, &vlan_id) )
    {
        /* All ports in the vlan are the members of the specified instance */
        lport   = 0;
        while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
            VLAN_VID_CONVERTTO_IFINDEX(vlan_id, vid_ifindex);
            if (    (XSTP_OM_IsMstFullMemberTopology())
                ||  (VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
               )
            {
                pom_ptr             = &(om_ptr->port_info[lport-1]);
                pom_ptr->is_member  = TRUE;
                pom_ptr->parent_index = 0;
            } /* End of if (VLAN_OM_IsPortVlanMember) */
        } /* End of while (SWCTRL_GetNextLogicalPort) */
    } /* End of while (XSTP_OM_GetNextXstpMember) */

    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_OM_CreateTreeOm */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_DeleteTreeOm
 * ------------------------------------------------------------------------
 * PURPOSE  : Delete a Spanning Tree OM.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : XSTP_TYPE_RETURN_OK   : Delete ok
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_DeleteTreeOm(XSTP_OM_InstanceData_T *om_ptr)
{
    UI32_T                  index;
    XSTP_OM_PortVar_T       *pom_ptr;

    memset(om_ptr->instance_vlans_mapped, 0, 512);
    om_ptr->instance_exist = FALSE;
    om_ptr->row_status     = VAL_dot1qVlanStaticRowStatus_notReady;
    XSTP_OM_SystemInfo.num_of_active_tree--;

    /* The variable is_member for each port in the specified instance
     * is marked as FALSE
     */
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    {
        pom_ptr             = &(om_ptr->port_info[index]);
        pom_ptr->is_member  = FALSE;
    } /* End of for */

    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_OM_DeleteTreeOm */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextXstpMember
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next XSTP member
 * INPUT    : om_ptr    -- om pointer for this instance
 *            vid       -- vlan id pointer
 * OUTPUT   : vid       -- next vlan id pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : instance_vlans_mapped[n] stores vlan [n*8+7 .. n*8]
 *            current (initial) bitmap is byte n with m bit masked
 *            in which 512 > n >= 0, 8 > m >= 0
 *            vlan 0 is stored at the least bit in byte 0.      ... (a)
 *            for a specified vlan vid, n = vid/8               ... (b)
 *                                      m = vid%8               ... (c)
 *            if byte n stores the next vlan, then vmap_byte    ... (d)
 *            should be non-zero.                               ... (e)
 *            Otherwise we check the following byte.            ... (f)
 *            When the non-zero vmap_byte is found,
 *            we determinate the least_non_zero_bit.            ... (g)
 *            Thus next_vlan_id = vmap_index*8 + least_non_zero_bit.
 *                                                              ... (h)
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextXstpMember(XSTP_OM_InstanceData_T *om_ptr, UI32_T *vid)
{
    UI32_T  num_of_masked_bit, next_vlan_id = 0, vmap_index;
    BOOL_T  found;
    UI8_T   vmap_byte, vmap_mask, least_non_zero_bit;

    num_of_masked_bit    = *vid + 1;                                            /* (a) */
    vmap_index      = (num_of_masked_bit >> 3);                                 /* (b) */
    vmap_mask       = 0xFF << ( (UI8_T)(num_of_masked_bit) & 0x07 );            /* (c) */
    found           = FALSE;

    while (!found && (vmap_index < 512) )
    {
        vmap_byte   = om_ptr->instance_vlans_mapped[vmap_index] & vmap_mask;    /* (d) */
        if (vmap_byte)                                                          /* (e) */
        {
            found           = TRUE;
            least_non_zero_bit  = 0;
            while ( !(vmap_byte & (0x01<<least_non_zero_bit) ) )                /* (g) */
            {
                least_non_zero_bit++;
            }
            next_vlan_id    = vmap_index * 8 + least_non_zero_bit;              /* (h) */
        }
        else                                                                    /* (f) */
        {
            vmap_index++;
            vmap_mask       = 0xFF;
        }
    }

    if (found)
    {
        *vid    = next_vlan_id;
        return  TRUE;
    }
    else
    {
        *vid    = 0;
        return  FALSE;
    }
} /* End of XSTP_OM_GetNextXstpMember */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMemberVlanOfInstance
 *-------------------------------------------------------------------------
 * PURPOSE  : Check whether the specified vlan is the member of this
 *            spanning tree instance
 * INPUT    : om_ptr    -- om pointer for this instance
 *            vid       -- vlan id
 * OUTPUT   : None
 * RETUEN   : TRUE if the specified vlan is the member of this instance, else
 *            FALSE
 * NOTES    :
 *          FEDCBA9876543210
 *          ----------------
 *  vid =   ----iiiiiiiiibbb
 *          is represented at
 *          bit (bbb) in
 *          instance_vlans_mapped[iiiiiiiii]
 *          iiiiiiiii = [0..511]
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_IsMemberVlanOfInstance(XSTP_OM_InstanceData_T *om_ptr, UI16_T vid)
{
    UI8_T   vlans_bmap;

    vlans_bmap  = om_ptr->instance_vlans_mapped[(vid & 0x0FF8)>>3];

    return ( ( ( (UI8_T)0x0001 << (vid&0x0007)) & vlans_bmap ) != 0 );
} /* XSTP_OM_IsMemberVlanOfInstance */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMemberPortOfInstance
 *-------------------------------------------------------------------------
 * PURPOSE  : Check whether the specified lport is the member of this
 *            spanning tree instance
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : None
 * RETUEN   : TRUE if the specified vlan is the member of this instance, else
 *            FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_IsMemberPortOfInstance(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    return (om_ptr->port_info[lport-1].is_member);
} /* XSTP_OM_IsMemberPortOfInstance */


/* ===================================================================== */
/* System Information function
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetForceVersion
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the system force version
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : force version
 *-------------------------------------------------------------------------
 */
UI8_T   XSTP_OM_GetForceVersion(void)
{
    return XSTP_OM_SystemInfo.force_version;
} /* End of XSTP_OM_GetForceVersion */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMaxHopCount
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the max_hop_count
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : max_hop_count
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetMaxHopCount(void)
{
    return XSTP_OM_SystemInfo.max_hop_count;
} /* End of XSTP_OM_GetMaxHopCount */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMaxInstanceNumber
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the max_instance_number
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : max_instance_number
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetMaxInstanceNumber(void)
{
    return XSTP_OM_SystemInfo.max_instance_number;
} /* End of XSTP_OM_GetMaxInstanceNumber */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNumOfActiveTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the num_of_active_tree
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : num_of_active_tree
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetNumOfActiveTree(void)
{
    return XSTP_OM_SystemInfo.num_of_active_tree;
} /* End of XSTP_OM_GetNumOfActiveTree */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPathCostMethod
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the path_cost_method
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : path_cost_method
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetPathCostMethod(void)
{
    return XSTP_OM_SystemInfo.path_cost_method;
} /* End of XSTP_OM_GetPathCostMethod */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRegionName
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the region name
 * INPUT    : None
 * OUTPUT   : str       -- region name
 * RETURN   : None
 * NOTE     : The string encoded winthin a fixed field of
 *            (XSTP_TYPE_REGION_NAME_MAX_LENGTH+1) octets.
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_GetRegionName(char *str)
{
    memcpy(str, XSTP_OM_SystemInfo.region_name, XSTP_TYPE_REGION_NAME_MAX_LENGTH);
    str[XSTP_TYPE_REGION_NAME_MAX_LENGTH] = 0;

    return;
} /* End of XSTP_OM_GetRegionName */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRegionRevision
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the region_revision
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : region_revision
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetRegionRevision(void)
{
    return XSTP_OM_SystemInfo.region_revision;
} /* End of XSTP_OM_GetRegionRevision */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the spanning tree status
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : spanning tree status
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetSpanningTreeStatus(void)
{
    return XSTP_OM_SystemInfo.spanning_tree_status;
} /* End of XSTP_OM_GetSpanningTreeStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTrapFlagTc
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the trap_flag_tc
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : trap_flag_tc
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetTrapFlagTc(void)
{
    return XSTP_OM_SystemInfo.trap_flag_tc;
} /* End of XSTP_OM_GetTrapFlagTc */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTrapFlagNewRoot
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the trap_flag_new_root
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : trap_flag_new_root
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetTrapFlagNewRoot(void)
{
    return XSTP_OM_SystemInfo.trap_flag_new_root;
} /* End of XSTP_OM_GetTrapFlagNewRoot */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTrapRxFlagTc
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the trap_flag_tc
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : trap_flag_tc
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetTrapRxFlagTc(void)
{
    return XSTP_OM_SystemInfo.trap_rx_tc;
} /* End of XSTP_OM_GetTrapRxFlagTc */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetForceVersion
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the system force version
 * INPUT    : force_version
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetForceVersion(UI8_T force_version)
{
    XSTP_OM_SystemInfo.force_version    = force_version;

    return;
} /* End of XSTP_OM_SetForceVersion */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetMaxHopCount
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the max_hop_count
 * INPUT    : max_hop_count
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetMaxHopCount(UI32_T max_hop_count)
{
    XSTP_OM_SystemInfo.max_hop_count = max_hop_count;

    return;
} /* End of XSTP_OM_SetMaxHopCount */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetMaxInstanceNumber
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the max_instance_number
 * INPUT    : max_instance_number
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetMaxInstanceNumber(UI32_T max_instance_number)
{
    XSTP_OM_SystemInfo.max_instance_number = max_instance_number;

    return;
} /* End of XSTP_OM_SetMaxInstanceNumber */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetPathCostMethod
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the path_cost_method
 * INPUT    : path_cost_method
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetPathCostMethod(UI32_T path_cost_method)
{
    XSTP_OM_SystemInfo.path_cost_method = path_cost_method;

    return;
} /* End of XSTP_OM_SetPathCostMethod */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetRegionName
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the region name
 * INPUT    : str       -- region name
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetRegionName(char *str)
{
    if (strlen(str) <= XSTP_TYPE_REGION_NAME_MAX_LENGTH)
    {
        memset(XSTP_OM_SystemInfo.region_name, 0, XSTP_TYPE_REGION_NAME_MAX_LENGTH);
        memcpy(XSTP_OM_SystemInfo.region_name, str, XSTP_TYPE_REGION_NAME_MAX_LENGTH);
    }

    return;
} /* End of XSTP_OM_SetRegionName */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetRegionRevision
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the region_revision
 * INPUT    : region_revision
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetRegionRevision(UI32_T region_revision)
{
    XSTP_OM_SystemInfo.region_revision = region_revision;

    return;
} /* End of XSTP_OM_SetRegionRevision */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the spanning tree status
 * INPUT    : spanning_tree_status
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetSpanningTreeStatus(UI32_T spanning_tree_status)
{
    XSTP_OM_SystemInfo.spanning_tree_status = spanning_tree_status;

    return;
} /* End of XSTP_OM_SetSpanningTreeStatus */


#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the port tc prop status
 * INPUT    : logical port number
 *            enable_status
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void XSTP_OM_SetPortTcPropStop(UI32_T lport, BOOL_T enable_status)
{
    XSTP_OM_InstanceInfo[XSTP_TYPE_CISTID].port_info[lport-1].common->tc_prop_stop = enable_status;
    return;
}
#endif
#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the port tc prop status
 * INPUT    : logical port number
 *            enable_status
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsPortTcPropStop(UI32_T lport)
{
    return XSTP_OM_InstanceInfo[XSTP_TYPE_CISTID].port_info[lport-1].common->tc_prop_stop;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the port tc prop status
 * INPUT    : logical port number
 *            enable_status
 * OUTPUT   : None
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS - need to store configuration
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - configure no change
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortTcPropStop(UI32_T lport, UI32_T *enable_status_p)
{
    *enable_status_p = XSTP_OM_InstanceInfo[XSTP_TYPE_CISTID].port_info[lport-1].common->tc_prop_stop;

    if(*enable_status_p!=FALSE) /*default always false*/
      return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetTrapFlagTc
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the trap_flag_tc
 * INPUT    : trap_flag_tc
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetTrapFlagTc(BOOL_T trap_flag_tc)
{
    XSTP_OM_SystemInfo.trap_flag_tc = trap_flag_tc;

    return;
} /* End of XSTP_OM_SetTrapFlagTc */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetTrapFlagNewRoot
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the trap_flag_new_root
 * INPUT    : trap_flag_new_root
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetTrapFlagNewRoot(BOOL_T trap_flag_new_root)
{
    XSTP_OM_SystemInfo.trap_flag_new_root = trap_flag_new_root;

    return;
} /* End of XSTP_OM_SetTrapFlagNewRoot */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetTrapRxFlagTc
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the trap_flag_tc
 * INPUT    : trap_flag_tc
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetTrapRxFlagTc(BOOL_T trap_flag_tc)
{
    XSTP_OM_SystemInfo.trap_rx_tc = trap_flag_tc;

    return;
} /* End of XSTP_OM_SetTrapFlagTc */


#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetLportPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the path cost of the specified logical port
 * INPUT    :   lport       : lport
 * OUTPUT   :   path_cost   : path cost
 * RETURN   :   XSTP_TYPE_RETURN_OK/XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetLportPathCost(UI32_T lport, UI32_T *path_cost)
{
    UI32_T                  speed_duplex, count;
    Port_Info_T             port_info;
    UI32_T                  unit, port, trunk_id;
    SWCTRL_Lport_Type_T     result;
    UI32_T                  current_method;

    if (SWCTRL_GetPortInfo(lport, &port_info))
    {
        speed_duplex = port_info.speed_duplex_oper;
    }
    else
    {
        *path_cost = XSTP_TYPE_PATH_COST_F10M;
        return XSTP_TYPE_RETURN_ERROR;
    }

    result = SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
    if (result == SWCTRL_LPORT_NORMAL_PORT)
    {
        count = 1;
    }
    else if (result == SWCTRL_LPORT_TRUNK_PORT)
    {
        count = TRK_PMGR_GetTrunkMemberCounts(trunk_id);
    }
    else
    {
        *path_cost = XSTP_TYPE_PATH_COST_F10M;
        return XSTP_TYPE_RETURN_ERROR;
    }

    if (result == SWCTRL_LPORT_TRUNK_PORT)
    {
        switch (speed_duplex)
        {
            case XSTP_TYPE_SPDPLX_10HALF:
            case XSTP_TYPE_SPDPLX_10FULL:
                *path_cost = XSTP_TYPE_PATH_COST_T10M;
                break;
            case XSTP_TYPE_SPDPLX_100HALF:
            case XSTP_TYPE_SPDPLX_100FULL:
                *path_cost = XSTP_TYPE_PATH_COST_T100M;
                break;
            case XSTP_TYPE_SPDPLX_1000HALF:
            case XSTP_TYPE_SPDPLX_1000FULL:
                *path_cost = XSTP_TYPE_PATH_COST_T1G;
                break;
            case XSTP_TYPE_SPDPLX_10GHALF:
            case XSTP_TYPE_SPDPLX_10GFULL:
                *path_cost = XSTP_TYPE_PATH_COST_T10G;
                break;

            default:
                *path_cost = XSTP_TYPE_PATH_COST_T10M;
                break;
        }
    }
    else
    {
        switch (speed_duplex)
        {
            case XSTP_TYPE_SPDPLX_10HALF:
                *path_cost = XSTP_TYPE_PATH_COST_H10M;
                break;
            case XSTP_TYPE_SPDPLX_10FULL:
                *path_cost = XSTP_TYPE_PATH_COST_F10M;
                break;
            case XSTP_TYPE_SPDPLX_100HALF:
                *path_cost = XSTP_TYPE_PATH_COST_H100M;
                break;
            case XSTP_TYPE_SPDPLX_100FULL:
                *path_cost = XSTP_TYPE_PATH_COST_F100M;
                break;
            case XSTP_TYPE_SPDPLX_1000HALF:
                *path_cost = XSTP_TYPE_PATH_COST_H1G;
                break;
            case XSTP_TYPE_SPDPLX_1000FULL:
                *path_cost = XSTP_TYPE_PATH_COST_F1G;
                break;
            case XSTP_TYPE_SPDPLX_10GHALF:
                *path_cost = XSTP_TYPE_PATH_COST_H10G;
                break;
            case XSTP_TYPE_SPDPLX_10GFULL:
                *path_cost = XSTP_TYPE_PATH_COST_F10G;
                break;
            default:
                *path_cost = XSTP_TYPE_PATH_COST_F10M;
                break;
        }
    }
    current_method = (UI32_T)XSTP_OM_GetPathCostMethod();
    if ((current_method == XSTP_TYPE_PATH_COST_DEFAULT_SHORT) && (*path_cost > 65535))
    {
        *path_cost = 65535;
    }
    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_OM_GetLportPathCost */
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetLportDefaultPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the default path cost of the specified logical port
 * INPUT    :   lport       -- lport
 * OUTPUT   :   path_cost   -- path cost
 * RETURN   :   XSTP_TYPE_RETURN_OK/XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetLportDefaultPathCost(UI32_T lport, UI32_T *path_cost)
{
    UI32_T                  speed_duplex, count;
    Port_Info_T             port_info;
    UI32_T                  unit, port, trunk_id;
    SWCTRL_Lport_Type_T     result;
    UI32_T                  active_lportlist[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
    UI8_T                   cost_format, cost_speed, cost_type, cost_factor;

    /* Default assignment */
    cost_format = XSTP_OM_FORMAT_LONG;
    cost_speed  = XSTP_OM_SPEED_10M;
    cost_type   = XSTP_OM_TYPE_HDPLX;

    /* cost_format */
    if (XSTP_OM_GetPathCostMethod() == VAL_xstInstanceCfgPathCostMethod_short)
    {
        cost_format = XSTP_OM_FORMAT_SHORT;
    }
    if (SWCTRL_GetPortInfo(lport, &port_info))
    {
        speed_duplex = port_info.speed_duplex_oper;
    }
    else
    {
        *path_cost = XSTP_OM_DefaultCost[cost_format][cost_speed][cost_type];
        return XSTP_TYPE_RETURN_ERROR;
    }

    result = SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    /* count */
    if (result == SWCTRL_LPORT_NORMAL_PORT)
    {
        count = 1;
    }
    else if (result == SWCTRL_LPORT_TRUNK_PORT)
    {
        if (    (!SWCTRL_GetActiveTrunkMember(lport, active_lportlist, &count))
            ||  (count == 0)
           )
        {
            count = 1;
        }
    }
    else
    {
        *path_cost = XSTP_OM_DefaultCost[cost_format][cost_speed][cost_type];
        return XSTP_TYPE_RETURN_ERROR;
    }

    /* cost_speed, cost_factor */
    switch (speed_duplex)
    {
        case VAL_portSpeedDpxCfg_halfDuplex10:
        case VAL_portSpeedDpxCfg_fullDuplex10:
            cost_speed  = XSTP_OM_SPEED_10M;
            cost_factor = XSTP_OM_FACTOR(count, SYS_DFLT_XSTP_PATH_COST_TM_FACTOR_10M);
            break;
        case VAL_portSpeedDpxCfg_halfDuplex100:
        case VAL_portSpeedDpxCfg_fullDuplex100:
            cost_speed  = XSTP_OM_SPEED_100M;
            cost_factor = XSTP_OM_FACTOR(count, SYS_DFLT_XSTP_PATH_COST_TM_FACTOR_100M);
            break;
        case VAL_portSpeedDpxCfg_halfDuplex1000:
        case VAL_portSpeedDpxCfg_fullDuplex1000:
            cost_speed  = XSTP_OM_SPEED_1G;
            cost_factor = XSTP_OM_FACTOR(count, SYS_DFLT_XSTP_PATH_COST_TM_FACTOR_1G);
            break;
        case VAL_portSpeedDpxCfg_halfDuplex10g:
        case VAL_portSpeedDpxCfg_fullDuplex10g:
            cost_speed  = XSTP_OM_SPEED_10G;
            cost_factor = XSTP_OM_FACTOR(count, SYS_DFLT_XSTP_PATH_COST_TM_FACTOR_10G);
            break;
        case VAL_portSpeedDpxCfg_halfDuplex25g:
        case VAL_portSpeedDpxCfg_fullDuplex25g:
            cost_speed  = XSTP_OM_SPEED_25G;
            cost_factor = XSTP_OM_FACTOR(count, SYS_DFLT_XSTP_PATH_COST_TM_FACTOR_25G);
            break;
        case VAL_portSpeedDpxCfg_halfDuplex40g:
        case VAL_portSpeedDpxCfg_fullDuplex40g:
                cost_speed  = XSTP_OM_SPEED_40G;
                cost_factor = XSTP_OM_FACTOR(count, SYS_DFLT_XSTP_PATH_COST_TM_FACTOR_40G);
                break;
        case VAL_portSpeedDpxCfg_halfDuplex100g:
        case VAL_portSpeedDpxCfg_fullDuplex100g:
            cost_speed  = XSTP_OM_SPEED_100G;
            cost_factor = XSTP_OM_FACTOR(count, SYS_DFLT_XSTP_PATH_COST_TM_FACTOR_100G);
            break;
        default:
            cost_speed  = XSTP_OM_SPEED_10M;
            cost_factor = XSTP_OM_FACTOR(count, XSTP_OM_PATH_COST_TM_FACTOR_OFF);
            break;
    }

    /* cost_type, path_cost */
    if (result == SWCTRL_LPORT_TRUNK_PORT)
    {
        cost_type   = XSTP_OM_TYPE_TRUNK;
        *path_cost  = XSTP_OM_DefaultCost[cost_format][cost_speed][cost_type]/cost_factor;
    }
    else
    {
        switch (speed_duplex)
        {
            case VAL_portSpeedDpxCfg_fullDuplex10:
            case VAL_portSpeedDpxCfg_fullDuplex100:
            case VAL_portSpeedDpxCfg_fullDuplex1000:
            case VAL_portSpeedDpxCfg_fullDuplex10g:
            case VAL_portSpeedDpxCfg_fullDuplex25g:
            case VAL_portSpeedDpxCfg_fullDuplex40g:
            case VAL_portSpeedDpxCfg_fullDuplex100g:
                cost_type   = XSTP_OM_TYPE_FDPLX;
                break;
            default:
                cost_type   = XSTP_OM_TYPE_HDPLX;
                break;
        }
        *path_cost  = XSTP_OM_DefaultCost[cost_format][cost_speed][cost_type];
    }

    if (*path_cost == 0)
    {
        *path_cost = 1;
    }
    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_OM_GetLportDefaultPathCost */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_RefreshPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Refresh the path cost of the specified logical port
 * INPUT    :   om_ptr      : om pointer for this instance
 *              lport       : lport
 * OUTPUT   :   path_cost   : path cost
 * RETURN   :   TRUE if the path cost is changed, else FALSE returned.
 * NOTE     :   There are 3 cases which may change the path cost.
 *              1. The path cost overflow when the path cost method is changed
 *                 from long to short.
 *              2. The speed-duplex mode of the port changed.
 *              3. Users erase the static path cost set by manual operations.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_RefreshPathCost(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  old_value, new_value, short_value;
    BOOL_T                  result  = FALSE;
    BOOL_T                  is_short_method;
    BOOL_T                  default_value_existing;
    UI32_T                  index;
    XSTP_OM_PortVar_T       *index_pom_ptr;


    pom_ptr         = &(om_ptr->port_info[lport-1]);

    is_short_method = (XSTP_OM_GetPathCostMethod() == VAL_xstInstanceCfgPathCostMethod_short);
    default_value_existing  = (XSTP_OM_GetLportDefaultPathCost(lport, &new_value) == XSTP_TYPE_RETURN_OK);

    #ifdef XSTP_TYPE_PROTOCOL_RSTP
    old_value   = pom_ptr->port_path_cost;
    if (    (pom_ptr->static_path_cost)
        &&  is_short_method
       )
    {
        /* Force to fit in short format */
        short_value = XSTP_OM_SHORT_FORMAT(old_value);
        pom_ptr->port_path_cost = short_value;
        result  = result || (short_value != old_value);
    }
    else
    if (    (default_value_existing)
        &&  (!pom_ptr->static_path_cost)
       )
    {
        pom_ptr->port_path_cost = new_value;
        result  = result || (new_value != old_value);
    }
    if (result)
    {
        /* Root port. For consistency */
        if (lport == om_ptr->bridge_info.root_port_id.data.port_num)
        {
            UI32_T                  root_path_cost;
            root_path_cost = pom_ptr->port_priority.root_path_cost + pom_ptr->port_path_cost;
            om_ptr->bridge_info.root_priority.root_path_cost = root_path_cost;
            for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
            {
                index_pom_ptr = &(om_ptr->port_info[index]);
                index_pom_ptr->designated_priority.root_path_cost = root_path_cost;
                if(   (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                    ||(index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
                    ||((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                        && (root_path_cost  < index_pom_ptr->port_priority.root_path_cost)
                      )
                  )
                index_pom_ptr->port_priority.root_path_cost = root_path_cost;
            }
        }
    }
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef XSTP_TYPE_PROTOCOL_MSTP
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
    {
        old_value   = pom_ptr->common->external_port_path_cost;
        if (    (pom_ptr->common->static_external_path_cost)
            &&  is_short_method
           )
        {
            /* Force to fit in short format */
            short_value = XSTP_OM_SHORT_FORMAT(old_value);
            pom_ptr->common->external_port_path_cost = short_value;
            result  = result || (short_value != old_value);
        }
        else
        if (    (default_value_existing)
            &&  (!pom_ptr->common->static_external_path_cost)
           )
        {
            pom_ptr->common->external_port_path_cost = new_value;
            result  = result || (new_value != old_value);
        }

        if (result)
        {
            /* Root port. For consistency */
            if (lport == om_ptr->bridge_info.root_port_id.data.port_num)
            {
                UI32_T                  ext_root_path_cost;
                ext_root_path_cost = pom_ptr->port_priority.ext_root_path_cost + pom_ptr->common->external_port_path_cost;
                if (!pom_ptr->common->rcvd_internal)
                {
                    om_ptr->bridge_info.root_priority.ext_root_path_cost = ext_root_path_cost;
                    for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
                    {
                        index_pom_ptr = &(om_ptr->port_info[index]);
                        index_pom_ptr->designated_priority.ext_root_path_cost = ext_root_path_cost;
                        if( (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                            || (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
                            || ( (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                                && (ext_root_path_cost  < index_pom_ptr->port_priority.ext_root_path_cost)
                               )
                          )
                        index_pom_ptr->port_priority.ext_root_path_cost = ext_root_path_cost;
                    }
                }
            }
        }
    } /* End of if (_CISTID_) */

    old_value   = pom_ptr->internal_port_path_cost;
    if (    (pom_ptr->static_internal_path_cost)
        &&  is_short_method
       )
    {
        /* Force to fit in short format */
        short_value = XSTP_OM_SHORT_FORMAT(old_value);
        pom_ptr->internal_port_path_cost = short_value;
        result  = result || (short_value != old_value);
    }
    else
    if (    (default_value_existing)
        &&  (!pom_ptr->static_internal_path_cost)
       )
    {
        pom_ptr->internal_port_path_cost = new_value;
        result  = result || (new_value != old_value);
    }

    if (result)
    {
        /* Root port. For consistency */
        if (lport == om_ptr->bridge_info.root_port_id.data.port_num)
        {
            UI32_T                  int_root_path_cost;
            int_root_path_cost = pom_ptr->port_priority.int_root_path_cost + pom_ptr->internal_port_path_cost;
            om_ptr->bridge_info.root_priority.int_root_path_cost = int_root_path_cost;
            for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
            {
                index_pom_ptr = &(om_ptr->port_info[index]);
                index_pom_ptr->designated_priority.int_root_path_cost = int_root_path_cost;
                if( (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                    || (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
                    || ( (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                        && (int_root_path_cost  < index_pom_ptr->port_priority.int_root_path_cost)
                       )
                  )
                index_pom_ptr->port_priority.int_root_path_cost = int_root_path_cost;
            }
        }
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return result;
} /* End of XSTP_OM_RefreshPathCost */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_RefreshOperLinkType
 * ------------------------------------------------------------------------
 * PURPOSE  : Refresh the oper link_type for the specified port.
 * INPUT    : om_ptr                  -- the pointer of the instance entry.
 *            lport                   -- lport number
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    XSTP_OM_RefreshOperLinkType(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  speed_duplex_oper;
    Port_Info_T             port_info;

    /* auto-determinated link type */
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
    {
        pom_ptr = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->common->admin_point_to_point_mac_auto)
        {
            SWCTRL_GetPortInfo(lport, &port_info);
            speed_duplex_oper = port_info.speed_duplex_oper;
            if (    (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_10FULL)
                ||  (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_100FULL)
                ||  (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_1000FULL)
                ||  (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_10GFULL)
                ||  (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_25GFULL)
                ||  (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_40GFULL)
                ||  (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_100GFULL)
               )
            {
                pom_ptr->common->oper_point_to_point_mac    = TRUE;
            }
            else
            {
                pom_ptr->common->oper_point_to_point_mac    = FALSE;
            }
        }
    }
    return;
}/* End of XSTP_OM_RefreshOperLinkType */

#ifdef  XSTP_TYPE_PROTOCOL_RSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmPriorityConsistency
 * ------------------------------------------------------------------------
 * PURPOSE  : Let priority recorded in OM consist with bridge_priority
 *            set by user
 * INPUT    : om_ptr    -- om pointer for this instance
 *            priority  -- bridge_priority value
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmPriorityConsistency(XSTP_OM_InstanceData_T *om_ptr, UI32_T priority)
{
    UI32_T                  index;
    UI16_T                  om_priority;
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_PortId_T      zero_port_id;

    om_priority = (UI16_T)(0x000f & (priority >> 12));
    XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.bridge_identifier, (UI16_T)priority);
    XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.bridge_priority.root_bridge_id, (UI16_T)priority);
    XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.bridge_priority.designated_bridge_id, (UI16_T)priority);

    XSTP_OM_PORT_ID_FORMAT(zero_port_id, 0, 0);

    /* bridge_priority_vector */
    XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->bridge_info.bridge_priority, om_ptr->bridge_info.bridge_identifier,
                                    0, om_ptr->bridge_info.bridge_identifier, zero_port_id, zero_port_id);

    for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
    {
        pom_ptr     = &(om_ptr->port_info[index]);

        /* 17.18.2 designated_priority */
        XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->port_info[index].designated_priority,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        pom_ptr->port_id,
                                        pom_ptr->port_id);
        /* 17.18.17 port_priority */
        XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->port_info[index].port_priority,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        pom_ptr->port_id,
                                        pom_ptr->port_id);


    }
    /* For bridge variable: root_priority
       If root_priority.root_bridge_id is same as bridge_identifier, update root_bridge_id.priority
    */
    if(memcmp( (UI8_T *)&(om_ptr->bridge_info.root_priority.root_bridge_id.addr),
               (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
               6
             )==0
       )
    {
        XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.root_priority.root_bridge_id, (UI16_T)priority);
    }
    /* For bridge variable: root_priority
       If root_priority.designated_bridge_id is same as bridge_identifier, update designated_bridge_id.priority
    */
    if(memcmp( (UI8_T *)&(om_ptr->bridge_info.root_priority.designated_bridge_id.addr),
               (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
               6
             )==0
       )
    {
        XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.root_priority.designated_bridge_id, (UI16_T)priority);
    }
    /* For per-port variable: port_priority and designated_priority,
       If port_priority.root_bridge_id is same as bridge_identifier, update root_bridge_id.priority.
       If port_priority.designated_bridge_id is same as bridge_identifier, update designated_bridge_id.priority.
       If designated_priority.root_bridge_id is same as bridge_identifier, update root_bridge_id.priority.
       If designated_priority.designated_bridge_id is same as bridge_identifier, update designated_bridge_id.priority.
    */
    for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
    {
        pom_ptr     = &(om_ptr->port_info[index]);
        if(memcmp( (UI8_T *)&(pom_ptr->port_priority.root_bridge_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->port_priority.root_bridge_id, (UI16_T)priority);
        }
        if(memcmp( (UI8_T *)&(pom_ptr->port_priority.designated_bridge_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->port_priority.designated_bridge_id, (UI16_T)priority);
        }
        if(memcmp( (UI8_T *)&(pom_ptr->designated_priority.root_bridge_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->designated_priority.root_bridge_id, (UI16_T)priority);
        }
        if(memcmp( (UI8_T *)&(pom_ptr->designated_priority.designated_bridge_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->designated_priority.designated_bridge_id, (UI16_T)priority);
        }
    }

}/* End of XSTP_OM_MakeOmPriorityConsistency */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmPortPriorityConsistency
 * ------------------------------------------------------------------------
 * PURPOSE  : Let priority recorded in OM consist with port_priority
 *            set by user
 * INPUT    : om_ptr            -- om pointer for this instance
 *            lport             -- lport value
 *            port_priority     -- port_priority value
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmPortPriorityConsistency(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T port_priority)
{
    UI16_T                  om_port_priority;
    XSTP_OM_PortVar_T       *pom_ptr;
    I32_T                   cmp_a, cmp_b;

    om_port_priority = (UI16_T)(0x0f & (port_priority >> 4));
    pom_ptr = &(om_ptr->port_info[lport-1]);
    /* Kelly_Chen, 07/18/02, for per-port variable: port_priority and designated_priority,
       If designated_priority.bridge_port_id is same as port_id, update bridge_port_id.data.priority.
    */
    XSTP_OM_CMP_PORT_ID(cmp_a, (pom_ptr->designated_priority.designated_port_id), (pom_ptr->port_id));
    XSTP_OM_CMP_BRIDGE_ID(cmp_b, (pom_ptr->designated_priority.designated_bridge_id), (om_ptr->bridge_info.bridge_identifier));
    if(     (cmp_a == 0)
        &&  (cmp_b == 0)
      )
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(pom_ptr->designated_priority.designated_port_id, (UI8_T)port_priority);
    }

    /* If designated_priority.bridge_port_id is same as port_id, update bridge_port_id.data.priority. */
    XSTP_OM_CMP_PORT_ID(cmp_a, (pom_ptr->designated_priority.bridge_port_id), (pom_ptr->port_id));
    if(cmp_a == 0)
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(pom_ptr->designated_priority.bridge_port_id, (UI8_T)port_priority);
    }

    /* If port_priority.designated_port_id is same as port_id, update designated_port_id.data.priority. */
    XSTP_OM_CMP_PORT_ID(cmp_a, (pom_ptr->port_priority.designated_port_id), (pom_ptr->port_id));
    XSTP_OM_CMP_BRIDGE_ID(cmp_b, (pom_ptr->port_priority.designated_bridge_id), (om_ptr->bridge_info.bridge_identifier));
    if(     (cmp_a == 0)
        &&  (cmp_b == 0)
      )
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(pom_ptr->port_priority.designated_port_id, (UI8_T)port_priority);
    }
    /* Kelly_Chen, 07/18/02, for bridge variable: bridge_priority and root_priority
       If bridge_priority.designated_port_id is same as port_id, update designated_port_id.data.priority.
     */
    XSTP_OM_CMP_PORT_ID(cmp_a, (om_ptr->bridge_info.bridge_priority.designated_port_id), (pom_ptr->port_id));
    if(cmp_a == 0)
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(om_ptr->bridge_info.bridge_priority.designated_port_id, (UI8_T)port_priority);
    }
    /* If bridge_priority.bridge_port_id is same as port_id, update bridge_port_id.data.priority. */
    XSTP_OM_CMP_PORT_ID(cmp_a, (om_ptr->bridge_info.bridge_priority.bridge_port_id), (pom_ptr->port_id));
    if(cmp_a == 0)
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(om_ptr->bridge_info.bridge_priority.bridge_port_id, (UI8_T)port_priority);
    }
    /* If root_priority.designated_port_id is same as port_id, update designated_port_id.data.priority. */
    XSTP_OM_CMP_PORT_ID(cmp_a, (om_ptr->bridge_info.root_priority.designated_port_id), (pom_ptr->port_id));
    XSTP_OM_CMP_BRIDGE_ID(cmp_b, (om_ptr->bridge_info.root_priority.designated_bridge_id), (om_ptr->bridge_info.bridge_identifier));
    if(     (cmp_a == 0)
        &&  (cmp_b == 0)
      )
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(om_ptr->bridge_info.root_priority.designated_port_id, (UI8_T)port_priority);
    }
    /* If root_priority.bridge_port_id is same as port_id, update bridge_port_id.data.priority. */
    XSTP_OM_CMP_PORT_ID(cmp_a, (om_ptr->bridge_info.root_priority.bridge_port_id), (pom_ptr->port_id));
    if(cmp_a == 0)
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(om_ptr->bridge_info.root_priority.bridge_port_id, (UI8_T)port_priority);
    }

    XSTP_OM_SET_PORT_ID_PRIORITY(pom_ptr->port_id, (UI8_T)port_priority);
    XSTP_OM_SET_PORT_ID_PRIORITY(pom_ptr->port_priority.bridge_port_id, (UI8_T)port_priority);

}/* End of XSTP_OM_MakeOmPortPriorityConsistency */

#endif /* XSTP_TYPE_PROTOCOL_RSTP */


#ifdef  XSTP_TYPE_PROTOCOL_MSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmLportConsistency
 * ------------------------------------------------------------------------
 * PURPOSE  : Rename the specified lport if it is a root port or designated
 *            port
 * INPUT    : om_ptr    -- om pointer for this instance
 *            dst_lport -- destinating lport
 *            src_lport -- source lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmLportConsistency(XSTP_OM_InstanceData_T *om_ptr, UI32_T dst_lport, UI32_T src_lport)
{
    XSTP_OM_PortVar_T       *dst_pom_ptr;
    XSTP_OM_PortVar_T       *src_pom_ptr;

    dst_pom_ptr         = &(om_ptr->port_info[dst_lport-1]);
    src_pom_ptr         = &(om_ptr->port_info[src_lport-1]);

    /* port_priority.bridge_port_id */
    dst_pom_ptr->port_priority.rcv_port_id.port_id               = dst_pom_ptr->port_id.port_id;

    /* designated_priority.designated_port_id, designated_priority.bridge_port_id */
    dst_pom_ptr->designated_priority.designated_port_id.port_id     = dst_pom_ptr->port_id.port_id;
    dst_pom_ptr->designated_priority.rcv_port_id.port_id         = dst_pom_ptr->port_id.port_id;

    /* If src_lport is the bridge's root port */
    /* bridge_info.root_port_id */
    /* bridge_info.root_priority.bridge_port_id, bridge_info.root_priority.designated_port_id */
    if (om_ptr->bridge_info.root_port_id.port_id == src_pom_ptr->port_id.port_id)
    {
        om_ptr->bridge_info.root_port_id.port_id                        = dst_pom_ptr->port_id.port_id;
    }
    if (om_ptr->bridge_info.root_priority.rcv_port_id.port_id == src_pom_ptr->port_id.port_id)
    {
        om_ptr->bridge_info.root_priority.rcv_port_id.port_id        = dst_pom_ptr->port_id.port_id;
    }
    if (om_ptr->bridge_info.root_priority.designated_port_id.port_id == src_pom_ptr->port_id.port_id)
    {
        om_ptr->bridge_info.root_priority.designated_port_id.port_id    = dst_pom_ptr->port_id.port_id;
    }

    return;
} /* End of XSTP_OM_MakeOmLportConsistency */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_ConvertPortOm
 * ------------------------------------------------------------------------
 * PURPOSE  : Convert the Port OM due to its lport number is changed
 * INPUT    : om_ptr    -- om pointer for this instance
 *            dst_lport -- destinating lport
 *            src_lport -- source lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_ConvertPortOm(XSTP_OM_InstanceData_T *om_ptr, UI32_T dst_lport, UI32_T src_lport)
{
    XSTP_OM_PortVar_T       *dst_pom_ptr;
    XSTP_OM_PortVar_T       *src_pom_ptr;
    XSTP_OM_PortCommonVar_T *common;
    UI32_T                  path_cost;
    XSTP_OM_MstPortVar_T    *cist;

    /* Copy Port OM */
    dst_pom_ptr         = &(om_ptr->port_info[dst_lport-1]);
    src_pom_ptr         = &(om_ptr->port_info[src_lport-1]);
    common              = dst_pom_ptr->common;
    cist                = dst_pom_ptr->cist;
    memcpy(dst_pom_ptr, src_pom_ptr, sizeof(XSTP_OM_PortVar_T) );
    dst_pom_ptr->common = common;
    if (om_ptr->bridge_info.cist == NULL)
    {
        memcpy(dst_pom_ptr->common, src_pom_ptr->common, sizeof(XSTP_OM_PortCommonVar_T) );
    }
    dst_pom_ptr->cist = cist;

    /* Convert state machine performance parameters */      /* 13.22 */
    /* 13.22 (g) */
    XSTP_OM_GetLportDefaultPathCost(dst_lport, &path_cost);
    if (    (!dst_pom_ptr->common->static_external_path_cost)
        &&  (om_ptr->bridge_info.cist == NULL)
       )
    {
        dst_pom_ptr->common->external_port_path_cost     = path_cost;
    }
    /* 13.22 (h) */
    if (!dst_pom_ptr->static_internal_path_cost)
    {
        dst_pom_ptr->internal_port_path_cost             = path_cost;
    }

    /* Convert per-port variables */                        /* 13.24 */
    /* 13.24 (ae) */
    dst_pom_ptr->port_id.port_id    =   (src_pom_ptr->port_id.port_id & 0xF000) | ( (UI16_T)dst_lport & 0x0FFF);

    return;
} /* End of XSTP_OM_ConvertPortOm */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmPriorityConsistencyCist
 * ------------------------------------------------------------------------
 * PURPOSE  : Let priority recorded in OM consist with bridge_priority
 *            set by user
 * INPUT    : om_ptr    -- om pointer for this instance
 *            priority  -- bridge_priority value
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmPriorityConsistencyCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T priority)
{
    UI32_T                  index;
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_PortId_T      zero_port_id;

    XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.bridge_identifier,(UI16_T)priority);
    XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.bridge_priority.root_id,(UI16_T)priority);
    XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.bridge_priority.designated_bridge_id,(UI16_T)priority);

    XSTP_OM_PORT_ID_FORMAT(zero_port_id, 0, 0);

    /* bridge_priority_vector */
    XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->bridge_info.bridge_priority,
                                    om_ptr->bridge_info.bridge_identifier,
                                    0,
                                    om_ptr->bridge_info.bridge_identifier,
                                    0,
                                    om_ptr->bridge_info.bridge_identifier,
                                    zero_port_id,
                                    zero_port_id);

    for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
    {
        pom_ptr     = &(om_ptr->port_info[index]);

        /* 13.24.5 cist designated_priority */
        XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->port_info[index].designated_priority,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        pom_ptr->port_id,
                                        pom_ptr->port_id);
        /* 13.24.8 cist port_priority */
        XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->port_info[index].port_priority,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        pom_ptr->port_id,
                                        pom_ptr->port_id);


    }
    /* For bridge variable: root_priority
       If root_priority.root_id is same as bridge_identifier, update root_id.priority
    */
    if(memcmp( (UI8_T *)&(om_ptr->bridge_info.root_priority.root_id.addr),
               (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
               6
             )==0
       )
    {
        XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.root_priority.root_id,(UI16_T)priority);
    }

    /* For bridge variable: root_priority
       If root_priority.r_root_id is same as bridge_identifier, update root_id.priority
    */
    if(memcmp( (UI8_T *)&(om_ptr->bridge_info.root_priority.r_root_id.addr),
               (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
               6
             )==0
       )
    {
        XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.root_priority.r_root_id,(UI16_T)priority);
    }

    /* For bridge variable: root_priority
       If root_priority.designated_bridge_id is same as bridge_identifier, update designated_bridge_id.priority
    */
    if(memcmp( (UI8_T *)&(om_ptr->bridge_info.root_priority.designated_bridge_id.addr),
               (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
               6
             )==0
       )
    {
        XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.root_priority.designated_bridge_id,(UI16_T)priority);
    }
    /* For per-port variable: port_priority and designated_priority,
       If port_priority.root_id is same as bridge_identifier, update root_id.priority.
       If port_priority.r_root_id is same as bridge_identifier, update r_root_id.priority.
       If port_priority.designated_bridge_id is same as bridge_identifier, update designated_bridge_id.priority.
       If designated_priority.root_id is same as bridge_identifier, update root_id.priority.
       If designated_priority.r_root_id is same as bridge_identifier, update r_root_id.priority.
       If designated_priority.designated_bridge_id is same as bridge_identifier, update designated_bridge_id.priority.
    */
    for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
    {
        pom_ptr     = &(om_ptr->port_info[index]);
        if(memcmp( (UI8_T *)&(pom_ptr->port_priority.root_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->port_priority.root_id,(UI16_T)priority);
        }

        if(memcmp( (UI8_T *)&(pom_ptr->port_priority.r_root_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->port_priority.r_root_id,(UI16_T)priority);
        }

        if(memcmp( (UI8_T *)&(pom_ptr->port_priority.designated_bridge_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->port_priority.designated_bridge_id,(UI16_T)priority);
        }
        if(memcmp( (UI8_T *)&(pom_ptr->designated_priority.root_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->designated_priority.root_id,(UI16_T)priority);
        }
        if(memcmp( (UI8_T *)&(pom_ptr->designated_priority.r_root_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->designated_priority.r_root_id,(UI16_T)priority);
        }
        if(memcmp( (UI8_T *)&(pom_ptr->designated_priority.designated_bridge_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->designated_priority.designated_bridge_id,(UI16_T)priority);
        }
    }

}/* End of XSTP_OM_MakeOmPriorityConsistencyCist */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmPriorityConsistencyMsti
 * ------------------------------------------------------------------------
 * PURPOSE  : Let priority recorded in OM consist with bridge_priority
 *            set by user
 * INPUT    : om_ptr    -- om pointer for this instance
 *            priority  -- bridge_priority value
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmPriorityConsistencyMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T priority)
{
    UI32_T                  index;
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_PortId_T      zero_port_id;
    XSTP_TYPE_BridgeId_T    bridge_id;

    XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.bridge_identifier,(UI16_T)priority);
    XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.bridge_priority.root_id,(UI16_T)priority);
    XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.bridge_priority.designated_bridge_id,(UI16_T)priority);

    XSTP_OM_PORT_ID_FORMAT(zero_port_id, 0, 0);
    memset(&bridge_id, 0, XSTP_TYPE_BRIDGE_ID_LENGTH);

    /* bridge_priority_vector */
    XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->bridge_info.bridge_priority,
                                    bridge_id,
                                    0,
                                    om_ptr->bridge_info.bridge_identifier,
                                    0,
                                    om_ptr->bridge_info.bridge_identifier,
                                    zero_port_id,
                                    zero_port_id);

    for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
    {
        pom_ptr     = &(om_ptr->port_info[index]);

        /* 13.24.11 msti designated_priority */
        XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->port_info[index].designated_priority,
                                        bridge_id,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        pom_ptr->port_id,
                                        pom_ptr->port_id);
        /* 13.24.17 msti port_priority */
        XSTP_OM_PRIORITY_VECTOR_FORMAT( om_ptr->port_info[index].port_priority,
                                        bridge_id,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        pom_ptr->port_id,
                                        pom_ptr->port_id);


    }

    /* For bridge variable: root_priority
       If root_priority.r_root_id is same as bridge_identifier, update r_root_id.priority
    */
    if(memcmp( (UI8_T *)&(om_ptr->bridge_info.root_priority.r_root_id.addr),
               (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
               6
             )==0
       )
    {
        XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.root_priority.r_root_id,(UI16_T)priority);
    }
    /* For bridge variable: root_priority
       If root_priority.designated_bridge_id is same as bridge_identifier, update designated_bridge_id.priority
    */
    if(memcmp( (UI8_T *)&(om_ptr->bridge_info.root_priority.designated_bridge_id.addr),
               (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
               6
             )==0
       )
    {
        XSTP_OM_SET_BRIDGE_ID_PRIORITY(om_ptr->bridge_info.root_priority.designated_bridge_id,(UI16_T)priority);
    }
    /* For per-port variable: port_priority and designated_priority,
       If port_priority.r_root_id is same as bridge_identifier, update r_root_id.priority.
       If port_priority.designated_bridge_id is same as bridge_identifier, update designated_bridge_id.priority.
       If designated_priority.r_root_id is same as bridge_identifier, update r_root_id.priority.
       If designated_priority.designated_bridge_id is same as bridge_identifier, update designated_bridge_id.priority.
    */
    for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
    {
        pom_ptr     = &(om_ptr->port_info[index]);
        if(memcmp( (UI8_T *)&(pom_ptr->port_priority.r_root_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->port_priority.r_root_id,(UI16_T)priority);
        }
        if(memcmp( (UI8_T *)&(pom_ptr->port_priority.designated_bridge_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->port_priority.designated_bridge_id,(UI16_T)priority);
        }
        if(memcmp( (UI8_T *)&(pom_ptr->designated_priority.r_root_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->designated_priority.r_root_id,(UI16_T)priority);
        }
        if(memcmp( (UI8_T *)&(pom_ptr->designated_priority.designated_bridge_id.addr),
                   (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr),
                   6
                 )==0
          )
        {
            XSTP_OM_SET_BRIDGE_ID_PRIORITY(pom_ptr->designated_priority.designated_bridge_id,(UI16_T)priority);
        }
    }

}/* End of XSTP_OM_MakeOmPriorityConsistencyMsti */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmPortPriorityConsistency
 * ------------------------------------------------------------------------
 * PURPOSE  : Let priority recorded in OM consist with port_priority
 *            set by user
 * INPUT    : om_ptr            -- om pointer for this instance
 *            lport             -- lport value
 *            port_priority     -- port_priority value
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmPortPriorityConsistency(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T port_priority)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    I32_T                   cmp_a, cmp_b;

    pom_ptr = &(om_ptr->port_info[lport-1]);
    /* Kelly_Chen, 07/18/02, for per-port variable: port_priority and designated_priority,
       If designated_priority.bridge_port_id is same as port_id, update bridge_port_id.data.priority.
    */
    XSTP_OM_CMP_PORT_ID(cmp_a, (pom_ptr->designated_priority.designated_port_id), (pom_ptr->port_id));
    XSTP_OM_CMP_BRIDGE_ID(cmp_b, (pom_ptr->designated_priority.designated_bridge_id), (om_ptr->bridge_info.bridge_identifier));
    if(     (cmp_a == 0)
        &&  (cmp_b == 0)
      )
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(pom_ptr->designated_priority.designated_port_id, (UI8_T)port_priority);
    }

    /* If designated_priority.bridge_port_id is same as port_id, update bridge_port_id.data.priority. */
    XSTP_OM_CMP_PORT_ID(cmp_a, (pom_ptr->designated_priority.rcv_port_id), (pom_ptr->port_id));
    if(cmp_a == 0)
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(pom_ptr->designated_priority.rcv_port_id, (UI8_T)port_priority);
    }

    /* If port_priority.designated_port_id is same as port_id, update designated_port_id.data.priority. */
    XSTP_OM_CMP_PORT_ID(cmp_a, (pom_ptr->port_priority.designated_port_id), (pom_ptr->port_id));
    XSTP_OM_CMP_BRIDGE_ID(cmp_b, (pom_ptr->port_priority.designated_bridge_id), (om_ptr->bridge_info.bridge_identifier));
    if(     (cmp_a == 0)
        &&  (cmp_b == 0)
      )
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(pom_ptr->port_priority.designated_port_id, (UI8_T)port_priority);
    }

    /* Kelly_Chen, 07/18/02, for bridge variable: bridge_priority and root_priority
       If bridge_priority.designated_port_id is same as port_id, update designated_port_id.data.priority.
     */
    XSTP_OM_CMP_PORT_ID(cmp_a, (om_ptr->bridge_info.bridge_priority.designated_port_id), (pom_ptr->port_id));
    if(cmp_a == 0)
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(om_ptr->bridge_info.bridge_priority.designated_port_id, (UI8_T)port_priority);
    }

    /* If bridge_priority.bridge_port_id is same as port_id, update bridge_port_id.data.priority. */
    XSTP_OM_CMP_PORT_ID(cmp_a, (om_ptr->bridge_info.bridge_priority.rcv_port_id), (pom_ptr->port_id));
    if(cmp_a == 0)
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(om_ptr->bridge_info.bridge_priority.rcv_port_id, (UI8_T)port_priority);
    }

    /* If root_priority.designated_port_id is same as port_id, update designated_port_id.data.priority. */
    XSTP_OM_CMP_PORT_ID(cmp_a, (om_ptr->bridge_info.root_priority.designated_port_id), (pom_ptr->port_id));
    XSTP_OM_CMP_BRIDGE_ID(cmp_b, (om_ptr->bridge_info.root_priority.designated_bridge_id), (om_ptr->bridge_info.bridge_identifier));
    if(     (cmp_a == 0)
        &&  (cmp_b == 0)
      )
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(om_ptr->bridge_info.root_priority.designated_port_id, (UI8_T)port_priority);
    }

    /* If root_priority.bridge_port_id is same as port_id, update bridge_port_id.data.priority. */
    XSTP_OM_CMP_PORT_ID(cmp_a, (om_ptr->bridge_info.root_priority.rcv_port_id), (pom_ptr->port_id));
    if(cmp_a == 0)
    {
        XSTP_OM_SET_PORT_ID_PRIORITY(om_ptr->bridge_info.root_priority.rcv_port_id, (UI8_T)port_priority);
    }

    XSTP_OM_SET_PORT_ID_PRIORITY(pom_ptr->port_id, (UI8_T)port_priority);
    XSTP_OM_SET_PORT_ID_PRIORITY(pom_ptr->port_priority.rcv_port_id, (UI8_T)port_priority);

}/* End of XSTP_OM_MakeOmPortPriorityConsistency */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_FromSameRegion
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if rcvd_rstp is TRUE, and the received BPDU
 *            conveys an MST Configuration Identifier that matches that
 *            held for the Bridge. Return FALSE otherwise.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : TRUE/FALSE
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.5, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_FromSameRegion(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_Bpdu_T        *bpdu;
    XSTP_TYPE_BpduHeader_T  *bpdu_header;
    int                     cmp_result;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu_header = &(pom_ptr->common->bpdu->bpdu_header);
    bpdu        = (XSTP_TYPE_Bpdu_T*)pom_ptr->common->bpdu;

    if (    (bpdu_header->protocol_version_identifier == XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID)
         && (XSTP_OM_SystemInfo.force_version == XSTP_TYPE_MSTP_MODE)
       )
    {
        /* 1. Compare config_id_format_selector */
        (cmp_result) = (((bpdu->mst_bpdu.mst_configuration_identifier).config_id_format_selector) - ((om_ptr->bridge_info.common->mst_config_id).config_id_format_selector));
        if ((cmp_result) == 0)
        {
            /* 2. Compare config_name */
            (cmp_result) = memcmp((bpdu->mst_bpdu.mst_configuration_identifier).config_name,
                                  (om_ptr->bridge_info.common->mst_config_id).config_name,
                                  XSTP_TYPE_REGION_NAME_MAX_LENGTH);
            if ((cmp_result) == 0)
            {
                /* 3. Compare revision_level */
                (cmp_result) = (L_STDLIB_Ntoh16((bpdu->mst_bpdu.mst_configuration_identifier).revision_level) - ((om_ptr->bridge_info.common->mst_config_id).revision_level));
                if ((cmp_result) == 0)
                {
                    /* 4. Compare config_digest */
                    (cmp_result) = memcmp((bpdu->mst_bpdu.mst_configuration_identifier).config_digest,
                                          (om_ptr->bridge_info.common->mst_config_id).config_digest,
                                          16);
                }
            }
        }

        if (    ( pom_ptr->common->rcvd_rstp)
            &&  ( cmp_result == 0 )
           )
        {
            return TRUE;
        }
    }
    return FALSE;

} /* End of XSTP_OM_FromSameRegion */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GenerateConfigurationDigest
 * ------------------------------------------------------------------------
 * PURPOSE  : generate configuration digest
 * INPUT    : om_ptr    -- om pointer for CIST
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.7, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_GenerateConfigurationDigest(XSTP_OM_InstanceData_T *om_ptr)
{
    UI8_T                   digest[16];

    L_MD5_HMAC_MD5(XSTP_OM_Mst_Configuration_Table, XSTP_OM_Mst_Configuration_Table_Size, XSTP_OM_Configuration_Digest_Signature_Key, 16, digest);

    memcpy(om_ptr->bridge_info.common->mst_config_id.config_digest, digest, 16 );

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        BACKDOOR_MGR_Printf("\r\nXSTP_OM_GenerateConfigurationDigest:: ");
        BACKDOOR_MGR_Printf("\r\nXSTP_OM_Configuration_Digest_Signature_Key[0X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X]",
                    XSTP_OM_Configuration_Digest_Signature_Key[0], XSTP_OM_Configuration_Digest_Signature_Key[1], XSTP_OM_Configuration_Digest_Signature_Key[2],
                    XSTP_OM_Configuration_Digest_Signature_Key[3],XSTP_OM_Configuration_Digest_Signature_Key[4], XSTP_OM_Configuration_Digest_Signature_Key[5],
                    XSTP_OM_Configuration_Digest_Signature_Key[6], XSTP_OM_Configuration_Digest_Signature_Key[7], XSTP_OM_Configuration_Digest_Signature_Key[8],
                    XSTP_OM_Configuration_Digest_Signature_Key[9], XSTP_OM_Configuration_Digest_Signature_Key[10], XSTP_OM_Configuration_Digest_Signature_Key[11],
                    XSTP_OM_Configuration_Digest_Signature_Key[12], XSTP_OM_Configuration_Digest_Signature_Key[13], XSTP_OM_Configuration_Digest_Signature_Key[14],
                    XSTP_OM_Configuration_Digest_Signature_Key[15]);

        BACKDOOR_MGR_Printf("\r\nDigest[0X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X]",
                    digest[0], digest[1], digest[2], digest[3],digest[4], digest[5], digest[6], digest[7],
                    digest[8], digest[9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15]);

    }
    return;
}/* End of XSTP_OM_GenerateConfigurationDigest */

#endif /* XSTP_TYPE_PROTOCOL_MSTP */



/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_CpuMacToString
 * ------------------------------------------------------------------------
 * PURPOSE  : Convert CPU MAC to a string
 * INPUT    : None
 * OUTPUT   : str   -- pointer of the config_name
 * RETUEN   : TRUE/FALSE
 * NOTES    : config_name is guaranteed to be a string ending with a NULL character.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_CpuMacToString(char *str)
{
    UI8_T                   mac_addr[6];

    if (SWCTRL_GetCpuMac(mac_addr)==TRUE)
    {
        sprintf(str, "%02X %02X %02X %02X %02X %02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        return TRUE;
    }
    else
    {
        strncpy(str, XSTP_TYPE_REGION_NAME, XSTP_TYPE_REGION_NAME_MAX_LENGTH);
        return FALSE;
    }
} /* End of XSTP_OM_CpuMacToString */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_InitMstConfigId
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize Mst region configuration
 * INPUT    : om_ptr        -- om pointer for CIST
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_InitMstConfigId(XSTP_OM_InstanceData_T *om_ptr)
{
    /* XSTP_OM_SystemData_T*/
    memset(XSTP_OM_SystemInfo.region_name, 0, XSTP_TYPE_REGION_NAME_MAX_LENGTH);
    XSTP_OM_SystemInfo.region_revision          = XSTP_TYPE_DEFAULT_CONFIG_REVISION;

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    /* XSTP_OM_MstBridgeCommonVar_T, mst_config_id */
    /* 13.7, 13.23.8 */
    om_ptr->bridge_info.common->mst_config_id.config_id_format_selector = XSTP_TYPE_DEFAULT_CONFIG_ID_FORMAT_SELECTOR;
    memset(om_ptr->bridge_info.common->mst_config_id.config_name, 0, XSTP_TYPE_REGION_NAME_MAX_LENGTH);
    om_ptr->bridge_info.common->mst_config_id.revision_level            = XSTP_TYPE_DEFAULT_CONFIG_REVISION;
    XSTP_OM_GenerateConfigurationDigest(om_ptr);
    /* init. restart_state_machine flag */
    om_ptr->bridge_info.common->restart_state_machine                    = FALSE;
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */
} /* End of XSTP_OM_InitMstConfigId */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstidFromMstConfigurationTableByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mstid value form mst configuration table for a specified
 *            vlan.
 * INPUT    : vid       -- vlan number
 *            mstid     -- mstid value point
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_GetMstidFromMstConfigurationTableByVlan(UI32_T vid, UI32_T *mstid)
{
    XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local(vid, mstid);
    return;
} /* End of XSTP_OM_GetMstidFromMstConfigurationTableByVlan */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetMstidToMstConfigurationTableByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Set mstid value to mst configuration table for a specified
 *            vlan.
 * INPUT    : vid   -- vlan number
 *            mstid -- mstid value
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_SetMstidToMstConfigurationTableByVlan(UI32_T vid, UI32_T mstid)
{
    UI8_T   odd_byte, even_byte;

    if (    (vid > 0)
        &&  (vid <= SYS_DFLT_DOT1QMAXVLANID )
       )
    {
        even_byte   =   (UI8_T)((mstid >> 8) & 0xFF);
        odd_byte    =   (UI8_T)(mstid & 0xFF);

        XSTP_OM_Mst_Configuration_Table[2*vid]   = even_byte;
        XSTP_OM_Mst_Configuration_Table[2*vid+1] = odd_byte;
    }
    return;
} /* End of XSTP_OM_SetMstidToMstConfigurationTableByVlan */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetVlanToMstidMappingTable
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the specified vlan id into the Mstid mapping table
 *            vlan.
 * INPUT    : om_ptr    -- om pointer
 *            vlan_id   -- vlan number
 *            state     -- TRUE to set the corresponding bit, or FALSE to reset
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    XSTP_OM_SetVlanToMstidMappingTable( XSTP_OM_InstanceData_T *om_ptr,
                                            UI32_T vlan_id,
                                            BOOL_T state)
{
    UI8_T   value;

    if (    (vlan_id > 0)
        &&  (vlan_id <= SYS_DFLT_DOT1QMAXVLANID )
       )
    {
        value   = om_ptr->instance_vlans_mapped[vlan_id >> 3];
        if (state)
        {
            value   |= (0x01 << (vlan_id % 8));
        }
        else
        {
            value   &= ~(0x01 << (vlan_id % 8));
        }

        om_ptr->instance_vlans_mapped[vlan_id >> 3]  = value;
    }

    return;
} /* End of XSTP_OM_SetVlanToMstidMappingTable */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextXstpMemberFromMstConfigurationTable
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next XSTP member form mst config table for a
 *            specified instance
 * INPUT    : mstid     -- this instance value
 *            vid       -- vlan id pointer
 * OUTPUT   : vid       -- next vlan id pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextXstpMemberFromMstConfigurationTable(UI32_T mstid, UI32_T *vid)
{
    UI32_T  vlan_mstid;
    UI32_T  vlan_index;

    vlan_index = (*vid)+1;
    while (vlan_index <= XSTP_TYPE_SYS_MAX_VLAN_ID)
    {
        XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local(vlan_index, &vlan_mstid);
        if (vlan_mstid == mstid)
        {
            *vid = vlan_index;
            return TRUE;
        }
        vlan_index++;
    }
    return FALSE;
} /* End of XSTP_OM_GetNextXstpMemberFromMstConfigurationTable */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_AddExistingMemberToInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Add all existing members to specified instance.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_AddExistingMemberToInstance(XSTP_OM_InstanceData_T *om_ptr)
{
    UI32_T              lport;
    XSTP_OM_PortVar_T   *pom_ptr;

    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr             = &(om_ptr->port_info[lport-1]);
        pom_ptr->is_member  = TRUE;
        pom_ptr->parent_index = 0;
    }
    return;
} /* End of XSTP_OM_AddExistingMemberToInstance */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetEntryOfMstid
 * ------------------------------------------------------------------------
 * PURPOSE  : Set/clear the entry of the specified mstid to/from the OM
 * INPUT    : mstid         -- the mstid of the entry to be set/clear
 *            set_instance  -- TRUE to set, or FALSE to clear
 * OUTPUT   : None
 * RETURN   : TRUE if ok, else FALSE if the number of the instance exceeds
 *            the maximum.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_SetEntryOfMstid(UI32_T mstid, BOOL_T set_instance)
{
    UI32_T  instance_index, null_index;

    instance_index  = XSTP_TYPE_MAX_INSTANCE_NUM;
    null_index      = XSTP_TYPE_MAX_INSTANCE_NUM;
    if (mstid)
    {
        /* Record the index of the entry storing the instance with the specified mstid. */
        instance_index  = XSTP_OM_InstanceEntryIndex[mstid];

        /* Pre-allocated an unused entry */
        /* Record the index of the entry which is empty. */
        null_index  = XSTP_OM_InstanceInfo[XSTP_TYPE_MAX_INSTANCE_NUM].next;

        /* Operation */
        if (set_instance)
        {
            if (    (instance_index == XSTP_TYPE_MAX_INSTANCE_NUM)
                &&  (null_index != XSTP_TYPE_MAX_INSTANCE_NUM)
               )
            {
                /* Allocate a entry from the free list */
                XSTP_OM_InstanceInfo[XSTP_TYPE_MAX_INSTANCE_NUM].next   = XSTP_OM_InstanceInfo[null_index].next;
                /* Init the allocated entry for the specified MSTI. */
                XSTP_OM_SystemInfo.num_of_cfg_msti++;
                XSTP_OM_InstanceInfo[null_index].instance_id    = mstid;
                XSTP_OM_InstanceEntryIndex[mstid]               = (UI8_T)null_index;
                instance_index                                  = null_index;
                XSTP_OM_InsertMstiEntryList(instance_index);

                #ifdef  XSTP_TYPE_PROTOCOL_MSTP
                /* Patch for 13.32 */
                XSTP_OM_InstanceInfo[null_index].bridge_info.cist_role_updated   = FALSE;
                #endif /* XSTP_TYPE_PROTOCOL_MSTP */

                XSTP_OM_InstanceInfo[null_index].delay_flag     = FALSE;

                XSTP_OM_InitStaticFlag(&(XSTP_OM_InstanceInfo[null_index]));
                XSTP_OM_NullifyInstance(&(XSTP_OM_InstanceInfo[null_index]));
            }
        }
        else
        {
            if (    (instance_index != 0)
                &&  (instance_index<XSTP_TYPE_MAX_INSTANCE_NUM)
               )
            {
                XSTP_OM_SystemInfo.num_of_cfg_msti--;
                XSTP_OM_InstanceInfo[instance_index].instance_id    = XSTP_TYPE_INEXISTENT_MSTID;
                XSTP_OM_InstanceEntryIndex[mstid]                   = XSTP_TYPE_MAX_INSTANCE_NUM;
                XSTP_OM_DeleteMstiEntryList(instance_index);
            }
        }
    }
    return  ( (instance_index != 0)&&(instance_index<XSTP_TYPE_MAX_INSTANCE_NUM));
} /* End of XSTP_OM_SetEntryOfMstid */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_InitEdgePortAndLinkType
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the Port OM about edge_port (oper_edge, admin_edge)
 *            and link_type (oper_point_to_point_mac, admin_point_to_point_mac,
 *            admin_point_to_point_mac_auto, and )
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void XSTP_OM_InitEdgePortAndLinkType(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              index;

    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    {
        pom_ptr             = &(om_ptr->port_info[index]);
        pom_ptr->common->auto_edge                      = (XSTP_TYPE_DEFAULT_PORT_ADMIN_EDGE_PORT == VAL_staPortAdminEdgePortWithAuto_auto) ? TRUE : FALSE;
        pom_ptr->common->admin_edge                     = (XSTP_TYPE_DEFAULT_PORT_ADMIN_EDGE_PORT == VAL_staPortAdminEdgePortWithAuto_true) ? TRUE : FALSE;
        pom_ptr->common->oper_edge                      = pom_ptr->common->admin_edge;
        pom_ptr->common->admin_point_to_point_mac       = FALSE;
        pom_ptr->common->admin_point_to_point_mac_auto  = TRUE;
        pom_ptr->common->oper_point_to_point_mac        = TRUE;
    }
    return;
} /* End of XSTP_OM_InitEdgePortAndLinkType */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_InitStaticFlag
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize static configuration flag
 * INPUT    : om_ptr        -- om pointer
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void XSTP_OM_InitStaticFlag(XSTP_OM_InstanceData_T *om_ptr)
{
    UI32_T              index;
    XSTP_OM_PortVar_T   *pom_ptr;

    om_ptr->bridge_info.static_bridge_priority = FALSE;
    om_ptr->bridge_info.admin_bridge_priority  = SYS_DFLT_STP_BRIDGE_PRIORITY;

    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    {
        pom_ptr = &(om_ptr->port_info[index]);

        pom_ptr->static_port_priority   = FALSE;

        #ifdef  XSTP_TYPE_PROTOCOL_RSTP
        pom_ptr->static_path_cost       = FALSE;
        #endif /* XSTP_TYPE_PROTOCOL_RSTP */

        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        if (om_ptr->instance_id == XSTP_TYPE_CISTID)
        {
            pom_ptr->common->static_external_path_cost = FALSE;
        }
        pom_ptr->static_internal_path_cost = FALSE;
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */
    }
} /* End of XSTP_OM_InitStaticFlag */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_InitMstiEntryList
 * ------------------------------------------------------------------------
 * PURPOSE  : Init the MSTI entry list
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static  void    XSTP_OM_InitMstiEntryList(void)
{
    UI32_T  index;

    /* Init the head for CIST */
    XSTP_OM_InstanceInfo[0].instance_id = 0;
    XSTP_OM_InstanceInfo[0].next        = XSTP_TYPE_MAX_INSTANCE_NUM;

    /* Init the tail for free entry collection */
    XSTP_OM_InstanceInfo[XSTP_TYPE_MAX_INSTANCE_NUM].instance_id    = XSTP_TYPE_INEXISTENT_MSTID;
    XSTP_OM_InstanceInfo[XSTP_TYPE_MAX_INSTANCE_NUM].next           = 1;
    XSTP_OM_InstanceInfo[XSTP_TYPE_MAX_INSTANCE_NUM].instance_exist = FALSE;

    /* Init the unused entries */
    for (index = 1; index < XSTP_TYPE_MAX_INSTANCE_NUM; index++)
    {
        XSTP_OM_InstanceInfo[index].instance_id = XSTP_TYPE_INEXISTENT_MSTID;
        XSTP_OM_InstanceInfo[index].next        = index+1;
    }

    /* Init the mapping of the MSTI to entry index */
    XSTP_OM_InstanceEntryIndex[0]   = 0;
    for (index = 1; index < (XSTP_OM_ENTRY_INDEX_ARRAY_SIZE); index++)
    {
        XSTP_OM_InstanceEntryIndex[index]   = XSTP_TYPE_MAX_INSTANCE_NUM;
    }

    return;
} /* End of XSTP_OM_InitMstiEntryList */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_InsertMstiEntryList
 * ------------------------------------------------------------------------
 * PURPOSE  : Insert the MSTI entry
 * INPUT    : insert_index  -- the index of the entry to be inserted
 * OUTPUT   : None
 * RETUEN   : TRUE if the specified entry is inserted into the list,
 *            else FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static  BOOL_T  XSTP_OM_InsertMstiEntryList(UI32_T insert_index)
{
    UI32_T  index   = 0;
    UI32_T  xstid   = XSTP_OM_InstanceInfo[insert_index].instance_id;

    while (XSTP_OM_InstanceInfo[XSTP_OM_InstanceInfo[index].next].instance_id <= xstid)
    {
        index   = XSTP_OM_InstanceInfo[index].next;
    }
    if (XSTP_OM_InstanceInfo[index].instance_id < xstid)
    {
        /* Insert into the list */
        XSTP_OM_InstanceInfo[insert_index].next = XSTP_OM_InstanceInfo[index].next;
        XSTP_OM_InstanceInfo[index].next        = insert_index;
    }
    return TRUE;
} /* End of XSTP_OM_InsertMstiEntryList */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_DeleteMstiEntryList
 * ------------------------------------------------------------------------
 * PURPOSE  : Delete the MSTI entry and return it into the free list
 * INPUT    : delete_index  -- the index of the entry to be deleted
 * OUTPUT   : None
 * RETUEN   : TRUE if the specified entry is deleted from the list,
 *            else FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static  BOOL_T  XSTP_OM_DeleteMstiEntryList(UI32_T delete_index)
{
    UI32_T  index       = 0;
    UI32_T  prev_index  = 0;
    BOOL_T  result      = FALSE;

    while (     (XSTP_OM_GetNextMstiEntryIndex(&index))
            &&  (index != delete_index)
          )
    {
        prev_index  = index;
    }
    if (index == XSTP_TYPE_MAX_INSTANCE_NUM)
    {
        result  = FALSE;
    }
    else
    {
        /* Delete from the list */
        XSTP_OM_InstanceInfo[prev_index].next   = XSTP_OM_InstanceInfo[delete_index].next;

        /* Return the free entry to the list */
        XSTP_OM_InstanceInfo[delete_index].next = XSTP_OM_InstanceInfo[XSTP_TYPE_MAX_INSTANCE_NUM].next;
        XSTP_OM_InstanceInfo[XSTP_TYPE_MAX_INSTANCE_NUM].next   = delete_index;
    }

    return result;
} /* End of XSTP_OM_DeleteMstiEntryList */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextMstiEntryIndex
 * ------------------------------------------------------------------------
 * PURPOSE  : Delete the MSTI entry
 * INPUT    : delete_index  -- the index of the entry to be deleted
 * OUTPUT   : None
 * RETUEN   : TRUE if the specified entry is deleted from the list,
 *            else FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static  BOOL_T  XSTP_OM_GetNextMstiEntryIndex(UI32_T *index)
{
    *index  = XSTP_OM_InstanceInfo[*index].next;
    return ((*index) != XSTP_TYPE_MAX_INSTANCE_NUM);
} /* End of XSTP_OM_GetNextMstiEntryIndex */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMstFullMemberTopology
 * ------------------------------------------------------------------------
 * PURPOSE  : This function returns TRUE if mst_topology_method is vlan egree topology.
 *            Otherwise, return FALSE.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 *-------------------------------------------------------------------------
 */
BOOL_T   XSTP_OM_IsMstFullMemberTopology(void)
{
    return (XSTP_OM_SystemInfo.mst_topology_method == SYS_CPNT_MST_FULL_MEMBER_TOPOLOGY);
} /* End of XSTP_OM_IsMstFullMemberTopology */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetCurrentCfgInstanceNumber
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the current instance number created by user.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : num_of_cfg_msti
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetCurrentCfgInstanceNumber(void)
{
    return (XSTP_OM_SystemInfo.num_of_cfg_msti + 1); /* CIST + MSTIs */
} /* End of XSTP_OM_GetCurrentCfgInstanceNumber */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetInstanceEntryId
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the entry_id for the specified xstid
 * INPUT    : xstid -- MST instance ID
 * OUTPUT   : None
 * RETUEN   : entry_id (stg_id) for the specified xstid
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI8_T   XSTP_OM_GetInstanceEntryId(UI32_T xstid)
{
    return  (XSTP_OM_InstanceEntryIndex[xstid]);
} /* End of XSTP_OM_GetInstanceEntryId */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetInstanceEntryPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the om_ptr for the specified mstidx
 * INPUT    : mstidx -- MST instance entry index
 * OUTPUT   : None
 * RETUEN   : Pointer to the om_ptr for the specified mstidx
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
XSTP_OM_InstanceData_T*   XSTP_OM_GetInstanceEntryPtr(UI32_T mstidx)
{
    return  (&(XSTP_OM_InstanceInfo[mstidx]));
} /* End of XSTP_OM_GetInstanceEntryPtr */


/* ===================================================================== */
/* ===================================================================== */
/* ========                     Utilities                         =======*/
/* ===================================================================== */
/* ===================================================================== */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - XSTP_OM_LongHexStrToVal
 * ------------------------------------------------------------------------
 * PURPOSE  : Convert a string to a value
 * INPUT    : hex_str   -- hexadecimal string with a leading "0x" and a
 *                         ending NULL
 *                         the leading characters should be 0x, otherwise
 *                         output value is 0 and FALSE returned
 *            buf_size  -- size of output buffer
 * OUTPUT   : buf       -- output buffer
 * RETURN   : TRUE if convert successful, else FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_LongHexStrToVal(char *hex_str, UI8_T *buf, UI8_T buf_size)
{
    BOOL_T  result = FALSE;
    BOOL_T  continue_parsing;
    UI8_T   value_length, string_length;
    UI8_T   str_index, val_index, ch, value;

    memset(buf, 0x00, buf_size);
    string_length   = strlen(hex_str) - 2;
    value_length    = (string_length + 1) / 2;
    if (    (hex_str[0] == '0')
        &&  (hex_str[1] == 'x' || hex_str[1] == 'X')
        &&  (value_length <= buf_size)
       )
    {
        continue_parsing    = TRUE;
        str_index           = 0;
        val_index           = 0;
        while ( (ch = hex_str[str_index+2]) && continue_parsing )
        {
            if ( (ch >= '0') && (ch <= '9') )
            {
                value   = ch - '0';
            }
            else if ( (ch >= 'a') && (ch <= 'f') )
            {
                value   = ch - 'a' + 10;
            }
            else if ( (ch >= 'A') && (ch <= 'F') )
            {
                value   = ch - 'A' + 10;
            }
            else
            {
                value   = 0;
                continue_parsing    = FALSE;
            }

            if ((string_length - str_index) % 2)
            {
                /* low  half-byte */
                buf[val_index]      |= (value & 0x0F);
                val_index++;
            }
            else
            {
                /* high half-byte */
                buf[val_index]      = value << 4;
            }
            str_index++;
        }
        result  = continue_parsing;
    }

    return result;
} /* End of XSTP_OM_LongHexStrToVal */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for XSTP OM.
 *
 * INPUT   : msgbuf_p - input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p - output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : 1. The size of msgbuf_p->msg_buf must be large enough to carry
 *              any response messages.
 *-----------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    XSTP_OM_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (XSTP_OM_IpcMsg_T*)msgbuf_p->msg_buf;

    /* dispatch IPC message and call the corresponding XSTP_OM function
     */
    switch (msg_p->type.cmd)
    {
        case XSTP_OM_IPC_GETFORCEVERSION:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui8 = XSTP_OM_GetForceVersion();
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETMAXHOPCOUNT:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetMaxHopCount();
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETMAXINSTANCENUMBER:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetMaxInstanceNumber();
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETNUMOFACTIVETREE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetNumOfActiveTree();
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETREGIONNAME:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            XSTP_OM_GetRegionName(msg_p->data.arg_ar1);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ar1);
            break;

        case XSTP_OM_IPC_GETREGIONREVISION:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRegionRevision();
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETSPANNINGTREESTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetSpanningTreeStatus();
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETTRAPFLAGTC:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetTrapFlagTc();
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETTRAPFLAGNEWROOT:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetTrapFlagNewRoot();
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETMSTIDFROMMSTCONFIGURATIONTABLEBYVLAN:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            XSTP_OM_GetMstidFromMstConfigurationTableByVlan(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETNEXTXSTPMEMBERFROMMSTCONFIGURATIONTABLE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextXstpMemberFromMstConfigurationTable(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_ISMSTFULLMEMBERTOPOLOGY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_IsMstFullMemberTopology();
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETCURRENTCFGINSTANCENUMBER:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetCurrentCfgInstanceNumber();
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETINSTANCEENTRYID:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui8 =
                XSTP_OM_GetInstanceEntryId(msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETPORTSTATEBYVLAN:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetPortStateByVlan(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETPORTSTATEBYINSTANCE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetPortStateByInstance(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_ISPORTFORWARDINGSTATEBYVLAN:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_IsPortForwardingStateByVlan(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_ISPORTFORWARDINGSTATEBYINSTANCE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_IsPortForwardingStateByInstance(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_ISPORTBLOCKINGSTATEBYVLAN:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_IsPortBlockingStateByVlan(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_ISPORTBLOCKINGSTATEBYINSTANCE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_IsPortBlockingStateByInstance(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETNEXTEXISTINGMSTIDBYLPORT:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextExistingMstidByLport(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETNEXTEXISTINGMEMBERVIDBYMSTID:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextExistingMemberVidByMstid(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETMSTINSTANCEINDEXBYMSTID:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetMstInstanceIndexByMstid(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETRUNNINGSYSTEMSPANNINGTREESTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 =
                XSTP_OM_GetRunningSystemSpanningTreeStatus(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETSYSTEMSPANNINGTREESTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetSystemSpanningTreeStatus(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETRUNNINGSYSTEMSPANNINGTREEVERSION:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 =
                XSTP_OM_GetRunningSystemSpanningTreeVersion(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETSYSTEMSPANNINGTREEVERSION:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetSystemSpanningTreeVersion(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETRUNNINGFORWARDDELAY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 =
                XSTP_OM_GetRunningForwardDelay(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETFORWARDDELAY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetForwardDelay(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETRUNNINGHELLOTIME:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 =
                XSTP_OM_GetRunningHelloTime(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETHELLOTIME:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetHelloTime(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETRUNNINGMAXAGE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 =
                XSTP_OM_GetRunningMaxAge(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETMAXAGE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetMaxAge(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETRUNNINGPATHCOSTMETHOD:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 =
                XSTP_OM_GetRunningPathCostMethod(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETPATHCOSTMETHOD_EX:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetPathCostMethod_Ex(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETRUNNINGTRANSMISSIONLIMIT:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 =
                XSTP_OM_GetRunningTransmissionLimit(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETTRANSMISSIONLIMIT:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetTransmissionLimit(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETRUNNINGMSTPRIORITY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningMstPriority(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETMSTPRIORITY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetMstPriority(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETRUNNINGMSTPORTPRIORITY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningMstPortPriority(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETMSTPORTPRIORITY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetMstPortPriority(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETRUNNINGPORTLINKTYPEMODE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningPortLinkTypeMode(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETPORTLINKTYPEMODE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetPortLinkTypeMode(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETRUNNINGPORTPROTOCOLMIGRATION:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningPortProtocolMigration(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETPORTPROTOCOLMIGRATION:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetPortProtocolMigration(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETRUNNINGPORTADMINEDGEPORT:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningPortAdminEdgePort(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETPORTADMINEDGEPORT:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetPortAdminEdgePort(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETDOT1DMSTPORTENTRY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetDot1dMstPortEntry(
                msg_p->data.arg_grp3.arg1, &msg_p->data.arg_grp3.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp3);
            break;

        case XSTP_OM_IPC_GETNEXTDOT1DMSTPORTENTRY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextDot1dMstPortEntry(
                &msg_p->data.arg_grp3.arg1, &msg_p->data.arg_grp3.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp3);
            break;

        case XSTP_OM_IPC_GETNEXTPORTMEMBERBYINSTANCE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextPortMemberByInstance(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETDOT1DMSTPORTENTRYX:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetDot1dMstPortEntryX(
                msg_p->data.arg_grp3.arg1, &msg_p->data.arg_grp3.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp3);
            break;

        case XSTP_OM_IPC_GETNEXTDOT1DMSTPORTENTRYX:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextDot1dMstPortEntryX(
                &msg_p->data.arg_grp3.arg1, &msg_p->data.arg_grp3.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp3);
            break;

        case XSTP_OM_IPC_GETDOT1DMSTEXTPORTENTRY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetDot1dMstExtPortEntry(
                msg_p->data.arg_grp4.arg1, msg_p->data.arg_grp4.arg2,
                &msg_p->data.arg_grp4.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp4);
            break;

        case XSTP_OM_IPC_GETNEXTDOT1DMSTEXTPORTENTRY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextDot1dMstExtPortEntry(
                msg_p->data.arg_grp4.arg1, &msg_p->data.arg_grp4.arg2,
                &msg_p->data.arg_grp4.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp4);
            break;

        case XSTP_OM_IPC_GETRUNNINGMSTPREVISIONLEVEL:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 =
                XSTP_OM_GetRunningMstpRevisionLevel(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETMSTPREVISIONLEVEL:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetMstpRevisionLevel(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETRUNNINGMSTPMAXHOP:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 =
                XSTP_OM_GetRunningMstpMaxHop(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETMSTPMAXHOP:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetMstpMaxHop(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETMSTPCONFIGURATIONENTRY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetMstpConfigurationEntry(&msg_p->data.arg_mstentry);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_mstentry);
            break;

        case XSTP_OM_IPC_GETMSTPINSTANCEVLANMAPPED:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetMstpInstanceVlanMapped(
                msg_p->data.arg_grp5.arg1, &msg_p->data.arg_grp5.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
            break;

        case XSTP_OM_IPC_GETMSTPINSTANCEVLANMAPPEDFORMSB:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetMstpInstanceVlanMappedForMSB(
                msg_p->data.arg_grp5.arg1, &msg_p->data.arg_grp5.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
            break;

        case XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANMAPPED:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextMstpInstanceVlanMapped(
                &msg_p->data.arg_grp5.arg1, &msg_p->data.arg_grp5.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
            break;

        case XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANMAPPEDFORMSB:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextMstpInstanceVlanMappedForMSB(
                &msg_p->data.arg_grp5.arg1, &msg_p->data.arg_grp5.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
            break;

        case XSTP_OM_IPC_GETMSTPINSTANCEVLANCONFIGURATION:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetMstpInstanceVlanConfiguration(
                msg_p->data.arg_grp5.arg1, &msg_p->data.arg_grp5.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
            break;

        case XSTP_OM_IPC_GETMSTPINSTANCEVLANCONFIGURATIONFORMSB:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetMstpInstanceVlanConfigurationForMSB(
                msg_p->data.arg_grp5.arg1, &msg_p->data.arg_grp5.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
            break;

        case XSTP_OM_IPC_GETRUNNINGMSTPINSTANCEVLANCONFIGURATION:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningMstpInstanceVlanConfiguration(
                msg_p->data.arg_grp5.arg1, &msg_p->data.arg_grp5.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
            break;

        case XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANCONFIGURATION:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextMstpInstanceVlanConfiguration(
                &msg_p->data.arg_grp5.arg1, &msg_p->data.arg_grp5.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
            break;

        case XSTP_OM_IPC_GETNEXTMSTPINSTANCEVLANCONFIGURATIONFORMSB:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextMstpInstanceVlanConfigurationForMSB(
                &msg_p->data.arg_grp5.arg1, &msg_p->data.arg_grp5.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp5);
            break;

        case XSTP_OM_IPC_ISMSTINSTANCEEXISTING:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_IsMstInstanceExisting(msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETNEXTEXISTEDINSTANCE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetNextExistedInstance(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_ISMSTINSTANCEEXISTINGINMSTCONFIGTABLE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_IsMstInstanceExistingInMstConfigTable(msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_OM_IPC_GETNEXTEXISTEDINSTANCEFORMSTCONFIGTABLE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetNextExistedInstanceForMstConfigTable(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETMSTPORTROLE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetMstPortRole(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETMSTPORTSTATE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetMstPortState(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETNEXTVLANMEMBERBYINSTANCE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextVlanMemberByInstance(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETNEXTMSTIDFROMMSTCONFIGURATIONTABLEBYVLAN:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextMstidFromMstConfigurationTableByVlan(
                &msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_ISMEMBERPORTOFINSTANCEEX:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_IsMemberPortOfInstanceEx(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
            break;

#ifdef  XSTP_TYPE_PROTOCOL_MSTP

        case XSTP_OM_IPC_GETCONFIGDIGEST:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool =
                XSTP_OM_GetConfigDigest(msg_p->data.arg_ar2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ar2);
            break;

        case XSTP_OM_IPC_GETMSTPROWSTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetMstpRowStatus(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETNEXTMSTPROWSTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetNextMstpRowStatus(
                &msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

        case XSTP_OM_IPC_GETPORTADMINPATHCOST:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetPortAdminPathCost(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETPORTOPERPATHCOST:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetPortOperPathCost(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETMSTPORTADMINPATHCOST:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetMstPortAdminPathCost(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETMSTPORTOPERPATHCOST:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetMstPortOperPathCost(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_OM_IPC_GETRUNNINGPORTSPANNINGTREESTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningPortSpanningTreeStatus(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETPORTSPANNINGTREESTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetPortSpanningTreeStatus(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETDESIGNATEDROOT:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetDesignatedRoot(
                msg_p->data.arg_grp6.arg1, &msg_p->data.arg_grp6.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp6);
            break;

        case XSTP_OM_IPC_GETBRIDGEIDCOMPONENT:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetBridgeIdComponent(
                msg_p->data.arg_grp6.arg1, &msg_p->data.arg_grp6.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp6);
            break;

        case XSTP_OM_IPC_GETPORTDESIGNATEDROOT:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetPortDesignatedRoot(
                msg_p->data.arg_grp7.arg1, msg_p->data.arg_grp7.arg2,
                &msg_p->data.arg_grp7.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp7);
            break;

        case XSTP_OM_IPC_GETPORTDESIGNATEDBRIDGE:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetPortDesignatedBridge(
                msg_p->data.arg_grp7.arg1, msg_p->data.arg_grp7.arg2,
                &msg_p->data.arg_grp7.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp7);
            break;

        case XSTP_OM_IPC_GETPORTDESIGNATEDPORT:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_bool = XSTP_OM_GetPortDesignatedPort(
                msg_p->data.arg_grp8.arg1, msg_p->data.arg_grp8.arg2,
                &msg_p->data.arg_grp8.arg3);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp8);
            break;

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
        case XSTP_OM_IPC_GETPORTROOTGUARDSTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetPortRootGuardStatus(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETRUNNINGPORTROOTGUARDSTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningPortRootGuardStatus(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
        case XSTP_OM_IPC_GETPORTBPDUGUARDSTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetPortBpduGuardStatus(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETRUNNINGPORTBPDUGUARDSTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningPortBpduGuardStatus(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETPORTBPDUGUARDAUTORECOVERY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetPortBPDUGuardAutoRecovery(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETRUNNINGPORTBPDUGUARDAUTORECOVERY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningPortBPDUGuardAutoRecovery(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETPORTBPDUGUARDAUTORECOVERYINTERVAL:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetPortBPDUGuardAutoRecoveryInterval(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETRUNNINGPORTBPDUGUARDAUTORECOVERYINTERVAL:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningPortBPDUGuardAutoRecoveryInterval(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
        case XSTP_OM_IPC_GETPORTBPDUFILTERSTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetPortBpduFilterStatus(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_OM_IPC_GETRUNNINGPORTBPDUFILTERSTATUS:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningPortBpduFilterStatus(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp1);
            break;
#endif /* #if (SYS_CPNT_STP_BPDU_FILTER == TRUE) */

#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
        case XSTP_OM_IPC_GETCISCOPRESTANDARDCOMPATIBILITY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            XSTP_OM_GetCiscoPrestandardCompatibility(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETRUNNINGCISCOPRESTANDARDCOMPATIBILITY:
            /* +++ EnterCriticalRegion +++ */
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningCiscoPrestandardCompatibility(&msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;
#endif /* #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */
#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
        case XSTP_OM_IPC_GETPORTTCPROPSTOP:
           original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_IsPortTcPropStop(msg_p->data.arg_ui32);
            /* +++ LeaveCriticalRegion +++ */
           SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;

        case XSTP_OM_IPC_GETRUNNINGPORTTCPROPSTOP:
            original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);
            msg_p->type.ret_ui32 = XSTP_OM_GetRunningPortTcPropStop(msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            /* +++ LeaveCriticalRegion +++ */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_ui32);
            break;
#endif
#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
        case XSTP_OM_IPC_GETTCPROPGROUPPORTBITMAP:
            msg_p->type.ret_bool = XSTP_OM_GetTcPropGroupPortbitmap(msg_p->data.arg_grp9.arg1,
                                                         msg_p->data.arg_grp9.arg_ar1,
                                                         &msg_p->data.arg_grp9.arg_bool);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp9);
            break;
        case XSTP_OM_IPC_GETTCPROPNEXTGROUPPORTBITMAP:
            msg_p->type.ret_bool = XSTP_OM_GetTcPropNextGroupPortbitmap(&msg_p->data.arg_grp9.arg1,
                                                         msg_p->data.arg_grp9.arg_ar1,
                                                         &msg_p->data.arg_grp9.arg_bool);
            msgbuf_p->msg_size = XSTP_OM_GET_MSG_SIZE(arg_grp9);
            break;
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_ui32 = XSTP_TYPE_RETURN_ERROR;
            msgbuf_p->msg_size = XSTP_OM_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of XSTP_OM_HandleIPCReqMsg */

/*=============================================================================
 * Move from xstp_svc.c
 *=============================================================================
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   U32_T  *state   -- the pointer of state value
 *                  VAL_dot1dStpPortState_blocking
 *                  VAL_dot1dStpPortState_learning
 *                  VAL_dot1dStpPortState_forwarding
 * RETURN   :   TRUE/FALSE
 * NOTES    :   It is strictly prohibited to invoke the functions other than
 *              those which get information from the OMs for the consideration
 *              of implementing this function.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortStateByVlan(UI32_T vid, UI32_T lport, UI32_T *state)
{
    UI32_T  xstid;
    UI32_T  current_st_status;
    UI32_T  current_st_mode;

    /* get port status for ethernet ring protocol, if it's a ring port
     */
    {
        BOOL_T  is_blk;

        if (TRUE == XSTP_OM_GetEthRingPortStatus(lport, vid, &is_blk))
        {
            if (TRUE == is_blk)
                *state = VAL_dot1dStpPortState_blocking;
            else
                *state = VAL_dot1dStpPortState_forwarding;
            return TRUE;
        }
    }

    current_st_status   = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();
    if (    (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
       )
    {
        XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local(vid, &xstid);
    }
    else
    {
        xstid = XSTP_TYPE_CISTID;
    }
    return  XSTP_OM_GetPortStateByInstance(xstid, lport, state);
} /* End of XSTP_OM_GetPortStateByVlan */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T Xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   U32_T  *state   -- the pointer of state value, as following:
 *                  VAL_dot1dStpPortState_blocking
 *                  VAL_dot1dStpPortState_learning
 *                  VAL_dot1dStpPortState_forwarding
 * RETURN   :   TRUE/FALSE
 * NOTES    :   It is strictly prohibited to invoke the functions other than
 *              those which get information from the OMs for the consideration
 *              of implementing this function.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortStateByInstance(UI32_T xstid, UI32_T lport, UI32_T *state)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    BOOL_T                  result;

    if (xstid > XSTP_TYPE_MAX_MSTID)
    {
        return FALSE;
    }

    /* get port status for ethernet ring protocol, if it's a ring port
     */
    {
        BOOL_T  is_blk;

        if (TRUE == XSTP_OM_GetEthRingPortStatus(lport, 0, &is_blk))
        {
            if (TRUE == is_blk)
                *state = VAL_dot1dStpPortState_blocking;
            else
                *state = VAL_dot1dStpPortState_forwarding;
            return TRUE;
        }
    }

    if (xstid > XSTP_TYPE_MAX_MSTID)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
        {
            printf("\r\nXSTP_OM_GetPortStateByInstance::Error! xstid = %ld", (long)xstid);
        }
        return FALSE;
    }
    if (    (lport == 0)
        ||  (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
       )
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
        {
            printf("\r\nXSTP_OM_GetPortStateByInstance::Error! lport = %ld", (long)lport);
        }
        return FALSE;
    }
    om_ptr  = XSTP_OM_GetInstanceInfoPtr(xstid);
    pom_ptr= &(om_ptr->port_info[lport-1]);
    *state = (UI32_T)VAL_dot1dStpPortState_blocking;

    result  = TRUE;

#if 0
    if (XSTP_OM_GetSpanningTreeStatus() == VAL_staSystemStatus_enabled)
    {
        if (XSTP_OM_IsMemberPortOfInstance(om_ptr, lport)== FALSE)
        {
            return  FALSE;
        }
    }
#endif

    if (pom_ptr->common->port_enabled )
    {
        if (pom_ptr->learning && !pom_ptr->forwarding )
        {
            *state = (UI32_T)VAL_dot1dStpPortState_learning;
        }
        else if (pom_ptr->learning && pom_ptr->forwarding )
        {
            *state = (UI32_T)VAL_dot1dStpPortState_forwarding;
        }
        else if (!pom_ptr->learning && pom_ptr->forwarding)
        {
            result  = FALSE;
        }
    } /* End of if (pom_ptr->common->port_enabled) */

    return  result;
} /* End of XSTP_OM_GetPortStateByInstance */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsPortForwardingStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in Forwarding, else FALSE
 * NOTES    :   It is strictly prohibited to invoke the functions other than
 *              those which get information from the OMs for the consideration
 *              of implementing this function.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsPortForwardingStateByVlan(UI32_T vid, UI32_T lport)
{
    UI32_T  xstid;
    UI32_T  current_st_status;
    UI32_T  current_st_mode;
    BOOL_T  is_blk = FALSE;

    /* get port status for ethernet ring protocol, if it's a ring port
     */
    {
        if (TRUE == XSTP_OM_GetEthRingPortStatus(lport, vid, &is_blk))
        {
            return (FALSE == is_blk);
        }
    }

    current_st_status   = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();
    if (    (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
       )
    {
        XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local(vid, &xstid);
    }
    else
    {
        xstid = XSTP_TYPE_CISTID;
    }

    return  XSTP_OM_IsPortForwardingStateByInstance(xstid, lport);
} /* End of XSTP_OM_IsPortForwardingStateByVlan */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsPortForwardingStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in Forwarding, else FALSE
 * NOTES    :   It is strictly prohibited to invoke the functions other than
 *              those which get information from the OMs for the consideration
 *              of implementing this function.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsPortForwardingStateByInstance(UI32_T xstid, UI32_T lport)
{
    UI32_T  state;
    BOOL_T  result;
    BOOL_T  is_blk = FALSE;

    /* get port status for ethernet ring protocol, if it's a ring port
     */
    {
        if (TRUE == XSTP_OM_GetEthRingPortStatus(lport, 0, &is_blk))
        {
            return (FALSE == is_blk);
        }
    }

    result  = FALSE;
    if (XSTP_OM_GetPortStateByInstance(xstid, lport, &state) )
    {
        result  = (state == VAL_dot1dStpPortState_forwarding);
    }

    return  result;
} /* End of XSTP_OM_IsPortForwardingStateByInstance */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsPortBlockingStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in Blocking, else FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsPortBlockingStateByVlan(UI32_T vid, UI32_T lport)
{
    UI32_T  xstid;
    UI32_T  current_st_status;
    UI32_T  current_st_mode;
    BOOL_T  is_blk;

    /* get port status for ethernet ring protocol, if it's a ring port
     */
    {
        if (TRUE == XSTP_OM_GetEthRingPortStatus(lport, vid, &is_blk))
        {
            return (TRUE == is_blk);
        }
    }

    current_st_status   = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();
    if (    (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
       )
    {
        XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local(vid, &xstid);
    }
    else
    {
        xstid = XSTP_TYPE_CISTID;
    }

    return  XSTP_OM_IsPortBlockingStateByInstance(xstid, lport);
} /* End of XSTP_OM_IsPortBlockingStateByVlan */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsPortBlockingStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in Blocking, else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsPortBlockingStateByInstance(UI32_T xstid, UI32_T lport)
{
    UI32_T  state;
    BOOL_T  result;
    BOOL_T  is_blk;

    /* get port status for ethernet ring protocol, if it's a ring port
     */
    {
        if (TRUE == XSTP_OM_GetEthRingPortStatus(lport, 0, &is_blk))
        {
            return (TRUE == is_blk);
        }
    }

    result  = FALSE;
    if (XSTP_OM_GetPortStateByInstance(xstid, lport, &state) )
    {
        result  = (state == VAL_dot1dStpPortState_blocking);
    }

    return  result;
} /* End of XSTP_OM_IsPortBlockingStateByInstance */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextExistingMstidByLport
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the next mst instance id with a specified lport.
 * INPUT    :   UI32_T lport    -- lport number
 * OUTPUT   :   UI32_T *mstid   -- mst instance id
 * OUTPUT   :   UI32_T *mstid   -- next mst instance id
 * RETURN   :   TRUE if the next mstid is existing, else FALSE
 * NOTES    :   It is strictly prohibited to invoke the functions other than
 *              those which get information from the OMs for the consideration
 *              of implementing this function.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextExistingMstidByLport(UI32_T lport, UI32_T *mstid)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
    BOOL_T                  iterative;

    result      = FALSE;
    iterative   = TRUE;
    if ( (*mstid < 0) || (*mstid >= XSTP_TYPE_MAX_MSTID) )
    {
        *mstid  = 0;
    }
    else
    {
        while(iterative && XSTP_OM_GetNextInstanceInfoPtr(mstid, &om_ptr))
        {
            if (om_ptr->instance_exist)
            {
                if ( XSTP_OM_IsMemberPortOfInstance(om_ptr, lport))
                {
                    result  = TRUE;
                    iterative = FALSE;
                }
            } /* End of if (om_ptr->instance_exist) */
        } /* End of for (_all_remainder_mstid_) */
    }
    return result;
} /* End of XSTP_OM_GetNextExistingMstidByLport */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextExistingMemberVidByMstid
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the next member vlan id with a specified mst instance id.
 * INPUT    :   UI32_T xstid    -- mst instance id
 * OUTPUT   :   UI32_T *vid     -- vlan id
 * OUTPUT   :   UI32_T *vid     -- next member vlan id
 * RETURN   :   TRUE if the next member vlan is existing, else FALSE
 * NOTES    :   It is strictly prohibited to invoke the functions other than
 *              those which get information from the OMs for the consideration
 *              of implementing this function.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextExistingMemberVidByMstid(UI32_T xstid, UI32_T *vid)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  iterative;
    BOOL_T                  result;
    UI32_T                  config_mstid;

    if ( xstid > XSTP_TYPE_MAX_MSTID )
    {
        return FALSE;
    }

    iterative   = TRUE;
    om_ptr      = XSTP_OM_GetInstanceInfoPtr(xstid);
    while (iterative && XSTP_OM_GetNextXstpMember(om_ptr, vid) )
    {
        XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local(*vid, &config_mstid);
        if (config_mstid == xstid)
        {
            iterative   = FALSE;
        }
    }
    if (iterative)
    {
        *vid    = 0;
        result  = FALSE;
    }
    else
    {
        result  = TRUE;
    }

    return  result;
} /* End of XSTP_OM_GetNextExistingMemberVidByMstid */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstInstanceIndexByMstid
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the entry_id for the specified xstid
 * INPUT    : xstid -- MST instance ID
 * OUTPUT   : mstudx -- MST instance INDEX
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T   XSTP_OM_GetMstInstanceIndexByMstid(UI32_T xstid, UI32_T *mstidx)
{
    if ( (xstid < 0) || (xstid > XSTP_TYPE_MAX_MSTID) )
    {
         return FALSE;
    }
    *mstidx = (UI32_T)XSTP_OM_GetInstanceEntryId(xstid);
    return TRUE;
} /* End of XSTP_OM_GetMstInstanceIndexByMstid */


/*=============================================================================
 * Move from xstp_mgr.c
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningSystemSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global spanning tree status.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningSystemSpanningTreeStatus(UI32_T *status)
{
    *status = XSTP_OM_GetSpanningTreeStatus();

    if (*status == XSTP_TYPE_DEFAULT_SPANNING_TREE_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetSystemSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global spanning tree status.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetSystemSpanningTreeStatus(UI32_T *status)
{
    *status = XSTP_OM_GetSpanningTreeStatus();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningSystemSpanningTreeVersion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree mode.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *mode          -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningSystemSpanningTreeVersion(UI32_T *mode)
{
    *mode = XSTP_OM_GetForceVersion();

    if (*mode == SYS_DFLT_STP_PROTOCOL_TYPE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetSystemSpanningTreeVersion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree mode.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *mode          -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetSystemSpanningTreeVersion(UI32_T *mode)
{
    *mode = XSTP_OM_GetForceVersion();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the forward_delay time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *forward_delay -- pointer of the forward_delay value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningForwardDelay(UI32_T *forward_delay)
{
    UI32_T                  result;
    UI32_T                  sec_forward_delay;
    XSTP_OM_InstanceData_T  *om_ptr;

    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    result = XSTP_OM_GetRunningMstForwardDelay(om_ptr, &sec_forward_delay);
    *forward_delay = sec_forward_delay*XSTP_TYPE_TICK_TIME_UNIT;

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the forward_delay time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *forward_delay -- pointer of the forward_delay value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetForwardDelay(UI32_T *forward_delay)
{
    UI32_T                  sec_forward_delay;
    XSTP_OM_InstanceData_T  *om_ptr;

    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    XSTP_OM_GetRunningMstForwardDelay(om_ptr, &sec_forward_delay);
    *forward_delay = sec_forward_delay*XSTP_TYPE_TICK_TIME_UNIT;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the hello_time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *hello_time      -- pointer of the hello_time value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningHelloTime(UI32_T *hello_time)
{
    UI32_T                  result;
    UI32_T                  sec_hello_time;
    XSTP_OM_InstanceData_T  *om_ptr;

    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    result = XSTP_OM_GetRunningMstHelloTime(om_ptr, &sec_hello_time);
    *hello_time = sec_hello_time*XSTP_TYPE_TICK_TIME_UNIT;

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the hello_time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *hello_time      -- pointer of the hello_time value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetHelloTime(UI32_T *hello_time)
{
    UI32_T                  sec_hello_time;
    XSTP_OM_InstanceData_T  *om_ptr;

    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    XSTP_OM_GetRunningMstHelloTime(om_ptr, &sec_hello_time);
    *hello_time = sec_hello_time*XSTP_TYPE_TICK_TIME_UNIT;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the max_age information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *max_age         -- pointer of the max_age value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMaxAge(UI32_T *max_age)
{
    UI32_T                  result;
    UI32_T                  sec_max_age;
    XSTP_OM_InstanceData_T  *om_ptr;

    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    result = XSTP_OM_GetRunningMstMaxAge(om_ptr, &sec_max_age);
    *max_age = sec_max_age * XSTP_TYPE_TICK_TIME_UNIT;

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the max_age information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *max_age         -- pointer of the max_age value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMaxAge(UI32_T *max_age)
{
    UI32_T                  sec_max_age;
    XSTP_OM_InstanceData_T  *om_ptr;

    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    XSTP_OM_GetRunningMstMaxAge(om_ptr, &sec_max_age);
    *max_age = sec_max_age * XSTP_TYPE_TICK_TIME_UNIT;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPathCostMethod
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the default path cost calculation method.
 * INPUT    :   UI32_T  *pathcost_method  -- pointer of the method value
 * OUTPUT   :   None
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPathCostMethod(UI32_T *pathcost_method)
{
    *pathcost_method = (UI32_T)XSTP_OM_GetPathCostMethod();
    if (*pathcost_method == XSTP_TYPE_DEFAULT_PATH_COST_METHOD)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPathCostMethod_Ex
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the default path cost calculation method.
 * INPUT    :   UI32_T  *pathcost_method  -- pointer of the method value
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPathCostMethod_Ex(UI32_T *pathcost_method)
{
    *pathcost_method = (UI32_T)XSTP_OM_GetPathCostMethod();

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the transmission limit count vlaue.
 * INPUT    :   None
 * OUTPUT   :   UI32_T  *tx_hold_count  -- pointer of the TXHoldCount value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningTransmissionLimit(UI32_T *tx_hold_count)
{
    UI32_T                  result;
    XSTP_OM_InstanceData_T  *om_ptr;

    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    result = XSTP_OM_GetRunningMstTransmissionLimit(om_ptr, tx_hold_count);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the transmission limit count vlaue.
 * INPUT    :   None
 * OUTPUT   :   UI32_T  *tx_hold_count  -- pointer of the TXHoldCount value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetTransmissionLimit(UI32_T *tx_hold_count)
{
    UI32_T              result;

    result = XSTP_OM_GetRunningTransmissionLimit(tx_hold_count);
    if (result == SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge priority information.
 * INPUT    :   None
 * OUTPUT   :   UI16_T  mstid            -- instance value
 *              UI32_T  *priority        -- pointer of the priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMstPriority(UI32_T mstid,
                                      UI32_T *priority)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  result;

    if (mstid > XSTP_TYPE_MAX_MSTID)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    result = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    if (om_ptr->instance_exist /*when global disable, instance_exit will be false*/
        ||mstid == XSTP_TYPE_CISTID )
    {
        *priority = om_ptr->bridge_info.admin_bridge_priority;
        if (om_ptr->bridge_info.static_bridge_priority == TRUE)
        {
            if (*priority == XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY)
            {
                result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
            }
            else
            {
                result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
            }
        }
        else
        {
            result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge priority information.
 * INPUT    :   None
 * OUTPUT   :   UI16_T  mstid            -- instance value
 *              UI32_T  *priority        -- pointer of the priority value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstPriority(UI32_T mstid,
                               UI32_T *priority)
{
    UI32_T                              result;

    result = XSTP_OM_GetRunningMstPriority(mstid, priority);
    if (result == SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port priority for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *priority        -- pointer of the priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * REF      :   RFC-1493/dot1dStpPortEntry 2
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMstPortPriority(UI32_T lport,
                                          UI32_T mstid,
                                          UI32_T *priority)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
/*    UI8_T                   arg_buf[20];*/
    UI8_T                   temp_priority;

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetRunningMstPortPriority_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetRunningMstPortPriority_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    pom_ptr = &(om_ptr->port_info[lport-1]);
    XSTP_OM_GET_PORT_ID_PRIORITY(temp_priority, pom_ptr->port_id);
    *priority = (UI32_T)temp_priority;
    if (*priority == XSTP_TYPE_DEFAULT_PORT_PRIORITY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port priority for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *priority        -- pointer of the priority value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * REF      :   RFC-1493/dot1dStpPortEntry 2
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstPortPriority(UI32_T lport,
                                   UI32_T mstid,
                                   UI32_T *priority)
{
    UI32_T                           result;

    result = XSTP_OM_GetRunningMstPortPriority(lport, mstid, priority);
    if (result == SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortLinkTypeMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port link_type mode of the port for the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetRunningPortLinkTypeMode(UI32_T lport,
                                            UI32_T mstid,
                                            UI32_T *mode)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortLinkTypeMode_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortLinkTypeMode_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    result = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    if (om_ptr->instance_exist /*when global disable, instance_exit will be false*/
        ||mstid == XSTP_TYPE_CISTID )
    {
        if (XSTP_OM_IsMemberPortOfInstance(om_ptr, lport))
        {
            pom_ptr = &(om_ptr->port_info[lport-1]);
            if (pom_ptr->common->admin_point_to_point_mac_auto)
            {
                *mode = XSTP_TYPE_PORT_ADMIN_LINK_TYPE_AUTO;
            }
            else if (pom_ptr->common->admin_point_to_point_mac)
            {
                *mode = XSTP_TYPE_PORT_ADMIN_LINK_TYPE_POINT_TO_POINT;
            }
            else if (!pom_ptr->common->admin_point_to_point_mac)
            {
                *mode = XSTP_TYPE_PORT_ADMIN_LINK_TYPE_SHARED;
            }
            if (*mode == XSTP_TYPE_DEFAULT_PORT_LINK_TYPE_MODE)
            {
                result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
            }
            else
            {
                result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
            }
        }
        else
        {
/*
            EH_MGR_Handle_Exception2(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortLinkTypeMode_Fun_No, EH_TYPE_MSG_NOT_MEMBER, SYSLOG_LEVEL_INFO, "Port", "specified instance ID");
*/
        }
    }
    else
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortLinkTypeMode_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Instance ID");
*/
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortLinkTypeMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port link_type mode of the port for the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetPortLinkTypeMode(UI32_T lport,
                                     UI32_T mstid,
                                     UI32_T *mode)
{
    UI32_T                        result;

    result = XSTP_OM_GetRunningPortLinkTypeMode(lport, mstid, mode);
    if (result == SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortProtocolMigration
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get protocol_migration status for a port in the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetRunningPortProtocolMigration(UI32_T lport,
                                                 UI32_T mstid,
                                                 UI32_T *mode)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortProtocolMigration_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortProtocolMigration_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    result = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    if (om_ptr->instance_exist /*when global disable, instance_exit will be false*/
        ||mstid == XSTP_TYPE_CISTID )
    {
        if (XSTP_OM_IsMemberPortOfInstance (om_ptr, lport))
        {
            pom_ptr = &(om_ptr->port_info[lport-1]);
            if (pom_ptr->common->mcheck)
            {
                *mode = XSTP_TYPE_PORT_PROTOCOL_MIGRATION_ENABLED;
            }
            else
            {
                *mode = XSTP_TYPE_PORT_PROTOCOL_MIGRATION_DISABLED;
            }
            if (*mode == XSTP_TYPE_DEFAULT_PORT_PROTOCOL_MIGRATION_STATUS)
            {
                result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
            }
            else
            {
                result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
            }
        }
        else
        {
/*
            EH_MGR_Handle_Exception2(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortProtocolMigration_Fun_No, EH_TYPE_MSG_NOT_MEMBER, SYSLOG_LEVEL_INFO, "Port", "specified instance ID");
*/
        }
    }
    else
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortProtocolMigration_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Instance ID");
*/
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortProtocolMigration
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get protocol_migration status for a port in the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetPortProtocolMigration(UI32_T lport,
                                          UI32_T mstid,
                                          UI32_T *mode)
{
    UI32_T                        result;

    result = XSTP_OM_GetRunningPortProtocolMigration(lport, mstid, mode);
    if (result == SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortAdminEdgePort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get edge_port status for a port for in specified spanning
 *              tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetRunningPortAdminEdgePort(UI32_T lport,
                                             UI32_T mstid,
                                             UI32_T *mode)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortAdminEdgePort_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortAdminEdgePort_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    result = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    if (om_ptr->instance_exist /*when global disable, instance_exit will be false*/
        ||mstid == XSTP_TYPE_CISTID )
    {
        if (XSTP_OM_IsMemberPortOfInstance (om_ptr, lport))
        {
            pom_ptr = &(om_ptr->port_info[lport-1]);
            if (pom_ptr->common->auto_edge)
            {
                *mode = XSTP_TYPE_PORT_ADMIN_EDGE_PORT_AUTO;
            }
            else if (pom_ptr->common->admin_edge)
            {
                *mode = XSTP_TYPE_PORT_ADMIN_EDGE_PORT_ENABLED;
            }
            else
            {
                *mode = XSTP_TYPE_PORT_ADMIN_EDGE_PORT_DISABLED;
            }

            if (*mode == XSTP_TYPE_DEFAULT_PORT_ADMIN_EDGE_PORT)
            {
                result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
            }
            else
            {
                result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
            }
        }
        else
        {
/*
            EH_MGR_Handle_Exception2(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortAdminEdgePort_Fun_No, EH_TYPE_MSG_NOT_MEMBER, SYSLOG_LEVEL_INFO, "Port", "specified instance ID");
*/
        }
    }
    else
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetRunningPortAdminEdgePort_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Instance ID");
*/
    }

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortAdminEdgePort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get edge_port status for a port for in specified spanning
 *              tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetPortAdminEdgePort(UI32_T lport,
                                      UI32_T mstid,
                                      UI32_T *mode)
{
    UI32_T                        result;

    result = XSTP_OM_GetRunningPortAdminEdgePort(lport, mstid, mode);
    if (result == SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetDot1dMstPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 *              UI32_T mstid                -- instance value
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetDot1dMstPortEntry(UI32_T mstid,
                                     XSTP_MGR_Dot1dStpPortEntry_T *port_entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetDot1dMstPortEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_OM_GetDot1dMstPortEntry_(om_ptr, port_entry);

    return result;

}/* End of XSTP_OM_GetDot1dMstPortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_OM_GetNextDot1dMstPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 *              mstid                       -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   If next available port entry is available, the
 *              port_entry->dot1d_stp_port will be updated and the entry
 *              info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextDot1dMstPortEntry(UI32_T *mstid,
                                         XSTP_MGR_Dot1dStpPortEntry_T *port_entry)
{
    UI32_T                  lport;
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
    BOOL_T                  found;
/*    UI8_T                   arg_buf[20];*/

    if ( (*mstid < 0 || *mstid > XSTP_TYPE_MAX_MSTID) && (*mstid!=XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP))
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetNextDot1dMstPortEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }
    lport = (UI32_T)port_entry->dot1d_stp_port;
    if (lport < 0 || lport >= XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetNextDot1dMstPortEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return FALSE;
    }

    if (*mstid == XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP)
    {
        *mstid =0;
    }
    result  = FALSE;
    found   = FALSE;

    om_ptr = XSTP_OM_GetInstanceInfoPtr(*mstid);
    if (XSTP_OM_GetNextPortMemberOfInstance(om_ptr, &lport))
    {
        port_entry->dot1d_stp_port  = (UI16_T)lport;
        found = TRUE;
    }
    else
    {
        while (XSTP_OM_GetNextExistingInstance_(mstid))
        {
          om_ptr = XSTP_OM_GetInstanceInfoPtr(*mstid);
          lport=0;
          if (XSTP_OM_GetNextPortMemberOfInstance(om_ptr, &lport))
          {
              port_entry->dot1d_stp_port  = (UI16_T)lport;
              found                       = TRUE;
              break;
          }
        }
    }

    if (found)
    {
        result = XSTP_OM_GetDot1dMstPortEntry_(om_ptr, port_entry);
    }

    return result;
}/* End of XSTP_OM_GetNextDot1dMstPortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_OM_GetNextPortMemberByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              lport                       -- lport value
 * OUTPUT   :   lport                       -- next lport
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextPortMemberByInstance(UI32_T mstid, UI32_T *lport)
{
    XSTP_OM_InstanceData_T  *om_ptr;
/*    UI8_T                   arg_buf[20];*/
    BOOL_T                  result;

    if (mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID)
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetNextDot1dMstPortEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }
    if (*lport < 0 || *lport >= XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetNextDot1dMstPortEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return FALSE;
    }

    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    result = XSTP_OM_GetNextPortMemberOfInstance(om_ptr, lport);

    return result;
}/* End of XSTP_OM_GetNextPortMemberByInstance */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetDot1dMstPortEntryX
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 *              UI32_T mstid                -- instance value
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. State is backing for a port with no link.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetDot1dMstPortEntryX(UI32_T mstid,
                                      XSTP_MGR_Dot1dStpPortEntry_T *port_entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetDot1dMstPortEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_OM_GetDot1dMstPortEntry_LinkdownAsBlocking_(om_ptr, port_entry);

    return result;

}/* End of XSTP_OM_GetDot1dMstPortEntryX */

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_OM_GetNextDot1dMstPortEntryX
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 *              mstid                       -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. If next available port entry is available, the
 *                 port_entry->dot1d_stp_port will be updated and the entry
 *                 info will be retrieved from the table.
 *              2. State is backing for a port with no link.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextDot1dMstPortEntryX(UI32_T *mstid,
                                          XSTP_MGR_Dot1dStpPortEntry_T *port_entry)
{
    UI32_T                  lport;
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
    BOOL_T                  found;
/*    UI8_T                   arg_buf[20];*/

    if ( (*mstid < 0 || *mstid > XSTP_TYPE_MAX_MSTID) && (*mstid!=XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP))
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetNextDot1dMstPortEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }
    lport = (UI32_T)port_entry->dot1d_stp_port;
    if (lport < 0 || lport >= XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetNextDot1dMstPortEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return FALSE;
    }

    if (*mstid == XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP)
    {
        *mstid =0;
    }
    result  = FALSE;
    found   = FALSE;

    om_ptr = XSTP_OM_GetInstanceInfoPtr(*mstid);
    if (XSTP_OM_GetNextPortMemberOfInstance(om_ptr, &lport))
    {
        port_entry->dot1d_stp_port  = (UI16_T)lport;
        found = TRUE;
    }
    else if (XSTP_OM_GetNextExistingInstance_(mstid))
    {
        om_ptr = XSTP_OM_GetInstanceInfoPtr(*mstid);
        lport=0;
        if (XSTP_OM_GetNextPortMemberOfInstance(om_ptr, &lport))
        {
            port_entry->dot1d_stp_port  = (UI16_T)lport;
            found                       = TRUE;
        }
    }
    if (found)
    {
        result = XSTP_OM_GetDot1dMstPortEntry_LinkdownAsBlocking_(om_ptr, port_entry);
    }

    return result;
}/* End of XSTP_OM_GetNextDot1dMstPortEntryX */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetDot1dMstExtPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *ext_port_entry                 -- pointer of the specified
 *                                                 port ext_entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetDot1dMstExtPortEntry(UI32_T mstid,
                                        UI32_T lport,
                                        XSTP_MGR_Dot1dStpExtPortEntry_T *ext_port_entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetDot1dMstExtPortEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetDot1dMstExtPortEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_OM_GetDot1dMstExtPortEntry_(om_ptr, lport, ext_port_entry);

    return result;
}/* End of XSTP_OM_GetDot1dMstExtPortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_OM_GetNextDot1dMstExtPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available ext port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   *lport                          -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *ext_port_entry                 -- pointer of the specified
 *                                                 port ext_entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   If next available port ext_entry is available, the
 *              ext_port_entry->dot1d_stp_port will be updated and the
 *              entry info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextDot1dMstExtPortEntry(UI32_T mstid,
                                            UI32_T *lport,
                                            XSTP_MGR_Dot1dStpExtPortEntry_T *ext_port_entry)
{
    BOOL_T                  result;
    XSTP_OM_InstanceData_T  *om_ptr;
/*    UI8_T                   arg_buf[20];*/

    result = FALSE;
    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetNextDot1dMstExtPortEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }

    if (SWCTRL_GetNextLogicalPort(lport)!=SWCTRL_LPORT_UNKNOWN_PORT)
    {
        om_ptr  = XSTP_OM_GetInstanceInfoPtr(mstid);
        result  = XSTP_OM_GetDot1dMstExtPortEntry_(om_ptr, *lport, ext_port_entry);
    }

    return result;
}

/*-------------------------------------------------------------------------
 * The following Functions only provide for MSTP
 *-------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstpRevisionLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP revision level value.
 * INPUT    :
 * OUTPUT   :   U32_T *revision     -- pointer of the revision value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMstpRevisionLevel(UI32_T *revision)
{
    *revision = XSTP_OM_GetRegionRevision();

    if (*revision == XSTP_TYPE_DEFAULT_CONFIG_REVISION)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpRevisionLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP revision level value.
 * INPUT    :
 * OUTPUT   :   U32_T *revision     -- pointer of the revision value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpRevisionLevel(UI32_T *revision)
{
    UI32_T                        result;

    result = XSTP_OM_GetRunningMstpRevisionLevel(revision);
    if (result == SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstpMaxHop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP Max_Hop count.
 * INPUT    :
 * OUTPUT   :   U32_T *hop_count              -- pointer of max_hop count
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMstpMaxHop(UI32_T *hop_count)
{
    *hop_count = XSTP_OM_GetMaxHopCount();

    if (*hop_count == XSTP_TYPE_DEFAULT_BRIDGE_MAXHOP)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpMaxHop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP Max_Hop count.
 * INPUT    :
 * OUTPUT   :   U32_T *hop_count              -- pointer of max_hop count
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpMaxHop(UI32_T *hop_count)
{
    UI32_T                        result;

    result = XSTP_OM_GetRunningMstpMaxHop(hop_count);
    if (result == SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpConfigurationEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the configuration entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :
 * OUTPUT   :   *mstp_entry      -- pointer of the configuration entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpConfigurationEntry(XSTP_MGR_MstpEntry_T  *mstp_entry)
{
    memset(mstp_entry , 0 , sizeof(XSTP_MGR_MstpEntry_T));
    mstp_entry->mstp_max_instance_number        = (UI32_T)XSTP_OM_GetMaxInstanceNumber();
    mstp_entry->mstp_max_mstid                  = (UI32_T)XSTP_TYPE_MAX_MSTID;
    mstp_entry->mstp_current_instance_number    = (UI32_T)XSTP_OM_GetCurrentCfgInstanceNumber();
    mstp_entry->mstp_format_selector            = (UI32_T)SYS_DFLT_STP_CONFIG_ID_FORMAT_SELECTOR;
    XSTP_OM_GetRegionName(mstp_entry->mstp_region_name);
    mstp_entry->mstp_region_revision            = (UI32_T)XSTP_OM_GetRegionRevision();
    mstp_entry->mstp_max_hop_count              = (UI32_T)XSTP_OM_GetMaxHopCount();

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpInstanceVlanMapped
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of map VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Mapping_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpInstanceVlanMapped(UI32_T mstid,
                                          XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;

    if ( mstid > XSTP_TYPE_MAX_MSTID )
    {
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_OM_GetMstpInstanceVlanMapped_(om_ptr, mstp_instance_entry);

    return result;
}/* End of XSTP_OM_GetMstpInstanceVlanMapped() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpInstanceVlanMappedForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of map VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Mapping_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpInstanceVlanMappedForMSB(UI32_T mstid,
                                                XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;

    if ( mstid > XSTP_TYPE_MAX_MSTID )
    {
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_OM_GetMstpInstanceVlanMapped_(om_ptr, mstp_instance_entry);
    XSTP_OM_LSB_To_MSB((UI8_T *) &(mstp_instance_entry->mstp_instance_vlans_mapped), 128);
    XSTP_OM_LSB_To_MSB((UI8_T *) &(mstp_instance_entry->mstp_instance_vlans_mapped2k), 128);
    XSTP_OM_LSB_To_MSB((UI8_T *) &(mstp_instance_entry->mstp_instance_vlans_mapped3k), 128);
    XSTP_OM_LSB_To_MSB((UI8_T *) &(mstp_instance_entry->mstp_instance_vlans_mapped4k), 128);

    return result;
}/* End of XSTP_OM_GetMstpInstanceVlanMappedForMSB() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_NextGetMstpInstanceVlanMapped
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Mapping_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextMstpInstanceVlanMapped(UI32_T *mstid,
                                              XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
    BOOL_T                  found;
/*    UI8_T                   arg_buf[20];*/

    if ( (*mstid < 0 || *mstid >= XSTP_TYPE_MAX_MSTID) && (*mstid!=XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP))
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetNextMstpInstanceVlanMapped_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }

    result = FALSE;
    found  = FALSE;
    if (*mstid == XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP)
    {
        *mstid = 0;
        found  = TRUE;
    }
    else if (XSTP_OM_GetNextExistingInstance_(mstid))
    {
        found = TRUE;
    }
    if (found)
    {
        om_ptr      = XSTP_OM_GetInstanceInfoPtr(*mstid);
        result      = XSTP_OM_GetMstpInstanceVlanMapped_(om_ptr, mstp_instance_entry);
    }

    return result;
}/* End of XSTP_OM_GetNextMstpInstanceVlanMapped() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextMstpInstanceVlanMappedForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Mapping_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextMstpInstanceVlanMappedForMSB(UI32_T *mstid,
                                                    XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    BOOL_T                  result;
    BOOL_T                  found;
/*    UI8_T                   arg_buf[20];*/

    if ( (*mstid < 0 || *mstid >= XSTP_TYPE_MAX_MSTID) && (*mstid!=XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP))
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetNextMstpInstanceVlanMapped_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }

    result = FALSE;
    found  = FALSE;
    if (*mstid == XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP)
    {
        *mstid = 0;
        found  = TRUE;
    }
    else if (XSTP_OM_GetNextExistingInstance_(mstid))
    {
        found = TRUE;
    }
    if (found)
    {
        result      = XSTP_OM_GetMstpInstanceVlanMappedForMSB(*mstid, mstp_instance_entry);
    }

    return result;
}/* End of XSTP_OM_GetNextMstpInstanceVlanMappedForMSB() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Configuration_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpInstanceVlanConfiguration(UI32_T mstid,
                                                 XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;

    if ( mstid > XSTP_TYPE_MAX_MSTID )
    {
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_OM_GetMstpInstanceVlanConfiguration_(om_ptr, mstp_instance_entry);

    return result;
}/* End of XSTP_OM_GetMstpInstanceVlanConfiguration() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpInstanceVlanConfigurationForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Configuration_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstpInstanceVlanConfigurationForMSB(UI32_T mstid,
                                                       XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;

    if ( mstid > XSTP_TYPE_MAX_MSTID )
    {
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_OM_GetMstpInstanceVlanConfiguration_(om_ptr, mstp_instance_entry);
    XSTP_OM_LSB_To_MSB((UI8_T *) &(mstp_instance_entry->mstp_instance_vlans_mapped), 128);
    XSTP_OM_LSB_To_MSB((UI8_T *) &(mstp_instance_entry->mstp_instance_vlans_mapped2k), 128);
    XSTP_OM_LSB_To_MSB((UI8_T *) &(mstp_instance_entry->mstp_instance_vlans_mapped3k), 128);
    XSTP_OM_LSB_To_MSB((UI8_T *) &(mstp_instance_entry->mstp_instance_vlans_mapped4k), 128);

    return result;
}/* End of XSTP_OM_GetMstpInstanceVlanConfigurationForMSB() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
  * RETURN   :  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   From Configuration_Table.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningMstpInstanceVlanConfiguration(UI32_T mstid,
                                                        XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    if (mstid == XSTP_TYPE_CISTID)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        XSTP_OM_GetMstpInstanceVlanConfiguration(mstid, mstp_instance_entry);
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* End of XSTP_OM_GetRunningMstpInstanceVlanConfiguration() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Configuration_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextMstpInstanceVlanConfiguration(UI32_T *mstid,
                                                     XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
    BOOL_T                  found;

    if ( (*mstid < 0 || *mstid >= XSTP_TYPE_MAX_MSTID) && (*mstid!=XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP))
    {
        return FALSE;
    }

    result = FALSE;
    found  = FALSE;
    if (*mstid == XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP)
    {
        *mstid = 0;
        found  = TRUE;
    }
    else if (XSTP_OM_GetNextInstanceInfoPtr(mstid, &om_ptr))
    {
        found = TRUE;
    }
    if (found)
    {
        om_ptr      = XSTP_OM_GetInstanceInfoPtr(*mstid);
        result      = XSTP_OM_GetMstpInstanceVlanConfiguration_(om_ptr, mstp_instance_entry);
    }

    return result;
}/* End of XSTP_OM_GetNextMstpInstanceVlanConfiguration() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextMstpInstanceVlanConfigurationForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Configuration_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextMstpInstanceVlanConfigurationForMSB(UI32_T *mstid,
                                                           XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
    BOOL_T                  found;

    if ( (*mstid < 0 || *mstid >= XSTP_TYPE_MAX_MSTID) && (*mstid!=XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP))
    {
        return FALSE;
    }

    result = FALSE;
    found  = FALSE;
    if (*mstid == XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP)
    {
        *mstid = 0;
        found  = TRUE;
    }
    else if (XSTP_OM_GetNextInstanceInfoPtr(mstid, &om_ptr))
    {
        found = TRUE;
    }

    if (found)
    {
        result      = XSTP_OM_GetMstpInstanceVlanConfigurationForMSB(*mstid, mstp_instance_entry);
    }

    return result;
}/* End of XSTP_OM_GetNextMstpInstanceVlanConfigurationForMSB() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMstInstanceExisting
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funcion returns true if the mst instance exist for mst
 *              mapping table (active).Otherwise, returns false.
 * INPUT    :   UI32_T mstid             -- the instance id
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsMstInstanceExisting (UI32_T mstid)
{
    BOOL_T                  instance_exist;
    XSTP_OM_InstanceData_T  *om_ptr;

    if (mstid > XSTP_TYPE_MAX_MSTID)
    {
        return FALSE;
    }

    instance_exist = FALSE;
    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    instance_exist = om_ptr->instance_exist;

    return instance_exist;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextExistedInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next existed MST instance(active) for mst mapping table.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextExistedInstance(UI32_T *mstid)
{
    BOOL_T                  result;

    result              = XSTP_OM_GetNextExistingInstance_(mstid);

    return result;
}/* End of XSTP_OM_GetNextExistedInstance() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMstInstanceExistingInMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funcion returns true if the mst instance exist for mst
 *              config table(inactive).Otherwise, returns false.
 * INPUT    :   UI32_T mstid             -- the instance id
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_IsMstInstanceExistingInMstConfigTable(UI32_T mstid)
{
    UI32_T                  vid;
    BOOL_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_IsMstInstanceExistingInMstConfigTable_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }
    vid = 0;
    result = XSTP_OM_GetNextXstpMemberFromMstConfigurationTable(mstid, &vid);

    return result;
}/* End of XSTP_OM_IsMstInstanceExistingInMstConfigTable() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextExistedInstanceForMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next existed MST instance (inactive) for mst config
 *            table.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextExistedInstanceForMstConfigTable(UI32_T *mstid)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;

    result              = XSTP_OM_GetNextInstanceInfoPtr(mstid, &om_ptr);

    return result;
}/* End of XSTP_OM_GetNextExistedInstanceForMstConfigTable */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port role in a specified spanning tree.
 * INPUT    :   UI32_T lport                 -- lport number
 *              UI32_T mstid                 -- instance value
 * OUTPUT   :   UI32_T *role                 -- the pointer of role value
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- get successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid  out of range
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetMstPortRole(UI32_T lport,
                               UI32_T mstid,
                               UI32_T *role)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetMstPortRole_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetMstPortRole_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_OM_GetMstPortRole_(om_ptr, lport, role);

    return result;

}/* End of XSTP_OM_GetMstPortRole() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state in a specified spanning tree.
 * INPUT    :   UI32_T lport                 -- lport number
 *              UI32_T mstid                 -- instance value
 * OUTPUT   :   U32_T  *state                -- the pointer of state value
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- get successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid  out of range
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetMstPortState(UI32_T lport,
                                UI32_T mstid,
                                UI32_T *state)

{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetMstPortState_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetMstPortState_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_OM_GetMstPortState_(om_ptr, lport, state);

    return result;
}/* End of XSTP_OM_GetMstPortState() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextVlanMemberByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next vlan member for a specified instance
 * INPUT    : mstid                 -- instance value
 *            vid       -- vlan id pointer
 * OUTPUT   : vid       -- next vlan id pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextVlanMemberByInstance(UI32_T mstid,
                                             UI32_T *vid)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetNextVlanMemberByInstance_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }
    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    result = XSTP_OM_GetNextXstpMember(om_ptr, vid);

    return result;
}/* End of XSTP_OM_GetNextVlanMemberByInstance*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstidFromMstConfigurationTableByVlanEx
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mstid value form mst configuration table for a specified
 *            vlan.
 * INPUT    : vid       -- vlan number
 *            mstid     -- mstid value point
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetMstidFromMstConfigurationTableByVlanEx(UI32_T vid, UI32_T *mstid)
{
    if ( vid < 1 || vid > XSTP_TYPE_SYS_MAX_VLAN_ID )
    {
        return FALSE;
    }

    XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local(vid, mstid);

    return TRUE;
} /* End of XSTP_OM_GetMstidFromMstConfigurationTableByVlan */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextMstidFromMstConfigurationTableByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mstid value form mst configuration table for a specified
 *            vlan.
 * INPUT    : vid       -- vlan number
 *            mstid     -- mstid value point
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetNextMstidFromMstConfigurationTableByVlan(UI32_T *vid, UI32_T *mstid)
{
    if ( *vid < 0 || *vid >= XSTP_TYPE_SYS_MAX_VLAN_ID )
    {
        return FALSE;
    }

    (*vid)++;
    XSTP_OM_GetMstidFromMstConfigurationTableByVlan(*vid, mstid);

    return TRUE;
} /* End of XSTP_OM_GetNextMstidFromMstConfigurationTableByVlan */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMemberPortOfInstanceEx
 *-------------------------------------------------------------------------
 * PURPOSE  : Check whether the specified lport is the member of this
 *            spanning tree instance
 * INPUT    : mstid     -- mstid value
 *            lport     -- lport
 * OUTPUT   : None
 * RETUEN   : TRUE if the specified vlan is the member of this instance, else
 *            FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_IsMemberPortOfInstanceEx(UI32_T mstid, UI32_T lport)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
        return FALSE;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_OM_IsMemberPortOfInstance(om_ptr, lport);

    return result;
} /* XSTP_OM_IsMemberPortOfInstance */

#ifdef  XSTP_TYPE_PROTOCOL_MSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetConfigDigest
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the configuration digest
 * INPUT    :   None
 * OUTPUT   :   config_digest           -- pointer of a 16 octet buffer for
 *                                         the configuration digest
 * RETURN   :   TRUE/FALSE
 * NOTES    :   Ref to the description in 13.7, IEEE 802.1s-2002
 * ------------------------------------------------------------------------
 */
BOOL_T    XSTP_OM_GetConfigDigest(UI8_T *config_digest)
{
    XSTP_OM_InstanceData_T  *om_ptr;

    memset(config_digest, 0, 16);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);

    memcpy(config_digest, om_ptr->bridge_info.common->mst_config_id.config_digest, 16);

    return TRUE;
}/* End of XSTP_OM_GetConfigDigest */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpRowStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the entry can be
 *            get successfully.  Otherwise, return false.
 * INPUT    : mstid       -- instance value
 * OUTPUT   : row_status  -- VAL_dot1qVlanStaticRowStatus_active
 *                           VAL_dot1qVlanStaticRowStatus_notInService
 *                           VAL_dot1qVlanStaticRowStatus_notReady
 *                           VAL_dot1qVlanStaticRowStatus_createAndGo
 *                           VAL_dot1qVlanStaticRowStatus_createAndWait
 *                           VAL_dot1qVlanStaticRowStatus_destroy
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T XSTP_OM_GetMstpRowStatus(UI32_T mstid, UI32_T *row_status)
{
    XSTP_OM_InstanceData_T  *om_ptr;
/*    UI8_T                   arg_buf[20];*/
    BOOL_T                  result;

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetMstpMstiRowStatus_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }

    if (XSTP_OM_IsMstInstanceExistingInMstConfigTable(mstid))
    {
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
        *row_status         = om_ptr->row_status;
        result = TRUE;
    }
    else
    {
        result = FALSE;
    }

    return result;
}/* End of XSTP_OM_GetMstpRowStatus() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextMstpRowStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the entry can be
 *            get successfully.  Otherwise, return false.
 * INPUT    : mstid       -- instance value
 * OUTPUT   : row_status  -- VAL_dot1qVlanStaticRowStatus_active
 *                           VAL_dot1qVlanStaticRowStatus_notInService
 *                           VAL_dot1qVlanStaticRowStatus_notReady
 *                           VAL_dot1qVlanStaticRowStatus_createAndGo
 *                           VAL_dot1qVlanStaticRowStatus_createAndWait
 *                           VAL_dot1qVlanStaticRowStatus_destroy
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T XSTP_OM_GetNextMstpRowStatus(UI32_T *mstid, UI32_T *row_status)
{
/*    UI8_T                   arg_buf[20];*/
    BOOL_T                  result;
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  found;

    if ( *mstid < 0 || *mstid >= XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetMstpMstiRowStatus_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }

    found = FALSE;
    if (XSTP_OM_GetNextInstanceInfoPtr(mstid, &om_ptr))
    {
        found = TRUE;
    }

    if (found)
    {
        result = XSTP_OM_GetMstpRowStatus(*mstid, row_status);
    }
    else
    {
        result = FALSE;
    }

    return result;
}/* End of XSTP_OM_GetNextMstpRowStatus() */

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortAdminPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the admin path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *admin_path_cost -- admin path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   1. If the default Path Cost is being used, return '0'.
 *              2. It is equal to external_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPortAdminPathCost(UI32_T lport, UI32_T *admin_path_cost)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

#ifdef  XSTP_TYPE_PROTOCOL_RSTP
    if (pom_ptr->static_path_cost)
    {
        *admin_path_cost = (UI32_T)pom_ptr->port_path_cost;
    }
    else
    {
        *admin_path_cost = 0;
    }
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (pom_ptr->common->static_external_path_cost)
    {
        *admin_path_cost = (UI32_T)pom_ptr->common->external_port_path_cost;
    }
    else
    {
        *admin_path_cost = 0;
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_OM_GetPortAdminPathCost() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortOperPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the oper path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *oper_path_cost  -- oper path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   It is equal to external_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPortOperPathCost(UI32_T lport, UI32_T *oper_path_cost)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);
#ifdef  XSTP_TYPE_PROTOCOL_RSTP
    *oper_path_cost = (UI32_T)pom_ptr->port_path_cost;
#endif /* XSTP_TYPE_PROTOCOL_RSTP */
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    *oper_path_cost = (UI32_T)pom_ptr->common->external_port_path_cost;
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_OM_GetPortOperPathCost() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortAdminPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the admin path_cost of the port for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *admin_path_cost -- admin path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   1. If the default Path Cost is being used, return '0'.
 *              2. It is equal to internal_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetMstPortAdminPathCost(UI32_T lport, UI32_T mstid, UI32_T *admin_path_cost)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  current_st_mode;

    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    if ( mstid > XSTP_TYPE_MAX_MSTID )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();
    if (    (current_st_mode != XSTP_TYPE_MSTP_MODE)
        &&  (mstid != XSTP_TYPE_CISTID)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    pom_ptr = &(om_ptr->port_info[lport-1]);
#ifdef  XSTP_TYPE_PROTOCOL_RSTP
    if (pom_ptr->static_path_cost)
    {
        *admin_path_cost = (UI32_T)pom_ptr->port_path_cost;
    }
    else
    {
        *admin_path_cost = 0;
    }
#endif /* XSTP_TYPE_PROTOCOL_RSTP */
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (current_st_mode == XSTP_TYPE_MSTP_MODE)
    {
        if (pom_ptr->static_internal_path_cost)
        {
            *admin_path_cost = (UI32_T)pom_ptr->internal_port_path_cost;
        }
        else
        {
            *admin_path_cost = 0;
        }
    }
    else
    {
        if (pom_ptr->common->static_external_path_cost)
        {
            *admin_path_cost = (UI32_T)pom_ptr->common->external_port_path_cost;
        }
        else
        {
            *admin_path_cost = 0;
        }
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_OM_GetMstPortAdminPathCost() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortOperPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the oper path_cost of the port for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *oper_path_cost  -- oper path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   It is equal to internal_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetMstPortOperPathCost(UI32_T lport, UI32_T mstid, UI32_T *oper_path_cost)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  current_st_mode;

    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    if (mstid > XSTP_TYPE_MAX_MSTID )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();
    if (    (current_st_mode != XSTP_TYPE_MSTP_MODE)
        &&  (mstid != XSTP_TYPE_CISTID)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    pom_ptr = &(om_ptr->port_info[lport-1]);
#ifdef  XSTP_TYPE_PROTOCOL_RSTP
    *oper_path_cost = (UI32_T)pom_ptr->port_path_cost;
#endif /* XSTP_TYPE_PROTOCOL_RSTP */
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (current_st_mode == XSTP_TYPE_MSTP_MODE)
    {
        *oper_path_cost = (UI32_T)pom_ptr->internal_port_path_cost;
    }
    else
    {
        *oper_path_cost = (UI32_T)pom_ptr->common->external_port_path_cost;
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_OM_GetMstPortOperPathCost() */

/* per_port spanning tree : begin */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree status of the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortSpanningTreeStatus(UI32_T lport, UI32_T *status)
{
    XSTP_OM_InstanceData_T          *om_ptr;
    XSTP_OM_PortVar_T               *pom_ptr;

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);
    *status = pom_ptr->common->port_spanning_tree_status;

    if (*status == XSTP_TYPE_DEFAULT_PORT_SPANNING_TREE_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* End of XSTP_OM_GetRunningPortSpanningTreeStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree status of the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortSpanningTreeStatus(UI32_T lport, UI32_T *status)
{
    UI32_T              result;

    result = XSTP_OM_GetRunningPortSpanningTreeStatus(lport, status);
    if (result == SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        return FALSE;
    }
    return TRUE;
}/* End of XSTP_OM_GetPortSpanningTreeStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetDesignatedRoot
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated root for specified instance.
 * INPUT    :   mstid                    -- instance value
 * OUTPUT   :   *designated_root         -- pointer of the specified
 *                                          designated_root
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetDesignatedRoot(UI32_T mstid,
                                  XSTP_MGR_BridgeIdComponent_T *designated_root)
{
    XSTP_OM_InstanceData_T  *om_ptr;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetDesignatedRootId_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
#ifdef  XSTP_TYPE_PROTOCOL_RSTP
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
    {
        designated_root->priority =(UI16_T) om_ptr->bridge_info.root_priority.root_bridge_id.bridge_id_priority.bridge_priority;
    }
    else
    {
        XSTP_OM_GET_BRIDGE_ID_PRIORITY(designated_root->priority, om_ptr->bridge_info.root_priority.root_bridge_id);
    }
    designated_root->system_id_ext = mstid;
    memcpy((UI8_T *)&(designated_root->addr),
           (UI8_T *)&(om_ptr->bridge_info.root_priority.root_bridge_id.addr), 6);
#endif /* XSTP_TYPE_PROTOCOL_RSTP */
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
    {
        designated_root->priority =(UI16_T) om_ptr->bridge_info.root_priority.root_id.bridge_id_priority.bridge_priority;
        memcpy((UI8_T *)&(designated_root->addr),
               (UI8_T *)&(om_ptr->bridge_info.root_priority.root_id.addr), 6);
    }
    else
    {
        XSTP_OM_GET_BRIDGE_ID_PRIORITY(designated_root->priority, om_ptr->bridge_info.root_priority.r_root_id);
        memcpy((UI8_T *)&(designated_root->addr),
               (UI8_T *)&(om_ptr->bridge_info.root_priority.r_root_id.addr), 6);
    }
    designated_root->system_id_ext = mstid;

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return TRUE;
}/* End of XSTP_OM_GetDesignatedRoot */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetBridgeIdComponent
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge_id_component for specified instance.
 * INPUT    :   mstid                    -- instance value
 * OUTPUT   :   *designated_root         -- pointer of the specified
 *                                          bridge_id_component
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetBridgeIdComponent(UI32_T mstid,
                                     XSTP_MGR_BridgeIdComponent_T *bridge_id_component)
{
    XSTP_OM_InstanceData_T  *om_ptr;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetBridgeIdComponent_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    XSTP_OM_GET_BRIDGE_ID_PRIORITY(bridge_id_component->priority, om_ptr->bridge_info.bridge_identifier);
    bridge_id_component->system_id_ext = mstid;
    memcpy((UI8_T *)&(bridge_id_component->addr),
           (UI8_T *)&(om_ptr->bridge_info.bridge_identifier.addr), 6);

    return TRUE;
}/* End of XSTP_OM_GetBridgeIdComponent */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortDesignatedRoot
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated root for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              mstid                           -- instance value
 * OUTPUT   :   *designated_root                -- pointer of the specified
 *                                                 designated_root
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortDesignatedRoot(UI32_T lport,
                                      UI32_T mstid,
                                      XSTP_MGR_BridgeIdComponent_T *designated_root)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    BOOL_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetPortDesignatedRootId_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }

    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetPortDesignatedRootId_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    pom_ptr             = &(om_ptr->port_info[lport-1]);
    if (XSTP_OM_IsMemberPortOfInstance(om_ptr, lport))
    {
        if (!pom_ptr->common->link_up)
        {
            memcpy(&pom_ptr->port_priority, &pom_ptr->designated_priority,  sizeof(XSTP_TYPE_PriorityVector_T) );
        }
#ifdef  XSTP_TYPE_PROTOCOL_RSTP
        if (om_ptr->instance_id == XSTP_TYPE_CISTID)
        {
            designated_root->priority =(UI16_T) pom_ptr->port_priority.root_bridge_id.bridge_id_priority.bridge_priority;
        }
        else
        {
            XSTP_OM_GET_BRIDGE_ID_PRIORITY(designated_root->priority, pom_ptr->port_priority.root_bridge_id);
        }
        designated_root->system_id_ext = mstid;
        memcpy((UI8_T *)&(designated_root->addr),
               (UI8_T *)&(pom_ptr->port_priority.root_bridge_id.addr), 6);
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
        if (om_ptr->instance_id == XSTP_TYPE_CISTID)
        {
             designated_root->priority =(UI16_T) pom_ptr->port_priority.root_id.bridge_id_priority.bridge_priority;
             memcpy((UI8_T *)&(designated_root->addr),
                    (UI8_T *)&(pom_ptr->port_priority.root_id.addr), 6);
        }
        else
        {
            XSTP_OM_GET_BRIDGE_ID_PRIORITY(designated_root->priority, pom_ptr->port_priority.r_root_id);
            memcpy((UI8_T *)&(designated_root->addr),
                   (UI8_T *)&(pom_ptr->port_priority.r_root_id.addr), 6);
        }
        designated_root->system_id_ext = mstid;

#endif /* XSTP_TYPE_PROTOCOL_MSTP */
        result = TRUE;
    }
    else
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            printf("\r\nXSTP_OM_GetPortDesignatedRoot::ERROR!! port is not member.");
/*        EH_MGR_Handle_Exception2(SYS_MODULE_XSTP, XSTP_OM_GetPortDesignatedRootId_Fun_No, EH_TYPE_MSG_NOT_MEMBER, SYSLOG_LEVEL_INFO, "Port", "specified instance ID");*/
        result = FALSE;
    }

    return result;
}/* End of XSTP_OM_GetPortDesignatedRoot */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortDesignatedBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated bridge for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *designated_bridge              -- pointer of the specified
 *                                                 port ext_entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortDesignatedBridge(UI32_T lport,
                                        UI32_T mstid,
                                        XSTP_MGR_BridgeIdComponent_T *designated_bridge)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    BOOL_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetPortDesignatedBridgeId_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }

    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetPortDesignatedBridgeId_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    pom_ptr             = &(om_ptr->port_info[lport-1]);
    if (XSTP_OM_IsMemberPortOfInstance(om_ptr, lport))
    {
        if (om_ptr->instance_id == XSTP_TYPE_CISTID)
        {
            designated_bridge->priority =(UI16_T) pom_ptr->port_priority.designated_bridge_id.bridge_id_priority.bridge_priority;
        }
        else
        {
            XSTP_OM_GET_BRIDGE_ID_PRIORITY(designated_bridge->priority, pom_ptr->port_priority.designated_bridge_id);
        }
        designated_bridge->system_id_ext = mstid;
        memcpy((UI8_T *)&(designated_bridge->addr),
               (UI8_T *)&(pom_ptr->port_priority.designated_bridge_id.addr), 6);

        result = TRUE;
    }
    else
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            printf("\r\nXSTP_OM_GetPortDesignatedBridge::ERROR!! port is not member.");
/*        EH_MGR_Handle_Exception2(SYS_MODULE_XSTP, XSTP_OM_GetPortDesignatedBridgeId_Fun_No, EH_TYPE_MSG_NOT_MEMBER, SYSLOG_LEVEL_INFO, "Port", "specified instance ID");*/
        result = FALSE;
    }

    return result;
}/* End of XSTP_OM_GetPortDesignatedBridge */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortDesignatedPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated port for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *designated_port                -- pointer of the specified
 *                                                 designated_port
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortDesignatedPort(UI32_T lport,
                                      UI32_T mstid,
                                      XSTP_MGR_PortIdComponent_T *designated_port)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    BOOL_T                  result;
/*    UI8_T                   arg_buf[20];*/

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetPortDesignatedPort_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetPortDesignatedPort_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
*/
        return FALSE;
    }

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    pom_ptr             = &(om_ptr->port_info[lport-1]);
    if (XSTP_OM_IsMemberPortOfInstance(om_ptr, lport))
    {
        XSTP_OM_GET_PORT_ID_PRIORITY(designated_port->priority, pom_ptr->port_priority.designated_port_id);
        XSTP_OM_GET_PORT_ID_PORTNUM(designated_port->port_num, pom_ptr->port_priority.designated_port_id);
        result = TRUE;
    }
    else
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            printf("\r\nXSTP_OM_GetPortDesignatedPort::ERROR!! port is not member.");
/*        EH_MGR_Handle_Exception2(SYS_MODULE_XSTP, XSTP_OM_GetPortDesignatedPort_Fun_No, EH_TYPE_MSG_NOT_MEMBER, SYSLOG_LEVEL_INFO, "Port", "specified instance ID");*/
        result = FALSE;
    }

    return result;
}/* End of XSTP_OM_GetPortDesignatedPort */

/* per_port spanning tree : end */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the forward_delay time information of the specified tree.
 * INPUT    :   *om_ptr               -- the pointer of info entry
 * OUTPUT   :   UI32_T *forward_delay -- pointer of the forward_delay value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1 sec
 * ------------------------------------------------------------------------
 */
static  UI32_T  XSTP_OM_GetRunningMstForwardDelay(XSTP_OM_InstanceData_T *om_ptr,
                                                   UI32_T *forward_delay)
{
    *forward_delay = ((UI32_T)om_ptr->bridge_info.bridge_times.forward_delay);

    if (*forward_delay == XSTP_TYPE_DEFAULT_FORWARD_DELAY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the hello_time information of the specified tree.
 * INPUT    :   *om_ptr                 -- the pointer of info entry
 * OUTPUT   :   UI32_T *hello_time      -- pointer of the hello_time value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1 sec
 * ------------------------------------------------------------------------
 */
static  UI32_T  XSTP_OM_GetRunningMstHelloTime(XSTP_OM_InstanceData_T *om_ptr,
                                                UI32_T *hello_time)
{
    *hello_time = (UI32_T)om_ptr->bridge_info.bridge_times.hello_time;

    if (*hello_time == XSTP_TYPE_DEFAULT_HELLO_TIME)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the max_age information of the specified tree.
 * INPUT    :   *om_ptr                 -- the pointer of info entry
 * OUTPUT   :   UI32_T *hello_time      -- pointer of the hello_time value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1 sec
 * ------------------------------------------------------------------------
 */
static  UI32_T  XSTP_OM_GetRunningMstMaxAge(XSTP_OM_InstanceData_T *om_ptr,
                                             UI32_T *max_age)
{

    *max_age = (UI32_T)om_ptr->bridge_info.bridge_times.max_age;

    if (*max_age == XSTP_TYPE_DEFAULT_MAX_AGE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningMstTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the transmission limit count vlaue.
 * INPUT    :   *om_ptr                 -- the pointer of info entry
 * OUTPUT   :   UI32_T  *tx_hold_count  -- pointer of the TXHoldCount value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_OM_GetRunningMstTransmissionLimit(XSTP_OM_InstanceData_T *om_ptr,
                                                      UI32_T *tx_hold_count)
{

    *tx_hold_count = (UI32_T)om_ptr->bridge_info.common->tx_hold_count;

    if (*tx_hold_count == XSTP_TYPE_DEFAULT_TX_HOLD_COUNT)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* End of XSTP_OM_GetRunningMstTransmissionLimit */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextPortMemberOfInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next lport member of this spanning tree instance.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T XSTP_OM_GetNextPortMemberOfInstance(XSTP_OM_InstanceData_T *om_ptr,
                                                   UI32_T *lport)
{
    UI32_T  i;

    for(i= ((*lport)+1); i<XSTP_TYPE_MAX_NUM_OF_LPORT; i++)
    {
        if (XSTP_OM_IsMemberPortOfInstance(om_ptr, i)==TRUE)
        {
            *lport = i;
            return TRUE;
        }
    }
    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortRole_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port role in a specified spanning tree.
 * INPUT    :   om_ptr                -- the pointer of the instance entry.
 *              UI32_T lport          -- lport number
 * OUTPUT   :   UI32_T  *role         -- the pointer of role value
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- get successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid  out of range
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_OM_GetMstPortRole_(XSTP_OM_InstanceData_T *om_ptr,
                                       UI32_T lport,
                                       UI32_T *role)
{
    UI32_T                  current_role;
    XSTP_OM_PortVar_T       *pom_ptr;


    if (XSTP_OM_GetSpanningTreeStatus() == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
    {
        if (XSTP_OM_IsMemberPortOfInstance(om_ptr, lport)== FALSE)
        {
            return XSTP_TYPE_RETURN_ERROR;
        }
    }

    pom_ptr= &(om_ptr->port_info[lport-1]);
    current_role = pom_ptr->role;

    switch(current_role)
    {
        case XSTP_ENGINE_PORTVAR_ROLE_DISABLED:
            *role = XSTP_TYPE_PORT_ROLE_DISABLED;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_ROOT:
            *role = XSTP_TYPE_PORT_ROLE_ROOT;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED:
            *role = XSTP_TYPE_PORT_ROLE_DESIGNATED;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE:
            *role = XSTP_TYPE_PORT_ROLE_ALTERNATE;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_BACKUP:
            *role = XSTP_TYPE_PORT_ROLE_BACKUP;
            break;
#ifdef XSTP_TYPE_PROTOCOL_MSTP
        case XSTP_ENGINE_PORTVAR_ROLE_MASTER:
            *role = XSTP_TYPE_PORT_ROLE_MASTER;
            break;
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
        default:
            return XSTP_TYPE_RETURN_ERROR;
    }
    return XSTP_TYPE_RETURN_OK;

}/* End of XSTP_OM_GetMstPortRole_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstPortState_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state in a specified spanning tree.
 * INPUT    :   om_ptr                  -- the pointer of the instance entry.
 *              UI32_T lport            -- lport number
 * OUTPUT   :   U32_T  *state           -- the pointer of state value
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- get successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid  out of range
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_OM_GetMstPortState_(XSTP_OM_InstanceData_T *om_ptr,
                                        UI32_T lport,
                                        UI32_T *state)

{
    XSTP_OM_PortVar_T       *pom_ptr;

    pom_ptr= &(om_ptr->port_info[lport-1]);

#if 0
    if (XSTP_OM_GetSpanningTreeStatus() == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
    {
        if (XSTP_OM_IsMemberPortOfInstance(om_ptr, lport)== FALSE)
        {
            return XSTP_TYPE_RETURN_ERROR;
        }
    }
#endif

    *state = (UI32_T)XSTP_TYPE_PORT_STATE_DISCARDING;
    if (!pom_ptr->common->port_enabled ) /* this port is "DISABLED". */
    {
        *state = XSTP_TYPE_PORT_STATE_DISCARDING;
    }
    else if ((pom_ptr->learning == TRUE) && (pom_ptr->forwarding == FALSE) )
    {
        *state = (UI32_T)XSTP_TYPE_PORT_STATE_LEARNING;
    }
    else if ((pom_ptr->learning == TRUE) && (pom_ptr->forwarding == TRUE) )
    {
        *state = (UI32_T)XSTP_TYPE_PORT_STATE_FORWARDING;
    }
    else if ((pom_ptr->learning ==FALSE) && (pom_ptr->forwarding ==TRUE))
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return XSTP_TYPE_RETURN_OK;

}/* End of XSTP_OM_GetMstPortState_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpInstanceVlanMapped_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of map VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   om_ptr                -- the pointer of the instance entry.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T XSTP_OM_GetMstpInstanceVlanMapped_(XSTP_OM_InstanceData_T *om_ptr,
                                                  XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    memset(mstp_instance_entry , 0 , sizeof(XSTP_MGR_MstpInstanceEntry_T));

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    mstp_instance_entry->mstp_instance_remaining_hop_count = om_ptr->bridge_info.root_times.remaining_hops;
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    memcpy( (UI8_T *)&(mstp_instance_entry->mstp_instance_vlans_mapped),
            (UI8_T *)&(om_ptr->instance_vlans_mapped[0]), 128);
    memcpy( (UI8_T *)&(mstp_instance_entry->mstp_instance_vlans_mapped2k),
            (UI8_T *)&(om_ptr->instance_vlans_mapped[128]), 128);
    memcpy( (UI8_T *)&(mstp_instance_entry->mstp_instance_vlans_mapped3k),
            (UI8_T *)&(om_ptr->instance_vlans_mapped[256]), 128);
    memcpy( (UI8_T *)&(mstp_instance_entry->mstp_instance_vlans_mapped4k),
            (UI8_T *)&(om_ptr->instance_vlans_mapped[384]), 128);

    return TRUE;

}/* End of XSTP_OM_GetMstpInstanceVlanMapped_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstpInstanceVlanConfiguration_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of map VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   om_ptr                -- the pointer of the instance entry.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T XSTP_OM_GetMstpInstanceVlanConfiguration_(XSTP_OM_InstanceData_T *om_ptr,
                                                         XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry)
{
    UI32_T                  vid;
    UI32_T                  vlan_mstid;
    UI32_T                  mstid;


    memset(mstp_instance_entry , 0 , sizeof(XSTP_MGR_MstpInstanceEntry_T));
    mstid = om_ptr->instance_id;

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    mstp_instance_entry->mstp_instance_remaining_hop_count = om_ptr->bridge_info.root_times.remaining_hops;
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
#if 0 /* Follow Cisco, instance 0 will not show vlans that had been mapped to other instances. */
    if (mstid == XSTP_TYPE_CISTID)
    {
         for(index=0; index<128; index++)
         {
            mstp_instance_entry->mstp_instance_vlans_mapped[index]      = 0xFF;
            mstp_instance_entry->mstp_instance_vlans_mapped2k[index]    = 0xFF;
            mstp_instance_entry->mstp_instance_vlans_mapped3k[index]    = 0xFF;
            if(index != 127)
            {
                mstp_instance_entry->mstp_instance_vlans_mapped4k[index]   = 0xFF;
            }
            else
            {
                mstp_instance_entry->mstp_instance_vlans_mapped4k[index]   = 0x7F;
            }
        }
    }
    else
#endif
    {
        for(vid =1; vid <= XSTP_TYPE_SYS_MAX_VLAN_ID; vid++)
        {
            XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local(vid, &vlan_mstid);
            if (vlan_mstid == mstid)
            {
                if ((0 < vid) && (vid < 1024))
                {
                    mstp_instance_entry->mstp_instance_vlans_mapped[vid>>3] = mstp_instance_entry->mstp_instance_vlans_mapped[vid>>3] | (0x01<<(vid%8));
                }
                else if ((1023 < vid) && (vid < 2048))
                {
                    mstp_instance_entry->mstp_instance_vlans_mapped2k[(vid>>3)-128] = mstp_instance_entry->mstp_instance_vlans_mapped2k[(vid>>3)-128] | (0x01<<(vid%8));
                }
                else if ((2047 < vid) && (vid < 3072))
                {
                    mstp_instance_entry->mstp_instance_vlans_mapped3k[(vid>>3)-256] = mstp_instance_entry->mstp_instance_vlans_mapped3k[(vid>>3)-256] | (0x01<<(vid%8));
                }
                else if ((3071 < vid) && (vid < 4096))
                {
                    mstp_instance_entry->mstp_instance_vlans_mapped4k[(vid>>3)-384] = mstp_instance_entry->mstp_instance_vlans_mapped4k[(vid>>3)-384] | (0x01<<(vid%8));
                }
            }
        }
    }
    return TRUE;

}/* End of XSTP_OM_GetMstpInstanceVlanConfiguration_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetDot1dMstPortEntry_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   om_ptr                      -- the pointer of the instance
 *                                             entry.
 *              port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T XSTP_OM_GetDot1dMstPortEntry_(XSTP_OM_InstanceData_T *om_ptr,
                                             XSTP_MGR_Dot1dStpPortEntry_T *port_entry)
{
    UI32_T                  lport;
    UI32_T                  port_role;
    XSTP_OM_PortVar_T       *pom_ptr;

    lport = (UI32_T)port_entry->dot1d_stp_port;
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        return FALSE;
    }
    if (!XSTP_OM_IsMemberPortOfInstance(om_ptr, lport))
    {
        return FALSE;
    }

    XSTP_OM_GetMstPortRole_(om_ptr, lport, &port_role);
    pom_ptr= &(om_ptr->port_info[lport-1]);
    XSTP_OM_GET_PORT_ID_PRIORITY(port_entry->dot1d_stp_port_priority, pom_ptr->port_id);

    /* get port stste */
    if (!pom_ptr->common->port_enabled )
    {
        port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_DISABLED;
    }
    else if (!pom_ptr->common->link_up )
    {
        port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_BROKEN;
    }
    else if ((pom_ptr->learning) == FALSE && (pom_ptr->forwarding) == FALSE )
    {
        switch (pom_ptr->role)
        {
            case XSTP_ENGINE_PORTVAR_ROLE_DISABLED:
                port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_DISABLED;
                break;
            case XSTP_ENGINE_PORTVAR_ROLE_BACKUP:
            case XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE:
                port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_BLOCKING;
                break;
            case XSTP_ENGINE_PORTVAR_ROLE_ROOT:
            case XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED:
#ifdef XSTP_TYPE_PROTOCOL_MSTP
            case XSTP_ENGINE_PORTVAR_ROLE_MASTER:
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
                port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_LISTENING;
                break;
        }
    }
    else if ((pom_ptr->learning) == TRUE && (pom_ptr->forwarding) == FALSE )
    {
        port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_LEARNING;
    }
    else if ((pom_ptr->learning) == TRUE && (pom_ptr->forwarding) == TRUE )
    {
        port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_FORWARDING;
    }
    else
    {
        return FALSE;
    }

    if (pom_ptr->common->port_enabled)
    {
        port_entry->dot1d_stp_port_enable   = (UI8_T )XSTP_TYPE_PORT_ENABLED;
    }
    else
    {
        port_entry->dot1d_stp_port_enable   = (UI8_T )XSTP_TYPE_PORT_DISABLED;
    }

    if (!pom_ptr->common->link_up)
    {
        memcpy(&pom_ptr->port_priority, &pom_ptr->designated_priority,  sizeof(XSTP_TYPE_PriorityVector_T) );
    }

#ifdef  XSTP_TYPE_PROTOCOL_RSTP
    port_entry->dot1d_stp_port_path_cost        = port_entry->oper_path_cost
                                                = port_entry->oper_external_path_cost
                                                = (UI32_T)pom_ptr->port_path_cost;
    if (pom_ptr->static_path_cost)
    {
        port_entry->admin_path_cost = port_entry->admin_external_path_cost
                                    = port_entry->oper_path_cost;
    }
    else
    {
        port_entry->admin_path_cost = port_entry->admin_external_path_cost
                                    = 0;
    }
    port_entry->mstp_hello_time                 = (UI32_T)(pom_ptr->port_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
    port_entry->mstp_port_hello_time            = (UI32_T)(om_ptr->bridge_info.bridge_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
    port_entry->mstp_boundary_port              = FALSE;
    port_entry->mstp_internal_port_path_cost    = (UI32_T)pom_ptr->port_path_cost;
    memcpy( (UI8_T *)&(port_entry->dot1d_stp_port_designated_root),
            (UI8_T *)&(pom_ptr->port_priority.root_bridge_id), XSTP_TYPE_BRIDGE_ID_LENGTH);

    port_entry->dot1d_stp_port_designated_cost  = (UI32_T)pom_ptr->port_priority.root_path_cost;
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    port_entry->dot1d_stp_port_path_cost        = port_entry->oper_external_path_cost
                                                = (UI32_T)pom_ptr->common->external_port_path_cost;
    port_entry->mstp_internal_port_path_cost    = port_entry->oper_path_cost
                                                = (UI32_T)pom_ptr->internal_port_path_cost;
    if (pom_ptr->common->static_external_path_cost)
    {
        port_entry->admin_external_path_cost = port_entry->oper_external_path_cost;
    }
    else
    {
        port_entry->admin_external_path_cost = 0;
    }
    if (pom_ptr->static_internal_path_cost)
    {
        port_entry->admin_path_cost = port_entry->oper_path_cost;
    }
    else
    {
        port_entry->admin_path_cost = 0;
    }
    port_entry->mstp_boundary_port              = (!pom_ptr->common->rcvd_internal);
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
    {
        memcpy( (UI8_T *)&(port_entry->dot1d_stp_port_designated_root),
                (UI8_T *)&(pom_ptr->port_priority.root_id), XSTP_TYPE_BRIDGE_ID_LENGTH);

        port_entry->dot1d_stp_port_designated_cost = (UI32_T)pom_ptr->port_priority.ext_root_path_cost;
        port_entry->mstp_hello_time                = (UI32_T)(pom_ptr->port_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
        port_entry->mstp_port_hello_time           = (UI32_T)(om_ptr->bridge_info.bridge_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
    }
    else
    {
        memcpy( (UI8_T *)&(port_entry->dot1d_stp_port_designated_root),
                (UI8_T *)&(pom_ptr->port_priority.r_root_id), XSTP_TYPE_BRIDGE_ID_LENGTH);

        port_entry->dot1d_stp_port_designated_cost = (UI32_T)pom_ptr->port_priority.int_root_path_cost;
        port_entry->mstp_hello_time                = (UI32_T)(pom_ptr->cist->port_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
        port_entry->mstp_port_hello_time           = (UI32_T)(om_ptr->bridge_info.cist->bridge_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    memcpy( (UI8_T *)&(port_entry->dot1d_stp_port_designated_bridge),
            (UI8_T *)&(pom_ptr->port_priority.designated_bridge_id), XSTP_TYPE_BRIDGE_ID_LENGTH);

    memcpy( (UI8_T *)&(port_entry->dot1d_stp_port_designated_port),
            (UI8_T *)&(pom_ptr->port_priority.designated_port_id), XSTP_TYPE_PORT_ID_LENGTH);

    port_entry->port_role = port_role;

    port_entry->dot1d_stp_port_forward_transitions
                                           = (UI16_T)pom_ptr->port_forward_transitions;
    return TRUE;

}/* End of XSTP_OM_GetDot1dMstPortEntry_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetDot1dMstPortEntry_LinkdownAsBlocking_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   om_ptr                      -- the pointer of the instance
 *                                             entry.
 *              port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T XSTP_OM_GetDot1dMstPortEntry_LinkdownAsBlocking_(XSTP_OM_InstanceData_T *om_ptr,
                                                                XSTP_MGR_Dot1dStpPortEntry_T *port_entry)
{
    UI32_T                  lport;
    UI32_T                  port_role;
    XSTP_OM_PortVar_T       *pom_ptr;

    lport = (UI32_T)port_entry->dot1d_stp_port;
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        return FALSE;
    }
    if (!XSTP_OM_IsMemberPortOfInstance(om_ptr, lport))
    {
        return FALSE;
    }

    XSTP_OM_GetMstPortRole_(om_ptr, lport, &port_role);
    pom_ptr= &(om_ptr->port_info[lport-1]);
    XSTP_OM_GET_PORT_ID_PRIORITY(port_entry->dot1d_stp_port_priority, pom_ptr->port_id);

    /* get port stste */
    if (!pom_ptr->common->port_enabled )
    {
        port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_DISABLED;
    }
    else if (!pom_ptr->common->link_up )
    {
        port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_BLOCKING;
    }
    else if ((pom_ptr->learning) == FALSE && (pom_ptr->forwarding) == FALSE )
    {
        switch (pom_ptr->role)
        {
            case XSTP_ENGINE_PORTVAR_ROLE_DISABLED:
                port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_DISABLED;
                break;
            case XSTP_ENGINE_PORTVAR_ROLE_BACKUP:
            case XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE:
                port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_BLOCKING;
                break;
            case XSTP_ENGINE_PORTVAR_ROLE_ROOT:
            case XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED:
#ifdef XSTP_TYPE_PROTOCOL_MSTP
            case XSTP_ENGINE_PORTVAR_ROLE_MASTER:
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
                port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_LISTENING;
                break;
        }
    }
    else if ((pom_ptr->learning) == TRUE && (pom_ptr->forwarding) == FALSE )
    {
        port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_LEARNING;
    }
    else if ((pom_ptr->learning) == TRUE && (pom_ptr->forwarding) == TRUE )
    {
        port_entry->dot1d_stp_port_state = (UI8_T )XSTP_TYPE_PORT_STATE_FORWARDING;
    }
    else
    {
        return FALSE;
    }

    if (pom_ptr->common->port_enabled)
    {
        port_entry->dot1d_stp_port_enable   = (UI8_T )XSTP_TYPE_PORT_ENABLED;
    }
    else
    {
        port_entry->dot1d_stp_port_enable   = (UI8_T )XSTP_TYPE_PORT_DISABLED;
    }

    if (!pom_ptr->common->link_up)
    {
        memcpy(&pom_ptr->port_priority, &pom_ptr->designated_priority,  sizeof(XSTP_TYPE_PriorityVector_T) );
    }

#ifdef  XSTP_TYPE_PROTOCOL_RSTP
    port_entry->dot1d_stp_port_path_cost        = port_entry->oper_path_cost
                                                = port_entry->oper_external_path_cost
                                                = (UI32_T)pom_ptr->port_path_cost;
    if (pom_ptr->static_path_cost)
    {
        port_entry->admin_path_cost = port_entry->admin_external_path_cost
                                    = port_entry->oper_path_cost;
    }
    else
    {
        port_entry->admin_path_cost = port_entry->admin_external_path_cost
                                    = 0;
    }
    port_entry->mstp_hello_time                 = (UI32_T)(pom_ptr->port_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
    port_entry->mstp_port_hello_time            = (UI32_T)(om_ptr->bridge_info.bridge_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
    port_entry->mstp_boundary_port              = FALSE;
    port_entry->mstp_internal_port_path_cost    = (UI32_T)pom_ptr->port_path_cost;
    memcpy( (UI8_T *)&(port_entry->dot1d_stp_port_designated_root),
            (UI8_T *)&(pom_ptr->port_priority.root_bridge_id), XSTP_TYPE_BRIDGE_ID_LENGTH);

    port_entry->dot1d_stp_port_designated_cost  = (UI32_T)pom_ptr->port_priority.root_path_cost;
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    port_entry->dot1d_stp_port_path_cost        = port_entry->oper_external_path_cost
                                                = (UI32_T)pom_ptr->common->external_port_path_cost;
    port_entry->mstp_internal_port_path_cost    = port_entry->oper_path_cost
                                                = (UI32_T)pom_ptr->internal_port_path_cost;
    if (pom_ptr->common->static_external_path_cost)
    {
        port_entry->admin_external_path_cost = port_entry->oper_external_path_cost;
    }
    else
    {
        port_entry->admin_external_path_cost = 0;
    }
    if (pom_ptr->static_internal_path_cost)
    {
        port_entry->admin_path_cost = port_entry->oper_path_cost;
    }
    else
    {
        port_entry->admin_path_cost = 0;
    }
    port_entry->mstp_boundary_port              = (!pom_ptr->common->rcvd_internal);
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
    {
        memcpy( (UI8_T *)&(port_entry->dot1d_stp_port_designated_root),
                (UI8_T *)&(pom_ptr->port_priority.root_id), XSTP_TYPE_BRIDGE_ID_LENGTH);

        port_entry->dot1d_stp_port_designated_cost = (UI32_T)pom_ptr->port_priority.ext_root_path_cost;
        port_entry->mstp_hello_time                = (UI32_T)(pom_ptr->port_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
        port_entry->mstp_port_hello_time           = (UI32_T)(om_ptr->bridge_info.bridge_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
    }
    else
    {
        memcpy( (UI8_T *)&(port_entry->dot1d_stp_port_designated_root),
                (UI8_T *)&(pom_ptr->port_priority.r_root_id), XSTP_TYPE_BRIDGE_ID_LENGTH);

        port_entry->dot1d_stp_port_designated_cost = (UI32_T)pom_ptr->port_priority.int_root_path_cost;
        port_entry->mstp_hello_time                = (UI32_T)(pom_ptr->cist->port_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
        port_entry->mstp_port_hello_time           = (UI32_T)(om_ptr->bridge_info.cist->bridge_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    memcpy( (UI8_T *)&(port_entry->dot1d_stp_port_designated_bridge),
            (UI8_T *)&(pom_ptr->port_priority.designated_bridge_id), XSTP_TYPE_BRIDGE_ID_LENGTH);

    memcpy( (UI8_T *)&(port_entry->dot1d_stp_port_designated_port),
            (UI8_T *)&(pom_ptr->port_priority.designated_port_id), XSTP_TYPE_PORT_ID_LENGTH);

    port_entry->port_role = port_role;

    port_entry->dot1d_stp_port_forward_transitions
                                           = (UI16_T)pom_ptr->port_forward_transitions;

    return TRUE;
}/* End of XSTP_OM_GetDot1dMstPortEntry_LinkdownAsBlocking_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetDot1dMstExtPortEntry_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   om_ptr                -- the pointer of the instance entry.
 *              lport                 -- lport number
 * OUTPUT   :   *ext_port_entry       -- pointer of the specified
 *                                                 port ext_entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T XSTP_OM_GetDot1dMstExtPortEntry_( XSTP_OM_InstanceData_T *om_ptr,
                                                UI32_T lport,
                                                XSTP_MGR_Dot1dStpExtPortEntry_T *ext_port_entry)
{
    XSTP_OM_PortVar_T       *pom_ptr;

    if (!XSTP_OM_IsMemberPortOfInstance(om_ptr, lport))
    {
/*        EH_MGR_Handle_Exception2(SYS_MODULE_XSTP, XSTP_OM_GetDot1dMstExtPortEntry_Fun_No, EH_TYPE_MSG_NOT_MEMBER, SYSLOG_LEVEL_INFO, "Port", "specified instance ID");*/
        return FALSE;
    }
    memset(ext_port_entry , 0 , sizeof(XSTP_MGR_Dot1dStpExtPortEntry_T));
    pom_ptr= &(om_ptr->port_info[lport-1]);
    if (pom_ptr->common->mcheck)
    {
        ext_port_entry->dot1d_stp_port_protocol_migration   = XSTP_TYPE_PORT_PROTOCOL_MIGRATION_ENABLED;
    }
    else
    {
        ext_port_entry->dot1d_stp_port_protocol_migration   = XSTP_TYPE_PORT_PROTOCOL_MIGRATION_DISABLED;
    }
    if (pom_ptr->common->auto_edge)
    {
        ext_port_entry->dot1d_stp_port_admin_edge_port      = (UI8_T)XSTP_TYPE_PORT_ADMIN_EDGE_PORT_AUTO;
    }
    else if (pom_ptr->common->admin_edge)
    {
        ext_port_entry->dot1d_stp_port_admin_edge_port      = (UI8_T)XSTP_TYPE_PORT_ADMIN_EDGE_PORT_ENABLED;
    }
    else
    {
        ext_port_entry->dot1d_stp_port_admin_edge_port      = (UI8_T)XSTP_TYPE_PORT_ADMIN_EDGE_PORT_DISABLED;
    }
    if (pom_ptr->common->oper_edge)
    {
        ext_port_entry->dot1d_stp_port_oper_edge_port       = (UI8_T)XSTP_TYPE_PORT_ADMIN_EDGE_PORT_ENABLED;
    }
    else
    {
        ext_port_entry->dot1d_stp_port_oper_edge_port       = (UI8_T)XSTP_TYPE_PORT_ADMIN_EDGE_PORT_DISABLED;
    }
    if (pom_ptr->common->admin_point_to_point_mac_auto)
    {
        ext_port_entry->dot1d_stp_port_admin_point_to_point =(UI8_T)XSTP_TYPE_PORT_ADMIN_LINK_TYPE_AUTO;
    }
    else if(pom_ptr->common->admin_point_to_point_mac)
    {
        ext_port_entry->dot1d_stp_port_admin_point_to_point =(UI8_T)XSTP_TYPE_PORT_ADMIN_LINK_TYPE_POINT_TO_POINT;
    }
    else if (!pom_ptr->common->admin_point_to_point_mac)
    {
        ext_port_entry->dot1d_stp_port_admin_point_to_point =(UI8_T)XSTP_TYPE_PORT_ADMIN_LINK_TYPE_SHARED;
    }
    if (pom_ptr->common->oper_point_to_point_mac)
    {
        ext_port_entry->dot1d_stp_port_oper_point_to_point  = (UI8_T)XSTP_TYPE_PORT_OPER_LINK_TYPE_POINT_TO_POINT;
    }
    else
    {
        ext_port_entry->dot1d_stp_port_oper_point_to_point  = (UI8_T)XSTP_TYPE_PORT_OPER_LINK_TYPE_SHARED;
    }
#ifdef  XSTP_TYPE_PROTOCOL_RSTP
    ext_port_entry->dot1d_stp_port_long_path_cost           =  ext_port_entry->port_oper_long_path_cost
                                                            =  (UI32_T)pom_ptr->port_path_cost;
    if (pom_ptr->static_path_cost)
    {
        ext_port_entry->port_admin_long_path_cost = ext_port_entry->port_oper_long_path_cost;
    }
    else
    {
        ext_port_entry->port_admin_long_path_cost = 0;
    }
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    ext_port_entry->dot1d_stp_port_long_path_cost       = ext_port_entry->port_oper_long_path_cost
                                                        = (UI32_T)pom_ptr->common->external_port_path_cost;
    if (pom_ptr->common->static_external_path_cost)
    {
        ext_port_entry->port_admin_long_path_cost = ext_port_entry->port_oper_long_path_cost;
    }
    else
    {
        ext_port_entry->port_admin_long_path_cost = 0;
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    ext_port_entry->dot1d_stp_port_spanning_tree_status     = (UI8_T)pom_ptr->common->port_spanning_tree_status;
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
    if (pom_ptr->common->root_guard_status == TRUE)
        ext_port_entry->port_root_guard_status              = XSTP_TYPE_PORT_ROOT_GUARD_ENABLED;
    else
        ext_port_entry->port_root_guard_status              = XSTP_TYPE_PORT_ROOT_GUARD_DISABLED;
#endif
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    ext_port_entry->port_bpdu_guard_status = ((pom_ptr->common->bpdu_guard_status == TRUE)
        ? XSTP_TYPE_PORT_BPDU_GUARD_ENABLED : XSTP_TYPE_PORT_BPDU_GUARD_DISABLED);
    ext_port_entry->port_bpdu_guard_auto_recovery = pom_ptr->common->bpdu_guard_auto_recovery;
    ext_port_entry->port_bpdu_guard_auto_recovery_interval = pom_ptr->common->bpdu_guard_auto_recovery_interval;
#endif
#if(SYS_CPNT_STP_BPDU_FILTER == TRUE)
    ext_port_entry->port_bpdu_filter_status = ((pom_ptr->common->bpdu_filter_status == TRUE)
        ? XSTP_TYPE_PORT_BPDU_FILTER_ENABLED : XSTP_TYPE_PORT_BPDU_FILTER_DISABLED);
#endif
#if(SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
    ext_port_entry->tc_prop_stop= pom_ptr->common->tc_prop_stop;
#endif
    return TRUE;
}/* End of XSTP_OM_GetDot1dMstExtPortEntry_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextExistingInstance_
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next existed MST instance(active) for mst mapping table.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T XSTP_OM_GetNextExistingInstance_(UI32_T *mstid)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
/*    UI8_T                   arg_buf[20];*/

    result      = FALSE;

    if ( (*mstid < 0 || *mstid > XSTP_TYPE_MAX_MSTID) && (*mstid!=XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP))
    {
/*
        sprintf(arg_buf, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_OM_GetNextExistedInstance_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
*/
        return FALSE;
    }
    if (*mstid == XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP)
    {
        *mstid  = 0;
        result  = TRUE;
    }
    else
    {
        while(      XSTP_OM_GetNextInstanceInfoPtr(mstid, &om_ptr)
                &&  (!om_ptr->instance_exist)
             )
        {
            /* Do nothing */
        }
        result = om_ptr->instance_exist;
    }

    return result;
}/* End of XSTP_OM_GetNextExistingInstance_() */

/*=========================================================================
 * Utility
 *=========================================================================
 */
static void XSTP_OM_LSB_To_MSB(UI8_T *data, UI32_T data_len)
{
   UI32_T i = 0;
   UI8_T  j = 0;

   for (i = 0; i < data_len ; i ++)
   {
      for (j = 0; j < 4 ; j++)
      {  /*if bit not the same will swap this two bit*/
         if (((data[i] & (UI32_T) 1<<(7-j))>>(7-j)) != ((data[i] & (UI32_T) 1<<j)>>j))
         {
            if (data[i] & (UI32_T) 1<<(7-j))/*if bit value = 1*/
            {
               data[i] |= (UI32_T) 1<<j;
               data[i] &= ~((UI32_T) 1<<(7-j));
            }
            else
            {
               data[i] &= ~((UI32_T) 1<<j);
               data[i] |= (UI32_T) 1<<(7-j);
            }
         }
      }
   }

   return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mstid value form mst configuration table for a specified
 *            vlan.
 * INPUT    : vid       -- vlan number
 *            mstid     -- mstid value point
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local(UI32_T vid, UI32_T *mstid)
{
    UI8_T   odd_byte, even_byte;

    *mstid       =   0;
    if (    (vid > 0)
        &&  (vid <= SYS_DFLT_DOT1QMAXVLANID )
       )
    {
        even_byte   =   XSTP_OM_Mst_Configuration_Table[2*vid];
        odd_byte    =   XSTP_OM_Mst_Configuration_Table[2*vid+1];

        *mstid = (((*mstid | even_byte) << 8 ) | odd_byte );
    }

    return;
} /* End of XSTP_OM_GetMstidFromMstConfigurationTableByVlan_Local */
BOOL_T  XSTP_OM_GetMappedInstanceByVlan(UI32_T vid,
                                         UI32_T *mstid)
{
 UI32_T                  current_st_status;
 UI32_T                  current_st_mode;

 if(!mstid)
   return FALSE;

    XSTP_OM_EnterCriticalSection(*mstid);
    current_st_status   = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();
    if (    (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
       )
    {
        XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vid, mstid);
    }
    else
    {
        *mstid = XSTP_TYPE_CISTID;
    }

   XSTP_OM_LeaveCriticalSection(*mstid);
   //SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
 return TRUE;

}

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get root guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- status value.
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPortRootGuardStatus(UI32_T lport, UI32_T *status)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP,
                                 XSTP_MGR_GetPortRootGuardStatus_Fun_No,
                                 EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                 "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");*/
        return XSTP_TYPE_RETURN_ERROR;
    }

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);
    if (pom_ptr->common->root_guard_status == TRUE)
        *status = XSTP_TYPE_PORT_ROOT_GUARD_ENABLED;
    else
        *status = XSTP_TYPE_PORT_ROOT_GUARD_DISABLED;

    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_OM_GetPortRootGuardStatus() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get running root guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- admin status value.
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :   None.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortRootGuardStatus(UI32_T lport, UI32_T *status)
{
    XSTP_OM_InstanceData_T          *om_ptr;
    XSTP_OM_PortVar_T               *pom_ptr;

    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
/*        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP,
                                 XSTP_MGR_GetRunningPortRootGuardStatus_Fun_No,
                                 EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                 "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");*/
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    if (pom_ptr->common->root_guard_status == TRUE)
        *status = XSTP_TYPE_PORT_ROOT_GUARD_ENABLED;
    else
        *status = XSTP_TYPE_PORT_ROOT_GUARD_DISABLED;

    if (*status == XSTP_TYPE_DEFAULT_PORT_ROOT_GUARD_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* End of XSTP_OM_GetRunningPortRootGuardStatus() */
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortBpduGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU guard status on the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPortBpduGuardStatus(UI32_T lport, UI32_T *status)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (   (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
        || (status == NULL)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    *status = ((pom_ptr->common->bpdu_guard_status == TRUE)
        ? XSTP_TYPE_PORT_BPDU_GUARD_ENABLED : XSTP_TYPE_PORT_BPDU_GUARD_DISABLED);

    return XSTP_TYPE_RETURN_OK;
} /* XSTP_OM_GetPortBpduGuardStatus() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortBpduGuardStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get running BPDU Guard status on the specified port.
 * INPUT    : lport     -- the logical port number
 * OUTPUT   : status    -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : (interface function)
 *------------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortBpduGuardStatus(UI32_T lport, UI32_T *status)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (   (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
        || (status == NULL)
       )
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    *status = ((pom_ptr->common->bpdu_guard_status == TRUE)
        ? XSTP_TYPE_PORT_BPDU_GUARD_ENABLED : XSTP_TYPE_PORT_BPDU_GUARD_DISABLED);

    /*compare current value with default one */
    if (*status == XSTP_TYPE_DEFAULT_PORT_BPDU_GUARD_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* XSTP_OM_GetRunningPortBpduGuardStatus() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortBPDUGuardAutoRecovery
 * ------------------------------------------------------------------------
 * PURPOSE : Get the BPDU guard auto recovery status on the specified port.
 * INPUT   : lport -- lport number
 * OUTPUT  : status -- the status value
 * RETURN  : XSTP_TYPE_RETURN_CODE_E
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPortBPDUGuardAutoRecovery(UI32_T lport, UI32_T  *status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    /* BODY
     */

    if ((lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT))
    {
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    if (status == NULL)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);
    *status = pom_ptr->common->bpdu_guard_auto_recovery;

    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_OM_GetPortBPDUGuardAutoRecovery */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortBPDUGuardAutoRecovery
 *-------------------------------------------------------------------------
 * PURPOSE : Get running BPDU guard auto recovery status for the specified
 *           port.
 * INPUT   : lport -- the logical port number
 * OUTPUT  : status -- the status value
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortBPDUGuardAutoRecovery(UI32_T lport,
                                                   UI32_T *status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    /* BODY
     */

    if (    (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
         || (status == NULL)
       )
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);
    *status = pom_ptr->common->bpdu_guard_auto_recovery;

    if (*status == XSTP_TYPE_DEFAULT_PORT_BPDU_GUARD_AUTO_RECOVERY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* End of XSTP_OM_GetRunningPortBPDUGuardAutoRecovery */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortBPDUGuardAutoRecoveryInterval
 * ------------------------------------------------------------------------
 * PURPOSE : Get the BPDU guard auto recovery interval on the specified
 *           port.
 * INPUT   : lport -- lport number
 * OUTPUT  : interval -- the interval value
 * RETURN  : XSTP_TYPE_RETURN_CODE_E
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetPortBPDUGuardAutoRecoveryInterval(UI32_T lport,
                                                    UI32_T *interval)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    /* BODY
     */

    if ((lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT))
    {
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    if (interval == NULL)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);
    *interval = pom_ptr->common->bpdu_guard_auto_recovery_interval;

    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_OM_GetPortBPDUGuardAutoRecoveryInterval */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortBPDUGuardAutoRecoveryInterval
 *-------------------------------------------------------------------------
 * PURPOSE : Get running BPDU guard auto recovery interval for the specified
 *           port.
 * INPUT   : lport -- the logical port number
 * OUTPUT  : interval -- the status value
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortBPDUGuardAutoRecoveryInterval(UI32_T lport,
                                                           UI32_T *interval)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    /* BODY
     */

    if (    (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
         || (interval == NULL)
       )
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);
    *interval = pom_ptr->common->bpdu_guard_auto_recovery_interval;

    if (*interval == XSTP_TYPE_DEFAULT_PORT_BPDU_GUARD_AUTO_RECOVERY_INTERVAL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* End of XSTP_OM_GetRunningPortBPDUGuardAutoRecoveryInterval */
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPortBpduFilterStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU filter status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPortBpduFilterStatus(UI32_T lport, UI32_T *status)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (    (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
         || (status == NULL)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    *status = ((pom_ptr->common->bpdu_filter_status == TRUE)
        ? XSTP_TYPE_PORT_BPDU_FILTER_ENABLED : XSTP_TYPE_PORT_BPDU_FILTER_DISABLED);

    return XSTP_TYPE_RETURN_OK;
} /* XSTP_OM_GetPortBpduFilterStatus() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningPortBpduFilterStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get per port variable bpdu_filter_status value.
 * INPUT    : lport     -- the logical port number
 * OUTPUT   : status    -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : (interface function)
 *------------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningPortBpduFilterStatus(UI32_T lport, UI32_T *status)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (    (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
         || (status == NULL)
       )
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    *status = ((pom_ptr->common->bpdu_filter_status == TRUE)
        ? XSTP_TYPE_PORT_BPDU_FILTER_ENABLED : XSTP_TYPE_PORT_BPDU_FILTER_DISABLED);

    /*compare current value with default one */
    if (*status == XSTP_TYPE_DEFAULT_PORT_BPDU_FILTER)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* XSTP_OM_GetRunningPortBpduFilterStatus() */
#endif /* #if (SYS_CPNT_STP_BPDU_FILTER == TRUE) */

#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetCiscoPrestandardCompatibility
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the cisco prestandard compatibility status
 * INPUT    :   status    -- the status value
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_SetCiscoPrestandardCompatibility(UI32_T status)
{
    XSTP_OM_SystemInfo.cisco_prestandard = status;
    return;
}/* End of XSTP_OM_SetCiscoPrestandardCompatibility() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetCiscoPrestandardCompatibility
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the cisco prestandard compatibility status
 * INPUT    :   UI32_T status           -- the status value
 * OUTPUT   :   UI32_T status           -- the status value
 * RETURN   :   None
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_GetCiscoPrestandardCompatibility(UI32_T *status)
{
    *status = XSTP_OM_SystemInfo.cisco_prestandard;
    return;
}/* End of XSTP_OM_GetCiscoPrestandardCompatibility() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetRunningCiscoPrestandardCompatibility
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the cisco prestandard compatibility status.
 * INPUT    : UI32_T status           -- the status value
 * OUTPUT   : UI32_T status           -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    :
 *------------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetRunningCiscoPrestandardCompatibility(UI32_T *status)
{
    if (status == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status = XSTP_OM_SystemInfo.cisco_prestandard;

    /*compare current value with default one */
    if (*status == XSTP_TYPE_DEFAULT_CISCO_PRESTANDARD_COMPATIBILITY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* End of XSTP_MGR_GetCiscoPrestandardCompatibility() */
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetEthRingPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : To get the ring port status.
 * INPUT    : lport    -- lport number to check
 *            vid      -- VLAN id to check
 * OUTPUT   : is_blk_p -- pointer to content of blocking status
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetEthRingPortStatus(
    UI32_T  lport,
    UI32_T  vid,
    BOOL_T  *is_blk_p)
{
    BOOL_T  ret = FALSE;

#if (SYS_CPNT_EAPS == TRUE)
    {
        UI32_T  rp_role;

        if (TRUE == XSTP_OM_GetEthRingPortRole(lport, &rp_role))
        {
            switch (rp_role)
            {
#if (SYS_CPNT_EAPS == TRUE)
            case XSTP_TYPE_ETH_RING_PORT_ROLE_EAPS:
                *is_blk_p = EAPS_OM_IsPortBlockingByVLAN(vid, lport);
                ret = TRUE;
                break;
#endif

            default:
                break;
            }
        }
    }
#endif /* #if (SYS_CPNT_EAPS == TRUE) */

    return ret;
}

#if (SYS_CPNT_EAPS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetEthRingPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   To get the port role for eth ring protocol.
 * INPUT    :   lport       -- lport number (1-based)
 * OUTPUT   :   port_role_p -- pointer to content of port role
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetEthRingPortRole(
    UI32_T  lport,
    UI32_T  *port_role_p)
{
    BOOL_T  ret = FALSE;

    if (  (NULL != port_role_p)
        &&(1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
       )
    {
        *port_role_p = XSTP_OM_EthRingProle[lport-1];
        ret = TRUE;
    }

//    printf(" %s:%d, lport/port_role/ret-%ld/%ld/%d\r\n",
//        __FUNCTION__, __LINE__, lport, *port_role_p, ret);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetEthRingPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   To set the port role for eth ring protocol.
 * INPUT    :   lport     -- lport number (1-based)
 *              port_role -- port role to set
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_SetEthRingPortRole(
    UI32_T  lport,
    UI32_T  port_role)
{
    BOOL_T  ret = FALSE;

    if ((1 <= lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        /* port role:  none           to EAPS/ERPS/none
         *             EAPS/ERPS/none to none
         */
        if (XSTP_TYPE_ETH_RING_PORT_ROLE_NONE == XSTP_OM_EthRingProle[lport-1])
        {
            XSTP_OM_EthRingProle[lport-1] = port_role;
            ret = TRUE;
        }
        else
        {
            if (XSTP_TYPE_ETH_RING_PORT_ROLE_NONE == port_role)
            {
                XSTP_OM_EthRingProle[lport-1] = XSTP_TYPE_ETH_RING_PORT_ROLE_NONE;
                ret = TRUE;
            }
        }
    }

    return ret;
}

#endif /* End of #if (SYS_CPNT_EAPS == TRUE) */

/* FUNCTION NAME - XSTP_OM_SetTcCausePort
 * PURPOSE : Set the logical port which causes the topology change. (becomes
 *           forwarding from non-forwarding or receives BPDU with TC flag)
 * INPUT   : lport - the logical port causing topology change
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void  XSTP_OM_SetTcCausePort(UI32_T lport)
{
    XSTP_OM_SystemInfo.tc_cause_port = lport;
}

/* FUNCTION NAME - XSTP_OM_GetTcCausePort
 * PURPOSE : Get the logical port which causes the topology change. (becomes
 *           forwarding from non-forwarding or receives BPDU with TC flag)
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : the logical port causing topology change
 * NOTES   : None
 */
UI32_T  XSTP_OM_GetTcCausePort(void)
{
    return XSTP_OM_SystemInfo.tc_cause_port;
}

/* FUNCTION NAME - XSTP_OM_SetTcCausePortAndBrdgMac
 * PURPOSE : Set the logical port which causes the topology change. (becomes
 *           forwarding from non-forwarding or receives BPDU with TC flag)
 * INPUT   : lport - the logical port causing topology change
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void  XSTP_OM_SetTcCausePortAndBrdgMac(UI32_T lport, UI8_T brdg_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    XSTP_OM_SystemInfo.tc_rx_port = lport;
    if(brdg_mac == NULL)
        memset(XSTP_OM_SystemInfo.tc_casee_brdg_mac, 0, SYS_ADPT_MAC_ADDR_LEN);
    else
        memcpy(XSTP_OM_SystemInfo.tc_casee_brdg_mac, brdg_mac, SYS_ADPT_MAC_ADDR_LEN);
}

/* FUNCTION NAME - XSTP_OM_GetTcCausePortAndBrdgMac
 * PURPOSE : Get the logical port which causes the topology change. (becomes
 *           forwarding from non-forwarding or receives BPDU with TC flag)
 * INPUT   : None
 * OUTPUT  : the logical port rx tc and bridge mac
 * RETURN  :
 * NOTES   : None
 */
void  XSTP_OM_GetTcCausePortAndBrdgMac(UI32_T *lport_p, UI8_T brdg_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    *lport_p = XSTP_OM_SystemInfo.tc_rx_port;
    memcpy(brdg_mac, XSTP_OM_SystemInfo.tc_casee_brdg_mac, SYS_ADPT_MAC_ADDR_LEN);

    return;
}


#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_AddTcPropGroupPortList
 * ------------------------------------------------------------------------
 * PURPOSE  : Add ports to a TC propagation control group.
 * INPUT    : group_id     -- group ID
 *            portbitmap   -- group member ports
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_AddTcPropGroupPortList(UI32_T group_id,
                                UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    UI32_T i;

    /* +++ EnterCriticalRegion +++ */
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);

    /* Record this new port
     */
    for(i=1; i <= SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        if(!L_BITMAP_PORT_ISSET(portbitmap, i))
            continue;

        XSTP_OM_InstanceInfo[XSTP_TYPE_CISTID].port_info[i-1].common->tc_prop_group_id = group_id;
    }

    /* +++ LeaveCriticalRegion +++ */
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_DelTcPropGroupPortList
 * ------------------------------------------------------------------------
 * PURPOSE  : Remove ports from a TC propagation control group.
 * INPUT    : group_id     -- group ID
 *            portbitmap   -- group member ports
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_DelTcPropGroupPortList(UI32_T group_id,
                                UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    UI32_T i;

    /* +++ EnterCriticalRegion +++ */
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);

    for(i=1; i <= SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        if(!L_BITMAP_PORT_ISSET(portbitmap, i))
        {
            continue;
        }

        XSTP_OM_InstanceInfo[XSTP_TYPE_CISTID].port_info[i-1].common->tc_prop_group_id
            = XSTP_TYPE_TC_PROP_DEFAULT_GROUP_ID;
    }

    /* +++ LeaveCriticalRegion +++ */
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTcPropGroupPortbitmpLocal
 * ------------------------------------------------------------------------
 * PURPOSE  : Get port list for specified group ID.
 * INPUT    : group_id     -- group ID
 * OUTPUT   : portbitmap   -- group member ports
 *            has_port_p   -- have port or not
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static BOOL_T XSTP_OM_GetTcPropGroupPortbitmpLocal(UI32_T group_id,
                                     UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                     BOOL_T *has_port_p)
{
    UI32_T i;
    UI32_T current_group_id = XSTP_TYPE_TC_PROP_DEFAULT_GROUP_ID;

    *has_port_p = FALSE;

    memset(portbitmap, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    for(i=1; i <= SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        current_group_id = XSTP_OM_InstanceInfo[XSTP_TYPE_CISTID].port_info[i-1].common->tc_prop_group_id;

        if(current_group_id == group_id)
        {
            L_BITMAP_PORT_SET(portbitmap, i);
            *has_port_p = TRUE;
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTcPropGroupPortbitmap
 * ------------------------------------------------------------------------
 * PURPOSE  : Get port list for specified group ID.
 * INPUT    : group_id     -- group ID
 * OUTPUT   : portbitmap   -- group member ports
 *            has_port_p   -- have port or not
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetTcPropGroupPortbitmap(UI32_T group_id,
                                UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                BOOL_T *has_port_p)
{
    BOOL_T ret;

    if (group_id > XSTP_TYPE_TC_PROP_MAX_GROUP_ID)
    {
        return FALSE;
    }

    /* +++ EnterCriticalRegion +++ */
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);

    ret = XSTP_OM_GetTcPropGroupPortbitmpLocal(group_id, portbitmap, has_port_p);

    /* +++ LeaveCriticalRegion +++ */
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetTcPropNextGroupPortbitmap
 * ------------------------------------------------------------------------
 * PURPOSE  : Get next group ID and port list.
 * INPUT    : group_id_p   -- group ID pointer
 * OUTPUT   : group_id_p   -- group ID pointer
 *            portbitmap   -- group member ports
 *            has_port_p   -- have port or not
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_GetTcPropNextGroupPortbitmap(UI32_T *group_id_p,
                                     UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                     BOOL_T *has_port_p)
{
    BOOL_T ret = FALSE;

    /* +++ EnterCriticalRegion +++ */
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(xstp_om_sem_id);

    if (*group_id_p < XSTP_TYPE_TC_PROP_MAX_GROUP_ID)
    {
        (*group_id_p)++;
        ret = XSTP_OM_GetTcPropGroupPortbitmpLocal(*group_id_p, portbitmap, has_port_p);
    }

    /* +++ LeaveCriticalRegion +++ */
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(xstp_om_sem_id, original_priority);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetPropGropIdByPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Get group ID for specified port.
 * INPUT    : lport   -- logical port number
 * OUTPUT   : None
 * RETURN   : group ID
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_OM_GetPropGropIdByPort(UI32_T lport)
{
  if(lport <1
     || lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
    return XSTP_TYPE_TC_PROP_DEFAULT_GROUP_ID;

  return XSTP_OM_InstanceInfo[XSTP_TYPE_CISTID].port_info[lport-1].common->tc_prop_group_id;
}
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/

BOOL_T XSTP_OM_GetPortTcInfo(UI32_T xstid, UI32_T lport, BOOL_T *is_mstp_mode,
    BOOL_T *is_root, UI32_T *tc_timer)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    I32_T                   cmp;


    if (xstid > XSTP_TYPE_MAX_MSTID)
    {
        return FALSE;
    }
    if ((lport == 0) || (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        return FALSE;
    }
    if ((is_mstp_mode == NULL) || (is_root == NULL) || (tc_timer == NULL))
    {
        return FALSE;
    }

    if (XSTP_OM_SystemInfo.force_version == XSTP_TYPE_MSTP_MODE)
    {
        *is_mstp_mode = TRUE;
    }
    else
    {
        *is_mstp_mode = FALSE;
    }

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(xstid);
    pom_ptr = &(om_ptr->port_info[lport-1]);

#ifdef XSTP_TYPE_PROTOCOL_RSTP
    XSTP_OM_CMP_BRIDGE_ID(cmp, (om_ptr->bridge_info.root_priority.root_bridge_id), (om_ptr->bridge_info.bridge_identifier));
#endif /* XSTP_TYPE_PROTOCOL_RSTP */
#ifdef XSTP_TYPE_PROTOCOL_MSTP
    if (om_ptr->bridge_info.cist == NULL)   /* CIST */
    {
        XSTP_OM_CMP_BRIDGE_ID(cmp, (om_ptr->bridge_info.root_priority.root_id), (om_ptr->bridge_info.bridge_identifier));
    }
    else    /* MSTI */
    {
        XSTP_OM_CMP_BRIDGE_ID(cmp, (om_ptr->bridge_info.root_priority.r_root_id), (om_ptr->bridge_info.bridge_identifier));
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
    if (cmp == 0)
    {
        *is_root = TRUE;
    }
    else
    {
        *is_root = FALSE;
    }

    if (    (pom_ptr->common->oper_point_to_point_mac)
        &&  (pom_ptr->common->send_rstp)
       )
    {
        *tc_timer = pom_ptr->port_times.hello_time + 1;
    }
    else
    {
        *tc_timer = om_ptr->bridge_info.root_times.max_age + om_ptr->bridge_info.root_times.forward_delay;
    }

    return TRUE;
}

UI32_T XSTP_OM_GetMstidByEntryId(UI32_T entry_id)
{
    return XSTP_OM_InstanceInfo[entry_id].instance_id;
}

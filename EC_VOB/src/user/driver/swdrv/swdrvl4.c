/*------------------------------------------------------------------------------
 * File_Name : swdrvl4.c
 *
 * Purpose   : Provide physical port APIs to SWCTRL
 *
 * 2002/10/17    : Jeff Kao Create
 * 2002/10/21    : Jeff Kao add error code & stacking function
 * 2002/10/24    : Jeff Kao move l4swdrv API. into here
 *
 * Copyright(C)      Accton Corporation, 2002, 2003
 *
 * Note    : Designed for Mercury (Common Platform 2.0)
 * history : vivid 05/28/2004 08:22
 *           for option and slave ISC_REMOTE_CALL use STKTPLG_OM_GetNextDriverUnit
 *------------------------------------------------------------------------------
 */

/*------------------------------------------------------------------------------
 * INCLUDE FILES
 *------------------------------------------------------------------------------
 */
#include "stktplg_pom.h"
#include "l_mm.h"
#include "sysfun.h"
#include "swdrv.h"
#include "swdrvl4.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_module.h"
#include "dev_swdrvl4.h"
#if (SYS_CPNT_ISCDRV == TRUE)
#include "isc.h"
#endif
#include "string.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#if (SYS_CPNT_HRDRV == TRUE)
#include "hrdrv.h"
#endif

#include "sys_dflt.h"
#include "sys_bld.h"
#include "sysrsc_mgr.h"
#include "dev_swdrvl4_pmgr.h"



/******************* Default values of Ingress IP precedence to internal DSCP Mapping Table ****************/
const static swdrvl4_internal_dscp_t SWDRVL4_DEFAULT_PRE_TO_DSCP_MAPPING[SWDRVL4_INGRESS_PRE_TO_DSCP_MAPPING_ENTRY_NMB] = {
            {0,0},{1,0},{2,0},{3,0},{4,0},{5,0},{6,0},{7,0}
};

/******************* Default values of Ingress DSCP to internal DSCP Mapping Table ****************/
const static swdrvl4_internal_dscp_t SWDRVL4_DEFAULT_DSCP_TO_DSCP_MAPPING[SWDRVL4_INGRESS_DSCP_TO_DSCP_MAPPING_ENTRY_NMB] = {
            {0,0},{0,1},{0,0},{0,3},{0,0},{0,1},{0,0},{0,3},
            {1,0},{1,1},{1,0},{1,3},{1,0},{1,1},{1,0},{1,3},
            {2,0},{2,1},{2,0},{2,3},{2,0},{2,1},{2,0},{2,3},
            {3,0},{3,1},{3,0},{3,3},{3,0},{3,1},{3,0},{3,3},
            {4,0},{4,1},{4,0},{4,3},{4,0},{4,1},{4,0},{4,3},
            {5,0},{5,1},{5,0},{5,3},{5,0},{5,1},{5,0},{5,3},
            {6,0},{6,1},{6,0},{6,3},{6,0},{6,1},{6,0},{6,3},
            {7,0},{7,1},{7,0},{7,3},{7,0},{7,1},{7,0},{7,3}
};


/* if support L2 stacking
 */

#if (SYS_CPNT_STACKING == TRUE)

/* service ID list
 */
typedef enum
{
    SWDRVL4_ENABLE_TOS_COS_MAP = 0,
    SWDRVL4_DISABLE_TOS_COS_MAP ,
    SWDRVL4_ENABLE_DSCP_COS_MAP,
    SWDRVL4_DISABLE_DSCP_COS_MAP,
    SWDRVL4_ENABLE_TCPPORT_COS_MAP,
    SWDRVL4_DISABLE_TCPPORT_COS_MAP,
    SWDRVL4_SET_TOS_COS_MAP,
    SWDRVL4_SET_DSCP_COS_MAP,
    SWDRVL4_SET_TCPPORT_COS_MAP,
    SWDRVL4_DEL_TOS_COS_MAP,
    SWDRVL4_DEL_DSCP_COS_MAP,
    SWDRVL4_DEL_TCPPORT_COS_MAP,
    SWDRVL4_GLOBAL_SET_TOS_MAP,
    SWDRVL4_GLOBAL_SET_DSCP_MAP,
    SWDRVL4_COS_TRUST_MODE,
    SWDRVL4_COS_INGCOS2DSCP,
    SWDRVL4_COS_INGPRE2DSCP,
    SWDRVL4_COS_INGDSCP2DSCP,
    SWDRVL4_COS_INGDSCP2QUEUE,
    SWDRVL4_COS_INGDSCP2COLOR,
    SWDRVL4_COS_INGDSCP2COS,
    SWDRVL4_COS_SET_PORT_LIST_INFO,
    SWDRVL4_MAX_SERVICE_ID
} SWDRVL4_ServicesID_T;

typedef struct
{
    UI8_T    ServiceID;      /* Service ID  */
    UI32_T   SubServiceID;   /* Service ID  */
    UI8_T    ISC_ID;         /* ISC ID      */
    UI32_T   unit;           /* stack id (unit number) */
    UI32_T   port;           /* port number */
    UI32_T   value;          /* about tos/dscp/tcpport */
    UI32_T   cos;            /* cos value   */
    UI32_T   cfi;            /* cfi value   */
    UI32_T   color;          /* color value */
    UI32_T   phb;            /* phb value   */
    UI8_T    portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
}__attribute__((packed, aligned(1)))SWDRVL4_Rx_IscBuf_T;

typedef BOOL_T (*SWDRVL4_ServiceFunc_t) (ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);

/* slave callback function
 */
static BOOL_T SlaveEnableTosCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveDisableTosCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveEnableDscpCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveDisableDscpCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveEnableTcpPortCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveDisableTcpPortCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetTosCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetDscpCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetTcpPortCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveDelTosCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveDelDscpCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveDelTcpPortCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetGlobalTosCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetGlobalDscpCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);

static BOOL_T SlaveSetTrustMode(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetIngCos2Dscp(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetIngPre2Dscp(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetIngDscp2Dscp(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetIngDscp2Queue(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetIngDscp2Color(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetIngDscp2Cos(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
static BOOL_T SlaveSetCoSLocalPortListInfo(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p);
/* service function table
 */
static SWDRVL4_ServiceFunc_t SWDRVL4_func_tab[] =
{
    SlaveEnableTosCosMap,                   /* SWDRVL4_ENABLE_TOS_COS_MAP       */
    SlaveDisableTosCosMap,                  /* SWDRVL4_DISABLE_TOS_COS_MAP      */
    SlaveEnableDscpCosMap,                  /* SWDRVL4_ENABLE_DSCP_COS_MAP      */
    SlaveDisableDscpCosMap,                 /* SWDRVL4_DISABLE_DSCP_COS_MAP     */
    SlaveEnableTcpPortCosMap,               /* SWDRVL4_ENABLE_TCPPORT_COS_MAP   */
    SlaveDisableTcpPortCosMap,              /* SWDRVL4_DISABLE_TCPPORT_COS_MAP  */
    SlaveSetTosCosMap,                      /* SWDRVL4_SET_TOS_COS_MAP      */
    SlaveSetDscpCosMap,                     /* SWDRVL4_SET_DSCP_COS_MAP     */
    SlaveSetTcpPortCosMap,                  /* SWDRVL4_SET_TCPPORT_COS_MAP  */
    SlaveDelTosCosMap,                      /* SWDRVL4_DEL_TOS_COS_MAP      */
    SlaveDelDscpCosMap,                     /* SWDRVL4_DEL_DSCP_COS_MAP     */
    SlaveDelTcpPortCosMap,                  /* SWDRVL4_DEL_TCPPORT_COS_MAP  */
    SlaveSetGlobalTosCosMap,                /* SWDRVL4_GLOBAL_SET_TOS_MAP   */
    SlaveSetGlobalDscpCosMap,               /* SWDRVL4_GLOBAL_SET_DSCP_MAP  */
    SlaveSetTrustMode,                      /* SWDRVL4_COS_TRUST_MODE       */
    SlaveSetIngCos2Dscp,                    /* SWDRVL4_COS_INGCOS2DSCP      */
    SlaveSetIngPre2Dscp,                    /* SWDRVL4_COS_INGPRE2DSCP      */
    SlaveSetIngDscp2Dscp,                   /* SWDRVL4_COS_INGDSCP2DSCP     */
    SlaveSetIngDscp2Queue,                  /* SWDRVL4_COS_INGDSCP2QUEUE    */
    SlaveSetIngDscp2Color,                  /* SWDRVL4_COS_INGDSCP2COLOR    */
    SlaveSetIngDscp2Cos,                    /* SWDRVL4_COS_INGDSCP2COS      */
    SlaveSetCoSLocalPortListInfo               /* SWDRV_SET_PORT_LIST_INFO     */                     

};

/* struct definiation
 */

/* L2 STACKING FUNCTIONS
 */
#endif

/* TYPE DECLARATIONS
 */
typedef struct
{
    UI8_T   stack_id;               /* the id in a stacking */
    UI16_T  port_number;            /* total number of ports */
    UI32_T  num_of_units;
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T  stack_unit_tbl[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    UI32_T  stacking_port;
#endif /*SYS_CPNT_STACKING*/
} SWDRVL4_Switch_Info_T;

#define SWDRVL4_IFINDEX_TO_UNIT(ifindex)             ( ((UI32_T)(((ifindex)-1)/(SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)))+1 )

#define SWDRVL4_IFINDEX_TO_PORT(ifindex)             ( (ifindex) - (SWDRVL4_IFINDEX_TO_UNIT(ifindex)-1)*(SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) )

#define SWDRVL4_IS_PORTLIST_MEMBER(port_list, port)  (((port_list[((port) - 1) >> 3]) & (1 << (7 - (((port) - 1) & 7)))) != 0)

#define LOCAL_HOST                        1

#if (SYS_CPNT_STACKING == TRUE)
#define SWDRVL4_TIME_OUT                  10000          /* time to wait for ISC reply : 100 sec */
#define SWDRVL4_TRY_TIMES                 3
#define SWDRVL4_ALL_UNIT                  255            /* all unit number  */
#define MASTER_UNIT                       1
#define SWDRV_OPTION_MODULE               255

#define SWDRVL4_POOL_ID_ISC_SEND          0
#define SWDRVL4_POOL_ID_ISC_REPLY         1
#endif /*SYS_CPNT_STACKING*/
#define ALL_PORT                          65535          /* For global configuration (all l_port) */

/* Local Function */
#if (SYS_CPNT_STACKING == TRUE)
static void SWDRVL4_InitMasterDataBase(void);
static void SWDRVL4_InitSlaveDataBase(void);
#endif
static BOOL_T SWDRVL4_LocalEnableTosCosMap(void);
static BOOL_T SWDRVL4_LocalDisableTosCosMap(void);
static BOOL_T SWDRVL4_LocalEnableDscpCosMap(void);
static BOOL_T SWDRVL4_LocalDisableDscpCosMap(void);
static BOOL_T SWDRVL4_LocalEnableTcpPortCosMap(void);
static BOOL_T SWDRVL4_LocalDisableTcpPortCosMap(void);
static BOOL_T SWDRVL4_LocalSetTosCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos);
static BOOL_T SWDRVL4_LocalSetDscpCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos);
static BOOL_T SWDRVL4_LocalSetTcpPortCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos);
static BOOL_T SWDRVL4_LocalDelTosCosMap(UI32_T unit, UI32_T port, UI32_T value);
static BOOL_T SWDRVL4_LocalDelDscpCosMap(UI32_T unit, UI32_T port, UI32_T value);
static BOOL_T SWDRVL4_LocalDelTcpPortCosMap(UI32_T unit, UI32_T port, UI32_T value);
static BOOL_T SWDRVL4_LocalSetCosTrustMode(UI32_T unit, UI32_T port, UI32_T mode);
static BOOL_T SWDRVL4_LocalSetQosIngCos2Dscp(UI32_T unit, UI32_T port,UI32_T cos,UI32_T cfi,UI32_T phb,UI32_T color);
static BOOL_T SWDRVL4_LocalSetQosIngPre2Dscp(UI32_T unit, UI32_T port,UI32_T dscp,UI32_T phb,UI32_T color);
static BOOL_T SWDRVL4_LocalSetQosIngDscp2Dscp(UI32_T unit, UI32_T port,UI32_T o_dscp,UI32_T phb,UI32_T color);
static BOOL_T SWDRVL4_LocalSetQosIngDscp2Queue(UI32_T unit, UI32_T port,UI32_T phb,UI32_T queue);
static BOOL_T SWDRVL4_LocalSetQosIngDscp2Color(UI32_T unit, UI32_T port,UI32_T phb,UI32_T color);
static BOOL_T SWDRVL4_LocalSetQosIngDscp2Cos(UI32_T unit, UI32_T port,UI32_T phb,UI32_T color,UI32_T cos,UI32_T cfi);


/* LOCAL VARIABLES
 */
#define LOCAL_UNIT_ID         shmem_data_p->swdrvl4_system_info.stack_id
#define SWDRVL4_EnterCriticalSection() SYSFUN_TakeSem(swdrvl4_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SWDRVL4_LeaveCriticalSection() SYSFUN_GiveSem(swdrvl4_sem_id)

typedef struct
{
	SYSFUN_DECLARE_CSC_ON_SHMEM
	SWDRVL4_Switch_Info_T  swdrvl4_system_info;
/*pre to internal dscp */
    swdrvl4_per_port_pre_dscp_t SWDRVL4_PER_PORT_PRE_DSCP[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
/*dscp to internal dscp */
    swdrvl4_per_port_dscp_dscp_t  SWDRVL4_PER_PORT_DSCP_DSCP[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
		
}SWDRVL4_ShmemData_T;
/* LOCAL FUNCTIONS
 */
static SWDRVL4_ShmemData_T *shmem_data_p;
static UI32_T swdrvl4_sem_id;

/* FUNCTION NAME: SWDRVL4_AttachSystemResources
 *------------------------------------------------------------------------------
 * PURPOSE: init share memory semaphore
 *------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *------------------------------------------------------------------------------
 * NOTES:
 */
void SWDRVL4_AttachSystemResources(void)
{
	shmem_data_p = (SWDRVL4_ShmemData_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SWDRVL4_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SWDRVL4, &swdrvl4_sem_id);
}

/* FUNCTION NAME: SWDRVL4_GetShMemInfo
 *------------------------------------------------------------------------------
 * PURPOSE: Get share memory info
 *------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRVL4_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_SWDRVL4_SHMEM_SEGID;
    *seglen_p = sizeof(SWDRVL4_ShmemData_T);
}

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_Init
 *------------------------------------------------------------------------------
 * FUNCTION: This function allocates and initiates the system resource for
 *           Switch Control module
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_Init(void)
{
    /* gordon_kao: The initialization will happend in driver process */
    /* DEV_SWDRVL4_Init(); */
    return;
} /* End of SWDRV_Init() */

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_Create_InterCSC_Relation
 *------------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_Create_InterCSC_Relation(void)
{
    return;
} /* End of SWDRVL4_Create_InterCSC_Relation() */

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_EnterMasterMode
 *------------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 4 Switch Driver module to
 *           enter master mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_EnterMasterMode(void)
{
#if (SYS_CPNT_STACKING == FALSE)
	UI32_T num_of_units;
    UI32_T stack_id, i, port_type;
#endif

#if (SYS_CPNT_STACKING == TRUE)
    /* if stacking
     */
    SWDRVL4_InitMasterDataBase();
#else
    /* if standalone
     */
    STKTPLG_POM_GetMyUnitID(&stack_id);
    STKTPLG_POM_GetNumberOfUnit(&num_of_units);
	SWDRVL4_EnterCriticalSection();
    shmem_data_p->swdrvl4_system_info.stack_id = (UI8_T)stack_id;
	shmem_data_p->swdrvl4_system_info.num_of_units = num_of_units;
	SWDRVL4_LeaveCriticalSection();

#endif

    /* gordon_kao: The initialization will happend in driver process */
    /*DEV_SWDRVL4_Init();*/

    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_EnterSlaveMode
 *------------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 4 Switch Driver module to
 *           enter slave mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_EnterSlaveMode(void)
{

#if (SYS_CPNT_STACKING == TRUE)
    SWDRVL4_InitSlaveDataBase();
#endif

    /* gordon_kao: The initialization will happend in driver process */
    /* DEV_SWDRVL4_Init(); */

    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_EnterTransitionMode
 *------------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 4 Switch Driver module to
 *           enter transition mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_EnterTransitionMode(void)
{
    /* wait other callers leave */
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_SetTransitionMode
 *------------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 4 Switch Driver module to
 *           set transition mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
void SWDRVL4_SetTransitionMode(void)
{
    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
}

#if (SYS_CPNT_STACKING == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  SWDRVL4_InitMasterDataBase
 *------------------------------------------------------------------------------
 * FUNCTION: This function will init database for SWDRVL4 module when Master Mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
static void SWDRVL4_InitMasterDataBase(void)
{
    UI32_T stack_id, port_number, num_of_units;
    UI32_T previous_unitId;
    UI32_T index;
    int i,j;

    STKTPLG_POM_GetLocalMaxPortCapability(&port_number);
    STKTPLG_POM_GetMyUnitID(&stack_id);
    STKTPLG_POM_GetNumberOfUnit(&num_of_units);
	SWDRVL4_EnterCriticalSection();
    shmem_data_p->swdrvl4_system_info.port_number = (UI16_T )port_number;
    shmem_data_p->swdrvl4_system_info.stack_id = (UI8_T)stack_id;
	shmem_data_p->swdrvl4_system_info.num_of_units = num_of_units;
    shmem_data_p->swdrvl4_system_info.stack_unit_tbl[0] = shmem_data_p->swdrvl4_system_info.stack_id;
	SWDRVL4_LeaveCriticalSection();

    previous_unitId = 0;
    index=1;

    while (STKTPLG_POM_GetNextUnit(&previous_unitId))
    {
		SWDRVL4_EnterCriticalSection();
        if (shmem_data_p->swdrvl4_system_info.stack_id != previous_unitId)
            shmem_data_p->swdrvl4_system_info.stack_unit_tbl[index++] = previous_unitId;
		SWDRVL4_LeaveCriticalSection();
    }

    /* temporary, waiting for STKTPLG be able to detect stacking port
     */
	SWDRVL4_EnterCriticalSection();
    shmem_data_p->swdrvl4_system_info.stacking_port = 25;
    for(j=0;j<SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT;j++)
    {
            //initialize qos map pre to dscp table
     	for(i = 0; i <= SWDRVL4_MAX_PRE_VAL; i++)
        {
            shmem_data_p->SWDRVL4_PER_PORT_PRE_DSCP[j].CURRENT_PRE_TO_DSCP_MAPPING[i].phb = SWDRVL4_DEFAULT_PRE_TO_DSCP_MAPPING[i].phb;
            shmem_data_p->SWDRVL4_PER_PORT_PRE_DSCP[j].CURRENT_PRE_TO_DSCP_MAPPING[i].color = SWDRVL4_DEFAULT_PRE_TO_DSCP_MAPPING[i].color;
        }
        //initialize qos map dscp to dscp table
        for(i = 0; i <= SWDRVL4_MAX_DSCP_VAL; i++)
        {
            shmem_data_p->SWDRVL4_PER_PORT_DSCP_DSCP[j].CURRENT_DSCP_TO_DSCP_MAPPING[i].phb = SWDRVL4_DEFAULT_DSCP_TO_DSCP_MAPPING[i].phb;
            shmem_data_p->SWDRVL4_PER_PORT_DSCP_DSCP[j].CURRENT_DSCP_TO_DSCP_MAPPING[i].color = SWDRVL4_DEFAULT_DSCP_TO_DSCP_MAPPING[i].color;
        }
    }
	SWDRVL4_LeaveCriticalSection();

    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME  SWDRVL4_InitSlaveDataBase
 *------------------------------------------------------------------------------
 * FUNCTION: This function will init database for SWDRVL4 module when Slave Mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------------*/
static void SWDRVL4_InitSlaveDataBase(void)
{
    UI32_T stack_id, port_number, num_of_units;
    UI32_T index;
    int i,j;

    STKTPLG_POM_GetLocalMaxPortCapability(&port_number);
    STKTPLG_POM_GetMyUnitID(&stack_id);
    STKTPLG_POM_GetNumberOfUnit(&num_of_units);

	SWDRVL4_EnterCriticalSection();
    shmem_data_p->swdrvl4_system_info.port_number = (UI16_T )port_number;
    shmem_data_p->swdrvl4_system_info.stack_id = (UI8_T)stack_id;
	shmem_data_p->swdrvl4_system_info.num_of_units = num_of_units;
    for(index=0; index<shmem_data_p->swdrvl4_system_info.num_of_units; index++)
    {
        shmem_data_p->swdrvl4_system_info.stack_unit_tbl[index] = 0;
    }

    /* temporary, waiting for STKTPLG be able to detect stacking port
     */
    shmem_data_p->swdrvl4_system_info.stacking_port = 25;
    for(j=0;j<SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT;j++)
    {
            //initialize qos map pre to dscp table
     	for(i = 0; i <= SWDRVL4_MAX_PRE_VAL; i++)
        {
            shmem_data_p->SWDRVL4_PER_PORT_PRE_DSCP[j].CURRENT_PRE_TO_DSCP_MAPPING[i].phb = SWDRVL4_DEFAULT_PRE_TO_DSCP_MAPPING[i].phb;
            shmem_data_p->SWDRVL4_PER_PORT_PRE_DSCP[j].CURRENT_PRE_TO_DSCP_MAPPING[i].color = SWDRVL4_DEFAULT_PRE_TO_DSCP_MAPPING[i].color;
        }
        //initialize qos map dscp to dscp table
        for(i = 0; i <= SWDRVL4_MAX_DSCP_VAL; i++)
        {
            shmem_data_p->SWDRVL4_PER_PORT_DSCP_DSCP[j].CURRENT_DSCP_TO_DSCP_MAPPING[i].phb = SWDRVL4_DEFAULT_DSCP_TO_DSCP_MAPPING[i].phb;
            shmem_data_p->SWDRVL4_PER_PORT_DSCP_DSCP[j].CURRENT_DSCP_TO_DSCP_MAPPING[i].color = SWDRVL4_DEFAULT_DSCP_TO_DSCP_MAPPING[i].color;
        }
    }
	SWDRVL4_LeaveCriticalSection();


    return;
}
#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_EnableTosCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will enable TOS/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_EnableTosCosMap()
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 drv_unit, pdu_len;
    UI16_T                 dst_bmp=0;

#endif /*SYS_CPNT_STACKING*/

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    drv_unit=0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_ENABLE_TOS_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL4_ENABLE_TOS_COS_MAP;

        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            
            return FALSE;
        }
    }
#endif

    /* if standalone
     */
    if(SWDRVL4_LocalEnableTosCosMap()== FALSE)
    {
        /* Remote units has been configured sucessfully,
         * but configuring local unit fails.
         * May need to design a error handling mechanism
         * for this condition
         */
        
        return FALSE;
    }

    
    return TRUE;


}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_DisableTosCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will disable TOS/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_DisableTosCosMap()
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
    SWDRVL4_Rx_IscBuf_T* isc_buf_p;
    UI32_T               drv_unit, pdu_len;
    UI16_T               dst_bmp=0;
#endif /*SYS_CPNT_STACKING*/

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }


#if (SYS_CPNT_STACKING == TRUE)
    drv_unit=0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_DISABLE_TOS_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL4_DISABLE_TOS_COS_MAP;

        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            
            return FALSE;
        }
    }
#endif

    /* if standalone
     */
    if(SWDRVL4_LocalDisableTosCosMap()== FALSE)
    {
        /* Remote units has been configured sucessfully,
         * but configuring local unit fails.
         * May need to design a error handling mechanism
         * for this condition
         */

        /* UIMSG_MGR_SetErrorCode(error_code); */
        
        return FALSE;
    }

    
    return TRUE;

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_EnableDscpCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will enable DSCP/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_EnableDscpCosMap()
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
    SWDRVL4_Rx_IscBuf_T* isc_buf_p;
    UI32_T               drv_unit, pdu_len;
    UI16_T               dst_bmp=0;
#endif /*SYS_CPNT_STACKING*/

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    drv_unit=0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_ENABLE_DSCP_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL4_ENABLE_DSCP_COS_MAP;
        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            
            return FALSE;
        }
    }
#endif

    if(SWDRVL4_LocalEnableDscpCosMap()== FALSE)
    {
        /* Remote units has been configured sucessfully,
         * but configuring local unit fails.
         * May need to design a error handling mechanism
         * for this condition
         */

        /* UIMSG_MGR_SetErrorCode(error_code); */
        
        return FALSE;
    }

    
    return TRUE;

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_DisableDscpCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will disable DSCP/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_DisableDscpCosMap()
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
    SWDRVL4_Rx_IscBuf_T* isc_buf_p;
    UI32_T               drv_unit, pdu_len;
    UI16_T               dst_bmp=0;
#endif /*SYS_CPNT_STACKING*/

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    drv_unit=0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_DISABLE_DSCP_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL4_DISABLE_DSCP_COS_MAP;

        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            
            return FALSE;
        }
    }
#endif

    if(SWDRVL4_LocalDisableDscpCosMap()== FALSE)
    {
        /* Remote units has been configured sucessfully,
         * but configuring local unit fails.
         * May need to design a error handling mechanism
         * for this condition
         */

        /* UIMSG_MGR_SetErrorCode(error_code); */
        
        return FALSE;
    }

    
    return TRUE;

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_EnableTcpPortCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will enable TCP_PORT/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : TRUE /FALSE
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_EnableTcpPortCosMap()
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
    SWDRVL4_Rx_IscBuf_T* isc_buf_p;
    UI32_T               drv_unit, pdu_len;
    UI16_T               dst_bmp=0;;
#endif /*SYS_CPNT_STACKING*/

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    drv_unit=0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_ENABLE_TCPPORT_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL4_ENABLE_TCPPORT_COS_MAP;
        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            
            return FALSE;
        }
    }
#endif

    if(SWDRVL4_LocalEnableTcpPortCosMap()== FALSE)
    {
        /* Remote units has been configured sucessfully,
         * but configuring local unit fails.
         * May need to design a error handling mechanism
         * for this condition
         */

        /* UIMSG_MGR_SetErrorCode(error_code); */
        
        return FALSE;
    }

    
    return TRUE;

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_DisableTcpPortCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will disable TCP_PORT/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_DisableTcpPortCosMap()
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
    SWDRVL4_Rx_IscBuf_T* isc_buf_p;
    UI32_T               drv_unit, pdu_len;
    UI16_T               dst_bmp=0;
#endif /*SYS_CPNT_STACKING*/

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)

    drv_unit=0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_DISABLE_TCPPORT_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL4_DISABLE_TCPPORT_COS_MAP;

        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
            /* If some of the stacking units fails, data among units will be
             * different. May need to design a error handling mechanism for this
             * condition
             */
            
            return FALSE;
        }
    }
#endif

    if(SWDRVL4_LocalDisableTcpPortCosMap()== FALSE)
    {
        /* Remote units has been configured sucessfully,
         * but configuring local unit fails.
         * May need to design a error handling mechanism
         * for this condition
         */

        /* UIMSG_MGR_SetErrorCode(error_code); */
        
        return FALSE;
    }

    
    return TRUE;

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_SetTosCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will config per port TOS/COS mapping of system
 * INPUT   : unit  -- unit number
 *           port  -- user port
 *           value -- tos
 *           cos
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_SetTosCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
    SWDRVL4_Rx_IscBuf_T* isc_buf_p;
    UI32_T               drv_unit, dst_unit, pdu_len;
    UI16_T               dst_bmp=0;
#endif

    UI32_T  max_port_number;
    BOOL_T  is_option;

    /* if standalone
     */
    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if  (SYS_CPNT_COS_PER_PORT == FALSE)
        port = ALL_PORT;
#endif
    /* this is global command
     */
    if (port == ALL_PORT)
    {
#if (SYS_CPNT_STACKING == TRUE)
        drv_unit=0;
        while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
        {
            if (drv_unit != LOCAL_UNIT_ID)
            {
                dst_bmp |= BIT_VALUE(drv_unit-1);
            }
        }

        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_SET_TOS_COS_MAP));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                
                return FALSE;
            }

            isc_buf_p->ServiceID = SWDRVL4_SET_TOS_COS_MAP;
            isc_buf_p->port      = port;
            isc_buf_p->value     = value;
            isc_buf_p->cos       = cos;

            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                                        SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
                /* If some of the stacking units fails, data among units will be
                 * different. May need to design a error handling mechanism for this
                 * condition
                 */
                
                return FALSE;
            }
        }

#endif

        if(SWDRVL4_LocalSetTosCosMap(unit, port, value, cos) == FALSE)
        {
            /* Remote units has been configured sucessfully,
             * but configuring local unit fails.
             * May need to design a error handling mechanism
             * for this condition
             */
            
            /* UIMSG_MGR_SetErrorCode() */
            return FALSE;
        }

        
        return TRUE;
    }
    /* !ALL_PORT
     * per port settting
     */
    else
    {
        is_option = FALSE;
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            
            return FALSE;
        }

        if(port > max_port_number)
        {
            is_option=TRUE;
        }
        else
        {
            is_option=FALSE;
        }
        /* local call, neither slave or module
         */
        if((LOCAL_UNIT_ID == unit) && (is_option == FALSE))
        {
            if(SWDRVL4_LocalSetTosCosMap(unit, port, value, cos) == FALSE)
            {
                
                return FALSE;
            }

             
             return TRUE;
        }

#if (SYS_CPNT_STACKING == TRUE)
        /* must be slave or option module
         */
        if(is_option == TRUE)
        {
            if (STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit) == FALSE)
            {
                
                return FALSE;
            }
            dst_unit = drv_unit;
        }
        else
        {
            dst_unit = unit;
        }

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_SET_TOS_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s(): L_MM_Mref_GetPdu() fails(dst_unit=%lu)", __FUNCTION__, dst_unit);
            
            return FALSE;
        }
        isc_buf_p->ServiceID = SWDRVL4_SET_TOS_COS_MAP;
        isc_buf_p->port      = port;
        isc_buf_p->value     = value;
        isc_buf_p->cos       = cos;

        if(0!=ISC_SendMcastReliable(BIT_VALUE(dst_unit-1), ISC_SWDRVL4_SID,
                                    mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
            
            return FALSE;
        }

        
        return TRUE;
#endif
    } /* per port settting */

    
    return FALSE;
}


/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_SetTosCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will config per port TOS/COS mapping of system
 * INPUT   : unit  -- unit number
 *           port  -- user port
 *           value -- tos
 *           cos
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_SetTcpPortCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
    SWDRVL4_Rx_IscBuf_T* isc_buf_p;
    UI32_T               drv_unit, dst_unit, pdu_len;
    UI16_T               dst_bmp=0;
#endif

    UI32_T  max_port_number;
    BOOL_T  is_option;

    /* if standalone
     */
    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if  (SYS_CPNT_COS_PER_PORT == FALSE)
        port = ALL_PORT;
#endif
    /* this is global command
     */
    if (port == ALL_PORT)
    {
#if (SYS_CPNT_STACKING == TRUE)
        drv_unit=0;
        while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
        {
            if (drv_unit != LOCAL_UNIT_ID)
            {
                dst_bmp |= BIT_VALUE(drv_unit-1);
            }
        }

        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_SET_TCPPORT_COS_MAP));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                
                return FALSE;
            }
            isc_buf_p->ServiceID = SWDRVL4_SET_TCPPORT_COS_MAP;
            isc_buf_p->port = port;
            isc_buf_p->value = value;
            isc_buf_p->cos = cos;

            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                                        SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
                /* If some of the stacking units fails, data among units will be
                 * different. May need to design a error handling mechanism for this
                 * condition
                 */
                
                return FALSE;
            }
        }
#endif
        if(SWDRVL4_LocalSetTcpPortCosMap(unit, port, value, cos) == FALSE)
        {
            /* Remote units has been configured sucessfully,
             * but configuring local unit fails.
             * May need to design a error handling mechanism
             * for this condition
             */
            
            /* UIMSG_MGR_SetErrorCode() */
            return FALSE;
        }

        
        return TRUE;
    }
    /* !ALL_PORT
     * per port settting
     */
    else
    {
        is_option = FALSE;
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            
            return FALSE;
        }

        if(port > max_port_number)
        {
            is_option=TRUE;
        }

        /* local call, neither slave or module
         */
        if((LOCAL_UNIT_ID == unit) && (is_option == FALSE))
        {
            if(SWDRVL4_LocalSetTcpPortCosMap(unit, port, value, cos) == FALSE)
            {
                
                return FALSE;
            }

            
            return TRUE;
        }

#if (SYS_CPNT_STACKING == TRUE)
        /* must be slave or option module
         */
        if(is_option == TRUE)
        {
            if (STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit) == FALSE)
            {
                
                return FALSE;
            }
            dst_unit = drv_unit;
        }
        else
        {
            dst_unit = unit;
        }

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_SET_TCPPORT_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s(): L_MM_Mref_GetPdu() fails(dst_unit=%lu)", __FUNCTION__, dst_unit);
            
            return FALSE;
        }
        isc_buf_p->ServiceID = SWDRVL4_SET_TCPPORT_COS_MAP;
        isc_buf_p->port      = port;
        isc_buf_p->value     = value;
        isc_buf_p->cos       = cos;

        if(0!=ISC_SendMcastReliable(BIT_VALUE(dst_unit-1), ISC_SWDRVL4_SID,
                                    mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
            
            return FALSE;
        }

        
        return TRUE;
#endif
    } /* per port settting */

    
    return FALSE;
}


/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_SetDscpCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will config per port DSCP/COS mapping of system
 * INPUT   : unit  -- unit number
 *           port  -- user port
 *           value -- dscp
 *           cos
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_SetDscpCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit, dst_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number;
    BOOL_T                 is_option=FALSE;

    /* if standalone
     */
    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if  (SYS_CPNT_COS_PER_PORT == FALSE)
        port = ALL_PORT;
#endif

    /* this is global command
     */
    if (port == ALL_PORT)
    {
#if (SYS_CPNT_STACKING == TRUE)
        /* sent to each slave
         */
        drv_unit=0;
        while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
        {
            if (drv_unit != LOCAL_UNIT_ID)
            {
                dst_bmp |= BIT_VALUE(drv_unit-1);
            }
        }

        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_SET_DSCP_COS_MAP));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                
                return FALSE;
            }

            isc_buf_p->ServiceID = SWDRVL4_SET_DSCP_COS_MAP;
            isc_buf_p->unit      = unit;
            isc_buf_p->port      = port;
            isc_buf_p->value     = value;
            isc_buf_p->cos       = cos;

            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY, SWDRVL4_TRY_TIMES,
                SWDRVL4_TIME_OUT, FALSE))
            {
                /* If some of the stacking units fails, data among units will be
                 * different. May need to design a error handling mechanism for this
                 * condition
                 */
                
                return FALSE;
            }
        }

#endif
        if(SWDRVL4_LocalSetDscpCosMap(unit, port, value, cos) == FALSE)
        {
            /* Remote units has been configured sucessfully,
             * but configuring local unit fails.
             * May need to design a error handling mechanism
             * for this condition
             */
            
            return FALSE;
        }

        
        return TRUE;
    }
    /* !ALL_PORT
     * per port settting
     */
    else
    {
        is_option = FALSE;
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            
            return FALSE;
        }

        if(port > max_port_number)
        {
            is_option=TRUE;
        }

        /* local call, neither slave or module
         */
        if((LOCAL_UNIT_ID == unit) && (is_option == FALSE))
        {
            if(SWDRVL4_LocalSetDscpCosMap(unit, port, value, cos) == FALSE)
            {
                
                return FALSE;
            }

             
             return TRUE;
        }

#if (SYS_CPNT_STACKING == TRUE)
        /* must be slave or option module
         */

        if(is_option == TRUE)
        {
            if (STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit) == FALSE)
            {
                
                return FALSE;
            }

            dst_unit = drv_unit;
        }
        else
        {
            dst_unit = unit;
        }

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_SET_DSCP_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s(): L_MM_Mref_GetPdu() fails(dst_unit=%lu)", __FUNCTION__, dst_unit);
            
            return FALSE;
        }


        isc_buf_p->ServiceID = SWDRVL4_SET_DSCP_COS_MAP;
        isc_buf_p->port      = port;
        isc_buf_p->value     = value;
        isc_buf_p->cos       = cos;

        if(0!=ISC_SendMcastReliable(BIT_VALUE(dst_unit-1), ISC_SWDRVL4_SID,
                                    mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
            
            return FALSE;
        }

#endif
    } /*per port setting */

    
    return FALSE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_DelTosCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will delete TOS/COS mapping of system
 * INPUT   : unit  -- unit number
 *           port  -- user port
 *           value -- tos
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_DelTosCosMap(UI32_T unit, UI32_T port, UI32_T value)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit, dst_unit;
    UI16_T                 dst_bmp=0;
#endif
    BOOL_T                 is_option=FALSE;
    UI32_T                 max_port_number;

    /* if standalone
     */
    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if  (SYS_CPNT_COS_PER_PORT == FALSE)
    port = ALL_PORT;
#endif

    if (port == ALL_PORT)
    {
       #if (SYS_CPNT_STACKING == TRUE)
        drv_unit = 0;
        while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
        {
            if (drv_unit != LOCAL_UNIT_ID)
            {
                dst_bmp |= BIT_VALUE(drv_unit-1);
            }
        }

        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_DEL_TOS_COS_MAP));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                
                return FALSE;
            }

            isc_buf_p->ServiceID = SWDRVL4_DEL_TOS_COS_MAP;
            isc_buf_p->port = port;
            isc_buf_p->value = value;

            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                                        SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
                /* If some of the stacking units fails, data among units will be
                 * different. May need to design a error handling mechanism for this
                 * condition
                 */
                
                return FALSE;
            }
        }
        #endif

        if(SWDRVL4_LocalDelTosCosMap(unit, port, value) == FALSE)
        {
            /* Remote units has been configured sucessfully,
             * but configuring local unit fails.
             * May need to design a error handling mechanism
             * for this condition
             */
            
            return FALSE;
        }

        
        return TRUE;
    }
    /* !ALL_PORT
     * pert port settting
     */
    else
    {
        is_option = FALSE;
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            
            return FALSE;
        }

        if(port > max_port_number)
        {
            is_option=TRUE;
        }

        /* local call, neither slave or module
         */
        if((LOCAL_UNIT_ID == unit) && (is_option == FALSE))
        {
            if(SWDRVL4_LocalDelTosCosMap(unit, port, value) == FALSE)
            {
                
                return FALSE;
            }

            
            return TRUE;
        }

#if (SYS_CPNT_STACKING == TRUE)
        /* must be slave or option module
         */
        if(is_option == TRUE)
        {
            if (STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit) == FALSE)
            {
                
                return FALSE;
            }
            dst_unit = drv_unit;
        }
        else
        {
            dst_unit = unit;
        }

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_DEL_TOS_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s(): L_MM_Mref_GetPdu() fails(dst_unit=%lu)", __FUNCTION__, dst_unit);
            
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL4_DEL_TOS_COS_MAP;
        isc_buf_p->port      = port;
        isc_buf_p->value     = value;

        if(0!=ISC_SendMcastReliable(BIT_VALUE(dst_unit-1), ISC_SWDRVL4_SID,
                                    mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
            
            return FALSE;
        }

        
        return TRUE;
#endif
    } /*per port setting */

    
    return FALSE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_DelDscpCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will delete DSCP/COS mapping of system
 * INPUT   : unit  -- unit number
 *           port  -- user port
 *           value -- dscp
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_DelDscpCosMap(UI32_T unit, UI32_T port, UI32_T value)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit, dst_unit;
    UI16_T                 dst_bmp=0;
#endif

    BOOL_T                 is_option=FALSE;
    UI32_T                 max_port_number;

    /* if standalone
     */
    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
#if  (SYS_CPNT_COS_PER_PORT == FALSE)
    port = ALL_PORT;
#endif

    if (port == ALL_PORT)
    {
       #if (SYS_CPNT_STACKING == TRUE)

        /* sent to each slave individually
         */
        drv_unit = 0;
        while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
        {

            if (drv_unit != LOCAL_UNIT_ID)
            {
                dst_bmp |= BIT_VALUE(drv_unit-1);
            }
        }

        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_DEL_DSCP_COS_MAP));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                
                return FALSE;
            }

            isc_buf_p->ServiceID = SWDRVL4_DEL_DSCP_COS_MAP;
            isc_buf_p->port = port;
            isc_buf_p->value = value;

            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                                        SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
                /* If some of the stacking units fails, data among units will be
                 * different. May need to design a error handling mechanism for this
                 * condition
                 */
                
                return FALSE;
            }
        }
        #endif
        if(SWDRVL4_LocalDelDscpCosMap(unit, port, value) == FALSE)
        {
            /* Remote units has been configured sucessfully,
             * but configuring local unit fails.
             * May need to design a error handling mechanism
             * for this condition
             */
            
            return FALSE;
        }

        
        return TRUE;
    }
    /* !ALL_PORT
     * pert port settting
     */
    else
    {
        is_option = FALSE;
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            
            return FALSE;
        }

        if(port > max_port_number)
        {
            is_option=TRUE;
        }

        /* local call, neither slave or module
         */
        if((LOCAL_UNIT_ID == unit) && (is_option == FALSE))
        {
            if(SWDRVL4_LocalDelDscpCosMap(unit, port, value) == FALSE)
            {
                
                return FALSE;
            }

             
             return TRUE;
        }

        #if (SYS_CPNT_STACKING == TRUE)
        /* must be slave or option module
         */
        if(is_option == TRUE)
        {
            if (STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit) == FALSE)
            {
                
                return FALSE;
            }
            dst_unit = drv_unit;
        }
        else
        {
            dst_unit = unit;
        }

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_DEL_DSCP_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails(dst_unit=%lu)", __FUNCTION__, dst_unit);
            
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL4_DEL_DSCP_COS_MAP;
        isc_buf_p->port = port;
        isc_buf_p->value = value;

        if(0!=ISC_SendMcastReliable(BIT_VALUE(dst_unit-1), ISC_SWDRVL4_SID,
                                    mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
                
                return FALSE;
        }

        
        return TRUE;

        #endif
    } /*per port setting */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL4_DelTcpPortCosMap
 *------------------------------------------------------------------------------
 * FUNCTION: This function will delete TCPPORT/COS mapping of system
 * INPUT   : unit  -- unit number
 *           port  -- user port
 *           value -- tcp port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 *------------------------------------------------------------------------------*/
BOOL_T SWDRVL4_DelTcpPortCosMap(UI32_T unit, UI32_T port, UI32_T value)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit, dst_unit;
    UI16_T                 dst_bmp=0;
#endif
    BOOL_T                 is_option=FALSE;
    UI32_T                 max_port_number;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if  (SYS_CPNT_COS_PER_PORT == FALSE)
    port = ALL_PORT;
#endif

    if (port == ALL_PORT)
    {
#if (SYS_CPNT_STACKING == TRUE)
        /* sent to each slave
         */
        drv_unit = 0;
        while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
        {
            if (drv_unit != LOCAL_UNIT_ID)
            {
                dst_bmp |= BIT_VALUE(drv_unit-1);
            }
        }

        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_DEL_TCPPORT_COS_MAP));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                
                return FALSE;
            }

            isc_buf_p->ServiceID = SWDRVL4_DEL_TCPPORT_COS_MAP;
            isc_buf_p->port = port;
            isc_buf_p->value = value;

            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
                                        mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
                /* If some of the stacking units fails, data among units will be
                 * different. May need to design a error handling mechanism for this
                 * condition
                 */
                
                return FALSE;
            }
        }
#endif
        if(SWDRVL4_LocalDelTcpPortCosMap(unit, port, value) == FALSE)
        {
            /* Remote units has been configured sucessfully,
             * but configuring local unit fails.
             * May need to design a error handling mechanism
             * for this condition
             */
            
            return FALSE;
        }

        
        return TRUE;
    }
    /* !ALL_PORT
     * per port settting
     */
    else
    {
        is_option = FALSE;
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            
            return FALSE;
        }

        if(port > max_port_number)
        {
            is_option=TRUE;
        }

        /* local call, neither slave or module
         */
        if((LOCAL_UNIT_ID == unit) && (is_option == FALSE))
        {
            if(SWDRVL4_LocalDelTcpPortCosMap(unit, port, value) == FALSE)
            {
                
                return FALSE;
            }

             
             return TRUE;
        }

#if (SYS_CPNT_STACKING == TRUE)
        /* must be slave or option module
         */

        if(is_option == TRUE)
        {
            if (STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit) == FALSE)
            {
                
                return FALSE;
            }
            dst_unit = drv_unit;
        }
        else
        {
            dst_unit = unit;
        }

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_DEL_TCPPORT_COS_MAP));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s(): L_MM_Mref_GetPdu() fails(dst_unit=%lu)", __FUNCTION__, dst_unit);
            
            return FALSE;
        }

        isc_buf_p->ServiceID = SWDRVL4_DEL_TCPPORT_COS_MAP;
        isc_buf_p->port = port;
        isc_buf_p->value = value;

        if(0!=ISC_SendMcastReliable(BIT_VALUE(dst_unit-1), ISC_SWDRVL4_SID,
                                    mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
            
            return FALSE;
        }

        
        return TRUE;
#endif
    } /*per port setting */

    
    return FALSE;
}

/************************************************************/
/*                  LOCAL FUNCITION                         */
/************************************************************/
static BOOL_T SWDRVL4_LocalEnableTosCosMap(void)
{
#if (SYS_CPNT_HRDRV == FALSE)
    return DEV_SWDRVL4_PMGR_EnableCosMapping(DEV_SWDRVL4_MapTosToCos);
#else
    #if(defined(STRATA_SWITCH)||defined(XGS_SWITCH))
        return HRDRV_EnableCosMapping(HRDRV_L4_MapTosToCos);
    #else
        return FALSE;
    #endif
#endif
}

static BOOL_T SWDRVL4_LocalDisableTosCosMap(void)
{
#if (SYS_CPNT_HRDRV == FALSE)
    return DEV_SWDRVL4_PMGR_DisableCosMapping(DEV_SWDRVL4_MapTosToCos);
#else
    #if(defined(STRATA_SWITCH)||defined(XGS_SWITCH))
        return HRDRV_DisableCosMapping(HRDRV_L4_MapTosToCos);
    #else
        return FALSE;
    #endif
#endif
}

static BOOL_T SWDRVL4_LocalEnableDscpCosMap(void)
{
#if (SYS_CPNT_HRDRV == FALSE)
    return DEV_SWDRVL4_PMGR_EnableCosMapping(DEV_SWDRVL4_MapDscpToCos);
#else
    #if(defined(STRATA_SWITCH)||defined(XGS_SWITCH))
        return HRDRV_EnableCosMapping(HRDRV_L4_MapDscpToCos);
    #else
        return FALSE;
    #endif
#endif
}

static BOOL_T SWDRVL4_LocalDisableDscpCosMap(void)
{
#if (SYS_CPNT_HRDRV == FALSE)
    return DEV_SWDRVL4_PMGR_DisableCosMapping(DEV_SWDRVL4_MapDscpToCos);
#else
    #if(defined(STRATA_SWITCH)||defined(XGS_SWITCH))
        return HRDRV_DisableCosMapping(HRDRV_L4_MapDscpToCos);
    #else
        return FALSE;
    #endif
#endif
}

static BOOL_T SWDRVL4_LocalEnableTcpPortCosMap(void)
{
#if (SYS_CPNT_HRDRV == FALSE)
    return DEV_SWDRVL4_PMGR_EnableCosMapping(DEV_SWDRVL4_MapTcpPortToCos);
#else
    return HRDRV_EnableCosMapping(HRDRV_L4_MapTcpPortToCos);
#endif
}

static BOOL_T SWDRVL4_LocalDisableTcpPortCosMap(void)
{
#if (SYS_CPNT_HRDRV == FALSE)
    return DEV_SWDRVL4_PMGR_DisableCosMapping(DEV_SWDRVL4_MapTcpPortToCos);
#else
    #if(defined(STRATA_SWITCH)||defined(XGS_SWITCH))
        return HRDRV_DisableCosMapping(HRDRV_L4_MapTcpPortToCos);
    #else
        return FALSE;
    #endif
#endif
}

static BOOL_T SWDRVL4_LocalSetTosCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos)
{
#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
    return DEV_SWDRVL4_PMGR_SetCosMapping(DEV_SWDRVL4_MapTosToCos, value, cos);
    #else
        return DEV_SWDRVL4_PMGR_SetPortCosMapping(DEV_SWDRVL4_MapTosToCos, 0, port, value, cos);
    #endif
#else
    #if(defined(STRATA_SWITCH)||defined(XGS_SWITCH))
        #if (SYS_CPNT_COS_PER_PORT == FALSE)
            return HRDRV_SetCosMapping(HRDRV_L4_MapTosToCos, value, cos);
        #else
            return HRDRV_SetPortCosMapping(unit, port, HRDRV_L4_MapTosToCos, value, cos);
        #endif
    #else
        return FALSE;
    #endif
#endif

}

static BOOL_T SWDRVL4_LocalSetDscpCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos)
{
#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return DEV_SWDRVL4_PMGR_SetCosMapping(DEV_SWDRVL4_MapDscpToCos, value, cos);
    #else
        return DEV_SWDRVL4_PMGR_SetPortCosMapping(DEV_SWDRVL4_MapDscpToCos, 0, port, value, cos);
    #endif
#else
    #if(defined(STRATA_SWITCH)||defined(XGS_SWITCH))
        #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return HRDRV_SetCosMapping(HRDRV_L4_MapDscpToCos, value, cos);
        #else
            return HRDRV_SetPortCosMapping(unit, port, HRDRV_L4_MapDscpToCos, value, cos);
        #endif
    #else
        return FALSE;
    #endif
#endif

}

static BOOL_T SWDRVL4_LocalSetTcpPortCosMap(UI32_T unit, UI32_T port, UI32_T value, UI32_T cos)
{
#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return DEV_SWDRVL4_PMGR_SetCosMapping(DEV_SWDRVL4_MapTcpPortToCos, value, cos);
    #else
        return DEV_SWDRVL4_PMGR_SetPortCosMapping(DEV_SWDRVL4_MapTcpPortToCos, 0, port, value, cos);
    #endif
#else
    #if(defined(STRATA_SWITCH)||defined(XGS_SWITCH))
        #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return HRDRV_SetCosMapping(HRDRV_L4_MapTcpPortToCos, value, cos);
        #else
            return HRDRV_SetPortCosMapping(unit, port, HRDRV_L4_MapTcpPortToCos, value, cos);
        #endif
    #else
        return FALSE;
    #endif
#endif
}

static BOOL_T SWDRVL4_LocalDelTosCosMap(UI32_T unit, UI32_T port, UI32_T value)
{
#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return DEV_SWDRVL4_PMGR_DeleteCosMapping(DEV_SWDRVL4_MapTosToCos, value, 0);
    #else
        return DEV_SWDRVL4_PMGR_DeletePortCosMapping(DEV_SWDRVL4_MapTosToCos, 0, port, value, 0);
    #endif
#else
    #if(defined(STRATA_SWITCH)||defined(XGS_SWITCH))
        #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return HRDRV_DelCosMapping(HRDRV_L4_MapTosToCos, value) ;
        #else
            return HRDRV_DelPortCosMapping(unit, port, HRDRV_L4_MapTosToCos, value);
        #endif
    #else
        return FALSE;
    #endif
#endif
}

static BOOL_T SWDRVL4_LocalDelDscpCosMap(UI32_T unit, UI32_T port, UI32_T value)
{
#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return DEV_SWDRVL4_PMGR_DeleteCosMapping(DEV_SWDRVL4_MapDscpToCos, value, 0);
    #else
        return DEV_SWDRVL4_PMGR_DeletePortCosMapping(DEV_SWDRVL4_MapDscpToCos, 0, port, value, 0);
    #endif
#else
    #if(defined(STRATA_SWITCH)||defined(XGS_SWITCH))
        #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return HRDRV_DelCosMapping(HRDRV_L4_MapDscpToCos, value) ;
        #else
            return HRDRV_DelPortCosMapping(unit, port, HRDRV_L4_MapDscpToCos, value);
        #endif
    #else
        return FALSE;
    #endif
#endif
}

static BOOL_T SWDRVL4_LocalDelTcpPortCosMap(UI32_T unit, UI32_T port, UI32_T value)
{
#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return DEV_SWDRVL4_PMGR_DeleteCosMapping(DEV_SWDRVL4_MapTcpPortToCos, value, 0);
    #else
        return DEV_SWDRVL4_PMGR_DeletePortCosMapping(DEV_SWDRVL4_MapTcpPortToCos, 0, port, value, 0);
    #endif
#else
    #if(defined(STRATA_SWITCH)||defined(XGS_SWITCH))
        #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return HRDRV_DelCosMapping(HRDRV_L4_MapTcpPortToCos, value) ;
        #else
            return HRDRV_DelPortCosMapping(unit, port, HRDRV_L4_MapTcpPortToCos, value);
        #endif
    #else
        return FALSE;
    #endif
#endif
}

/******************* Stacking Function : Slave SubRoutine *************************/
#if (SYS_CPNT_STACKING == TRUE)
static BOOL_T SlaveEnableTosCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalEnableTosCosMap())
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalEnableTosCosMap fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveDisableTosCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalDisableTosCosMap())
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SlaveDisableTosCosMap fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveEnableDscpCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalEnableDscpCosMap())
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SlaveEnableDscpCosMap fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveDisableDscpCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalDisableDscpCosMap())
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalDisableDscpCosMap fails", __FUNCTION__);
    }
    return ret;
}

static BOOL_T SlaveEnableTcpPortCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalEnableTcpPortCosMap())
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalEnableTcpPortCosMap fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveDisableTcpPortCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalDisableTcpPortCosMap())
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalDisableTcpPortCosMap fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetTosCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalSetTosCosMap(buf_p->unit,
                                 buf_p->port,
                                 buf_p->value,
                                 buf_p->cos) == TRUE)
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetTosCosMap fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetDscpCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalSetDscpCosMap(buf_p->unit,
                                  buf_p->port,
                                  buf_p->value,
                                  buf_p->cos) == TRUE)
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetDscpCosMap() fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetTcpPortCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    UI8_T ret;

    if(SWDRVL4_LocalSetTcpPortCosMap(buf_p->unit,
                                     buf_p->port,
                                     buf_p->value,
                                     buf_p->cos) == TRUE)
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetTcpPortCosMap fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveDelTosCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalDelTosCosMap(buf_p->unit,
                                 buf_p->port,
                                 buf_p->value) == TRUE)
    {
        ret = TRUE;
    }
    else
    {
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalDelTosCosMap fails", __FUNCTION__);
        ret = FALSE;
    }

    return ret;
}

static BOOL_T SlaveDelDscpCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalDelDscpCosMap(buf_p->unit,
                                  buf_p->port,
                                  buf_p->value) == TRUE)
    {
        ret = TRUE;
}
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalDelDscpCosMap fails", __FUNCTION__);
    }

    return ret;
}
static BOOL_T SlaveDelTcpPortCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalDelTcpPortCosMap(buf_p->unit,
                                     buf_p->port,
                                     buf_p->value) == TRUE)
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalDelTcpPortCosMap() fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetGlobalTosCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalSetTosCosMap(buf_p->unit,
                                 buf_p->port,
                                 buf_p->value,
                                 buf_p->cos) == TRUE)
    {
         ret = TRUE;
    }
    else
    {
         ret = FALSE;
         SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetTosCosMap() fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetGlobalDscpCosMap(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;

    if(SWDRVL4_LocalSetDscpCosMap(buf_p->unit,
                                  buf_p->port,
                                  buf_p->value,
                                  buf_p->cos) == TRUE)
    {
         ret = TRUE;
    }
    else
    {
         ret = FALSE;
         SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetDscpCosMap() fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetTrustMode(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;
    if(SWDRVL4_LocalSetCosTrustMode(buf_p->unit,
                                  buf_p->port,
                                  buf_p->value) == TRUE)
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetCosTrustMode fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetIngCos2Dscp(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;
    if(SWDRVL4_LocalSetQosIngCos2Dscp(buf_p->unit,buf_p->port,
                                   buf_p->cos,buf_p->cfi,
                                   buf_p->phb,buf_p->color) == TRUE)
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetQosIngCos2Dscp fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetIngPre2Dscp(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;
    if(SWDRVL4_LocalSetQosIngPre2Dscp(buf_p->unit,buf_p->port,
                                   buf_p->value,buf_p->phb,buf_p->color) == TRUE)
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetQosIngPre2Dscp fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetIngDscp2Dscp(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;
    if(SWDRVL4_LocalSetQosIngDscp2Dscp(buf_p->unit,buf_p->port,
                                   buf_p->value,buf_p->phb,buf_p->color))
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetQosIngPre2Dscp fails", __FUNCTION__);
    }

    return ret;
}
static BOOL_T SlaveSetIngDscp2Queue(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;
    if(SWDRVL4_LocalSetQosIngDscp2Queue(buf_p->unit,buf_p->port,
                                   buf_p->phb,buf_p->value))
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetQosIngDscp2Queue fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetIngDscp2Color(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;
    if(SWDRVL4_LocalSetQosIngDscp2Color(buf_p->unit,buf_p->port,
                                   buf_p->phb,buf_p->color))
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetQosIngDscp2Color fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetIngDscp2Cos(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *buf_p)
{
    BOOL_T ret;
    if(SWDRVL4_LocalSetQosIngDscp2Cos(buf_p->unit,buf_p->port,
                                   buf_p->phb,buf_p->color,
                                   buf_p->cos,buf_p->cfi))
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
        SYSFUN_Debug_Printf("\r\n%s():SWDRVL4_LocalSetQosIngDscp2Cos fails", __FUNCTION__);
    }

    return ret;
}

static BOOL_T SlaveSetCoSLocalPortListInfo(ISC_Key_T *key, SWDRVL4_Rx_IscBuf_T *request_p)
{
    UI32_T	lport;
    UI32_T max_port_number;
    for(lport = 0;lport < SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
    {
        if(!SWDRVL4_IS_PORTLIST_MEMBER(request_p->portlist,lport+1))
            continue;
        request_p->unit = SWDRVL4_IFINDEX_TO_UNIT(lport);
    
        if(LOCAL_UNIT_ID != request_p->unit)
            continue;
     
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(request_p->unit, &max_port_number) == FALSE)
        {
            return FALSE;
        }
    
        request_p->port = SWDRVL4_IFINDEX_TO_PORT(lport);
        if(request_p->port > max_port_number)
            continue;
        if(!SWDRVL4_func_tab[request_p->SubServiceID](key,request_p))
        {
            return FALSE;
        }
    }
    return TRUE;
}
/*------------------------------------------------------------------------------
 * Function : SWDRVL4_ISC_Handler
 *------------------------------------------------------------------------------
 * Purpose  : This function is called by isc_agent to handle non Global-wised
 *            configurations.
 * INPUT    : *key      -- key of ISC
 *            *mref_handle_p  -- transfer data
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : call by isc_agent
 *------------------------------------------------------------------------------
 */
BOOL_T SWDRVL4_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    SWDRVL4_Rx_IscBuf_T *buf_p;
    UI32_T  service;
    BOOL_T              ret;

    buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &service); /* service is used as dummy here */
    if(buf_p==NULL)
    {
        SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails", __FUNCTION__);
        return FALSE;
    }

    service = buf_p->ServiceID;

    if (service<SWDRVL4_MAX_SERVICE_ID && SWDRVL4_func_tab[service] != NULL)
    {
        ret=SWDRVL4_func_tab[service](key, buf_p);
    }
    else
    {
        SYSFUN_Debug_Printf("\r\n%s():Invalid service id(%lu)", __FUNCTION__, service);
        return FALSE;
    }

    L_MM_Mref_Release(&mref_handle_p);

    return ret;
}
#endif
static BOOL_T SWDRVL4_LocalSetCosTrustMode(UI32_T unit, UI32_T port, UI32_T mode)
{
    swdrvl4_qos_if_mapping_mode_t map_mode;
    UI32_T related_dscp;
    int i;

    if(SWDRVL4_COS_MAPPING_MODE== mode)
    {
        map_mode = SWDRVL4_L2_MAPPING_MODE;
    }
    else  if(SWDRVL4_PRECEDENCE_MAPPING_MODE == mode)
    {
        map_mode = SWDRVL4_DSCP_MAPPING_MODE;
    }
    else if(SWDRVL4_DSCP_IF_MAPPING_MODE == mode)
    {
        map_mode = SWDRVL4_DSCP_MAPPING_MODE;
    }
    else
        return FALSE;

#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)

        if(!DEV_SWDRVL4_PMGR_SetCosTrustMode(unit,port, map_mode))
            return FALSE;

        if(SWDRVL4_PRECEDENCE_MAPPING_MODE == mode)
        {
            for(i = 0; i <= SWDRVL4_MAX_PRE_VAL; i++)
            {
                related_dscp = i<<3;
                if(FALSE == SWDRVL4_LocalSetQosIngPre2Dscp(unit,port,related_dscp,
                shmem_data_p->SWDRVL4_PER_PORT_PRE_DSCP[port-1].CURRENT_PRE_TO_DSCP_MAPPING[i].phb,
                shmem_data_p->SWDRVL4_PER_PORT_PRE_DSCP[port-1].CURRENT_PRE_TO_DSCP_MAPPING[i].color))
                    return FALSE;
            }

        }
        else if(SWDRVL4_DSCP_IF_MAPPING_MODE == mode)
        {
            for(i = 0; i <= SWDRVL4_MAX_DSCP_VAL; i++)
            {
                if(FALSE == SWDRVL4_LocalSetQosIngDscp2Dscp(unit,port,i,
                shmem_data_p->SWDRVL4_PER_PORT_DSCP_DSCP[port-1].CURRENT_DSCP_TO_DSCP_MAPPING[i].phb,
                shmem_data_p->SWDRVL4_PER_PORT_DSCP_DSCP[port-1].CURRENT_DSCP_TO_DSCP_MAPPING[i].color))
                    return FALSE;
            }
        }
        return TRUE;

    #else
        return DEV_SWDRVL4_PMGR_DeletePortCosMapping(DEV_SWDRVL4_MapTcpPortToCos, 0, port, mode, 0);
    #endif
#endif
}
BOOL_T SWDRVL4_SetCosTrustMode(UI32_T unit, UI32_T port, UI32_T mode)
{

#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 drv_unit, pdu_len;
    UI16_T                 dst_bmp=0;

#endif /*SYS_CPNT_STACKING*/

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    UI32_T  max_port_number;
    if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
    {
        return FALSE;
    }
    if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
    && (port != SYS_ADPT_MGMT_PORT)
#endif
       )
    {
        if (port > max_port_number)
        {
            if (!STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit))
            {
                return FALSE;
            }
            dst_bmp = BIT_VALUE(drv_unit-1);
        }
        else
        {
            dst_bmp = BIT_VALUE(unit-1);
        }
    
        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_TRUST_MODE));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                return FALSE;
            }
        
            isc_buf_p->ServiceID = SWDRVL4_COS_TRUST_MODE;
            isc_buf_p->unit      = unit;
            isc_buf_p->port      = port;
            isc_buf_p->value     = mode;
            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                   SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                   SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
            /* If some of the stacking units fails, data among units will be
            * different. May need to design a error handling mechanism for this
            * condition
            */
                return FALSE;
            }
        }
    #endif
    }
    else
    {
    /* if local unit or standalone
    */
    if(SWDRVL4_LocalSetCosTrustMode(unit, port, mode) == FALSE)
        {
        
        return FALSE;
        }
    
    }
    return TRUE;
}

BOOL_T SWDRVL4_SetPortListCosTrustMode(UI8_T *port_list, UI32_T mode)
{

#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 drv_unit, pdu_len,unit,lport,port;
    UI16_T                 dst_bmp=0;
    UI32_T                 max_port_number;
#endif /*SYS_CPNT_STACKING*/

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    drv_unit=0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_SET_PORT_LIST_INFO));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }
    
        isc_buf_p->ServiceID = SWDRVL4_COS_SET_PORT_LIST_INFO;
        isc_buf_p->SubServiceID  =  SWDRVL4_COS_TRUST_MODE;
        memcpy(isc_buf_p->portlist, port_list,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        isc_buf_p->value = mode;
        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID, mref_handle_p,
                        SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                        SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
        /* If some of the stacking units fails, data among units will be
        * different. May need to design a error handling mechanism for this
        * condition
        */
             return FALSE;
        }
    }
#endif
    for(lport = 0;lport < SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
    {
        if(!SWDRVL4_IS_PORTLIST_MEMBER(port_list,lport+1))
            continue;
        unit = SWDRVL4_IFINDEX_TO_UNIT(lport+1);
        port = SWDRVL4_IFINDEX_TO_PORT(lport+1);
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            return FALSE;
        }

        if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
            && (port != SYS_ADPT_MGMT_PORT)
#endif
           )
            continue;
        else
        {
           /* if local unit or standalone
            */
           if(SWDRVL4_LocalSetCosTrustMode(unit, port, mode) == FALSE)
           {
               return FALSE;
           }
        }
    }
    return TRUE;
}
static BOOL_T SWDRVL4_LocalSetQosIngCos2Dscp(UI32_T unit, UI32_T port,UI32_T cos,UI32_T cfi,UI32_T phb,UI32_T color)
{
#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return DEV_SWDRVL4_PMGR_SetQosIngCos2Dscp(unit,port,cos,cfi,phb,color);
    #else
        return DEV_SWDRVL4_PMGR_DeletePortCosMapping(DEV_SWDRVL4_MapTcpPortToCos, 0, port, mode, 0);
    #endif
#endif
}
BOOL_T SWDRVL4_SetQosIngCos2Dscp(UI32_T unit, UI32_T port,UI32_T cos,UI32_T cfi,UI32_T phb,UI32_T color)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
    if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
    {
        return FALSE;
    }
    
    if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
    && (port != SYS_ADPT_MGMT_PORT)
#endif
       )
    {
        if (port > max_port_number)
        {
            if (!STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit))
            {
                return FALSE;
            }
            dst_bmp = BIT_VALUE(drv_unit-1);
        }
        else
        {
            dst_bmp = BIT_VALUE(unit-1);
        }

        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_INGCOS2DSCP));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                return FALSE;
            }
        
            isc_buf_p->ServiceID = SWDRVL4_COS_INGCOS2DSCP;
            isc_buf_p->unit = unit;
            isc_buf_p->port = port;
            isc_buf_p->cos = cos;
            isc_buf_p->cfi = cfi;
            isc_buf_p->phb = phb;
            isc_buf_p->color = color;
        
            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
                  mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                  SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
            /* If some of the stacking units fails, data among units will be
            * different. May need to design a error handling mechanism for this
            * condition
            */
                return FALSE;
            }
        }
    }
    else
    {
        if(SWDRVL4_LocalSetQosIngCos2Dscp(unit,port,cos,cfi,phb,color) == FALSE)
        {
        /* Remote units has been configured sucessfully,
        * but configuring local unit fails.
        * May need to design a error handling mechanism
        * for this condition
        */
        
        return FALSE;
        }
    }

    return TRUE;
}

BOOL_T SWDRVL4_SetPortListQosIngCos2Dscp(UI8_T *port_list,UI32_T cos,UI32_T cfi,UI32_T phb,UI32_T color)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number,lport,unit,port;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
#if (SYS_CPNT_STACKING == TRUE)
    drv_unit = 0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_SET_PORT_LIST_INFO));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }
        isc_buf_p->ServiceID = SWDRVL4_COS_SET_PORT_LIST_INFO;
        isc_buf_p->SubServiceID = SWDRVL4_COS_INGCOS2DSCP;
        memcpy(isc_buf_p->portlist, port_list,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        isc_buf_p->cos = cos;
        isc_buf_p->cfi = cfi;
        isc_buf_p->phb = phb;
        isc_buf_p->color = color;
    
        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
              mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
              SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
        /* If some of the stacking units fails, data among units will be
        * different. May need to design a error handling mechanism for this
        * condition
        */
            return FALSE;
        }
    }
#endif
    for(lport = 0;lport < SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
    {
        if(!SWDRVL4_IS_PORTLIST_MEMBER(port_list,lport+1))
            continue;
        unit = SWDRVL4_IFINDEX_TO_UNIT(lport+1);
        port = SWDRVL4_IFINDEX_TO_PORT(lport+1);
        
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            return FALSE;
        }

        if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
            && (port != SYS_ADPT_MGMT_PORT)
#endif
           )
            continue;
        else
        {
            if(SWDRVL4_LocalSetQosIngCos2Dscp(unit,port,cos,cfi,phb,color) == FALSE)
            {
            /* Remote units has been configured sucessfully,
            * but configuring local unit fails.
            * May need to design a error handling mechanism
            * for this condition
            */
            
                return FALSE;
            }
        }
    }

    return TRUE;
}

static BOOL_T SWDRVL4_LocalSetQosIngPre2Dscp(UI32_T unit, UI32_T port,UI32_T dscp,UI32_T phb,UI32_T color)
{
    int i;
    UI32_T pre,related_dscp;
    related_dscp = dscp;

#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
        for(i = 0; i < 8; i++)
        {
            if(FALSE == DEV_SWDRVL4_PMGR_SetQosIngPre2Dscp(unit,port,related_dscp,phb,color))
            {
                return FALSE;
            }
        	related_dscp++;
        }
        pre = dscp >> 3;
        shmem_data_p->SWDRVL4_PER_PORT_PRE_DSCP[port-1].CURRENT_PRE_TO_DSCP_MAPPING[pre].phb = phb;
        shmem_data_p->SWDRVL4_PER_PORT_PRE_DSCP[port-1].CURRENT_PRE_TO_DSCP_MAPPING[pre].color = color;
        return TRUE;

    #else
        return DEV_SWDRVL4_PMGR_DeletePortCosMapping(DEV_SWDRVL4_MapTcpPortToCos, 0, port, mode, 0);
    #endif
#endif
}

BOOL_T SWDRVL4_SetQosIngPre2Dscp(UI32_T unit, UI32_T port,UI32_T dscp,UI32_T phb,UI32_T color)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
    if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
    {
        return FALSE;
    }
    if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
    && (port != SYS_ADPT_MGMT_PORT)
#endif
       )
    {
        if (port > max_port_number)
        {
            if (!STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit))
            {
                return FALSE;
            }
            dst_bmp = BIT_VALUE(drv_unit-1);
        }
        else
        {
            dst_bmp = BIT_VALUE(unit-1);
        }
    
        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_INGPRE2DSCP));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
            }
        
            isc_buf_p->ServiceID = SWDRVL4_COS_INGPRE2DSCP;
            isc_buf_p->unit = unit;
            isc_buf_p->port = port;
            isc_buf_p->value = dscp;
            isc_buf_p->phb = phb;
            isc_buf_p->color = color;
        
            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
                mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
            /* If some of the stacking units fails, data among units will be
            * different. May need to design a error handling mechanism for this
            * condition
            */
            
                return FALSE;
            }
        }
    }
    else
    {
        if(SWDRVL4_LocalSetQosIngPre2Dscp(unit,port,dscp,phb,color) == FALSE)
        {
        /* Remote units has been configured sucessfully,
        * but configuring local unit fails.
        * May need to design a error handling mechanism
        * for this condition
        */
        
            return FALSE;
        }
    }

    return TRUE;
}

BOOL_T SWDRVL4_SetPortListQosIngPre2Dscp(UI8_T *port_list,UI32_T dscp,UI32_T phb,UI32_T color)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number,lport,port,unit;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
#if (SYS_CPNT_STACKING == TRUE)
    drv_unit = 0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }
  
    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_SET_PORT_LIST_INFO));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }
        isc_buf_p->ServiceID = SWDRVL4_COS_SET_PORT_LIST_INFO;
        isc_buf_p->SubServiceID = SWDRVL4_COS_INGPRE2DSCP;
        memcpy(isc_buf_p->portlist, port_list,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        isc_buf_p->value = dscp;
        isc_buf_p->phb = phb;
        isc_buf_p->color = color;
    
        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
            mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
            SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
        /* If some of the stacking units fails, data among units will be
        * different. May need to design a error handling mechanism for this
        * condition
        */
            return FALSE;
        }
    }
#endif

    for(lport = 0;lport < SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
    {
        if(!SWDRVL4_IS_PORTLIST_MEMBER(port_list,lport+1))
            continue;
        unit = SWDRVL4_IFINDEX_TO_UNIT(lport+1);
        port = SWDRVL4_IFINDEX_TO_PORT(lport+1);
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            return FALSE;
        }

        if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
            && (port != SYS_ADPT_MGMT_PORT)
#endif
           )
            continue;
        else
        {
            if(SWDRVL4_LocalSetQosIngPre2Dscp(unit,port,dscp,phb,color) == FALSE)
            {
            /* Remote units has been configured sucessfully,
            * but configuring local unit fails.
            * May need to design a error handling mechanism
            * for this condition
            */
            
                return FALSE;
            }
        }
    }
    return TRUE;
}

static BOOL_T SWDRVL4_LocalSetQosIngDscp2Dscp(UI32_T unit, UI32_T port,UI32_T o_dscp,UI32_T phb,UI32_T color)
{
#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
        if(TRUE == DEV_SWDRVL4_PMGR_SetQosIngDscp2Dscp(unit,port,o_dscp,phb,color))
        {
            shmem_data_p->SWDRVL4_PER_PORT_DSCP_DSCP[port-1].CURRENT_DSCP_TO_DSCP_MAPPING[o_dscp].phb = phb;
            shmem_data_p->SWDRVL4_PER_PORT_DSCP_DSCP[port-1].CURRENT_DSCP_TO_DSCP_MAPPING[o_dscp].color = color;
            return TRUE;
        }
        else
            return FALSE;
    #else
        return DEV_SWDRVL4_PMGR_DeletePortCosMapping(DEV_SWDRVL4_MapTcpPortToCos, 0, port, mode, 0);
    #endif
#endif
}

BOOL_T SWDRVL4_SetQosIngDscp2Dscp(UI32_T unit, UI32_T port,UI32_T o_dscp,UI32_T phb,UI32_T color)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
    if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
    {
        return FALSE;
    }
    if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
    && (port != SYS_ADPT_MGMT_PORT)
#endif
       )
    {
        if (port > max_port_number)
        {
            if (!STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit))
            {
                return FALSE;
            }
            dst_bmp = BIT_VALUE(drv_unit-1);
        }
        else
        {
            dst_bmp = BIT_VALUE(unit-1);
        }

        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_INGDSCP2DSCP));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                return FALSE;
            }
        
            isc_buf_p->ServiceID = SWDRVL4_COS_INGDSCP2DSCP;
            isc_buf_p->unit = unit;
            isc_buf_p->port = port;
            isc_buf_p->value = o_dscp;
            isc_buf_p->phb = phb;
            isc_buf_p->color = color;
            
            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
                  mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                  SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
            /* If some of the stacking units fails, data among units will be
            * different. May need to design a error handling mechanism for this
            * condition
            */
            
                return FALSE;
            }
        }
    }
    else
    {
        if(SWDRVL4_LocalSetQosIngDscp2Dscp(unit,port,o_dscp,phb,color) == FALSE)
        {
        /* Remote units has been configured sucessfully,
        * but configuring local unit fails.
        * May need to design a error handling mechanism
        * for this condition
        */
        
        return FALSE;
        }
    }

return TRUE;
}

BOOL_T SWDRVL4_SetPortListQosIngDscp2Dscp(UI8_T *port_list,UI32_T o_dscp,UI32_T phb,UI32_T color)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number,lport,unit,port;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
#if (SYS_CPNT_STACKING == TRUE)
    drv_unit = 0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }
    
    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_SET_PORT_LIST_INFO));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }
        isc_buf_p->ServiceID = SWDRVL4_COS_SET_PORT_LIST_INFO;
        isc_buf_p->SubServiceID = SWDRVL4_COS_INGDSCP2DSCP;
        memcpy(isc_buf_p->portlist, port_list,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        isc_buf_p->value = o_dscp;
        isc_buf_p->phb = phb;
        isc_buf_p->color = color;
        
        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
              mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
              SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
        /* If some of the stacking units fails, data among units will be
        * different. May need to design a error handling mechanism for this
        * condition
        */
            return FALSE;
        }
    }
#endif
    for(lport = 0;lport < SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
    {
        if(!SWDRVL4_IS_PORTLIST_MEMBER(port_list,lport+1))
            continue;
        
        unit = SWDRVL4_IFINDEX_TO_UNIT(lport+1);
        port = SWDRVL4_IFINDEX_TO_PORT(lport+1);
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            return FALSE;
        }

        if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
            && (port != SYS_ADPT_MGMT_PORT)
#endif
           )
            continue;
        else
        {
            if(SWDRVL4_LocalSetQosIngDscp2Dscp(unit,port,o_dscp,phb,color) == FALSE)
            {
            /* Remote units has been configured sucessfully,
            * but configuring local unit fails.
            * May need to design a error handling mechanism
            * for this condition
            */
            
            return FALSE;
            }
        }
    }
    return TRUE;
}

static BOOL_T SWDRVL4_LocalSetQosIngDscp2Queue(UI32_T unit, UI32_T port,UI32_T phb,UI32_T queue)
{
#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return DEV_SWDRVL4_PMGR_SetQosIngDscp2Queue(unit,port,phb,queue);
    #else
        return DEV_SWDRVL4_PMGR_DeletePortCosMapping(DEV_SWDRVL4_MapTcpPortToCos, 0, port, mode, 0);
    #endif
#endif
}
BOOL_T SWDRVL4_SetQosIngDscp2Queue(UI32_T unit, UI32_T port,UI32_T phb,UI32_T queue)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
    if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
    {
        return FALSE;
    }
    if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
    && (port != SYS_ADPT_MGMT_PORT)
#endif
       )
    {
        if (port > max_port_number)
        {
            if (!STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit))
            {
                return FALSE;
            }
            dst_bmp = BIT_VALUE(drv_unit-1);
        }
        else
        {
            dst_bmp = BIT_VALUE(unit-1);
        }

        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_INGDSCP2QUEUE));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                return FALSE;
            }
        
            isc_buf_p->ServiceID = SWDRVL4_COS_INGDSCP2QUEUE;
            isc_buf_p->unit = unit;
            isc_buf_p->port = port;
            isc_buf_p->phb = phb;
            isc_buf_p->value = queue;
            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
                  mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                  SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
            /* If some of the stacking units fails, data among units will be
            * different. May need to design a error handling mechanism for this
            * condition
            */
            
                return FALSE;
            }
        }
    }
    else
    {
        if(SWDRVL4_LocalSetQosIngDscp2Queue(unit,port,phb,queue) == FALSE)
        {
        /* Remote units has been configured sucessfully,
        * but configuring local unit fails.
        * May need to design a error handling mechanism
        * for this condition
        */
        
        return FALSE;
        }
    }

return TRUE;
}

BOOL_T SWDRVL4_SetPortListQosIngDscp2Queue(UI8_T *port_list,UI32_T phb,UI32_T queue)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number,unit,port,lport;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
#if (SYS_CPNT_STACKING == TRUE)
    drv_unit = 0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }
    
    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_SET_PORT_LIST_INFO));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }
        isc_buf_p->ServiceID = SWDRVL4_COS_SET_PORT_LIST_INFO;
        isc_buf_p->SubServiceID = SWDRVL4_COS_INGDSCP2QUEUE;
        memcpy(isc_buf_p->portlist, port_list,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        isc_buf_p->phb = phb;
        isc_buf_p->value = queue;
        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
              mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
              SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
        /* If some of the stacking units fails, data among units will be
        * different. May need to design a error handling mechanism for this
        * condition
        */
            return FALSE;
        }
    }
#endif
    for(lport = 0;lport < SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
    {
        if(!SWDRVL4_IS_PORTLIST_MEMBER(port_list,lport+1))
            continue;
        unit = SWDRVL4_IFINDEX_TO_UNIT(lport+1);
        port = SWDRVL4_IFINDEX_TO_PORT(lport+1);
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            return FALSE;
        }

        if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
            && (port != SYS_ADPT_MGMT_PORT)
#endif
           )
            continue;
        else
        {
           if(SWDRVL4_LocalSetQosIngDscp2Queue(unit,port,phb,queue) == FALSE)
           {
           /* Remote units has been configured sucessfully,
           * but configuring local unit fails.
           * May need to design a error handling mechanism
           * for this condition
           */
           
               return FALSE;
           }
        }
    }

return TRUE;
}
static BOOL_T SWDRVL4_LocalSetQosIngDscp2Color(UI32_T unit, UI32_T port,UI32_T phb,UI32_T color)
{
#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return DEV_SWDRVL4_PMGR_SetQosIngDscp2Color(unit,port,phb,color);
    #else
        return DEV_SWDRVL4_PMGR_DeletePortCosMapping(DEV_SWDRVL4_MapTcpPortToCos, 0, port, mode, 0);
    #endif
#endif
}
BOOL_T SWDRVL4_SetQosIngDscp2Color(UI32_T unit, UI32_T port,UI32_T phb,UI32_T color)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
    if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
    {
        return FALSE;
    }
    if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
    && (port != SYS_ADPT_MGMT_PORT)
#endif
       )
    {
        if (port > max_port_number)
        {
            if (!STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit))
            {
                return FALSE;
            }
            dst_bmp = BIT_VALUE(drv_unit-1);
        }
        else
        {
            dst_bmp = BIT_VALUE(unit-1);
        }

        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_INGDSCP2COLOR));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                return FALSE;
            }
        
            isc_buf_p->ServiceID = SWDRVL4_COS_INGDSCP2COLOR;
            isc_buf_p->unit = unit;
            isc_buf_p->port = port;
            isc_buf_p->phb = phb;
            isc_buf_p->color = color;
            
            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
                  mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                  SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
            /* If some of the stacking units fails, data among units will be
            * different. May need to design a error handling mechanism for this
            * condition
            */
            
                return FALSE;
            }
        }
    }
    else
    {
        if(SWDRVL4_LocalSetQosIngDscp2Color(unit,port,phb,color) == FALSE)
        {
        /* Remote units has been configured sucessfully,
        * but configuring local unit fails.
        * May need to design a error handling mechanism
        * for this condition
        */
        
        return FALSE;
        }
    }

return TRUE;
}

BOOL_T SWDRVL4_SetPortListQosIngDscp2Color(UI8_T *port_list,UI32_T phb,UI32_T color)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number,unit,lport,port;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
#if (SYS_CPNT_STACKING == TRUE)
    drv_unit = 0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }
    
    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_SET_PORT_LIST_INFO));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }
        isc_buf_p->ServiceID = SWDRVL4_COS_SET_PORT_LIST_INFO;
        isc_buf_p->SubServiceID = SWDRVL4_COS_INGDSCP2COLOR;
        memcpy(isc_buf_p->portlist, port_list,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        isc_buf_p->phb = phb;
        isc_buf_p->color = color;
        
        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
              mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
              SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
        /* If some of the stacking units fails, data among units will be
        * different. May need to design a error handling mechanism for this
        * condition
        */
            return FALSE;
        }
    }
#endif
    for(lport = 0;lport < SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
    {
        if(!SWDRVL4_IS_PORTLIST_MEMBER(port_list,lport+1))
            continue;
        unit = SWDRVL4_IFINDEX_TO_UNIT(lport+1);
        port = SWDRVL4_IFINDEX_TO_PORT(lport+1);
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            return FALSE;
        }

        if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
            && (port != SYS_ADPT_MGMT_PORT)
#endif
           )
            continue;
        else
        {
            if(SWDRVL4_LocalSetQosIngDscp2Color(unit,port,phb,color) == FALSE)
            {
            /* Remote units has been configured sucessfully,
            * but configuring local unit fails.
            * May need to design a error handling mechanism
            * for this condition
            */
            
                return FALSE;
            }
        }
    }

return TRUE;
}

static BOOL_T SWDRVL4_LocalSetQosIngDscp2Cos(UI32_T unit, UI32_T port,UI32_T phb,UI32_T color,UI32_T cos,UI32_T cfi)
{
#if (SYS_CPNT_HRDRV == FALSE)
    #if (SYS_CPNT_COS_PER_PORT == FALSE)
        return DEV_SWDRVL4_PMGR_SetQosIngDscp2Cos(unit,port,phb,color,cos,cfi);
    #else
        return DEV_SWDRVL4_PMGR_DeletePortCosMapping(DEV_SWDRVL4_MapTcpPortToCos, 0, port, mode, 0);
    #endif
#endif
}
BOOL_T SWDRVL4_SetQosIngDscp2Cos(UI32_T unit, UI32_T port,UI32_T phb,UI32_T color,UI32_T cos,UI32_T cfi)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
    if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
    {
        return FALSE;
    }
    if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
    && (port != SYS_ADPT_MGMT_PORT)
#endif
       )
    {
        if (port > max_port_number)
        {
            if (!STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit))
            {
                return FALSE;
            }
            dst_bmp = BIT_VALUE(drv_unit-1);
        }
        else
        {
            dst_bmp = BIT_VALUE(unit-1);
        }

        if(dst_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_INGDSCP2COS));
            isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if(isc_buf_p==NULL)
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
                return FALSE;
            }
        
            isc_buf_p->ServiceID = SWDRVL4_COS_INGDSCP2COS;
            isc_buf_p->unit = unit;
            isc_buf_p->port = port;
            isc_buf_p->phb = phb;
            isc_buf_p->color = color;
            isc_buf_p->cos = cos;
            isc_buf_p->cfi = cfi;
            
            if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
                  mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                  SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
            {
            /* If some of the stacking units fails, data among units will be
            * different. May need to design a error handling mechanism for this
            * condition
            */
            
                return FALSE;
            }
        }
    }
    else
    {
        if(SWDRVL4_LocalSetQosIngDscp2Cos(unit,port,phb,color,cos,cfi) == FALSE)
        {
        /* Remote units has been configured sucessfully,
        * but configuring local unit fails.
        * May need to design a error handling mechanism
        * for this condition
        */
        
        return FALSE;
        }
    }

return TRUE;
}

BOOL_T SWDRVL4_SetPortListQosIngDscp2Cos(UI8_T *port_list,UI32_T phb,UI32_T color,UI32_T cos,UI32_T cfi)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*    mref_handle_p;
    SWDRVL4_Rx_IscBuf_T*   isc_buf_p;
    UI32_T                 pdu_len, drv_unit;
    UI16_T                 dst_bmp=0;
#endif
    UI32_T                 max_port_number,lport,port,unit;

    
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* UIMSG_MGR_SetErrorCode(error_code); */
        return FALSE;
    }
#if (SYS_CPNT_STACKING == TRUE)
    drv_unit = 0;
    while(TRUE == STKTPLG_POM_GetNextDriverUnit(&drv_unit))
    {
        if (drv_unit != LOCAL_UNIT_ID)
        {
            dst_bmp |= BIT_VALUE(drv_unit-1);
        }
    }

    if(dst_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SWDRVL4_Rx_IscBuf_T), L_MM_USER_ID(SYS_MODULE_SWDRVL4, SWDRVL4_POOL_ID_ISC_SEND, SWDRVL4_COS_SET_PORT_LIST_INFO));
        isc_buf_p = (SWDRVL4_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu() fails", __FUNCTION__);
            return FALSE;
        }
        isc_buf_p->ServiceID = SWDRVL4_COS_SET_PORT_LIST_INFO;
        isc_buf_p->SubServiceID = SWDRVL4_COS_INGDSCP2COS;
        memcpy(isc_buf_p->portlist, port_list,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        isc_buf_p->phb = phb;
        isc_buf_p->color = color;
        isc_buf_p->cos = cos;
        isc_buf_p->cfi = cfi;
        
        if(0!=ISC_SendMcastReliable(dst_bmp, ISC_SWDRVL4_SID,
              mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
              SWDRVL4_TRY_TIMES, SWDRVL4_TIME_OUT, FALSE))
        {
        /* If some of the stacking units fails, data among units will be
        * different. May need to design a error handling mechanism for this
        * condition
        */
            return FALSE;
        }
    }
#endif        
    for(lport = 0;lport < SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
    {
        if(!SWDRVL4_IS_PORTLIST_MEMBER(port_list,lport+1))
            continue;
        unit = SWDRVL4_IFINDEX_TO_UNIT(lport+1);
        port = SWDRVL4_IFINDEX_TO_PORT(lport+1);
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            return FALSE;
        }

        if ( ((LOCAL_UNIT_ID != unit) || (port > max_port_number))
#if (SYS_CPNT_MGMT_PORT == TRUE)
            && (port != SYS_ADPT_MGMT_PORT)
#endif
           )
            continue;
        else
        {
            if(SWDRVL4_LocalSetQosIngDscp2Cos(unit,port,phb,color,cos,cfi) == FALSE)
            {
            /* Remote units has been configured sucessfully,
            * but configuring local unit fails.
            * May need to design a error handling mechanism
            * for this condition
            */
            
                return FALSE;
            }
        }
    }

return TRUE;
}


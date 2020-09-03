#ifndef AMTR_H
#define AMTR_H

/*------------------------------------------------------------------
 * FILE NAME - AMTR_TYPE.H
 *------------------------------------------------------------------
 * Purpose : This package defines the naming constants
 *           for the basic data type which is used by
 *           AMTR_MGR & AMTRDRV_MGR
 *
 * Modification History:
 *   Date          Modifier,    Reason
 *   ----------------------------------------------------------------
 *   04-29-2013    Charlie Chen Add new mac source type "security".
 *                              The mac entry with mac source type
 *                              "security" indicates the mac has been
 *                              authenticated by a security protocol.
 *-------------------------------------------------------------------
 * Copyright(C)
 *-------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_type.h"
#include "leaf_1493.h"
#if (SYS_CPNT_STACKING == TRUE)
#include "isc.h"
#endif
/*add by Tony.Lei*/
/*add by Tony.Lei*/
#define AMTRDRV_MGR_DEBUG_PERFORMANCE  TRUE
#define AMTR_TYPE_ENABLE_RESEND_MECHANISM   1
#define AMTR_TYPE_MEMCPY_MEMCOM_PERFORMANCE_TEST        1
#define AMTRDRV_SLAVE_REPEATING_TEST
#define AMTR_TYPE_DRV_SLAVE_ISC_TEST
#define AMTR_TYPE_MEMCPY_MEMCOM_PERFORMANCE_TEST        1
#define AMTR_TYPE_DEBUG_PACKET_FLOW                     1
/* NAMING CONSTANT DECLARATIONS
 */
#define AMTR_TYPE_MAC_LEN                            SYS_ADPT_MAC_ADDR_LEN

#define AMTR_TYPE_CHECK_INTRUSION_RETVAL_DROP        BIT_0
#define AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN       BIT_1

#define AMTR_TYPE_ADDRESS_WITHOUT_PRIORITY          0xff

#if (SYS_CPNT_ADD == TRUE)
#define AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK    SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT
#else
#define AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK    0
#endif

/* AMTR_TYPE_MAX_LOGICAL_VLAN_ID:
 *     When SYS_CPNT_VXLAN is TRUE, the VLAN id field in AMTR/AMTRDRV OM will be
 *     extended to contain two kinds of IDs. The first kind of ID is the Dot1q
 *     VLAN ID. The other kind of ID is the logical virtual forwarding instance(a.k.a. logical VFI)
 *     for VxLAN. The domain for these two types of ID in AMTR logical VLAN is
 *     shown below:
 *     |<-- Dot1q VLAN ID     -->| |<--                          logical VFI                              -->|
 *     [1 .. SYS_ADPT_MAX_VLAN_ID] [SYS_ADPT_MAX_VLAN_ID+1 .. SYS_ADPT_MAX_VLAN_ID+SYS_ADPT_VXLAN_MAX_NBR_VNI]
 *
 */
#if(SYS_CPNT_VXLAN == TRUE)
#define AMTR_TYPE_LOGICAL_VID_L_VFI_BASE (SYS_ADPT_MAX_VLAN_ID+1)

#define AMTR_TYPE_MAX_LOGICAL_VLAN_ID (SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VXLAN_MAX_NBR_VNI)
#define AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(logical_vid) ((logical_vid)>SYS_ADPT_MAX_VLAN_ID && (logical_vid)<=AMTR_TYPE_MAX_LOGICAL_VLAN_ID)
#define AMTR_TYPE_IS_REAL_VFI(real_vid) ((real_vid) >= SYS_ADPT_MIN_VXLAN_VFI_ID && ((real_vid) <= (SYS_ADPT_MIN_VXLAN_VFI_ID + SYS_ADPT_MAX_VFI_NBR)))
#define AMTR_TYPE_IS_VALID_VID_OR_REAL_VFI(real_vid) \
    (((real_vid) >= 1 && (real_vid) <= SYS_ADPT_MAX_VLAN_ID) || \
     (AMTR_TYPE_IS_REAL_VFI(real_vid)) \
    )
    /* SYS_ADPT_MIN_VXLAN_VFI_ID must be larger than SYS_ADPT_MAX_VLAN_ID
     */
    #if (SYS_ADPT_MIN_VXLAN_VFI_ID <= SYS_ADPT_MAX_VLAN_ID)
    #error "Incorrect definition of SYS_ADPT_MIN_VXLAN_VFI_ID"
    #endif
#define AMTR_TYPE_MAX_LOGICAL_PORT_ID (SYS_ADPT_TOTAL_NBR_OF_LPORT_INCLUDE_VXLAN)
#else
#define AMTR_TYPE_MAX_LOGICAL_VLAN_ID (SYS_ADPT_MAX_VLAN_ID)
#define AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(logical_vid) (FALSE)
#define AMTR_TYPE_IS_REAL_VFI(real_vid) (FALSE)
#define AMTR_TYPE_IS_VALID_VID_OR_REAL_VFI(real_vid) ((real_vid) >= 1 && (real_vid) <= SYS_ADPT_MAX_VLAN_ID)
#define AMTR_TYPE_MAX_LOGICAL_PORT_ID (SYS_ADPT_TOTAL_NBR_OF_LPORT)
#endif

#define AMTR_TYPE_LOGICAL_VID_IS_DOT1Q_VID(logical_vid) ((logical_vid)>1 && logical_vid<=SYS_ADPT_MAX_VLAN_ID)

/* TYPE DEFINITIONS
 */
typedef enum
{
    AMTR_TYPE_ADDRESS_ACTION_FORWARDING     = 1 ,
    AMTR_TYPE_ADDRESS_ACTION_DISCARD_BY_SA_MATCH,
    AMTR_TYPE_ADDRESS_ACTION_DISCARD_BY_DA_MATCH,
    AMTR_TYPE_ADDRESS_ACTION_TRAP_TO_CPU_ONLY   ,
    AMTR_TYPE_ADDRESS_ACTION_FORWARDING_AND_TRAP_TO_CPU
}AMTR_TYPE_AddressAction_T;

/* NOTE: Need to update AMTRDRV_OM_TOTAL_NUMBER_OF_SOURCE defined in amtrdrv_om.h
 *       if total number of source type is changed
 */
typedef enum
{
    AMTR_TYPE_ADDRESS_SOURCE_INTERNAL = VAL_dot1dTpFdbStatus_other,    /* 1 */
    AMTR_TYPE_ADDRESS_SOURCE_INVALID  = VAL_dot1dTpFdbStatus_invalid,  /* 2 */
    AMTR_TYPE_ADDRESS_SOURCE_LEARN    = VAL_dot1dTpFdbStatus_learned , /* 3 */
    AMTR_TYPE_ADDRESS_SOURCE_SELF     = VAL_dot1dTpFdbStatus_self,     /* 4 */
    AMTR_TYPE_ADDRESS_SOURCE_CONFIG   = VAL_dot1dTpFdbStatus_mgmt,     /* 5 */
    AMTR_TYPE_ADDRESS_SOURCE_SECURITY = VAL_dot1dTpFdbStatus_mgmt+1,   /* 6 */
    AMTR_TYPE_ADDRESS_SOURCE_MLAG     = VAL_dot1dTpFdbStatus_mgmt+2,   /* 7 */
    AMTR_TYPE_ADDRESS_SOURCE_MAX,
}AMTR_TYPE_AddressSource_T;

/* This is one to one mapping to Dot1d attribute ==> dot1d"static" may include dynamic & static.
 * NOTE: Need to update AMTRDRV_OM_TOTAL_NUMBER_OF_LIFETIME defined in amtrdrv_om.h
 *       if total number of lifetime type is changed
 */
typedef enum
{
    AMTR_TYPE_ADDRESS_LIFETIME_OTHER             = VAL_dot1dStaticStatus_other,  /* under create*/ /* 1 */
    AMTR_TYPE_ADDRESS_LIFETIME_INVALID           = VAL_dot1dStaticStatus_invalid,                  /* 2 */
    AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT         = VAL_dot1dStaticStatus_permanent,                /* 3 */
    AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET   = VAL_dot1dStaticStatus_deleteOnReset,            /* 4 */
    AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT = VAL_dot1dStaticStatus_deleteOnTimeout           /* 5 */
}AMTR_TYPE_AddressLifeTime_T;


/* Return types of adding MAC address.
 */
typedef enum
{
    AMTR_TYPE_RET_SUCCESS       ,
    AMTR_TYPE_RET_ERROR_UNKNOWN ,        
    AMTR_TYPE_RET_COLLISION     , 
    AMTR_TYPE_RET_L3_FAIL       ,     
}AMTR_TYPE_Ret_T; 

/* The type of Command to program chip & OM
 */
#define AMTR_TYPE_ENUM_NAME(arg) arg
#define AMTR_TYPE_TO_STR(arg) #arg

#define AMTR_TYPE_COMMAND_LIST(_) \
    _(AMTR_TYPE_COMMAND_NULL), \
    _(AMTR_TYPE_COMMAND_SET_ENTRY), \
    _(AMTR_TYPE_COMMAND_DELETE_ENTRY), \
    _(AMTR_TYPE_COMMAND_DELETE_ALL), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_SOURCE), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_PORT), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_LIFE_TIME), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_SOURCE), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_VID), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_VID_N_LIFETIME), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_VID_N_SOURCE), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_VID_N_PORT), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_LIFETIME), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_SOURCE), \
    _(AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_EXCEPT_CERTAIN_ADDR), \
    _(AMTR_TYPE_COMMAND_CHANGE_PORT_LIFE_TIME), \
    _(AMTR_TYPE_COMMAND_MAX)

typedef enum
{
    AMTR_TYPE_COMMAND_LIST(AMTR_TYPE_ENUM_NAME)
}AMTR_TYPE_Command_T;

/*-----------------
 * DATA TYPES
 *----------------*/
/* use this data structure to send ISC, need pack.
*/
typedef struct
{
    UI16_T vid;
    UI16_T ifindex;
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  action;
    UI8_T  source;
    UI8_T  life_time;
    UI8_T  priority;
#if (SYS_CPNT_VXLAN == TRUE)    
    UI8_T  r_vtep_ip[SYS_ADPT_IPV4_ADDR_LEN]; /* for static MAC of vxlan network port use */
#endif    
}__attribute__((packed, aligned(1)))AMTR_TYPE_AddrEntry_T;

/* Data record information in hash */
typedef struct
{
    AMTR_TYPE_AddrEntry_T address;
    UI32_T                aging_timestamp;
    /* If you have memory issue you can reduce this field "trunk_hit_bit_value_for_each_unit" to the max supported units
    * for example if we only can support 8 units stacking then you can modify this field as UI8_T
    */
    UI16_T                trunk_hit_bit_value_for_each_unit; /* This field is for the entry which is learnt on trunk port to keep each member units hit bit value */
    UI8_T                 hit_bit_value_on_local_unit;  /* This field is for the entry which is learnt on trunk port to keep the previous hit_bit value */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    BOOL_T                mark;
#endif
}__attribute__((packed, aligned(1)))AMTRDRV_TYPE_Record_T;


/* Cookie for message queue sending.;water_huang;2006.5.2
 */
typedef struct
{
    UI32_T  num_in_list;
    UI8_T   mac_list_ar[AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK][SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   mask_list_ar[AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK][SYS_ADPT_MAC_ADDR_LEN];
}AMTRDRV_MsgCookis;

typedef struct
{
    UI32_T                       blocked_command;
    UI16_T                       ifindex;
    UI16_T                       vid;
    AMTR_TYPE_AddressSource_T    source;
    AMTR_TYPE_AddressLifeTime_T  life_time;
#if (SYS_CPNT_STACKING == TRUE)
    ISC_Key_T                    isc_key;
#endif
    UI32_T                       vlan_counter;
    UI16_T                       vlanlist[AMTR_TYPE_MAX_LOGICAL_VLAN_ID + 1];
    AMTRDRV_MsgCookis            cookie;
    BOOL_T                       is_sync_op; /* TRUE - synchronous block command operation */
}AMTR_TYPE_BlockedCommand_T;
/* task event definition */
#define AMTR_TYPE_EVENT_ADDRESS_OPERATION         BIT_2
/* task event definition */
#define AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION         BIT_3
#if 0
/* use this data structure to send ISC, need pack.
 */
typedef struct
{
    UI16_T vid;
    UI16_T ifindex;
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  action;
    UI8_T  source;
    UI8_T  life_time;
}__attribute__((packed, aligned(1)))AMTR_TYPE_AddrEntry_T;

#define AMTRDRV_TYPE_ADDRENTRY_T_SIZEOF(type, field)         (sizeof ((type*) 0)->field) 
//#define AMTRDRV_OM_OFFSET(offset, type, field)        {type v; offset=(UI32_T)&v.##field - (UI32_T)&v;}
#define AMTRDRV_TYPE_ADDRENTRY_T_OFFSET_MAC(offset, type)        {type v; offset=(UI32_T)&v.mac - (UI32_T)&v;}
#define AMTRDRV_TYPE_ADDRENTRY_T_OFFSET_VID(offset, type)        {type v; offset=(UI32_T)&v.vid - (UI32_T)&v;}
typedef struct
{
	AMTR_TYPE_AddrEntry_T address;
/* the offset means the next valid index address entry 
 * the status means the status ower is valid or not  0: invalid 1: valid
*/
    UI8_T  status  ;		
	UI8_T  offset ;		
}__attribute__((packed, aligned(1)))AMTR_TYPE_AddrEntry_Forque_T;
/* add by Tony.Le i */
typedef struct amtr_type_naentry{
    UI16_T vid;
    UI16_T ifindex;
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
	UI8_T  next_offset;
}__attribute__((packed, aligned(1)))AMTR_TYPE_NAEntry_T;
typedef struct amtr_type_naentry_array{
     AMTR_TYPE_NAEntry_T * addr_entry_p;
	 int counter;
	 struct amtr_type_naentry_array * previous;
	 struct amtr_type_naentry_array * next;
}__attribute__((packed, aligned(1)))AMTR_TYPE_NAEntry_Array_T;
typedef struct amtr_type_naentry_array_p{
       AMTR_TYPE_NAEntry_Array_T * used_header;
	   AMTR_TYPE_NAEntry_Array_T * used_tailer;
	   AMTR_TYPE_NAEntry_Array_T * freed_header;
	   AMTR_TYPE_NAEntry_Array_T * freed_tailer;
} AMTR_TYPE_NAEntry_Array_P_T;

#endif

/* For AMTRDRV, the max PUD length = SYS_ADPT_ISC_MAX_PDU_LEN - AMTRDRV_TYPE_ISC_HEADER
 * AMTRDRV_TYPE_ISC_HEADER = service_id + number_of_entries = 2 + 2
 * AMTRDRV_MGR_DirectCallServiceID_T    service_id = 2 bytes.
 * UI16_T                        number_of_entries = 2 bytes.
 * If anyone want to change structure "AMTRDRV_MGR_ISCBuffer_T",
 * AMTRDRV_OM_ISC_HEADER must be updated.
 */
#define AMTRDRV_TYPE_ISC_HEADER                  (2+2)
#define AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET   ((SYS_ADPT_ISC_MAX_PDU_LEN - AMTRDRV_TYPE_ISC_HEADER)/sizeof(AMTRDRV_TYPE_Record_T))
#define AMTRDRV_TYPE_NUM_ISC_IN_PROCESS           3

/* for tracking addr entry modification and callback CSCs
 * that need these information.
 */
typedef struct
{
    UI16_T  vid;
    UI16_T  ifindex;
    UI8_T   mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   source;
}AMTR_TYPE_EventEntry_T;

/* for port move callback
 */
typedef struct
{
    AMTR_TYPE_EventEntry_T  event_entry;
    UI32_T                  original_port;
}AMTR_TYPE_PortMoveEntry_T;

typedef struct
{
    UI32_T   total_address_count;
    UI32_T   total_static_address_count;
    /*total_dymaic + total_static + other + self = total_number */
    UI32_T   total_dynamic_address_count;
    /*the counter of lport 1 is recorded on counter[0].*/
    UI32_T   static_address_count_by_port[AMTR_TYPE_MAX_LOGICAL_PORT_ID];	/* by port;static: life_time != delete on timeout */
    UI32_T   learnt_address_count_by_port[AMTR_TYPE_MAX_LOGICAL_PORT_ID];  /* by port source=learnt */
    UI32_T   security_address_count_by_port[AMTR_TYPE_MAX_LOGICAL_PORT_ID];/* by port source=security */
    UI32_T   dynamic_address_count_by_port[AMTR_TYPE_MAX_LOGICAL_PORT_ID];
    UI32_T   config_address_count_by_port[AMTR_TYPE_MAX_LOGICAL_PORT_ID];
    UI32_T   static_address_count_by_vid[AMTR_TYPE_MAX_LOGICAL_VLAN_ID];    /* valid vid: 1..AMTR_TYPE_MAX_LOGICAL_VLAN_ID*/
    UI32_T   dynamic_address_count_by_vid[AMTR_TYPE_MAX_LOGICAL_VLAN_ID];
}AMTR_TYPE_Counters_T;

/* MACRO DEFINITIONS
 */
#define AMTR_TYPE_COUNTER_FLAGS                           (1<< 0)
#define AMTR_TYPE_DONOTDELETEFROMCHIP_FLAGS               (1<< 1)
#define AMTR_TYPE_OM_CHIP_ATTRIBUTE_NOT_SYNC_FLAGS        (1<< 2)
#define AMTR_ISSET_COUNTER_FLAG(state)                (state&AMTR_TYPE_COUNTER_FLAGS)  
#define AMTR_ISSET_DONOTDELETEFROMCHIP_FLAG(state)    (state&AMTR_TYPE_DONOTDELETEFROMCHIP_FLAGS)  
#define AMTR_ISSET_OMCHIPATTRIBUTENOTSYNC_FLAG(state) (state&AMTR_TYPE_OM_CHIP_ATTRIBUTE_NOT_SYNC_FLAGS)

/* AMTR Task synchronize AMTR_TYPE_SYNC2HISAM_NUM records from Hash table to Hisam table every time.
 */
#ifdef SYS_ADPT_AMTR_TYPE_SYNC2HISAM_NUM
#define AMTR_TYPE_SYNC2HISAM_NUM        SYS_ADPT_AMTR_TYPE_SYNC2HISAM_NUM
#else
#define AMTR_TYPE_SYNC2HISAM_NUM        100
#endif

#endif

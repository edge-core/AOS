
/*--------------------------------------------------------------------------+
 * FILE NAME - amtr_mgr.h                                                   +
 *--------------------------------------------------------------------------+
 * ABSTRACT: This file defines Address Monitor functions                    +
 *                                                                          +
 * 2001/06/13 arthur                                                        +
 * 2004/07/13 water: refinement and use CPU learning                        +
 *--------------------------------------------------------------------------*/
/* NOTES:
 * 1. RFC 1493 MIB contains the following group:
 *    a) the dot1dBase   group (dot1dBridge 1)  -- implemented in sta_mgr
 *    b) the dot1dStp    group (dot1dBridge 2)  -- implemented in sta_mgr
 *    c) the dot1dSr     group (dot1dBridge 3)  -- not implemented
 *    d) the dot1dTp     group (dot1dBridge 4)  -- implemented partial, partial in nmtr_mgr
 *    e) the dot1dStatic group (dot1dBridge 5)  -- implemented here
 * 2. RFC 2674Q MIB
 *    a) the dot1qTp group 1
 *    b) the dot1qTp group 2
 *    c) the dot1qStatic group 1
 *----------------------------------------------------------------------------
 * After refinement:
*
 * <1>
 *  entry = vid + mac + ifindex + source + life_time + action
 *
 *  addr_entry.attribute => addr_entry.source + addr_entry.life_time
 *
 *  life_time:
 *      under create(for SNMP)
 *      empty (to delete an entry)
 *      permanent
 *      delete on reset
 *      delete on timeout
 *
 *    source:
 *      Learnt(learn from chip)
 *      Config(User config)
 *      Internal (No Age out)(UI invisible)( Community VLAN)
 *      Self (management VLAN mac)
 * <2>
 *    AMTR_MGR keep the amtr_port_info[ifindex] table.
 *  amtr_port_info.protocol:
 *      normal(the port is normal port)
 *      Psec      (secure port)
 *      Dot1X (secure port)
 *
 *  amtr_port_info.learn_with_count:
 *      (AMTR refer to learn if the port is a secure port)
 *
 *  amtr_port_info.life_time
 *      (the entrys are learnt with the life_time)
 */

#ifndef AMTR_MGR_H
#define AMTR_MGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "amtr_type.h"
#include "sys_dflt.h"
#include "sys_callback_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define AMTR_MGR_DEBUG
#define AMTR_MGR_EVENT_TIMER        BIT_1    /* timer event */
#define AMTR_MGR_IPCMSG_TYPE_SIZE   sizeof(union AMTR_MGR_IpcMsg_Type_U)

#define AMTR_MGR_MAC_NTFY_ACT_ADD                  0
#define AMTR_MGR_MAC_NTFY_ACT_REM                  1
#define AMTR_MGR_MAC_NTFY_LST_MAX                  500

#define AMTR_MGR_MAC_NTFY_ACT_MASK                 0x8000
#define AMTR_MGR_MAC_NTFY_VID_MASK                 0x0FFF
#define AMTR_MGR_MAC_NTFY_INTERVAL_SEND_NOWAIT     0
#define AMTR_MGR_NBR_OF_BYTE_FOR_1BIT_VLAN_LIST ((SYS_ADPT_MAX_VLAN_ID + 7) / 8)

/* command used in IPC message
 */
enum
{
    AMTR_MGR_IPC_GETRUNNINGAGINGTIME,
    AMTR_MGR_IPC_SETADDRENTRY,
    AMTR_MGR_IPC_DELETEADDR,
    AMTR_MGR_IPC_DELETEADDRBYLIFETIME,
    AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDLPORT,
    AMTR_MGR_IPC_DELETEADDRBYSOURCEANDLPORT,
    AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDVIDRANGEANDLPORT,
    AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDMSTIDANDLPORT,
    AMTR_MGR_IPC_DELETEADDRBYVIDANDLPORT,
    AMTR_MGR_IPC_GETEXACTADDRENTRY,
    AMTR_MGR_IPC_GETEXACTADDRENTRYFROMCHIP,
    AMTR_MGR_IPC_GETNEXTMVADDRENTRY,
    AMTR_MGR_IPC_GETNEXTVMADDRENTRY,
    AMTR_MGR_IPC_GETNEXTIFINDEXADDRENTRY,
    AMTR_MGR_IPC_GETNEXTRUNNINGSTATICADDRENTRY,
    AMTR_MGR_IPC_SETINTERVENTIONENTRY,
    AMTR_MGR_IPC_DELETEINTERVENTIONENTRY,
    AMTR_MGR_IPC_CREATEMULTICASTADDRTBLENTRY,
    AMTR_MGR_IPC_DESTROYMULTICASTADDRTBLENTRY,
    AMTR_MGR_IPC_SETMULTICASTPORTMEMBER,
    AMTR_MGR_IPC_GETAGINGSTATUS,
    AMTR_MGR_IPC_GETRUNNINGAGINGSTATUS,
    AMTR_MGR_IPC_SETAGINGSTATUS,
    AMTR_MGR_IPC_GETDOT1DTPAGINGTIME,
    AMTR_MGR_IPC_SETDOT1DTPAGINGTIME,
#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)    
    AMTR_MGR_IPC_GETHASHLOOKUPDEPTHFROMCONFIG,
    AMTR_MGR_IPC_GETHASHLOOKUPDEPTHFROMCHIP,
    AMTR_MGR_IPC_SETHASHLOOKUPDEPTH,
#endif    
    AMTR_MGR_IPC_GETDOT1DTPFDBENTRY,
    AMTR_MGR_IPC_GETNEXTDOT1DTPFDBENTRY,
    AMTR_MGR_IPC_GETDOT1DSTATICENTRY,
    AMTR_MGR_IPC_GETNEXTDOT1DSTATICENTRY,
    AMTR_MGR_IPC_SETDOT1DSTATICADDRESS,
    AMTR_MGR_IPC_SETDOT1DSTATICRECEIVEPORT,
    AMTR_MGR_IPC_SETDOT1DSTATICALLOWEDTOGOTO,
    AMTR_MGR_IPC_SETDOT1DSTATICSTATUS,
    AMTR_MGR_IPC_GETDOT1QFDBENTRY,
    AMTR_MGR_IPC_GETNEXTDOT1QFDBENTRY,
    AMTR_MGR_IPC_GETDOT1QTPFDBENTRY,
    AMTR_MGR_IPC_GETNEXTDOT1QTPFDBENTRY,
    AMTR_MGR_IPC_GETDOT1QSTATICUNICASTENTRY,
    AMTR_MGR_IPC_GETNEXTDOT1QSTATICUNICASTENTRY,
    AMTR_MGR_IPC_SETDOT1QSTATICUNICASTALLOWEDTOGOTO,
    AMTR_MGR_IPC_SETDOT1QSTATICUNICASTSTATUS,
    AMTR_MGR_IPC_SETLEARNINGMODE,
    AMTR_MGR_IPC_ISPORTSECURITYENABLE,
    AMTR_MGR_IPC_NOTIFYINTRUSIONMAC,
    AMTR_MGR_IPC_NOTIFYSECURITYPORTMOVE,
    AMTR_MGR_IPC_GETMACLEARNINGBYPORT,
    AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDVID,
    AMTR_MGR_IPC_SETMACLEARNINGBYPORT,
    AMTR_MGR_IPC_SETCPUMAC,
    AMTR_MGR_IPC_SETROUTERADDITIONALCTRLREG,
    AMTR_MGR_IPC_SETMACNOTIFYGLOBALSTATUS,
    AMTR_MGR_IPC_SETMACNOTIFYINTERVAL,
    AMTR_MGR_IPC_SETMACNOTIFYPORTSTATUS,
    AMTR_MGR_IPC_GETRUNNINGMACNOTIFYINTERVAL,
    AMTR_MGR_IPC_GETRUNNINGMACNOTIFYGLOBALSTATUS,
    AMTR_MGR_IPC_GETRUNNINGMACNOTIFYPORTSTATUS,
    AMTR_MGR_IPC_CLEARCOLLISIONVLANMACTABLE,
    AMTR_MGR_IPC_GETVLANLEARNINGSTATUS,
    AMTR_MGR_IPC_GETRUNNINGVLANLEARNINGSTATUS,
    AMTR_MGR_IPC_SETVLANLEARNINGSTATUS,
    AMTR_MGR_IPC_SETMLAGMACNOTIFYPORTSTATUS,
    AMTR_MGR_IPC_SETADDRENTRYFORMLAG,
    AMTR_MGR_IPC_DELETEADDRFORMLAG,
    AMTR_MGR_IPC_AUTHENTICATEPACKET,
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in LACP_MGR_IpcMsg_T.data
 */
#define AMTR_MGR_GET_MSG_SIZE(field_name)                       \
            (AMTR_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((AMTR_MGR_IpcMsg_T*)0)->data.field_name))

#define AMTR_MGR_MAC_NTFY_TST_BIT(pbmp, ifidx) \
            (pbmp[(ifidx-1)>>3] & (1 << (7 - ((ifidx-1) & 7))))

#define AMTR_MGR_MAC_NTFY_SET_BIT(pbmp, ifidx) \
            do { pbmp[(ifidx-1)>>3] |=  (1 << (7 - ((ifidx-1) & 7))); } while(0)

#define AMTR_MGR_MAC_NTFY_CLR_BIT(pbmp, ifidx) \
            do { pbmp[(ifidx-1)>>3] &= ~(1 << (7 - ((ifidx-1) & 7))); } while(0)

#define AMTR_MGR_VLAN_LEARNING_TST_BIT(pbmp, vid) \
            (pbmp[(vid-1)>>3] & (1 << (7 - ((vid-1) & 7))))

#define AMTR_MGR_VLAN_LEARNING_SET_BIT(pbmp, vid) \
            do { pbmp[(vid-1)>>3] |=  (1 << (7 - ((vid-1) & 7))); } while(0)

#define AMTR_MGR_VLAN_LEARNING_CLR_BIT(pbmp, vid) \
            do { pbmp[(vid-1)>>3] &= ~(1 << (7 - ((vid-1) & 7))); } while(0)

#define AMTR_MGR_MAC_NTFY_NONE    0
#define AMTR_MGR_MAC_NTFY_TRAP    BIT_0
#define AMTR_MGR_MAC_NTFY_MLAG    BIT_1

/* DATA TYPE DECLARATIONS
 */

/* for AMTR_MGR_IntrusionMac_CallBack
 */
typedef BOOL_T (* AMTR_IntrusionMac_T) ( UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type);

typedef enum
{
   AMTR_MGR_PROTOCOL_NORMAL = 0,
   AMTR_MGR_PROTOCOL_PSEC,
   AMTR_MGR_PROTOCOL_DOT1X,
   AMTR_MGR_PROTOCOL_MACAUTH,
   AMTR_MGR_PROTOCOL_MACAUTH_OR_DOT1X,
   AMTR_MGR_PROTOCOL_MAX
} AMTR_MGR_ProtocolMode_T;

#define  AMTR_MGR_ENABLE_MACLEARNING_ONPORT   1
#define  AMTR_MGR_DISABLE_MACLEARNING_ONPORT   2
typedef struct
{
   AMTR_TYPE_AddressLifeTime_T life_time;   /* Permanent/Other/Delete on Reset/Delete on Timeout*/
   AMTR_MGR_ProtocolMode_T protocol;        /* Pseec/Dot1X/Normal*/
   UI32_T learn_with_count;                 /* learn with the count*/
   UI32_T is_learn;
} AMTR_MGR_PortInfo_T;

typedef struct
{
    /* key
     */
    UI8_T   dot1d_static_address[6];
    UI32_T  dot1d_static_receive_port;

    UI8_T   dot1d_static_allowed_to_go_to[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI32_T  dot1d_static_status;
} AMTR_MGR_Dot1dStaticEntry_T;

typedef struct
{
    /* key
     */
    UI8_T   dot1d_tp_fdb_address[6];
    UI32_T  dot1d_tp_fdb_port;
    UI32_T  dot1d_tp_fdb_status;
} AMTR_MGR_Dot1dTpFdbEntry_T;

typedef struct
{
    /* key
     */
    UI32_T   dot1q_fdb_id;
    UI32_T  dot1q_fdb_dynamic_count;
} AMTR_MGR_Dot1qFdbEntry_T;


typedef struct
{
    /* key
     */
    UI8_T   dot1q_tp_fdb_address[6];
    UI32_T  dot1q_tp_fdb_port;
    UI32_T  dot1q_tp_fdb_status;
} AMTR_MGR_Dot1qTpFdbEntry_T;


typedef struct
{
    /* key
     */
    UI8_T   dot1q_static_unicast_address[6];
    UI32_T  dot1q_static_unicast_receive_port;
    UI8_T   dot1q_static_unicast_allowed_to_go_to[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI32_T  dot1q_static_unicast_status;
} AMTR_MGR_Dot1qStaticUnicastEntry_T;

typedef enum
{
    AMTR_MGR_UNKNOWN,
    AMTR_MGR_UPORT_DOWN,
    AMTR_MGR_TRUNK_MEMBER_ADD,
    AMTR_MGR_TRUNK_DESTORY,
    AMTR_MGR_ADMIN_DISABLE,
    AMTR_MGR_LEARNING_DISABLE,
} AMTR_MGR_DeleteByPortCallbackReason_T;

typedef enum
{
    AMTR_MGR_GET_ALL_ADDRESS = 0,
    AMTR_MGR_GET_STATIC_ADDRESS,
    AMTR_MGR_GET_DYNAMIC_ADDRESS,
    AMTR_MGR_GET_MIB_ALL_ADDRESS
} AMTR_MGR_GetMode_T;

typedef struct
{
    UI32_T  ifidx;
    UI16_T  act_vid; /* act : 0x8000, bit15: 1 - add, 0 - remove
                      * vid : 0x0fff
                      */
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
}   AMTR_MGR_MacNotifyRec_T;

/* IPC message structure
 */
typedef struct
{
    union AMTR_MGR_IpcMsg_Type_U
    {
        UI32_T                     cmd;
        BOOL_T                     ret_bool;
        AMTR_TYPE_Ret_T            ret_amtr_type;        
        SYS_TYPE_Get_Running_Cfg_T ret_running_cfg;
    } type; /* the intended action or return value */

    union
    {
        UI32_T                      arg_ui32;
        AMTR_TYPE_AddrEntry_T       arg_addr_entry;
        AMTR_TYPE_AddressLifeTime_T arg_address_life_time;
        AMTR_MGR_Dot1dTpFdbEntry_T  arg_dot1d_tp_fdb_entry;
        AMTR_MGR_Dot1dStaticEntry_T arg_dot1d_static_entry;
        AMTR_MGR_Dot1qFdbEntry_T    arg_dot1q_fdb_entry;
        BOOL_T                      arg_bool;

        struct
        {
            UI32_T arg_ui32;
            UI8_T  arg_mac[AMTR_TYPE_MAC_LEN];
        } arg_grp_ui32_mac;

        struct
        {
            UI32_T                      arg_ui32;
            AMTR_TYPE_AddressLifeTime_T arg_addresslifetime;
        } arg_grp_ui32_alt;

        struct
        {
            UI32_T                      arg_ui32_1;
            UI32_T                      arg_ui32_2;
            AMTR_TYPE_AddressLifeTime_T arg_addresslifetime;
        } arg_grp_ui32_ui32_alt;

        struct
        {
            AMTR_TYPE_AddrEntry_T arg_addr_entry;
            UI32_T                arg_ui32;
        } arg_grp_addrentry_ui32;

        struct
        {
            UI32_T arg_ui32;
            UI8_T  arg_mac[AMTR_TYPE_MAC_LEN];
            UI8_T  arg_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            UI8_T  arg_tagged[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
        } arg_grp_ui32_mac_ports_tagged;

        struct
        {
            UI8_T arg_mac_1[AMTR_TYPE_MAC_LEN];
            UI8_T arg_mac_2[AMTR_TYPE_MAC_LEN];
        } arg_grp_mac_mac;

        struct
        {
            UI8_T  arg_mac[AMTR_TYPE_MAC_LEN];
            UI32_T arg_ui32;
        } arg_grp_mac_ui32;

        struct
        {
            UI8_T  arg_mac[AMTR_TYPE_MAC_LEN];
            UI8_T  arg_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
        } arg_grp_mac_ports;

        struct
        {
            UI32_T                     arg_ui32;
            AMTR_MGR_Dot1qTpFdbEntry_T arg_dot1q_tp_fdb;
        } arg_grp_ui32_dot1qtpfdb;

        struct
        {
            UI32_T                             arg_ui32;
            AMTR_MGR_Dot1qStaticUnicastEntry_T arg_dot1q_static_unicast;
        } arg_grp_ui32_dot1qstaticunicast;

        struct
        {
            UI32_T arg_ui32;
            UI8_T  arg_mac[AMTR_TYPE_MAC_LEN];
            UI8_T  arg_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
        } arg_grp_ui32_mac_ports;

        struct
        {
            UI32_T arg_ui32_1;
            UI8_T  arg_mac[AMTR_TYPE_MAC_LEN];
            UI32_T arg_ui32_2;
        } arg_grp_ui32_mac_ui32;

        struct
        {
            UI32_T arg_ui32;
            UI16_T arg_ui16_1;
            UI8_T  arg_mac_1[AMTR_TYPE_MAC_LEN];
            UI8_T  arg_mac_2[AMTR_TYPE_MAC_LEN];
            UI16_T arg_ui16_2;
        } arg_grp_ui32_ui16_mac_mac_ui16;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI8_T  arg_mac[AMTR_TYPE_MAC_LEN];
            UI32_T arg_ui32_3;
        } arg_grp_ui32_ui32_mac_ui32;
        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
        } arg_port_mode_ui32_ui32;
        struct 
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI16_T arg_ui16_1;
            UI16_T arg_ui16_2;
        }arg_ui32_ui32_ui16_ui16;
        struct 
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI16_T arg_ui16_1;
            UI16_T arg_ui16_2;
            BOOL_T arg_bool_1;
        }arg_ui32_ui32_ui16_ui16_bool;
        struct 
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI32_T arg_ui32_3;
        }arg_ui32_ui32_ui32;
        struct
        {
            UI32_T arg_ui32;
            UI8_T  arg_mac[AMTR_TYPE_MAC_LEN];
            UI32_T arg_bool;
        } arg_grp_ui32_mac_bool;
        struct
        {
            UI32_T arg_ui32;
            BOOL_T arg_bool;
        } arg_grp_ui32_bool;

        struct
        {
            UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI32_T vid;
            UI32_T lport;
            SYS_CALLBACK_MGR_AuthenticatePacket_Result_T auth_result;
        } arg_auth_pkt;
    } data; /* the argument(s) for the function corresponding to cmd */
} AMTR_MGR_IpcMsg_T;

typedef struct
{
    UI32_T  mac_ntfy_time_stamp;    /* in ticks, time stamp for checking if notification should be launched  */
    UI16_T  mac_ntfy_used_head;     /* head for the queue       */
    UI16_T  mac_ntfy_used_cnt;      /* used count for the queue */
    UI32_T  mac_ntfy_interval;      /* in ticks, interval for each notification */
    UI8_T   mac_ntfy_en_lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]; /* port status bitmap */
    BOOL_T  mac_ntfy_en_global;     /* global status */
    AMTR_MGR_MacNotifyRec_T mac_ntfy_rec_lst[AMTR_MGR_MAC_NTFY_LST_MAX];/* queue list for processing */
}AMTR_MGR_MacNotify_T;

typedef struct
{
    UI32_T  mlag_mac_ntfy_time_stamp;    /* in ticks, time stamp for checking if notification should be launched  */
    UI16_T  mlag_mac_ntfy_used_head;     /* head for the queue       */
    UI16_T  mlag_mac_ntfy_used_cnt;      /* used count for the queue */
    UI32_T  mlag_mac_ntfy_interval;      /* in ticks, interval for each notification */
    UI8_T   mlag_mac_ntfy_en_lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]; /* port status bitmap */
    AMTR_MGR_MacNotifyRec_T mlag_mac_ntfy_rec_lst[AMTR_MGR_MAC_NTFY_LST_MAX];/* queue list for processing */
}AMTR_MGR_MlagMacNotify_T;

typedef struct
{
    UI8_T   vlan_learn_dis[AMTR_MGR_NBR_OF_BYTE_FOR_1BIT_VLAN_LIST]; /* vid learning disable status bitmap */
}AMTR_MGR_VlanLearning_T;

typedef struct 
{
    UI32_T  ovsvtep_mac_ntfy_time_stamp;    /* in ticks, time stamp for checking if notification should be launched  */
    UI16_T  ovsvtep_mac_ntfy_used_head;     /* head for the queue       */
    UI16_T  ovsvtep_mac_ntfy_used_cnt;      /* used count for the queue */
    UI32_T  ovsvtep_mac_ntfy_interval;      /* in ticks, interval for each notification */
    AMTR_MGR_MacNotifyRec_T ovsvtep_mac_ntfy_rec_lst[AMTR_MGR_MAC_NTFY_LST_MAX];/* queue list for processing */
}AMTR_MGR_OvsvtepMacNotify_T;

typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
#if (SYS_CPNT_VXLAN == TRUE)
    AMTR_MGR_PortInfo_T      amtr_port_info[SYS_ADPT_TOTAL_NBR_OF_LPORT_INCLUDE_VXLAN]; /*unit(1)+port(1)<=>ifindex(1)<=>amtr_port_info[0]*/
#else
    AMTR_MGR_PortInfo_T      amtr_port_info[SYS_ADPT_TOTAL_NBR_OF_LPORT]; /*unit(1)+port(1)<=>ifindex(1)<=>amtr_port_info[0]*/
#endif
    AMTR_MGR_MacNotify_T     amtr_mac_notify;
    AMTR_MGR_MlagMacNotify_T amtr_mlag_mac_notify;
    AMTR_MGR_OvsvtepMacNotify_T amtr_ovs_mac_notify;
    AMTR_MGR_VlanLearning_T  amtr_vlan_learning;
    AMTR_TYPE_Counters_T     counters;
} AMTR_MGR_Shmem_Data_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the Address monitor module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void AMTR_MGR_ProvisionComplete(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Init
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Invoked by root.c()
 *------------------------------------------------------------------------*/
void AMTR_MGR_Init(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTR_MGR_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_EnterTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will create and initialize all system resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTR_MGR_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void AMTR_MGR_SetTransitionMode(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_EnterMasterMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTR_MGR_EnterMasterMode(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_EnterSlaveMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTR_MGR_EnterSlaveMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_HandleHotInsertion
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * -------------------------------------------------------------------------*/
void AMTR_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void AMTR_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetPortInfo
 *------------------------------------------------------------------------------
 * Purpose  : This function Set the port Infomation
 * INPUT    :  ifindex                      -set which port
 *          port_info -> learning_mode      -The port is learn or no learn now
 *          port_info -> life_time          -When entry is learnt from this port, it's life time
 *          port_info -> count              -The port is learning with the count
 *          port_info -> protocol           -The port is normal port or secur port(port security or dot1X)
 * OUTPUT   : None
 * RETURN   : BOOL_T                        - True : successs, False : failed
 * NOTE     : (refinement; water_huang create(93.8.10))
 *          1. The PortInfo has to be set default value in AMTR_MGR_EnterMasterMode().
 *          2. Other CSCs can use this API to set port information.
 *          3. In this API, AMTR will not check the relationship between inputs.
 *             Callers must set the correct input to this APIs.
 *          4. Iff amtr_port_info[ifindex].protocol is PSEC or DOT1X, AMTR will check the
 *             amtr_port_info[ifindex].learn_with_count when learning.
 *
 *          protocol:   AMTR_MGR_PROTOCOL_NORMAL
 *                      AMTR_MGR_PROTOCOL_PSEC
 *                      AMTR_MGR_PROTOCOL_DOT1X
 *
 *          life_time:  AMTRDRV_OM_ADDRESS_LIFETIME_OTHER
 *                      AMTRDRV_OM_ADDRESS_LIFETIME_PERMANENT
 *                      AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONTIMEOUT
 *                      AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONRESET
 *
 *          count:      if set "0"  - it means no learn !
 *                      else        - learn with the "count"!
 *
 *     count    |   dyanmic(del on timeout)                     static(!del on timeout)
 *   --------------------------------------------------------------------------------
 *  normal port |   ignore                                      ignore
 *              |
 *  secure port |   SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY   SYS_ADPT_MAX_NBR_OF_STATIC_MAC_PER_PORT
 *              |
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_SetPortInfo(UI32_T ifindex, AMTR_MGR_PortInfo_T *port_info);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetRunningAgeingTime
 *------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          non-default ageing time can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None
 * OUTPUT:  *aging_time   - the non-default ageing time
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default ageing time.
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetRunningAgingTime(UI32_T *aging_time);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will set address table entry
 * INPUT :   addr_entry -> vid       - VLAN ID
 *           addr_entry -> mac       - MAC address
 *           addr_entry -> ifindex   - interface index
 *           addr_entry -> action    -Forwarding/Discard by SA match/Discard by DA match/Trap to CPU only/Forwarding and trap to CPU
 *           addr_entry -> source    -Config/Learn/Internal/Self
 *           addr_entry -> life_time -permanent/Other/Delete on Reset/Delete on Timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T status           - Success or not
 * NOTE    : 1. parameters:
 *              action:     AMTRDRV_OM_ADDRESS_ACTION_FORWARDING
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYSAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYDAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_TRAPTOCPUONLY
 *                          AMTRDRV_OM_ADDRESS_ACTION_FORWARDINGANDTRAPTOCPU
 *
 *              source:     AMTRDRV_OM_ADDRESS_SOURCE_LEARN
 *                          AMTRDRV_OM_ADDRESS_SOURCE_CONFIG
 *                          AMTRDRV_OM_ADDRESS_SOURCE_INTERNAL
 *                          AMTRDRV_OM_ADDRESS_SOURCE_SELF
 *
 *              life_time:  AMTRDRV_OM_ADDRESS_LIFETIME_OTHER
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_INVALID
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_PERMANENT
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONRESET
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONTIMEOUT
 *           2.Set CPU mac don't care the ifinedx, but can't be "0".
 *           In AMTR, ifindex ==0 means just set to OM, don't program chip.
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_SetAddrEntry( AMTR_TYPE_AddrEntry_T *addr_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetAddrEntryForMlag
 *------------------------------------------------------------------------
 * FUNCTION: This function will set address table entry
 * INPUT :   addr_entry -> vid       - VLAN ID
 *           addr_entry -> mac       - MAC address
 *           addr_entry -> ifindex   - interface index
 *           addr_entry -> action    - Forwarding/Discard by SA match/Discard by DA match/Trap to CPU only/Forwarding and trap to CPU
 *           addr_entry -> source    - Config/Learn/Internal/Self/Security
 *           addr_entry -> life_time - permanent/Other/Delete on Reset/Delete on Timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T status           - Success or not
 * NOTE    :
 *           1.parameter:
 *             action:    AMTR_TYPE_ADDRESS_ACTION_FORWARDING
 *                        AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYSAMATCH
 *                        AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYDAMATCH
 *                        AMTRDRV_OM_ADDRESS_ACTION_TRAPTOCPUONLY
 *                        AMTR_TYPE_ADDRESS_ACTION_FORWARDINGANDTRAPTOCPU
 *
 *             source:    AMTR_TYPE_ADDRESS_SOURCE_LEARN
 *                        AMTR_TYPE_ADDRESS_SOURCE_CONFIG
 *                        AMTR_TYPE_ADDRESS_SOURCE_INTERNAL
 *                        AMTR_TYPE_ADDRESS_SOURCE_SELF
 *                        AMTR_TYPE_ADDRESS_SOURCE_SECURITY
 *
 *             life_time: AMTR_TYPE_ADDRESS_LIFETIME_OTHER
 *                        AMTR_TYPE_ADDRESS_LIFETIME_INVALID
 *                        AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT
 *                        AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET
 *                        AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 *           2.Set CPU mac don't care the ifinedx, but can't be "0".
 *             In AMTR, ifindex==0 means just set to OM, don't program chip.
 *           3.The mac entry which is added through this function will not be
 *             notified to MLAG, and this function shall only be called by MLAG.
 *------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetAddrEntryForMlag( AMTR_TYPE_AddrEntry_T *addr_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetAddrEntryWithPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function will set address table entry
 * INPUT :   addr_entry -> vid       - VLAN ID
 *           addr_entry -> mac       - MAC address
 *           addr_entry -> ifindex   - interface index
 *           addr_entry -> action    -Forwarding/Discard by SA match/Discard by DA match/Trap to CPU only/Forwarding and trap to CPU
 *           addr_entry -> source    -Config/Learn/Internal/Self
 *           addr_entry -> life_time -permanent/Other/Delete on Reset/Delete on Timeout
 *           addr_entry -> priority  - QoS priority
 * OUTPUT  : None
 * RETURN  : BOOL_T status           - Success or not
 * NOTE    : 1. parameters:
 *              action:     AMTRDRV_OM_ADDRESS_ACTION_FORWARDING
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYSAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYDAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_TRAPTOCPUONLY
 *                          AMTRDRV_OM_ADDRESS_ACTION_FORWARDINGANDTRAPTOCPU
 *
 *              source:     AMTRDRV_OM_ADDRESS_SOURCE_LEARN
 *                          AMTRDRV_OM_ADDRESS_SOURCE_CONFIG
 *                          AMTRDRV_OM_ADDRESS_SOURCE_INTERNAL
 *                          AMTRDRV_OM_ADDRESS_SOURCE_SELF
 *
 *              life_time:  AMTRDRV_OM_ADDRESS_LIFETIME_OTHER
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_INVALID
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_PERMANENT
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONRESET
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONTIMEOUT
 *           2.Set CPU mac don't care the ifinedx, but can't be "0".
 *           In AMTR, ifindex ==0 means just set to OM, don't program chip.
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_SetAddrEntryWithPriority(AMTR_TYPE_AddrEntry_T *addr_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddr
 * -------------------------------------------------------------------------
 * FUNCTION: This function will clear specific address table entry
 * INPUT   : UI16_T  vid   - vlan ID
 *           UI8_T  *Mac   - Specified MAC to be cleared
 *
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE:     This API can't delete CPU MAC!
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddr(UI32_T vid, UI8_T *mac);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrForMlag
 * -------------------------------------------------------------------------
 * FUNCTION: This function will clear specific address table entry (either
 *           dynamic or static)
 * INPUT   : UI16_T  vid   - vlan ID
 *           UI8_T  *Mac   - Specified MAC to be cleared
 *
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE:     1.This API can't delete CPU MAC!
 *           2.The mac entry which is deleted through this function will not be
 *             notified to MLAG, and this function shall only be called by MLAG.
 * -------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_DeleteAddrForMlag(UI32_T vid, UI8_T *mac);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAllAddr
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete all address table entries (both
 *           dynamic and static entries)
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAllAddr(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLifeTime
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete entries by life time
 * INPUT   : AMTR_TYPE_AddressLifeTime_T life_time
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByLifeTime(AMTR_TYPE_AddressLifeTime_T life_time);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrBySource
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete entries by source from OM
 * INPUT   : AMTR_TYPE_AddressSource_T source
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrBySource(AMTR_TYPE_AddressSource_T source);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will clear dynamic & static address table entries by a specific port
 * INPUT   : UI32_T ifindex - interface index
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * Note    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByLPort(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByVidAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by vid and Port.
 * INPUT   : UI32_T ifindex - interface index
 *           UI32_T vid - VLAN ID
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByVidAndLPort(UI32_T ifindex, UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLifeTimeAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time and Port.
 * INPUT   : UI32_T ifindex - interface index
 *           AMTR_MGR_LiftTimeMode_T life_time - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByLifeTimeAndLPort(UI32_T ifindex, AMTR_TYPE_AddressLifeTime_T life_time);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrBySourceAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by specific source and Lport.
 * INPUT   : UI32_T ifindex                 - interface index
 *           AMTR_MGR_SourceMode_T source   - learnt, config, intrenal, self
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrBySourceAndLPort(UI32_T ifindex, AMTR_TYPE_AddressSource_T source);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByVID
 * -------------------------------------------------------------------------
 * FUNCTION: This function will clear dynamic & static address table entries
 *           by vid
 * INPUT   : UI32_T vid     - vlan id
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByVID(UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByVidAndLifeTime
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete entries by vid + life time from OM
 * INPUT   : UI32_T vid     - vlan id
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByVidAndLifeTime(UI32_T vid, AMTR_TYPE_AddressLifeTime_T life_time);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByVidAndSource
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete entries by vid+source from OM
 * INPUT   : UI32_T vid     - vlan id
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByVidAndSource(UI32_T vid, AMTR_TYPE_AddressSource_T source);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port.
 * INPUT   : UI32_T ifindex                     - interface index
 *           UI16_T start_vid - the starting vid
 *           UI16_T end_vid   - the end vid
*            UI32_T vid                         - vlan id
 *           AMTR_MGR_LiftTimeMode_T life_time  - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status                      - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port
 *           synchronously.
 * INPUT   : UI32_T ifindex   - interface index
 *           UI16_T start_vid - the starting vid
 *           UI16_T end_vid   - the end vid
 *           AMTR_MGR_LiftTimeMode_T life_time - delete on timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : 1. This function will not return until the delete operation is done
 *           2. Only support life_time as AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrBySourceAndVidAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by specific source and Lport.
 * INPUT   : UI32_T ifindex                 - interface index
 *           UI32_T vid                     - vlan id
 *           AMTR_MGR_SourceMode_T source   - learnt, config, intrenal, self
 * OUTPUT  : None
 * RETURN  : BOOL_T Status                  - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrBySourceAndVidAndLPort(UI32_T ifindex, UI32_T vid,AMTR_TYPE_AddressSource_T source);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will delete all entries by port + vid, except
 *            mac address which is matched with mac masks of list.
 * INPUT    : UI32_T ifindex  -- specific logical port
 *            UI32_T vid      -- specific vlan id
 *            UI8_T mac_list_p[AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK][SYS_ADPT_MAC_ADDR_LEN]
 *            UI8_T mask_list_p[AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK][SYS_ADPT_MAC_ADDR_LEN]
 *            UI32_T number_of_entry_in_list -- there are "number_of_entry_in_list"
 *                                              in mac_mask_list[][SYS_ADPT_MAC_ADDR_LEN]
 * OUTPUT   : None
 * RETURN   : TRUE                             -- success
 *            FALSE                            -- fail
 * NOTE     : 1. caller should use this function like as below.
 *               function()
 *               {
 *                   UI8_T mac_mask_list[][SYS_ADPT_MAC_ADDR_LEN];
 *                   UI8_T mask_list[][SYS_ADPT_MAC_ADDR_LEN];
 *                      mac_list_p[][6]= {{00,01,01,00,aa,cc},  --> number_of_entry_in_list=1
 *                                        {0a,01,01,00,cc,00},  --> number_of_entry_in_list=2
 *                                        {00,01,01,00,00,ff}}; --> number_of_entry_in_list=3
 *                      mask_list_p[][6]= {{FF,FF,00,00,00,00},
 *                                        {FF,FF,FF,FF,00,00},
 *                                        {FF,00,00,00,00,00}};
 *                   AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr(lport,
 *                                                                       vid,
 *                                                                       mask_mac_list,
 *                                                                       mask_list, 3 );
 *               }
 *            2. Don't support trunk port.
 *               So, it only delete entries from specific unit's hisam table.
 *            3. number_of_entry_in_list can't bigger than AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK
 *            4. 2004.4.23 water_huang create
 *-------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr(UI32_T ifindex,
                                                         UI32_T vid,
                                                         UI8_T mac_list_ar_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                         UI8_T mask_list_ar_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                         UI32_T number_of_entry_in_list);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetExactVMAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get exact address table entry (vid+mac)
 * INPUT   : addr_entry->vid    - VLAN ID     (key)
 *           addr_entry->mac    - MAC address (key)
 * OUTPUT  : addr_entry         - address entry info
 * RETURN  : TRUE  if a valid address exists and retrieved from macList
 *           FALSE if no address can be found
 * NOTE    : 1. using key generated from (vlanID,mac address)
 *           2. search the entry from Hash table(driver layer)
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetExactAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetExactAddrEntryFromChip                                
 *------------------------------------------------------------------------
 * FUNCTION: This function will get exact address table entry (mac+vid)            
 * INPUT   : addr_entry->mac - mac address
 *           addr_entry->vid - vlan id
 * OUTPUT  : addr_entry      - addresss entry info
 * RETURN  : TRUE  if a valid address exists and retrieved from macList   
 *           FALSE if no address can be found                             
 * NOTE    : 1.This API is only support IML_MGR to get exact mac address from chip 
 *             We don't support MIB to get under_create entry since l_port =0 
 *             will return false
 *           2.This API only support MV key or VM key not IVM key                                                       
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetExactAddrEntryFromChip(AMTR_TYPE_AddrEntry_T *addr_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextMVAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get next address table entry (mac+vid)
 * INPUT   : addr_entry->mac    - MAC address (primary key)
 *           addr_entry->vid    - VLAN ID     (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if mac[0]~mac[5] == \0 => get the first entry
 *           2. search the entry from Hisam table(core layer)
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetNextMVAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextVMAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get next address table entry (vid+mac)
 * INPUT   : addr_entry->vid    - VLAN ID        (primary key)
 *           addr_entry->mac    - MAC address    (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if vid == 0 => get the first entry
 *           2. using key generated from (vlanID,mac address)
 *           3. search the entry from Hisam table(core layer)
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetNextVMAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetExactIfIndexAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get exact address table entry (ifindex+vid+mac)
 * INPUT   : addr_entry->l_port - interface index   (key)
 *           addr_entry->vid    - vlan id           (key)
 *           addr_entry->mac    - mac address       (key)
 * OUTPUT  : addr_entry         - addresss entry info
 * RETURN  : TRUE  if a valid address exists and retrieved from macList
 *           FALSE if no address can be found
 * NOTE    : 1. ifindex is a normal port or trunk port
 *           2. Get exact address entry by vid+mac.
 *              Before return, AMTR_MGR has to check ifindex by itself.
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetExactIfIndexAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextIfIndexAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get next address table entry (ifindex+vid+mac)
 * INPUT   : addr_entry->l_port - interface index   (primary key)
 *           addr_entry->vid    - vlan id           (key)
 *           addr_entry->mac    - MAC address       (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. l_port is a physical port or trunk port
 *           2. search the entry from Hisam table(core layer)
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetNextIfIndexAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get next entry on specific lport
 *            until entry.mac matched mac with mask.
 * INPUT    : addr_entry->mac[SYS_ADPT_MAC_ADDR_LEN]  -- mac address
 *            addr_entry->vid                         -- vlan ID
 *            addr_entry->ifindex                     -- specific logical port
 *            UI8_T mask_mac[SYS_ADPT_MAC_ADDR_LEN]   -- ex. {AA,BB,11,10,20,03}
 *            UI8_T mask[SYS_ADPT_MAC_ADDR_LEN]       -- ex. {FF,FF,FF,FF,00,00}
 *            get_mode                                -- AMTR_MGR_GET_ALL_ADDRESS
 *                                                       AMTR_MGR_GET_STATIC_ADDRESS
 *                                                       AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT   : AMTR_MGR_AddrEntry_T *addr_entry
 * RETURN   : TRUE                             -- success
 *            FALSE                            -- fail
 * NOTE     : 1. if addr_entry->mac[]== NULL_MAC and addr_entry->vid == 0,
 *               AMTR will get the first entry on this l_port.
 *            2. addr_entry->l_port can't be zero.
 *            3. If(memcpr(mask_mac & mask, addr_entry->mac & mask, 6)==0),
 *               return TRUE.
 *            4. This function is created for Voice Vlan.
 *            5. This function won't check input(vid, mac,l_port), because
 *               caller may set zero to get first entry.
 *            6. The input "addr_entry->l_port" can't be zero.
 *            7. This function doesn't support trunk port.
 *            8. 2006.4.23 water_huang create
 *            9. This API doesn't filter out entries that have drop attribute
 *-------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(AMTR_TYPE_AddrEntry_T *addr_entry,
                                                    UI8_T mask_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                    UI8_T mask[SYS_ADPT_MAC_ADDR_LEN],
                                                    UI32_T get_mode);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextRunningStaticAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           static address can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL is returned.
 * INPUT   : addr_entry->mac    - MAC address (primary key)
 *           addr_entry->vid    - VLAN ID     (key)
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if mac[0]~mac[5] == \0 => get the first entry
 *           2. the function shall be only invoked by cli
 *           3. search the entry from Hash table
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetNextRunningStaticAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetInterventionEntry
 *------------------------------------------------------------------------------
 * Purpose  : This routine will add a CPU MAC to the chip.  Packets with this
 *            MAC in the specified VLAN will trap to CPU.
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address
 * OUTPUT   : None
 * RETURN   : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTE     : This API doesn't forbid that CPU has only one MAC.  It will only
 *            depend on spec.
 *------------------------------------------------------------------------------*/
AMTR_TYPE_Ret_T  AMTR_MGR_SetInterventionEntry(UI32_T vid, UI8_T *mac);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_DeleteInterventionEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will delete a intervention mac address from
 *            address table
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_DeleteInterventionEntry(UI32_T vid, UI8_T *mac);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_CreateMulticastAddrTblEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will create a multicast address table entry
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - multicast mac address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_CreateMulticastAddrTblEntry(UI32_T vid, UI8_T *mac);


/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_DestroyMulticastAddrTblEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will destroy a multicast address table entry
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - multicast mac address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_DestroyMulticastAddrTblEntry(UI32_T vid, UI8_T* mac);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetMulticastPortMember
 *------------------------------------------------------------------------------
 * Purpose  : This function sets the port member(s) of a given multicast address
 * INPUT    : UI32_T vid
 *            UI8_T *mac                                                                                - multicast MAC address
 *            UI8_T ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]   - the member ports of the MAC
 *            UI8_T tagged[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] - tagged/untagged member
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_SetMulticastPortMember(UI32_T vid,
                                        UI8_T *mac,
                                        UI8_T ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                        UI8_T tagged[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_GetAgingStatus
 *------------------------------------------------------------------------------
 * Purpose  : This function Get the Address Ageing Status
 * INPUT    : None
 * OUTPUT   : UI32_T *status - VAL_amtrMacAddrAgingStatus_enabled
 *                             VAL_amtrMacAddrAgingStatus_disabled
 * RETURN   : BOOL_T         - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_GetAgingStatus(UI32_T *status);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_GetRunningAgingStatus
 *------------------------------------------------------------------------------
 * Purpose  : This function Get the Address Ageing Status
 * INPUT    : None
 * OUTPUT   : UI32_T *status -  VAL_amtrMacAddrAgingStatus_enabled
 *                              VAL_amtrMacAddrAgingStatus_disabled
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T -
 *                              1. SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *                              2. SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *                              3. SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :
 *-----------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  AMTR_MGR_GetRunningAgingStatus(UI32_T *status);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetAgingStatus
 *------------------------------------------------------------------------------
 * Purpose  : This function Get the Address Ageing Status
 * INPUT    : None
 * OUTPUT   : UI32_T status  - VAL_amtrMacAddrAgingStatus_enabled
 *                             VAL_amtrMacAddrAgingStatus_disabled
 * RETURN   : BOOL_T         - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_SetAgingStatus(UI32_T status);

/*---------------------------------------------------------------------- */
/* The dot1dTp group (dot1dBridge 4 ) -- dot1dTp 2 & dot1dTp 3 */
/*
 *         dot1dTpAgingTime
 *             SYNTAX   INTEGER (10..1000000)
 *             ::= { dot1dTp 2 }
 *----------------------------------------------------------------------
 *         dot1dTpFdbTable OBJECT-TYPE
 *             SYNTAX  SEQUENCE OF Dot1dTpFdbEntry
 *             ::= { dot1dTp 3 }
 *         Dot1dTpFdbEntry
 *             INDEX   { dot1dTpFdbAddress }
 *             ::= { dot1dTpFdbTable 1 }
 *         Dot1dTpFdbEntry ::=
 *             SEQUENCE {
 *                 dot1dTpFdbAddress    MacAddress,
 *                 dot1dTpFdbPort       INTEGER,
 *                 dot1dTpFdbStatus     INTEGER
 *             }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetDot1dTpAgingTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified aging time
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   aging_time                  -- aging time
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTp 2
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetDot1dTpAgingTime(UI32_T *aging_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1dTpAgingTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified aging time
 *              can be successfully set. Otherwise, false is returned.
 * INPUT    :   aging_time                  -- aging time
 * OUTPUT   :   NOne
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. RFC1493/dot1dTp 2
 *              2. aging time is in [10..1000000] seconds
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetDot1dTpAgingTime(UI32_T aging_time);

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetMaxHashLookupLenFromConfig
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified max hash lookup length
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   lookup_len_p   -- max hash lookup length
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetHashLookupDepthFromConfig(UI32_T *lookup_len_p);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetHashLookupDepthFromChip
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified hash lookup depth
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   lookup_depth_p   -- hash lookup depth
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetHashLookupDepthFromChip(UI32_T *lookup_depth_p);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetMaxHashLookupLen
 *-----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified hash lookup depth
 *              can be successfully set. Otherwise, false is returned.
 * INPUT    :   lookup_depth     -- hash lookup depth
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetHashLookupDepth(UI32_T lookup_depth);
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetDot1dTpFdbEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified forwarding database
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   tp_fdb_entry -- Forwarding Database for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpFdbTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetDot1dTpFdbEntry(AMTR_MGR_Dot1dTpFdbEntry_T *tp_fdb_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetNextDot1dTpFdbEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified forwarding database
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   tp_fdb_entry -- Forwarding Database for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpFdbTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetNextDot1dTpFdbEntry(AMTR_MGR_Dot1dTpFdbEntry_T *tp_fdb_entry);

/*---------------------------------------------------------------------- */
/* The Static (Destination-Address Filtering) Database (dot1dBridge 5) */
/*
 *         Dot1dStaticEntry
 *             INDEX   { dot1dStaticAddress, dot1dStaticReceivePort }
 *             ::= { dot1dStaticTable 1 }
 *
 *         Dot1dStaticEntry ::=
 *             SEQUENCE {
 *                 dot1dStaticAddress           MacAddress,
 *                 dot1dStaticReceivePort       INTEGER,
 *                 dot1dStaticAllowedToGoTo     OCTET STRING,
 *                 dot1dStaticStatus            INTEGER
 *             }
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetDot1dStaticEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   static_entry->dot1d_static_address
 * OUTPUT   :   static_entry                  -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dStaticTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetDot1dStaticEntry(AMTR_MGR_Dot1dStaticEntry_T *static_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetNextDot1dStaticEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   static_entry->dot1d_static_address
 * OUTPUT   :   static_entry                  -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dStaticTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetNextDot1dStaticEntry(AMTR_MGR_Dot1dStaticEntry_T *static_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1dStaticAddress
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the static address information.
 * INPUT    :   UI8_T old_mac -- the original mac address (key)
 *              UI8_T new_mac -- the new mac to replace original mac
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 1
 * ------------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_SetDot1dStaticAddress(UI8_T *old_mac, UI8_T *new_mac);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1dStaticReceivePort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the static receive port information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI32_T receive_port     -- the receive port number
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 2
 * ------------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_SetDot1dStaticReceivePort(UI8_T *mac, UI32_T receive_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1dStaticAllowedToGoTo
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the static allowed to go to information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI8_T allow_to_go_to    -- the set of ports
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 3
 * ------------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_SetDot1dStaticAllowedToGoTo(UI8_T *mac, UI8_T *allowed_to_go_to);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1dStaticStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set/Create the static status information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI32_T status           -- VAL_dot1dStaticStatus_other
 *                                         VAL_dot1dStaticStatus_invalid
 *                                         VAL_dot1dStaticStatus_permanent
 *                                         VAL_dot1dStaticStatus_deleteOnReset
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 4
 * ------------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_SetDot1dStaticStatus(UI8_T *mac, UI32_T status);

/*---------------------------------------------------------------------- */
/* the current Filtering Database Table (the dot1qTp group 1) */
/*
 *       INDEX   { dot1qFdbId }
 *       Dot1qFdbEntry ::=
 *       SEQUENCE {
 *           dot1qFdbId             Unsigned32,
 *           dot1qFdbDynamicCount   Counter32
 *       }
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetDot1qFdbEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the filtering database entry info
 * INPUT   : dot1q_fdb_entry->dot1q_fdb_id  - The identity of this Filtering Database
 * OUTPUT  : dot1q_fdb_entry                - The  Filtering Database entry
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 1
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetDot1qFdbEntry(AMTR_MGR_Dot1qFdbEntry_T *dot1q_fdb_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextDot1qFdbEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next filtering database entry info
 * INPUT   : dot1q_fdb_entry->dot1q_fdb_id  - The identity of this Filtering Database
 * OUTPUT  : dot1q_fdb_entry                - The  Filtering Database entry
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 1
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetNextDot1qFdbEntry(AMTR_MGR_Dot1qFdbEntry_T *dot1q_fdb_entry);

/*---------------------------------------------------------------------- */
/* (the dot1qTp group 2) */
/*
 *      INDEX   { dot1qFdbId, dot1qTpFdbAddress }
 *      Dot1qTpFdbEntry ::=
 *          SEQUENCE {
 *              dot1qTpFdbAddress  MacAddress,
 *              dot1qTpFdbPort     INTEGER,
 *              dot1qTpFdbStatus   INTEGER
 *          }
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetDot1qTpFdbEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the dot1qTpFdbEntry info
 * INPUT   : dot1q_tp_fdb_entry->dot1q_fdb_id  - vlan id
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_address - mac address
 * OUTPUT  : dot1q_tp_fdb_entry                - The dot1qTpFdbEntry info
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 2
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_status == addr_entry->source
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetDot1qTpFdbEntry(UI32_T dot1q_fdb_id, AMTR_MGR_Dot1qTpFdbEntry_T *dot1q_tp_fdb_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextDot1qTpFdbEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next dot1qTpFdbEntry info
 * INPUT   : dot1q_tp_fdb_entry->dot1q_fdb_id  - vlan id
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_address - mac address
 * OUTPUT  : dot1q_tp_fdb_entry             - The dot1qTpFdbEntry info
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 2
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_status == addr_entry->source
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetNextDot1qTpFdbEntry(UI32_T *dot1q_fdb_id, AMTR_MGR_Dot1qTpFdbEntry_T *dot1q_tp_fdb_entry);

/*---------------------------------------------------------------------- */
/* The Static (Destination-Address Filtering) Database (dot1qStatic 1) */
/*
 *             INDEX   { dot1qFdbId, dot1qStaticUnicastAddress, dot1qStaticUnicastReceivePort }
 *             ::= { dot1qStaticUnicastTable 1 }
 *
 *         Dot1qStaticUnicastEntry ::=
 *             SEQUENCE {
 *                 dot1qStaticUnicastAddress           MacAddress,
 *                 dot1qStaticUnicastReceivePort       INTEGER,
 *                 dot1qStaticUnicastAllowedToGoTo     OCTET STRING,
 *                 dot1qStaticUnicastStatus            INTEGER
 *             }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetDot1qStaticUnicastEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   dot1q_fdb_id                -- vlan id
 *              static_unitcast_entry->dot1q_static_unicast_address
 * OUTPUT   :   static_unitcast_entry       -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674q/dot1qStaticUnicastTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetDot1qStaticUnicastEntry(UI32_T dot1q_fdb_id,
                                           AMTR_MGR_Dot1qStaticUnicastEntry_T *static_unitcast_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetNextDot1qStaticUnicastEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   dot1q_fdb_id                -- vlan id
 *              static_unitcast_entry->dot1q_static_unicast_address
 * OUTPUT   :   static_unitcast_entry       -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674q/dot1qStaticUnicastTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetNextDot1qStaticUnicastEntry(UI32_T *dot1q_fdb_id,
                                               AMTR_MGR_Dot1qStaticUnicastEntry_T *static_unitcast_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1qStaticUnicastAllowedToGoTo
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the static allowed to go to information.
 * INPUT    :   UI32_T vid              -- vlan id
 *              UI8_T mac               -- the mac address (key)
 *              UI8_T allow_to_go_to    -- the set of ports
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC2674q/dot1qStaticUnicastEntry 3
 * ------------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_SetDot1qStaticUnicastAllowedToGoTo(UI32_T vid, UI8_T *mac, UI8_T *allowed_to_go_to);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1qStaticUnicastStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set/Create the static status information.
 * INPUT    :   UI32_T vid              -- vlan id
 *              UI8_T mac               -- the mac address (key)
 *              UI32_T status           -- VAL_dot1qStaticUnicastStatus_other
 *                                         VAL_dot1qStaticUnicastStatus_invalid
 *                                         VAL_dot1qStaticUnicastStatus_permanent
 *                                         VAL_dot1qStaticUnicastStatus_deleteOnReset
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC2674q/dot1qStaticUnicastEntry 4
 * ------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_SetDot1qStaticUnicastStatus(UI32_T vid, UI8_T *mac, UI32_T status);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetLearningMode
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the learning mode for whole system
 * INPUT    : UI32_T learning_mode    - VAL_dot1qConstraintType_independent (IVL)
 *                                      VAL_dot1qConstraintType_shared (SVL)
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : RFC2674q
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_SetLearningMode (UI32_T learning_mode);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetLearningMode
 *------------------------------------------------------------------------------
 * Purpose  : This function check whether port security is enabled on a specific port or not.
 * INPUT    : UI32_T ifindex    - interface index
 * OUTPUT   : None
 * RETURN   : True : enabled, False : disabled
 * NOTE     :
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_IsPortSecurityEnabled(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_IsPortSecurityEnableByAutoLearn
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return Ture if the port security is enabled by
 *           auto learn
 * INPUT   : ifindex        -- which port to set
 * OUTPUT  : none
 * RETURN  : TRUE/FALSE
 * NOTE    : If the status of port security is enabled by Auto Learn,
 *           1. the status of port security should not be got from GetRunning.
 *           2. the security mac should not be got form GetNextRunning.
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_IsPortSecurityEnableByAutoLearn(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SyncHashToHisam
 * -------------------------------------------------------------------------
 * FUNCTION: this function will sync entry(add or delete) from Hash table to Hisam talbe
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : count
 * NOTE    : refinement ;water_huang add (93.8.16)
 *          1.Amtr_task will call this API in periodic, then sync entry or action
 *            frome Hash table to Hisam table.
 *--------------------------------------------------------------------------*/
int AMTR_MGR_SyncHashToHisam(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AMTR_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for AMTR MGR.
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
BOOL_T AMTR_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

void AMTR_MGR_PortLinkDown_CallBack      (UI32_T l_port);
void AMTR_MGR_PortAddIntoTrunk_CallBack(UI32_T l_port_trunk, UI32_T l_port_member,BOOL_T is_firstmem);
void AMTR_MGR_DestroyTrunk_CallBack      (UI32_T l_port_trunk, UI32_T l_port_member);
void AMTR_MGR_DestroyVlan_CallBack       (UI32_T vid_ifidx, UI32_T vlan_status);
void AMTR_MGR_PortAdminDisable_CallBack  (UI32_T l_port);
void AMTR_MGR_PortLearningStatusChanged_Callback(UI32_T l_port, BOOL_T learning);
void AMTR_MGR_AnnounceNewAddress_CallBack(UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[]);
void AMTR_MGR_AgingOut_CallBack          (UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[]);
#if 0 /* obsoleted API */
UI32_T AMTR_MGR_SecurityCheck_Callback   (UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type);
#endif
void AMTR_MGR_VlanMemberDelete_CallBack  (UI32_T vid_ifidx, UI32_T l_port, UI32_T vlan_status);

BOOL_T  AMTR_MGR_SetMACLearningStatusOnPort(UI32_T unit,UI32_T port,UI32_T status);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetCpuMac
 *------------------------------------------------------------------------------
 * Purpose  : This routine will add a CPU MAC to the chip.  Packets with this
 *            MAC in the specified VLAN will trap to CPU.
 * INPUT    : vid        -- vlan ID
 *            mac        -- MAC address
 *            is_routing -- whether the routing feature is supported and enabled
 * OUTPUT   : None
 * RETURN   : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTE     : None
 *------------------------------------------------------------------------------*/
AMTR_TYPE_Ret_T  AMTR_MGR_SetCpuMac(UI32_T vid, UI8_T *mac, BOOL_T is_routing);

#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
BOOL_T  AMTR_MGR_SetRouterAdditionalCtrlReg(UI32_T value);
#endif

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_MacNotifyProcessQueue
 *------------------------------------------------------------------------------
 * Purpose  : To process the mac-notification entries in the queue
 * INPUT    : cur_time_stamp - current time stamp in ticks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void AMTR_MGR_MacNotifyProcessQueue(UI32_T  cur_time_stamp);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetMacNotifyGlobalStatus
 *------------------------------------------------------------------------
 * FUNCTION: To get the mac-notification-trap global status
 * INPUT   : None
 * OUTPUT  : *is_enabled_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetMacNotifyGlobalStatus(BOOL_T *is_enabled_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetMacNotifyGlobalStatus
 *------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap global status
 * INPUT   : None
 * OUTPUT  : is_enabled - global status to set
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetMacNotifyGlobalStatus(BOOL_T is_enabled);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetMacNotifyInterval
 *------------------------------------------------------------------------
 * FUNCTION: To get the mac-notification-trap interval
 * INPUT   : None
 * OUTPUT  : interval_p - pointer to interval to get (in seconds)
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetMacNotifyInterval(UI32_T  *interval_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetMacNotifyInterval
 *------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap interval
 * INPUT   : None
 * OUTPUT  : interval - interval to set (in seconds)
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetMacNotifyInterval(UI32_T  interval);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetMacNotifyPortStatus
 *------------------------------------------------------------------------
 * FUNCTION: To get the mac-notification-trap port status
 * INPUT   : ifidx        - lport ifindex
 * OUTPUT  : *is_enabled_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_GetMacNotifyPortStatus(UI32_T  ifidx, BOOL_T *is_enabled_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetMacNotifyPortStatus
 *------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap port status
 * INPUT   : ifidx      - lport ifindex
 * OUTPUT  : is_enabled - port status to set
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetRunningMacNotifyInterval
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default interval can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : None
 * OUTPUT  : interval_p   - the non-default interval
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default interval.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetRunningMacNotifyInterval( UI32_T  *interval_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetRunningMacNotifyGlobalStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default global status can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : None
 * OUTPUT  : is_enabled_p   - the non-default global status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default global status.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetRunningMacNotifyGlobalStatus( BOOL_T  *is_enabled_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetRunningMacNotifyPortStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default port status can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : ifidx          - lport ifindex
 * OUTPUT  : is_enabled_p   - the non-default port status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default port status.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetRunningMacNotifyPortStatus(
    UI32_T  ifidx,
    BOOL_T  *is_enabled_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_AddFstTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * FUNCTION: Service the callback from SWCTRL when the port joins the trunk
 *           as the 1st member
 * INPUT   : trunk_ifindex  - specify which trunk to join.
 *           member_ifindex - specify which member port being add to trunk.
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : member_ifindex is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void AMTR_MGR_MacNotifyAddFstTrkMbr_CallBack(UI32_T  trunk_ifindex, UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_DelTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * FUNCTION: Service the callback from SWCTRL when the trunk member is
 *           removed from the trunk
 * INPUT   : trunk_ifindex  - specify which trunk to remove from
 *           member_ifindex - specify which member port being removed from
 *                            trunk
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void AMTR_MGR_MacNotifyDelTrkMbr_CallBack(UI32_T  trunk_ifindex, UI32_T  member_ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_DelLstTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * FUNCTION: Service the callback from SWCTRL when the last trunk member
 *           is removed from the trunk
 * INPUT   : trunk_ifindex   -- specify which trunk to join to
 *          member_ifindex  -- specify which member port being add to trunk
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void AMTR_MGR_MacNotifyDelLstTrkMbr_CallBack(UI32_T  trunk_ifindex, UI32_T  member_ifindex);
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_ClearCollisionVlanMacTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Remove all entries in the collision mac table.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_ClearCollisionVlanMacTable(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_GetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get vlan learning status of specified vlan
 * INPUT    : vid
 * OUTPUT   : *learning_p
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetVlanLearningStatus(UI32_T vid, BOOL_T *learning_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_GetRunningVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get running vlan learning status of specified vlan
 * INPUT    : vid
 * OUTPUT   : *learning_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetRunningVlanLearningStatus(UI32_T vid, BOOL_T *learning_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_SetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Enable/disable vlan learning of specified vlan
 * INPUT    : vid
 *            learning
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetVlanLearningStatus(UI32_T vid, BOOL_T learning);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetMlagMacNotifyPortStatus
 *------------------------------------------------------------------------
 * FUNCTION: To set the MLAG mac notify port status
 * INPUT   : ifidx      - lport ifindex
 * OUTPUT  : is_enabled - port status to set
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetMlagMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled);

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_MlagMacNotifyProcessQueue
 *------------------------------------------------------------------------------
 * Purpose  : To process the MLAG mac notify entries in the queue
 * INPUT    : cur_time_stamp - current time stamp in ticks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void AMTR_MGR_MlagMacNotifyProcessQueue(UI32_T  cur_time_stamp);

#endif /* AMTR_MGR_H */



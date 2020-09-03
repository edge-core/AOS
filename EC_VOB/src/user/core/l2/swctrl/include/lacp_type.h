/*-----------------------------------------------------------------------------
 * Module Name: LACP.h
 *-----------------------------------------------------------------------------
 * PURPOSE: A header file for the declaration of LACP.c
 *-----------------------------------------------------------------------------
 * NOTES:
 * 1. This header file provides the declarations for LACP initialization.
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/04/2001 - Lewis Kang, Created. Refer to ckhsu's lacp_dat.h
 *    12/10/2001 - ckhsu     , Merged.  Refer to v0.01a of lacp_type.h of MC2.
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-----------------------------------------------------------------------------
 */
#ifndef _LACP_TYPE_H
#define _LACP_TYPE_H


#include "sys_type.h"
//#include "leaf_1213.h"
//#include "leaf_es3626a.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_mm.h"
#include "l_stdlib.h"


/* LACP Default values */
#define LACP_SYSTEM_DEFAULT_PRIORITY    0x8000
#define LACP_PORT_DEFAULT_PRIORITY      0x8000
#define LACP_ADMIN_ON                   VAL_lacpPortStatus_enabled        /* 1 : defined in Leaf_es3626a.h */
#define LACP_ADMIN_OFF                  VAL_lacpPortStatus_disabled       /* 2 : defined in Leaf_es3626a.h */
#define LACP_DEFAULT_SYSTEM_ADMIN       LACP_ADMIN_ON
#define LACP_DEFAULT_PORT_ADMIN         SYS_DFLT_PORT_LACP_PORT_STATUS               /* 1 */

#define LACP_dot3adAggPortActorOperState_LACP_Activity      0x01
#define LACP_dot3adAggPortActorOperState_LACP_Timeout       0x02
#define LACP_dot3adAggPortActorOperState_Aggregation        0x04
#define LACP_dot3adAggPortActorOperState_Synchronization    0x08
#define LACP_dot3adAggPortActorOperState_Collecting         0x10
#define LACP_dot3adAggPortActorOperState_Distributing       0x20
#define LACP_dot3adAggPortActorOperState_Defaulted          0x40
#define LACP_dot3adAggPortActorOperState_Expired            0x80

#define LACP_DEFAULT_PORT_ACTOR_ADMIN_STATE  LACP_dot3adAggPortActorOperState_LACP_Activity | LACP_dot3adAggPortActorOperState_Aggregation | LACP_dot3adAggPortActorOperState_Defaulted
#define LACP_DEFAULT_PORT_PARTNER_ADMIN_STATE LACP_dot3adAggPortActorOperState_Synchronization | LACP_dot3adAggPortActorOperState_Collecting | LACP_dot3adAggPortActorOperState_Distributing | LACP_dot3adAggPortActorOperState_Defaulted

enum
{
    LACP_TYPE_TRACE_ID_LACP_TASK_SERVICEREQUEST = 0,
    LACP_TYPE_TRACE_ID_LACP_TX_MACHINE,
    LACP_TYPE_TRACE_ID_LACP_MARKER_MACHINE
};

enum LACP_EVENT_MASK_E
{
    LACP_EVENT_NONE       =   0x0000L,
    LACP_EVENT_LACPDURCVD =   0x0001L,
    LACP_EVENT_TIMER      =   0x0002L,
    LACP_EVENT_CALLBACK   =   0x0004L,
    LACP_TASK_EVENT_ENTER_TRANSITION = 0x0008L,
    LACP_EVENT_ALL        =   0xFFFFL
};

enum LACP_MSG_TYPE_E
{
    LACP_MSG_LACPDU        =   1,
    LACP_MSG_REQUEST,
    LACP_MSG_SERVICE,
    LACP_MSG_MISC
};

/* The following is constant definition in 802.3ad */
typedef enum {
    LACP_PASSIVE_LACP = 0,
    LACP_ACTIVE_LACP  = 1
} LACP_ACTIVITY_E;

typedef enum {
    LACP_LONG_TIMEOUT  = 0,
    LACP_SHORT_TIMEOUT = 1
} LACP_TIMEOUT_E;

typedef enum {
    LACP_INDIVIDUAL_LINK    = 0,
    LACP_AGGREGATABLE_LINK  = 1
} LACP_AGGREGATION_E;

typedef enum {
    LACP_LACP   = 1,
    LACP_MARKER = 2
} LACP_PROTOCOL_SUBTYPE_E;

typedef enum {
    LACP_LAC_PDU = 1,
    LACP_MARKER_INFO_PDU = 2,
    LACP_MARKER_RESPONSE_PDU = 3
} LACP_PDU_TYPE_E;

typedef enum
{
    UNSELECTED = 0,
    STANDBY,
    SELECTED
}SELECTED_VALUE_E;

typedef enum
{
    /* This value comply to MIB values defined in 802.3ad */
    Rxm_BEGIN   = 0,
    Rxm_CURRENT,
    Rxm_EXPIRED,
    Rxm_DEFAULTED,
    Rxm_INITIALIZE,
    Rxm_LACP_DISABLED,
    Rxm_PORT_DISABLED
} RX_MACHINE_STATE_E;

typedef enum
{
    /* This value comply to MIB values defined in 802.3ad */
    Muxm_BEGIN   = 0,
    Muxm_DETACHED,
    Muxm_WAITING,
    Muxm_ATTACHED,
    Muxm_COLLECTING,
    Muxm_DISTRIBUTING,
    Muxm_COLLECTING_DISTRIBUTING
} MUX_MACHINE_STATE_E;

typedef enum
{
    NO_CHURN      = 1,
    CHURN         = 2,
    CHURN_MONITOR = 3
} CHURN_STATE_E;

typedef enum
{
    /* Do not change the sequence of the following event */
    /* It is related to the event number                 */
    LACP_NULL_EVENT = 0,
    LACP_SYS_INIT,
    LACP_TIMER_EV,
    LACP_NEW_INFO_EV,
    LACP_RX_PDU_EV,
    LACP_PORT_ENABLED_EV,
    LACP_PORT_DISABLED_EV,
    /* LACP RX State machine events */
    LACP_RX_PORT_UP_EV,
    LACP_RX_PORT_DOWN_EV,
    LACP_RX_INIT_EV,
    LACP_RX_ENABLED_EV,
    LACP_RX_DISABLED_EV,
    LACP_RX_CURRENT_WHILE_TIMER_EV,
    /* LACP Actor Churn Detection State machine events */
    LACP_ACD_INIT_EV,
    LACP_ACD_ACTOR_CHURN_TIMER_EXPIRED_EV,
    LACP_ACD_SYNC_FALSE_EV,
    LACP_ACD_SYNC_TRUE_EV,
    LACP_ACD_PORT_DISABLED_EV,
    /* LACP Partner Churn Detection State machine events */
    LACP_PCD_INIT_EV,
    LACP_PCD_PARTNER_CHURN_TIMER_EXPIRED_EV,
    LACP_PCD_SYNC_FALSE_EV,
    LACP_PCD_SYNC_TRUE_EV,
    LACP_PCD_PORT_DISABLED_EV,
    /* LACP MARKER State Machine Events */
    LACP_MARKER_PDU_EV,
    LACP_MARKER_RESPONSE_PDU_EV,
    /* LACP SELECTION State Machine Events */
    LACP_SELECT_FALSE_EV,
    LACP_SELECT_PORT_DOWN_EV,
    LACP_SELECT_REINITIALIZE_EV,
    LACP_HOT_SWAP_EV,
    /* LACP Mux Machine state */
    LACP_MUX_DETACHED_EV,
    LACP_MUX_WAITING_EV,
    LACP_MUX_ATTACHED_EV,
    LACP_MUX_COLLECTING_DISTRIBUTING_EV,
    LACP_MUX_STATE_CHANGE_EV,  /* This means MUX machine has state change. */
    LACP_MUX_REEVALUATE_EV,
    /* LACP Transmit Machine */
    LACP_TRANSMIT_EV,
    LACP_FORCE_TRANSMIT_EV
}LACP_EVENT_E;

#ifndef MAX_PHYSICAL_PORT /* XXX steven.jiang for warnings */
#define MAX_PHYSICAL_PORTS                ((SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)*(SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
#define MAX_LPORTS                        SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM /* ckhsu sees Lport as Trunk port only for MC2 */
#endif /* MAX_PHYSICAL_PORTS */

#define MAX_PORTS                         SYS_ADPT_TOTAL_NBR_OF_LPORT
#define LACP_LINEAR_ID( unit, port)       ((((unit)-1) * (SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)) + (port))
#if 0
#define LACP_FIND_PORT( unit, port)       &(lacp_port[(LACP_LINEAR_ID(unit,port))-1])
#define LACP_FIND_AGGREGATOR( unit, port) &(lacp_system.aggregator[(LACP_LINEAR_ID(unit,port))-1])
#define LACP_FIND_TRUE_AGGREGATOR( id)    &(lacp_system.aggregator[(MAX_PHYSICAL_PORTS)+((id)-1)])
#endif

#define  MAX_PORTS_IN_PER_AGGREGATOR      SYS_ADPT_TOTAL_NBR_OF_LPORT

#define LACP_DEFAULT_SYSTEM_TIMEOUT       LACP_LONG_TIMEOUT

/* #define  LACP_SEMAPHORE_TIMEOUT        0             */
/* #define  LACP_SM                       "LACP_SEMA"   */

#define  LACP_ENTER_CRITICAL_SECTION(x)  SYSFUN_GetSem(x, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define  LACP_LEAVE_CRITICAL_SECTION(x)  SYSFUN_SetSem((x))

/* Several Macros for Little Endian Conversion */
/*-------------------------------------------------------------------------
 * MACRO NAME   - LACP_TYPE_LAC_INFO_NtoHCpy
 * ------------------------------------------------------------------------
 * PURPOSE      : Copy LAC_INFO_T format variables from network to host
 * INPUT        : LAC_INFO_T      __dest_info;
 *              : LAC_INFO_T      __src_info;
 * OUTPUT       : None
 * NOTES        :
 * ------------------------------------------------------------------------
 */
#define  LACP_TYPE_LAC_INFO_NtoHCpy(__dest_info, __src_info)                         \
{                                                                                    \
    (__dest_info).system_priority = L_STDLIB_Ntoh16((__src_info).system_priority);   \
    (__dest_info).key             = L_STDLIB_Ntoh16((__src_info).key);               \
    (__dest_info).port_priority   = L_STDLIB_Ntoh16((__src_info).port_priority);     \
    (__dest_info).port_no         = L_STDLIB_Ntoh16((__src_info).port_no);           \
    (__dest_info).state           = (__src_info).state;                              \
    memcpy( &((__dest_info).system_id),                                              \
            &((__src_info).system_id),                                               \
            sizeof( MAC_ADDRESS )                                                    \
          );                                                                         \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - LACP_TYPE_LAC_INFO_HtoNCpy
 * ------------------------------------------------------------------------
 * PURPOSE      : Copy LAC_INFO_T format variables from network host to network.
 * INPUT        : LAC_INFO_T      __dest_info;
 *              : LAC_INFO_T      __src_info;
 * OUTPUT       : None
 * NOTES        :
 * ------------------------------------------------------------------------
 */
#define  LACP_TYPE_LAC_INFO_HtoNCpy(__dest_info, __src_info) \
{                                                                                    \
    (__dest_info).system_priority = L_STDLIB_Hton16((__src_info).system_priority);   \
    (__dest_info).key             = L_STDLIB_Hton16((__src_info).key);               \
    (__dest_info).port_priority   = L_STDLIB_Hton16((__src_info).port_priority);     \
    (__dest_info).port_no         = L_STDLIB_Hton16((__src_info).port_no);           \
    (__dest_info).state           = (__src_info).state;                              \
    memcpy( &((__dest_info).system_id),                                              \
            &((__src_info).system_id),                                               \
            sizeof( MAC_ADDRESS )                                                    \
          );                                                                         \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - LACP_TYPE_CMP_LAC_INFO
 * ------------------------------------------------------------------------
 * PURPOSE      : Comapre 2 LAC_INFO_T format variables.
 * INPUT        : LAC_INFO_T      __lac_1;
 *              : LAC_INFO_T      __lac_2;
 * OUTPUT       : __result <  0:  __lac_1 < __lac_2
 *              : __result >  0:  __lac_1 > __lac_2
 *              : __result =  0:  __lac_1 = __lac_2
 * NOTES        :
 * ------------------------------------------------------------------------
 */
#define  LACP_TYPE_CMP_LAC_INFO(__result, __lac_1, __lac_2)                                                     \
{                                                                                                               \
    /* 1. Compare system_priority */                                                                            \
    (__result) = ((__lac_1).system_priority - (__lac_2).system_priority);                                       \
    if ((__result) == 0)                                                                                        \
    {                                                                                                           \
        /* 2. Compare system_id */                                                                              \
        (__result) = memcmp(&((__lac_1).system_id), &((__lac_2).system_id), sizeof(MAC_ADDRESS));               \
        if ((__result) == 0)                                                                                    \
        {                                                                                                       \
            /* 3. Compare lacp_key */                                                                           \
            (__result) = ((__lac_1).key - (__lac_2).key);                                                       \
            if ((__result) == 0)                                                                                \
            {                                                                                                   \
                /* 4. Compare port_priority */                                                                  \
                (__result) = ((__lac_1).port_priority - (__lac_2).port_priority);                               \
                if ((__result) == 0)                                                                            \
                {                                                                                               \
                    /* 5. Compare port_no */                                                                    \
                    (__result) = ((__lac_1).port_no - (__lac_2).port_no);                                       \
                    if ((__result) == 0)                                                                        \
                    {                                                                                           \
                        /* 6. Compare port_state.Expired */                                                     \
                        (__result) = (  LACP_TYPE_GET_PORT_STATE_BIT((((__lac_1).state).port_state), LACP_dot3adAggPortActorOperState_Expired)                                      \
                                      - LACP_TYPE_GET_PORT_STATE_BIT((((__lac_2).state).port_state), LACP_dot3adAggPortActorOperState_Expired));                                    \
                        if ((__result) == 0)                                                                    \
                        {                                                                                       \
                            /* 7. Compare port_state.Defaulted */                                               \
                            (__result) = (  LACP_TYPE_GET_PORT_STATE_BIT((((__lac_1).state).port_state), LACP_dot3adAggPortActorOperState_Defaulted)                                \
                                          - LACP_TYPE_GET_PORT_STATE_BIT((((__lac_2).state).port_state), LACP_dot3adAggPortActorOperState_Defaulted));                              \
                            if ((__result) == 0)                                                                \
                            {                                                                                   \
                                /* 8. Compare port_state.Distributing */                                        \
                                (__result) = (  LACP_TYPE_GET_PORT_STATE_BIT((((__lac_1).state).port_state), LACP_dot3adAggPortActorOperState_Distributing)                         \
                                              - LACP_TYPE_GET_PORT_STATE_BIT((((__lac_2).state).port_state), LACP_dot3adAggPortActorOperState_Distributing));                       \
                                if ((__result) == 0)                                                            \
                                {                                                                               \
                                    /* 9. Compare port_state.Collecting */                                      \
                                    (__result) = (  LACP_TYPE_GET_PORT_STATE_BIT((((__lac_1).state).port_state), LACP_dot3adAggPortActorOperState_Collecting)                       \
                                                  - LACP_TYPE_GET_PORT_STATE_BIT((((__lac_2).state).port_state), LACP_dot3adAggPortActorOperState_Collecting));                     \
                                    if ((__result) == 0)                                                        \
                                    {                                                                           \
                                        /* 10. Compare port_state.Synchronization */                            \
                                        (__result) = (  LACP_TYPE_GET_PORT_STATE_BIT((((__lac_1).state).port_state), LACP_dot3adAggPortActorOperState_Synchronization)              \
                                                      - LACP_TYPE_GET_PORT_STATE_BIT((((__lac_2).state).port_state), LACP_dot3adAggPortActorOperState_Synchronization));            \
                                        if ((__result) == 0)                                                    \
                                        {                                                                       \
                                            /* 11. Compare port_state.Aggregation */                            \
                                            (__result) = (  LACP_TYPE_GET_PORT_STATE_BIT((((__lac_1).state).port_state), LACP_dot3adAggPortActorOperState_Aggregation)              \
                                                          - LACP_TYPE_GET_PORT_STATE_BIT((((__lac_2).state).port_state), LACP_dot3adAggPortActorOperState_Aggregation));            \
                                            if ((__result) == 0)                                                \
                                            {                                                                   \
                                                /* 12. Compare port_state.LACP_Timeout */                       \
                                                (__result) = (  LACP_TYPE_GET_PORT_STATE_BIT((((__lac_1).state).port_state), LACP_dot3adAggPortActorOperState_LACP_Timeout)         \
                                                              - LACP_TYPE_GET_PORT_STATE_BIT((((__lac_2).state).port_state), LACP_dot3adAggPortActorOperState_LACP_Timeout));       \
                                                if ((__result) == 0)                                            \
                                                {                                                               \
                                                    /* 13. Compare port_state.LACP_Activity */                  \
                                                    (__result) = (  LACP_TYPE_GET_PORT_STATE_BIT((((__lac_1).state).port_state), LACP_dot3adAggPortActorOperState_LACP_Activity)    \
                                                                  - LACP_TYPE_GET_PORT_STATE_BIT((((__lac_2).state).port_state), LACP_dot3adAggPortActorOperState_LACP_Activity));  \
                                                } /* End of 12th if */                                                                                                              \
                                            } /* End of 11th if */                                                                                                                  \
                                       } /* End of 10th if */                                                                                                                       \
                                    } /* End of 9th if */                                                                                                                           \
                                } /* End of 8th if */                                                                                                                               \
                            } /* End of 7th if */                                                                                                                                   \
                        } /* End of 6th if */                                                                                                                                       \
                    } /* End of 5th if */                                                                                                                                           \
                } /* End of 4th if */                                                                                                                                               \
            } /* End of 3rd if */                                                                                                                                                   \
        } /* End of 2nd if */                                                                                                                                                       \
    } /* End of 1st if */                                                                                                                                                           \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - LACP_TYPE_CMP_LAC_SYSTEM
 * ------------------------------------------------------------------------
 * PURPOSE      : Comapre 2 LAC_INFO_T format variables' first 2 members.
 * INPUT        : LAC_INFO_T      __lac_1;
 *              : LAC_INFO_T      __lac_2;
 * OUTPUT       : __result <  0:  __lac_1 < __lac_2
 *              : __result >  0:  __lac_1 > __lac_2
 *              : __result =  0:  __lac_1 = __lac_2
 * NOTES        :
 * ------------------------------------------------------------------------
 */
#define  LACP_TYPE_CMP_LAC_SYSTEM(__result, __lac_1, __lac_2)                                       \
{                                                                                                   \
    /* 1. Compare system_priority */                                                                \
    (__result) = ((__lac_1).system_priority - (__lac_2).system_priority);                           \
    if ((__result) == 0)                                                                            \
    {                                                                                               \
        /* 2. Compare system_id */                                                                  \
        (__result) = memcmp(&((__lac_1).system_id), &((__lac_2).system_id), sizeof(MAC_ADDRESS));   \
    }                                                                                               \
}

/*---------------------------------------------------------------------------
 * MACRO NAME   - LACP_TYPE_GET_PORT_STATE_BIT
 * --------------------------------------------------------------------------
 * PURPOSE      : Get the specified bit field in the LACP_PORT_STATE_T format
 * INPUT        : PORT_STATE_T          __port_state;
 *                                      __bit_mask;
 * RETURN       : The corresponding bit value
 * NOTES        : The bit_mask is defined as one of the following values.
 *              #define LACP_dot3adAggPortActorOperState_LACP_Activity   0x01
 *              #define LACP_dot3adAggPortActorOperState_LACP_Timeout    0x02
 *              #define LACP_dot3adAggPortActorOperState_Aggregation     0x04
 *              #define LACP_dot3adAggPortActorOperState_Synchronization 0x08
 *              #define LACP_dot3adAggPortActorOperState_Collecting      0x10
 *              #define LACP_dot3adAggPortActorOperState_Distributing    0x20
 *              #define LACP_dot3adAggPortActorOperState_Defaulted       0x40
 *              #define LACP_dot3adAggPortActorOperState_Expired         0x80
 * --------------------------------------------------------------------------
 */
#define LACP_TYPE_GET_PORT_STATE_BIT(__port_state, __bit_mask) (((__port_state) & (__bit_mask)) != 0)


/*---------------------------------------------------------------------------
 * MACRO NAME   - LACP_TYPE_SET_PORT_STATE_BIT
 * --------------------------------------------------------------------------
 * PURPOSE      : Set the specified bit field in the LACP_PORT_STATE_U format
 * INPUT        : PORT_STATE_T           __port_state;
 *                BOOL_T                 __bit_value;
 *                                       __bit_mask;
 * OUTPUT       : None
 * NOTES        : The bit_mask is defined as one of the following values.
 *              #define LACP_dot3adAggPortActorOperState_LACP_Activity   0x01
 *              #define LACP_dot3adAggPortActorOperState_LACP_Timeout    0x02
 *              #define LACP_dot3adAggPortActorOperState_Aggregation     0x04
 *              #define LACP_dot3adAggPortActorOperState_Synchronization 0x08
 *              #define LACP_dot3adAggPortActorOperState_Collecting      0x10
 *              #define LACP_dot3adAggPortActorOperState_Distributing    0x20
 *              #define LACP_dot3adAggPortActorOperState_Defaulted       0x40
 *              #define LACP_dot3adAggPortActorOperState_Expired         0x80
 * --------------------------------------------------------------------------
 */
#define LACP_TYPE_SET_PORT_STATE_BIT(__port_state, __bit_value, __bit_mask)         \
{                                                                                   \
    (__port_state) = (__bit_value)?((__port_state)|(__bit_mask))                  \
                                   :((__port_state)&(~(__bit_mask)));             \
}

/* This value have to change according to tick value in different OS. */
#define LACPDUD_TIMER_TICKS1SEC                             100			/* every 1 sec send LACP a timer event */

#define LACP_SHORT_TIMEOUT_TIME                             3
#define LACP_LONG_TIMEOUT_TIME                              90

#define LACP_FAST_PERIODIC_TIME                             1
#define LACP_SLOW_PERIODIC_TIME                             30

#define LACP_CHURN_DETECTION_TIME                           60

#define LACP_AGGREGATE_WAIT_TIME                            2
#define LACP_AGGREGATE_NOWAIT_TIME                          0

#define LACP_TLV_TYPE_ACTOR_INFORMATION                     0x01
#define LACP_TLV_TYPE_PARTNER_INFORMATION                   0x02
#define LACP_TLV_TYPE_COLLECTOR_INFORMATION                 0x03
#define LACP_TLV_TYPE_TERMINATOR_INFORMATION                0x00
#define LACP_TLV_TYPE_MARKER_INFORMATION                    0x01
#define LACP_TLV_TYPE_MARKER_RESPONSE                       0x02

#define LACP_ACTOR_INFORMATION_LENGTH                       0x14
#define LACP_PARTNER_INFORMATION_LENGTH                     0x14
#define LACP_COLLECTOR_INFORMATION_LENGTH                   0x10
#define LACP_TERMINATOR_INFORMATION_LENGTH                  0x00
#define LACP_MARKER_INFORMATION_LENGTH                      0x10

#define LACPDU_Slow_Protocols_Type                          0x8809

#define LACP_COLLECTOR_MAX_DELAY                            0   /* If not forwarding, then discard it. */

#define LACP_MAX_FRAMES_IN_ONE_SECOND                       3   /* Check P150 in 802.3ad */

#define MAX_AGGREGATOR_ID       65535


typedef UI8_T   MAC_ADDRESS[6];
typedef UI16_T  SYSTEM_PRIORITY_T;
typedef UI32_T  AGGREGATOR_IDENTIFIER_T;
typedef UI8_T   AGGREGATOR_DESCRIPTION_T[255];
typedef UI16_T  LACP_KEY_T;

typedef UI16_T  PORT_PRIORITY_T;
typedef UI8_T   PORT_STATE_T;
typedef UI16_T  PPORT_T;       /* Physical port number (1 to MAX_PORTS)    */

/* You should use 1 byte alignment here. */
#pragma pack(1)
typedef struct
{
    UI16_T      msg_type;   /* 2 bytes              */
    UI8_T       saddr[6];   /* 6 bytes              */
    L_MM_Mref_Handle_T   *mref_handle_p; /* 4 bytes           */
    UI16_T      unit_no;    /* 2 bytes              */
    UI16_T      port_no;    /* 2 bytes              */
} LACP_MSG_T;               /* 16 bytes in total    */

/* 2001/11/30 ckhsu comment                                                                 */
/* This is my mistake, originally I just think we should use the Bit-Field in C to assign   */
/* the bit in the frame which will send to the network. But finally I found that I made     */
/* a mistake in it. Now I can only suggest the successor who will maintain this or write    */
/* a fully new protocol, if you want do use the bit shift in C rather than this.            */

typedef union LACP_PORT_STATE_U
{
    struct
    {
        PORT_STATE_T Expired:1;
        PORT_STATE_T Defaulted:1;
        PORT_STATE_T Distributing:1;
        PORT_STATE_T Collecting:1;
        PORT_STATE_T Synchronization:1;
        PORT_STATE_T Aggregation:1;
        PORT_STATE_T LACP_Timeout:1;
        PORT_STATE_T LACP_Activity:1;
    } bit_field;
    PORT_STATE_T      port_state;
}LACP_PORT_STATE_T;

typedef struct LAC_INFO_S
{
    SYSTEM_PRIORITY_T   system_priority;
    MAC_ADDRESS         system_id;
    LACP_KEY_T          key;
    PORT_PRIORITY_T     port_priority;
    PPORT_T             port_no;
    LACP_PORT_STATE_T   state;
} LAC_INFO_T;

typedef struct /* Lac_pdu */
{
    /* In Mercury, the Pdu always has no ether header */
    /* MAC_ADDRESS slow_protocols_address;  */
    /* MAC_ADDRESS destination_address;     */
    /* UI16_T      ethertype;               */
    UI8_T       protocol_subtype;
    UI8_T       protocol_version;
    UI8_T       tlv_type_actor;
    UI8_T       actor_info_length;
    LAC_INFO_T  actor;
    UI8_T       actor_reserved[3];
    UI8_T       tlv_type_partner;
    UI8_T       partner_info_length;
    LAC_INFO_T  partner;
    UI8_T       partner_reserved[3];
    UI8_T       tlv_type_collector;
    UI8_T       collector_info_length;
    UI16_T      collector_max_delay;
    UI8_T       collector_reserved[12];
    UI8_T       tlv_type_terminator;
    UI8_T       terminator_info_length;
    UI8_T       terminator_reserved[50];
} Lac_pdu;

typedef struct /* Marker_pdu */
{
    /* In Mercury, the Pdu always has no ether header */
    /* MAC_ADDRESS slow_protocols_address;  */
    /* MAC_ADDRESS destination_address;     */
    /* UI16_T      ethertype;               */
    UI8_T       protocol_subtype;
    UI8_T       protocol_version;
    UI8_T       tlv_type_marker_infomation;
    UI8_T       marker_info_length;
    UI16_T      request_port;
    MAC_ADDRESS request_system;
    UI32_T      request_transaction_id;
    UI8_T       pad[2];
    UI8_T       tlv_type_terminator;
    UI8_T       terminator_info_length;
    UI8_T       reserved[90];
} Marker_pdu;


typedef union
{
    Lac_pdu     lacp;
    Marker_pdu  marker;
} LACP_PDU_U;

typedef struct LACP_PORT_S       LACP_PORT;
typedef struct LACP_AGGREGATOR_S LACP_AGGREGATOR;

typedef struct LACP_PORT_STATISTICS_S /* LACP Port Statistics */
{
    UI32_T           LACPDUsRx;
    UI32_T           MarkerPDUsRx;
    UI32_T           MarkerResponsePDUsRx;
    UI32_T           UnknownRx;
    UI32_T           IllegalRx;
    UI32_T           LACPDUsTx;
    UI32_T           MarkerPDUsTx;
    UI32_T           MarkerResponsePDUsTx;
    UI32_T			 AnnouncePDUsRx;
}LACP_PORT_STATISTICS_T;

typedef struct LACP_PORT_DEBUG_INFO_S /* LACP Port Statistics */
{
    RX_MACHINE_STATE_E  RxState;
    UI32_T              LastRxTime;
    UI8_T               MatchState;
    MUX_MACHINE_STATE_E MuxState;
    UI8_T               MuxReason[5];
    CHURN_STATE_E       ActorChurnState;
    CHURN_STATE_E       PartnerChurnState;
    UI32_T              ActorChurnCount;
    UI32_T              PartnerChurnCount;
    UI32_T              ActorSyncTransitionCount;
    UI32_T              PartnerSyncTransitionCount;
    UI32_T              ActorChangeCount;
    UI32_T              PartnerChangeCount;
}LACP_PORT_DEBUG_INFO_T;

typedef struct LACP_TIMER_S
{
    BOOL_T  enabled;
    UI32_T  tick;
} LACP_TIMER_T;

typedef struct LACP_PORT_TIMER_S
{
    LACP_TIMER_T    current_while_timer;
    LACP_TIMER_T    actor_churn_timer;
    LACP_TIMER_T    periodic_timer;
    LACP_TIMER_T    partner_churn_timer;
    LACP_TIMER_T    wait_while_timer;
} LACP_PORT_TIMER_T;


typedef struct LACP_PORT_S /* LACP Port data */
{
    LAC_INFO_T              AggActorPort;           /* ActorOper                */
    LAC_INFO_T              AggPartnerPort;         /* PartnerOper              */
    LAC_INFO_T              AggActorAdminPort;      /* ActorAdmin               */
    LAC_INFO_T              AggPartnerAdminPort;    /* PartnerAdmin             */
    LACP_PORT_STATISTICS_T  AggPortStats;           /* PortStatus MIB variables */
    LACP_PORT_DEBUG_INFO_T  AggPortDebug;           /* Debug MIB variables      */
    UI8_T                   AggPortAggregateOrIndividual;
    BOOL_T                  static_admin_key;
    BOOL_T                  LACP_OperStatus;        /* LACP Operation Status    */
    BOOL_T                  Port_OperStatus;        /* Port Operation Status    */
    BOOL_T                  PortEnabled;            /* Port admin status        */
    BOOL_T                  LACPEnabled;            /* LACP admin status        */
    BOOL_T                  LinkUp;                 /* Link status              */
    BOOL_T                  NTT;                    /* Need To Transmit         */
    UI32_T                  PortSpeed;              /* Port Speed Status        */
    BOOL_T                  FullDuplexMode;         /* Duplex Status            */
    SELECTED_VALUE_E        Selected;               /* SELECTED of port         */
    BOOL_T                  Matched;                /* Partner match indicator  */
    BOOL_T                  Attach;                 /* Attach indication        */
    BOOL_T                  Attached;               /* Attached indication      */
    BOOL_T                  PartnerChurn;           /* Partner Churn indication */
    BOOL_T                  ActorChurn;             /* Actor Churn indication   */
    UI8_T                   FramesInLastSec;        /* Frames in Last Second    */
    UI8_T                   LogicalTrunkMemberFlag; /* Reserved for future      */
    UI8_T                   DisconDelayedFlag;      /* Reserved for future      */
    UI8_T                   PortBlockedFlag;        /* Reserved for future      */
    UI32_T                  InitialPDUTimer;        /* Reserved for future      */
    BOOL_T                  PortMoved;              /* Reserved for future      */
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)    
    BOOL_T                  Is_static_lacp;         /* Static lacp port flag      */
    BOOL_T                  Last_InterfaceStatus;   /* Last interface  status      */
#endif
    LACP_PORT_TIMER_T       port_timer;             /* Port Timer               */
    UI32_T                  slot;                   /* Logical slot number      */
    UI32_T                  port;                   /* Logical port number      */
    LACP_AGGREGATOR         *pAggregator;   /* Each port belongs to the aggregator     */
    LACP_PORT               *pNext;         /* Link to next port in the aggregator     */
                                            /* Note: the above list should be a sorted */
                                            /* list.                                   */
    UI32_T                  port_index;     /* The sequence in the LAG */
}LACP_PORT_T;

/* Lewis: the following is used when state machine is running */
/* It will link the ports to its data structure.           */
typedef struct LACP_AGGREGATOR_S /* LACP Aggregator data */
{
    AGGREGATOR_IDENTIFIER_T     AggID;
    AGGREGATOR_DESCRIPTION_T    AggDescription;
    UI8_T                       AggAggregateOrIndividual;
    MAC_ADDRESS                 AggMACAddress;
    MAC_ADDRESS                 AggActorSystemID;
    SYSTEM_PRIORITY_T           AggActorSystemPriority;
    LACP_KEY_T                  AggActorOperKey;
    LACP_KEY_T                  AggActorAdminKey;
    MAC_ADDRESS                 AggPartnerSystemID;
    SYSTEM_PRIORITY_T           AggPartnerSystemPriority;
    LACP_KEY_T                  AggPartnerOperKey;
    UI8_T                       AggAdminState;
    UI8_T                       AggOperState;
    UI32_T                      AggTimeOfLastOperChange;
    UI8_T                       AggLinkUpDownNotificationEnable;
    UI32_T                      AggLinkUpNotification;
    UI32_T                      AggLinkDownNotification;
    UI32_T                      AggCollectorMaxDelay;
    UI32_T                      AggActorTimeout;
    UI32_T                      NumOfPortsInAgg;
    UI32_T                      NumOfSelectedPorts;
    UI32_T                      NumOfCollectingDistributingPorts; /* # of Partner.Sync = TRUE */
    BOOL_T                      static_admin_key;
    /*
     * Each Aggregator may consists of up to SYS_ADPT_MAX_NBR_OF_PORT_INSTALLED ports,
     * this is a hardware limitation.
     */
    LACP_PORT_T                 *pLACP_Ports;
}LACP_AGGREGATOR_T;

typedef struct LACP_SYSTEM_S
{
    /* In this data structure, we should include the following: */
    /* 1. system data                                           */
    /* 2. all aggregator data.                                  */
    SYSTEM_PRIORITY_T    SystemPriority;
    MAC_ADDRESS          SystemId;
    /* all aggregators.                                         */
    LACP_AGGREGATOR_T    aggregator[SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM];
    /* CKhsu:                                                   */
    /* This is special for whole system LACP Enabled/Disabled   */
    /* A foolish setting, if all ports are disabled, and this   */
    /* is enabled, then what is the meaning of whole system?    */
    UI8_T               lacp_oper;  /* 1: up, 2: down */
    UI32_T              last_changed;
}LACP_SYSTEM_T;

#pragma pack()

#if 0
/* The following is global section */
extern LACP_SYSTEM_T        lacp_system;
extern LACP_PORT_T          lacp_port[MAX_PORTS];
#endif

/*
extern UI32_T               LACP_Smid;
extern UI32_T               LACP_Tsk_ID;
extern UI32_T               LACPDU_PacketQueueID;
extern UI8_T                LACPDU_Slow_Protocols_Multicast[6];*/


extern BOOL_T bLACP_DbgMsg;
extern BOOL_T bLACP_DbgPdu;
extern BOOL_T bLACP_DbgAthAL;
extern BOOL_T bLACP_DbgAttach;
extern BOOL_T bLACP_DbgTrace;

/* Function inside LACP, not seen from outside LACP.    */

/* The following functions are defined in lacp_init.c */
void LACP_init_port( LACP_PORT_T *port);

/* The following functions are defined in lacp_rxm.c */
void LACP_Rx_Machine( LACP_PORT_T *pPort, LACP_EVENT_E event, LACP_PDU_U *pPdu);

/* The following functions are defined in lacp_txm.c */
void LACP_Tx_Machine( LACP_PORT_T *pPort, LACP_EVENT_E event);
void LACP_Periodic_Machine( LACP_PORT_T *pPort, LACP_EVENT_E event);

/* The following functions are defined in lacp_muxm.c */
void LACP_Mux_Machine( LACP_PORT_T *pPort, LACP_EVENT_E event);

/* The following functions are defined in lacp_mkrm.c */
void LACP_Marker_Machine( LACP_PORT_T *pPort, LACP_EVENT_E event, LACP_PDU_U *pPdu);

/* The following are defined in lacp_hwdep.c                    */
/* These are OS/Hardware dependent calls.                       */
/* Remember to modify if you want to port to another platform.  */
void LACP_CreateSemaphore();
void LACP_CreatePacketQueue();
void LACP_LACPduDaemonInit(void);
void LACP_SendPDU( LACP_PDU_U *pPDU, UI32_T portNo);
void LACP_Print_PDU_Info( LACP_PDU_U *pPDU);

/* Moved from lacp_mgr.h
 */
typedef enum
{
    LACP_RETURN_SUCCESS= 0,          /* OK, Successful, Without any Error */
    LACP_RETURN_PORT_NOT_EXIST,
    LACP_RETURN_CANNOT_DELETE_TRUNK_MEMBER,
    LACP_RETURN_INVALID_VALUE,
    LACP_RETURN_ERROR           /* Error */
} LACP_RETURN_VALUE;


#endif /* _LACP_TYPE_H */

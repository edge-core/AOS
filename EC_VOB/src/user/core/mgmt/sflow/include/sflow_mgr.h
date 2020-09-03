/* -------------------------------------------------------------------------------------
 * FILE NAME:  SFLOW_MGR.h
 * -------------------------------------------------------------------------------------
 * PURPOSE:  This package provides the sevices to manage the RFC3176 MIB.
 * NOTE:  1. The key functions of this module are to provide interfaces for the
 *           upper layer to configure sFlow, update database information base on the
 *           confiugration, and configure the lower layers(swctrl).
 *        2. This package shall be a reusable package for all the L2/L3 switchs.
 *
 * MODIFICATION HISTORY:
 * Modifier      Date          Version      Description
 * -------------------------------------------------------------------------------------
 * Joeanne      10-25-2007       V1.0       First Created
 * Nelson Dai   09-13-2009       V2.0       Porting from vxWorks to Linux platform
 * Nelson Dai   09-25-2009       V2.1       sFlow over IPv6
 * Nelson Dai   12-03-2009       V2.2       Support RFC-3176 MIB
 * -------------------------------------------------------------------------------------
 * Copyright(C)                 Accton Technology Corp. 2007
 * -------------------------------------------------------------------------------------*/

#ifndef _SFLOW_MGR_H
#define _SFLOW_MGR_H

#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "l_inet.h"
#include "sysfun.h"

#if (SYS_CPNT_SFLOW == TRUE)

/* NAMING CONSTANT DECLARATIONS
 */
#define SFLOW_TIMER_TICKS2SEC   100     /* every 1 sec send a sFlow timer event */

#define SFLOW_MGR_MIN_IFINDEX                            1
#define SFLOW_MGR_MAX_IFINDEX                            SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_TOTAL_PORTS_PER_UNIT_ON_BOARD

#define SFLOW_MGR_RECEIVER_TIMEOUT_DISABLE               0
#define SFLOW_MGR_SAMPLING_RATE_DISABLE                  0
#define SFLOW_MGR_POLLING_INTERVAL_DISABLE               0
#define SFLOW_MGR_MIN_INSTANCE_ID                        1
#define SFLOW_MGR_MAX_INSTANCE_ID                        SYS_ADPT_SFLOW_MAX_INSTANCE_OF_DATASOURCE

#define SFLOW_MGR_MIN_RECEIVER_SOCK_PORT                 1
#define SFLOW_MGR_MAX_RECEIVER_SOCK_PORT                 65535
#define SFLOW_MGR_RECEIVER_SOCK_PORT                     6343

#define SFLOW_MGR_MIN_RECEIVER_DATAGRAM_VERSION          SFLOW_MGR_DATAGRAM_VERSION_4
#define SFLOW_MGR_MAX_RECEIVER_DATAGRAM_VERSION          SFLOW_MGR_DATAGRAM_VERSION_5

#define SFLOW_MGR_MIN_RECEIVER_INDEX                     1
#define SFLOW_MGR_MAX_RECEIVER_INDEX                     SYS_ADPT_SFLOW_MAX_NUMBER_OF_RECEIVER_ENTRY

#define SFLOW_MGR_MIN_RECEIVER_OWNER_STR_LEN             1

#define SFLOW_MGR_FCS_BYTES                              4
#define SFLOW_MGR_TPID_BYTES                             2
#define SFLOW_MGR_TAG_BYTES                              2

/* MACRO FUNCTION DECLARATIONS
 */
#define SFLOW_MGR_MSGBUF_TYPE_SIZE sizeof(union SFLOW_MGR_IPCMsg_Type_U)
#define SFLOW_MGR_GET_MSG_SIZE(field_name)                       \
                    (SFLOW_MGR_MSGBUF_TYPE_SIZE +                \
                    sizeof(((SFLOW_MGR_IPCMsg_T*)0)->data.field_name))

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    SFLOW_MGR_RETURN_SUCCESS = 0,
    SFLOW_MGR_RETURN_FAIL
} SFLOW_MGR_RETURN_VALUE_T;

typedef enum
{
    SFLOW_MGR_DATAGRAM_VERSION_4 = 4,
    SFLOW_MGR_DATAGRAM_VERSION_5 = 5,
} SFLOW_MGR_DATAGRAM_VERSION_T;

typedef enum
{
    SFLOW_MGR_SAMPLE_TYPE_PACKET  = 1,            /* enterprise = 0 : format = 1 */
    SFLOW_MGR_SAMPLE_TYPE_COUNTER = 2             /* enterprise = 0 : format = 2 */
} SFLOW_MGR_SAMPLE_TYPE_T;

typedef enum
{
    SFLOW_MGR_SAMPLE_RECORD_TYPE_HEADER   = 1,    /* enterprise = 0 : format = 1 */
    SFLOW_MGR_SAMPLE_RECORD_TYPE_ETHERNET = 2,    /* enterprise = 0 : format = 2 */
    SFLOW_MGR_SAMPLE_RECORD_TYPE_IPV4     = 3,    /* enterprise = 0 : format = 3 */
    SFLOW_MGR_SAMPLE_RECORD_TYPE_IPV6     = 4     /* enterprise = 0 : format = 4 */
} SFLOW_MGR_SAMPLE_RECORD_TYPE_T;

typedef enum
{
    SFLOW_MGR_SAMPLE_HEADER_PROTOCOL_ETHERNET = 1,
    SFLOW_MGR_SAMPLE_HEADER_PROTOCOL_IPV4     = 11,
    SFLOW_MGR_SAMPLE_HEADER_PROTOCOL_IPV6     = 12
} SFLOW_MGR_SAMPLE_HEADER_PROTOCOL_T;

typedef enum
{
    SFLOW_MGR_COUNTER_RECORD_TYPE_IF       = 1,    /* enterprise = 0 : format = 1 */
    SFLOW_MGR_COUNTER_RECORD_TYPE_ETHERNET = 2,    /* enterprise = 0 : format = 2 */
} SFLOW_MGR_COUNTER_RECORD_TYPE_T;

typedef struct
{
    L_INET_AddrIp_T  address;
    UI32_T           receiver_index;    /* index */
    UI32_T           udp_port;
    UI32_T           timeout;
    UI32_T           max_datagram_size;
    UI32_T           datagram_version;
    char             owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1];
} SFLOW_MGR_Receiver_T;

typedef struct
{
    UI32_T  ifindex;                    /* index 1 */
    UI32_T  instance_id;                /* index 2 */
    UI32_T  max_header_size;            /* byte */
    UI32_T  sampling_rate;
    UI32_T  receiver_index;
    char    receiver_owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1];
} SFLOW_MGR_Sampling_T;

typedef struct
{
    UI32_T  ifindex;                    /* index 1 */
    UI32_T  instance_id;                /* index 2 */
    UI32_T  polling_interval;           /* sec */
    UI32_T  receiver_index;
    char    receiver_owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1];
} SFLOW_MGR_Polling_T;

typedef struct
{
    UI8_T  dst_mac;
    UI8_T  src_mac;
    UI16_T ether_type;
} SFLOW_MGR_EthernetHeader_T;

typedef struct
{
    UI32_T ifIndex;
    UI32_T ifType;
    UI64_T ifSpeed;
    UI32_T ifDirection;    /* 0=unknown|1=full-duplex|2=half-duplex|3=in|4=out */
    UI32_T ifStatus;       /* bit 0 => ifAdminStatus 0=down|1=up, bit 1 => ifOperStatus 0=down|1=up */
    UI64_T ifInOctets;
    UI32_T ifInUcastPkts;
    UI32_T ifInMulticastPkts;
    UI32_T ifInBroadcastPkts;
    UI32_T ifInDiscards;
    UI32_T ifInErrors;
    UI32_T ifInUnknownProtos;
    UI64_T ifOutOctets;
    UI32_T ifOutUcastPkts;
    UI32_T ifOutMulticastPkts;
    UI32_T ifOutBroadcastPkts;
    UI32_T ifOutDiscards;
    UI32_T ifOutErrors;
    UI32_T ifPromiscuousMode;
} SFLOW_MGR_IfCounters_T;

typedef struct
{
    UI32_T dot3StatsAlignmentErrors;
    UI32_T dot3StatsFCSErrors;
    UI32_T dot3StatsSingleCollisionFrames;
    UI32_T dot3StatsMultipleCollisionFrames;
    UI32_T dot3StatsSQETestErrors;
    UI32_T dot3StatsDeferredTransmissions;
    UI32_T dot3StatsLateCollisions;
    UI32_T dot3StatsExcessiveCollisions;
    UI32_T dot3StatsInternalMacTransmitErrors;
    UI32_T dot3StatsCarrierSenseErrors;
    UI32_T dot3StatsFrameTooLongs;
    UI32_T dot3StatsInternalMacReceiveErrors;
    UI32_T dot3StatsSymbolErrors;
} SFLOW_MGR_EthernetCounters_T;

/* structure for the request/response ipc message in csc pmgr and mgr
 */
typedef struct SFLOW_MGR_IPCMsg_S
{
    union SFLOW_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;              /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        UI32_T result_ui32;      /* respond SFLOW_MGR_RETURN_VALUE_T return */
    } type;

    union
    {
        BOOL_T bool_v;
        UI32_T ui32_v;
        L_INET_AddrIp_T addr_v;

        struct
        {
            char owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1];
        } ownername;

        struct
        {
            UI32_T ui32_a1;
            char   owner_name_a2[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1];
        } ui32_ownername;

        struct
        {
            UI32_T          ui32_a1;
            L_INET_AddrIp_T ip_a2;
        } ui32_ip;

        struct
        {
            UI32_T ui32_a1;
            UI32_T ui32_a2;
        } ui32_ui32;

        struct
        {
            UI32_T ui32_a1;
            UI32_T ui32_a2;
            UI32_T ui32_a3;
        } ui32_ui32_ui32;

        struct
        {
            SFLOW_MGR_Receiver_T entry;
        } receiver;

        struct
        {
            SFLOW_MGR_Sampling_T entry;
        } sampling;

        struct
        {
            SFLOW_MGR_Polling_T entry;
        } polling;

    } data; /* contains the supplemental data for the corresponding cmd */
} SFLOW_MGR_IPCMsg_T;

typedef enum SFLOW_MGR_IPCCMD_E
{
    SFLOW_MGR_IPCCMD_GET_AGENT_ADDRESS,
    SFLOW_MGR_IPCCMD_GET_AGENT_ADDRESS_TYPE,

    SFLOW_MGR_IPCCMD_SET_RECEIVER_OWNER,
    SFLOW_MGR_IPCCMD_SET_RECEIVER_TIMEOUT,
    SFLOW_MGR_IPCCMD_SET_RECEIVER_DESTINATION,
    SFLOW_MGR_IPCCMD_SET_RECEIVER_SOCK_PORT,
    SFLOW_MGR_IPCCMD_SET_RECEIVER_MAX_DATAGRAM_SIZE,
    SFLOW_MGR_IPCCMD_SET_RECEIVER_DATAGRAM_VERSION,
    SFLOW_MGR_IPCCMD_CREATE_RECEIVER_ENTRY,
    SFLOW_MGR_IPCCMD_DESTROY_RECEIVER_ENTRY_BY_OWNER_NAME,
    SFLOW_MGR_IPCCMD_GET_RECEIVER_ENTRY,
    SFLOW_MGR_IPCCMD_GETNEXT_RECEIVER_ENTRY,
    SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_RECEIVER_ENTRY,
    SFLOW_MGR_IPCCMD_GET_ACTIVE_RECEIVER_ENTRY_BY_OWNER_NAME,

    SFLOW_MGR_IPCCMD_SET_SAMPLING_RATE,
    SFLOW_MGR_IPCCMD_SET_SAMPLING_MAX_HEADER_SIZE,
    SFLOW_MGR_IPCCMD_SET_SAMPLING_RECEIVER_INDEX,
    SFLOW_MGR_IPCCMD_CREATE_SAMPLING_ENTRY,
    SFLOW_MGR_IPCCMD_DESTROY_SAMPLING_ENTRY,
    SFLOW_MGR_IPCCMD_GET_SAMPLING_ENTRY,
    SFLOW_MGR_IPCCMD_GET_ACTIVE_SAMPLING_ENTRY,
    SFLOW_MGR_IPCCMD_GETNEXT_SAMPLING_ENTRY,
    SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_SAMPLING_ENTRY_BY_RECEIVER_OWNER_NAME,
    SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_SAMPLING_ENTRY_BY_DATASOURCE,

    SFLOW_MGR_IPCCMD_SET_POLLING_INTERVAL,
    SFLOW_MGR_IPCCMD_SET_POLLING_RECEIVER_INDEX,
    SFLOW_MGR_IPCCMD_CREATE_POLLING_ENTRY,
    SFLOW_MGR_IPCCMD_DESTROY_POLLING_ENTRY,
    SFLOW_MGR_IPCCMD_GET_POLLING_ENTRY,
    SFLOW_MGR_IPCCMD_GET_ACTIVE_POLLING_ENTRY,
    SFLOW_MGR_IPCCMD_GETNEXT_POLLING_ENTRY,
    SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_POLLING_ENTRY_BY_RECEIVER_OWNER_NAME,
    SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_POLLING_ENTRY_BY_DATASOURCE,

} SFLOW_MGR_IPCCMD_T;

enum SFLOW_MGR_EVENT_MASK_E
{
    SFLOW_MGR_EVENT_NONE             = 0x0000L,
    SFLOW_MGR_EVENT_PACKET           = 0x0001L,
    SFLOW_MGR_EVENT_TIMER            = 0x0002L,
    SFLOW_MGR_EVENT_TIMEOUT          = 0x0004L,
    SFLOW_MGR_EVENT_ENTER_TRANSITION = 0x0008L,
    SFLOW_MGR_EVENT_ALL              = 0xFFFFL
};

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SFLOW_MGR_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTE:    None.
 *---------------------------------------------------------------------------
 */
void SFLOW_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It is used to enable the sflow operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It's the temporary transition mode between system into master
 *            mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_MGR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Set SFLOW op_state to "SYS_TYPE_STACKING_SLAVE_MODE"
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void SFLOW_MGR_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void SFLOW_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This functions returns the current operation mode of this component
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T SFLOW_MGR_GetOperationMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetReceiverOwner
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver owner.
 * INPUT    : receiver_index  -- receiver index
 *            owner_name_p    -- owner name
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_SetReceiverOwner(
    UI32_T receiver_index,
    char *owner_name_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetReceiverTimeout
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver timeout.
 * INPUT    : receiver_index  -- receiver index
 *            timeout         -- timeout value
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_SetReceiverTimeout(
    UI32_T receiver_index,
    UI32_T timeout);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetReceiverDestination
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver destination IP address.
 * INPUT    : receiver_index  -- receiver index
 *            address_p       -- destination IP address
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_SetReceiverDestination(
    UI32_T receiver_index,
    L_INET_AddrIp_T *address_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetReceiverSockPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver sock port.
 * INPUT    : receiver_index -- receiver index
 *            udp_port       -- udp port number
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_SetReceiverSockPort(
    UI32_T receiver_index,
    UI32_T udp_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetReceiverMaxDatagramSize
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver maximum datagram size.
 * INPUT    : receiver_index     -- receiver index
 *            max_datagram_size  -- maximum datagram size
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_SetReceiverMaxDatagramSize(
    UI32_T receiver_index,
    UI32_T max_datagram_size);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetReceiverDatagramVersion
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver datagram version.
 * INPUT    : receiver_index    -- receiver index
 *            datagram_version  -- datagram version
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_SetReceiverDatagramVersion(
    UI32_T receiver_index,
    UI32_T datagram_version);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CreateReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create receiver entry or update receiver timeout.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : owner_name is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_CreateReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_DestroyReceiverEntryByOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy receiver entry by owner name.
 * INPUT    : owner_name_p  -- owner name
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_DestroyReceiverEntryByOwnerName(
    char *owner_name_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetNextReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetNextReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetActiveReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get active receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetActiveReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetNextActiveReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetNextActiveReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetActiveReceiverEntryByOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Get active receiver entry by owner name.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : owner_name is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetActiveReceiverEntryByOwnerName(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetSamplingRate
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling rate.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 *            rate         -- sampling rate
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_SetSamplingRate(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T rate);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetSamplingMaxHeaderSize
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling maximum header size.
 * INPUT    : ifindex          -- interface index
 *            instance_id      -- instance id
 *            max_header_size  -- sampling maximum header size
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_SetSamplingMaxHeaderSize(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T max_header_size);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetSamplingReceiverIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling receiver index.
 * INPUT    : ifindex         -- interface index
 *            instance_id     -- instance id
 *            receiver_index  -- sampling receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_SetSamplingReceiverIndex(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CreateSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_CreateSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_DestroySamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy sampling entry.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_DestroySamplingEntry(
    UI32_T ifindex,
    UI32_T instance_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetActiveSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get active sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetActiveSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetNextSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetNextSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetNextActiveSamplingEntryByReveiverOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active sampling entry by receiver owner name.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_owner_name is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetNextActiveSamplingEntryByReveiverOwnerName(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetNextActiveSamplingEntryByDatasource
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active sampling entry by data source.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetNextActiveSamplingEntryByDatasource(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetPollingInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Set polling interval.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 *            interval     -- polling interval
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_SetPollingInterval(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_SetPollingReceiverIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Set polling receiver index.
 * INPUT    : ifindex         -- interface index
 *            instance_id     -- instance id
 *            receiver_index  -- polling receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_SetPollingReceiverIndex(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CreatePollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_CreatePollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_DestroyPollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy polling entry.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_DestroyPollingEntry(
    UI32_T ifindex,
    UI32_T instance_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetPollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetPollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetActivePollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get active polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetActivePollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetNextPollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetNextPollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetNextActivePollingEntryByReveiverOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active polling entry by receiver owner name.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_owner_name is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetNextActivePollingEntryByReveiverOwnerName(
    SFLOW_MGR_Polling_T *polling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_GetNextActivePollingEntryByDatasource
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active polling entry by data source.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetNextActivePollingEntryByDatasource(
    SFLOW_MGR_Polling_T *polling_entry_p);

/***************************************************************************/
/* sFlow agent */
/***************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SFLOW_MGR_GetAgentAddress
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sFlow agent's address.
 * INPUT   : addr_p -- agent's address
 * OUTPUT  : None
 * RETURN  : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetAgentAddress(
    L_INET_AddrIp_T *addr_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SFLOW_MGR_GetAgentAddressType
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sFlow agent's address type.
 * INPUT   : None
 * OUTPUT  : type  -- agent's address type
 * RETURN  : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_GetAgentAddressType(
    UI32_T *type);

/***************************************************************************/
/* sFlow config */
/***************************************************************************/


/***************************************************************************/
/* sFlow agent*/
/***************************************************************************/
/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_AnnounceSamplePacket_CallBack
 *--------------------------------------------------------------------------
 * PURPOSE  : It is used to announce sample packet.
 * INPUT    : mref_handle_p  -- the memory reference address
 *            dst_mac        -- destination mac address
 *            src_mac        -- source mac address
 *            tag_info       -- packet tag info
 *            ether_type     -- ethernet type
 *            pkt_length     -- packet length
 *            src_unit       -- source unit
 *            src_port       -- source port
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
void
SFLOW_MGR_AnnounceSamplePacket_CallBack(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T *dst_mac,
    UI8_T *src_mac,
    UI16_T tag_info,
    UI16_T ether_type,
    UI32_T pkt_length,
    UI32_T src_unit,
    UI32_T src_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_ProcessReceiverTimeoutCountdown
 *-------------------------------------------------------------------------
 * PURPOSE  : Process receiver timeout value countdown.
 * INPUT    : receiver_index  -- receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_ProcessReceiverTimeoutCountdown(
    UI32_T receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_ProcessPolling
 *-------------------------------------------------------------------------
 * PURPOSE  : Process polling.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_ProcessPolling(
    UI32_T ifindex,
    UI32_T instance_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_HandleIpcReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the ipc request message for SFLOW MGR.
 * INPUT    : ipcmsg_p -- input request ipc message buffer
 * OUTPUT   : ipcmsg_p -- output response ipc message buffer
 * RETUEN   : TRUE  - There is a response need to send,
 *            FALSE - There is no response to send.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T SFLOW_MGR_HandleIpcReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif /* #if (SYS_CPNT_SFLOW == TRUE) */
#endif /* #ifndef _SFLOW_MGR_H */

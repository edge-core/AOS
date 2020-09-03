/* -----------------------------------------------------------------------------
 * FILE NAME: SFLOW_OM.h
 *
 * PURPOSE: This package serves as a database to store sFlow MIB defined
 *          information.
 * MODIFICATION HISOTRY:
 * MODIFIER               DATE         DESCRIPTION
 * ------------------------------------------------------------------------------
 * Joeanne              10-25-2007      First created
 * Nelson Dai           09-13-2009      Porting from vxWorks to Linux platform
 * Nelson Dai           09-25-2009      sFlow over IPv6
 * ------------------------------------------------------------------------------
 * Copyright(C)        Accton Techonology Corporation 2007
 * ------------------------------------------------------------------------------*/

/* INCLUDE FILE DECLARATIONS
 */
#ifndef _SFLOW_OM_H
#define _SFLOW_OM_H

#include "sys_adpt.h"
#include "sys_type.h"
#include "sysfun.h"
#include "l_inet.h"
#include "sysrsc_mgr.h"
#include "sflow_mgr.h"

#if (SYS_CPNT_SFLOW == TRUE)

/* NAMING CONSTANT DECLARATIONS
 */
#define SFLOW_OM_DATA_PAD 400

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    SFLOW_OM_RETURN_SUCCESS = 0,
    SFLOW_OM_RETURN_FAIL
} SFLOW_OM_RETURN_VALUE_T;

typedef struct
{
    UI32_T version;
    UI32_T max_size; /* The maximum number of data bytes that can be sent in a single sample datagram. */
    UI32_T sequence;
    UI32_T sample_number;
    UI32_T data_ar[(SYS_ADPT_SFLOW_MAX_RECEIVER_DATAGRAM_SIZE + SFLOW_OM_DATA_PAD) / sizeof(UI32_T)];
    UI32_T *data_p;
    UI32_T data_len;
} SFLOW_OM_Datagram_T;

typedef struct
{
    SFLOW_OM_Datagram_T datagram;
    L_INET_AddrIp_T     address;
    UI32_T              receiver_index; /* index */
    UI32_T              udp_port;
    UI32_T              timeout;
    char                owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1];
} SFLOW_OM_Receiver_T;

typedef struct
{
    UI32_T ifindex;                    /* index 1 */
    UI32_T instance_id;                /* index 2 */
    UI32_T max_header_size;
    UI32_T sampling_rate;
    UI32_T sample_sequence;
    UI32_T sample_pool;                /* total number of packets that have been sampled */
    UI32_T receiver_index;
    char   receiver_owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1];
}SFLOW_OM_Sampling_T;

typedef struct
{
    UI32_T ifindex;                    /* index 1 */
    UI32_T instance_id;                /* index 2 */
    UI32_T polling_interval;           /* sec */
    UI32_T last_polling_time;          /* sec */
    UI32_T sample_sequence;
    UI32_T receiver_index;
    char   receiver_owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1];
}SFLOW_OM_Polling_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_InitiateSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource.
 * INPUT    : None
 * OUTPUT   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void SFLOW_OM_InitiateSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_AttachSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for sFlow OM in the context of the
 *            calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void SFLOW_OM_AttachSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : Provide shared memory information for SYSRSC.
 * INPUT    : None
 * OUTPUT   : segid_p  -- shared memory segment id
 *            seglen_p -- length of the shared memroy segment
 * NOTES    : This function is called in SYSRSC_MGR_CreateSharedMemory().
 * ------------------------------------------------------------------------
 */
void
SFLOW_OM_GetShMemInfo(
    SYSRSC_MGR_SEGID_T *segid_p,
    UI32_T *seglen_p);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_Init
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Initialize sflow system config and port config
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
void SFLOW_OM_Init(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_InitReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Initial receiver entry.
 * INPUT    : receiver_index  -- receiver index
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void
SFLOW_OM_InitReceiverEntry(
    UI32_T receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetReceiverOwner
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver owner.
 * INPUT    : receiver_index  -- receiver index
 *            owner_name_p    -- owner name
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetReceiverOwner(
    UI32_T receiver_index,
    char *owner_name_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_GetReceiverOwner
 *-------------------------------------------------------------------------
 * PURPOSE  : Get receiver owner.
 * INPUT    : receiver_index  -- receiver index
 * OUTPUT   : owner_name_p    -- owner name
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_GetReceiverOwner(
    UI32_T receiver_index,
    char *owner_name_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetReceiverTimeout
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver timeout.
 * INPUT    : receiver_index  -- receiver index
 *            timeout         -- timeout value
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetReceiverTimeout(
    UI32_T receiver_index,
    UI32_T timeout);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetReceiverDestination
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver destination IP address.
 * INPUT    : receiver_index  -- receiver index
 *            address_p       -- destination IP address
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetReceiverDestination(
    UI32_T receiver_index,
    L_INET_AddrIp_T *address_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetReceiverSockPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver sock port.
 * INPUT    : receiver_index -- receiver index
 *            udp_port       -- udp port number
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetReceiverSockPort(
    UI32_T receiver_index,
    UI32_T udp_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetReceiverMaxDatagramSize
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver maximum datagram size.
 * INPUT    : receiver_index     -- receiver index
 *            max_datagram_size  -- maximum datagram size
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetReceiverMaxDatagramSize(
    UI32_T receiver_index,
    UI32_T max_datagram_size);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetReceiverDatagramVersion
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver datagram version.
 * INPUT    : receiver_index    -- receiver index
 *            datagram_version  -- datagram version
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetReceiverDatagramVersion(
    UI32_T receiver_index,
    UI32_T datagram_version);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetReceiverDatagramSequence
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver datagram sequence.
 * INPUT    : receiver_index     -- receiver index
 *            datagram_sequence  -- datagram sequence
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetReceiverDatagramSequence(
    UI32_T receiver_index,
    UI32_T datagram_sequence);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetReceiverDatagramSampleNumber
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver datagram sample number.
 * INPUT    : receiver_index  -- receiver index
 *            sample_number   -- datagram sample number
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetReceiverDatagramSampleNumber(
    UI32_T receiver_index,
    UI32_T sample_number);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetReceiverDatagramData
 *-------------------------------------------------------------------------
 * PURPOSE  : Set data to receiver datagram.
 * INPUT    : receiver_index -- receiver index
 *            data_p         -- data arrary pointer
 *            data_len       -- data length
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetReceiverDatagramData(
    UI32_T receiver_index,
    UI32_T *data_p,
    UI32_T data_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetReceiverSampleNumber
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver sample_number.
 * INPUT    : receiver_index  -- receiver index
 *            sample_number   -- sample_number
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetReceiverSampleNumber(
    UI32_T receiver_index,
    UI32_T sample_number);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_GetReceiverDataLength
 *-------------------------------------------------------------------------
 * PURPOSE  : Get receiver data length.
 * INPUT    : receiver_index  -- receiver index
 * OUTPUT   : data_len_p      -- data length
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_GetReceiverDataLength(
    UI32_T receiver_index,
    UI32_T *data_len_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_ClearReceiverDatagram
 *-------------------------------------------------------------------------
 * PURPOSE  : Clear receiver datagram data.
 * INPUT    : receiver_index  -- receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void
SFLOW_OM_ClearReceiverDatagram(
    UI32_T receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_GetReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_GetReceiverEntry(
    SFLOW_OM_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_InitSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Initial sampling entry.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void
SFLOW_OM_InitSamplingEntry(
    UI32_T ifindex,
    UI32_T instance_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetSamplingRate
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling rate.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 *            rate         -- sampling rate
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetSamplingRate(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T rate);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetSamplingMaxHeaderSize
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling maximum header size.
 * INPUT    : ifindex          -- interface index
 *            instance_id      -- instance id
 *            max_header_size  -- sampling maximum header size
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetSamplingMaxHeaderSize(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T max_header_size);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetSamplingReceiverIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling receiver index.
 * INPUT    : ifindex         -- interface index
 *            instance_id     -- instance id
 *            receiver_index  -- sampling receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetSamplingReceiverIndex(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetSamplingReceiverOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling receiver owner name.
 * INPUT    : ifindex                -- interface index
 *            instance_id            -- instance id
 *            receiver_owner_name_p  -- sampling receiver owner name
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetSamplingReceiverOwnerName(
    UI32_T ifindex,
    UI32_T instance_id,
    char *receiver_owner_name_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetSamplingSamplePool
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling sample pool.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 *            sample_pool  -- total number of packets that have been sampled
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetSamplingSamplePool(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T sample_pool);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetSamplingSampleSequence
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling sample sequence.
 * INPUT    : ifindex          -- interface index
 *            instance_id      -- instance id
 *            sample_sequence  -- sample sequence
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetSamplingSampleSequence(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T sample_sequence);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_GetSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_GetSamplingEntry(
    SFLOW_OM_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_InitPollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Initial polling entry.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void
SFLOW_OM_InitPollingEntry(
    UI32_T ifindex,
    UI32_T instance_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetPollingInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Set polling interval.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 *            interval     -- polling interval
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetPollingInterval(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetPollingReceiverIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Set polling receiver index.
 * INPUT    : ifindex         -- interface index
 *            instance_id     -- instance id
 *            receiver_index  -- polling receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetPollingReceiverIndex(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetPollingReceiverOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Set polling receiver owner name.
 * INPUT    : ifindex                -- interface index
 *            instance_id            -- instance id
 *            receiver_owner_name_p  -- polling receiver owner name
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetPollingReceiverOwnerName(
    UI32_T ifindex,
    UI32_T instance_id,
    char *receiver_owner_name_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetPollingSampleSequence
 *-------------------------------------------------------------------------
 * PURPOSE  : Set polling sample sequence.
 * INPUT    : ifindex          -- interface index
 *            instance_id      -- instance id
 *            sample_sequence  -- sample sequence
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetPollingSampleSequence(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T sample_sequence);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetPollingLastPollingTime
 *-------------------------------------------------------------------------
 * PURPOSE  : Set polling last polling time.
 * INPUT    : ifindex            -- interface index
 *            instance_id        -- instance id
 *            last_polling_time  -- last polling time
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetPollingLastPollingTime(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T last_polling_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_GetPollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_GetPollingEntry(
    SFLOW_OM_Polling_T *polling_entry_p);

#endif /* #if (SYS_CPNT_SFLOW == TRUE) */
#endif /* #ifndef _SFLOW_OM_H */

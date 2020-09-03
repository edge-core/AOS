/* -------------------------------------------------------------------------------------
 * FILE NAME: sflow_pmgr.h
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
 * Nelson Dai    09-09-2009    V1.0         First Created
 * Nelson Dai    12-03-2009    V1.1         Support RFC-3176 MIB
 * -------------------------------------------------------------------------------------
 * Copyright(C)                 Accton Technology Corp. 2009
 * -------------------------------------------------------------------------------------*/

#ifndef _SFLOW_PMGR_H
#define _SFLOW_PMGR_H

#include "sflow_mgr.h"
#include "sflow_om.h"

#if (SYS_CPNT_SFLOW == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_InitiateProcessResource
 *-------------------------------------------------------------------------
 * PURPOSE  : Initiate resource used in the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE  - Success,
 *            FALSE - Error
 * NOTES    : Before other CSC use SFLOW_PMGR, it should initiate the resource (get the message queue handler internally)
 * ------------------------------------------------------------------------
 */
BOOL_T SFLOW_PMGR_InitiateProcessResource(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SFLOW_PMGR_GetAgentAddress
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sFlow agent's address.
 * INPUT   : addr_p -- agent's address
 * OUTPUT  : None
 * RETURN  : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetAgentAddress(
    L_INET_AddrIp_T *addr_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SFLOW_PMGR_GetAgentAddressType
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sFlow agent's address type.
 * INPUT   : None
 * OUTPUT  : type  -- agent's address type
 * RETURN  : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetAgentAddressType(
    UI32_T *type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverOwner
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
SFLOW_PMGR_SetReceiverOwner(
    UI32_T receiver_index,
    char *owner_name_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverTimeout
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
SFLOW_PMGR_SetReceiverTimeout(
    UI32_T receiver_index,
    UI32_T timeout);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverDestination
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
SFLOW_PMGR_SetReceiverDestination(
    UI32_T receiver_index,
    L_INET_AddrIp_T *address_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverSockPort
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
SFLOW_PMGR_SetReceiverSockPort(
    UI32_T receiver_index,
    UI32_T udp_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverMaxDatagramSize
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
SFLOW_PMGR_SetReceiverMaxDatagramSize(
    UI32_T receiver_index,
    UI32_T max_datagram_size);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverDatagramVersion
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
SFLOW_PMGR_SetReceiverDatagramVersion(
    UI32_T receiver_index,
    UI32_T datagram_version);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_CreateReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create receiver entry or update receiver timeout.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : owner_name is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_CreateReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_DestroyReceiverEntryByOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy receiver entry by owner name.
 * INPUT    : owner_name_p  -- owner name
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_DestroyReceiverEntryByOwnerName(
    char *owner_name_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextActiveReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextActiveReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetActiveReceiverEntryByOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetActiveReceiverEntryByOwnerName(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetSamplingRate
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
SFLOW_PMGR_SetSamplingRate(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T rate);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetSamplingMaxHeaderSize
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
SFLOW_PMGR_SetSamplingMaxHeaderSize(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T max_header_size);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetSamplingReceiverIndex
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
SFLOW_PMGR_SetSamplingReceiverIndex(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_CreateSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_CreateSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_DestroySamplingEntry
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
SFLOW_PMGR_DestroySamplingEntry(
    UI32_T ifindex,
    UI32_T instance_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetActiveSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get active sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetActiveSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextActiveSamplingEntryByReveiverOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active sampling entry by receiver owner name.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_owner_name is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextActiveSamplingEntryByReveiverOwnerName(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextActiveSamplingEntryByDatasource
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active sampling entry by data source.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextActiveSamplingEntryByDatasource(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetPollingInterval
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
SFLOW_PMGR_SetPollingInterval(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetPollingReceiverIndex
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
SFLOW_PMGR_SetPollingReceiverIndex(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_CreatePollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_CreatePollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_DestroyPollingEntry
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
SFLOW_PMGR_DestroyPollingEntry(
    UI32_T ifindex,
    UI32_T instance_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetPollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetPollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetActivePollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get active polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetActivePollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextPollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextPollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextActivePollingEntryByReveiverOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active polling entry by receiver owner name.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_owner_name is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextActivePollingEntryByReveiverOwnerName(
    SFLOW_MGR_Polling_T *polling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextActivePollingEntryByDatasource
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active polling entry by data source.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextActivePollingEntryByDatasource(
    SFLOW_MGR_Polling_T *polling_entry_p);

#endif /* #if (SYS_CPNT_SFLOW == TRUE) */
#endif /* #ifndef _SFLOW_PMGR_H */

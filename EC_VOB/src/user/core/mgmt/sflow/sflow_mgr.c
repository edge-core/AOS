/* -------------------------------------------------------------------------------------
 * FILE NAME:  SFLOW_MGR.C
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
 * Copyright(C)                 Accton Technology Corp. 2009
 * -------------------------------------------------------------------------------------*/

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_time.h"
#include "sys_type.h"
#include "sys_module.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "leaf_es3626a.h"
#include "sflow_mgr.h"
#include "swctrl_pom.h"
#include "swctrl_pmgr.h"
#include "l_rstatus.h"
#include "l_stdlib.h"
#include "if_pmgr.h"
#include "swdrv.h"
#include "nmtr_pmgr.h"
#include "netcfg_pmgr_route.h"
#include "netcfg_pom_ip.h"
#include "ip_lib.h"
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include "leaf_sflowv5.h"
#include "backdoor_mgr.h"
#include "sflow_backdoor.h"
#include "sflow_om.h"
#include "lan.h"
#include "swctrl_pom.h"

#if (SYS_CPNT_SFLOW == TRUE)

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */
#define SFLOW_MGR_CHECK_OPERATING_MODE()                                              \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)             \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_POINTER_NOT_NULL(pointer)                                     \
    if (NULL == pointer)                                                              \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_RECEIVER_INDEX_RANGE(receiver_index)                          \
    if (receiver_index < SFLOW_MGR_MIN_RECEIVER_INDEX  ||                             \
        receiver_index > SFLOW_MGR_MAX_RECEIVER_INDEX)                                \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_RECEIVER_OWNER_NAME_LENGTH_RANGE(length)                      \
    if (length < SFLOW_MGR_MIN_RECEIVER_OWNER_STR_LEN  ||                             \
        length > SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN)                           \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_RECEIVER_TIMEOUT_RANGE(timeout)                               \
    if (timeout < SYS_ADPT_SFLOW_MIN_RECEIVER_TIMEOUT  ||                             \
        timeout > SYS_ADPT_SFLOW_MAX_RECEIVER_TIMEOUT)                                \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_RECEIVER_DESTINATION(address_p)                               \
    if (TRUE != SFLOW_MGR_CheckDestinationIsValid(address_p))                         \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_RECEIVER_SOCK_PORT_RANGE(upd_port)                            \
    if (upd_port < SFLOW_MGR_MIN_RECEIVER_SOCK_PORT  ||                               \
        upd_port > SFLOW_MGR_MAX_RECEIVER_SOCK_PORT)                                  \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_RECEIVER_MAX_DATAGRAM_SIZE_RANGE(max_datagram_size)           \
    if (max_datagram_size < SYS_ADPT_SFLOW_MIN_RECEIVER_DATAGRAM_SIZE  ||             \
        max_datagram_size > SYS_ADPT_SFLOW_MAX_RECEIVER_DATAGRAM_SIZE)                \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_RECEIVER_DATAGRAM_VERSION_RANGE(datagram_version)             \
    if (datagram_version < SFLOW_MGR_MIN_RECEIVER_DATAGRAM_VERSION  ||                \
        datagram_version > SFLOW_MGR_MAX_RECEIVER_DATAGRAM_VERSION)                   \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_RECEIVER_IS_CLAIMED(receiver_index)                           \
    if (TRUE != SFLOW_MGR_CheckReceiverIsClaimed(receiver_index))                     \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(ifindex, instance_id)                 \
    if (TRUE != SFLOW_MGR_CheckDataSourceAndInstanceIsValid(ifindex, instance_id))    \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_SAMPLING_MAX_HEADER_SIZE_RANGE(max_header_size)               \
    if (max_header_size < SYS_ADPT_SFLOW_MIN_SAMPLING_HEADER_SIZE  ||                 \
        max_header_size > SYS_ADPT_SFLOW_MAX_SAMPLING_HEADER_SIZE)                    \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_SAMPLING_RATE_RANGE(rate)                                     \
    if (rate < SYS_ADPT_SFLOW_MIN_SAMPLING_RATE  ||                                   \
        rate > SYS_ADPT_SFLOW_MAX_SAMPLING_RATE)                                      \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

#define SFLOW_MGR_CHECK_POLLING_INTERVAL_RANGE(interval)                              \
    if (interval < SYS_ADPT_SFLOW_MIN_POLLING_INTERVAL  ||                            \
        interval > SYS_ADPT_SFLOW_MAX_POLLING_INTERVAL)                               \
    {                                                                                 \
        return SFLOW_MGR_RETURN_FAIL;                                                 \
    }

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckReceiverIsClaimed
 *-------------------------------------------------------------------------
 * PURPOSE  : Check the receiver is claimed or not
 * INPUT    : receiver_index -- receiver index
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckReceiverIsClaimed(
    UI32_T receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckDestinationIsValid
 *-------------------------------------------------------------------------
 * PURPOSE  : check destination IP address is valid or not.
 * INPUT    : address_p -- destination IP address
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckDestinationIsValid(
    L_INET_AddrIp_T *address_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckDataSourceAndInstanceIdIsValid
 *-------------------------------------------------------------------------
 * PURPOSE  : check data source and instance id is valid or not.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckDataSourceAndInstanceIsValid(
    UI32_T ifindex,
    UI32_T instance_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckReceiverEntryIsActive
 *-------------------------------------------------------------------------
 * PURPOSE  : Check receiver entry is active or not.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckReceiverEntryIsActive(
    SFLOW_MGR_Receiver_T *receiver_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckSamplingEntryIsActive
 *-------------------------------------------------------------------------
 * PURPOSE  : Check samplingr entry is active or not.
 * INPUT    : sampling_entry_p  -- samplingr entry
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckSamplingEntryIsActive(
    SFLOW_MGR_Sampling_T *sampling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckPollingEntryIsActive
 *-------------------------------------------------------------------------
 * PURPOSE  : Check pollingr entry is active or not.
 * INPUT    : polling_entry_p  -- pollingr entry
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckPollingEntryIsActive(
    SFLOW_MGR_Polling_T *polling_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalDestroyReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy receiver entry.
 * INPUT    : receiver_index -- receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalDestroyReceiverEntry(
    UI32_T receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalGetNextDataSourceAndInstanceId
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next ifindex and instance_id.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalGetNextDataSourceAndInstanceId(
    UI32_T *ifindex_p,
    UI32_T *instance_id_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalProcessSampling
 *-------------------------------------------------------------------------
 * PURPOSE  : Process sampling.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_LocalProcessSampling(
    UI32_T ifindex,
    UI32_T instance_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalDestroySamplingEntryByReceiverIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy sampling entry by receiver index.
 * INPUT    : receiver_index  -- receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalDestroySamplingEntryByReceiverIndex(
    UI32_T  receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalDestroyPollingEntryByReceiverIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy polling entry by receiver index.
 * INPUT    : receiver_index  -- receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalDestroyPollingEntryByReceiverIndex(
    UI32_T  receiver_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalComputeDatagramHeaderSize
 *-------------------------------------------------------------------------
 * PURPOSE  : compute the datagram header size.
 * INPUT    : datagram_version  -- datagram version
 * OUTPUT   : header_size_p     -- datagram header size
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalComputeDatagramHeaderSize(
    UI32_T datagram_version,
    UI32_T *header_size_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalComputePacketSampleSize
 *-------------------------------------------------------------------------
 * PURPOSE  : Compute counter sample packet size.
 * INPUT    : datagram_version        -- datagram version
 * INPUT    : header_entity_length    -- header entity UI8 length
 * OUTPUT   : header_length_p         -- header entity UI32 length
 *            header_record_length_p  -- header record data length
 *            sample_packet_size_p    -- packet sample packet size
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalComputePacketSampleSize(
    UI32_T datagram_version,
    UI32_T header_entity_length,
    UI32_T *header_length_p,
    UI32_T *header_record_length_p,
    UI32_T *sample_packet_size_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalComputeCounterSampleSize
 *-------------------------------------------------------------------------
 * PURPOSE  : Compute counter sample packet size.
 * INPUT    : datagram_version     -- datagram version
 * OUTPUT   : sample_packet_size_p -- counter sample packet size
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalComputeCounterSampleSize(
    UI32_T datagram_version,
    UI32_T *sample_packet_size_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalWritePacketSampleToReceiverDatagram
 *-------------------------------------------------------------------------
 * PURPOSE  : Write packet sample to receiver datagram.
 * INPUT    : ifindex               -- interface index
 *            instance_id           -- instance id
 *            header_protocol       -- heaer protocol type
 *            frame_length          -- original length of packet before sampling
 *            stripped              -- number of byte removed from the packet
 *            header_entity_length  -- header entity UI8 length
 *            header_entity_p       -- header entity
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalWritePacketSampleToReceiverDatagram(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T header_protocol,
    UI32_T frame_length,
    UI32_T stripped,
    UI32_T header_entity_length,
    UI8_T *header_entity_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalWriteCounterSampleToReceiverDatagram
 *-------------------------------------------------------------------------
 * PURPOSE  : Write counter sample to receiver datagram.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalWriteCounterSampleToReceiverDatagram(
    UI32_T ifindex,
    UI32_T instance_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalGetIfCounterData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get interface generic counter data.
 * INPUT    : ifindex       -- interface index
 * OUTPUT   : if_counter_p  -- interface counter entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalGetIfCounterData(
    UI32_T ifindex,
    SFLOW_MGR_IfCounters_T *if_counter_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalEncodeIfCounterData
 *-------------------------------------------------------------------------
 * PURPOSE  : Encode interface generic counter data.
 * INPUT    : if_counter_p  -- interface counter entry
 * OUTPUT   : if_counter_p  -- interface counter entry
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void
SFLOW_MGR_LocalEncodeIfCounterData(
    SFLOW_MGR_IfCounters_T *if_counter_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalGetEthernetCounterData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get ethernet counter data.
 * INPUT    : ifindex             -- interface index
 * OUTPUT   : ethernet_counter_p  -- ethernet counter entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalGetEthernetCounterData(
    UI32_T ifindex,
    SFLOW_MGR_EthernetCounters_T *ethernet_counter_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalEncodeEthernetCounterData
 *-------------------------------------------------------------------------
 * PURPOSE  : Encode ethernet counter data.
 * INPUT    : ethernet_counter_p  -- ethernet counter entry
 * OUTPUT   : ethernet_counter_p  -- ethernet counter entry
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void
SFLOW_MGR_LocalEncodeEthernetCounterData(
    SFLOW_MGR_EthernetCounters_T *ethernet_counter_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalSendReceiverDatagram
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy receiver entry.
 * INPUT    : receiver_index -- receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalSendReceiverDatagram(
    UI32_T receiver_index);

/*  declare variables used for Transition mode
 */
SYSFUN_DECLARE_CSC

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
void SFLOW_MGR_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("sflow",
        SYS_BLD_SFLOW_GROUP_IPCMSGQ_KEY, SFLOW_BACKDOOR_CallBack);
}/* End of SFLOW_MGR_Create_InterCSC_Relation() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It is used to enable the sflow operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_MGR_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE();
    SFLOW_OM_Init();
    return;
} /* End of SFLOW_MGR_EnterMasterMode() */

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
void SFLOW_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    return;
} /* End of SFLOW_MGR_EnterTransitionMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Set SFLOW op_state to "SYS_TYPE_STACKING_SLAVE_MODE"
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void SFLOW_MGR_EnterSlaveMode(void)
{
    /* set mgr in slave mode */
    SYSFUN_ENTER_SLAVE_MODE();
    return;
} /* End of SFLOW_MGR_EnterSlaveMode() */

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
void SFLOW_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
    return;
} /* End of SFLOW_MGR_SetTransitionMode() */

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
SYS_TYPE_Stacking_Mode_T SFLOW_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
} /* End of SFLOW_MGR_GetOperationMode() */

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
    L_INET_AddrIp_T *addr_p)
{
    NETCFG_TYPE_InetRifConfig_T rifNode;

    memset(&rifNode, 0, sizeof(rifNode));
    /* Try to get IPv4 address first.
     */
    if (NETCFG_POM_IP_GetNextRifConfig(&rifNode) != NETCFG_TYPE_OK)
    {
        /* If failed, try to get IPv6 address.
         */
        memset(&rifNode, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
        rifNode.addr.type = L_INET_ADDR_TYPE_IPV6;

        if (NETCFG_POM_IP_GetNextIPv6RifConfig(&rifNode) != NETCFG_TYPE_OK)
        {
#if (SYS_CPNT_CRAFT_PORT == TRUE)
            /* If failed, means the connection is from craft port. But craft port no support sflow currently
             */
            NETCFG_TYPE_CraftInetAddress_T craft_addr;

            memset(&craft_addr, 0, sizeof(craft_addr));
            if(NETCFG_POM_IP_GetCraftInterfaceInetAddress(&craft_addr) == NETCFG_TYPE_OK)
            {
                memcpy(&(rifNode.addr), &(craft_addr.addr), sizeof(rifNode.addr));
            }
            else
            {
                memset(&craft_addr, 0, sizeof(craft_addr));
                craft_addr.addr.type = L_INET_ADDR_TYPE_IPV6;

                if (NETCFG_POM_IP_GetCraftInterfaceInetAddress(&craft_addr) == NETCFG_TYPE_OK)
                {
                    memcpy(&(rifNode.addr), &(craft_addr.addr), sizeof(rifNode.addr));
                }
                else
                {
                    return SFLOW_MGR_RETURN_FAIL;
                }
            }
#else
            return SFLOW_MGR_RETURN_FAIL;
#endif /* SYS_CPNT_CRAFT_PORT */
        }
    }

    memcpy(addr_p, &(rifNode.addr), sizeof(L_INET_AddrIp_T));
    return SFLOW_MGR_RETURN_SUCCESS;
}

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
    UI32_T *type)
{
    L_INET_AddrIp_T addr;
    memset(&addr, 0, sizeof(addr));

    if (SFLOW_MGR_GetAgentAddress(&addr) != SFLOW_MGR_RETURN_SUCCESS)
        return SFLOW_MGR_RETURN_FAIL;

    if (addr.type == L_INET_ADDR_TYPE_IPV4)
    {
        *type = VAL_sFlowAgentAddressType_ipv4;
    }
    else if (addr.type == L_INET_ADDR_TYPE_IPV4Z)
    {
        *type = VAL_sFlowAgentAddressType_ipv4z;
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if (addr.type == L_INET_ADDR_TYPE_IPV6)
    {
        *type = VAL_sFlowAgentAddressType_ipv6;
    }
    else if (addr.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        *type = VAL_sFlowAgentAddressType_ipv6z;
    }
#endif
    else
    {
        *type = VAL_sFlowAgentAddressType_unknown;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
}

/***************************************************************************/
/* sFlow config */
/***************************************************************************/

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
    char *owner_name_p)
{
    SFLOW_MGR_Receiver_T  receiver_entry;
    char  ori_owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1] = {0};

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_RECEIVER_INDEX_RANGE(receiver_index);

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(owner_name_p);

    if (SFLOW_OM_RETURN_SUCCESS !=
        SFLOW_OM_GetReceiverOwner(receiver_index, ori_owner_name))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if ('\0' != ori_owner_name[0])
    {
        if ('\0' != owner_name_p[0])
        {
            /* when claimed, can't change to other name
             */
            if (0 != strcmp(ori_owner_name, owner_name_p))
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }
        else
        {
            /* destroy receiver entry if it is active
             */
            SFLOW_MGR_LocalDestroyReceiverEntry(receiver_index);
            owner_name_p[0] = '\0';

            if (SFLOW_OM_RETURN_SUCCESS !=
                SFLOW_OM_SetReceiverOwner(receiver_index, owner_name_p))
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }
    }
    else
    {
        if ('\0' != owner_name_p[0])
        {
            SFLOW_MGR_CHECK_RECEIVER_OWNER_NAME_LENGTH_RANGE(strlen(owner_name_p));

            /* check owner name not be used
             */
            memset(&receiver_entry, 0, sizeof(receiver_entry));
            strncpy(receiver_entry.owner_name, owner_name_p,
                sizeof(receiver_entry.owner_name)-1);
            receiver_entry.owner_name[sizeof(receiver_entry.owner_name)-1] = '\0';

            if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetActiveReceiverEntryByOwnerName(&receiver_entry))
            {
                return SFLOW_MGR_RETURN_FAIL;
            }

            if (SFLOW_OM_RETURN_SUCCESS !=
                SFLOW_OM_SetReceiverOwner(receiver_index, owner_name_p))
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_SetReceiverOwner */

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
    UI32_T timeout)
{
    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_RECEIVER_INDEX_RANGE(receiver_index);

    SFLOW_MGR_CHECK_RECEIVER_IS_CLAIMED(receiver_index);

    if (SFLOW_MGR_RECEIVER_TIMEOUT_DISABLE == timeout)
    {
        /* destroy receiver entry if it is active
         */
        SFLOW_MGR_LocalDestroyReceiverEntry(receiver_index);
    }
    else
    {
        SFLOW_MGR_CHECK_RECEIVER_TIMEOUT_RANGE(timeout);
    }

    if (SFLOW_OM_RETURN_FAIL == SFLOW_OM_SetReceiverTimeout(receiver_index, timeout))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_SetReceiverTimeout */

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
    L_INET_AddrIp_T *address_p)
{
    SFLOW_MGR_Receiver_T receiver_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_RECEIVER_INDEX_RANGE(receiver_index);

    SFLOW_MGR_CHECK_RECEIVER_IS_CLAIMED(receiver_index);

    SFLOW_MGR_CHECK_RECEIVER_DESTINATION(address_p);

    /* when active not allow to change value
     */
    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.receiver_index = receiver_index;

    if (SFLOW_MGR_RETURN_SUCCESS ==
        SFLOW_MGR_GetActiveReceiverEntry(&receiver_entry))
    {
        if (0 == L_INET_CompareInetAddr((L_INET_Addr_T *)address_p,
            (L_INET_Addr_T *)&receiver_entry.address, 0))
        {
            return SFLOW_MGR_RETURN_SUCCESS;
        }
        else
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    if (SFLOW_OM_RETURN_SUCCESS !=
        SFLOW_OM_SetReceiverDestination(receiver_index, address_p))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_SetReceiverDestination */

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
    UI32_T udp_port)
{
    SFLOW_MGR_Receiver_T receiver_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_RECEIVER_INDEX_RANGE(receiver_index);

    SFLOW_MGR_CHECK_RECEIVER_IS_CLAIMED(receiver_index);

    SFLOW_MGR_CHECK_RECEIVER_SOCK_PORT_RANGE(udp_port);

    /* when active not allow to change value
     */
    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.receiver_index = receiver_index;

    if (SFLOW_MGR_RETURN_SUCCESS ==
        SFLOW_MGR_GetActiveReceiverEntry(&receiver_entry))
    {
        if (udp_port == receiver_entry.udp_port)
        {
            return SFLOW_MGR_RETURN_SUCCESS;
        }
        else
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    if (SFLOW_OM_RETURN_SUCCESS !=
        SFLOW_OM_SetReceiverSockPort(receiver_index, udp_port))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_SetReceiverSockPort */

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
    UI32_T max_datagram_size)
{
    SFLOW_MGR_Receiver_T receiver_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_RECEIVER_INDEX_RANGE(receiver_index);

    SFLOW_MGR_CHECK_RECEIVER_IS_CLAIMED(receiver_index);

    SFLOW_MGR_CHECK_RECEIVER_MAX_DATAGRAM_SIZE_RANGE(max_datagram_size);

    /* when active not allow to change value
     */
    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.receiver_index = receiver_index;

    if (SFLOW_MGR_RETURN_SUCCESS ==
        SFLOW_MGR_GetActiveReceiverEntry(&receiver_entry))
    {
        if (max_datagram_size == receiver_entry.max_datagram_size)
        {
            return SFLOW_MGR_RETURN_SUCCESS;
        }
        else
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    if (SFLOW_OM_RETURN_SUCCESS !=
        SFLOW_OM_SetReceiverMaxDatagramSize(receiver_index, max_datagram_size))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_SetReceiverMaxDatagramSize */

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
    UI32_T datagram_version)
{
    SFLOW_MGR_Receiver_T receiver_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_RECEIVER_INDEX_RANGE(receiver_index);

    SFLOW_MGR_CHECK_RECEIVER_IS_CLAIMED(receiver_index);

    SFLOW_MGR_CHECK_RECEIVER_DATAGRAM_VERSION_RANGE(datagram_version);

    /* when active not allow to change value
     */
    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.receiver_index = receiver_index;

    if (SFLOW_MGR_RETURN_SUCCESS ==
        SFLOW_MGR_GetActiveReceiverEntry(&receiver_entry))
    {
        if (datagram_version == receiver_entry.datagram_version)
        {
            return SFLOW_MGR_RETURN_SUCCESS;
        }
        else
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    if (SFLOW_OM_RETURN_SUCCESS !=
        SFLOW_OM_SetReceiverDatagramVersion(receiver_index, datagram_version))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_SetReceiverDatagramVersion */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CreateReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create receiver entry or update receiver timeout.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : 1. owner_name is index
 *            2. If owner is active, only allow to update timeout.
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_CreateReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    SFLOW_MGR_Receiver_T  receiver_entry;
    UI32_T  i = 0;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(receiver_entry_p);

    SFLOW_MGR_CHECK_RECEIVER_OWNER_NAME_LENGTH_RANGE(strlen(receiver_entry_p->owner_name));

    SFLOW_MGR_CHECK_RECEIVER_TIMEOUT_RANGE(receiver_entry_p->timeout);

    if (0 != receiver_entry_p->address.addrlen)
    {
        SFLOW_MGR_CHECK_RECEIVER_DESTINATION(&receiver_entry_p->address);
    }

    if (0 != receiver_entry_p->udp_port)
    {
        SFLOW_MGR_CHECK_RECEIVER_SOCK_PORT_RANGE(receiver_entry_p->udp_port);
    }

    if (0 != receiver_entry_p->max_datagram_size)
    {
        SFLOW_MGR_CHECK_RECEIVER_MAX_DATAGRAM_SIZE_RANGE(receiver_entry_p->max_datagram_size);
    }

    if (0 != receiver_entry_p->datagram_version)
    {
        SFLOW_MGR_CHECK_RECEIVER_DATAGRAM_VERSION_RANGE(receiver_entry_p->datagram_version);
    }

    memset(&receiver_entry, 0, sizeof(receiver_entry));
    strncpy(receiver_entry.owner_name,
        receiver_entry_p->owner_name,
        sizeof(receiver_entry.owner_name)-1);
    receiver_entry.owner_name[sizeof(receiver_entry.owner_name)-1] = '\0';

    /* if owner is active, only allow to update timeout
     */
    if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetActiveReceiverEntryByOwnerName(&receiver_entry))
    {
        if (0 != receiver_entry_p->max_datagram_size)
        {
            if (receiver_entry_p->max_datagram_size != receiver_entry.max_datagram_size)
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }

        if (0 != receiver_entry_p->datagram_version)
        {
            if (receiver_entry_p->datagram_version != receiver_entry.datagram_version)
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }

        if (SFLOW_OM_RETURN_FAIL == SFLOW_OM_SetReceiverTimeout(
            receiver_entry.receiver_index, receiver_entry_p->timeout))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (0 != receiver_entry_p->address.addrlen)
        {
            if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverDestination(
                   receiver_entry.receiver_index, &receiver_entry_p->address))
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }

        if (0 != receiver_entry_p->udp_port)
        {
            if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverSockPort(
                receiver_entry.receiver_index, receiver_entry_p->udp_port))
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }
    }
    else
    {
        if (SFLOW_MGR_RECEIVER_TIMEOUT_DISABLE == receiver_entry_p->timeout)
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (0 == receiver_entry_p->address.addrlen)
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        /* get unclaimed receiver index
         */
        for (i = SFLOW_MGR_MIN_RECEIVER_INDEX; i <= SFLOW_MGR_MAX_RECEIVER_INDEX; ++i)
        {
            if (TRUE != SFLOW_MGR_CheckReceiverIsClaimed(i))
            {
                receiver_entry_p->receiver_index = i;
                break;
            }
        }

        if (0 == receiver_entry_p->receiver_index)
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverOwner(
            receiver_entry_p->receiver_index, receiver_entry_p->owner_name))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_FAIL == SFLOW_OM_SetReceiverTimeout(
            receiver_entry_p->receiver_index, receiver_entry_p->timeout))
        {
          return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverDestination(
            receiver_entry_p->receiver_index, &receiver_entry_p->address))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (0 != receiver_entry_p->udp_port)
        {
            if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverSockPort(
                receiver_entry_p->receiver_index, receiver_entry_p->udp_port))
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }

        if (0 != receiver_entry_p->max_datagram_size)
        {
            if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverMaxDatagramSize(
                receiver_entry_p->receiver_index, receiver_entry_p->max_datagram_size))
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }

        if (0 != receiver_entry_p->datagram_version)
        {
            if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverDatagramVersion(
                receiver_entry_p->receiver_index, receiver_entry_p->datagram_version))
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }
    }

   return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_CreateReceiverEntry */

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
    char *owner_name_p)
{
    SFLOW_MGR_Receiver_T  receiver_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(owner_name_p);

    memset(&receiver_entry, 0, sizeof(receiver_entry));
    strncpy(receiver_entry.owner_name, owner_name_p,
        sizeof(receiver_entry.owner_name)-1);
    receiver_entry.owner_name[sizeof(receiver_entry.owner_name)-1] = '\0';

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetActiveReceiverEntryByOwnerName(&receiver_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_LocalDestroyReceiverEntry(receiver_entry.receiver_index))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_DestroyReceiverEntryByOwnerName */

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
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    SFLOW_OM_Receiver_T  receiver_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(receiver_entry_p);

    SFLOW_MGR_CHECK_RECEIVER_INDEX_RANGE(receiver_entry_p->receiver_index);

    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.receiver_index = receiver_entry_p->receiver_index;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetReceiverEntry(&receiver_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    strncpy(receiver_entry_p->owner_name,
        receiver_entry.owner_name,
        sizeof(receiver_entry_p->owner_name));
    receiver_entry_p->owner_name[sizeof(receiver_entry_p->owner_name)-1] = '\0';
    memcpy(&receiver_entry_p->address,
        &receiver_entry.address,
        sizeof(receiver_entry_p->address));
    receiver_entry_p->timeout = receiver_entry.timeout;
    receiver_entry_p->udp_port = receiver_entry.udp_port;
    receiver_entry_p->max_datagram_size = receiver_entry.datagram.max_size;
    receiver_entry_p->datagram_version = receiver_entry.datagram.version;

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_GetReceiverEntry */

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
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(receiver_entry_p);

    ++receiver_entry_p->receiver_index;

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetReceiverEntry(receiver_entry_p))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_GetNextReceiverEntry */

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
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(receiver_entry_p);

    if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetReceiverEntry(receiver_entry_p))
    {
        if (TRUE == SFLOW_MGR_CheckReceiverEntryIsActive(receiver_entry_p))
        {
            return SFLOW_MGR_RETURN_SUCCESS;
        }
    }

    return SFLOW_MGR_RETURN_FAIL;
} /* End of SFLOW_MGR_GetActiveReceiverEntry */

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
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    SFLOW_MGR_Receiver_T  receiver_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(receiver_entry_p);

    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.receiver_index = receiver_entry_p->receiver_index;

    while (SFLOW_MGR_RETURN_SUCCESS ==
        SFLOW_MGR_GetNextReceiverEntry(&receiver_entry))
    {
        if (TRUE == SFLOW_MGR_CheckReceiverEntryIsActive(&receiver_entry))
        {
            memcpy(receiver_entry_p, &receiver_entry, sizeof(SFLOW_MGR_Receiver_T));
            return SFLOW_MGR_RETURN_SUCCESS;
        }
    }

    return SFLOW_MGR_RETURN_FAIL;
} /* End of SFLOW_MGR_GetNextActiveReceiverEntry */

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
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    SFLOW_MGR_Receiver_T  receiver_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(receiver_entry_p);

    memset(&receiver_entry, 0, sizeof(receiver_entry));

    while (SFLOW_MGR_RETURN_SUCCESS ==
        SFLOW_MGR_GetNextActiveReceiverEntry(&receiver_entry))
    {
        if (0 == strcmp(receiver_entry_p->owner_name, receiver_entry.owner_name))
        {
            memcpy(receiver_entry_p, &receiver_entry, sizeof(SFLOW_MGR_Receiver_T));
            return SFLOW_MGR_RETURN_SUCCESS;
        }
    }

    return SFLOW_MGR_RETURN_FAIL;
} /* End of SFLOW_MGR_GetActiveReceiverEntryByOwnerName */

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
    UI32_T rate)
{
    SFLOW_MGR_Sampling_T sampling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(ifindex, instance_id);

    if (SFLOW_MGR_SAMPLING_RATE_DISABLE == rate)
    {
        /* destroy sampling entry if it is active
         */
        SFLOW_MGR_DestroySamplingEntry(ifindex, instance_id);

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetSamplingRate(
            ifindex, instance_id, SFLOW_MGR_SAMPLING_RATE_DISABLE))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }
    else
    {
        SFLOW_MGR_CHECK_SAMPLING_RATE_RANGE(rate);

        /* when active not allow to change value
         */
        memset(&sampling_entry, 0, sizeof(sampling_entry));
        sampling_entry.ifindex = ifindex;
        sampling_entry.instance_id = instance_id;

        if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetActiveSamplingEntry(&sampling_entry))
        {
            if (rate == sampling_entry.sampling_rate)
            {
                return SFLOW_MGR_RETURN_SUCCESS;
            }
            else
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetSamplingRate(
            ifindex, instance_id, rate))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        SFLOW_MGR_LocalProcessSampling(ifindex, instance_id);
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_SetSamplingRate */

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
    UI32_T max_header_size)
{
    SFLOW_MGR_Sampling_T sampling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(ifindex, instance_id);

    SFLOW_MGR_CHECK_SAMPLING_MAX_HEADER_SIZE_RANGE(max_header_size);

    /* when active not allow to change value
     */
    memset(&sampling_entry, 0, sizeof(sampling_entry));
    sampling_entry.ifindex = ifindex;
    sampling_entry.instance_id = instance_id;

    if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetActiveSamplingEntry(&sampling_entry))
    {
        if (max_header_size == sampling_entry.max_header_size)
        {
            return SFLOW_MGR_RETURN_SUCCESS;
        }
        else
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    if (SFLOW_OM_RETURN_SUCCESS !=
        SFLOW_OM_SetSamplingMaxHeaderSize(ifindex, instance_id, max_header_size))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_SetSamplingMaxHeaderSize */

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
    UI32_T receiver_index)
{
    SFLOW_MGR_Sampling_T sampling_entry;
    SFLOW_MGR_Receiver_T receiver_entry;
    char  owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1] = {0};

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(ifindex, instance_id);

    if (0 == receiver_index)
    {
        /* destroy sampling entry if it is active
         */
        if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_DestroySamplingEntry(ifindex, instance_id))
        {
            return SFLOW_MGR_RETURN_SUCCESS;
        }

        /* sampling entry is inactive, reset receiver index and receiver owner name
         */
        if (SFLOW_OM_RETURN_SUCCESS !=
            SFLOW_OM_SetSamplingReceiverIndex(ifindex, instance_id, receiver_index))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetSamplingReceiverOwnerName(
            ifindex, instance_id, owner_name))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }
    else
    {
        SFLOW_MGR_CHECK_RECEIVER_INDEX_RANGE(receiver_index);

        /* when active not allow to change value
         */
        memset(&sampling_entry, 0, sizeof(sampling_entry));
        sampling_entry.ifindex = ifindex;
        sampling_entry.instance_id = instance_id;

        if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetActiveSamplingEntry(&sampling_entry))
        {
            if (receiver_index == sampling_entry.receiver_index)
            {
                return SFLOW_MGR_RETURN_SUCCESS;
            }
            else
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }

        /* check receiver is active
         */
        memset(&receiver_entry, 0, sizeof(receiver_entry));
        receiver_entry.receiver_index = receiver_index;

        if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetActiveReceiverEntry(&receiver_entry))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS !=
            SFLOW_OM_SetSamplingReceiverIndex(ifindex, instance_id, receiver_index))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetSamplingReceiverOwnerName(
            ifindex, instance_id, receiver_entry.owner_name))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        SFLOW_MGR_LocalProcessSampling(ifindex, instance_id);
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_SetSamplingReceiverIndex */

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
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    SFLOW_MGR_Sampling_T  sampling_entry;
    SFLOW_MGR_Receiver_T  receiver_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(sampling_entry_p);

    SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(sampling_entry_p->ifindex, sampling_entry_p->instance_id);

    SFLOW_MGR_CHECK_SAMPLING_RATE_RANGE(sampling_entry_p->sampling_rate);

    if (0 != sampling_entry_p->max_header_size)
    {
        SFLOW_MGR_CHECK_SAMPLING_MAX_HEADER_SIZE_RANGE(sampling_entry_p->max_header_size);
    }

    memset(&sampling_entry, 0, sizeof(sampling_entry));
    sampling_entry.ifindex = sampling_entry_p->ifindex;
    sampling_entry.instance_id = sampling_entry_p->instance_id;

    if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetActiveSamplingEntry(&sampling_entry))
    {
        /* when active, can't change value
         */
        if ((sampling_entry_p->sampling_rate != sampling_entry.sampling_rate) ||
           (0 != strcmp(sampling_entry_p->receiver_owner_name, sampling_entry.receiver_owner_name)))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (0 != sampling_entry_p->max_header_size)
        {
            if (sampling_entry_p->max_header_size != sampling_entry.max_header_size)
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }
    }
    else
    {
        /* check receiver is active
         */
        memset(&receiver_entry, 0, sizeof(receiver_entry));
        strncpy(receiver_entry.owner_name, sampling_entry_p->receiver_owner_name,
            sizeof(receiver_entry.owner_name)-1);
        receiver_entry.owner_name[sizeof(receiver_entry.owner_name)-1] = '\0';

        if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetActiveReceiverEntryByOwnerName(&receiver_entry))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (TRUE != SWCTRL_PMGR_SetSflowPortPacketSamplingRate(
            sampling_entry_p->ifindex, sampling_entry_p->sampling_rate))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetSamplingRate(sampling_entry_p->ifindex,
            sampling_entry_p->instance_id, sampling_entry_p->sampling_rate))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (0 != sampling_entry_p->max_header_size)
        {
            if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetSamplingMaxHeaderSize(sampling_entry_p->ifindex,
                sampling_entry_p->instance_id, sampling_entry_p->max_header_size))
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetSamplingReceiverIndex(sampling_entry_p->ifindex,
            sampling_entry_p->instance_id, receiver_entry.receiver_index))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetSamplingReceiverOwnerName(sampling_entry_p->ifindex,
            sampling_entry_p->instance_id, sampling_entry_p->receiver_owner_name))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_CreateSamplingEntry */

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
    UI32_T instance_id)
{
    SFLOW_MGR_Sampling_T  sampling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(ifindex, instance_id);

    memset(&sampling_entry, 0, sizeof(sampling_entry));
    sampling_entry.ifindex = ifindex;
    sampling_entry.instance_id = instance_id;

    if (SFLOW_MGR_RETURN_SUCCESS !=
        SFLOW_MGR_GetActiveSamplingEntry(&sampling_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (TRUE != SWCTRL_PMGR_SetSflowPortPacketSamplingRate(
        ifindex, SFLOW_MGR_RECEIVER_TIMEOUT_DISABLE))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    SFLOW_OM_InitSamplingEntry(ifindex, instance_id);

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_DestroySamplingEntry */

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
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    SFLOW_OM_Sampling_T  sampling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(sampling_entry_p);

    SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(sampling_entry_p->ifindex, sampling_entry_p->instance_id);

    memset(&sampling_entry, 0, sizeof(sampling_entry));
    sampling_entry.ifindex = sampling_entry_p->ifindex;
    sampling_entry.instance_id = sampling_entry_p->instance_id;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetSamplingEntry(&sampling_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    sampling_entry_p->max_header_size = sampling_entry.max_header_size;
    sampling_entry_p->sampling_rate = sampling_entry.sampling_rate;
    sampling_entry_p->receiver_index = sampling_entry.receiver_index;
    strncpy(sampling_entry_p->receiver_owner_name,
        sampling_entry.receiver_owner_name,
        sizeof(sampling_entry_p->receiver_owner_name)-1);
    sampling_entry_p->receiver_owner_name[sizeof(sampling_entry_p->receiver_owner_name)-1] = '\0';

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_GetSamplingEntry */

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
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetSamplingEntry(sampling_entry_p))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (TRUE != SFLOW_MGR_CheckSamplingEntryIsActive(sampling_entry_p))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_GetActiveSamplingEntry */

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
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    SFLOW_OM_Sampling_T  sampling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(sampling_entry_p);

    memset(&sampling_entry, 0, sizeof(sampling_entry));

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_LocalGetNextDataSourceAndInstanceId(
        &sampling_entry_p->ifindex,&sampling_entry_p->instance_id))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetSamplingEntry(sampling_entry_p))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_GetNextSamplingEntry */

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
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    SFLOW_MGR_Sampling_T sampling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(sampling_entry_p);

    memset(&sampling_entry, 0, sizeof(sampling_entry));
    sampling_entry.ifindex = sampling_entry_p->ifindex;
    sampling_entry.instance_id = sampling_entry_p->instance_id;

    while (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetNextSamplingEntry(&sampling_entry))
    {
        if (TRUE != SFLOW_MGR_CheckSamplingEntryIsActive(&sampling_entry))
        {
            continue;
        }

        if (0 == strcmp(sampling_entry.receiver_owner_name,
            sampling_entry_p->receiver_owner_name))
        {
            memcpy(sampling_entry_p, &sampling_entry, sizeof(SFLOW_MGR_Sampling_T));
            return SFLOW_MGR_RETURN_SUCCESS;
        }
    }

    return SFLOW_MGR_RETURN_FAIL;
} /* End of SFLOW_MGR_GetNextActiveSamplingEntryByReveiverOwnerName */

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
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    SFLOW_MGR_Sampling_T sampling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(sampling_entry_p);

    memset(&sampling_entry, 0, sizeof(sampling_entry));
    sampling_entry.ifindex = sampling_entry_p->ifindex;
    sampling_entry.instance_id = sampling_entry_p->instance_id;

    while (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetNextSamplingEntry(&sampling_entry))
    {
        if (TRUE != SFLOW_MGR_CheckSamplingEntryIsActive(&sampling_entry))
        {
            continue;
        }

        if (sampling_entry.ifindex != sampling_entry_p->ifindex)
        {
            break;
        }

        memcpy(sampling_entry_p, &sampling_entry, sizeof(SFLOW_MGR_Sampling_T));
        return SFLOW_MGR_RETURN_SUCCESS;
    }

    return SFLOW_MGR_RETURN_FAIL;
} /* End of SFLOW_MGR_GetNextActiveSamplingEntryByDatasource */

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
    UI32_T interval)
{
    SFLOW_MGR_Polling_T polling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(ifindex, instance_id);

    if (SFLOW_MGR_POLLING_INTERVAL_DISABLE == interval)
    {
        /* destroy polling entry if it is active
         */
        SFLOW_MGR_DestroyPollingEntry(ifindex, instance_id);

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetPollingInterval(
            ifindex, instance_id, SFLOW_MGR_POLLING_INTERVAL_DISABLE))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }
    else
    {
        SFLOW_MGR_CHECK_POLLING_INTERVAL_RANGE(interval);

        /* when active not allow to change value
         */
        memset(&polling_entry, 0, sizeof(polling_entry));
        polling_entry.ifindex = ifindex;
        polling_entry.instance_id = instance_id;

        if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetActivePollingEntry(&polling_entry))
        {
            if (interval == polling_entry.polling_interval)
            {
                return SFLOW_MGR_RETURN_SUCCESS;
            }
            else
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetPollingInterval(
            ifindex, instance_id, interval))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_SetPollingInterval */

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
    UI32_T receiver_index)
{
    SFLOW_MGR_Polling_T polling_entry;
    SFLOW_MGR_Receiver_T receiver_entry;
    char  owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1] = {0};

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(ifindex, instance_id);

    if (0 == receiver_index)
    {
        /* destroy polling entry if it is active
         */
        if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_DestroyPollingEntry(ifindex, instance_id))
        {
            return SFLOW_MGR_RETURN_SUCCESS;
        }

        /* polling entry is inactive, reset receiver index and receiver owner name
         */
        if (SFLOW_OM_RETURN_SUCCESS !=
            SFLOW_OM_SetPollingReceiverIndex(ifindex, instance_id, receiver_index))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetPollingReceiverOwnerName(
            ifindex, instance_id, owner_name))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }
    else
    {
        SFLOW_MGR_CHECK_RECEIVER_INDEX_RANGE(receiver_index);

        /* when active not allow to change value
         */
        memset(&polling_entry, 0, sizeof(polling_entry));
        polling_entry.ifindex = ifindex;
        polling_entry.instance_id = instance_id;

        if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetActivePollingEntry(&polling_entry))
        {
            if (receiver_index == polling_entry.receiver_index)
            {
                return SFLOW_MGR_RETURN_SUCCESS;
            }
            else
            {
                return SFLOW_MGR_RETURN_FAIL;
            }
        }

        /* check receiver is active
         */
        memset(&receiver_entry, 0, sizeof(receiver_entry));
        receiver_entry.receiver_index = receiver_index;

        if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetActiveReceiverEntry(&receiver_entry))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS !=
            SFLOW_OM_SetPollingReceiverIndex(ifindex, instance_id, receiver_index))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetPollingReceiverOwnerName(
            ifindex, instance_id, receiver_entry.owner_name))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_SetPollingReceiverIndex */

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
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    SFLOW_MGR_Polling_T  polling_entry;
    SFLOW_MGR_Receiver_T receiver_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(polling_entry_p);

    SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(polling_entry_p->ifindex, polling_entry_p->instance_id);

    SFLOW_MGR_CHECK_POLLING_INTERVAL_RANGE(polling_entry_p->polling_interval);

    memset(&polling_entry, 0, sizeof(polling_entry));
    polling_entry.ifindex = polling_entry_p->ifindex;
    polling_entry.instance_id = polling_entry_p->instance_id;

    if (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetActivePollingEntry(&polling_entry))
    {
        /* when active, can't change value
         */
        if ((polling_entry_p->polling_interval != polling_entry.polling_interval) ||
           (0 != strcmp(polling_entry_p->receiver_owner_name, polling_entry.receiver_owner_name)))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }
    else
    {
        /* check receiver is active
         */
        memset(&receiver_entry, 0, sizeof(receiver_entry));
        strncpy(receiver_entry.owner_name, polling_entry_p->receiver_owner_name,
            sizeof(receiver_entry.owner_name)-1);
        receiver_entry.owner_name[sizeof(receiver_entry.owner_name)-1] = '\0';

        if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetActiveReceiverEntryByOwnerName(&receiver_entry))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetPollingInterval(polling_entry_p->ifindex,
            polling_entry_p->instance_id, polling_entry_p->polling_interval))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetPollingReceiverIndex(polling_entry_p->ifindex,
            polling_entry_p->instance_id, receiver_entry.receiver_index))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetPollingReceiverOwnerName(polling_entry_p->ifindex,
            polling_entry_p->instance_id, polling_entry_p->receiver_owner_name))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_CreatePollingEntry */

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
    UI32_T instance_id)
{
    SFLOW_MGR_Polling_T  polling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(ifindex, instance_id);

    memset(&polling_entry, 0, sizeof(polling_entry));
    polling_entry.ifindex = ifindex;
    polling_entry.instance_id = instance_id;

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetActivePollingEntry(&polling_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    SFLOW_OM_InitPollingEntry(ifindex, instance_id);

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_DestroyPollingEntry */

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
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    SFLOW_OM_Polling_T  polling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(polling_entry_p);

    SFLOW_MGR_CHECK_DATASOURCE_AND_INSTANCE(polling_entry_p->ifindex, polling_entry_p->instance_id);

    memset(&polling_entry, 0, sizeof(polling_entry));
    polling_entry.ifindex = polling_entry_p->ifindex;
    polling_entry.instance_id = polling_entry_p->instance_id;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetPollingEntry(&polling_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    polling_entry_p->polling_interval = polling_entry.polling_interval;
    polling_entry_p->receiver_index = polling_entry.receiver_index;
    strncpy(polling_entry_p->receiver_owner_name,
        polling_entry.receiver_owner_name,
        sizeof(polling_entry_p->receiver_owner_name)-1);
    polling_entry_p->receiver_owner_name[sizeof(polling_entry_p->receiver_owner_name)-1] = '\0';

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_GetPollingEntry */

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
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetPollingEntry(polling_entry_p))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (TRUE != SFLOW_MGR_CheckPollingEntryIsActive(polling_entry_p))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_GetActivePollingEntry */

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
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    SFLOW_OM_Polling_T  polling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(polling_entry_p);

    memset(&polling_entry, 0, sizeof(polling_entry));

    if (SFLOW_MGR_RETURN_SUCCESS !=
        SFLOW_MGR_LocalGetNextDataSourceAndInstanceId(
            &polling_entry_p->ifindex,&polling_entry_p->instance_id))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetPollingEntry(polling_entry_p))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_GetNextPollingEntry */

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
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    SFLOW_MGR_Polling_T polling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(polling_entry_p);

    memset(&polling_entry, 0, sizeof(polling_entry));
    polling_entry.ifindex = polling_entry_p->ifindex;
    polling_entry.instance_id = polling_entry_p->instance_id;

    while (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetNextPollingEntry(&polling_entry))
    {
        if (TRUE != SFLOW_MGR_CheckPollingEntryIsActive(&polling_entry))
        {
            continue;
        }

        if (0 == strcmp(polling_entry.receiver_owner_name, polling_entry_p->receiver_owner_name))
        {
            memcpy(polling_entry_p, &polling_entry, sizeof(SFLOW_MGR_Polling_T));
            return SFLOW_MGR_RETURN_SUCCESS;
        }
    }

    return SFLOW_MGR_RETURN_FAIL;
} /* End of SFLOW_MGR_GetNextActivePollingEntryByReveiverOwnerName */

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
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    SFLOW_MGR_Polling_T  polling_entry;

    SFLOW_MGR_CHECK_OPERATING_MODE();

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(polling_entry_p);

    memset(&polling_entry, 0, sizeof(polling_entry));
    polling_entry.ifindex = polling_entry_p->ifindex;
    polling_entry.instance_id = polling_entry_p->instance_id;

    while (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetNextPollingEntry(&polling_entry))
    {
        if (TRUE != SFLOW_MGR_CheckPollingEntryIsActive(&polling_entry))
        {
            continue;
        }

        if (polling_entry.ifindex != polling_entry_p->ifindex)
        {
            break;
        }

        memcpy(polling_entry_p, &polling_entry, sizeof(SFLOW_MGR_Polling_T));
        return SFLOW_MGR_RETURN_SUCCESS;
    }

    return SFLOW_MGR_RETURN_FAIL;
} /* End of SFLOW_MGR_GetNextActivePollingEntryByDatasource */

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
    UI32_T src_port)
{
    SFLOW_OM_Sampling_T sampling_entry;
    UI32_T ifindex;
    UI32_T instance_id;
    UI32_T pdu_len;
    UI32_T header_protocol;
    UI32_T frame_length;
    UI32_T stripped;
    UI32_T header_entity_length;
    UI8_T  header_entity[SYS_ADPT_SFLOW_MAX_SAMPLING_HEADER_SIZE] = {0};
    UI8_T  *packet_p;

    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UserPortToIfindex(
        src_unit, src_port, &ifindex))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    instance_id = 1; /* a data source only support 1 instance id now */

    memset(&sampling_entry, 0, sizeof(sampling_entry));
    sampling_entry.ifindex = ifindex;
    sampling_entry.instance_id = instance_id;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetSamplingEntry(&sampling_entry))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* update sample pool
     */
    ++sampling_entry.sample_pool;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetSamplingSamplePool(
        ifindex, instance_id, sampling_entry.sample_pool))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if (NULL == packet_p)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    if (ether_type == 0x0800)
    {
        header_protocol = SFLOW_MGR_SAMPLE_HEADER_PROTOCOL_IPV4;
        frame_length = pkt_length;
        stripped = 0;
        header_entity_length = 0;
    }
    else if (ether_type == 0x86DD)
    {
        header_protocol = SFLOW_MGR_SAMPLE_HEADER_PROTOCOL_IPV6;
        frame_length = pkt_length;
        stripped = 0;
        header_entity_length = 0;
    }
    else
    {
        UI16_T ether_type_network;

        header_protocol = SFLOW_MGR_SAMPLE_HEADER_PROTOCOL_ETHERNET;
        frame_length = pkt_length + SFLOW_MGR_FCS_BYTES;
        stripped = SFLOW_MGR_FCS_BYTES;

        /* ethernet header was removed by lan.c, need put ethernet header back.
         */
        header_entity_length = 0;
        memcpy(header_entity + header_entity_length, dst_mac, SYS_ADPT_MAC_ADDR_LEN);
        header_entity_length += SYS_ADPT_MAC_ADDR_LEN;
        memcpy(header_entity + header_entity_length, src_mac, SYS_ADPT_MAC_ADDR_LEN);
        header_entity_length += SYS_ADPT_MAC_ADDR_LEN;

        if (mref_handle_p->pkt_info.rx_is_tagged == TRUE)
        {
            UI32_T tpid;
            UI16_T tpid_network, tag_info_network;

            tpid = SYS_DFLT_DOT1Q_PORT_TPID_FIELD;

            tpid_network = L_STDLIB_Hton16((UI16_T)tpid);
            memcpy(header_entity + header_entity_length, &tpid_network, SFLOW_MGR_TPID_BYTES);
            header_entity_length += SFLOW_MGR_TPID_BYTES;

            tag_info_network = L_STDLIB_Hton16(tag_info);
            memcpy(header_entity + header_entity_length, &tag_info_network, SFLOW_MGR_TAG_BYTES);
            header_entity_length += SFLOW_MGR_TAG_BYTES;

            if (mref_handle_p->pkt_info.inner_tag_info > 0)
            {
                stripped += (SFLOW_MGR_TPID_BYTES + SFLOW_MGR_TAG_BYTES);
            }
        }

        ether_type_network = L_STDLIB_Hton16(ether_type);
        memcpy(header_entity + header_entity_length, &ether_type_network, sizeof(ether_type));
        header_entity_length += sizeof(ether_type);
    }

    /* copy correct length of packet to header entity
     */
    if ((pkt_length + header_entity_length) > sampling_entry.max_header_size)
    {
        memcpy(header_entity + header_entity_length, packet_p,
            sampling_entry.max_header_size - header_entity_length);
        header_entity_length = sampling_entry.max_header_size;
    }
    else
    {
        memcpy(header_entity + header_entity_length, packet_p, pkt_length);
        header_entity_length += pkt_length;
    }

    L_MM_Mref_Release(&mref_handle_p);

    SFLOW_MGR_LocalWritePacketSampleToReceiverDatagram(ifindex, instance_id,
        header_protocol, frame_length, stripped,
        header_entity_length, header_entity);

    return;
} /* end of SFLOW_MGR_AnnounceSamplePacket_CallBack()*/

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
    UI32_T receiver_index)
{
    SFLOW_MGR_Receiver_T receiver_entry;
    UI32_T timeout;

    /* check receiver is active
     */
    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.receiver_index = receiver_index;

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetActiveReceiverEntry(&receiver_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* receiver is active, timeout value large than 0
     */
    timeout = receiver_entry.timeout;
    --timeout;

    if (SFLOW_MGR_RECEIVER_TIMEOUT_DISABLE == timeout)
    {
        SFLOW_MGR_LocalDestroyReceiverEntry(receiver_index);
    }
    else
    {
        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverTimeout(receiver_index, timeout))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_ProcessReceiverTimeoutCountdown */

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
    UI32_T instance_id)
{
    SFLOW_MGR_Polling_T polling_entry_active;
    SFLOW_OM_Polling_T polling_entry;
    UI32_T current_time;

    /* check entry is active
     */
    memset(&polling_entry_active, 0, sizeof(polling_entry_active));
    polling_entry_active.ifindex = ifindex;
    polling_entry_active.instance_id = instance_id;

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetActivePollingEntry(&polling_entry_active))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memset(&polling_entry, 0, sizeof(polling_entry));
    polling_entry.ifindex = ifindex;
    polling_entry.instance_id = instance_id;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetPollingEntry(&polling_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    SYS_TIME_GetSoftwareClockBySec(&current_time);

    if ((current_time - polling_entry.last_polling_time) >= polling_entry.polling_interval)
    {
        SFLOW_MGR_LocalWriteCounterSampleToReceiverDatagram(ifindex, instance_id);

        /* update last polling time
         */
        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetPollingLastPollingTime(
            ifindex, instance_id, current_time))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_ProcessPolling */

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
BOOL_T SFLOW_MGR_HandleIpcReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p;

    if (ipcmsg_p == NULL)
        return FALSE;

    msg_p = (SFLOW_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        SYSFUN_Debug_Printf("%s(): In transition mode.\r\n", __FUNCTION__);
        msg_p->type.result_ui32 = SFLOW_MGR_RETURN_FAIL;
        ipcmsg_p->msg_size = SFLOW_MGR_MSGBUF_TYPE_SIZE;
        return TRUE;
    }

    switch (msg_p->type.cmd)
    {
        case SFLOW_MGR_IPCCMD_GET_AGENT_ADDRESS:
            msg_p->type.result_ui32 =
                SFLOW_MGR_GetAgentAddress(&(msg_p->data.addr_v));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(addr_v);
            break;

        case SFLOW_MGR_IPCCMD_GET_AGENT_ADDRESS_TYPE:
            msg_p->type.result_ui32 =
                SFLOW_MGR_GetAgentAddressType(&(msg_p->data.ui32_v));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_v);
            break;

        case SFLOW_MGR_IPCCMD_SET_RECEIVER_OWNER:
            msg_p->type.result_ui32 = SFLOW_MGR_SetReceiverOwner(
                msg_p->data.ui32_ownername.ui32_a1,
                msg_p->data.ui32_ownername.owner_name_a2);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ownername);
            break;

        case SFLOW_MGR_IPCCMD_SET_RECEIVER_TIMEOUT:
            msg_p->type.result_ui32 = SFLOW_MGR_SetReceiverTimeout(
                msg_p->data.ui32_ui32.ui32_a1,
                msg_p->data.ui32_ui32.ui32_a2);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
            break;

        case SFLOW_MGR_IPCCMD_SET_RECEIVER_DESTINATION:
            msg_p->type.result_ui32 = SFLOW_MGR_SetReceiverDestination(
                msg_p->data.ui32_ip.ui32_a1,
                &(msg_p->data.ui32_ip.ip_a2));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ip);
            break;

        case SFLOW_MGR_IPCCMD_SET_RECEIVER_SOCK_PORT:
            msg_p->type.result_ui32 = SFLOW_MGR_SetReceiverSockPort(
                msg_p->data.ui32_ui32.ui32_a1,
                msg_p->data.ui32_ui32.ui32_a2);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
            break;

        case SFLOW_MGR_IPCCMD_SET_RECEIVER_MAX_DATAGRAM_SIZE:
            msg_p->type.result_ui32 = SFLOW_MGR_SetReceiverMaxDatagramSize(
                msg_p->data.ui32_ui32.ui32_a1,
                msg_p->data.ui32_ui32.ui32_a2);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
            break;

        case SFLOW_MGR_IPCCMD_SET_RECEIVER_DATAGRAM_VERSION:
            msg_p->type.result_ui32 = SFLOW_MGR_SetReceiverDatagramVersion(
                msg_p->data.ui32_ui32.ui32_a1,
                msg_p->data.ui32_ui32.ui32_a2);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
            break;

        case SFLOW_MGR_IPCCMD_CREATE_RECEIVER_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_CreateReceiverEntry(
                &(msg_p->data.receiver.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(receiver);
            break;

        case SFLOW_MGR_IPCCMD_DESTROY_RECEIVER_ENTRY_BY_OWNER_NAME:
            msg_p->type.result_ui32 = SFLOW_MGR_DestroyReceiverEntryByOwnerName(
                msg_p->data.ownername.owner_name);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ownername);
            break;

        case SFLOW_MGR_IPCCMD_GET_RECEIVER_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_GetReceiverEntry(
                &(msg_p->data.receiver.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(receiver);
            break;

        case SFLOW_MGR_IPCCMD_GETNEXT_RECEIVER_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_GetNextReceiverEntry(
                &(msg_p->data.receiver.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(receiver);
            break;

        case SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_RECEIVER_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_GetNextActiveReceiverEntry(
                &(msg_p->data.receiver.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(receiver);
            break;

        case SFLOW_MGR_IPCCMD_GET_ACTIVE_RECEIVER_ENTRY_BY_OWNER_NAME:
            msg_p->type.result_ui32 = SFLOW_MGR_GetActiveReceiverEntryByOwnerName(
                &(msg_p->data.receiver.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(receiver);
            break;

        case SFLOW_MGR_IPCCMD_SET_SAMPLING_RATE:
            msg_p->type.result_ui32 = SFLOW_MGR_SetSamplingRate(
                msg_p->data.ui32_ui32_ui32.ui32_a1,
                msg_p->data.ui32_ui32_ui32.ui32_a2,
                msg_p->data.ui32_ui32_ui32.ui32_a3);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32_ui32);
            break;

        case SFLOW_MGR_IPCCMD_SET_SAMPLING_MAX_HEADER_SIZE:
            msg_p->type.result_ui32 = SFLOW_MGR_SetSamplingMaxHeaderSize(
                msg_p->data.ui32_ui32_ui32.ui32_a1,
                msg_p->data.ui32_ui32_ui32.ui32_a2,
                msg_p->data.ui32_ui32_ui32.ui32_a3);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32_ui32);
            break;

        case SFLOW_MGR_IPCCMD_SET_SAMPLING_RECEIVER_INDEX:
            msg_p->type.result_ui32 = SFLOW_MGR_SetSamplingReceiverIndex(
                msg_p->data.ui32_ui32_ui32.ui32_a1,
                msg_p->data.ui32_ui32_ui32.ui32_a2,
                msg_p->data.ui32_ui32_ui32.ui32_a3);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32_ui32);
            break;

        case SFLOW_MGR_IPCCMD_CREATE_SAMPLING_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_CreateSamplingEntry(
                &(msg_p->data.sampling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
            break;

        case SFLOW_MGR_IPCCMD_DESTROY_SAMPLING_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_DestroySamplingEntry(
                msg_p->data.ui32_ui32_ui32.ui32_a1,
                msg_p->data.ui32_ui32_ui32.ui32_a2);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
            break;

        case SFLOW_MGR_IPCCMD_GET_SAMPLING_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_GetSamplingEntry(
                &(msg_p->data.sampling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
            break;

        case SFLOW_MGR_IPCCMD_GET_ACTIVE_SAMPLING_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_GetActiveSamplingEntry(
                &(msg_p->data.sampling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
            break;

        case SFLOW_MGR_IPCCMD_GETNEXT_SAMPLING_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_GetNextSamplingEntry(
                &(msg_p->data.sampling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
            break;

        case SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_SAMPLING_ENTRY_BY_RECEIVER_OWNER_NAME:
            msg_p->type.result_ui32 = SFLOW_MGR_GetNextActiveSamplingEntryByReveiverOwnerName(
                &(msg_p->data.sampling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
            break;

        case SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_SAMPLING_ENTRY_BY_DATASOURCE:
            msg_p->type.result_ui32 = SFLOW_MGR_GetNextActiveSamplingEntryByDatasource(
                &(msg_p->data.sampling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
            break;

        case SFLOW_MGR_IPCCMD_SET_POLLING_INTERVAL:
            msg_p->type.result_ui32 = SFLOW_MGR_SetPollingInterval(
                msg_p->data.ui32_ui32_ui32.ui32_a1,
                msg_p->data.ui32_ui32_ui32.ui32_a2,
                msg_p->data.ui32_ui32_ui32.ui32_a3);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32_ui32);
            break;

        case SFLOW_MGR_IPCCMD_SET_POLLING_RECEIVER_INDEX:
            msg_p->type.result_ui32 = SFLOW_MGR_SetPollingReceiverIndex(
                msg_p->data.ui32_ui32_ui32.ui32_a1,
                msg_p->data.ui32_ui32_ui32.ui32_a2,
                msg_p->data.ui32_ui32_ui32.ui32_a3);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32_ui32);
            break;

        case SFLOW_MGR_IPCCMD_CREATE_POLLING_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_CreatePollingEntry(
                &(msg_p->data.polling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
            break;

        case SFLOW_MGR_IPCCMD_DESTROY_POLLING_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_DestroyPollingEntry(
                msg_p->data.ui32_ui32_ui32.ui32_a1,
                msg_p->data.ui32_ui32_ui32.ui32_a2);
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
            break;

        case SFLOW_MGR_IPCCMD_GET_POLLING_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_GetPollingEntry(
                &(msg_p->data.polling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
            break;

        case SFLOW_MGR_IPCCMD_GET_ACTIVE_POLLING_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_GetActivePollingEntry(
                &(msg_p->data.polling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
            break;

        case SFLOW_MGR_IPCCMD_GETNEXT_POLLING_ENTRY:
            msg_p->type.result_ui32 = SFLOW_MGR_GetNextPollingEntry(
                &(msg_p->data.polling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
            break;

        case SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_POLLING_ENTRY_BY_RECEIVER_OWNER_NAME:
            msg_p->type.result_ui32 = SFLOW_MGR_GetNextActivePollingEntryByReveiverOwnerName(
                &(msg_p->data.polling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
            break;

        case SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_POLLING_ENTRY_BY_DATASOURCE:
            msg_p->type.result_ui32 = SFLOW_MGR_GetNextActivePollingEntryByDatasource(
                &(msg_p->data.polling.entry));
            ipcmsg_p->msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
            break;

        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd(%d).\n", __FUNCTION__, (int)(ipcmsg_p->cmd));
            msg_p->type.result_ui32 = SFLOW_MGR_RETURN_FAIL;
            ipcmsg_p->msg_size = SFLOW_MGR_MSGBUF_TYPE_SIZE;
            break;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckReceiverIsClaimed
 *-------------------------------------------------------------------------
 * PURPOSE  : Check the receiver is claimed or not
 * INPUT    : receiver_index -- receiver index
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckReceiverIsClaimed(
    UI32_T receiver_index)
{
    char  owner_name[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN + 1] = {0};

    if (SFLOW_OM_RETURN_FAIL == SFLOW_OM_GetReceiverOwner(receiver_index, owner_name))
    {
        return FALSE;
    }

    if ('\0' == owner_name[0])
    {
        return FALSE;
    }

    return TRUE;
} /* End of SFLOW_MGR_CheckReceiverIsClaimed */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckDestinationIsValid
 *-------------------------------------------------------------------------
 * PURPOSE  : check destination IP address is valid or not.
 * INPUT    : address_p -- destination IP address
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckDestinationIsValid(
    L_INET_AddrIp_T *address_p)
{
    SFLOW_MGR_CHECK_POINTER_NOT_NULL(address_p);

    switch(address_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
        {

            if (IP_LIB_OK != IP_LIB_IsValidForRemoteIp(address_p->addr))
            {
                return FALSE;
            }
        }
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
        {
            /* IPv6 address should not be
             * IP_LIB_INVALID_IPV6_UNSPECIFIED
             * IP_LIB_INVALID_IPV6_LOOPBACK
             * IP_LIB_INVALID_IPV6_MULTICAST
             */
            if (IP_LIB_OK != IP_LIB_CheckIPv6PrefixForInterface(address_p->addr, SYS_ADPT_IPV6_ADDR_LEN))
            {
                return FALSE;
            }
        }
            break;

        default:
            return FALSE;
    }

    return TRUE;
} /* End of SFLOW_MGR_CheckDestinationIsValid */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckDataSourceAndInstanceIdIsValid
 *-------------------------------------------------------------------------
 * PURPOSE  : check data source and instance id is valid or not.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckDataSourceAndInstanceIsValid(
    UI32_T ifindex,
    UI32_T instance_id)
{
    UI32_T unit;
    UI32_T port;

    if (TRUE != SWCTRL_POM_IfindexToUport(ifindex, &unit, &port))
    {
        return FALSE;
    }

#if (SYS_CPNT_MGMT_PORT == TRUE)
    if (port == SYS_ADPT_MGMT_PORT)
    {
        return FALSE;
    }
#endif

    if (instance_id < SFLOW_MGR_MIN_INSTANCE_ID  ||
        instance_id > SFLOW_MGR_MAX_INSTANCE_ID)
    {
        return FALSE;
    }

    return TRUE;
} /* End of SFLOW_MGR_CheckDataSourceAndInstanceIsValid */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckReceiverEntryIsActive
 *-------------------------------------------------------------------------
 * PURPOSE  : Check receiver entry is active or not.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckReceiverEntryIsActive(
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    if ('\0' == receiver_entry_p->owner_name[0])
    {
        return FALSE;
    }

    if (SFLOW_MGR_RECEIVER_TIMEOUT_DISABLE == receiver_entry_p->timeout)
    {
        return FALSE;
    }

    if (0 == receiver_entry_p->address.addrlen)
    {
        return FALSE;
    }

    return TRUE;
} /* End of SFLOW_MGR_CheckReceiverEntryIsActive */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckSamplingEntryIsActive
 *-------------------------------------------------------------------------
 * PURPOSE  : Check samplingr entry is active or not.
 * INPUT    : sampling_entry_p  -- samplingr entry
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckSamplingEntryIsActive(
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    if (SFLOW_MGR_SAMPLING_RATE_DISABLE == sampling_entry_p->sampling_rate)
    {
        return FALSE;
    }

    if ('\0' == sampling_entry_p->receiver_owner_name[0])
    {
        return FALSE;
    }

    if (0 == sampling_entry_p->receiver_index)
    {
        return FALSE;
    }

    return TRUE;
} /* End of SFLOW_MGR_CheckSamplingEntryIsActive */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_CheckPollingEntryIsActive
 *-------------------------------------------------------------------------
 * PURPOSE  : Check pollingr entry is active or not.
 * INPUT    : polling_entry_p  -- pollingr entry
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_CheckPollingEntryIsActive(
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    if (SFLOW_MGR_POLLING_INTERVAL_DISABLE == polling_entry_p->polling_interval)
    {
        return FALSE;
    }

    if ('\0' == polling_entry_p->receiver_owner_name[0])
    {
        return FALSE;
    }

    if (0 == polling_entry_p->receiver_index)
    {
        return FALSE;
    }

    return TRUE;
} /* End of SFLOW_MGR_CheckPollingEntryIsActive */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalDestroyReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy receiver entry.
 * INPUT    : receiver_index -- receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalDestroyReceiverEntry(
    UI32_T receiver_index)
{
    SFLOW_MGR_Receiver_T receiver_entry;
    UI32_T data_len;

    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.receiver_index = receiver_index;

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetActiveReceiverEntry(&receiver_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetReceiverDataLength(receiver_index, &data_len))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (data_len > 0)
    {
        SFLOW_MGR_LocalSendReceiverDatagram(receiver_index);
    }

    SFLOW_MGR_LocalDestroySamplingEntryByReceiverIndex(receiver_index);
    SFLOW_MGR_LocalDestroyPollingEntryByReceiverIndex(receiver_index);
    SFLOW_OM_InitReceiverEntry(receiver_index);

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalDestroyReceiverEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalGetNextDataSourceAndInstanceId
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next ifindex and instance_id.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalGetNextDataSourceAndInstanceId(
    UI32_T *ifindex_p,
    UI32_T *instance_id_p)
{
    UI32_T  i, j;

    SFLOW_MGR_CHECK_POINTER_NOT_NULL(ifindex_p);
    SFLOW_MGR_CHECK_POINTER_NOT_NULL(instance_id_p);

    /* get first
     */
    if ((0 == *ifindex_p) && (0 == *instance_id_p))
    {
        ++(*ifindex_p);
        ++(*instance_id_p);
    }
    /* if is last instance id of ifindex, get first instance id of next ifindex
     */
    else if (SFLOW_MGR_MAX_INSTANCE_ID == *instance_id_p)
    {
        ++(*ifindex_p);
        *instance_id_p = SFLOW_MGR_MIN_INSTANCE_ID;
    }
    /* get next instance id of ifindex
     */
    else
    {
        ++(*instance_id_p);
    }

    for (i = *ifindex_p; i <= SFLOW_MGR_MAX_IFINDEX; ++i)
    {
        for (j = *instance_id_p; j <= SFLOW_MGR_MAX_INSTANCE_ID; ++j)
        {
            if (TRUE == SFLOW_MGR_CheckDataSourceAndInstanceIsValid(i, j))
            {
                *ifindex_p = i;
                *instance_id_p = j;
                return SFLOW_MGR_RETURN_SUCCESS;
            }
        }
    }

    return SFLOW_MGR_RETURN_FAIL;
} /* End of SFLOW_MGR_LocalGetNextDataSourceAndInstanceId */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalProcessSampling
 *-------------------------------------------------------------------------
 * PURPOSE  : Process sampling.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
SFLOW_MGR_LocalProcessSampling(
    UI32_T ifindex,
    UI32_T instance_id)
{
    SFLOW_MGR_Sampling_T sampling_entry;

    memset(&sampling_entry, 0, sizeof(sampling_entry));
    sampling_entry.ifindex = ifindex;
    sampling_entry.instance_id = instance_id;

    if (SFLOW_MGR_RETURN_SUCCESS !=
        SFLOW_MGR_GetActiveSamplingEntry(&sampling_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (TRUE != SWCTRL_PMGR_SetSflowPortPacketSamplingRate(
        ifindex, sampling_entry.sampling_rate))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalProcessSampling */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalDestroySamplingEntryByReceiverIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy sampling entry by receiver index.
 * INPUT    : receiver_index  -- receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalDestroySamplingEntryByReceiverIndex(
    UI32_T  receiver_index)
{
    SFLOW_MGR_Sampling_T  sampling_entry;

    memset(&sampling_entry, 0, sizeof(sampling_entry));

    while (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetNextSamplingEntry(&sampling_entry))
    {
        if (TRUE != SFLOW_MGR_CheckSamplingEntryIsActive(&sampling_entry))
        {
            continue;
        }

        if (sampling_entry.receiver_index != receiver_index)
        {
            continue;
        }

        SWCTRL_PMGR_SetSflowPortPacketSamplingRate(
            sampling_entry.ifindex, SFLOW_MGR_RECEIVER_TIMEOUT_DISABLE);

        SFLOW_OM_InitSamplingEntry(sampling_entry.ifindex, sampling_entry.instance_id);
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalDestroySamplingEntryByReceiverIndex */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalDestroyPollingEntryByReceiverIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy polling entry by receiver index.
 * INPUT    : receiver_index  -- receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalDestroyPollingEntryByReceiverIndex(
    UI32_T  receiver_index)
{
    SFLOW_MGR_Polling_T  polling_entry;

    memset(&polling_entry, 0, sizeof(polling_entry));

    while (SFLOW_MGR_RETURN_SUCCESS == SFLOW_MGR_GetNextPollingEntry(&polling_entry))
    {
        if (TRUE != SFLOW_MGR_CheckPollingEntryIsActive(&polling_entry))
        {
            continue;
        }

        if (polling_entry.receiver_index != receiver_index)
        {
            continue;
        }

        SFLOW_OM_InitPollingEntry(polling_entry.ifindex, polling_entry.instance_id);
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalDestroyPollingEntryByReceiverIndex */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalComputeDatagramHeaderSize
 *-------------------------------------------------------------------------
 * PURPOSE  : compute the datagram header size.
 * INPUT    : datagram_version  -- datagram version
 * OUTPUT   : header_size_p     -- datagram header size
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalComputeDatagramHeaderSize(
    UI32_T datagram_version,
    UI32_T *header_size_p)
{
    UI32_T addr_type;

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetAgentAddressType(&addr_type))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    switch (addr_type)
    {
        case VAL_sFlowAgentAddressType_ipv6:
        case L_INET_ADDR_TYPE_IPV6Z:
            /* 9 fields(sFlow version, agent ip version, agent ip address(4 fields),
               datagram sequence number, switch uptime, sample numbers)
             */
            *header_size_p = sizeof(UI32_T) * 9;
            break;
        case VAL_sFlowAgentAddressType_ipv4:
        case VAL_sFlowAgentAddressType_ipv4z:
            /* 6 fields(sFlow version, agent ip version, agent ip address(1 field),
               datagram sequence number, switch uptime, sample numbers)
             */
            *header_size_p = sizeof(UI32_T) * 6;
            break;

        case VAL_sFlowAgentAddressType_unknown:
        default:
            return SFLOW_MGR_RETURN_FAIL;
    }

    switch (datagram_version)
    {
        case SFLOW_MGR_DATAGRAM_VERSION_4:
            break;

        case SFLOW_MGR_DATAGRAM_VERSION_5:
            *header_size_p += sizeof(UI32_T); /* 1 field (sub agent id) */
            break;

        default:
            return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalComputeDatagramHeaderSize() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalComputePacketSampleSize
 *-------------------------------------------------------------------------
 * PURPOSE  : Compute counter sample packet size.
 * INPUT    : datagram_version        -- datagram version
 * INPUT    : header_entity_length    -- header entity UI8 length
 * OUTPUT   : header_length_p         -- header entity UI32 length
 *            header_record_length_p  -- header record data length
 *            sample_packet_size_p    -- packet sample packet size
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalComputePacketSampleSize(
    UI32_T datagram_version,
    UI32_T header_entity_length,
    UI32_T *header_length_p,
    UI32_T *header_record_length_p,
    UI32_T *sample_packet_size_p)
{
    UI32_T header_length;
    UI32_T header_record_length;
    UI32_T header_record_size;
    UI32_T sample_packet_size;

    /* header entity length rounded up to nearest UI32 size
     */
    header_length = ((header_entity_length + 3) / sizeof(UI32_T)) * sizeof(UI32_T);

    switch (datagram_version)
    {
        case SFLOW_MGR_DATAGRAM_VERSION_4:
            /* 8 fields(sample type, sample sequence, data source, sampling rate,
             * sample pool, sample drops, input interface, output interface)
             */
            sample_packet_size = sizeof(UI32_T) * 8;

            header_record_length = 0;

            /* 4 fields(record type, header protocol, frame length, header length, extend number)
             */
            header_record_size = sizeof(UI32_T) * 5 + header_length;
            break;

       case SFLOW_MGR_DATAGRAM_VERSION_5:
            /* 10 fields(sample type, sample length, sample sequence, data source, sampling rate,
             * sample pool, sample drops, input interface, output interface, record number)
             */
            sample_packet_size = sizeof(UI32_T) * 10;

            /* 6 fields(record type, record length, header protocol, frame length, stripped, header length)
             */
            header_record_size = sizeof(UI32_T) * 6 + header_length;

            /* not include 2 fields(header protocl, frame length)
             */
            header_record_length = header_record_size - (sizeof(UI32_T) * 2);
            break;

        default:
            return SFLOW_MGR_RETURN_FAIL;
    }

    *header_length_p = header_length;
    *header_record_length_p = header_record_length;
    *sample_packet_size_p = sample_packet_size + header_record_size;

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalComputePacketSampleSize() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalComputeCounterSampleSize
 *-------------------------------------------------------------------------
 * PURPOSE  : Compute counter sample packet size.
 * INPUT    : datagram_version     -- datagram version
 * OUTPUT   : sample_packet_size_p -- counter sample packet size
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalComputeCounterSampleSize(
    UI32_T datagram_version,
    UI32_T *sample_packet_size_p)
{
    UI32_T if_counter_record_size;
    UI32_T ethernet_counter_record_size;
    UI32_T sample_packet_size;

    switch (datagram_version)
    {
        case SFLOW_MGR_DATAGRAM_VERSION_4:
            /* 3 fields(sample type, sample sequence, data source, interval)
             */
            sample_packet_size = sizeof(UI32_T) * 4;

            if_counter_record_size = 0;

            /* 1 field(record type)
             */
            ethernet_counter_record_size = sizeof(UI32_T) + sizeof(SFLOW_MGR_IfCounters_T) + sizeof(SFLOW_MGR_EthernetCounters_T);
            break;

        case SFLOW_MGR_DATAGRAM_VERSION_5:
            /* 5 fields(sample type, sample length, sample sequence, data source, record number)
             */
            sample_packet_size = sizeof(UI32_T) * 5;

            /* 2 fields(record type, record length)
             */
            if_counter_record_size = sizeof(UI32_T) * 2 + sizeof(SFLOW_MGR_IfCounters_T);

            /* 2 fields(record type, record length)
             */
            ethernet_counter_record_size = sizeof(UI32_T) * 2 + sizeof(SFLOW_MGR_EthernetCounters_T);
            break;

        default:
            return SFLOW_MGR_RETURN_FAIL;
    }

    *sample_packet_size_p = sample_packet_size + if_counter_record_size + ethernet_counter_record_size;

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalComputeCounterSampleSize() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalWritePacketSampleToReceiverDatagram
 *-------------------------------------------------------------------------
 * PURPOSE  : Write packet sample to receiver datagram.
 * INPUT    : ifindex               -- interface index
 *            instance_id           -- instance id
 *            header_protocol       -- heaer protocol type
 *            frame_length          -- original length of packet before sampling
 *            stripped              -- number of byte removed from the packet
 *            header_entity_length  -- header entity UI8 length
 *            header_entity_p       -- header entity
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalWritePacketSampleToReceiverDatagram(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T header_protocol,
    UI32_T frame_length,
    UI32_T stripped,
    UI32_T header_entity_length,
    UI8_T *header_entity_p)
{
    SFLOW_OM_Sampling_T sampling_entry;
    SFLOW_OM_Receiver_T receiver_entry;
    UI32_T datagrame_header_size;
    UI32_T sample_packet_size;
    UI32_T sample_length;
    UI32_T header_record_length;
    UI32_T header_length;
    UI32_T index;
    UI32_T data_ar[(SYS_ADPT_SFLOW_MAX_RECEIVER_DATAGRAM_SIZE + SFLOW_OM_DATA_PAD) / sizeof(UI32_T)] = {0};

    memset(&sampling_entry, 0, sizeof(sampling_entry));
    sampling_entry.ifindex = ifindex;
    sampling_entry.instance_id = instance_id;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetSamplingEntry(&sampling_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.receiver_index = sampling_entry.receiver_index;

    SFLOW_MGR_CHECK_RECEIVER_INDEX_RANGE(receiver_entry.receiver_index);

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetReceiverEntry(&receiver_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* compute sample packet size
     */
    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_LocalComputePacketSampleSize(
        receiver_entry.datagram.version, header_entity_length,
        &header_length, &header_record_length, &sample_packet_size))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* check if the sample is too big
     */
    if (sample_packet_size > receiver_entry.datagram.max_size)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* compute datagram header size
     */
    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_LocalComputeDatagramHeaderSize(
        receiver_entry.datagram.version, &datagrame_header_size))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* check datagram length
     */
    if ((datagrame_header_size + (receiver_entry.datagram.data_len * sizeof(UI32_T)) + sample_packet_size)
        >= receiver_entry.datagram.max_size)
    {
        SFLOW_MGR_LocalSendReceiverDatagram(receiver_entry.receiver_index);

        /* after send datagram, get current receiver information
         */
        memset(&receiver_entry, 0, sizeof(receiver_entry));
        receiver_entry.receiver_index = sampling_entry.receiver_index;

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetReceiverEntry(&receiver_entry))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    /* update sample sequence
     */
    ++sampling_entry.sample_sequence;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetSamplingSampleSequence(
        ifindex, instance_id, sampling_entry.sample_sequence))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* fill sample fileds
     */
    index = 0;
    data_ar[index++] = L_STDLIB_Hton32(SFLOW_MGR_SAMPLE_TYPE_PACKET);   /* sample type */

    if (SFLOW_MGR_DATAGRAM_VERSION_5 == receiver_entry.datagram.version)
    {
        sample_length = sample_packet_size - (sizeof(UI32_T) * 2);     /* not include 2 fields(sample type, sample length) */
        data_ar[index++] = L_STDLIB_Hton32(sample_length);             /* sample length */
    }

    data_ar[index++] = L_STDLIB_Hton32(sampling_entry.sample_sequence); /* sample sequence */
    data_ar[index++] = L_STDLIB_Hton32(sampling_entry.ifindex);         /* data source */
    data_ar[index++] = L_STDLIB_Hton32(sampling_entry.sampling_rate);   /* sampling rate */
    data_ar[index++] = L_STDLIB_Hton32(sampling_entry.sample_pool);     /* sample pool */
    data_ar[index++] = L_STDLIB_Hton32(sampling_entry.sample_pool - sampling_entry.sample_sequence);  /* sample drops */
    data_ar[index++] = L_STDLIB_Hton32(sampling_entry.ifindex);        /* input interface */
    data_ar[index++] = L_STDLIB_Hton32(0);                             /* output interface, unknow fill 0 */

    if (SFLOW_MGR_DATAGRAM_VERSION_5 == receiver_entry.datagram.version)
    {
        data_ar[index++] = L_STDLIB_Hton32(1);  /* record number, only support header record fill 1 */
    }

    /* fill headr record fileds
     */
    data_ar[index++] = L_STDLIB_Hton32(SFLOW_MGR_SAMPLE_RECORD_TYPE_HEADER); /* record type */

    if (SFLOW_MGR_DATAGRAM_VERSION_5 == receiver_entry.datagram.version)
    {
        data_ar[index++] = L_STDLIB_Hton32(header_record_length);            /* record length */
    }

    data_ar[index++] = L_STDLIB_Hton32(header_protocol);                     /* header protocl */
    data_ar[index++] = L_STDLIB_Hton32(frame_length);                        /* frame length */

    if (SFLOW_MGR_DATAGRAM_VERSION_5 == receiver_entry.datagram.version)
    {
        data_ar[index++] = L_STDLIB_Hton32(stripped);                        /* stripped */
    }

    data_ar[index++] = L_STDLIB_Hton32(header_length);                       /* header length */

    memcpy(data_ar+index, header_entity_p, header_entity_length);            /* header entity */

    if (SFLOW_MGR_DATAGRAM_VERSION_5 != receiver_entry.datagram.version)
    {
        data_ar[index + header_length] = L_STDLIB_Hton32(0);                /* extend number */
    }

    /* set to receiver datagram
     */
    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverDatagramData(
        receiver_entry.receiver_index, data_ar, sample_packet_size))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* update datagram sample number
     */
    ++receiver_entry.datagram.sample_number;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverDatagramSampleNumber(
        receiver_entry.receiver_index, receiver_entry.datagram.sample_number))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalWritePacketSampleToReceiverDatagram */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalWriteCounterSampleToReceiverDatagram
 *-------------------------------------------------------------------------
 * PURPOSE  : Write counter sample to receiver datagram.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalWriteCounterSampleToReceiverDatagram(
    UI32_T ifindex,
    UI32_T instance_id)
{
    SFLOW_OM_Polling_T           polling_entry;
    SFLOW_OM_Receiver_T          receiver_entry;
    SFLOW_MGR_IfCounters_T       if_counter;
    SFLOW_MGR_EthernetCounters_T ethernet_counter;
    UI32_T datagrame_header_size;
    UI32_T sample_packet_size;
    UI32_T sample_length;
    UI32_T index;
    UI32_T data_ar[(SYS_ADPT_SFLOW_MAX_RECEIVER_DATAGRAM_SIZE + SFLOW_OM_DATA_PAD) / sizeof(UI32_T)] = {0};

    /* get data
     */
    memset(&polling_entry, 0, sizeof(polling_entry));
    polling_entry.ifindex = ifindex;
    polling_entry.instance_id = instance_id;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetPollingEntry(&polling_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.receiver_index = polling_entry.receiver_index;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetReceiverEntry(&receiver_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memset(&if_counter, 0, sizeof(if_counter));

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_LocalGetIfCounterData(
        ifindex, &if_counter))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memset(&ethernet_counter, 0, sizeof(ethernet_counter));

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_LocalGetEthernetCounterData(
        ifindex, &ethernet_counter))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* counter sample packet size
     */
    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_LocalComputeCounterSampleSize(
        receiver_entry.datagram.version, &sample_packet_size))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* check if the sample is too big
     */
    if (sample_packet_size > receiver_entry.datagram.max_size)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* compute datagram header size
     */
    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_LocalComputeDatagramHeaderSize(
        receiver_entry.datagram.version, &datagrame_header_size))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* check datagram length
     */
    if ((datagrame_header_size + (receiver_entry.datagram.data_len * sizeof(UI32_T)) + sample_packet_size)
        >= receiver_entry.datagram.max_size)
    {
        SFLOW_MGR_LocalSendReceiverDatagram(receiver_entry.receiver_index);

        /* after send datagram, get current receiver information
         */
        memset(&receiver_entry, 0, sizeof(receiver_entry));
        receiver_entry.receiver_index = polling_entry.receiver_index;

        if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetReceiverEntry(&receiver_entry))
        {
            return SFLOW_MGR_RETURN_FAIL;
        }
    }

    /* update sample sequence
     */
    ++polling_entry.sample_sequence;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetPollingSampleSequence(
        ifindex, instance_id, polling_entry.sample_sequence))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* fill sample fileds
     */
    index = 0;
    data_ar[index++] = L_STDLIB_Hton32(SFLOW_MGR_SAMPLE_TYPE_COUNTER);  /* sample type */

    if (SFLOW_MGR_DATAGRAM_VERSION_5 == receiver_entry.datagram.version)
    {
        sample_length = sample_packet_size - (sizeof(UI32_T) * 2);     /* not include 2 fields(sample type, sample length) */
        data_ar[index++] = L_STDLIB_Hton32(sample_length);             /* sample length */
    }

    data_ar[index++] = L_STDLIB_Hton32(polling_entry.sample_sequence); /* sample sequence */
    data_ar[index++] = L_STDLIB_Hton32(polling_entry.ifindex);         /* data source */

    SFLOW_MGR_LocalEncodeIfCounterData(&if_counter);
    SFLOW_MGR_LocalEncodeEthernetCounterData(&ethernet_counter);

    if (SFLOW_MGR_DATAGRAM_VERSION_5 == receiver_entry.datagram.version)
    {
        data_ar[index++] = L_STDLIB_Hton32(2);  /* record number, support if_counter and ethernet counter record fill 2 */

        /* fill if_counter record fileds
         */
        data_ar[index++] = L_STDLIB_Hton32(SFLOW_MGR_COUNTER_RECORD_TYPE_IF); /* record type */
        data_ar[index++] = L_STDLIB_Hton32(sizeof(SFLOW_MGR_IfCounters_T));   /* record length */
        memcpy(data_ar+index, &if_counter, sizeof(SFLOW_MGR_IfCounters_T));   /* counter entity */
        index += (sizeof(SFLOW_MGR_IfCounters_T) / sizeof(UI32_T));

        /* fill ethernet counter record fileds
         */
        data_ar[index++] = L_STDLIB_Hton32(SFLOW_MGR_COUNTER_RECORD_TYPE_ETHERNET);       /* record type */
        data_ar[index++] = L_STDLIB_Hton32(sizeof(SFLOW_MGR_EthernetCounters_T));         /* record length */
        memcpy(data_ar + index, &ethernet_counter, sizeof(SFLOW_MGR_EthernetCounters_T)); /* counter entity */
    }
    else
    {
        data_ar[index++] = L_STDLIB_Hton32(polling_entry.polling_interval);               /* interval */

        /* fill ethernet counter record fileds
         */
        data_ar[index++] = L_STDLIB_Hton32(SFLOW_MGR_COUNTER_RECORD_TYPE_ETHERNET);       /* record type */
        memcpy(data_ar + index, &if_counter, sizeof(SFLOW_MGR_IfCounters_T));             /* counter entity */
        index += (sizeof(SFLOW_MGR_IfCounters_T) / sizeof(UI32_T));
        memcpy(data_ar + index, &ethernet_counter, sizeof(SFLOW_MGR_EthernetCounters_T)); /* counter entity */
    }

    /* set to receiver datagram
     */
    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverDatagramData(
        receiver_entry.receiver_index, data_ar, sample_packet_size))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* update datagram sample number
     */
    ++receiver_entry.datagram.sample_number;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_SetReceiverDatagramSampleNumber(
        receiver_entry.receiver_index, receiver_entry.datagram.sample_number))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalWriteCounterSampleToReceiverDatagram */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalGetIfCounterData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get interface generic counter data.
 * INPUT    : ifindex       -- interface index
 * OUTPUT   : if_counter_p  -- interface counter entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalGetIfCounterData(
    UI32_T ifindex,
    SFLOW_MGR_IfCounters_T *if_counter_p)
{
    IF_MGR_IfEntry_T      if_entry;
    SWDRV_IfTableStats_T  state;
    SWDRV_IfXTableStats_T state_x;
    Port_Info_T           port_info;
    UI32_T status;

    memset(&if_entry, 0, sizeof(if_entry));
    memset(&state, 0, sizeof(state));
    memset(&state_x, 0, sizeof(state_x));
    memset(&port_info, 0, sizeof(port_info));

    if_entry.if_index = ifindex;

    if (TRUE != IF_PMGR_GetIfEntry(&if_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (TRUE != NMTR_PMGR_GetIfTableStats(ifindex, &state))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (TRUE != NMTR_PMGR_GetIfXTableStats(ifindex, &state_x))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (TRUE != SWCTRL_POM_GetPortInfo(ifindex, &port_info))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex10 ||
        port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex100 ||
        port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex1000 ||
        port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_halfDuplex10g)
    {
        if_counter_p->ifDirection = 0x02; /* half-duplex */
    }
    else
    {
        if_counter_p->ifDirection = 0x01; /* full-duplex */
    }

    /* bit 0 => ifAdminStatus 0=down|1=up, bit 1 => ifOperStatus 0=down|1=up
     */
    status = ((port_info.link_oper_status == VAL_ifOperStatus_up) << 1) | (port_info.admin_state == VAL_ifAdminStatus_up);
    if_counter_p->ifStatus = status;

    if_counter_p->ifIndex            = ifindex;
    if_counter_p->ifType             = if_entry.if_type;
    if_counter_p->ifSpeed            = if_entry.if_speed;
    if_counter_p->ifInOctets         = state.ifInOctets;
    if_counter_p->ifInUcastPkts      = (UI32_T)state.ifInUcastPkts;
    if_counter_p->ifInMulticastPkts  = (UI32_T)state_x.ifInMulticastPkts;
    if_counter_p->ifInBroadcastPkts  = (UI32_T)state_x.ifInBroadcastPkts;
    if_counter_p->ifInDiscards       = (UI32_T)state.ifInDiscards;
    if_counter_p->ifInErrors         = (UI32_T)state.ifInErrors;
    if_counter_p->ifInUnknownProtos  = (UI32_T)state.ifInUnknownProtos;
    if_counter_p->ifOutOctets        = state.ifOutOctets;
    if_counter_p->ifOutUcastPkts     = (UI32_T)state.ifOutUcastPkts;
    if_counter_p->ifOutMulticastPkts = (UI32_T)state_x.ifOutMulticastPkts;
    if_counter_p->ifOutBroadcastPkts = (UI32_T)state_x.ifOutBroadcastPkts;
    if_counter_p->ifOutDiscards      = (UI32_T)state.ifOutDiscards;
    if_counter_p->ifOutErrors        = (UI32_T)state.ifOutErrors;
    if_counter_p->ifPromiscuousMode  = 0;

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalGetIfCounterData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalEncodeIfCounterData
 *-------------------------------------------------------------------------
 * PURPOSE  : Encode interface generic counter data.
 * INPUT    : if_counter_p  -- interface counter entry
 * OUTPUT   : if_counter_p  -- interface counter entry
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void
SFLOW_MGR_LocalEncodeIfCounterData(
    SFLOW_MGR_IfCounters_T *if_counter_p)
{
    if_counter_p->ifIndex            = L_STDLIB_Hton32(if_counter_p->ifIndex);
    if_counter_p->ifType             = L_STDLIB_Hton32(if_counter_p->ifType);
    if_counter_p->ifSpeed            = L_STDLIB_Hton64(if_counter_p->ifSpeed);
    if_counter_p->ifDirection        = L_STDLIB_Hton32(if_counter_p->ifDirection);
    if_counter_p->ifStatus           = L_STDLIB_Hton32(if_counter_p->ifStatus);
    if_counter_p->ifInOctets         = L_STDLIB_Hton64(if_counter_p->ifInOctets);
    if_counter_p->ifInUcastPkts      = L_STDLIB_Hton32(if_counter_p->ifInUcastPkts);
    if_counter_p->ifInMulticastPkts  = L_STDLIB_Hton32(if_counter_p->ifInMulticastPkts);
    if_counter_p->ifInBroadcastPkts  = L_STDLIB_Hton32(if_counter_p->ifInBroadcastPkts);
    if_counter_p->ifInDiscards       = L_STDLIB_Hton32(if_counter_p->ifInDiscards);
    if_counter_p->ifInErrors         = L_STDLIB_Hton32(if_counter_p->ifInErrors);
    if_counter_p->ifInUnknownProtos  = L_STDLIB_Hton32(if_counter_p->ifInUnknownProtos);
    if_counter_p->ifOutOctets        = L_STDLIB_Hton64(if_counter_p->ifOutOctets);
    if_counter_p->ifOutUcastPkts     = L_STDLIB_Hton32(if_counter_p->ifOutUcastPkts);
    if_counter_p->ifOutMulticastPkts = L_STDLIB_Hton32(if_counter_p->ifOutMulticastPkts);
    if_counter_p->ifOutBroadcastPkts = L_STDLIB_Hton32(if_counter_p->ifOutBroadcastPkts);
    if_counter_p->ifOutDiscards      = L_STDLIB_Hton32(if_counter_p->ifOutDiscards);
    if_counter_p->ifOutErrors        = L_STDLIB_Hton32(if_counter_p->ifOutErrors);
    if_counter_p->ifPromiscuousMode  = L_STDLIB_Hton32(if_counter_p->ifPromiscuousMode);
} /* End of SFLOW_MGR_LocalEncodeIfCounterData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalGetEthernetCounterData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get ethernet counter data.
 * INPUT    : ifindex             -- interface index
 * OUTPUT   : ethernet_counter_p  -- ethernet counter entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalGetEthernetCounterData(
    UI32_T ifindex,
    SFLOW_MGR_EthernetCounters_T *ethernet_counter_p)
{
    SWDRV_EtherlikeStats_T ether_like_stats;

    memset(&ether_like_stats, 0, sizeof(ether_like_stats));

#if(SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)
    if (TRUE != NMTR_PMGR_GetSystemwideEtherLikeStats(ifindex, &ether_like_stats))
#else
    if (TRUE != NMTR_PMGR_GetEtherLikeStats(ifindex, &ether_like_stats))
#endif
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    ethernet_counter_p->dot3StatsAlignmentErrors           = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsAlignmentErrors);
    ethernet_counter_p->dot3StatsFCSErrors                 = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsFCSErrors);
    ethernet_counter_p->dot3StatsSingleCollisionFrames     = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsSingleCollisionFrames);
    ethernet_counter_p->dot3StatsMultipleCollisionFrames   = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsMultipleCollisionFrames);
    ethernet_counter_p->dot3StatsSQETestErrors             = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsSQETestErrors);
    ethernet_counter_p->dot3StatsDeferredTransmissions     = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsDeferredTransmissions);
    ethernet_counter_p->dot3StatsLateCollisions            = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsLateCollisions);
    ethernet_counter_p->dot3StatsExcessiveCollisions       = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsExcessiveCollisions);
    ethernet_counter_p->dot3StatsInternalMacTransmitErrors = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsInternalMacTransmitErrors);
    ethernet_counter_p->dot3StatsCarrierSenseErrors        = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsCarrierSenseErrors);
    ethernet_counter_p->dot3StatsFrameTooLongs             = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsFrameTooLongs);
    ethernet_counter_p->dot3StatsInternalMacReceiveErrors  = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsInternalMacReceiveErrors);
    ethernet_counter_p->dot3StatsSymbolErrors              = L_STDLIB_UI64_L32(ether_like_stats.dot3StatsSymbolErrors);

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalGetEthernetCounterData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalEncodeEthernetCounterData
*-------------------------------------------------------------------------
 * PURPOSE  : Encode ethernet counter data.
 * INPUT    : ethernet_counter_p  -- ethernet counter entry
 * OUTPUT   : ethernet_counter_p  -- ethernet counter entry
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void
SFLOW_MGR_LocalEncodeEthernetCounterData(
    SFLOW_MGR_EthernetCounters_T *ethernet_counter_p)
{
    ethernet_counter_p->dot3StatsAlignmentErrors           = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsAlignmentErrors);
    ethernet_counter_p->dot3StatsFCSErrors                 = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsFCSErrors);
    ethernet_counter_p->dot3StatsSingleCollisionFrames     = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsSingleCollisionFrames);
    ethernet_counter_p->dot3StatsMultipleCollisionFrames   = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsMultipleCollisionFrames);
    ethernet_counter_p->dot3StatsSQETestErrors             = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsSQETestErrors);
    ethernet_counter_p->dot3StatsDeferredTransmissions     = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsDeferredTransmissions);
    ethernet_counter_p->dot3StatsLateCollisions            = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsLateCollisions);
    ethernet_counter_p->dot3StatsExcessiveCollisions       = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsExcessiveCollisions);
    ethernet_counter_p->dot3StatsInternalMacTransmitErrors = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsInternalMacTransmitErrors);
    ethernet_counter_p->dot3StatsCarrierSenseErrors        = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsCarrierSenseErrors);
    ethernet_counter_p->dot3StatsFrameTooLongs             = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsFrameTooLongs);
    ethernet_counter_p->dot3StatsInternalMacReceiveErrors  = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsInternalMacReceiveErrors);
    ethernet_counter_p->dot3StatsSymbolErrors              = L_STDLIB_Hton32(ethernet_counter_p->dot3StatsSymbolErrors);
} /* End of SFLOW_MGR_LocalEncodeEthernetCounterData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_MGR_LocalSendReceiverDatagram
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy receiver entry.
 * INPUT    : receiver_index -- receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static SFLOW_MGR_RETURN_VALUE_T
SFLOW_MGR_LocalSendReceiverDatagram(
    UI32_T receiver_index)
{
    SFLOW_OM_Receiver_T receiver_entry;
    struct sockaddr sockaddr;
    L_INET_AddrIp_T agent_ip;
    UI32_T index;
    UI32_T ip_address;
    UI32_T tick;
    UI32_T socket_id, sockaddr_len;
    UI32_T data_ar[(SYS_ADPT_SFLOW_MAX_RECEIVER_DATAGRAM_SIZE + SFLOW_OM_DATA_PAD) / sizeof(UI32_T)] = {0};

    memset(&receiver_entry, 0, sizeof(receiver_entry));
    receiver_entry.receiver_index = receiver_index;

    if (SFLOW_OM_RETURN_SUCCESS != SFLOW_OM_GetReceiverEntry(&receiver_entry))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    if (SFLOW_MGR_RETURN_SUCCESS != SFLOW_MGR_GetAgentAddress(&agent_ip))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memset(data_ar, 0, sizeof(data_ar));
    index = 0;
    data_ar[index++] = L_STDLIB_Hton32(receiver_entry.datagram.version); /* datagram version */

    switch (agent_ip.type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            data_ar[index++] = L_STDLIB_Hton32(1);   /* agent ip version, v4 = 1 */

            /* agent ip, v4 = 4bytes/field x 1 filed
             */
            IP_LIB_ArraytoUI32(agent_ip.addr, &ip_address);
            data_ar[index++] = ip_address;
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            data_ar[index++] = L_STDLIB_Hton32(2);  /* agent ip version, v6 = 2 */

            /* agent ip, v6 = 4bytes/field x 4 filed
            */
            IP_LIB_ArraytoUI32(agent_ip.addr, &ip_address);
            data_ar[index++] = ip_address;

            IP_LIB_ArraytoUI32(agent_ip.addr+4, &ip_address);
            data_ar[index++] = ip_address;

            IP_LIB_ArraytoUI32(agent_ip.addr+8, &ip_address);
            data_ar[index++] = ip_address;

            IP_LIB_ArraytoUI32(agent_ip.addr+12, &ip_address);
            data_ar[index++] = ip_address;
            break;

        default:
            return SFLOW_MGR_RETURN_FAIL;
    }

    switch (receiver_entry.datagram.version)
    {
        case 4:
            break;

        case 5:
            data_ar[index++] = L_STDLIB_Hton32(0);  /* sub agent id, not support fill 0 */
            break;

        default:
            return SFLOW_MGR_RETURN_FAIL;
    }

    /* update datagram sequence
     */
    ++receiver_entry.datagram.sequence;

    if (SFLOW_OM_RETURN_SUCCESS !=
        SFLOW_OM_SetReceiverDatagramSequence(receiver_index, receiver_entry.datagram.sequence))
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    data_ar[index++] = L_STDLIB_Hton32(receiver_entry.datagram.sequence);         /* datagram sequence number */

    SYS_TIME_GetSystemUpTimeByTick(&tick);
    data_ar[index++] = L_STDLIB_Hton32(tick * (1000 / SYS_BLD_TICKS_PER_SECOND)); /* switch uptime (milliseconds) */
    data_ar[index++] = L_STDLIB_Hton32(receiver_entry.datagram.sample_number);    /* sample numbers */

    memcpy(data_ar + index, receiver_entry.datagram.data_ar,
        receiver_entry.datagram.data_len * sizeof(UI32_T));                       /* sample entity */

    switch (receiver_entry.address.type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            sockaddr_len = sizeof(struct sockaddr_in);
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            sockaddr_len = sizeof(struct sockaddr_in6);
            break;

        default:
            return SFLOW_MGR_RETURN_FAIL;
    }

    /* convert from L_INET_Addr type to sockaddr type
     */
    L_INET_InaddrToSockaddr(&receiver_entry.address, receiver_entry.udp_port, sockaddr_len, &sockaddr);

    socket_id = socket(sockaddr.sa_family, SOCK_DGRAM, 0);
    bind(socket_id, &sockaddr, sockaddr_len);
    sendto(socket_id,
                    data_ar,
                    sizeof(UI32_T) * (index + receiver_entry.datagram.data_len),
                    0,
                    &sockaddr,
                    sockaddr_len);
    close(socket_id);

    SFLOW_OM_ClearReceiverDatagram(receiver_entry.receiver_index);

    return SFLOW_MGR_RETURN_SUCCESS;
} /* End of SFLOW_MGR_LocalSendReceiverDatagram */

#endif /* #if (SYS_CPNT_SFLOW == TRUE) */

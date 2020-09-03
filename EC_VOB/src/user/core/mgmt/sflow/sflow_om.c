/* -----------------------------------------------------------------------------
 * FILE NAME: SFLOW_OM.c
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
#include <assert.h>
#include <string.h>
#include "sys_dflt.h"
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sflow_om.h"


#if (SYS_CPNT_SFLOW == TRUE)

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define SFLOW_OM_ENTER_CRITICAL_SECTION() \
    om_orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(om_sem_id);
#define SFLOW_OM_LEAVE_CRITICAL_SECTION() \
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(om_sem_id, om_orig_priority);

/* DATA TYPE DECLARATIONS
 */
typedef struct SFLOW_OM_ShmemData_S
{
    SFLOW_OM_Receiver_T receiver_table[SYS_ADPT_SFLOW_MAX_NUMBER_OF_RECEIVER_ENTRY];
    SFLOW_OM_Sampling_T port_sampling_table[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_TOTAL_PORTS_PER_UNIT_ON_BOARD][SYS_ADPT_SFLOW_MAX_INSTANCE_OF_DATASOURCE];
    SFLOW_OM_Polling_T  port_polling_table[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_TOTAL_PORTS_PER_UNIT_ON_BOARD][SYS_ADPT_SFLOW_MAX_INSTANCE_OF_DATASOURCE];
} SFLOW_OM_ShmemData_T;

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T om_sem_id;
static UI32_T om_orig_priority;
static SFLOW_OM_ShmemData_T *shmem_data_p;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_InitiateSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource.
 * INPUT    : None
 * OUTPUT   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void SFLOW_OM_InitiateSystemResources(void)
{
    return;
} /* End of SFLOW_OM_InitiateSystemResources */

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
void SFLOW_OM_AttachSystemResources(void)
{
    shmem_data_p = (SFLOW_OM_ShmemData_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SFLOW_OM_SHMEM_SEGID);

    if (NULL == shmem_data_p)
    {
        SYSFUN_Debug_Printf("\r\n SFLOW: Get shared memory failed.");
    }

    memset(shmem_data_p, 0, sizeof(SFLOW_OM_ShmemData_T));

    if (SYSFUN_OK != SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SFLOW_OM, &om_sem_id))
    {
        SYSFUN_Debug_Printf("\r\n SFLOW: Create semaphore failed.");
    }
} /* End of SFLOW_OM_AttachSystemResources */

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
    UI32_T *seglen_p)
{
    *segid_p  = SYSRSC_MGR_SFLOW_OM_SHMEM_SEGID;
    *seglen_p = sizeof(SFLOW_OM_ShmemData_T);
} /* End of SFLOW_OM_GetShMemInfo */

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
void SFLOW_OM_Init(void)
{
    UI32_T i = 0, j = 0;

    for (i = 0; i < sizeof(shmem_data_p->receiver_table)/sizeof(shmem_data_p->receiver_table[0]); ++i)
    {
        SFLOW_OM_InitReceiverEntry(i+1);
    }

    for (i = 0; i < sizeof(shmem_data_p->port_sampling_table)/sizeof(shmem_data_p->port_sampling_table[0]); ++i)
    {
        for (j = 0; j < sizeof(shmem_data_p->port_sampling_table[0])/sizeof(shmem_data_p->port_sampling_table[0][0]); ++j)
        {
            SFLOW_OM_InitSamplingEntry(i+1, j+1);
        }
    }

    for (i = 0; i < sizeof(shmem_data_p->port_polling_table)/sizeof(shmem_data_p->port_polling_table[0]); ++i)
    {
        for (j = 0; j < sizeof(shmem_data_p->port_polling_table[0])/sizeof(shmem_data_p->port_polling_table[0][0]); ++j)
        {
            SFLOW_OM_InitPollingEntry(i+1, j+1);
        }
    }

    return;
}  /* End of SFLOW_OM_Init */

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
    UI32_T receiver_index)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->receiver_table[receiver_index-1], 0,
        sizeof(shmem_data_p->receiver_table[0]));
    shmem_data_p->receiver_table[receiver_index-1].receiver_index = receiver_index;
    shmem_data_p->receiver_table[receiver_index-1].address.type = L_INET_ADDR_TYPE_IPV4;
    shmem_data_p->receiver_table[receiver_index-1].udp_port = SFLOW_MGR_RECEIVER_SOCK_PORT;
    shmem_data_p->receiver_table[receiver_index-1].datagram.max_size = SYS_DFLT_SFLOW_MAX_RECEIVER_DATAGRAM_SIZE;
    shmem_data_p->receiver_table[receiver_index-1].datagram.version = SYS_DFLT_SFLOW_RECEIVER_DATAGRAM_VERSION;
    shmem_data_p->receiver_table[receiver_index-1].datagram.data_p = shmem_data_p->receiver_table[receiver_index-1].datagram.data_ar;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return;
} /* End of SFLOW_OM_InitReceiverEntry */

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
    char *owner_name_p)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    strncpy(shmem_data_p->receiver_table[receiver_index-1].owner_name,
        owner_name_p, sizeof(shmem_data_p->receiver_table[receiver_index-1].owner_name)-1);
    shmem_data_p->receiver_table[receiver_index-1].owner_name[sizeof(shmem_data_p->receiver_table[receiver_index-1].owner_name)-1] = '\0';
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetReceiverOwner */

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
    char *owner_name_p)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    strncpy(owner_name_p,
        shmem_data_p->receiver_table[receiver_index-1].owner_name,
        SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN);
    owner_name_p[SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN] = '\0';
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_GetReceiverOwner */

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
    UI32_T timeout)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->receiver_table[receiver_index-1].timeout = timeout;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetReceiverTimeout */

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
    L_INET_AddrIp_T *address_p)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    memcpy(&shmem_data_p->receiver_table[receiver_index-1].address, address_p,
    sizeof(shmem_data_p->receiver_table[receiver_index-1].address));
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetReceiverDestination */

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
    UI32_T udp_port)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->receiver_table[receiver_index-1].udp_port = udp_port;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetReceiverSockPort */

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
    UI32_T max_datagram_size)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->receiver_table[receiver_index-1].datagram.max_size = max_datagram_size;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetReceiverMaxDatagramSize */

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
    UI32_T datagram_version)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->receiver_table[receiver_index-1].datagram.version = datagram_version;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetReceiverDatagramVersion */

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
    UI32_T datagram_sequence)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->receiver_table[receiver_index-1].datagram.sequence = datagram_sequence;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetReceiverDatagramSequence */

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
    UI32_T sample_number)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->receiver_table[receiver_index-1].datagram.sample_number = sample_number;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetReceiverDatagramSampleNumber */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_SetReceiverDatagramData
 *-------------------------------------------------------------------------
 * PURPOSE  : Set data to receiver datagram.
 * INPUT    : receiver_index -- receiver index
 *            data_p         -- data arrary pointer
 *            data_len       -- data length of byte
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_OM_RETURN_VALUE_T
SFLOW_OM_SetReceiverDatagramData(
    UI32_T receiver_index,
    UI32_T *data_p,
    UI32_T data_len)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    memcpy(shmem_data_p->receiver_table[receiver_index-1].datagram.data_p,
        data_p, data_len);
    shmem_data_p->receiver_table[receiver_index-1].datagram.data_p += data_len / sizeof(UI32_T);
    shmem_data_p->receiver_table[receiver_index-1].datagram.data_len += data_len / sizeof(UI32_T);
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetReceiverDatagramData */

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
    UI32_T sample_number)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->receiver_table[receiver_index-1].datagram.sample_number = sample_number;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetReceiverSampleNumber */

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
    UI32_T *data_len_p)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    *data_len_p = shmem_data_p->receiver_table[receiver_index-1].datagram.data_len;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_GetReceiverDataLength */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_OM_ClearReceiverDatagram
 *-------------------------------------------------------------------------
 * PURPOSE  : Clear receiver datagram.
 * INPUT    : receiver_index  -- receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_OM_RETURN_SUCCESS/SFLOW_OM_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void
SFLOW_OM_ClearReceiverDatagram(
    UI32_T receiver_index)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->receiver_table[receiver_index-1].datagram.sample_number = 0;
    memset(shmem_data_p->receiver_table[receiver_index-1].datagram.data_ar, 0,
        sizeof(shmem_data_p->receiver_table[receiver_index-1].datagram.data_ar));
    shmem_data_p->receiver_table[receiver_index-1].datagram.data_p = shmem_data_p->receiver_table[receiver_index-1].datagram.data_ar;
    shmem_data_p->receiver_table[receiver_index-1].datagram.data_len = 0;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return;
} /* End of SFLOW_OM_ClearReceiverDatagram */

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
    SFLOW_OM_Receiver_T *receiver_entry_p)
{
    memcpy(receiver_entry_p,
        &shmem_data_p->receiver_table[receiver_entry_p->receiver_index-1],
        sizeof(SFLOW_OM_Receiver_T));

    return SFLOW_OM_RETURN_SUCCESS;
} /* end of SFLOW_OM_GetReceiverEntry */

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
    UI32_T instance_id)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->port_sampling_table[ifindex-1][instance_id-1], 0,
        sizeof(shmem_data_p->port_sampling_table[0][0]));
    shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].ifindex = ifindex;
    shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].instance_id = instance_id;
    shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].max_header_size = SYS_DFLT_SFLOW_MAX_SAMPLING_HEADER_SIZE;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return;
} /* End of SFLOW_OM_InitSamplingEntry */

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
    UI32_T rate)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].sampling_rate = rate;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetSamplingRate */

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
    UI32_T max_header_size)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].max_header_size = max_header_size;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetSamplingMaxHeaderSize */

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
    UI32_T receiver_index)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].receiver_index = receiver_index;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetSamplingReceiverIndex */

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
    char *receiver_owner_name_p)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    strncpy(shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].receiver_owner_name,
        receiver_owner_name_p,
        sizeof(shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].receiver_owner_name)-1);
    shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].receiver_owner_name[sizeof(shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].receiver_owner_name)-1] = '\0';
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetSamplingReceiverOwnerName */

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
    UI32_T sample_pool)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].sample_pool = sample_pool;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetSamplingSamplePool */

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
    UI32_T sample_sequence)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_sampling_table[ifindex-1][instance_id-1].sample_sequence = sample_sequence;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetSamplingSampleSequence */

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
    SFLOW_OM_Sampling_T *sampling_entry_p)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    memcpy(sampling_entry_p,
        &shmem_data_p->port_sampling_table[sampling_entry_p->ifindex-1][sampling_entry_p->instance_id-1],
        sizeof(SFLOW_OM_Sampling_T));
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_GetSamplingEntry */

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
    UI32_T instance_id)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    memset(&shmem_data_p->port_polling_table[ifindex-1][instance_id-1], 0,
        sizeof(shmem_data_p->port_polling_table[0][0]));
    shmem_data_p->port_polling_table[ifindex-1][instance_id-1].ifindex = ifindex;
    shmem_data_p->port_polling_table[ifindex-1][instance_id-1].instance_id = instance_id;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return;
} /* End of SFLOW_OM_InitPollingEntry */

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
    UI32_T interval)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_polling_table[ifindex-1][instance_id-1].polling_interval = interval;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetPollingInterval */

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
    UI32_T receiver_index)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_polling_table[ifindex-1][instance_id-1].receiver_index = receiver_index;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetPollingReceiverIndex */

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
    char *receiver_owner_name_p)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    strncpy(shmem_data_p->port_polling_table[ifindex-1][instance_id-1].receiver_owner_name,
        receiver_owner_name_p,
        sizeof(shmem_data_p->port_polling_table[ifindex-1][instance_id-1].receiver_owner_name)-1);
    shmem_data_p->port_polling_table[ifindex-1][instance_id-1].receiver_owner_name[sizeof(shmem_data_p->port_polling_table[ifindex-1][instance_id-1].receiver_owner_name)-1] = '\0';
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetPollingReceiverOwnerName */

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
    UI32_T sample_sequence)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_polling_table[ifindex-1][instance_id-1].sample_sequence = sample_sequence;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetPollingSampleSequence */

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
    UI32_T last_polling_time)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_polling_table[ifindex-1][instance_id-1].last_polling_time = last_polling_time;
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_SetPollingLastPollingTime */

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
    SFLOW_OM_Polling_T *polling_entry_p)
{
    SFLOW_OM_ENTER_CRITICAL_SECTION();
    memcpy(polling_entry_p,
        &shmem_data_p->port_polling_table[polling_entry_p->ifindex-1][polling_entry_p->instance_id-1],
        sizeof(SFLOW_OM_Polling_T));
    SFLOW_OM_LEAVE_CRITICAL_SECTION();

    return SFLOW_OM_RETURN_SUCCESS;
} /* End of SFLOW_OM_GetPollingEntry */

#endif /* #if (SYS_CPNT_SFLOW == TRUE) */

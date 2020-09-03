#ifndef _DEV_NICDRV_TYPE_H
#define _DEV_NICDRV_TYPE_H

#include "sysfun.h"
#include "dev_nicdrv_gateway.h"

#define DEV_NICDRV_TIME_EVENT                  BIT_0     /* all event that DEV_NICDRV_Task */
#define DEV_NICDRV_NOTIFY_DEQUEUE_EVENT        BIT_1     /* will reveive                   */
#define DEV_NICDRV_NOTIFY_MSGQ_EVENT           BIT_2
#define DEV_NICDRV_UPDATE_QUEUE_SUSPEND_EVENT  BIT_3

#define DEV_NICDRV_BC_PKT                    DEV_NICDRV_GATEWAY_BC_PKT
#define DEV_NICDRV_MC_PKT                    DEV_NICDRV_GATEWAY_MC_PKT
#define DEV_NICDRV_CPU_PKT                   DEV_NICDRV_GATEWAY_CPU_PKT
#define DEV_NICDRV_IUC_ETHERNET_TYPE         DEV_NICDRV_GATEWAY_IUC_ETHERNET_TYPE

/* Define upper bound for packets that announced to upper layers which haven't free.
 * When reach this upper bound, NIC task will sleep DEV_NICDRV_MAX_ANNOUNCE_SLLEP_TIME
 * ticks before dequeue and announce next packet
 */
/*#define DEV_NICDRV_BUFFER_ANNOUNCE_LIMIT       8*/
#define DEV_NICDRV_PER_QUEUE_BUFFER_ANNOUNCE_LIMIT  16
#define DEV_NICDRV_MAX_ANNOUNCE_SLEEP_TIME          10

/* WFQ queue size always set to equal this value. If SYS_ADPT_NIC_WFQ_QUEUE_SIZE is different
 * to this constant, WFQ queue number will be smaller than SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE_FOR_CPU_PORT
 */
#define DEV_NICDRV_WFQ_QUEUE_SIZE              SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_PER_PRIORITY

/* DEV_NICDRV_BASE_WFQ_QUEUE_NUM is the number of queue for that exclude ISC and STKTPLG
 * SYS_ADPT_NIC_WFQ_QUEUE_SIZE is required to be power of 2
 */
#define DEV_NICDRV_WFQ_BASE_QUEUE_NUM          ((SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE_FOR_CPU_PORT* \
                                                 SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_PER_PRIORITY)/ \
                                                 DEV_NICDRV_WFQ_QUEUE_SIZE)

#if (SYS_CPNT_STACKING == TRUE)
#define DEV_NICDRV_MAX_NBR_OF_WFQ              (DEV_NICDRV_WFQ_BASE_QUEUE_NUM + 2)      /* 2 stand for ISC and STKTPLG packet */
#define DEV_NICDRV_MAX_NBR_OF_PRIORITY         (SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE_FOR_CPU_PORT + 2) /* 2 stand for ISC and STKTPLG packet */
#else
#define DEV_NICDRV_MAX_NBR_OF_WFQ              DEV_NICDRV_WFQ_BASE_QUEUE_NUM
#define DEV_NICDRV_MAX_NBR_OF_PRIORITY         SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE_FOR_CPU_PORT
#endif

typedef enum
{
    DEV_NICDRV_MEM_ALLOC_SUCCESS = 0, /* call L_MPOOL_AllocateBlock success             */
    DEV_NICDRV_MEM_ALLOC_FAIL,        /* call L_MPOOL_AllocateBlock fail                */
    DEV_NICDRV_MEM_ALLOC_DENIED,      /* mem allocate denied when rate limit high       */
    DEV_NICDRV_MEM_FREE_SUCCESS,      /* call L_MPOOL_FreeBlock success                 */
    DEV_NICDRV_MEM_FREE_FAIL,         /* call L_MPOOL_FreeBlock fail                    */
    DEV_NICDRV_RECV_PACKET_TOTAL,     /* all call in DEV_NICDRV_RecvPacket              */
    DEV_NICDRV_REPLY_ENQUEUE_SUCCESS,       
    DEV_NICDRV_REPLY_DEQUEUE_SUCCESS,
    DEV_NICDRV_ENQUEUE_SUCCESS,       /* all call DEV_NICDRV_Enqueue success            */
    DEV_NICDRV_DEQUEUE_SUCCESS,       /* all call DEV_NICDRV_Dequeue success            */
    DEV_NICDRV_UNIT_PORT_ERROR,       /* transform device_id/rx_port to unit/port error */
    DEV_NICDRV_BACKDOOR_COUNTER_MAX
}BackdoorCounter_T;

typedef struct
{
    UI32_T  enqueueTotal;     /* in DEV_NICDRV_ENQUEUE function, after get packet priority */
    UI32_T  queueFullDrop;   /* drop because of WFQ queue full                            */
    UI32_T  enqueueFail;      /* Enqueue into WFQ queue fail(in case WFQ is not full)      */
    UI32_T  ageOutDrop;      /* packet drop because of age out in WFQ                     */
    UI32_T  current;           /* packet drop because of age out in WFQ                     */
    UI32_T  peak;              /* max number of packets of each priority                    */
}PriorityTrafficInfo_T;

typedef struct 
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    UI16_T priorityToQueue[DEV_NICDRV_MAX_NBR_OF_PRIORITY]; /* mapping priority ID to WFQ queue index */
    UI16_T queueToPriority[DEV_NICDRV_MAX_NBR_OF_WFQ];      /* mapping WFQ queue index to priority ID */
    UI32_T packetsInQueue[DEV_NICDRV_MAX_NBR_OF_WFQ];       /* actual number of packets in each WFQ queue, exclude the age out label */
    UI32_T packetAnnouncedInQueue[DEV_NICDRV_MAX_NBR_OF_WFQ];
    UI32_T packetAnnouncedThresholdInQueue[DEV_NICDRV_MAX_NBR_OF_WFQ];
    BOOL_T queueSuspend[DEV_NICDRV_MAX_NBR_OF_WFQ];
    UI32_T totalPacketAnnounced;                            /* total number of packets that announce to upper layer */
    UI32_T devNicdrvThreadId;
    UI32_T fragmentBufferInUse;
    UI32_T backdoorCounter[DEV_NICDRV_BACKDOOR_COUNTER_MAX];
    PriorityTrafficInfo_T priorityTraffic[DEV_NICDRV_MAX_NBR_OF_PRIORITY + 1]; /*  the extra one (last one) is for ISC reply packet messageQ (not in WFQ) */
} DEV_NICDRV_Shmem_Data_T;

#endif


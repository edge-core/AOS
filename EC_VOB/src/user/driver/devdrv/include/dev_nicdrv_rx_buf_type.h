#ifndef _DEV_NICDRV_RX_BUF_TYPE_H
#define _DEV_NICDRV_RX_BUF_TYPE_H

#include "sys_adpt.h"
#include "l_pt.h"

#define DEV_NICDRV_TOTAL_NBR_OF_PACKET_BUFFER        ((SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE_FOR_CPU_PORT* \
                                                       SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_PER_PRIORITY)+ \
                                                       SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_ISC + \
                                                       SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_ISC_REPLY + \
                                                       SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_ISC_STKTPLG + \
                                                       (SYS_ADPT_MAX_NBR_OF_CHIP_PER_UNIT* \
                                                        SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_1_ASIC_DMA))

#define DEV_NICDRV_TOTAL_RX_BUFFER_SIZE              (DEV_NICDRV_TOTAL_NBR_OF_PACKET_BUFFER* \
                                                      SYS_ADPT_NIC_MAX_RX_BUF_SIZE_PER_PACKET)

#define DEV_NICDRV_TOTAL_RESERVED_RX_BUFFER_SIZE     (sizeof(L_PT_ShMem_Descriptor_T)+DEV_NICDRV_TOTAL_RX_BUFFER_SIZE)

#endif

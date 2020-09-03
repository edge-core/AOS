/* -------------------------------------------------------------------------------------
 * FILE NAME : dev_nicdrv.h
 * -------------------------------------------------------------------------------------
 * PURPOSE   : A header file for the definition of functions in dev_nicdrv.c
 *             This package provides an interface above dev_nicdrv_gateway which support
 *             WFQ (Weighted Fair Queue or Multiple Priority Queue,MQ) when processing
 *             received packets from dev_nicdrv_gateway.
 *
 * NOTES      : 1. Use l_mq.c and l_mq.h to implement the WFQ enqueue and dequeue
 *              2. There is one DEV_NICDRV_Task to carry out the work of dequeue and
 *                 dispatch packets to upper layer protocol callback functions
 *              3. Support packet input rate control according to their cpu priority,
 *                 when packet arrival rate is high only higher cpu priority packet will
 *                 be served (in order to realize storm protection feature)
 *              4. Support each priority queue packet total number control and overall
 *                 packet number control (drop packet if exceed their limit)
 *              5. Control the total number of packet that announced to upper layer
 *              6. Drop packet that stay in queue too long which haven't been dequeue
 *                 and announce to upper layer protocol callback functions
 *
 *
 * MODIFICATION HISTORY :
 * Modifier     Date         Version     Description
 * -------------------------------------------------------------------------------------
 * kh_shi       28-06-2005   V1.0        First Created
 *
 * -------------------------------------------------------------------------------------
 * Copyright(C)                         Accton Technology Corp. 2005
 * -------------------------------------------------------------------------------------
 */

#ifndef DEV_NICDRV_H
#define DEV_NICDRV_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "dev_nicdrv_gateway.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define DEV_NICDRV_MSGBUF_TYPE_SIZE sizeof(union DEV_NICDRV_IPCMsg_Type_U)

/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for calculation of ipc msg_buf size based on structure name
 * used in L2MUX_OM_IPCMsg_T.data
 */
#define DEV_NICDRV_GET_MSGBUFSIZE(struct_name) \
        (DEV_NICDRV_MSGBUF_TYPE_SIZE + sizeof(struct struct_name))

/* DATA TYPE DECLARATIONS
 */

typedef enum DEV_NICDRV_CallBackProtocolType_E
{
    DEV_NICDRV_PROTOCOL_IUC = 0,
    DEV_NICDRV_PROTOCOL_LAN,
    DEV_NICDRV_PROTOCOL_MAX
} DEV_NICDRV_CallBackProtocolType_T;


/* definitions of command in DEV_NICDRV which will be used in ipc message
 */
enum
{
    DEV_NICDRV_IPC_SEND_PACKET_TO_PORT,
    DEV_NICDRV_IPC_SEND_PACKET_BY_PORT,
    DEV_NICDRV_IPC_SEND_PACKET_BY_PORT_LIST,
    DEV_NICDRV_IPC_SEND_LOOPBACK_PACKET_BY_PORT_LIST,
    DEV_NICDRV_IPC_SEND_XBAR_PACKET_BY_PORT_LIST,
    DEV_NICDRV_IPC_SEND_PACKET_BY_VID,
    DEV_NICDRV_IPC_SEND_IUC_PACKET,
    DEV_NICDRV_IPC_SEND_PACKET_TO_PIPELINE,    
    DEV_NICDRV_IPC_FREE_RX_PACKET/* anzhen.zheng, 2009-05-20, 18:01:58 */
};

enum
{
    DEV_NICDRV_RESULT_OK=0,
    DEV_NICDRV_RESULT_FAIL
};

typedef struct
{
    union DEV_NICDRV_IPCMsg_Type_U
    {
        UI32_T cmd;         /* for sending IPC request */
        BOOL_T result_bool; /* for response of boolean value */
        UI32_T result;      /* for response            */
    } type;
    
    union
    {
        struct DEV_NICDRV_IPCMsg_SendPacketToPort_Data_S
        {
            UI32_T dst_unit; 
            UI32_T dst_port;
            BOOL_T is_tagged;
            UI16_T tag_info;
            I32_T  packet;
            UI32_T length;                                 
            UI32_T priority;
            void   *callback_function;
            void   *cookie;
        }SendPacketToPort_data;
        
        struct DEV_NICDRV_IPCMsg_SendPacketByPort_Data_S
        {
            UI32_T unit;
            UI32_T port;
            BOOL_T is_tagged;
            I32_T  packet;
            UI32_T length;                                
            UI32_T priority;
            void   *callback_function;
            void   *cookie;
        }SendPacketByPort_data;    

        struct DEV_NICDRV_IPCMsg_SendPacketByPortList_Data_S
        {
            UI32_T port_count;
            UI8_T  port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            UI8_T  untagged_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            I32_T  packet;
            UI32_T length; 
            UI32_T priority;
            void   *callback_function;
            void   *cookie;
        }SendPacketByPortList_data;

        struct DEV_NICDRV_IPCMsg_SendPacketByVid_Data_S
        {
            UI32_T tag_info;
            void   *packet;
            UI32_T length;                                
            UI32_T priority;
            void   *callback_function;
            void   *mref_handle_p;
        }SendPacketByVid_data;

        struct DEV_NICDRV_IPCMsg_SendIUCPacket_Data_S
        {
            I32_T  mref_handle_offset;
            UI16_T dst_unit;
            UI8_T  uplink_downlink;
            UI32_T priority;
        }SendIUCPacket_data;
        
        struct DEV_NICDRV_IPCMsg_FreeRxPacket_Data_S
        {
            void *buf;
        }FreeRxPacket_data;
        struct DEV_NICDRV_IPCMsg_SendPacketToPipeline_Data_S
        {
            UI32_T in_port;
            I32_T  packet;
            UI32_T length;                                
            void   *callback_function;
            void   *cookie;
        }SendPacketToPipeline_data;   
    } data;
} DEV_NICDRV_IPCMsg_T;


/* MACRO FUNCTION DECLARATIONS
 */
#if 0
#ifndef INCLUDE_DIAG
#define DEV_NICDRV_SendPacketToPort(dst_unit, dst_port, is_tagged, tag_info, packet, length, \
                                    cos_value, callback_function, cookie) \
        DEV_NICDRV_GATEWAY_SendPacketToPort(dst_unit, dst_port, is_tagged, tag_info, packet, length, \
                                    cos_value, callback_function, cookie)

#define DEV_NICDRV_SendPacketByVid(tag_info, packet, length, cos_value, callback_function, mem_ref)\
        DEV_NICDRV_GATEWAY_SendPacketByVid(tag_info, packet, length, cos_value, callback_function, mem_ref)

#define DEV_NICDRV_SendIUCPacket(mem_ref, dst_unit, uplink_downlink, priority) \
        DEV_NICDRV_GATEWAY_SendIUCPacket(mem_ref, dst_unit, uplink_downlink, priority)
#endif

#define DEV_NICDRV_SendPacketByPort(unit, port, is_tagged, packet, length, cos_value, callback_function, cookie) \
        DEV_NICDRV_GATEWAY_SendPacketByPort(unit, port, is_tagged, packet, length, cos_value, callback_function, cookie)

#define DEV_NICDRV_SendPacketByPortList(port_count, port_list, untagged_port_list, packet, length, \
                                        cos_value, callback_function, cookie) \
        DEV_NICDRV_GATEWAY_SendPacketByPortList(port_count, port_list, untagged_port_list, packet, \
                                                length, cos_value, callback_function, cookie)

#define DEV_NICDRV_SendLoopBackPacketByPortList(port_count, port_list, untagged_port_list, packet, \
                                                length, cos_value, callback_function, cookie) \
        DEV_NICDRV_GATEWAY_SendLoopBackPacketByPortList(port_count, port_list, untagged_port_list, packet, \
                                                        length, cos_value, callback_function, cookie)

#define DEV_NICDRV_SendXbarPacketByPortList(port_count, port_list, untagged_port_list, packet, \
                                            length, cos_value, callback_function, cookie) \
        DEV_NICDRV_GATEWAY_SendXbarPacketByPortList(port_count, port_list, untagged_port_list, packet, \
                                                    length, cos_value, callback_function, cookie)

#define DEV_NICDRV_ReceivePacketReason(reason) DEV_NICDRV_GATEWAY_ReceivePacketReason(reason)

#define DEV_NICDRV_CosValue(val) DEV_NICDRV_GATEWAY_CosValue(val)

#endif

/* Get the CPU queue (priority) from the reason pass up. It is Chip dependent.
 */
#define DEV_NICDRV_GET_COS_FROM_REASON(reason)     (((reason)>>28) & 0x7)
#define DEV_NICDRV_MAKE_REASON_FROM_COS(cos_value) ((cos_value)<<28)


/* EXPORTED FUNCTION DECLARACTIONS
 */
/* ----------------------------------------------------------------------------------
 * FUNCTION : DEV_NICDRV_Init
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Initialize the DEV_NICDRV and DEV_NICDRV_GATEWAY
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void DEV_NICDRV_Init(void);

/* FUNCTION NAME: DEV_NICDRV_Create_InterCSC_Relation
 *---------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration
 *          operations.
 *---------------------------------------------------------------------
 * INPUT:   None
 * RETURN:  None.
 *---------------------------------------------------------------------
 * NOTE:
 */
void DEV_NICDRV_Create_InterCSC_Relation(void);

/* ----------------------------------------------------------------------------------
 * FUNCTION : DEV_NICDRV_CreateTask
 * ----------------------------------------------------------------------------------
 * PURPOSE  : create the DEV_NICDRV_Task
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void DEV_NICDRV_CreateTask(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DEV_NICDRV_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for l2mux mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DEV_NICDRV_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

BOOL_T DEV_NICDRV_GetDriverGroupModuleProcessTime();

BOOL_T DEV_NICDRV_SetDriverGroupModuleProcessTime(UI32_T module,UI32_T time);

/* exported for a driver_proc to support backdoor from linux shell without using Simba/CLI  backdoor
 */
void DEV_NICDRV_Backdoor(void);

#endif


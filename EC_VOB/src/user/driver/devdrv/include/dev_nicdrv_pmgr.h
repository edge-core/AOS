#ifndef DEV_NICDRV_PMGR_H
#define DEV_NICDRV_PMGR_H

#include "sys_type.h"
#include "l_mm.h"

/* EXPORTED FUNCTION DECLARACTIONS
 */
BOOL_T DEV_NICDRV_PMGR_InitiateProcessResource(void);

BOOL_T DEV_NICDRV_PMGR_SendPacketToPort(UI32_T dst_unit, 
                                    UI32_T dst_port,
                                    BOOL_T is_tagged,
                                    UI16_T tag_info,
                                    void   *packet,
                                    UI32_T length,                                 
                                    UI32_T cos_value,
                                    void   *callback_function, 
                                    void   *cookie);


BOOL_T DEV_NICDRV_PMGR_SendPacketByPort(UI32_T unit, 
                                   UI32_T port,
                                   BOOL_T is_tagged,
                                   void   *packet,
                                   UI32_T length,                                 
                                   UI32_T cos_value,
                                   void   *callback_function, 
                                   void   *cookie);


BOOL_T DEV_NICDRV_PMGR_SendPacketByPortList(UI32_T port_count, 
                                       UI8_T *port_list, 
                                       UI8_T *untagged_port_list, 
                                       void *packet, 
                                       UI32_T length, 
                                       UI32_T cos_value,
                                       void   *callback_function, 
                                       void   *cookie);

BOOL_T DEV_NICDRV_PMGR_SendPacketByVid(UI32_T tag_info,
                                          void   *packet,
                                          UI32_T length,                                 
                                          UI32_T cos_value,
                                          void   *callback_function, 
                                          void   *mref_handle_p);

BOOL_T DEV_NICDRV_PMGR_SendIUCPacket(L_MM_Mref_Handle_T *mref_handle_p,
                                     UI16_T dst_unit,
                                     UI8_T uplink_downlink, 
                                     UI32_T priority);
/* Added by anzhen.zheng, 2009-05-20, 18:05:35 */
BOOL_T DEV_NICDRV_PMGR_FreeRxPacket(void   *buf);

BOOL_T DEV_NICDRV_PMGR_SendPacketPipeline(UI32_T in_port, 
                                   void   *packet,
                                   UI32_T length,                                 
                                   void   *cookie);

#endif


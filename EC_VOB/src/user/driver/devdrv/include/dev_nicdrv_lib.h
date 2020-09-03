#ifndef DEV_NICDRV_LIB_H
#define DEV_NICDRV_LIB_H

#include "sys_type.h"

#define DEV_NICDRV_LIB_MAKE_REASON_FROM_COS(cos_value) ((cos_value)<<28)
#define BROADCOM_NIC

#define DEV_NIC_MDATA_HASH_COMP(frame)  (((frame)->da.addr[3] ^ (frame)->da.addr[4] ^ \
                            (frame)->da.addr[5] ^ (frame)->sa.addr[5] ^ (frame)->sa.addr[4] ^ (frame)->sa.addr[5]) \
                            & (DEV_NIC_MDATA_VLAN_RESERVE_BIT_POSITION_FROM - 1))
                            
/*control the array size for dnic_mdata, now it is 128/32 = 4*/
#define DEV_NIC_MDATA_HASH_SIZE (128)
/*reserve the last element in dnic_mdata for vlan, therefore the  hash value cannot be greater than 32*3=96 bits*/
#define DEV_NIC_MDATA_VLAN_RESERVE_BIT_POSITION_FROM (96)
  
#define DEV_NIC_MDATA_WIDTH (sizeof(UI32_T) * 8)/*the width of unit, each unit indicate 32 elements*/
#define DEV_NIC_MDATA_COUNT (DEV_NIC_MDATA_HASH_SIZE/DEV_NIC_MDATA_WIDTH)/*the count of unit*/

#define DEV_NICDRV_RX_PACKET_INFO_MAGIC_WORD    0x19191919

struct dev_nic_mdata
{
  UI32_T dnic_mdata[DEV_NIC_MDATA_COUNT];
};

typedef struct
{
    UI32_T magic;       /* DEV_NICDRV_RX_PACKET_INFO_MAGIC_WORD */
    UI64_T rx_timestamp;
    UI32_T ingress_vlan;
    struct dev_nic_mdata *mhash;
    void *cookie;
    BOOL_T is_single_inner_tagged;
    BOOL_T rx_is_tagged;
    BOOL_T pkt_is_truncated;
    BOOL_T workaround_vlan_xlate;   /* TRUE if need to perform vlan xlate by sw */
} DEV_NICDRV_RxPacketInfo_T;

UI32_T DEV_NICDRV_LIB_ReceivePacketReason(UI32_T reason);

UI32_T DEV_NICDRV_LIB_CosValue(UI32_T val); 

UI32_T DEV_NICDRV_LIB_PktClassId(UI32_T pkt_type);

#endif


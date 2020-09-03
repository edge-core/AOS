/*
 * MODULE NAME: SWCTRL_OM.h
 *
 * PURPOSE: To store and manage switch information

 *
 * NOTES:	Due to swctrl do not have OM before, only new or necessary function will join OM now.
 *
 *  History :   
 *      Date            Modifier    Reason
 *  ---------------------------------------------------------------------
 *      06-11-2007       Simon      init: Lan.c need to get QinQ information
 * ----------------------------------------------------------------------
 * Copyright(C)        Accton Techonology Corporation, 2007
 * ======================================================================
 */
#ifndef _SWCTRL_OM_H_
#define _SWCTRL_OM_H_

/**********************************/
/* INCLUDE FILE DECLARATIONS      */
/**********************************/
#include "sys_type.h"
#include "sys_dflt.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysrsc_mgr.h"
#include "swdrv_type.h"

/**********************************/
/* NAME	CONSTANT DECLARATIONS     */
/**********************************/
#define SWCTRL_OM_UPORT_TO_IFINDEX(unit, port)         ( (unit-1) * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + port  )
#define SWCTRL_OM_IFINDEX_TO_UNIT(ifindex)             ( ((UI32_T)((ifindex-1)/SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))+1 )
#define SWCTRL_OM_IFINDEX_TO_PORT(ifindex)             ( ifindex - (SWCTRL_OM_IFINDEX_TO_UNIT(ifindex)-1)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT )
#define SWCTRL_OM_IS_LPORT(ifindex)                    ( (ifindex != 0) && (ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT) ) 

#define SWCTRL_OM_NONE_MIRROR_MODE 0x0  /* none */
#define SWCTRL_OM_PORT_MIRROR_MODE 0x1  /* port mirror */
#define SWCTRL_OM_VLAN_MIRROR_MODE 0x2  /* vlan mirror */
#define SWCTRL_OM_MAC_MIRROR_MODE  0x4  /* mac-address mirror */
#define SWCTRL_OM_ACL_MIRROR_MODE  0x8  /* acl-based mirror for ingress packet of the specified acl ingress port */
#define SWCTRL_OM_ACL_EGRESS_MIRROR_MODE 0x10  /* acl-based mirror for egress packet of the specified acl ingress port */

#define SWCTRL_OM_SfpInfo_T                SWDRV_TYPE_SfpInfo_T
#define SWCTRL_OM_SfpDdmInfo_T             SWDRV_TYPE_SfpDdmInfo_T
#define SWCTRL_OM_SfpDdmInfoMeasured_T     SWDRV_TYPE_SfpDdmInfoMeasured_T
#define SWCTRL_OM_SfpDdmThreshold_T        SWDRV_TYPE_SfpDdmThreshold_T
#define SWCTRL_OM_SfpDdmThresholdInteger_T SWDRV_TYPE_SfpDdmThresholdInteger_T
#define SWCTRL_OM_SfpDdmThresholdEntry_T   SWDRV_TYPE_SfpDdmThresholdEntry_T
#define SWCTRL_OM_SfpDdmThresholdStatus_T  SWDRV_TYPE_SfpDdmThresholdStatus_T
#define SWCTRL_OM_SfpEntry_T               SWDRV_TYPE_SfpEntry_T
#define SWCTRL_OM_SfpDdmEntry_T            SWDRV_TYPE_SfpDdmEntry_T

#if (SYS_CPNT_HASH_SELECTION == TRUE)
typedef enum    /*Bitmap*/
{
    SWCTRL_OM_HASH_SEL_SERVICE_ECMP   =  0x00000001,
    SWCTRL_OM_HASH_SEL_SERVICE_TRUNK  =  0x00000002,
} SWCTRL_OM_HashSelService_T;

typedef struct
{
    SWCTRL_OM_HashSelService_T   ref_service_bmp; /* Bitmap to know the block is referenced by which services */

    DEV_SWDRV_HashSelection_L2_T      pkt_l2;
    DEV_SWDRV_HashSelection_IPv4_T    pkt_ipv4;
    DEV_SWDRV_HashSelection_IPv6_T    pkt_ipv6;
} SWCTRL_OM_HashSelBlockInfo_T;

typedef enum
{
    SWCTRL_OM_HASH_PACKET_TYPE_L2,
    SWCTRL_OM_HASH_PACKET_TYPE_IPV4,
    SWCTRL_OM_HASH_PACKET_TYPE_IPV6,
}SWCTRL_OM_HashPacketType_T;

typedef struct
{
    SWCTRL_OM_HashPacketType_T    type;

    union {
        DEV_SWDRV_HashSelection_L2_T     l2;
        DEV_SWDRV_HashSelection_IPv4_T   ipv4;        
        DEV_SWDRV_HashSelection_IPv6_T   ipv6;
    } sel;
} SWCTRL_OM_HashSelection_T;   /*For UI to set hash selection*/
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

enum {
    SWCTRL_OM_QINQ_DBG_TRACE = 0x01,
    SWCTRL_OM_QINQ_DBG_ERROR = 0x02,
};

/**********************************/
/* DATA TYPE DECLARATIONS         */
/**********************************/


#if (SYS_CPNT_WRED == TRUE)
typedef struct
{
  UI32_T min[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE];
  UI32_T max[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE];
  UI32_T drop[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE];
  UI32_T ecn[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE];
  BOOL_T valid[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE];
  BOOL_T all_queue;
}SWCTRL_OM_RandomDetect_T;
#endif

typedef struct
{
#if(SYS_CPNT_WRED == TRUE)
   SWCTRL_OM_RandomDetect_T random_detect;
#endif
}SWCTRL_OM_PortInfo_T;

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
typedef struct
{
    UI32_T   addr_entry_index;                  /* mirror address index */
    UI8_T    mac_addr[SYS_ADPT_MAC_ADDR_LEN];   /* mac address */
    BOOL_T   is_valid;                          /* address entry is validated */
}__attribute__((packed, aligned(1)))SWCTRL_MacAddrMirrorEntry_T;
#endif /* End of #if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) */

typedef struct
{
    BOOL_T                             present;
    BOOL_T                             trap_enable;
    BOOL_T                             auto_mode;
    SWCTRL_OM_SfpInfo_T                sfp_info;
    SWCTRL_OM_SfpDdmInfo_T             sfp_ddm_info;
    SWCTRL_OM_SfpDdmThreshold_T        sfp_ddm_threshold;
    SWCTRL_OM_SfpDdmThresholdStatus_T  sfp_ddm_threshold_status;
} SWCTRL_OM_PortSfpInfo_T;

/*************************************/
/* EXPORTED SUBPROGRAM DECLARACTION  */
/*************************************/
/*-----------------------------------------------------------------------------
 * ROUTINE NAME: SWCTRL_OM_InitiateSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will init system resource
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_OM_InitiateSystemResources(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: SWCTRL_OM_AttachSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_OM_AttachSystemResources(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: SWCTRL_OM_GetShMemInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *-----------------------------------------------------------------------------
 */
void SWCTRL_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

void SWCTRL_OM_Init();
void SWCTRL_OM_ResetAll();
void SWCTRL_OM_ResetPortInfo(UI32_T ifindex);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: SWCTRL_OM_ResetPortSfpInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: Reset SFP info
 * INPUT:   unit, sfp_index
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-----------------------------------------------------------------------------
 */
void SWCTRL_OM_ResetPortSfpInfo(UI32_T unit, UI32_T sfp_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_TrunkFollowUserPortAttributes
 * -------------------------------------------------------------------------
 * FUNCTION: This function will copy config of first member to trunk
 * INPUT   : trunk_ifindex -- the port index of trunk
             tm_ifindex    -- the port index of trunk member
 * OUTPUT  : None
 * RETURN  : None
 * Note    :
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_TrunkFollowUserPortAttributes(UI32_T trunk_ifindex, UI32_T tm_ifindex);

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetMacAddrMirrorEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set mirror MAC address entry
 * INPUT   : *mac_addr     -- mac addess
             is_add        -- TRUE  : add address entry  
                           -- FALSE : remove address entry   
 * OUTPUT  : None
 * RETURN  : TRUE: success , FALSE: fail
 * NOTE    : SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant 
 *           allow maximal mac based mirror entries
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_SetMacAddrMirrorEntry(UI8_T *mac_addr, BOOL_T is_add);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetExactMacAddrMirrorEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get exactly mirror entry by MAC-address 
 * INPUT   : addr_entry->mac_addr   - mac address
 * OUTPUT  : addr_entry->addr_entry_index
 *           addr_entry->mac_addr
 *           addr_entry->is_valid
 *
 * RETURN  : TRUE: success , FALSE: fail
 * NOTE    : We shall returns exactly mac_address entry and index
 *           addr_entry->addr_entry_index
 *           addr_entry->mac_addr
 *
 *           SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant
 *           allow maximal mac based mirror entries
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetExactMacAddrMirrorEntry(SWCTRL_MacAddrMirrorEntry_T *addr_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetNextMacAddrMirrorEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get next mac address of mirror entry
 * INPUT   : *addr_entry -- mac address entry 
 * OUTPUT  : *addr_entry -- mac entry
 *           addr_entry->addr_entry_index
 *           addr_entry->mac_addr    
 *           addr_entry->is_valid
 *
 * RETURN  : TRUE: success , FALSE: fail
 * NOTE    : if the addr_entry_index = 0
 *           , means returns first mac-address entry 
 *           otherwise, we shall verify current index id and mac-address
 *
 *           SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant 
 *           allow maximal mac based mirror entries
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetNextMacAddrMirrorEntry(SWCTRL_MacAddrMirrorEntry_T *addr_entry);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetNextMacAddrMirrorEntryForSnmp
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get next mac address of mirror entry (for SNMP)
 * INPUT   : *mac           -- mac address 
 * OUTPUT  : *mac           -- mac address
 *           *ifindex_dest  -- dest port
 *
 * RETURN  : TRUE: success , FALSE: fail
 * NOTE    : if the mac_addr = 00-00-00-00-00-00
 *           , means returns first mac-address entry 
 *           otherwise, we shall use current mac-address to find next one
 *           and no sort request for this API
 *
 *           SNMP must use the field(s) of mib entry to act as key(s), so 
 *           SWCTRL_POM_GetNextMacAddrMirrorEntry is not suitable for the 
 *           usage of SNMP. And this api is created for the reason.
 *
 *           SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant 
 *           allow maximal mac based mirror entries 
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetNextMacAddrMirrorEntryForSnmp(UI8_T *mac, UI32_T *ifindex_dst);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_IsExistedMacAddrMirrorEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check if exist the mirror entry (mac+dst-port)
 * INPUT   : *mac_addr      -- mac address 
 *           ifindex_dest   -- dest port
 * OUTPUT  : None
 *
 * RETURN  : TRUE: exist , FALSE: un-exist
 * NOTE    : SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant 
 *           allow maximal mac based mirror entries 
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_IsExistedMacAddrMirrorEntry(UI8_T *mac, UI32_T ifindex_dst);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetMacAddrMirrorCnt
 * -------------------------------------------------------------------------
 * FUNCTION: This function will returns current mirror counter
 * INPUT   : nbr
 * OUTPUT  : nbr  -- return how many mac-address mirroring counter
 *
 * RETURN  : None
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_GetMacAddrMirrorCnt(UI32_T *nbr);
#endif /* end of #if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetMirrorConfigMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set or remove mirror mode 
 * INPUT   : mode       -- SWCTRL_OM_PORT_MIRROR_MODE
 *                         SWCTRL_OM_VLAN_MIRROR_MODE
 *                         SWCTRL_OM_MAC_MIRROR_MODE 
 *           is_active  -- TRUE : add 
 *                         FALSE: remove
 * OUTPUT  : None
 * RETURN  : None
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_SetMirrorConfigMode(UI32_T mode, BOOL_T is_active); 

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetMirrorConfigMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will returns current mirror configuration
 * INPUT   : mode
 * OUTPUT  : mode       -- SWCTRL_OM_PORT_MIRROR_MODE |
 *                         SWCTRL_OM_VLAN_MIRROR_MODE |
 *                         SWCTRL_OM_MAC_MIRROR_MODE 
 *
 * RETURN  : None
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_GetMirrorConfigMode(UI32_T *mode);

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) || (SYS_CPNT_VLAN_MIRROR == TRUE) || (SYS_CPNT_ACL_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetCommonMirrorDestPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set common mirror destination port.
 * INPUT   : ifindex       -- destination port
 * OUTPUT  : None
 * RETURN  : None
 * Note    : ifindex 0 indicates NULL.
 *           used for VLAN/MAC/ACL based mirror
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_SetCommonMirrorDestPort(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetCommonMirrorDestPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will returns common mirror destination port.
 * INPUT   : ifindex
 * OUTPUT  : ifindex      -- returns common mirror destination port
 * RETURN  : None
 * Note    : ifindex 0 indicates NULL.
 *           used for VLAN/MAC/ACL based mirror
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_GetCommonMirrorDestPort(UI32_T *ifindex);
#endif /*#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)||(SYS_CPNT_VLAN_MIRROR == TRUE)||(SYS_CPNT_ACL_MIRROR == TRUE)*/

#if (SYS_CPNT_ACL_MIRROR_EGRESS == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetCommonMirrorTxDestPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set common mirror tx destination port.
 * INPUT   : ifindex       -- destination port
 * OUTPUT  : None
 * RETURN  : None
 * Note    : ifindex 0 indicates NULL.
 *           used for VLAN/MAC/ACL based mirror
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_SetCommonMirrorTxDestPort(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetCommonMirrorTxDestPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will returns common mirror tx destination port.
 * INPUT   : ifindex
 * OUTPUT  : ifindex      -- returns common mirror destination port
 * RETURN  : None
 * Note    : ifindex 0 indicates NULL.
 *           used for VLAN/MAC/ACL based mirror
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_GetCommonMirrorTxDestPort(UI32_T *ifindex);
#endif /*#if (SYS_CPNT_ACL_MIRROR_EGRESS == TRUE)*/

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get PSE check status
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: PSE check enabled, FALSE: PSE check disabled
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetPSECheckStatus(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set PSE check status
 * INPUT   : pse_check_status - TRUE:enabled, FALSE:disabled
 * OUTPUT  : None
 * RETURN  : TRUE: PSE check enabled, FALSE: PSE check disabled
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_SetPSECheckStatus(BOOL_T pse_check_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPortSfpPresent
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp present status
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_OM_GetPortSfpPresent(UI32_T unit, UI32_T sfp_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpPresent
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp present status
 *
 * INPUT   : unit
 *           sfp_index
 *           is_present
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpPresent(UI32_T unit, UI32_T sfp_index, BOOL_T is_present);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_info_p
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpInfo_T *sfp_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 *           sfp_info_p
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpInfo_T *sfp_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_info_p
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfo_T *sfp_ddm_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 *           sfp_ddm_info_p
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfo_T *sfp_ddm_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPortSfpDdmInfoMeasured
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm measured eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_info_measured_p
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmInfoMeasured(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmInfoMeasured
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp ddm measured eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 *           sfp_ddm_info_measured_p
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmInfoMeasured(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfoMeasured_T *sfp_ddm_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmTrapEnable
 * -------------------------------------------------------------------------
 * FUNCTION: Enable/Disable SFP ddm threshold alarm trap
 * INPUT   : unit
 *           sfp_index
 *           trap_enable -- TRUE/FALSE
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmTrapEnable(UI32_T unit, UI32_T sfp_index, BOOL_T trap_enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPortSfpDdmTrapEnable
 * -------------------------------------------------------------------------
 * FUNCTION: Get statu of SFP ddm threshold alarm trap
 * INPUT   : unit
 *           sfp_index
 *           trap_enable_p
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmTrapEnable(UI32_T unit, UI32_T sfp_index, BOOL_T *trap_enable_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmThresholdAutoMode
 * -------------------------------------------------------------------------
 * FUNCTION: Enable/Disable SFP ddm threshold auto mode
 * INPUT   : unit
 *           sfp_index
 *           auto_mode -- TRUE/FALSE
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThresholdAutoMode(UI32_T unit, UI32_T sfp_index, BOOL_T auto_mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmThresholdAutoMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm threshold
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : auto_mode_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmThresholdAutoMode(UI32_T unit, UI32_T sfp_index, BOOL_T *auto_mode_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmThresholdDefault
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold to default
 * INPUT   : unit
 *           sfp_index
 *           threshold_type -- which threshold_type
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThresholdDefault(UI32_T unit, UI32_T sfp_index, UI32_T threshold_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp ddm threshold
 *
 * INPUT   : unit
 *           sfp_index
 *           threshold_type
 *           val(float)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThreshold(UI32_T unit, UI32_T sfp_index, UI32_T threshold_type, float val);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmThresholdForWeb
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp ddm threshold for web which sets four val at once
 *
 * INPUT   : unit
 *           sfp_index
 *           threshold_type
 *           high_alarm(float)
 *           high_warning(float)
 *           low_warning(float)
 *           low_alarm(float)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThresholdForWeb(UI32_T unit, UI32_T sfp_index, UI32_T threshold_type, float high_alarm,
     float high_warning, float low_warning, float low_alarm);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmThresholdAll
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp ddm threshold
 *
 * INPUT   : unit
 *           sfp_index
 *           sfp_ddm_threshold_p
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThresholdAll(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmThreshold_T *sfp_ddm_threshold_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm threshold
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_threshold_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmThreshold(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmThreshold_T *sfp_ddm_threshold_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmThresholdStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp ddm threshold status
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_threshold_status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThresholdStatus(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmThresholdStatus_T *sfp_ddm_threshold_status_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmThresholdStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm threshold status
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_threshold_status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmThresholdStatus(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmThresholdStatus_T *sfp_ddm_threshold_status_p);

#if (SYS_CPNT_HASH_SELECTION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_IsHashSelBindForTheService
 * -------------------------------------------------------------------------
 * FUNCTION: Check if there is hash-selection list bound for the service
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 * OUTPUT  : list_index - the block index bound for the service
 * RETURN  : TRUE  - bind
 *           FALSE - not bind
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_IsHashSelBindForTheService(
    SWCTRL_OM_HashSelService_T service, 
    UI8_T *list_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_BindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: add service reference of the hash-selection list
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_BindHashSelForService(
    SWCTRL_OM_HashSelService_T service, 
    UI8_T list_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_UnBindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: remove service reference of the hash-selection list
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_UnBindHashSelForService(
    SWCTRL_OM_HashSelService_T service, 
    UI8_T list_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_AddHashSelection
 * -------------------------------------------------------------------------
 * FUNCTION: add hash-selection field
 * INPUT   : list_index - the index of hash-selection list
 *           selection_p - hash-selection field
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If the hash-selection list has been bound, then it can't be modified
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_AddHashSelection(
    UI8_T list_index , 
    SWCTRL_OM_HashSelection_T *selection_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_RemoveHashSelection
 * -------------------------------------------------------------------------
 * FUNCTION: remove hash-selection field
 * INPUT   : list_index - the index of hash-selection list
 *           selection_p - hash-selection field
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If the hash-selection list has been bound, then it can't be modified
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_RemoveHashSelection(
    UI8_T list_index , 
    SWCTRL_OM_HashSelection_T *selection_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetHashBlockInfo
 * -------------------------------------------------------------------------
 * FUNCTION: get hash-selection block info
 * INPUT   : list_index - the index of hash-selection list
 * OUTPUT  : block_info_p - the hash-selection block info
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetHashBlockInfo(
    UI8_T list_index , 
    SWCTRL_OM_HashSelBlockInfo_T *block_info_p);
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

#if(SYS_CPNT_WRED == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetRandomDetect
 * -------------------------------------------------------------------------
 * FUNCTION: Set ecn marking percentage
 * INPUT   : ifindex      - which port to set
 *           queue_id     - which queue to set
 *           min          - min threshold
 *           max          - max threshold
 *           drop         - drop threshold
 *           valid        - entry is valid
 * OUTPUT  : None
 * RETURN  : TRUE - success
 *           FALSE- fail
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_SetRandomDetect(UI32_T ifindex, 
                                 I8_T queue_id,
                                 UI32_T min,
                                 UI32_T max,
                                 UI32_T drop,
                                 UI32_T ecn,
                                 BOOL_T valid);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetRandomDetect
 * -------------------------------------------------------------------------
 * FUNCTION: get ecn marking percentage
 * INPUT   : lport      - which port to set
 * RETURN  : random_detect_p - how percentage 
 * OUTPUT  : None
 * NOTE    : defualt is disable and percentage is 0
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetRandomDetect(UI32_T lport, SWCTRL_OM_RandomDetect_T *random_detect_p);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_CpyRandomDetectValue
 * -------------------------------------------------------------------------
 * FUNCTION: get ecn marking percentage
 * INPUT   : src_lport - which port to copy from
 *           target_lport - which port to copy to
 * RETURN  : 
 * OUTPUT  : None
 * NOTE    : 
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_CpyRandomDetectValue(UI32_T src_lport, UI32_T target_lport);
#endif
/*end of swctrl_om.h*/
#endif


/*---------------------------------------------------------------------
 * File_Name : HRDRV_ADPT.H
 *
 * Purpose   : 
 * 
 * Copyright(C)      Accton Corporation, 2002, 2003
 *
 * Note    : 
 *---------------------------------------------------------------------
 */

#ifndef _HRDRV_ADPT_H
#define _HRDRV_ADPT_H

#include "hrdrv.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"

enum 
{
    HRDRV_ADPT_SERVICE_STATIC , 
    HRDRV_ADPT_SERVICE_DYNAMIC
};
   

#define HRDRV_ADPT_MAX_NBR_OF_RID               50 
#define HRDRV_ADPT_MAX_LEN_OF_FILTERf           72


/*********************************/
/* Hardware Dependent Definition */
/*********************************/

/* Broadcom XGS CHIP : 5690 / 5691 / 5692   */
#define HRDRV_ADPT_GPIC_MAX_RULE_CNT            128

#define HRDRV_ADPT_GPIC_MAX_MASK_CNT            16
#define HRDRV_ADPT_MAX_NBR_OF_MASK_PER_PIC      16 

#define HRDRV_ADPT_MAX_NBR_OF_PORT_PER_PIC      1 
#define HRDRV_ADPT_MAX_NBR_OF_PIC               12

#define HRDRV_ADPT_RULE_CNT_PER_SOC             1536
#define HRDRV_ADPT_MASK_CNT_PER_SOC             128 


/*
ARP + RARP 
*/ 
     
#define HRDRV_DO_TRAP_ARP_MASK_CNT              1
#define HRDRV_DO_TRAP_ARP_RULE_CNT              2

#define HRDRV_DO_TRAP_ARP_SERVICE               HRDRV_ADPT_SERVICE_STATIC
#define HRDRV_ADPT_TRAP_ARP_PRI                 0 


/*
IGMP + PIM + DVMRP
*/
#define HRDRV_DO_TRAP_IGMP_MASK_CNT             3
#define HRDRV_DO_TRAP_IGMP_RULE_CNT             6

#define HRDRV_ADPT_TRAP_IGMP_SERVICE            HRDRV_ADPT_SERVICE_STATIC
#define HRDRV_ADPT_TRAP_IGMP_PRI                1 
#define HRDRV_ADPT_TRAP_IGMP_OPTION_1_PRI       2
#define HRDRV_ADPT_TRAP_IGMP_OPTION_2_PRI       3


#if((SYS_CPNT_DHCP_CLIENT == TRUE) || (SYS_CPNT_DHCP_SERVER == TRUE))    
    #define HRDRV_DO_TRAP_DHCP_SERVER_CNT       1    
#else
    #define HRDRV_DO_TRAP_DHCP_SERVER_CNT       0
#endif

#if(SYS_CPNT_DHCP_CLIENT == TRUE)
    #define HRDRV_DO_TRAP_DHCP_CLIENT_CNT       1
#else
    #define HRDRV_DO_TRAP_DHCP_CLIENT_CNT       0 
#endif

#if(SYS_CPNT_DHCP == TRUE)
    #define HRDRV_DO_TRAP_DHCP_RULE_CNT         (HRDRV_DO_TRAP_DHCP_SERVER_CNT + HRDRV_DO_TRAP_DHCP_CLIENT_CNT)
#else
    #define HRDRV_DO_TRAP_DHCP_RULE_CNT         0    
#endif
    
/*
DHCP share one mask
*/    
#if(SYS_CPNT_DHCP == TRUE)
    #define HRDRV_DO_TRAP_DHCP_MASK_CNT         1
#else
    #define HRDRV_DO_TRAP_DHCP_MASK_CNT         0 
#endif
         
#if(SYS_CPNT_INGRESS_RATE_LIMIT == TRUE)           
    #define HRDRV_DO_RATE_LIMIT_MASK_CNT        1
    #define HRDRV_DO_RATE_LIMIT_RULE_CNT        8
    #define HRDRV_DO_GPIC_RATE_LIMIT_RULE_CNT   1
#else
    #define HRDRV_DO_RATE_LIMIT_MASK_CNT        0
    #define HRDRV_DO_RATE_LIMIT_RULE_CNT        0
    #define HRDRV_DO_GPIC_RATE_LIMIT_RULE_CNT   0
#endif

#if(SYS_CPNT_COS == TRUE)
    #define HRDRV_DO_COS_TOS_MASK_CNT           1
    #define HRDRV_DO_COS_TOS_RULE_CNT           8 
    #define HRDRV_DO_COS_DSCP_MASK_CNT          1
    #define HRDRV_DO_COS_DSCP_RULE_CNT          64
    #define HRDRV_DO_COS_TCP_UDP_PORT_RULE_CNT  8
    #define HRDRV_DO_COS_TCP_UDP_PORT_MASK_CNT  1 
#else
    #define HRDRV_DO_COS_TOS_MASK_CNT           0
    #define HRDRV_DO_COS_TOS_RULE_CNT           0 
    #define HRDRV_DO_COS_DSCP_MASK_CNT          0
    #define HRDRV_DO_COS_DSCP_RULE_CNT          0
    #define HRDRV_DO_COS_TCP_UDP_PORT_RULE_CNT  0
    #define HRDRV_DO_COS_TCP_UDP_PORT_MASK_CNT  0    
#endif

#if((SYS_CPNT_RIP == TRUE) || (SYS_CPNT_OSPF == TRUE))
    #define HRDRV_DO_TRAP_ROUTING_MASK_CNT      1
#else
    #define HRDRV_DO_TRAP_ROUTING_MASK_CNT      0
#endif
    
//#if(SYS_CPNT_RIP == TRUE)
   //#define HRDRV_DO_TRAP_RIP_RULE_CNT          1 
//#else
    #define HRDRV_DO_TRAP_RIP_RULE_CNT          0 
//#endif  

#if(SYS_CPNT_OSPF == TRUE)
    #define HRDRV_DO_TRAP_OSPF_RULE_CNT         2
#else
    #define HRDRV_DO_TRAP_OSPF_RULE_CNT         0 
#endif

     #define HRDRV_DO_TRAP_DVMRP_RULE_CNT       0 

#if(SYS_CNPT_PIM == TRUE)
    #define HRDRV_DO_TRAP_PIM_RULE_CNT          1
#else
    #define HRDRV_DO_TRAP_PIM_RULE_CNT          0 
#endif

#define HRDRV_ADPT_DO_TRAP_DHCP_PRI             4

#define HRDRV_ADPT_TRAP_ROUTING_SERVICE     HRDRV_ADPT_SERVICE_STATIC
#define HRDRV_ADPT_TRAP_ROUTING_PRI         5

#define HRDRV_ADPT_COS_TOS_SERVICE          HRDRV_ADPT_SERVICE_STATIC 
#define HRDRV_ADPT_COS_TOS_PRI              6

#define HRDRV_ADPT_COS_DSCP_SERVICE         HRDRV_ADPT_SERVICE_STATIC 
#define HRDRV_ADPT_COS_DSCP_PRI             7

#define HRDRV_ADPT_COS_TCP_UDP_PORT_SERVICE  HRDRV_ADPT_SERVICE_STATIC 
#define HRDRV_ADPT_COS_TCP_UDP_PORT_PRI      8 

#if(SYS_CPNT_ACL == TRUE)

#define HRDRV_ADPT_DO_IP_INGRESS_ACL_BASE_PRI           0x20
#define HRDRV_ADPT_DO_IP_INGRESS_ACL_DEFAULT_PRI        0x10

#define HRDRV_ADPT_DO_MAC_INGRESS_ACL_DEFAULT_PRI       0x11
#define HRDRV_ADPT_DO_MAC_INGRESS_ACL_BASE_PRI          0x30

#define HRDRV_ADPT_DO_IP_EGRESS_ACL_BASE_PRI            0x40
#define HRDRV_ADPT_DO_IP_EGRESS_ACL_DEFAULT_PRI         0x40

#define HRDRV_ADPT_DO_MAC_EGRESS_ACL_BASE_PRI           0x50
#define HRDRV_ADPT_DO_MAC_EGRESS_ACL_DEFAULT_PRI   0x50


#define HRDRV_ADPT_DO_MAC_ACL_PRI                       10

    #if(SYS_ADPT_ADD_DEFAULT_INGRESS_IP_MASK == TRUE)
        #define HRDRV_ADPT_DEFAULT_INGRESS_IP_MASK 1
    #else
        #define HRDRV_ADPT_DEFAULT_INGRESS_IP_MASK 0
    #endif    
    
    #if(SYS_ADPT_ADD_DEFAULT_EGRESS_IP_MASK == TRUE)
        #define HRDRV_ADPT_DEFAULT_EGRESS_IP_MASK 1
    #else
        #define HRDRV_ADPT_DEFAULT_EGRESS_IP_MASK 0
    #endif
        
    #if(SYS_ADPT_ADD_DEFAULT_INGRESS_MAC_MASK == TRUE)
        #define HRDRV_ADPT_DEFAULT_INGRESS_MAC_MASK 1
    #else
        #define HRDRV_ADPT_DEFAULT_INGRESS_MAC_MASK 0
    #endif
            
    #if(SYS_ADPT_ADD_DEFAULT_EGRESS_MAC_MASK == TRUE)
        #define HRDRV_ADPT_DEFAULT_EGRESS_MAC_MASK  1
    #else
        #define HRDRV_ADPT_DEFAULT_EGRESS_MAC_MASK  0 
    #endif
 
#define   HRDRV_ADPT_DEFAULT_ACL_MASK (\
          HRDRV_ADPT_DEFAULT_INGRESS_IP_MASK + \
          HRDRV_ADPT_DEFAULT_EGRESS_IP_MASK + \
          HRDRV_ADPT_DEFAULT_INGRESS_MAC_MASK +\
          HRDRV_ADPT_DEFAULT_EGRESS_MAC_MASK)

#else
#define   HRDRV_ADPT_DEFAULT_ACL_MASK  0
#endif

#define HRDRV_ADPT_RATE_LIMIT_SERVICE       HRDRV_ADPT_SERVICE_STATIC 
#define HRDRV_ADPT_RATE_LIMIT_PRI           0x10000000 

#define HRDRV_ADPT_GPIC_PERMANENT_RULE   (\
                            HRDRV_DO_TRAP_ARP_RULE_CNT+                        \
                            HRDRV_DO_TRAP_IGMP_RULE_CNT +                      \
                            HRDRV_DO_COS_TOS_RULE_CNT +                        \
                            HRDRV_DO_COS_DSCP_RULE_CNT +                       \
                            HRDRV_DO_COS_TCP_UDP_PORT_RULE_CNT+                \
                            HRDRV_DO_TRAP_DHCP_RULE_CNT +                      \
                            HRDRV_DO_TRAP_RIP_RULE_CNT+                        \
                            HRDRV_DO_TRAP_OSPF_RULE_CNT+                       \
                            HRDRV_DO_TRAP_DVMRP_RULE_CNT+                      \
                            HRDRV_DO_TRAP_PIM_RULE_CNT+                        \
                            HRDRV_DO_GPIC_RATE_LIMIT_RULE_CNT+                 \
                            HRDRV_DO_COS_TCP_UDP_PORT_RULE_CNT)                    
                            
#define HRDRV_ADPT_GPIC_PERMANENT_MASK   (\
                            HRDRV_DO_TRAP_ARP_MASK_CNT+                        \
                            HRDRV_DO_TRAP_IGMP_MASK_CNT +                      \
                            HRDRV_DO_COS_TOS_MASK_CNT +                        \
                            HRDRV_DO_COS_DSCP_MASK_CNT +                       \
                            HRDRV_DO_COS_TCP_UDP_PORT_MASK_CNT+                \
                            HRDRV_DO_TRAP_ROUTING_MASK_CNT +                   \
                            HRDRV_DO_RATE_LIMIT_MASK_CNT   +                   \
                            HRDRV_DO_TRAP_DHCP_MASK_CNT +                      \
                            HRDRV_ADPT_DEFAULT_ACL_MASK)
                            
/*******************************/
/* PROJECT DEPEDENT DEFINITION */
/*******************************/

#define HRDRV_ADPT_MAX_NBR_OF_SOC               SYS_ADPT_MAX_NBR_OF_CHIP_PER_UNIT

                             
#endif
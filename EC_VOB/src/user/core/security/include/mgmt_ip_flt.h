/* Project Name: New Feature
 * File_Name   : Mgmt_ip_flt.h
 * Purpose     :
 *
 * 2003/01/14  : Ahten Chen    Create this file
 * 2007/12/27  : Shumin.Wang   Modify this file
 *
 * Copyright(C)      Accton Corporation, 2003
 *
 * Note    : Designed for new platform (Mercury_DC)
 */
#ifndef _MGMT_IP_FLT_H
#define _MGMT_IP_FLT_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "l_inet.h"
#include "sysrsc_mgr.h"

#define MGMT_IP_FLT_STATUS_ENABLE      1L                
#define MGMT_IP_FLT_STATUS_DISABLE     2L  
#define MGMT_IP_FLT_WEB MGMT_IP_FLT_HTTP

#define MGMT_IP_FLT_BLOCK_CACHE_SIZE	5

/* TYPE DECLARATIONS
 */
typedef struct
{
    L_INET_AddrIp_T  start_ipaddress; //key
    L_INET_AddrIp_T  end_ipaddress;
}MGMT_IP_FLT_IpFilter_T;

typedef struct
{
    UI32_T              alive_time;     /* (absolute) tick */
    L_INET_AddrIp_T     inet;           /* key */
    BOOL_T              available;
}MGMT_IP_FLT_BlockCacheEntry_T;

typedef struct MGMT_IP_FLT_BlockCache_S
{
    MGMT_IP_FLT_BlockCacheEntry_T entries[MGMT_IP_FLT_BLOCK_CACHE_SIZE];
}MGMT_IP_FLT_BlockCache_T;

/* NAMING CONSTANT
 */ 
typedef enum 
{
    MGMT_IP_FLT_STATUS_VALID = 1,
    MGMT_IP_FLT_STATUS_INVALID
}MGMT_IP_FLT_STATUS_T;

typedef enum 
{
    MGMT_IP_FLT_HTTP = 1,
    MGMT_IP_FLT_SNMP,
    MGMT_IP_FLT_TELNET,
    MGMT_IP_FLT_HTTPS,
    MGMT_IP_FLT_SSH,

    MGMT_IP_FLT_MIN_MODE = MGMT_IP_FLT_HTTP,
    MGMT_IP_FLT_MAX_MODE = MGMT_IP_FLT_SSH
}MGMT_IP_FLT_MODE_T;

typedef enum 
{
    MGMT_IP_FLT_OK = 1,
    MGMT_IP_FLT_NOT_IN_MASTER_MODE,
    MGMT_IP_FLT_IP_RANGE_START_BIG_THEN_END,
    MGMT_IP_FLT_IP_RANGE_OVERLAP,
    MGMT_IP_FLT_IP_RANGE_NOT_EXISTED,
    MGMT_IP_FLT_IP_RANGE_NOT_THE_SAME,
    MGMT_IP_FLT_ENTRY_IS_FULL,
    MGMT_IP_FLT_IP_IS_INVALID,
    MGMT_IP_FLT_MODE_IS_INVALID,
    MGMT_IP_FLT_STATUS_IS_INVALID,
    MGMT_IP_FLT_FAIL
}MGMT_IP_FLT_ERROR_T;

typedef enum 
{
    MGMT_IP_FLT_GETNEXTRUNNINGIPFILTER = 1,
    MGMT_IP_FLT_SETIPFILTER,
    MGMT_IP_FLT_DELETEIPFILTER,
    MGMT_IP_FLT_ISVALIDIPFILTERADDRESS,
    MGMT_IP_FLT_GETIPFILTER,
    MGMT_IP_FLT_GETNEXTIPFILTER,
    MGMT_IP_FLT_CHECK_PARAMETERS
}MGMT_IP_FLT_FUN_T;
#if 0
/* definitions of command which will be used in ipc message
 */
enum
{
    MGMT_IP_FLT_MGR_IPCCMD_SNMP_GETIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_HTTPS_GETIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_SSH_GETIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_WEB_GETIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_TELNET_GETIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_WEB_GETNEXTIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_SNMP_GETNEXTIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_TELNET_GETNEXTIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_HTTPS_GETNEXTIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_SSH_GETNEXTIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_GETNEXTRUNNINGIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_SETIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_SETIPFILTERSTATUS,
    MGMT_IP_FLT_MGR_IPCCMD_SETIPFILTERENTRY,
    MGMT_IP_FLT_MGR_IPCCMD_DELETEIPFILTER,
    MGMT_IP_FLT_MGR_IPCCMD_ISVALIDIPFILTERADDRESS,
    MGMT_IP_FLT_MGR_IPCCMD_SETIPFLTSTATUS,
    MGMT_IP_FLT_MGR_IPCCMD_GETIPFLTSTATUS,
    MGMT_IP_FLT_MGR_IPCCMD_GETRUNNINGFLTSTATUS,
    MGMT_IP_FLT_MGR_IPCCMD_SETIPFILTERSUBNET,
    MGMT_IP_FLT_MGR_IPCCMD_DELETEIPFILTERSUBNET,
    MGMT_IP_FLT_MGR_IPCCMD_GETNEXTIPMETHODSUBNET,
    MGMT_IP_FLT_MGR_IPCCMD_GETTRUSTEDCOUNT,
    MGMT_IP_FLT_MGR_IPCCMD_ISIPALLOWEDTOADD       ,
    MGMT_IP_FLT_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC
};
#endif
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------
 * ROUTINE NAME - IPfilter_MGR_Init                 
 *-----------------------------------------------------------
 * FUNCTION: To initialize the MGR module of IP_FILTER
 * INPUT   : None.                                            
 * OUTPUT  : None.                                            
 * RETURN  : None.                   
 * NOTE    : None.
 *----------------------------------------------------------*/
void MGMT_IP_FLT_Init(void);
/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_Initiate_System_Resources                                               
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources               
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : Invoked by root.c()                                        
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_Initiate_System_Resources (void);

/* FUNCTION NAME: MGMT_IP_FLT_GetShMemInfo
 *----------------------------------------------------------------------------------
 * PURPOSE: Get share memory info
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void MGMT_IP_FLT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: MGMT_IP_FLT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for COS_OM in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void MGMT_IP_FLT_AttachSystemResources(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_Create_InterCSC_Relation                                               
 *------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_Create_InterCSC_Relation(void);

/*-----------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_Create_InterCSC_Relation                 
 *-----------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None.                                            
 * OUTPUT  : None.                                            
 * RETURN  : None.                   
 * NOTE    : None.
 *----------------------------------------------------------*/
void MGMT_IP_FLT_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_SetTransitionMode                      
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the transition mode
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                        
 * NOTE    : None
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_SetTransitionMode(void);
/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_EnterTransitionMode                      
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize all system resource     
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                        
 * NOTE    : None
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_EnterTransitionMode(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_EnterMasterMode                      
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services     
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                        
 * NOTE    : None
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_EnterMasterMode(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - MGMT_IP_FLT_INIT_EnterSlaveMode                                   
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable address monitor services          
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None                                                         
 *------------------------------------------------------------------------*/
void MGMT_IP_FLT_INIT_EnterSlaveMode(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Snmp_GetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the specified IP filter 
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- ip filter info.
 * RETURN: One of MGMT_IP_FLT_MGR_ERROR_E
 * NOTES: 1. The ip filter receiver can only be accessed by SNMP.
 *        2. The total number of ip filter receivers supported by the system 
 *           is defined by sys_adpt.h.
 *        3. By default, there is no ip filter receiver configued in the system.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Snmp_GetIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Https_GetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the specified IP filter 
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_T
 * NOTES: 1. The ip filter receiver can only be accessed by CLI.
 *        2. The total number of ip filter receivers supported by the system 
 *           is defined by sys_adpt.h.
 *        3. By default, there is no ip filter receiver configued in the system.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Https_GetIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Ssh_GetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the specified IP filter 
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_T
 * NOTES: 1. The ip filter receiver can only be accessed by CLI.
 *        2. The total number of ip filter receivers supported by the system 
 *           is defined by sys_adpt.h.
 *        3. By default, there is no ip filter receiver configued in the system.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Ssh_GetIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Web_GetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the specified IP filter 
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_T
 * NOTES: 1. The ip filter receiver can only be accessed by WEB.
 *        2. The total number of ip filter receivers supported by the system 
 *           is defined by sys_adpt.h.
 *        3. By default, there is no ip filter receiver configued in the system.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Web_GetIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Telnet_GetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the specified IP filter 
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: 1. ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_T
 * NOTES: 1. The ip filter receiver can only be accessed by CLI.
 *        2. The total number of ip filter receivers supported by the system 
 *           is defined by sys_adpt.h.
 *        3. By default, there is no ip filter receiver configued in the system.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Telnet_GetIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_WEB_GetNextIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the next available IP filter 
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_T
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_WEB_GetNextIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_Snmp_GetNextIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the next available IP filter 
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_T
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Snmp_GetNextIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_TELNET_GetNextIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the next available IP filter 
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_T
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_TELNET_GetNextIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_HTTPS_GetNextIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the next available IP filter 
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_T
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Https_GetNextIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_SSH_GetNextIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the next available IP filter 
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available ip filter info.
 * RETURN: One of MGMT_IP_FLT_ERROR_T
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_Ssh_GetNextIpFilter(MGMT_IP_FLT_IpFilter_T *ip_filter_entry);



/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_GetNextRunningIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          next available entry can be retrieved successfully. Otherwise, 
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:  ip_filter_entry->start_ipaddress - (main key) to specify a unique ip filter address.
 * OUTPUT: ip_filter_entry -- next available non-default ip filter.
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or 
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL.
 * NOTES: 1. This function shall only be invoked by CLI to save the  
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this 
 *           function shall return non-default ip filter.
 * ---------------------------------------------------------------------
 */ 		
UI32_T MGMT_IP_FLT_GetNextRunningIpFilter(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_SetIpFilter
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the status can be successfully 
 *          set to the specified ip filter. Otherwise, false is returned.
 *
 * INPUT: 1. mode: the mode for this ip filter(Web/SNMP/Telnet).
 *           - MGMT_IP_FLT_WEB(1).
 *           - MGMT_IP_FLT_SNMP(2).
 *           - MGMT_IP_FLT_TELNET(3).
 *        2. ip_filter_entry->start_ipaddress   - (main key) to specify a unique ip filter address.
 * OUTPUT: None                                      				
 * RETURN: One of MGMT_IP_FLT_ERROR_T
 * NOTES: 1. This function will create a new ip filter to the system if 
 *           the specified (start_ipaddress) does not exist, 
 *           and total number of ip filter configured is less than 
 *           SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT.
 *        2. This function will update an existed ip address if 
 *           the specified (start_ipaddress) existed already.
 *        3. start/end address can set 0.
 *        4. if only specify start address, this function will automatically
 *           fill end address. 
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_SetIpFilter(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_DeleteIpFilter     							
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the status can be successfully 
 *          delete to the specified ip filter. Otherwise, false is returned.
 * 																		
 * INPUT: 1. mode: the mode for this ip filter(Web/SNMP/Telnet).
 *           - MGMT_IP_FLT_WEB(1).
 *           - MGMT_IP_FLT_SNMP(2).
 *           - MGMT_IP_FLT_TELNET(3).
 *        2. ip_filter_entry->start_ipaddress   - (main key) to specify a unique ip filter address.
 * OUTPUT: None
 * RETURN: One of MGMT_IP_FLT_ERROR_T
 * NOTES: 1. This function will delete an ip filter from system if 
 *           the specified (start_ipaddress) exist.
 *        2. False is returned if the specified (start_ipaddress) does not exist.
 * ---------------------------------------------------------------------
 */
UI32_T MGMT_IP_FLT_DeleteIpFilter(UI32_T mode, MGMT_IP_FLT_IpFilter_T *ip_filter_entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_AddBlockCache
 * ---------------------------------------------------------------------
 * PURPOSE: Add a IP address to block cache
 *
 * INPUT: 1. mode         : mode (HTTP/SNMP/Telnet).
 *        2. inet_p       : IP address
 *        3. alive_second : alive time(second) in block cache
 * OUTPUT: None
 * RETURN: One of MGMT_IP_FLT_ERROR_E
 * NOTES: 1. If the cache full, the IP address have shortest alive time
 *           will be removed.
 * ---------------------------------------------------------------------
 */
BOOL_T MGMT_IP_FLT_AddBlockCache(
                                 UI32_T mode,
                                 const L_INET_AddrIp_T *inet_p,
                                 UI32_T alive_second
                                 );

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MGMT_IP_FLT_IsValidIpFilterAddress
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns MGMT_IP_FLT_OK if the ip address is belong valid 
 *          ip filter group. Otherwise, false is returned.
 *			
 * INPUT: 1. ip_address - ip address
 *        2. mode:
 *           - MGMT_IP_FLT_WEB(1).
 *           - MGMT_IP_FLT_SNMP(2).
 *           - MGMT_IP_FLT_TELNET(3).
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: 1. The function is only used by for checking the ip address 
 *           of the manager is valid or not to access the device.
 *        2. 0.0.0.0 is invalid ip address
 *        3. If do not set any ip filter range, all ip can access device     
 * ---------------------------------------------------------------------
 */
BOOL_T MGMT_IP_FLT_IsValidIpFilterAddress(UI32_T mode ,const L_INET_AddrIp_T *ip_address);

#endif


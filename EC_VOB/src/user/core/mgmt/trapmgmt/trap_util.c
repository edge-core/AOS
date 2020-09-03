/* =====================================================================================
 * FILE	NAME: TRAP_UTIL.c                                                               
 *                                                                                      
 * ABSTRACT:  This file contains the function to send different type of traps. Due to 
 *            possibility of growing amount of trap requested by customer as well as
 *            modification to SNMP version, all API to send trap are move to trap_util.c
 *                                                                                      
 * MODIFICATION	HISOTRY:	                                                            
 *                                                                                      
 * MODIFIER		   DATE		   DESCRIPTION                                              
 * -------------------------------------------------------------------------------------
 * amytu		07-23-2002	   First Create     							            
 *                                                                                      
 * -------------------------------------------------------------------------------------
 * Copyright(C)		   Accton Techonology Corporation 2002                              
 * =====================================================================================
 */
 
#include <envoy/h/buffer.h>
#include <envoy/h/asn1conf.h>
#include <envoy/h/snmpstat.h>
#include <envoy/h/snmpdefs.h>
#include <string.h>
#include "sys_type.h"
#include "sysfun.h"
#include "l_mem.h"  
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_type.h"
#include "trap_event.h"
#include "leaf_es3626a.h"
#include "snmp_mgr.h"
#include "trap1215.h"
#include "trap1493.h"
#include "trap1757.h"
#include "trap_util.h"
#include "trap_mgr.h"
#include "trapEs3626a.h"        
#include "sys_cpnt.h"
#if (SYS_CPNT_EH == TRUE)
#include "syslog_type.h"
#include "sys_module.h"
#include "eh_type.h"
#include "eh_mgr.h"
#endif//end of #if (SYS_CPNT_EH == TRUE)
#if (SYS_CPNT_LLDP == TRUE)
#include "trap_Ieeelldp.h"
#endif

BOOL_T TRAP_UTIL_SendColdStartTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
    /* BODY */
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];    
#endif
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if (crt_coldStart_trap(ebuff,
                                     local_ip,
                                     trap_receiver->trap_dest_community,
                                     strlen(trap_receiver->trap_dest_community),
    	                             system_time) <= 0)
    	    {    	        
#if (SYS_CPNT_EH == TRUE)
    	        EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_coldStart_trap return false");
#endif
                return FALSE;
            } /* End of if */ 	     
            break;
    	case VAL_trapDestVersion_version2:
            if (crt_coldStart_trapv2(ebuff,
                                     trap_receiver->trap_dest_community,
                                     strlen(trap_receiver->trap_dest_community),
                                     0,
    	                             system_time) <= 0)
    	    {
#if (SYS_CPNT_EH == TRUE)
    	        EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_coldStart_trapv2 return false");
#endif
                return FALSE;
            } /* End of if */
    	     
    	    break;
    	default:
#if (SYS_CPNT_EH == TRUE)
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	     return FALSE;
    	     break;
    }       
    return TRUE;     	         
} /* end of TRAP_UTIL_SendColdStartTrap() */
                                
BOOL_T TRAP_UTIL_SendWarmStartTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];    
#endif  
    /* check if we want to send V1 or V2 Trap*/
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if (crt_warmStart_trap(ebuff,
                                     local_ip,
                                     trap_receiver->trap_dest_community,
                                     strlen(trap_receiver->trap_dest_community),
    	                             system_time) <= 0)
    	    {
#if (SYS_CPNT_EH == TRUE)
    	        EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_warmStart_trap return false");
#endif
                return FALSE;
            } /* End of if */
    	    break;
    	case VAL_trapDestVersion_version2:
            if (crt_warmStart_trapv2(ebuff,
                                     trap_receiver->trap_dest_community,
                                     strlen(trap_receiver->trap_dest_community),
                                     0, /*Request ID, need to check what value need 
                                          to bind, we bind 0 first, James*/
    	                             system_time) <= 0)
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_warmStart_trapv2 return false");
#endif
                return FALSE; 
            } /* End of if */
    	    break;
    	default:
#if (SYS_CPNT_EH == TRUE)    	      
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	    return FALSE;
    	    break;
    }
    return TRUE;    
    
} /* end of TRAP_UTIL_SendWarmStartTrap() */

BOOL_T TRAP_UTIL_SendLinkDownTrap( EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256]; 
#endif
    TRAP_EVENT_LinkTrap_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_LinkTrap_T *)trap_data->dynamicData;
    /* check if we want to send V1 or V2 Trap*/               
     switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if (crt_linkDown_trap(ebuff,
                    	  local_ip,
                          trap_receiver->trap_dest_community,
                          strlen(trap_receiver->trap_dest_community),
                          system_time,
                          pDynamicData->ifindex,                        /* INTEGER */
                          (OIDC_T *) &(pDynamicData->instance_ifindex), /* Instance */
                          1)<= 0)                                               /* Instance length */ 
    	    {
#if (SYS_CPNT_EH == TRUE)
    	        EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_linkDown_trap return false");
#endif
    	        return FALSE;
            }
    	    break;
    	case VAL_trapDestVersion_version2:
            if (crt_linkDown_trapV2(ebuff,
                          trap_receiver->trap_dest_community,
                          strlen(trap_receiver->trap_dest_community),
                          0, /*Request ID, we don't know bind what, bind 0 first, James*/
                          system_time,
                          pDynamicData->ifindex,                        /* INTEGER */
                          (OIDC_T *) &(pDynamicData->instance_ifindex), /* Instance */
                          1,
                          pDynamicData->adminstatus,                        /* INTEGER */
                          (OIDC_T *) &(pDynamicData->instance_adminstatus), /* Instance */
                          1,
                          pDynamicData->operstatus,                        /* INTEGER */
                          (OIDC_T *) &(pDynamicData->instance_operstatus), /* Instance */
                          1)<= 0)                                               /* Instance length */ 
    	    {
#if (SYS_CPNT_EH == TRUE)
    	        EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_linkDown_trapv2 return false");
#endif
                return FALSE;
            } /* End of if */
    	    break;
    	default:
#if (SYS_CPNT_EH == TRUE)    	       
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	    return FALSE;
    	    break;
    }
    return TRUE;     
    
} /* end of TRAP_UTIL_SendLinkDownTrap() */



BOOL_T TRAP_UTIL_SendLinkUpTrap(   EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];
#endif
    TRAP_EVENT_LinkTrap_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_LinkTrap_T *)trap_data->dynamicData;
     /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if (crt_linkUp_trap(ebuff,
                        local_ip,
                        trap_receiver->trap_dest_community,
                        strlen(trap_receiver->trap_dest_community),
                        system_time,
                        pDynamicData->ifindex,                        /* INTEGER */
                        (OIDC_T *) &(pDynamicData->instance_ifindex), /* Instance */
                        1) <= 0)                                            /* Instance length */
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_linkUp_trap return false");
#endif
                return FALSE;
            } /* End of if */    
    	    break;
    	case VAL_trapDestVersion_version2:
    	    if (crt_linkUp_trapV2(ebuff,
                        trap_receiver->trap_dest_community,
                        strlen(trap_receiver->trap_dest_community),
                        0, /*Request ID, need to check the value, James*/
                        system_time,
                        pDynamicData->ifindex,                        /* INTEGER */
                        (OIDC_T *) &(pDynamicData->instance_ifindex), /* Instance */
                        1,
                        pDynamicData->adminstatus,                        /* INTEGER */
                        (OIDC_T *) &(pDynamicData->instance_adminstatus), /* Instance */
                        1,
                        pDynamicData->operstatus,                        /* INTEGER */
                        (OIDC_T *) &(pDynamicData->instance_operstatus), /* Instance */
                        1) <= 0)                                            /* Instance length */
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_linkUp_trapv2 return false");
#endif
                return FALSE;
            } /* End of if */                                        
    	    break;
    	default:
#if (SYS_CPNT_EH == TRUE)
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	    return FALSE;
    	    break;
     } /* end of switch */
     
     return TRUE;     
} /* end of TRAP_UTIL_SendLinkUpTrap() */

BOOL_T TRAP_UTIL_SendCraftLinkDownTrap( EBUFFER_T                  *ebuff,
                                   OCTET_T                    *local_ip,
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256]; 
#endif
    TRAP_EVENT_LinkTrap_T *pDynamicData;
    pDynamicData=(TRAP_EVENT_LinkTrap_T *)trap_data->dynamicData;
    /* check if we want to send V1 or V2 Trap*/
     switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if (crt_craftLinkDown_trap(ebuff,
                          local_ip,
                          trap_receiver->trap_dest_community,
                          strlen(trap_receiver->trap_dest_community),
                          system_time)<= 0)
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_linkDown_trap return false");
#endif
                return FALSE;
            }
            break;
        case VAL_trapDestVersion_version2:
            if (crt_craftLinkDown_trapV2(ebuff,
                          trap_receiver->trap_dest_community,
                          strlen(trap_receiver->trap_dest_community),
                          0, /*Request ID, we don't know bind what, bind 0 first, James*/
                          system_time)<= 0)
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_linkDown_trapv2 return false");
#endif
                return FALSE;
            } /* End of if */
            break;
        default:
#if (SYS_CPNT_EH == TRUE)
            sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
            EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
            return FALSE;
            break;
    }
    return TRUE;
    
} /* end of TRAP_UTIL_SendCraftLinkDownTrap() */

BOOL_T TRAP_UTIL_SendCraftLinkUpTrap(   EBUFFER_T                  *ebuff,
                                   OCTET_T                    *local_ip,
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];
#endif
    TRAP_EVENT_CraftLinkTrap_T *pDynamicData;
    pDynamicData=(TRAP_EVENT_LinkTrap_T *)trap_data->dynamicData;
     /* check if we want to send V1 or V2 Trap */
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if (crt_craftLinkUp_trap(ebuff,
                        local_ip,
                        trap_receiver->trap_dest_community,
                        strlen(trap_receiver->trap_dest_community),
                        system_time) <= 0)
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_linkUp_trap return false");
#endif
                return FALSE;
            } /* End of if */
            break;
        case VAL_trapDestVersion_version2:
            if (crt_craftLinkUp_trapV2(ebuff,
                        trap_receiver->trap_dest_community,
                        strlen(trap_receiver->trap_dest_community),
                        0, /*Request ID, need to check the value, James*/
                        system_time) <= 0)
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_linkUp_trapv2 return false");
#endif
                return FALSE;
            } /* End of if */
            break;
        default:
#if (SYS_CPNT_EH == TRUE)
            sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
            EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
            return FALSE;
            break;
     } /* end of switch */
     
     return TRUE;     
} /* end of TRAP_UTIL_SendCraftLinkUpTrap() */
                                   
BOOL_T TRAP_UTIL_SendAutehticationFailureTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];
#endif
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if (crt_authenticationFailure_trap(ebuff,
                                       local_ip,
                                       trap_receiver->trap_dest_community,
                                       strlen(trap_receiver->trap_dest_community),
                                       system_time) <= 0)                                           /* Instance length */
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_authenticationFailure_trap return false");
#endif
                return FALSE;
            } /* End of if */    
    	    break;
    	case VAL_trapDestVersion_version2:
    	    if (crt_authenticationFailure_trapv2(ebuff,
                                       trap_receiver->trap_dest_community,
                                       strlen(trap_receiver->trap_dest_community),
                                       0, /*Need to check Request ID. James*/
                                       system_time) <= 0) 
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_authenticationFailure_trapv2 return false");
#endif
                return FALSE;
            } /* End of if */                                        
    	    break;
    	default:
#if (SYS_CPNT_EH == TRUE)
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	    return FALSE;
    	    break;
   } /* end of switch */
   return TRUE; 
   
} /* end of TRAP_UTIL_SendAutehticationFailureTrap() */

BOOL_T TRAP_UTIL_SendNewRootTrap( EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];
#endif
     /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if (crt_newRoot_trap(ebuff, 
                         local_ip,
                         trap_receiver->trap_dest_community,
                         strlen(trap_receiver->trap_dest_community), 
                         system_time) <= 0)                                           /* Instance length */
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_newRoot_trap return false");
#endif
                return FALSE;
            } /* End of if */    
    	    break;
        /* Currently no V2 Trap for New Root, Send V1 first */
    	case VAL_trapDestVersion_version2:
    	    if (crt_newRoot_trap(ebuff, 
    	                 local_ip,
                         trap_receiver->trap_dest_community,
                         strlen(trap_receiver->trap_dest_community),
                         system_time) <= 0)
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_newRoot_trap return false");
#endif
                return FALSE;
            } /* End of if */                                        
    	    break;
    	default:
#if (SYS_CPNT_EH == TRUE)
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	    return FALSE;
    	    break; 
    } /* end of switch */  
    return TRUE; 
    
} /* end of TRAP_UTIL_SendNewRootTrap() */

BOOL_T TRAP_UTIL_SendTCNTrap(    EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];
#endif    
     /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if (crt_topologyChange_trap(ebuff, 
                                local_ip,
                                trap_receiver->trap_dest_community,
                                strlen(trap_receiver->trap_dest_community), 
  	                            system_time) <= 0)                                        /* Instance length */
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_topologyChange_trap return false");
#endif
                return FALSE;
            } /* End of if */    
    	    break;
    	/* Currently No V2 trap for topology Change, Send V1 first*/
    	case VAL_trapDestVersion_version2:
    	    if (crt_topologyChange_trap(ebuff,
    	                        local_ip,
                                trap_receiver->trap_dest_community,
                                strlen(trap_receiver->trap_dest_community), 
  	                            system_time) <= 0)
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_topologyChange_trap return false");
#endif
                return FALSE;
            } /* End of if */                                        
    	    break;
    	default:
#if (SYS_CPNT_EH == TRUE)
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	    return FALSE;
    	    break;
   } /* end of switch */
   return TRUE;    
} /* end of TRAP_UTIL_SendTCNTrap() */


BOOL_T TRAP_UTIL_SendRaisingAlarmTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)                                                                                                                                            
{
    OBJ_ID_T 	object_id; 
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];
#endif
    TRAP_EVENT_RisingFallingAlarmTrap_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_RisingFallingAlarmTrap_T *)trap_data->dynamicData;
    /* BODY */
    
	object_id.num_components = pDynamicData->alarm_variable_len;
	object_id.component_list = pDynamicData->alarm_variable;
    
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if (crt_risingAlarm_trap(ebuff, 
                             local_ip,
                             trap_receiver->trap_dest_community,
	                         strlen(trap_receiver->trap_dest_community), 
    	                     system_time,
							 pDynamicData->alarm_index,                          /* INTEGER */
							 (OIDC_T *)&pDynamicData->instance_alarm_index,      /* Instance */
							 1,                                                             /* Instance length */
							 &object_id,                                                    /* OBJECT IDENTIFIER */
							 (OIDC_T *)&pDynamicData->instance_alarm_variable,   /* Instance */
							 1,                                                             /* Instance length */
							 pDynamicData->alarm_sample_type,                    /* INTEGER */
							 (OIDC_T *)&pDynamicData->instance_alarm_sample_type,/* Instance */
					         1,                                                             /* Instance length */
					         pDynamicData->alarm_value,                          /* INTEGER */
					         (OIDC_T *)&pDynamicData->instance_alarm_value,      /* Instance */
					         1,                                                             /* Instance length */
					         pDynamicData->alarm_rising_falling_threshold,       /* INTEGER */
					         (OIDC_T *)&pDynamicData->instance_alarm_rising_falling_threshold,/* Instance */
					         1/* Instance length */) <= 0)                                       /* Instance length */
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_risingAlarm_trap return false");
#endif
                return FALSE;
            } /* End of if */    
    	    break;
    	case VAL_trapDestVersion_version2:
            if (crt_risingAlarm_trapv2(ebuff,
                             trap_receiver->trap_dest_community,
	                         strlen(trap_receiver->trap_dest_community),
	                         0, /* RequestID, need to check, James*/
    	                     system_time,
							 pDynamicData->alarm_index,                          /* INTEGER */
							 (OIDC_T *)&pDynamicData->instance_alarm_index,      /* Instance */
							 1,                                                             /* Instance length */
							 &object_id,                                                    /* OBJECT IDENTIFIER */
							 (OIDC_T *)&pDynamicData->instance_alarm_variable,   /* Instance */
							 1,                                                             /* Instance length */
							 pDynamicData->alarm_sample_type,                    /* INTEGER */
							 (OIDC_T *)&pDynamicData->instance_alarm_sample_type,/* Instance */
					         1,                                                             /* Instance length */
					         pDynamicData->alarm_value,                          /* INTEGER */
					         (OIDC_T *)&pDynamicData->instance_alarm_value,      /* Instance */
					         1,                                                             /* Instance length */
					         pDynamicData->alarm_rising_falling_threshold,       /* INTEGER */
					         (OIDC_T *)&pDynamicData->instance_alarm_rising_falling_threshold,/* Instance */
					         1/* Instance length */) <= 0)
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_risingAlarm_trapv2 return false");
#endif
                return FALSE;
            } /* End of if */                                        
    	    break;
          	     
    	default:
#if (SYS_CPNT_EH == TRUE)
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	    return FALSE;
    	    break;
   } /* end of switch */
   
   return TRUE;
   
} /* end of TRAP_UTIL_SendRaisingAlarmTrap() */


BOOL_T TRAP_UTIL_SendFallingAlarmTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)                                                                                                                                            
{
    OBJ_ID_T 	object_id; 
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];
#endif
    TRAP_EVENT_RisingFallingAlarmTrap_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_RisingFallingAlarmTrap_T *)trap_data->dynamicData;    
	object_id.num_components = pDynamicData->alarm_variable_len;
	object_id.component_list = pDynamicData->alarm_variable;    
    
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if (crt_fallingAlarm_trap(ebuff, 
                             local_ip,
                             trap_receiver->trap_dest_community,
	                         strlen(trap_receiver->trap_dest_community), 
    	                     system_time,
							 pDynamicData->alarm_index,                          /* INTEGER */
							 (OIDC_T *)&pDynamicData->instance_alarm_index,      /* Instance */
							 1,                                                             /* Instance length */
							 &object_id,                                                    /* OBJECT IDENTIFIER */
							 (OIDC_T *)&pDynamicData->instance_alarm_variable,   /* Instance */
							 1,                                                             /* Instance length */
							 pDynamicData->alarm_sample_type,                    /* INTEGER */
							 (OIDC_T *)&pDynamicData->instance_alarm_sample_type,/* Instance */
					         1,                                                             /* Instance length */
					         pDynamicData->alarm_value,                          /* INTEGER */
					         (OIDC_T *)&pDynamicData->instance_alarm_value,      /* Instance */
					         1,                                                             /* Instance length */
					         pDynamicData->alarm_rising_falling_threshold,       /* INTEGER */
					         (OIDC_T *)&pDynamicData->instance_alarm_rising_falling_threshold,/* Instance */
					         1/* Instance length */) <= 0)                                       /* Instance length */
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_fallingAlarm_trap return false");
#endif
                return FALSE;
            } /* End of if */    
    	    break;
    	case VAL_trapDestVersion_version2:
    	    if (crt_fallingAlarm_trapv2(ebuff,
                             trap_receiver->trap_dest_community,
	                         strlen(trap_receiver->trap_dest_community),
	                         0, /* RequestID, need to check, James*/
    	                     system_time,
							 pDynamicData->alarm_index,                          /* INTEGER */
							 (OIDC_T *)&pDynamicData->instance_alarm_index,      /* Instance */
							 1,                                                             /* Instance length */
							 &object_id,                                                    /* OBJECT IDENTIFIER */
							 (OIDC_T *)&pDynamicData->instance_alarm_variable,   /* Instance */
							 1,                                                             /* Instance length */
							 pDynamicData->alarm_sample_type,                    /* INTEGER */
							 (OIDC_T *)&pDynamicData->instance_alarm_sample_type,/* Instance */
					         1,                                                             /* Instance length */
					         pDynamicData->alarm_value,                          /* INTEGER */
					         (OIDC_T *)&pDynamicData->instance_alarm_value,      /* Instance */
					         1,                                                             /* Instance length */
					         pDynamicData->alarm_rising_falling_threshold,       /* INTEGER */
					         (OIDC_T *)&pDynamicData->instance_alarm_rising_falling_threshold,/* Instance */
					         1/* Instance length */) <= 0)
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_fallingAlarm_trapv2 return false");
#endif
                return FALSE;
            } /* End of if */                                        
    	    break;
          	     
    	default:
#if (SYS_CPNT_EH == TRUE)
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	    return FALSE;
    	    break;    
    } /* end of switch */
    return TRUE;
                	     
} /* end of TRAP_UTIL_SendFallingAlarmTrap() */



BOOL_T TRAP_UTIL_SendPowerStatusChangeTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)                                                                                                                                            
{
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];
#endif
    TRAP_EVENT_PowerStatusChangeTrap_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_PowerStatusChangeTrap_T *)trap_data->dynamicData;    
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if ( crt_swPowerStatusChangeTrap_trap(ebuff,
        	         local_ip,
            	     trap_receiver->trap_dest_community,
            	     strlen(trap_receiver->trap_dest_community), 
    	             system_time,
            	     pDynamicData->sw_indiv_power_unit_index,	                    /* INTEGER */
    	             (OIDC_T *)&pDynamicData->instance_sw_indiv_power_unit_index, /* instance */
            	     2,	                                                                                    /* Instance length */
            	     pDynamicData->sw_indiv_power_index,	                        /* INTEGER */
            	     (OIDC_T *)&pDynamicData->instance_sw_indiv_power_index,	    /* Instance */
            	     2,	                                                                                    /* Instance length */
            	     pDynamicData->sw_indiv_power_status,	                        /* INTEGER */
            	     (OIDC_T *)&pDynamicData->instance_sw_indiv_power_status,	    /* Instance */
            	     2) <= 0)                                           /* Instance length */
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_swPowerStatusChangeTrap_trap return false");
#endif
                return FALSE;
            } /* End of if */    
    	    break;
    	case VAL_trapDestVersion_version2:
    	    if ( crt_swPowerStatusChangeTrap_trapV2(ebuff,
            	     trap_receiver->trap_dest_community,
            	     strlen(trap_receiver->trap_dest_community), 
    	             0, /*Request ID, need to check, James*/
    	             system_time,
            	     pDynamicData->sw_indiv_power_unit_index,	                    /* INTEGER */
    	             (OIDC_T *)&pDynamicData->instance_sw_indiv_power_unit_index, /* instance */
            	     2,	                                                                                    /* Instance length */
            	     pDynamicData->sw_indiv_power_index,	                        /* INTEGER */
            	     (OIDC_T *)&pDynamicData->instance_sw_indiv_power_index,	    /* Instance */
            	     2,	                                                                                    /* Instance length */
            	     pDynamicData->sw_indiv_power_status,	                        /* INTEGER */
            	     (OIDC_T *)&pDynamicData->instance_sw_indiv_power_status,	    /* Instance */
            	     2) <= 0)     
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_swPowerStatusChangeTrap_trapv2 return false");
#endif
                return FALSE;
            } /* End of if */                                        
    	    break;
    	default:
#if (SYS_CPNT_EH == TRUE)
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	    return FALSE;
    	    break;
   } /* end of switch */
   return TRUE; 
} /* end of TRAP_UTIL_SendPowerStatusChangeTrap() */

BOOL_T TRAP_UTIL_SendActivePowerChangeTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)                                                                                                                                            
{
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];
#endif
    TRAP_EVENT_ActivePowerChangeTrap_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_ActivePowerChangeTrap_T *)trap_data->dynamicData;    
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
            if ( crt_swActivePowerChangeTrap_trap(ebuff,
        	         local_ip,
            	     trap_receiver->trap_dest_community,
            	     strlen(trap_receiver->trap_dest_community), 
    	             system_time,
            	     pDynamicData->sw_indiv_power_unit_index,	                  /* INTEGER */
    	             (OIDC_T *)&pDynamicData->instance_sw_indiv_power_unit_index, /* instance */
            	     2,	                                                /* Instance length */
            	     pDynamicData->sw_indiv_power_index,	                  /* INTEGER */
            	     (OIDC_T *)&pDynamicData->instance_sw_indiv_power_index,	  /* Instance */
            	     2) <= 0)                                           /* Instance length */
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_swActivePowerChangeTrap_trap return false");
#endif
                return FALSE;
            } /* End of if */    
    	    break;
    	case VAL_trapDestVersion_version2:
    	    if ( crt_swActivePowerChangeTrap_trapV2(ebuff,
            	     trap_receiver->trap_dest_community,
            	     strlen(trap_receiver->trap_dest_community), 
    	             0, /*Request ID, need to check, James*/
    	             system_time,
            	     pDynamicData->sw_indiv_power_unit_index,	                  /* INTEGER */
    	             (OIDC_T *)&pDynamicData->instance_sw_indiv_power_unit_index, /* instance */
            	     2,	                                                          /* Instance length */
            	     pDynamicData->sw_indiv_power_index,	                  /* INTEGER */
            	     (OIDC_T *)&pDynamicData->instance_sw_indiv_power_index,	  /* Instance */
            	     2) <= 0)     
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_swActivePowerChangeTrap_trapv2 return false");
#endif
                return FALSE;
            } /* End of if */                                        
    	    break;
    	default:
#if (SYS_CPNT_EH == TRUE)
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	    return FALSE;
    	    break;
   } /* end of switch */
   return TRUE; 
} /* end of TRAP_UTIL_SendActivePowerChangeTrap() */

#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE) 

BOOL_T TRAP_UTIL_SendPortSecurityTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];
#endif
    TRAP_EVENT_PortSecurityTrap_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_PortSecurityTrap_T *)trap_data->dynamicData;    
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
            if (crt_swPortSecurityTrap_trap(ebuff,
	                                        local_ip,
	                                        trap_receiver->trap_dest_community,
    	             		                strlen(trap_receiver->trap_dest_community), 
                                            system_time,
	                                        pDynamicData->instance_ifindex,	
	                                        (OIDC_T *) &(pDynamicData->ifindex),	
	                                        1) <= 0)
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_swPortSecurityTrap_trap return false");
#endif
                return FALSE;
            }
            break;
        
        case VAL_trapDestVersion_version2:
            if (crt_swPortSecurityTrap_trapV2(ebuff,
                                              trap_receiver->trap_dest_community,
        	             		              strlen(trap_receiver->trap_dest_community), 
        	             		              0,
            	             		          system_time,
	                                          pDynamicData->instance_ifindex,	
	                                          (OIDC_T *) &(pDynamicData->ifindex),	
	                                          1) <= 0)
            {
#if (SYS_CPNT_EH == TRUE)
                EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "crt_swPortSecurityTrap_trapv2 return false");
#endif
                return FALSE;
            }
            break;
    	default:
#if (SYS_CPNT_EH == TRUE)
    	    sprintf(msg, "unknow version=[%lu]",trap_receiver->trap_dest_version);
    	    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0, 
                                 EH_TYPE_MSG_DEB_MSG, 
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif
    	    return FALSE;
    	    break;
   } /* end of switch */
   return TRUE; 

} /* end of TRAP_UTIL_SendPortSecurityTrap () */
#endif  #if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)                                  

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)  
BOOL_T TRAP_UTIL_SendLoopbackTestFailureTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip,
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                    
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T      *trap_data,
                                   UI32_T                     system_time)
{
	TRAP_EVENT_LoopBackFailureTrap_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_LoopBackFailureTrap_T *)trap_data->dynamicData; 
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
            if (crt_swLoopbackTestFailureTrap_trap(ebuff,
	                                        local_ip,
	                                        trap_receiver->trap_dest_community,
    	             		                strlen(trap_receiver->trap_dest_community), 
                                            system_time,
	                                        SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST * SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK,	
	                                        (OCTET_T*)&(pDynamicData->ports)) <= 0)	
	                                        
            {
                return FALSE;
            }
            break;
        
        case VAL_trapDestVersion_version2:
            if (crt_swLoopbackTestFailureTrap_trapV2(ebuff,
                                              trap_receiver->trap_dest_community,
        	             		              strlen(trap_receiver->trap_dest_community), 
        	             		              0,
            	             		          system_time,
	                                          SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST * SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK,	
	                                        (OCTET_T*)&(pDynamicData->ports)) <= 0)
            {
                return FALSE;
            }
            break;
    	default:
    	    return FALSE;
    	    break;
   } /* end of switch */
   return TRUE; 

} /* end of TRAP_UTIL_SendLoopbackTestFailureTrap () */
#endif   
                                   
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
BOOL_T TRAP_UTIL_SendFanFailureTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{   
	TRAP_EVENT_FanTrap_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_FanTrap_T *)trap_data->dynamicData;  
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
            if (crt_swFanFailureTrap_trap(ebuff,
                                    local_ip,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
            	                    system_time,                                    
                                    pDynamicData->trap_unit_index,
                                    (OIDC_T *)&pDynamicData->instance_trap_unit_index,
                                    2,
                                    pDynamicData->trap_fan_index,
                                    (OIDC_T *)&pDynamicData->instance_trap_fan_index,
                                    2) <= 0)
            {
                return FALSE;
            }
            break;
        case VAL_trapDestVersion_version2:
            if (crt_swFanFailureTrap_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,                                    
                                    pDynamicData->trap_unit_index,
                                    (OIDC_T *)&pDynamicData->instance_trap_unit_index,
                                    2,
                                    pDynamicData->trap_fan_index,
                                    (OIDC_T *)&pDynamicData->instance_trap_fan_index,
                                    2) <= 0)
            {
                return FALSE;
            }
            break;
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendFanFailureTrap() */

BOOL_T TRAP_UTIL_SendFanRecoverTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{   
	TRAP_EVENT_FanTrap_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_FanTrap_T *)trap_data->dynamicData; 
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
            if (crt_swFanRecoverTrap_trap(ebuff,
                                    local_ip,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
            	                    system_time,                                    
                                    pDynamicData->trap_unit_index,
                                    (OIDC_T *)&pDynamicData->instance_trap_unit_index,
                                    2,
                                    pDynamicData->trap_fan_index,
                                    (OIDC_T *)&pDynamicData->instance_trap_fan_index,
                                    2) <= 0)
            {
                return FALSE;
            }
            break;
        case VAL_trapDestVersion_version2:
            if (crt_swFanRecoverTrap_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,                                    
                                    pDynamicData->trap_unit_index,
                                    (OIDC_T *)&pDynamicData->instance_trap_unit_index,
                                    2,
                                    pDynamicData->trap_fan_index,
                                    (OIDC_T *)&pDynamicData->instance_trap_fan_index,
                                    2) <= 0)
            {
                return FALSE;
            }
            break;
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendFanRecoverTrap() */
#endif
#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
BOOL_T TRAP_UTIL_SendIpFilterRejectTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
	TRAP_EVENT_IpFilterRejectTrap_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_IpFilterRejectTrap_T *)trap_data->dynamicData; 
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
            if (crt_swIpFilterRejectTrap_trap(ebuff,
                                    local_ip,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
            	                    system_time,
                                    pDynamicData->mode,
                                    pDynamicData->ip) <= 0)
            {
                return FALSE;
            }
            break;
        case VAL_trapDestVersion_version2:
            if (crt_swIpFilterRejectTrap_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
                                    pDynamicData->mode,
                                    pDynamicData->ip) <= 0)
            {
                return FALSE;
            }
            break;
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendIpFilterRejectTrap() */
#endif//end of #if (SYS_CPNT_MGMT_IP_FLT == TRUE)

#if (SYS_CPNT_SMTP == TRUE)
BOOL_T TRAP_UTIL_SendSmtpConnFailureTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{    
	TRAP_EVENT_swSmtpConnFailureTrap_T *pDynamicData;	    		
	pDynamicData=(TRAP_EVENT_swSmtpConnFailureTrap_T *)trap_data->dynamicData; 
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:               
            if (crt_swSmtpConnFailureTrap_trap(ebuff,
                                    local_ip,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
            	                    system_time,
            	                    (OCTET_T*) &(pDynamicData->smtpServerIp),            	               
            	                    (OIDC_T *) &(pDynamicData->instance_smtpServerIp),            	                    
            	                    4) <= 0)
            {
                return FALSE;
            }
            break;
        case VAL_trapDestVersion_version2:
        {            
            if (crt_swSmtpConnFailureTrap_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
            	                    (OCTET_T*) &(pDynamicData->smtpServerIp),
            	                    (OIDC_T *) &(pDynamicData->instance_smtpServerIp),            	                    
            	                    4) <= 0)
            {
                return FALSE;
            }
            break;
        }
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendSmtpConnFailureTrap() */
#endif//end of #if (SYS_CPNT_SMTP == TRUE)

#if (SYS_CPNT_POE == TRUE)
BOOL_T TRAP_UTIL_SendPethPsePortOnOffTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{    
	TRAP_EVENT_PethPsePortOnOffTrap_T *pDynamicData;	    		
	pDynamicData=(TRAP_EVENT_PethPsePortOnOffTrap_T *)trap_data->dynamicData;   	  
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        //standard don't support v1 version, we send both in v2 version
        case VAL_trapDestVersion_version1:  
        case VAL_trapDestVersion_version2:             
            if (crt_pethPsePortOnOffNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
            	                    pDynamicData->pethPsePortDetectionStatus,
            	                    (OIDC_T *)&pDynamicData->instance_pethPsePortDetectionStatus,
            	                    2) <= 0)
            {
                return FALSE;
            }
            break;        
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendPethPsePortOnOffTrap() */

BOOL_T TRAP_UTIL_SendPethMainPowerUsageOnTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{ 
	TRAP_EVENT_PethMainPowerUsageOnTrap_T *pDynamicData;	    		
	pDynamicData=(TRAP_EVENT_PethMainPowerUsageOnTrap_T *)trap_data->dynamicData;     	  
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        //standard don't support v1 version, we send both in v2 version
        case VAL_trapDestVersion_version1:    
        case VAL_trapDestVersion_version2:           
            if (crt_pethMainPowerUsageOnNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
            	                    pDynamicData->pethMainPseConsumptionPower,
            	                    (OIDC_T *)&pDynamicData->instance_pethMainPseConsumptionPower,
            	                    1) <= 0)
            {
                return FALSE;
            }
            break;        
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendPethMainPowerUsageOnTrap() */


BOOL_T TRAP_UTIL_SendPethMainPowerUsageOffTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                 
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{   
	TRAP_EVENT_PethMainPowerUsageOffTrap_T *pDynamicData;	    		
	pDynamicData=(TRAP_EVENT_PethMainPowerUsageOffTrap_T *)trap_data->dynamicData;    	  
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        //standard don't support v1 version, we send both in v2 version
        case VAL_trapDestVersion_version1: 
        case VAL_trapDestVersion_version2:              
            if (crt_pethMainPowerUsageOffNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
            	                    pDynamicData->pethMainPseConsumptionPower,
            	                    (OIDC_T *)&pDynamicData->instance_pethMainPseConsumptionPower,
            	                    1) <= 0)
            {
                return FALSE;
            }
            break;              
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendPethMainPowerUsageOffTrap() */
#endif //end of #if (SYS_CPNT_POE == TRUE)
#if (SYS_CPNT_VDSL == TRUE)

BOOL_T TRAP_UTIL_SendVdslPerfLofsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                    
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{  
	TRAP_EVENT_vdslPerfLofsThresh_T *pDynamicData;	    		
	pDynamicData=(TRAP_EVENT_vdslPerfLofsThresh_T *)trap_data->dynamicData; 	  
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        //standard don't support v1 version, we send both in v2 version
        case VAL_trapDestVersion_version1:
        case VAL_trapDestVersion_version2:                      
            if (crt_vdslPerfLofsThreshNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
            	                    (UINT_64_T*)&pDynamicData->vdslPerfCurr15MinLofs,
            	                    (OIDC_T *)&pDynamicData->instance_vdslPerfCurr15MinLofs,
            	                    2) <= 0)             	                          	                           	                                	                    
            {
                return FALSE;
            }
            break;        
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendVdslPerfLofsThreshTrap() */

BOOL_T TRAP_UTIL_SendVdslPerfLossThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                    
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{  
	TRAP_EVENT_vdslPerfLossThresh_T *pDynamicData;	    		
	pDynamicData=(TRAP_EVENT_vdslPerfLossThresh_T *)trap_data->dynamicData; 	  
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        //standard don't support v1 version, we send both in v2 version
        case VAL_trapDestVersion_version1:
        case VAL_trapDestVersion_version2:                      
            if (crt_vdslPerfLossThreshNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
            	                    (UINT_64_T*)&pDynamicData->vdslPerfCurr15MinLoss,
            	                    (OIDC_T *)&pDynamicData->instance_vdslPerfCurr15MinLoss,
            	                    2) <= 0)             	                          	                           	                                	                    
            {
                return FALSE;
            }
            break;        
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendVdslPerfLossThreshTrap() */

BOOL_T TRAP_UTIL_SendVdslPerfLprsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                    
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{  
	TRAP_EVENT_vdslPerfLprsThresh_T *pDynamicData;	    		
	pDynamicData=(TRAP_EVENT_vdslPerfLprsThresh_T *)trap_data->dynamicData; 	  
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        //standard don't support v1 version, we send both in v2 version
        case VAL_trapDestVersion_version1:
        case VAL_trapDestVersion_version2:                      
            if (crt_vdslPerfLprsThreshNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
            	                    (UINT_64_T*)&pDynamicData->vdslPerfCurr15MinLprs,
            	                    (OIDC_T *)&pDynamicData->instance_vdslPerfCurr15MinLprs,
            	                    2) <= 0)             	                          	                           	                                	                    
            {
                return FALSE;
            }
            break;        
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendVdslPerfLprsThreshTrap() */

BOOL_T TRAP_UTIL_SendVdslPerfLolsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                    
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{  
	TRAP_EVENT_vdslPerfLolsThresh_T *pDynamicData;	    		
	pDynamicData=(TRAP_EVENT_vdslPerfLolsThresh_T *)trap_data->dynamicData; 	  
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        //standard don't support v1 version, we send both in v2 version
        case VAL_trapDestVersion_version1:
        case VAL_trapDestVersion_version2:                      
            if (crt_vdslPerfLolsThreshNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
            	                    (UINT_64_T*)&pDynamicData->vdslPerfCurr15MinLols,
            	                    (OIDC_T *)&pDynamicData->instance_vdslPerfCurr15MinLols,
            	                    2) <= 0)             	                          	                           	                                	                    
            {
                return FALSE;
            }
            break;        
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendVdslPerfLolsThreshTrap() */

BOOL_T TRAP_UTIL_SendVdslPerfESsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                    
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{  
	TRAP_EVENT_vdslPerfESsThresh_T *pDynamicData;	    		
	pDynamicData=(TRAP_EVENT_vdslPerfESsThresh_T *)trap_data->dynamicData; 	  
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        //standard don't support v1 version, we send both in v2 version
        case VAL_trapDestVersion_version1:
        case VAL_trapDestVersion_version2:                      
            if (crt_vdslPerfESsThreshNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
            	                    (UINT_64_T*)&pDynamicData->vdslPerfCurr15MinESs,
            	                    (OIDC_T *)&pDynamicData->instance_vdslPerfCurr15MinESs,
            	                    2) <= 0)             	                          	                           	                                	                    
            {
                return FALSE;
            }
            break;        
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendVdslPerfESsThreshTrap() */

BOOL_T TRAP_UTIL_SendVdslPerfSESsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                    
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{  
	TRAP_EVENT_vdslPerfSESsThresh_T *pDynamicData;	    		
	pDynamicData=(TRAP_EVENT_vdslPerfSESsThresh_T *)trap_data->dynamicData; 	  
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        //standard don't support v1 version, we send both in v2 version
        case VAL_trapDestVersion_version1:
        case VAL_trapDestVersion_version2:                      
            if (crt_vdslPerfSESsThreshNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
            	                    (UINT_64_T*)&pDynamicData->vdslPerfCurr15MinSESs,
            	                    (OIDC_T *)&pDynamicData->instance_vdslPerfCurr15MinSESs,
            	                    2) <= 0)             	                          	                           	                                	                    
            {
                return FALSE;
            }
            break;        
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendVdslPerfSESsThreshTrap() */

BOOL_T TRAP_UTIL_SendVdslPerfUASsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else 
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{  
	TRAP_EVENT_vdslPerfUASsThresh_T *pDynamicData;	    		
	pDynamicData=(TRAP_EVENT_vdslPerfUASsThresh_T *)trap_data->dynamicData; 	  
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        //standard don't support v1 version, we send both in v2 version
        case VAL_trapDestVersion_version1:
        case VAL_trapDestVersion_version2:                      
            if (crt_vdslPerfUASsThreshNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
            	                    (UINT_64_T*)&pDynamicData->vdslPerfCurr15MinUASs,
            	                    (OIDC_T *)&pDynamicData->instance_vdslPerfCurr15MinUASs,
            	                    2) <= 0)             	                          	                           	                                	                    
            {
                return FALSE;
            }
            break;        
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendVdslPerfUASsThreshTrap() */

#endif//end of #if (SYS_CPNT_VDSL == TRUE)

BOOL_T TRAP_UTIL_SendSwMainBoardVerMismatchTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
	TRAP_EVENT_mainBoardVerMismatch_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_mainBoardVerMismatch_T *)trap_data->dynamicData; 
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
            if (crt_swMainBoardVerMismatchNotificaiton_trap(ebuff,
                                    local_ip,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
            	                    system_time,
                                    strlen(pDynamicData->swOpCodeVerMaster),
                                    (OCTET_T*)&(pDynamicData->swOpCodeVerMaster),
                                    (OIDC_T *)&pDynamicData->instance_swOpCodeVerMaster,
                                    1,
                                    strlen(pDynamicData->swOpCodeVerSlave),
                                    (OCTET_T*)&(pDynamicData->swOpCodeVerSlave),
                                    (OIDC_T *)&pDynamicData->instance_swOpCodeVerSlave,
                                    1) <= 0)                                                                        
            {
                return FALSE;
            }
            break;
        case VAL_trapDestVersion_version2:
            if (crt_swMainBoardVerMismatchNotificaiton_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
                                    strlen(pDynamicData->swOpCodeVerMaster),
                                    (OCTET_T*)&(pDynamicData->swOpCodeVerMaster),
                                    (OIDC_T *)&pDynamicData->instance_swOpCodeVerMaster,
                                    1,
                                    strlen(pDynamicData->swOpCodeVerSlave),
                                    (OCTET_T*)&(pDynamicData->swOpCodeVerSlave),
                                    (OIDC_T *)&pDynamicData->instance_swOpCodeVerSlave,
                                    1) <= 0) 
            {
                return FALSE;
            }
            break;
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendSwMainBoardVerMismatchTrap() */

BOOL_T TRAP_UTIL_SendSwModuleVerMismatchTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
	TRAP_EVENT_moduleVerMismatch_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_moduleVerMismatch_T *)trap_data->dynamicData; 
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
            if (crt_swModuleVerMismatchNotificaiton_trap(ebuff,
                                    local_ip,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
            	                    system_time,                                    
                                    strlen(pDynamicData->swModuleExpectedOpCodeVer),
                                    (OCTET_T*)&(pDynamicData->swModuleExpectedOpCodeVer),
                                    (OIDC_T *)&pDynamicData->instance_swModuleExpectedOpCodeVer,
                                    1,
                                    strlen(pDynamicData->swModuleOpCodeVer),
                                    (OCTET_T*)&(pDynamicData->swModuleOpCodeVer),
                                    (OIDC_T *)&pDynamicData->instance_swModuleOpCodeVer,
                                    2) <= 0)                                                                        
            {
                return FALSE;
            }
            break;
        case VAL_trapDestVersion_version2:
            if (crt_swModuleVerMismatchNotificaiton_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,                                    
                                    strlen(pDynamicData->swModuleExpectedOpCodeVer),
                                    (OCTET_T*)&(pDynamicData->swModuleExpectedOpCodeVer),
                                    (OIDC_T *)&pDynamicData->instance_swModuleExpectedOpCodeVer,
                                    1,
                                    strlen(pDynamicData->swModuleOpCodeVer),
                                    (OCTET_T*)&(pDynamicData->swModuleOpCodeVer),
                                    (OIDC_T *)&pDynamicData->instance_swModuleOpCodeVer,
                                    2) <= 0)
            {
                return FALSE;
            }
            break;
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendSwModuleVerMismatchTrap() */

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
BOOL_T TRAP_UTIL_SendSwThermalRisingTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
	TRAP_EVENT_thermalRising_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_thermalRising_T *)trap_data->dynamicData; 
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
            if (crt_swThermalRisingNotification_trap(ebuff,
                                    local_ip,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
            	                    system_time,
                                    pDynamicData->switchThermalTempValue,                                    
                                    (OIDC_T *)&pDynamicData->instance_switchThermalTempValue,
                                    2,
                                    pDynamicData->switchThermalActionRisingThreshold,                                    
                                    (OIDC_T *)&pDynamicData->instance_switchThermalActionRisingThreshold,
                                    3) <= 0)                                                                        
            {
                return FALSE;
            }
            break;
        case VAL_trapDestVersion_version2:
            if (crt_swThermalRisingNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
                                    pDynamicData->switchThermalTempValue,                                    
                                    (OIDC_T *)&pDynamicData->instance_switchThermalTempValue,
                                    2,
                                    pDynamicData->switchThermalActionRisingThreshold,                                    
                                    (OIDC_T *)&pDynamicData->instance_switchThermalActionRisingThreshold,
                                    3) <= 0)   
            {
                return FALSE;
            }
            break;
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendSwThermalRisingTrap() */

BOOL_T TRAP_UTIL_SendSwThermalFallingTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
	TRAP_EVENT_thermalFalling_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_thermalFalling_T *)trap_data->dynamicData; 
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
            if (crt_swThermalFallingNotification_trap(ebuff,
                                    local_ip,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
            	                    system_time,
                                    pDynamicData->switchThermalTempValue,                                    
                                    (OIDC_T *)&pDynamicData->instance_switchThermalTempValue,
                                    2,
                                    pDynamicData->switchThermalActionFallingThreshold,                                    
                                    (OIDC_T *)&pDynamicData->instance_switchThermalActionFallingThreshold,
                                    3) <= 0)                                                                        
            {
                return FALSE;
            }
            break;
        case VAL_trapDestVersion_version2:
            if (crt_swThermalFallingNotification_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
                                    pDynamicData->switchThermalTempValue,                                    
                                    (OIDC_T *)&pDynamicData->instance_switchThermalTempValue,
                                    2,
                                    pDynamicData->switchThermalActionFallingThreshold,                                    
                                    (OIDC_T *)&pDynamicData->instance_switchThermalActionFallingThreshold,
                                    3) <= 0)   
            {
                return FALSE;
            }
            break;
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendSwThermalFallingTrap() */

#endif//end of #if (SYS_CPNT_THERMAL_DETECT == TRUE)


#if (SYS_CPNT_MODULE_WITH_CPU == TRUE)

BOOL_T TRAP_UTIL_SendSwModuleInsertionTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
	TRAP_EVENT_moduleInsertion_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_moduleInsertion_T *)trap_data->dynamicData; 
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
            if (crt_swModuleInsertionNotificaiton_trap(ebuff,
                                    local_ip,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
            	                    system_time,
                                    strlen(pDynamicData->swModuleOpCodeVer), 
                                    (OCTET_T*)&(pDynamicData->swModuleOpCodeVer),                                   
                                    (OIDC_T *)&pDynamicData->instance_swModuleOpCodeVer,
                                    2) <= 0)                                                                        
            {
                return FALSE;
            }
            break;
        case VAL_trapDestVersion_version2:
            if (crt_swModuleInsertionNotificaiton_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
                                    strlen(pDynamicData->swModuleOpCodeVer), 
                                    (OCTET_T*)&(pDynamicData->swModuleOpCodeVer),                                   
                                    (OIDC_T *)&pDynamicData->instance_swModuleOpCodeVer,
                                    2) <= 0)   
            {
                return FALSE;
            }
            break;
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendSwModuleInsertionTrap() */

BOOL_T TRAP_UTIL_SendSwModuleRemovalTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time)
{
	TRAP_EVENT_moduleRemoval_T *pDynamicData;			
	pDynamicData=(TRAP_EVENT_moduleRemoval_T *)trap_data->dynamicData; 
    /* check if we want to send V1 or V2 Trap*/               
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
            if (crt_swModuleRemovalNotificaiton_trap(ebuff,
                                    local_ip,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
            	                    system_time,
                                    strlen(pDynamicData->swModuleOpCodeVer), 
                                    (OCTET_T*)&(pDynamicData->swModuleOpCodeVer),                                   
                                    (OIDC_T *)&pDynamicData->instance_swModuleOpCodeVer,
                                    2) <= 0)                                                                        
            {
                return FALSE;
            }
            break;
        case VAL_trapDestVersion_version2:
            if (crt_swModuleRemovalNotificaiton_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,
            	                    system_time,
                                    strlen(pDynamicData->swModuleOpCodeVer), 
                                    (OCTET_T*)&(pDynamicData->swModuleOpCodeVer),                                   
                                    (OIDC_T *)&pDynamicData->instance_swModuleOpCodeVer,
                                    2) <= 0)   
            {
                return FALSE;
            }
            break;
        default:
            return FALSE;
            break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendSwModuleRemovalTrap() */

#endif//end of #if (SYS_CPNT_MODULE_WITH_CPU == TRUE)

BOOL_T TRAP_UTIL_TrapReason(   EBUFFER_T                  *ebuff, 
                               OCTET_T                    *local_ip, 
                               #if (SYS_CPNT_SNMP_VERSION == 3)
                               SNMP_MGR_TrapDestEntry_T   *trap_receiver,
                               #else
                               TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                               #endif
                               TRAP_EVENT_TrapQueueData_T *trap_data,
                               UI32_T                     system_time)
{
    TRAP_EVENT_tcn_T *pDynamicData;			
    pDynamicData=(TRAP_EVENT_tcn_T *)trap_data->dynamicData; 

    /* check if we want to send V1 or V2 Trap*/ 
                  
    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1: 
             if (crt_tcnTrap_trap(ebuff,
                                  local_ip,             
            	                  trap_receiver->trap_dest_community,
            	                  strlen(trap_receiver->trap_dest_community),
            	                  system_time,
                                  (UI32_T)pDynamicData->tcnReason)<=0)                                                                      
             {
                return FALSE;
             }
             break;

        case VAL_trapDestVersion_version2:
             if (crt_tcnTrap_trapV2(ebuff,
            	                    trap_receiver->trap_dest_community,
            	                    strlen(trap_receiver->trap_dest_community),
                                    0,               	                    
            	                    system_time,
                                    (UI32_T)pDynamicData->tcnReason)<=0 )              
             {
                return FALSE;
             }
             break;
             
        default:
             return FALSE;
             break;
    }
    return TRUE;                        
} /* end of TRAP_UTIL_SendSwModuleRemovalTrap() */

#if (SYS_CPNT_LLDP == TRUE)
BOOL_T TRAP_UTIL_SendLldpRemTablesChangedTrap(EBUFFER_T                  *ebuff,
                                              OCTET_T                    *local_ip,
                                              #if (SYS_CPNT_SNMP_VERSION == 3)
                                              SNMP_MGR_TrapDestEntry_T   *trap_receiver,
                                              #else
                                              TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                              #endif
                                              TRAP_EVENT_TrapQueueData_T *trap_data,
                                              UI32_T                     system_time)
{
    TRAP_EVENT_lldpRemTablesChange_T *pDynamicData;
    pDynamicData=(TRAP_EVENT_lldpRemTablesChange_T *)trap_data->dynamicData;

    switch( trap_receiver->trap_dest_version)
    {
        case VAL_trapDestVersion_version1:
             if (crt_lldpRemTablesChange_trap(ebuff,
                                              local_ip,
                        	                  trap_receiver->trap_dest_community,
                        	                  strlen(trap_receiver->trap_dest_community),
                        	                  system_time,
                                              (UI32_T)pDynamicData->lldpStatsRemTablesInserts,
                                              (UI32_T)pDynamicData->lldpStatsRemTablesDeletes,
                                              (UI32_T)pDynamicData->lldpStatsRemTablesDrops,
                                              (UI32_T)pDynamicData->lldpStatsRemTablesAgeouts)<=0)
             {
                return FALSE;
             }
             break;

        case VAL_trapDestVersion_version2:
             if (crt_lldpRemTablesChange_trapV2(ebuff,
                        	                    trap_receiver->trap_dest_community,
                        	                    strlen(trap_receiver->trap_dest_community),
                                                0,
                        	                    system_time,
                                                (UI32_T)pDynamicData->lldpStatsRemTablesInserts,
                                                (UI32_T)pDynamicData->lldpStatsRemTablesDeletes,
                                                (UI32_T)pDynamicData->lldpStatsRemTablesDrops,
                                                (UI32_T)pDynamicData->lldpStatsRemTablesAgeouts)<=0 )
             {
                return FALSE;
             }
             break;

        default:
             return FALSE;
             break;
    }
    return TRUE;
}
#endif

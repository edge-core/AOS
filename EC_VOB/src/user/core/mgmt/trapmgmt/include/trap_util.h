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
 
#ifndef	_TRAP_UTIL_H
#define	_TRAP_UTIL_H



/* INCLUDE FILE DECLARATIONS
 */
#include <envoy/h/buffer.h>
#include <envoy/h/asn1conf.h>
#include <envoy/h/snmpstat.h>
#include <envoy/h/snmpdefs.h>
#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "leaf_es3626a.h"
#include "trap_event.h"
#include "trap_mgr.h"
#include "sys_cpnt.h"

BOOL_T TRAP_UTIL_SendColdStartTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendWarmStartTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendLinkDownTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendLinkUpTrap(   EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);

BOOL_T TRAP_UTIL_SendCraftLinkDownTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);

BOOL_T TRAP_UTIL_SendCraftLinkUpTrap(   EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);

BOOL_T TRAP_UTIL_SendAutehticationFailureTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendNewRootTrap(  EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);       

BOOL_T TRAP_UTIL_SendTCNTrap(      EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendRaisingAlarmTrap( EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendFallingAlarmTrap( EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);   
BOOL_T TRAP_UTIL_SendPowerStatusChangeTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);    
BOOL_T TRAP_UTIL_SendActivePowerChangeTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T     *trap_data,
                                   UI32_T                     system_time);
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)                                       
BOOL_T TRAP_UTIL_SendPortSecurityTrap( EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                               
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif 
                                   TRAP_EVENT_TrapQueueData_T      *trap_data,
                                   UI32_T                     system_time);            
#endif//end of #if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)                                    
BOOL_T TRAP_UTIL_SendLoopbackTestFailureTrap( EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);                                   
#endif//end of #if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)                                    

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) 
BOOL_T TRAP_UTIL_SendFanFailureTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);  
BOOL_T TRAP_UTIL_SendFanRecoverTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T            *trap_data,
                                   UI32_T                     system_time);
#endif//end of #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) 

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)                                     
BOOL_T TRAP_UTIL_SendIpFilterRejectTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
#endif  

#if (SYS_CPNT_SMTP == TRUE)
BOOL_T TRAP_UTIL_SendSmtpConnFailureTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else  
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                   TRAP_EVENT_TrapQueueData_T            *trap_data,
                                   UI32_T                     system_time);
#endif                                                                                                        
#if (SYS_CPNT_POE == TRUE)                                  
BOOL_T TRAP_UTIL_SendPethPsePortOnOffTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendPethMainPowerUsageOnTrap( EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendPethMainPowerUsageOffTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time); 
#endif//end of #if (SYS_CPNT_POE == TRUE)                                  
 
#if (SYS_CPNT_VDSL == TRUE) 

BOOL_T TRAP_UTIL_SendVdslPerfLofsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendVdslPerfLossThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time); 
BOOL_T TRAP_UTIL_SendVdslPerfLprsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendVdslPerfLolsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendVdslPerfESsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendVdslPerfSESsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendVdslPerfUASsThreshTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendSwMaxMacCountTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
#if (SYS_CPNT_SNMP_VERSION == 3)
                                   SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else                                   
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif                                   
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);                                                                                                                                                                                                                                                    
#endif//end of #if (SYS_CPNT_VDSL == TRUE) 
BOOL_T TRAP_UTIL_SendSwMainBoardVerMismatchTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendSwModuleVerMismatchTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);                                   
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
BOOL_T TRAP_UTIL_SendSwThermalRisingTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendSwThermalFallingTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);                                   
#endif

#if (SYS_CPNT_MODULE_WITH_CPU == TRUE)
BOOL_T TRAP_UTIL_SendSwModuleInsertionTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
BOOL_T TRAP_UTIL_SendSwModuleRemovalTrap(EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
                                   #else
                                   TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);                                   
#endif

BOOL_T TRAP_UTIL_TrapReason(	   EBUFFER_T                  *ebuff, 
                                   OCTET_T                    *local_ip, 
                                   #if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #else
                                       TRAP_MGR_TrapDestEntry_T   *trap_receiver,
                                   #endif
                                   TRAP_EVENT_TrapQueueData_T *trap_data,
                                   UI32_T                     system_time);
                                   
#endif  /* end of TRAP_UTIL_H */
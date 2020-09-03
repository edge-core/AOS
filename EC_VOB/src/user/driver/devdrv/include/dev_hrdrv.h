#ifndef DEV_HRDRV_H
#define DEV_HRDRV_H

#include "sys_type.h"
#include "hrdrv_type.h"


typedef enum 
{  
    DEV_HRDRV_DO_IP_ACL,
    DEV_HRDRV_DO_MAC_ACL, 
    DEV_HRDRV_DO_IP_DIFFSERV,
    DEV_HRDRV_DO_MAC_DIFFSERV
             
} DEV_HRDRV_FUN_TYPE_T;

typedef enum 
{
   DEV_SWDRV_DOT1X_PACKET_DISCARD = 0,
   DEV_SWDRV_DOT1X_PACKET_FORWARD,
   DEV_SWDRV_DOT1X_PACKET_TRAPTOCPU
} SWCTRL_DOT1X_Packet_Operation_T;

#define DEV_HRDRV_NBRS_OF_FUN_TYPE   4

#define DEV_HRDRV_L4_MapTosToCos     1
#define DEV_HRDRV_L4_MapDscpToCos    2
#define DEV_HRDRV_L4_MapTcpPortToCos 3
#define DEV_HRDRV_L4_MapUdpPortToCos 4
     

enum {DEV_HRDRV_ADMIN_MODE=0x1, DEV_HRDRV_OP_MODE};


/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_SetCosMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set TOS/COS mapping of system
 * INPUT   : maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 *           value  -- the cos value
 *           cos    -- the dot1p priority used to map to hardware queue 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is global configuration compared to DEV_HRDRV_SetPortCosMapping
 * -------------------------------------------------------------------------*/  
BOOL_T DEV_HRDRV_SetCosMapping(UI32_T maptype, UI32_T value, UI32_T cos);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_SetPortCosMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set TOS/COS mapping of system
 * INPUT   : unit   -- device_id
 *           port   -- port number
 *           maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 *           value  -- the cos value
 *           cos    -- the dot1p priority used to map to hardware queue 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 
 * -------------------------------------------------------------------------*/       
BOOL_T DEV_HRDRV_SetPortCosMapping(UI32_T maptype, UI32_T unit, UI32_T port,  UI32_T value, UI32_T cos);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_DelCosMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete TOS/COS mapping of system
 * INPUT   : maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 *           value  -- the cos value
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is global conifuration compared to HRDRV_DelPortCosMapping
 * -------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_DelCosMapping(UI32_T maptype, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_DelPortCosMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete TOS/COS mapping of system
 * INPUT   : unit   -- device_id
 *           port 
 *           maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 *           value  -- the cos value
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_DelPortCosMapping(UI32_T unit, UI32_T port , UI32_T maptype, UI32_T value);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_EnableCosMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable tos mapping
 * INPUT   : maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : it is as the driver of swdrvl4.c 
 *           this API is global configuration compared to HRDRV_EnablePortCosMapping
 *           which to call , depend on upper layer 
 * -------------------------------------------------------------------------
 */
BOOL_T DEV_HRDRV_EnableCosMapping(UI32_T maptype);




/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_DisableCosMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable tos mapping
 * INPUT   : maptype is one of HRDRV_L4_CosMapType_E
 *           HRDRV_L4_MapTosToCos, 
 *           HRDRV_L4_MapDscpToCos
 *           HRDRV_L4_MapTcpPortToCos
 *           HRDRV_L4_MapUdpPortToCos
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : it is as the driver of swdrvl4.c 
 *           this API is global configuration compared to HRDRV_EnablePortCosMapping
 *           which to call , depend on upper layer 
 *
 * -------------------------------------------------------------------------
 */
BOOL_T DEV_HRDRV_DisableCosMapping(UI32_T maptype);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_SetPortIngressRateLimit
 *---------------------------------------------------------------------------
 * PURPOSE:  to set the ingress rate limit 
 * INPUT:    unit
 *           port
 *           rate   -- ingress rate
 * OUTPUT:   None.
 * RETURN:   TRUE / FALSE
 * NOTES :   
 *---------------------------------------------------------------------------
 */
BOOL_T DEV_HRDRV_SetPortIngressRateLimit(UI32_T unit, UI32_T port, UI32_T rate) ;


/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_DisablePortIngressRateLimit
 *---------------------------------------------------------------------------
 * PURPOSE:  to diable port ingress rate-limit
 * INPUT:    unit, port.
 * OUTPUT:   None.
 * RETURN:   TRUE 
 *           FALSE -- if port ingress rate-limit is not supported
 * NOTES :  
 *---------------------------------------------------------------------------
 */
BOOL_T DEV_HRDRV_DisablePortIngressRateLimit(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnablePortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the port ingress rate limit
 * INPUT   : unit, port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_EnablePortIngressRateLimit(UI32_T unit, UI32_T port);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_DisableBroadcastIpTrapToCpu
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will diable broadcast ip packet trap to cpu
 * INPUT:    mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT:   None
 * RETURN:   TRUE 
 *           FALSE 
 * NOTES :   no core component will call this , because DHCP install trap broadcast ip 
 *           to cpu by DEV_HRDRV_SetInterventionEntry
 *---------------------------------------------------------------------------
 */
BOOL_T DEV_HRDRV_DisableBroadcastIpTrapToCpu(UI32_T mode) ;



/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnableBroadcastIpTrapToCpu
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will enable broadcast ip packet trap to cpu
 * INPUT:    mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : call by lan.c to do cpu storm control 
 * OUTPUT:   None
 * RETURN:   TRUE 
 *           FALSE 
 * NOTES :   no core component will call this , because DHCP install trap broadcast ip 
 *           to cpu by DEV_HRDRV_SetInterventionEntry
 * 
 *---------------------------------------------------------------------------
 */
BOOL_T DEV_HRDRV_EnableBroadcastIpTrapToCpu(UI32_T mode) ;




/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable igmp control packet trap to cpu
 * INPUT   : mode : HRDRV_ADMIN_MODE : call by IGMP compnent
 *                  HRDRV_OP_MODE    : pass by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_EnableIgmpTrap(UI32_T mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DisableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will diable igmp control packet trap to cpu
 * INPUT   : mode : HRDRV_ADMIN_MODE : pass by IGMP compnent
 *                  HRDRV_OP_MODE    : pass by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_DisableIgmpTrap(UI32_T mode);






#if(SYS_CPNT_ACL == TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_AddSysDefinedAclMaskEntry
 *---------------------------------------------------------------------------
 * PURPOSE:  to add sys defined mask for l4 
 * INPUT:    mask
 * OUTPUT:   None
 * RETURN:   TRUE / FALSE
 * NOTE:     call by hrdrv.c when init 
 ----------------------------------------------------------------
 */ 
BOOL_T DEV_HRDRV_AddSysDefinedAclMaskEntry(HRDRV_TYPE_AclMaskEntry_T *mask);


    
/* -------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_DelMaskTbl
 * -------------------------------------------------------------------------
 * FUNCTION: use to delete a user-defined mask table
 * INPUT   : type  -- one of HRDRV_MaskType_T
 *                 HRDRV_IP_MASK
 *                 HRDRV_MAC_MASK
 *           inout -- one of HRDRV_Inout_T
 *                HRDRV_INBOUND
 *                HRDRV_OUTBOUND
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1. before an ace can write to FFP, mask must be added first
 *              sys_defined mask is created by hrdrv automaticaly
 *           2. if a mask is used by irule, it will return FALSE
 * -------------------------------------------------------------------------*/ 
BOOL_T DEV_HRDRV_DelAclMaskEntry(HRDRV_TYPE_AclMaskEntry_T *mask);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_AddAclMaskEntry
 * -------------------------------------------------------------------------
 * FUNCTION: use to add a user-defined a mask
 * INPUT   : mask
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : before an ace can write to hrdrv, mask must be added first
 *           sys_defined mask is created by hrdrv automaticaly
 * -------------------------------------------------------------------------*/  
BOOL_T DEV_HRDRV_AddAclMaskEntry(HRDRV_TYPE_AclMaskEntry_T *mask);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_ModifyAclMaskEntry
 * -------------------------------------------------------------------------
 * FUNCTION: modify a existed acl mask entry
 * INPUT   : old -- the mask to be replaced 
 *           input -- new setting
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : if a mask is in-use, the operation is not allowed 
 * -------------------------------------------------------------------------*/  
BOOL_T DEV_HRDRV_ModifyAclMaskEntry(HRDRV_TYPE_AclMaskEntry_T *old, HRDRV_TYPE_AclMaskEntry_T *input) ;



/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_DelAclMaskTbl
 * -------------------------------------------------------------------------
 * FUNCTION: use to delete a user-defined mask table
 * INPUT   : type  -- one of HRDRV_MaskType_T
 *                 HRDRV_IP_MASK
 *                 HRDRV_MAC_MASK
 *           inout -- one of HRDRV_Inout_T
 *                HRDRV_INBOUND
 *                HRDRV_OUTBOUND
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1. before an ace can write to hrdrv, mask must be added first
 *              sys_defined mask is created by hrdrv automaticaly
 *           2. if a mask is used by irule, it will return FALSE
 * -------------------------------------------------------------------------*/ 
BOOL_T DEV_HRDRV_DelAclMaskTbl(UI32_T type, UI32_T inout);

#endif

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_Reserve
 *---------------------------------------------------------------------------
 * PURPOSE:  reserve the resouce for a L4 rule 
 * INPUT:    rid, re
 * OUTPUT:   NONE
 * RETURN:   TRUE / FALSE 
 *           TRUE : hardware has resouce for this L4 rule
 *           FALSE: short of resouce for this L4 rule
 * NOTE:     
 *         
 *---------------------------------------------------------------------------
 */    
BOOL_T DEV_HRDRV_Reserve(UI32_T rid, HRDRV_TYPE_ResouceEntity_T *re);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_Abort
 *---------------------------------------------------------------------------
 * PURPOSE:  releas all the resocues allocate for this trasaction id 
 * INPUT:    rid : transaction id 
 * OUTPUT:   None.
 * RETURN:   TRUE / FALSE
 * NOTE:     a transaction is a atomic operation, if any of the allocate faield for
 *           this transaction, then user must abort this transaction instead of commit it 
 *---------------------------------------------------------------------------
 */     
BOOL_T DEV_HRDRV_Abort(UI32_T rid);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_Commit
 *---------------------------------------------------------------------------
 * PURPOSE:  to write the l4 rule with the same rid to ASIC
 * INPUT:    rid;
 * OUTPUT:   None.
 * RETURN:   TRUE / FALSE
 * NOTE:     
 *          
 *----------------------------------------------------------------------------
 */   
BOOL_T DEV_HRDRV_Commit(UI32_T rid);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_Release
 *---------------------------------------------------------------------------
 * PURPOSE:  delete the L4 rule from the ASIC 
 * INPUT:    re
 * OUTPUT:   NONE
 * RETURN:   TRUE / FALSE 
 * NOTE:     
 *           
 *---------------------------------------------------------------------------
 */ 
BOOL_T DEV_HRDRV_Release(HRDRV_TYPE_ResouceEntity_T *re);



/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_AllocateClassInstantRateLimitID
 *---------------------------------------------------------------------------
 * PURPOSE:  to allocate the meter resouces for instance of the ACL class binding 
 *           in a interface 
 * INPUT:    fun_type, unit, port
 *           rate : Kbps, 
 *           burst_size : bytes
 *           class_instant_id : is defined by L4 core layer , a unique key to indentify the meter resouce
 * OUTPUT:   support_burst_size : is the burst size supported in chipset
 * RETURN:   TRUE / FALSE
 * NOTE:     
 *---------------------------------------------------------------------------
 */
BOOL_T DEV_HRDRV_AllocateClassInstantRateLimitID(UI32_T fun_type, UI32_T unit, UI32_T port, UI32_T class_instant_id, UI32_T rate, UI32_T burst_size,  UI32_T *support_burst_size);



/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_FreeClassInstantRateLimitID
 *---------------------------------------------------------------------------
 * PURPOSE:  to free meter resouces for instance of the ACL class binding 
 *           in a interface 
 * INPUT:    fun_type, unit, port
 *           class_instant_id : is defined by L4 core layer, a key to identify the
 *                              meter resource
 * OUTPUT:   
 * RETURN:   TRUE / FALSE
 * NOTE:     
 *---------------------------------------------------------------------------
 */
BOOL_T DEV_HRDRV_FreeClassInstantRateLimitID(UI32_T fun_type, UI32_T unit, UI32_T port, UI32_T class_instant_id);




/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_Reinit
 *---------------------------------------------------------------------------
 * PURPOSE:  to init dev_hrdrv.c 
 * INPUT  :  None.
 * OUTPUT :  None.        
 * RETURN :  None.
 * NOTE   :  it is called when enter master and slave mode 
 *           this API will reset the FFP table and internal data 
 *---------------------------------------------------------------------------
*/
BOOL_T DEV_HRDRV_Reinit();



/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_EnableTrapOspfToCpu
 *---------------------------------------------------------------------------
 * PURPOSE:  to enable trap ospf protocl packet  to CPU
 * INPUT:    *cpu_mac : cpu mac address(6 bytes)
 * OUTPUT:   None
 * RETURN:   TRUE  
 *           FALSE -- if trap ospf is not supported
 * NOTES :   This API will add three rule and one mask
 *           1)Eth-Type=0800, Protocol Type=89(0x59), DA=CPU Mac-address
 *           2)Eth-Type=0800, Protocol Type=89(0x59), DA=01005e000005
 *           3)Eth-Type=0800, Protocol Type=89(0x59), DA=01005e000006
 *---------------------------------------------------------------------------
 */
BOOL_T DEV_HRDRV_EnableTrapOspfToCpu(UI8_T *cpu_mac);



/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_DisableTrapOspfToCpu
 *---------------------------------------------------------------------------
 * PURPOSE:  to disable trap ospf protocl packet  to CPU
 * INPUT:    *cpu_mac : cpu mac address(6 bytes)
 * OUTPUT:   None
 * RETURN:   TRUE  
 *           FALSE -- if trap ospf is not supported
 * NOTES :   This API will remvoe mask and rule from FFP
 *           1)Eth-Type=0800, Protocol Type=89(0x59), DA=CPU Mac-address
 *           2)Eth-Type=0800, Protocol Type=89(0x59), DA=01005e000005
 *           3)Eth-Type=0800, Protocol Type=89(0x59), DA=01005e000006
 *---------------------------------------------------------------------------
 */
BOOL_T DEV_HRDRV_DisableTrapOspfToCpu(UI8_T *cpu_mac);





/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_EnableTrapArpReply
 * -------------------------------------------------------------------------
 * FUNCTION: to enable trap arp reply packet to cpu
 * INPUT   : mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : pass by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is call when CPU STORM of clas arp reply is off
 *           no core layer component will call this, arp reply entry is trap to 
 *           cpu by l2 mac address 
 * -------------------------------------------------------------------------*/  
BOOL_T DEV_HRDRV_EnableTrapArpReply(UI32_T mode);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_DisableTrapArpReply
 * -------------------------------------------------------------------------
 * FUNCTION: to disable trap arp reply packet to cpu
 * INPUT   : mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : pass by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is call when CPU STORM of clas arp reply is off
 *           no core layer component will call this, arp reply entry is trap to 
 *           cpu by l2 mac address 
 * -------------------------------------------------------------------------*/  
BOOL_T DEV_HRDRV_DisableTrapArpReply(UI32_T mode);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_EnableMyMacTrap2Cpu
 * -------------------------------------------------------------------------
 * FUNCTION: This function will only used in master unit, not work in slave unit
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is for ip unicast only 
 * -------------------------------------------------------------------------*/  
BOOL_T DEV_HRDRV_EnableMyMacTrap2Cpu(UI32_T mode);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_DisableMyMacTrap2Cpu
 * -------------------------------------------------------------------------
 * FUNCTION: TThis function will only used in master unit, not work in slave unit
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 
 * -------------------------------------------------------------------------*/  
BOOL_T DEV_HRDRV_DisableMyMacTrap2Cpu(UI32_T mode);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_EnableTrapArpToCpu
 * -------------------------------------------------------------------------
 * FUNCTION: to disable packet with DA =master's mac address cpu 
 * INPUT   : mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : pass by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is call when CPU STORM of my mac class is on
 * -------------------------------------------------------------------------*/  
BOOL_T DEV_HRDRV_DisableMyMacTrap2Cpu(UI32_T mode);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - HRDRV_EnableTrapArpToCpu
 *---------------------------------------------------------------------------
 * FUNCTION: to enable trap arp request packet to cpu
 * INPUT   : mode : HRDRV_ADMIN_MODE : no core component will call this 
 *                  HRDRV_OP_MODE    : pass by lan.c to do cpu storm control 
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : this is call when CPU STORM of clas arp is off
 *           no core layer component will call this, arp entry is trap to CPU 
 *           by HRDRV_SetInterventionEntry
 *---------------------------------------------------------------------------
 */
BOOL_T DEV_HRDRV_EnableTrapArpToCpu(UI32_T mode);





/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_SetPortSTAState
 * -------------------------------------------------------------------------
 * FUNCTION:  to set the port spanning tree state 
 * INPUT   : unit, port, state
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 1: if port is not in forwarding state, 
 *              set config.FIL_EN to disable the FFP function 
 *              it is called in lan.c to avoid receive any packet trap by FFP
 *           2. if port is in forwarding state, 
 *              set config.FIL_EN to enable the FFP function to trap traffic from this 
 *              port to cpu 
 *           3. this API only used in XGS chipset, because we can turn off FFP by port
 *           
 * -------------------------------------------------------------------------*/  
BOOL_T DEV_HRDRV_SetPortSTAState(UI32_T unit, UI32_T port, UI32_T state);




/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_PrintResouces
 *---------------------------------------------------------------------------
 * PURPOSE:  to print the resocues available
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   TRUE 
 *           FALSE --
 * NOTES :   
 *---------------------------------------------------------------------------
 */
void DEV_HRDRV_PrintResouces();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_CloseDebugMsg
 *---------------------------------------------------------------------------
 * PURPOSE:  close the debug msg
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   TRUE 
 *           FALSE --
 * NOTES :   
 *---------------------------------------------------------------------------
 */
void DEV_HRDRV_CloseDebugMsg();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_OpenDebugMsg
 *---------------------------------------------------------------------------
 * PURPOSE:  open the debug msg
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   TRUE 
 *           FALSE --
 * NOTES :   
 *---------------------------------------------------------------------------
 */
void DEV_HRDRV_OpenDebugMsg();

/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_SetInterventionEntry
 *------------------------------------------------------------------------------
 * Purpose  : Packets with this MAC in the specified VLAN will trap to CPU.
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address                                     
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : This API doesn't forbid that CPU has only one MAC.  It will only 
 *            depend on spec.                                                            
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_SetInterventionEntry(UI32_T vid, UI8_T *mac);

/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_DeleteInterventionEntry
 *------------------------------------------------------------------------------
 * Purpose  : Delete specific MAC address from the list of traping packet to CPU.
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address                                     
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : None.                                                    
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_DeleteInterventionEntry(UI32_T vid, UI8_T *mac);



/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_RemoveUnknownIPMC2Cpu
 *------------------------------------------------------------------------------
 * Purpose  : Delete Unknown IPMC rule from FFP (da=0x01005exxxxxx, ethtype=0x0800)
 * INPUT    : None                                    
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : Called by SWDRV_LocalDisableUMCASTMacTrap                                                    
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_RemoveUnknownIPMC2Cpu();

/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_AddUnknownIPMC2Cpu
 *------------------------------------------------------------------------------
 * Purpose  : Add Unknown IPMC rule from FFP (da=0x01005exxxxxx, ethtype=0x0800)
 * INPUT    : None                                    
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : Called by SWDRV_LocalEnableUMCASTMacTrap                                                    
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_AddUnknownIPMC2Cpu();


/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_AddMyMACIP2Cpu
 *------------------------------------------------------------------------------
 * Purpose  : Add MgmtMAC and IP address rules 
 * INPUT    : mac --> mgmt mac address, ip --> mgmt ip                                    
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : Now support 5 entries(l3) for this feature                                                    
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_AddMyMACIP2Cpu(UI8_T *mac, UI32_T ip);

/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_RemoveMyMACIP2Cpu
 *------------------------------------------------------------------------------
 * Purpose  : Delete MgmtMAC and IP address rules 
 * INPUT    : mac --> mgmt mac address, ip --> mgmt ip                                    
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : Now support 5 entries(l3) for this feature                                                    
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_RemoveMyMACIP2Cpu(UI8_T *mac, UI32_T ip);


/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_AddRIPv12Cpu
 *------------------------------------------------------------------------------
 * Purpose  : add rip v1/v2 rules (protocol=udp, port=520, ethtype=0x0800)
 * INPUT    : None                                   
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : V1/V2 share the same rule                                                   
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_AddRIPv12Cpu();

/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_RemoveRIPv12Cpu
 *------------------------------------------------------------------------------
 * Purpose  : Remove rip v1/v2 rules (protocol=udp, port=520, ethtype=0x0800)
 * INPUT    : None                                   
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : V1/V2 share the same rule                                                   
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_RemoveRIPv12Cpu();

/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_AddPIM2Cpu
 *------------------------------------------------------------------------------
 * Purpose  : Add PIM rules
 * INPUT    : None                                   
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     :                                             
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_AddPIM2Cpu();


/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_RemovePIM2Cpu
 *------------------------------------------------------------------------------
 * Purpose  : Add PIM rules
 * INPUT    : None                                   
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     :                                             
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_RemovePIM2Cpu();

/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_AddDot1xAuth2Cpu
 *------------------------------------------------------------------------------
 * Purpose  : Add dot1x auth rules
 * INPUT    : UI8_T *mac                                   
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : for backdoor only                                            
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_AddDot1xAuth2Cpu(UI8_T *mac);


/*------------------------------------------------------------------------------
 * Function : DEV_HRDRV_RemoveDot1xAuth2Cpu
 *------------------------------------------------------------------------------
 * Purpose  : Add dot1x auth rules
 * INPUT    : UI8_T *mac                                   
 * OUTPUT   : None                                                           
 * RETURN   : BOOL_T Status - True : successs, False : failed                
 * NOTE     : for backdoor only                                            
 *------------------------------------------------------------------------------*/
BOOL_T DEV_HRDRV_RemoveDot1xAuth2Cpu(UI8_T *mac);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_SetDot1xAuthTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will support trap dot1x mac packet
 * INPUT   : unit  --- unit id
 *           port  --- unit port
 *           mode      --  DEV_SWDRV_DOT1X_PACKET_DISCARD = 0,
 *                         DEV_SWDRV_DOT1X_PACKET_FORWARD,
 *                         DEV_SWDRV_DOT1X_PACKET_TRAPTOCPU
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : so this function will set ETHER_TYPE=888E packet to switch/trap/discard
 * -------------------------------------------------------------------------*/  
BOOL_T DEV_HRDRV_SetDot1xAuthTrap(UI32_T unit, UI32_T port, UI32_T mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_AddIPv6ToCpu()
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to trap IPv6 packets to CPU
 * INPUT   : mac	- DA MAC Address
 *			 ipAddr - DstIP Address 
 *			 type	- packet types
 *			 pri	- priority value
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 
 * -------------------------------------------------------------------------*/	

BOOL_T DEV_HRDRV_AddIPv6ToCpu(UI8_T mac[],UI8_T ipaddr[],UI32_T type, UI32_T pri);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_HRDRV_DeleteIPv6ToCpu()
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to trap IPv6 packets to CPU
 * INPUT   : mac	- DA MAC Address
 *			 ipAddr - DstIP Address 
 *			 type	- packet types
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : 
 * -------------------------------------------------------------------------*/	

BOOL_T DEV_HRDRV_RemoveIPv6ToCpu(UI8_T mac[],UI8_T ipaddr[],UI32_T type);



#endif









    







    


 


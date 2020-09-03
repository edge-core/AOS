/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP_ALGO.h                                                                 
 *                                                                                      
 * PURPOSE: This package provides the data types used in VRRP algorithm used (RFC 2338)
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009                               
 * -------------------------------------------------------------------------------------*/
 
 

#ifndef __VRRP_VM_H
#define __VRRP_VM_H

/*#include <sock_port.h>*/
#include <sys_type.h> 
#include "vrrp_task.h"
#include "vrrp_type.h"
#include "l_mm_type.h"

/* TYPE DECLARATIONS 
 */
 
/* EXPORTED SUBPROGRAM DECLARATION
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_RxVrrpPkt
 *--------------------------------------------------------------------------
 * PURPOSE  : When receive ADVERTISEMENT packet, this function handle mref release
 * INPUT    : mref_handle_p     --  mref
 *            pkt_length        --  packet length
 *            dst_mac           --  destination MAC address
 *            src_mac           --  source MAC address
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
void VRRP_VM_RxVrrpPkt(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI32_T    ifindex,
    UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN]
);

BOOL_T VRRP_VM_Startup(UI32_T if_index, UI8_T vrid);
BOOL_T VRRP_VM_ShutDown(UI32_T if_index, UI8_T vrid);
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_Check_Master_Down_Timer
 *--------------------------------------------------------------------------
 * PURPOSE  : This function actions with Master_Down_Timer Expire event
 * INPUT    : vrrp_oper_entry   --  VRRP operation entry
 *            pass_time         --  passed time interval in ticks
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_VM_Check_Master_Down_Timer(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI32_T pass_time);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_Check_Adver_Timer
 *--------------------------------------------------------------------------
 * PURPOSE  : This function actions with Adver_Timer Expire event
 * INPUT    : vrrp_oper_entry   --  VRRP operation entry
 *            pass_time         --  passed time interval in ticks
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_VM_Check_Adver_Timer(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI32_T pass_time);

void VRRP_VM_Init(void);
 
/* NAMING CONSTANT DECLARATION 
 */
#define IP_PROT_VRRP            112             /* IP protocol number -- rfc2338.5.2.4*/
#define INADDR_VRRP_GROUP       0xe0000012	    /* multicast addr - rfc2338.5.2.2 */
#define VRRP_IP_TTL	            255	            /* in and out pkt ttl -- rfc2338.5.2.3 */
#define VRRP_VERSION	        2	            /* current version -- rfc2338.5.3.1 */
#define VRRP_PKT_ADVERT	        1	            /* packet type -- rfc2338.5.3.2 */
#define VRRP_PRIO_DFL	        100	            /* default priority -- rfc2338.5.3.4 */
#define VRRP_PRIO_STOP	        0	            /* priority to stop -- rfc2338.5.3.4 */
#define VRRP_AUTH_NONE	        0	            /* no authentification -- rfc2338.5.3.6 */
#define VRRP_AUTH_PASS	        1	            /* password authentification -- rfc2338.5.3.6 */
#define VRRP_AUTH_AH	        2	            /* AH(IPSec) authentification - rfc2338.5.3.6 */
#define VRRP_ADVER_DFL	        1	            /* advert. interval (in sec) -- rfc2338.5.3.7 */
#define VRRP_PREEMPT_DFL        1	            /* rfc2338.6.1.2.Preempt_Mode */

#define VRRP_VM_INTERNAL_ERROR                0x80000001
#define VRRP_VM_ADVER_TIMER_EXPIRE            0x80000002
#define VRRP_VM_ADVER_TIMER_NOT_EXPIRE        0x80000003
#define VRRP_VM_MASTER_DOWN_TIMER_EXPIRE      0x80000004
#define VRRP_VM_MASTER_DOWN_TIMER_NOT_EXPIRE  0x80000005
#define VRRP_VM_PREEMPT_DELAY_TIMER_EXPIRE    0x80000006
#define VRRP_VM_PREEMPT_DELAY_TIMER_NOT_EXPIRE 0x80000007

#endif




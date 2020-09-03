/* MODULE NAME:  k_iml_mgr.h
 * PURPOSE:
 *  header file for IML_MGR linux kernel implementation
 *
 * NOTES:
 *
 * HISTORY
 *    7/23/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef K_IML_MGR_H
#define K_IML_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* liunx kernel header files */
#include "linux/skbuff.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IML_MGR_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize IML_MGR.
 *
 * 	INPUT:   
 *   None.
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   None.
 *
 * 	NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
void IML_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE:
 *   This function initializes all function pointer registration operations.
 *
 * INPUT:
 *   None.
 *    
 * OUTPUT:
 *   None.
 *
 * RETURN
 *   None.
 *
 * NOTES:
 *   None.
 *--------------------------------------------------------------------------*/
void IML_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IML_MGR_EnqToIPTxPktList
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   This function will enqueue the ip packet from linux kernel tcp/ip stack
 *   to the tail of the tx packet list.
 *
 * 	INPUT:
 *   skb  --  The packet to be transmitted. This skb will be freed when return.
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   IML_TYPE_RETVAL_OK                           --  Success
 *   IML_TYPE_RETVAL_L_MM_ALLOCATE_TX_BUFFER_FAIL --  Fail to allocate tx buffer
 *   IML_TYPE_RETVAL_TX_BUF_FULL                  --  IP packet tx buf is full
 *   IML_TYPE_RETVAL_GET_VID_FROM_VLAN_DEV_ERROR  --  Fail to get vid from vlan net device
 *
 * 	NOTES:
 *   1. Need not to free skb in this function, the caller VLAN_NET_hard_start_xmit
 *      will take care of it.
 *   2. Pdu of the output mref_handle will point to the ip header.
 *   
 *--------------------------------------------------------------------------
 */
UI32_T IML_MGR_EnqToIPTxPktList(struct sk_buff *skb);

#endif    /* End of K_IML_MGR_H */


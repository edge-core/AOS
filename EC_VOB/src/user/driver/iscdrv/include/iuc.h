/* Module Name: IUC.H
 * Purpose: 
 *      This module provides an interface for Inter-Unit-Communucation(which IUC stands for)
 *      between stacked switch units. IUC relay packet from ISC to NIC and passing received 
 *      packet from NIC to ISC.
 * Notes: 
 * History:                                                               
 *    
 * Copyright(C)      Accton Corporation, 2005   				
 */
 
#ifndef _IUC_H
#define _IUC_H

/* INCLUDE FILE DECLARATIONS
 */
#include "l_mm.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define IUC_STACK_UNIT_NEXT       0
#define IUC_STACK_UNIT_ALL        0xFFFF
#define IUC_STACK_UNIT_BMP(unit)  BIT_VALUE(unit - 1)

/* EXPORTED FUNCTION SPECIFICATIONS
 */

/* FUNCTION NAME:   IUC_Init
 * PURPOSE: 
 *          Initial system variables of IUC
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:   
 */
void IUC_Init(void);


/* FUNCTION NAME : IUC_Create_InterCSC_Relation
 * PURPOSE: 
 *          This function initiates all function pointer registration operations.
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          Register a callback function IUC_PacketArrival() to dev_nicdrv
 */
void IUC_Create_InterCSC_Relation(void);


/* FUNCTION NAME:   IUC_SendPacket
 * PURPOSE: 
 *          Send an IUC packet
 * INPUT:   
 *          mem_ref          -- points to the data block that holds the packet content
 *          dest_unit        -- unit_id or IUC_STACK_UNIT_NEXT, IUC_STACK_UNIT_ALL
 *          Uplink_Downlink  -- LAN_TYPE_TX_UP_LINK: Uplink
 *                              LAN_TYPE_TX_DOWN_LINK: Downlink
 *                              LAN_TYPE_TX_UP_DOWN_LINKS: Uplink and Downlink
 *          priority         -- the send packet priority
 * OUTPUT:  
 *          None
 * RETURN:  
 *          TRUE        -- success
 *          FALSE       -- fail
 * NOTES:   
 */
BOOL_T  IUC_SendPacket(L_MM_Mref_Handle_T *mem_ref, UI32_T dst_unit, UI8_T Uplink_Downlink, UI32_T priority);


/* FUNCTION NAME:   ICU_GetIUCEthHeaderLen
 * PURPOSE: 
 *          Get the IUC_ETHERNET_HEADER_LEN of IUC packet
 * INPUT:   
 *          None
 * OUTPUT:  
 *          length
 * RETURN:  
 *          TRUE        -- success
 *          FALSE       -- fail
 * NOTES:   
 */
BOOL_T ICU_GetIUCEthHeaderLen(UI16_T *length);

#endif

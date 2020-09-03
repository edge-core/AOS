/* Module Name: STKTPLG_TX.H
 *
 * Purpose: 
 *
 * Notes:
 *
 * History:
 *    10/04/2002       -- David Lin, Create
 *
 * Copyright(C)      Accton Corporation, 2002 - 2005
 */
 
 
#ifndef  STKTPLG_TX_H
#define  STKTPLG_TX_H 

/* INCLUDE FILE DECLARATIONS
 */
 
#include "sys_type.h"
#include "stktplg_om.h"
#include "l_mm.h"
#include "sys_adpt.h"

/* EXPORTED SUBPROGRAM DECLARATIONS
 */

/* FUNCTION NAME : STKTPLG_TX_SendHBTType0
 * PURPOSE: This function sends out HBT type 0
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Normal (0)
 *                     Bounce (1)
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHBTType0(L_MM_Mref_Handle_T *mref_handle_rx, UI32_T mode, UI32_T port);


/* FUNCTION NAME : STKTPLG_TX_SendHBTType1
 * PURPOSE: This function sends out HBT type 1
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Normal (0)
 *                     Bounce (1)
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHBTType1(BOOL_T flag, L_MM_Mref_Handle_T *mref_handle_rx, UI32_T mode, UI32_T port);


/* FUNCTION NAME : STKTPLG_TX_SendHBTType2
 * PURPOSE: This function sends out HBT type 2
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          query_unit -- if mem_ref is NULL, means we need to send out query packet
 *                        we need this parameter to know which unit that we want
 *                        to query.
 *          mode    -- Normal (0)
 *                     Bounce (1)
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHBTType2(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T query_unit, UI32_T mode, UI32_T port);


/* FUNCTION NAME : STKTPLG_TX_SendHello
 * PURPOSE: This function sends out Hello
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- HELLO 0 or HELLO 1 PDU
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHello(L_MM_Mref_Handle_T *mref_handle_rx, UI32_T mode, UI32_T port);


/* FUNCTION NAME : STKTPLG_TX_SendHelloType1
 * PURPOSE: This function sends out Hello Type 1
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Type 1 Enquire
 *                  -- Type 1 Ready   
 *
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHelloType1(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T mode);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME : STKTPLG_TX_SendTCN
 * PURPOSE: This function sends out TCN
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          port    -- UP (0) / DOWN (1) stacking port
 *          renumber-- TRUE - This TCN packet is due to renumber command
 *                     FALSE- This is a normal TCN packet
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendTCN(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T  port, BOOL_T renumber);

/* FUNCTION NAME : STKTPLG_TX_SendTCNType1
 * PURPOSE: This function sends out TCN Type 1 packet
 * INPUT:   mref_handle_rx -- packet that we need to handle.
 *                            if NULL, means we should create a new one and send it.
 *          port           -- UP (0) / DOWN (1) stacking port
 *          exist_units    -- The units bmp for which master think there are exist
 * OUTPUT:  None.
 * RETURN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendTCNType1(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T  port, UI16_T exist_units);
#else
/* FUNCTION NAME : STKTPLG_TX_SendTCN
 * PURPOSE: This function sends out TCN
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *
 */
void STKTPLG_TX_SendTCN(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T src_port);
#endif

/* FUNCTION NAME : STKTPLG_TX_SendHBTType0Ack
 * PURPOSE: This function sends out HBT type 0
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Normal (0)
 *                     Bounce (1)
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHBTType0Ack(L_MM_Mref_Handle_T *mref_handle_rx, UI32_T mode, UI32_T port);


/* FUNCTION NAME : STKTPLG_TX_SendClosedLoopTCN
 * PURPOSE: This function sends out Closed Loop TCN
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          mode    -- Normal
 *                  -- Bounce   
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendClosedLoopTCN(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T mode, UI8_T port);

BOOL_T STKTPLG_TX_GetDebugMode(void);
void STKTPLG_TX_ToggleDebugMode(void);

void STKTPLG_TX_SendExpNotify(L_MM_Mref_Handle_T *mref_handle_rx, UI32_T port);

/* FUNCTION NAME : STKTPLG_TX_SendTplgSync
 * PURPOSE: This function sends out topology sync.
 * INPUT:   mem_ref  -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 *          sync_bmp -- Unit bit map need to sync to.
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:   None.
 *          
 */
void STKTPLG_TX_SendTplgSync(L_MM_Mref_Handle_T *mref_handle_rx, UI32_T port, UI8_T sync_bmp);


/* FUNCTION NAME : STKTPLG_TX_SendPacketToExp
 *-----------------------------------------------------
 * PURPOSE: This function sends  packet to option module
 * INPUT:   None.
 *                     
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *-----------------------------------------------------          
 */
void STKTPLG_TX_SendPacketToExp(void);

/* FUNCTION NAME : STKTPLG_TX_SendResetPacketToExp
 *--------------------------------------------------------
 * PURPOSE: This function sends  reset packet to option module
 * INPUT :  None.
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *-----------------------------------------------------          
 */
void STKTPLG_TX_SendResetPacketToExp(void);



/* FUNCTION NAME : STKTPLG_TX_SendHBT
 * PURPOSE: This function sends out Halt
 * INPUT:   mem_ref -- packet that we need to handle.
 *                     if NULL, means we should create a new one and send it.
 *          port    -- UP (LAN_TYPE_TX_UP_LINK) / DOWN (LAN_TYPE_TX_DOWN_LINK) stacking port
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKTPLG_TX_SendHBT(L_MM_Mref_Handle_T *mref_handle_rx, UI8_T mode, UI8_T port);

#endif  /* STKTPLG_TX_H */


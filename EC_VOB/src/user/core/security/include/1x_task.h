/* Project Name: New Feature
 * File_Name : 1x_task.h
 * Purpose     : 1X initiation and TACACS task creation
 *
 * 2002/06/25    : Kevin Cheng     Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */
 
#ifndef	DOT1X_TASK_H
#define	DOT1X_TASK_H 
 
#include "sys_type.h"
#include "l_mm.h"

/*---------------------------------------------------------------------------
 * Routine Name : DOT1X_TASK_CreateDOT1XTask()
 *---------------------------------------------------------------------------
 * Function : Create and start DOT1X task
 * Input    : None								                             +
 * Output   :
 * Return   : never returns
 * Note     :
 *---------------------------------------------------------------------------*/
void DOT1X_TASK_CreateDOT1XTask(void);

/*---------------------------------------------------------------------------+
 * Routine Name : DOT1X_TASK_Initiate_System_Resources                                          +
 *---------------------------------------------------------------------------+
 * Function : Initialize 1X 's Task .	                                     +
 * Input    : None                                                           +
 * Output   : None                                                               +
 * Return   : None                                                +
 * Note     : None                                                               +
 *---------------------------------------------------------------------------*/
BOOL_T DOT1X_TASK_Initiate_System_Resources(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_TASK_SetTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void DOT1X_TASK_SetTransitionMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_TASK_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void 	DOT1X_TASK_EnterTransitionMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_TASK_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enter Master mode calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void DOT1X_TASK_EnterMasterMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_TASK_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enter Slave mode calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void DOT1X_TASK_EnterSlaveMode();

#if (SYS_CPNT_NETACCESS == FALSE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_TASK_AnnounceDOT1XduPacket
 *-------------------------------------------------------------------------
 * PURPOSE  : Whenever Network Interface received a DOT1X packet,it calls
 *            this function to handle the packet.
 * INPUT    :
 *      L_MM_Mref_Handle_T *mem_ref   -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 * RETURN   :   none
 * NOTE:
 *      This function is called at interrupt time, so it need to be fast.
 *      To reduce the processing time, this function just create a message
 *      that contain the buffer pointer and other packet information. Then
 *      it sends the message to the packet queue. And it sends an event to
 *      the DOT1X task to notify the arrival of the DOT1X packet. The DOT1X task
 *      will handle the packet after it get out the message from the packet
 *      message queue. It is the receiving routine's responsibility to call
 *      the L_MM_Mref_Release() function to free the receiving buffer.
 *-------------------------------------------------------------------------
 */
void DOT1X_TASK_AnnounceDOT1XPacket  ( L_MM_Mref_Handle_T *mem_ref,
                                            UI8_T    *dst_mac,
                                            UI8_T    *src_mac,
                                            UI16_T   tag_info,
                                            UI16_T   type,
                                            UI32_T   pkt_length,
                                            UI32_T   unit_no,
                                            UI32_T   port_no);
#endif /* #if (SYS_CPNT_NETACCESS == FALSE) */
 /*added by Jinhua Wei ,to remove warning ,becaued the relative function's head file not declared */
/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - DOT1X_TASK_Create_InterCSC_Relation
 * ------------------------------------------------------------------------
 *  PURPOSE  : This function initializes all function pointer registration operations.
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : None
 * ------------------------------------------------------------------------*/
void DOT1X_TASK_Create_InterCSC_Relation(void);

#endif /*DOT1X_TASK_H*/
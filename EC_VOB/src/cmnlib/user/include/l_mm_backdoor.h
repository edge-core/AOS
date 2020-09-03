/* MODULE NAME:  l_mm_backdoor.h
 * PURPOSE:
 *     User space l_mm backdoor
 *
 * NOTES:
 *
 * HISTORY
 *    2007/10/9 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef L_MM_BACKDOOR_H
#define L_MM_BACKDOOR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "backdoor_mgr.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{   
    UI32_T  msgq_key;	
    UI32_T  bd_choose;	           
    UI32_T  task_id;
    UI32_T  module_id;
    UI32_T  elapsed_time;
    void    *dump_buf_addr_p;
} L_MM_BACKDOOR_IpcMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_BACKDOOR_BackDoorMenu
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    L_MM kernel space backdoor menu entry funtion.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    This function should be called directly by backdoor_mgr which handles backdoor
 *    transactions.
 *------------------------------------------------------------------------------
 */
void K_L_MM_BACKDOOR_BackDoorMenu(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_BackDoorMenu
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    L_MM kernel space backdoor menu entry funtion.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    This function should be called directly by backdoor_mgr which handles backdoor
 *    transactions.
 *------------------------------------------------------------------------------
 */
void L_MM_BACKDOOR_BackDoorMenu(void);
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_BackDoorMenu4Process
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    L_MM user space backdoor menu entry funtion for process.
 *
 * INPUT:
 *    msgbuf_p
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *  TRUE  -- done
 * NOTES:
 *  According to bd_choose from msgbuf_p, calling the corresponding function.  
 *  (Ex show buff info, toggle flag, dump buffer)
 *------------------------------------------------------------------------------
 */
BOOL_T L_MM_BACKDOOR_BackDoorMenu4Process(SYSFUN_Msg_T* msgbuf_p);

#endif    /* End of L_MM_BACKDOOR_H */


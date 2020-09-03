/* MODULE NAME:  ptp_isr.c
 * PURPOSE:
 *     User space interrupt service routine for PTP.
 *
 * NOTES:
 *     None
 *
 * CREATOR:      Charlie Chen
 * HISTORY
 *    4/27/2012 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

#include "sys_type.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "sysfun.h"
#include "ptp_isr_type.h"
#include "ptp_isr.h"
#include "dev_swdrv.h"
#include "uc_mgr.h"

#if(SYS_CPNT_PTP_ISR == TRUE)

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T PTP_ISR_EnableInterrupt(void);
static BOOL_T PTP_ISR_WaitInterrupt(UI32_T* interrupt_bmp_p);
static void PTP_ISR_Task();

/* STATIC VARIABLE DECLARATIONS
 */

/* IMPORTED SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PTP_ISR_InitiateProcessResource
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all process resource.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PTP_ISR_InitiateProcessResource(void)
{

}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PTP_ISR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PTP_ISR_Create_InterCSC_Relation(void)
{

}


/* FUNCTION NAME : PTP_ISR_CreateTask
 * PURPOSE:
 *      Spawn task for PTP ISR.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void PTP_ISR_CreateTask(void)
{
    UI32_T  task_id;

    if(SYSFUN_SpawnThread(SYS_BLD_PTP_ISR_THREAD_PRIORITY,
                          SYS_BLD_PTP_ISR_THREAD_SCHED_POLICY,
                          SYS_BLD_PTP_ISR_THREAD,
                          SYS_BLD_PTP_ISR_TASK_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          PTP_ISR_Task,
                          NULL,
                          &task_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }

}

/* LOCAL SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : PTP_ISR_Task
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *  	PTP task main function
 *
 * 	INPUT:
 *   None
 *
 * 	OUTPUT:
 *   None
 *
 * 	RETURN:
 *   TRUE  -  PTP interrupt is enabled successfully
 *   FALSE -  Failed to enable PTP interrupt
 *
 * 	NOTES:
 *   PTP interrupt can only be enabled once.
 *--------------------------------------------------------------------------
 */
static void PTP_ISR_Task()
{
    UI32_T interrupt_bmp=0;
    UI16_T int_port =0, i;

    PTP_ISR_EnableInterrupt();

    while(TRUE)
    {
      int_port =0;
      interrupt_bmp=0;
      if(FALSE == PTP_ISR_WaitInterrupt(&interrupt_bmp))
        continue;

      for(i=0; i<3; i++)
      {
        if(((UI32_T)interrupt_bmp) & (1<<i))
        {
          DEV_SWDRV_Phy_HandleInterrupt(0, i);
        }
      }
    }
}

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : PTP_ISR_EnableInterrupt
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *  	Enable PTP interrupt.
 *
 * 	INPUT:
 *   None
 *
 * 	OUTPUT:
 *   None
 *
 * 	RETURN:
 *   TRUE  -  PTP interrupt is enabled successfully
 *   FALSE -  Failed to enable PTP interrupt
 *
 * 	NOTES:
 *   PTP interrupt can only be enabled once.
 *--------------------------------------------------------------------------
 */
static BOOL_T PTP_ISR_EnableInterrupt(void)
{
    UC_MGR_Sys_Info_T sys_info;

    if(UC_MGR_GetSysInfo(&sys_info)==FALSE)
    {
        printf("%s(%d)Failed to get sysinfo from UC.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return (BOOL_T)(SYSFUN_Syscall(SYSFUN_SYSCALL_PTP_ISR, PTP_ISR_TYPE_SYSCALL_CMD_ENABLE_INTERRUPT, sys_info.board_id, 0, 0, 0));
}

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : PTP_ISR_WaitInterrupt
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *  	Wait for the PTP interrupt to be occurred.
 *
 * 	INPUT:
 *   None
 *
 * 	OUTPUT:
 *   The bitmap which contains the pending interrupts. The bit set as 1 menas
 *   that interrupt is pending.
 *
 * 	RETURN:
 *   TRUE  -  At least one interrupt is occurred.
 *   FALSE -  An error occured while waiting on PTP interrupt.
 *
 * 	NOTES:
 *   1. The number of PTP interrupt depends on the hardware design of the project
 *   2. The mapping of real irq number and each bit in interrupt_bmp_p is
 *      determined by SYS_HWCFG_PTP_IRQ_NUM_ARRAY. For example, the status of
 *      bit 0 of interrupt_bmp_p reflect the status of SYS_HWCFG_PTP_IRQ_NUM_ARRAY[0]
 *--------------------------------------------------------------------------
 */
static BOOL_T PTP_ISR_WaitInterrupt(UI32_T* interrupt_bmp_p)
{
    return (BOOL_T)(SYSFUN_Syscall(SYSFUN_SYSCALL_PTP_ISR, PTP_ISR_TYPE_SYSCALL_CMD_WAIT_EVENT, (UI32_T)interrupt_bmp_p, 0, 0, 0));
}

#endif


/* MODULE NAME:  k_dying_gasp.c
 * PURPOSE:
 *   dying gasp implementations for linux kernel.
 * 
 * NOTES:
 *   None.
 *
 * HISTORY
 *    7/29/2010 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Corporation, 2010
 */
 
/* for debug use
 */
/* #define DYING_GASP_DEBUG 0 */

/* INCLUDE FILE DECLARATIONS
 */
/* EdgeCore header files
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "k_sysfun.h"
#include "dying_gasp_type.h"
#include "k_dying_gasp.h"

/* linux kernel header files
 */
#include "linux/wait.h"
#include "linux/irqreturn.h"
#include "linux/interrupt.h"
#include "linux/errno.h"
#include "linux/interrupt.h"

/* include cpu architecutre dependent header files
 * for the irq number SYS_HWCFG_DYING_GASP_IRQ_NUM
 * which defines in sys_hwcfg.h
 */
#ifdef CONFIG_ARCH_FEROCEON_KW
#include "asm-arm/arch-feroceon-kw/irqs.h"
#elif defined(CONFIG_PPC_83xx)
#include "asm/mpc83xx.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define INVALID_IRQ_NO (-1)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static irqreturn_t DYING_GASP_irq_handler(int irq, void *dev_id);
static BOOL_T DYING_GASP_EnableInterrupt(void);
static UI32_T DYING_GASP_Syscall(UI32_T cmd, UI32_T arg1, UI32_T arg2, UI32_T arg3, UI32_T arg4);

/* STATIC VARIABLE DECLARATIONS
 */
static wait_queue_head_t wq;
static BOOL_T dying_gasp_interrupt_asserted=FALSE;
static BOOL_T show_timestamp_to_death=FALSE;
#if defined(SYS_HWCFG_DYING_GASP_IRQ_NUM_GET_FROM_STKTPLG)
static int    irq_no=INVALID_IRQ_NO;
#else
static int    irq_no=SYS_HWCFG_DYING_GASP_IRQ_NUM;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : DYING_GASP_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize DYING_GASP.
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
void DYING_GASP_Init(void)
{
    init_waitqueue_head(&wq);
#if !defined(SYS_HWCFG_DYING_GASP_IRQ_NUM_GET_FROM_STKTPLG)
    DYING_GASP_EnableInterrupt();
#endif


}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DYING_GASP_Create_InterCSC_Relation
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
void DYING_GASP_Create_InterCSC_Relation(void)
{
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_DYING_GASP, DYING_GASP_Syscall);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : DYING_GASP_irq_handler
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *  	This function will be registered to linux kernel for dying gasp
 *      interrupt.
 *
 * 	INPUT:
 *      irq     -- irq number
 *      dev_id  -- a cookie which is passed to request_irq when this
 *                 handler function is registered
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   IRQ_NONE    --  Nothing is handled by this function.
 *   IRQ_HANDLED --  The irq had been handled by this function.
 *
 * 	NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
static irqreturn_t DYING_GASP_irq_handler(int irq, void *dev_id)
{
    /* since dying gasp interrupt is asserted when the
     * system is going to die, simply mask this interrupt
     * and set dying_gasp_interrupt_asserted as TRUE.
     * There is no need to deassert the interrupt signal
     * of the external device.
     */
    disable_irq_nosync(irq_no);
#if defined(DYING_GASP_DEBUG)
    printk("<0>Handle dying gasp irq %d(%d) ts=%lu\n", irq, irq_no, SYSFUN_GetSysTick());
#endif

    dying_gasp_interrupt_asserted=TRUE;
    wake_up(&wq);
    return IRQ_HANDLED;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME : DYING_GASP_EnableInterrupt
 *--------------------------------------------------------------------------
 * PURPOSE: 
 *   This function will enable the dying gasp interrupt.
 *
 * INPUT:   
 *   None.
 *
 * OUTPUT:
 *   None.
 *
 * RETURN:
 *   TRUE  -  Success
 *   FALSE -  Failed to enable interrupt
 *
 * NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
static BOOL_T DYING_GASP_EnableInterrupt(void)
{
    int rc;

    if(irq_no==INVALID_IRQ_NO)
    {
        printk("<0>%s(): Invalid irq number. Failed to enable irq.\n",
            __FUNCTION__);
        return FALSE;
    }

    rc=request_irq(irq_no,
                   DYING_GASP_irq_handler,
                   IRQF_DISABLED,
                   "Dying gasp",
                   (void*)NULL);
    if(rc!=0)
    {
        printk("<0>%s(): Failed to hook dying gasp irq handler on irq %d(rc=%d)\n",
               __FUNCTION__, irq_no, rc);
        return FALSE;
    }

    return TRUE;
}

/* system call implementation
 */

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : DYING_GASP_Syscall
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *  	This function implements the system call for DYING_GASP.
 *
 * 	INPUT:
 *   cmd        --  The command to be executed
 *   arg1-arg4  --  The meaning of arg1 to arg4 depends on the cmd.
 *
 * 	OUTPUT:
 *   arg1-arg4  --  The meaning of arg1 to arg4 depends on the cmd.
 *
 * 	RETURN:
 *   The meaning of the return value depends on the cmd.
 *
 * 	NOTES:
 *   When cmd is invalid, return value is always FALSE
 *--------------------------------------------------------------------------
 */
static UI32_T DYING_GASP_Syscall(UI32_T cmd, UI32_T arg1, UI32_T arg2, UI32_T arg3, UI32_T arg4)
{
    UI32_T ret=TRUE;

    switch(cmd)
    {
        case DYING_GASP_TYPE_SYSCALL_CMD_WAIT_EVENT:
        {
            int rc;

            /* arg1 to arg4 are not used
             */

            rc=wait_event_interruptible(wq, dying_gasp_interrupt_asserted==TRUE);
            if(rc!=0)
            {
                if(rc==EINTR)
                    printk("%s wait event error due to EINTR\n", __FUNCTION__);
                else
                    printk("%s wait event error rc=%d\n", __FUNCTION__, rc);

                ret = FALSE;
            }
            else
            {
                #if defined(DYING_GASP_DEBUG)
                printk("<0>DYING_GASP thread wake up timestamp:%lu\n", SYSFUN_GetSysTick());
                #endif
                if(show_timestamp_to_death)
                {
                    while(1)
                        printk("<0>DYING_GASP timestamp:%lu\n", SYSFUN_GetSysTick());
                }
                dying_gasp_interrupt_asserted=FALSE;
                ret = TRUE;
            }
        }
            break;
        case DYING_GASP_TYPE_SYSCALL_CMD_EMULATE_INTERRUPT: /* for debug */
            /* arg1 to arg4 are not used
             */
            dying_gasp_interrupt_asserted=TRUE;
            wake_up(&wq);
            break;
        case DYING_GASP_TYPE_SYSCALL_CMD_SHOW_TIMESTAMP_TO_DEATH: /* for debug */
            /* arg1: BOOL_T show_timestamp_to_death
             */
            show_timestamp_to_death = arg1 & 0x1;
            printk("<0>show_timestamp_to_death=%d\n", (int)show_timestamp_to_death);
            break;
#if defined(SYS_HWCFG_DYING_GASP_IRQ_NUM_GET_FROM_STKTPLG)
        case DYING_GASP_TYPE_SYSCALL_CMD_ENABLE_INTERRUPT:
            /* arg1: irq no for dying gasp
             */
            irq_no=arg1;
            ret=DYING_GASP_EnableInterrupt();
            break;
#endif
        default:
            printk("%s:Invalid cmd(0x%08lx)\r\n", __FUNCTION__, cmd);
            ret=FALSE;
            break;
    }

    return ret;
}


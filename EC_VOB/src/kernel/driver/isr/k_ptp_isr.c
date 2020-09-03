/* MODULE NAME:  k_ptp_isr.c
 * PURPOSE:
 *   Interrupt service routine for PTP.
 * 
 * NOTES:
 *   None.
 *
 * HISTORY
 *    4/27/2012 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Corporation, 2012
 */
 
/* for debug use
 */
/* #define PTP_ISR_DEBUG 1 */

/* INCLUDE FILE DECLARATIONS
 */
/* EdgeCore header files
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "k_sysfun.h"
#include "ptp_isr_type.h"
#include "k_ptp_isr.h"

/* linux kernel header files
 */
#include "linux/wait.h"
#include "linux/irqreturn.h"
#include "linux/interrupt.h"
#include "linux/errno.h"
#include "linux/interrupt.h"

/* include cpu architecutre dependent header files
 * for the irq number SYS_HWCFG_PTP_IRQ_NUM_ARRAY
 * which defines in sys_hwcfg.h
 */
#ifdef CONFIG_ARCH_FEROCEON_KW
#include "asm-arm/arch-feroceon-kw/irqs.h"
#elif defined(CONFIG_PPC_83xx)
#include "asm/mpc83xx.h"
#endif

#ifdef CONFIG_ES4810_12MV2
#include "gpp/mvGpp.h"
#include "mvOs.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define NUM_OF_IRQ (sizeof(irq_no)/sizeof(irq_no[0]))

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static irqreturn_t PTP_ISR_irq_handler(int irq, void *dev_id);
static BOOL_T PTP_ISR_EnableInterrupt(UI8_T board_id);
static BOOL_T PTP_ISR_InitForSpecificBoard(UI8_T board_id);
static UI32_T PTP_ISR_GetIRQAssertedStatus(void);
static UI32_T PTP_ISR_Syscall(UI32_T cmd, UI32_T arg1, UI32_T arg2, UI32_T arg3, UI32_T arg4);

/* STATIC VARIABLE DECLARATIONS
 */
static wait_queue_head_t wq;
static UI32_T interrupt_pending_bmp=0;
static UI32_T interrupt_disabled_bmp=0;
static BOOL_T interrupt_enabled=FALSE;
static UI8_T  my_board_id;
static int irq_no[]=SYS_HWCFG_PTP_IRQ_NUM_ARRAY;

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : PTP_ISR_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize PTP_ISR.
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
void PTP_ISR_Init(void)
{
    init_waitqueue_head(&wq);
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PTP_ISR_Create_InterCSC_Relation
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
void PTP_ISR_Create_InterCSC_Relation(void)
{
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_PTP_ISR, PTP_ISR_Syscall);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : PTP_ISR_irq_handler
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *  	This function will be registered to linux kernel for PTP
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
static irqreturn_t PTP_ISR_irq_handler(int irq, void *dev_id)
{
    int i;
    BOOL_T is_handled=FALSE;

#if defined(PTP_ISR_DEBUG)
    printk("<0>Handle PTP irq %d ts=%lu\n", irq, SYSFUN_GetSysTick());
#endif

    /* look up the index of this irq
     */
    for(i=0; i<NUM_OF_IRQ; i++)
    {
        if(irq_no[i]==irq)
        {
            interrupt_pending_bmp  |= 1<<i;
            interrupt_disabled_bmp |= 1<<i;
            is_handled=TRUE;
        }
    }


    if(is_handled==FALSE)
    {
#if defined(PTP_ISR_DEBUG)
        printk("<0>Handle Non PTP irq %d ts=%lu\n", irq, SYSFUN_GetSysTick());
#endif
        return IRQ_NONE;
    }

    /* disable irq, the irq will be re-enabled when
     * the signal status of irq pin become de-asserted state
     */
    disable_irq_nosync(irq);
    wake_up(&wq);

    return IRQ_HANDLED;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME : PTP_ISR_EnableInterrupt
 *--------------------------------------------------------------------------
 * PURPOSE: 
 *   This function will enable the PTP interrupt.
 *
 * INPUT:   
 *   board_id - board id
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
static BOOL_T PTP_ISR_EnableInterrupt(UI8_T board_id)
{
    int    rc, i;
    BOOL_T ret=TRUE;

    if(interrupt_enabled==TRUE)
    {
        printk("<0>%s():Interrupt had already been enabled\n", __FUNCTION__);
        return FALSE;
    }

    if(PTP_ISR_InitForSpecificBoard(board_id)==FALSE)
    {
        printk("<0>%s():PTP_ISR_InitForSpecificBoard() failed\n", __FUNCTION__);
        return FALSE;
    }

    my_board_id = board_id;
    for(i=0; i<NUM_OF_IRQ; i++)
    {
        rc=request_irq(irq_no[i],
                       PTP_ISR_irq_handler,
                       IRQF_DISABLED,
                       "PTP",
                       (void*)NULL);
        if(rc!=0)
        {
            printk("<0>%s(): Failed to hook PTP irq handler on irq %d(rc=%d)\n",
                   __FUNCTION__, irq_no[i], rc);
            ret = FALSE;
        }
    }

    if(ret==TRUE)
        interrupt_enabled=TRUE;

    return ret;
}

#ifdef CONFIG_ES4810_12MV2
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : PTP_ISR_InitForSpecificBoard
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *  	This function will initialize registers related to PTP interrupt for
 *      the specified board id.
 *
 * 	INPUT:
 *      board_id -- board id
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   TRUE        --  Success
 *   FALSE       --  Failed
 *
 * 	NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
static BOOL_T PTP_ISR_InitForSpecificBoard(UI8_T board_id)
{
    UI32_T gppData;
    MV_STATUS rc;

    /* ptp isr is done through GPIO pin of the CPU on ECS4810-12M
     * board id0 /board id1
     * MPP42/GPIO22 - PHY 1 interrupt
     * MPP43/GPIO23 - PHY 2 interrupt
     * MPP45/GPIO32 - PHY 3 interrupt
     */
    if(board_id==0)
    {
        /* set polarity as inverted (high to low interrupt)
         */
        rc=mvGppPolaritySet(0, MV_GPP22|MV_GPP23,
            (MV_GPP_IN_INVERT & MV_GPP22) |
            (MV_GPP_IN_INVERT & MV_GPP23));
        if(rc!=MV_OK)
        {
            printk("<0>%s Set GPIO polarity failed\n", __FUNCTION__);
            return FALSE;
        }

        rc=mvGppPolaritySet(1, MV_GPP0,
            (MV_GPP_IN_INVERT & MV_GPP0));
        if(rc!=MV_OK)
        {
            printk("<0>%s Set GPIO HIGH polarity failed\n", __FUNCTION__);
            return FALSE;
        }

        /* set level trigger interrupt
         */
        gppData = MV_REG_READ(GPP_INT_LVL_REG(0));
        gppData |= MV_GPP22|MV_GPP23;
        MV_REG_WRITE(GPP_INT_LVL_REG(0), gppData);
        gppData = MV_REG_READ(GPP_INT_LVL_REG(1));
        gppData |= MV_GPP0;
        MV_REG_WRITE(GPP_INT_LVL_REG(1), gppData);

        /* set interrupt cause register
         */
        gppData = MV_REG_READ(GPP_INT_CAUSE_REG(0));
        gppData |= MV_GPP22|MV_GPP23;
        MV_REG_WRITE(GPP_INT_CAUSE_REG(0), gppData);
        gppData = MV_REG_READ(GPP_INT_CAUSE_REG(1));
        gppData |= MV_GPP0;
        MV_REG_WRITE(GPP_INT_CAUSE_REG(1), gppData);
    }
    else if(board_id==1)
    {
        /* set polarity as inverted (high to low interrupt)
         */
        rc=mvGppPolaritySet(1, MV_GPP10|MV_GPP11|MV_GPP13,
            (MV_GPP_IN_INVERT & MV_GPP10) |
            (MV_GPP_IN_INVERT & MV_GPP11) |
            (MV_GPP_IN_INVERT & MV_GPP13));
        if(rc!=MV_OK)
        {
            printk("<0>%s Set GPIO polarity failed\n", __FUNCTION__);
            return FALSE;
        }

        /* set level trigger interrupt
         */
        gppData = MV_REG_READ(GPP_INT_LVL_REG(1));
        gppData |= MV_GPP10|MV_GPP11|MV_GPP13;
        MV_REG_WRITE(GPP_INT_LVL_REG(1), gppData);

        /* set interrupt cause register
         */
        gppData = MV_REG_READ(GPP_INT_CAUSE_REG(1));
        gppData |= MV_GPP10|MV_GPP11|MV_GPP13;
        MV_REG_WRITE(GPP_INT_CAUSE_REG(1), gppData);
    }

    /* SYS_HWCFG_PTP_IRQ_NUM_ARRAY defines the irq number for board id 1
     * redefine irq_no if board id is not 1.
     */
    if(board_id!=1)
    {
        irq_no[0]=IRQ_GPP_22;
        irq_no[1]=IRQ_GPP_23;
        irq_no[2]=IRQ_GPP_32;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : PTP_ISR_GetIRQAssertedStatus
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *  	This function returns the current irq asserted status.
 *
 * 	INPUT:
 *   None
 *
 * 	OUTPUT:
 *   None
 *
 * 	RETURN:
 *   The irq is asserted if the bit is set as 1, the bit which represent the
 *   irq number can be found in irq_no[]. For example, if bit 0 is 1, it means
 *   irq number irq_no[0] is in asserted state.
 *
 * 	NOTES:
 *   None
 *--------------------------------------------------------------------------
 */
static UI32_T PTP_ISR_GetIRQAssertedStatus(void)
{
    #define GPP_HIGH_BASE_PIN_NO 32

    UI32_T gppData, retData=0;
    int    i;
    int    gpio_pin_no_array_bid0[] =
    {
        22, /* GPIO22 - PHY 1 interrupt */
        23, /* GPIO23 - PHY 2 interrupt */
        32-GPP_HIGH_BASE_PIN_NO, /* GPIO32 - PHY 3 interrupt */
    };
    int    gpio_pin_no_array_bid1[] =
    {
        42-GPP_HIGH_BASE_PIN_NO, /* MPP42 - PHY 1 interrupt */
        43-GPP_HIGH_BASE_PIN_NO, /* MPP43 - PHY 2 interrupt */
        45-GPP_HIGH_BASE_PIN_NO, /* MPP45 - PHY 3 interrupt */
    };

    if(my_board_id==0)
    {
        gppData = MV_REG_READ(GPP_DATA_IN_REG(0));
        for(i=0; i<2; i++)
        {
            if(gppData & 1<<gpio_pin_no_array_bid0[i])
                retData |= 1<<i;
        }

        gppData = MV_REG_READ(GPP_DATA_IN_REG(1));
        if(gppData & 1<<gpio_pin_no_array_bid0[2])
                retData |= 1<<2;

    }
    else if(my_board_id==1)
    {
        
        gppData = MV_REG_READ(GPP_DATA_IN_REG(1));
        for(i=0; i<NUM_OF_IRQ; i++)
        {
            if(gppData & 1<<gpio_pin_no_array_bid1[i])
                retData |= 1<<i;
        }
    }
    else
    {
        retData = 0;
    }

    return retData;
}
#else
#error "No IRQ status function defined for this project."
#endif

/* system call implementation
 */

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : PTP_ISR_Syscall
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *  	This function implements the system call for PTP_ISR.
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
static UI32_T PTP_ISR_Syscall(UI32_T cmd, UI32_T arg1, UI32_T arg2, UI32_T arg3, UI32_T arg4)
{
    int    i, rc;
    SYSFUN_IntMask_T int_mask;
    UI32_T int_assert_bmp, ret=TRUE;

    switch(cmd)
    {
        case PTP_ISR_TYPE_SYSCALL_CMD_ENABLE_INTERRUPT:
            /* arg1: board_id
             * arg2 to arg4 are not used
             */
            if(PTP_ISR_EnableInterrupt((UI8_T)arg1)==FALSE)
                ret=FALSE;
            break;
        case PTP_ISR_TYPE_SYSCALL_CMD_WAIT_EVENT:
            /* arg1: OUTPUT, pending interrupt bitmap
             * arg2-4: not used
             */

            if(interrupt_pending_bmp)
            {
                /* output pending interrupt bitmap
                 */

                int_mask=SYSFUN_InterruptLock();
                SYSFUN_CopyToUser((UI32_T *)arg1, &interrupt_pending_bmp, sizeof(UI32_T));
                interrupt_pending_bmp=0;
                SYSFUN_InterruptUnlock(int_mask);
            }
            else if(int_assert_bmp = PTP_ISR_GetIRQAssertedStatus())
            {
                /* interrupt is asserted but not pending
                 */
                #ifdef PTP_ISR_DEBUG
                printk("<0>PTP interrupt asserted but not pending\n");
                #endif
                SYSFUN_CopyToUser((UI32_T *)arg1, &int_assert_bmp, sizeof(UI32_T));
            }
            else
            {
                /* re-enabled irq that are disabled, not pending and is
                 * in de-asserted state
                 */
                int_mask=SYSFUN_InterruptLock();
                for(i=0; i<NUM_OF_IRQ; i++)
                {
                    if(interrupt_disabled_bmp & 1<<i)
                    {
                        enable_irq(irq_no[i]);
                        interrupt_disabled_bmp ^= 1<<i;
                    }
                }
                SYSFUN_InterruptUnlock(int_mask);

                /* block the thread until the interrupt occurs
                 */
                rc=wait_event_interruptible(wq, interrupt_pending_bmp!=0);
                if(rc!=0)
                {

                    if(rc==EINTR)
                    {
                        #ifdef PTP_ISR_DEBUG
                        printk("%s wait event error due to EINTR\n", __FUNCTION__);
                        #endif
                    }
                    else
                    {
                        printk("%s wait event error rc=%d\n", __FUNCTION__, rc);
                    }
    

                    ret = FALSE;
                }
                else
                {
                    #ifdef PTP_ISR_DEBUG
                    printk("<0>PTP_ISR thread wake up timestamp:%lu\n", SYSFUN_GetSysTick());
                    #endif

                    int_mask=SYSFUN_InterruptLock();
                    SYSFUN_CopyToUser((UI32_T *)arg1, &interrupt_pending_bmp, sizeof(UI32_T));
                    interrupt_pending_bmp=0;
                    SYSFUN_InterruptUnlock(int_mask);
                }

            }
            break;
        default:
            printk("%s:Invalid cmd(0x%08lx)\r\n", __FUNCTION__, cmd);
            ret=FALSE;
            break;

    }

    return ret;
}


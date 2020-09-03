/* MODULE NAME:  k_ipal_if.c
 * PURPOSE:
 *   IPAL_IF implementation for linux kernel
 *
 * NOTES:
 *
 * HISTORY
 *    11/20/2014 - Irene Pan, Created
 *
 * Copyright(C)      Accton Corporation, 2014
 */

/* INCLUDE FILE DECLARATIONS
 */
/* linux kernel header files */
#include "linux/netdevice.h"

#include "sys_adpt.h"

#include "ipal_types.h"
#include "k_ipal_if.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T IPAL_IF_CreateLoopbackDev(UI32_T ifindex);
static UI32_T IPAL_IF_DestroyLoopbackDev(UI32_T lo_id);
static UI32_T IPAL_IF_Syscall(UI32_T cmd, UI32_T arg1, UI32_T arg2, UI32_T arg3, UI32_T arg4);
/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IPAL_IF_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *   Initialize IPAL_IF.
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
void IPAL_IF_Init(void)
{

}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IPAL_IF_Create_InterCSC_Relation
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
void IPAL_IF_Create_InterCSC_Relation(void)
{
    /* TBD: add SYSFUN_SYSCALL_IPAL_IF to SYSFUN_SYSCALL_CMD_ID_E in sysfun.h
     */
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_IPAL_IF, IPAL_IF_Syscall);
}

/* LOCAL SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IPAL_IF_CreateLoopbackDev
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *   Create the loopback net device for the specified lo_id.
 *
 * 	INPUT:
 *   lo_id        --  The loopback id of the loopback net device which is going to be created.
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   IPAL_TYPE_RETVAL_OK                    --  Success
 *   IPAL_TYPE_RETVAL_CREATE_LOOPBACK_DEV_FAIL  --  Fail to create vlan net device
 *
 * 	NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
static UI32_T IPAL_IF_CreateLoopbackDev(UI32_T lo_id)
{
    extern struct net_device* loopback_create_dev(UI32_T);

printk("%s, %d. lo_id:%u.\n", __FUNCTION__, __LINE__, lo_id);
    if(loopback_create_dev(lo_id)==NULL)
    {
printk("%s, %d. X IPAL_TYPE_RETVAL_CREATE_LOOPBACK_DEV_FAIL\n", __FUNCTION__, __LINE__);
        return IPAL_TYPE_RETVAL_CREATE_LOOPBACK_DEV_FAIL;
    }
    return IPAL_TYPE_RETVAL_OK;
}

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IPAL_IF_DestroyLoopbackDev
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *   Destroy the loopback net device for the specified lo_id.
 *
 * 	INPUT:
 *   lo_id        --  The loopback id of the loopback device which is going to be created.
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   IPAL_TYPE_RETVAL_OK                    --  Success
 *   IPAL_TYPE_RETVAL_UNKNOWN_ERROR         --  Unknown error
 *
 * 	NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
static UI32_T IPAL_IF_DestroyLoopbackDev(UI32_T lo_id)
{
    extern BOOL_T loopback_destroy_dev(UI32_T);

printk("%s, %d. lo_id:%u.\n", __FUNCTION__, __LINE__, lo_id);
    if(loopback_destroy_dev(lo_id)==TRUE)
        return IPAL_TYPE_RETVAL_OK;
printk("%s, %d. X IPAL_TYPE_RETVAL_UNKNOWN_ERROR\n", __FUNCTION__, __LINE__);
    return IPAL_TYPE_RETVAL_UNKNOWN_ERROR;
}

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IPAL_IF_Syscall
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *  	This function implements the system call for LOOPBACK.
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
 *   When cmd is invalid, return value is always IPAL_TYPE_RETVAL_UNKNOWN_SYSCALL_CMD
 *--------------------------------------------------------------------------
 */
static UI32_T IPAL_IF_Syscall(UI32_T cmd, UI32_T arg1, UI32_T arg2, UI32_T arg3, UI32_T arg4)
{
    UI32_T ret;

    switch(cmd)
    {
        case IPAL_TYPE_SYSCALL_CMD_CREATE_LOOPBACK_DEV:
            /* arg1(IN): UI32_T lo_id
             */
            ret=IPAL_IF_CreateLoopbackDev(arg1);
            break;
        case IPAL_TYPE_SYSCALL_CMD_DESTROY_LOOPBACK_DEV:
            /* arg1(IN): UI32_T lo_id
             */
             ret=IPAL_IF_DestroyLoopbackDev(arg1);
            break;
        default:
            ret=IPAL_TYPE_RETVAL_UNKNOWN_SYSCALL_CMD;
    }

    return ret;
}


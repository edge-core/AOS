/* MODULE NAME:  k_vlan_mgr.c
 * PURPOSE:
 *   VLAN_MGR implementation for linux kernel
 *
 * NOTES:
 *
 * HISTORY
 *    7/23/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "k_sysfun.h"
#include "l_cvrt.h"

#include "vlan_net.h"
#include "vlan_type.h"
#include "k_vlan_mgr.h"

/* linux kernel header files */
#include "linux/netdevice.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T VLAN_MGR_CreateVlanDev(UI32_T vid, UI8_T mac_addr[SYS_ADPT_MAC_ADDR_LEN]);
static UI32_T VLAN_MGR_DestroyVlanDev(UI32_T vid);
static UI32_T VLAN_MGR_Syscall(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4);

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : VLAN_MGR_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize VLAN_MGR.
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
void VLAN_MGR_Init(void)
{

}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Create_InterCSC_Relation
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
void VLAN_MGR_Create_InterCSC_Relation(void)
{
    /* TBD: add SYSFUN_SYSCALL_VLAN_MGR to SYSFUN_SYSCALL_CMD_ID_E in sysfun.h
     */
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_VLAN_MGR, VLAN_MGR_Syscall);
}

/* LOCAL SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : VLAN_MGR_CreateVlanDev
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Create the vlan net device for the specified vid.
 *
 * 	INPUT:
 *   vid        --  The vid of the vlan net device which is going to be created.
 *   mac_addr   --  The mac addr of the vlan net device which is going to be created.
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   VLAN_TYPE_RETVAL_OK                    --  Success
 *   VLAN_TYPE_RETVAL_INVALID_ARG           --  Invalid arguments
 *   VLAN_TYPE_RETVAL_CREATE_VLAN_DEV_FAIL  --  Fail to create vlan net device
 *
 * 	NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
static UI32_T VLAN_MGR_CreateVlanDev(UI32_T vid, UI8_T mac_addr[SYS_ADPT_MAC_ADDR_LEN])
{

    if(mac_addr==NULL)
        return VLAN_TYPE_RETVAL_INVALID_ARG;

    if(VLAN_NET_CreateVlanDev(vid, mac_addr)==NULL)
        return VLAN_TYPE_RETVAL_CREATE_VLAN_DEV_FAIL;

    return VLAN_TYPE_RETVAL_OK;
}

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : VLAN_MGR_DestroyVlanDev
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Destroy the vlan net device for the specified vid.
 *
 * 	INPUT:
 *   vid        --  The vid of the vlan device which is going to be created.
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   VLAN_TYPE_RETVAL_OK                    --  Success
 *   VLAN_TYPE_RETVAL_UNKNOWN_ERROR         --  Unknown error
 *
 * 	NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
static UI32_T VLAN_MGR_DestroyVlanDev(UI32_T vid)
{
    if(VLAN_NET_DestroyVlanDev(vid)==TRUE)
        return VLAN_TYPE_RETVAL_OK;

    return VLAN_TYPE_RETVAL_UNKNOWN_ERROR;
}


/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : VLAN_MGR_Syscall
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *  	This function implements the system call for VLAN_MGR.
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
 *   When cmd is invalid, return value is always VLAN_TYPE_RETVAL_UNKNOWN_SYSCALL_CMD
 *--------------------------------------------------------------------------
 */
static UI32_T VLAN_MGR_Syscall(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4)
{
    UI32_T ret;
    int cmd = L_CVRT_PTR_TO_UINT(arg0);

    switch(cmd)
    {        
        case VLAN_TYPE_SYSCALL_CMD_CREATE_VLAN_DEV:
            /* arg1(IN): UI32_T vid
             * arg2(IN): UI8_T  mac_addr[SYS_ADPT_MAC_ADDR_LEN]
             */
        {
            UI8_T mac_addr[SYS_ADPT_MAC_ADDR_LEN];

            SYSFUN_CopyFromUser(mac_addr, arg2, SYS_ADPT_MAC_ADDR_LEN);
            ret=VLAN_MGR_CreateVlanDev(L_CVRT_PTR_TO_UINT(arg1), mac_addr);
        }
            break;
        case VLAN_TYPE_SYSCALL_CMD_DESTROY_VLAN_DEV:
            /* arg1(IN): UI32_T vid
             */
            ret=VLAN_MGR_DestroyVlanDev(L_CVRT_PTR_TO_UINT(arg1));
            break;
        default:
            ret=VLAN_TYPE_RETVAL_UNKNOWN_SYSCALL_CMD;
    }

    return ret;
}


/*-----------------------------------------------------------------------------
 * Module   : l2mcast_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 12/03/2001 - Lyn Yeh, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */
#include "sys_cpnt.h"

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "l2mcast_mgr.h"
#include "igv3snp_init.h"
#include "igv3snp_mgr.h"

#if(SYS_CPNT_MLDSNP == TRUE)
#include "mldsnp_init.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_INIT_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to initialize the multicast filtering module.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_INIT_InitiateProcessResources(void)
{
    IGV3SNP_INIT_InitiateProcessResources();

    L2MCAST_MGR_InitiateProcessResources();

#if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_INIT_InitiateProcessResources();
#endif
}   /* End of L2MCAST_INIT_InitiateProcessResources() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_INIT_Create_InterCSC_Relation(void)
{
    IGV3SNP_INIT_Create_InterCSC_Relation();

    L2MCAST_MGR_Create_InterCSC_Relation();

}   /* End of L2MCAST_INIT_Create_InterCSC_Relation() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_INIT_Create_Tasks
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to create the task of the L2MCAST module.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_INIT_Create_Tasks(void)
{
    IGV3SNP_INIT_Create_Tasks();
}   /* End of L2MCAST_INIT_Create_Tasks()*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function enters transition mode and free all IGMPSNP
 *           resources and reset database to factory default.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_INIT_EnterTransitionMode (void)
{
    IGV3SNP_INIT_EnterTransitionMode();

    L2MCAST_MGR_EnterTransitionMode();
    
#if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_INIT_EnterTransitionMode();
#endif

}   /* End of L2MCAST_INIT_EnterTransitionMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function enters master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_INIT_EnterMasterMode(void)
{
    IGV3SNP_INIT_EnterMasterMode();

    L2MCAST_MGR_EnterMasterMode();

#if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_INIT_EnterMasterMode();
#endif

}   /* End of L2MCAST_INIT_EnterMasterMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function enters slave mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_INIT_EnterSlaveMode(void)
{
    IGV3SNP_INIT_EnterSlaveMode();

    L2MCAST_MGR_EnterSlaveMode();
    
#if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_INIT_EnterSlaveMode();
#endif

}   /* End of L2MCAST_INIT_EnterSlaveMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function sets transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_INIT_SetTransitionMode(void)
{
    IGV3SNP_INIT_SetTransitionMode();

    L2MCAST_MGR_SetTransitionMode();

#if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_INIT_SetTransitionMode();
#endif

}   /* End of L2MCAST_INIT_EnterTransitionMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_INIT_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE : The function tell L2MCAST that provision is completed.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MCAST_INIT_ProvisionComplete()
{
    IGV3SNP_INIT_ProvisionComplete();

    L2MCAST_MGR_ProvisionComplete();

}


/* ------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void L2MCAST_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    L2MCAST_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
}


/* -----------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void L2MCAST_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    L2MCAST_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
}


/* MODULE NAME:  k_amtrl3_mgr.h
 * PURPOSE:
 *   linux kernel module to provide APIs for AMTRL3_MGR
 *
 * NOTES:
 *
 * HISTORY
 *    11/17/2009 - Peter Yu, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */
#ifndef K_AMTRL3_MGR_H
#define K_AMTRL3_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
//#include "sys_type.h"

/* liunx kernel header files */

/* NAMING CONSTANT DECLARATIONS
 */
#define K_AMTRL3_MGR_RESULT_OK                      0
#define K_AMTRL3_MGR_RESULT_UNKNOWN_SYSCALL_CMD     1
#define K_AMTRL3_MGR_RESULT_INVALID_ARG             2
#define K_AMTRL3_MGR_RESULT_FAIL                    3

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
enum
{
    K_AMTRL3_MGR_SYSCALL_CMD_HIT,
    K_AMTRL3_MGR_SYSCALL_CMD_GET_NEIGHBOR   
};

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : K_AMTRL3_MGR_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize K_AMTRL3_MGR.
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
void K_AMTRL3_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - K_AMTRL3_MGR_Create_InterCSC_Relation
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
void K_AMTRL3_MGR_Create_InterCSC_Relation(void);


#endif    /* End of K_AMTRL3_MGR_H */


/* Module Name: k_phyaddr_address.h
 * Purpose: 
 *         the header file to access physical address in linux kernel mode
 *         
 *
 * Notes:  
 *        
 * History:                                                               
 *    07/30/2007 - Echo Chen, Created	
 *    02/19/2008 - Anzhen Zheng, Modified
 * 
 * Copyright(C)      Accton Corporation, 2007   				
 */

#ifndef	K_PHYADDR_ACCESS_H
#define	K_PHYADDR_ACCESS_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
 
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: PHYADDR_ACCESS_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for PHYADDR_ACCESS
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  : successful
 *            FALSE : failure
 * NOTES:   
 */
BOOL_T PHYADDR_ACCESS_Init(void);

/* FUNCTION NAME: PHYADDR_ACCESS_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES:
 */
void PHYADDR_ACCESS_Create_InterCSC_Relation(void);

/* FUNCTION NAME: PHYSICAL_ADDR_ACCESS_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for PHYSICAL_ADDR_ACCESS
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  : successful
 *            FALSE : failure
 * NOTES:   
 */
BOOL_T PHYSICAL_ADDR_ACCESS_Init(void);

/* FUNCTION NAME: PHYSICAL_ADDR_ACCESS_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES:
 */
void PHYSICAL_ADDR_ACCESS_Create_InterCSC_Relation(void);


#endif /* K_PHYADDR_ACCESS_H  */

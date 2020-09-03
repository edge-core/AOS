/* Module Name: k_i2c.h
 * Purpose: 
 *         the header file to access i2c in linux kernel mode
 *         
 *
 * Notes:  
 *        
 * History:                                                               
 *    09/05/2007 - Echo Chen, Created	
 *       
 * Copyright(C)      Accton Corporation, 2007   				
 */

#ifndef	K_I2C_H
#define	K_I2C_H

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
/* FUNCTION NAME: I2C_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for I2C
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  : successful
 *            FALSE : failure
 * NOTES:   
 */
BOOL_T I2C_Init(void);

/* FUNCTION NAME: I2C_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES:
 */
void I2C_Create_InterCSC_Relation(void);


#endif /* K_I2C_H  */


/* Module Name: UCMGMT_INIT.C
 * Purpose: Initialize the resources for the ucmgmt module.
 *
 * Notes:
 *
 * History:
 *    11/28/01       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "stdio.h"
#include "sys_type.h"
#include "uc_mgr.h"



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


/* FUNCTION NAME: UCMGMT_INIT_InitiateProcessResources
 * PURPOSE: This function is used to initialize the un-cleared memory management module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Initialize the ucmgmt module.
 *
 */
void UCMGMT_INIT_InitiateProcessResources(void)
{
    if (!UC_MGR_InitiateProcessResources())
    {
        perror ("\r\nUC_MGR_InitiateProcessResources Error\r\n");
        while (TRUE);
    }

    return;
} /* End of UCMGMT_INIT_InitiateProcessResources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - UCMGMT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void UCMGMT_INIT_Create_InterCSC_Relation(void)
{
    return;
} /* end of UCMGMT_INIT_Create_InterCSC_Relation */

#ifdef AARON
void UC_MEM_Init (void)
{
   if ( memcmp ( header->signature, signature, sizeof(signature) ) == 0 )
      return;

   /* First time be initiated from power on */
   /* 1. clear memory */
   memset ( header, 0, UC_MEM_SIZE );

   /* 2. Initiate header */
   memmove ( header->signature, signature, sizeof(signature) );
   header->free_ptr = header->data;
} /* UC_MEM_Initiate */
#endif

/* MODULE NAME:  app_protocol_group.h
 * PURPOSE:
 *     Files for app protocol group.
 *
 * NOTES:
 *
 * HISTORY
 *    11/13/2007 - Squid Ro, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef APP_PROTOCOL_GROUP_H
#define APP_PROTOCOL_GROUP_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - APP_PROTOCOL_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for APP_PROTOCOL_GROUP.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void APP_PROTOCOL_GROUP_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in APP_PROTOCOL_GROUP.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *
 *------------------------------------------------------------------------------
 */
void APP_PROTOCOL_GROUP_Create_All_Threads(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will init APP_PROTOCOL_GROUP.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *------------------------------------------------------------------------------
 */
void APP_PROTOCOL_GROUP_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will call all CSC in APP_PROTOCOL_GROUP inform that provision complete
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *
 *------------------------------------------------------------------------------
 */
void APP_PROTOCOL_GROUP_ProvisionComplete(void);

#endif    /* End of APP_PROTOCOL_GROUP_H */


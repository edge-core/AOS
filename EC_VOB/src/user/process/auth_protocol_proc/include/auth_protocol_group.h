/* MODULE NAME:  auth_protocol_group.h
 * PURPOSE:
 *     This file is a demonstration code for implementations of auth_protocol group.
 *
 * NOTES:
 *
 * HISTORY
 *    6/12/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef AUTH_PROTOCOL_GROUP_H
#define AUTH_PROTOCOL_GROUP_H

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
/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in AUTH_PROTOCOL_Group.
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
 *    All threads in the same AUTH_PROTOCOL group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void AUTH_PROTOCOL_GROUP_Create_All_Threads(void);

/*---------------------------------------------------------------------------
---
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_InitiateProcessResources
 *---------------------------------------------------------------------------
---
 * PURPOSE:
 *    This function will init  CSC Group.
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
 *---------------------------------------------------------------------------
---
 */
void AUTH_PROTOCOL_GROUP_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AUTH_PROTOCOL_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for SWCTRL Group.
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
void AUTH_PROTOCOL_GROUP_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_GROUP_SendAuthCheckResponseMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Sends response message of authentication check request.
 *
 * INPUT:
 *    result    -- Authentication result
 *    privilege -- Privilege of the user
 *    cookie    -- Cookie (message type of the response)
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void AUTH_PROTOCOL_GROUP_SendAuthCheckResponseMsg(I32_T result, I32_T privilege,
    UI32_T cookie);

#endif    /* End of AUTH_PROTOCOL_GROUP_H */

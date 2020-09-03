/* MODULE NAME: ovsvtep_group.h
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*    {2. Related documents or hardware information}
* NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - Give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitation}
*    {4. Any performance limitation}
*    {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/4/2017    squid Create
* Copyright(C)      Edge-Core Corporation, 2017
*/

#ifndef _OVSVTEP_GROUP_H
#define _OVSVTEP_GROUP_H


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate process resource for OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
void OVSVTEP_GROUP_InitiateProcessResources(void);


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE: Create inter-CSC relationships for OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
void OVSVTEP_GROUP_Create_InterCSC_Relation(void);


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will spawn all threads in OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : All threads in the same CSC group will join the same thread group.
 *-----------------------------------------------------------------------------
 */
void OVSVTEP_GROUP_Create_All_Threads(void);


#endif /*End of _OVSVTEP_GROUP_H*/

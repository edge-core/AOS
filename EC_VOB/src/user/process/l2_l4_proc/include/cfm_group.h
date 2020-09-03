/* MODULE NAME: cfm_group.H
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
*    1/21/2008    Macauley_Cheng Create
* Copyright(C)      Accton Corporation, 2008
*/

#ifndef _CFM_GROUP_H
#define _CFM_GROUP_H


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_GROUP_InitiateProcessResource
 *-------------------------------------------------------------------------
 * PURPOSE : initial the resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFM_GROUP_InitiateProcessResource(void);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_MainMenu
 *-------------------------------------------------------------------------
 * PURPOSE : create the inter csc relationship
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFM_GROUP_Create_InterCSC_Relation(void);

/*-------------------------------------------------------------------------
* FUNCTION NAME - L2MCAST_GROUP_Create_All_Threads
*-------------------------------------------------------------------------
* PURPOSE : This function will spawn all threads in CFM
* INPUT   : None
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
void CFM_GROUP_Create_All_Threads(void);

#endif /*End of CFM_GROUP_H*/

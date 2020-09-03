/* MODULE NAME - DCB_GROUP.H
 * PURPOSE : Provides the declarations for DCB thread group functionalities.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef DCB_GROUP_H
#define DCB_GROUP_H

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

/* FUNCTION NAME - DCB_GROUP_InitiateProcessResources
 * PURPOSE : Initiate process resources for DCB group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void DCB_GROUP_InitiateProcessResources(void);

/* FUNCTION NAME - DCB_GROUP_Create_InterCSC_Relation
 * PURPOSE : Create inter-CSC relations for DCB group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void DCB_GROUP_Create_InterCSC_Relation(void);

/* FUNCTION NAME - DCB_GROUP_Create_All_Threads
 * PURPOSE : Spawn all threads in DCB group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void DCB_GROUP_Create_All_Threads(void);

#endif /* End of DCB_GROUP_H */


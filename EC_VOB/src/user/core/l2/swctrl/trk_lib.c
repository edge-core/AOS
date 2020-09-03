/*-----------------------------------------------------------------------------
 * FILE NAME: TRK_LIB.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Implements pure-code functions of TRUNK.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/08/08     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sys_adpt.h"
#include "trk_lib.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */


/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_OM_TrunkIdToIfindex
 *------------------------------------------------------------------------
 * FUNCTION: Convert trunk ID to ifindex
 * INPUT   : trunk_id  -- trunk id
 * OUTPUT  : ifindex   -- trunk ifindex
 * RETURN  : TRUE/FALSE
 * NOTE    : Ignore trunk existing state.
 *------------------------------------------------------------------------
 */
BOOL_T TRK_OM_TrunkIdToIfindex(UI32_T trunk_id, UI32_T *ifindex)
{
    if (trunk_id == 0                                   ||
        trunk_id > SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM )
    {
        return FALSE;
    }

    if (NULL == ifindex)
    {
        return FALSE;
    }

    *ifindex = trunk_id + SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER - 1;

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - TRK_OM_IfindexToTrunkId
 *------------------------------------------------------------------------
 * FUNCTION: Convert trunk ID to ifindex
 * INPUT   : ifindex   -- trunk ifindex
 * OUTPUT  : trunk_id  -- trunk id
 * RETURN  : TRUE/FALSE
 * NOTE    : Ignore trunk existing state.
 *------------------------------------------------------------------------
 */
BOOL_T TRK_OM_IfindexToTrunkId(UI32_T ifindex, UI32_T *trunk_id)
{
    if (ifindex < SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER                                   ||
        ifindex > SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER + SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM - 1)
    {
        return FALSE;
    }

    if (NULL == trunk_id)
    {
        return FALSE;
    }

    *trunk_id = ifindex - SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER + 1;

    return TRUE;
}


/* LOCAL SUBPROGRAM BODIES
 */

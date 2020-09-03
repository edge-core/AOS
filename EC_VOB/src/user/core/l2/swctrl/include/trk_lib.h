/*-----------------------------------------------------------------------------
 * FILE NAME: TRK_LIB.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Declares pure-code functions of TRUNK.
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

#ifndef TRK_LIB_H
#define TRK_LIB_H


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
BOOL_T TRK_OM_TrunkIdToIfindex(UI32_T trunk_id, UI32_T *ifindex);

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
BOOL_T TRK_OM_IfindexToTrunkId(UI32_T ifindex, UI32_T *trunk_id);


#endif /* #ifndef TRK_LIB_H */

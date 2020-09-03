/*-----------------------------------------------------------------------------
 * FILE NAME: VLAN_LIB.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Implements pure-code functions of VLAN.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/26     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "vlan_lib.h"


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

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ConvertToIfindex
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function will convert a vid to a matching dot1q_vlan_index existing
 *            in the IF_table.
 * INPUT    : vid       -- specify the vid input by management
 * OUTPUT   : *vlan_ifindex -- returns the matching dot1q_vlan_index in the iftable
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T  VLAN_OM_ConvertToIfindex(UI32_T vid, UI32_T *vlan_ifindex)
{
    UI32_T      temp_vlan_ifindex;

    /* BODY */

    /* Allen Cheng, Dec/17/2002 */
    /* Ignore checking the operating mode */
    if (    (vid == 0)
        ||  (vid > SYS_ADPT_MAX_VLAN_ID)
       )
    {
        *vlan_ifindex   = SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1;
        return  FALSE;
    }

    temp_vlan_ifindex = vid +  SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1 ;

    memcpy(vlan_ifindex, &temp_vlan_ifindex, sizeof(UI32_T));
    return  TRUE;
} /* End of VLAN_OM_ConvertToIfindex() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ConvertFromIfindex
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function will convert a dot1q_vlan_index from the database to vid
 *            value for user inteface.
 * INPUT    : dot1q_vlan_index  -- specify the dot1q_vlan_index in the IF table
 * OUTPUT   : *vid       -- specify the vid for management
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T  VLAN_OM_ConvertFromIfindex(UI32_T dot1q_vlan_index, UI32_T *vid)
{
    UI32_T  convert_vid;

    /* BODY */

    /* Allen Cheng, Dec/17/2002 */
    /* Ignore checking the operating mode */
    if (dot1q_vlan_index < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
    {
        *vid    = 0;
        return  FALSE;
    }

    convert_vid = dot1q_vlan_index -  SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1;

    memcpy(vid, &convert_vid,sizeof(UI32_T));
    return  TRUE;
} /* End of VLAN_OM_ConvertFromIfindex() */


/* LOCAL SUBPROGRAM BODIES
 */

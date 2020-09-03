/*-----------------------------------------------------------------------------
 * FILE NAME: VLAN_LIB.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Declares pure-code functions of VLAN.
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

#ifndef VLAN_LIB_H
#define VLAN_LIB_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ConvertToIfindex
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function will convert a vid to a matching vid_ifindex existing
 *            in the IF_table.
 * INPUT    : vid       -- specify the vid input by management
 * OUTPUT   : *vid_ifindex -- returns the matching vid_ifindex in the iftable
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T  VLAN_OM_ConvertToIfindex(UI32_T vid, UI32_T *vid_ifindex);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ConvertFromIfindex
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function will convert a vid_ifindex from the database to vid
 *            value for user inteface.
 * INPUT    : vid_ifindex  -- specify the vid_ifindex in the IF table
 * OUTPUT   : *vid       -- specify the vid for management
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T  VLAN_OM_ConvertFromIfindex(UI32_T vid_ifindex, UI32_T *vid);

/*define marco for convert vlan_ifindex to vid(specify the VLAN index)--DanXie*/

#define VLAN_VID_CONVERTTO_IFINDEX(vid,vlan_ifindex)\
    (vlan_ifindex = ((vid > SYS_ADPT_MAX_VLAN_ID)?(SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1):(vid +  SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1)) )
    
/*define marco for convert vid to vlan_ifindex(specify the VLAN index)--DanXie*/
#define VLAN_IFINDEX_CONVERTTO_VID(vlan_ifindex,vid)\
    (vid = ((vlan_ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)? 0 : (vlan_ifindex -  SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1)))

#define IS_VLAN_IFINDEX_VAILD(vlan_ifindex)\
    ((vlan_ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER||vlan_ifindex > SYS_ADPT_MAX_VLAN_ID+SYS_ADPT_VLAN_1_IF_INDEX_NUMBER-1)? FALSE : TRUE)

#define IS_VLAN_ID_VAILD(vid)\
    ((vid > SYS_ADPT_MAX_VLAN_ID||vid == 0)?FALSE:TRUE)

#endif /* #ifndef VLAN_LIB_H */


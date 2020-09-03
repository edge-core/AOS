/* MODULE NAME: mldsnp_unknwon.H
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
*    12/03/2007    Macauley_Cheng Create
* Copyright(C)      Accton Corporation, 2007
*/


#ifndef _MLDSNP_UNKNOWN_H
#define _MLDSNP_UNKNOWN_H

/* INCLUDE FILE DECLARATIONS
*/
#include "mldsnp_type.h"
#include "mldsnp_engine.h"
/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_UNKNOWN_HandleJoinReport
*------------------------------------------------------------------------------
* Purpose: This function process a port join a group but there is a unknown entry already.
* INPUT  : vid      - the vlan id
*         lport     - the logical port
*         *gip_ap   - the group ip array pointer
*         *sip_ap   - the source ip array pointer
*         list_type - the list type for this (vid, gip, sip)
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : the unknow will be registered at the (vid, gip, sip)
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_UNKNOWN_HandleJoinReport(
    UI16_T                 vid,
    UI16_T                 lport,
    UI8_T                  *gip_ap, UI8_T *sip_ap,
    MLDSNP_TYPE_ListType_T list_type);

/*------------------------------------------------------------------------------
* Function : MLDSNP_UNKNOWN_ProcessUnknownMcastData
*------------------------------------------------------------------------------
* Purpose: This function process the unknown multicast data
* INPUT  : unknown_data_p  - the pointer the unknown data packet
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_UNKNOWN_ProcessUnknownMcastData(
    MLDNSP_ENGINE_Msg_T *unknown_data_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_UNKNOWN_SetFloodBehavior
 *-------------------------------------------------------------------------
 * PURPOSE : This function isto set the unknown flood behavior
 * INPUT   : flood_behavior   - the flood behavior
 *           vlan_id          - which vlan
 * OUTPUT  : None
 * RETURN  :
 * NOTE    :  1. search all (vid, gip, sip) and its router port, if exsit unknown registed, delete the port
 *               if router port dosn't exist, we register port 0 to the group
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_UNKNOWN_SetFloodBehavior(
    UI32_T vlan_id,
    UI32_T new_flood_behavior);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_UNKNOWN_AddRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function process the new router port is added
 * INPUT   : flood_behavior   - the flood behavior
 * OUTPUT  : None
 * RETURN  :
 * NOTE    : check if there is unknown data need flood to this port
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_UNKNOWN_AddRouterPort(
    UI16_T vid,
    UI16_T lport);

/*------------------------------------------------------------------------------
* Function : MLDSNP_UNKNOWN_PortRxUnknwonDataTimeout
*------------------------------------------------------------------------------
* Purpose: This function process the receiver port has a long timer over timer without receiving the multicast data
* INPUT   : *timer_para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_UNKNOWN_PortRxUnknwonDataTimeout(
    void * timer_para_p);

/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_UNKNOWN_DeleteAllJoinedPort
*-------------------------------------------------------------------------
* PURPOSE : process port leave this group entry
* INPUT   : vid        - the input vid
*           *gip_ap   - the group ip address
*           *sip_ap    - the sip ip address
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
void MLDSNP_UNKNOWN_DeleteAllUnknownExtraJoinedPort(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap);


/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_UNKNOWN_GetUnknownForwardPortbitmp
*-------------------------------------------------------------------------
* PURPOSE : To get the portbitmap this unknown forward ports
* INPUT   : vid            - the input vid
*               lport      - the port register this group
* OUTPUT  : *port_bitmap_a - the port bitmap
* RETURN  : None
* NOTE    : if flooding, return vlan's all member port bitmap, if to-router port, return all router port bitmap
*-------------------------------------------------------------------------
*/
void MLDSNP_UNKNOWN_GetUnknownForwardPortbitmp(
    UI16_T vid,
    UI16_T lport,
    UI8_T *port_bimap_ap);
#endif/* End of mldsnp_unknown_H */




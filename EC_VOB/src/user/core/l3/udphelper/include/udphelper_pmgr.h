/* MODULE NAME:  udphelper_pmgr.hs
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access UDPHELPER_MGR and UDPHELPER_OM service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call UDPHELPER_PMGR_XXX for APIs UDPHELPER_MGR_XXX provided by UDPHELPER, and same as UDPHELPER_POM for
 *    UDPHELPER_OM APIs
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    03/31/2009 - LinLi, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */
#ifndef _UDPHELPER_PMGR_H_
#define _UDPHELPER_PMGR_H_

#include "udphelper_type.h"
#include "l_inet.h"

/* EXPORTED SUBPROGRAM BODIES
 */
    /* FUNCTION NAME : UDPHELPER_PMGR_VlanCreate
    * PURPOSE:
    *     Create l3 interface
    *
    * INPUT:
    *      vid: the port number
    *
    * OUTPUT:
    *      None
    * RETURN:
    *       
    *
    * NOTES:
    *      None.
    */
    UI32_T UDPHELPER_PMGR_L3IfCreate(UI32_T ifindex);
 /* FUNCTION NAME : UDPHELPER_PMGR_VlanDelete
* PURPOSE:
*     Delete l3 interface
*
* INPUT:
*      vid: the port number
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_L3IfDelete(UI32_T ifindex);
 /* FUNCTION NAME : UDPHELPER_PMGR_RifCreate
* PURPOSE:
*     Add primary address for l3 interface
*
* INPUT:
*     
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_RifCreate(UI32_T ifindex, L_INET_AddrIp_T addr);
 /* FUNCTION NAME : UDPHELPER_PMGR_RifDelete
* PURPOSE:
*     Delete primary address for l3 interface
*
* INPUT:
*     
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_RifDelete(UI32_T ifindex, L_INET_AddrIp_T addr);
 
/* FUNCTION NAME : UDPHELPER_PMGR_AddForwardUdpPort
* PURPOSE:
*     Add forward port
*
* INPUT:
*      port: the port number
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_AddForwardUdpPort(UI32_T port);
/* FUNCTION NAME : UDPHELPER_PMGR_DelForwardUdpPort
* PURPOSE:
*     Delete forward port
*
* INPUT:
*      port: the port number
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_DelForwardUdpPort(UI32_T port);
/* FUNCTION NAME : UDPHELPER_PMGR_AddHelperAddress
* PURPOSE:
*     Add helper address
*
* INPUT:
*      vid: the vlan id number
*      addr: the helper address
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_AddHelperAddress(UI32_T ifindex, L_INET_AddrIp_T addr);
/* FUNCTION NAME : UDPHELPER_PMGR_DelHelperAddress
* PURPOSE:
*     Delete helper address
*
* INPUT:
*      vid: the vlan id number
*      addr: the helper address
*
* OUTPUT:
*      None
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_DelHelperAddress(UI32_T ifindex, L_INET_AddrIp_T addr);
/* FUNCTION NAME : UDPHELPER_PMGR_SetStatus
* PURPOSE:
*     Set the udp helper status
*
* INPUT:
*
* OUTPUT:
*      status: the status
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_SetStatus(UI32_T status);

/* FUNCTION NAME : UDPHELPER_PMGR_GetStatus
* PURPOSE:
*     Get the udp helper status
*
* INPUT:
*
* OUTPUT:
*      status: the status
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_GetStatus(UI32_T *status);
/* FUNCTION NAME : UDPHELPER_PMGR_GetNextForwardPort
* PURPOSE:
*     Get next forward port
*
* INPUT:
*
* OUTPUT:
*     
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_GetNextForwardPort(UI32_T *port);
/* FUNCTION NAME : UDPHELPER_PMGR_GetNextHelper
* PURPOSE:
*     Get next helper address
*
* INPUT:
*
* OUTPUT:
*     
* RETURN:
*       
*
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_GetNextHelper(UI32_T ifindex, L_INET_AddrIp_T *helper);
/* FUNCTION NAME : UDPHELPER_PMGR_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource used in the calling process.
 *
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    Before other CSC use UDPHELPER_PMGR, it should initiate the resource (get the message queue handler internally)
 *
 */
BOOL_T UDPHELPER_PMGR_InitiateProcessResource(void);
/* FUNCTION NAME : UDPHELPER_PMGR_GetHelper
* PURPOSE:
*     Check if this helper address exist.
*
* INPUT:
*
* OUTPUT:
*     
* RETURN:
*       UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL  -- error.
*       UDPHELPER_TYPE_RESULT_FAIL  -- doesn't exist.
*       UDPHELPER_TYPE_RESULT_SUCCESS  -- exist.
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_GetHelper(UI32_T ifindex, L_INET_AddrIp_T *helper);
/* FUNCTION NAME : UDPHELPER_PMGR_GetForwardPort
* PURPOSE:
*     Get next forward port
*
* INPUT:
*
* OUTPUT:
*     
* RETURN:
*       UDPHELPER_TYPE_RESULT_SEND_MSG_FAIL  -- error.
*       UDPHELPER_TYPE_RESULT_FAIL  -- doesn't exist.
*       UDPHELPER_TYPE_RESULT_SUCCESS  -- exist.
* NOTES:
*      None.
*/
UI32_T UDPHELPER_PMGR_GetForwardPort(UI32_T port);

#endif


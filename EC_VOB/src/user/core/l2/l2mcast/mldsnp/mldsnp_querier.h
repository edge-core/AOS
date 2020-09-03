/*-----------------------------------------------------------------------------
 * Module Name: mldsnp_querier.c
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/3/2007 - Macauley_Cheng, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef _MLDSNP_QUERIER_H
#define _MLDSNP_QUERIER_H

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



/* EXPORTED SUBPROGRAM BODIES
*/
/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_QUERIER_AddStaticRouterPort
*-------------------------------------------------------------------------
* PURPOSE : This function is to add the static router port
* INPUT   : vid        - the vlan id
*           lport      - the logical port
* OUTPUT  : None
* RETURN  : TRUE  - success
*           FALSE - failure
* NOTE    : None
*-------------------------------------------------------------------------
*/
BOOL_T MLDSNP_QUERIER_AddStaticRouterPort(
    UI16_T vid,
    UI16_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_QUERIER_DeleteStaticRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to delete the static router port
 * INPUT   : vid        - the vlan id
 *           lport      - the logical port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_QUERIER_DeleteStaticRouterPort(
    UI16_T vid,
    UI16_T lport);

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SendGeneralQeury
*------------------------------------------------------------------------------
* Purpose: This function send the general query to all querying running status enabled vlan
* INPUT  : specify_vid - spcify send to this vlan
*          skip_router_port - skip router port
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_SendGeneralQeury(
    UI16_T specify_vid,
    BOOL_T skip_router_port);
#if 0
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SendGeneralQeuryToSpcificVlan
*------------------------------------------------------------------------------
* Purpose: This function send the general query to specific vlan id
* INPUT  : vid - the vlan id
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_SendGeneralQeuryToSpcificVlan(
    UI16_T vid);
#endif
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SendSpecificQuery
*------------------------------------------------------------------------------
* Purpose: This function send the group spcific qeury
* INPUT  : vid            - the vlan id
*          gip_ap         - the group ip address
*          lport          - the port to send the g-s query
*          *src_ip_list_ap- the source ip array pointer
*          num_of_src     - the number of src ip
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_SendSpecificQuery(
    UI16_T   vid,
    UI8_T    *gip_ap,
    UI16_T   lport,
    UI8_T    *src_ip_list_ap,
    UI16_T   num_of_src);

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_EnableQuerier
*------------------------------------------------------------------------------
* Purpose: This function enable the querier
* INPUT  :
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_EnableQuerier();

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_DisableQuerier
*------------------------------------------------------------------------------
* Purpose: This function disable the querier
* INPUT  :
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_DisableQuerier();
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_StartQuerier
*------------------------------------------------------------------------------
* Purpose: This function start the querier
* INPUT  :
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_StartQuerier();
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_StopQuerier
*------------------------------------------------------------------------------
* Purpose: This function stop the querier
* INPUT  :
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_StopQuerier();
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_EnableMrdSolicitation
*------------------------------------------------------------------------------
* Purpose: This function enable the querier
* INPUT  :
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_SetMrdSolicitationStatus(BOOL_T enabled);

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessQuery
*------------------------------------------------------------------------------
* Purpose: This function change the query inteval
* INPUT  : new_interval  - new query interval
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUEIRER_ChangeQueryInterval(
    UI32_T new_interval);

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessQuery
*------------------------------------------------------------------------------
* Purpose: This function process the query
* INPUT  : *msg_p   - the message pointer
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_ProcessQuery(
    MLDNSP_ENGINE_Msg_T *msg_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_IsQuerierRunning
*------------------------------------------------------------------------------
* Purpose: Is querier running?
* INPUT  :vid - the vlan id to check querier running status
* OUTPUT : TRUE - running
*          FALSE- not running
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_IsQuerierRunning(
    UI16_T vid);
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_GenerateCheckSum
*------------------------------------------------------------------------------
* Purpose: This function genereate the checksum
* INPUT  : *icmp_p    - the icmp start address
*          *sip_ap    - the source ip first four octect
*          *dip_ap    - the dest ip first fourt octect
*          icmp_length- the icmp pdu length
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
UI16_T  MLDSNP_QUERIER_GenerateCheckSum(
    UI8_T  *icmp_p,
    UI8_T  *sip_ap,
    UI8_T  *dip_ap,
    UI8_T  next_header,
    UI32_T icmp_length);
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_GeneralQueryTimeOut
*------------------------------------------------------------------------------
* Purpose: This function process the quereir time out to send the query to all port in all vlan
* INPUT  : *timer_para_p  - the parameter pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_GeneralQueryTimeOut(
    void *parameter_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SpecificQueryTimeOut
*------------------------------------------------------------------------------
* Purpose: This function process the specific quereir time out to send the specific group query to
*          the want to leave port
* INPUT  : *timer_para_p  - the parameter pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_SpecificQueryTimeOut(
    void *parameter_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_OtherQuerierPrentTimeout
*------------------------------------------------------------------------------
* Purpose: This function process the other querier timer timeout
* INPUT  : *parameter_p  - the parameter pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_OtherQuerierPrentTimeout(
    void *parameter_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_QuerierStartTimeout
*------------------------------------------------------------------------------
* Purpose: This function process the querier startup timeout
* INPUT  : *parameter_p  - the parameter pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_QuerierStartTimeout(
    void *parameter_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_RouterPortTimeout
*------------------------------------------------------------------------------
* Purpose: This function process the dynamic learned router port expired
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_RouterPortTimeout(
    void * para_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_RemoveDynamicRouterPortOnSpecifyPort
*------------------------------------------------------------------------------
* Purpose: This function delete the dynamic learned router port from all vlan
* INPUT  :  vlan - the vlan id
*           lport - the router port
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :if vid =0 means all vlan, else specify the vlan.
*         This function is provided for caller not in mldsnp_querier.c
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_RemoveDynamicRouterPortOnSpecifyPort(
    UI16_T specify_vid,
    UI16_T lport);

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessTopologyChange
*------------------------------------------------------------------------------
* Purpose: This function process topology change
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_ProcessTopologyChange(UI32_T xstp_id, UI16_T lport);

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_MrdSolicitationTimeout
*------------------------------------------------------------------------------
* Purpose: This function send the mrd solicitation to all port
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_MrdSolicitationTimeout(
    void *para_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessMrdAdvertisement
*------------------------------------------------------------------------------
* Purpose: This function process the multicast router advertisement message
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_ProcessMrdAdvertisement(
    MLDNSP_ENGINE_Msg_T *msg_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessMrdSolicitation
*------------------------------------------------------------------------------
* Purpose: This function process the multicast router solicitation message
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_ProcessMrdSolicitation(
    MLDNSP_ENGINE_Msg_T *msg_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessMrdTermination
*------------------------------------------------------------------------------
* Purpose: This function process the multicast router termination message
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_ProcessMrdTermination(
    MLDNSP_ENGINE_Msg_T *msg_p);


/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SetMRouteStatus
*------------------------------------------------------------------------------
* Purpose: This function set multicast routing status
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_SetMRouteStatus(BOOL_T is_enabled);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_QUERIER_AddDynamicRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to add the dynamic router port
 * INPUT   : vid   - the vlan id
 *           lport - the logical port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_QUERIER_AddDynamicRouterPort(UI16_T vid, UI16_T lport);

#endif/* End of mldsnp_querier_H */



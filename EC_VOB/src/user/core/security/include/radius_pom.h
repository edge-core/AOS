/*---------------------------------------------------------------------------
 * Module   : radius_pom.h
 *---------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access RADIUS.
 *---------------------------------------------------------------------------
 * NOTES    :
 *
 *---------------------------------------------------------------------------
 * HISTORY  : 08/15/2007 - Wakka Tu, Create
 *
 *---------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *---------------------------------------------------------------------------
 */

#ifndef	RADIUS_POM_H
#define	RADIUS_POM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "radius_type.h"

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_POM_InitiateProcessResources
 *---------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_InitiateProcessResources(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningRequestTimeout
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS request timeout is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: timeout
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_GetRunningRequestTimeout(UI32_T *timeout);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   timeout value.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_Get_Request_Timeout(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningServerPort
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server port is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverport
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_GetRunningServerPort(UI32_T *serverport);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the UDP port number of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Prot number
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_Get_Server_Port(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningServerSecret
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server secret is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serversecret[MAX_SECRET_LENGTH + 1]
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_POM_GetRunningServerSecret(UI8_T *serversecret);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   secret text string pointer
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI8_T * RADIUS_POM_Get_Server_Secret(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningRetransmitTimes
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS retransmit times is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: retimes
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_GetRunningRetransmitTimes(UI32_T *retimes);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   retransmit times
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_Get_Retransmit_Times(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningServerIP
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_GetRunningServerIP(UI32_T *serverip);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   RADIUS server IP address
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_POM_Get_Server_IP(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_UnknowAddress_Packets
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the number of RADIUS Access-Response packets
 *           received from unknown addresses.
 * INPUT:    None
 * OUTPUT:   None.
 * RETURN:   Number of RADIUS Access-Response packets received from unknown addresses
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_POM_Get_UnknowAddress_Packets(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_NAS_ID
 *---------------------------------------------------------------------------
 * PURPOSE:  Get he NAS-Identifier of the RADIUS authentication client.
 *           This is not necessarily the same as sysName in MIB II.
 * INPUT:    None
 * OUTPUT:   None.
 * RETURN:   NASID
 *           NASID = NULL  ---No NAS ID
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_Get_NAS_ID(UI8_T *nas_id);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_GetAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client MIB to get radiusAuthServerTable
 * INPUT:    adiusAuthServerIndex
 * OUTPUT:   AuthServerEntry pointer
 * RETURN:   Get table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_GetAuthServerTable(UI32_T index,AuthServerEntry *ServerEntry);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_GetNextAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This funtion returns true if the next available table entry info
 *           can be successfully retrieved. Otherwise, false is returned.
 * INPUT:    RadiusAuthServerIndex ,
 * OUTPUT:   NextAuthServerEntry pointer
 * RETURN:   Get next table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_GetNextAuthServerTable(UI32_T *index,AuthServerEntry *ServerEntry);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetNext_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE: Get next RADIUS server ip.
 * INPUT:  index
 * OUTPUT: index
 *         server_host
 * RETURN: TRUE/FALSE
 * NOTES:
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_GetNext_Server_Host(UI32_T *index,RADIUS_Server_Host_T *server_host);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetNextRunning_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_GetNextRunning_Server_Host(UI32_T *index,RADIUS_Server_Host_T *server_host);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_Get_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE: Get RADIUS server ip.
 * INPUT:  index
 * OUTPUT: server_host
 * RETURN: TRUE/FALSE
 * NOTES:
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_Get_Server_Host(UI32_T index,RADIUS_Server_Host_T *server_host);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server port is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverport
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_GetRunningServerAcctPort(UI32_T *acct_port);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_POM_GetServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : get global server accounting port
 * INPUT    : none
 * OUTPUT   : acct_port
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_GetServerAcctPort(UI32_T *acct_port);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_POM_GetAccClientInvalidServerAddresses
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of RADIUS Accounting-Response packets received from unknown addresses.
 * INPUT    : none.
 * OUTPUT   : invalid_server_address_counter
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_GetAccClientInvalidServerAddresses(UI32_T *invalid_server_address_counter);

#endif /* RADIUS_POM_H */



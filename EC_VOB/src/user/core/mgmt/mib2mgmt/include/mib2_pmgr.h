#ifndef MIB2_PMGR_H
#define MIB2_PMGR_H


/*------------------------------------------------------------------------------
 * ROUTINE NAME : TACACS_PMGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for TACACS_PMGR.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void MIB2_PMGR_Init(void);

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_PMGR_GetSysObjectID
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system authoritative identification
 *           of this device is successfully retrieved. Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_oid    - system decsription of this device.
 *           length     - length of system object id.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. This value is allocated within the SMI enterprises subtree (1.3.6.1.4.1).
 *           2. This field is read-only.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_PMGR_GetSysObjectID(UI32_T *sys_oid, UI32_T *length);

/* ---------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_PMGR_SetSysContact
 * ---------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system contact info of this device
 *           is successfully configured. Otherwise, false is returned.
 * INPUT   : sys_contact        - system contact info of this device
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Contact is a Octet string.
 *              The max length of System Contact infomation could be up to 255 bytes.
 *           2. The Octet string is NOT a Zero-ASCII string as defined by C.
 *              The Octet string is an ASCII string represented as a byte-array format
 *              with string length.
 *           3. The max length of System Contact supported by this device will be
 *              OEM customer dependent and shall be defined in the OEM_ADPT.H.
 * ---------------------------------------------------------------------------------------*/
BOOL_T  MIB2_PMGR_SetSysContact(UI8_T *sys_contact);
/*
  EPR_ID:ES3628BT-FLF-ZZ-00057
  Problem: CLI: The behavior of hostname command is NOT correct.
  Root Cause: use error command line.
  Solution: 1. use "hostname" command is modification of "system name".
            2. Add "prompt" CLI command.
*/
#if 0 /* shumin.wang delete command "snmp-server sysname" */
/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_PMGR_SetSysName
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system name is successfully configured.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_name - An administratively assigned name.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 *           2. This is the node's qualify domain name.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_PMGR_SetSysName(UI8_T *sys_name);
#endif 
/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_PMGR_SetHostName
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the host name is successfully configured.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : host_name - An administratively assigned name.
 * RETURN  : TRUE/FALSE
 * NOTE    : Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_PMGR_SetHostName(UI8_T *host_name);

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_PMGR_SetSysLocation
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system location can be successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_location   - Physical location of the node.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_PMGR_SetSysLocation(UI8_T *sys_location);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MIB2_PMGR_GetRunningSysName
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default system name is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: system_name      - system name in byte (ASCII char) array
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *        4. Based on RFC1213, system name is defined as Octet string
 *           (ASCII char array) with length up to 255 bytes.
 *           Octet string is not an ASCII Zero string as defined in C.
 * ---------------------------------------------------------------------*/
UI32_T  MIB2_PMGR_GetRunningSysName(UI8_T *sys_name);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MIB2_PMGR_GetRunningSysContact
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default system contact info is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: system_contact   - system contact info in byte (ASCII char) array
 *         length           - length of system contact info
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system contact info.
 *        3. Caller has to prepare buffer for storing system contact info
 *        4. Based on RFC1213, system contact info is defined as Octet string
 *           (ASCII char array) with length up to 255 bytes.
 *           Octet string is not an ASCII Zero string as defined in C.
 * ---------------------------------------------------------------------*/
UI32_T  MIB2_PMGR_GetRunningSysContact(UI8_T *sys_contact);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - MIB2_PMGR_GetRunningSysLocation
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default system location info is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: system_contact   - system location info in byte (ASCII char) array
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system location info.
 *        3. Caller has to prepare buffer for storing system location info
 *        4. Based on RFC1213, system location info is defined as Octet string
 *           (ASCII char array) with length up to 255 bytes.
 *           Octet string is not an ASCII Zero string as defined in C.
 * ---------------------------------------------------------------------*/
UI32_T  MIB2_PMGR_GetRunningSysLocation(UI8_T *sys_location);
#endif /*end of MIB2_PMGR_H*/


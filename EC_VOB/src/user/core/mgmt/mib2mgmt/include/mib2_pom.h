#ifndef MIB2_POM_H
#define MIB2_POM_H


/*------------------------------------------------------------------------------
 * ROUTINE NAME : MIB2_POM_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for MIB2_POM_Init.
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
void MIB2_POM_Init(void);

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_POM_GetSysDescr
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system description of this device
 *           is successfully retrieved. Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_descrption     - system decsription of this device.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 *           2. The Octet string is NOT a Zero-ASCII string as defined by C.
 *              The Octet string is an ASCII string represented as a byte-array format
 *              with string length.
 *           3. The max length of System Decsription supported by this device will be
 *              OEM customer dependent and shall be defined in the OEM_ADPT.H.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_POM_GetSysDescr(UI8_T *sys_description);

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_POM_GetSysContact
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the contact information is successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_contact    - identification and contact info of the contact person.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_POM_GetSysContact(UI8_T *sys_contact);

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_POM_GetSysName
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system name is successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_name       - An administratively assigned name.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 *              The max length of System Decsription could be up to 255 bytes.
 *           2. This is the node's qualify domain name.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_POM_GetSysName(UI8_T *sys_name);

/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_POM_GetSysLocation
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the system location can be successfully retrieved.
 *           Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_location   - Physical location of the node.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. Based on SNMP defintion, the syntax of System Decsription is a Octet string.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_POM_GetSysLocation(UI8_T *sys_location);


/* -----------------------------------------------------------------------------------------
 * ROUTINE NAME - MIB2_POM_GetSysServices
 * -----------------------------------------------------------------------------------------
 * FUNCTION: This function returns true if the type of service provided by the system can be
 *           successfully retrieved.  Otherwise, false is returned.
 * INPUT   : None
 * OUTPUT  : sys_services   - indicates the set of services current system provides.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. sys_services is the sum of set of services provided by the system.
 *           2. This value is interpreted as a 7-bit code such that each bit of the code
 *              correspondes to a layer in the TCP/IP or OSI architecture whith the LSB
 *              corresponds to layer 1.
 *           3. The max value is 127.
 * -----------------------------------------------------------------------------------------*/
BOOL_T  MIB2_POM_GetSysServices(UI8_T *sys_services);

#endif /*end of MIB2_POM_H*/


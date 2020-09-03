#ifndef _SNMP_CLUSTER_H
#define _SNMP_CLUSTER_H

typedef struct SNMP_CLUSTER_RelayManagemnetBackSession {
    
    I32_T to_member_reqid;/*key*/
    
    UI32_T to_member_time;


    /*
     * Protocol-version independent fields
     */
    /** snmp version */
    I32_T            version;

    /** Request id - note: not incremented on retries */
    I32_T            reqid;  
    /** Message id for V3 messages note: incremented for each retry */
    I32_T            msgid;
    

    I32_T             securityModel;
    /** noAuthNoPriv, authNoPriv, authPriv */
    I32_T             securityLevel;  
    I32_T             msgParseModel;

    /**
     * Transport-specific opaque data.  This replaces the IP-centric address
     * field.  
     */
    
    void           *transport_data;
    I32_T             transport_data_length;

    /*
     * SNMPv1 & SNMPv2c fields
     */
    /** community for outgoing requests. */
    UI8_T         *community;
    /** length of community name. */
    UI32_T          community_len;  

    /*
     *  SNMPv3 fields
     */
    /** context snmpEngineID */
    UI8_T         *contextEngineID;
    /** Length of contextEngineID */
    UI32_T          contextEngineIDLen;     
    /** authoritative contextName */
    UI8_T           *contextName;
    /** Length of contextName */
    UI32_T          contextNameLen;
    /** authoritative snmpEngineID for security */
    UI8_T         *securityEngineID;
    /** Length of securityEngineID */
    UI32_T          securityEngineIDLen;    
    /** on behalf of this principal */
    UI8_T           *securityName;
    /** Length of securityName. */
    UI32_T          securityNameLen;        
    
    struct SNMP_CLUSTER_RelayManagemnetBackSession  *next;
    
} SNMP_CLUSTER_RelayManagemnetBackSession_T;

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_GetClusterRole
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the cluster role.
 * INPUT:    
 * OUTPUT:   
 * RETURN:  TRUE/FALSE
 * NOTE:     
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_CLUSTER_GetClusterRole(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_CheckFromCommanderIp
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will check packets which is from the commander by ip.
 * INPUT:    pdu
 * OUTPUT:   
 * RETURN:  TRUE/FALSE
 * NOTE:     
 *---------------------------------------------------------------------------
 */

BOOL_T SNMP_CLUSTER_CheckFromCommanderIp(netsnmp_pdu *pdu);
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_NameParseToMemberId
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will separate original_name for geting member id and name by  the "@swXXX" token.
 * INPUT:    original_name
 * OUTPUT:   out_name,member_id
 * RETURN:  TRUE/FALSE
 * NOTE:     
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_NameParseToMemberId(UI8_T *original_name,UI8_T *out_name,UI32_T *member_id);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_ChangeSecurityNameForRelayTrap
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is the begining point to do relaying pdu to the member or the management.
 * INPUT:    session,pdu
 * OUTPUT:   
 * RETURN:  TRUE/FALSE
 * NOTE:     1.if return TRUE, pdu which have be handled by the relaing procedure.
 *               2.if return FALSE, pdu should be processed by normal way  in the feature.
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_RelayProcess(netsnmp_session * session, netsnmp_pdu *pdu);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_ChangeToMember
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will  initial resources for being the commander.
 * INPUT:    
 * OUTPUT:   
 * RETURN:  TRUE/FALSE
 * NOTE:    
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_ChangeToCommander(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_ChangeToMember
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will  release resources for changing the cluster role from the commander.
 * INPUT:    
 * OUTPUT:   
 * RETURN:  TRUE/FALSE
 * NOTE:    
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_ChangeToOtherFromCommander(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_ChangeToMember
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will  initial resources for being member.
 * INPUT:    
 * OUTPUT:   
 * RETURN:  TRUE/FALSE
 * NOTE:    
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_ChangeToMember(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_ChangeToMember
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will  release resources for changing the cluster role from the member.
 * INPUT:    
 * OUTPUT:   
 * RETURN:  TRUE/FALSE
 * NOTE:    
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_ChangeToOtherFromMember(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_SendTrapToCommander
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is procedure to send v1/v2 trap to the commander.
 * INPUT:    minor,template_pdu
 * OUTPUT:   
 * RETURN:  TRUE/FALSE
 * NOTE:     
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_SendTrapToCommander(I32_T minor,netsnmp_pdu *template_pdu);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_ChangeCommunityForRelayTrap
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will change community for relaying trap.
 * INPUT:    session,pdu
 * OUTPUT:   
 * RETURN:  TRUE/FALSE
 * NOTE:     
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_ChangeCommunityForRelayTrap(netsnmp_session * session, netsnmp_pdu *pdu);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_ChangeSecurityNameForRelayTrap
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will change SecurityName for relaying trap.
 * INPUT:    session,pdu
 * OUTPUT:   
 * RETURN:  TRUE/FALSE
 * NOTE:     
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_ChangeSecurityNameForRelayTrap(netsnmp_session * session, netsnmp_pdu *pdu);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_CheckRoleChange
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is checked and hadled that role has to change.
 * INPUT:    role
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
void SNMP_CLUSTER_CheckRoleChange (void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_InitRole
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is initiate role.
 * INPUT:    
 * OUTPUT:   
 * RETURN:  
 * NOTE:    
 *---------------------------------------------------------------------------
 */
void SNMP_CLUSTER_InitRole(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_SetClusterRole
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is set snmp next cluster role.
 * INPUT:    role
 * OUTPUT:   
 * RETURN:  
 * NOTE:    
 *---------------------------------------------------------------------------
 */
void SNMP_CLUSTER_SetClusterRole (UI32_T role);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_GetCommanderIp
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get tthe commander ip .
 * INPUT:
 * OUTPUT:   commander_ip
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_GetCommanderIp(UI8_T *commander_ip);
#endif


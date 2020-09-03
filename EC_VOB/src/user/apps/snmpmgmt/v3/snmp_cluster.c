#include <net-snmp/net-snmp-config.h>

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/utilities.h>
#include <net-snmp/library/tools.h>     /* for "internal" definitions */

#include <net-snmp/types.h>
#include <net-snmp/library/snmp_api.h>
#include "net-snmp/agent/agent_callbacks.h"
#include "net-snmp/library/callback.h"
#include "sys_type.h"
#include "snmp_cluster.h"
#include "sys_adpt.h"
#include "leaf_es3626a.h"
#include "sys_time.h"
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/snmp_assert.h>

#include "snmpd.h"
#include "mibgroup/struct.h"
#include "mibgroup/util_funcs.h"
#include <net-snmp/agent/mib_module_config.h>
#include <net-snmp/agent/mib_modules.h>
#include <net-snmp/agent/agent_trap.h>
#include <net-snmp/agent/agent_callbacks.h>
#include <net-snmp/library/vacm.h>
#include "snmp_mgr.h"
#if (SYS_CPNT_CLUSTER==TRUE)
    #include "cluster_pom.h"

#define SNMP_CLUSTER_MAX_PEERNAME 31 /*for ip v4,"udp:%u.%u.%u.%u:%u",3+1+3+1+3+1+3+1+3+1+10+1,*/
#define SNMP_CLUSTER_TO_MEMBER_TRAP_COMMUNITY "private"
#define SNMP_CLUSTER_TO_MEMBER_COMMUNITY "private"
#define SNMP_CLUSTER_TO_MEMBER_PORT 161  /*161*/
#define SNMP_CLUSTER_TO_COMMANDER_TRAP_PORT 161 /*161*/
#define SNMP_CLUSTER_MANAGEMENT_SESSION_TIMEOUT 300 /* 1/100 second*/
#define SNMP_CLUSTER_MAX_NUMBER_BACKUP_MANAGEMENT_SESSION 32

static SNMP_CLUSTER_RelayManagemnetBackSession_T *managemnet_session_entrys[SYS_ADPT_CLUSTER_MAX_NBR_OF_MEMBERS];
static UI32_T member_request_id[SYS_ADPT_CLUSTER_MAX_NBR_OF_MEMBERS];
static UI32_T snmp_current_cluster_role;
static UI32_T snmp_next_cluster_role;

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_ControlRelayManagemnetBackSessionCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will control session_counter for limiting the number of RelayManagemnetBackSession
 * INPUT:    number.
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:     number =1 to session_counter is increased by 1,
 *               number =-1 to session_counter is decreased by 1
 *               number=0 to set session_counter as 0;
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_CLUSTER_ControlRelayManagemnetBackSessionCounter(I32_T  number)
{
    static UI32_T session_counter;

    /* Clear counter
     */
    if(number==0)
    {
        session_counter =0;
    }
    /* Increase counter by 1, and check it is not over the max number.
     */
    else if(number==1)
    {
        /* check counter is not over the max number.
         */
        if(session_counter>=SNMP_CLUSTER_MAX_NUMBER_BACKUP_MANAGEMENT_SESSION)
        {
            session_counter = SNMP_CLUSTER_MAX_NUMBER_BACKUP_MANAGEMENT_SESSION;
            return FALSE;
        }
        session_counter =session_counter+1;
    }
    /* Decrease counter by 1, and check it is not over the max number.
     */
    else if(number==-1)
    {

        /* check counter is not less 0.
         */
        if(session_counter<=0)
        {
            session_counter = 0;
            return FALSE;
        }
        session_counter =session_counter-1;
     }
    else
    {
        return FALSE;
    }
    return TRUE;
}

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
UI32_T SNMP_CLUSTER_GetClusterRole(void)
{
    return snmp_current_cluster_role;
}

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
BOOL_T SNMP_CLUSTER_GetCommanderIp(UI8_T *commander_ip)
{
    CLUSTER_TYPE_EntityInfo_T cluster;

    /* If do not get cammander,then commander is seted 0.0.0.0
     */
    if(CLUSTER_POM_GetClusterInfo(&cluster)!=TRUE)
    {
        memset(commander_ip, 0, SYS_ADPT_IPV4_ADDR_LEN);
        return FALSE;
    }
    memcpy(commander_ip, cluster.commander_ip, SYS_ADPT_IPV4_ADDR_LEN);
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_NameParseToMemberId
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will separate original_name for geting member id and name by the "@swXXX" token.
 * INPUT:    original_name
 * OUTPUT:   out_name,member_id
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_NameParseToMemberId(UI8_T *original_name,UI8_T *out_name,UI32_T *member_id)
{
    UI32_T len,i,fields;
	UI8_T temp[SYS_ADPT_MAX_COMM_STR_NAME_LEN];
    len=strlen((char *)original_name);
	if(len>SYS_ADPT_MAX_COMM_STR_NAME_LEN)
        return FALSE;
    fields=0;
    for(i=0;i<len;i++)
    {

        /* Separate original_name for geting member id and name by the "@swXXX" token.
         */
        if(original_name[i]=='@')
        {
            fields=sscanf((char *)&original_name[i+1],"sw%lu%s",member_id,temp);
            memcpy(out_name,original_name,i);
            out_name[i]='\0';
            break;
        }
    }

    /* Strings is not contained "@sw%d" then retrun fasle.
     */
    if(fields!=1)
    {
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T SNMP_CLUSTER_CheckFromCommanderIp( netsnmp_pdu *pdu)
{
    UI8_T commander_ip[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI32_T com_ip;
    struct sockaddr_in *addr ;

    /* Get commander ip for compareing.
     */
    if(SNMP_CLUSTER_GetCommanderIp(commander_ip)!=TRUE)
    {
         return FALSE;
    }
    memcpy(&com_ip, commander_ip, sizeof(UI32_T));

    addr =  (struct sockaddr_in *) pdu->transport_data;
    if(addr->sin_family != AF_INET)
    {
        return FALSE;
    }

    /* Useing commander ip to compare the ip of PDU, if they are not the same ip return false.
     */
    if(com_ip != addr->sin_addr.s_addr)
    {
        return FALSE;
    }

    return TRUE;
}


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_GetToMemberIdFromManagementPdu
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get  member_id by the community or the securityName.
 * INPUT:    pdu
 * OUTPUT:   member_id,new_name
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_GetToMemberIdFromManagementPdu(netsnmp_pdu *pdu,UI32_T *member_id,UI8_T  *new_name)
{
    UI8_T name[SYS_ADPT_MAX_COMM_STR_NAME_LEN+1];

    /* Get community or name by version.
     */
    switch(pdu->version)
    {
        case SNMP_VERSION_1:
        case SNMP_VERSION_2c:
            memcpy(name,pdu->community,pdu->community_len);
            name[pdu->community_len]='\0';
            if(SNMP_CLUSTER_NameParseToMemberId(name,new_name, member_id)==TRUE)
            {
                return TRUE;
            }
            break;
        case SNMP_VERSION_3:
            memcpy(name,pdu->securityName,pdu->securityNameLen);
            name[pdu->securityNameLen]='\0';
            if(SNMP_CLUSTER_NameParseToMemberId(name,new_name,member_id)==TRUE)
            {
                return TRUE;
            }
            break;
        default:
            return FALSE;
            break;
    }
    return FALSE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_CheckFromMemberPdu
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will check pdu if it is from member by ip.
 * INPUT:    pdu
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_CheckFromMemberPdu( netsnmp_pdu *pdu)
{
    UI8_T  temp_ip[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI32_T member_ip;
    UI8_T member_id;
    struct sockaddr_in *addr ;
    addr =  (struct sockaddr_in *) pdu->transport_data;
    if(addr->sin_family != AF_INET)
    {
        return FALSE;
    }

    member_ip = L_STDLIB_Ntoh32(addr->sin_addr.s_addr);
    memcpy(temp_ip, &member_ip, sizeof(UI32_T));

    /* Get member id by the ip of the pdu.
     */
    if(CLUSTER_POM_MemberIpToId(&member_id, temp_ip)!=TRUE)
    {
        return FALSE;
    }
    return TRUE;

}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_GetNewMemberSessionRequestId
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the new member_request_id for such member.
 * INPUT:    pdu
 * OUTPUT:   member_id
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_CLUSTER_GetNewMemberSessionRequestId(UI32_T member_id)
{
    /* Increade by 1, when call this get new request id.
     */
    member_request_id[member_id-1]++;
    return member_request_id[member_id-1];
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_DeleteAgeOutManagementSession
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will delete the age out ManagementSessions.
 * INPUT:
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_DeleteAgeOutManagementSession(void)
{
    UI32_T tick,i,condition_tick;
    SNMP_CLUSTER_RelayManagemnetBackSession_T *temp_session,*free_session;
    SYS_TIME_GetSystemUpTimeByTick(&tick);
    condition_tick=tick-SNMP_CLUSTER_MANAGEMENT_SESSION_TIMEOUT;

    for(i=0;i<SYS_ADPT_CLUSTER_MAX_NBR_OF_MEMBERS;i++)
    {
        temp_session = managemnet_session_entrys[i];

        while(temp_session!=NULL)
        {

            /* Find the receent seesion which is not less than the currect time is decreased
             * by SNMP_CLSTE_MANAGEMENT_SESSION_TIMEOUT.
             */
            if(temp_session->to_member_time>=condition_tick)
            {
                break;
            }

            /* The ageout session, so free it.
             */
            free_session = temp_session;
            temp_session=temp_session->next;
            SNMP_FREE( free_session->transport_data);
            SNMP_FREE( free_session->community);
            SNMP_FREE( free_session->contextEngineID);
            SNMP_FREE( free_session->contextName);
            SNMP_FREE( free_session->securityEngineID);
            SNMP_FREE( free_session->securityName);
            free(free_session);
            SNMP_CLUSTER_ControlRelayManagemnetBackSessionCounter(-1);
        }
        managemnet_session_entrys[i]=temp_session;
    }
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_StoreManagementSessionEntry
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will store ManagementSession for relaying to home in the feature(Management).
 * INPUT:    pdu,member_id,new_request_id
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_StoreManagementSessionEntry(netsnmp_pdu *pdu,UI32_T member_id, UI32_T *new_request_id)
{
    SNMP_CLUSTER_RelayManagemnetBackSession_T *management_session_entry=NULL,*temp_management_session_entry=NULL;
    UI8_T  member_ip[SYS_ADPT_IPV4_ADDR_LEN] = {0};

    /* Useing ip to get the member id.
     */
    if(CLUSTER_POM_MemberIdToIp(member_id,member_ip)!=TRUE)
    {
        return FALSE;
    }

    *new_request_id=SNMP_CLUSTER_GetNewMemberSessionRequestId(member_id);

    /* Check and add counter.if true we could store new session.
     */
    if(SNMP_CLUSTER_ControlRelayManagemnetBackSessionCounter(1)==TRUE)
    {
        management_session_entry=malloc(sizeof(SNMP_CLUSTER_RelayManagemnetBackSession_T));

        /* Check if no memory resource
         */
        if(management_session_entry==NULL)
        {
            SNMP_CLUSTER_ControlRelayManagemnetBackSessionCounter(-1);
            return FALSE;
        }
    }
    else
    {
        /* Over the number of the backup session, so to delete age out session.
         */
        SNMP_CLUSTER_DeleteAgeOutManagementSession();

        /* Check and add counter.if true we could store new session.
         */
        if(SNMP_CLUSTER_ControlRelayManagemnetBackSessionCounter(1)==TRUE)
        {
           /* Check if no memory resource
            */
            management_session_entry=malloc(sizeof(SNMP_CLUSTER_RelayManagemnetBackSession_T));
            if(management_session_entry==NULL)
            {
                SNMP_CLUSTER_ControlRelayManagemnetBackSessionCounter(-1);
                return FALSE;
            }
        }
        else
        {
            /* Over the number of the backup session, and it do not delete age out session.
             * So,we return false.
             */
            return FALSE;
        }

    }

    /* Copy PDU session to backup session
     */
    memset(management_session_entry,0,sizeof(management_session_entry));
    management_session_entry->to_member_reqid= *new_request_id;
    SYS_TIME_GetSystemUpTimeByTick(&management_session_entry->to_member_time);
    management_session_entry->version=pdu->version;
    management_session_entry->reqid=pdu->reqid;
    management_session_entry->msgid=pdu->msgid;
    management_session_entry->securityModel=pdu->securityModel;
    management_session_entry->securityLevel=pdu->securityLevel;
    management_session_entry->msgParseModel=pdu->msgParseModel;
    management_session_entry->transport_data_length=pdu->transport_data_length;
    memdup((u_char **)&management_session_entry->transport_data,pdu->transport_data,
        management_session_entry->transport_data_length);
    management_session_entry->community_len=pdu->community_len;
    memdup((u_char **)&management_session_entry->community,pdu->community,
        management_session_entry->community_len);
    management_session_entry->contextEngineIDLen=pdu->contextEngineIDLen;
    memdup((u_char **)&management_session_entry->contextEngineID,pdu->contextEngineID,
        management_session_entry->contextEngineIDLen);
    management_session_entry->contextNameLen=pdu->contextNameLen;
    memdup((u_char **)&management_session_entry->contextName,(u_char *)(pdu->contextName),
        management_session_entry->contextNameLen);
    management_session_entry->securityEngineIDLen=pdu->securityEngineIDLen;
    memdup((u_char **)&management_session_entry->securityEngineID,pdu->contextEngineID,
        management_session_entry->securityEngineIDLen);
    management_session_entry->securityNameLen=pdu->securityNameLen;
    memdup((u_char **)&management_session_entry->securityName,(u_char *)(pdu->securityName),
        management_session_entry->securityNameLen);
    management_session_entry->next=NULL;

    /* Append new backup session to the managemnet_session_entrys
     */
    if(managemnet_session_entrys[member_id-1]==NULL)
    {
        managemnet_session_entrys[member_id-1]=management_session_entry;
    }
    else
    {
        temp_management_session_entry=managemnet_session_entrys[member_id-1];
        while(temp_management_session_entry->next!=NULL)
        {
            temp_management_session_entry=temp_management_session_entry->next;
        }
        temp_management_session_entry->next=management_session_entry;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_ReplacePduForSendToMember
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will replace the pdu for relaying to the member.
 * INPUT:    pdu,member_id,new_request_id
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_ReplacePduForSendToMember(netsnmp_pdu *pdu,UI32_T member_id, UI32_T new_request_id)
{

    UI8_T temp_ip[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI8_T member_community[SYS_ADPT_MAX_COMM_STR_NAME_LEN+1]={0};
    UI32_T member_community_len=0;
    UI32_T member_ip;
    struct sockaddr_in *to;

    member_community_len = strlen(SNMP_CLUSTER_TO_MEMBER_COMMUNITY);
    memcpy(member_community,SNMP_CLUSTER_TO_MEMBER_COMMUNITY,member_community_len);
    member_community[member_community_len]='\0';
    pdu->reqid=new_request_id;
    pdu->version=SNMP_MP_MODEL_SNMPv2c;
    pdu->securityModel=SNMP_SEC_MODEL_SNMPv2c;
    pdu->securityLevel=SNMP_SEC_LEVEL_NOAUTH;
    if(CLUSTER_POM_MemberIdToIp(member_id ,temp_ip )!=TRUE)
        return FALSE;
    to=pdu->transport_data;
    memcpy(&member_ip, temp_ip, sizeof(UI32_T));
    to->sin_addr.s_addr = L_STDLIB_Hton32(member_ip);
    to->sin_port = L_STDLIB_Hton16(SNMP_CLUSTER_TO_MEMBER_PORT);

    SNMP_FREE(pdu->community);
    pdu->community_len=member_community_len;
    memdup((u_char **)&pdu->community,member_community,pdu->community_len);

    SNMP_FREE(pdu->contextEngineID);
    pdu->contextEngineIDLen=0;
    SNMP_FREE(pdu->contextName);
    pdu->contextNameLen=0;
    SNMP_FREE(pdu->securityEngineID);
    pdu->securityEngineIDLen=0;
    SNMP_FREE(pdu->securityName);
    pdu->securityNameLen=0;
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_RelayToMember
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is the procedure for  relaying pdu to member.
 * INPUT:    pdu,member_id
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_RelayToMember(netsnmp_pdu *pdu, UI32_T member_id)
{
    UI32_T new_request_id;

    /* Store management session
     */
    if(SNMP_CLUSTER_StoreManagementSessionEntry(pdu,member_id,&new_request_id)!=TRUE)
    {
        return FALSE;
    }

    /* Replace Pdu for sending to member.
     */
    if(SNMP_CLUSTER_ReplacePduForSendToMember(pdu,member_id,new_request_id)!=TRUE)
    {
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_RelayToManagement
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will produce the new pdu for relaying mamagement.
 * INPUT:    pdu
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_RelayToManagement(netsnmp_pdu *pdu)
{
    UI8_T temp_ip[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI32_T member_ip;
    UI8_T member_id;
    struct sockaddr_in *addr ;
    SNMP_CLUSTER_RelayManagemnetBackSession_T *temp_management_session_entry,*before_temp_management_session_entry;

    addr =  (struct sockaddr_in *) pdu->transport_data;

    member_ip = L_STDLIB_Ntoh32(addr->sin_addr.s_addr);
    memcpy(temp_ip, &member_ip, sizeof(UI32_T));
    /* Get member id from ip.
     */
    if(CLUSTER_POM_MemberIpToId(&member_id, temp_ip)!=TRUE)
        return FALSE;

    /* Find to back managemnet way, by the request id.
     */
    if(managemnet_session_entrys[member_id-1]==NULL)
    {
        /* It is not back up the way to management in this member.
         */
        return FALSE;
    }
    else
    {
        before_temp_management_session_entry=NULL;
        temp_management_session_entry=managemnet_session_entrys[member_id-1];
        while(temp_management_session_entry!=NULL)
        {

            /* Find the same request id of the back up sessions.
             */
            if(temp_management_session_entry->to_member_reqid==pdu->reqid)
            {
                break;
            }
            before_temp_management_session_entry=temp_management_session_entry;
            temp_management_session_entry=temp_management_session_entry->next;
        }

        /* We do not find the same request id of the back up sessions.
         * So, return false.
         */
        if(temp_management_session_entry==NULL)
        {
            return FALSE;
        }
    }

    /* Replace pdu for sending back to managaemnet.
     */
    pdu->version=temp_management_session_entry->version;
    pdu->reqid=temp_management_session_entry->reqid;
    pdu->msgid=temp_management_session_entry->msgid;
    pdu->securityModel=temp_management_session_entry->securityModel;
    pdu->securityLevel=temp_management_session_entry->securityLevel;
    pdu->msgParseModel=temp_management_session_entry->msgParseModel;
    SNMP_FREE(pdu->transport_data);
    pdu->transport_data_length=temp_management_session_entry->transport_data_length;
    memdup((u_char **)&pdu->transport_data,temp_management_session_entry->transport_data,
        pdu->transport_data_length);
    SNMP_FREE(pdu->community);
    pdu->community_len=temp_management_session_entry->community_len;
    memdup((u_char **)&pdu->community,temp_management_session_entry->community,
        pdu->community_len);
    SNMP_FREE(pdu->contextEngineID);
    pdu->contextEngineIDLen=temp_management_session_entry->contextEngineIDLen;
    memdup((u_char **)&pdu->contextEngineID,temp_management_session_entry->contextEngineID,
        pdu->contextEngineIDLen);
    SNMP_FREE(pdu->contextName);
    pdu->contextNameLen=temp_management_session_entry->contextNameLen;
    memdup((u_char **)&pdu->contextName,temp_management_session_entry->contextName,
        pdu->contextNameLen);
    SNMP_FREE(pdu->securityEngineID);
    pdu->securityEngineIDLen=temp_management_session_entry->securityEngineIDLen;
    memdup((u_char **)&pdu->securityEngineID,temp_management_session_entry->contextEngineID,
        pdu->securityEngineIDLen);
    SNMP_FREE(pdu->securityName);
    pdu->securityNameLen=temp_management_session_entry->securityNameLen;
    memdup((u_char **)&pdu->securityName,temp_management_session_entry->securityName,
        pdu->securityNameLen);

    /* Modify link list for Release the backup management session.
     */
    if(before_temp_management_session_entry==NULL)
    {
        managemnet_session_entrys[member_id-1]=temp_management_session_entry->next;
    }
    else
    {
        before_temp_management_session_entry->next=temp_management_session_entry->next;
    }

    /* Release the backup management session.
     */
    SNMP_FREE( temp_management_session_entry->transport_data);
    SNMP_FREE( temp_management_session_entry->community);
    SNMP_FREE( temp_management_session_entry->contextEngineID);
    SNMP_FREE( temp_management_session_entry->contextName);
    SNMP_FREE( temp_management_session_entry->securityEngineID);
    SNMP_FREE( temp_management_session_entry->securityName);
    SNMP_FREE(temp_management_session_entry);
    SNMP_CLUSTER_ControlRelayManagemnetBackSessionCounter(-1);
    return TRUE;
}


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_CheckWriteView
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will check community or security name if it has the write aceess.
 * INPUT:    pdu
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_CLUSTER_CheckWriteView(netsnmp_pdu *pdu)
{
    netsnmp_variable_list *varbind_ptr, *vbsave;

    /* Only check write view for seting
     */
    if(pdu->command!=SNMP_MSG_SET)
        return TRUE;

    for (varbind_ptr = pdu->variables; varbind_ptr;
         varbind_ptr = vbsave)
    {
        vbsave = varbind_ptr->next_variable;

        /*
         * check access control
         */
        if (in_a_view(varbind_ptr->name, &varbind_ptr->name_length,
                             pdu, varbind_ptr->type) != VACM_SUCCESS)
        {
            return FALSE;
        }

    }
    return TRUE;
}

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
BOOL_T SNMP_CLUSTER_SendTrapToCommander(I32_T minor,netsnmp_pdu *template_pdu)
{
    UI8_T peername[SNMP_CLUSTER_MAX_PEERNAME+1];
    UI8_T commander_ip[4];
    netsnmp_session trap_v1_session;
    netsnmp_session trap_v2_session;
    I32_T rel;
    UI8_T private[] = SNMP_CLUSTER_TO_MEMBER_TRAP_COMMUNITY;

    SNMP_CLUSTER_GetCommanderIp(commander_ip);
    /* set commander ip for sending trap.
     */
    sprintf((char *)peername,"udp:%u.%u.%u.%u:%u",(UI8_T)commander_ip[0],(UI8_T)commander_ip[1]
                 ,(UI8_T)commander_ip[2],(UI8_T)commander_ip[3],SNMP_CLUSTER_TO_COMMANDER_TRAP_PORT);
    peername[SNMP_CLUSTER_MAX_PEERNAME]='\0';

    /* Handle v1 trap and v2 trap.
     */
    if(SNMPD_CALLBACK_SEND_TRAP1==minor)
    {
        memset(&trap_v1_session,0,sizeof(netsnmp_session ));
        trap_v1_session.version =SNMP_VERSION_1;
        memdup((u_char **)&trap_v1_session.peername,peername,SNMP_CLUSTER_MAX_PEERNAME) ;
        trap_v1_session.community_len = strlen(SNMP_CLUSTER_TO_MEMBER_TRAP_COMMUNITY);
        memdup((u_char **)&trap_v1_session.community,private,
        trap_v1_session.community_len);
        rel=(I32_T)SNMP_API_SendTrapToCommander(template_pdu,&trap_v1_session);
        SNMP_FREE(trap_v1_session.peername);
        SNMP_FREE(trap_v1_session.community);
        SNMP_FREE(trap_v1_session.contextEngineID);
        SNMP_FREE(trap_v1_session.contextName);
        SNMP_FREE(trap_v1_session.securityEngineID);
        SNMP_FREE(trap_v1_session.securityName);
        SNMP_FREE(trap_v1_session.securityAuthProto);
        SNMP_FREE(trap_v1_session.securityPrivProto);
    }
    else
    {
        memset(&trap_v2_session,0,sizeof(netsnmp_session ));
        trap_v2_session.version =SNMP_VERSION_2c;
        memdup((u_char **)&trap_v2_session.peername,peername,SNMP_CLUSTER_MAX_PEERNAME) ;
        trap_v2_session.community_len = strlen(SNMP_CLUSTER_TO_MEMBER_TRAP_COMMUNITY);
        memdup((u_char **)&trap_v2_session.community,private,
        trap_v2_session.community_len);
        rel=(I32_T)SNMP_API_SendTrapToCommander(template_pdu,&trap_v2_session);
        SNMP_FREE(trap_v2_session.peername);
        SNMP_FREE(trap_v2_session.community);
        SNMP_FREE(trap_v2_session.contextEngineID);
        SNMP_FREE(trap_v2_session.contextName);
        SNMP_FREE(trap_v2_session.securityEngineID);
        SNMP_FREE(trap_v2_session.securityName);
        SNMP_FREE(trap_v2_session.securityAuthProto);
        SNMP_FREE(trap_v2_session.securityPrivProto);
    }
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_SendTrapToCommander
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is the procedure to send v1/v2 trap to the management.
 * INPUT:    pdu
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
 BOOL_T SNMP_CLUSTER_RelayTrapToManagement(netsnmp_pdu *pdu)
{
    /* Handle v1 trap and v2 trap.
     */
    if(pdu->command==SNMP_MSG_TRAP)
    {
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION, SNMPD_CALLBACK_SEND_TRAP1, pdu);
        return TRUE;
    }
    else if(pdu->command==SNMP_MSG_TRAP2)
    {
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,SNMPD_CALLBACK_SEND_TRAP2, pdu);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

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
BOOL_T SNMP_CLUSTER_ChangeCommunityForRelayTrap(netsnmp_session * session, netsnmp_pdu *pdu)
{
    struct sockaddr_in * addr;
    UI8_T member_id;
    UI8_T community[SYS_ADPT_MAX_COMM_STR_NAME_LEN+14];
    UI8_T temp_ip[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI32_T member_ip;

    /* Check null.
     */
    if(session==NULL||pdu==NULL||pdu->transport_data==NULL)
        return FALSE;

    /* Only handle trap.
     */
    if(pdu->command!=SNMP_MSG_TRAP &&pdu->command!=SNMP_MSG_INFORM&&pdu->command!=SNMP_MSG_TRAP2)
        return FALSE;
    addr =  (struct sockaddr_in *) pdu->transport_data;

    member_ip = L_STDLIB_Ntoh32(addr->sin_addr.s_addr);
    memcpy(temp_ip, &member_ip, sizeof(UI32_T));

    /* Only handle from member.
     */
    if(CLUSTER_POM_MemberIpToId(&member_id, temp_ip)==FALSE)
        return FALSE;

    /* chage community.
     */
    SNMP_FREE(pdu->community);
    memcpy(community,session->community,session->community_len);
    sprintf((char *)(community+session->community_len),"@sw%u",member_id);
    if(strlen((char *)community)>SYS_ADPT_MAX_COMM_STR_NAME_LEN)
        return FALSE;
    pdu->community_len = strlen((char *)community);
    pdu->community = (UI8_T *) malloc(pdu->community_len);
    memcpy(pdu->community,community,pdu->community_len);

    /* Reset transport_data, beacuse it use the setting of session.
     */
    SNMP_FREE(pdu->transport_data);
    pdu->transport_data_length=0;
    return TRUE;
}

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
BOOL_T SNMP_CLUSTER_ChangeSecurityNameForRelayTrap(netsnmp_session * session, netsnmp_pdu *pdu)
{
    struct sockaddr_in * addr;
    UI8_T member_id;
    UI8_T security_name[SYS_ADPT_MAX_COMM_STR_NAME_LEN+14];
    UI8_T temp_ip[SYS_ADPT_IPV4_ADDR_LEN]={0};
    UI32_T member_ip;

    /* Check null.
     */
    if(session==NULL||pdu==NULL||pdu->transport_data==NULL)
        return FALSE;

    /* Only handle trap.
     */
    if(pdu->command!=SNMP_MSG_TRAP &&pdu->command!=SNMP_MSG_INFORM&&pdu->command!=SNMP_MSG_TRAP2)
        return FALSE;
    addr =  (struct sockaddr_in *) pdu->transport_data;

    member_ip = L_STDLIB_Ntoh32(addr->sin_addr.s_addr);
    memcpy(temp_ip, &member_ip, sizeof(UI32_T));

    if(CLUSTER_POM_MemberIpToId(&member_id, temp_ip)==FALSE)
        return FALSE;

    /* chage community.
     */
    memcpy(security_name,session->securityName,session->securityNameLen);
    sprintf((char *)(security_name+session->securityNameLen),"@sw%u",member_id);
    if(strlen((char *)security_name)>SYS_ADPT_MAX_COMM_STR_NAME_LEN)
        return FALSE;
    pdu->securityModel=session->securityModel;
    SNMP_FREE(pdu->securityEngineID);
    pdu->securityEngineIDLen=session->securityEngineIDLen;
    memdup((u_char **)&pdu->securityEngineID,session->securityEngineID,pdu->securityEngineIDLen);
    SNMP_FREE(pdu->securityName);
    pdu->securityNameLen = strlen((char *)security_name);
    memdup((u_char **)&pdu->securityName,security_name,pdu->securityNameLen);
    pdu->securityLevel=session->securityLevel;

    /* Reset transport_data, beacuse it use the setting of session.
     */
    SNMP_FREE(pdu->transport_data);
    pdu->transport_data_length=0;
    return TRUE;
}

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
BOOL_T SNMP_CLUSTER_RelayProcess(netsnmp_session * session, netsnmp_pdu *pdu)
{
    UI32_T member_id;
    netsnmp_pdu *new_pdu;
    UI8_T new_name[SYS_ADPT_MAX_COMM_STR_NAME_LEN+1];
    UI8_T old_name[SYS_ADPT_MAX_COMM_STR_NAME_LEN+1];
    UI32_T check_write_rel,check_read_rel;

    /* use "@sw%d" to identify as management PDU.
     */
    if(SNMP_CLUSTER_GetToMemberIdFromManagementPdu(pdu,&member_id,new_name)==TRUE)
    {
        /* Only handle get, getnext, set, getbulk packet.
         */
        if(pdu->command!=SNMP_MSG_GET&&pdu->command!=SNMP_MSG_GETNEXT&&
            pdu->command!=SNMP_MSG_SET&&pdu->command!=SNMP_MSG_GETBULK)
        {
            return FALSE;
        }

        /* Filter "@sw%d" of the community for checkiing the write view access.
         */
        switch(pdu->version)
        {
            case SNMP_VERSION_1:
            case SNMP_VERSION_2c:
                memcpy(old_name,pdu->community,pdu->community_len);
                old_name[pdu->community_len]='\0';
                SNMP_FREE(pdu->community);
                pdu->community_len=strlen((char *)new_name);
                memdup((u_char **)&pdu->community,new_name,pdu->community_len+1);
                pdu->community[pdu->community_len]='\0';
                break;
            case SNMP_VERSION_3:
                memcpy(old_name,pdu->securityName,pdu->securityNameLen);
                old_name[pdu->securityNameLen]='\0';
                SNMP_FREE(pdu->securityName);
                pdu->securityNameLen=strlen((char *)new_name);
                memdup((u_char **)&pdu->securityName,new_name,pdu->securityNameLen+1);
                pdu->securityName[pdu->securityNameLen]='\0';
                break;
            default:
                break;
        }

        check_read_rel=check_access(pdu);
        check_write_rel=SNMP_CLUSTER_CheckWriteView(pdu);

        /* Reesotre "@sw%d" of the community for checkiing the write view access.
         */
        switch(pdu->version)
        {
            case SNMP_VERSION_1:
            case SNMP_VERSION_2c:
                SNMP_FREE(pdu->community);
                pdu->community_len=strlen((char *)old_name);
                memdup((u_char **)&pdu->community,old_name,pdu->community_len);
                break;
            case SNMP_VERSION_3:
                SNMP_FREE(pdu->securityName);
                pdu->securityNameLen=strlen((char *)old_name);
                memdup((u_char **)&pdu->securityName,old_name,pdu->securityNameLen);
                break;
            default:
                break;
        }

        /* if access right of the setting packet is not in read view, then return.
         */
        if(check_read_rel!= 0)
        {
            /* Issue an authenticationFailure trap, referencing RFC 3414
             */
            SNMP_MGR_Gen_Auth_Failure_Trap();

            return TRUE;
        }

        /* if access right of the setting packet is not in write view, then return.
         */
        if(check_write_rel==FALSE)
        {
            pdu->command = SNMP_MSG_RESPONSE;
            pdu->errstat = SNMP_ERR_NOTWRITABLE;
            pdu->errindex = 0;
            new_pdu = snmp_clone_pdu(pdu);
            snmp_send(session,new_pdu);
            return TRUE;
        }

        /* Relay to member.
         */
        if(SNMP_CLUSTER_RelayToMember(pdu,member_id)!=TRUE)
        {
            return TRUE;
        }
    }
    /* if the packet is from the member, we should handle it in here.
     */
    else if(SNMP_CLUSTER_CheckFromMemberPdu(pdu)==TRUE)
    {

       /* Hanle the trap paccket from the member.
        */
        if(SNMP_CLUSTER_RelayTrapToManagement(pdu)==TRUE)
        {
            return TRUE;

        }
       /* Hanle the the paccket from the member.
        */
        else if(SNMP_CLUSTER_RelayToManagement(pdu)!=TRUE)
        {
            return TRUE;
        }
    }
    else /*not management and member pdu,so not relay,and should be handle this packet*/
    {
        return FALSE;
    }
    new_pdu = snmp_clone_pdu(pdu);

    /* We relay this packet as the REsponse PDU.
     */
    new_pdu->flags = new_pdu->flags | UCD_MSG_FLAG_RESPONSE_PDU;
    /* Send PDU.
     */
    snmp_send(session,new_pdu);
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_ChangeToCommander
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will  initial resources for being the commander.
 * INPUT:
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T  SNMP_CLUSTER_ChangeToCommander(void)
{
    UI32_T i;

    for(i=0;i<SYS_ADPT_CLUSTER_MAX_NBR_OF_MEMBERS;i++)
    {
        managemnet_session_entrys[i]=NULL;
        member_request_id[i]=0;
    }
    SNMP_CLUSTER_ControlRelayManagemnetBackSessionCounter(0);
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_CLUSTER_ChangeToOtherFromCommander
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will  release resources for changing the cluster role from the commander.
 * INPUT:
 * OUTPUT:
 * RETURN:  TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T  SNMP_CLUSTER_ChangeToOtherFromCommander(void)
{
    UI32_T i;
    SNMP_CLUSTER_RelayManagemnetBackSession_T *temp_session,*free_session;

    for(i=0;i<SYS_ADPT_CLUSTER_MAX_NBR_OF_MEMBERS;i++)
    {
        temp_session = managemnet_session_entrys[i];
        while(temp_session!=NULL)
        {
            free_session = temp_session;
            temp_session=temp_session->next;
            SNMP_FREE( free_session->transport_data);
            SNMP_FREE( free_session->community);
            SNMP_FREE( free_session->contextEngineID);
            SNMP_FREE( free_session->contextName);
            SNMP_FREE( free_session->securityEngineID);
            SNMP_FREE( free_session->securityName);
            SNMP_FREE(free_session);
        }
         managemnet_session_entrys[i]=NULL;
    }
    return TRUE;
}


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
void SNMP_CLUSTER_SetClusterRole (UI32_T role)
{
    snmp_next_cluster_role = role;
    return;
}

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
void SNMP_CLUSTER_InitRole(void)
{
    snmp_current_cluster_role=CLUSTER_TYPE_CANDIDATE;
    snmp_next_cluster_role=CLUSTER_TYPE_CANDIDATE;
}

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
void SNMP_CLUSTER_CheckRoleChange (void)
{
    UI32_T next_cluster_role;

    next_cluster_role = snmp_next_cluster_role;

    if(snmp_current_cluster_role==next_cluster_role)
    {
        return;
    }

    if(snmp_current_cluster_role==CLUSTER_TYPE_COMMANDER)
    {
        SNMP_CLUSTER_ChangeToOtherFromCommander();
    }

    if(next_cluster_role==CLUSTER_TYPE_COMMANDER)
    {
        SNMP_CLUSTER_ChangeToCommander();
    }

    snmp_current_cluster_role=next_cluster_role;
}
#endif  /* #if (SYS_CPNT_CLUSTER==TRUE) */

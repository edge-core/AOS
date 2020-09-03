/* MODULE NAME: dns_pom.h
 * PURPOSE:
 *    pom implement for dns.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    11/28/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef DNS_POM_H
#define DNS_POM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "dns_om.h"
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : DNS_POM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in the calling process, means the process that use
 *    this pom functions should call this init.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DNS_POM_InitiateProcessResource(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterReqUnparses
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterReqUnparses(int *dns_serv_counter_req_unparses_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsAuthAns
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsAuthAns(int *dns_serv_opt_counter_friends_auth_ans_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsAuthNoDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsAuthNoDataResps(UI32_T *dns_serv_opt_counter_friends_auth_no_data_resps);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsAuthNoNames
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsAuthNoNames(int *dns_serv_opt_counter_friends_auth_no_names);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsErrors
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsErrors(UI32_T *dns_serv_opt_counter_friends_errors_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsNonAuthDatas
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsNonAuthDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_datas_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsNonAuthNoDatas
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsNonAuthNoDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_no_datas_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsOtherErrors
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsOtherErrors(UI32_T *dns_serv_opt_counter_friends_other_errors_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsNonAuthNoDatas
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsNonAuthNoDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_no_datas_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsReferrals
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsReferrals(UI32_T *dns_serv_opt_counter_friends_referrals_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsRelNames
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsRelNames(UI32_T *dns_serv_opt_counter_friends_rel_names_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsReqRefusals
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsReqRefusals(UI32_T *dns_serv_opt_counter_friends_req_refusals_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsReqUnparses
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsReqUnparses(UI32_T *dns_serv_opt_counter_friends_req_unparses_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfAuthAns
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfAuthAns(int *dns_serv_opt_counter_self_auth_ans_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsRelNames
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsRelNames(UI32_T *dns_serv_opt_counter_friends_rel_names_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfAuthNoDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfAuthNoDataResps(int *dns_serv_opt_counter_self_auth_no_data_resps);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfAuthNoNames
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfAuthNoNames(int *dns_serv_opt_counter_self_auth_no_names_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfErrors
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfErrors(int *dns_serv_opt_counter_self_errors_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfNonAuthDatas
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfNonAuthDatas(int *dns_serv_opt_counter_self_non_auth_datas_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfNonAuthNoDatas
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfNonAuthNoDatas(int *dns_serv_opt_counter_self_non_auth_no_datas_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfOtherErrors
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfOtherErrors(int *dns_serv_opt_counter_self_other_errors_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfReferrals
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfReferrals(int *dns_serv_opt_counter_self_referrals);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfRelNames
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfRelNames( int *dns_serv_opt_counter_self_rel_names);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfReqRefusals
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfReqRefusals(int *dns_serv_opt_counter_self_req_refusals_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfReqUnparses
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfReqUnparses(int *dns_serv_opt_counter_self_req_unparses_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfReqUnparses
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfReqUnparses(int *dns_serv_opt_counter_self_req_unparses_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfOtherErrors
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfOtherErrors(int *dns_serv_opt_counter_self_other_errors_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetNameServerByIndex
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T DNS_POM_GetNameServerByIndex(UI32_T index, L_INET_AddrIp_T *ip);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDomainNameList
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T DNS_POM_GetDomainNameList(I8_T *dns_ip_domain_name);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterByOpcodeEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterByRcodeEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterFallbacks
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterFallbacks(int *dns_res_counter_fallbacks_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterMartians
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterMartians(int *dns_res_counter_martians_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterNonAuthDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterNonAuthDataResps(int *dns_res_counter_non_auth_data_resps_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterNonAuthNoDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterNonAuthNoDataResps(int *dns_res_counter_non_auth_no_data_resps_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterRecdResponses
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterRecdResponses(int *dns_res_counter_recd_responses_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterUnparseResps
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterUnparseResps(int *dns_res_counter_unparse_resps_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterInternals
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterInternals(int *dns_res_opt_counter_internals);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterInternalTimeOuts
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterInternalTimeOuts(int *dns_res_opt_counter_internal_time_outs_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterNoResponses
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterNoResponses(int *dns_res_opt_counter_no_responses_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterReferals
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterReferals(int *dns_res_opt_counter_referals_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterRetrans
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterRetrans(int *dns_res_opt_counter_retrans_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterRootRetrans
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterRootRetrans(int *dns_res_opt_counter_root_retrans_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServConfigImplementIdent
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServConfigImplementIdent(I8_T *dns_serv_config_implement_ident_p) ;

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServConfigRecurs
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServConfigRecurs(int *config_recurs_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServConfigResetTime
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServConfigResetTime(UI32_T	*dns_serv_config_reset_time);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServConfigUpTime
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServConfigUpTime(UI32_T *dns_serv_config_up_time_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterAuthAns
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterAuthAns(int *dns_serv_counter_auth_ans_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterAuthNoDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterAuthNoDataResps(int *dns_serv_counter_auth_no_data_resps_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterAuthNoNames
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterAuthNoNames(int *dns_serv_counter_auth_ans_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterAuthNoDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterErrors
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterErrors(int *dns_serv_counter_errors_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterNonAuthDatas
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterNonAuthDatas(int *dns_serv_counter_non_auth_datas_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterNonAuthNoDatas
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterNonAuthNoDatas(int *dns_serv_counter_non_auth_no_datas_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterOtherErrors
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterOtherErrors(int *dns_serv_counter_other_errors_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterReferrals
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterReferrals(int *dns_serv_counter_referrals_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterRelNames
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterRelNames(int *dns_serv_counter_rel_names_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetNextNameServerList
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_POM_GetNextNameServerList(L_INET_AddrIp_T *ip_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterReqRefusals
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterReqRefusals(int *dns_serv_counter_req_refusals_p);

/* FUNCTION NAME : DNS_POM_GetDomainNameListEntry
 * PURPOSE: To get entry from the dnsDomainListEntry table.
 * INPUT  : idx           -- index of dnsDomainListEntry
 *                           (1-based, key to search the entry)
 * OUTPUT : domain_name_p -- pointer to domain name content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_POM_GetDomainNameListEntry(UI32_T idx, I8_T *domain_name_p);

/* FUNCTION NAME : DNS_POM_GetNextDomainNameListEntry
 * PURPOSE: To get next entry from the dnsDomainListEntry table.
 * INPUT  : idx_p         -- index of dnsDomainListEntry
 *                           (1-based, 0 to get the first,
 *                            key to search the entry)
 * OUTPUT : idx_p         -- next index of dnsDomainListEntry
 *          domain_name_p -- pointer to domain name content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_POM_GetNextDomainNameListEntry(UI32_T *idx_p, I8_T *domain_name_p);

#endif  /* #ifndef DNS_POM_H */

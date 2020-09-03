/* MODULE NAME: dns_pom.c
 * PURPOSE:
 *    PMGR implement for dns.
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

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "dns_pom.h"
#include "dns_om.h"
#include "sys_module.h"
#include "l_mm.h"
#include "sysfun.h"
#include "sys_module.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void DNS_POM_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

/* STATIC VARIABLE DECLARATIONS
 */

static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : DNS_POM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in the calling process, means the process that use
 *    this pmgr functions should call this init.
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
BOOL_T DNS_POM_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_APP_PROTOCOL_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterReqUnparses
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterReqUnparses(int *dns_serv_counter_req_unparses_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVER_COUNTER_REQ_UNPARSES,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_counter_req_unparses_p = data_p->dnsServCounterReqUnparses;
    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsAuthAns
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsAuthAns(int *dns_serv_opt_counter_friends_auth_ans_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVER_OPT_COUNTER_FRIENDS_AUTH_ANS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
		*dns_serv_opt_counter_friends_auth_ans_p = data_p->dnsServOptCounterFriendsAuthAns;
    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsAuthNoDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsAuthNoDataResps(UI32_T *dns_serv_opt_counter_friends_auth_no_data_resps)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_AUTH_NO_DATE_RESPS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
		*dns_serv_opt_counter_friends_auth_no_data_resps = data_p->dnsServOptCounterFriendsAuthNoDataResps;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsAuthNoNames
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsAuthNoNames(int *dns_serv_opt_counter_friends_auth_no_names)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SEROPT_COUNTER_FRIENDS_AUTH_NO_NAMES,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_friends_auth_no_names = data_p->dnsServOptCounterFriendsAuthNoNames;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsErrors
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsErrors(UI32_T *dns_serv_opt_counter_friends_errors_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_DNS_SERVOPT_COUNTER_FRIENDS_ERRORS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_friends_errors_p = data_p->dnsServOptCounterFriendsErrors;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsNonAuthDatas
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsNonAuthDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_datas_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_DNS_SERVOPT_COUNTER_FRIENDS_NON_AUTH_DATES,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_friends_non_auth_datas_p = data_p->dnsServOptCounterFriendsNonAuthDatas;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsNonAuthNoDatas
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsNonAuthNoDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_no_datas_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_NON_AUTH_NO_DATAS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_friends_non_auth_no_datas_p = data_p->dnsServOptCounterFriendsNonAuthNoDatas;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsOtherErrors
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsOtherErrors(UI32_T *dns_serv_opt_counter_friends_other_errors_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_GETSERVOPT_COUNTER_FRIENDS_OTHER_ERRORS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_friends_other_errors_p = data_p->dnsServOptCounterFriendsOtherErrors;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsNonAuthNoDatas
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
/*
int DNS_POM_GetDnsServOptCounterFriendsNonAuthNoDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_no_datas_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_NON_AUTH_NO_DATAS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	dns_serv_opt_counter_friends_non_auth_no_datas_p = data_p->dnsServOptCounterFriendsNonAuthNoDatas;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}
*/

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsReferrals
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsReferrals(UI32_T *dns_serv_opt_counter_friends_referrals_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDSREFERRALS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_friends_referrals_p = data_p->dnsServOptCounterFriendsReferrals;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsRelNames
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsRelNames(UI32_T *dns_serv_opt_counter_friends_rel_names_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_RELNAMES,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
   	*dns_serv_opt_counter_friends_rel_names_p = data_p->dnsServOptCounterFriendsRelNames;
    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsReqRefusals
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsReqRefusals(UI32_T *dns_serv_opt_counter_friends_req_refusals_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_REQREFUSALS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_friends_req_refusals_p = data_p->dnsServOptCounterFriendsReqRefusals;
    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsReqUnparses
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterFriendsReqUnparses(UI32_T *dns_serv_opt_counter_friends_req_unparses_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_REQUNPARSES,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_friends_req_unparses_p = data_p->dnsServOptCounterFriendsReqUnparses;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfAuthAns
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfAuthAns(int *dns_serv_opt_counter_self_auth_ans_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_AUTHANS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_auth_ans_p = data_p->dnsServOptCounterSelfAuthAns;
    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterFriendsRelNames
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
/*
int DNS_POM_GetDnsServOptCounterFriendsRelNames(UI32_T *dns_serv_opt_counter_friends_rel_names_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_RELNAMES,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_friends_rel_names_p = data_p->dnsServOptCounterFriendsRelNames;
    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}
*/
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfAuthNoDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfAuthNoDataResps(int *dns_serv_opt_counter_self_auth_no_data_resps)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_AUTH_NO_DATA_RESPS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_auth_no_data_resps = data_p->dnsServOptCounterSelfAuthNoDataResps;
    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfAuthNoNames
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfAuthNoNames(int *dns_serv_opt_counter_self_auth_no_names_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_AUTH_NO_NAMES,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_auth_no_names_p = data_p->dnsServOptCounterSelfAuthNoNames;
    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfErrors
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfErrors(int *dns_serv_opt_counter_self_errors_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_ERRORS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_errors_p = data_p->dnsServOptCounterSelfErrors;
    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfNonAuthDatas
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfNonAuthDatas(int *dns_serv_opt_counter_self_non_auth_datas_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_NON_AUTH_DATAS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_non_auth_datas_p = data_p->dnsServOptCounterSelfNonAuthDatas;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfNonAuthNoDatas
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfNonAuthNoDatas(int *dns_serv_opt_counter_self_non_auth_no_datas_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_NON_AUTH_NO_DATAS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_non_auth_no_datas_p = data_p->dnsServOptCounterSelfNonAuthNoDatas;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfOtherErrors
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfOtherErrors(int *dns_serv_opt_counter_self_other_errors_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_OTHER_ERRORS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_other_errors_p = data_p->dnsServOptCounterSelfOtherErrors;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfReferrals
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfReferrals(int *dns_serv_opt_counter_self_referrals)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_REFERRALS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_referrals = data_p->dnsServOptCounterSelfReferrals;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfRelNames
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfRelNames( int *dns_serv_opt_counter_self_rel_names)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_RELNAMES,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_rel_names = data_p->dnsServOptCounterSelfRelNames;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfReqRefusals
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfReqRefusals(int *dns_serv_opt_counter_self_req_refusals_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_REQ_REFUSALS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_req_refusals_p = data_p->dnsServOptCounterSelfReqRefusals;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfReqUnparses
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServOptCounterSelfReqUnparses(int *dns_serv_opt_counter_self_req_unparses_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_REQ_UNPARSES,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_req_unparses_p = data_p->dnsServOptCounterSelfReqUnparses;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServOptCounterSelfOtherErrors
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
/*
int DNS_POM_GetDnsServOptCounterSelfOtherErrors(int *dns_serv_opt_counter_self_other_errors_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_ProxyCounter_T   *data_p;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_OTHER_ERRORS,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                      (UI32_T)FALSE);
	*dns_serv_opt_counter_self_other_errors_p = data_p->dnsServOptCounterSelfOtherErrors;
    return (int)DNS_OM_MSG_RETVAL(msg_p);
}
*/
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetNameServerByIndex
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T DNS_POM_GetNameServerByIndex(UI32_T index, L_INET_AddrIp_T *ip)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_Name_Server_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_Name_Server_T   *data_p;

	if(ip == NULL)
		return DNS_ERROR;

	data_p = DNS_OM_MSG_DATA(msg_p);

	data_p->index = index;
    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_NAME_SERVER_BY_INDEX,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_Name_Server_T),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_Name_Server_T),
                      (UI32_T)FALSE);

    memcpy(ip, &data_p->name_server_ip, sizeof(data_p->name_server_ip));
    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDomainNameList
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T DNS_POM_GetDomainNameList(I8_T *dns_ip_domain_name)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_IpDomain_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_IpDomain_T   *data_p;

	if(dns_ip_domain_name == NULL)
		return FALSE;

	data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_DOMAIN_NAME_LIST,
                      msg_p,
                      DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_IpDomain_T),
                      (UI32_T)FALSE);
	strcpy((char *)dns_ip_domain_name, (char *)data_p->DnsIpDomainName);
    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterByOpcodeEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounterByOpcodeEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounterByOpcodeEntry_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);
    memcpy(data_p, dns_res_counter_by_opcode_entry_p, sizeof(DNS_ResCounterByOpcodeEntry_T));

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_DNS_RES_COUNTER_BY_OPCODE_ENTRY,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounterByOpcodeEntry_T),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounterByOpcodeEntry_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        memcpy(dns_res_counter_by_opcode_entry_p, data_p, sizeof(DNS_ResCounterByOpcodeEntry_T));

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterByRcodeEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounterByRcodeEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounterByRcodeEntry_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);

    memcpy(data_p, dns_res_counter_by_rcode_entry_p, sizeof(DNS_ResCounterByRcodeEntry_T));
    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_COUNTER_BY_RCODE_ENTRY,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounterByRcodeEntry_T),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounterByRcodeEntry_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        memcpy(dns_res_counter_by_rcode_entry_p, data_p, sizeof(DNS_ResCounterByRcodeEntry_T));

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterFallbacks
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterFallbacks(int *dns_res_counter_fallbacks_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_COUNTER_FALLBACKS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_counter_fallbacks_p = data_p->dnsResCounterFallbacks;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterMartians
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterMartians(int *dns_res_counter_martians_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_COUNTER_MARTIANS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_counter_martians_p = data_p->dnsResCounterMartians;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterNonAuthDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterNonAuthDataResps(int *dns_res_counter_non_auth_data_resps_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_COUNTER_NON_AUTH_DATA_RESPS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_counter_non_auth_data_resps_p = data_p->dnsResCounterNonAuthDataResps;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterNonAuthNoDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterNonAuthNoDataResps(int *dns_res_counter_non_auth_no_data_resps_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_COUNTER_NON_AUTH_NO_DATA_RESPS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_counter_non_auth_no_data_resps_p = data_p->dnsResCounterNonAuthNoDataResps;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterRecdResponses
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterRecdResponses(int *dns_res_counter_recd_responses_p)
{
        UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_COUNTER_RECD_RESPONSES,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_counter_recd_responses_p = data_p->dnsResCounterRecdResponses;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResCounterUnparseResps
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResCounterUnparseResps(int *dns_res_counter_unparse_resps_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_COUNTER_UNPARSE_RESPS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_counter_unparse_resps_p = data_p->dnsResCounterUnparseResps;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterInternals
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterInternals(int *dns_res_opt_counter_internals)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_INTERNALS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_opt_counter_internals = data_p->dnsResOptCounterInternals;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterInternalTimeOuts
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterInternalTimeOuts(int *dns_res_opt_counter_internal_time_outs_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_INTERNAL_TIMEOUTS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_opt_counter_internal_time_outs_p = data_p->dnsResOptCounterInternalTimeOuts;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterNoResponses
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterNoResponses(int *dns_res_opt_counter_no_responses_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_NO_RESPONSES,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_opt_counter_no_responses_p = data_p->dnsResOptCounterNoResponses;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterReferals
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterReferals(int *dns_res_opt_counter_referals_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_REFERALS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_opt_counter_referals_p = data_p->dnsResOptCounterReferals;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterRetrans
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterRetrans(int *dns_res_opt_counter_retrans_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_RETRANS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_opt_counter_retrans_p = data_p->dnsResOptCounterRetrans;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsResOptCounterRootRetrans
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsResOptCounterRootRetrans(int *dns_res_opt_counter_root_retrans_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_ROOT_RETRANS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_res_opt_counter_root_retrans_p = data_p->dnsResOptCounterRootRetrans;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServConfigImplementIdent
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServConfigImplementIdent(I8_T *dns_serv_config_implement_ident_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyConfig_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyConfig_T *data_p;

    if(dns_serv_config_implement_ident_p == NULL)
    {
        return DNS_ERROR;
    }

    data_p = DNS_OM_MSG_DATA(msg_p);

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_CONFIG_OMPLEMENT_IDENT,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyConfig_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        strcpy((char *)dns_serv_config_implement_ident_p ,  (char *)data_p->dnsServConfigImplementIdent);

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServConfigRecurs
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServConfigRecurs(int *config_recurs_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyConfig_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyConfig_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_CONFIG_RECURS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyConfig_T),
                       0);

    if (DNS_OM_MSG_RETVAL(msg_p) != DNS_ERROR)
        *config_recurs_p = data_p->dnsServConfigRecurs;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServConfigResetTime
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServConfigResetTime(UI32_T	*dns_serv_config_reset_time)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_TIME_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_TIME_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_CONFIG_RESET_TIME,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_TIME_T),
                       0);


    *dns_serv_config_reset_time = data_p->time;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServConfigUpTime
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServConfigUpTime(UI32_T *dns_serv_config_up_time_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_TIME_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_TIME_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_CONFIG_UP_TIME,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_TIME_T),
                       0);


    *dns_serv_config_up_time_p = data_p->time;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterAuthAns
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterAuthAns(int *dns_serv_counter_auth_ans_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_COUNTER_AUTH_ANS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                       0);


    *dns_serv_counter_auth_ans_p = data_p->dnsServCounterAuthAns;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterAuthNoDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterAuthNoDataResps(int *dns_serv_counter_auth_no_data_resps_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_COUNTER_AUTH_NO_DATA_RESPS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                       0);


    *dns_serv_counter_auth_no_data_resps_p = data_p->dnsServCounterAuthNoDataResps;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterAuthNoNames
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterAuthNoNames(int *dns_serv_counter_auth_ans_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_COUNTER_AUTH_NO_NAMES,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                       0);


    *dns_serv_counter_auth_ans_p = data_p->dnsServCounterAuthNoNames;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterAuthNoDataResps
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ServCounterEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ServCounterEntry_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);

    if(dns_serv_counter_entry_t_p == NULL)
        return DNS_ERROR;

    memcpy(data_p, dns_serv_counter_entry_t_p, sizeof(DNS_OM_IPCMsg_ServCounterEntry_T));

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_COUNTER_ENTRY,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ServCounterEntry_T),
                       0);


    memcpy(dns_serv_counter_entry_t_p, data_p, sizeof(DNS_ServCounterEntry_T));

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterErrors
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterErrors(int *dns_serv_counter_errors_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_COUNTER_ERRORS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                       0);


    *dns_serv_counter_errors_p = data_p->dnsServCounterErrors;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterNonAuthDatas
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterNonAuthDatas(int *dns_serv_counter_non_auth_datas_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_COUNTER_NON_AUTH_DATAS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                       0);


    *dns_serv_counter_non_auth_datas_p = data_p->dnsServCounterNonAuthDatas;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterNonAuthNoDatas
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterNonAuthNoDatas(int *dns_serv_counter_non_auth_no_datas_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_COUNTER_NON_AUTH_NO_DATAS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                       0);


    *dns_serv_counter_non_auth_no_datas_p = data_p->dnsServCounterNonAuthNoDatas;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterOtherErrors
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterOtherErrors(int *dns_serv_counter_other_errors_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_COUNTER_OTHER_ERRORS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                       0);


    *dns_serv_counter_other_errors_p = data_p->dnsServCounterOtherErrors;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterReferrals
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterReferrals(int *dns_serv_counter_referrals_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_COUNTER_REFERRALS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                       0);


    *dns_serv_counter_referrals_p = data_p->dnsServCounterReferrals;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterRelNames
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterRelNames(int *dns_serv_counter_rel_names_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERV_COUNTER_RELNAMES,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                       0);


    *dns_serv_counter_rel_names_p = data_p->dnsServCounterRelNames;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetNextNameServerList
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_POM_GetNextNameServerList(L_INET_AddrIp_T *ip_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_Name_Server_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_Name_Server_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);

    memcpy(&data_p->name_server_ip, ip_p, sizeof(L_INET_AddrIp_T));

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_NEXT_SERVER_LIST,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_Name_Server_T),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_Name_Server_T),
                       0);

    memcpy(ip_p, &data_p->name_server_ip, sizeof(L_INET_AddrIp_T));

    return DNS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_POM_GetDnsServCounterReqRefusals
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_POM_GetDnsServCounterReqRefusals(int *dns_serv_counter_req_refusals_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ProxyCounter_T *data_p;

    data_p = DNS_OM_MSG_DATA(msg_p);


    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_SERVER_COUNTER_REQ_REFUSALS,
                       msg_p,
                       DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T),
                       0);


    *dns_serv_counter_req_refusals_p = data_p->dnsServCounterReqRefusals;

    return DNS_OM_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_POM_GetDomainNameListEntry
 * PURPOSE: To get entry from the dnsDomainListEntry table.
 * INPUT  : idx           -- index of dnsDomainListEntry
 *                           (1-based, key to search the entry)
 * OUTPUT : domain_name_p -- pointer to domain name content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_POM_GetDomainNameListEntry(UI32_T idx, I8_T *domain_name_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_IdxNameStr_T   *data_p;

	if(domain_name_p == NULL)
		return FALSE;

	data_p = DNS_OM_MSG_DATA(msg_p);
    data_p->index = idx;

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_DOMAIN_NAME_LIST_ENTRY,
                    msg_p,
                    DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_IdxNameStr_T),
                    DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_IdxNameStr_T),
                    (UI32_T)FALSE);

    if (TRUE == (BOOL_T) DNS_OM_MSG_RETVAL(msg_p))
    {
        memcpy(domain_name_p, data_p->name_str, sizeof(data_p->name_str));
    }

    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}

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
BOOL_T DNS_POM_GetNextDomainNameListEntry(UI32_T *idx_p, I8_T *domain_name_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
	DNS_OM_IPCMsg_IdxNameStr_T   *data_p;

	if ((idx_p == NULL) && (domain_name_p == NULL))
		return FALSE;

	data_p = DNS_OM_MSG_DATA(msg_p);
    data_p->index = *idx_p;

    DNS_POM_SendMsg(DNS_OM_IPC_CMD_GET_NEXT_DOMAIN_NAME_LIST_ENTRY,
                    msg_p,
                    DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_IdxNameStr_T),
                    DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_IdxNameStr_T),
                    (UI32_T)FALSE);

    if (TRUE == (BOOL_T) DNS_OM_MSG_RETVAL(msg_p))
    {
        *idx_p  = data_p->index;
        memcpy(domain_name_p, data_p->name_str, sizeof(data_p->name_str));
    }

    return (BOOL_T)DNS_OM_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_POM_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the DNS message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of DNS request message.
 *           res_size  - the size of DNS response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static void DNS_POM_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_DNS;
    msg_p->msg_size = req_size;

    DNS_OM_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,  /* pgr0695, return value of statement block in macro */
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                0,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if (ret != SYSFUN_OK)
        DNS_OM_MSG_RETVAL(msg_p) = ret_val;
}

#endif /* #if (SYS_CPNT_DNS == TRUE) */


/* Project Name: New Feature
 * File_Name : tac_account_c.h
 * Purpose     : TACACS+ authorization
 *
 * 2002/05/07    : Ricky Lin Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    :
 */
#ifndef TAC_ACCOUNT_C_H
#define TAC_ACCOUNT_C_H

#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
#include "tacacs_mgr.h"

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - tacacs_acc_main
 * ---------------------------------------------------------------------
 * PURPOSE: This function will do accounting.
 * INPUT : None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES :
 * ---------------------------------------------------------------------
 */
BOOL_T tacacs_acc_main(UI32_T server_ip,
                       UI8_T *secret,
                       UI32_T server_port,
                       UI32_T retransmit,
                       UI32_T timeout,
                       int flag,
                       TACACS_AccUserInfo_T *user_info,
                       UI32_T current_sys_time);

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)
BOOL_T TAC_ACCOUNT_C_AcctCmdMain(UI32_T server_ip,
                                 UI8_T *secret,
                                 UI32_T server_port,
                                 UI32_T timeout,
                                 UI32_T retransmit,
                                 int    flag,
                                 const TPACC_AccCommandmdMessage_T *cmd_entry_p,
                                 UI32_T current_sys_time);
#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)*/

#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */
#endif /* End of TAC_ACCOUNT_C_H */


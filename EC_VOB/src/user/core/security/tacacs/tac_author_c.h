/* Project Name: New Feature
 * File_Name : tac_author_c.h
 * Purpose     : TACACS+ authorization
 *
 * 2002/05/07    : Ricky Lin Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    :
 */
#ifndef TAC_AUTHOR_C_H
#define TAC_AUTHOR_C_H

#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE) || (SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD == SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION)
#include "tacacs_mgr.h"

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - tacacs_author_main
 * ---------------------------------------------------------------------
 * PURPOSE: This function will do authorization.
 * INPUT : None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES :
 * ---------------------------------------------------------------------
 */
BOOL_T tacacs_author_main(TACACS_AuthorRequest_T *request,TACACS_AuthorReply_T *reply);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - tacacs_author_shell_service
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will do shell service authorization.
 * INPUT    : request
 * OUTPUT   : reply
 * RETURN   : TRUE -- success /FALSE -- failure
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
BOOL_T tacacs_author_shell_service(TACACS_AuthorRequest_T *request, TACACS_AuthorReply_T *reply);

#endif /* SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE || SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD == SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION */

#endif /* End of TAC_AUTHOR_C_H */

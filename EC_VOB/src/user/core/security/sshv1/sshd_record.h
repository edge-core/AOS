/* MODULE NAME: sshd_record.h
* PURPOSE: 
*
*
* NOTES:
*   
* History:                                                               
*       Date          -- Modifier,  Reason
*     2002-05-24      -- Isiah , created.
*   
* Copyright(C)      Accton Corporation, 2002
*/

#ifndef SSHD_RECORD_H

#define SSHD_RECORD_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sshd_type.h"
#include "ssh_cipher.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define SSHD_Context_T  sshd_context_t
#define SSHD_Cipher_T   ssh_cipher_t

/* DATA TYPE DECLARATIONS
 */

typedef struct SSHD_Session_Connection_S
{
	UI32_T	keepalive;
	sshd_context_t	ssh_context;
	UI32_T	tid;
	I32_T   connection_id;
	UI32_T	major_version;
	UI32_T  minor_version;
	UI8_T   *username;
	SSHD_ConnectionState_T	status;
	UI32_T          tnsh_tid;   /* the tnsh task-id associated with this socket connection. */
	unsigned        user_ip;            /*  remote ip of user       */
    unsigned short  user_port;          /*  remote port of user     */
    unsigned short  user_local_port;    /*  local side port to user */
    unsigned short  tnsh_port;          /*  port connect to TNSHD   */
    unsigned short  remote_tnsh_port;   /*  remote side(tnsh) port to TNSHD */
}SSHD_Session_Connection_T;

typedef struct SSHD_Session_Record_S
{
	UI32_T	created_session_count;
	UI32_T	timeout;
	UI32_T	authentication_retries;
	UI32_T	server_major_version;
	UI32_T  server_minor_version;
	SSHD_Session_Connection_T	connection[SSHD_DEFAULT_MAX_SESSION_NUMBER+1];
}SSHD_Session_Record_T;





/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */










#endif /* ifndef SSHD_RECORD_H */




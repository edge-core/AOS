/* $Id: ssh_messages.h,v 1.3.4.1 2000/08/25 09:32:10 tls Exp $ */

/*
 * Copyright 1999 RedBack Networks, Incorporated.
 * All rights reserved.
 *
 * This software is not in the public domain.  It is distributed 
 * under the terms of the license in the file LICENSE in the
 * same directory as this file.  If you have received a copy of this
 * software without the LICENSE file (which means that whoever gave
 * you this software violated its license) you may obtain a copy from
 * http://www.panix.com/~tls/LICENSE.txt
 */


#ifndef _SSH_MESSAGES_H
#define _SSH_MESSAGES_H

#define SSH_MSG_NONE			0
#define SSH_MSG_DISCONNECT		1
#define SSH_SMSG_PUBLIC_KEY		2
#define SSH_CMSG_SESSION_KEY		3
#define SSH_CMSG_USER			4
#define SSH_CMSG_AUTH_RHOSTS		5
#define SSH_CMSG_AUTH_RSA		6
#define SSH_SMSG_AUTH_RSA_CHALLENGE	7
#define SSH_CMSG_AUTH_RSA_RESPONSE	8
#define SSH_CMSG_AUTH_PASSWORD		9
#define SSH_CMSG_REQUEST_PTY		10
#define SSH_CMSG_WINDOW_SIZE		11
#define SSH_CMSG_EXEC_SHELL		12
#define SSH_CMSG_EXEC_CMD		13
#define SSH_SMSG_SUCCESS		14
#define SSH_SMSG_FAILURE		15
#define SSH_CMSG_STDIN_DATA		16
#define SSH_SMSG_STDOUT_DATA		17
#define SSH_SMSG_STDERR_DATA		18
#define SSH_CMSG_EOF			19
#define SSH_SMSG_EXITSTATUS		20
#define SSH_MSG_CHANNEL_OPEN_CONFIRMATION	21
#define SSH_MSG_CHANNEL_OPEN_FAILURE	22
#define SSH_MSG_CHANNEL_DATA		23
#define SSH_MSG_CHANNEL_CLOSE		24
#define SSH_MSG_CHANNEL_CLOSE_CONFIRMATION	25
/*obsolete 				26 */
#define SSH_SMSG_X11_OPEN		27
#define SSH_CMSG_PORT_FORWARD_REQUEST	28
#define SSH_MSG_PORT_OPEN		29
#define SSH_CMSG_AGENT_REQUEST_FORWARDING	30
#define SSH_SMSG_AGENT_OPEN		31
#define SSH_MSG_IGNORE			32
#define SSH_CMSG_EXIT_CONFIRMATION	33
#define SSH_CMSG_X11_REQUEST_FORWARDING	34
#define SSH_CMSG_AUTH_RHOSTS_RSA	35
#define SSH_MSG_DEBUG			36
#define SSH_CMSG_REQUEST_COMPRESSION	37
#define SSH_CMSG_MAX_PACKET_SIZE	38
#define SSH_CMSG_AUTH_TIS		39
#define SSH_SMSG_AUTH_TIS_CHALLENGE	40
#define SSH_CMSG_AUTH_TIS_RESPONSE	41
#define SSH_CMSG_AUTH_KERBEROS		42
#define SSH_SMSG_AUTH_KERBEROS_RESPONSE	43
#define SSH_CMSG_HAVE_KERBEROS_TGT	44

#endif /* _SSH_MESSAGES_H */

/* $Id: sshd_prep.c,v 1.28.2.1 2000/08/25 09:32:23 tls Exp $ */

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

/*#include <errno.h>*/
#include <string.h>
#include <stdlib.h>
/*#include <unistd.h>*/
/*Isiah.2002-05-27*/
/* Don't support compress now */
/*#define alloc_func	z_alloc_func
#define free_func	z_free_func
#include <zlib.h>
#undef alloc_func
#undef free_func*/

#include "options.h"

#include "sshd.h"
#include "ssh_buffer.h"
#include "ssh_sys.h"
#include "ssh_util.h"
#include "ssh_messages.h"

/*Isiah.2002-06-10*/
#include "sshd_mgr.h"
#include "sshd_vm.h"


/*
 * doPrepOps: pre-exec state.  Listen for how to set up the connection.
 *		User is already authenticated.
 *		We are running as the authenticated user now.
 *
 *	Args: s		socket to talk on.
 *
 *	Returns: 0	Program has been exec'd.
 *		 1	Client has disconnected.
 */
int doPrepOps(sshd_context_t *context) 
{
    u_int32_t tmp_int32;
    struct ssh_packet *pc;
    int num;
/*    fd_set  readfds;*/
/*    struct 	timeval     timeout;*/

    pc = &(context->in_pc);

    while(1) 
    {
#if 0
        FD_ZERO(&readfds);
        FD_SET(context->s, &readfds);
        timeout.tv_sec = SSHD_VM_GetNegotiationTimeout();     /*  no.  of seconds  */
        timeout.tv_usec = 0;    /*  no. of micro seconds  */
        if ( select(context->s + 1, &readfds, NULL, NULL, &timeout) <= 0 )
        {
            context = (sshd_context_t *)SSHD_VM_GetContextAddress();
            if ( context==NULL)
            {
                return (1);
            }
            else
            {
                SEND_DISCONNECT(context, "Negotiation timeout\n");
                return (1);
            }
        }
#endif
	    
	    do 
	    {
	        if ((num = ssh_read_packet(context, pc, PKT_WAITALL)) > 0)
		        num = process_packet(context, pc);
	    } while (num == 0);
	    if (num < 0) 
	    {
	        if (num == EPIPE) 
	        {
/*		        SSH_DLOG(3, ("doPrepOps: client unexpectedly disconnected.\n"));*/
		        break;
	        }
/*	        SSH_ERROR("doPrepOps: read/process_packet failed\n");*/
	        break;
	    }

/*isiah.2002-06-10*/
        if ( (SSHD_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) || (SSHD_VM_GetSshdStatus() != SSHD_STATE_ENABLED) )
        {
            SEND_DISCONNECT(context, "Administrator compalled this connection to break.");
            return (1);
        }

	    switch(packet_type(pc)) 
	    {
	        case SSH_CMSG_MAX_PACKET_SIZE:
		        packet_get_int32(pc, &tmp_int32);
/*		        SSH_DLOG(4, ("Client requested max packet size:%d\n", 
				        			tmp_int32));*/
		        if (tmp_int32 >= 4096 && tmp_int32 <= (1024*1024*1024)) 
		        {
		            context->max_packet_size = tmp_int32;
		            SEND_SUCCESS(context);
		        } 
		        else 
		        {
/*		            SSH_DLOG(3, ("Invalid max packet size:%d\n", tmp_int32));*/
		            SEND_FAILURE(context);
		        }
		        break;
	        case SSH_CMSG_REQUEST_PTY: 
	        {
	            u_int8_t *term;
	            u_int32_t y_row, x_col, x_pix, y_pix;
	            u_int8_t *modes;
	            int msize;
/*		        SSH_DLOG(4, ("Client requested a pty.\n"));*/
		        if (context->usepty) 
		        {
		            SEND_DISCONNECT(context, "A Pseudotermal session(pty) has already been requested.");
		            return(1);
		        }

		        packet_get_binstr(pc, &term, NULL);
		        packet_get_int32(pc, &y_row);
        		packet_get_int32(pc, &x_col);
		        packet_get_int32(pc, &x_pix);
		        packet_get_int32(pc, &y_pix);
		        packet_get_unk_bytes(pc, &modes, &msize);

		        if (ssh_sys_allocpty(context, term, y_row, x_col, x_pix, y_pix,
						        modes, msize) != 0) 
				{
/*		            SSH_DLOG(1, ("Failed to allocate pty: %s\n", 
					        		strerror(errno)));*/
		            SEND_FAILURE(context);
		        } 
		        else 
		        {
		            SEND_SUCCESS(context);
		        }
		        free(term);
		        free(modes);
		        break;
	        }
	        case SSH_CMSG_REQUEST_COMPRESSION:
/*		        SSH_DLOG(4, ("Client requested compression.\n"));*/
		
		        packet_get_int32(pc, &tmp_int32);

/*		        SSH_DLOG(5, ("Compression level: %d\n", tmp_int32));*/
#ifdef WITH_COMPRESSION	
		        if(tmp_int32 < 1 || tmp_int32 > 9) 
		        {
		            SEND_DEBUGX(context, "No compression level %d.", tmp_int32);
		            SEND_FAILURE(context);
		            break;
		        }
				
		        if (context->compressing) 
		        {
		            SEND_DEBUG(context, "Already compressing!");
		            SEND_FAILURE(context);
		            break;
		        }

		        SEND_SUCCESS(context);

        		memset(&context->inz, 0, sizeof(z_stream));
		        memset(&context->outz, 0, sizeof(z_stream));

		        inflateInit(&(context->inz));

		        /* don't let the client eat all of our CPU compressing. */
		        deflateInit(&(context->outz), tmp_int32 > 5 ? 5 : tmp_int32);

		        context->compressing = 1;
#else	/* WITH_COMPRESSION */
		        SEND_DEBUG(context, "Compression not supported.");
		        SEND_FAILURE(context);
#endif	/* WITH_COMPRESSION */
		        break;

	        case SSH_CMSG_X11_REQUEST_FORWARDING:
/*		        SSH_DLOG(4, ("Client requested X11 forwarding.\n"));
		        SEND_DEBUG(context, "X11 forwarding unimplemented.");*/
		        SEND_FAILURE(context);
		        break;
	        case SSH_CMSG_PORT_FORWARD_REQUEST:
/*		        SSH_DLOG(4, ("Client requested port forwarding.\n"));
		        SEND_DEBUG(context, "Port forwarding unimplemented.");*/
		        SEND_FAILURE(context);
		        break;
	        case SSH_CMSG_AGENT_REQUEST_FORWARDING:
/*		        SSH_DLOG(4, ("Client requested agent forwarding.\n"));
		        SEND_DEBUG(context, "Agent forwarding unimplemented.");*/
		        SEND_FAILURE(context);
		        break;
	        case SSH_CMSG_EXEC_SHELL:
/*		        SSH_DLOG(4, ("Client requests shell.\n"));*/
		        if (ssh_sys_execcmd(context, "") != 0) 
		        {
		            SEND_DISCONNECT(context, "Command execution failed.");
		            ssh_sys_exit();
		            break;
		        }
		        return(0);
	        case SSH_CMSG_EXEC_CMD: 
	        {
/*Isiah.2002-07-10*/
#if 0
	            u_int8_t *cmd;
/*		        SSH_DLOG(4, ("Client requested command exec.\n"));*/
		        packet_get_binstr(pc, &cmd, NULL);
		        if (ssh_sys_execcmd(context, cmd) != 0) 
		        {
		            SEND_DISCONNECT(context, "Command execution failed.");
		            free(cmd);
		            ssh_sys_exit();
		            break;
		        }
		        free(cmd);
		        return(0);
#endif /* isiah. end of #if 0 */
/*Isiah.2002-07-10*/
/*		        SSH_DLOG(4, ("Client requested execute command.\n"));*/
		        SEND_DEBUG(context, "Execute command unimplemented.");
		        SEND_FAILURE(context);
		        break;
	        }
	        default:
/*		        SSH_DLOG(2, ("Unknown client request:%d\n", packet_type(pc)));*/
		        SEND_FAILURE(context);
		        break;
	    }

    }

/*    SSH_DLOG(2, ("disconnect during doPrepOps.\n"));*/
    return(1);
}


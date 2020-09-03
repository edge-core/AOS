/* $Id: sshd_iactive.c,v 1.30.2.1 2000/08/25 09:32:21 tls Exp $ */

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
#include <sys/types.h>
/*Isiah. 2002-05-27 
	Use phase2 socket, mask sys/socket.h and netinet/in.h,
	add "skt_vx.h" and "socket.h" */
/*#include <sys/socket.h>*/
#include "skt_vx.h"
#include "socket.h"

/*Isiah. 2002-05-27 .replace sys/time.h with time.h */
/*#include <sys/time.h>*/
/*#include <time.h>*/
#include <sys/wait.h>
#include <signal.h>
/*#include <unistd.h>*/
#include <stdlib.h>

#include "options.h"

#include "sshd.h"
#include "ssh_util.h"
#include "ssh_messages.h"

/* Isiah.2002-05-28 */
#include "sysfun.h"
#include "ssh_def.h"
#include "ssh_buffer.h"

extern int sprintf( char *buffer, const char *format, ... );

int Sshd_Pass_Username_And_Password(sshd_context_t *context);

/*static int child_exited;
static int child_exitstat;
static int child_exitpid;*/

/*
 * doInteractive: Interactive session state.
 *
 *	Args: s		socket to talk on.
 *
 *	Returns: 0	usually.
 *		 1	an error occurred.
 *			(this does not include a client disconnecting.)
 *
 * Note: when this function returns the connection
 *	will be completed and the socket closed.
 */
int doInteractive(sshd_context_t *context) 
{
    int                 error_flag;
    char                *error_why;
    int                 client_gone;
    fd_set              readfds;
    int                 maxfd, numfd;
    char                *readbuf;
    int                 readsize;
    int                 num;
    struct  ssh_packet  *pc;
    struct 	timeval     timeout;

    pc = &(context->in_pc);
    client_gone = 0;
    error_flag = 0;
    error_why = "No error.";

    /* Allocate a buffer to read into. */
    if ((readbuf = malloc(context->sock_bufsize)) == NULL) 
    {
	    context->sock_bufsize = SSH_MIN_READSIZE;
	    readbuf = malloc(SSH_MIN_READSIZE);
    }
    if (readbuf == NULL)
    {
        /*isiah.2003-01-27*/
        if (context->child_stdin > 0)
	        s_close(context->child_stdin);
        if (context->child_stdin != context->child_stdout)
    	    s_close(context->child_stdout);
        if (context->child_stdout != context->child_stderr)
	        s_close(context->child_stderr);
        context->child_stdin = -1;
        context->child_stdout = -1;
        context->child_stderr = -1;
	    return(1);
 	} 
   readsize = context->sock_bufsize;

    /* Adjust max size if compressing. (zlib isn't perfect) */
    if (context->compressing)
	    readsize = (readsize - 12) / 1.001;

/*Isiah.2002-06-05*/
    if ( Sshd_Pass_Username_And_Password(context) != 0 )
    {
        SEND_DISCONNECT(context, "Close connection by server");
        return (1);
    }

    while(1) 
    {
	    FD_ZERO(&readfds);
	    FD_SET(context->s, &readfds);
	    FD_SET(context->child_stdout, &readfds);
	    if (context->child_stdout != context->child_stderr)
	        FD_SET(context->child_stderr, &readfds);
	    maxfd = context->s|context->child_stdout|context->child_stderr;
	    numfd = select(maxfd + 1, &readfds, NULL, NULL, NULL);
	    if (numfd < 0) 
        {
	        if (numfd == EWOULDBLOCK)
		        continue;
/*	        SSH_ERROR("doInteractive: select failed: %s\n", strerror(errno));*/
	        error_why = "Select failed";
	        error_flag = 1;
	    }

/*Isiah.2002-06-11*/
        if ( numfd > 0 )
        {
            int ret_errno;
    	    /* If the sending packet hasn't finished yet, continue it. */
	        if (!packet_done(&(context->out_pc))) 
            {
	            if ( (ret_errno = xmit_packet(context, 0, NULL, 0)) < 0 ) 
                {
		            if (ret_errno != EPIPE) 
                    {
		                error_why = "doInteractive: xmit_data(continue) failed";
		                error_flag = 1;
		                client_gone = 1;
		            } 
                    else 
                    {
/*		                SSH_DLOG(3, ("doInteractive: client dropped connection\n"));*/
		                client_gone = 1;
		            }
		            break;
	            }
	        }

    	    /* If the previous packet is done, start the next one. */
	        if (packet_done(&(context->out_pc))) 
            {
	            if (FD_ISSET(context->child_stdout, &readfds)) 
                {
		            if ((num = read(context->child_stdout, readbuf, readsize)) < 0)
                    {
		                if (num != EWOULDBLOCK) 
                        {
/*			                SSH_ERROR("doInteractive: stdout read failed: %s\n",
					                		strerror(errno));*/
			                error_why = "stdout read error.";
			                error_flag = 1;
			                break;
		                }
                    }
		            if (num == 0) 
                    {
/*    		            SSH_DLOG(4, ("EOF on stdout.\n"));*/
	    	            break;
		            }
		            if ( (ret_errno = xmit_data(context, SSH_SMSG_STDOUT_DATA,
				            			readbuf, num, 0)) < 0) 
                    {
		                if (ret_errno != EPIPE) 
                        {
			                error_why = "doInteractive: xmit_data(stdout) failed";
			                error_flag = 1;
			                client_gone = 1;	/* Maybe not, but we can't */
						    /* talk to him anyway.     */
		                } 
                        else 
                        {
/*    			            SSH_DLOG(3, 
	    		                ("doInteractive: client dropped connection\n"));*/
		    	            client_gone = 1;
		                }
		                break;
		            }
	            } 
                else if (context->child_stdout != context->child_stderr) 
                {
    		        if (FD_ISSET(context->child_stderr, &readfds)) 
                    {
		                if ((num = read(context->child_stderr, readbuf, readsize)) < 0)
                        {
            	    		if (num != EWOULDBLOCK) 
                            {
/*			                    SSH_ERROR("doInteractive: stderr read failed: %s\n",
							                strerror(errno));*/
			                    error_why = "stderr read error.";
			                    error_flag = 1;
			                    break;
			                }
                        }
                        if (num == 0) 
                        {
/*    			            SSH_DLOG(4, ("EOF on stderr\n"));*/
	    		            break;
		                }
		                if ( (ret_errno = xmit_data(context, SSH_SMSG_STDERR_DATA,
        		    					readbuf, num, 0)) < 0) 
                        {
			                if (ret_errno != EPIPE) 
                            {
			                    error_why =
				                "doInteractive: xmit_data(stderr) failed";
			                    error_flag = 1;
			                    client_gone = 1;
			                } 
                            else 
                            {
/*    			                SSH_DLOG(5,
	    			            ("doInteractive: client dropped connection\n"));*/
		    	                client_gone = 1;
			                }
			                break;
		                }
		            }
	            }
	        }

    	    if (!FD_ISSET(context->s, &readfds))
	            continue;

	        if ((num = ssh_read_packet(context, pc, 0)) < 0) 
            {
    	        if (num == EPIPE) 
                {
/*		            SSH_DLOG(5, ("doInteractive: client dropped connection.\n"));*/
		            client_gone = 1;
	            } 
                else 
                {
/*    		        SSH_DLOG(1, ("doInteractive: read_packet failed: %s\n",
	    		        				strerror(errno)));*/
		            error_why = "Unable to read packet";
		            error_flag = 1;
	            }
	            break;		/* error. */
	        }
	        if (num == 0)
    	        continue;		/* not enought data yet. */

	        while ((num = process_packet(context, pc)) > 0) 
            {
    	        switch(packet_type(pc)) 
                {
	                case SSH_MSG_NONE:
		                break;
	                case SSH_MSG_PORT_OPEN:
	                case SSH_MSG_CHANNEL_OPEN_CONFIRMATION:
	                case SSH_MSG_CHANNEL_OPEN_FAILURE:
	                case SSH_MSG_CHANNEL_DATA:
	                case SSH_MSG_CHANNEL_CLOSE:
	                case SSH_MSG_CHANNEL_CLOSE_CONFIRMATION:
		                /* unimplemented. */
    		            SEND_DEBUG(context, "Unimplemented.");
	    	            SEND_FAILURE(context);
		                break;
	                case SSH_CMSG_STDIN_DATA:
		            {
	                    u_int8_t *indata;
	                    size_t dlen;
		                packet_get_binstr(pc, &indata, &dlen);
		                if (write(context->child_stdin, indata, dlen) != dlen)
    			            SEND_FAILURE(context);
	    	            free(indata);
		                break;
	                }
	                case SSH_CMSG_EOF:
		            /* All done with input. */
		            /* Closing stdin should cause the program to exit. */
/*    		            SSH_DLOG(4, ("Client sent EOF, closing child_stdin.\n"));*/

            		/* If stdin == stdout we can't just close b/c */
		            /* we will still try to read until an EOF.    */
                		if (context->child_stdin == context->child_stdout)
            	    	    shutdown(context->child_stdin, SHUT_WR);
            		    else
    						/*isiah.2002-05-29 */
	    					/* replace close() with s_close()*/
                		    s_close(context->child_stdin);
		                context->child_stdin = -1;
		                break;
	                case SSH_CMSG_WINDOW_SIZE:
	                /*isiah.2002-07-18*/
                    {
                        u_int32_t y_row, x_col, x_pix, y_pix;
                        
                        packet_get_int32(pc, &y_row);
                        packet_get_int32(pc, &x_col);
                        packet_get_int32(pc, &x_pix);
                        packet_get_int32(pc, &y_pix);
/*	            	    SEND_FAILURE(context);*/
		                break;
}
	                case SSH_MSG_DISCONNECT:
		                client_gone = 1;
		                break;
	                default:
/*		                SSH_DLOG(2, ("Client sent unknown packet type: %d\n",
				                	packet_type(pc)));*/
		                error_why = "Unkown packet type";
		                error_flag = 1;
		                break;

	            }	/* end switch */

	            if (error_flag)
    		        break;
	            if (client_gone)
		            break;
	        } /* end while(process_packet) */
	        if (num < 0) 
            {
/*	            SSH_DLOG(2, ("doInteractive: process_packet failed: %s\n",
			    				    strerror(errno)));*/
	            error_why = "Unable to process packet.";
	            error_flag = 1;
	            break;
	        }
	        if (client_gone || error_flag)
	            break;
	    }/*isiah. end of if(numfd>0) */
    } /* while(1): select loop. */

/*Isiah.2002-07-10*/
    free(readbuf);
    
    /* Check for any errors from above. */
    if (error_flag) 
    {
/*	    SSH_DLOG(5, ("doInteractive: error_flag set.\n"));*/
	    if (!client_gone) 
	    {
	        char tmpbuf[200];
	        sprintf(tmpbuf, "ssh server: %s", error_why);
	        SEND_DISCONNECT(context, tmpbuf);
	        client_gone = 1;
	    }
    }

    /* Close connection to child; should cause it to exit, */
    /* if it hasn't already. */
    if (context->child_stdin > 0)
	    s_close(context->child_stdin);
    if (context->child_stdin != context->child_stdout)
	    s_close(context->child_stdout);
    if (context->child_stdout != context->child_stderr)
	    s_close(context->child_stderr);
    context->child_stdin = -1;
    context->child_stdout = -1;
    context->child_stderr = -1;

    /* -- child should have exited by now, one way or another. */
    /* -- if the client is still around we can notify him of the exit */
    /* -- We can only get here w/o the exit status if an error happens. */
    /* -- in which case the client will have been send a disconnect. */

    if (!client_gone) 
    {
	/* Client is still around.  Figure out what to notify him of. */
	/* We should only end up here if the child has exited. */
        u_int32_t retval;

   	    /* Send the exit status. */
        retval = 0;
   	    xmit_int32(context, SSH_SMSG_EXITSTATUS, retval, PKT_WAITALL);

        /* Wait for EXIT_CONFIRMATION.  There might be other packets */
        /* so just keep reading until read_packet returns an error.   */
   	    while (packet_type(pc) != SSH_CMSG_EXIT_CONFIRMATION) 
        {
            /*isiah.2002-11-12*/
            FD_ZERO(&readfds);
            FD_SET(context->s, &readfds);
            timeout.tv_sec = 1;     
            timeout.tv_usec = 0;    
            if ( select(context->s + 1, &readfds, NULL, NULL, &timeout) <= 0 )
            {
                break;
            }
            
            do 
            {
	            if ((num = ssh_read_packet(context, pc, PKT_WAITALL)) > 0)
		            num = process_packet(context, pc);
	        } while (num == 0);
	        if (num > 0) 
            {
	            if (packet_type(pc) != SSH_CMSG_EXIT_CONFIRMATION) 
                {
//		            SSH_DLOG(1, ("Weird exit confirmation: %d\n",
//				            packet_type(pc)));
	            } 
                else 
                {
//	                SSH_DLOG(3, ("Received EXIT_CONFIRMATION\n"));
		            break;
	            }
	        } 
            else 
            {
	            if (num < 0) 
                {
/*		            SSH_DLOG(1,
		            ("doInteractive failed to get exit confirm: %s\n",
					            	strerror(errno)));*/
		            break;
	            } 
                else                    
                {
//			            SSH_DLOG(3, ("doInteractive: partial exit confirm:%d\n",
//					            				num));
	            }
	        }
        }
    }

    /* Close socket to client. */
    s_close(context->s);
    context->s = -1;

    return(0);
}



/*Isiah.2002-06-05*/
int Sshd_Pass_Username_And_Password(sshd_context_t *context)
{
    fd_set              readfds;
    int                 maxfd, numfd;
    char                *readbuf;
    int                 readsize;
    struct 	timeval     timeout;
    int                 username_done = 0, password_done = 0;
    int                 i;
    int                 len;
    u_int8_t            indata;
    u_int8_t            username[SYS_ADPT_MAX_USER_NAME_LEN+1];
    int                 dlen = 1;
        
    /* Allocate a buffer to read into. */
    if ((readbuf = malloc(context->sock_bufsize)) == NULL) 
    {
	    context->sock_bufsize = SSH_MIN_READSIZE;
	    readbuf = malloc(SSH_MIN_READSIZE);
    }

    if (readbuf == NULL)
    {
        /*isiah.2003-01-27*/
        if (context->child_stdin > 0)
	        s_close(context->child_stdin);
        if (context->child_stdin != context->child_stdout)
    	    s_close(context->child_stdout);
        if (context->child_stdout != context->child_stderr)
	        s_close(context->child_stderr);
        context->child_stdin = -1;
        context->child_stdout = -1;
        context->child_stderr = -1;
	    return(1);
	} 

    readsize = context->sock_bufsize;

    while(1) 
    {
	    FD_ZERO(&readfds);
	    FD_SET(context->child_stdout, &readfds);
	    if (context->child_stdout != context->child_stderr)
	    {
	        FD_SET(context->child_stderr, &readfds);
	    }
	    maxfd = context->s|context->child_stdout|context->child_stderr;
    	timeout.tv_sec = 0;     /*  no.  of seconds  */
        timeout.tv_usec = 100;    /*  no. of micro seconds  */
	    numfd = select(maxfd + 1, &readfds, NULL, NULL, &timeout);
	    if (numfd < 0) 
        {
	        if (numfd == EWOULDBLOCK)
		        continue;
            /*isiah.2003-01-29*/
            if (context->child_stdin > 0)
                s_close(context->child_stdin);
            if (context->child_stdin != context->child_stdout)
           	    s_close(context->child_stdout);
            if (context->child_stdout != context->child_stderr)
                s_close(context->child_stderr);
            context->child_stdin = -1;
            context->child_stdout = -1;
            context->child_stderr = -1;
            return (1);
	    }
        if (numfd == 0)
        {
            if ( username_done == 0 )
            {
                len = strlen(context->username);
                memset(username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
                strncpy(username,context->username, SYS_ADPT_MAX_USER_NAME_LEN);
                for ( i=0 ; i<len ; i++ )
                {
                    indata = username[i];
                    if ( write(context->child_stdin, &indata, dlen) < 0 )
                    {
                        free(readbuf);
                        if (context->child_stdin > 0)
	                        s_close(context->child_stdin);
                        if (context->child_stdin != context->child_stdout)
                    	    s_close(context->child_stdout);
                        if (context->child_stdout != context->child_stderr)
                    	    s_close(context->child_stderr);
                        context->child_stdin = -1;
                        context->child_stdout = -1;
                        context->child_stderr = -1;
                        return (1);
                    }
                    if ( read(context->child_stdout, readbuf, readsize) < 0 )
                    {
                        free(readbuf);
                        if (context->child_stdin > 0)
	                        s_close(context->child_stdin);
                        if (context->child_stdin != context->child_stdout)
                    	    s_close(context->child_stdout);
                        if (context->child_stdout != context->child_stderr)
                    	    s_close(context->child_stderr);
                        context->child_stdin = -1;
                        context->child_stdout = -1;
                        context->child_stderr = -1;
                        return (1);
                    }
                }
                indata = 0x0d;
                if ( write(context->child_stdin, &indata, dlen) < 0 )
                {
                    free(readbuf);
                    if (context->child_stdin > 0)
                        s_close(context->child_stdin);
                    if (context->child_stdin != context->child_stdout)
                   	    s_close(context->child_stdout);
                    if (context->child_stdout != context->child_stderr)
                   	    s_close(context->child_stderr);
                    context->child_stdin = -1;
                    context->child_stdout = -1;
                    context->child_stderr = -1;
                    return (1);
                }
                username_done = 1;
                continue;
            }

            if ( password_done == 0 )
            {
                len = strlen(context->password);
                for ( i=0 ; i<len ; i++ )
                {
                    indata = context->password[i];
                    if ( write(context->child_stdin, &indata, dlen) < 0 )
                    {
                        free(readbuf);
                        if (context->child_stdin > 0)
	                        s_close(context->child_stdin);
                        if (context->child_stdin != context->child_stdout)
                    	    s_close(context->child_stdout);
                        if (context->child_stdout != context->child_stderr)
                    	    s_close(context->child_stderr);
                        context->child_stdin = -1;
                        context->child_stdout = -1;
                        context->child_stderr = -1;
                        return (1);
                    }
                }
                indata = 0x0d;
                if ( write(context->child_stdin, &indata, dlen) < 0 )
                {
                    free(readbuf);
                    if (context->child_stdin > 0)
                        s_close(context->child_stdin);
                    if (context->child_stdin != context->child_stdout)
                   	    s_close(context->child_stdout);
                    if (context->child_stdout != context->child_stderr)
                   	    s_close(context->child_stderr);
                    context->child_stdin = -1;
                    context->child_stdout = -1;
                    context->child_stderr = -1;
                    return (1);
                }
                password_done = 1;
    		    break;
            }
        }

        if (FD_ISSET(context->child_stdout, &readfds)) 
        {
	        if ( read(context->child_stdout, readbuf, readsize) <= 0 )
	        {
                free(readbuf);
                if (context->child_stdin > 0)
                    s_close(context->child_stdin);
                if (context->child_stdin != context->child_stdout)
               	    s_close(context->child_stdout);
                if (context->child_stdout != context->child_stderr)
               	    s_close(context->child_stderr);
                context->child_stdin = -1;
                context->child_stdout = -1;
                context->child_stderr = -1;
                return (1);
	        }
	    } 
    } /* while(1): select loop. */
    free(readbuf);
    return (0);
}



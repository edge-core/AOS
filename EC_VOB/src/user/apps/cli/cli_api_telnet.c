#include <stdint.h>
#include "cli_api.h"
#include "cli_msg.h"
#include "cli_task.h"
#include "l_inet.h"
#include <stdio.h>
#include "telnet.h"
#include "telnet_pmgr.h"
#include "cli_api_system.h"
#include "netcfg_pom_ip.h"
#include "l_math.h"
#include "sys_bld.h"

#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>    /* for SOL_TCP */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/*
 * State for recv fsm
 */
#define TS_DATA  0    /* base state */
#define TS_IAC   1    /* look for double IAC's */
#define TS_CR    2    /* CR-LF ->'s CR */
#define TS_SB    3    /* throw away begin's... */
#define TS_SE    4    /* ...end's (suboption negotiation) */
#define TS_WILL  5    /* will option negotiation */
#define TS_WONT  6    /* wont " */
#define TS_DO    7    /* do " */
#define TS_DONT  8    /* dont " */

#define OPT_NO                  0        /* won't do this option */
#define OPT_YES                 1        /* will do this option */
#define OPT_YES_BUT_ALWAYS_LOOK 2
#define OPT_NO_BUT_ALWAYS_LOOK  3

/* Telnet client will auto disconnect in max time + (intvl * probes) seconds
 */
#define TELNET_CLIENT_KEEP_ALIVE_TIME       30
#define TELNET_CLIENT_KEEP_ALIVE_INTVL      5
#define TELNET_CLIENT_KEEP_ALIVE_PROBES     6

#define TELNET_CLIENT_ERROR_EVENT_ARRIVAL BIT_0
#define TELNET_SHELL_ERROR_EVENT_ARRIVAL BIT_0
#define TELNET_CLIENT_DEBUG 0
#define TELNET_CLIENT_PRINT(desc)                 \
    {                                                        \
        if (TELNET_CLIENT_DEBUG)                             \
            printf("%s(%d):%s\r\n", __FILE__, __LINE__, desc); \
    }

static char doopt[] = { IAC, DO, '%', 'c', 0 };
static char dont[] = { IAC, DONT, '%', 'c', 0 };
static char will[] = { IAC, WILL, '%', 'c', 0 };
static char wont[] = { IAC, WONT, '%', 'c', 0 };

void telnet_session_close(int fd)
{
	close(fd);
}

BOOL_T telnet_client_event_check(int fd)
{
    UI32_T receive_event, local_event = 0;

    if (SYSFUN_ReceiveEvent(TELNET_CLIENT_ERROR_EVENT_ARRIVAL, SYSFUN_EVENT_WAIT_ANY,
							SYSFUN_TIMEOUT_NOWAIT, &receive_event) == SYSFUN_OK)
    {
		local_event |= receive_event;
		if (local_event & TELNET_CLIENT_ERROR_EVENT_ARRIVAL)
		{
			telnet_session_close(fd);
			local_event ^= TELNET_CLIENT_ERROR_EVENT_ARRIVAL;
			return TRUE;
		}
    }

	return FALSE;
}

static void tnc_task(void *args[])
{
	CLI_TASK_WorkingArea_T *ctrl_P = (CLI_TASK_WorkingArea_T *)args[0];
	int remote_socket = (uintptr_t)args[1];
	int    rc, len;
	fd_set ibits;
	struct timeval timeout;
    UI32_T receive_event, local_event = 0;
	char   buf[CLI_DEF_OUTPUT_BUFFER];

	while (1)
	{
	    if (SYSFUN_ReceiveEvent(TELNET_SHELL_ERROR_EVENT_ARRIVAL, SYSFUN_EVENT_WAIT_ANY,
								SYSFUN_TIMEOUT_NOWAIT, &receive_event) == SYSFUN_OK)
	    {
			local_event |= receive_event;
			if (local_event & TELNET_SHELL_ERROR_EVENT_ARRIVAL)
			{
				local_event ^= TELNET_SHELL_ERROR_EVENT_ARRIVAL;
				return;
			}
	    }

		FD_ZERO(&ibits);
		FD_SET(remote_socket, &ibits);
		timeout.tv_sec  = 0;
		timeout.tv_usec = 50;

		rc = select(remote_socket + 1, (fd_set *)&ibits, NULL, NULL, &timeout);
		if (rc < 0)
		{
			TELNET_CLIENT_PRINT("tnc_task select failed");
    		SYSFUN_SendEvent(ctrl_P->cli_tid, TELNET_CLIENT_ERROR_EVENT_ARRIVAL);
			return;
		}

		if (rc > 0)
		{
			if (FD_ISSET(remote_socket, &ibits))
			{
		    	rc = recv(remote_socket, buf, sizeof(buf), 0);
		 	}

			if (rc <= 0) /*error; maybe remote telnet server closed connection */
			{
	    		SYSFUN_SendEvent(ctrl_P->cli_tid, TELNET_CLIENT_ERROR_EVENT_ARRIVAL);
				return;
			}
			else
			{
				buf[rc] = 0;
				len = 0;
				while (len < rc)
				{
					strcpy((char *)ctrl_P->send_buf, buf + len);
					CLI_IO_PrintOut(ctrl_P);
					len += strlen(buf + len) + 1;
				}
			}
		}
	}
}

static BOOL_T options_negotiation(CLI_TASK_WorkingArea_T *ctrl_P, int fd, char *myopts, char *hisopts)
{
	char local_myopts[256], local_hisopts[256], ibuf[64], obuf[64], *in, *out, *fmt;
	char sub_buf[16], *sub;
	char sub_opts[] = { IAC, SB, TELOPT_TTYPE, TELQUAL_IS, 'A', 'N', 'S', 'I', IAC, SE};
	int state, len, rc, c;
#if 0
    struct  sockaddr    sock_addr;
    struct  sockaddr_in *sock_in;
    size_t     sock_addr_size = sizeof(sock_addr);
	UI16_T port;
#endif
	fd_set ibits;
	struct timeval timeout;

	/* Sync current telnet options */
	out = obuf;
	if (ctrl_P->sess_type == CLI_TYPE_TELNET)
	{
#if 0
        if (getsockname(ctrl_P->socket, &sock_addr, &sock_addr_size) == 0)
    	{
	        sock_in = (struct sockaddr_in*) &sock_addr;
	        port = L_STDLIB_Ntoh16(sock_in->sin_port);

		    if (!TELNET_PMGR_CurrentTelnetSession(port, local_myopts, local_hisopts))
	    	{
				return FALSE;
	    	}
    	}
		else
		{
			return FALSE;
		}

		if (local_myopts[TELOPT_SGA] == OPT_YES)
		{
			hisopts[TELOPT_SGA] = OPT_YES;
			sprintf(out, doopt, TELOPT_SGA);
			out += (sizeof(doopt) - 2);
		}

		if (local_myopts[TELOPT_ECHO] == OPT_YES)
		{
			hisopts[TELOPT_ECHO] = OPT_YES;
			sprintf(out, doopt, TELOPT_ECHO);
			out += (sizeof(doopt) - 2);
		}
#endif
	}
	else if (ctrl_P->sess_type == CLI_TYPE_UART)
	{
		/* Do sga option, Do echo option */
		hisopts[TELOPT_SGA] = OPT_YES;
		sprintf(out, doopt, TELOPT_SGA);
		out += (sizeof(doopt) - 2);
		hisopts[TELOPT_ECHO] = OPT_YES;
		sprintf(out, doopt, TELOPT_ECHO);
		out += (sizeof(doopt) - 2);
		memcpy(local_myopts, hisopts, sizeof(local_myopts));
		memcpy(local_hisopts, myopts, sizeof(local_hisopts));
	}

	len = out - obuf;
	if (len > 0)
		len = send(fd, obuf, len, 0);
	if (len < 0)
		return FALSE;

	while (1)
	{
		FD_ZERO(&ibits);
		FD_SET(fd, &ibits);
		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;

		rc = select(fd + 1, (fd_set *)&ibits, NULL, NULL, &timeout);
		if (rc < 0)
		{
			return FALSE;
		}

		if (rc == 0) /* negotiation finished */
		{
			return TRUE;
		}

		if (rc > 0)
		{
			if (FD_ISSET(fd, &ibits))
			{
				in = ibuf;
		    	rc = recv(fd, in, sizeof(ibuf), 0);
				if (rc <= 0)
					return FALSE;

				state = TS_DATA;
				out = obuf;
				sub = sub_buf;
				while (rc > 0)
				{
					c = *in++ & 0xff;
					rc--;

					switch (state)
					{
						case TS_DATA:
			            	if (c == IAC)
				                state = TS_IAC;
							else
							{
								memcpy(ctrl_P->send_buf, in - 1, rc + 1);
								ctrl_P->send_buf[rc + 1] = 0;
								CLI_IO_PrintOut(ctrl_P);
								rc = 0;
							}
							break;

						case TS_IAC:
							switch (c)
							{
					            case SB:
					                state = TS_SB;
					                continue;

					            case SE:
					                state = TS_DATA;
					                continue;

					            case WILL:
					                state = TS_WILL;
					                continue;

					            case WONT:
					                state = TS_WONT;
					                continue;

					            case DO:
					                state = TS_DO;
					                continue;

					            case DONT:
					                state = TS_DONT;
					                continue;

					            default: /* not negotiated command */
					                rc = 0;
					                continue;

							}
							break;

				        case TS_SB:
				            if (c == IAC)
				                state = TS_SE;
				            else
				                *sub++ = c;;

				            break;

				        case TS_SE:
				            if (c == SE)
							{
								/* handle sub-option */
				                if ((sub - sub_buf == 2) && (*sub == TELOPT_TTYPE)
									&& (*(sub + 1) == TELQUAL_SEND))
			                	{
			                		memcpy(out, sub_opts, sizeof(sub_opts));
								    out += sizeof(sub_opts);
									sub = sub_buf;
			                	}
				                state = TS_DATA;
				            }
				            break;

				        case TS_WILL:
				            if (hisopts[c] != OPT_YES)
			            	{
								switch (c)
								{
								    case TELOPT_ECHO:
										if (local_myopts[c] == OPT_YES)
										{
									        fmt = doopt;
											hisopts[TELOPT_ECHO] = OPT_YES;
										}
										else
										{
									        fmt = dont;
										}
								        break;

								    case TELOPT_SGA:
										if (local_myopts[c] == OPT_YES)
										{
											hisopts[TELOPT_SGA] = OPT_YES;
									        fmt = doopt;
										}
										else
										{
									        fmt = dont;
										}
								        break;

								    default:
								        fmt = dont;
								        break;
								}
							    sprintf(out, fmt, c);
							    out += strlen(out);
			            	}
				            state = TS_DATA;
				            continue;

				        case TS_WONT:
				            if (hisopts[c] != OPT_NO)
			            	{
							    fmt = dont;
							    hisopts[c] = (unsigned char)OPT_NO;
							    sprintf(out, fmt, c);
							    out += strlen(out);
			            	}
				            state = TS_DATA;
				            continue;

				        case TS_DO:
				            if (myopts[c] != OPT_YES)
			            	{
								switch (c)
								{
								    case TELOPT_TTYPE:
										if (local_hisopts[c] == OPT_YES)
										{
									        fmt = will;
											myopts[TELOPT_TTYPE] = OPT_YES;
										}
										else
										{
											fmt = wont;
										}
								        break;

								    case TELOPT_SGA:
										if (local_hisopts[c] == OPT_YES)
										{
											myopts[TELOPT_SGA] = OPT_YES;
									        fmt = will;
										}
										else
										{
											fmt = wont;
										}
								        break;

								    default:
								        fmt = wont;
								        break;
								}

							    sprintf(out, fmt, c);
							    out += strlen(out);
			            	}
				            state = TS_DATA;
				            continue;

				        case TS_DONT:
				            if (myopts[c] != OPT_NO)
							{
							    fmt = wont;
							    myopts[c] = (unsigned char)OPT_NO;
							    sprintf(out, fmt, c);
							    out += strlen(out);
				            }
				            state = TS_DATA;
				            continue;

				        default:
							rc = 0;
							break;
					}
				}

				len = out - obuf;
				if (len > 0)
					len = send(fd, obuf, len, 0);
				if (len < 0)
					return FALSE;
		 	}
		}
	}

	return TRUE;
}

/* FUNCTION NAME : set_keep_alive_option
 * PURPOSE : For telnet sessions, use TCP keep-alive mechanism to detect client is alive
 *           or not.
 *           For more detail about the following definition, please see:
 *           http://tldp.org/HOWTO/html_single/TCP-Keepalive-HOWTO/
 * INPUT   : sockfd   -- socket ID
 *           enable     -- TRUE / FALSE
 *           time       -- keep alive time in seconds
 *           intvl      -- keep alive intvl time in seconds
 *           probes     -- keep alive probes count
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE -- success or failure
 * NOTE    : For example
 *           # cat /proc/sys/net/ipv4/tcp_keepalive_time
 *           7200
 *           # cat /proc/sys/net/ipv4/tcp_keepalive_intvl
 *           75
 *           # cat /proc/sys/net/ipv4/tcp_keepalive_probes
 *           9
 *           The first two parameters are expressed in seconds,
 *           and the last is the pure number. This means that the keepalive
 *           routines wait for two hours (7200 secs) before sending the first
 *           keepalive probe, and then resend it every 75 seconds. If no ACK
 *           response is received for nine consecutive times, the connection is
 *           marked as broken.
 */
static BOOL_T telnet_client_set_keep_alive_option(
    int sockfd,
    BOOL_T enabled,
    int time,
    int intvl,
    int probes)
{
    int ret;
    int value;

    socklen_t optlen = sizeof(value);

    if (TRUE == enabled)
    {
        value = 1;

        ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));
        if (ret < 0)
        {
            TELNET_CLIENT_PRINT("Enable keep alive failed");
            return FALSE;
        }

        value = time;
        ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, &value, sizeof(value));

        if (ret < 0)
        {
            TELNET_CLIENT_PRINT("Set keep time failed");
            return FALSE;
        }

        optlen = sizeof(value);
        ret = getsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, &value, &optlen);

        value = intvl;
        ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, &value, sizeof(value));
        if (ret < 0)
        {
            TELNET_CLIENT_PRINT("Set keep intvl failed");
            return FALSE;
        }

        optlen = sizeof(value);
        ret = getsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, &value, &optlen);

        value = probes;
        ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, &value, sizeof(value));

        if (ret < 0)
        {
            TELNET_CLIENT_PRINT("Set keep probes failed");
            return FALSE;
        }

        optlen = sizeof(value);
        ret = getsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, &value, &optlen);

    }
    else
    {
        value = 0;

        ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));
        if (ret < 0)
        {
            TELNET_CLIENT_PRINT("Enable keep alive failed");
            return FALSE;
        }
    }

    return TRUE;
}


static BOOL_T
telnet_client_session(
    UI32_T ip_addr,
    UI16_T port,
    CLI_TASK_WorkingArea_T *ctrl_P)
{
#define CLI_TELNET_MAX_CONNECT_SECOND           15
#define CLI_TELNET_MAX_WAIT_TNC_TASK_SECOND     30
    UI32_T tnc_task_id;
    UI32_T expire_tick;
    struct sockaddr_in sin;
    int fd, f_status;
    int non_blocking , blocking;
    void *argv[4];
    char  tnc_task_name[SYSFUN_TASK_NAME_LENGTH];
    UI16_T  c;
    char    ch, send_buf[4];
    char myopts[256], hisopts[256];

    {
        NETCFG_TYPE_InetRifConfig_T rif_config;
        UI32_T vid, ifindex;

        VLAN_POM_GetManagementVlan(&vid);
        VLAN_OM_ConvertToIfindex(vid, &ifindex);

        memset(&rif_config, 0, sizeof(rif_config));
        rif_config.ifindex = ifindex;

        if (NETCFG_POM_IP_GetNextInetRifOfInterface(&rif_config) == NETCFG_TYPE_OK)
        {
            if (memcmp(&ip_addr, rif_config.addr.addr, SYS_ADPT_IPV4_ADDR_LEN) == 0)
            {
                TELNET_CLIENT_PRINT("DUT should not telnet to itself.");
                return FALSE;
            }
        }
    }

    if( ip_addr == 0 || L_STDLIB_Ntoh32(ip_addr) == INADDR_LOOPBACK)
        return FALSE;

    if ((fd = socket(PF_INET , SOCK_STREAM, 0)) < 0)
    {
        TELNET_CLIENT_PRINT("failed to create socket");
        return FALSE;
    }

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip_addr;
    sin.sin_port = L_STDLIB_Hton16(port);

    /* Enable non-blocking flag. Every 1 second, check user whether
     * press CTRL_K during connecting to telnet server.
     * If user press CTRL_K, stop the connect action immediately.
     */
    non_blocking  = 1;
    blocking = 0;

    ioctl(fd, FIONBIO, (char *)&non_blocking);
    expire_tick = SYS_TIME_GetSystemTicksBy10ms() +
                    (SYS_BLD_TICKS_PER_SECOND * CLI_TELNET_MAX_CONNECT_SECOND);

    if (connect(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        fd_set  rset;
        struct timeval tval;
        int sel_ret;

        while (1)
        {
            FD_ZERO (&rset);
            FD_SET (fd, &rset);
            tval.tv_sec  = 1;
            tval.tv_usec = 0;

            sel_ret = select(fd+1, &rset, NULL, NULL, &tval);

            /* select error
             */
            if (sel_ret < 0)
            {
                TELNET_CLIENT_PRINT("connect error");
                close(fd);
                return FALSE;
            }

            ch = (ctrl_P->sess_type == CLI_TYPE_UART) ?
                    CLI_IO_ReadACharFromConsole(ctrl_P) :
                    CLI_IO_ReadACharFromTelnet(ctrl_P);

            /* User type "ctrl+k" to exit telnet client session
             */
            if ((ch == KEYIN_ERROR) || (ch == CTRL_K))
            {
                close(fd);
                return TRUE;
            }

            if (FD_ISSET(fd, &rset))
            {
                int error;
                socklen_t len;
                int ret;

                error = 0;
                len = sizeof(error);
                ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);

                if (ret < 0 || error)
                {
                    /* getsockopt failed or have error on remote_socket
                     */
        	       TELNET_CLIENT_PRINT("connect timeout");
                       close(fd);
                       return FALSE;
                }

                /* connect successfully
                 */
                ioctl(fd, FIONBIO, (char *)&blocking);
                break;
            }

            if (L_MATH_TimeOut32(SYS_TIME_GetSystemTicksBy10ms(), expire_tick))
            {
                TELNET_CLIENT_PRINT("connect timeout");
                close(fd);
                return FALSE;
            }
        }
    }

    ctrl_P->telnet_client.socket = fd;

    telnet_client_set_keep_alive_option(fd, TRUE, TELNET_CLIENT_KEEP_ALIVE_TIME,
                                                  TELNET_CLIENT_KEEP_ALIVE_INTVL,
                                                  TELNET_CLIENT_KEEP_ALIVE_PROBES);

    f_status = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, f_status | O_NONBLOCK);

    memset(myopts, 0, sizeof(myopts));
    memset(hisopts, 0, sizeof(hisopts));
    /* Start telent option negotiation */
    if (!options_negotiation(ctrl_P, fd, myopts, hisopts))
    {
        TELNET_CLIENT_PRINT("Negotiation failed.");
        telnet_session_close(fd);
        return FALSE;
    }

	argv[0] = (void *)ctrl_P;
	argv[1] = (void *)(uintptr_t)fd;
    /* start the telnet client task */
    if(SYSFUN_SpawnThread(SYS_BLD_TELNET_CLIENT_THREAD_PRIORITY,
                          SYS_BLD_TELNET_THREAD_SCHED_POLICY,
                          SYS_BLD_TELNET_CLIENT_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          tnc_task,
                          argv,
                          &tnc_task_id) != SYSFUN_OK)
    {
        TELNET_CLIENT_PRINT("SYSFUN_SpawnThread failed.");
        telnet_session_close(fd);
        return FALSE;
    }

    while(1)
    {
        /* Wait data come from user
         */
        c = CLI_IO_GetKey(&ch);

        /* User type "ctrl+k" to exit telnet client session
         */
        if ((c == KEYIN_ERROR) || (c == CTRL_K)) 
        {
            break;
        }

        switch(c)
        {
        case UP:
            send_buf[0] = 0x1b;
            send_buf[1] = ch;
            send_buf[2] =0x41;
            send_buf[3] =0;
            break;
        case DOWN:
            send_buf[0] = 0x1b;
            send_buf[1] = ch;
            send_buf[2] =0x42;
            send_buf[3] =0;
            break;
        case LEFT:
            send_buf[0] = 0x1b;
            send_buf[1] = ch;
            send_buf[2] =0x44;
            send_buf[3] =0;
            break;
        case RIGHT:
            send_buf[0] = 0x1b;
            send_buf[1] = ch;
            send_buf[2] =0x43;
            send_buf[3] =0;
            break;
        default:
            send_buf[0] = 0;
            break;
        }

        if(send_buf[0] == 0x1b)
        {
            if (send(fd, send_buf, strlen(send_buf), 0) < 0)
            {
                TELNET_CLIENT_PRINT("Failed to send data to remote telnet server.");
                telnet_session_close(fd);
                break;
            }
        }
        else
        {
            if (send(fd, &ch, 1, 0) < 0)
            {
                TELNET_CLIENT_PRINT("Failed to send data to remote telnet server.");
                telnet_session_close(fd);
                break;
            }
        }
    }

    /* Make sure tnc task is alive before send event to it.
     * 1st and 2nd check are make sure the tnc task is not in exiting.
     * After send event, wait max 30 second due to event lost or some
     * thing is wrong. The CLI task will not be locked.
     */
    if (SYSFUN_TaskIDToName(tnc_task_id, tnc_task_name, sizeof(tnc_task_name)) == 
        SYSFUN_OK)
    {
        SYSFUN_Sleep(50);

        if (SYSFUN_TaskIDToName(tnc_task_id, tnc_task_name, sizeof(tnc_task_name)) == 
            SYSFUN_OK)
        {
            SYSFUN_SendEvent(tnc_task_id, TELNET_SHELL_ERROR_EVENT_ARRIVAL);
        }

        expire_tick = SYS_TIME_GetSystemTicksBy10ms() +
               (SYS_BLD_TICKS_PER_SECOND * CLI_TELNET_MAX_WAIT_TNC_TASK_SECOND);

        while (SYSFUN_TaskIDToName(tnc_task_id, tnc_task_name, sizeof(tnc_task_name)) == 
               SYSFUN_OK &&
               L_MATH_TimeOut32(SYS_TIME_GetSystemTicksBy10ms(), expire_tick))
        {
            SYSFUN_Sleep(20);
        }
    }

    return TRUE;
#undef CLI_TELNET_MAX_CONNECT_SECOND
#undef CLI_TELNET_MAX_WAIT_TNC_TASK_SECOND
}

UI32_T CLI_API_Telnet_Client(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
	UI32_T  ip_addr;
	UI16_T  port = 23;
	char string[128];

	if(arg[0] != NULL)
	{
	    sprintf(string, "Connect To %s...\r\n", arg[0]);
		CLI_LIB_PrintStr(string);
    	if (CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr) == 0)
        	if (HostString2IP((UI8_T*)arg[0], &ip_addr) != 0) /* Support DNS */
    		{
    			sprintf(string, "Failed to connect %s[unknown host]\r\n", arg[0]);
                CLI_LIB_PrintStr(string);
				return CLI_NO_ERROR;
    		}
	}
	else
	    return CLI_ERR_INTERNAL;

	if(arg[1] != NULL)
	{
	    port = atoi(arg[1]);
	}

    if(!telnet_client_session(ip_addr, port, ctrl_P))
    {
    	sprintf(string, "Could not open a connection to host on port %u : Connect failed\r\n", port);
	    CLI_LIB_PrintStr(string);
    }
	else /* Exit telnet client session */
	{
    	sprintf(string, "\r\n\r\nConnection to host lost\r\n\r\n");
	    CLI_LIB_PrintStr(string);
	}

	return CLI_NO_ERROR;
}


/*
 * snmptrapd.c - receive and log snmp traps
 *
 */
/*****************************************************************
	Copyright 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#else
#include <sys/socket.h>
#endif
#if HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <stdio.h>
#if HAVE_SYS_TIME_H
# include <sys/time.h>
# if TIME_WITH_SYS_TIME
#  include <time.h>
# endif
#else
# include <time.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYSLOG_H
#include <syslog.h>
#endif
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#if HAVE_NET_IF_H
#include <net/if.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_PROCESS_H              /* Win32-getpid */
#include <process.h>
#endif
#include <signal.h>
#include <errno.h>

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "snmptrapd_handlers.h"
#include "snmptrapd_log.h"
#include "notification_log.h"

#if USE_LIBWRAP
#include <tcpd.h>

int             allow_severity = LOG_INFO;
int             deny_severity = LOG_WARNING;
#endif

/*
 * #define NETSNMP_DS_APP_DONT_LOG 9 defined in notification_log.h 
 */

#ifndef BSD4_3
#define BSD4_2
#endif

#ifndef FD_SET

typedef long    fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)        /* bits per mask */

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      memset((p), 0, sizeof(*(p)))
#endif

char           *logfile = 0;
int             Log = 0;
int             Print = 0;
int             Syslog = 0;
int             SyslogTrap = 0;
int             Event = 0;
int             dropauth = 0;
int             running = 1;
int             reconfig = 0;
u_long          num_received = 0;
char            ddefault_port[] = "udp:162";	/* Default default port */
char           *default_port = ddefault_port;

/*
 * These definitions handle 4.2 systems without additional syslog facilities.
 */
#ifndef LOG_CONS
#define LOG_CONS	0       /* Don't bother if not defined... */
#endif
#ifndef LOG_PID
#define LOG_PID		0       /* Don't bother if not defined... */
#endif
#ifndef LOG_LOCAL0
#define LOG_LOCAL0	0
#endif
#ifndef LOG_LOCAL1
#define LOG_LOCAL1	0
#endif
#ifndef LOG_LOCAL2
#define LOG_LOCAL2	0
#endif
#ifndef LOG_LOCAL3
#define LOG_LOCAL3	0
#endif
#ifndef LOG_LOCAL4
#define LOG_LOCAL4	0
#endif
#ifndef LOG_LOCAL5
#define LOG_LOCAL5	0
#endif
#ifndef LOG_LOCAL6
#define LOG_LOCAL6	0
#endif
#ifndef LOG_LOCAL7
#define LOG_LOCAL7	0
#endif
#ifndef LOG_DAEMON
#define LOG_DAEMON	0
#endif

/*
 * Include an extra Facility variable to allow command line adjustment of
 * syslog destination 
 */
int Facility = LOG_DAEMON;

struct timeval  Now;

void            trapd_update_config(void);

static oid      risingAlarm[] = { 1, 3, 6, 1, 6, 3, 2, 1, 1, 3, 1 };
static oid      fallingAlarm[] = { 1, 3, 6, 1, 6, 3, 2, 1, 1, 3, 2 };
static oid      unavailableAlarm[] = { 1, 3, 6, 1, 6, 3, 2, 1, 1, 3, 3 };

void
event_input(netsnmp_variable_list * vp)
{
    int             eventid = 0;
    oid             variable[MAX_OID_LEN];
    int             variablelen = 0;
    u_long          destip = 0;
    int             sampletype = 0;
    int             value = 0;
    int             threshold = 0;
    int             i;
    int             nvars = 0;

    netsnmp_variable_list	*vp2 = vp;
    
    oid            *op = NULL;

    /* Make sure there are 5 variables.  Otherwise, don't bother */
    for (i=1; i <= 5; i++) {
      vp2 = vp2->next_variable;
      if (!vp2) {
	nvars = -1;
	break;
      }
    }
    
    if (nvars != -1)
    {
      vp = vp->next_variable;     /* skip sysUptime */
      if (vp->val_len != sizeof(risingAlarm) ||
	  !memcmp(vp->val.objid, risingAlarm, sizeof(risingAlarm)))
	eventid = 1;
      else if (vp->val_len != sizeof(risingAlarm) ||
	  !memcmp(vp->val.objid, fallingAlarm, sizeof(fallingAlarm)))
	eventid = 2;
      else if (vp->val_len != sizeof(risingAlarm) ||
	  !memcmp(vp->val.objid, unavailableAlarm, sizeof(unavailableAlarm)))
	eventid = 3;
      else {
	fprintf(stderr, "unknown event\n");
	eventid = 0;
      }
      
      vp = vp->next_variable;
      memmove(variable, vp->val.objid, vp->val_len * sizeof(oid));
      variablelen = vp->val_len;
      op = vp->name + 22;
      destip = 0;
      destip |= (*op++) << 24;
      destip |= (*op++) << 16;
      destip |= (*op++) << 8;
      destip |= *op++;
      
      vp = vp->next_variable;
      sampletype = *vp->val.integer;
      
      vp = vp->next_variable;
      value = *vp->val.integer;
      
      vp = vp->next_variable;
      threshold = *vp->val.integer;
    }
    printf("%d: 0x%02lX %d %d %d\n", eventid, destip, sampletype, value,
	threshold);
}


void
usage(void)
{
    fprintf(stderr, "Usage: snmptrapd [OPTIONS] [LISTENING ADDRESSES]\n");
    fprintf(stderr, "\n\tNET-SNMP Version:  %s\n", netsnmp_get_version());
    fprintf(stderr, "\tWeb:      http://www.net-snmp.org/\n");
    fprintf(stderr, "\tEmail:    net-snmp-coders@lists.sourceforge.net\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -a\t\t\tignore authentication failure traps\n");
    fprintf(stderr, "  -c FILE\t\tread FILE as a configuration file\n");
    fprintf(stderr,
            "  -C\t\t\tdo not read the default configuration files\n");
    fprintf(stderr, "  -d\t\t\tdump sent and received SNMP packets\n");
    fprintf(stderr, "  -D\t\t\tturn on debugging output\n");
    fprintf(stderr,
            "  -e\t\t\tprint event # (rising/falling alarm, etc.)\n");
    fprintf(stderr, "  -f\t\t\tdo not fork from the shell\n");
    fprintf(stderr,
            "  -F FORMAT\t\tuse specified format for logging to standard error\n");
    fprintf(stderr, "  -h, --help\t\tdisplay this usage message\n");
    fprintf(stderr,
            "  -H\t\t\tdisplay configuration file directives understood\n");
    fprintf(stderr,
            "  -m MIBLIST\t\tuse MIBLIST instead of the default MIB list\n");
    fprintf(stderr,
            "  -M DIRLIST\t\tuse DIRLIST as the list of locations\n\t\t\t  to look for MIBs\n");
    fprintf(stderr,
            "  -n\t\t\tuse numeric addresses instead of attempting\n\t\t\t  hostname lookups (no DNS)\n");
#if HAVE_GETPID
    fprintf(stderr, "  -p FILE\t\tstore process id in FILE\n");
#endif
    fprintf(stderr, "  -t\t\t\tPrevent traps from being logged to syslog\n");
    fprintf(stderr, "  -v, --version\t\tdisplay version information\n");
    fprintf(stderr,
            "  -O <OUTOPTS>\t\ttoggle options controlling output display\n");
    snmp_out_toggle_options_usage("\t\t\t", stderr);
    fprintf(stderr,
            "  -L <LOGOPTS>\t\ttoggle options controlling where to log to\n");
    snmp_log_options_usage("\t\t\t", stderr);
    fprintf(stderr, "\n  Deprecated options:\n");
    fprintf(stderr, "  -o FILE\t\tuse -Lf <FILE> instead\n");
    fprintf(stderr, "  -P\t\t\tuse -Le  instead\n");
    fprintf(stderr, "  -s\t\t\tuse -Lsd instead\n");
    fprintf(stderr, "  -S d|i|0-7\t\tuse -Ls <facility> instead\n");
    fprintf(stderr, "  -u FILE\t\tuse -p <FILE> instead\n");
}

static void
version(void)
{
    printf("\nNET-SNMP Version:  %s\n", netsnmp_get_version());
    printf("Web:               http://www.net-snmp.org/\n");
    printf("Email:             net-snmp-coders@lists.sourceforge.net\n\n");
    exit(0);
}

RETSIGTYPE
term_handler(int sig)
{
    running = 0;
}

#ifdef SIGHUP
RETSIGTYPE
hup_handler(int sig)
{
    reconfig = 1;
    signal(SIGHUP, hup_handler);
}
#endif

static int
pre_parse(netsnmp_session * session, netsnmp_transport *transport,
          void *transport_data, int transport_data_length)
{
#if USE_LIBWRAP
    char *addr_string = NULL;

    if (transport != NULL && transport->f_fmtaddr != NULL) {
        /*
         * Okay I do know how to format this address for logging.  
         */
        addr_string = transport->f_fmtaddr(transport, transport_data,
                                           transport_data_length);
        /*
         * Don't forget to free() it.  
         */
    }

    if (addr_string != NULL) {
        if (hosts_ctl("snmptrapd", STRING_UNKNOWN, 
		      addr_string, STRING_UNKNOWN) == 0) {
            free(addr_string);
            return 0;
        }
        free(addr_string);
    } else {
        if (hosts_ctl("snmptrapd", STRING_UNKNOWN,
                      STRING_UNKNOWN, STRING_UNKNOWN) == 0) {
            return 0;
        }
    }
#endif/*  USE_LIBWRAP  */
    return 1;
}

static netsnmp_session *
snmptrapd_add_session(netsnmp_transport *t)
{
    netsnmp_session sess, *session = &sess, *rc = NULL;

    snmp_sess_init(session);
    session->peername = SNMP_DEFAULT_PEERNAME;  /* Original code had NULL here */
    session->version = SNMP_DEFAULT_VERSION;
    session->community_len = SNMP_DEFAULT_COMMUNITY_LEN;
    session->retries = SNMP_DEFAULT_RETRIES;
    session->timeout = SNMP_DEFAULT_TIMEOUT;
    session->callback = snmp_input;
    session->callback_magic = (void *) t;
    session->authenticator = NULL;
    sess.isAuthoritative = SNMP_SESS_UNKNOWNAUTH;

    rc = snmp_add(session, t, pre_parse, NULL);
    if (rc == NULL) {
        snmp_sess_perror("snmptrapd", session);
    }
    return rc;
}

static void
snmptrapd_close_sessions(netsnmp_session * sess_list)
{
    netsnmp_session *s = NULL, *next = NULL;

    for (s = sess_list; s != NULL; s = next) {
        next = s->next;
        snmp_close(s);
    }
}

void
parse_trapd_address(const char *token, char *cptr)
{
    char buf[BUFSIZ];
    cptr = copy_nword(cptr, buf, sizeof(buf));

    if (default_port == ddefault_port) {
        default_port = strdup(buf);
    } else {
        strcat( buf, "," );
        strcat( buf, default_port );
        free(default_port);
        default_port = strdup(buf);
    }
}

void
free_trapd_address(void)
{
    if (default_port != ddefault_port) {
        free(default_port);
    }
}

int
main(int argc, char *argv[])
{
    char            options[128] = "ac:CdD::efF:hHl:L:m:M:no:PqsS:tvO:-:";
    netsnmp_session *sess_list = NULL, *ss = NULL;
    netsnmp_transport *transport = NULL;
    int             arg, i = 0;
    int             count, numfds, block;
    fd_set          fdset;
    struct timeval  timeout, *tvp;
    int             dofork = 1;
    char           *cp, *listen_ports = NULL;
    char           *trap1_fmt_str_remember = NULL;
    int             agentx_subagent = 1, depmsg = 0;
#if HAVE_GETPID
    FILE           *PID;
    char           *pid_file = NULL;
#endif

    /*
     * register our configuration handlers now so -H properly displays them 
     */
    snmptrapd_register_configs( );
    register_config_handler("snmptrapd", "createUser",
                            usm_parse_create_usmUser, NULL,
                           "username (MD5|SHA) passphrase [DES [passphrase]]");
    register_config_handler("snmptrapd", "usmUser",
                            usm_parse_config_usmUser, NULL, NULL);
    register_config_handler("snmptrapd", "snmptrapdaddr",
                            parse_trapd_address, free_trapd_address, NULL);

    /*
     * we need to be called back later 
     */
    snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                           usm_store_users, NULL);

#ifdef WIN32
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
#else
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
#endif

    /*
     * Add some options if they are available.  
     */
#if HAVE_GETPID
    strcat(options, "p:u:");
#endif

    /*
     * Now process options normally.  
     */

    while ((arg = getopt(argc, argv, options)) != EOF) {
        switch (arg) {
        case '-':
            if (strcasecmp(optarg, "help") == 0) {
                usage();
                exit(0);
            }
            if (strcasecmp(optarg, "version") == 0) {
                version();
                exit(0);
            }

            handle_long_opt(optarg);
            break;

        case 'a':
            dropauth = 1;
            break;

        case 'c':
            if (optarg != NULL) {
                netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, 
				      NETSNMP_DS_LIB_OPTIONALCONFIG, optarg);
            } else {
                usage();
                exit(1);
            }
            break;

        case 'C':
            netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, 
				   NETSNMP_DS_LIB_DONT_READ_CONFIGS, 1);
            break;

        case 'd':
            snmp_set_dump_packet(1);
            break;

        case 'D':
            debug_register_tokens(optarg);
            snmp_set_do_debugging(1);
            break;

        case 'e':
            Event++;
            break;

        case 'f':
            dofork = 0;
            break;

        case 'F':
            if (optarg != NULL) {
                trap1_fmt_str_remember = optarg;
            } else {
                usage();
                exit(1);
            }
            break;

        case 'h':
            usage();
            exit(0);

        case 'H':
            init_notification_log();
            init_snmp("snmptrapd");
            fprintf(stderr, "Configuration directives understood:\n");
            read_config_print_usage("  ");
            exit(0);

	case 'S':
            fprintf(stderr,
                    "Warning: -S option is deprecated; use -Ls <facility> instead\n");
            depmsg = 1;
            if (optarg != NULL) {
                switch (*optarg) {
                case 'd':
                case 'D':
                    Facility = LOG_DAEMON;
                    break;
                case 'i':
                case 'I':
                    Facility = LOG_INFO;
                    break;
                case '0':
                    Facility = LOG_LOCAL0;
                    break;
                case '1':
                    Facility = LOG_LOCAL1;
                    break;
                case '2':
                    Facility = LOG_LOCAL2;
                    break;
                case '3':
                    Facility = LOG_LOCAL3;
                    break;
                case '4':
                    Facility = LOG_LOCAL4;
                    break;
                case '5':
                    Facility = LOG_LOCAL5;
                    break;
                case '6':
                    Facility = LOG_LOCAL6;
                    break;
                case '7':
                    Facility = LOG_LOCAL7;
                    break;
                default:
                    fprintf(stderr, "invalid syslog facility: -S%c\n",*optarg);
                    usage();
                    exit(1);
                }
            } else {
                fprintf(stderr, "no syslog facility specified\n");
                usage();
                exit(1);
            }
            break;

        case 'm':
            if (optarg != NULL) {
                setenv("MIBS", optarg, 1);
            } else {
                usage();
                exit(1);
            }
            break;

        case 'M':
            if (optarg != NULL) {
                setenv("MIBDIRS", optarg, 1);
            } else {
                usage();
                exit(1);
            }
            break;

        case 'n':
            netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
				   NETSNMP_DS_APP_NUMERIC_IP, 1);
            break;

        case 'o':
            fprintf(stderr,
                    "Warning: -o option is deprecated; use -Lf <file> instead\n");
            Print++;
            if (optarg != NULL) {
                logfile = optarg;
                snmp_enable_filelog(optarg, 0);
            } else {
                usage();
                exit(1);
            }
            break;

        case 'O':
            cp = snmp_out_toggle_options(optarg);
            if (cp != NULL) {
                fprintf(stderr, "Unknown output option passed to -O: %c\n",
			*cp);
                usage();
                exit(1);
            }
            break;

        case 'L':
	    if  (snmp_log_options( optarg, argc, argv ) < 0 ) {
                usage();
                exit(1);
            }
            Log++;
            break;

        case 'P':
            fprintf(stderr,
                    "Warning: -P option is deprecated; use -f -Le instead\n");
            dofork = 0;
            snmp_enable_stderrlog();
            Print++;
            break;

        case 's':
            fprintf(stderr,
                    "Warning: -s option is deprecated; use -Lsd instead\n");
            depmsg = 1;
            Syslog++;
            break;

        case 't':
            SyslogTrap++;
            break;


#if HAVE_GETPID
        case 'u':
            fprintf(stderr,
                    "Warning: -u option is deprecated; use -p instead\n");
        case 'p':
            if (optarg != NULL) {
                pid_file = optarg;
            } else {
                usage();
                exit(1);
            }
            break;
#endif

        case 'v':
            version();
            exit(0);
            break;

        default:
            fprintf(stderr, "invalid option: -%c\n", arg);
            usage();
            exit(1);
            break;
        }
    }

    if (optind < argc) {
        /*
         * There are optional transport addresses on the command line.  
         */
        for (i = optind; i < argc; i++) {
            char *astring;
            if (listen_ports != NULL) {
                astring = malloc(strlen(listen_ports) + 2 + strlen(argv[i]));
                if (astring == NULL) {
                    fprintf(stderr, "malloc failure processing argv[%d]\n", i);
                    exit(1);
                }
                sprintf(astring, "%s,%s", listen_ports, argv[i]);
                free(listen_ports);
                listen_ports = astring;
            } else {
                listen_ports = strdup(argv[i]);
                if (listen_ports == NULL) {
                    fprintf(stderr, "malloc failure processing argv[%d]\n", i);
                    exit(1);
                }
            }
        }
    }

    /*
     * I'm being lazy here, and not checking the
     * return value from these registration calls.
     * Don't try this at home, children!
     */
    if (!Log && !Print) {
        Syslog = 1;
        netsnmp_add_global_traphandler(NETSNMPTRAPD_PRE_HANDLER, syslog_handler);
    } else {
        netsnmp_add_global_traphandler(NETSNMPTRAPD_PRE_HANDLER, print_handler);
    }
    netsnmp_add_global_traphandler(NETSNMPTRAPD_POST_HANDLER, notification_handler);

    if (Event) {
        netsnmp_add_traphandler(event_handler, risingAlarm,
                                    OID_LENGTH(risingAlarm));
        netsnmp_add_traphandler(event_handler, fallingAlarm,
                                    OID_LENGTH(fallingAlarm));
        netsnmp_add_traphandler(event_handler, unavailableAlarm,
                                    OID_LENGTH(unavailableAlarm));
	/* XXX - might be worth setting some "magic data"
	 * in the traphandler structure that 'event_handler'
	 * can use to avoid checking the trap OID values.
	 */
    }

#ifdef USING_AGENTX_SUBAGENT_MODULE
    /*
     * we're an agentx subagent? 
     */
    if (agentx_subagent) {
        /*
         * make us a agentx client. 
         */
        netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID,
			       NETSNMP_DS_AGENT_ROLE, 1);
    }
#endif

    /*
     * don't fail if we can't do agentx (ie, socket not there, or not root) 
     */
    netsnmp_ds_toggle_boolean(NETSNMP_DS_APPLICATION_ID, 
			      NETSNMP_DS_AGENT_NO_ROOT_ACCESS);
    /*
     * ignore any warning messages.
     */
    netsnmp_ds_toggle_boolean(NETSNMP_DS_APPLICATION_ID, 
			      NETSNMP_DS_AGENT_NO_CONNECTION_WARNINGS);

    /*
     * initialize the agent library 
     */
    init_agent("snmptrapd");

    /*
     * initialize local modules 
     */
    if (agentx_subagent) {
#ifdef USING_AGENTX_SUBAGENT_MODULE
        init_subagent();
#endif
        init_notification_log();
    }

    /*
     * Initialize the world. Create initial user 
     */
    init_snmp("snmptrapd");
    if (trap1_fmt_str_remember) {
        free_trap1_fmt();
        free_trap2_fmt();
        print_format1 = strdup(trap1_fmt_str_remember);
        print_format2 = strdup(trap1_fmt_str_remember);
    }

    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_QUIT_IMMEDIATELY)) {
        /*
         * just starting up to process specific configuration and then
         * shutting down immediately. 
         */
        running = 0;
    }
#ifndef WIN32
    /*
     * fork the process to the background if we are not printing to stderr 
     */
    if (dofork && running) {
        int             fd;

        switch (fork()) {
        case -1:
            fprintf(stderr, "bad fork - %s\n", strerror(errno));
            _exit(1);

        case 0:
            /*
             * become process group leader 
             */
            if (setsid() == -1) {
                fprintf(stderr, "bad setsid - %s\n", strerror(errno));
                _exit(1);
            }

            /*
             * if we are forked, we don't want to print out to stdout or stderr 
             */
            fd = open("/dev/null", O_RDWR);
            dup2(fd, STDIN_FILENO);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            break;

        default:
            _exit(0);
        }
    }
#endif                          /* WIN32 */
#if HAVE_GETPID
    if (pid_file != NULL) {
        if ((PID = fopen(pid_file, "w")) == NULL) {
            snmp_log_perror("fopen");
            exit(1);
        }
        fprintf(PID, "%d\n", (int) getpid());
        fclose(PID);
    }
#endif

    if (Syslog) {
        snmp_enable_syslog_ident("snmptrapd", Facility);
        snmp_log(LOG_INFO, "Starting snmptrapd %s\n", netsnmp_get_version());
	if (depmsg) {
	    snmp_log(LOG_WARNING, "-s and -S options are deprecated; use -Ls <facility> instead\n");
	}
    }
    if (Print || Log) {
        struct tm      *tm;
        time_t          timer;
        time(&timer);
        tm = localtime(&timer);
        snmp_log(LOG_INFO,
                "%.4d-%.2d-%.2d %.2d:%.2d:%.2d NET-SNMP version %s Started.\n",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                 tm->tm_hour, tm->tm_min, tm->tm_sec,
                 netsnmp_get_version());
    }

    SOCK_STARTUP;

    if (listen_ports)
        cp = listen_ports;
    else
        cp = default_port;

    while (cp != NULL) {
        char *sep = strchr(cp, ',');
        if (sep != NULL) {
            *sep = 0;
        }

        transport = netsnmp_tdomain_transport(cp, 1, "udp");
        if (transport == NULL) {
            snmp_log(LOG_ERR, "couldn't open %s -- errno %d (\"%s\")\n",
                     cp, errno, strerror(errno));
            snmptrapd_close_sessions(sess_list);
            SOCK_CLEANUP;
            exit(1);
        } else {
            ss = snmptrapd_add_session(transport);
            if (ss == NULL) {
                /*
                 * Shouldn't happen?  We have already opened the transport
                 * successfully so what could have gone wrong?  
                 */
                snmptrapd_close_sessions(sess_list);
                netsnmp_transport_free(transport);
                if (Syslog) {
                    snmp_log(LOG_ERR, "couldn't open snmp - %m");
                }
                SOCK_CLEANUP;
                exit(1);
            } else {
                ss->next = sess_list;
                sess_list = ss;
            }
        }

        /*
         * Process next listen address, if there is one.  
         */

        if (sep != NULL) {
            *sep = ',';
            cp = sep + 1;
        } else {
            cp = NULL;
        }
    }

    signal(SIGTERM, term_handler);
#ifdef SIGHUP
    signal(SIGHUP, hup_handler);
#endif
    signal(SIGINT, term_handler);

    while (running) {
        if (reconfig) {
            if (Print) {
                struct tm      *tm;
                time_t          timer;
                time(&timer);
                tm = localtime(&timer);

                /*
                 * If we are logging to a file, receipt of SIGHUP also
                 * indicates the the log file should be closed and
                 * re-opened.  This is useful for users that want to
                 * rotate logs in a more predictable manner.
                 */
                if (logfile)
                    snmp_enable_filelog(logfile, 1);

                snmp_log(LOG_INFO,
                         "%.4d-%.2d-%.2d %.2d:%.2d:%.2d NET-SNMP version %s Reconfigured.\n",
                         tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                         tm->tm_hour, tm->tm_min, tm->tm_sec,
                         netsnmp_get_version());
            }
            if (Syslog)
                snmp_log(LOG_INFO, "Snmptrapd reconfiguring");
            trapd_update_config();
            if (trap1_fmt_str_remember) {
                free_trap1_fmt();
                print_format1 = strdup(trap1_fmt_str_remember);
            }
            reconfig = 0;
        }
        numfds = 0;
        FD_ZERO(&fdset);
        block = 0;
        tvp = &timeout;
        timerclear(tvp);
        tvp->tv_sec = 5;
        snmp_select_info(&numfds, &fdset, tvp, &block);
        if (block == 1)
            tvp = NULL;         /* block without timeout */
        count = select(numfds, &fdset, 0, 0, tvp);
        gettimeofday(&Now, 0);
        if (count > 0) {
            snmp_read(&fdset);
        } else
            switch (count) {
            case 0:
                snmp_timeout();
                break;
            case -1:
                if (errno == EINTR)
                    continue;
                snmp_log_perror("select");
                running = 0;
                break;
            default:
                fprintf(stderr, "select returned %d\n", count);
                running = 0;
            }
    }

    if (Print) {
        struct tm      *tm;
        time_t          timer;
        time(&timer);
        tm = localtime(&timer);
        snmp_log(LOG_INFO,
                "%.4d-%.2d-%.2d %.2d:%.2d:%.2d NET-SNMP version %s Stopped.\n",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
                 tm->tm_min, tm->tm_sec, netsnmp_get_version());
    }
    if (Syslog) {
        snmp_log(LOG_INFO, "Stopping snmptrapd");
    }

    snmptrapd_close_sessions(sess_list);
    snmp_shutdown("snmptrapd");
    snmp_disable_log();
    SOCK_CLEANUP;
    return 0;
}

/*
 * Read the configuration files. Implemented as a signal handler so that
 * receipt of SIGHUP will cause configuration to be re-read when the
 * trap deamon is running detatched from the console.
 *
 */
void
trapd_update_config(void)
{
    free_config();
    read_configs();
}


#if !defined(HAVE_GETDTABLESIZE) && !defined(WIN32)
#include <sys/resource.h>
int
getdtablesize(void)
{
    struct rlimit   rl;
    getrlimit(RLIMIT_NOFILE, &rl);
    return (rl.rlim_cur);
}
#endif
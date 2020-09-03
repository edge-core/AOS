/*
 * logging.c - generic logging for snmp-agent
 * * Contributed by Ragnar Kj�rstad, ucd@ragnark.vestdata.no 1999-06-26 
 */

#include <net-snmp/net-snmp-config.h>
#ifndef VXWORKS
#include <stdio.h>
#if HAVE_MALLOC_H
#include <malloc.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif /*end of #ifndef VXWORKS*/
#include <errno.h>

#if HAVE_SYSLOG_H
#include <syslog.h>
#ifndef LOG_CONS                /* Interesting Ultrix feature */
#include <sys/syslog.h>
#endif
#endif
#ifndef VXWORKS
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#endif /*end of #ifndef VXWORKS*/

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_STDARG_H
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_WINDOWS_H
#include <windows.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/library/snmp_logging.h>      /* For this file's "internal" definitions */
#include <net-snmp/config_api.h>
#include <net-snmp/utilities.h>

#include <net-snmp/library/callback.h>
#include <ctype.h>
#include <time.h>
#include "snmp_mgr.h"
#define LOGLENGTH 1024

/*
 * logh_head:  A list of all log handlers, in increasing order of priority
 * logh_priorities:  'Indexes' into this list, by priority
 */
netsnmp_log_handler *logh_head;
netsnmp_log_handler *logh_priorities[LOG_DEBUG+1];

/*
static int      do_syslogging = 0;
static int      do_filelogging = 0;
static int      do_stderrlogging = 1;
static int      do_log_callback = 0;
static FILE    *logfile;
*/
static int      newline = 1;
#ifdef WIN32
static HANDLE   eventlog_h;
#endif

#ifndef HAVE_VSNPRINTF
                /*
                 * Need to use the UCD-provided one 
                 */
int             vsnprintf(char *str, size_t count, const char *fmt,
                          va_list arg);
#endif

void
init_snmp_logging(void)
{
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "logTimestamp", 
			 NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_LOG_TIMESTAMP);
}

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
#ifndef LOG_USER
#define LOG_USER	0
#endif

int
decode_priority( char *optarg, int *pri_max )
{
    int pri_low = LOG_DEBUG;

    switch (*optarg) {
        case '0': 
        case '!': 
            pri_low = LOG_EMERG;
            break;
        case '1': 
        case 'a': 
        case 'A': 
            pri_low = LOG_ALERT;
            break;
        case '2': 
        case 'c': 
        case 'C': 
            pri_low = LOG_CRIT;
            break;
        case '3': 
        case 'e': 
        case 'E': 
            pri_low = LOG_ERR;
            break;
        case '4': 
        case 'w': 
        case 'W': 
            pri_low = LOG_WARNING;
            break;
        case '5': 
        case 'n': 
        case 'N': 
            pri_low = LOG_NOTICE;
            break;
        case '6': 
        case 'i': 
        case 'I': 
            pri_low = LOG_INFO;
            break;
        case '7': 
        case 'd': 
        case 'D': 
            pri_low = LOG_DEBUG;
            break;
    }

    if (pri_max && *(optarg+1)=='-') {
        *pri_max = decode_priority( optarg+2, NULL );
    }
    return pri_low;
}

int
decode_facility( char *optarg )
{
    if (optarg == NULL)
        return -1;

    switch (*optarg) {
    case 'd':
    case 'D':
        return LOG_DAEMON;
    case 'u':
    case 'U':
        return LOG_USER;
    case '0':
        return LOG_LOCAL0;
    case '1':
        return LOG_LOCAL1;
    case '2':
        return LOG_LOCAL2;
    case '3':
        return LOG_LOCAL3;
    case '4':
        return LOG_LOCAL4;
    case '5':
        return LOG_LOCAL5;
    case '6':
        return LOG_LOCAL6;
    case '7':
        return LOG_LOCAL7;
    default:
        fprintf(stderr, "invalid syslog facility: -S%c\n",*optarg);
        return -1;
    }
}

int
snmp_log_options(char *optarg, int argc, char *const *argv)
{
    char           *cp = optarg;
        /*
	 * Hmmm... this doesn't seem to work.
	 * The main agent 'getopt' handling assumes
	 *   that the -L option takes an argument,
	 *   and objects if this is missing.
	 * Trying to differentiate between
	 *   new-style "-Lx", and old-style "-L xx"
	 *   is likely to be a major headache.
	 */
    char            missing_opt = 'e';	/* old -L is new -Le */
    int             priority = LOG_DEBUG;
    int             pri_max  = LOG_EMERG;
    int             inc_optind = 0;
    netsnmp_log_handler *logh;

    optarg++;
    if (!*cp)
        cp = &missing_opt;

    /*
     * Support '... -Lx=value ....' syntax
     */
    if (*optarg == '=') {
        optarg++;
    }
    /*
     * and '.... "-Lx value" ....'  (*with* the quotes)
     */
    while (*optarg && isspace(*optarg)) {
        optarg++;
    }
    /*
     * Finally, handle ".... -Lx value ...." syntax
     *   (*without* surrounding quotes)
     */
    if (!*optarg) {
        /*
         * We've run off the end of the argument
         *  so move on to the next.
         * But we might not actually need it, so don't
	 *  increment optind just yet!
         */
        optarg = argv[optind];
        inc_optind = 1;
    }

    switch (*cp) {

    /*
     * Log to Standard Error
     */
    case 'E':
        priority = decode_priority( optarg, &pri_max );
        if (inc_optind)
            optind++;
        /* Fallthrough */
    case 'e':
        logh = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_STDERR, priority);
        if (logh) {
            logh->pri_max = pri_max;
            logh->token   = "stderr";
	}
        break;

    /*
     * Log to Standard Output
     */
    case 'O':
        priority = decode_priority( optarg, &pri_max );
        if (inc_optind)
            optind++;
        /* Fallthrough */
    case 'o':
        logh = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_STDERR, priority);
        if (logh) {
            logh->pri_max = pri_max;
            logh->token   = "stdout";
            logh->imagic  = 1;	    /* stdout, not stderr */
	}
        break;

    /*
     * Log to a named file
     */
    case 'F':
        priority = decode_priority( optarg, &pri_max );
        optarg = argv[++optind];
        /* Fallthrough */
    case 'f':
        if (inc_optind)
            optind++;
        if (!optarg) {
            fprintf(stderr, "Missing log file\n");
            return -1;
        }
        logh = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_FILE, priority);
        if (logh) {
            logh->pri_max = pri_max;
            logh->token   = strdup(optarg);
	}
        break;

    /*
     * Log to syslog
     */
    case 'S':
        priority = decode_priority( optarg, &pri_max );
        optarg = argv[++optind];
        /* Fallthrough */
    case 's':
        if (inc_optind)
            optind++;
        if (!optarg) {
            fprintf(stderr, "Missing syslog facility\n");
            return -1;
        }
        logh = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_SYSLOG, priority);
        if (logh) {
            logh->pri_max = pri_max;
            logh->token   = strdup(DEFAULT_LOG_ID);	/*  ident string  */
#ifndef WIN32
            logh->imagic  = decode_facility(optarg);
#endif
	}
        break;

    default:
        fprintf(stderr, "Unknown logging option passed to -L: %c.\n", *cp);
        return -1;
    }
    return 0;
}

void
snmp_log_options_usage(const char *lead, FILE * outf)
{
    const char *pri1_msg = " for level 'pri' and above";
    const char *pri2_msg = " for levels 'p1' to 'p2'";
    fprintf(outf, "%se:           log to standard error\n", lead);
    fprintf(outf, "%so:           log to standard output\n", lead);
    fprintf(outf, "%sf file:      log to the specified file\n", lead);
    fprintf(outf, "%ss facility:  log to syslog (via the specified facility)\n", lead);
    fprintf(outf, "\n%s(variants)\n", lead);
    fprintf(outf, "%s[EO] pri:    log to standard error/output%s\n", lead, pri1_msg);
    fprintf(outf, "%s[EO] p1-p2:  log to standard error/output%s\n", lead, pri2_msg);
    fprintf(outf, "%s[FS] pri token:    log to file/syslog%s\n", lead, pri1_msg);
    fprintf(outf, "%s[FS] p1-p2 token:  log to file/syslog%s\n", lead, pri2_msg);
}

int
snmp_get_do_logging(void)
{
    netsnmp_log_handler *logh;
    for (logh = logh_head; logh; logh = logh->next)
        if (logh->enabled)
            return 1;
    return 0;
}

#ifndef VXWORKS /* XXX steven.jiang for removing compilation warnings */
static char    *
sprintf_stamp(time_t * now, char *sbuf)
{
    time_t          Now;
    struct tm      *tm;

    if (now == NULL) {
        now = &Now;
        time(now);
    }
    tm = localtime(now);
#ifndef VXWORKS
    sprintf(sbuf, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d ",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);
    return sbuf;
#else
    return NULL;
#endif
}
#endif /* !VXWORKS */

#ifdef WIN32
void
snmp_disable_syslog(void)
{
    netsnmp_log_handler *logh;
    HANDLE               eventlog_h;

    for (logh = logh_head; logh; logh = logh->next)
        if (logh->enabled && logh->type == NETSNMP_LOGHANDLER_SYSLOG) {

            eventlog_h = (HANDLE)logh->magic;
            if (eventlog_h != NULL) {
                if (!CloseEventLog(eventlog_h)) {
	            /*
	             * Hmmm.....
	             * Maybe disable this handler, and log the error ?
	             */
                    fprintf(stderr, "Could not close event log. "
                            "Last error: 0x%x\n", GetLastError());
                } else {
                    logh->magic = NULL;
                }
            }
            logh->enabled = 0;
	}
}
#else
void
snmp_disable_syslog(void)
{
    /* we don't need netsnmp logging*/
    #ifndef VXWORKS
    netsnmp_log_handler *logh;

    for (logh = logh_head; logh; logh = logh->next)
        if (logh->enabled && logh->type == NETSNMP_LOGHANDLER_SYSLOG) {
            closelog();
            logh->imagic  = 0;
            logh->enabled = 0;
	}
	#endif
}
#endif

void
snmp_disable_filelog(void)
{
    netsnmp_log_handler *logh;

    for (logh = logh_head; logh; logh = logh->next)
        if (logh->enabled && logh->type == NETSNMP_LOGHANDLER_FILE) {
            if (logh->magic) {
                fputs("\n", (FILE*)logh->magic);	/* XXX - why? */
                fclose((FILE*)logh->magic);
                logh->magic   = NULL;
	    }
            logh->enabled = 0;
	}
}

void
snmp_disable_stderrlog(void)
{
    netsnmp_log_handler *logh;

    for (logh = logh_head; logh; logh = logh->next)
        if (logh->enabled && (logh->type == NETSNMP_LOGHANDLER_STDOUT ||
                              logh->type == NETSNMP_LOGHANDLER_STDERR)) {
            logh->enabled = 0;
	}
}

void
snmp_disable_calllog(void)
{
    netsnmp_log_handler *logh;

    for (logh = logh_head; logh; logh = logh->next)
        if (logh->enabled && logh->type == NETSNMP_LOGHANDLER_CALLBACK) {
            logh->enabled = 0;
	}
}

void
snmp_disable_log(void)
{
    netsnmp_log_handler *logh;

    for (logh = logh_head; logh; logh = logh->next)
        logh->enabled = 0;	/* XXX - any per-handler closedown? */
}

/* ================================================== */

void
snmp_enable_syslog(void)
{
    snmp_enable_syslog_ident(DEFAULT_LOG_ID, LOG_DAEMON);
}

void
snmp_enable_syslog_ident(const char *ident, const int facility)
{
#ifndef VXWORKS

    netsnmp_log_handler *logh;
    int                  found = 0;
    int                  enable = 1;
#ifdef WIN32
    HANDLE               eventlog_h;
#else
    void                *eventlog_h = NULL;
#endif

    snmp_disable_syslog();	/* ??? */
#ifdef WIN32
    eventlog_h = OpenEventLog(NULL, ident);
    if (eventlog_h == NULL) {
	    /*
	     * Hmmm.....
	     * Maybe disable this handler, and log the error ?
	     */
        fprintf(stderr, "Could not open event log for %s. "
                "Last error: 0x%x\n", ident, GetLastError());
        enable = 0;
    }
#else
    if (ident == NULL)
        ident = DEFAULT_LOG_ID;
    openlog(ident, LOG_CONS | LOG_PID, facility);
#endif

    for (logh = logh_head; logh; logh = logh->next)
        if (logh->type == NETSNMP_LOGHANDLER_SYSLOG) {
            logh->magic   = (void*)eventlog_h;
            logh->imagic  = enable;	/* syslog open */
            logh->enabled = enable;
            found         = 1;
	}

    if (!found) {
        logh = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_SYSLOG,
                                           LOG_DEBUG );
        if (logh) {
            logh->magic    = (void*)eventlog_h;
            logh->token    = strdup("syslog");
        }
    }
#endif /*end of #ifndef VXWORKS*/

}

void
netsnmp_enable_filelog(netsnmp_log_handler *logh, int dont_zero_log)
{
   #ifndef VXWORKS
    FILE *logfile;

    if (!logh)
        return;

    if (!logh->magic) {
        logfile = fopen(logh->token, dont_zero_log ? "a" : "w");
        if (!logfile)
            return;
        logh->magic = (void*)logfile;
#ifdef WIN32
        /*
         * Apparently, "line buffering" under Windows is
         *  actually implemented as "full buffering".
         *  Let's try turning off buffering completely.
         */
        setvbuf(logfile, NULL, _IONBF, BUFSIZ);
#else
        setvbuf(logfile, NULL, _IOLBF, BUFSIZ);
#endif
    }
    logh->enabled = 1;
    #endif /*end of #ifndef VXWORKS*/
}

void
snmp_enable_filelog(const char *logfilename, int dont_zero_log)
{
    netsnmp_log_handler *logh;

    snmp_disable_filelog();	/* XXX ??? */

    if (logfilename) {
        logh = netsnmp_find_loghandler( logfilename );
        if (logh)
            netsnmp_enable_filelog(logh, dont_zero_log);
	else {
            logh = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_FILE,
                                               LOG_DEBUG );
            if (logh) {
                logh->token = strdup(logfilename);
                logh->magic = (void*)fopen(logfilename,
                                           (dont_zero_log ? "a" : "w" ));
	    }
        }
    } else {
        for (logh = logh_head; logh; logh = logh->next)
            if (logh->type == NETSNMP_LOGHANDLER_FILE)
                netsnmp_enable_filelog(logh, dont_zero_log);
    }
}


void
snmp_enable_stderrlog(void)
{
    netsnmp_log_handler *logh;
    int                  found = 0;

    for (logh = logh_head; logh; logh = logh->next)
        if (logh->type == NETSNMP_LOGHANDLER_STDOUT ||
            logh->type == NETSNMP_LOGHANDLER_STDERR) {
            logh->enabled = 1;
            found         = 1;
        }

    if (!found) {
        logh = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_STDERR,
                                           LOG_DEBUG );
        if (logh)
            logh->token    = strdup("stderr");
    }
}


void
snmp_enable_calllog(void)	/* XXX - or take a callback routine ??? */
{
    netsnmp_log_handler *logh;
    int                  found = 0;

    for (logh = logh_head; logh; logh = logh->next) {
        if (logh->type == NETSNMP_LOGHANDLER_CALLBACK)
            logh->enabled = 1;
            found         = 1;
	}

    if (!found) {
        logh = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_CALLBACK,
                                           LOG_DEBUG );
        if (logh)
            logh->token    = strdup("callback");
    }
}



/* ==================================================== */


netsnmp_log_handler *
netsnmp_find_loghandler( const char *token )
{
    netsnmp_log_handler *logh;
    if (!token)
        return NULL;

    for (logh = logh_head; logh; logh = logh->next)
        if (logh->token && !strcmp( token, logh->token ))
            break;

    return logh;
}

int
netsnmp_add_loghandler( netsnmp_log_handler *logh )
{
    int i;
    netsnmp_log_handler *logh2;

    if (!logh)
        return 0;

    /*
     * Find the appropriate point for the new entry...
     *   (logh2 will point to the entry immediately following)
     */
    for (logh2 = logh_head; logh2; logh2 = logh2->next)
        if ( logh2->priority >= logh->priority )
            break;

    /*
     * ... and link it into the main list.
     */
    if (logh2) {
        if (logh2->prev)
            logh2->prev->next = logh;
        else
            logh_head = logh;
        logh->next  = logh2;
        logh2->prev = logh;
    } else if (logh_head ) {
        /*
         * If logh2 is NULL, we're tagging on to the end
         */
        for (logh2 = logh_head; logh2->next; logh2 = logh2->next)
            ;
        logh2->next = logh;
    } else {
        logh_head = logh;
    }

    /*
     * Also tweak the relevant priority-'index' array.
     */
    for (i=LOG_EMERG; i<=logh->priority; i++)
        if (!logh_priorities[i] ||
             logh_priorities[i]->priority >= logh->priority)
             logh_priorities[i] = logh;

    return 1;
}

netsnmp_log_handler *
netsnmp_register_loghandler( int type, int priority )
{
    netsnmp_log_handler *logh;

    logh = SNMP_MALLOC_TYPEDEF(netsnmp_log_handler);
    if (!logh)
        return NULL;

    logh->type     = type;
    switch ( type ) {
    case NETSNMP_LOGHANDLER_STDOUT:
        logh->imagic  = 1;
        /* fallthrough */
    case NETSNMP_LOGHANDLER_STDERR:
        logh->handler = log_handler_stdouterr;
        break;

    case NETSNMP_LOGHANDLER_FILE:
        logh->handler = log_handler_file;
        break;
    case NETSNMP_LOGHANDLER_SYSLOG:
        logh->handler = log_handler_syslog;
        break;
    case NETSNMP_LOGHANDLER_CALLBACK:
        logh->handler = log_handler_callback;
        break;
    default:
        free(logh->handler);
        return NULL;
    }
    logh->priority = priority;
    logh->enabled  = 1;
    netsnmp_add_loghandler( logh );
    return logh;
}


int
netsnmp_enable_loghandler( const char *token )
{
    netsnmp_log_handler *logh;

    logh = netsnmp_find_loghandler( token );
    if (!logh)
        return 0;
    logh->enabled = 1;
    return 1;
}


int
netsnmp_disable_loghandler( const char *token )
{
    netsnmp_log_handler *logh;

    logh = netsnmp_find_loghandler( token );
    if (!logh)
        return 0;
    logh->enabled = 0;
    return 1;
}

int
netsnmp_remove_loghandler( netsnmp_log_handler *logh )
{
    if (!logh)
        return 0;

    if (logh->prev)
        logh->prev->next = logh->next;
    else
        logh_head = logh->next;

    if (logh->next)
        logh->next->prev = logh->prev;
    return 1;
}

/* ==================================================== */

int
log_handler_stdouterr(  netsnmp_log_handler* logh, int pri, const char *string)
{
    char            sbuf[40];

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, 
                               NETSNMP_DS_LIB_LOG_TIMESTAMP) && newline) 
                               {
        #ifndef VXWORKS
        sprintf_stamp(NULL, sbuf);
        #endif
    } else {
        strcpy(sbuf, "");
    }
    newline = string[strlen(string) - 1] == '\n';	/* XXX - Eh ? */

    if (logh->imagic)
       printf(         "%s%s", sbuf, string);
    else
       fprintf(stderr, "%s%s", sbuf, string);

    return 1;
}


#ifdef WIN32
int
log_handler_syslog(  netsnmp_log_handler* logh, int pri, const char *string)
{
    char            sbuf[40];
    WORD            etype;
    LPCTSTR         event_msg[2];

        /*
         **  EVENT TYPES:
         **
         **  Information (EVENTLOG_INFORMATION_TYPE)
         **      Information events indicate infrequent but significant
         **      successful operations.
         **  Warning (EVENTLOG_WARNING_TYPE)
         **      Warning events indicate problems that are not immediately
         **      significant, but that may indicate conditions that could
         **      cause future problems. Resource consumption is a good
         **      candidate for a warning event.
         **  Error (EVENTLOG_ERROR_TYPE)
         **      Error events indicate significant problems that the user
         **      should know about. Error events usually indicate a loss of
         **      functionality or data.
         */
    switch (pri) {
        case LOG_EMERG:
        case LOG_ALERT:
        case LOG_CRIT:
        case LOG_ERR:
            etype = EVENTLOG_ERROR_TYPE;
            break;
        case LOG_WARNING:
            etype = EVENTLOG_WARNING_TYPE;
            break;
        case LOG_NOTICE:
        case LOG_INFO:
        case LOG_DEBUG:
            etype = EVENTLOG_INFORMATION_TYPE;
            break;
        default:
            etype = EVENTLOG_INFORMATION_TYPE;
            break;
    }
    event_msg[0] = string;
    event_msg[1] = NULL;
    if (!ReportEvent(eventlog_h, etype, 0, 0, NULL, 1, 0, event_msg, NULL)) {
	    /*
	     * Hmmm.....
	     * Maybe disable this handler, and log the error ?
	     */
        fprintf(stderr, "Could not report event.  Last error: 0x%x\n",
			GetLastError());
        return 0;
    }
    return 1;
}
#else
int
log_handler_syslog(  netsnmp_log_handler* logh, int pri, const char *string)
{
	/*
	 * XXX
	 * We've got three items of information to work with:
	 *     Is the syslog currently open?
	 *     What ident string to use?
	 *     What facility to log to?
	 *
	 * We've got two "magic" locations (imagic & magic) plus the token
	 */
#ifndef VXWORKS

    if (!(logh->imagic)) {
        const char *ident    = logh->token;
        int   facility = (int)logh->magic;
        openlog(ident, LOG_CONS | LOG_PID, facility);
        logh->imagic = 1;
    }
    syslog( pri, "%s", string );
#endif /*end of #ifndef VXWORKS*/
    return 1;
}
#endif /* !WIN32 */


int
log_handler_file(    netsnmp_log_handler* logh, int pri, const char *string)
{
    FILE           *fhandle;
    char            sbuf[40];

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, 
                               NETSNMP_DS_LIB_LOG_TIMESTAMP) && newline) {
                                
        #ifndef VXWORKS
        sprintf_stamp(NULL, sbuf);
        #endif
    } else {
        strcpy(sbuf, "");
    }
    newline = string[strlen(string) - 1] == '\n';	/* XXX - Eh ? */

    /*
     * If we haven't already opened the file, then do so.
     * Save the filehandle pointer for next time.
     *
     * Note that this should still work, even if the file
     * is closed in the meantime (e.g. a regular "cleanup" sweep)
     */
    fhandle = (FILE*)logh->magic;
    if (!logh->magic) {
        fhandle = fopen(logh->token, "a+");
        if (!fhandle)
            return 0;
        logh->magic = (void*)fhandle;
    }
    fprintf(fhandle, "%s%s", sbuf, string);
fflush(fhandle);
    return 1;
}

int
log_handler_callback(netsnmp_log_handler* logh, int pri, const char *string)
{
	/*
	 * XXX - perhaps replace 'snmp_call_callbacks' processing
	 *       with individual callback log_handlers ??
	 */
    struct snmp_log_message slm;
    int             dodebug = snmp_get_do_debugging();

    slm.priority = pri;
    slm.msg = string;
    if (dodebug)            /* turn off debugging inside the callbacks else will loop */
        snmp_set_do_debugging(0);
    snmp_call_callbacks(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_LOGGING, &slm);
    if (dodebug)
        snmp_set_do_debugging(dodebug);
    return 1;
}

void
snmp_log_string(int priority, const char *string)
{
    netsnmp_log_handler *logh;

    /*
     * Start at the given priority, and work "upwards"....
     */
    if (!logh_head)
	snmp_enable_stderrlog();
    logh = logh_priorities[priority];
    for ( ; logh; logh = logh->next ) {
        /*
         * ... but skipping any handlers with a "maximum priority"
	 *     that we have already exceeded.
         */
        if (priority >= logh->pri_max)
            logh->handler( logh, priority, string );
    }
}

/* ==================================================== */


int
snmp_vlog(int priority, const char *format, va_list ap)
{
    char            buffer[LOGLENGTH];
    int             length;
    char           *dynamic;
    
    #ifndef VXWORKS
    length = vsnprintf(buffer, LOGLENGTH, format, ap);
    #else
    length = snprintf(buffer, LOGLENGTH, format, ap);
    #endif
    if (length == 0)
        return (0);             /* Empty string */

    if (length == -1) {
        snmp_log_string(LOG_ERR, "Could not format log-string\n");
        return (-1);
    }

    if (length < LOGLENGTH) {
        snmp_log_string(priority, buffer);
        return (0);
    }

    dynamic = (char *) malloc(length + 1);
    if (dynamic == NULL) {
        snmp_log_string(LOG_ERR,
                        "Could not allocate memory for log-message\n");
        snmp_log_string(priority, buffer);
        return (-2);
    }
    #ifndef VXWORKS
    vsnprintf(dynamic, length + 1, format, ap);
    #else
    snprintf(dynamic, length + 1, format, ap);
    #endif
    snmp_log_string(priority, dynamic);
    free(dynamic);
    return 0;
}


int
#if HAVE_STDARG_H
snmp_log(int priority, const char *format, ...)
#else
snmp_log(va_alist)
     va_dcl
#endif
{
#ifdef VXWORKS 
    if (SNMP_MGR_IsDebugMode())
    {
#endif
    va_list         ap;
    int             ret;
#if HAVE_STDARG_H
    va_start(ap, format);
#else
    int             priority;
    const char     *format;
    va_start(ap);

    priority = va_arg(ap, int);
    format = va_arg(ap, const char *);
#endif
    ret = snmp_vlog(priority, format, ap);
    va_end(ap);
    return (ret);
#ifdef VXWORKS 
    } /* end of if (SNMP_MGR_IsDebugMode())*/

    return 0;
#endif

}

/*
 * log a critical error.
 */
void
snmp_log_perror(const char *s)
{
    char           *error = strerror(errno);
    if (s) {
        if (error)
            snmp_log(LOG_ERR, "%s: %s\n", s, error);
        else
            snmp_log(LOG_ERR, "%s: Error %d out-of-range\n", s, errno);
    } else {
        if (error)
            snmp_log(LOG_ERR, "%s\n", error);
        else
            snmp_log(LOG_ERR, "Error %d out-of-range\n", errno);
    }
}

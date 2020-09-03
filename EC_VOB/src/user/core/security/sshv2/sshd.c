/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * This program is the ssh daemon.  It listens for connections from clients,
 * and performs authentication, executes use commands or shell, and forwards
 * information to/from the application to the user client over an encrypted
 * connection.  This can also handle forwarding of X11, TCP/IP, and
 * authentication agent connections.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 *
 * SSH2 implementation:
 * Privilege Separation:
 *
 * Copyright (c) 2000, 2001, 2002 Markus Friedl.  All rights reserved.
 * Copyright (c) 2002 Niels Provos.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "includes.h"
RCSID("$OpenBSD: sshd.c,v 1.263 2003/02/16 17:09:57 markus Exp $");

#include "openssl/dh.h"
#include "openssl/bn.h"
#include "openssl/md5.h"
#include "openssl/rand.h"

#include "ssh.h"
#include "ssh1.h"
#include "ssh2.h"
#include "xmalloc.h"
#include "ssh_rsa.h"
#include "sshpty.h"
#include "packet.h"
#include "mpaux.h"
#include "log.h"
#include "servconf.h"
#include "uidswap.h"
#include "compat.h"
#include "buffer.h"
#include "cipher.h"
#include "kex.h"
#include "key.h"
#include "ssh_dh.h"
#include "myproposal.h"
#include "authfile.h"
#include "pathnames.h"
#include "atomicio.h"
#include "canohost.h"
#include "auth.h"
#include "misc.h"
#include "dispatch.h"
#include "channels.h"
#include "session.h"
#include "monitor_mm.h"
#include "monitor.h"
#include "monitor_wrap.h"
#include "monitor_fdpass.h"

#include "sshd_mgr.h"
#include "sshd_vm.h"
#include "sshd_task.h"
#include "keygen_mgr.h"

#if 0 /* squid, does not need for linux platform */
    #include "ip_task.h"
#endif

#include "sysfun.h"

#ifdef LIBWRAP
//#include <tcpd.h>
//#include <syslog.h>/*isiah*/
//static int allow_severity = LOG_INFO; /*isiah*/
//static int deny_severity = LOG_WARNING; /*isiah*/
#endif /* LIBWRAP */

#ifndef O_NOCTTY
#define O_NOCTTY	0
#endif

//extern char *__progname; /*isiah*/

/* Server configuration options. */
//static ServerOptions options; /*isiah*/

/* Name of the server configuration file. */
//static char *config_file_name = _PATH_SERVER_CONFIG_FILE; /*isiah*/

/*
 * Flag indicating whether IPv4 or IPv6.  This can be set on the command line.
 * Default value is AF_UNSPEC means both IPv4 and IPv6.
 */
//static int IPv4or6 = AF_UNSPEC; /*isiah*/

/*
 * Debug mode flag.  This can be set on the command line.  If debug
 * mode is enabled, extra debugging output will be sent to the system
 * log, the daemon will not go to background, and will exit after processing
 * the first connection.
 */
//static int debug_flag = 0; /*isiah*/

/* Flag indicating that the daemon should only test the configuration and keys. */
//static int test_flag = 0; /*isiah*/

/* Flag indicating that the daemon is being started from inetd. */
//static int inetd_flag = 0; /*isiah*/

/* Flag indicating that sshd should not detach and become a daemon. */
//static int no_daemon_flag = 0; /*isiah*/

/* debug goes to stderr unless inetd_flag is set */
//static int log_stderr = 0; /*isiah*/

/* Saved arguments to main(). */
//static char **saved_argv; /*isiah*/

/*
 * The sockets that the server is listening; this is used in the SIGHUP
 * signal handler.
 */
//#define	MAX_LISTEN_SOCKS	16
//static int listen_socks[MAX_LISTEN_SOCKS]; /*isiah*/
//static int num_listen_socks = 0; /*isiah*/

/*
 * the client's version string, passed by sshd2 in compat mode. if != NULL,
 * sshd will skip the version-number exchange
 */
//static char *client_version_string = NULL; /*isiah*/
//static char *server_version_string = NULL; /*isiah*/

/* for rekeying XXX fixme */
//static Kex *xxx_kex; /*isiah*/

/*
 * Any really sensitive data in the application is contained in this
 * structure. The idea is that this structure could be locked into memory so
 * that the pages do not get written into swap.  However, there are some
 * problems. The private key contains BIGNUMs, and we do not (in principle)
 * have access to the internals of them, and locking just the structure is
 * not very useful.  Currently, memory locking is not implemented.
 */
#if 0
struct {
	Key	*server_key;		/* ephemeral server key */
	Key	*ssh1_host_key;		/* ssh1 host key */
	Key	**host_keys;		/* all private host keys */
	int	have_ssh1_key;
	int	have_ssh2_key;
	u_char	ssh1_cookie[SSH_SESSION_KEY_LENGTH];
} sensitive_data;
#endif

/*
 * Flag indicating whether the RSA server key needs to be regenerated.
 * Is set in the SIGALRM handler and cleared when the key is regenerated.
 */
//static volatile sig_atomic_t key_do_regen = 0;

/* This is set to true when a signal is received. */
//static volatile sig_atomic_t received_sighup = 0;
//static volatile sig_atomic_t received_sigterm = 0;

/* session identifier, used by RSA-auth */
//static u_char session_id[16]; /*isiah*/

/* same for ssh2 */
//static u_char *session_id2 = NULL; /*isiah*/
//static int session_id2_len = 0; /*isiah*/

/* record remote hostname or ip */
//static u_int utmp_len = MAXHOSTNAMELEN; /*isiah*/

/* options.max_startup sized array of fd ints */
//static int *startup_pipes = NULL; /*isiah*/
//static int startup_pipe;		/* in child */ /*isiah*/

/* variables used for privilege separation */
//static int use_privsep; /*isiah*/
//static struct monitor *pmonitor; /*isiah*/

int close(int fd);

/* Prototypes for various functions defined later in this file. */
void destroy_sensitive_data(void);
//static void demote_sensitive_data(void);

static void do_ssh1_kex(void);
static void do_ssh2_kex(void);

/*
 * Close all listening sockets
 */
#if 0
static void
close_listen_socks(void)
{
	int i;

	for (i = 0; i < num_listen_socks; i++)
		s_close(listen_socks[i]);
	num_listen_socks = -1;
}

static void
close_startup_pipes(void)
{
	int i;

	if (startup_pipes)
		for (i = 0; i < options.max_startups; i++)
			if (startup_pipes[i] != -1)
				s_close(startup_pipes[i]);
}

/*
 * Signal handler for SIGHUP.  Sshd execs itself when it receives SIGHUP;
 * the effect is to reread the configuration file (and to regenerate
 * the server key).
 */
static void
sighup_handler(int sig)
{
	int save_errno = errno;

	received_sighup = 1;
	signal(SIGHUP, sighup_handler);
	errno = save_errno;
}

/*
 * Called from the main program after receiving SIGHUP.
 * Restarts the server.
 */
static void
sighup_restart(void)
{
	log("Received SIGHUP; restarting.");
	close_listen_socks();
	close_startup_pipes();
	execv(saved_argv[0], saved_argv);
	log("RESTART FAILED: av[0]='%.100s', error: %.100s.", saved_argv[0],
	    strerror(errno));
	exit(1);
}

/*
 * Generic signal handler for terminating signals in the master daemon.
 */
static void
sigterm_handler(int sig)
{
	received_sigterm = sig;
}

/*
 * SIGCHLD handler.  This is called whenever a child dies.  This will then
 * reap any zombies left by exited children.
 */
static void
main_sigchld_handler(int sig)
{
	int save_errno = errno;
	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0 ||
	    (pid < 0 && errno == EINTR))
		;

	signal(SIGCHLD, main_sigchld_handler);
	errno = save_errno;
}

/*
 * Signal handler for the alarm after the login grace period has expired.
 */
static void
grace_alarm_handler(int sig)
{
	/* XXX no idea how fix this signal handler */

	/* Log error and exit. */
	fatal("Timeout before authentication for %s", get_remote_ipaddr());
}

/*
 * Signal handler for the key regeneration alarm.  Note that this
 * alarm only occurs in the daemon waiting for connections, and it does not
 * do anything with the private key or random state before forking.
 * Thus there should be no concurrency control/asynchronous execution
 * problems.
 */
static void
generate_ephemeral_server_key(void)
{
	u_int32_t rnd = 0;
	int i;

	verbose("Generating %s%d bit RSA key.",
	    sensitive_data.server_key ? "new " : "", options.server_key_bits);
	if (sensitive_data.server_key != NULL)
		key_free(sensitive_data.server_key);
	sensitive_data.server_key = key_generate(KEY_RSA1,
	    options.server_key_bits);
	verbose("RSA key generation complete.");

/*isiah.2003-04-11*/
/*replace arc4random with RAND_bytes() */
/*	for (i = 0; i < SSH_SESSION_KEY_LENGTH; i++) {
		if (i % 4 == 0)
			rnd = arc4random();
		sensitive_data.ssh1_cookie[i] = rnd & 0xff;
		rnd >>= 8;
	}*/
	RAND_btyes(sensitive_data.ssh1_cookie, SSH_SESSION_KEY_LENGTH);
//	arc4random_stir();
}

static void
key_regeneration_alarm(int sig)
{
	int save_errno = errno;

	signal(SIGALRM, SIG_DFL);
	errno = save_errno;
	key_do_regen = 1;
}
#endif

static void
sshd_exchange_identification(int sock_in, int sock_out)
{
	int i, mismatch;
	int remote_major, remote_minor;
	int major, minor;
	char *s;
	char buf[256];			/* Must not be larger than remote_version. */
	char remote_version[256];	/* Must be at least as big as buf. */
	ServerOptions *options; /*isiah*/
    char *server_version_string = NULL; /*isiah*/
    char *client_version_string = NULL; /*isiah*/
	I32_T datafellows;/*isiah*/
//	int xx_len;
//    fd_set  readfds;
//    struct 	timeval     timeout;

    SSHD_VM_GetSshServerOptions(&options);
	if ((options->protocol & SSH_PROTO_1) &&
	    (options->protocol & SSH_PROTO_2)) {
		major = PROTOCOL_MAJOR_1;
		minor = 99;
	} else if (options->protocol & SSH_PROTO_2) {
		major = PROTOCOL_MAJOR_2;
		minor = PROTOCOL_MINOR_2;
	} else {
		major = PROTOCOL_MAJOR_1;
		minor = PROTOCOL_MINOR_1;
	}
/*isiah. 2003-04-10 */
/* repleaced snprintf() with sprintf() */
/*	snprintf(buf, sizeof buf, "SSH-%d.%d-%.100s\n", major, minor, SSH_VERSION);*/
	sprintf(buf, "SSH-%d.%d-%.100s\n", major, minor, SSH_VERSION);
	server_version_string = xstrdup(buf);

	if (client_version_string == NULL) {
		/* Send our protocol version identification. */

		if (atomicio(send, sock_out, server_version_string,
		    strlen(server_version_string))
		    != strlen(server_version_string)) {

#if 0 /* squid, does not need to log */
			log("Could not write ident string to %s", get_remote_ipaddr());
#endif
			xfree(server_version_string);
			fatal_cleanup();
		}

		/* Read other sides version identification. */
		memset(buf, 0, sizeof(buf));

#if 0
        FD_ZERO(&readfds);
        FD_SET(sock_in, &readfds);
   	    timeout.tv_sec = SSHD_MGR_GetNegotiationTimeout();     /*  no.  of seconds  */
        timeout.tv_usec = 0;    /*  no. of micro seconds  */
        if( select(sock_in + 1, &readfds, NULL, NULL, &timeout) <= 0 )
        {
            write(sock_in,"Negotiation timeout\n",strlen("Negotiation timeout\n"));
            fatal_cleanup();
        }
#endif

		for (i = 0; i < sizeof(buf) - 1; i++) {
			if (atomicio(recv, sock_in, &buf[i], 1) != 1) {
#if 0 /* squid, does not need to log */
				log("Did not receive identification string from %s",
				    get_remote_ipaddr());
#endif
				xfree(server_version_string);
				fatal_cleanup();
			}
			if (buf[i] == '\r') {
				buf[i] = 0;
				/* Kludge for F-Secure Macintosh < 1.0.2 */
				if (i == 12 &&
				    strncmp(buf, "SSH-1.5-W1.0", 12) == 0)
					break;
				continue;
			}
			if (buf[i] == '\n') {
				buf[i] = 0;
				break;
			}
		}
		buf[sizeof(buf) - 1] = 0;
		client_version_string = xstrdup(buf);
	}

	/*
	 * Check that the versions match.  In future this might accept
	 * several versions and set appropriate flags to handle them.
	 */
	if (sscanf(client_version_string, "SSH-%d.%d-%[^\n]\n",
	    &remote_major, &remote_minor, remote_version) != 3) {
		s = "Protocol mismatch.\n";
		(void) atomicio(send, sock_out, s, strlen(s));
		s_close(sock_in);
		s_close(sock_out);

#if 0 /* squid, does not need to log */
		log("Bad protocol version identification '%.100s' from %s",
		    client_version_string, get_remote_ipaddr());
#endif
		xfree(server_version_string);
		xfree(client_version_string);
		fatal_cleanup();
	}
/*	debug("Client protocol version %d.%d; client software version %.100s",
	    remote_major, remote_minor, remote_version);*//*isiah*/

	compat_datafellows(remote_version);

	SSHD_VM_GetSshDataFellows(&datafellows);
	if (datafellows & SSH_BUG_PROBE) {
#if 0 /* squid, does not need to log */
		log("probed from %s with %s.  Don't panic.",
		    get_remote_ipaddr(), client_version_string);
#endif
		xfree(server_version_string);
		xfree(client_version_string);
		fatal_cleanup();
	}

	if (datafellows & SSH_BUG_SCANNER) {
#if 0 /* squid, does not need to log */
		log("scanned from %s with %s.  Don't panic.",
		    get_remote_ipaddr(), client_version_string);
#endif
		xfree(server_version_string);
		xfree(client_version_string);
		fatal_cleanup();
	}

	mismatch = 0;
	switch (remote_major) {
	case 1:
		if (remote_minor == 99)
		{
			if (options->protocol & SSH_PROTO_2)
			{
				enable_compat20();
				SSHD_VM_SetSshConnectionVersion(2, 0);
			}
			else
			{
				mismatch = 1;
			}
			break;
		}
		if (!(options->protocol & SSH_PROTO_1))
		{
			mismatch = 1;
			break;
		}
		if (remote_minor < 3)
		{
			packet_disconnect("Your ssh version is too old and "
			    "is no longer supported.  Please install a newer version.");
		}
		else if (remote_minor == 3)
		{
			/* note that this disables agent-forwarding */
			enable_compat13();
			SSHD_VM_SetSshConnectionVersion(1, 3);
		}
		else
		{
		    SSHD_VM_SetSshConnectionVersion(1, 5);
		}
		break;
	case 2:
		if (options->protocol & SSH_PROTO_2)
		{
			enable_compat20();
			SSHD_VM_SetSshConnectionVersion(2, 0);
			break;
		}
		/* FALLTHROUGH */
	default:
		mismatch = 1;
		break;
	}
	chop(server_version_string);
//	debug("Local version string %.200s", server_version_string);/*isiah*/

	if (mismatch) {
		s = "Protocol major versions differ.\n";
		(void) atomicio(send, sock_out, s, strlen(s));
		s_close(sock_in);
		s_close(sock_out);

#if 0 /* squid, does not need to log */
		log("Protocol major versions differ for %s: %.200s vs. %.200s",
		    get_remote_ipaddr(),
		    server_version_string, client_version_string);
#endif
		xfree(server_version_string);
		xfree(client_version_string);
		fatal_cleanup();
	}
	SSHD_VM_SetSshServerVersionString((I8_T *)server_version_string);
	SSHD_VM_SetSshClientVersionString((I8_T *)client_version_string);
	xfree(server_version_string);
	xfree(client_version_string);
}

/* Destroy the host and server keys.  They will no longer be needed. */
void
destroy_sensitive_data(void)
{
	int i;
	ServerOptions *options; /*isiah*/
	SSHD_SensitiveData_T *sensitive_data;/*isiah*/

    SSHD_VM_GetSshServerOptions(&options);
    SSHD_VM_GetSshSensitiveData(&sensitive_data);

	if (sensitive_data->server_key) {
		key_free(sensitive_data->server_key);
		sensitive_data->server_key = NULL;
	}
	for (i = 0; i < options->num_host_key_files; i++) {
		if (sensitive_data->host_keys[i]) {
			key_free(sensitive_data->host_keys[i]);
			sensitive_data->host_keys[i] = NULL;
		}
	}
	sensitive_data->ssh1_host_key = NULL;
	memset(sensitive_data->ssh1_cookie, 0, SSH_SESSION_KEY_LENGTH);

/*isiah.2003-05-22*/
#if 0
	if(sensitive_data->host_keys)/*isiah*/
	{
	    xfree(sensitive_data->host_keys);
	    sensitive_data->host_keys = NULL;
	}
#endif
}

#if 0
/* Demote private to public keys for network child */
void
demote_sensitive_data(void)
{
	Key *tmp;
	int i;

	if (sensitive_data.server_key) {
		tmp = key_demote(sensitive_data.server_key);
		key_free(sensitive_data.server_key);
		sensitive_data.server_key = tmp;
	}

	for (i = 0; i < options.num_host_key_files; i++) {
		if (sensitive_data.host_keys[i]) {
			tmp = key_demote(sensitive_data.host_keys[i]);
			key_free(sensitive_data.host_keys[i]);
			sensitive_data.host_keys[i] = tmp;
			if (tmp->type == KEY_RSA1)
				sensitive_data.ssh1_host_key = tmp;
		}
	}

	/* We do not clear ssh1_host key and cookie.  XXX - Okay Niels? */
}

static void
privsep_preauth_child(void)
{
	u_int32_t rnd[256];
	gid_t gidset[1];
	struct passwd *pw;
	int i;

	/* Enable challenge-response authentication for privilege separation */
	privsep_challenge_enable();

	for (i = 0; i < 256; i++)
		rnd[i] = arc4random();
	RAND_seed(rnd, sizeof(rnd));

	/* Demote the private keys to public keys. */
	demote_sensitive_data();

	if ((pw = getpwnam(SSH_PRIVSEP_USER)) == NULL)
		fatal("Privilege separation user %s does not exist",
		    SSH_PRIVSEP_USER);
	memset(pw->pw_passwd, 0, strlen(pw->pw_passwd));
	endpwent();

	/* Change our root directory */
	if (chroot(_PATH_PRIVSEP_CHROOT_DIR) == -1)
		fatal("chroot(\"%s\"): %s", _PATH_PRIVSEP_CHROOT_DIR,
		    strerror(errno));
	if (chdir("/") == -1)
		fatal("chdir(\"/\"): %s", strerror(errno));

	/* Drop our privileges */
	debug3("privsep user:group %u:%u", (u_int)pw->pw_uid,
	    (u_int)pw->pw_gid);
#if 0
	/* XXX not ready, to heavy after chroot */
	do_setusercontext(pw);
#else
	gidset[0] = pw->pw_gid;
	if (setgid(pw->pw_gid) < 0)
		fatal("setgid failed for %u", pw->pw_gid );
	if (setgroups(1, gidset) < 0)
		fatal("setgroups: %.100s", strerror(errno));
	permanently_set_uid(pw);
#endif
}

static Authctxt *
privsep_preauth(void)
{
	Authctxt *authctxt = NULL;
	int status;
	pid_t pid;

	/* Set up unprivileged child process to deal with network data */
	pmonitor = monitor_init();
	/* Store a pointer to the kex for later rekeying */
	pmonitor->m_pkex = &xxx_kex;

	pid = fork();
	if (pid == -1) {
		fatal("fork of unprivileged child failed");
	} else if (pid != 0) {
		fatal_remove_cleanup((void (*) (void *)) packet_close, NULL);

		debug2("Network child is on pid %ld", (long)pid);

		s_close(pmonitor->m_recvfd);
		authctxt = monitor_child_preauth(pmonitor);
		s_close(pmonitor->m_sendfd);

		/* Sync memory */
		monitor_sync(pmonitor);

		/* Wait for the child's exit status */
/*		while (waitpid(pid, &status, 0) < 0)
			if (errno != EINTR)
				break;*//*isiah*/

		/* Reinstall, since the child has finished */
		fatal_add_cleanup((void (*) (void *)) packet_close, NULL);

		return (authctxt);
	} else {
		/* child */

		s_close(pmonitor->m_sendfd);

		/* Demote the child */
		if (getuid() == 0 || geteuid() == 0)
			privsep_preauth_child();
		setproctitle("%s", "[net]");
	}
	return (NULL);
}

static void
privsep_postauth(Authctxt *authctxt)
{
//	extern Authctxt *x_authctxt;

	/* XXX - Remote port forwarding */
	x_authctxt = authctxt;

	if (authctxt->pw->pw_uid == 0 || options.use_login) {
		/* File descriptor passing is broken or root login */
		monitor_apply_keystate(pmonitor);
		use_privsep = 0;
		return;
	}

	/* Authentication complete */
	alarm(0);
	if (startup_pipe != -1) {
		s_close(startup_pipe);
		startup_pipe = -1;
	}

	/* New socket pair */
	monitor_reinit(pmonitor);

	pmonitor->m_pid = fork();
	if (pmonitor->m_pid == -1)
		fatal("fork of unprivileged child failed");
	else if (pmonitor->m_pid != 0) {
		fatal_remove_cleanup((void (*) (void *)) packet_close, NULL);

		debug2("User child is on pid %ld", (long)pmonitor->m_pid);
		s_close(pmonitor->m_recvfd);
		monitor_child_postauth(pmonitor);

		/* NEVERREACHED */
		exit(0);
	}

	s_close(pmonitor->m_sendfd);

	/* Demote the private keys to public keys. */
	demote_sensitive_data();

	/* Drop privileges */
	do_setusercontext(authctxt->pw);

	/* It is safe now to apply the key state */
	monitor_apply_keystate(pmonitor);
}
#endif

static char *
list_hostkey_types(void)
{
	Buffer b;
	char *p;
	int i;
	ServerOptions *options; /*isiah*/
	SSHD_SensitiveData_T *sensitive_data;/*isiah*/

    SSHD_VM_GetSshServerOptions(&options);
    SSHD_VM_GetSshSensitiveData(&sensitive_data);
	buffer_init(&b);
	for (i = 0; i < options->num_host_key_files; i++) {
		Key *key = sensitive_data->host_keys[i];
		if (key == NULL)
			continue;
		switch (key->type) {
		case KEY_RSA:
		case KEY_DSA:
			if (buffer_len(&b) > 0)
				buffer_append(&b, ",", 1);
			p = key_ssh_name(key);
			buffer_append(&b, p, strlen(p));
			break;
		}
	}
	buffer_append(&b, "", 1);
	p = xstrdup(buffer_ptr(&b));
	buffer_free(&b);
//	debug("list_hostkey_types: %s", p);/*isiah*/
	return p;
}

Key *
get_hostkey_by_type(int type)
{
	int i;
	ServerOptions *options; /*isiah*/
	SSHD_SensitiveData_T *sensitive_data;/*isiah*/

    SSHD_VM_GetSshServerOptions(&options);
    SSHD_VM_GetSshSensitiveData(&sensitive_data);
	for (i = 0; i < options->num_host_key_files; i++) {
		Key *key = sensitive_data->host_keys[i];
		if (key != NULL && key->type == type)
			return key;
	}
	return NULL;
}

Key *
get_hostkey_by_index(int ind)
{
	ServerOptions *options; /*isiah*/
	SSHD_SensitiveData_T *sensitive_data;/*isiah*/

    SSHD_VM_GetSshServerOptions(&options);
    SSHD_VM_GetSshSensitiveData(&sensitive_data);
	if (ind < 0 || ind >= options->num_host_key_files)
		return (NULL);
	return (sensitive_data->host_keys[ind]);
}

int
get_hostkey_index(Key *key)
{
	int i;
	ServerOptions *options; /*isiah*/
	SSHD_SensitiveData_T *sensitive_data;/*isiah*/

    SSHD_VM_GetSshServerOptions(&options);
    SSHD_VM_GetSshSensitiveData(&sensitive_data);
	for (i = 0; i < options->num_host_key_files; i++) {
		if (key == sensitive_data->host_keys[i])
			return (i);
	}
	return (-1);
}

/*
 * returns 1 if connection should be dropped, 0 otherwise.
 * dropping starts at connection #max_startups_begin with a probability
 * of (max_startups_rate/100). the probability increases linearly until
 * all connections are dropped for startups > max_startups
 */
#if 0
static int
drop_connection(int startups)
{
	double p, r;

	if (startups < options.max_startups_begin)
		return 0;
	if (startups >= options.max_startups)
		return 1;
	if (options.max_startups_rate == 100)
		return 1;

	p  = 100 - options.max_startups_rate;
	p *= startups - options.max_startups_begin;
	p /= (double) (options.max_startups - options.max_startups_begin);
	p += options.max_startups_rate;
	p /= 100.0;
	r = arc4random() / (double) UINT_MAX;

	debug("drop_connection: p %g, r %g", p, r);
	return (r < p) ? 1 : 0;
}

static void
usage(void)
{
	fprintf(stderr, "sshd version %s\n", SSH_VERSION);
	fprintf(stderr, "Usage: %s [options]\n", __progname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -f file    Configuration file (default %s)\n", _PATH_SERVER_CONFIG_FILE);
	fprintf(stderr, "  -d         Debugging mode (multiple -d means more debugging)\n");
	fprintf(stderr, "  -i         Started from inetd\n");
	fprintf(stderr, "  -D         Do not fork into daemon mode\n");
	fprintf(stderr, "  -t         Only test configuration file and keys\n");
	fprintf(stderr, "  -q         Quiet (no logging)\n");
	fprintf(stderr, "  -p port    Listen on the specified port (default: 22)\n");
	fprintf(stderr, "  -k seconds Regenerate server key every this many seconds (default: 3600)\n");
	fprintf(stderr, "  -g seconds Grace period for authentication (default: 600)\n");
	fprintf(stderr, "  -b bits    Size of server RSA key (default: 768 bits)\n");
	fprintf(stderr, "  -h file    File from which to read host key (default: %s)\n",
	    _PATH_HOST_KEY_FILE);
	fprintf(stderr, "  -u len     Maximum hostname length for utmp recording\n");
	fprintf(stderr, "  -4         Use IPv4 only\n");
	fprintf(stderr, "  -6         Use IPv6 only\n");
	fprintf(stderr, "  -o option  Process the option as if it was read from a configuration file.\n");
	exit(1);
}
#endif

/*
 * Main program for the daemon.
 */
void
sshd_main(SSHD_TASK_SessionThreadArgs *args)
{
	int /*opt,*/    sock_in = 0, sock_out = 0, /*newsock, j,*/ i, /*fdsetsz,*/ on = 1;
	int             remote_port;
	Authctxt        *authctxt = NULL;
	Key             *key = NULL;
	ServerOptions   *options = NULL; /*isiah*/
    I32_T           compat20;/*isiah*/
	SSHD_SensitiveData_T *sensitive_data = NULL;/*isiah*/
	UI32_T          index;
	EVP_PKEY        *pkey = NULL;
    int             accepted_socket;

    accepted_socket = args->accepted_socket;
    free(args);

    sock_in     = accepted_socket;
    sock_out    = accepted_socket;

    SSHD_VM_SetSshServerOptions();
    if (   (FALSE == SSHD_VM_GetSshServerOptions(&options))
        || (NULL == options)
        )
    {
        s_close(accepted_socket);
        fatal("SSHD_VM_GetSshServerOptions error");
    }

	/* Initialize configuration options to their default values. */
	initialize_server_options(options);

/*isiah.2003-08-11*/
//	SSLeay_add_all_algorithms();
//	channel_set_af(IPv4or6);

	/*
	 * Force logging to stderr until we have loaded the private host
	 * key (unless started from inetd)
	 */
/*	log_init(__progname,
	    options.log_level == SYSLOG_LEVEL_NOT_SET ?
	    SYSLOG_LEVEL_INFO : options.log_level,
	    options.log_facility == SYSLOG_FACILITY_NOT_SET ?
	    SYSLOG_FACILITY_AUTH : options.log_facility,
	    log_stderr || !inetd_flag);*/

	/* Read server configuration options from the configuration file. */
//	read_server_config(&options, config_file_name); /*isiah*/

	/* Fill in default values for those options not explicitly set. */
	fill_default_server_options(options);

//	debug("sshd version %.100s", SSH_VERSION);/*isiah*/

    if (   (FALSE == SSHD_VM_GetSshSensitiveData(&sensitive_data))
        || (NULL == sensitive_data)
        )
    {
        s_close(accepted_socket);
        fatal("SSHD_VM_GetSshSensitiveData error");
    }

	/* load private host keys */
/*isiah.2003-05-22*/
/*	sensitive_data->host_keys = xmalloc(options->num_host_key_files *
	    sizeof(Key *));*/
	for (i = 0; i < options->num_host_key_files; i++)
		sensitive_data->host_keys[i] = NULL;
	sensitive_data->server_key = NULL;
	sensitive_data->ssh1_host_key = NULL;
	sensitive_data->have_ssh1_key = 0;
	sensitive_data->have_ssh2_key = 0;
	fatal_add_cleanup((void (*) (void *)) destroy_sensitive_data, NULL);

    key = xmalloc(sizeof(*key));
    if( key == NULL )
    {
        s_close(accepted_socket);
        fatal("xmalloc error");
    }
    key->type = KEY_RSA1;
    key->flags = 0;
    key->rsa = (RSA *)KEYGEN_MGR_GetSshdRsaHostkey();
    if( key->rsa == NULL )
    {
        s_close(accepted_socket);
        fatal("KEYGEN_MGR_GetSshdRsaHostkey error");
    }
    sensitive_data->host_keys[0] = key;
	sensitive_data->ssh1_host_key = key;
	sensitive_data->have_ssh1_key = 1;

/*
    key = xmalloc(sizeof(*key));
    if( key == NULL )
    {
        s_close(accepted_socket);
        fatal("");
    }
    key->type = KEY_RSA;
    key->flags = 0;
    key->rsa = (RSA *)KEYGEN_MGR_GetSshdRsaHostkey();
    if( key->rsa == NULL )
    {
        s_close(accepted_socket);
        fatal("");
    }
    sensitive_data->host_keys[1] = key;
*/

    key = xmalloc(sizeof(*key));
    if( key == NULL )
    {
        s_close(accepted_socket);
        fatal("xmalloc error");
    }
    key->type = KEY_DSA;
    key->flags = 0;
    key->dsa = (DSA *)KEYGEN_MGR_GetSshdDsaHostkey();
    if( key->dsa == NULL )
    {
        s_close(accepted_socket);
        fatal("KEYGEN_MGR_GetSshdDsaHostkey error");
    }
    sensitive_data->host_keys[2] = key;
    sensitive_data->have_ssh2_key = 1;

    key = xmalloc(sizeof(*key));
    if( key == NULL )
    {
        s_close(accepted_socket);
        fatal("xmalloc error");
    }
    key->type = KEY_RSA1;
    key->flags = 0;
    SSHD_VM_GetSshServerKeyIndex(&index);
    pkey = (EVP_PKEY *)KEYGEN_MGR_GetSshdServerkey(index);
    if( pkey == NULL )
    {
        s_close(accepted_socket);
        fatal("KEYGEN_MGR_GetSshdServerkey error");
    }
    key->rsa = pkey->pkey.rsa;
/*isiah.2003-05-22*/
    pkey->pkey.rsa = NULL;
    EVP_PKEY_free(pkey);
    sensitive_data->server_key = key;
    options->server_key_bits = BN_num_bits(key->rsa->n);


#if 0
	for (i = 0; i < options.num_host_key_files; i++) {
		key = key_load_private(options.host_key_files[i], "", NULL);
		sensitive_data.host_keys[i] = key;
		if (key == NULL) {
			error("Could not load host key: %s",
			    options.host_key_files[i]);
			sensitive_data.host_keys[i] = NULL;
			continue;
		}
		switch (key->type) {
		case KEY_RSA1:
			sensitive_data.ssh1_host_key = key;
			sensitive_data.have_ssh1_key = 1;
			break;
		case KEY_RSA:
		case KEY_DSA:
			sensitive_data.have_ssh2_key = 1;
			break;
		}
		debug("private host key: #%d type %d %s", i, key->type,
		    key_type(key));
	}
#endif

	if ((options->protocol & SSH_PROTO_1) && !sensitive_data->have_ssh1_key) {
#if 0 /* squid, does not need to log */
		log("Disabling protocol version 1. Could not load host key");
#endif
		options->protocol &= ~SSH_PROTO_1;
	}
	if ((options->protocol & SSH_PROTO_2) && !sensitive_data->have_ssh2_key) {
#if 0 /* squid, does not need to log */
		log("Disabling protocol version 2. Could not load host key");
#endif
		options->protocol &= ~SSH_PROTO_2;
	}
	if (!(options->protocol & (SSH_PROTO_1|SSH_PROTO_2))) {
#if 0 /* squid, does not need to log */
		log("sshd: no hostkeys available -- exiting.");
#endif
		/*exit(1);*//*isiah*/
		s_close(accepted_socket);
		fatal("sshd: no hostkeys available -- exiting.");
	}

	/* Check certain values for sanity. */
	if (options->protocol & SSH_PROTO_1) {
		if (options->server_key_bits < 512 ||
		    options->server_key_bits > 32768) {
/*			fprintf(stderr, "Bad server key size.\n");
			exit(1);*//*isiah*/
			s_close(accepted_socket);
			fatal("Bad server key size.");
		}
		/*
		 * Check that server and host key lengths differ sufficiently. This
		 * is necessary to make double encryption work with rsaref. Oh, I
		 * hate software patents. I dont know if this can go? Niels
		 */
		if (options->server_key_bits >
		    BN_num_bits(sensitive_data->ssh1_host_key->rsa->n) -
		    SSH_KEY_BITS_RESERVED && options->server_key_bits <
		    BN_num_bits(sensitive_data->ssh1_host_key->rsa->n) +
		    SSH_KEY_BITS_RESERVED) {
			options->server_key_bits =
			    BN_num_bits(sensitive_data->ssh1_host_key->rsa->n) +
			    SSH_KEY_BITS_RESERVED;
/*			debug("Forcing server key to %d bits to make it differ from host key.",
			    options.server_key_bits);*//*isiah*/
		}
	}

	/* Set keepalives if requested. */
	if (options->keepalives &&
	    setsockopt(sock_in, SOL_SOCKET, SO_KEEPALIVE, &on,
	    sizeof(on)) < 0)
//		error("setsockopt SO_KEEPALIVE: %.100s", strerror(errno));/*isiah*/
        ;

	/*
	 * Register our connection.  This turns encryption off because we do
	 * not have a key.
	 */
	packet_set_connection(sock_in, sock_out);

	remote_port = get_remote_port();
//	remote_ip = get_remote_ipaddr();/*isiah*/

	/* Log the connection. */
//	verbose("Connection from %.500s port %d", remote_ip, remote_port);/*isiah*/

	/*
	 * We don\'t want to listen forever unless the other side
	 * successfully authenticates itself.  So we set up an alarm which is
	 * cleared after successful authentication.  A limit of zero
	 * indicates no limit. Note that we don\'t set the alarm in debugging
	 * mode; it is just annoying to have the server exit just when you
	 * are about to discover the bug.
	 */

    if ( (SSHD_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) || (SSHD_MGR_GetSshdStatus() == SSHD_STATE_DISABLED) )
    {
        fatal("SSHD not available");
    }
    SSHD_VM_SetSshConnectionStatus(NEGOTIATION_STARTED);
	sshd_exchange_identification(sock_in, sock_out);

	/*
	 * Check that the connection comes from a privileged port.
	 * Rhosts-Authentication only makes sense from privileged
	 * programs.  Of course, if the intruder has root access on his local
	 * machine, he can connect from any port.  So do not use these
	 * authentication methods from machines that you do not trust.
	 */
	if (options->rhosts_authentication &&
	    (remote_port >= IPPORT_RESERVED ||
	    remote_port < IPPORT_RESERVED / 2)) {
/*		debug("Rhosts Authentication disabled, "
		    "originating port %d not trusted.", remote_port);*//*isiah*/
		options->rhosts_authentication = 0;
	}
#if defined(KRB4) && !defined(KRB5)
	if (!packet_connection_is_ipv4() &&
	    options.kerberos_authentication) {
		debug("Kerberos Authentication disabled, only available for IPv4.");
		options.kerberos_authentication = 0;
	}
#endif /* KRB4 && !KRB5 */
#ifdef AFS
	/* If machine has AFS, set process authentication group. */
	if (k_hasafs()) {
		k_setpag();
		k_unlog();
	}
#endif /* AFS */

	packet_set_nonblocking();

/*	if (use_privsep)
		if ((authctxt = privsep_preauth()) != NULL)
			goto authenticated;*/

	/* perform the key exchange */
	/* authenticate user and start session */
    SSHD_VM_GetSshCompat20(&compat20);
	if (compat20) {
		do_ssh2_kex();
        if ( (SSHD_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) || (SSHD_MGR_GetSshdStatus() == SSHD_STATE_DISABLED) )
        {
            fatal("SSHD not available");
        }
        SESSION_ForkTnsh();
        SSHD_VM_SetSshConnectionStatus(AUTHENTICATION_STARTED);
		authctxt = do_authentication2();
	} else {
		do_ssh1_kex();
        if ( (SSHD_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) || (SSHD_MGR_GetSshdStatus() == SSHD_STATE_DISABLED) )
        {
            fatal("SSHD not available");
        }
        SESSION_ForkTnsh();
        SSHD_VM_SetSshConnectionStatus(AUTHENTICATION_STARTED);
		authctxt = do_authentication();
	}
	/*
	 * If we use privilege separation, the unprivileged child transfers
	 * the current keystate and exits
	 */
/*	if (use_privsep) {
		mm_send_keystate(pmonitor);
		exit(0);
	}*//*isiah*/

// authenticated:
	/*
	 * In privilege separation, we fork another child and prepare
	 * file descriptor passing.
	 */
#if 0
	if (use_privsep) {
		privsep_postauth(authctxt);
		/* the monitor process [priv] will not return */
		if (!compat20)
			destroy_sensitive_data();
	}
#endif

	/* Perform session preparation. */
    if ( (SSHD_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) || (SSHD_MGR_GetSshdStatus() == SSHD_STATE_DISABLED) )
    {
        fatal("SSHD not available");
    }
    SSHD_VM_SetSshConnectionStatus(SESSION_STARTED);
	do_authenticated(authctxt);

	/* The connection has been terminated. */
//	verbose("Closing connection to %.100s", remote_ip);/*isiah*/
	packet_close();

/*	if (use_privsep)
		mm_terminate();*//*isiah*/

//	exit(0);/*isiah*/
    fatal("exit");
}

/*
 * Decrypt session_key_int using our private server key and private host key
 * (key with larger modulus first).
 */
int
ssh1_session_key(BIGNUM *session_key_int)
{
	int rsafail = 0;
	SSHD_SensitiveData_T *sensitive_data;/*isiah*/

    SSHD_VM_GetSshSensitiveData(&sensitive_data);
	if (BN_cmp(sensitive_data->server_key->rsa->n, sensitive_data->ssh1_host_key->rsa->n) > 0) {
		/* Server key has bigger modulus. */
		if (BN_num_bits(sensitive_data->server_key->rsa->n) <
		    BN_num_bits(sensitive_data->ssh1_host_key->rsa->n) + SSH_KEY_BITS_RESERVED) {
			fatal("do_connection: %s: server_key %d < host_key %d + SSH_KEY_BITS_RESERVED %d",
			    get_remote_ipaddr(),
			    BN_num_bits(sensitive_data->server_key->rsa->n),
			    BN_num_bits(sensitive_data->ssh1_host_key->rsa->n),
			    SSH_KEY_BITS_RESERVED);
		}
		if (rsa_private_decrypt(session_key_int, session_key_int,
		    sensitive_data->server_key->rsa) <= 0)
			rsafail++;
		if (rsa_private_decrypt(session_key_int, session_key_int,
		    sensitive_data->ssh1_host_key->rsa) <= 0)
			rsafail++;
	} else {
		/* Host key has bigger modulus (or they are equal). */
		if (BN_num_bits(sensitive_data->ssh1_host_key->rsa->n) <
		    BN_num_bits(sensitive_data->server_key->rsa->n) + SSH_KEY_BITS_RESERVED) {
			fatal("do_connection: %s: host_key %d < server_key %d + SSH_KEY_BITS_RESERVED %d",
			    get_remote_ipaddr(),
			    BN_num_bits(sensitive_data->ssh1_host_key->rsa->n),
			    BN_num_bits(sensitive_data->server_key->rsa->n),
			    SSH_KEY_BITS_RESERVED);
		}
		if (rsa_private_decrypt(session_key_int, session_key_int,
		    sensitive_data->ssh1_host_key->rsa) < 0)
			rsafail++;
		if (rsa_private_decrypt(session_key_int, session_key_int,
		    sensitive_data->server_key->rsa) < 0)
			rsafail++;
	}
	return (rsafail);
}
/*
 * SSH1 key exchange
 */
static void
do_ssh1_kex(void)
{
	int i, len;
	int rsafail = 0;
	BIGNUM *session_key_int;
	char session_key[SSH_SESSION_KEY_LENGTH];
	u_char cookie[8];
	u_int cipher_type, auth_mask, protocol_flags;
//	u_int32_t rnd = 0;
	ServerOptions *options; /*isiah*/
    u_char session_id[16]; /*isiah*/
	SSHD_SensitiveData_T *sensitive_data;/*isiah*/

    SSHD_VM_GetSshServerOptions(&options);
    SSHD_VM_GetSshSensitiveData(&sensitive_data);

	/*
	 * Generate check bytes that the client must send back in the user
	 * packet in order for it to be accepted; this is used to defy ip
	 * spoofing attacks.  Note that this only works against somebody
	 * doing IP spoofing from a remote machine; any machine on the local
	 * network can still see outgoing packets and catch the random
	 * cookie.  This only affects rhosts authentication, and this is one
	 * of the reasons why it is inherently insecure.
	 */
/*isiah.2003-04-11*/
/*replace arc4random with RAND_bytes()*/
/*	for (i = 0; i < 8; i++) {
		if (i % 4 == 0)
			rnd = arc4random();
		cookie[i] = rnd & 0xff;
		rnd >>= 8;
	}*/
	RAND_bytes(cookie, 8);

	/*
	 * Send our public key.  We include in the packet 64 bits of random
	 * data that must be matched in the reply in order to prevent IP
	 * spoofing.
	 */
	packet_start(SSH_SMSG_PUBLIC_KEY);
	for (i = 0; i < 8; i++)
		packet_put_char(cookie[i]);

	/* Store our public server RSA key. */
	packet_put_int(BN_num_bits(sensitive_data->server_key->rsa->n));
	packet_put_bignum(sensitive_data->server_key->rsa->e);
	packet_put_bignum(sensitive_data->server_key->rsa->n);

	/* Store our public host RSA key. */
	packet_put_int(BN_num_bits(sensitive_data->ssh1_host_key->rsa->n));
	packet_put_bignum(sensitive_data->ssh1_host_key->rsa->e);
	packet_put_bignum(sensitive_data->ssh1_host_key->rsa->n);

	/* Put protocol flags. */
	packet_put_int(SSH_PROTOFLAG_HOST_IN_FWD_OPEN);

	/* Declare which ciphers we support. */
	packet_put_int(cipher_mask_ssh1(0));

	/* Declare supported authentication types. */
	auth_mask = 0;
	if (options->rhosts_authentication)
		auth_mask |= 1 << SSH_AUTH_RHOSTS;
	if (options->rhosts_rsa_authentication)
		auth_mask |= 1 << SSH_AUTH_RHOSTS_RSA;
	if (options->rsa_authentication)
		auth_mask |= 1 << SSH_AUTH_RSA;
#if defined(KRB4) || defined(KRB5)
	if (options.kerberos_authentication)
		auth_mask |= 1 << SSH_AUTH_KERBEROS;
#endif
#if defined(AFS) || defined(KRB5)
	if (options.kerberos_tgt_passing)
		auth_mask |= 1 << SSH_PASS_KERBEROS_TGT;
#endif
#ifdef AFS
	if (options.afs_token_passing)
		auth_mask |= 1 << SSH_PASS_AFS_TOKEN;
#endif
	if (options->challenge_response_authentication == 1)
		auth_mask |= 1 << SSH_AUTH_TIS;
	if (options->password_authentication)
		auth_mask |= 1 << SSH_AUTH_PASSWORD;
	packet_put_int(auth_mask);

	/* Send the packet and wait for it to be sent. */
	packet_send();
	packet_write_wait();

/*	debug("Sent %d bit server key and %d bit host key.",
	    BN_num_bits(sensitive_data.server_key->rsa->n),
	    BN_num_bits(sensitive_data.ssh1_host_key->rsa->n));*//*isiah*/

	/* Read clients reply (cipher type and session key). */
	packet_read_expect(SSH_CMSG_SESSION_KEY);

	/* Get cipher type and check whether we accept this. */
	cipher_type = packet_get_char();

	if (!(cipher_mask_ssh1(0) & (1 << cipher_type)))
		packet_disconnect("Warning: client selects unsupported cipher.");

	switch( cipher_type )
	{
	    case SSH_CIPHER_3DES:
	        SSHD_VM_SetSshConnectionCipher("3DES");
	        break;

	    case SSH_CIPHER_DES:
	        SSHD_VM_SetSshConnectionCipher("DES");
	        break;
	}

	/* Get check bytes from the packet.  These must match those we
	   sent earlier with the public key packet. */
	for (i = 0; i < 8; i++)
		if (cookie[i] != packet_get_char())
			packet_disconnect("IP Spoofing check bytes do not match.");

//	debug("Encryption type: %.200s", cipher_name(cipher_type));/*isiah*/

	/* Get the encrypted integer. */
	if ((session_key_int = BN_new()) == NULL)
		fatal("do_ssh1_kex: BN_new failed");
	packet_get_bignum(session_key_int);

	protocol_flags = packet_get_int();
	packet_set_protocol_flags(protocol_flags);
	packet_check_eom();

	/* Decrypt session_key_int using host/server keys */
/*	rsafail = PRIVSEP(ssh1_session_key(session_key_int));*//*isiah*/
	rsafail = ssh1_session_key(session_key_int);

	/*
	 * Extract session key from the decrypted integer.  The key is in the
	 * least significant 256 bits of the integer; the first byte of the
	 * key is in the highest bits.
	 */
	if (!rsafail) {
		BN_mask_bits(session_key_int, sizeof(session_key) * 8);
		len = BN_num_bytes(session_key_int);
		if (len < 0 || len > sizeof(session_key)) {
/*			error("do_connection: bad session key len from %s: "
			    "session_key_int %d > sizeof(session_key) %lu",
			    get_remote_ipaddr(), len, (u_long)sizeof(session_key));*//*isiah*/
			rsafail++;
		} else {
			memset(session_key, 0, sizeof(session_key));
			BN_bn2bin(session_key_int,
			    (unsigned char *)(session_key + sizeof(session_key) - len));

			compute_session_id(session_id, cookie,
			    sensitive_data->ssh1_host_key->rsa->n,
			    sensitive_data->server_key->rsa->n);
			/*
			 * Xor the first 16 bytes of the session key with the
			 * session id.
			 */
			for (i = 0; i < 16; i++)
				session_key[i] ^= session_id[i];
		}
	}
	if (rsafail) {
		int bytes = BN_num_bytes(session_key_int);
		u_char *buf = xmalloc(bytes);
		MD5_CTX md;

#if 0 /* squid, does not need to log */
		log("do_connection: generating a fake encryption key");
#endif
		BN_bn2bin(session_key_int, buf);
		MD5_Init(&md);
		MD5_Update(&md, buf, bytes);
		MD5_Update(&md, sensitive_data->ssh1_cookie, SSH_SESSION_KEY_LENGTH);
		MD5_Final((unsigned char *)session_key, &md);
		MD5_Init(&md);
		MD5_Update(&md, session_key, 16);
		MD5_Update(&md, buf, bytes);
		MD5_Update(&md, sensitive_data->ssh1_cookie, SSH_SESSION_KEY_LENGTH);
		MD5_Final((unsigned char *)session_key + 16, &md);
		memset(buf, 0, bytes);
		xfree(buf);
		for (i = 0; i < 16; i++)
			session_id[i] = session_key[i] ^ session_key[i + 16];
	}
	/* Destroy the private and public keys. No longer. */
//	destroy_sensitive_data(); /*isiah*/

    SSHD_VM_SetSshSessionId(session_id);

/*	if (use_privsep)
		mm_ssh1_session_id(session_id);*//*isiah*/

	/* Destroy the decrypted integer.  It is no longer needed. */
	BN_clear_free(session_key_int);

	/* Set the session key.  From this on all communications will be encrypted. */
	packet_set_encryption_key(session_key, SSH_SESSION_KEY_LENGTH, cipher_type);

	/* Destroy our copy of the session key.  It is no longer needed. */
	memset(session_key, 0, sizeof(session_key));

//	debug("Received session key; encryption turned on.");/*isiah*/

	/* Send an acknowledgment packet.  Note that this packet is sent encrypted. */
	packet_start(SSH_SMSG_SUCCESS);
	packet_send();
	packet_write_wait();
}

/*
 * SSH2 key exchange: diffie-hellman-group1-sha1
 */
static void
do_ssh2_kex(void)
{
    char *myproposal[PROPOSAL_MAX] = {
	    KEX_DEFAULT_KEX,
	    KEX_DEFAULT_PK_ALG,
	    KEX_DEFAULT_ENCRYPT,
	    KEX_DEFAULT_ENCRYPT,
	    KEX_DEFAULT_MAC,
	    KEX_DEFAULT_MAC,
	    KEX_DEFAULT_COMP,
	    KEX_DEFAULT_COMP,
	    KEX_DEFAULT_LANG,
	    KEX_DEFAULT_LANG
    };
	Kex *kex;
	ServerOptions *options; /*isiah*/
    char *server_version_string = NULL; /*isiah*/
    char *client_version_string = NULL; /*isiah*/
    u_char *session_id2 = NULL; /*isiah*/
    int session_id2_len = 0; /*isiah*/

    SSHD_VM_GetSshServerOptions(&options);
	if (options->ciphers != NULL) {
		myproposal[PROPOSAL_ENC_ALGS_CTOS] =
		myproposal[PROPOSAL_ENC_ALGS_STOC] = options->ciphers;
	}
	myproposal[PROPOSAL_ENC_ALGS_CTOS] =
	    compat_cipher_proposal(myproposal[PROPOSAL_ENC_ALGS_CTOS]);
	myproposal[PROPOSAL_ENC_ALGS_STOC] =
	    compat_cipher_proposal(myproposal[PROPOSAL_ENC_ALGS_STOC]);

	if (options->macs != NULL) {
		myproposal[PROPOSAL_MAC_ALGS_CTOS] =
		myproposal[PROPOSAL_MAC_ALGS_STOC] = options->macs;
	}
	if (!options->compression) {
		myproposal[PROPOSAL_COMP_ALGS_CTOS] =
		myproposal[PROPOSAL_COMP_ALGS_STOC] = "none";
	}
	myproposal[PROPOSAL_SERVER_HOST_KEY_ALGS] = list_hostkey_types();

	SSHD_VM_GetSshServerVersionString((I8_T **)&server_version_string);
	SSHD_VM_GetSshClientVersionString((I8_T **)&client_version_string);

	/* start key exchange */
	kex = kex_setup(myproposal);
	kex->kex[KEX_DH_GRP1_SHA1] = kexdh_server;
	kex->kex[KEX_DH_GEX_SHA1] = kexgex_server;
	kex->server = 1;
	kex->client_version_string=client_version_string;
	kex->server_version_string=server_version_string;
	kex->load_host_key=&get_hostkey_by_type;
	kex->host_key_index=&get_hostkey_index;

//	xxx_kex = kex;/*isiah*/
	SSHD_VM_SetSshKex(kex);

	dispatch_run(DISPATCH_BLOCK, &kex->done, kex);

	session_id2 = kex->session_id;
	session_id2_len = kex->session_id_len;
	SSHD_VM_SetSshSessionId2(session_id2, session_id2_len);

#ifdef DEBUG_KEXDH
	/* send 1st encrypted/maced/compressed message */
	packet_start(SSH2_MSG_IGNORE);
	packet_put_cstring("markus");
	packet_send();
	packet_write_wait();
#endif
//	debug("KEX done");/*isiah*/
    /*isiah.2003-08-22*/
    xfree(myproposal[PROPOSAL_SERVER_HOST_KEY_ALGS]);
}

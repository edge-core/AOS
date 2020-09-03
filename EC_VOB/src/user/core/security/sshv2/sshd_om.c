/* MODULE NAME:  sshd_om.c
* PURPOSE:
*   Initialize the database resource and provide some get/set functions for accessing the
*   sshd database.
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2002-05-24      -- Isiah , created.
*
* Copyright(C)      Accton Corporation, 2002
*/

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "sshd_om.h"
#include "sys_adpt.h"
#include "xmalloc.h"
#include "uuencode.h"
#include "keygen_type.h"


/* NAMING CONSTANT DECLARATIONS
 */
#ifndef DISPATCH_MAX
#define DISPATCH_MAX    255
#endif
#ifndef MAX_SESSIONS
#define MAX_SESSIONS 10
#endif



/* MACRO FUNCTION DECLARATIONS
 */
#define SSHD_OM_ENTER_CRITICAL_SECTION() { \
        sshd_om_orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(sshd_om_sem_id); \
    }

#define SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(RET_VAL) { \
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(sshd_om_sem_id,sshd_om_orig_priority); \
        return (RET_VAL); \
    }

#define SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE() { \
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(sshd_om_sem_id,sshd_om_orig_priority); \
        return; \
    }


/* DATA TYPE DECLARATIONS
 */
typedef struct SSHD_ServerOptions_S
{
    UI32_T          task_id;
    SSHD_SensitiveData_T    sensitive_data;
/* Server configuration options. */
    ServerOptions   options;
/*
 * the client's version string, passed by sshd2 in compat mode. if != NULL,
 * sshd will skip the version-number exchange
 */
    I8_T            *client_version_string;
    I8_T            *server_version_string;
/* for rekeying XXX fixme */
    Kex             *xxx_kex;
/* session identifier, used by RSA-auth */
    UI8_T           session_id[16];
/* same for ssh2 */
    UI8_T           *session_id2;
    I32_T           session_id2_len;
/*
 * This variable contains the file descriptors used for communicating with
 * the other side.  connection_in is used for reading; connection_out for
 * writing.  These can be the same descriptor, in which case it is assumed to
 * be a socket.
 */
    I32_T           connection_in;
    I32_T           connection_out;
/* Encryption context for receiving data.  This is only used for decryption. */
    CipherContext   receive_context;
/* Encryption context for sending data.  This is only used for encryption. */
    CipherContext   send_context;
/* Session key information for Encryption and MAC */
    Newkeys         *newkeys[MODE_MAX];
    Newkeys         *current_keys[MODE_MAX];
/* Buffer for raw input data from the socket. */
    Buffer          input;
/* Buffer for raw output data going to the socket. */
    Buffer          output;
/* Buffer for the partial outgoing packet being constructed. */
    Buffer          outgoing_packet;
/* Buffer for the incoming packet currently being processed. */
    Buffer          incoming_packet;

    I32_T           datafellows;
    I32_T           compat20;
    I32_T           compat13;

/*
 * Pointer to an array containing all allocated channels.  The array is
 * dynamically extended as needed.
 */
    Channel         **channels;
/*
 * Size of the channel array.  All slots of the array must always be
 * initialized (at least the type field); unused slots set to NULL
 */
    I32_T           channels_alloc;
/*
 * Maximum file descriptor value used in any of the channels.  This is
 * updated in channel_new.
 */
    I32_T           channel_max_fd;

    dispatch_fn     *dispatch[DISPATCH_MAX];

/* Protocol flags for the remote side. */
    UI32_T          remote_protocol_flags;
#if 0
/* Session key for protocol v1 */
    UI8_T           ssh1_key[SSH_SESSION_KEY_LENGTH];
    UI32_T          ssh1_keylen;
#endif
/* Set to true if the connection is interactive. */
    I32_T           interactive_mode;
/* default maximum packet size */
    I32_T           max_packet_size;/*32768*/
/*
 * 'channel_pre*' are called just before select() to add any bits relevant to
 * channels in the select bitmasks.
 */
    chan_fn         *channel_pre[SSH_CHANNEL_MAX_TYPE];
/*
 * 'channel_post*': perform any appropriate operations for channels which
 * have events pending.
 */
    chan_fn         *channel_post[SSH_CHANNEL_MAX_TYPE];

    I32_T           channel_handler_init;
    I32_T           had_channel;
    Buffer          stdin_buffer;   /* Buffer for stdin data. */
    Buffer          stdout_buffer;  /* Buffer for stdout data. */
    Buffer          stderr_buffer;  /* Buffer for stderr data. */
    I32_T           fdin;       /* Descriptor for stdin (for writing) */
    I32_T           fdout;      /* Descriptor for stdout (for reading);*/
    I32_T           fderr;      /* Descriptor for stderr.  May be -1. */
    I32_T           stdin_eof;  /* EOF message received from client. */
    I32_T           fdout_eof;  /* EOF encountered reading from fdout. */
    I32_T           fderr_eof;  /* EOF encountered readung from fderr. */
    I32_T           connection_closed;  /* Connection to client closed. */
    UI32_T          buffer_high;    /* "Soft" max buffer size. */
    I32_T           client_alive_timeouts;
    Authctxt        *xxx_authctxt;
    Session         sessions[MAX_SESSIONS];
    I32_T           session_did_init;
    struct fatal_cleanup *fatal_cleanups;
    UI32_T          read_seqnr;
    UI32_T          send_seqnr;

    SSHD_ConnectionState_T  connection_state;
    UI32_T          connection_major_version;
    UI32_T          connection_minor_version;
    UI8_T           connection_cipher_string[SSHD_MAX_SSH_CIPHER_STRING_LEN+1];
    UI8_T           connection_username[SYS_ADPT_MAX_USER_NAME_LEN+1];
    UI32_T          connection_id; /* -1 */
    UI32_T          tnsh_tid;   /* the tnsh task-id associated with this socket connection. */
    int             tnsh_fd;            /* socket ID that connects to tnsh */
    L_INET_AddrIp_T user_addr;          /*  remote ip(IPv4, IPv6) of user */
    UI32_T          user_port;          /*  remote port of user     */
    UI32_T          user_local_port;    /*  local side port to user */
    UI32_T          tnsh_port;          /*  port connect to TNSHD   */
    UI32_T          remote_tnsh_port;   /*  remote side(tnsh) port to TNSHD */

    USERAUTH_AuthResult_T   auth_result;
    char            password[ SYS_ADPT_MAX_PASSWORD_LEN + 1];   /* for authorization, it should be encrypted */

    UI32_T          packet_length;
    UI8_T           mac_buffer[EVP_MAX_MD_SIZE];
    UI8_T           kexgex_hash_digest[EVP_MAX_MD_SIZE];
    UI8_T           kex_dh_hash_digest[EVP_MAX_MD_SIZE];

}SSHD_ServerOptions_T;



/* LOCAL SUBPROGRAM DECLARATIONS
 */



/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T   sshd_om_sem_id;
static UI32_T   sshd_om_orig_priority;

static UI32_T   sshd_port_number = SSHD_DEFAULT_PORT_NUMBER;
static SSHD_State_T sshd_status = SSHD_DEFAULT_STATE;
static UI32_T   sshd_om_server_key_size = SSHD_DEFAULT_SERVER_KEY_SIZE;
static UI32_T   sshd_om_authentication_retries = SSHD_DEFAULT_AUTHENTICATION_RETRIES;
static UI32_T   sshd_om_timeout = SSHD_DEFAULT_NEGOTIATION_TIMEOUT;
static UI32_T   sshd_om_server_major_version = 2;
static UI32_T   sshd_om_server_minor_version = 0;
static SSHD_ServerOptions_T   sshd_om_options[SYS_ADPT_MAX_SSH_NUMBER];
static UI32_T   keeper_task_id = 0;
static UI32_T   sshd_om_created_session_count = 0;
static UI32_T   sshd_om_generate_host_key_action = NON_GENERATE_KEY;
static UI32_T   sshd_om_generate_host_key_status = UNKNOW_STATE;
static UI32_T   sshd_om_delete_user_key_action = NON_DELETE_KEY;
static UI32_T   sshd_om_delete_user_key_status = UNKNOW_STATE;
static UI32_T   sshd_om_save_host_key_action = NON_SAVE;
static UI32_T   sshd_om_save_host_key_status = UNKNOW_STATE;
static SSHD_PasswordAuthenticationStatus_T  sshd_om_password_authentication_status = SSHD_DEFAULT_PASSWORD_AUTHENTICATION_STATE;
static SSHD_PubkeyAuthenticationStatus_T  sshd_om_pubkey_authentication_status = SSHD_DEFAULT_PUBKEY_AUTHENTICATION_STATE;
static SSHD_RsaAuthenticationStatus_T  sshd_om_rsa_authentication_status = SSHD_DEFAULT_RSA_AUTHENTICATION_STATE;


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SSHD_OM_Init
 * PURPOSE:
 *          Initiate the semaphore for SSHD objects
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in SSHD_MGR_Init.
 */
BOOL_T SSHD_OM_Init(void)
{
    UI32_T  i;

    memset(sshd_om_options, 0, sizeof(SSHD_ServerOptions_T)*SYS_ADPT_MAX_SSH_NUMBER);
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        sshd_om_options[i].max_packet_size = 32768;
        sshd_om_options[i].connection_id = -1; /* -1 */
        sshd_om_options[i].connection_in = -1;
        sshd_om_options[i].connection_out = -1;
        sshd_om_options[i].auth_result.privilege = 0xff;
        sshd_om_options[i].auth_result.authen_by_whom = USERAUTH_AUTH_NONE;
        sshd_om_options[i].auth_result.method = USERAUTH_UNKNOWN;
        sshd_om_options[i].tnsh_fd = -1;
    }

    if(SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SSHD_OM, &sshd_om_sem_id)!=SYSFUN_OK)
    {
        printf("%s:get om sem id fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

#if 0
/* FUNCTION NAME:  SSHD_OM_EnterCriticalSection
 * PURPOSE:
 *          Enter critical section before a task invokes the sshd objects.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_EnterCriticalSection(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if (SYSFUN_GetSem(sshd_om_semaphore_id, SYSFUN_TIMEOUT_WAIT_FOREVER) != SYSFUN_OK)
    {
        // Error
        return FALSE;
    }

    return TRUE;
}



/* FUNCTION NAME:  SSHD_OM_LeaveCriticalSection
 * PURPOSE:
 *          Leave critical section after a task invokes the sshd objects.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_LeaveCriticalSection(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if (SYSFUN_SetSem(sshd_om_semaphore_id) != SYSFUN_OK)
    {
        // Error
        return FALSE;
    }

    return TRUE;
}
#endif

/* FUNCTION NAME:  SSHD_OM_SetSshdStatus
 * PURPOSE:
 *          This function set sshd state.
 *
 * INPUT:
 *          SSHD_State_T - SSHD status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_SetSshdStatus(SSHD_State_T state)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    if (state != sshd_status)
    {
        sshd_status = state;
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
}

/* FUNCTION NAME:  SSHD_OM_GetSshdStatus
 * PURPOSE:
 *          This function get sshd state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SSHD_State_T - SSHD status.
 * NOTES:
 *          .
 */
SSHD_State_T SSHD_OM_GetSshdStatus(void)
{
    SSHD_State_T    ret;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    ret = sshd_status;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshdPort
 * PURPOSE:
 *          This function set sshd port number.
 *
 * INPUT:
 *          UI32_T - SSHD port number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshdPort (UI32_T port)
{
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    if ((port != SYS_DFLT_TELNET_SOCKET_PORT) &&
        (port != sshd_port_number))
    {
        sshd_port_number = port;
        ret = TRUE;
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshdPort
 * PURPOSE:
 *          This function get sshd port number.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - SSHD port value.
 * NOTES:
 *          default is tcp/22.
 */
UI32_T SSHD_OM_GetSshdPort(void)
{
    UI32_T  ret;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    ret = sshd_port_number;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

#if 0
/* FUNCTION NAME:  SSHD_OM_ResetSshdSessionRecord
 * PURPOSE:
 *          This function reset sshd session record.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_ResetSshdSessionRecord(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  index;

    /* BODY */
    memset(&sshd_session_context,0,sizeof(SSHD_Session_Record_T));
    for ( index=1 ; index<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; index++ )
    {
        sshd_session_context.connection[index].connection_id = -1;
    }
    return ;
}
#endif

/* FUNCTION NAME:  SSHD_OM_SetAuthenticationRetries
 * PURPOSE:
 *          This function set number of retries for authentication user.
 *
 * INPUT:
 *          UI32_T -- number of retries for authentication user.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetAuthenticationRetries(UI32_T retries)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_authentication_retries = retries;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME:  SSHD_OM_GetAuthenticationRetries
 * PURPOSE:
 *          This function get number of retries for authentication user.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T --  number of retries for authentication user.
 * NOTES:
 *          .
 */
UI32_T SSHD_OM_GetAuthenticationRetries(void)
{
    UI32_T  ret;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    ret = sshd_om_authentication_retries;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetNegotiationTimeout
 * PURPOSE:
 *          This function set number of negotiation timeout .
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetNegotiationTimeout(UI32_T timeout)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_timeout = timeout;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME:  SSHD_OM_GetNegotiationTimeout
 * PURPOSE:
 *          This function get number of negotiation timeout .
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T --  number of negotiation timeout .
 * NOTES:
 *          .
 */
UI32_T SSHD_OM_GetNegotiationTimeout(void)
{
    UI32_T  ret;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    ret = sshd_om_timeout;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

#if 0
/* FUNCTION NAME:  SSHD_OM_GetSshdSessionRecord
 * PURPOSE:
 *          This function get sshd session record pointer.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SSHD_Session_Record_T - SSHD session record pointer.
 * NOTES:
 *          .
 */
SSHD_Session_Record_T *SSHD_OM_GetSshdSessionRecord(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    return &sshd_session_context;
}
#endif

/* FUNCTION NAME:  SSHD_OM_GetCreatedSessionNumber
 * PURPOSE:
 *          This function get number of  ssh connection.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T -- number of  ssh connection.
 * NOTES:
 *          .
 */
UI32_T SSHD_OM_GetCreatedSessionNumber(void)
{
    UI32_T  ret;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    ret = sshd_om_created_session_count;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetCreatedSessionNumber
 * PURPOSE:
 *          This function set number of  ssh connection.
 * INPUT:
 *          UI32_T -- number of  ssh connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_SetCreatedSessionNumber(UI32_T number)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_created_session_count = number;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
}

#if 0
/* FUNCTION NAME:  SSHD_OM_SetTaskID
 * PURPOSE:
 *          This function set task id to session record.
 * INPUT:
 *          UI32_T -- index of session record.
 *          UI32_T -- task id.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_SetTaskID(UI32_T index, UI32_T tid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    sshd_session_context.connection[index].keepalive = 1;
    sshd_session_context.connection[index].tid = tid;
    sshd_session_context.connection[index].connection_id = -1;
    sshd_session_context.connection[index].user_ip = 0;
    sshd_session_context.connection[index].user_port = 0;
    sshd_session_context.connection[index].user_local_port = 0;
    sshd_session_context.connection[index].tnsh_port = 0;
    sshd_session_context.connection[index].remote_tnsh_port = 0;

    return ;
}



/* FUNCTION NAME:  SSHD_OM_GetTaskID
 * PURPOSE:
 *          This function get task id from session record.
 * INPUT:
 *          UI32_T -- index of session record.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T -- task id.
 * NOTES:
 *          .
 */
UI32_T SSHD_OM_GetTaskID(UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    return sshd_session_context.connection[index].tid;
}
#endif

#if 0
/* FUNCTION NAME:  SSHD_OM_GetContextAddress
 * PURPOSE:
 *          This function get ssh_context pointer.
 * INPUT:
 *          UI32_T -- index of session record.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SSHD_Context_T * -- ssh_context pointer.
 * NOTES:
 *          .
 */
SSHD_Context_T *SSHD_OM_GetContextAddress(UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    return &(sshd_session_context.connection[index].ssh_context);
}

/* FUNCTION NAME:  SSHD_OM_ResetTaskID
 * PURPOSE:
 *          This function reset task id to session record.
 * INPUT:
 *          UI32_T -- index of session record.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_ResetTaskID(UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    sshd_session_context.connection[index].keepalive = 0;
    sshd_session_context.connection[index].tid = 0;
    sshd_session_context.connection[index].connection_id = -1;
    sshd_session_context.connection[index].user_ip = 0;
    sshd_session_context.connection[index].user_port = 0;
    sshd_session_context.connection[index].user_local_port = 0;
    sshd_session_context.connection[index].tnsh_port = 0;
    sshd_session_context.connection[index].remote_tnsh_port = 0;

    return ;
}
#endif

/* FUNCTION NAME:  SSHD_OM_SetSshServerVersion
 * PURPOSE:
 *          This function set version of ssh server.
 *
 * INPUT:
 *          UI32_T - number of major version.
 *          UI32_T - number of minor version.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshServerVersion(UI32_T major, UI32_T minor)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_server_major_version = major;
    sshd_om_server_minor_version = minor;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME:  SSHD_OM_GetSshServerVersion
 * PURPOSE:
 *          This function get version of ssh server.
 *
 * INPUT:
 *
 * OUTPUT:
 *          UI32_T * - number of major version.
 *          UI32_T * - number of minor version.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshServerVersion(UI32_T *major, UI32_T *minor)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();

    *major = sshd_om_server_major_version;
    *minor = sshd_om_server_minor_version;

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

#if 0
/* FUNCTION NAME : SSHD_OM_GetNextActiveConnectionID
 * PURPOSE:
 *      Get next active connection id .
 *
 * INPUT:
 *      UI32_T * -- previous active connection id.
 *
 * OUTPUT:
 *      UI32_T * -- current active connection id.
 *
 * RETURN:
 *      TRUE  - The output value is current active connection id.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      Initial input value is -1.
 */
BOOL_T SSHD_OM_GetNextActiveConnectionID(I32_T *cid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  i;

    /* BODY */
    while (1)
    {
        *cid = *cid + 1;
        if ( *cid >= SSHD_DEFAULT_MAX_SESSION_NUMBER )
        {
            return FALSE;
        }

        for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
        {
            if ( sshd_session_context.connection[i].connection_id == *cid )
            {
                if ( sshd_session_context.connection[i].keepalive == 1 )
                {
                    sshd_session_context.connection[i].keepalive = 0;
                    return TRUE;
                }
            }
        }/* end of for loop */
    }/* end of while(1) loop */
}
#endif

/* FUNCTION NAME : SSHD_OM_GetNextSshConnectionEntry
 * PURPOSE:
 *      Get next active connection entry.
 *
 * INPUT:
 *      UI32_T * -- previous active connection id.
 *
 * OUTPUT:
 *      UI32_T * -- current active connection id.
 *      SSHD_ConnectionInfo_T * -- current active connection information.
 *
 * RETURN:
 *      TRUE  - The output value is current active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in CLI command "show ssh".
 *      Initial input value is -1.
 */
BOOL_T SSHD_OM_GetNextSshConnectionEntry(I32_T *cid, SSHD_ConnectionInfo_T *info)
{
    UI32_T  i;
    UI32_T  start_idx;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    start_idx = 0;
    for ( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if ( sshd_om_options[i].task_id != 0 )
        {
            if ( sshd_om_options[i].connection_id == *cid )
            {
                start_idx = i+1;
                break;
            }
        }
    }

    for ( i=start_idx ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if ( sshd_om_options[i].task_id != 0 )
        {
            *cid = sshd_om_options[i].connection_id;

            info->connection_id = sshd_om_options[i].connection_id;
            info->major_version = sshd_om_options[i].connection_major_version;
            info->minor_version = sshd_om_options[i].connection_minor_version;
            info->status = sshd_om_options[i].connection_state;
            memset(info->cipher, 0, SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN+1);
            strncpy((char *)info->cipher, (char *)sshd_om_options[i].connection_cipher_string, SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN);
            memset(info->username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
            strncpy((char *)info->username, (char *)sshd_om_options[i].connection_username, SYS_ADPT_MAX_USER_NAME_LEN);

            ret = TRUE;
            break;
        }
    }/* end of for loop */

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshConnectionEntry
 * PURPOSE:
 *      Get specify active connection entry.
 *
 * INPUT:
 *      UI32_T   -- specify active connection id.
 *
 * OUTPUT:
 *      SSHD_ConnectionInfo_T * -- specify active connection information.
 *
 * RETURN:
 *      TRUE  - The output value is specify active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in SNMP.
 */
BOOL_T SSHD_OM_GetSshConnectionEntry(UI32_T cid, SSHD_ConnectionInfo_T *info)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for ( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if ( sshd_om_options[i].connection_id == cid )
        {
            info->connection_id = sshd_om_options[i].connection_id;
            info->major_version = sshd_om_options[i].connection_major_version;
            info->minor_version = sshd_om_options[i].connection_minor_version;
            info->status = sshd_om_options[i].connection_state;
            memset(info->cipher, 0, SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN+1);
            strncpy((char *)info->cipher, (char *)sshd_om_options[i].connection_cipher_string, SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN);
            memset(info->username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
            strncpy((char *)info->username, (char *)sshd_om_options[i].connection_username, SYS_ADPT_MAX_USER_NAME_LEN);

            ret = TRUE;
            break;
        }
    }/* end of for loop */

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetServerKeySize
 * PURPOSE:
 *          This function set number of bits for server key.
 *
 * INPUT:
 *          UI32_T key_size --  number of bits for server key. The number of key size range is
 *                              512 to 896 bits.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - Fault.
 * NOTES:
 *          This function maybe invoked in CLI command 'ip ssh server-key size'.
 */
BOOL_T SSHD_OM_SetServerKeySize(UI32_T key_size)
{
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    if( (key_size <= SSHD_MAX_SERVER_KEY_SIZE) && (key_size >= SSHD_MIN_SERVER_KEY_SIZE) )
    {
        sshd_om_server_key_size = key_size;
        ret = TRUE;
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetServerKeySize
 * PURPOSE:
 *          This function get number of bits for server key.
 *
 * INPUT:
 *          UI32_T *key_size    --  number of bits for server key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - Fault.
 * NOTES:
 *
 */
BOOL_T SSHD_OM_GetServerKeySize(UI32_T *key_size)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    *key_size = sshd_om_server_key_size;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME:  SSHD_OM_GetSshServerOptions
 * PURPOSE:
 *          This function get ssh server options pointer.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          ServerOptions   **options   -- SSH server options pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshServerOptions(UI32_T my_tid, ServerOptions **options)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *options = &(sshd_om_options[i].options);
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshServerOptions
 * PURPOSE:
 *          This function alloc ssh server options pointer and set owner tid.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshServerOptions(UI32_T my_tid)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == 0 )
        {
            sshd_om_options[i].task_id = my_tid;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshServerVersionString
 * PURPOSE:
 *          This function set ssh server version string.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I8_T    *version_string --  server version string.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshServerVersionString(UI32_T my_tid, I8_T *version_string)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].server_version_string = (I8_T *)malloc(strlen((char *)version_string)+1);
            strcpy((char *)sshd_om_options[i].server_version_string,(char *) version_string);
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshClientVersionString
 * PURPOSE:
 *          This function set ssh client version string.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I8_T    *version_string --  client version string.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshClientVersionString(UI32_T my_tid, I8_T *version_string)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].client_version_string = (I8_T *)malloc(strlen((char *)version_string)+1);
            strcpy((char *)sshd_om_options[i].client_version_string, (char *)version_string);
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshSessionId
 * PURPOSE:
 *          This function set ssh session id for sshv1.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI8_T   *session_id --  session id.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshSessionId(UI32_T my_tid, UI8_T *session_id)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            memcpy(sshd_om_options[i].session_id, session_id, 16);
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshSessionId
 * PURPOSE:
 *          This function get ssh session id for sshv1.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          UI8_T    *session_id --  session id.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshSessionId(UI32_T my_tid, UI8_T *session_id)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            memcpy(session_id, sshd_om_options[i].session_id, 16);
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshServerVersionString
 * PURPOSE:
 *          This function get ssh server version string pointer.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I8_T    **version_string    --  server version string pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshServerVersionString(UI32_T my_tid, I8_T **version_string)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *version_string = sshd_om_options[i].server_version_string;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshClientVersionString
 * PURPOSE:
 *          This function get ssh client version string pointer.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I8_T    **version_string    --  client version string pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshClientVersionString(UI32_T my_tid, I8_T **version_string)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *version_string = sshd_om_options[i].client_version_string;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshKex
 * PURPOSE:
 *          This function set ssh kex pointer.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          Kex     *xxx_kex    --  kex pointer.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshKex(UI32_T my_tid, Kex *xxx_kex)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].xxx_kex = xxx_kex;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshKex
 * PURPOSE:
 *          This function get ssh kex pointer.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          Kex **xxx_kex   --  kex pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshKex(UI32_T my_tid, Kex **xxx_kex)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *xxx_kex = sshd_om_options[i].xxx_kex;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshSessionId2
 * PURPOSE:
 *          This function set ssh session id and id length for sshv2.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          UI8_T   *session_id2    --  session id.
 *          I32_T   session_id2_len --  id length.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshSessionId2(UI32_T my_tid, UI8_T *session_id2, I32_T session_id2_len)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].session_id2 = session_id2;
            sshd_om_options[i].session_id2_len = session_id2_len;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshSessionId2
 * PURPOSE:
 *          This function get ssh session id and id length for sshv2.
 * INPUT:
 *          UI32_T  my_tid              --  caller tid.
 *
 * OUTPUT:
 *          UI8_T   **session_id2       --  session id.
 *          I32_T   *session_id2_len    --  id length.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshSessionId2(UI32_T my_tid, UI8_T **session_id2, I32_T *session_id2_len)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *session_id2 = sshd_om_options[i].session_id2;
            *session_id2_len = sshd_om_options[i].session_id2_len;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshConnectionSockId
 * PURPOSE:
 *          This function set socket id for communicating with
 *          the other side.  connection_in is used for reading;
 *          connection_out for writing.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I32_T   connection_in   --  connection_in is used for reading.
 *          I32_T   connection_out  --  connection_in is used for writing.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionSockId(UI32_T my_tid, I32_T connection_in, I32_T connection_out)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].connection_in = connection_in;
            sshd_om_options[i].connection_out = connection_out;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshConnectionSockId
 * PURPOSE:
 *          This function get socket id for communicating with
 *          the other side.  connection_in is used for reading;
 *          connection_out for writing.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *connection_in  --  connection_in is used for reading.
 *          I32_T   *connection_out --  connection_in is used for writing.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshConnectionSockId(UI32_T my_tid, I32_T *connection_in, I32_T *connection_out)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *connection_in = sshd_om_options[i].connection_in;
            *connection_out = sshd_om_options[i].connection_out;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshSendContext
 * PURPOSE:
 *          This function get ssh connection cipher context for sending.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          CipherContext   **send_context   --  cipher context for sending.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshSendContext(UI32_T my_tid, CipherContext **send_context)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *send_context = &(sshd_om_options[i].send_context);
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshReceiveContext
 * PURPOSE:
 *          This function get ssh connection cipher context for receiving.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          CipherContext   **Receive_context   --  cipher context for receiving.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshReceiveContext(UI32_T my_tid, CipherContext **receive_context)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *receive_context = &(sshd_om_options[i].receive_context);
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshCurrentKeys
 * PURPOSE:
 *          This function get pointer of Session key information for Encryption and MAC.
 *          MODE_IN is used for reading, MODE_OUT is used for writing.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Newkeys ***current_keys --  pointer of Session key information for Encryption and MAC.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshCurrentKeys(UI32_T my_tid, Newkeys ***current_keys)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *current_keys = sshd_om_options[i].current_keys;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshNewKey
 * PURPOSE:
 *          This function get pointer of Session key information for Encryption and MAC.
 *          MODE_IN is used for reading, MODE_OUT is used for writing.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Newkeys ***newkeys  --  pointer of Session key information for Encryption and MAC.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshNewKey(UI32_T my_tid, Newkeys ***newkeys)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *newkeys = sshd_om_options[i].newkeys;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshInputBuffer
 * PURPOSE:
 *          This function get buffer pointer for raw input data from the socket.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Buffer  **input --  buffer pointer for raw input data from the socket.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshInputBuffer(UI32_T my_tid, Buffer **input)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *input = &(sshd_om_options[i].input);
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshOutputBuffer
 * PURPOSE:
 *          This function get buffer pointer for raw output data going to the socket.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Buffer  **output    --  buffer pointer for raw output data going to the socket.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshOutputBuffer(UI32_T my_tid, Buffer **output)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *output = &(sshd_om_options[i].output);
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshOutgoingPacketBuffer
 * PURPOSE:
 *          This function get buffer pointer for the partial outgoing packet being constructed.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Buffer  **outgoing_packet   --  buffer pointer for the partial outgoing packet being constructed.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshOutgoingPacketBuffer(UI32_T my_tid, Buffer **outgoing_packet)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *outgoing_packet = &(sshd_om_options[i].outgoing_packet);
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshIncomingPacketBuffer
 * PURPOSE:
 *          This function get buffer pointer for the incoming packet currently being processed.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Buffer  **incoming_packet   --  buffer pointer for the incoming packet currently being processed.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshIncomingPacketBuffer(UI32_T my_tid, Buffer **incoming_packet)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *incoming_packet = &(sshd_om_options[i].incoming_packet);
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshDataFellows
 * PURPOSE:
 *          This function set datafellows bug compatibility.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          I32_T   datafellows --  datafellows bug compatibility.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshDataFellows(UI32_T my_tid, I32_T datafellows)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].datafellows = datafellows;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshDataFellows
 * PURPOSE:
 *          This function get datafellows bug compatibility.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *datafellows    --  datafellows bug compatibility.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshDataFellows(UI32_T my_tid, I32_T *datafellows)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *datafellows = sshd_om_options[i].datafellows;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshCompat20
 * PURPOSE:
 *          This function enabling compatibility mode for protocol 2.0 .
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshCompat20(UI32_T my_tid)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].compat20 = 1;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshCompat20
 * PURPOSE:
 *          This function get compatibility mode for protocol 2.0 .
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *compat20   --  compatibility mode for protocol 2.0 .
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshCompat20(UI32_T my_tid, I32_T *compat20)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *compat20 = sshd_om_options[i].compat20;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshCompat13
 * PURPOSE:
 *          This function enabling compatibility mode for protocol 1.3 .
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshCompat13(UI32_T my_tid)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].compat13 = 1;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshCompat13
 * PURPOSE:
 *          This function get compatibility mode for protocol 1.3 .
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *compat13   --  compatibility mode for protocol 1.3 .
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshCompat13(UI32_T my_tid, I32_T *compat13)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *compat13 = sshd_om_options[i].compat13;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshSensitiveData
 * PURPOSE:
 *          This function get ssh sensitive_data pointer.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          SSHD_SensitiveData_T    **sensitive_data    -- SSH sensitive_data pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshSensitiveData(UI32_T my_tid, SSHD_SensitiveData_T **sensitive_data)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *sensitive_data = &(sshd_om_options[i].sensitive_data);
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshChannelsAllocNumber
 * PURPOSE:
 *          This function get ssh size of the channel array.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *channels_alloc -- size of the channel array.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshChannelsAllocNumber(UI32_T my_tid, I32_T *channels_alloc)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *channels_alloc = sshd_om_options[i].channels_alloc;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshChannelsAllocNumber
 * PURPOSE:
 *          This function set ssh size of the channel array.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I32_T   channels_alloc  -- size of the channel array.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshChannelsAllocNumber(UI32_T my_tid, I32_T channels_alloc)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].channels_alloc = channels_alloc;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshChannels
 * PURPOSE:
 *          This function get Pointer of an array containing all allocated channels.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Channel ***channels -- Pointer to an array containing all allocated channels.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshChannels(UI32_T my_tid, Channel ***channels)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *channels = sshd_om_options[i].channels;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshChannels
 * PURPOSE:
 *          This function set pointer of an array containing all allocated channels.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          Channel **channels  -- Pointer to an array containing all allocated channels.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshChannels(UI32_T my_tid, Channel **channels)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].channels = channels;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshChannelMaxSock
 * PURPOSE:
 *          This function set maximum socket id value used in any of the channels.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I32_T   channel_max_fd  --  maximum socket id value used in any of the channels.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshChannelMaxSock(UI32_T my_tid, I32_T channel_max_fd)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].channel_max_fd = channel_max_fd;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshChannelMaxSock
 * PURPOSE:
 *          This function get maximum socket id value used in any of the channels.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *channel_max_fd --  maximum socket id value used in any of the channels.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshChannelMaxSock(UI32_T my_tid, I32_T *channel_max_fd)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *channel_max_fd = sshd_om_options[i].channel_max_fd;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshDispatch
 * PURPOSE:
 *          This function get pointer of an array containing all dispatch functions.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          dispatch_fn ***dispatch --  pointer to an array containing all dispatch functions.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshDispatch(UI32_T my_tid, dispatch_fn ***dispatch)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *dispatch = sshd_om_options[i].dispatch;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshRemoteProtocolFlags
 * PURPOSE:
 *          This function set protocol flags for the remote side for SSHv1.
 * INPUT:
 *          UI32_T  my_tid                  --  caller tid.
 *          UI32_T  remote_protocol_flags   --  protocol flags for the remote side.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshRemoteProtocolFlags(UI32_T my_tid, UI32_T remote_protocol_flags)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].remote_protocol_flags = remote_protocol_flags;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshRemoteProtocolFlags
 * PURPOSE:
 *          This function get protocol flags for the remote side for SSHv1.
 * INPUT:
 *          UI32_T  my_tid                  --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *remote_protocol_flags  --  protocol flags for the remote side.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshRemoteProtocolFlags(UI32_T my_tid, UI32_T *remote_protocol_flags)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *remote_protocol_flags = sshd_om_options[i].remote_protocol_flags;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

#if 0
/* FUNCTION NAME:  SSHD_OM_SetSshv1SessionKey
 * PURPOSE:
 *          This function set session key for protocol v1.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI8_T   *ssh1_key   --  session key for SSHv1.
 *          UI32_T  ssh1_keylen --  session key len for SSHv1.
 *
 * OUTPUT:
 *          UI32_T  *remote_protocol_flags  --  protocol flags for the remote side.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshv1SessionKey(UI32_T my_tid, UI8_T *ssh1_key, UI32_T ssh1_keylen)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  i;

    /* BODY */
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            memcpy(sshd_om_options[i].ssh1_key, ssh1_key, ssh1_keylen);
            sshd_om_options[i].ssh1_keylen = ssh1_keylen;
            return TRUE;
        }
    }
    return FALSE;
}
#endif

/* FUNCTION NAME:  SSHD_OM_SetKeeperTaskId
 * PURPOSE:
 *          Set task id which keep public key buffer.
 *
 * INPUT:
 *          UI32_T  tid --  task id.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetKeeperTaskId(UI32_T tid)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    keeper_task_id = tid;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME:  SSHD_OM_GetKeeperTaskId
 * PURPOSE:
 *          Get task id which keep public key buffer.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T  *tid    --  task id.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetKeeperTaskId(UI32_T *tid)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    *tid = keeper_task_id;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_GetSshInteractiveMode
 * PURPOSE:
 *          This function get ssh interactive mode status.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *interactive_mode   --  interactive mode.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshInteractiveMode(UI32_T my_tid, I32_T *interactive_mode)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *interactive_mode = sshd_om_options[i].interactive_mode;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshInteractiveMode
 * PURPOSE:
 *          This function set ssh interactive mode status.
 *
 * INPUT:
 *          UI32_T  my_tid              --  caller tid.
 *          I32_T   interactive_mode    --  interactive mode.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshInteractiveMode(UI32_T my_tid, I32_T interactive_mode)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].interactive_mode = interactive_mode;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshMaxPacketSize
 * PURPOSE:
 *          This function get ssh maximum packet size.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *max_packet_size    --  maximum packet size.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshMaxPacketSize(UI32_T my_tid, I32_T *max_packet_size)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *max_packet_size = sshd_om_options[i].max_packet_size;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshMaxPacketSize
 * PURPOSE:
 *          This function set ssh maximum packet size.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I32_T   max_packet_size --  maximum packet size.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshMaxPacketSize(UI32_T my_tid, I32_T max_packet_size)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].max_packet_size = max_packet_size;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshChannelPre
 * PURPOSE:
 *          This function get array of channel_pre.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          chan_fn ***channel_pre  --  called just before select() to add any bits relevant to
 *                                      channels in the select bitmasks.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshChannelPre(UI32_T my_tid, chan_fn ***channel_pre)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *channel_pre = sshd_om_options[i].channel_pre;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshChannelPost
 * PURPOSE:
 *          This function get array of channel_post.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          chan_fn ***channel_post --  perform any appropriate operations for channels which
 *                                      have events pending.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshChannelPost(UI32_T my_tid, chan_fn ***channel_post)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *channel_post = sshd_om_options[i].channel_post;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshChannelHandlerInit
 * PURPOSE:
 *          This function get channel handler init status.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *did_init   --  have do channel handler init.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshChannelHandlerInit(UI32_T my_tid, I32_T *did_init)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *did_init = sshd_om_options[i].channel_handler_init;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshChannelHandlerInit
 * PURPOSE:
 *          This function set channel handler init status.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          I32_T   did_init    --  have do channel handler init.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshChannelHandlerInit(UI32_T my_tid, I32_T did_init)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].channel_handler_init = did_init;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshHadChannel
 * PURPOSE:
 *          This function get channel have exist.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *had_channel    --  have channel exist.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshHadChannel(UI32_T my_tid, I32_T *had_channel)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *had_channel = sshd_om_options[i].had_channel;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshHadChannel
 * PURPOSE:
 *          This function set channel have exist.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          I32_T   had_channel --  have channel exist.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshHadChannel(UI32_T my_tid, I32_T had_channel)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].had_channel = had_channel;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshStdioBuffer
 * PURPOSE:
 *          This function get buffer pointer of stdin, stdout, stderr.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Buffer  **stdin_buffer  --  Buffer for stdin data.
 *          Buffer  **stdout_buffer --  Buffer for stdout data.
 *          Buffer  **stderr_buffer --  Buffer for stderr data.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshStdioBuffer(UI32_T my_tid, Buffer **stdin_buffer, Buffer **stdout_buffer, Buffer **stderr_buffer)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *stdin_buffer = &(sshd_om_options[i].stdin_buffer);
            *stdout_buffer = &(sshd_om_options[i].stdout_buffer);
            *stderr_buffer = &(sshd_om_options[i].stderr_buffer);
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshStdioDescriptor
 * PURPOSE:
 *          This function get descriptor for stdin, stdout, stderr.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *fdin   --  Descriptor for stdin (for writing)
 *          I32_T   *fdout  --  Descriptor for stdout (for reading)
 *          I32_T   *fderr  --  Descriptor for stderr.  May be -1.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshStdioDescriptor(UI32_T my_tid, I32_T *fdin, I32_T *fdout, I32_T *fderr)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *fdin = sshd_om_options[i].fdin;
            *fdout = sshd_om_options[i].fdout;
            *fderr = sshd_om_options[i].fderr;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshStdioDescriptor
 * PURPOSE:
 *          This function get descriptor for stdin, stdout, stderr.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *          I32_T   fdin    --  Descriptor for stdin (for writing)
 *          I32_T   fdout   --  Descriptor for stdout (for reading)
 *          I32_T   fderr   --  Descriptor for stderr.  May be -1.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshStdioDescriptor(UI32_T my_tid, I32_T fdin, I32_T fdout, I32_T fderr)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].fdin = fdin;
            sshd_om_options[i].fdout = fdout;
            sshd_om_options[i].fderr = fderr;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshEofStatus
 * PURPOSE:
 *          This function get EOF status from client, stdout, stderr.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *stdin_eof  --  EOF message received from client.
 *          I32_T   *fdout_eof  --  EOF encountered reading from fdout.
 *          I32_T   *fderr_eof  --  EOF encountered readung from fderr.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshEofStatus(UI32_T my_tid, I32_T *stdin_eof, I32_T *fdout_eof, I32_T *fderr_eof)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *stdin_eof = sshd_om_options[i].stdin_eof;
            *fdout_eof = sshd_om_options[i].fdout_eof;
            *fderr_eof = sshd_om_options[i].fderr_eof;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshEofStatus
 * PURPOSE:
 *          This function set EOF status from client, stdout, stderr.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          I32_T   stdin_eof   --  EOF message received from client.
 *          I32_T   fdout_eof   --  EOF encountered reading from fdout.
 *          I32_T   fderr_eof   --  EOF encountered readung from fderr.
 *
 * OUTPUT:
 *          noen.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshEofStatus(UI32_T my_tid, I32_T stdin_eof, I32_T fdout_eof, I32_T fderr_eof)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].stdin_eof = stdin_eof;
            sshd_om_options[i].fdout_eof = fdout_eof;
            sshd_om_options[i].fderr_eof = fderr_eof;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshConnectionClosed
 * PURPOSE:
 *          This function set ssh connection closed.
 *
 * INPUT:
 *          UI32_T  my_tid              --  caller tid.
 *          I32_T   connection_closed   --  connection close status.
 *
 * OUTPUT:
 *          noen.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshConnectionClosed(UI32_T my_tid, I32_T connection_closed)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].connection_closed = connection_closed;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshConnectionClosed
 * PURPOSE:
 *          This function get ssh connection closed status.
 *
 * INPUT:
 *          UI32_T  my_tid              --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *connection_closed  --  connection close status.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshConnectionClosed(UI32_T my_tid, I32_T *connection_closed)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *connection_closed = sshd_om_options[i].connection_closed;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshBufferHigh
 * PURPOSE:
 *          This function get "Soft" max buffer size.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *buffer_high    --  "Soft" max buffer size.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshBufferHigh(UI32_T my_tid, UI32_T *buffer_high)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *buffer_high = sshd_om_options[i].buffer_high;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshBufferHigh
 * PURPOSE:
 *          This function set "Soft" max buffer size.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI32_T  buffer_high --  "Soft" max buffer size.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshBufferHigh(UI32_T my_tid, UI32_T buffer_high)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].buffer_high = buffer_high;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshClientAliveTimeouts
 * PURPOSE:
 *          This function get client alive timeouts.
 *
 * INPUT:
 *          UI32_T  my_tid                  --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *client_alive_timeouts  --  client alive timeouts.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshClientAliveTimeouts(UI32_T my_tid, I32_T *client_alive_timeouts)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *client_alive_timeouts = sshd_om_options[i].client_alive_timeouts;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshClientAliveTimeouts
 * PURPOSE:
 *          This function set client alive timeouts.
 *
 * INPUT:
 *          UI32_T  my_tid                  --  caller tid.
 *          I32_T   client_alive_timeouts   --  client alive timeouts.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshClientAliveTimeouts(UI32_T my_tid, I32_T client_alive_timeouts)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].client_alive_timeouts = client_alive_timeouts;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshAuthctxt
 * PURPOSE:
 *          This function set authenaction context.
 *
 * INPUT:
 *          UI32_T      my_tid      --  caller tid.
 *          Authctxt    *authctxt   --  pointer authenaction context.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshAuthctxt(UI32_T my_tid, Authctxt *authctxt)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].xxx_authctxt = authctxt;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshAuthctxt
 * PURPOSE:
 *          This function get authenaction context.
 *
 * INPUT:
 *          UI32_T      my_tid      --  caller tid.
 *
 * OUTPUT:
 *          Authctxt    **authctxt  --  pointer authenaction context.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshAuthctxt(UI32_T my_tid, Authctxt **authctxt)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *authctxt = sshd_om_options[i].xxx_authctxt;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshSessions
 * PURPOSE:
 *          This function get array of sessions.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          Session ***sessions --  array of sessions.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshSessions(UI32_T my_tid, Session **sessions)
{
    UI32_T  i, j;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            for( j=0 ; j<MAX_SESSIONS ; j++ )
            {
                *(sessions+j) = &(sshd_om_options[i].sessions[j]);
            }
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_CheckSshSessions
 * PURPOSE:
 *          This function check all ssh sessions to confirm
 *          whether the session with task id "my_tid" has been assigned tnsh_tid value.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          none
 *
 * RETURN:
 *          TRUE  - tnsh_tid value has been assigned.
 *          FALSE - tnsh_tid value has not been assigned.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_CheckSshSessions(UI32_T my_tid)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            if(sshd_om_options[i].tnsh_tid != 0)
            {
                ret = TRUE;
            }
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshSessionNewInit
 * PURPOSE:
 *          This function get session array init init status.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *did_init   --  have session array init..
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshSessionNewInit(UI32_T my_tid, I32_T *did_init)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *did_init = sshd_om_options[i].session_did_init;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshSessionNewInit
 * PURPOSE:
 *          This function set session array init status.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          I32_T   did_init    --  have do init session array.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshSessionNewInit(UI32_T my_tid, I32_T did_init)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].session_did_init = did_init;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshServerKeyIndex
 * PURPOSE:
 *          This function get index of server key.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *index  --  index of server key.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshServerKeyIndex(UI32_T my_tid, UI32_T *index)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *index = (i % NUMBER_OF_TEMP_KEY)+1;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshFatalCleanups
 * PURPOSE:
 *          This function get fatal cleanup function link list.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          struct fatal_cleanup    **fatal_cleanups    --  fatal cleanup function link list.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshFatalCleanups(UI32_T my_tid, struct fatal_cleanup **fatal_cleanups)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *fatal_cleanups = sshd_om_options[i].fatal_cleanups;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshFatalCleanups
 * PURPOSE:
 *          This function set fatal cleanup function link list.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *          struct fatal_cleanup    *fatal_cleanups --  fatal cleanup function link list.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshFatalCleanups(UI32_T my_tid, struct fatal_cleanup *fatal_cleanups)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].fatal_cleanups = fatal_cleanups;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_ClearSshServerOptions
 * PURPOSE:
 *          This function release ssh server options resource.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_ClearSshServerOptions(UI32_T my_tid)
{
    UI32_T  i,j;
    Enc     *enc;
    Mac     *mac;
    Comp    *comp;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            free(sshd_om_options[i].client_version_string);
            sshd_om_options[i].client_version_string = NULL;
            free(sshd_om_options[i].server_version_string);
            sshd_om_options[i].server_version_string = NULL;

            if( sshd_om_options[i].xxx_kex != NULL )
            {
                buffer_free(&sshd_om_options[i].xxx_kex->peer);
                buffer_free(&sshd_om_options[i].xxx_kex->my);
//              memset(sshd_om_options[i].xxx_kex, 0, sizeof(*(sshd_om_options[i].xxx_kex)));
                xfree(sshd_om_options[i].xxx_kex);
                sshd_om_options[i].xxx_kex = NULL;
            }

            memset(sshd_om_options[i].session_id, 0, 16);

            if( sshd_om_options[i].session_id2 != NULL )
            {
                xfree(sshd_om_options[i].session_id2);
                sshd_om_options[i].session_id2 = NULL;
            }
            sshd_om_options[i].session_id2_len = 0;

            for( j=0 ; j<MODE_MAX ; j++ )
            {
                if( sshd_om_options[i].newkeys[j] != NULL )
                {
                    enc  = &sshd_om_options[i].newkeys[j]->enc;
                    mac  = &sshd_om_options[i].newkeys[j]->mac;
                    comp = &sshd_om_options[i].newkeys[j]->comp;
                    memset(mac->key, 0, mac->key_len);
                    xfree(enc->name);
                    xfree(enc->iv);
                    xfree(enc->key);
                    xfree(mac->name);
                    xfree(mac->key);
                    xfree(comp->name);
                    xfree(sshd_om_options[i].newkeys[j]);
                    sshd_om_options[i].newkeys[j] = NULL;
                }
            }

            for( j=0 ; j<MODE_MAX ; j++ )
            {
                if( sshd_om_options[i].current_keys[j] != NULL )
                {
                    enc  = &sshd_om_options[i].current_keys[j]->enc;
                    mac  = &sshd_om_options[i].current_keys[j]->mac;
                    comp = &sshd_om_options[i].current_keys[j]->comp;
                    memset(mac->key, 0, mac->key_len);
                    xfree(enc->name);
                    xfree(enc->iv);
                    xfree(enc->key);
                    xfree(mac->name);
                    xfree(mac->key);
                    xfree(comp->name);
                    xfree(sshd_om_options[i].current_keys[j]);
                    sshd_om_options[i].current_keys[j] = NULL;
                }
            }

            sshd_om_options[i].datafellows = 0;
            sshd_om_options[i].compat20 = 0;
            sshd_om_options[i].compat13 = 0;

            if( sshd_om_options[i].channels != NULL )
            {
                xfree(sshd_om_options[i].channels);
                sshd_om_options[i].channels = NULL;
            }
            sshd_om_options[i].channels_alloc = 0;
            sshd_om_options[i].channel_max_fd = 0;
            sshd_om_options[i].remote_protocol_flags = 0;
            sshd_om_options[i].interactive_mode = 0;
            sshd_om_options[i].max_packet_size = 32768;
            sshd_om_options[i].channel_handler_init = 0;
            sshd_om_options[i].had_channel = 0;
            sshd_om_options[i].fdin = 0;
            sshd_om_options[i].fdout = 0;
            sshd_om_options[i].fderr = 0;
            sshd_om_options[i].stdin_eof = 0;
            sshd_om_options[i].fdout_eof = 0;
            sshd_om_options[i].fderr_eof = 0;
            sshd_om_options[i].connection_closed = 0;
            sshd_om_options[i].buffer_high = 0;

            if( sshd_om_options[i].xxx_authctxt != NULL )
            {
                if( sshd_om_options[i].xxx_authctxt->user != NULL )
                {
                    xfree(sshd_om_options[i].xxx_authctxt->user);
                    sshd_om_options[i].xxx_authctxt->user = NULL;
                }
                if( sshd_om_options[i].xxx_authctxt->service != NULL )
                {
                    xfree(sshd_om_options[i].xxx_authctxt->service);
                    sshd_om_options[i].xxx_authctxt->service = NULL;
                }
                xfree(sshd_om_options[i].xxx_authctxt);
                sshd_om_options[i].xxx_authctxt = NULL;
            }

            sshd_om_options[i].session_did_init = 0;
            sshd_om_options[i].read_seqnr = 0;
            sshd_om_options[i].send_seqnr = 0;

            sshd_om_options[i].connection_state = 0;
            sshd_om_options[i].connection_major_version = 0;
            sshd_om_options[i].connection_minor_version = 0;
            memset(sshd_om_options[i].connection_cipher_string, 0, SSHD_MAX_SSH_CIPHER_STRING_LEN+1);
            memset(sshd_om_options[i].connection_username, 0, SYS_ADPT_MAX_USER_NAME_LEN+1);
            sshd_om_options[i].connection_id = -1; /* -1 */
            sshd_om_options[i].tnsh_tid = 0;   /* the tnsh task-id associated with this socket connection. */
            memset(&sshd_om_options[i].user_addr, 0, sizeof(sshd_om_options[i].user_addr));
            sshd_om_options[i].user_port = 0;          /*  remote port of user     */
            sshd_om_options[i].user_local_port = 0;    /*  local side port to user */
            sshd_om_options[i].tnsh_port = 0;          /*  port connect to TNSHD   */
            sshd_om_options[i].remote_tnsh_port = 0;   /*  remote side(tnsh) port to TNSHD */

            sshd_om_options[i].auth_result.privilege = 0xff;
            sshd_om_options[i].auth_result.authen_by_whom = USERAUTH_AUTH_NONE;
            sshd_om_options[i].auth_result.method = USERAUTH_UNKNOWN;

            memset(sshd_om_options[i].password, 0, sizeof(sshd_om_options[i].password));


            sshd_om_options[i].connection_in = -1;
            sshd_om_options[i].connection_out = -1;

            sshd_om_options[i].tnsh_fd = -1;

            sshd_om_options[i].packet_length = 0;

            sshd_om_options[i].task_id = 0;

            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshReadSequenceNumber
 * PURPOSE:
 *          This function get read sequence number.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *read_seqnr --  read sequence number.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshReadSequenceNumber(UI32_T my_tid, UI32_T *read_seqnr)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *read_seqnr = sshd_om_options[i].read_seqnr;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshReadSequenceNumber
 * PURPOSE:
 *          This function set read sequence number.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI32_T  read_seqnr  --  read sequence number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshReadSequenceNumber(UI32_T my_tid, UI32_T read_seqnr)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].read_seqnr = read_seqnr;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshSendSequenceNumber
 * PURPOSE:
 *          This function get send sequence number.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *send_seqnr --  send sequence number.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshSendSequenceNumber(UI32_T my_tid, UI32_T *send_seqnr)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *send_seqnr = sshd_om_options[i].send_seqnr;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshSendSequenceNumber
 * PURPOSE:
 *          This function set send sequence number.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI32_T  send_seqnr  --  send sequence number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshSendSequenceNumber(UI32_T my_tid, UI32_T send_seqnr)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].send_seqnr = send_seqnr;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshConnectionStatus()
 * PURPOSE:
 *          This function set status of ssh connection.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          SSHD_ConnectionState_T  --  current state of connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionStatus(UI32_T my_tid, SSHD_ConnectionState_T state)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].connection_state = state;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshConnectionVersion()
 * PURPOSE:
 *          This function set version of ssh connection.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *          UI32_T  major   --  number of major version of connection.
 *          UI32_T  minor   --  number of minor version of connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionVersion(UI32_T my_tid, UI32_T major, UI32_T minor)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].connection_major_version = major;
            sshd_om_options[i].connection_minor_version = minor;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshConnectionUsername()
 * PURPOSE:
 *          This function set username of ssh connection.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI8_T   *username   --  username
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionUsername(UI32_T my_tid, UI8_T *username)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            strncpy((char *)sshd_om_options[i].connection_username, (char *)username, SYS_ADPT_MAX_USER_NAME_LEN);
            sshd_om_options[i].connection_username[SYS_ADPT_MAX_USER_NAME_LEN] = '\0';
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSessionPair
 * PURPOSE:
 *      Add a session pair to session record.
 *
 * INPUT:
 *          UI32_T  my_tid              --  caller tid.
 *          UI32_T  remote_tnsh_port    --  remote site port of TNSH (pty) session.
 *          UI32_T  tnsh_port           --  the local side port connect to TNSHD.
 *          UI32_T  user_local_port     --  local site port of SSHD (net) session.
 *          UI32_T  user_ip             --  the ip of remote site in socket.
 *          UI32_T  user_port           --  the port of remote site in socket.(net)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE to indicate successful and FALSE to indicate failure.
 *
 * NOTES:
 *          (  Something must be known to use this function. )
 */
BOOL_T SSHD_OM_SetSessionPair(UI32_T my_tid, UI32_T remote_tnsh_port, UI32_T tnsh_port,
                              UI32_T user_local_port,
                              const L_INET_AddrIp_T *user_addr, UI32_T user_port)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].remote_tnsh_port = remote_tnsh_port;
            sshd_om_options[i].tnsh_port = tnsh_port;
            sshd_om_options[i].user_local_port = user_local_port;
            sshd_om_options[i].user_addr = *user_addr;
            sshd_om_options[i].user_port = user_port;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetConnectionIDAndTnshID
 * PURPOSE:
 *      Set tnsh id and ssh connection id to session record.
 *
 * INPUT:
 *      UI32_T  tnsh_port   --  the port connect to TNSHD.
 *      UI32_T  tid         --  TNSH id.
 *      UI32_T  cid         --  ssh connection id.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - tnsh_port found and ssh server enabled, or don't found tnsh_port.
 *      FALSE - tnsh_port found and ssh server disabled.
 *
 * NOTES:
 *      This function invoked in SSHD_MGR_SetConnectionIDAndTnshID().
 */
BOOL_T SSHD_OM_SetConnectionIDAndTnshID(UI32_T tnsh_port, UI32_T tid, UI32_T cid)
{
    UI32_T  i;
    BOOL_T  ret = TRUE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].tnsh_port == tnsh_port )
        {
            if( sshd_status != SSHD_STATE_DISABLED )
            {
                sshd_om_options[i].tnsh_tid = tid;
                sshd_om_options[i].connection_id = cid;
            }
            else
            {
                ret = FALSE;
            }
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshConnectionId
 * PURPOSE:
 *      Get SSH pty connection id.
 *
 * INPUT:
 *      UI32_T  task_id     --  SSH task id.
 *
 * OUTPUT:
 *      UI32_T  *conn_id_p  --  SSH connection id.
 *
 * RETURN:
 *      TRUE  - Succeeded.
 *      FALSE - Failed.
 *
 * NOTES:
 *      None.
 */
BOOL_T SSHD_OM_GetSshConnectionId(UI32_T task_id, UI32_T *conn_id_p)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    if (NULL == conn_id_p)
    {
        return FALSE;
    }

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == task_id )
        {
            *conn_id_p = sshd_om_options[i].connection_id;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_CheckSshConnection
 * PURPOSE:
 *      Check connection is ssh or not.
 *
 * INPUT:
 *      UI32_T  cid --  connection id.
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - This connection is ssh connection.
 *      FALSE - This connection is not ssh connection.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_CheckSshConnection(UI32_T cid)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].connection_id == cid )
        {
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetConnectingTnshFd
 * PURPOSE:
 *      Set connecting tnsh socket ID to OM.
 *
 * INPUT:
 *      UI32_T  task_id --  SSH task id
 *      int  sock_id    --  socket id that connects
 *                          to tnsh
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      None.
 */
BOOL_T SSHD_OM_SetConnectingTnshFd(UI32_T task_id, int sock_id)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == task_id )
        {
            sshd_om_options[i].tnsh_fd = sock_id;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetConnectingTnshFd
 * PURPOSE:
 *      Gets connecting tnsh socket ID from OM.
 *
 * INPUT:
 *      UI32_T  task_id --  SSH task id.
 *
 * OUTPUT:
 *      int  *tnsh_fd_p --  socket id that connects
 *                           to tnsh
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      None.
 */
BOOL_T SSHD_OM_GetConnectingTnshFd(UI32_T task_id, int *sock_id_p)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    if (NULL == sock_id_p)
    {
        return FALSE;
    }

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for ( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == task_id )
        {
            *sock_id_p = sshd_om_options[i].tnsh_fd;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshConnectionUsername
 * PURPOSE:
 *      Get username of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  cid         --  connection_id.
 *
 * OUTPUT:
 *      UI8_T   *username   --  Buffer for username. The size of buffer MUST
 *                              larger than SYS_ADPT_MAX_USER_NAME_LEN+1.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_GetSshUsername
 */
BOOL_T SSHD_OM_GetSshConnectionUsername(UI32_T cid, UI8_T *username)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].connection_id == cid )
        {
            strncpy((char *)username, (char *)sshd_om_options[i].connection_username, SYS_ADPT_MAX_USER_NAME_LEN);
            username[SYS_ADPT_MAX_USER_NAME_LEN] = '\0';
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshConnectionPassword
 * PURPOSE:
 *          This function set password of ssh connection.
 *
 * INPUT:
 *          UI32_T       my_tid      --  caller tid.
 *          const char   *password   --  password
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionPassword(
    UI32_T my_tid,
    const char *password)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            strncpy(sshd_om_options[i].password, password, SYS_ADPT_MAX_PASSWORD_LEN);
            sshd_om_options[i].password[SYS_ADPT_MAX_PASSWORD_LEN] = '\0';
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshConnectionPassword
 * PURPOSE:
 *      Get password of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  cid         --  connection_id.
 *
 * OUTPUT:
 *      UI8_T   *password   --  Buffer for username. The size of buffer MUST
 *                              larger than SYS_ADPT_MAX_PASSWORD_LEN+1.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_GetSshUsername
 */
BOOL_T SSHD_OM_GetSshConnectionPassword(
    UI32_T cid,
    char *password,
    UI32_T password_size)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    if (password_size < SYS_ADPT_MAX_PASSWORD_LEN+1)
    {
        return FALSE;
    }

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].connection_id == cid )
        {
            strncpy(password, sshd_om_options[i].password, SYS_ADPT_MAX_PASSWORD_LEN);
            password[SYS_ADPT_MAX_PASSWORD_LEN] = '\0';
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshConnectionPrivilege
 * PURPOSE:
 *      Set username privilege of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  my_tid      --  caller tid.
 *      UI32_T  privilege   --  privilege.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_VM_SetSshConnectionPrivilege
 */
BOOL_T SSHD_OM_SetSshConnectionPrivilege(UI32_T my_tid, UI32_T privilege)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].auth_result.privilege = privilege;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSshConnectionPrivilege
 * PURPOSE:
 *      Get username privilege of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  cid         --  connection_id.
 *
 * OUTPUT:
 *      UI32_T  *privilege  --  privilege.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_GetSshConnectionPrivilege
 */
BOOL_T SSHD_OM_GetSshConnectionPrivilege(UI32_T cid, UI32_T *privilege)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].connection_id == cid )
        {
            *privilege = sshd_om_options[i].auth_result.privilege;
            if( *privilege != 0xff )
            {
                ret = TRUE;
            }
            else
            {
                ret = FALSE;
            }
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_SetSshConnectionAuthResult
 * PURPOSE:
 *      Set authenticated result of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  my_tid      --  caller tid.
 *      USERAUTH_AuthResult_T auth_result_p   --  authenticated result.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      none.
 */
BOOL_T SSHD_OM_SetSshConnectionAuthResult(
    UI32_T my_tid,
    USERAUTH_AuthResult_T *auth_result_p)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].auth_result   = *auth_result_p;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}



/* FUNCTION NAME : SSHD_OM_GetSshConnectionAuthResult
 * PURPOSE:
 *      Get authenticated result of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  cid         --  connection_id.
 *
 * OUTPUT:
 *      USERAUTH_AuthResult_T  *auth_result_p  --  authenticated result.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      None.
 */
BOOL_T SSHD_OM_GetSshConnectionAuthResult(
    UI32_T cid,
    USERAUTH_AuthResult_T *auth_result_p)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].connection_id == cid )
        {
            if( sshd_om_options[i].auth_result.authen_by_whom != USERAUTH_AUTH_NONE )
            {
                *auth_result_p = sshd_om_options[i].auth_result;
                ret = TRUE;
            }
            else
            {
                ret = FALSE;
            }
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}


/* FUNCTION NAME:  SSHD_OM_SetSshConnectionCipher
 * PURPOSE:
 *          This function set cipher of ssh connection.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *          UI32_T  *cipher --  cipher of connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionCipher(UI32_T my_tid, UI8_T *cipher)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            memset(sshd_om_options[i].connection_cipher_string, 0, SSHD_MAX_SSH_CIPHER_STRING_LEN+1);
            strncpy((char *)sshd_om_options[i].connection_cipher_string, (char *)cipher, SSHD_MAX_SSH_CIPHER_STRING_LEN);
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshPacketLength
 * PURPOSE:
 *          This function get packet length.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *packet_length --  packet length.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshPacketLength(UI32_T my_tid, UI32_T *packet_length)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *packet_length = sshd_om_options[i].packet_length;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_SetSshPacketLength
 * PURPOSE:
 *          This function set send sequence number.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          UI32_T  packet_length   --  packet_length.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshPacketLength(UI32_T my_tid, UI32_T packet_length)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();
    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            sshd_om_options[i].packet_length = packet_length;
            ret = TRUE;
            break;
        }
    }
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshMacBuffer
 * PURPOSE:
 *          This function get mac buffer for mac_compute.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          UI8_T   **mac_buffer    --  mac buffer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshMacBuffer(UI32_T my_tid, UI8_T **mac_buffer)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *mac_buffer = sshd_om_options[i].mac_buffer;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshKexgexHashDigest
 * PURPOSE:
 *          This function get digest of kexgex_hash.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          UI8_T   **digest    --  digest of kexgex_hash.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshKexgexHashDigest(UI32_T my_tid, UI8_T **digest)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *digest = sshd_om_options[i].kexgex_hash_digest;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME:  SSHD_OM_GetSshkexDhHashDigest
 * PURPOSE:
 *          This function get digest of kex_dh_hash.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          UI8_T   **digest    --  digest of kex_dh_hash.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshkexDhHashDigest(UI32_T my_tid, UI8_T **digest)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if( sshd_om_options[i].task_id == my_tid )
        {
            *digest = sshd_om_options[i].kex_dh_hash_digest;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetSessionPair
 * PURPOSE:
 *      Retrieve a session pair from session record.
 *
 * INPUT:
 *      UI32_T  -- the port connect to TNSHD.
 *
 * OUTPUT:
 *      UI32_T * -- the ip of remote site in socket.
 *      UI32_T * -- the port of remote site in socket.
 *
 * RETURN:
 *      TRUE to indicate successful and FALSE to indicate failure.
 *
 * NOTES:
 *          (  Something must be known to use this function. )
 */
BOOL_T SSHD_OM_GetSessionPair(UI32_T tnsh_port, L_INET_AddrIp_T *user_addr, UI32_T *user_port)
{
    UI32_T  i;
    BOOL_T  ret = FALSE;

    SSHD_OM_ENTER_CRITICAL_SECTION();

    for ( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if ( sshd_om_options[i].tnsh_port == tnsh_port )
        {
            *user_addr = sshd_om_options[i].user_addr;
            *user_port = sshd_om_options[i].user_port;
            ret = TRUE;
            break;
        }
    }

    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/* FUNCTION NAME : SSHD_OM_GetGenerateHostKeyAction
 * PURPOSE:
 *      Get tpye of host key generation.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_type    --  tpye of host key generation.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetGenerateHostKeyAction(UI32_T *action_type)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    *action_type = sshd_om_generate_host_key_action;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_SetGenerateHostKeyAction
 * PURPOSE:
 *      Set tpye of host key generation.
 *
 * INPUT:
 *      UI32_T  action_type --  tpye of host key generation.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetGenerateHostKeyAction(UI32_T action_type)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_generate_host_key_action = action_type;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_GetGenerateHostKeyStatus
 * PURPOSE:
 *      Get result of host key generation.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_result  --  result of host key generation.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetGenerateHostKeyStatus(UI32_T *action_result)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    *action_result = sshd_om_generate_host_key_status;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_SetGenerateHostKeyStatus
 * PURPOSE:
 *      Set result of host key generation.
 *
 * INPUT:
 *      UI32_T  action_result   --  result of host key generation.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetGenerateHostKeyStatus(UI32_T action_result)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_generate_host_key_status = action_result;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_GetDeleteUserPublicKeyAction
 * PURPOSE:
 *      Get tpye of user key delete.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_type    --  tpye of user key delete.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetDeleteUserPublicKeyAction(UI32_T *action_type)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    *action_type = sshd_om_delete_user_key_action;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_SetDeleteUserPublicKeyAction
 * PURPOSE:
 *      Set tpye of user key delete.
 *
 * INPUT:
 *      UI32_T  action_type --  tpye of user key delete.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetDeleteUserPublicKeyAction(UI32_T action_type)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_delete_user_key_action = action_type;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_GetDeleteUserPublicKeyStatus
 * PURPOSE:
 *      Get result of user key delete.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_result  --  result of user key delete.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetDeleteUserPublicKeyStatus(UI32_T *action_result)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    *action_result = sshd_om_delete_user_key_status;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_SetDeleteUserPublicKeyStatus
 * PURPOSE:
 *      Set result of user key delete.
 *
 * INPUT:
 *      UI32_T  action_result   --  result of user key delete.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetDeleteUserPublicKeyStatus(UI32_T action_result)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_delete_user_key_status = action_result;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_GetWriteHostKey2FlashAction
 * PURPOSE:
 *      Get tpye of host key writing.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_type    --  tpye of host key writing.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetWriteHostKey2FlashAction(UI32_T *action_type)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    *action_type = sshd_om_save_host_key_action;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_SetWriteHostKey2FlashAction
 * PURPOSE:
 *      Set tpye of host key writing.
 *
 * INPUT:
 *      UI32_T  action_type --  tpye of host key writing.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetWriteHostKey2FlashAction(UI32_T action_type)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_save_host_key_action = action_type;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_GetWriteHostKey2FlashStatus
 * PURPOSE:
 *      Get result of host key writing.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_result  --  result of host key writing.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetWriteHostKey2FlashStatus(UI32_T *action_result)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    *action_result = sshd_om_save_host_key_status;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME : SSHD_OM_SetWriteHostKey2FlashStatus
 * PURPOSE:
 *      Set result of host key writing.
 *
 * INPUT:
 *      UI32_T  action_result   --  result of host key writing.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetWriteHostKey2FlashStatus(UI32_T action_result)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_save_host_key_status = action_result;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME:  SSHD_OM_SetSshPasswordAuthenticationStatus
 * PURPOSE:
 *          This function set password authentication state.
 *
 * INPUT:
 *          SSHD_PasswordAuthenticationStatus_T state   --  Password Authentication Status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_SetSshPasswordAuthenticationStatus(SSHD_PasswordAuthenticationStatus_T state)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_password_authentication_status = state;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME:  SSHD_OM_GetSshPasswordAuthenticationStatus
 * PURPOSE:
 *          This function get password authentication state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          SSHD_PasswordAuthenticationStatus_T *state  --  Password Authentication Status.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_GetSshPasswordAuthenticationStatus(SSHD_PasswordAuthenticationStatus_T *state)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    *state = sshd_om_password_authentication_status;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME:  SSHD_OM_SetSshPubkeyAuthenticationStatus
 * PURPOSE:
 *          This function set password authentication state.
 *
 * INPUT:
 *          SSHD_PubkeyAuthenticationStatus_T state   --  Pubkey Authentication Status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_SetSshPubkeyAuthenticationStatus(SSHD_PubkeyAuthenticationStatus_T state)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_pubkey_authentication_status = state;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME:  SSHD_OM_GetSshPubkeyAuthenticationStatus
 * PURPOSE:
 *          This function get pubkey authentication state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          SSHD_PubkeyAuthenticationStatus_T *state  --  Pubkey Authentication Status.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_GetSshPubkeyAuthenticationStatus(SSHD_PubkeyAuthenticationStatus_T *state)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    *state = sshd_om_pubkey_authentication_status;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME:  SSHD_OM_SetSshRsaAuthenticationStatus
 * PURPOSE:
 *          This function set rsa authentication state.
 *
 * INPUT:
 *          SSHD_RsaAuthenticationStatus_T state   --  Rsa Authentication Status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_SetSshRsaAuthenticationStatus(SSHD_RsaAuthenticationStatus_T state)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    sshd_om_rsa_authentication_status = state;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* FUNCTION NAME:  SSHD_OM_GetSshRsaAuthenticationStatus
 * PURPOSE:
 *          This function get rsa authentication state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          SSHD_RsaAuthenticationStatus_T *state  --  Rsa Authentication Status.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_GetSshRsaAuthenticationStatus(SSHD_RsaAuthenticationStatus_T *state)
{
    SSHD_OM_ENTER_CRITICAL_SECTION();
    *state = sshd_om_rsa_authentication_status;
    SSHD_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}


/* LOCAL SUBPROGRAM BODIES
 */




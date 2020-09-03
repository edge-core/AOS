/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include "sysfun.h"
#include "cmdftp.h"
#include "l_md5.h"
#include "openssl/ssl.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define CMDFTP_DFLT_FTP_BLOCK_SIZE      1024    /* the default block size for receiving file */
#define CMDFTP_BUF_SIZE                 2047
#define CMDFTP_DFLT_FTP_TIMEOUT         10      /* in seconds */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    CMDFTP_Error_NULL,
    CMDFTP_Error_SERV,
    CMDFTP_Error_CONN,
    CMDFTP_Error_LGIN,
    CMDFTP_Error_UNER,
    CMDFTP_Error_HEAP,
    CMDFTP_Error_NPTR, /* null pointer */
    CMDFTP_Error_CONN_DATA,
    CMDFTP_Error_SIZE_EXCEED,
    CMDFTP_Error_NOT_SUPPORT_FTPS,
    CMDFTP_Error_NOT_ACCEPT_CIPHERS,
    CMDFTP_Error_FILE_NOT_FOUND,
    CMDFTP_Error_TIMEOUT,
} CMDFTP_Error_T;

typedef struct
{
    union
    {
        struct sockaddr_in sin4;
        struct sockaddr_in6 sin6;
    } sockaddr;

    struct sockaddr *sockaddr_p;
    UI32_T sockaddr_len;

    UI32_T socket;

    SSL *ssl;
} CMDFTP_Conn_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void init_ssl_library();
static BOOL_T ftp_connect(CMDFTP_Conn_T *conn_p);
static BOOL_T init_address_for_conn(CMDFTP_Conn_T *conn_p, L_INET_AddrIp_T *in_addr_p);
static BOOL_T set_data_conn_port(UI32_T port);
static BOOL_T greeting();
static BOOL_T login(UI8_T *username, UI8_T *password);
static BOOL_T get_remote_file_size(UI8_T *filename, UI32_T *file_size_p);
static I32_T my_ssl_read(char *buf, UI32_T n, CMDFTP_Conn_T *conn_p);
static I32_T my_ssl_write(char *buf, UI32_T n, CMDFTP_Conn_T *conn_p);
static BOOL_T tls_ssl_connect(CMDFTP_Conn_T *conn_p);
static I32_T my_socket_read(char *buf, UI32_T n, UI32_T socket);
static I32_T my_raw_read(char *buf, UI32_T n, CMDFTP_Conn_T *conn_p);
static I32_T my_socket_write(char *buf, UI32_T n, UI32_T socket);
static I32_T my_raw_write(char *buf, UI32_T n, CMDFTP_Conn_T *conn_p);
static BOOL_T send_command(char *cmd, BOOL_T to_recv_answer, UI32_T *answer_p);
static void reset_cmd_buffer();
static BOOL_T recv_confirm();
static BOOL_T recv_answer(UI32_T *answer_p);
static BOOL_T get_pasv_port(UI32_T *port_p);
static char *recv_line(CMDFTP_Conn_T *conn_p);
static void close_connection(CMDFTP_Conn_T *conn_p);
static void shutdown_connection(CMDFTP_Conn_T *conn_p);
static BOOL_T download_file(UI8_T *src_filename, BOOL_T get_whole_file, BOOL_T is_security, UI8_T *dst_buffer_p, UI32_T dst_buffer_size, UI32_T *dst_buffer_size_used_p);
static BOOL_T upload_file(UI8_T *dst_filename, BOOL_T is_security, UI8_T *src_buffer, UI32_T src_buff_size, UI32_T *src_buff_size_uploaded);
static void CMDFTP_NotifyTransmittingStatus(UI32_T percent);

/* STATIC VARIABLE DEFINITIONS
 */
static BOOL_T is_ssl_init = FALSE;
static L_INET_AddrIp_T ip_addr = {0};                        /* address type (ipv4, ipv6 or others) */

static CMDFTP_Conn_T control_conn;                           /* control connection */
static CMDFTP_Conn_T data_conn;                              /* data connection */

static SSL_CTX *ssl_context = NULL;                          /* core data structure for TLS/SSL */

static char cmd_buffer[CMDFTP_BUF_SIZE + 1];                 /* in/out commands */
static char cmd_line[CMDFTP_BUF_SIZE + 1];                   /* in     line sep */
static char *cmd_ptr;                                        /* ->cmd_buffer */

static UI32_T cmdftp_debug_flag;

static CMDFTP_Error_T cmdftp_error_code = CMDFTP_Error_NULL;
static UI32_T cmdftp_remote_answer = 0;
static SYS_TYPE_CallBack_T *cmdftp_transmitt_callback;

/* EXPORTED SUBPROGRAM BODIES
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - CMDFTP_SetCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function. The registered function will
 *           be called while tftp is transmitting file.
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *
 * -------------------------------------------------------------------------*/
void CMDFTP_SetCallback(void (*fun)(UI32_T percent))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(cmdftp_transmitt_callback);

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - CMDFTP_GetErrorCode
 *-------------------------------------------------------------------------
 * FUNCTION : This function will return the error code of the current status.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : XFER_MGR_FileCopyStatus_T
 * NOTE     : None.
 *------------------------------------------------------------------------
 */
XFER_MGR_FileCopyStatus_T CMDFTP_GetErrorCode()
{
    if (cmdftp_remote_answer > 0)
    {
        switch (cmdftp_remote_answer)
        {
            case 425:
                return XFER_MGR_FILE_COPY_DATA_CONNECTION_OPEN_ERROR;

            case 500: /* syntax error */
            case 501: /* syntax error in parameters */
            case 502: /* command not implemented. */
            case 503: /* bad sequence of commands */
            case 504: /* remote command not implemented for that parameter */
                return XFER_MGR_FILE_COPY_UNCLASSIFIED_ERROR;

            case 530:
                return XFER_MGR_FILE_COPY_LOG_IN_ERROR;

            case 450:
            case 550:
                return XFER_MGR_FILE_COPY_FILE_UNAVAILABLE;

            case 452:
            case 552:
                return XFER_MGR_FILE_COPY_STORAGE_FULL;

            case 453:
            case 553:
                return XFER_MGR_FILE_COPY_INVALID_FILE_NAME;

            default:
                return XFER_MGR_FILE_COPY_UNCLASSIFIED_ERROR;
        }
    }

    switch (cmdftp_error_code)
    {
        case CMDFTP_Error_NULL:
            return XFER_MGR_FILE_COPY_SUCCESS;

        case CMDFTP_Error_UNER:
        case CMDFTP_Error_NPTR:
        case CMDFTP_Error_HEAP:
            return XFER_MGR_FILE_COPY_UNCLASSIFIED_ERROR;

        case CMDFTP_Error_SIZE_EXCEED:
            return XFER_MGR_FILE_COPY_FILE_SIZE_EXCEED;

        case CMDFTP_Error_CONN:
            return XFER_MGR_FILE_COPY_CONNECT_ERROR;

        case CMDFTP_Error_SERV:
            return XFER_MGR_FILE_COPY_SERVER_NOT_IN_SERVICE;

        case CMDFTP_Error_NOT_SUPPORT_FTPS:
            return XFER_MGR_FILE_COPY_SERVER_NOT_SUPPORT_FTPS;

        case CMDFTP_Error_NOT_ACCEPT_CIPHERS:
            return XFER_MGR_FILE_COPY_SERVER_NOT_ACCEPT_PROVIDED_CIPHERS;

        case CMDFTP_Error_FILE_NOT_FOUND:
            return XFER_MGR_FILE_COPY_FILE_NOT_FOUND;

        case CMDFTP_Error_CONN_DATA:
            return XFER_MGR_FILE_COPY_DATA_CONNECTION_OPEN_ERROR;

        case CMDFTP_Error_LGIN:
            return XFER_MGR_FILE_COPY_LOG_IN_ERROR;

        case CMDFTP_Error_TIMEOUT:
            return XFER_MGR_FILE_COPY_TIMEOUT;
    }

    return XFER_MGR_FILE_COPY_UNCLASSIFIED_ERROR;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - CMDFTP_SetDebugFlag
 *-------------------------------------------------------------------------
 * FUNCTION : This function will set the debug flag.
 * INPUT    : flag
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTE     : None.
 *------------------------------------------------------------------------
 */
void CMDFTP_SetDebugFlag(UI32_T flag)
{
    cmdftp_debug_flag = flag;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - CMDFTP_GetDebugFlag
 *-------------------------------------------------------------------------
 * FUNCTION : This function will get the debug flag.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : flag
 * NOTE     : None.
 *------------------------------------------------------------------------
 */
UI32_T CMDFTP_GetDebugFlag()
{
    return cmdftp_debug_flag;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - CMDFTP_Transfer
 *-------------------------------------------------------------------------
 * FUNCTION : This function will transfer a file between FTP server and memory
 *            stream.
 * INPUT    : ip_addr_p            -- FTP server IP addrss
 *            username             -- User to login
 *            password             -- Password of login user
 *            is_security          -- This parameter indicates to use FTPS to transfer or not.
 *                                    TRUE, use FTPS to transfer files
 *                                    FALSE, use FTP to transfer files
 *            dir                  -- CMDFTP_TRANS_DIR_DOWNLOAD, download from server,
 *                                    CMDFTP_TRANS_DIR_UPLOAD, download to server
 *            get_whole_file       -- This parameter be used for download file.
 *                                    TRUE, get a whole file;
 *                                    FALSE, get the specified size (buffer size)
 *                                    of the file.
 *            src_filename         -- Filename for download or upload
 *            dst_buffer           -- buffer for download/upload
 *            dst_buffer_size      -- buffer size
 *            dst_buffer_size_used -- exact amount size for donload/upload
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Support only one file transfer requirement by FTP at the same time.
 *------------------------------------------------------------------------
 */
BOOL_T CMDFTP_Transfer(L_INET_AddrIp_T *ip_addr_p, UI8_T *username, UI8_T *password,
    BOOL_T is_security, CMDFTP_TransferDirection_T dir, BOOL_T get_whole_file,
    UI8_T *src_filename, UI8_T *dst_buffer, UI32_T dst_buffer_size, UI32_T *dst_buffer_size_used)
{
    cmdftp_error_code = CMDFTP_Error_NULL;
    cmdftp_remote_answer = 0;

    if (   (NULL == ip_addr_p)
        || (NULL == username)
        || (NULL == password)
        || (NULL == src_filename)
        || (NULL == dst_buffer)
        || (0 == dst_buffer_size)
        || (NULL == dst_buffer_size_used)
        || (   (CMDFTP_TRANS_DIR_DOWNLOAD != dir)
            && (CMDFTP_TRANS_DIR_UPLOAD != dir)))
    {
        cmdftp_error_code = CMDFTP_Error_NPTR;
        return FALSE;
    }

    if (FALSE == is_ssl_init)
    {
        init_ssl_library();
        is_ssl_init = TRUE;
    }

    memcpy(&ip_addr, ip_addr_p, sizeof(ip_addr));

    if (   (FALSE == init_address_for_conn(&control_conn, ip_addr_p))
        || (FALSE == init_address_for_conn(&data_conn, ip_addr_p)))
    {
        cmdftp_error_code = CMDFTP_Error_UNER;
        return FALSE;
    }

    if (FALSE == ftp_connect(&control_conn))
    {
        cmdftp_error_code = CMDFTP_Error_CONN;
        return FALSE;
    }

    /* check FTP server status is ready (220)
     */
    if (FALSE == greeting())
    {
        cmdftp_error_code = CMDFTP_Error_SERV;
        send_command("QUIT", FALSE, NULL);
        close_connection(&control_conn);
        return FALSE;
    }

    if (TRUE == is_security)
    {
        UI32_T answer;

        /* send AUTH command and establish the TLS/SSL channel before to do user authentication.
         */
        if (FALSE == send_command("AUTH TLS", TRUE, &answer))
        {
            cmdftp_error_code = CMDFTP_Error_CONN;
            shutdown_connection(&control_conn);
            return FALSE;
        }

        if (234 != answer)
        {
            cmdftp_error_code = CMDFTP_Error_NOT_SUPPORT_FTPS;
            send_command("QUIT", FALSE, NULL);
            close_connection(&control_conn);
            return FALSE;
        }

        if (FALSE == tls_ssl_connect(&control_conn))
        {
            cmdftp_error_code = CMDFTP_Error_NOT_ACCEPT_CIPHERS;
            send_command("QUIT", FALSE, NULL);
            close_connection(&control_conn);
            return FALSE;
        }
    }

    if (FALSE == login(username, password))
    {
        /* some ftp server (like zFTPServer) will disconnect immediately when login fail,
         * so that we don't send QUIT command.
         */
        cmdftp_error_code = CMDFTP_Error_LGIN;
        send_command("QUIT", FALSE, NULL);
        close_connection(&control_conn);
        return FALSE;
    }

    if (TRUE == is_security)
    {
        UI32_T answer;

        if (FALSE == send_command("PBSZ 0", TRUE, &answer))
        {
            cmdftp_error_code = CMDFTP_Error_CONN;
            shutdown_connection(&control_conn);
            return FALSE;
        }

        if (200 != answer)
        {
            cmdftp_error_code = CMDFTP_Error_NOT_SUPPORT_FTPS;
            send_command("QUIT", FALSE, NULL);
            close_connection(&control_conn);
            return FALSE;
        }

        if (FALSE == send_command("PROT P", TRUE, &answer))
        {
            cmdftp_error_code = CMDFTP_Error_CONN;
            shutdown_connection(&control_conn);
            return FALSE;
        }

        if (200 != answer)
        {
            cmdftp_error_code = CMDFTP_Error_NOT_SUPPORT_FTPS;
            send_command("QUIT", FALSE, NULL);
            close_connection(&control_conn);
            return FALSE;
        }
    }

    if (dir == CMDFTP_TRANS_DIR_DOWNLOAD)
    {   /* download file from FTP server
         */
        if (FALSE == download_file(src_filename, get_whole_file, is_security, dst_buffer, dst_buffer_size, dst_buffer_size_used))
        {
            /* cmdftp_error_code is already set in download_file function.
             */
            send_command("QUIT", FALSE, NULL);
            close_connection(&control_conn);
            return FALSE;
        }
    }
    else
    {
        if (FALSE == upload_file(src_filename, is_security, dst_buffer, dst_buffer_size, dst_buffer_size_used))
        {
            /* cmdftp_error_code is already set in upload_file function.
             */
            send_command("QUIT", FALSE, NULL);
            close_connection(&control_conn);
            return FALSE;
        }
    }

    send_command("QUIT", FALSE, NULL);
    close_connection(&control_conn);

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */
static unsigned long id_callback(void)
{
    return SYSFUN_TaskIdSelf();
}

static void init_ssl_library()
{
    SSL_METHOD *meth;

    CRYPTO_set_id_callback(id_callback);

    /* to register the available ciphers and digests used by FTPS.
     */
    SSL_library_init();

    /* TLS/SSL handshake will understand SSLv2, SSLv3 and TLSv1 protocols.
     */
    meth = SSLv23_method();

    ssl_context = SSL_CTX_new(meth);
    SSL_CTX_set_verify(ssl_context, SSL_VERIFY_NONE, NULL);
}

static BOOL_T send_command(char *cmd, BOOL_T to_recv_answer, UI32_T *answer_p)
{
    char *s;
    UI32_T len;
    UI32_T rv = 0;

    if (   (TRUE == to_recv_answer)
        && (NULL == answer_p))
    {
        return FALSE;
    }

    len = strlen(cmd) + 2;
    s = malloc(len + 1);
    if (NULL == s)
    {
        return FALSE;
    }

    sprintf(s, "%s\r\n", cmd);

    rv = my_raw_write(s, len, &control_conn);
    if (rv <= 0)
    {
        free(s);
        return FALSE;
    }

    if (TRUE == to_recv_answer)
    {
        if (FALSE == recv_answer(answer_p))
        {
            return FALSE;
        }
    }

    free(s);
    return TRUE;
}

static BOOL_T greeting()
{
    UI32_T answer;

    if (FALSE == recv_answer(&answer))
    {
        return FALSE;
    }

    if (220 != answer)
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL_T login(UI8_T *username, UI8_T *password)
{
    UI32_T answer;

    if (   (NULL == username)
        || (NULL == password))
    {
        return FALSE;
    }

    sprintf(cmd_buffer, "USER %s", username);

    if (FALSE == send_command(cmd_buffer, TRUE, &answer))
    {
        return FALSE;
    }

    if (230 == answer)
    {
        /* password is unnecessary
         */
        return TRUE;
    }
    else if (331 != answer)
    {
        /* it should ask to provide password at this time
         */
        return FALSE;
    }

    /* password is necessary
     */
    sprintf(cmd_buffer, "PASS %s", password);

    if (FALSE == send_command(cmd_buffer, TRUE, &answer))
    {
        return FALSE;
    }

    if (answer != 230)
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL_T get_remote_file_size(UI8_T *filename, UI32_T *file_size_p)
{
    UI32_T answer;
    unsigned long ul;

    sprintf(cmd_buffer, "SIZE %s", filename);

    if (FALSE == send_command(cmd_buffer, TRUE, &answer))
    {
        return FALSE;
    }

    if (213 != answer)
    {
        return FALSE;
    }

    sscanf(cmd_buffer + 4, "%lu", &ul);
    *file_size_p = ul;
    return TRUE;
}

static BOOL_T ftp_connect(CMDFTP_Conn_T *conn_p)
{
    I32_T s;

    s = socket(conn_p->sockaddr_p->sa_family, SOCK_STREAM, 0);
    if (s < 0)
    {
        return FALSE;
    }

#if HAVE_SYS_SOCKET_H
    {
        I32_T opvalue = 8;

        if (setsockopt(s, IPPROTO_IP, IP_TOS, &opvalue, slen) < 0)
        {
            return FALSE;
        }
    }
#endif /* #if HAVE_SYS_SOCKET_H */

    if (connect(s, conn_p->sockaddr_p, conn_p->sockaddr_len) < 0)
    {
        return FALSE;
    }

    conn_p->socket = s;
    return TRUE;
}

static BOOL_T init_address_for_conn(CMDFTP_Conn_T *conn_p, L_INET_AddrIp_T *ip_addr_p)
{
    memset(&conn_p->sockaddr, 0, sizeof(conn_p->sockaddr));
    conn_p->sockaddr_p = NULL;
    conn_p->sockaddr_len = 0;

    switch (ip_addr_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            conn_p->sockaddr_len = sizeof(struct sockaddr_in);
            conn_p->sockaddr_p = (struct sockaddr *)&conn_p->sockaddr.sin4;
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            conn_p->sockaddr_len = sizeof(struct sockaddr_in6);
            conn_p->sockaddr_p = (struct sockaddr *)&conn_p->sockaddr.sin6;
            break;

        default:
            return FALSE;
    }

    return L_INET_InaddrToSockaddr(ip_addr_p, XFER_DNLD_DFLT_FTP_PORT, conn_p->sockaddr_len, conn_p->sockaddr_p);
}

static I32_T my_ssl_read(char *buf, UI32_T n, CMDFTP_Conn_T *conn_p)
{
    struct timeval timeout;

    timeout.tv_sec = CMDFTP_DFLT_FTP_TIMEOUT;
    timeout.tv_usec = 0;

    while (1)
    {
        I32_T error_value;
        I32_T rv;
        BOOL_T read_would_block = FALSE;
        fd_set read_fs;

        FD_ZERO(&read_fs);
        FD_SET(conn_p->socket, &read_fs);

        rv = select(conn_p->socket + 1, &read_fs, NULL, NULL, &timeout);
        if (rv <= 0)
        {
            return -1;
        }
        if (0 == FD_ISSET(conn_p->socket, &read_fs))
        {
            continue;
        }

        do
        {
            read_would_block = FALSE;

            rv = SSL_read(conn_p->ssl, buf, n);
            error_value = SSL_get_error(conn_p->ssl, rv);
            switch (error_value)
            {
                case SSL_ERROR_NONE:
                    return rv;


                case SSL_ERROR_WANT_READ:
                    read_would_block = TRUE;
                    break;

                case SSL_ERROR_ZERO_RETURN:
                case SSL_ERROR_WANT_WRITE:
                default:
                    return -1;
            }

        }
        while (SSL_pending(conn_p->ssl) && read_would_block == FALSE);
    }
}

static I32_T my_socket_read(char *buf, UI32_T n, UI32_T socket)
{
    I32_T rv;
    fd_set read_fs;
    struct timeval timeout;

    FD_ZERO(&read_fs);
    FD_SET(socket, &read_fs);
    timeout.tv_sec = CMDFTP_DFLT_FTP_TIMEOUT;
    timeout.tv_usec = 0;

    while (1)
    {
        rv = select(socket + 1, &read_fs, NULL, NULL, &timeout);
        if (rv <= 0)
        {
            return -1;
        }
        if (0 == FD_ISSET(socket, &read_fs))
        {
            continue;
        }

        return recv(socket, buf, n, 0);
    }
}

static I32_T my_raw_read(char *buf, UI32_T n, CMDFTP_Conn_T *conn_p)
{
    if (NULL != conn_p->ssl)
    {
        return my_ssl_read(buf, n, conn_p);
    }

    return my_socket_read(buf, n, conn_p->socket);
}

static I32_T my_ssl_write(char *buf, UI32_T n, CMDFTP_Conn_T *conn_p)
{
    UI32_T offset = 0;
    struct timeval timeout;

    timeout.tv_sec = CMDFTP_DFLT_FTP_TIMEOUT;
    timeout.tv_usec = 0;

    while (1)
    {
        I32_T error_value;
        I32_T rv;
        fd_set write_fs;

        FD_ZERO(&write_fs);
        FD_SET(conn_p->socket, &write_fs);

        rv = select(conn_p->socket + 1, NULL, &write_fs, NULL, &timeout);
        if (rv <= 0)
        {
            return -1;
        }
        if (0 == FD_ISSET(conn_p->socket, &write_fs))
        {
            continue;
        }

        rv = SSL_write(conn_p->ssl, buf + offset, n);
        error_value = SSL_get_error(conn_p->ssl, rv);
        switch (error_value)
        {
            case SSL_ERROR_NONE:
                n -= rv;
                offset += rv;

                /* all of the buffer is sent.
                 */
                if (n <= 0)
                {
                    return offset;
                }
                break;

            case SSL_ERROR_WANT_WRITE:
                /* it would block when reading.
                 */
                break;

            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_ZERO_RETURN:
            default:
                return -1;
        }
    }
}

static I32_T my_socket_write(char *buf, UI32_T n, UI32_T socket)
{
    I32_T rv;
    fd_set write_fs;
    struct timeval timeout;

    FD_ZERO(&write_fs);
    FD_SET(socket, &write_fs);
    timeout.tv_sec = CMDFTP_DFLT_FTP_TIMEOUT;
    timeout.tv_usec = 0;

    while (1)
    {
        rv = select(socket + 1, NULL, &write_fs, NULL, &timeout);
        if (rv <= 0)
        {
            return -1;
        }

        if (0 == FD_ISSET(socket, &write_fs))
        {
            continue;
        }

        return send(socket, buf, n, 0);
    }
}

static I32_T my_raw_write(char *buf, UI32_T n, CMDFTP_Conn_T *conn_p)
{
    if (NULL != conn_p->ssl)
    {
        return my_ssl_write(buf, n, conn_p);
    }

    return my_socket_write(buf, n, conn_p->socket);
}

static void reset_cmd_buffer()
{
    cmd_ptr = cmd_buffer + CMDFTP_BUF_SIZE;
}

static char *recv_line(CMDFTP_Conn_T *conn_p)
{
    char *new_ptr;
    UI32_T n;
    UI32_T len;

    if (*cmd_ptr == 0)
    {
        I32_T bytes = my_raw_read(cmd_buffer, CMDFTP_BUF_SIZE, conn_p);

        if (bytes <= 0)
        {
            return 0;
        }

        cmd_buffer[bytes] = 0;
        cmd_ptr = cmd_buffer;

        if (*cmd_ptr == '\n' && cmd_buffer[CMDFTP_BUF_SIZE - 1] == '\r')
        {
            cmd_ptr++;
        }
    }

    new_ptr = strchr(cmd_ptr, '\r');
    if (!new_ptr)
    {
        new_ptr = cmd_buffer + CMDFTP_BUF_SIZE;
    }

    n = new_ptr - cmd_ptr;
    strncpy(cmd_line, cmd_ptr, n); cmd_line[n] = 0;
    cmd_ptr = new_ptr;

    if (cmd_ptr == cmd_buffer + CMDFTP_BUF_SIZE)
    {
        I32_T bytes = my_raw_read(cmd_buffer, CMDFTP_BUF_SIZE, conn_p);

        if (bytes <= 0)
        {
            return 0;
        }

        cmd_buffer[bytes] = 0;
        cmd_ptr = cmd_buffer;

        new_ptr = strchr(cmd_ptr, '\r');
        if (!new_ptr)
        {
            new_ptr = cmd_buffer + CMDFTP_BUF_SIZE;
        }

        n = new_ptr - cmd_ptr;
        len = strlen(cmd_line);
        strncat(cmd_line, cmd_ptr, n);
        cmd_line[len + n] = 0;
        cmd_ptr = new_ptr;
    }

    if (*cmd_ptr == '\r')
    {
        cmd_ptr++;

        if (*cmd_ptr == '\n')
        {
            cmd_ptr++;
        }
    }

    return cmd_line;
}

static BOOL_T recv_confirm()
{
    UI32_T answer;

    if (FALSE == recv_answer(&answer))
    {
        return FALSE;
    }

    if (226 != answer)
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL_T recv_answer(UI32_T *answer_p)
{
    unsigned long code;
    char *answer_str_p;
    char str_code[] = { 0, 0, 0, ' ', 0 };

    if (NULL == answer_p)
    {
        return FALSE;
    }

    reset_cmd_buffer();

    answer_str_p = recv_line(&control_conn);

    if (   (NULL == answer_str_p)
        || (strlen(answer_str_p) < 3))
    {
        return FALSE;
    }

    strncpy(str_code, answer_str_p, 3);

    if (answer_str_p[3] == '-')
    {
        while ((answer_str_p = recv_line(&control_conn)))
        {
            if (strncmp(answer_str_p, str_code, 4) == 0)
            {
                break;
            }
        }

        if (NULL == answer_str_p)
        {
            return FALSE;
        }
    }

    sscanf(str_code, "%lu", &code);

    if (code >= 400)
    {
        cmdftp_remote_answer = code;
        return FALSE;
    }

    /* For error only.
     */
    cmdftp_remote_answer = 0;

    *answer_p = code;
    return TRUE;
}

static BOOL_T get_pasv_port_from_pasv_cmd(UI32_T *port_p)
{
    UI32_T i;
    UI32_T len;
    UI32_T answer;
    unsigned long param[6];
    char *port_str_p;

    if (send_command("PASV", TRUE, &answer) == FALSE)
    {
        return FALSE;
    }

    /* 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
     */
    if (answer != 227)
    {
        return FALSE;
    }

    port_str_p = cmd_buffer + 4;
    len = strlen(port_str_p);

    /* set all non-digit characters to ' ' for later to parse 6 numbers
     */
    for (i = 0; i < len; i++)
    {
        if (port_str_p[i] >= '0' && port_str_p[i] <= '9')
        {
            continue;
        }

        port_str_p[i] = ' ';
    }

    if (sscanf(port_str_p, "%lu %lu %lu %lu %lu %lu", param, param + 1,
        param + 2, param + 3, param + 4, param + 5) != 6)
    {
        return FALSE;
    }

    *port_p = param[4] * 256 + param[5];

    return TRUE;
}

static BOOL_T get_pasv_port_from_epsv_cmd(UI32_T *port_p)
{
    UI32_T i;
    UI32_T answer;
    UI32_T delimiter_count = 3; /* number of delimiters to skip for TCP port */
    unsigned long param;
    char *port_str_p;
    char *tmp_str_p;
    char *parsing_str_p;

    if (send_command("EPSV", TRUE, &answer) == FALSE)
    {
        return FALSE;
    }

    /* <text indicating server is entering extended passive mode> \
     *     (<d><d><d><tcp-port><d>)
     *
     * 2yz Positive Completion
     * x2z Connections
     * xy9 Extended Passive Mode Entered
     */
    if (answer != 229)
    {
        return FALSE;
    }

    port_str_p = cmd_buffer + 4;

    /* Remove the last '(' and ')' for later to parse the tcp-port easily.
     */
    tmp_str_p = parsing_str_p = strrchr(port_str_p, '(');
    if (tmp_str_p == NULL)
    {
        return FALSE;
    }
    *tmp_str_p = ' ';
    tmp_str_p = strrchr(port_str_p, ')');
    if (tmp_str_p == NULL)
    {
        return FALSE;
    }
    *tmp_str_p = ' ';

    /* Point to the parsing string.
     */
    parsing_str_p++;

    /* Skip specifc number of delimiters in order to get TCP port.
     */
    for (; '\0' != *parsing_str_p && delimiter_count; parsing_str_p++)
    {
        if (*parsing_str_p == '|')
        {
            delimiter_count--;
        }
    }

    if (   ('\0' == *parsing_str_p)
        || (delimiter_count > 0)
        )
    {
        return FALSE;
    }

    if (sscanf(parsing_str_p, "%lu", &param) != 1)
    {
        return FALSE;
    }

    *port_p = param;

    return TRUE;
}

static BOOL_T get_pasv_port(UI32_T *port_p)
{
    /* Try to get passive port from PASV command (RFC 959) first. If it is fail,
     * try to get the port from EPSV command (RFC 2428).
     */

    if (get_pasv_port_from_pasv_cmd(port_p) == TRUE)
    {
        return TRUE;
    }

    if (get_pasv_port_from_epsv_cmd(port_p) == TRUE)
    {
        return TRUE;
    }

    return FALSE;
}

static BOOL_T download_file(UI8_T *src_filename, BOOL_T get_whole_file, BOOL_T is_security, UI8_T *dst_buffer_p, UI32_T dst_buffer_size, UI32_T *dst_buffer_size_used_p)
{
    I32_T start_pos;
    UI32_T port;
    UI32_T answer;
    UI32_T total_size;

    start_pos = 0;

    if (FALSE == send_command("TYPE I", TRUE, &answer))
    {
        cmdftp_error_code = CMDFTP_Error_CONN;
        return FALSE;
    }

    if (FALSE == get_remote_file_size(src_filename, &total_size))
    {
        cmdftp_error_code = CMDFTP_Error_FILE_NOT_FOUND;
        return FALSE;
    }

    /* only check the size when need to get a whole file
     */
    if (get_whole_file == TRUE)
    {
        /* check the remote size
         */
        if (total_size > dst_buffer_size)
        {
            cmdftp_error_code = CMDFTP_Error_SIZE_EXCEED;
            return FALSE;
        }
    }

    /* if just need to get a piece of file, the buffer_size shall be used to indicate
     * how much data should be obtained.
     */
    if (get_whole_file == FALSE)
    {
        total_size = dst_buffer_size;
    }

    if (FALSE == get_pasv_port(&port))
    {
        cmdftp_error_code = CMDFTP_Error_UNER;
        return FALSE;
    }

    if (FALSE == set_data_conn_port(port))
    {
        cmdftp_error_code = CMDFTP_Error_UNER;
        return FALSE;
    }

    if (FALSE == ftp_connect(&data_conn))
    {
        cmdftp_error_code = CMDFTP_Error_CONN_DATA;
        return FALSE;
    }

    sprintf(cmd_buffer, "RETR %s", src_filename);

    if (FALSE == send_command(cmd_buffer, TRUE, &answer))
    {
        cmdftp_error_code = CMDFTP_Error_CONN;
        close_connection(&data_conn);
        return FALSE;
    }

    /* 125:Data connection already open; transfer starting (E.g., Microsoft FTP server)
     * 150:File status okay; about to open data connection
     */
    if (   (125 != answer)
        && (150 != answer))
    {
        cmdftp_error_code = CMDFTP_Error_CONN_DATA;
        close_connection(&data_conn);
        return FALSE;
    }

    if (TRUE == is_security)
    {
        if (FALSE == tls_ssl_connect(&data_conn))
        {
            cmdftp_error_code = CMDFTP_Error_NOT_ACCEPT_CIPHERS;
            close_connection(&data_conn);
            return FALSE;
        }
    }

    while (start_pos < total_size)
    {
        I32_T bytes;
        UI32_T toread;

        CMDFTP_NotifyTransmittingStatus(start_pos * 100 / total_size);

        toread = ((total_size - start_pos) < CMDFTP_DFLT_FTP_BLOCK_SIZE) ? total_size - start_pos : CMDFTP_DFLT_FTP_BLOCK_SIZE;

        bytes = my_raw_read((char *)(dst_buffer_p + start_pos), toread, &data_conn);
        if (bytes <= 0)
        {
            cmdftp_error_code = CMDFTP_Error_TIMEOUT;
            close_connection(&data_conn);
            return FALSE;
        }

        start_pos += bytes;
    }

    if (start_pos < total_size)
    {
        cmdftp_error_code = CMDFTP_Error_UNER;
        close_connection(&data_conn);
        return FALSE;
    }

    close_connection(&data_conn);

#if 0
    /* Due to different FTP servers response different answer code when we
     * close data connection actively. Some reply 426 then 226. Some reply
     * 226 only. And some even no response any answer code. So we decide to
     * remove this check for this situation.
     */
    if (FALSE == recv_confirm())
    {
        cmdftp_error_code = CMDFTP_Error_UNER;
        return FALSE;
    }
#endif

    *dst_buffer_size_used_p = total_size;

    return TRUE;
}

static BOOL_T upload_file(UI8_T *dst_filename, BOOL_T is_security, UI8_T *src_buffer, UI32_T src_buff_size, UI32_T *src_buff_size_uploaded)
{
    I32_T start_pos, total_size;
    UI32_T port;
    UI32_T answer;

    if (FALSE == send_command("TYPE I", TRUE, &answer))
    {
        cmdftp_error_code = CMDFTP_Error_CONN;
        return FALSE;
    }

    total_size = src_buff_size;
    start_pos = 0;

    if (FALSE == get_pasv_port(&port))
    {
        cmdftp_error_code = CMDFTP_Error_UNER;
        return FALSE;
    }

    if (FALSE == set_data_conn_port(port))
    {
        cmdftp_error_code = CMDFTP_Error_UNER;
        return FALSE;
    }

    if (FALSE == ftp_connect(&data_conn))
    {
        cmdftp_error_code = CMDFTP_Error_CONN_DATA;
        return FALSE;
    }

    sprintf(cmd_buffer, "STOR %s", dst_filename);

    if (FALSE == send_command(cmd_buffer, TRUE, &answer))
    {
        cmdftp_error_code = CMDFTP_Error_CONN;
        close_connection(&data_conn);
        return FALSE;
    }

    /* 125:Data connection already open; transfer starting (E.g., Microsoft FTP server)
     * 150:File status okay; about to open data connection
     */
    if (   (125 != answer)
        && (150 != answer))
    {
        cmdftp_error_code = CMDFTP_Error_UNER;
        close_connection(&data_conn);
        return FALSE;
    }

    if (TRUE == is_security)
    {
        if (FALSE == tls_ssl_connect(&data_conn))
        {
            cmdftp_error_code = CMDFTP_Error_NOT_ACCEPT_CIPHERS;
            close_connection(&data_conn);
            return FALSE;
        }
    }

    while (start_pos < total_size)
    {
        UI32_T toread;
        I32_T written;

        CMDFTP_NotifyTransmittingStatus(start_pos * 100 / total_size);

        toread = ((total_size - start_pos) < CMDFTP_DFLT_FTP_BLOCK_SIZE) ? total_size - start_pos : CMDFTP_DFLT_FTP_BLOCK_SIZE;

        written = my_raw_write((char *)(src_buffer + start_pos), toread, &data_conn);
        if (written != toread)
        {
            cmdftp_error_code = CMDFTP_Error_TIMEOUT;
            close_connection(&data_conn);
            return FALSE;
        }

        start_pos += toread;
    }

    if (start_pos < total_size)
    {
        cmdftp_error_code = CMDFTP_Error_UNER;
        close_connection(&data_conn);
        return FALSE;
    }

    close_connection(&data_conn);

    if (FALSE == recv_confirm())
    {
        cmdftp_error_code = CMDFTP_Error_UNER;
        return FALSE;
    }

    *src_buff_size_uploaded = start_pos;

    return TRUE;
}

static BOOL_T tls_ssl_connect(CMDFTP_Conn_T *conn_p)
{
    /* establish the SSL/TLS channel first before to transfer data
     */
    BIO *sbio = NULL;
    UI32_T ret;

    conn_p->ssl = SSL_new(ssl_context);
    if (NULL == conn_p->ssl)
    {
        return FALSE;
    }

    sbio = BIO_new_socket(conn_p->socket, BIO_NOCLOSE);
    if (NULL == sbio)
    {
        SSL_free(conn_p->ssl);
        conn_p->ssl = NULL;
        return FALSE;
    }

    SSL_set_bio(conn_p->ssl, sbio, sbio);

    ret = SSL_connect(conn_p->ssl);
    if (ret <= 0)
    {
        SSL_free(conn_p->ssl);
        conn_p->ssl = NULL;
        return FALSE;
    }

    return TRUE;
}

static void close_connection(CMDFTP_Conn_T *conn_p)
{
    if (NULL == conn_p)
    {
        return;
    }

    if (NULL != conn_p->ssl)
    {
        SSL_shutdown(conn_p->ssl);
        SSL_free(conn_p->ssl);
        conn_p->ssl = NULL;
    }

    if (conn_p->socket > 0)
    {
        close(conn_p->socket);
        conn_p->socket = -1;
    }
}

static void shutdown_connection(CMDFTP_Conn_T *conn_p)
{
    if (NULL == conn_p)
    {
        return;
    }

    if (NULL != conn_p->ssl)
    {
        SSL_shutdown(conn_p->ssl);
        SSL_free(conn_p->ssl);
        conn_p->ssl = NULL;
    }

    if (conn_p->socket > 0)
    {
        shutdown(conn_p->socket, SHUT_RDWR);
        conn_p->socket = -1;
    }
}

static BOOL_T set_data_conn_port(UI32_T port)
{
    switch (ip_addr.type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            data_conn.sockaddr.sin4.sin_port = htons(port);
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            data_conn.sockaddr.sin6.sin6_port = htons(port);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - CMDFTP_NotifyTransmittingStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the file is transmitting.
 * INPUT   : percent - File tansmitt status.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void CMDFTP_NotifyTransmittingStatus(UI32_T percent)
{
    SYS_TYPE_CallBack_T  *fun_list;

    for(fun_list = cmdftp_transmitt_callback; fun_list != NULL; fun_list = fun_list->next)
    {
        fun_list->func(percent);
    }
}

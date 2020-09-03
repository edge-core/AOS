#include <string.h>
#include <stdlib.h>
#include "libtacacs.h"
#include "l_stdlib.h"
#include "sysfun.h"
#include "tacacs_om.h"
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define AF_INET4 PF_INET
#define s_close(a) close(a)

/* NAMING CONSTANT DECLARATIONS
 */
/* define naming constant TAC_PACKET_DEBUG_MODE
 * to build tac_packet.c with DEBUG version
 * And let following macros print debug messages
 */
#define TAC_PACKET_DEBUG_MODE
/* MACRO FUNCTION DECLARATIONS
 */
#ifdef TAC_PACKET_DEBUG_MODE
#include "backdoor_mgr.h"
    #define TAC_PACKET_TRACE(fmt, ...)    (TACACS_LOG(TACACS_MGR_GetDebugFlag(TACACS_PACKET), fmt, ##__VA_ARGS__))

#else

    #define TAC_PACKET_TRACE(fmt, ...)    ((void)0)

#endif /* TAC_PACKET_DEBUG_MODE */

#if 0
BOOL_T tac_connect( UI32_T server_ip, UI8_T *key, UI32_T port,  UI32_T timeout, struct session *session_s )
{
    struct sockaddr_in s;
    int sock_fd;

    if (port==0) port=TAC_PLUS_PORT;

    /* connection */
    if (( sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        TAC_PACKET_TRACE("[tac_connect] create socket failed(%d)", sock_fd);
        return FALSE;
    }

    session_s->sock = sock_fd;
    memcpy(&s.sin_addr,&server_ip,sizeof(struct in_addr));/*TACACS_OM_Get_Server_IP();*/ /*ES3550C-77-00084*//*Mercury_V2-00220*/
    if (s.sin_addr.s_addr == 0xffffff || s.sin_addr.s_addr == 0)
    {
        TAC_PACKET_TRACE("\r\n[tac_connect] bad server ip (%lu)", server_ip);
        tac_close(session_s);
        return FALSE;
    }
    s.sin_family = AF_INET4;
    s.sin_port = L_STDLIB_Hton16(port);

    if (connect(sock_fd, (struct sockaddr *)&s, sizeof(s)) < 0)
    {
        TAC_PACKET_TRACE("[tac_connect] call connect() failed");
        tac_close(session_s);
        return FALSE;
    }

    srand((unsigned int)(call_time()));
    /* session_s->session_id is Host Bye Order */
    session_s->session_id = rand() % 255;

    /* sequence to zero */
    session_s->seq_no = 0;
    /* and dont see using this */
    session_s->last_exch = call_time();
    session_s->timeout   = timeout;

    TAC_PACKET_TRACE("[tac_connect] Success! session_id(%lu), seq_no(%d)",
        session_s->session_id, session_s->seq_no);

    return TRUE;
}
#endif

/*
 * tac_connect_nonblocking(): Connect to TACACS+ server using no blocking IO.
 * input
 *      server_ip   - server ip address
 *      key         - secret key
 *      port        - server port
 *      timeout     - waiting for connection to establish (second)
 * output
 *      session_s
 * return
 *      1           - SUCCESS
 *      0           - FAILURE
 *     -1           - TIMEOUT
 */
int tac_connect_nonblocking(
    UI32_T server_ip,
    UI8_T *key,
    UI32_T port,
    UI32_T timeout,
    struct session *session_s
    )
{
    struct  sockaddr_in s;
    int     sock_fd, socket_status;
    int     error;
    int     res;

    if (server_ip == 0xffffff || server_ip == 0)
    {
        TAC_PACKET_TRACE("TACACS: aborted tac_connect on invalid server address\r\n");
        return 0;  /*failure*/
    }

    if (port==0) port=TAC_PLUS_PORT;

    /* create socket
     */
    error = 0;
    if (( sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        TAC_PACKET_TRACE("TACACS: aborted tac_connect on creating socket\r\n");
        return 0;  /*failure*/
    }

    session_s->sock = sock_fd;
    memcpy(&s.sin_addr, &server_ip, sizeof(struct in_addr));
    s.sin_family = AF_INET4;
    s.sin_port   = htons(port);

    /* set sock_fd socket to non-blocking
    */
    socket_status = fcntl(sock_fd, F_GETFL);
    fcntl(sock_fd, F_SETFL, socket_status | O_NONBLOCK);

    res = connect(sock_fd, (struct sockaddr *)&s, sizeof(s));
    if (res < 0)
    {
        if (errno != EINPROGRESS)
        {
            TAC_PACKET_TRACE("TACACS: connect not inprogress");
            return 0; /*failure*/
        }
    }

    if (res < 0)
    {
        fd_set  rset, wset;
        struct timeval tval;
        int len = sizeof(error);
        int sel_ret;

        FD_ZERO (&rset);
        FD_ZERO (&wset);
        FD_SET (sock_fd, &rset);
        FD_SET (sock_fd, &wset);
        tval.tv_sec  = timeout;
        tval.tv_usec = 0;

        sel_ret = select(sock_fd+1, &rset, &wset/*NULL*/, NULL, &tval);
        if (sel_ret == 0)     /*select timeout*/
        {
            TAC_PACKET_TRACE("TACACS: aborted tac_connect on timeout\r\n");

            tac_close(session_s);

            return (-1);  /*timeout*/
        }
        else if (sel_ret < 0) /*select error*/
        {
            TAC_PACKET_TRACE("TACACS: aborted tac_connect on select error\r\n");

            tac_close(session_s);

            return 0;  /*failure*/
        }

        if (FD_ISSET(sock_fd, &rset) || FD_ISSET(sock_fd, &wset))
        {
            if (getsockopt (sock_fd, SOL_SOCKET, SO_ERROR, &error, (unsigned int *)&len) < 0)
            {
                TAC_PACKET_TRACE("TACACS: aborted tac_connect on handshaking error\r\n");

                tac_close(session_s);

                return 0;  /*failure*/
            }

            if (error)
            {
                TAC_PACKET_TRACE("TACACS: aborted tac_connect on handshaking error(%d)\r\n", error);

                tac_close(session_s);

                return 0;  /*failure*/
            }

        }
        else
        {
            TAC_PACKET_TRACE("TACACS: aborted tac_connect on sock_fd(%d) no set\r\n", sock_fd);

            tac_close(session_s);

            return 0;  /*failure*/
        }

        /* create connection success !
         */
        TAC_PACKET_TRACE("TACACS: connection established\r\n");
    }
    else /*connect() return success! */
    {
        TAC_PACKET_TRACE("TACACS: connection established immediately\r\n");
    }

    /* set socket to original status
    */
    fcntl(sock_fd, F_SETFL, socket_status);

    srand((unsigned int)(call_time()));
    session_s->session_id = htonl((rand() % 255));

    /* sequence to zero */
    session_s->seq_no     = 0;
    /* and dont see using this */
    session_s->last_exch  = call_time();
    session_s->timeout    = timeout;

    return 1;
}

/*
	tac_close - Close connection with TACACS+ server
*/
void tac_close(struct session* session) {
     s_close(session->sock);
}


/*
 * create_md5_hash(): create an md5 hash of the "session_id", "the user's
 * key", "the version number", the "sequence number", and an optional
 * 16 bytes of data (a previously calculated hash). If not present, this
 * should be NULL pointer.
 *
 * Write resulting hash into the array pointed to by "hash".
 *
 * The caller must allocate sufficient space for the resulting hash
 * (which is 16 bytes long). The resulting hash can safely be used as
 * input to another call to create_md5_hash, as its contents are copied
 * before the new hash is generated.
 */
int create_md5_hash(int session_id, char* key, UI8_T version, UI8_T seq_no, UI8_T* prev_hash, UI8_T* hash)
{
    UI8_T md_stream[64], *mdp;
    int md_len;

    md_len = sizeof(session_id) + strlen(key) + sizeof(version) + sizeof(seq_no);
    if (prev_hash) {
	md_len += MD5_LEN;
    }

    mdp = &(md_stream[0]);
    memcpy(mdp,(char *)&session_id, sizeof(session_id));
    mdp += sizeof(session_id);
    memcpy(mdp,key,strlen(key));
    mdp += strlen(key);
    memcpy(mdp,&version,sizeof(version));
    mdp += sizeof(version);
    memcpy(mdp,&seq_no,sizeof(seq_no));
    mdp += sizeof(seq_no);
    if (prev_hash) {
        memcpy(mdp,prev_hash, MD5_LEN);
	mdp += MD5_LEN;
    }
    tac_md5_calc (hash,&(md_stream[0]), md_len);
   return 0;
}


/*
 * Overwrite input data with en/decrypted version by generating an MD5 hash and
 * xor'ing data with it.
 *
 * When more than 16 bytes of hash is needed, the MD5 hash is performed
 * again with the same values as before, but with the previous hash value
 * appended to the MD5 input stream.
 *
 * Return 0 on success, -1 on failure.
 */
int md5_xor(HDR* hdr, UI8_T* data, char* key) {
    int i, j;
    UI8_T hash[MD5_LEN];       /* the md5 hash */
    UI8_T last_hash[MD5_LEN];  /* the last hash we generated */
    UI8_T *prev_hashp = (UI8_T *) NULL;       /* pointer to last created
						 * hash */
    int data_len;
    int session_id;
    UI8_T version;
    UI8_T seq_no;


    data_len = L_STDLIB_Ntoh32(hdr->datalength);
    session_id = hdr->session_id; /* always in network order for hashing */
    version = hdr->version;
    seq_no = hdr->seq_no;
    if (!key) return 0;
    for (i = 0; i < data_len; i += 16) {
	create_md5_hash(session_id, key, version, seq_no, prev_hashp, hash);
        memcpy(last_hash,hash,MD5_LEN);
	prev_hashp = last_hash;
	for (j = 0; j < 16; j++) {

	    if ((i + j) >= data_len) {
		hdr->encryption = (hdr->encryption == TAC_PLUS_CLEAR) ? TAC_PLUS_ENCRYPTED : TAC_PLUS_CLEAR;
		return 0;
	    }
	    data[i + j] ^= hash[j];
	}
    }
    hdr->encryption = (hdr->encryption == TAC_PLUS_CLEAR) ? TAC_PLUS_ENCRYPTED : TAC_PLUS_CLEAR;
    return 0;
}


/* Reading n bytes from descriptor fd to array ptr with timeout t sec
 * Timeout set for each read
 *
 * Return -1 if error, eof or timeout. Else returns
 * number reads bytes. */
int sockread(struct session* session, int fd, UI8_T* ptr, int nbytes, int timeout)
{
    int nleft, nread;

    if (fd == -1)
	return -1;
    nleft = nbytes;

    while (nleft > 0)
    {
	nread = recv(fd, ptr, nleft,0);
        if (nread < 0)
        {
            TAC_PACKET_TRACE("[sockread] recv() return error %d", nread);
	    return (-1);        /* error */
        }
        else if (nread == 0)
        {
            TAC_PACKET_TRACE("[sockread] %s fd %d eof (connection closed)",
                session->port, fd);
            return (-1);        /* eof */
	}
	nleft -= nread;
	if (nleft)
	    ptr += nread;
    }

    return (nbytes - nleft);
}


/* Write n bytes to descriptor fd from array ptr with timeout t
 * seconds. Note the timeout is applied to each write, not for the
 * overall operation.
 *
 * Return -1 on error, eof or timeout. Otherwise return number of
 * bytes written. */
int sockwrite(struct session* session, int fd, UI8_T* ptr, int bytes, int timeout)
{
    int remaining, sent;

    if (fd == -1)
	return -1;

    sent = 0;
    remaining = bytes;

    while (remaining > 0)
    {
	sent = send(fd, ptr, remaining,0);
        if (sent <= 0)
        {
            TAC_PACKET_TRACE("[sockwrite] send() return error %d", sent);
	    return (sent);      /* error */
	}
	remaining -= sent;
	ptr += sent;
    }
    return (bytes - remaining);
}


/*
	read_packet - Read a packet and decrypt it from TACACS+ server
	return
		pointer to a newly allocated memory buffer containing packet data
		NULL	FAILURE
*/
#if 0
/* obsolete by TACACS_LIB_ReadPacket()
 * because there is aligment issue in this function
 */
UI8_T *read_packet(struct session* session ,UI8_T *pkt, UI8_T *secret) {
    HDR hdr;
    UI8_T *data;/*ES3526V-CO-00229*/
    int len;

    if (session == NULL)
	return NULL;

    /* read a packet header */
    len = sockread(session, session->sock, (UI8_T *) & hdr, TAC_PLUS_HDR_SIZE, TAC_PLUS_READ_TIMEOUT);
    if (len != TAC_PLUS_HDR_SIZE) {
	return(NULL);
    }

    if ((hdr.version & TAC_PLUS_MAJOR_VER_MASK) != TAC_PLUS_MAJOR_VER) {
	return(NULL);
    }

    /* get memory for the packet */
    len = TAC_PLUS_HDR_SIZE + L_STDLIB_Ntoh32(hdr.datalength);

    /* initialise the packet */
    memcpy(pkt,(char *)&hdr, TAC_PLUS_HDR_SIZE);

    /* the data start here */
    data = pkt + TAC_PLUS_HDR_SIZE;

    /* read the rest of the packet data */
    if (sockread(session, session->sock, data,
        L_STDLIB_Ntoh32(hdr.datalength), TAC_PLUS_READ_TIMEOUT) !=
        L_STDLIB_Ntoh32(hdr.datalength))
    {
	return (NULL);
    }
    session->seq_no++;           /* should now equal that of incoming packet */
    session->last_exch = call_time();

    if (session->seq_no != hdr.seq_no) {
	return (NULL);
    }

    /* decrypt the data portion */
    if (secret && md5_xor((HDR *)pkt, data, (char *)secret)) {
	return (NULL);
    }

    session->version = hdr.version;

    return (pkt);
}
#endif /* if 0 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_ReadPacket
 *---------------------------------------------------------------------------
 * PURPOSE  : Read data packet from TACACS+ server
 * INPUT    : session_p, secret_p
 * OUTPUT   : pkt_p
 * RETURN   : TRUE -- success / FALSE -- failure
 * NOTE     : None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_LIB_ReadPacket(struct session* session_p, TACACS_LIB_Packet_T *pkt_p, UI8_T *secret_p)
{
    UI32_T  length;

    /* read a packet header */
    length = sockread(session_p, session_p->sock, (UI8_T*)&pkt_p->packet_header,
                TAC_PLUS_HDR_SIZE, TAC_PLUS_READ_TIMEOUT);

    /* check readed header length */
    if (TAC_PLUS_HDR_SIZE != length)
    {
        TAC_PACKET_TRACE("[TACACS_LIB_ReadPacket] error! sockread() return len(%lu) != %u",
            length, TAC_PLUS_HDR_SIZE);
        return FALSE;
    }

    /* check header version */
    if ((pkt_p->packet_header.version & TAC_PLUS_MAJOR_VER_MASK) != TAC_PLUS_MAJOR_VER)
    {
        TAC_PACKET_TRACE("[TACACS_LIB_ReadPacket] error! bad version %d (expected major ver %d)",
            pkt_p->packet_header.version, TAC_PLUS_MAJOR_VER);
        return FALSE;
    }

    length = L_STDLIB_Ntoh32(pkt_p->packet_header.datalength);

    /* check whether buffer is large enough */
    if (TACACS_LIB_MAX_LEN_OF_PACKET_BODY < length)
    {
        TAC_PACKET_TRACE("[TACACS_LIB_ReadPacket] error! body len(%lu) > buffer len(%u))",
            length, TACACS_LIB_MAX_LEN_OF_PACKET_BODY);
        return FALSE;
    }

    /* read the rest of the packet data */
    if (sockread(session_p, session_p->sock, (UI8_T*)&pkt_p->packet_body, length,
            TAC_PLUS_READ_TIMEOUT) != length)
    {
        TAC_PACKET_TRACE("[TACACS_LIB_ReadPacket] error! sockread() return length != %lu",
            length);
        return FALSE;
    }

    /* check repled seq_no with session->seq_no */
    session_p->seq_no++;
    session_p->last_exch = call_time();
    if (session_p->seq_no != pkt_p->packet_header.seq_no)
    {
        TAC_PACKET_TRACE("[TACACS_LIB_ReadPacket] error! session->seq_no(%d) != hdr.seq_no(%d)",
            session_p->seq_no, pkt_p->packet_header.seq_no);
        return FALSE;
    }

    /* decrypt the data portion */
    if (0 != md5_xor(&pkt_p->packet_header, pkt_p->packet_body, (char *)secret_p))
    {
        TAC_PACKET_TRACE("[TACACS_LIB_ReadPacket] call md5_xor() failed");
        return FALSE;
    }

    session_p->version = pkt_p->packet_header.version;
    return TRUE;
}

/*
	write_packet - Send a data packet to TACACS+ server
		pak	pointer to packet data to send
	return
		1       SUCCESS
		0       FAILURE
*/
#if 0
/* obsolete by TACACS_LIB_WritePacket()
 * because there is aligment issue in this function
 */
int write_packet(struct session* session, unsigned char *pak, UI8_T *secret) {
    HDR *hdr = (HDR *) pak;
    unsigned char *data;
    int len;

    if (session == NULL) {
        #ifdef TAC_DEBUG
	printf("session = NULL\n");
        #endif
	return 0;
    }

    len = TAC_PLUS_HDR_SIZE + L_STDLIB_Ntoh32(hdr->datalength);


    /* the data start here */
    data = pak + TAC_PLUS_HDR_SIZE;

    /* encrypt the data portion */
    if (secret && md5_xor((HDR *)pak, data, (char *)secret)) {
        #ifdef TAC_DEBUG
	printf("%s: write_packet: error encrypting data", session->peer);
        #endif
	return 0;
    }
    if (sockwrite(session, session->sock, pak, len, TAC_PLUS_WRITE_TIMEOUT) != len) {
	return 0;
    }
    session->last_exch = call_time();
return 1;
}
#endif /* if 0 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_WritePacket
 *---------------------------------------------------------------------------
 * PURPOSE  : Send a data packet to TACACS+ server
 *            pak pointer to packet data to send
 * INPUT    : session_p, pkt_p, secret_p
 * OUTPUT   : None.
 * RETURN   : TRUE -- success / FALSE -- fail
 * NOTE     : None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_LIB_WritePacket(struct session *session_p, TACACS_LIB_Packet_T *pkt_p, UI8_T *secret_p)
{
    UI32_T  len;

    /* check input pointers */
    if ((NULL == session_p) || (NULL == pkt_p) || (NULL == secret_p))
        return FALSE;

    /* encrypt the data portion */
    if (0 != md5_xor(&pkt_p->packet_header, pkt_p->packet_body, (char *)secret_p))
    {
        TAC_PACKET_TRACE("[TACACS_LIB_WritePacket] call md5_xor() failed");
        return FALSE;
    }

    len = TAC_PLUS_HDR_SIZE + L_STDLIB_Ntoh32(pkt_p->packet_header.datalength);

    /* write packet via socket */
    if (sockwrite(session_p, session_p->sock, (UI8_T*)pkt_p, len, TAC_PLUS_WRITE_TIMEOUT) != len)
    {
        TAC_PACKET_TRACE("[TACACS_LIB_WritePacket] error! sockwrite() return length != %lu", len);
        return 0;
    }

    session_p->last_exch = call_time();
    return TRUE;
}


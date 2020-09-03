/* MODULE NAME: dns_proxy.c
 * PURPOSE:
 *       This module provide functions receive requests from remote clients and
 *       functions foward those requests to real dns servers and
 *       functions foward responses from real dns servers to remote clients.
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,   Reason
 *       2002-09-06    -- Wiseway , created
 *       2002-10-24    -- Wiseway   modified for convention
 *       2002-11-08    -- Isiah, porting to ACP2.0
 *
 * Copyright(C)      Accton Corporation, 2002
 */

/* INCLUDE FILE DECLARATIONS
 */
/* isiah.2004-01-06. remove all compile warring message.*/
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "sys_module.h"
#include "l_mm.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "l_stdlib.h"
#include "l_inet.h"
#include "netcfg_type.h"

#include "dns.h"
#include "dns_mgr.h"
#include "dns_vm.h"
#include "dns_cmm.h"                    /*include the common part .h file*/
#include "dns_type.h"
#include "dns_resolver.h"

//#include "netif_mgr.h"
#define s_close(sd) close(sd)

/* NAMING CONSTANT DECLARATIONS
 */

#define DNS_SOCKET_TYPE_TCP                         1
#define DNS_SOCKET_TYPE_UDP                         2

/*The following macros are used for internal socket to  identify the message type*/
#define  DNS_PROXY_INTERNAL_SOCK_ADD_IF             1
#define  DNS_PROXY_INTERNAL_SOCK_DEL_IF             2
#define  DNS_PROXY_INTERNAL_SOCK_RESP_ARRIVE        3
#define  DNS_PROXY_INTERNAL_SOCK_DNS_ENABLE         4
#define  DNS_PROXY_INTERNAL_SOCK_DNS_DISABLE        5

#define  DNS_PROXY_TAB_ENTRY_USED                   1
#define  DNS_PROXY_TAB_ENTRY_UNUSED                 0

#define  DNS_PROXY_DEFAULT_PORT                     53
#define  DNS_PROXY_DEFAULT_ADDR                     INADDR_ANY
#define  DNS_PROXY_INTERNAL_SOCK                    1

#define  DNS_PROXY_ADDIFNOTICE                      ((I8_T)253)
#define  DNS_PROXY_DELETEIFNOTICE                   ((I8_T)254)
#define  DNS_PROXY_RESPNOTICE                       ((I8_T)255)

#define  DNS_PROXY_DELETEENTRY                      0

/* MACRO FUNCTION DECLARATIONS
 */
#define DNS_PROXY_TMPBUF_MALLOC(size, user_id) L_MM_Malloc(size, user_id)
#define DNS_PROXY_TMPBUF_FREE(buf_p)           L_MM_Free(buf_p)

/* DATA TYPE DECLARATIONS
 */

extern UI32_T NETCFG_POM_IP_GetNextRifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/*typedef int       (*FUNCPTR)();    */  /* ptr to function returning int */

/*The following structrue is used for thos socket which has received DNS requset  from remote client                     */
typedef struct DNS_ProxyEntry_S
{
    int             used;              /*means whether this entry is in use, 1 means in use, 0 means not in use*/
    int             in_sock;           /*used for communication between proxy and remote client*/
    int             protocol_flag;    /* 1 means tcp; 2 means udp ; */
    DNS_TxRec_T     tx_record;         /* pending send data (TCP)    */
    DNS_RxRec_T     rx_record;         /* pending recv message     */
    DNS_TYPE_SockAddr_T client_addr;       /*the ip address of remote client*/
}
DNS_ProxyEntry_T;

/***********************************************************************************
 Variables
************************************************************************************/
/* isiah.2004-01-06. remove all compile warring message.*/
/*1static int dns_proxy_id=0;*/                  /*modified by wiseway for dns proxy disable enable 2002-10-23*/
static DNS_ProxyEntry_T*         dns_proxy_Tab = NULL;            /*dns proxy entry table*/

/*****************************************************/
/* isiah.2004-01-06. remove all compile warring message.*/
//static void   DNS_PROXY_Start(void);
static int  DNS_PROXY_Reset(int * udpsock_p, int * tcpsock_p);
static int  DNS_PROXY_InitTCP(void);
static int  DNS_PROXY_InitUDP(void);
static int  DNS_PROXY_Accept(int tcpsock);
static void DNS_PROXY_ReplyClient(int idx);
/*isiah.mask*/
/*static void   DNS_PROXY_Daemon(void);*/
static int  DNS_PROXY_InternalSock(int opcode);
static int  DNS_PROXY_VerifySockType(struct sockaddr * addr_p);
static int  DNS_PROXY_EntryAdd(int in_sock, int protocol, struct sockaddr * client_addr, DNS_Rx_PTR rx_p);
static int  DNS_PROXY_DeleteEntry(int proxy_tab_index);
static int  DNS_PROXY_InitSocket(int family, int type);
/*******************************************************/

/* isiah.2004-01-06. remove all compile warring message.*/
#if 0
/* FUNCTION NAME : DNS_proxyStart
 * PURPOSE:
 *      This functions will create a dns proxy task.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *      This routine is called by dns configuration  sub module..
 */
static void DNS_PROXY_Start(void)
{
/*isiah. move to DNS_TASK_CreateProxyDaemon*/
#if 0
    dns_proxy_id = taskSpawn("tDnsProxy",       /* task name modified by wiseway */
                            100,                            /* task priority */
                            0,                              /* task option  */
                            0x5000,                         /* stack size   */
                            (FUNCPTR)DNS_PROXY_Daemon,  /* task function */
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif

}
#endif

/*******************************************************/

/* FUNCTION NAME : DNS_ProxyStop
 * PURPOSE:
 *      This function disables the proxy function by
 *      1.terminate the proxy daemon,
 *      2.freed space dynamically for the proxy.
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *
 */
void DNS_PROXY_Stop(void)
{
    int i;
    int max_request;
/*  The following is added by wiseway for Disable and Enable proxy 2002-10-09*/
    DNS_VM_SetServServiceEnable(DNS_SERV_DISABLED);
/*isiah.*/
/*    if (dns_proxy_id!=0)
    {
        DNS_MGR_SetServStatus(DNS_SERV_STATUS_STOP);
        while (DNS_SERV_STATUS_STOPPED != DNS_MGR_GetServStatus())
            SYSFUN_Sleep(10);*/

        /*delete proxy task    */
/*isiah*/
/*        taskDelete(dns_proxy_id); */
/*        SYSFUN_DeleteTask(dns_proxy_id);
        dns_proxy_id=0;*/

        if(dns_proxy_Tab != NULL)
           free(dns_proxy_Tab);

        DNS_VM_GetDnsServConfigMaxRequests(&max_request);
        dns_proxy_Tab = (DNS_ProxyEntry_T *)malloc(sizeof(DNS_ProxyEntry_T) * max_request);
        if(dns_proxy_Tab == NULL)
        {
            return;
        }

        for(i = 0; i < max_request; i++)
        {
            dns_proxy_Tab[i].used = DNS_PROXY_TAB_ENTRY_UNUSED;
            dns_proxy_Tab[i].in_sock = 0;
            dns_proxy_Tab[i].protocol_flag = DNS_SOCKET_TYPE_UDP;

            dns_proxy_Tab[i].rx_record.pRx_Buf = NULL;
            dns_proxy_Tab[i].rx_record.rx_buf_len = 0;
            dns_proxy_Tab[i].rx_record.rx_len = 0;
            dns_proxy_Tab[i].rx_record.size = 0;
            dns_proxy_Tab[i].rx_record.stat = DNS_RX_STAT_LEN1;

            dns_proxy_Tab[i].tx_record.pTx_Buf = NULL;
            dns_proxy_Tab[i].tx_record.tx_buf_len = 0;
            dns_proxy_Tab[i].tx_record.tx_len = 0;
            dns_proxy_Tab[i].tx_record.size = 0;
        }

        DNS_VM_SetServResetStatus(VAL_dnsServConfigReset_other);  /* 1 other*/
/*      }    */
  /*    The above is added by wiseway for Disable and Enable proxy 2002-10-09*/
}



/* FUNCTION NAME : DNS_PROXY_Init
 * PURPOSE:
 *      This function initializes the dns proxy task variables,
 *      creates a interal socket used for comunnition between proxy and local resolver
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *
 */
void DNS_PROXY_Init(void)
{
    /* initilize the table dns_proxy_Tab; */
    int i;
    int max_request;

    DNS_VM_SetServResetStatus(VAL_dnsServConfigReset_initializing);  /* 3 initializing*/
    DNS_VM_SetServServiceEnable(DNS_SERV_ENABLED);
    DNS_VM_SetServStatus(DNS_SERV_INIT);
    DNS_VM_GetDnsServConfigMaxRequests(&max_request);
    dns_proxy_Tab = (DNS_ProxyEntry_T *)malloc(sizeof(DNS_ProxyEntry_T) * max_request);
    if(dns_proxy_Tab == NULL)
    {
        return;
    }

    for(i = 0; i < max_request; i++)              /*modified by wiseway to max_request*/
    {
        dns_proxy_Tab[i].used = DNS_PROXY_TAB_ENTRY_UNUSED;
        dns_proxy_Tab[i].in_sock = 0;
        dns_proxy_Tab[i].protocol_flag = DNS_SOCKET_TYPE_UDP;

        dns_proxy_Tab[i].rx_record.pRx_Buf = NULL;
        dns_proxy_Tab[i].rx_record.rx_buf_len = 0;
        dns_proxy_Tab[i].rx_record.rx_len = 0;
        dns_proxy_Tab[i].rx_record.size = 0;
        dns_proxy_Tab[i].rx_record.stat = DNS_RX_STAT_LEN1;

        dns_proxy_Tab[i].tx_record.pTx_Buf = NULL;
        dns_proxy_Tab[i].tx_record.tx_buf_len = 0;
        dns_proxy_Tab[i].tx_record.tx_len = 0;
        dns_proxy_Tab[i].tx_record.size = 0;
    }
/*isiah.move to DNS_TASK_CreateProxyDaemon()*/
#if 0
    DNS_PROXY_Start();
    DNS_MGR_SetServStatus(DNS_SERV_STARTED);
    DNS_MGR_SetServResetStatus(VAL_dnsServConfigReset_running);  /* 4 running*/
    DNS_MGR_ServUpTimeInit();
    DNS_MGR_ServResetTimeInit();
#endif
/*------------------------------------------------*/

    if (DNS_VM_GetDnsDebugStatus() == DNS_ENABLE)
     printf("\nDNS Proxy Daemon init OK!\n");
}


/* FUNCTION NAME : DNS_PROXY_Reset
 * PURPOSE:
 *      This function reinitializes the dns proxy task variables,
 *
 *
 * INPUT:
 *      int * -- Udp socket for receiving remote DNS requests.
 *      int * -- TCP socket for receiving remote DNS requests.
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      0,   sucess;
 *      -1,  failure;
 * NOTES:
 *
 */
int DNS_PROXY_Reset(int * udpsock_p, int * tcpsock_p)
{
    int i;
    int max_request;

    s_close(*udpsock_p);
    s_close(*tcpsock_p);

    free(dns_proxy_Tab);
    dns_proxy_Tab = NULL;/*---*/

    DNS_MGR_SetServStatus(DNS_SERV_INIT);


    /* get udp socket */
    if((*udpsock_p = DNS_PROXY_InitUDP()) < 0)
    {
        /* error */
        return -1;
    }

    /* get tcp socket */
    if((*tcpsock_p = DNS_PROXY_InitTCP()) < 0)
    {
        /* error */
        return -1;
    }

    if(dns_proxy_Tab != NULL)
    {
        free(dns_proxy_Tab);
        dns_proxy_Tab = NULL;
    }

    DNS_MGR_GetDnsServConfigMaxRequests(&max_request);
    dns_proxy_Tab = (DNS_ProxyEntry_T *)malloc(sizeof(DNS_ProxyEntry_T) * max_request);
    if(dns_proxy_Tab == NULL)
    {
        return -1;
    }

    for(i = 0; i < max_request; i++)
    {
        dns_proxy_Tab[i].used = DNS_PROXY_TAB_ENTRY_UNUSED;
        dns_proxy_Tab[i].in_sock = 0;
        dns_proxy_Tab[i].protocol_flag = DNS_SOCKET_TYPE_UDP;

        dns_proxy_Tab[i].rx_record.pRx_Buf = NULL;
        dns_proxy_Tab[i].rx_record.rx_buf_len = 0;
        dns_proxy_Tab[i].rx_record.rx_len = 0;
        dns_proxy_Tab[i].rx_record.size = 0;
        dns_proxy_Tab[i].rx_record.stat = DNS_RX_STAT_LEN1;

        dns_proxy_Tab[i].tx_record.pTx_Buf = NULL;
        dns_proxy_Tab[i].tx_record.tx_buf_len = 0;
        dns_proxy_Tab[i].tx_record.tx_len = 0;
        dns_proxy_Tab[i].tx_record.size = 0;
    }

    DNS_MGR_SetServStatus(DNS_SERV_STARTED);
    DNS_MGR_ServResetTimeInit();
    DNS_MGR_SetServResetStatus(VAL_dnsServConfigReset_running);  /* 4 running*/
    DNS_MGR_ServCounterInit();
    if (DNS_MGR_GetDnsDebugStatus() == DNS_ENABLE)
      printf("\nDNS Proxy reset ok!\n");

    return 0;

}

/* FUNCTION NAME : DNS_Proxy_InitTcp
 * PURPOSE:
 *      This routine creates a socket to listening TCP connection
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      0,   sucess;
 *      -1,  failure;
 * NOTES:
 *      This routine is called by DNS_PROXY_Daemon.
 */
static int DNS_PROXY_InitTCP(void)
{
    int tcpsock;

#if (SYS_CPNT_IPV6 == TRUE)
    tcpsock = DNS_PROXY_InitSocket(AF_INET6, SOCK_STREAM);
#else
    tcpsock = DNS_PROXY_InitSocket(AF_INET, SOCK_STREAM);
#endif

    if(tcpsock < 0)
    {
        return -1;
    }

    return tcpsock;
}


/* FUNCTION NAME : DNS_Proxy_InitUdpDNS_Proxy_InitTcp
 * PURPOSE:
 *      This routine creates a socket to listening UDP message.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      0,   sucess;
 *      -1,  failure;
 * NOTES:
 *      This routine is called by DNS_PROXY_Daemon.
 */
static int DNS_PROXY_InitUDP(void)
{
    int udpSock;

#if (SYS_CPNT_IPV6 == TRUE)
    udpSock = DNS_PROXY_InitSocket(AF_INET6, SOCK_DGRAM);
#else
    udpSock = DNS_PROXY_InitSocket(AF_INET, SOCK_DGRAM);
#endif

    if(udpSock < 0)
    {
        return -1;
    }

    return udpSock;

}


/* FUNCTION NAME : DNS_PROXY_Accept
 * PURPOSE:
 *      This routine creates a socket to listening tcp message.
 *      accepts incoming connection attempt on the listening socket
 *
 * INPUT:
 *      int -- The tcp socket for listening tcp connections.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      0,   sucess;
 *      -1,  failure;
 * NOTES:
 *      This routine is called by DNS_PROXY_Daemon.
 */
static int DNS_PROXY_Accept(int tcpsock)
{
    int sock_in;
    socklen_t len = (socklen_t)sizeof(struct sockaddr);
    struct sockaddr peeraddr;

    if((sock_in = accept(tcpsock, &peeraddr, &len)) != ERROR)
    {
        if(DNS_PROXY_EntryAdd(sock_in, DNS_SOCKET_TYPE_TCP, &peeraddr, NULL) < 0)
        {
           if (DNS_MGR_GetDnsDebugStatus() == DNS_ENABLE)
              printf("DNNS Proxy table is full, can't accept the tcp req.\n");

            s_close(sock_in);
            return DNS_ERROR;
        }
    }
    else
    {
        /* accpet error */
        if (DNS_MGR_GetDnsDebugStatus() == DNS_ENABLE)
        printf("\nDNS Proxy accept failed.\n");
        return DNS_ERROR;
    }

    if (DNS_MGR_GetDnsDebugStatus() == DNS_ENABLE)
      printf("\nDNS Proxy accept a tcp connect req.\n");

    return 1;
}


/* FUNCTION NAME : DNS_Proxy_replyclient
 * PURPOSE:
 *      send txbuf to client in case tcp & udp.
 *
 *
 * INPUT:
 *      int -- The index of remote requests in request table.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none;
 *
 * NOTES:
 *      This routine is called by DNS_PROXY_Daemon.
 */
static void DNS_PROXY_ReplyClient(int idx)
{
    int cnt;
    struct sockaddr* addr;
    DNS_TxRec_T* tx_rec;

    addr = (struct sockaddr*)&dns_proxy_Tab[idx].client_addr;
    tx_rec = &dns_proxy_Tab[idx].tx_record;

    if(dns_proxy_Tab[idx].protocol_flag == DNS_SOCKET_TYPE_UDP)
    {
        cnt = DNS_TxUDPMsg(dns_proxy_Tab[idx].in_sock, tx_rec, addr);
    }
    else
    {
        cnt = DNS_TxTCPMsg(dns_proxy_Tab[idx].in_sock, tx_rec);
    }

    if(cnt == DNS_OK
    && dns_proxy_Tab[idx].tx_record.tx_buf_len == (*tx_rec).tx_len)
    {
        if(DNS_PROXY_DeleteEntry(idx) != 0)
        {
           if (DNS_MGR_GetDnsDebugStatus() == DNS_ENABLE)
               printf("DNS Proxy delete idex: %d entry failed\n",idx);
        }
    }
}

/* FUNCTION NAME : DNS_PROXY_Daemon
 * PURPOSE:
 *      The main routine of dns dns proxy.
 *      It listens dns request on lan port, relays request
 *      to real dns server through dns resolver .
 *      When receive response from server, it relay the response back to client.
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none;
 *
 * NOTES:
 *      This routine is called by DNS_PROXY_Daemon.
 */
void DNS_PROXY_Daemon(void)
{
    int tcpsock;
    int udpSock;
    struct timeval wait_time;
    DNS_TYPE_SockAddr_T saddr_buf;
    struct sockaddr* from=(struct sockaddr*)&saddr_buf;
    fd_set rcv_fds;     /* set of recv sockets */
    fd_set send_fds;    /* set of send sockets */
    int n;
    int i;
    int ret;
    int maxfd;
    int max_request;

    DNS_MGR_GetDnsServConfigMaxRequests(&max_request);

    /* get udp socket */
    if((udpSock = DNS_PROXY_InitUDP()) < 0)
    {
        return;
    }
    /* get tcp socket */
    if((tcpsock = DNS_PROXY_InitTCP()) < 0)
    {
        return;
    }

    while(1)
    {
        /* init the set of recv and send */
        /* recv set include udpsock & tcpsock to recv req, resp or connect req */
        /* send set only include socket which creat connection with tcpsock */
    /*the following is added by wiseway for enable and disable proxy 2002-10-09*/
/*isiah.*/
    /* set waiting time */
    wait_time.tv_sec = 0;           /*  no. of seconds  */
    wait_time.tv_usec = 200000;      /*  20 mini-second  */

        if ( (SYS_TYPE_STACKING_MASTER_MODE != DNS_MGR_GetOperationMode()) || (DNS_ENABLE != DNS_MGR_GetDnsStatus()) )
        {
            s_close(udpSock);
            s_close(tcpsock);
            DNS_MGR_SetServStatus(DNS_SERV_STATUS_STOPPED);
            break;
        }

#if 1
        if(DNS_MGR_GetServStatus() == DNS_SERV_STATUS_STOP)
        {
           if(DNS_MGR_GetServCurrentRequestNumber() == 0)
            {
                     if  (DNS_MGR_GetServServiceEnable() ==DNS_SERV_DISABLED)
                     {
                       s_close(udpSock);
                       s_close(tcpsock);
                       DNS_MGR_SetServStatus(DNS_SERV_STATUS_STOPPED);
                       break;
                     }
                    else  /* gdns_config.DnsProxyConfig.dnsServConfigReset==2*/
                     {
                       if(DNS_PROXY_Reset(&udpSock, &tcpsock) < 0)
                    return;
                    }
              }

        }   /* fi end */
    /*the above is added by wiseway for enable and disable proxy 2002-10-09*/
#endif

        FD_ZERO(&rcv_fds);
        FD_ZERO(&send_fds);

        maxfd = (tcpsock>udpSock ? tcpsock:udpSock);

        for(i = 0; i < max_request; i++)        /* modified by wiseway from DNS_PROXY_ENTRY_MAX to max_request 2002-10-31*/
        {
            if(dns_proxy_Tab[i].used == DNS_PROXY_TAB_ENTRY_USED &&
               dns_proxy_Tab[i].protocol_flag == DNS_SOCKET_TYPE_TCP)
            {
                FD_SET(dns_proxy_Tab[i].in_sock, &rcv_fds);
                FD_SET(dns_proxy_Tab[i].in_sock, &send_fds);
                if (dns_proxy_Tab[i].in_sock > maxfd)
                    maxfd = dns_proxy_Tab[i].in_sock;
            }
        }
        FD_SET(tcpsock, &rcv_fds);
        FD_SET(udpSock, &rcv_fds);


        n = select(maxfd+1,&rcv_fds,&send_fds,NULL,&wait_time);

        if(n <= 0)
        {
            continue;
        }

        /* accept tcp connection */
        if(FD_ISSET(tcpsock, &rcv_fds)
            && DNS_MGR_GetServStatus() != DNS_SERV_STATUS_STOP)
        {
            if(DNS_PROXY_Accept(tcpsock) < 0)
            {
                /* error */
                continue;
            }
        }

        /* recv from udpSock */
        if(FD_ISSET(udpSock, &rcv_fds))
        {
            DNS_Rx_PTR rx_p = NULL;

            rx_p = (DNS_RxRec_T *)DNS_PROXY_TMPBUF_MALLOC(sizeof(DNS_RxRec_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_PROXY_DAEMON));
            if(rx_p == NULL)
            {
                continue;
            }
            memset(rx_p,0,sizeof(DNS_RxRec_T));

            /* recv data from udpsock */

            if(DNS_RxUDPMsg(udpSock, from, rx_p) == DNS_ERROR)
            {
                DNS_PROXY_TMPBUF_FREE(rx_p);
                rx_p = NULL;
            }
            else if(DNS_PROXY_VerifySockType(from) == DNS_PROXY_INTERNAL_SOCK)
            {
                /* a resp send by callback */
                if(rx_p->pRx_Buf[0] == DNS_PROXY_RESPNOTICE)
                {
                    /* a resp recved, so reply the client */
                    int idx = atoi((char *)(rx_p->pRx_Buf + 1));
                    DNS_PROXY_ReplyClient(idx);
                    L_MM_Free(rx_p->pRx_Buf);
                    DNS_PROXY_TMPBUF_FREE(rx_p);
                    rx_p = NULL;
                }/* a add IF notice send by DNS_Proxy_AddIfNotice */
            }
            else if(DNS_MGR_GetServStatus() != DNS_SERV_STATUS_STOP)
            {
                int idx;
                idx = DNS_PROXY_EntryAdd(udpSock, DNS_SOCKET_TYPE_UDP, from, rx_p);
                if(idx >= 0)
                {
                    /* call resolver */
                    ret = DNS_RESOLVER_Process(idx, (UI8_T *)dns_proxy_Tab[idx].rx_record.pRx_Buf,
                                         dns_proxy_Tab[idx].rx_record.rx_buf_len,
                                         (FUNCPTR)(DNS_PROXY_CallBack));
                    DNS_PROXY_TMPBUF_FREE(rx_p);

                    rx_p = NULL;
                    if(ret == DNS_ERROR)
                    {
                        DNS_PROXY_DeleteEntry(idx);
                    }
                }
                else    /* insert failed */
                {
                    L_MM_Free(rx_p->pRx_Buf);
                    DNS_PROXY_TMPBUF_FREE(rx_p);
                    rx_p = NULL;
                }

            }   /* esle */
            else    /* a req from udpSock, ignore it */
            {
                L_MM_Free(rx_p->pRx_Buf);
                DNS_PROXY_TMPBUF_FREE(rx_p);
                rx_p = NULL;
            }   /* esle */

        }   /* fi ISSET udp */

        /* recv from tcp sock */
        for(i = 0; i < max_request; i++)
        {
            /* recv req from client */
            if(FD_ISSET(dns_proxy_Tab[i].in_sock, &rcv_fds))
            {
                if(dns_proxy_Tab[i].protocol_flag == DNS_SOCKET_TYPE_TCP)
                {
                    int result = DNS_RxTCPMsg(tcpsock, &dns_proxy_Tab[i].rx_record);
                    if(result == DNS_ERROR)
                    {
                        DNS_PROXY_DeleteEntry(i);
                    }/* recv finished */
                    else if(dns_proxy_Tab[i].rx_record.stat == DNS_RX_STAT_FINL)
                    {
                        ret = DNS_RESOLVER_Process(i, (UI8_T *)dns_proxy_Tab[i].rx_record.pRx_Buf,
                                             dns_proxy_Tab[i].rx_record.rx_buf_len,
                                             (FUNCPTR)(DNS_PROXY_CallBack));
                        if(DNS_ERROR == ret)
                        {
                            DNS_PROXY_DeleteEntry(i);
                        }

                    }
                }

            }   /* fi rcv_fds*/
            /* data will send on this socket */
            if(FD_ISSET(dns_proxy_Tab[i].in_sock, &send_fds))
            {
                if(dns_proxy_Tab[i].protocol_flag == DNS_SOCKET_TYPE_TCP)
                {
                    DNS_PROXY_ReplyClient(i);
                }
            }

        }   /* rof */


    }   /* elihw */

}


/* FUNCTION NAME : DNS_Proxy_CreateInternalSock
 * PURPOSE:
 *      This function create a UDP socket for internal communication in proxy.
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      A socket descriptor,or
 *      -1, error (in this case , dns_internal_sock should be set to the default value 0.
 * NOTES:
 *      This function will be called by DNS_PROXY_CallBack to notify proxy that some event occurs.
 */
static int DNS_PROXY_InternalSock(int opcode)   /*  1, if internal socket exists return the internal socket;
                                                else create one then return the created one.
                                                0, close the socket;*/
{
    /* It is the interal socket used for internal comunnition in proxy */
    static int dns_internal_sock = -1;
    if(opcode == 1)
    {
        if(dns_internal_sock >= 0 )
        {
            return dns_internal_sock;
        }
        else
        {
            /*  create one; */
            if((dns_internal_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
            {
                dns_internal_sock = -1;
                return -1;
            }
            return dns_internal_sock;
        }
    }
    else if(opcode == 0)
    {
        s_close(dns_internal_sock);
        dns_internal_sock = -1;
        return -1;
    }
    return -1;
}



/* FUNCTION NAME : DNS_PROXY_CallBack
 * PURPOSE:
 *      This function will copy resp_buf to proxy's space, send a packet to internal_sock notifying proxy to get response.
 *
 *
 * INPUT:
 *      proxy_req_id -- idx of dns_proxy_Tab
 *      resp_buf -- pointer to a buffer which storing the response packet to remote hosts
 *      buf_len -- the size of res_buf
 *                if buf_len==DNS_PROXY_DELETEENTRY  then delete the index related entry in proxy table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none;
 *
 * NOTES:
 *      This routine is called by DNS_PROXY_Daemon.
 *      Resolver uses IPV4 UDP to communicate to proxy in DNS.
 *
 */
void DNS_PROXY_CallBack(int proxy_req_id,
                               const I8_T *resp_buf,
                               UI32_T buf_len)
{
    int internal_sock = 0;
    struct sockaddr_in to;
    I8_T idx[5] = {0};
    /*isiah*/
    NETCFG_TYPE_InetRifConfig_T rif_config;

    idx[0] = DNS_PROXY_RESPNOTICE;
    sprintf((char *)(idx + 1), "%d", proxy_req_id);

    if(resp_buf != NULL)
    {
        /* get internal socket
         */
        if((internal_sock = DNS_PROXY_InternalSock(1)) < 0)
        {
            return;
        }

        /* copy the resp_buf to proxy's space */
        if(dns_proxy_Tab[proxy_req_id].tx_record.pTx_Buf)
        {
            L_MM_Free(dns_proxy_Tab[proxy_req_id].tx_record.pTx_Buf);
            dns_proxy_Tab[proxy_req_id].tx_record.pTx_Buf = NULL;
        }

        dns_proxy_Tab[proxy_req_id].tx_record.size = buf_len + 1;
        dns_proxy_Tab[proxy_req_id].tx_record.pTx_Buf = (I8_T *)L_MM_Malloc(buf_len + 1, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_PROXY_CALLBACK));
        memcpy(dns_proxy_Tab[proxy_req_id].tx_record.pTx_Buf, resp_buf, buf_len);
        dns_proxy_Tab[proxy_req_id].tx_record.tx_buf_len = buf_len;
        dns_proxy_Tab[proxy_req_id].tx_record.tx_len = 0;

        /*then send the message to internal_sock*/

        memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
        NETCFG_POM_IP_GetNextRifConfig(&rif_config);

        to.sin_family = AF_INET;
        to.sin_port = L_STDLIB_Hton16(DNS_PROXY_DEFAULT_PORT);
        memcpy(&to.sin_addr.s_addr, rif_config.addr.addr, 4);

        if((sendto(internal_sock, idx, 5, 0, (struct sockaddr *)&to, sizeof(struct sockaddr_in))) == ERROR)
        {
            DNS_PROXY_InternalSock(0);
            return;
        }

        DNS_PROXY_InternalSock(0);
    }
    else if(buf_len == DNS_PROXY_DELETEENTRY)
    {
        if(DNS_PROXY_DeleteEntry(proxy_req_id) != 0)
        {
            /* failed */
        }
    }
}



/* FUNCTION NAME : DNS_PROXY_VerifySockType
 * PURPOSE:
 *      This function verifies whether the socket "sock" is a internal socket.
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      0, means "sock' is the internal socket
 *      1, means "sock" is a socket from proxy will receive packets from remote client
 *      -1, means parameter is errorneous
 * NOTES:
 *      This routine is called by DNS_PROXY_Daemon.
 *      Internal IP is only IPV4 at present.
 */
static int DNS_PROXY_VerifySockType(struct sockaddr * addr_p)
{
    L_INET_AddrIp_T peer_addr;
    NETCFG_TYPE_InetRifConfig_T rif_config;

    memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
    memset(&peer_addr, 0, sizeof(peer_addr));
    rif_config.ifindex = 0;

    NETCFG_POM_IP_GetNextRifConfig(&rif_config);

    if(L_INET_SockaddrToInaddr(addr_p, &peer_addr) == FALSE)
    {
        if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        {
            printf("%s:SocketToInaddr fail\n", __FUNCTION__);
        }
    }

    if(peer_addr.type == rif_config.addr.type)
    {
        if (L_INET_CompareInetAddr((L_INET_Addr_T *) &peer_addr, (L_INET_Addr_T *) & rif_config.addr,
            0) == 0)
        {
            return DNS_PROXY_INTERNAL_SOCK;
        }
    }

    return 0;
}

/* FUNCTION NAME : DNS_PROXY_EntryAdd
 * PURPOSE:
 *      This fcuntion find an available entry of
 *      dns_proxy_Tab for storing the client address and the new
 *      socket by running a algorithm.
 * INPUT:
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      the index number of dns_proxy_Tab entry;
 *
 * NOTES:
 *
 */
static int DNS_PROXY_EntryAdd(int in_sock, int protocol, struct sockaddr * client_addr, DNS_Rx_PTR rx_p)
{
    int i;
    int max_request=0;
    DNS_MGR_GetDnsServConfigMaxRequests(&max_request);

    for(i = 0; i < max_request; i++)
    {
        if(dns_proxy_Tab[i].used == DNS_PROXY_TAB_ENTRY_UNUSED)
        {
            dns_proxy_Tab[i].used = DNS_PROXY_TAB_ENTRY_USED;
            dns_proxy_Tab[i].in_sock = in_sock;
            dns_proxy_Tab[i].protocol_flag = protocol;
            dns_proxy_Tab[i].client_addr = *(DNS_TYPE_SockAddr_T*)client_addr;
            if(client_addr->sa_family == AF_INET)
            {
                dns_proxy_Tab[i].client_addr.saddr_len =sizeof(struct sockaddr_in);
            }
            else if(client_addr->sa_family == AF_INET6)
            {
                dns_proxy_Tab[i].client_addr.saddr_len =sizeof(struct sockaddr_in6);
            }

            if(rx_p) /* protocol == DNS_SOCKET_TYPE_UDP */
            {
                dns_proxy_Tab[i].rx_record.pRx_Buf = rx_p->pRx_Buf;
                dns_proxy_Tab[i].rx_record.rx_buf_len = rx_p->rx_buf_len;
                dns_proxy_Tab[i].rx_record.rx_len = rx_p->rx_len;
                dns_proxy_Tab[i].rx_record.size = rx_p->size;
                dns_proxy_Tab[i].rx_record.stat = rx_p->stat;
            }
            DNS_MGR_ServCurrentRequestNumberInc();
            return i;
        }
    }

    return -1;
}


/* FUNCTION NAME : DNS_PROXY_DeleteEntry
 * PURPOSE:
 *      This function deletes the entry in dns_proxy_Tab, because of timeout or having received response.
 *      The field used will be marked as 0.
 *      The related space allocated specially for this entry should be freed.
 * INPUT:
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      0, success;
 *      -1,  parameter is beyond the limits;
 *      -2,  the designated entry is not in use;
 * NOTES:
 *
 */
static int  DNS_PROXY_DeleteEntry(int proxy_tab_index)  /* the index number in dns_proxy_Tab */
{
    int max_request=0;
    DNS_MGR_GetDnsServConfigMaxRequests(&max_request);
    if(proxy_tab_index >= max_request)
    {
        return -1;
    }
    if(dns_proxy_Tab[proxy_tab_index].used == DNS_PROXY_TAB_ENTRY_UNUSED)
    {
        return -2;
    }
    if(dns_proxy_Tab[proxy_tab_index].rx_record.pRx_Buf)
    {
        L_MM_Free(dns_proxy_Tab[proxy_tab_index].rx_record.pRx_Buf);
        dns_proxy_Tab[proxy_tab_index].rx_record.pRx_Buf = NULL;
    }
    if(dns_proxy_Tab[proxy_tab_index].tx_record.pTx_Buf)
    {
        L_MM_Free(dns_proxy_Tab[proxy_tab_index].tx_record.pTx_Buf);
        dns_proxy_Tab[proxy_tab_index].tx_record.pTx_Buf = NULL;
    }

    dns_proxy_Tab[proxy_tab_index].rx_record.pRx_Buf = NULL;
    dns_proxy_Tab[proxy_tab_index].rx_record.rx_buf_len = 0;
    dns_proxy_Tab[proxy_tab_index].rx_record.rx_len = 0;
    dns_proxy_Tab[proxy_tab_index].rx_record.size = 0;
    dns_proxy_Tab[proxy_tab_index].rx_record.stat = DNS_RX_STAT_LEN1;

    dns_proxy_Tab[proxy_tab_index].tx_record.pTx_Buf = NULL;
    dns_proxy_Tab[proxy_tab_index].tx_record.tx_buf_len = 0;
    dns_proxy_Tab[proxy_tab_index].tx_record.tx_len = 0;
    dns_proxy_Tab[proxy_tab_index].tx_record.size = 0;

    dns_proxy_Tab[proxy_tab_index].used = DNS_PROXY_TAB_ENTRY_UNUSED;

    DNS_MGR_ServCurrentRequestNumberDec();
    return 0;

}

/* Create Socket and bind address.
 * Returns 0 on success, otherwise return -1.
 */
static int DNS_PROXY_InitSocket(int family, int type)
{
    int sockfd;
    int optval = 1;
    int ret;
    DNS_TYPE_SockAddr_T addr;

    memset(&addr, 0, sizeof(addr));

    if(family == AF_INET)
    {
        struct sockaddr_in *sin;

        sin = (struct sockaddr_in*)&addr;
        sin->sin_family = AF_INET;
        sin->sin_port = L_STDLIB_Hton16(DNS_PROXY_DEFAULT_PORT);
        addr.saddr_len = sizeof(struct sockaddr_in);
    }
    else
    {
        struct sockaddr_in6 *sin6;

        sin6 = (struct sockaddr_in6*)&addr;
        sin6->sin6_family = AF_INET6;
        sin6->sin6_port = L_STDLIB_Hton16(DNS_PROXY_DEFAULT_PORT);
        addr.saddr_len = sizeof(struct sockaddr_in6);
    }

    sockfd = socket(family, type, 0);

    if(sockfd < 0)
    {
        return -1;
    }

    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (I8_T *)&optval, sizeof(optval));
    if(ret != 0)
    {
        return -1;
    }

    if(bind(sockfd, (struct sockaddr *)&addr, addr.saddr_len) == ERROR)
    {
        s_close(sockfd);
        return -1;
    }

    if(type == SOCK_STREAM)
    {
        if(listen(sockfd, 5) < 0)
        {
            s_close(sockfd);
            return -1;
        }
    }

    return sockfd;
}

#endif /* #if (SYS_CPNT_DNS == TRUE) */


/*
  Main file for FPM process

  1. Create FPM Server Socket.
  2. Accept Zebra FPM Client Connection.
  3. Whenever FPM Server receive the data(FIB update), decode the FIB info/update.
  4. Send to AMTRL3 driver wrapper with RIB Table and Route info.

  Dependencies : ATAN AMTRL3 function calls/ libraries/ global variables

*/
#include <errno.h>
#include <argp.h>
#include <assert.h>
#include <libgen.h>
#include <signal.h>

#include "fpm_link.h"

/* Header files from AMTRL3 */
#include "amtrl3_type.h"
#include "amtrl3_pmgr.h"

/* Header file from  sysinclude/mibconstants/leaf_2096.h */
#include "leaf_2096.h"

typedef struct {
    int     debuglvl;
} arguments_t;

static arguments_t arguments = {
    .debuglvl = FPM_DBG_NONE,
};

#define xstr(s) str(s)
#define str(s)  #s
#define dbglvl_help   "The verbosity of debug messages (" xstr(FPM_DBG_MIN) \
                      "~" xstr(FPM_DBG_MAX) ")."

static struct argp_option options[] =
{
  { "debuglvl", 'd', "DEBUGLVL", 0, dbglvl_help, 0 },
  { 0 }
};

/* FIB Table id */
unsigned int fib_table = 0;

/* next-hop handling db*/
next_hop_db * Next_Hop_DB = NULL;

/* GLobal variable for Client FD , for temp usage*/
extern int fpm_recv_len;
extern char zebra_fib_data[ZFPM_IBUF_SIZE];
extern char recv_netlink_msg[ZFPM_IBUF_SIZE];
extern char send_netlink_msg[ZFPM_OBUF_SIZE];

pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

static void fpm_signal_handler(int sig)
{
    if (sig == SIGHUP)
    {
        arguments.debuglvl =
            (arguments.debuglvl) ? FPM_DBG_NONE : FPM_DBG_TRC;
    }
}

static void fpm_init_signals(void)
{
    struct sigaction sigact;

    sigact.sa_handler = fpm_signal_handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGHUP, &sigact, (struct sigaction *)NULL);
}

/* Parse a single option. */
static error_t
fpm_parse_opt (int key, char *arg, struct argp_state *state)
{
    /* Get the input argument from argp_parse, which we
       know is a pointer to our arguments structure. */
    arguments_t *arguments_p = state->input;

    switch (key)
    {
    case 'd':
        errno = 0;

        arguments_p->debuglvl = strtoul(arg, NULL, 0);
        if (errno != 0)
        {
            argp_error(state, "Invalid debuglvl \"%s\"", arg);
            return errno;
        }
        break;

    case ARGP_KEY_ARG:
        /* Too many arguments. */
        if (state->arg_num >= 2)
            argp_usage (state);

        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

/* Listening for FPM Connection */
void fpm_init(in_addr_t fpm_server_ip, unsigned short fpm_server_port)
{
    int sock_fd = 0;
    int opt = 1;
    struct sockaddr_in fpm_server;

    memset(&fpm_server, '0', sizeof(fpm_server));
    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // For address re-use
    if( setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    fpm_server.sin_family = AF_INET;
    fpm_server.sin_addr.s_addr = fpm_server_ip;
    fpm_server.sin_port = htons(fpm_server_port);

    bind(sock_fd, (struct sockaddr*)&fpm_server, sizeof(fpm_server));

    // Zebra Client to FPM server 1-to-1 connection
    if(listen(sock_fd, 1) == 0) {
        // This is only for debugging purpose
        FPM_DBG_MSG(DBG_INF, "Listening for Client Connection!!");
    } else {
        // Error, if FPM Server NOT able to listen to client connections
        FPM_DBG_MSG(DBG_ERR, "NOT able to listen Client Connection");
    }

    // Accept Connection
    fpm_link(sock_fd, fpm_server);

} /* End of fpm_init function */


/* Accept Zebra FPM Client Connection */
void fpm_link(int zfpm_serv_fd, struct sockaddr_in fpm_server)
{
    pthread_t fpm_read, fpm_proc;

    FPM_DBG_MSG(DBG_INF, "Spawn two threads for read and process fib info");
    if(pthread_create(&fpm_read, NULL, fpm_read_data, (void *)&zfpm_serv_fd) != 0) {
        FPM_DBG_MSG(DBG_ERR, "Failed to create read thread for FPM");
    }
    #if 0
        if(pthread_create(&fpm_write, NULL, fpm_write_data, (void *)&zfpm_cli_fd) != 0) {
            FPM_DBG_MSG(DBG_ERR, "Failed to create read thread for FPM");
        }
    #endif
    if(pthread_create(&fpm_proc, NULL, fpm_process_data, NULL) != 0) {
        FPM_DBG_MSG(DBG_ERR, "Failed to create read thread for FPM");
    }

    pthread_join(fpm_read, NULL);
    //pthread_join(fpm_write, NULL);
    pthread_join(fpm_proc, NULL);

} /* End of fpm_link function */


/* Read Zebra FPM Client data from the client */
void *fpm_read_data (void *zfpm_serv_fd)
{
    int fpm_cli_fd = *(int *)zfpm_serv_fd;
    int n = 0, sel, err;

    struct sockaddr_in zfpm_cleint;
    int sock_addr_size, zfpm_cli_fd = 0;

    /* Use conditional variable with mutex for sync. between read and write threads */
    /* For thread sync. by default read */
    sock_addr_size = sizeof(struct sockaddr_in);
    memset(recv_netlink_msg, '\0', ZFPM_IBUF_SIZE);

    FPM_DBG_MSG(DBG_INF, "Waiting for incoming Zebra FPM Client Connection...");
    while(1) {
        zfpm_cli_fd = accept(fpm_cli_fd, (struct sockaddr *)&zfpm_cleint,
                             (socklen_t*)&sock_addr_size);
        if(zfpm_cli_fd < 0) {
            FPM_DBG_MSG(DBG_ERR, "Error to accept Zebra FPM Client Connection");
        }

        /* Whenever a client connects, create the next hop DB.
           Next-hop DB, is NOT much useful here.
        */
        err = next_hop_db_init(&Next_Hop_DB);
        if (err) {
            FPM_DBG_MSG(DBG_ERR, "Failed to initialize next_hop database: %d", err);
            break ;
        } else {
            FPM_DBG_MSG(DBG_INF, "next hop DB init successful !!");
        }
        FPM_DBG_MSG(DBG_INF, "Zebra FPM Client Connected, Client id : %02x", zfpm_cli_fd);

        if (zfpm_cli_fd > 0) {
            fd_set          read_sd;
            fpm_msg_hdr_t   *fpm_hdr_p = (fpm_msg_hdr_t *) recv_netlink_msg;
            char            *rec_cur_p;
            int             is_full_msg, need_len, have_len =0, byte_read =0;

            FD_ZERO(&read_sd);
            FD_SET(zfpm_cli_fd, &read_sd);

            while(1) { // checking updates/ activity on a zebra fpm client connection
                fd_set rsd = read_sd;
                sel = select(zfpm_cli_fd + 1, &rsd, 0, 0, 0);

                if(sel > 0) {
                    // client has performed some activity (sent data or disconnected?)
                    rec_cur_p = &recv_netlink_msg[have_len];
                    is_full_msg = 0;

                    if (have_len < FPM_MSG_HDR_LEN) {
                        need_len = FPM_MSG_HDR_LEN - have_len;
                    } else {
                        need_len =fpm_msg_len(fpm_hdr_p) - have_len;
                        is_full_msg = 1;
                    }

                    n = recv(zfpm_cli_fd, rec_cur_p, need_len, 0);
                    FPM_DBG_MSG(DBG_INF, "Number of Bytes read : %d", n);
                    if(n > 0) {
                        have_len += n;

                        if (n < need_len) {
                            continue;
                        } else {
                            if (!is_full_msg) {
                                if (!fpm_msg_ok(fpm_hdr_p, sizeof(recv_netlink_msg))) {
                                    FPM_DBG_MSG(DBG_ERR, "Malformed fpm message");
                                    have_len = 0;
                                }

                                continue;
                            }
                        }

                        // got data from the client.
                        FPM_DBG_MSG(DBG_INF, "In read data flag is : %d", fpm_recv_len);

                        pthread_mutex_lock(&count_mutex);
                        while (fpm_recv_len > 0) {
                            pthread_cond_wait( &condition_var, &count_mutex );
                        }

                        memcpy(zebra_fib_data, recv_netlink_msg, have_len);

                        fpm_recv_len = have_len;
                        have_len     = 0;

                        pthread_cond_signal(&condition_var);

                        pthread_mutex_unlock(&count_mutex);

                    } else if(0 == n) {
                        // client disconnected.
                        FPM_DBG_MSG(DBG_ERR, "Zebra FPM Client Disconnected !!");
                        /* Zebra FPM Client connection closed/ lost, free the next-hop info */
                        next_hop_db_free(Next_Hop_DB);
                        break;
                    } else {
                        // error receiving data from client.
                        // You may want to break from
                        // while-loop here as well.
                        FPM_DBG_MSG(DBG_ERR, "Error while receiving data from Zebra FPM Client");
                        break;
                    }

                } else if (sel < 0) {
                    // grave error occurred.
                    FPM_DBG_MSG(DBG_ERR, "Invalid Client Descriptor/ Signal/ Resource Exceed/");
                    break;
                }
            }
        } // Handled connection
    } // Waiting for Client forver

    // Unreachable !!?? !!
    FPM_DBG_MSG(DBG_ERR, "Zebra Client Socket : %d", fpm_cli_fd);

    return 0;

} /* End of FPM read data function */



/* Process received data */
void *fpm_process_data (void *arg)
{
    memset(zebra_fib_data, '\0', ZFPM_IBUF_SIZE);
    FPM_DBG_MSG(DBG_INF, "Process thread");
    while(1) {
        // Lock mutex and then wait for signal to relase mutex
        pthread_mutex_lock( &count_mutex );
        // mutex unlocked if condition varialbe in fpm_read_data signaled.
        while (fpm_recv_len == 0)
            pthread_cond_wait( &condition_var, &count_mutex );

        FPM_DBG_MSG(DBG_INF,"In process data flag is : %d", fpm_recv_len);
        FPM_DBG_MSG(DBG_INF,"Process the client message");

        /* process fib info , get the required fileds and add to db */
        fpm_route_info_decode(zebra_fib_data);
        FPM_DBG_MSG(DBG_INF, "Once FPM message decode done, go back to read!!");

        /* Once process is done, re-set the flag and wait for next message */
        fpm_recv_len = 0;
        pthread_cond_signal(&condition_var);

        pthread_mutex_unlock( &count_mutex );
    }

} /* End of FPM process data function */


#if 0
/*
Future puspose, later we may need for new features/ enhancements
Use conditional variables for signaling threads, like between read and process threads.
*/
/* Write data to Zebra FPM Client */
void *fpm_write_data (void *zfpm_cli_fd)
{
    int fpm_cli_fd = *(int *)zfpm_cli_fd;
    int n = 0;
    char send_netlink_msg[ZFPM_OBUF_SIZE];

    memset(send_netlink_msg, '\0', ZFPM_OBUF_SIZE);
    FPM_DBG_MSG(DBG_INF, "Write thread");
    while(1) {
        // Lock mutex and then wait for signal to relase mutex
        pthread_mutex_lock( &count_mutex );
        // mutex unlocked if condition varialbe in fpm_read_data signaled.
        pthread_cond_wait( &condition_var, &count_mutex );
        FPM_DBG_MSG(DBG_INF, "In write data flag is : %d", read_write_flag);
        read_write_flag = 0;
        FPM_DBG_MSG(DBG_INF, "Writing back to client");
        strncpy(send_netlink_msg, "Success!!", 10);
        write(fpm_cli_fd,send_netlink_msg,ZFPM_OBUF_SIZE);
        //printf("\nRW Flag : %d\n",read_write_flag);
        pthread_mutex_unlock( &count_mutex );
    }

} /* End of FPM write data function */
#endif

/* Process FPM FIB info */
void fpm_route_info_decode(char *fib_buf)
{
    unsigned int msg_len=0;
    unsigned char *data;
    fpm_msg_hdr_t *hdr;
    struct nlmsghdr *nl_msg;

    FPM_DBG_MSG(DBG_INF, "Process FPM FIB info - fpm_route_info_decode()");
    /* Received route info size */
    msg_len = fpm_recv_len; // Global variable
    /*
     This is key part of handling message received form Zebra Client.
     1. Check for complete messages
     2. Check whether the message is valid or not
     3. If this message type is NETLINK message, then convet to a nl_msg
     4. Set nl msg as NETLINK
     5. Call dispatcher/ store required info in DB
     6. Release a reference(allocated at step 3) from an netlink message
    */

    /*IMPORTANT : Keep the below debugs, which will help to understand/ confirm
      what exactly Zebra Client is sending to dataplane/ forwarding.
    */
    FPM_DBG_MSG(DBG_INF, "Process FPM FIB info - fpm_route_info_decode()");
    /* Data received in FPM mesg header format, decode it!! */
    hdr = (fpm_msg_hdr_t *)fib_buf;
    FPM_DBG_MSG(DBG_INF, "FPM Protocol Version :%d", hdr->version);
    FPM_DBG_MSG(DBG_INF, "FPM Message Type : %x", hdr->msg_type);
    data = fpm_msg_data(hdr);

    /* Note: Check for complete RIB-->FIB update (multiple routes bundle),
       like BGP, OSPF, etc. protocols route updates. */
    nl_msg = (struct nlmsghdr *)data;
    FPM_DBG_MSG(DBG_INF, "NLMSG Length : %d", nl_msg->nlmsg_len);
    FPM_DBG_MSG(DBG_INF, "NLMSG Type   : %x", nl_msg->nlmsg_type);
    FPM_DBG_MSG(DBG_INF, "NLMSG Flags  : %x", nl_msg->nlmsg_flags);
    FPM_DBG_MSG(DBG_INF, "NLMSG Seq    : %d", nl_msg->nlmsg_seq);
    FPM_DBG_MSG(DBG_INF, "NLMSG pid    : %d", nl_msg->nlmsg_pid);

    /* Iterate through NL msgs */
    for( ; NLMSG_OK(nl_msg, msg_len); nl_msg=NLMSG_NEXT(nl_msg, msg_len))
    {
        FPM_DBG_MSG(DBG_INF, "Received route info size :%d\n", (int)msg_len);
        switch(nl_msg->nlmsg_type)
        {
            /* NOTE : There are several NL MSG Types, for static route used below NL MSG Types */
            case RTM_NEWROUTE:
               FPM_DBG_MSG(DBG_INF, "-- RTN_NEWROUTE add IPv4 route");
               fpm_handle_v4_route(OP_ADD, nl_msg);
               break;
            case RTM_DELROUTE:
               FPM_DBG_MSG(DBG_INF, "-- RTM_DELROUTE: del IPv4 route");
               fpm_handle_v4_route(OP_DEL, nl_msg);
               break;
            default:
               FPM_DBG_MSG(DBG_ERR, "NOT Handling other NL MSG update %d -- ignoring",
                       nl_msg->nlmsg_type);
        };
    }

} /* End of fpm fib process function */

/* Function to handle IPv4 Route ADD/ DEL updates/info */
void fpm_handle_v4_route(enum route_operation rt_op, struct nlmsghdr * nl_msg)
{
    char                buf[ZFPM_IBUF_SIZE];
    char                intfname[IFNAMSIZ];
    l3_next_hop_id_t    next_hop;
    unsigned int        netmask, err;
    fpm_v4_route_t      entry;

    struct prefix *p = (struct prefix *)malloc(sizeof(struct prefix));
    /* Didn't free this memory, this has to be freed later . TODO*/

    switch (fpm_processv4_route(nl_msg, &entry))
    {
    case 0:
        FPM_DBG_MSG(DBG_ERR, "Ignoring irrelevant(?) route update");
        return ;

    case -1:
        FPM_DBG_MSG(DBG_ERR, "Error parsing v4 route update - ignoring");
        return ;
    }

    if(entry.gateway_valid)
        snprintf(buf, BUFLEN, "via gateway "IPV4_FORMAT,
                IPV4_ADDR_PRINT(entry.gateway));
    else
        snprintf(buf, BUFLEN, "direct");

    FPM_DBG_MSG(DBG_INF, "%s IPv4 Route: " IPV4_FORMAT "/%d %s on intf %s (%d)",
            ((rt_op == OP_ADD)? "Adding" : "Deleting"),
            IPV4_ADDR_PRINT(entry.dst_ip),
            entry.dst_mask_len,
            buf,
            interface_index_to_name(entry.dst_if_index, intfname, IFNAMSIZ),
            entry.dst_if_index);

    netmask = ~((1<<(32-entry.dst_mask_len))-1);

    if(next_hop_lookup(Next_Hop_DB, entry.gateway, &next_hop)) {
        FPM_DBG_MSG(DBG_INF, "Found next_hop_id=%d for gateway " IPV4_FORMAT,
                    next_hop, IPV4_ADDR_PRINT(entry.gateway));
    } else {
        next_hop = NEXT_HOP_KERNEL;
        FPM_DBG_MSG(DBG_INF, "New next-hop !!");
    }

    /* NextHop DB is NOT much significant as of now, based on future need this has to be
       handled. Like multi-hop etc scenarios based on different NL ROUTE aatributes.
    */
    if(rt_op == OP_ADD) {
        /* add the route IF it has a gateway */
        if (entry.gateway_valid)
        {
            /** even if we know the next hop, register
             * as pending anyway because the next hop could
             * expire and we need to be able to look it up for the lifetime of
             * the route
             */
            FPM_DBG_MSG(DBG_INF, "add the route IF it has a gateway");

            if ((err = pending_next_hop_add_gateway(
                    Next_Hop_DB,
                    entry.gateway,
                    entry.dst_ip,
                    netmask)) != 0)
            {
                FPM_DBG_MSG(DBG_ERR, "pending_next_hop_add_gateway()");
            }

            FPM_DBG_MSG(DBG_INF, "add_v4_route(dst="IPV4_FORMAT",netmask="
            IPV4_FORMAT",next_hop=%d)",
            IPV4_ADDR_PRINT(entry.dst_ip),
            IPV4_ADDR_PRINT(netmask),
            next_hop);
        } else {
            FPM_DBG_MSG(DBG_INF, "Pending next-hop, NO gateway");
            if ((err = pending_next_hop_add_direct(
                    Next_Hop_DB,
                    entry.dst_ip,
                    netmask)) != 0)
            {
                FPM_DBG_MSG(DBG_ERR, "pending_next_hop_add_direct()");
            }
        }
        /* Update the prefix details */
        p->family = AF_INET; // For IPv4
        p->prefixlen = IPV4_MAX_PREFIXLEN; // IPv4
        p->u.prefix4.s_addr = entry.dst_ip;

        /* fpm_amtrl3_ipv4_add() has platform dependent code/ func calls,
           Got the route entry details, now pass to AMTRL3 wrapper func */
        if(fpm_amtrl3_ipv4_add (p, fib_table, &entry) == 0) {
            FPM_DBG_MSG(DBG_INF, "Successfully updated the AMTRL3");
        }

    } else { /* rt_op == OP_DEL  */

         /* NOTE : Check below input from netlink_route_info_fill() (in Zebra)*/
         /*
         * An RTM_DELROUTE need not be accompanied by any nexthops,
         * particularly in our communication with the FPM.
         */

        FPM_DBG_MSG(DBG_INF, "Route DELETE processing");
        if (entry.gateway_valid)
        {
            /* print */
            FPM_DBG_MSG(DBG_INF, "Valid gateway");
            /* delete the pending next_hop */
            if ((err = pending_next_hop_del_gateway(
                    Next_Hop_DB,
                    entry.gateway,
                    entry.dst_ip,
                    netmask)) != 0)
            {
                FPM_DBG_MSG(DBG_INF, "pending_next_hop_del_gateway()");
            }

        } else {    /* direct route */
            FPM_DBG_MSG(DBG_INF, "It's direct route");
            if ((err = pending_next_hop_del_direct(
                    Next_Hop_DB,
                    entry.dst_ip,
                    netmask)) != 0)
            {
                FPM_DBG_MSG(DBG_ERR, "pending_next_hop_del_direct()");
            }
       }
       /* Update the prefix details */
       p->family = AF_INET; // As of now IPv4
       p->prefixlen = IPV4_MAX_PREFIXLEN; // IPv4
       p->u.prefix4.s_addr = entry.dst_ip;
       /* Got the route entry details, now pass to AMTRL3 wrapper func */
       if(fpm_amtrl3_ipv4_delete(p, fib_table, &entry) == 0) {
           FPM_DBG_MSG(DBG_INF, "Successfully updated the AMTRL3");
       }

    } /* rt_op == OP_DEL */

} /* End of fpm_handle_v4_route() */



/* Get the route entries for IPv4 routes */
int fpm_processv4_route(struct nlmsghdr * nl_msg, fpm_v4_route_t *entry)
{
    struct rtmsg * rt_msg;
    struct rtattr * attr;
    int rtattr_len;
    int found_dst = 0;
    int found_dst_if = 0;

    rt_msg = (struct rtmsg *) NLMSG_DATA(nl_msg);
    /* extract the first attribute */
    attr = (struct rtattr *) RTM_RTA(rt_msg);
    rtattr_len = RTM_PAYLOAD(nl_msg);

    /* NOTE : DEL scenario rtm_type is RTN_UNSPEC,
              Need to be cautious in DEL scenario.

       - In RTM_DELROUTE scenario receiving rtm_type as RTN_UNSPEC,
         Second consition in if check is added for DEL case (also for debugging)

       - In RTM_ADDROUTE scenario, rtm_protocol is RTPROT_ZEBRA and
         for RTM_DELROUTE scenario, rtm_protocol is RTPROT_UNSPEC.
    */
    if ((rt_msg -> rtm_type != RTN_UNICAST) && (rt_msg -> rtm_type != RTN_UNSPEC))
        return 0;

    /* FIB table info
       IMPORTANT : Keep the below debugs, which will help to understand/ confirm
       what exactly Zebra Client is sending to dataplane/ forwarding.
    */
    fib_table = rt_msg -> rtm_table; // Default table is 254
    FPM_DBG_MSG(DBG_INF, "FIB table %x", fib_table);
    FPM_DBG_MSG(DBG_INF, "IPv4 route Update: \n"
            "       rtm_family      = 0x%x\n"
            "       rtm_dst_len     = %d\n"
            "       rtm_src_len     = %d\n"
            "       rtm_tos         = 0x%x\n"
            "       rtm_table       = 0x%x\n"
            "       rtm_protocol    = 0x%x\n"
            "       rtm_scope       = 0x%x\n"
            "       rtm_type        = 0x%x\n"
            "       rtm_flags       = 0x%x\n",
            rt_msg -> rtm_family,
            rt_msg -> rtm_dst_len,
            rt_msg -> rtm_src_len,
            rt_msg -> rtm_tos,
            rt_msg -> rtm_table,
            rt_msg -> rtm_protocol,
            rt_msg -> rtm_scope,
            rt_msg -> rtm_type,
            rt_msg -> rtm_flags);

    bzero(entry, sizeof(*entry));

    /* get the dest mask from main msg */
    entry->dst_mask_len = rt_msg->rtm_dst_len;
    entry->src_mask_len = rt_msg->rtm_src_len;

    /** note: the RTA_NEXT() macro decrements rtattr_len each time */
    for( ; RTA_OK(attr, rtattr_len); attr = RTA_NEXT(attr, rtattr_len))
    {
        switch (attr->rta_type)
        {
            /* For RTM_NEWROUTE/ RTM_DELROUTE NL messages following are the attr,
               key attr are handled, based on scenario/ req need to handle/ add in future.
            */
            case RTA_UNSPEC:
                FPM_DBG_MSG(DBG_INF, "Handle RTA_UNSPEC attribute");
                break;
            case RTA_DST:
                memcpy( &entry->dst_ip, RTA_DATA(attr), sizeof(entry->dst_ip));
                found_dst = 1;
                FPM_DBG_MSG(DBG_INF, "Parsed IPv4 route dst:" IPV4_FORMAT "/%d",
                        IPV4_ADDR_PRINT(entry->dst_ip), entry->dst_mask_len);
                break;
            case RTA_SRC:
                memcpy( &entry->src_ip, RTA_DATA(attr), sizeof(entry->src_ip));
                entry->src_route_valid = 1; /* TODO: check RTA_IIF as well */
                FPM_DBG_MSG(DBG_INF, "Parsed IPv4 route src:" IPV4_FORMAT "/%d",
                        IPV4_ADDR_PRINT(entry->src_ip), entry->src_mask_len);
                break;
            case RTA_IIF:
                memcpy( &entry->src_if_index, RTA_DATA(attr),
                        sizeof(entry->src_if_index));
                FPM_DBG_MSG(DBG_INF, "Parsed IPv4 route IIF index: %d",
                        entry->src_if_index);
                break;
            case RTA_OIF:
                memcpy( &entry->dst_if_index, RTA_DATA(attr),
                        sizeof(entry->dst_if_index));
                found_dst_if = 1;
                FPM_DBG_MSG(DBG_INF, "Parsed IPv4 route OIF index: %d",
                        entry->dst_if_index);
                break;
            case RTA_GATEWAY:
                memcpy( &entry->gateway, RTA_DATA(attr),
                        sizeof(entry->gateway));
                entry->gateway_valid = 1;
                FPM_DBG_MSG(DBG_INF, "Parsed IPv4 route gateway: " IPV4_FORMAT,
                    IPV4_ADDR_PRINT(entry->gateway));
                break;
            // Following attributes are NOT in scope as of now, useful for upcoming features
            case RTA_PREFSRC:
                FPM_DBG_MSG(DBG_INF, "RTA_PREFSRC route attr");
                break;
            case RTA_TABLE:
                FPM_DBG_MSG(DBG_INF, "RTA_TABLE route attr");
                break;
            case RTA_PRIORITY:
                FPM_DBG_MSG(DBG_INF, "RTA_PRIORITY route attr");
                break;
            case RTA_METRICS:
                FPM_DBG_MSG(DBG_INF, "RTA_METRICS route attr");
                break;
            case RTA_MULTIPATH:
                FPM_DBG_MSG(DBG_INF, "RTA_MULTIPATH route attr");
                break;
            case RTA_FLOW:
                FPM_DBG_MSG(DBG_INF, "RTA_FLOW route attr");
                break;
            case RTA_CACHEINFO:
                FPM_DBG_MSG(DBG_INF, "RTA_CACHEINFO route attr");
                break;
            case RTA_MARK:
                FPM_DBG_MSG(DBG_INF, "RTA_MARK route attr");
                break;
            case RTA_MFC_STATS:
                FPM_DBG_MSG(DBG_INF, "RTA_MFC_STATS route attr");
                break;
            case RTA_VIA:
                FPM_DBG_MSG(DBG_INF, "RTA_VIA route attr");
                break;
            case RTA_NEWDST:
                FPM_DBG_MSG(DBG_INF, "RTA_NEWDST route attr");
                break;
            case RTA_PREF:
                FPM_DBG_MSG(DBG_INF, "RTA_PREF route attr");
                break;
            case RTA_ENCAP_TYPE:
                FPM_DBG_MSG(DBG_INF, "RTA_ENCAP_TYPE route attr");
                break;
            case RTA_ENCAP:
                FPM_DBG_MSG(DBG_INF, "RTA_ENCAP route attr");
                break;
            case RTA_EXPIRES:
                FPM_DBG_MSG(DBG_INF, "RTA_EXPIRES route attr");
                break;
            case RTA_PAD:
                FPM_DBG_MSG(DBG_INF, "RTA_PAD route attr");
                break;
            //case RTA_UID:
            //     printf("FPM : RTA_UID route attr \n");
            //    break;
            //case RTA_TTL_PROPAGATE:
            //    printf("FPM : RTA_TTL_PROPAGATE route attr \n");
            //    break;
            case __RTA_MAX:
                FPM_DBG_MSG(DBG_INF, "__RTA_MAX route attr");
                break;
            default:
                FPM_DBG_MSG(DBG_ERR, "Skipping unhandled ipv4 route attr: %d",
                        attr->rta_type);
        } // Handled all attr for NL ROUTE Msg
    } // Go through attr by attr in a NL MSG

    /* For DEL scenario, following are the observations
       rtm_protocol value is RTPROT_UNSPEC and
       rtm_type value is RTN_UNSPEC

       Also we received only one attribute "entry->dst_ip", as per curreent code
       this func will return 0 (in DEL scenario) because there is NO 'dst_if_index'
       attribute in the NLMSG.
       If we return 0 from here then subsequent DEL functionality will NOT hit,
       i.e deleting the dstprefix from DB etc..

       Handling this here, no other corner cases found to check.
    */

    /* By default for some interfaces IPv6 address will be assigned, you can see below msg*/
    if (rt_msg->rtm_family == AF_INET6) {
        /* TODO: Future use */
        FPM_DBG_MSG(DBG_ERR, "Ignoring IPv6 route update: IPv6 unsupported");
        return 0;
    }

    if(found_dst && (rt_msg->rtm_protocol == RTPROT_UNSPEC)
                 && (rt_msg->rtm_protocol == RTN_UNSPEC)) {
        FPM_DBG_MSG(DBG_INF, "Route DEL scenario handling!!");
        return 1;
    }

    if(found_dst && found_dst_if)
        entry->dst_route_valid = 1;

    return entry->dst_route_valid;

} /* End of fpm_processv4_route */



/*
 * Add a IPv4 route to the forwarding layer.
 * pass action_flags, fib_id, net_route_entry to AMTRL3
*/
int fpm_amtrl3_ipv4_add (struct prefix *p, unsigned int fib_table, fpm_v4_route_t *entry)
{
    BOOL_T ret = FALSE;
    AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry;
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4;
    UI32_T fib_id;

    memset(&net_route_entry, 0, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));
    FPM_DBG_MSG(DBG_INF, "AMTRL3 : ipv4 ROUTE_ADD !");
    FPM_DBG_MSG(DBG_INF, "AMTRL3 : ROUTE_ADD ..Finally reached AMTRL3 wrapper !!");
    fib_id = fib_table; // the table id from NL msg, need to pass till here

    /* Understand the AMTRL3, check the scenario and assign appropriate values,
       Source : sysinclude/mibconstants

       #define VAL_ipCidrRouteProto_local      2L
       #define VAL_ipCidrRouteType_local       3L
    */
    net_route_entry.partial_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_local;
    net_route_entry.partial_entry.inet_cidr_route_type = VAL_ipCidrRouteType_local;

    /*
        Following macros can be used for addr type,
        L_INET_ADDR_TYPE_UNKNOWN    = 0,
        L_INET_ADDR_TYPE_IPV4       = 1,
        L_INET_ADDR_TYPE_IPV6       = 2,
        L_INET_ADDR_TYPE_IPV4Z      = 3,
        L_INET_ADDR_TYPE_IPV6Z      = 4,
        L_INET_ADDR_TYPE_DNS        = 16

        Address length,
        #define SYS_ADPT_IPV4_ADDR_LEN  4
        #define SYS_ADPT_IPV6_ADDR_LEN	16
    */

    /* fill route entry for non ECMP route */
    net_route_entry.partial_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
    net_route_entry.partial_entry.inet_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV4;
    net_route_entry.partial_entry.inet_cidr_route_dest.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    net_route_entry.partial_entry.inet_cidr_route_next_hop.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    net_route_entry.partial_entry.inet_cidr_route_pfxlen = (UI32_T)p->prefixlen;
    memcpy(net_route_entry.partial_entry.inet_cidr_route_dest.addr, &(p->u.prefix), SYS_ADPT_IPV4_ADDR_LEN);


    /* NOTE : Need to udnerstand what ATAN requires here, populate data which is
       known to AMTRL3.

       Understand ATAN/ AMTRL3 code to make it better!!
    */
    net_route_entry.partial_entry.inet_cidr_route_if_index = entry->dst_if_index;
    memcpy(net_route_entry.partial_entry.inet_cidr_route_next_hop.addr,
                    &(entry->gateway), SYS_ADPT_IPV4_ADDR_LEN);

    // Keep the debugs to check the parameters before passing AMTRL3
    FPM_DBG_MSG(DBG_INF, "FPM--> AMTRL3 ADD update");
    FPM_DBG_MSG(DBG_INF, "action_flags	: %02x", action_flags);
    FPM_DBG_MSG(DBG_INF, "fib_id		: %d", fib_id);
    FPM_DBG_MSG(DBG_INF, "AMTRL3_TYPE_InetCidrRouteEntry_T Info");
    FPM_DBG_MSG(DBG_INF, "AMTRL3_TYPE_InetCidrRoutePartialEntry_T Info");
    FPM_DBG_MSG(DBG_INF, "inet-cidr_route_proto : %d", net_route_entry.partial_entry.inet_cidr_route_proto);
    FPM_DBG_MSG(DBG_INF, "AMTRL3 : Info updated, Now SET function will be called");

    FPM_DBG_MSG(DBG_INF, "AMTRL3 Add Info: action_flags %d fib_id %d \n" \
              "net_route_entry:dest_type %d next_hop %d dest_addrlen %d nh_addrlen %d \n"
              "pref_len %d if_index %d route_proto %d route_type %d \n", \
                      action_flags, fib_id, \
                      net_route_entry.partial_entry.inet_cidr_route_dest.type, \
                      net_route_entry.partial_entry.inet_cidr_route_next_hop.type, \
                      net_route_entry.partial_entry.inet_cidr_route_dest.addrlen, \
                      net_route_entry.partial_entry.inet_cidr_route_next_hop.addrlen, \
                      net_route_entry.partial_entry.inet_cidr_route_pfxlen, \
                      net_route_entry.partial_entry.inet_cidr_route_if_index, \
                      net_route_entry.partial_entry.inet_cidr_route_proto, \
                      net_route_entry.partial_entry.inet_cidr_route_type
                      );

    ret = AMTRL3_PMGR_SetInetCidrRouteEntry(action_flags, fib_id, &net_route_entry);
    if (ret == FALSE) {
        FPM_DBG_MSG(DBG_ERR, "AMTRL3 : ipv4_route_add can't connect to Amtrl3: %d", ret);
        return -1;
    } else {
        FPM_DBG_MSG(DBG_INF, "AMTRL3 : Connect to Amtrl3 and add_ipv4_route success: %d", ret);
    }

    return 0;

} // End of fpm_amtrl3_ipv4_add()



/*
 * Delete a IPv4 route to the forwarding layer.
 */
int fpm_amtrl3_ipv4_delete (struct prefix *p, unsigned int fib_table, fpm_v4_route_t *entry)
{
    BOOL_T ret = FALSE;
    UI32_T fib_id;
    AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry;
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4;

    memset(&net_route_entry, 0, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));
    FPM_DBG_MSG(DBG_INF, "AMTRL3 : ipv4 ROUTE_DEL !");
    FPM_DBG_MSG(DBG_INF, "AMTRL3 : ROUTE_DEL ..Finally reached AMTRL3 wrapper !!");

    fib_id = fib_table; // table id from NL msg, need to pass till here

    /* Understand the AMTRL3, check the scenario and assign appropriate values,
       Source : sysinclude/mibconstants

       #define VAL_ipCidrRouteProto_local      2L
       #define VAL_ipCidrRouteType_local       3L
    */

    /* NOTE : Update the net_route_entry with proper values, which are NOT same as in add
       scenario.

       For connected route,
       net_route_entry->partial_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_local

    */

    net_route_entry.partial_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_local;
    net_route_entry.partial_entry.inet_cidr_route_type = VAL_ipCidrRouteType_local;

    /*
    Following macros can be used for addr type,
        L_INET_ADDR_TYPE_UNKNOWN    = 0,
        L_INET_ADDR_TYPE_IPV4       = 1,
        L_INET_ADDR_TYPE_IPV6       = 2,
        L_INET_ADDR_TYPE_IPV4Z      = 3,
        L_INET_ADDR_TYPE_IPV6Z      = 4,
        L_INET_ADDR_TYPE_DNS        = 16

    Address length,
        #define SYS_ADPT_IPV4_ADDR_LEN  4
        #define SYS_ADPT_IPV6_ADDR_LEN	16
    */

    /* fill route entry for non ECMP route to be deleted*/
    net_route_entry.partial_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
    net_route_entry.partial_entry.inet_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV4;
    net_route_entry.partial_entry.inet_cidr_route_dest.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    net_route_entry.partial_entry.inet_cidr_route_next_hop.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    net_route_entry.partial_entry.inet_cidr_route_pfxlen = (UI32_T)p->prefixlen;
    memcpy(net_route_entry.partial_entry.inet_cidr_route_dest.addr, &(p->u.prefix), SYS_ADPT_IPV4_ADDR_LEN);

    /* NOTE : Need to udnerstand what ATAN requires here, populate data which is
       known to AMTRL3.

       Understand ATAN/ AMTRL3 code to make it better!!
    */
    net_route_entry.partial_entry.inet_cidr_route_if_index = entry->dst_if_index;
    memcpy(net_route_entry.partial_entry.inet_cidr_route_next_hop.addr,
                    &(entry->gateway), SYS_ADPT_IPV4_ADDR_LEN);

    FPM_DBG_MSG(DBG_INF, "FPM--> AMTRL3  DELETE update");
    FPM_DBG_MSG(DBG_INF, "action_flags	: %02x", action_flags);
    FPM_DBG_MSG(DBG_INF, "fib_id		: %d", fib_id);
    FPM_DBG_MSG(DBG_INF, "AMTRL3_TYPE_InetCidrRouteEntry_T Info");
    FPM_DBG_MSG(DBG_INF, "AMTRL3_TYPE_InetCidrRoutePartialEntry_T Info");
    FPM_DBG_MSG(DBG_INF, "inet-cidr_route_proto : %d", net_route_entry.partial_entry.inet_cidr_route_proto);

    FPM_DBG_MSG(DBG_INF, "AMTRL3 Del Info: action_flags %d fib_id %d \n" \
              "net_route_entry:dest_type %d next_hop %d dest_addrlen %d nh_addrlen %d \n"
              "pref_len %d if_index %d route_proto %d route_type %d \n", \
                      action_flags, fib_id, \
                      net_route_entry.partial_entry.inet_cidr_route_dest.type, \
                      net_route_entry.partial_entry.inet_cidr_route_next_hop.type, \
                      net_route_entry.partial_entry.inet_cidr_route_dest.addrlen, \
                      net_route_entry.partial_entry.inet_cidr_route_next_hop.addrlen, \
                      net_route_entry.partial_entry.inet_cidr_route_pfxlen, \
                      net_route_entry.partial_entry.inet_cidr_route_if_index, \
                      net_route_entry.partial_entry.inet_cidr_route_proto, \
                      net_route_entry.partial_entry.inet_cidr_route_type
                      );

    ret =  AMTRL3_PMGR_DeleteInetCidrRouteEntry(action_flags, fib_id, &net_route_entry);

    if (ret == FALSE) {
        FPM_DBG_MSG(DBG_ERR, "AMTRL3 : ipv4 route Delete: can't connect to Amtrl3 %d", ret);
        return -1;
    } else {
        FPM_DBG_MSG(DBG_INF, "Connect to Amtrl3 and delete ipv4 route success %d", ret);
    }

    return 0;
}

/* return 1 if dbg flag is on
 */
int fpm_is_dbg_on(int flag)
{
    return (arguments.debuglvl >= flag);
}

int main(int argc, char *argv[])
{
    /* Our argp parser. */
    static struct argp argp = { options, fpm_parse_opt, NULL, NULL };

    in_addr_t fpm_server_ip; // IP address for the server socket
    unsigned short fpm_server_port = FPM_DEFAULT_PORT; // Port for the server socket

    if (AMTRL3_PMGR_InitiateProcessResource() == FALSE) {
        return -1;
    }

    fpm_init_signals();

    /* Parse our arguments; every option seen by `parse_opt' will be
     * reflected in arguments.
     */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    FPM_DBG_MSG(DBG_INF, "debuglvl - %d", arguments.debuglvl);

    fpm_server_ip = FPM_DEFAULT_IP;
    //fpm_server_ip = inet_addr("127.0.0.1");

    /* Pass FPM server IP and Port details for Server Socket creation */
    fpm_init(fpm_server_ip, fpm_server_port);

} /* End of main() */

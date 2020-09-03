/* Module Name: dns_resolver.c
 * Purpose: This package provide api for access DNS resolver.
 * Notes:
 * History:
 *    09/06/02        -- simon zhou, Create
 *    11/08/02        -- Isiah, porting to ACP@2.0
 * Copyright(C)      Accton Corporation, 1999, 2000
 */

/* INCLUDE FILE DECLARATIONS
 */
/* isiah.2004-01-06. remove all compile warring message.*/
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include <string.h>
#include <stdlib.h>

#include "l_stdlib.h"
#include <ctype.h>

#include "sys_bld.h"
#include "sys_module.h"
#include "l_mm.h"
#include "dns_type.h"
#include "dns.h"
#include "dns_list.h"
#include "dns_mgr.h"
#include "dns_vm.h"
#include "dns_cmm.h"
#include "dns_cache.h"
#include "dns_hostlib.h"
#include "dns_resolver.h"
#include "dns_om.h"
#include "sysfun.h"
#include "l_inet.h"

extern I32_T dns_inet_addr(register const char *cp);

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define read(s,b,n)         recv((s),(b),(n),0)
#define write(s,b,n)        send((s),(b),(n),0)

#define DNS_RESOLVER_TMP_BUFFER_MALLOC(size)           L_MM_Malloc(size, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_TMP_BUFFER))
#define DNS_RESOLVER_TMP_BUFFER_FREE(buf_p)            L_MM_Free(buf_p)
#define DNS_RESOLVER_DNS_SBELT_ENTRY_MALLOC()          (DNS_Sbelt_T*)L_MM_Malloc(sizeof(DNS_Sbelt_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_DNS_SBELT_ENTRY))
#define DNS_RESOLVER_DNS_SBELT_ENTRY_FREE(buf_p)       L_MM_Free(buf_p)
#define DNS_RESOLVER_SENDBUF_ENTRY_MALLOC(size)        L_MM_Malloc(size, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_SENDBUF_ENTRY))
#define DNS_RESOLVER_SENDBUF_ENTRY_FREE(buf_p)         L_MM_Free(buf_p)
#define DNS_RESOLVER_DNS_SLIST_ENTRY_MALLOC()          (DNS_Slist_T*)L_MM_Malloc(sizeof(DNS_Slist_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_DNS_SLIST_ENTRY))
#define DNS_RESOLVER_DNS_SLIST_ENTRY_FREE(buf_p)       L_MM_Free(buf_p)
#define DNS_RESOLVER_DNS_CACHERR_ENTRY_MALLOC()        (DNS_CacheRR_T*)L_MM_Malloc(sizeof(DNS_CacheRR_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_DNS_CACHERR_ENTRY))
#define DNS_RESOLVER_DNS_CACHERR_ENTRY_FREE(buf_p)     L_MM_Free(buf_p)
#define DNS_RESOLVER_RECVBUF_ENTRY_MALLOC(size)        (DNS_Hdr_T *)L_MM_Malloc(size, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_RECVBUF_ENTRY))
#define DNS_RESOLVER_RECVBUF_ENTRY_FREE(buf_p)         L_MM_Free(buf_p)
#define DNS_RESOLVER_DNS_SEND_ENTRY_MALLOC()           (DNS_Send_T*)L_MM_Malloc(sizeof(DNS_Send_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_DNS_SEND))
#define DNS_RESOLVER_DNS_SEND_ENTRY_FREE(buf_p)        L_MM_Free(buf_p)
#define DNS_RESOLVER_HOST_ENT_ENTRY_MALLOC()           (struct hostent *)L_MM_Malloc(sizeof(struct hostent), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_HOSTENT_ENTRY))
#define DNS_RESOLVER_HOST_ENT_ENTRY_FREE(buf_p)        L_MM_Free(buf_p)
#define DNS_RESOLVER_H_ADDR_LIST_ENTRY_MALLOC()        (char **)L_MM_Malloc(DNS_HOSTENT_IP_SIZE, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_H_ADDR_LIST_ENTRY))
#define DNS_RESOLVER_H_ADDR_LIST_ENTRY_FREE(buf_p)     L_MM_Free(buf_p)

#if(SYS_CPNT_IPV6 == TRUE)
#define DNS_RESOLVER_H_ADDR_LIST_ENTRY_ELM_MALLOC()    (char *)L_MM_Malloc(SYS_ADPT_IPV6_ADDR_LEN, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_H_ADDR_LIST_ENTRY_ELM))
#else
#define DNS_RESOLVER_H_ADDR_LIST_ENTRY_ELM_MALLOC()    (char *)L_MM_Malloc(SYS_ADPT_IPV4_ADDR_LEN, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_H_ADDR_LIST_ENTRY_ELM))
#endif

#define DNS_RESOLVER_H_ADDR_LIST_ENTRY_ELM_FREE(buf_p) L_MM_Free(buf_p)
#define DNS_RESOLVER_H_NAME_ENTRY_MALLOC()             (char *)L_MM_Malloc(strlen((char *)name) + 1, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_H_NAME_ENTRY))
#define DNS_RESOLVER_H_NAME_ENTRY_FREE(buf_p)          L_MM_Free(buf_p)
#define DNS_RESOLVER_H_ALIASES_ENTRY_MALLOC()          (char **)L_MM_Malloc(DNS_HOSTENT_ALIAS_SIZE, L_MM_USER_ID2(SYS_MODULE_DNS, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_H_ALIASES_ENTRY)))
#define DNS_RESOLVER_H_ALIASES_ENTRY_FREE(buf_p)       L_MM_Free(buf_p)
#define DNS_RESOLVER_H_ALIASES_ENTRY_ELM_MALLOC()      (char *)L_MM_Malloc(strlen((char *)name) + 1, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_H_ALIASES_ENTRY_ELM))
#define DNS_RESOLVER_H_ALIASES_ENTRY_ELM_FREE(buf_p)   L_MM_Free(buf_p)

#define s_close(sd) close(sd)
/* DATA TYPE DECLARATIONS
 */

typedef struct DNS_Send_S
{
    struct DNS_Send_S*  next_p; /* next element to queue (column)   */
    I8_T    qrhn[256];          /* query name,DNS format !!!!,DNS_RESOLVER_Str2Rhn */
    int inter_id;               /* identify which task to wakeup.proxy,or taskid*/
    int rid;                    /* for which query,proxy received query id   */
    int myrid;                  /* my query id                      */
    int qtype;                  /* query type                       */
    FUNCPTR         callback;   /* callback function hooked         */
    int                 udp;    /* protocol                         */
    struct sockaddr* addr;      /* same with sin in slist                */
    DNS_TYPE_SockAddr_T recv_addr;
    int sinl;                   /* socket struct length             */
    unsigned int timeout;       /* how long has query send,time_t?         */
    struct hostent* hp;         /* for return local request         */
    DNS_CacheRR_PTR_T rrlist_p; /* Cache list from recv buf         */
    int flags;                  /* misc issue can put here          */
    int opcode;                 /* for return */
    int event;                  /* send request or wait for reply           */
    int state;                  /* for protocol specified state,DNS_QS_XXX */
    int qm;                     /* UDP query or TCP query,qm=query machine*/
    int trusted;                /* does rr from this conn trust?            */
    int sock;                   /* socket to externel               */
    UI16_T sendl;       /* send buf len                     */
    UI16_T recvl;       /* recv buf len                     */
    DNS_Hdr_T* sendbuf;         /* hold or construct send buf for re-trans    */
    DNS_Hdr_T*  recvbuf;        /* received buf                     */
    UI8_T  nsdomain[256]; /* ns domain,identify which server to query  */
    DNS_Slist_T* slist;         /* slist,you know it. */
    UI32_T  rcode;
    /* timestamp to begin request */
}DNS_Send_T;

/*************************************************************************/
typedef enum
{
    DNS_QS_TCPINITIAL   = 1,    /* Start a TCP query. */
    DNS_QS_TCPALLOC,            /* Resources allocated */
    DNS_QS_TCPCONNECT,          /* Connected.    */
    DNS_QS_TCPLWRITTEN,         /* Query length has been transmitted. */
    DNS_QS_TCPQWRITTEN,         /* Query transmitted. */
    DNS_QS_TCPLREAD,            /* Answer length read */

    DNS_QS_UDPINITIAL = 20,     /* Start a UDP query */
    DNS_QS_UDPTRANSMIT,         /* Start a UDP query */
    DNS_QS_UDPRECEIVE,          /* Start a UDP query */
    DNS_QS_UDPRECEIVED,

    DNS_QS_DONE         = 32,   /* done, resources freed, result is in stat_t */
    DNS_QS_GOTTEN       = 64,

    DNS_QS_FREE         = 128
}DNS_QueryStatus_E;

/**************************************************************************/
#define DNS_QEV_WRITE           1
#define DNS_QEV_READ            2
#define DNS_QEV_READWRITE       3
/**************************************************************************/

typedef enum
{
    DNS_ANSWER_PART = 1,
    DNS_NSERVER_PART,
    DNS_ADDI_PART
}DNS_PktPart_E;

#define DNS_HOST_TTL        100
#define DNS_CNAME_NUM       4

#define DNS_REQ_RD          0x80000000
#define DNS_REQ_RA          0x40000000
#define DNS_SRV_RD          0x20000000
#define DNS_SRV_RA          0x10000000

static int  DNS_RESOLVER_Name2Domain(I8_T *name_p, char *domain_list_p);
static UI32_T DNS_RESOLVER_Ptr2Ip(const UI8_T* ptr);
static void DNS_RESOLVER_Rhn2Str(UI8_T* rhn, UI8_T* str);
static int  DNS_RESOLVER_Str2Rhn(const UI8_T* str,UI8_T* rhn);
static int  DNS_RESOLVER_RhnCmp(UI8_T* rhn1,UI8_T* rhn2);
static int  DNS_RESOLVER_IsDchar (UI8_T c);
static void DNS_RESOLVER_Srand(void);
static int  DNS_RESOLVER_RhnCopy(UI8_T* dst, UI8_T* src);
static int  DNS_RESOLVER_Decompress(UI8_T* data,UI8_T* name,
                UI8_T** rest,int *size,int* len,int msglen,int* underscore);
static int  DNS_RESOLVER_Compress(UI8_T* in,UI8_T* out,int offset,darray *d);
static int  DNS_RESOLVER_DomainMatch(int* offset,UI8_T* newname,
                                UI8_T* oldname,UI8_T* rest);
static int  DNS_RESOLVER_SbeltAdd(I8_T* domain, L_INET_AddrIp_T *addr_p, int port);
static void DNS_RESOLVER_SendFree(DNS_Send_T* send_p);
static int  DNS_RESOLVER_SendHook(DNS_Send_T* send_p,const I8_T* qname,
                        int qtype,int op,int inter_id,unsigned int rid,
                        DNS_CacheRR_T* cache_p);
static void DNS_RESOLVER_Retransmit(DNS_Send_T *send_p);
static void DNS_RESOLVER_AdjustRR(DNS_CacheRR_T* cache_p,I8_T* qname);
static int  DNS_RESOLVER_ResponseProcess(DNS_Send_T* send_p,long rlen);
static int  DNS_RESOLVER_RxTx(DNS_Send_T* send_p);
static int  DNS_RESOLVER_AddCache(DNS_Send_T* send_p,
                            UI8_T** rr,int rrnum,int* remain,int type);
/*isiah.mask*/
/*static void   DNS_RESOLVER_Task(void);*/
static int  DNS_RESOLVER_Entry(int inter_id,UI8_T* name,L_INET_AddrIp_T *pip_p,
                        UI16_T qtype,UI8_T* data,FUNCPTR routine);
            /*  construct pkt for proxy to reply */
static int  DNS_RESOLVER_CachedReply(DNS_CacheRR_T* cache_p,struct DNS_Hdr_S** rhdr);
                    /*  directly get answer from cache to local call */

/*static struct hostent* DNS_RESOLVER_CachedAnswer(DNS_CacheRR_T* cache_p);*/
static int  DNS_RESOLVER_LocalReply(DNS_Hdr_T** rhdr,int qtype,int op,
                                I8_T* name,L_INET_AddrIp_T pip_ar[]);
static int  DNS_RESOLVER_ErrorReply(UI16_T id, UI16_T opcode,
                               UI16_T rcode,int inter_id,FUNCPTR routine);
static DNS_Hdr_T* DNS_RESOLVER_ErrorMake(unsigned int rcode,unsigned int id,unsigned int op);
static int  DNS_RESOLVER_HostentAdd(struct hostent** php,int type,
                                const I8_T* name,L_INET_AddrIp_T *ip_p);

static int DNS_RESOLVER_GetIpFromIpv6Arpa(UI8_T *name_string, UI8_T *ip);
static UI32_T DNS_RESOLVER_ExtractIpByFamily(UI32_T ip_ar_len, L_INET_AddrIp_T ip_ar[], UI32_T af_family,
                                             UI32_T hostip_ar_len, L_INET_AddrIp_T hostip_ar[]);
static UI32_T DNS_RESOLVER_ExtractInaddrfromCache(DNS_CacheRR_T* cache_p, UI32_T ar_len, L_INET_AddrIp_T ip_ar[]);
static UI32_T DNS_RESOLVER_ExtractIpFromHostent(struct hostent* ghp, UI32_T host_idx, UI32_T ar_len, L_INET_AddrIp_T ip_ar[]);

/* may be called by DNS_MGR_HostNameToIp  */
/*static void   DNS_RESOLVER_HostentFree(struct hostent* hp);*/
static int gethostbyname_ip(const I8_T* name, UI32_T af_family, L_INET_AddrIp_T ip_ar[]);
static int search_cache(const I8_T* name, L_INET_AddrIp_T ip_ar[]);
static int gdns_max_pc = DNS_MAX_LOCAL_REQUEST;     /* max parallel query send,add 1 for local */
static unsigned int gdns_negative_ttl = 100;
static unsigned int gdns_max_ttl = 600000;
static unsigned int gdns_min_ttl = 120;
static int gdns_retransmit = 100000;  /* usec */

/*isiah*/
/*static struct hostent* gdns_hp[5];*/
static struct hostent* gdns_hp[DNS_HOSTENT_NUM];
/*static SEM_ID gdns_resolver_sem;*/
static UI32_T gdns_resolver_sem;
static int gdns_sbelt_recursive = 1;
static int gdns_sbelt_send_recursive = 1;       /* global but not static,so can */
static int gdns_sbelt_recv_recursive = 1;       /*configure from hypertrm */
static int gdns_slist_send_recursive = 0;
static int gdns_slist_recv_recursive = 0;

static DNS_Send_T* gdns_send_head = NULL;
static DNS_Sbelt_T* gdns_sbelt_head = NULL;

static int gdns_hp_num = 0;             /* control static hostent number */
static int gdns_hook_count = 0;         /* control proxy request number */
static int gdns_local_request = 0;      /* control local request number  */
static UI32_T gdns_remote_timeout = DNS_DEF_TIME_OUT;

static I8_T gdns_ip_buf[5];             /* DEBUG:show ip content       */
static int gdns_cache_hit = 0;
static int gdns_cache_dad = 0;
/*static UI32_T gdns_resolver_id;*/


/*
 * FUNCTION NAME : gethostbyname
 *
 * PURPOSE:
 *      common dns interface
 *
 * INPUT:
 *      const I8_T* -- query name
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
/*static struct hostent* gethostbyname(const I8_T* name);*/

/*
 * FUNCTION NAME : gethostbyname_r
 *
 * PURPOSE:
 *      gethostbyname,but won't free hostent space
 *
 * INPUT:
 *      const I8_T* -- query name
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      hostent* hp
 *
 * NOTES:
 *      none
 *
 */
static struct hostent* gethostbyname_r(const I8_T* name, UI32_T family, int *rc);

/*
 * FUNCTION NAME : DNS_RESOLVER_Name2Domain
 *
 * PURPOSE:
 *      Add a suffix to name,if name is bare( only 1 label in it).
 *
 * INPUT:
 *      name_p          -- Name
        domain_list_p   -- Domain list
 *
 * OUTPUT:
 *      I8_T*   --  ( google.com )
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      If domain list is empty string, the default domain name will be used.
 *
 */
static int  DNS_RESOLVER_Name2Domain(I8_T *name_p, char *domain_list_p)
{
    I8_T domain[DNS_MAX_NAME_LENGTH+1];
    UI32_T name_length,domain_length;

    if ( NULL == name_p || '\0' == *name_p)
        return DNS_ERROR;

    name_length=strlen((char *)name_p);

    DNS_MGR_ServCounterInc(LEAF_dnsServCounterRelNames);
    /* opt counter ?? */

    if(NULL == domain_list_p)
    {
        if( DNS_ERROR == DNS_MGR_GetDnsIpDomain((char *)domain) )
        {
            return DNS_ERROR;
        }
    }
    else
    {
        strcpy((char *)domain, domain_list_p);
    }

    domain_length=strlen((char *)domain);

    /* length of {name_p} {.} {domain} */
    if( (name_length + 1) + domain_length > 255)
        return DNS_ERROR;

    if( domain_length == 0)
    {
        return DNS_ERROR;
    }
    else
    {
        name_p[name_length] = '.';
        memcpy(name_p + name_length + 1 ,domain, domain_length + 1);
        name_p[name_length + 1 + domain_length]='\0';
    }

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_Ptr2Ip
 *
 * PURPOSE:
 *      extract & convert PTR RR to a IP address.
 *
 * INPUT:
 *      const UI8_T* -- pointer to PTR RR rdata.
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      UI32_T rip
 *      0xffffffff (255.255.255.255)
 *
 * NOTES:
 *      returned rip in network sequence
 *
 */
UI32_T DNS_RESOLVER_Ptr2Ip(const UI8_T* ptr)
{
    const UI8_T *str = ptr;
    int i,x,j = 0;
    I8_T buf[4];
    I8_T ret[4];
    UI32_T rip = 0xffffffff;

    for(i=0;i<4;i++)
    {
        while(*str != '.')
        {
/*isiah.*/
/*          if(j > 3 || !isdigit(*str))*/
            if ( (j>3) || ('0'>*str) || ('9'<*str) )
            {
                return 0xffffffff;
            }

            buf[j++] = *str++;
        }
        buf[j] = '\0';

        x = atoi((char *)buf);
        if(x>255)
        {
            return 0xffffffff;
        }
        else
        {
            ret[i] = x;
        }
        str++;
        j = 0;
    }

    memcpy(&rip,ret,4);     /* align */
    return rip;
    /*
    return *(UI32_T*)ret;
    */
}

/*
 * FUNCTION NAME : DNS_RESOLVER_Rhn2Str
 *
 * PURPOSE:
 *      convert DNS format domain name to common
 *
 * INPUT:
 *      UI8_T* -- in dns format ( eg. 3www6google3com )
 *
 * OUTPUT:
 *      UI8_T* -- common domain ( eg. www.google.com )
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      ( 3,6 in example is 0x3 not 0x33h ('3')
 *
 */
void DNS_RESOLVER_Rhn2Str(UI8_T* rhn,
                    UI8_T* str)
{
    UI8_T ch;
    int i;
    int count=1;

    str[0] = '\0';
    ch = rhn[0];
    if (!ch)
    {
        strcpy((char *)str,".");
        return;
    }
    while (ch)
    {
        for (i=0;i<ch;i++)
        {
            str[count-1] = tolower(rhn[count]);
            count++;
        }
        str[count-1]='.';
        str[count]='\0';
        ch = rhn[count];
        count++;
        if (count>255)
            break;
    }

    str[strlen((char *)str)-1] = '\0';
}

/*
 * FUNCTION NAME : DNS_RESOLVER_Str2Rhn
 *
 * PURPOSE:
 *      convert common domain name to dns format domain name
 *
 * INPUT:
 *      const UI8_T* -- common domain ( eg. www.google.com )
 *
 * OUTPUT:
 *      UI8_T* -- dns format domain name ( eg. 3www6google3com )
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      ( 3,6 in example is 0x3 not 0x33h ('3')
 *
 */
int DNS_RESOLVER_Str2Rhn(const UI8_T* str,
                  UI8_T* rhn)
{
    UI8_T buf[64];
    int cnt = 0;                            /* have processed count */
    int tcnt = 0;                           /* rhn number */
    int i   = 0;                            /* label number */
    int lb;                             /* current byte count in label */

    do
    {
        lb=0;
        if (lb + cnt >= 255)
            return DNS_ERROR;
        while(DNS_RESOLVER_IsDchar(str[lb+cnt]))
        {
            if (lb>62)
                return DNS_ERROR;       /* label too long */

            buf[lb]=str[lb+cnt];
            lb++;
            if (lb + cnt >= 255)
                return DNS_ERROR;
        }
        if (str[lb+cnt] == '\0')
        {
            if (ZERO == lb)
            {
                if (ZERO == i)          /* first label */
                    return DNS_ERROR;

                rhn[tcnt]='\0';
                return DNS_OK;
            }
            if(lb+tcnt+1>255)
                return DNS_ERROR;

            rhn[tcnt++]=(UI8_T)lb;
            memcpy(rhn+tcnt,buf,lb);
            rhn[tcnt+lb] = '\0';
            return DNS_OK;
        }
        else if (str[lb+cnt]=='.')
        {                   /* if two '.' continued !!,just as it OK. for easy op */
            i++;
            if (lb+tcnt+1>255) /* 255 because the termination 0 has to follow */
                return DNS_ERROR;

            rhn[tcnt]=(UI8_T)lb;
            tcnt++;
            memcpy(rhn+tcnt,buf,lb);    /* copy from next */
            tcnt += lb;
            cnt += lb+1;
        } else
            return DNS_ERROR;

    } while (1);

}

/*
 * FUNCTION NAME : DNS_RESOLVER_IsDchar
 *
 * PURPOSE:
 *      verify input character whether is legal chacter DNS permit.
 *
 * INPUT:
 *      UI8_T -- a character.
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_IsDchar (UI8_T c)
{
/*isiah*/
/*  if ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c >= '0' && c <= '9') || c=='-')*/
    if ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c >= '0' && c <= '9') || c=='-' || c=='_')
        return TRUE;

    return FALSE;   /* be care,don't rp as DNS_OK!! */
}

/*
 * FUNCTION NAME : DNS_RESOLVER_RhnCmp
 *
 * PURPOSE:
 *      compare 2 dns format domain
 *
 * INPUT:
 *      UI8_T* --
 *      UI8_T* --
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      DNS_OK              rhn1 is equal with rhn2
 *      DNS_ERROR
 *
 * NOTES:
 *      different in strcmp or memcmp,since dns is case insensitive
 *
 */
int DNS_RESOLVER_RhnCmp(UI8_T* rhn1,UI8_T* rhn2)
{
    I8_T ch;
    int i = 0;

    while(1)
    {
        ch = rhn1[i];
        if ( ch != rhn2[i])
        {
            return DNS_ERROR;
        }

        if (ZERO == ch)
            break;

        i++;
        for (;ch>0;ch--)
        {
            if (tolower(rhn1[i]) != tolower(rhn2[i])) /* all ++ op can be place in buf[++i]*/
            {
                return DNS_ERROR;
            }
            i++;
        }
    }

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_Srand
 *
 * PURPOSE:
 *      yield random seed.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_RESOLVER_Srand()
{
    srand(SYSFUN_GetSysTick());
}

/*
 * FUNCTION NAME : DNS_RESOLVER_RhnCopy
 *
 * PURPOSE:
 *      copy lower than 256 chacter
 *
 * INPUT:
 *      UI8_T* --
 *
 * OUTPUT:
 *      UI8_T* --
 *
 * RETURN:
 *      number of character copied
 *
 * NOTES:
 *      none
 *
 */
int  DNS_RESOLVER_RhnCopy(UI8_T* dst,
                  UI8_T* src)
{
    int len;

    len = strlen((char *)src) + 1;
    memcpy((UI8_T*)dst,(UI8_T*)src,len>256?256:len);

    return len;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_Decompress
 *
 * PURPOSE:
 *      extract domain from RR
 *
 * INPUT:
 *      UI8_T* data -- pointer to a RR
 *      int msglen -- full dns pkt len
 *
 * OUTPUT:
 *      UI8_T* name -- extracted domain name
 *      UI8_T** rest-- pointer to rest part of RR
 *      int* size -- remain msg size
 *      int* len -- decompressed name length,with
 *      int* underscore -- any underscore in name?
 *
 * RETURN:
 *      DNS_RC_OK
 *      DNS_RC_TRUNC
 *      DNS_RC_FORMAT
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_Decompress(UI8_T* data,
                      UI8_T* name,
                      UI8_T** rest,
                      int* size,
                      int* len,
                      int msglen,
                      int* underscore)
{
    long oldsize = *size;
    UI8_T ch;
    long offset;
    UI8_T* prr;
    int i,jumped = 0;
    int hops = 0;
    int npos = 0;

    if (ZERO == *size)
        return DNS_RC_TRUNC;

    if (underscore!=NULL)
        *underscore=0;

    prr = *rest;
    while (1)
    {
        if (prr - data >= msglen)
            return DNS_RC_FORMAT;

        if (!jumped)
            if (*size <= 0)
                return DNS_RC_FORMAT;

        if (npos>255)
            return DNS_RC_FORMAT;

        if ('_' == *prr && NULL != underscore)
            *underscore=1;

        if (!jumped)
            (*size)--;
        ch = *prr;
        prr++;

        do
        {
            /* The two highest bits must be either 00 or 11 */
            if ( ch>63 && ch<192 && !DNS_RESOLVER_IsDchar(ch) )
                return DNS_RC_FORMAT;

            if (ch >= 192)
            {
                if (prr - data >= msglen)
                    return DNS_RC_FORMAT;

                if (!jumped)
                {
                    if ((*size)<1)
                        return DNS_RC_TRUNC;
                    (*size)--;
                    jumped=1;
                }

                offset = (((UI16_T)ch & 0x3f)<<8)|(*prr);
                if (offset >= msglen)
                    return DNS_RC_FORMAT;

                prr = data + offset;
                hops++;
                if (hops > 255)
                    return DNS_RC_FORMAT;

                ch = *prr;
                prr++;
            }
        } while (ch > 63);

        name[npos] = ch;
        npos++;
        if (0 == ch)
        {
            break;          /* for last I8_T jump out */
        }

        for (i=0; i<ch; i++)
        {
            if (prr -data >= msglen)
                return DNS_RC_FORMAT;

            if (!jumped)
            {
                if (*size <= 0)
                    return DNS_RC_TRUNC;
            }

            if (npos>=255)
                return DNS_RC_FORMAT;

            if (!DNS_RESOLVER_IsDchar(*prr) && ((NULL == underscore) || ('_' != *prr)))
                return DNS_RC_FORMAT;

/*isiah*/
/*          if ('_' == *prr && NULL != underscore)
                *underscore=1;*/

            name[npos] = *prr;
            prr++;
            npos++;
            if (!jumped)
            {
                (*size)--;
            }
        }
    }

    *rest += oldsize - *size;
    *len = npos;

    return DNS_RC_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_Compress
 *
 * PURPOSE:
 *      compress domain name
 *
 * INPUT:
 *      UI8_T* -- name to be compressed
 *      UI8_T* -- offset in dns pkt
 *      darray* -- a series of domain name,added before.
 *
 * OUTPUT:
 *      UI8_T* -- left I8_Tacter.
 *      darray* -- add parameter in if needed.
 *
 * RETURN:
 *      int rl -- number of I8_Tacters after compress
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_Compress(UI8_T* in,
                     UI8_T* out,
                     int offs,
                     darray *d)
{
    UI8_T rest[256];
    UI8_T brest[256];
    int rc,rl,to,i;
    int add = 1;
    int coffs = -1;
    int longest = 0;

    rl = 0;

    /* part 1: compression */
    if (*d)
    {
        for (i=0;i<DNS_DaNumberOfElement(*d);i++)
        {
            rc = DNS_RESOLVER_DomainMatch(&to, in,
                    DNS_DA_INDEX(*d,i,DNS_Compress_T)->string,rest);
            if (rc > longest)
            {
                DNS_RESOLVER_RhnCopy(brest,rest);
                longest=rc;
                coffs = DNS_DA_INDEX(*d,i, DNS_Compress_T)->index+to;
            }
        }
        if (coffs>-1)
        {
            /* omit the length byte, because it needs to be frobbed */
            rl = DNS_RESOLVER_RhnCopy(out, brest)-1;
            out[rl]=192|((coffs&0x3f00)>>8);
            out[rl+1]=coffs&0xff;
            rl+=2;
            add=strlen((char *)brest)!=0;
        }
        else
            rl=DNS_RESOLVER_RhnCopy(out,in);
    }
    else
        rl=DNS_RESOLVER_RhnCopy(out,in);

    /* part 2: addition to the cache structure */
    if (add)
    {
        if (!*d)
        {
            if (!(*d = DNS_DA_CREATE(DNS_Compress_T)))
                return 0;
        }
        if (!(*d = DNS_DaGrow(*d, 1)))
            return 0;

        DNS_DA_LAST(*d, DNS_Compress_T)->index = offs;
        DNS_RESOLVER_RhnCopy(DNS_DA_LAST(*d, DNS_Compress_T)->string,in);
    }

    return rl;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_DomainMatch
 *
 * PURPOSE:
 *      match 2 domain name
 *
 * INPUT:
 *      UI8_T* --
 *      UI8_T* --

 * OUTPUT:
 *      UI8_T* -- left char
 *      int* --
 *
 * RETURN:
 *      number of same label
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_DomainMatch(int* offset,
                         UI8_T* newname,
                         UI8_T* oldname,
                         UI8_T* rest)
{
    UI8_T sbuf[257],dbuf[257];
    int offs, slen, dlen, count, nc,min;

    sbuf[0]='.';
    dbuf[0]='.';

    DNS_RESOLVER_Rhn2Str(newname,sbuf+1);
    DNS_RESOLVER_Rhn2Str(oldname,dbuf+1);

    /* If this is the root domain, we have two dots. bad. so this special case test: */
    if(ZERO == strcmp((char *)&sbuf[1],"."))
    {
        *offset = 0;
        rest[0] = '\0';

        return DNS_OK;
    }

    slen = strlen((char *)sbuf);
    dlen = strlen((char *)dbuf);

    if ( (slen < 2) || (dlen < 2) )
        return DNS_OK;

    slen -= 2;
    dlen -= 2;

    nc = count = 0;
    offs = -1;
    min = (slen>dlen? dlen : slen);
    while (count <= min)
    {
        if (tolower(sbuf[slen-count]) != tolower(dbuf[dlen-count]))
            break;

        if ('.' == sbuf[slen-count])
        {
            /* one complete name part matched. Set the offset */
            nc++;
            offs = count;
        }
        count++;
    }
    *offset = dlen-offs;
    memset(rest,'\0',256);

    if (slen-offs > 0)
        memcpy(rest,newname,slen-offs);

    return nc;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_SbeltInit
 *
 * PURPOSE:
 *      initiate sbelt list
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_SbeltInit()
{
    DNS_Sbelt_T* sbelt_p;
    DNS_ResConfigSbeltEntry_T *sbelt_entry_p;
    int ret;
    int i = 0;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(gdns_resolver_sem);  /* pgr0695, return value of statement block in macro */

    while(NULL != gdns_sbelt_head)
    {
        sbelt_p = gdns_sbelt_head;
        gdns_sbelt_head = gdns_sbelt_head->next_p;
        DNS_RESOLVER_DNS_SBELT_ENTRY_FREE(sbelt_p);
        sbelt_p = NULL;
    }

    if(NULL == DNS_MGR_GetDnsSbelt())
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);
        return DNS_ERROR;
    }

    sbelt_entry_p = (DNS_ResConfigSbeltEntry_T *)DNS_RESOLVER_TMP_BUFFER_MALLOC(sizeof(DNS_ResConfigSbeltEntry_T));
    if(NULL == sbelt_entry_p)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);
        return DNS_ERROR;
    }

    memset(&sbelt_entry_p->dnsResConfigSbeltAddr, 0, sizeof(L_INET_AddrIp_T));

    ret = DNS_OM_GetNextDnsResConfigSbeltEntry(sbelt_entry_p);

    while(DNS_OK == ret)
    {
        ret = DNS_RESOLVER_SbeltAdd(sbelt_entry_p->dnsResConfigSbeltName,
                        &sbelt_entry_p->dnsResConfigSbeltAddr,
                        53);

        if(DNS_ERROR == ret)
        {
            if( 0 == i)
            {

                SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);
                DNS_RESOLVER_TMP_BUFFER_FREE(sbelt_entry_p);
                return DNS_ERROR;
            }
            else
                break;
        }
        ++i;

            ret = DNS_OM_GetNextDnsResConfigSbeltEntry(sbelt_entry_p);

    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);

    DNS_RESOLVER_TMP_BUFFER_FREE(sbelt_entry_p);
    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_SbeltAdd
 *
 * PURPOSE:
 *      add sbelt ( 1 item ) to sbelt list
 *
 * INPUT:
 *      I8_T* domain -- sbelt domain name
 *      L_INET_AddrIp_T -- sbelt address
 *      int -- sbelt port
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_SbeltAdd(I8_T* domain, L_INET_AddrIp_T *addr_p, int port)
{

    DNS_Sbelt_T* sbelt_p;

    /* SBELT init, add primary&slave DNS server in gdns_send_head */

    sbelt_p = DNS_RESOLVER_DNS_SBELT_ENTRY_MALLOC();
    if(NULL == sbelt_p)
        return DNS_ERROR;

    memset(sbelt_p,0,sizeof(DNS_Sbelt_T));

    strcpy((char *)sbelt_p->nsdomain, (char *)domain);

    switch(addr_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        {
            struct sockaddr_in *sin;

            sin = (struct sockaddr_in *)&sbelt_p->sin;
            sin->sin_family = AF_INET;
            sin->sin_port = L_STDLIB_Hton16(port);
            memcpy(&sin->sin_addr, addr_p->addr, addr_p->addrlen);
            sbelt_p->sin.saddr_len =sizeof(struct sockaddr_in);
        }
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
        {
            struct sockaddr_in6 *sin6;

            sin6 = (struct sockaddr_in6 *)&sbelt_p->sin;
            sin6->sin6_family = AF_INET6;
            sin6->sin6_port = L_STDLIB_Hton16(port);
            sin6->sin6_scope_id = addr_p->zoneid;
            memcpy(&sin6->sin6_addr, addr_p->addr, addr_p->addrlen);
            sbelt_p->sin.saddr_len =sizeof(struct sockaddr_in6);
        }
            break;

        default:
            return DNS_ERROR;
    }

    sbelt_p->ra = 1;
    sbelt_p->priority = 1;
    sbelt_p->status = 1;            /* for on */

    sbelt_p->next_p = gdns_sbelt_head;
    gdns_sbelt_head = sbelt_p;

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_SendFree
 *
 * PURPOSE:
 *      free a request
 *
 * INPUT:
 *      DNS_Send_T* -- request node to be freed
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_RESOLVER_SendFree(DNS_Send_T* send_p)
{
    DNS_Slist_T* slist_p;
    DNS_CacheRR_T* cache_p;
    UI32_T orig_priority;

    if(NULL == send_p)
    {
        if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
            printf("\nDNS_SendFree:send_p is NULL,pls check error.");

        return;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(gdns_resolver_sem);  /* pgr0695, return value of statement block in macro */
    gdns_hook_count--;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);

    if(NULL != send_p->sendbuf)
    {
        DNS_RESOLVER_SENDBUF_ENTRY_FREE(send_p->sendbuf);   /* malloc a block with name added */
        send_p->sendbuf = NULL;
    }

    if(NULL != send_p->recvbuf)
    {
        DNS_RESOLVER_RECVBUF_ENTRY_FREE(send_p->recvbuf);   /* malloc a block with sizeof 512 */
        send_p->recvbuf = NULL;
    }

    while(NULL != send_p->rrlist_p)
    {
        cache_p = send_p->rrlist_p;
        send_p->rrlist_p = send_p->rrlist_p->next_p;
        DNS_RESOLVER_DNS_CACHERR_ENTRY_FREE(cache_p);
        cache_p = NULL;
    }

    while(NULL != send_p->slist)
    {
        slist_p = send_p->slist;
        send_p->slist = send_p->slist->next_p;
        DNS_RESOLVER_DNS_SLIST_ENTRY_FREE(slist_p);
        slist_p = NULL;
    }

    if(0 != send_p->sock)
        s_close(send_p->sock);

    /* DO NOT free HOSTENT,IT"S GLOBAL,SOMEONE WILL free IT */
    DNS_RESOLVER_DNS_SEND_ENTRY_FREE(send_p);
    send_p = NULL;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_SendHook
 *
 * PURPOSE:
 *      hook a request on request list
 *
 * INPUT:
 *      DNS_Send_T* send_p -- node to be hook
 *      const I8_T* qname -- name to query
 *      int qtype -- query type
 *      int op --  query opcode
 *      int inter_id -- callback parameter
 *      unsigned int rid -- request rr id,0 for local request
 *      DNS_CacheRR_T* cache_p -- matched nameserver in cache.
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_SendHook(DNS_Send_T* send_p,
                      const I8_T* qname,
                      int qtype,
                      int op,
                      int inter_id,
                      unsigned int rid,
                      DNS_CacheRR_T* cache_p)
{
    DNS_Slist_T* sbelt_head_p = gdns_sbelt_head;
    DNS_Slist_T* slist_p = NULL;
    DNS_Hdr_T*  hdr = NULL;
    UI8_T* qrr;
    UI16_T magic;
    int len,ret;
    int i = 0;
    int max_hook_count;
    UI32_T orig_priority;
    struct sockaddr_in *sin;
    struct sockaddr_in6 *sin6;

    if(DNS_DISABLE == DNS_MGR_GetDnsStatus())       /* dns disabled */
    {
        return DNS_ERROR;
    }

    DNS_MGR_GetDnsServConfigMaxRequests(&max_hook_count);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(gdns_resolver_sem);  /* pgr0695, return value of statement block in macro */

    if(gdns_hook_count  > max_hook_count)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);

        if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        {
            printf("\nHook count is excessive...\n");
        }
        return DNS_ERROR;
    }

    gdns_hook_count++;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);

    len = sizeof(DNS_Hdr_T) + strlen((char *)qname) + 6;/* ret = strlen(buf) = name + type + class */
                                    /* 6 = 2(type)+2(class)+1(blank)+1(rhn more than str)*/
    qrr = (UI8_T *)DNS_RESOLVER_SENDBUF_ENTRY_MALLOC(len);
    if(NULL == qrr)
        return DNS_ERROR;

    memset(qrr,0,len);

    hdr = (DNS_Hdr_T*)qrr;
    send_p->rid         = rid;
    send_p->timeout     = DNS_Ttl2Ticks(gdns_remote_timeout);
    send_p->flags   = 0;                /* later changed    */
    send_p->opcode  = op;
    send_p->event   = DNS_QEV_WRITE;
    send_p->qtype   = qtype;                /* no ntohs/htons  */
    send_p->qm      = DNS_TCP_UDP;          /* both TCP & UDP */
    send_p->state   = DNS_QS_UDPINITIAL;    /* if UDP data truncated,TCP */

    ret = DNS_RESOLVER_Str2Rhn((UI8_T *)qname, (UI8_T *)send_p->qrhn);/* buf = name+'\0'+type+class */
    if(DNS_OK != ret)
    {
        DNS_RESOLVER_SENDBUF_ENTRY_FREE(qrr);
        qrr = NULL;
        send_p->sendbuf = NULL;     /* equal to qrr */
        return DNS_ERROR;
    }

    send_p->sendbuf     = hdr;
    send_p->sendl   = len;
    send_p->inter_id    = inter_id;     /* inter_id,rid exclusive,but maybe equal */
    send_p->myrid   = RAND16;

    hdr->id         = L_STDLIB_Hton16(send_p->myrid);

    hdr->qr         = DNS_QR_QUERY;/* it's OK,since 2000 log inversed. */
    hdr->opcode     = DNS_OP_QUERY;
    hdr->aa         = 0;            /* endian !!!! */
    hdr->tc         = 0;
    hdr->z = 0;
    hdr->rcode      = DNS_RC_OK;     /* another I8_T */
    hdr->qdcount    = L_STDLIB_Hton16(1);

    if(NULL == cache_p)
    {
        DNS_MGR_GetDnsServConfigRecurs(&gdns_sbelt_send_recursive);
        if(gdns_sbelt_send_recursive == 3)
        {
            hdr->rd = 0;
        }
        else
        {
            hdr->rd     = gdns_sbelt_send_recursive;
        }
        hdr->ra     = gdns_sbelt_recv_recursive;
        send_p->trusted = 1;
    }
    else
    {
        DNS_MGR_GetDnsServConfigRecurs(&gdns_slist_send_recursive);
        if( gdns_slist_send_recursive == 3 )
        {
            hdr->rd = 0;
        }
        else
        {
            hdr->rd     = gdns_slist_send_recursive;
        }
        hdr->ra     = gdns_slist_recv_recursive;
        send_p->trusted = 0;
    }

    qrr = (UI8_T*)(hdr+1);
    strcpy((char *)qrr, (char *)send_p->qrhn);

    qrr += strlen((char *)send_p->qrhn)  ;  /* do not +1 !!,followed do this */
    *qrr = '\0';
    qrr += 1;                       /* for '\0' */
    magic = L_STDLIB_Hton16(qtype);
    memcpy(qrr,&magic,2);

    qrr += 2;
    magic = L_STDLIB_Hton16(DNS_RRC_IN);
    memcpy(qrr,&magic,2);

    if( NULL == sbelt_head_p )
    {
        return DNS_ERROR;
    }

    /* copy sbelt as slist
     */
    while(NULL != sbelt_head_p)
    {
        slist_p = DNS_RESOLVER_DNS_SLIST_ENTRY_MALLOC();
        if(NULL == slist_p)
        {
            DNS_RESOLVER_SendFree(send_p);
            if(0 == i)
            {
                if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
                {
                    printf("DNS_RESOLVER_SendHook: malloc fail.\n");
                }

                return DNS_ERROR;
            }
            else
            {
                break;
            }
        }

        memcpy(slist_p,sbelt_head_p,sizeof(DNS_Slist_T));
        slist_p->next_p = send_p->slist;
        send_p->slist = slist_p;

        sbelt_head_p = sbelt_head_p->next_p;
    }

    /* copy sbelt as slist */
    /* we have found his paraent. */
    while(NULL != cache_p)
    {
        if(DNS_RRT_NS != cache_p->type)
        {
            cache_p = cache_p->next_p;
            continue;
        }

        slist_p = DNS_RESOLVER_DNS_SLIST_ENTRY_MALLOC();
        if(NULL == slist_p)
        {
            break;              /* break is OK.*/
        }

        memset(slist_p,0,sizeof(DNS_Slist_T));

        /*IPV4*/
        if(cache_p->ip.type == L_INET_ADDR_TYPE_IPV4)
        {
            sin = (struct sockaddr_in *)&slist_p->sin;
            sin->sin_family = AF_INET;
            sin->sin_port = L_STDLIB_Hton16(DNS_RESOLVER_DEFAULT_PORT);        /* DNS service port*/
            memcpy(&sin->sin_addr, cache_p->ip.addr, cache_p->ip.addrlen);
            slist_p->sin.saddr_len=sizeof(struct sockaddr_in);

        }
        /*IPV6*/
        else if(cache_p->ip.type == L_INET_ADDR_TYPE_IPV6 || cache_p->ip.type == L_INET_ADDR_TYPE_IPV6Z)
        {
            sin6 = (struct sockaddr_in6 *)&slist_p->sin;
            sin6->sin6_family = AF_INET6;
            sin6->sin6_port = L_STDLIB_Hton16(DNS_RESOLVER_DEFAULT_PORT);      /* DNS service port*/
            memcpy(&sin6->sin6_addr, cache_p->ip.addr, cache_p->ip.addrlen);
            slist_p->sin.saddr_len=sizeof(struct sockaddr_in6);
        }

        slist_p->status = DNS_SLIST_STATUS_OK;  /* ready for send */
        slist_p->next_p = send_p->slist;        /* according cache_p struct,adjust it */
        send_p->slist = slist_p;

        cache_p         = cache_p->next_p;
    }

    send_p->addr = (struct sockaddr *)&send_p->slist->sin;
    send_p->sinl =send_p->slist->sin.saddr_len;
    send_p->sock = 0;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(gdns_resolver_sem);  /* pgr0695, return value of statement block in macro */
    send_p->next_p = gdns_send_head;
    gdns_send_head = send_p;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_Retransmit
 *
 * PURPOSE:
 *      re-transmit DNS request
 *
 * INPUT:
 *      DNS_Send_T * -- request node
 *
 * OUTPUT:
 *      DNS_Send_T * -- change name server to request.
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_RESOLVER_Retransmit(DNS_Send_T *send_p)
{
    DNS_Slist_T* slist_p;

    if(NULL == send_p)
    {
        if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        {
            printf("\nDNS_ResolverRetransmit:send_p is NULL,pls check error.\n");
        }
        return;
    }

    slist_p = send_p->slist;
    if(NULL == slist_p)
    {
        if(NULL == send_p->callback)
        {
            SYSFUN_ResumeThread(send_p->inter_id);
        }
        else
        {
            if(NULL != send_p->sendbuf)
            {
                DNS_RESOLVER_SENDBUF_ENTRY_FREE(send_p->sendbuf);
            }

            send_p->sendbuf =
                DNS_RESOLVER_ErrorMake(DNS_RC_SERVFAIL,send_p->rid,send_p->opcode);

            if(NULL != send_p->sendbuf)
            {
                send_p->callback(send_p->inter_id,send_p->sendbuf,12);
            }
            else
            {
                send_p->callback(send_p->inter_id,NULL,0);
            }
        }
        send_p->state = DNS_QS_FREE;
        return;
    }

    if(send_p->state < DNS_QS_TCPLREAD) /* if TCP,must once received sth.just re-try.*/
        return;

    send_p->state = DNS_QS_UDPINITIAL;

    /* if write,keep.*/
    if(DNS_QEV_READ == send_p->event)
    {
        send_p->event = DNS_QEV_READWRITE;
    }

    send_p->slist = slist_p->next_p;
    if( send_p->slist != NULL )
    {
        send_p->addr = (struct sockaddr*)&(send_p->slist->sin);
        strcpy((char *)send_p->nsdomain, (char *)send_p->slist->nsdomain);
        DNS_RESOLVER_DNS_SLIST_ENTRY_FREE(slist_p);
        slist_p = NULL;
    }
    else
    {
/*isiah.2003-09-03*/
        DNS_RESOLVER_DNS_SLIST_ENTRY_FREE(slist_p);
        slist_p = NULL;
        if(NULL == send_p->callback)
        {
            SYSFUN_ResumeThread(send_p->inter_id);
        }
        else
        {
            if(NULL != send_p->sendbuf)
            {
                DNS_RESOLVER_SENDBUF_ENTRY_FREE(send_p->sendbuf);
            }

            send_p->sendbuf =
                DNS_RESOLVER_ErrorMake(DNS_RC_SERVFAIL,send_p->rid,send_p->opcode);

            if(NULL != send_p->sendbuf)
            {
                send_p->callback(send_p->inter_id,send_p->sendbuf,12);
            }
            else
            {
                send_p->callback(send_p->inter_id,NULL,0);
            }
        }
        send_p->state = DNS_QS_FREE;
        return;
    }

    DNS_MGR_ResOptCounterInc(LEAF_dnsResOptCounterRetrans);

    if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
    {
        printf("\nServ failed.I am re-transmit for answer. \n");
    }
}

/*
 * FUNCTION NAME : DNS_RESOLVER_AdjustRR
 *
 * PURPOSE:
 *      make sure first cache node has min ttl, and, cache list has 2 different
 *      name at most.
 *
 * INPUT:
 *      DNS_CacheRR_T* -- cache list to be adjust
 *      I8_T* -- query name.
 *
 * OUTPUT:
 *      DNS_CacheRR_T* -- cache list having been adjusted
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      qname in dns format.
 *
 */
void DNS_RESOLVER_AdjustRR(DNS_CacheRR_T* cache_p,
                                I8_T* qname)
{
    DNS_CacheRR_T* pbCache = cache_p;
    unsigned int minttl = gdns_max_ttl;
    UI8_T buf[256];

    DNS_RESOLVER_Rhn2Str((UI8_T *)qname,buf);
    if(NULL == cache_p)
        return;

    /* make sure first element's every field not NULL,and,his rrl is min.*/
    while(NULL != cache_p)
    {
        cache_p->flag = DNS_CF_NOAUTH;
        if(cache_p->ttl < minttl)
            minttl = cache_p->ttl;

    /*
     * according to original processing in DNS_RESOLVER_AddCache, content of cache
     * only is the following: one A RR or one CNAME RR + one A RR. so the below code
     * actually isn't arriving, we remove it!
     * in fact, original processing in DNS_RESOLVER_AddCache has a problem, it is that
     * DNS server may return a CNAME RR followed by multiple A RRs, so when appears the
     * situation, if continuing to adjust cache content in terms of the below codes, it
     * will cause a error!! we MUST remove it!
     */
#if 0
        /* A->B;B->C */
        if(NULL != cache_p->next_p && ZERO != strcmp(cache_p->name, (char *)buf))
        {
            I8_T* bname = (I8_T *)cache_p->name;
            cache_p->type = DNS_RRT_A;
            strcpy((char *)cache_p->name, (char *)bname);
        }
#endif

        cache_p = cache_p->next_p;
    }

    if(minttl < gdns_min_ttl)
        minttl = gdns_min_ttl;

    pbCache->ttl = minttl;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_ResponseProcess
 *
 * PURPOSE:
 *      entry of process response pkt
 *
 * INPUT:
 *      DNS_Send_T* -- request node
 *      long -- response pke length
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_ResponseProcess(DNS_Send_T* send_p,long rlen)
{
    DNS_Hdr_T* hdr = send_p->recvbuf;
    DNS_CacheRR_T* del_p = NULL;
    UI8_T* prr = (UI8_T*)(hdr+1);
    int underscore,aa,nlen,rc,ret;
    int remain = rlen;
    UI16_T magic;
    unsigned int ttl;
    I8_T buf[256];
    int cache_status;

        /* original *aa from send_p */
    if (DNS_QT_ALL != send_p->qtype)
        aa = hdr->aa;
    else
    {
        aa = 0;
        send_p->flags |= DNS_CF_NOAUTH;

        DNS_MGR_ResCounterInc(LEAF_dnsResCounterNonAuthDataResps);
    }

        /* negative cacheing for domains */
    /*isiah.2003-02-17.Pass rcode to caller.*/
    send_p->rcode = (UI32_T)hdr->rcode;

    if (DNS_RC_NAMEERR == hdr->rcode)
    {
        if(0 == aa)
        {
            DNS_MGR_ResCounterInc(LEAF_dnsResCounterNonAuthNoDataResps);
            DNS_MGR_ServCounterInc(LEAF_dnsServCounterAuthNoDataResps);
        }
        else
        {
            DNS_MGR_ServCounterInc(LEAF_dnsServCounterAuthNoNames);
        }

        /* We did not get what we wanted. Cache according to policy */
        if(NULL != send_p->sendbuf)
        {
            DNS_RESOLVER_SENDBUF_ENTRY_FREE(send_p->sendbuf);
            send_p->sendbuf = NULL;
        }

        /* DNS_RESOLVER_ErrorMake malloced mem free in DNS_RESOLVER_SendFree */
        send_p->sendbuf = DNS_RESOLVER_ErrorMake(DNS_RC_NAMEERR,send_p->rid,send_p->opcode);
        if(NULL != send_p->sendbuf)
        {
            send_p->sendl = sizeof(DNS_Hdr_T);
        }
        else
        {
            send_p->sendl = 0;
        }

        if(DNS_RRT_PTR == send_p->qtype)
        {
            return DNS_OK;
        }

        while(NULL != send_p->rrlist_p)
        {
            del_p = send_p->rrlist_p;
            send_p->rrlist_p = send_p->rrlist_p->next_p;
            DNS_RESOLVER_DNS_CACHERR_ENTRY_FREE(del_p);
            del_p = NULL;
        }

        send_p->rrlist_p = DNS_RESOLVER_DNS_CACHERR_ENTRY_MALLOC();
        if(NULL == send_p->rrlist_p)
        {
            DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
            DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterFriendsOtherErrors);

            return DNS_OK;          /* though failed,just not cached.return */
        }
        memset(send_p->rrlist_p,0,sizeof(DNS_CacheRR_T));

        DNS_RESOLVER_Rhn2Str((UI8_T *)send_p->qrhn, (UI8_T *)send_p->rrlist_p->name);
        ttl = DNS_Ttl2Ticks(gdns_negative_ttl);
        memcpy(&(send_p->rrlist_p->ttl),&ttl,sizeof(ttl));
        send_p->rrlist_p->flag = DNS_CF_NEGATIVE;
        send_p->rrlist_p->next_p = NULL;
        send_p->rrlist_p->type = DNS_RRT_MIN;
        send_p->rrlist_p->arrived_time = SYSFUN_GetSysTick();
        send_p->hp = NULL;          /* name sure */

        return DNS_OK;
    }

    if(DNS_RC_OK != hdr->rcode && DNS_RC_NAMEERR != hdr->rcode)
    {
        DNS_RESOLVER_RECVBUF_ENTRY_FREE(send_p->recvbuf);
        send_p->recvbuf = NULL;
        send_p->hp = NULL;

        return DNS_ERROR;
    }

    remain -= sizeof(DNS_Hdr_T);
    if (1 != L_STDLIB_Ntoh16(hdr->qdcount))
    {
        DNS_MGR_ResCounterInc(LEAF_dnsResCounterMartians);
        /* Bad number of query records in answer */
        DNS_RESOLVER_RECVBUF_ENTRY_FREE(hdr);
        hdr = NULL;
        return DNS_RC_FORMAT;
    }

    /* decompress query part */
    rc = DNS_RESOLVER_Decompress((UI8_T*)hdr,(UI8_T *)buf,&prr,&remain,&nlen, (int)rlen,&underscore);
    if (DNS_RC_OK != rc)
    {
        DNS_MGR_ResCounterInc(LEAF_dnsResCounterUnparseResps);

        return rc == DNS_RC_TRUNC?DNS_RC_FORMAT:rc;
    }

    /* DNS format(rhn) string compare */
    if(ZERO != DNS_RESOLVER_RhnCmp((UI8_T *)send_p->qrhn, (UI8_T *)buf))
    {
        DNS_MGR_ResCounterInc(LEAF_dnsResCounterMartians);

        return DNS_RC_SERVFAIL;
    }

    /*copy QTYPE*/
    memcpy(&magic,prr,2);
    prr += 2;

    /*copy QCLASS*/
    memcpy(&magic,prr,2);
    prr += 2;

    remain  -= 4;

    magic = L_STDLIB_Ntoh16(hdr->ancount);
    if(L_STDLIB_Ntoh16(hdr->ancount) > 0)
    {
        /* prr point to answer part */
        ret = DNS_RESOLVER_AddCache(send_p,&prr,magic,&remain,DNS_ANSWER_PART);
        if(DNS_OK != ret)
        {
            return DNS_ERROR;
        }

        /* since following code addedm,appropriate code in _addcache should cut */

        if(NULL == send_p->callback)
        {
            /* be care, don't need make sure send_p->hp = NULL !! */
            del_p = send_p->rrlist_p;

            while(NULL != del_p)
            {
                if(NULL == send_p->hp || NULL == send_p->hp->h_name)    /* dangerous !! ??,hp = null  */
                {
                    DNS_RESOLVER_HostentAdd(&(send_p->hp),DNS_HOSTENT_CNAME,(I8_T *)del_p->name,0);
                }
                else
                {
                    DNS_RESOLVER_HostentAdd(&(send_p->hp),DNS_HOSTENT_ALIAS, (I8_T *)del_p->name,0);
                }

                if(0 != del_p->ip.addrlen)
                    DNS_RESOLVER_HostentAdd(&(send_p->hp),DNS_HOSTENT_IP,NULL,&del_p->ip);

                del_p = del_p->next_p;
            }
        }

    }

    if ((0 == L_STDLIB_Ntoh16(hdr->ancount)) &&
        (L_STDLIB_Ntoh16(hdr->nscount) > 0))
    {
        DNS_MGR_ResOptCounterInc(LEAF_dnsResOptCounterReferals);

        if(NULL != send_p->callback)
        {
            DNS_MGR_ResOptCounterInc(LEAF_dnsServOptCounterFriendsReferrals);
        }
        else
        {
            DNS_MGR_ResOptCounterInc(LEAF_dnsServOptCounterSelfReferrals);
        }
    }

    if(0 == L_STDLIB_Ntoh16(hdr->ancount)) /* since we have got answer,cut followed op */
    {
        magic = L_STDLIB_Ntoh16(hdr->nscount); /* for easy display,later cut */
        if (L_STDLIB_Ntoh16(hdr->nscount) > 0)
        {
             ret = DNS_RESOLVER_AddCache(send_p,&prr,magic,&remain,DNS_NSERVER_PART);
             if(DNS_OK != ret)
             {

                return DNS_ERROR;
             }
        }

         magic = L_STDLIB_Ntoh16(hdr->arcount); /* for easy display,later cut */
        if (L_STDLIB_Ntoh16(hdr->arcount) > 0)
        {
            ret = DNS_RESOLVER_AddCache(send_p,&prr,magic,&remain,DNS_ADDI_PART);
            if(DNS_OK != ret)
            {
                return DNS_ERROR;
            }
        }
    }
    /* warning ::: DO NOT change order !! */
    DNS_RESOLVER_AdjustRR(send_p->rrlist_p,send_p->qrhn); /* first element's every field not NULL */

    DNS_MGR_GetDnsResCacheStatus(&cache_status);
    if(VAL_dnsResCacheStatus_enabled == cache_status)
    {
        if(send_p->rrlist_p != NULL)
        {
            if(DNS_CACHE_SetRR(send_p->rrlist_p)!=DNS_OK)
            {
                if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
                {
                    printf("Set RR to cache error\n");
                }
            }
        }
        else
        {
            if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
            {
              printf("\n rrlist_p NULL!!!");
            }
        }
    }

    return DNS_RC_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_RxTx
 *
 * PURPOSE:
 *      transmit/receive pkt
 *
 * INPUT:
 *      DNS_Send_T* -- request node
 *
 * OUTPUT:
 *      DNS_Send_T* --
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_RxTx(DNS_Send_T* send_p)
{
    DNS_TYPE_SockAddr_T saddr_buf;
    struct sockaddr_in *sin;
    struct sockaddr_in6 *sin6;
    socklen_t addlen = sizeof(saddr_buf);
    int rc = DNS_ERROR;
    int ret = DNS_ERROR;

    switch (send_p->state)
    {
#ifdef DNS_UDP_QUERIES
    /* UDP query code */
    case DNS_QS_UDPINITIAL:
        if ( (send_p->sock = socket(send_p->addr->sa_family,SOCK_DGRAM,0)) <0 )
        {
            rc = DNS_RC_SERVFAIL; /* mock error code */
            break;
        }

        memset(&saddr_buf, 0, sizeof(saddr_buf));

        switch(send_p->addr->sa_family)
        {
             case AF_INET:
                sin = (struct sockaddr_in *)&saddr_buf;
                sin->sin_family = AF_INET;
                sin->sin_port = L_STDLIB_Hton16 (0);
                sin->sin_addr.s_addr = L_STDLIB_Hton16(INADDR_ANY);
                saddr_buf.saddr_len = sizeof(struct sockaddr_in);
                break;

             case AF_INET6:
                sin6 = (struct sockaddr_in6 *)&saddr_buf;
                sin6->sin6_family = AF_INET6;
                sin6->sin6_port = L_STDLIB_Hton16(0);
                memcpy((void *)&sin6->sin6_addr, (void *)&in6addr_any, sizeof(in6addr_any));
                saddr_buf.saddr_len = sizeof(struct sockaddr_in6);
                break;

            default:
                rc = DNS_RC_SERVFAIL;
                break;
        }

        if ( bind(send_p->sock, (struct sockaddr *)&saddr_buf, saddr_buf.saddr_len) < 0 )
        {
            rc = DNS_RC_SERVFAIL; /* mock error code */
            break;
        }

        /* we have initial a socket before */
        ret = sendto(send_p->sock,(I8_T*)(send_p->sendbuf),
                send_p->sendl,0,send_p->addr,saddr_buf.saddr_len);

        if ( ret<0 )
        {
            rc = DNS_RC_SERVFAIL; /* mock error code */
            break;
        }

        memset(&send_p->recv_addr, 0, sizeof(send_p->recv_addr));

        getsockname(send_p->sock, (struct sockaddr*)&saddr_buf, &addlen);

        memcpy(&send_p->recv_addr, &saddr_buf, addlen);

        s_close(send_p->sock);

        if ( (send_p->sock = socket(send_p->addr->sa_family,SOCK_DGRAM,0)) <0 )
        {
            rc = DNS_RC_SERVFAIL; /* mock error code */
            break;
        }

        if ( bind(send_p->sock, (struct sockaddr *)&(send_p->recv_addr), addlen) < 0 )
        {
            rc = DNS_RC_SERVFAIL; /* mock error code */
            break;
        }

        /* this maybe occur while re-transmit */
        if(NULL != send_p->recvbuf)
        {
            DNS_RESOLVER_RECVBUF_ENTRY_FREE(send_p->recvbuf);
        }
        send_p->recvbuf = DNS_RESOLVER_RECVBUF_ENTRY_MALLOC(512);
        if (NULL == send_p->recvbuf)
        {
            send_p->state = DNS_QS_DONE;
            rc = DNS_RC_SERVFAIL;
            break;
        }
        memset(send_p->recvbuf,0,512);
        send_p->state=DNS_QS_UDPRECEIVE;
        send_p->event=DNS_QEV_READ;

        return DNS_ERROR;   /* return for next select */

    case DNS_QS_UDPRECEIVE:
        /* check for NULL
         */
        if (send_p->recvbuf == NULL)
        {
            return DNS_ERROR;
        }

        ret = recvfrom(send_p->sock,(I8_T*)(send_p->recvbuf),
                        512,0,(struct sockaddr*)&(send_p->recv_addr.s_addr_u),&addlen);

        if(ret <0)
        {
            send_p->state = DNS_QS_DONE;
            rc = DNS_RC_SERVFAIL;
            break;
        }

        DNS_MGR_ResCounterInc(LEAF_dnsResCounterRecdResponses);
        DNS_MGR_ResCounterByRcodeInc(send_p->recvbuf->rcode);
        DNS_MGR_ResCounterByOpcodeInc(DNS_QR_RESP,send_p->recvbuf->opcode);

        send_p->recvl = ret;

        if ((send_p->recvl < sizeof(DNS_Hdr_T)) ||
            (L_STDLIB_Ntoh16(send_p->recvbuf->id) !=  send_p->myrid))
        {
            /* no need to care about timeouts here. That is done at an upper layer. */
            send_p->state = DNS_QS_UDPRECEIVE;
            send_p->event = DNS_QEV_READ;

            /*isiah.2003-09-01*/
            s_close(send_p->sock);
            send_p->sock = 0;
            return DNS_ERROR;
        }

        send_p->state = DNS_QS_DONE;
        s_close(send_p->sock);
        send_p->sock = 0;
        rc = DNS_RC_OK;
        break;
#endif

#ifdef DNS_TCP_QUERIES
    /* TCP query code */
    case DNS_QS_TCPINITIAL:
        /* socket created in DNS_RESOLVER_SendHook for UDP use,if udp received DNS pkt set */
        /* tc bit, then close socket,turn state to TCPINITIAL,so here must re-created.!!*/
        if (DNS_ERROR == (send_p->sock = socket(send_p->addr->sa_family,SOCK_STREAM,0)))
        {
            send_p->sendbuf = DNS_RESOLVER_ErrorMake(DNS_RC_SERVFAIL,send_p->rid,send_p->opcode);
            if(NULL == send_p->sendbuf)
                send_p->state = DNS_QS_FREE;
            else
                send_p->state = DNS_QS_DONE;    /* socket fail,what can I do! */
            return DNS_ERROR;       /* for next,it will send error or free Send. */
        }

        /* make the socket non-blocking for connect only, so that connect will not hang */
        if ((rc = connect(send_p->sock,send_p->addr,send_p->sinl)) != 0)
        {
            if (EINPROGRESS == rc || EPIPE == rc )
            {
                send_p->state = DNS_QS_TCPCONNECT;
                send_p->event = DNS_QEV_WRITE; /* wait for writablility; the connect have done */
                return DNS_ERROR;
            }

            else if (ECONNREFUSED == rc)
            {
                DNS_MGR_ServCounterInc(LEAF_dnsServCounterReqRefusals);

                send_p->sendbuf = DNS_RESOLVER_ErrorMake(DNS_RC_SERVFAIL,send_p->rid,send_p->opcode);
                if(NULL == send_p->sendbuf)
                    send_p->state = DNS_QS_FREE;
                else
                    send_p->state = DNS_QS_DONE;    /* socket fail,what can I do! */

                return DNS_ERROR;       /* for next,it will send error or free Send. */
            }
        }
        else
            send_p->state = DNS_QS_TCPCONNECT;

    case DNS_QS_TCPCONNECT:

        if (DNS_ERROR == write(send_p->sock,(I8_T*)&send_p->sendl,sizeof(send_p->sendl)))
        {
            if (rc==ECONNREFUSED || rc==EPIPE)
            {
                DNS_MGR_ServCounterInc(LEAF_dnsServCounterReqRefusals);

                if(NULL == send_p->callback)
                    DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterSelfReqRefusals);
                else
                    DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterFriendsReqRefusals);
            }

            send_p->sendbuf = DNS_RESOLVER_ErrorMake(DNS_RC_SERVFAIL,send_p->rid,send_p->opcode);
            if(NULL == send_p->sendbuf)
                send_p->state = DNS_QS_FREE;
            else
                send_p->state = DNS_QS_DONE;    /* socket fail,what can I do! */
            return DNS_ERROR;       /* for next,it will send error or free Send. */
        }
        send_p->state = DNS_QS_TCPLWRITTEN;
        send_p->event = DNS_QEV_WRITE;
        return DNS_ERROR;

    case DNS_QS_TCPLWRITTEN:
        if (DNS_ERROR == write(send_p->sock,(I8_T*)send_p->sendbuf,
            L_STDLIB_Ntoh16(send_p->sendl)))
        {
            send_p->state = DNS_QS_DONE;
            return DNS_RC_SERVFAIL; /* mock error code */
        }
        send_p->state = DNS_QS_TCPQWRITTEN;
        send_p->event = DNS_QEV_READ;
        return DNS_ERROR;

    case DNS_QS_TCPQWRITTEN:
        if (read(send_p->sock,(I8_T*)&send_p->recvl,sizeof(send_p->recvl))
                != sizeof(send_p->recvl))
        {
            send_p->state = DNS_QS_DONE;
            rc = DNS_RC_SERVFAIL; /* mock error code */
            break;
        }

        send_p->recvl=L_STDLIB_Ntoh16(send_p->recvl);

        if(NULL != send_p->recvbuf)
            DNS_RESOLVER_RECVBUF_ENTRY_FREE(send_p->recvbuf);

        send_p->recvbuf=DNS_RESOLVER_RECVBUF_ENTRY_MALLOC(send_p->recvl);
        if (NULL == send_p->recvbuf)
        {
            send_p->state = DNS_QS_DONE;
            rc = DNS_RC_SERVFAIL; /* mock error code */
            break;
        }
        send_p->state = DNS_QS_TCPLREAD;
        send_p->event = DNS_QEV_READ;
        return DNS_ERROR;

    case DNS_QS_TCPLREAD:
        if (read(send_p->sock,(I8_T*)send_p->recvbuf,send_p->recvl)
            != send_p->recvl)
        {
            send_p->state = DNS_QS_DONE;
            rc = DNS_RC_SERVFAIL; /* mock error code */
            break;
        }

        DNS_MGR_ResCounterInc(LEAF_dnsResCounterRecdResponses);
        DNS_MGR_ResCounterByRcodeInc(send_p->recvbuf->rcode);
        DNS_MGR_ResCounterByOpcodeInc(DNS_QR_RESP,send_p->recvbuf->opcode);

        send_p->state = DNS_QS_DONE;
        rc = DNS_RC_OK;
        break;
#endif

    case DNS_QS_DONE:
        return DNS_ERROR;
    }

    if (DNS_RC_SERVFAIL == rc || DNS_RC_TCPREFUSED == rc)
    {
        DNS_RESOLVER_Retransmit(send_p);
    }
	/* shumin.wang modified for ES4827G-FLF-ZZ-00048, 2008-07-31 */
    if(send_p->recvbuf != NULL)
    {
		if(ZERO != send_p->recvbuf->tc)
		{
		    DNS_RESOLVER_RECVBUF_ENTRY_FREE(send_p->recvbuf);
		    send_p->recvbuf = NULL;
		    send_p->state = DNS_QS_TCPINITIAL;
		    send_p->event = DNS_QEV_WRITE;
		    send_p->myrid = RAND16;
		    send_p->sendbuf->id = L_STDLIB_Hton16(send_p->myrid);
		    rc = DNS_RC_TRUNC;
		}
    }
    return rc;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_AddCache
 *
 * PURPOSE:
 *      unpack dns pkt,add rr to cache
 *
 * INPUT:
 *      DNS_Send_T* send_p -- request node
 *      UI8_T** rr -- pointer to current rr
 *      int rrnum -- number of rr
 *      int type-- which part of dns pkt(eg. answer part )
 *
 * OUTPUT:
 *      int* remain-- remain dns pkt length after extract
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_AddCache(DNS_Send_T* send_p,
                              UI8_T** rr,
                              int rrnum,
                              int* remain,
                              int type)
{
    DNS_CacheRR_PTR_T cache_p;
    DNS_Slist_T* slist_p = NULL;
    DNS_RR_T rhdr;
    struct sockaddr_in *sin;
    struct sockaddr_in6 *sin6;
    UI8_T* msg = (UI8_T*)send_p->recvbuf;
    unsigned int tc = send_p->recvbuf->tc;
    UI8_T name[256];
    UI8_T cname[256];   /* rdata part */
    UI8_T buf[256] = {0};
    int namelen,blen,rc,i,underscore;
    L_INET_AddrIp_T pip;

    for (i=0;i<rrnum;i++)
    {
        rc = DNS_RESOLVER_Decompress(msg, name, rr, remain,&namelen,send_p->recvl,&underscore);
        if (DNS_RC_OK != rc)
        {
            DNS_MGR_ResCounterInc(LEAF_dnsResCounterUnparseResps);

            if ((DNS_RC_TRUNC == rc) && tc)
                return DNS_RC_OK;

            return rc == DNS_RC_TRUNC? DNS_RC_FORMAT:rc;
        }

        if (underscore)
        {
            /* Underscore is only allowed in SRV records */
            /*isiah*/
            /*return DNS_RC_FORMAT;*/
        }

        memcpy(&rhdr,*rr,10);   /* replace to sizeof(DNS_RR_T)    */
        *rr += 10;              /* to rdara */
        *remain -= 10;          /* no magic,since not reach there. */

        if (*remain < L_STDLIB_Ntoh16(rhdr.rdlength)) /* make sure has enough rdata */
        {
            DNS_MGR_ResCounterInc(LEAF_dnsResCounterUnparseResps);

            if (tc)
                return DNS_RC_OK;

            return DNS_RC_FORMAT;
        }

        if ((DNS_RRT_MIN <= L_STDLIB_Ntoh16(rhdr.rtype) &&
            DNS_RRT_MAX >= L_STDLIB_Ntoh16(rhdr.rtype) &&
            DNS_RRC_IN == L_STDLIB_Ntoh16(rhdr.rclass)))
        {
            switch (L_STDLIB_Ntoh16(rhdr.rtype))
            {
                case DNS_RRT_MB:
                case DNS_RRT_MD:
                case DNS_RRT_MF:
                case DNS_RRT_MG:
                case DNS_RRT_MR:
                case DNS_RRT_MINFO:
                case DNS_RRT_MX:
                case DNS_RRT_SOA:
                    /*move *rr to point next RR
                     */
                    *remain -= L_STDLIB_Ntoh16(rhdr.rdlength);
                    *rr += L_STDLIB_Ntoh16(rhdr.rdlength);
                    break;

                case DNS_RRT_PTR:
                case DNS_RRT_CNAME:
                case DNS_RRT_NS:

/* make backups for decompression,because rdlength is the authoritative */
                    blen = *remain;

/* record length and pointer and size will by modified by that */
                    rc = DNS_RESOLVER_Decompress(msg, cname, rr, remain,&namelen,
                                        send_p->recvl,&underscore);
                    if (DNS_RC_OK != rc)
                    {
                        DNS_MGR_ResCounterInc(LEAF_dnsResCounterUnparseResps);

                        return rc == DNS_RC_TRUNC? DNS_RC_FORMAT:rc;
                    }

                    if (L_STDLIB_Ntoh16(rhdr.rdlength) != blen - *remain)
                    {
                        DNS_MGR_ResCounterInc(LEAF_dnsResCounterUnparseResps);

                        return DNS_RC_FORMAT;
                    }

                    if (DNS_RRT_NS == L_STDLIB_Ntoh16(rhdr.rtype))
                    {

                        if (!send_p->trusted)
                            DNS_RESOLVER_DomainMatch(&rc, send_p->nsdomain, name,buf);

                        if (send_p->trusted || buf[0]=='\0')
                        {
                            slist_p = DNS_RESOLVER_DNS_SLIST_ENTRY_MALLOC();

                            if(NULL != slist_p)
                            {
                                memset(slist_p,0,sizeof(DNS_Slist_T));
                                memcpy((char*)slist_p->cname, (char*)cname,strlen((char*)cname)+2);
                                DNS_RESOLVER_Rhn2Str(name,slist_p->nsdomain);
                                slist_p->ttl = L_STDLIB_Ntoh32(rhdr.rttl);
                                /*
                                 * since header node of slist is always acted as current name server,
                                 * DNS_RESOLVER_Retransmit will remove header node and use next node
                                 * to try, so we put the new name server into secondary location!!
                                 */
                                slist_p->next_p = send_p->slist->next_p;
                                send_p->slist->next_p = slist_p;
                            }
                        }

                    }
                    else if(DNS_RRT_CNAME == L_STDLIB_Ntoh16(rhdr.rtype))
                    {
                        cache_p = DNS_RESOLVER_DNS_CACHERR_ENTRY_MALLOC();
                        if(NULL == cache_p)
                            break;

                        memset((void *)cache_p, 0, sizeof(DNS_CacheRR_T));

                        DNS_RESOLVER_Rhn2Str(name,(UI8_T*)cache_p->name);
                        cache_p->type = DNS_RRT_CNAME;
                        cache_p->ttl = L_STDLIB_Ntoh32(rhdr.rttl);
                        cache_p->arrived_time = SYSFUN_GetSysTick();
                        cache_p->next_p = send_p->rrlist_p;
                        send_p->rrlist_p = cache_p;
                    }
                    else if(DNS_RRT_PTR == L_STDLIB_Ntoh16(rhdr.rtype))
                    {
                        if((ZERO == DNS_RESOLVER_RhnCmp((UI8_T *)send_p->qrhn,name)
                            && NULL == send_p->callback)
                            || DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
                        {
                            DNS_RESOLVER_Rhn2Str(cname,buf);
                            if (DNS_OK != DNS_RESOLVER_HostentAdd(&send_p->hp, DNS_HOSTENT_CNAME, (I8_T *)buf, 0))
                                return DNS_RC_FORMAT;

                            DNS_RESOLVER_Rhn2Str(name,buf);
                            if (DNS_OK != DNS_RESOLVER_HostentAdd(&send_p->hp, DNS_HOSTENT_IP, NULL, &pip))
                                return DNS_RC_FORMAT;
                        }
                    }
                    break;

                case DNS_RRT_A:
                case DNS_RRT_AAAA:

                    if ((((DNS_RRT_A == L_STDLIB_Ntoh16(rhdr.rtype)) && (4 != L_STDLIB_Ntoh16(rhdr.rdlength)))||
                        ((DNS_RRT_AAAA == L_STDLIB_Ntoh16(rhdr.rtype)) && (16 != L_STDLIB_Ntoh16(rhdr.rdlength)))))
                    {
                        DNS_MGR_ServCounterInc(LEAF_dnsServCounterErrors);

                        return DNS_RC_FORMAT;
                    }

                    memset((void *)&pip, 0, sizeof(pip));

                    if( DNS_RRT_A == L_STDLIB_Ntoh16(rhdr.rtype))
                    {
                        pip.type = L_INET_ADDR_TYPE_IPV4;
                        pip.addrlen = 4;
                        memcpy(pip.addr, *rr, 4);
                    }
#if (SYS_CPNT_IPV6 == TRUE)
                    else
                    {
                        pip.type = L_INET_ADDR_TYPE_IPV6;
                        pip.addrlen = 16;
                        memcpy(pip.addr, *rr, 16);
                    }
#endif

                    *remain -= pip.addrlen;
                    *rr += pip.addrlen;

                    if(DNS_ADDI_PART == type)
                    {
                        slist_p = send_p->slist;
                        while(NULL != slist_p)
                        {
                            DNS_RESOLVER_Rhn2Str(name,buf);
                            if(ZERO == strcmp((char*)slist_p->cname,(char*)buf))
                            {
                                if(0 == slist_p->sin.saddr_len)
                                {
                                    if( DNS_RRT_A == L_STDLIB_Ntoh16(rhdr.rtype))
                                {
                                        sin = (struct sockaddr_in *)&slist_p->sin;
                                        sin->sin_family = AF_INET;
                                        sin->sin_port = L_STDLIB_Ntoh16(53);
                                        memcpy(&sin->sin_addr.s_addr, pip.addr, pip.addrlen);
                                        slist_p->sin.saddr_len = sizeof(struct sockaddr_in);
                                    }
                                    else
                                {
                                        sin6 = (struct sockaddr_in6 *)&slist_p->sin;
                                        sin6->sin6_family = AF_INET6;
                                        sin6->sin6_port = L_STDLIB_Ntoh16(53);
                                        memcpy(&sin6->sin6_addr, &pip.addr, pip.addrlen);
                                        slist_p->sin.saddr_len = sizeof(struct sockaddr_in6);
                                    }
                                    break;
                                }
                            }
                            slist_p = slist_p->next_p;
                        }
                        break;
                    }

                    if(DNS_ANSWER_PART == type)
                    {
                        cache_p = DNS_RESOLVER_DNS_CACHERR_ENTRY_MALLOC();
                        if(NULL != cache_p)
                        {
                            memset(cache_p, 0 ,sizeof(DNS_CacheRR_T));

                            DNS_RESOLVER_Rhn2Str(name,(UI8_T*)cache_p->name);
                            memcpy(&cache_p->ip, &pip, sizeof(pip));
                            cache_p->type = L_STDLIB_Ntoh16(rhdr.rtype);
                            cache_p->ttl = L_STDLIB_Ntoh32(rhdr.rttl);
                            cache_p->arrived_time = SYSFUN_GetSysTick();
                            cache_p->next_p = send_p->rrlist_p;
                            send_p->rrlist_p = cache_p;
                        }

                        cache_p = send_p->rrlist_p;
                        while(NULL != cache_p)
                        {
                            if(0 == cache_p->ip.addrlen)
                            {
                                memcpy(&cache_p->ip, &pip, sizeof(pip));
                    }
                            else
                            {
                                cache_p = cache_p->next_p;
                    }
                    }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_Task
 *
 * PURPOSE:
 *      dns resolver task
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_RESOLVER_Task(void)
{
    DNS_Send_T* send_p = gdns_send_head;
    DNS_Send_T* del_p;
    int pc = 0;                     /* current parallel query count */
    fd_set              reads;
    struct timeval      tv;
    int                 maxfd = 0;
    UI32_T      ticks;
    int                 srv;
    int                 rc;
    int ra;
    /*isiah*/
    DNS_Slist_T *del_slist_p = NULL;
    int                 res_reset_status;
    UI32_T              i;

    if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
    {
        printf("\nDNS_ResolverTask start to work... \n");
    }

    while( (SYS_TYPE_STACKING_MASTER_MODE == DNS_MGR_GetOperationMode())/* && (DNS_ENABLE == DNS_MGR_GetDnsStatus())*/ )
    {
/*isiah.2003-10-30*/
        DNS_MGR_GetDnsResConfigReset(&res_reset_status);

        if( res_reset_status == VAL_dnsResConfigReset_reset )
        {
            DNS_MGR_SetResResetStatus(VAL_dnsResConfigReset_initializing);

            while(NULL != send_p)
            {
                del_p = send_p;
                send_p = send_p->next_p;
                DNS_RESOLVER_SendFree(del_p);
            }

            gdns_send_head = NULL;

            for( i=0 ; i<DNS_HOSTENT_NUM ; i++ )
            {
                DNS_RESOLVER_HostentFree(gdns_hp[i]);
            }

            DNS_MGR_DnsResetTimeInit();
            send_p = NULL;
            gdns_local_request = 0;
            gdns_hook_count = 0;
            DNS_MGR_SetResResetStatus(VAL_dnsResConfigReset_running);
        }

        pc = 0;
        FD_ZERO(&reads);

        while(NULL != send_p)
        {
            ticks = SYSFUN_GetSysTick();

            if( pc > gdns_max_pc)
                break;

            if(DNS_QS_FREE == send_p->state)
            {
                del_p = send_p;
                send_p = gdns_send_head;

                /* send_p: check for NULL (pgr0689 below)
                 */
                if (send_p == NULL)
                {
                    break;
                }

                if(gdns_send_head == del_p)
                {
                    gdns_send_head = del_p->next_p;
                    DNS_RESOLVER_SendFree(del_p);
                    send_p = gdns_send_head;
                    del_p = 0;

                    continue;
                }

                while(send_p->next_p != del_p)  /* pgr0689, send_p: check for NULL (above) */
                {
                    send_p = send_p->next_p;
                }
                send_p->next_p = del_p->next_p;

                DNS_RESOLVER_SendFree(del_p);
                del_p = NULL;

                continue;
            }

            /*timeout*/
            if(send_p->timeout < ticks)
            {
                /*isiah*/
                /* send_p->slist: check for NULL (pgr0699 below)
                 */
                if ( (send_p->slist != NULL) && (send_p->slist->next_p != NULL) )
                {
                    del_slist_p = send_p->slist;
                    send_p->slist = send_p->slist->next_p;
                    DNS_RESOLVER_DNS_SLIST_ENTRY_FREE(del_slist_p);
                    del_slist_p = NULL;
                    s_close(send_p->sock);
                    send_p->addr = (struct sockaddr*)&(send_p->slist->sin);  /* pgr0699, send_p->slist: check for NULL (above) */
                    send_p->sinl = sizeof(struct sockaddr);
                    send_p->state = DNS_QS_UDPINITIAL;
                    send_p->event = DNS_QEV_WRITE;
                    send_p->timeout = DNS_Ttl2Ticks(gdns_remote_timeout);
                    DNS_MGR_ResOptCounterInc(LEAF_dnsResOptCounterRetrans);
                    send_p = send_p->next_p;
                    continue;
                }
                /*end add--------------*/

                if(NULL == send_p->callback)
                {
                    DNS_MGR_ResOptCounterInc(LEAF_dnsResOptCounterInternalTimeOuts);

                    SYSFUN_ResumeThread(send_p->inter_id);
                }
                else
                {
                    if(NULL != send_p->sendbuf)
                    {
                        DNS_RESOLVER_SENDBUF_ENTRY_FREE(send_p->sendbuf);
                    }

                    send_p->sendbuf =
                        DNS_RESOLVER_ErrorMake(DNS_RC_SERVFAIL,send_p->rid,send_p->opcode);

                    if(NULL != send_p->sendbuf)
                    {
                        send_p->callback(send_p->inter_id,send_p->sendbuf,12);
                    }
                    else
                    {
                        send_p->callback(send_p->inter_id,NULL,0);
                    }
                }
                send_p->state = DNS_QS_FREE;
                continue;

            }

            if( DNS_QS_DONE == send_p->state )
            {
                send_p = send_p->next_p;
                continue;
            }

            switch(send_p->event)
            {
                case DNS_QEV_WRITE:
                    rc = DNS_RESOLVER_RxTx(send_p);
                    break;

                case DNS_QEV_READ:
                    FD_SET(send_p->sock,&reads);
                    break;

                case DNS_QEV_READWRITE:
                    send_p->state = DNS_QS_UDPINITIAL;
                    rc = DNS_RESOLVER_RxTx(send_p);
                    FD_SET(send_p->sock,&reads);
                    break;

            }

            if (maxfd < send_p->sock)
            {
                maxfd = send_p->sock;
            }

            pc++;
            send_p = send_p->next_p;
        }

        if( ZERO == pc )
        {
            send_p = gdns_send_head;
            SYSFUN_Sleep(10);       /* 20021014  10->0*/
            continue;
        }

        tv.tv_sec  = 0;
        tv.tv_usec = gdns_retransmit;

        srv = select(maxfd+1,&reads,NULL,NULL,&tv);

        if(srv <= 0)
        {
            continue;
        }

        send_p = gdns_send_head;
        while(send_p && (pc > 0))
        {
            if(DNS_QS_DONE != send_p->state)
            {
                pc--;
                srv = 0;
                switch(send_p->event)
                {
                    case DNS_QEV_WRITE:
                        break;

                    case DNS_QEV_READ:
                        srv = FD_ISSET(send_p->sock,&reads);
                        break;

                    case DNS_QEV_READWRITE:
                        if(ZERO < (srv = FD_ISSET(send_p->sock,&reads)))
                        {
                            send_p->state = DNS_QS_UDPRECEIVE;
                        }
                        break;
                }

                if(srv)
                {
                    rc = DNS_RESOLVER_RxTx(send_p);

                    if(DNS_RC_OK == rc)
                    {
                        rc = DNS_RESOLVER_ResponseProcess(send_p,send_p->recvl);
                        if(DNS_OK != rc)
                        {
                            DNS_RESOLVER_Retransmit(send_p);
                        }
                        else if(NULL != send_p->callback)
                        {
                            if(DNS_QS_DONE == send_p->state || !(send_p->flags&DNS_REQ_RD))
                            {
                                send_p->recvbuf->id = send_p->rid;
                                DNS_MGR_GetDnsServConfigRecurs(&ra);
                                if( ra == 3 )
                                {
                                    send_p->recvbuf->ra = 0;
                                }
                                else
                                {
                                    send_p->recvbuf->ra = ra;
                                }

                                send_p->recvbuf->ra = ra;
                                send_p->recvbuf->rd = 1;
                                send_p->callback(send_p->inter_id,send_p->recvbuf,send_p->recvl);
                                send_p->state = DNS_QS_FREE;
                            }
                        }
                        else if (DNS_QS_DONE == send_p->state)
                        {
                            SYSFUN_ResumeThread(send_p->inter_id);
                        }
                    }
                    //break;
                }
            }
            send_p = send_p->next_p;
        }
        send_p = gdns_send_head;

    }
}

/*
 * FUNCTION NAME : DNS_RESOLVER_Init
 *
 * PURPOSE:
 *      initiate dns resolver,include cache
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_Init(void)
{
    int ret;
    int i;
    int res_reset_status;

    if(DNS_ENABLE == DNS_VM_GetDnsDebugStatus())
        printf("\n\t\tDNS_RESOLVER_Init.....\n");

    DNS_VM_GetDnsResConfigReset(&res_reset_status);
    if(DNS_RES_RESET_RUNNING == res_reset_status)
    {
        printf("DNS resolver is running...Please disable resolver at first.\n");
        return DNS_ERROR;
    }
    res_reset_status = DNS_RES_RESET_INITIAL;       /* initialing...*/
    DNS_VM_SetResResetStatus(res_reset_status);

/*isiah*/
/*  DNS_MGR_HostTblInit();*/

    gdns_ip_buf[4] = '\0';          /* for test */

/*isiah.*/
/*  gdns_resolver_sem = semBCreate(SEM_Q_PRIORITY,SEM_FULL);
    if(NULL == gdns_resolver_sem)
        return DNS_ERROR;   */

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_DNS_RESOLVER, &gdns_resolver_sem) != SYSFUN_OK)
    {
        printf("\n%s:get resolver sem id fail.\n", __FUNCTION__);
        return FALSE;
    }


    DNS_VM_GetDnsTimeOut(&gdns_remote_timeout);
    DNS_VM_DnsUptimeInit();
    DNS_VM_DnsResetTimeInit();
    DNS_VM_GetDnsServConfigRecurs(&gdns_sbelt_recursive);
    gdns_local_request = 0;
    gdns_hook_count = 0;

    for(i=0;i<DNS_HOSTENT_NUM;i++)
        gdns_hp[i] = NULL;          /* initiate global hp pointer */


    ret = DNS_CACHE_Init(); /* till now, assume no error */
    if(DNS_ERROR == ret)/* && DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())*/
    {
        /*isiah*/
        if ( DNS_ENABLE == DNS_VM_GetDnsDebugStatus() )
        {
            printf("\nDNS cache init failed.\n");
        }
        return DNS_ERROR;
    }

/*isiah.*/
/*  ret = DNS_RESOLVER_SbeltInit();*/
    if(DNS_ERROR == ret)/* && DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())*/
    {
        /*isiah*/
        if ( DNS_ENABLE == DNS_VM_GetDnsDebugStatus() )
        {
            printf("\nDNS sbelt init failed.\n");
        }
        return DNS_ERROR;
    }

    DNS_RESOLVER_Srand();

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_Disable
 *
 * PURPOSE:
 *      disable dns resolver
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_RESOLVER_Disable(void)
{
    struct DNS_Send_S* send_p = gdns_send_head;
    struct DNS_Send_S* send_del_p;
    struct DNS_Sbelt_S* sbelt_p = gdns_sbelt_head;
    struct DNS_Sbelt_S* del_p;
    int i;

/*  DNS_MGR_GetDnsLocalMaxRequests(&gdns_local_request);
    DNS_MGR_GetDnsServConfigMaxRequests(&gdns_hook_count);*/

/*isiah.*/
/*  semTake(gdns_resolver_sem,WAIT_FOREVER);*/
/*    SYSFUN_GetSem(gdns_resolver_sem, SYSFUN_TIMEOUT_WAIT_FOREVER);*/

/*  DNS_MGR_SetDnsStatus(DNS_DISABLE);*/

/*move to DNS_TASK_ProxyMain() */
/*  DNS_PROXY_Stop();*/

    while(NULL != send_p)
    {
        send_del_p = send_p;
        send_p = send_p->next_p;
        DNS_RESOLVER_SendFree(send_del_p);
    }
    gdns_send_head = NULL;/*isiah*/

    while(NULL != sbelt_p)
    {
        del_p = sbelt_p;
        sbelt_p = sbelt_p->next_p;
        DNS_RESOLVER_DNS_SBELT_ENTRY_FREE(del_p);
        del_p = NULL;
    }
    gdns_sbelt_head = NULL;/*isiah.*/

    DNS_CACHE_Init();

    for(i=0;i<DNS_HOSTENT_NUM;i++)
        DNS_RESOLVER_HostentFree(gdns_hp[i]);

/*isiah.*/
/*  taskDelete(gdns_resolver_id);

    semFlush(gdns_resolver_sem);
    semDelete(gdns_resolver_sem);*/
/*    SYSFUN_DeleteTask(gdns_resolver_id);
    SYSFUN_FlushSem(gdns_resolver_sem);
    SYSFUN_DestroySem(gdns_resolver_sem);*/

}

/*
 * FUNCTION NAME : DNS_RESOLVER_Entry
 *
 * PURPOSE:
 *      dns resolver entry(non local hosts)
 *
 * INPUT:
 *      int inter_id -- taskid who send dns request
 *      UI8_T* name -- query name
 *      UI8_T* pip -- query ip address,inverse query
 *      UI16_T qtype-- query type
 *      UI8_T* data-- received dns request pkt
 *      FUNCPTR -- callback function
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_Entry(int inter_id,UI8_T* name, L_INET_AddrIp_T *pip_p, UI16_T qtype,UI8_T* data, FUNCPTR routine)
{
    DNS_Hdr_T*          hdr = (DNS_Hdr_T*)data;
    DNS_CacheRR_T*  cache_p= NULL;
    DNS_Send_T*         send_p = NULL;
    DNS_Hdr_T*          rhdr = NULL;  /* retrun dns hdr */
    char       buf[256];
    int length,ret;
    int cache_status;

    send_p = DNS_RESOLVER_DNS_SEND_ENTRY_MALLOC();

    if(NULL == send_p)
    {
        return DNS_ERROR;
    }

    memset(send_p,0,sizeof(DNS_Send_T));

    if(DNS_OP_QUERY == hdr->opcode)
    {
        if(DNS_RRT_PTR == qtype)
        {
            ret = DNS_RESOLVER_SendHook(send_p, (I8_T *)name,DNS_RRT_PTR,
                                DNS_OP_QUERY,inter_id,hdr->id,NULL);
            if(ret != DNS_OK)
            {
                DNS_RESOLVER_SendFree(send_p);      /* not free,since maybe malloc for slist */
                return DNS_ERROR;
            }
            send_p->callback = routine;
            if (1 == hdr->rd)
            {
                send_p->flags |= DNS_REQ_RD;
            }
            if (1 == hdr->ra)
            {
                send_p->flags |= DNS_REQ_RA;
            }

            return DNS_OK;
        }

        DNS_MGR_GetDnsResCacheStatus(&cache_status);

        if(VAL_dnsResCacheStatus_enabled == cache_status)
        {
            cache_p = DNS_CACHE_GetRR((char *)name);
        }
        else
        {
            cache_p = NULL;
        }

        /* returned cache_p maybe a most matched,other than himself */
        if(NULL != cache_p && ZERO == strcmp(cache_p->name, (char *)name))
        {
            if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
            {
                printf("\nFind answer in cache for %s.\n",name);

            }
            gdns_cache_hit++;

            DNS_RESOLVER_DNS_SEND_ENTRY_FREE(send_p);
            send_p = NULL;
            if(32 != cache_p->flag && 4 != cache_p->flag)
                printf("\nError cache flag.\n");

            if(cache_p->flag & DNS_CF_NEGATIVE) /* no ttl op */
            {
                DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterSelfNonAuthDatas);

                ret = DNS_RESOLVER_ErrorReply(hdr->id,hdr->opcode,DNS_RC_NAMEERR,
                                    inter_id,routine);
                DNS_CACHE_FreeRR(cache_p);
                return ret;
            }

            length = DNS_RESOLVER_CachedReply(cache_p,&rhdr);
            if(DNS_ERROR == length)
            {
                DNS_CACHE_FreeRR(cache_p);
                return DNS_ERROR;
            }

            rhdr->rd = hdr->rd;
            rhdr->id = hdr->id;         /* must be here,because not pass in */
            (*routine)(inter_id,(const I8_T*)rhdr,length);
            L_MM_Free(rhdr);                   /* malloced in DNS_cache_reply */
            rhdr = NULL;
        }
        else
        {
            if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
            {
                if(NULL != cache_p)
                {
                    gdns_cache_dad++;
                }
            }

            /* find his paraent or not,anyway .... */
            ret = DNS_RESOLVER_SendHook(send_p, (I8_T *)name,qtype,hdr->opcode,
                                        inter_id,hdr->id,cache_p);
            if(DNS_OK != ret)
            {
                DNS_RESOLVER_SendFree(send_p);
                DNS_CACHE_FreeRR(cache_p);
                return DNS_ERROR;
            }

            if (1 == hdr->rd)
            {
                send_p->flags |= DNS_REQ_RD;
            }
            if (1 == hdr->ra)
            {
                send_p->flags |= DNS_REQ_RA;
            }

            send_p->callback = routine;
        }
    }
    else if ( DNS_OP_IQUERY == hdr->opcode)
    {
        if( L_INET_ADDR_TYPE_IPV4 == pip_p->type)
        {
            sprintf(buf, "%u.%u.%u.%u.in-addr.arpa",
                                pip_p->addr[3],
                                pip_p->addr[2],
                                pip_p->addr[1],
                                pip_p->addr[0]);
        }
#if (SYS_CPNT_IPV6 == TRUE)
        else if((L_INET_ADDR_TYPE_IPV6 == pip_p->type) || (L_INET_ADDR_TYPE_IPV6Z == pip_p->type))
        {
            sprintf(buf,"%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x."
                         "%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.ip6.arpa",
                         pip_p->addr[15]&0x0f, (pip_p->addr[15]>>4)&0x0f,
                         pip_p->addr[14]&0x0f, (pip_p->addr[14]>>4)&0x0f,
                         pip_p->addr[13]&0x0f, (pip_p->addr[13]>>4)&0x0f,
                         pip_p->addr[12]&0x0f, (pip_p->addr[12]>>4)&0x0f,
                         pip_p->addr[11]&0x0f, (pip_p->addr[11]>>4)&0x0f,
                         pip_p->addr[10]&0x0f, (pip_p->addr[10]>>4)&0x0f,
                         pip_p->addr[9]&0x0f,  (pip_p->addr[9]>>4)&0x0f,
                         pip_p->addr[8]&0x0f,  (pip_p->addr[8]>>4)&0x0f,
                         pip_p->addr[7]&0x0f,  (pip_p->addr[7]>>4)&0x0f,
                         pip_p->addr[6]&0x0f,  (pip_p->addr[6]>>4)&0x0f,
                         pip_p->addr[5]&0x0f,  (pip_p->addr[5]>>4)&0x0f,
                         pip_p->addr[4]&0x0f,  (pip_p->addr[4]>>4)&0x0f,
                         pip_p->addr[3]&0x0f,  (pip_p->addr[3]>>4)&0x0f,
                         pip_p->addr[2]&0x0f,  (pip_p->addr[2]>>4)&0x0f,
                         pip_p->addr[1]&0x0f,  (pip_p->addr[1]>>4)&0x0f,
                         pip_p->addr[0]&0x0f, (pip_p->addr[0]>>4)&0x0f);
        }
#endif

        ret = DNS_RESOLVER_SendHook(send_p, (I8_T *)buf,DNS_RRT_PTR,DNS_OP_IQUERY,inter_id,hdr->id,NULL);
        if(DNS_OK != ret)
        {
            DNS_RESOLVER_SendFree(send_p);
            return DNS_ERROR;
        }

        if (1 == hdr->rd)
        {
            send_p->flags |= DNS_REQ_RD;
        }
        if (1 == hdr->ra)
        {
            send_p->flags |= DNS_REQ_RA;
        }

        send_p->opcode = DNS_OP_IQUERY;
        send_p->callback = routine;
    }

    DNS_CACHE_FreeRR(cache_p);
    return DNS_OK;
}


/* get IP ddress from ip6.arpa domain name string
 * note: suffix of input string must be ipv6.arpa.
 */

static int DNS_RESOLVER_GetIpFromIpv6Arpa(UI8_T *name_string, UI8_T *ip)
{
    enum
    {
        IPV6_ADD_STR_LEN = 39,
        VALID_IN_STR_LEN = sizeof("b.a.9.8.7.6.5.0.4.0.0.0.3.0.0.0.2.0.0.0.1.0.0.0.0.0.0.0.1.2.3.4.ip6.arpa")-1,
        IPV6_ARPA_SUFFIX_LEN = sizeof("ip6.arpa")-1,
        IPV6_STRING_LEN_WITHOUT_DOT = SYS_ADPT_IPV6_ADDR_LEN*2
    };

    int i;
    UI8_T *from;
    UI8_T *to = ip;

    if(strlen ((char *) name_string) != VALID_IN_STR_LEN)
    {
        return DNS_ERROR;
    }

    from= name_string + strlen((char*) name_string) - IPV6_ARPA_SUFFIX_LEN - 2;

    for(i=0 ; i<IPV6_STRING_LEN_WITHOUT_DOT;i++)
    {
        if(isxdigit((int)*name_string) == 0)
        {
            return DNS_ERROR;
        }

        *to++ = *from;

        if(i != (IPV6_STRING_LEN_WITHOUT_DOT-1))
        {
            if ((i&0x3) == 0x3)
            {
                *to++ = ':';
            }

            from--;

            if((name_string!= from) && (*from!='.'))
            {
                return DNS_ERROR;
            }
            else
            {
                from -= 1;
            }
        }
    }

    *to = '\0';

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_Process
 *
 * PURPOSE:
 *      interface to dns proxy
 *
 * INPUT:
 *      int --
 *      UI8_T* -- proxy received data
 *      UI32_T -- full request pkt length
 *      UI8_T -- request transmit protocol type
 *      FUNCPTR -- callback function
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_Process(int inter_id,
                             UI8_T* data,
                             UI32_T rlen,
                             FUNCPTR routine)
{
    L_INET_AddrIp_T iip_ar[MAXHOSTIPNUM];
    DNS_Hdr_T*   hdr = (DNS_Hdr_T*)data;
    DNS_Hdr_T*   rhdr = NULL;  /* retrun dns hdr */
    UI8_T* prr = (UI8_T*)(hdr+1);
    UI8_T* pch;
    UI8_T* addr = NULL;
    UI32_T  temp_pip = 0;
    UI32_T pip1 = 0;
    char  buf[256];
    char  qrhn[256];
    int remain = rlen;
    int qrhnlen;
    int underscore;
    int length ;
    int ret;
    UI16_T qtype;
    int queryorder;
    int ra;
    UI8_T ip[L_INET_MAX_IPADDR_STR_LEN+1]={0}; /*ip string*/
    L_INET_AddrIp_T pip;


    DNS_MGR_ResCounterByOpcodeInc(DNS_QR_QUERY,hdr->opcode);

    if(DNS_QR_RESP == hdr->qr)
    {
        if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
            printf("\nReceived pkt from proxy not a request.\n");

        return DNS_ERROR;
    }

    /* this 3 bit must be zero */
    if( ZERO != hdr->z              ||
        rlen < sizeof(DNS_Hdr_T)    ||
        DNS_RC_OK != hdr->rcode)
    {
        DNS_MGR_ServCounterInc(LEAF_dnsServCounterErrors);

        ret = DNS_RESOLVER_ErrorReply(hdr->id,hdr->opcode,
                            DNS_RC_FORMAT,inter_id,routine);
        return ret;
    }

    /* chance of this occur is tiny,but if that,we can cut malloc op */
     if(L_STDLIB_Ntoh16(hdr->qdcount) > 1
     || (DNS_OP_QUERY != hdr->opcode
        && DNS_OP_IQUERY != hdr->opcode))
    {
        DNS_MGR_ServCounterInc(LEAF_dnsServCounterErrors);

        ret = DNS_RESOLVER_ErrorReply(hdr->id,hdr->opcode,
                            DNS_RC_NOTSUPP,inter_id,routine);
        return ret;
    }

    ret = DNS_RESOLVER_Decompress(data,(UI8_T*)qrhn,&prr,&remain,&qrhnlen,rlen,&underscore);
    if(DNS_RC_OK != ret                         ||
      (4 > remain && L_STDLIB_Ntoh16(hdr->qdcount) == 1)  ||/* not std inverse query,4 is type + class*/
      (12> remain && L_STDLIB_Ntoh16(hdr->ancount) == 1))  /* not un-recommend inverse query,12 = 10 + 2 */
    {
        DNS_MGR_ServCounterInc(LEAF_dnsServCounterErrors);
        DNS_MGR_ServCounterInc(LEAF_dnsServCounterReqUnparses);

        ret = DNS_RESOLVER_ErrorReply(hdr->id,hdr->opcode,ret,inter_id,routine);
        return ret;
    }

    /* prr point to RR hdr */

    DNS_RESOLVER_Rhn2Str((UI8_T*)qrhn,(UI8_T*)buf);     /* after decompress,prr pointer rr hdr */

    pch = (UI8_T*)buf;
    while('.' != *pch && '\0' != *pch)
    {
        pch++;
    }

    if('\0' == *pch)
    {
        DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterFriendsRelNames);
    }

    if(DNS_OP_QUERY == hdr->opcode)
    {
        if(1 != L_STDLIB_Ntoh16(hdr->qdcount))
        {
            DNS_MGR_ServCounterInc(LEAF_dnsServCounterErrors);

            ret = DNS_RESOLVER_ErrorReply(hdr->id,hdr->opcode,
                    DNS_RC_NOTSUPP,inter_id,routine);
            return ret;
        }

        memcpy(&qtype,prr,2);
        qtype = L_STDLIB_Ntoh16(qtype);

        /* extract IP address from
         * in-addr.arpa and ip6.arpa domain
         */
        if(DNS_RRT_PTR==qtype)
        {
            if (DNS_MGR_GetDnsDebugStatus() == DNS_ENABLE)
            {
                printf("Receive PTR query: %s, ", buf);
            }

            /* IPv6 PTR
             * ex. b.a.9.8.7.6.5.0.4.0.0.0.3.0.0.0.2.0.0.0.1.0.0.0.0.0.0.0.1.2.3.4.ip6.arpa
             * extract to 4321:0000:0001:0002:0003:0004:0567:89ab
             */
            if ( 8 < strlen(buf) && 0 == strcmp(buf+strlen(buf)-8, "ip6.arpa"))
            {
                if( DNS_ERROR == DNS_RESOLVER_GetIpFromIpv6Arpa((UI8_T*)buf, ip))
                {
                    if (DNS_MGR_GetDnsDebugStatus() == DNS_ENABLE)
                    {
                        printf("Get ipv6 address from ip6.arpa fail");
                    }
                }

                addr=ip;
            }
            /* IPv4 PTR
             * ex.52.0.0.10.in-addr.arpa
             * extract to 10.0.0.52
             */
            else if (12 < strlen(buf) && 0 == strcmp(buf+strlen(buf)-12, "in-addr.arpa"))
            {
                temp_pip = DNS_RESOLVER_Ptr2Ip((UI8_T*)buf);

                pip1 = ( (((unsigned long)(temp_pip) & 0x000000FF)<<24) |
                         (((unsigned long)(temp_pip) & 0x0000FF00)<<8)  |
                         (((unsigned long)(temp_pip) & 0x00FF0000)>>8)  |
                         (((unsigned long)(temp_pip) & 0xFF000000)>>24));

                addr = L_INET_Ntoa(pip1,ip);
            }
            else
            {
                DNS_MGR_ServCounterInc(LEAF_dnsServCounterQType);
                ret = DNS_RESOLVER_ErrorReply(hdr->id,hdr->opcode,DNS_RC_NOTSUPP,inter_id,routine);
                return ret;
            }
        }
    }
    else if(DNS_OP_IQUERY == hdr->opcode)
    {
        if( 1 != L_STDLIB_Ntoh16(hdr->ancount) || ZERO != L_STDLIB_Ntoh16(hdr->qdcount))
        {
            DNS_MGR_ServCounterInc(LEAF_dnsServCounterErrors);

            ret = DNS_RESOLVER_ErrorReply(hdr->id,hdr->opcode,
                                DNS_RC_FORMAT,inter_id,routine);
            return ret;
        }

        memcpy(&qtype,prr,2);
        qtype = L_STDLIB_Ntoh16(qtype);

        memset(&pip, 0, sizeof(pip));

        if(DNS_RRT_A == qtype)
        {
            memcpy(pip.addr,prr+10,4);

            if(L_INET_Ntop(L_INET_AF_INET, (void*)pip.addr, (char*)ip, sizeof(ip)) == FALSE)
            {
                printf("Convert ipv4 address into a character string fail");
            }
        }
#if (SYS_CPNT_IPV6 == TRUE)
        else if(DNS_RRT_AAAA == qtype)
        {
            memcpy(pip.addr,prr+10,16);

            if(L_INET_Ntop(L_INET_AF_INET6, (void*)pip.addr, (char*)ip, sizeof(ip)) == FALSE)
            {
                printf("Convert ipv6 address into a character string fail");
            }

            addr=ip;
        }
#endif
        else
        {
            DNS_MGR_ServCounterInc(LEAF_dnsServCounterQType);
            ret = DNS_RESOLVER_ErrorReply(hdr->id,hdr->opcode,DNS_RC_NOTSUPP,inter_id,routine);
            return ret;
        }

        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                           (char *)addr,
                                                           (L_INET_Addr_T *)&pip,
                                                           sizeof(pip)))
        {
            return DNS_ERROR;
        }
    }
    else
    {
        DNS_MGR_ServCounterInc(LEAF_dnsServCounterErrors);

        ret = DNS_RESOLVER_ErrorReply(hdr->id,hdr->opcode,
                            DNS_RC_NOTSUPP,inter_id,routine);
        return ret;
    }

    DNS_MGR_GetDnsResConfigQueryOrder(&queryorder);

    if(DNS_QUERY_DNS_FIRST == queryorder)
    {
        ret = DNS_RESOLVER_Entry(inter_id,(UI8_T*)buf,&pip,qtype,data,routine);
        if(DNS_OK != ret)
        {
            if(( DNS_RRT_A == qtype || DNS_RRT_AAAA==qtype )&& DNS_OP_QUERY == hdr->opcode)
            {
                ret = DNS_MGR_HostGetByName(buf,iip_ar);       /* or hostGetByAddr */
            }
            else
            {
                ret = DNS_MGR_HostGetByAddr((I8_T *)addr, (I8_T *)buf);
            }

            if(DNS_ERROR != ret)
            {
                if((DNS_RRT_A != qtype) && (DNS_RRT_AAAA != qtype))
                {
                    memset(iip_ar,0,sizeof(iip_ar));

                    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                                       (char *)addr,
                                                                       (L_INET_Addr_T *)&iip_ar[0],
                                                                       sizeof(iip_ar[0])))
                    {
                        return DNS_ERROR;
                    }
                }

                length = DNS_RESOLVER_LocalReply(&rhdr,qtype,hdr->opcode,(I8_T*)buf,iip_ar);

                if(DNS_ERROR != length)
                {
                    rhdr->id = hdr->id;     /* must be here,because not pass in */
                    rhdr->rd = hdr->rd;

                    (*routine)(inter_id,(const I8_T*)rhdr,length);
                    L_MM_Free(rhdr);
                    rhdr = NULL;
                    return DNS_OK;
                }
            }
        }

        return ret;
    }
    else if(DNS_QUERY_LOCAL_FIRST == queryorder)
    {
        if(( DNS_RRT_A == qtype || DNS_RRT_AAAA==qtype )&& DNS_OP_QUERY == hdr->opcode)
        {
            ret = DNS_MGR_HostGetByName((char *)buf,iip_ar);
        }
        else if(DNS_RRT_PTR == qtype)
        {
            ret = DNS_MGR_HostGetByAddr((I8_T *)addr, (I8_T *)buf);
        }

        if(DNS_OK != ret)
        {
            ret = DNS_RESOLVER_Entry(inter_id,(UI8_T *)buf,&pip,qtype,data,routine);
        }
        else
        {
            if((DNS_RRT_A != qtype) && (DNS_RRT_AAAA != qtype))
            {
                memset(iip_ar,0,sizeof(iip_ar));

                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                                   (char *)addr,
                                                                   (L_INET_Addr_T *)&iip_ar[0],
                                                                   sizeof(iip_ar[0])))
                {
                    return DNS_ERROR;
                }
            }


            length = DNS_RESOLVER_LocalReply(&rhdr,qtype,hdr->opcode,(I8_T *)buf,iip_ar);

            if(DNS_ERROR != length)
            {
                rhdr->id = hdr->id;         /* must be here,because not pass in */
                DNS_MGR_GetDnsServConfigRecurs(&ra);

                /*isiah.2003-03-03*/
                if( ra == 3 )
                {
                    rhdr->ra = 0;
                }
                else
                {
                    rhdr->ra = ra;
                }

                (*routine)(inter_id,(const I8_T*)rhdr,length);
                L_MM_Free(rhdr);
                rhdr = NULL;
                return DNS_OK;
            }
            else
            {
                printf("LocalReply fail\n");
            }
        }

        return ret;
    }
    else
    {
        return DNS_RESOLVER_Entry(inter_id,(UI8_T *)buf,&pip,qtype,data,routine);
    }
}

/*
 * FUNCTION NAME : DNS_RESOLVER_CachedAnswer
 *
 * PURPOSE:
 *      answer local request
 *
 * INPUT:
 *      DNS_CacheRR_T* -- cache list
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      struct hostent* hp
 *
 * NOTES:
 *      none
 *
 */
#if 0
struct hostent* DNS_RESOLVER_CachedAnswer(DNS_CacheRR_T* cache_p)
{
    DNS_CacheRR_T* temp_p = cache_p;
    struct hostent* hp = NULL;
    int num = 0;

    if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        printf("Find answer in cache.  \n");

    while(NULL != temp_p && num < DNS_CNAME_NUM)
    {
        if(DNS_RRT_A == temp_p->type)   /* ?? OK ?? */
        {
            DNS_RESOLVER_HostentAdd(&hp,DNS_HOSTENT_CNAME, (I8_T *)temp_p->name,0);
        }
        else    /* alias */
        {
            DNS_RESOLVER_HostentAdd(&hp,DNS_HOSTENT_ALIAS, (I8_T *)temp_p->name,0);
        }

        if(0 != temp_p->ip)
            DNS_RESOLVER_HostentAdd(&hp,DNS_HOSTENT_IP,NULL,temp_p->ip);

        temp_p = temp_p->next_p;
    }

    if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        DNS_RESOLVER_DebugShowHostent(hp);

    return hp;
}
#endif
/*
 * FUNCTION NAME : DNS_RESOLVER_CachedReply
 *
 * PURPOSE:
 *      reply to remote request from cache data
 *
 * INPUT:
 *      DNS_CacheRR_T* cache_p -- cache list
 *
 * OUTPUT:
 *      DNS_Hdr_T** rhdr -- dns pkt header
 *
 * RETURN:
 *      int ret -- length of response pkt
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_CachedReply(DNS_CacheRR_T* cache_p,DNS_Hdr_T** rhdr)
{
    DNS_CacheRR_T* temp_p = cache_p;
    DNS_RR_T rr;
    darray ad = NULL;
    DNS_Hdr_T* hdr;
    I8_T buf[256];
    int offs=0;
    int ret = 0;
    UI8_T* prr;
    UI16_T val;
    UI16_T magic;
    UI8_T* prem;
    int ra;

    /* check for NULL
     */
    if (temp_p == NULL)
    {
        return DNS_ERROR;
    }

    hdr = (DNS_Hdr_T *)L_MM_Malloc(512, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_CACHEDREPLY));
    if(NULL == hdr)
    {
        DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
        DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterFriendsOtherErrors);

        return DNS_ERROR;
    }

    memset(hdr,0,512);

    /*header section*/
    hdr->rd = 1;
    DNS_MGR_GetDnsServConfigRecurs(&ra);

    /*isiah.2003-03-03*/
    if( ra == 3 )
    {
        hdr->ra = 0;
    }
    else
    {
        hdr->ra = ra;
    }

    hdr->tc = 0;
    hdr->aa = 0;
    hdr->qr = DNS_QR_RESP;
    hdr->opcode = DNS_OP_QUERY;

    /* query part */
    prr = (UI8_T*)(hdr+1);
    DNS_RESOLVER_Str2Rhn((UI8_T *)temp_p->name, (UI8_T *)buf);
    offs = 12;
    ret = DNS_RESOLVER_Compress((UI8_T *)buf,prr,offs,&ad);
    offs += ret + 4;    /* r for type & class */

    prr += strlen(temp_p->name)+2;
    val = DNS_RRT_A;//Fix bug for second ping here.2004/11/9
    memcpy(prr,&val,2);     /* type */
    prr+=2;
    val = DNS_RRC_IN;
    memcpy(prr,&val,2);     /* class */
    prr += 2;

    hdr->qdcount = L_STDLIB_Hton16(1);
    hdr->ancount = 0;

    /*answer section*/
    while(NULL != temp_p)
    {
        if (offs > 498)
            return DNS_ERROR;

        hdr->ancount += 1;

        DNS_RESOLVER_Str2Rhn((UI8_T *)temp_p->name, (UI8_T *)buf);
        ret = DNS_RESOLVER_Compress((UI8_T *)buf,prr,offs,&ad);
        offs += ret;
        prr += ret;

        prem = prr;

        prr += 10;
        offs += 10;

        if(DNS_RRT_CNAME == temp_p->type)
        {
            DNS_RESOLVER_Str2Rhn((UI8_T *)temp_p->next_p->name, (UI8_T *)buf);
            ret = DNS_RESOLVER_Compress((UI8_T *)buf,prr,offs,&ad);
            offs += 10 + ret;
            prr += ret;
            rr.rdlength = L_STDLIB_Hton16(ret);
            rr.rtype = L_STDLIB_Hton16(DNS_RRT_CNAME);
        }
#if (SYS_CPNT_IPV6 == TRUE)
        else if(DNS_RRT_AAAA == temp_p->type)
        {
            memcpy(prr,&temp_p->ip.addr, 16);
            prr += 16;
            offs += 10 + 16;
            rr.rdlength = htons(temp_p->ip.addrlen);
            rr.rtype = htons(DNS_RRT_AAAA);
        }
#endif
        else
        {
            memcpy(prr,temp_p->ip.addr,4);
            prr += 4;
            offs += 10 + 4;
            rr.rdlength = L_STDLIB_Hton16(4);
            rr.rtype = L_STDLIB_Hton16(DNS_RRT_A);
        }

        rr.rclass = L_STDLIB_Hton16(DNS_RRC_IN);
        rr.rttl = L_STDLIB_Hton32(temp_p->ttl);
        memcpy(prem,&rr,10);

        temp_p = temp_p->next_p;

        //DNS_RESOLVER_DebugShowRR(show);
    }

    magic = hdr->ancount;
    magic = L_STDLIB_Hton16(magic);
    hdr->ancount = magic;

    if(NULL != ad)          /* 10012002 */
    {
        free(ad);
        ad = NULL;
    }

    *rhdr = hdr;
    ret = prr - (UI8_T*)hdr;

    return ret;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_LocalReply
 *
 * PURPOSE:
 *      reply remote request from local hosts information
 * INPUT:
 *      int -- query time
 *      I8_T* -- query name
 *      struct in_addr[]    -- ip address from local hosts file
 *
 * OUTPUT:
 *      DNS_Hdr_T** -- dns response pkt
 *
 * RETURN:
 *      int ret -- length of response pkt
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_LocalReply(DNS_Hdr_T** rhdr, int qtype, int op, I8_T* name, L_INET_AddrIp_T pip_ar[])
{
    DNS_RR_T rr;
    DNS_Hdr_T* hdr = NULL;
    I8_T buf1[256];
    I8_T buf[256];
    int ret = 0;
    UI8_T* prr;
    UI16_T magic;
    int i = 0;
    int an = 0;
    L_INET_AddrIp_T tip = pip_ar[0];
    int ra;


    hdr = (DNS_Hdr_T *)L_MM_Malloc(512, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_LOCALREPLY));

    if(NULL == hdr)
    {
        DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
        DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterFriendsOtherErrors);

        return DNS_ERROR;
    }

    memset(hdr,0,512);

    /*header section*/
    hdr->rd = 1;
    DNS_MGR_GetDnsServConfigRecurs(&ra);

    /*isiah.2003-03-03*/
    if( ra == 3 )
    {
        hdr->ra = 0;
    }
    else
    {
        hdr->ra = ra;
    }
    hdr->tc = 0;
    hdr->aa = 1;
    hdr->qr = DNS_QR_RESP;
    hdr->opcode = op;
    hdr->qdcount = L_STDLIB_Hton16(1);           /* htons(1) */
    hdr->ancount = 0;

    /* query section */
    prr = (UI8_T*)(hdr+1);

    if(DNS_RRT_PTR == qtype)
    {
        if( L_INET_ADDR_TYPE_IPV4 == tip.type)
        {
            sprintf((char*)buf1, "%u.%u.%u.%u.in-addr.arpa",
                                tip.addr[3],
                                tip.addr[2],
                                tip.addr[1],
                                tip.addr[0]);
        }
#if (SYS_CPNT_IPV6 == TRUE)
        else if(L_INET_ADDR_TYPE_IPV6 == tip.type || L_INET_ADDR_TYPE_IPV6Z == tip.type)
        {
            sprintf((char*)buf1,"%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x."
                         "%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.ip6.arpa",
                         tip.addr[15]&0x0f, (tip.addr[15]>>4)&0x0f,
                         tip.addr[14]&0x0f, (tip.addr[14]>>4)&0x0f,
                         tip.addr[13]&0x0f, (tip.addr[13]>>4)&0x0f,
                         tip.addr[12]&0x0f, (tip.addr[12]>>4)&0x0f,
                         tip.addr[11]&0x0f, (tip.addr[11]>>4)&0x0f,
                         tip.addr[10]&0x0f, (tip.addr[10]>>4)&0x0f,
                         tip.addr[9]&0x0f,  (tip.addr[9]>>4)&0x0f,
                         tip.addr[8]&0x0f,  (tip.addr[8]>>4)&0x0f,
                         tip.addr[7]&0x0f,  (tip.addr[7]>>4)&0x0f,
                         tip.addr[6]&0x0f,  (tip.addr[6]>>4)&0x0f,
                         tip.addr[5]&0x0f,  (tip.addr[5]>>4)&0x0f,
                         tip.addr[4]&0x0f,  (tip.addr[4]>>4)&0x0f,
                         tip.addr[3]&0x0f,  (tip.addr[3]>>4)&0x0f,
                         tip.addr[2]&0x0f,  (tip.addr[2]>>4)&0x0f,
                         tip.addr[1]&0x0f,  (tip.addr[1]>>4)&0x0f,
                         tip.addr[0]&0x0f, (tip.addr[0]>>4)&0x0f);
        }
#endif

        DNS_RESOLVER_Str2Rhn((UI8_T *)buf1, (UI8_T *)buf);
    }
    else
    {
        DNS_RESOLVER_Str2Rhn((UI8_T *)name, (UI8_T *)buf);
    }

    strcpy((char *)prr, (char *)buf);/*query name*/
    prr += strlen((char *)buf)+1;
    magic = L_STDLIB_Hton16(qtype);
    memcpy(prr,&magic,2);       /* type */

    prr+=2;
    magic = L_STDLIB_Hton16(DNS_RRC_IN);
    memcpy(prr,&magic,2);       /* class */

    prr += 2;

    /*answer section*/
    rr.rtype = L_STDLIB_Hton16(qtype);
    rr.rclass = L_STDLIB_Hton16(DNS_RRC_IN);
    rr.rttl = L_STDLIB_Hton32(DNS_HOST_TTL);

    if(DNS_RRT_PTR != qtype)
    {
        for(i=0;i<MAXHOSTIPNUM;i++)
        {
            if(DNS_RRT_A == qtype && L_INET_ADDR_TYPE_IPV4 == pip_ar[i].type && 0 != pip_ar[i].addrlen)
            {
                *prr = 192;
                *(prr+1) = 12;
                prr += 2;                   /* offset */

                rr.rdlength = L_STDLIB_Hton16(4);

                memcpy(prr,&rr,10);         /* hdr */
                prr += 10;

                memcpy(prr,&pip_ar[i].addr,4);    /* RDATA*/
                prr += 4;

                an++;
            }
#if (SYS_CPNT_IPV6 == TRUE)
            else if(DNS_RRT_AAAA == qtype &&
                   (L_INET_ADDR_TYPE_IPV6 == pip_ar[i].type ||L_INET_ADDR_TYPE_IPV6Z == pip_ar[i].type) &&
                    0 != pip_ar[i].addrlen)
            {
                *prr = 192;
                *(prr+1) = 12;
                prr += 2;                   /* offset */

                rr.rdlength = htons(16);

                memcpy(prr,&rr,10);         /* hdr */
                prr += 10;

                memcpy(prr, &pip_ar[i].addr, 16);   /* RDATA */
                prr += 16;

                an++;
            }
#endif
            else
            {
                continue;
            }
        }

    }
    else
    {
        rr.rdlength = L_STDLIB_Hton16(strlen((char *)name) + 2);
        an++;

        *prr = 192;                         /* domain */
        *(prr + 1) = 12;                    /* point to rr part */
        prr += 2;

        memcpy(prr,&rr,10);                 /* hdr */
        prr += 10;

        DNS_RESOLVER_Str2Rhn((UI8_T *)name, (UI8_T *)buf);             /* name */
        memcpy(prr,buf,strlen((char *)buf)+1);
        prr += strlen((char *)buf)+1;               /* for compute length */
    }

    an = L_STDLIB_Hton16(an);

    hdr->ancount = an;
    *rhdr = hdr;
    ret = prr - (UI8_T*)hdr;

    return ret;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_ErrorReply
 *
 * PURPOSE:
 *      make a dns reply with error code
 *
 * INPUT:
 *      UI16_T -- query id
 *      UI16_T -- query opcode
 *      UI16_T -- error code
 *      int -- callback function parameter
 *      FUNCPTR -- proxy callback function
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_ErrorReply(UI16_T id,
                    UI16_T opcode,
                    UI16_T rcode,
                    int inter_id,
                    FUNCPTR routine)
{
    struct DNS_Hdr_S* resp;
    int len = sizeof(DNS_Hdr_T);
    int ra;

    resp = (DNS_Hdr_T *)L_MM_Malloc(len, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_ERRORREPLY));
    if(NULL == resp)
    {
        DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
        DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterFriendsOtherErrors);

        return DNS_ERROR;
    }
    memset(resp,0,len);

    resp->id = id;
    resp->qr = DNS_QR_RESP;
    resp->opcode = opcode;
    resp->rcode = rcode;
    DNS_MGR_GetDnsServConfigRecurs(&ra);

    /*isiah.2003-03-03*/
    if( ra == 3 )
    {
        resp->ra = 0;
    }
    else
    {
        resp->ra = ra;
    }
/*
    resp->aa = 0;
    resp->tc = 0;
    resp->rd = 0;
    resp->z = 0;
    resp->qdcount = 0;
    resp->ancount = 0;
    resp->nscount = 0;
    resp->arcount = 0;
*/
    (*routine)(inter_id,(I8_T*)resp,len);
    L_MM_Free(resp);
    resp = NULL;

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_ErrorMake
 *
 * PURPOSE:
 *      make dns reply with error code
 *
 * INPUT:
 *      unsigned int -- error code
 *      unsigned int -- query id
 *      unsigned int -- query opcode
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      pointer to reply pkt or NULL
 *
 * NOTES:
 *      NOT SEND HERE
 *
 */
DNS_Hdr_T* DNS_RESOLVER_ErrorMake(unsigned int rcode,unsigned int id,unsigned int op)
{
    DNS_Hdr_T* hdr;
    int ra;

    hdr = (DNS_Hdr_T *)L_MM_Malloc(sizeof(DNS_Hdr_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RESOLVER_ERRORMAKE));
    if(NULL == hdr)
        return NULL;
    memset(hdr,0,sizeof(DNS_Hdr_T));    /* for cut op */

    hdr->id = id;
    hdr->qr = DNS_QR_RESP;
    hdr->opcode = op;
    hdr->rcode = rcode;
    DNS_MGR_GetDnsServConfigRecurs(&ra);

    /*isiah.2003-03-03*/
    if( ra == 3 )
    {
        hdr->ra = 0;
    }
    else
    {
        hdr->ra = ra;
    }
    /* since we have memset it,following code are saved. */
/*
    hdr->rd = 0;
    hdr->ra = 0;
    hdr->tc = 0;
    hdr->aa = 0;
    hdr->z = 0;
    hdr->qdcount = 0;
    hdr->ancount = 0;
    hdr->nscount = 0;
    hdr->arcount = 0;
*/
    return hdr;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_HostentAdd
 *
 * PURPOSE:
 *      add 1 item to hostent*
 * INPUT:
 *      int --
 *      const I8_T* --
 *      UI32_T --
 *
 * OUTPUT:
 *      struct hostent** -- constructed hostent
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_HostentAdd(struct hostent** php,
                        int type,
                        const I8_T* name,
                        L_INET_AddrIp_T *ip_p)
{
    struct hostent* hp = NULL;
    int i;

    if(NULL == *php)
    {
        *php = DNS_RESOLVER_HOST_ENT_ENTRY_MALLOC();
        if(NULL == *php)
        {
            DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
            DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterSelfOtherErrors);

            return DNS_ERROR;
        }
        memset(*php,0,sizeof(struct hostent));
    }

    hp = *php;
    switch(type)
    {
        case DNS_HOSTENT_IP:
            if(0 == ip_p->addrlen)
                break;

            if(NULL == hp->h_addr_list)
            {
                hp->h_addr_list = DNS_RESOLVER_H_ADDR_LIST_ENTRY_MALLOC();
                if(NULL == hp->h_addr_list)
                {
                    DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
                    DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterSelfOtherErrors);

                    DNS_RESOLVER_HostentFree(hp);
                    return DNS_ERROR;
                }
                memset(hp->h_addr_list,0,DNS_HOSTENT_IP_SIZE);
            }

            for(i=0;i<DNS_HOSTENT_IP_NUM;i++)
            {
                if(NULL == hp->h_addr_list[i])
                {
                    hp->h_addr_list[i] = DNS_RESOLVER_H_ADDR_LIST_ENTRY_ELM_MALLOC();
                    if(NULL == hp->h_addr_list[i])
                    {
                        DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
                        DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterSelfOtherErrors);

                        DNS_RESOLVER_HostentFree(hp);
                        return DNS_ERROR;
                    }
                    memcpy(hp->h_addr_list[i],ip_p->addr,ip_p->addrlen);

                    if (0 == hp->h_addrtype)
                    {
                        if(ip_p->type == L_INET_ADDR_TYPE_IPV4)
                        {
                            hp->h_addrtype = AF_INET;
                        }
                        else
                        {
                            hp->h_addrtype = AF_INET6;
                        }

                        hp->h_length = ip_p->addrlen;
                    }
                    break;
                }
                else if (L_INET_CompareInetAddr((L_INET_Addr_T *) ip_p,
                    (L_INET_Addr_T *) & hp->h_addr_list[i], 0) == 0)  /* the IP has stored. */
                {
                    break;
                }
            }
            break;

        case DNS_HOSTENT_CNAME:
            if(NULL != hp->h_name)          /* have stored. does this occur ? */
            {
                strcpy(hp->h_name, (char *)name);
                return DNS_OK;
            }

            hp->h_name = DNS_RESOLVER_H_NAME_ENTRY_MALLOC();
            if(NULL == hp->h_name)
            {
                DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
                DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterSelfOtherErrors);

                DNS_RESOLVER_HostentFree(hp);
                return DNS_ERROR;
            }
            strcpy(hp->h_name, (char *)name);
            break;

        case DNS_HOSTENT_ALIAS:
#if 0 /* River@Apr 29, hp->h_name NULL, unuse check */
            if(ZERO == strcmp(name,hp->h_name))
                break;
#endif
            if(NULL == hp->h_aliases)
            {
                hp->h_aliases = DNS_RESOLVER_H_ALIASES_ENTRY_MALLOC();
                if(NULL == hp->h_aliases)
                {
                    DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
                    DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterSelfOtherErrors);

                    DNS_RESOLVER_HostentFree(hp);
                    return DNS_ERROR;
                }
                memset(hp->h_aliases,0,DNS_HOSTENT_ALIAS_SIZE);
            }

            for(i=0;i<DNS_HOSTENT_ALIAS_NUM;i++)
            {
                if(NULL == hp->h_aliases[i])
                {
                    hp->h_aliases[i] = DNS_RESOLVER_H_ALIASES_ENTRY_ELM_MALLOC();
                    if(NULL == hp->h_aliases[i])
                    {
                        DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
                        DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterSelfOtherErrors);

                        DNS_RESOLVER_HostentFree(hp);
                        return DNS_ERROR;
                    }
                    strcpy(hp->h_aliases[i], (char *)name);
                    break;
                }
                else if( ZERO == strcmp(hp->h_aliases[i], (char *)name))
                {
                    break;  /* has stored */
                }
            }
            break;

        default:
            return DNS_ERROR;   /* error type */
    }

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_HostentFree
 *
 * PURPOSE:
 *      free hostent
 *
 * INPUT:
 *      struct hostent* -- hostent node to be free
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_RESOLVER_HostentFree(struct hostent* hp)
{
    int i;

    if (NULL == hp)
        return;

    if(NULL != hp->h_addr_list)
    {
        for(i=0;i<DNS_HOSTENT_IP_NUM;i++)
        {
            if(NULL != hp->h_addr_list[i])
            {
                DNS_RESOLVER_H_ADDR_LIST_ENTRY_ELM_FREE(hp->h_addr_list[i]);
                hp->h_addr_list[i] = NULL;
                continue;
            }
            break;
        }
        DNS_RESOLVER_H_ADDR_LIST_ENTRY_FREE(hp->h_addr_list);
        hp->h_addr_list = NULL; /*---*/
    }

    /* free aliases part */
    if(NULL != hp->h_aliases)
    {
        for(i=0;i<DNS_HOSTENT_ALIAS_NUM;i++)
        {
            if(NULL != hp->h_aliases[i])
            {
                DNS_RESOLVER_H_ALIASES_ENTRY_ELM_FREE(hp->h_aliases[i]);
                hp->h_aliases[i] = NULL;
                continue;
            }
            break;
        }
        DNS_RESOLVER_H_ALIASES_ENTRY_FREE(hp->h_aliases);
        hp->h_aliases = NULL;/*---*/
    }

    /* free cname */
    if(NULL != hp->h_name)
        DNS_RESOLVER_H_NAME_ENTRY_FREE(hp->h_name);
    hp->h_name = NULL;/*---*/

    DNS_RESOLVER_HOST_ENT_ENTRY_FREE(hp);
    hp = NULL;/*---*/
}

/*
 * FUNCTION NAME : gethostbyname_ip
 *
 * PURPOSE:
 *      gethostbyname haveing ip domain added.
 *
 * INPUT:
 *      const I8_T* -- query name
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      hostent* ghp
 *
 * NOTES:
 *      Mofification: Don't search HostTable
 *
 */

static int gethostbyname_ip(const I8_T* name, UI32_T af_family, L_INET_AddrIp_T ip_ar[])
{
    struct hostent* ghp = NULL;
    UI32_T ip_ar_index =0;
    UI32_T ip_num = 0;
    int rc ;
    int local_max;
    UI32_T orig_priority;

    DNS_MGR_GetDnsLocalMaxRequests(&local_max);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(gdns_resolver_sem);  /* pgr0695, return value of statement block in macro */

    if(gdns_local_request > local_max)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);
        return DNS_ERROR;
    }

    if(af_family == AF_UNSPEC)
    {
        gdns_local_request+=2;
    }
    else
    {
        gdns_local_request++;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);

    if(af_family == AF_INET)
    {
        ghp = gethostbyname_r(name, AF_INET, &rc);
    }
    else
    {
        ghp = gethostbyname_r(name, AF_INET6, &rc);
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(gdns_resolver_sem);  /* pgr0695, return value of statement block in macro */
    gdns_local_request--;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);

    if(ghp != NULL)
    {
        ip_num = DNS_RESOLVER_ExtractIpFromHostent(ghp, ip_ar_index, MAXHOSTIPNUM, ip_ar);
        ip_ar_index += ip_num;

        if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        {
            DNS_RESOLVER_DebugShowHostent(ghp);
        }

        ghp = NULL;
    }

    /* af_family is AF_UNSPEC, it query v4 IP after query v6
     */
    if(af_family == AF_UNSPEC)
    {
        ghp = gethostbyname_r(name, AF_INET, &rc);

        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(gdns_resolver_sem);  /* pgr0695, return value of statement block in macro */
        gdns_local_request--;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);

        if(ghp != NULL)
        {
            ip_num = DNS_RESOLVER_ExtractIpFromHostent(ghp, ip_ar_index, MAXHOSTIPNUM, ip_ar);
            ip_ar_index += ip_num;

            if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
            {
                DNS_RESOLVER_DebugShowHostent(ghp);
            }
        }
    }
    return rc;
}

/*
 * FUNCTION NAME : gethostbyname_r
 *
 * PURPOSE:
 *      gethostbyname,but won't free hostent space
 *
 * INPUT:
 *      const I8_T* -- query name
 *      UI32_T family
 *
 * OUTPUT:
 *      rc -- rcode
 *
 * RETURN:
 *      hostent* hp
 *
 * NOTES:
 *      Modify: Let this function only do send hook.
 *              Don't search aynthing here.
 */

static struct hostent* gethostbyname_r(const I8_T* name, UI32_T family, int *rc)
{
    DNS_Send_T* send_p = NULL;
    struct hostent *hp = NULL;
    int ret = DNS_ERROR;
    I8_T rname[256];

    DNS_Name2Lower(name,rname);

    /* hook to send_p list(head),waitting....*/
    /* why not a function here? because i don't pass what args, :) */

    send_p = DNS_RESOLVER_DNS_SEND_ENTRY_MALLOC();
    if(NULL == send_p)
    {
        DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
        DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterSelfOtherErrors);

        return NULL;
    }
    memset(send_p,0,sizeof(DNS_Send_T));

    if(family == AF_INET)
    {
        ret = DNS_RESOLVER_SendHook(send_p,rname,DNS_RRT_A,
                        DNS_OP_QUERY,SYSFUN_TaskIdSelf(),0,NULL);
    }
    else
    {
        ret = DNS_RESOLVER_SendHook(send_p,rname,DNS_RRT_AAAA,
                        DNS_OP_QUERY,SYSFUN_TaskIdSelf(),0,NULL);
    }

    if(ret != DNS_OK)
    {
        if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        {
            printf("DNS send hook failed for %s.",rname);
        }

        DNS_MGR_ServCounterInc(LEAF_dnsServCounterOtherErrors);
        DNS_MGR_ServOptCounterInc(LEAF_dnsServOptCounterSelfOtherErrors);

        DNS_RESOLVER_SendFree(send_p);

        return NULL;
    }

    SYSFUN_SuspendThreadSelf();

    /* if here.we got a answer.or timeout */
    hp = send_p->hp;
    /*isiah.2003-02-14*/
    *rc = send_p->rcode;
    send_p->state = DNS_QS_FREE;
    if(NULL != hp && DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
    {
        DNS_RESOLVER_DebugShowHostent(hp);
    }

    return hp;
}

#if 0  /* was third-party code but no longer used */
/*
 * FUNCTION NAME : gethostbyname
 *
 * PURPOSE:
 *      common dns interface
 *
 * INPUT:
 *      const I8_T* -- query name
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
/*static struct hostent* gethostbyname(const I8_T* name)*/
struct hostent* gethostbyname(const I8_T* name, UI32_T *rc)
{
    struct hostent* hp=NULL;
    DNS_IpDomain_T* pIpDomain = DNS_MGR_GetDnsIpDomainList();

    I8_T doname[256];
    I8_T* ptr;
    int ret;
    UI32_T name_length;

    strcpy((char *)doname, (char *)name);
    ptr = doname;
    DNS_MGR_ResCounterByOpcodeInc(DNS_QR_QUERY,DNS_OP_QUERY);
    DNS_MGR_ResOptCounterInc(LEAF_dnsResOptCounterInternals);

    if( NULL == ptr )
        return NULL;

    name_length=strlen((char *)ptr);

    /* First, if user query is an absolute domain name, we don't append anything ex: test1.com.tw. */
    if( '.' == doname[name_length - 1] )
    {
        doname[name_length - 1] = '\0';
        hp = gethostbyname_ip(doname, rc);

        if(NULL != hp)
        {
            return hp;
        }
        return NULL;
    }


    while( '\0' != *ptr )
    {
        if('.' == *ptr)
            break;

        ptr++;
    }

    /* name is one Label long without dot  */
    if( '\0' == *ptr )
        DNS_MGR_ResOptCounterInc(LEAF_dnsServOptCounterFriendsReqUnparses); /* name is one Label long without dot  */


    /* Second, resolved by appending Domain Name List */
    while(NULL != pIpDomain)
    {
        /*isiah*/
        strcpy((char *)doname, (char *)name);
        ret = DNS_RESOLVER_Name2Domain(doname,pIpDomain);
        if(DNS_ERROR == ret)
        {
            DNS_MGR_ResOptCounterInc(LEAF_dnsServOptCounterFriendsReqUnparses);

            return NULL;
        }

        hp = gethostbyname_ip(doname, rc);

        if(NULL != hp)
        {
            return hp;
        }

        pIpDomain = pIpDomain->next_p;
    }

    /* Third, resolved by appending Default Domain Name */
    strcpy((char *)doname, (char *)name);
    ret = DNS_RESOLVER_Name2Domain(doname,NULL);

    if(DNS_OK == ret)
    {

        hp = gethostbyname_ip(doname, rc);

        if(NULL != hp)
        {
            return hp;
        }
    }
    /*Fourth. Third can't get ip. Go ahead.
    Finally, send name that user input to query*/
    if(hp == NULL)
    {
       strcpy((char *)doname, (char *)name);
        hp = gethostbyname_ip(doname, rc);
        if(NULL != hp)
        {
            return hp;
        }
    }

    /*isiah.2003-02-14*/
    if( (hp==NULL) && (*rc==0) )
    {
        *rc = 300;
    }
    return hp;
}
#endif  /* 0 */


/*
 * FUNCTION NAME : DNS_RESOLVER_DebugShowRR
 *
 * PURPOSE:
 *      show rr
 *
 * INPUT:
 *      UI8_T* -- pointer to rr
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_RESOLVER_DebugShowRR(UI8_T* prr)
{

    DNS_RR_T* rr;
    UI8_T buf[256];
    UI8_T* str;
    UI32_T ip;
    UI8_T ip_str[18];


    DNS_RESOLVER_Rhn2Str((UI8_T*)prr,buf);
    prr += strlen((char *)buf)+1;
    rr = (DNS_RR_T*)prr;

    printf("\n\t\tDNS_SHOW RR\t\t\n");
    printf("\n-------------------------------------------------------------\n");
    printf("TYPE\t:%4d", rr->rtype);
    printf("CLASS\t:%4d", rr->rclass);
    printf("TTL\t:%4lu", (unsigned long)rr->rttl);
    printf("RDLENGTH\t:%4d",rr->rdlength);
    printf("DOMAIN NAME\t:%s",buf);

    str = prr + 10;
    if(DNS_RRT_A == rr->rtype)
    {
        memcpy(&ip,str,4);
/*isiah*/
/*      printf("RDATA\t:%s",inet_ntoa(ip));*/
        L_INET_Ntoa(ip,ip_str);
        printf("RDATA\t:%s",ip_str);
    }
    else
    {
        DNS_RESOLVER_Rhn2Str(str,buf);
        printf("RDATA\t:%s",buf);
    }

}

/*
 * FUNCTION NAME : DNS_RESOLVER_DebugShowHdr
 *
 * PURPOSE:
 *      show dns header
 *
 * INPUT:
 *      DNS_Hdr_T* -- dns pkt header
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_RESOLVER_DebugShowHdr(DNS_Hdr_T* hdr)
{

    I8_T op[2][7] = {"SQUERY", "IQUERY"};
    I8_T qr[2][6] = {"QUERY", "RESP"};


    printf("\n\t\tDNS_SHOW HDR\t\t\n");
    printf("ID\t\tRD\tTC\tAA\tOP\tQR\t\tRC\tZ\tRA\t\n");
    printf("%4u\t%2u\t%2u\t%2u\t%s\t%s\t%2u\t%2u\t%2u\t\n",hdr->id,
        (unsigned int) hdr->rd,
        (unsigned int) hdr->tc, (unsigned int) hdr->aa, op[hdr->opcode], qr[hdr->qr],
        (unsigned int) hdr->rcode, (unsigned int) hdr->z, (unsigned int) hdr->ra);
    printf("\n-------------------------------------------------------------\n");

    printf("\nQDCOUNT\tANCOUNT\tNSCOUNT\tARCOUNT\t\n");
    printf("%4u\t%4u\t%4u\t%4u\t\n",L_STDLIB_Ntoh16(hdr->qdcount),
        L_STDLIB_Ntoh16(hdr->ancount), L_STDLIB_Ntoh16(hdr->nscount),
        L_STDLIB_Ntoh16(hdr->arcount));
}

/*
 * FUNCTION NAME : DNS_RESOLVER_DebugShowHostent
 *
 * PURPOSE:
 *      show hostent
 *
 * INPUT:
 *      struct hostent* -- hostent to be show
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_RESOLVER_DebugShowHostent(struct hostent* hp)
{
    UI8_T ip[SYS_ADPT_IPV6_ADDR_LEN]={0};
    int i;
    UI8_T str[L_INET_MAX_IPADDR_STR_LEN+1];

    printf("\n\t\tDNS_SHOW HOSTENT\t\t\n");
    printf("\n-------------------------------------------------------------\n");
    if(NULL == hp)
    {
        printf(" HOSTENT NULL.\n");
        return;
    }

    printf("CNAME is : %s.\n",hp->h_name);
    if(NULL != hp->h_aliases)
    {
        for(i=0;i<DNS_HOSTENT_ALIAS_NUM;i++)
        {
            if(NULL != hp->h_aliases[i])
            {
                printf("Aliases name[%d] : %s.\n",i,hp->h_aliases[i]);
            }
            else
                break;
        }
    }

    if(NULL != hp->h_addr_list)
    {
        if(hp->h_addrtype == AF_INET)
        {
        for(i=0;i<DNS_HOSTENT_IP_NUM;i++)
        {
            if(NULL != hp->h_addr_list[i])
            {
                memcpy(&ip,hp->h_addr_list[i],4);
                    L_INET_Ntop(L_INET_AF_INET, ip, (char*)str, sizeof(str));
                printf("IP address [%d] : %s \n",i,str);
            }
            else
                break;
        }
    }
        else
        {
            for(i=0;i<DNS_HOSTENT_IP_NUM;i++)
            {
                if(NULL != hp->h_addr_list[i])
                {
                    memcpy(&ip,hp->h_addr_list[i],16);
                    L_INET_Ntop(L_INET_AF_INET6, ip, (char*)str, sizeof(str));
                    printf("IP address [%d] : %s \n",i,str);
                }
                else
                    break;

            }
        }
    }

}

/*
 * FUNCTION NAME : DNS_RESOLVER_DebugShowIp
 *
 * PURPOSE:
 *      DBG
 * INPUT:
 *      I8_T* --
 *      UI32_T --
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_RESOLVER_DebugShowIp(I8_T* show,UI32_T pip)
{
    show[0] = (pip>>24) &0xff;
    show[1] = (pip>>16) &0xff;
    show[2] = (pip>>8 ) &0xff;
    show[3] = (pip    ) &0xff;
}

/*
 * FUNCTION NAME : DNS_MGR_ShowGlobal
 *
 * PURPOSE:
 *      DBG
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_RESOLVER_DebugShowGlobal(void)
{
    printf("\n\t\tDNS_SHOW GLOBAL\t\t\n");
    printf("\n-------------------------------------------------------------\n");
    printf("\nLocal requested has filled hostent number:%-4d.",gdns_hp_num);
    printf("\nResolver still has %d request to answer.\n",gdns_hook_count);
    printf("\nResolver has %d local request for answer.\n",gdns_local_request);
    printf("\nResolver has hit cache %d times.\n",gdns_cache_hit);
    printf("\nResolver hit cache's paraent %d times.\n",gdns_cache_dad);
}

/*
 * FUNCTION NAME : DNS_RESOLVER_DebugShowSendList
 *
 * PURPOSE:
 *      DBG
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      none
 * RETURN:  none
 * NOTES:   none
 *
 */
void DNS_RESOLVER_DebugShowSendList(void)
{
    DNS_Send_T* send_p = gdns_send_head;
    int i = 0,j;

    while(NULL != send_p)
    {
        for(j=0;j<10;j++)
        {
            printf("\n\t\tDNS_SHOW HOOKED REQUEST\t\t\n");
            printf("\n--------------------------------------------------------\n");
            printf("\nNO:\tQTYPE\tTIMEOUT\tSTATE\tRECV\t\tSEND\tSOCK");
            printf("%-4d",i++);
            printf("%-4d\t",send_p->qtype);
            printf("%-4d\t",send_p->timeout);
            printf("%-2d\t",send_p->state);
            printf("%-4d\t",send_p->recvl);
            printf("%-4d\t",send_p->sendl);
            printf("%-4d\t",send_p->sock);
        }

        send_p = send_p->next_p;
    }
}

/*
 * FUNCTION NAME : DNS_ResolverDebugPrintIp
 *
 * PURPOSE:
 *      Debug. This function will transate IP addr with UI32_t type to string
 *      and displays it on console port.
 *
 * INPUT:
 *      UI32_T -- pip
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      This function is just for test.
 *
 */
void DNS_RESOLVER_DebugPrintIp(UI32_T pip)
{
    UI8_T ip[18];
/*isiah*/
/*  printf(inet_ntoa(pip));*/
    L_INET_Ntoa(pip,ip);
    printf("%s\n",ip);
}
/*
 * FUNCTION NAME : DNS_RESOLVER_SearchHosTable()
 *
 * PURPOSE:
 *      Search hostname whether exist in
 *      hostbyname haveing ip domain added.
 *
 * INPUT:
 *      UI8_T   *name   --  string of hostname.
 *      int family -- spec AF_INET(V4), AF_INET6(V6), AF_UNSPEC(V4/V6)
 *
 * OUTPUT:
 *      L_INET_AddrIp_T hostip[] --  host address.
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_SearchHostTable(const UI8_T* name, UI32_T af_family, L_INET_AddrIp_T hostip_ar[])
{
    L_INET_AddrIp_T ip_ar[MAXHOSTIPNUM];
    int ret;
    int local_max;
    UI32_T orig_priority;

    memset(ip_ar, 0, sizeof(ip_ar));

    DNS_MGR_GetDnsLocalMaxRequests(&local_max);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(gdns_resolver_sem);  /* pgr0695, return value of statement block in macro */
    if(gdns_local_request > local_max)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);
        return DNS_ERROR;
    }
    gdns_local_request++;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);

    ret = DNS_HOSTLIB_HostGetByName((char*)name, ip_ar);

        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(gdns_resolver_sem);  /* pgr0695, return value of statement block in macro */
        gdns_local_request--;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(gdns_resolver_sem, orig_priority);

    if(DNS_ERROR != ret)
    {
        if(DNS_RESOLVER_ExtractIpByFamily(MAXHOSTIPNUM, ip_ar, af_family, MAXHOSTIPNUM, hostip_ar) !=0)
        {
            return DNS_OK;
    }

    }

    return DNS_ERROR;
}

/*
 * FUNCTION NAME : search_cache()
 *
 * PURPOSE:
 *      Searh dns cache
 *
 * INPUT:
 *          UI8_T   *name   --  string of hostname.
 *
 * OUTPUT:
 *          L_INET_AddrIp_T ip[] --  host address.
 *
 * RETURN:
 *          DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 *
 */

static int search_cache(const I8_T* name, L_INET_AddrIp_T ip_ar[])
{
    DNS_CacheRR_T* cache_p = NULL;
    I8_T rname[256]={0};
    int cache_status;

    DNS_Name2Lower(name,rname);
    DNS_MGR_GetDnsResCacheStatus(&cache_status);
    if(VAL_dnsResCacheStatus_enabled == cache_status)
    {
        cache_p = DNS_CACHE_GetRR((char *)rname);
    }
    else
    {
        cache_p = NULL;
    }

    if(NULL != cache_p && ZERO == strcmp(cache_p->name, (char *)rname))
    {
        if(DNS_RESOLVER_ExtractInaddrfromCache(cache_p, MAXHOSTIPNUM, ip_ar) != 0)
        {
        DNS_CACHE_FreeRR(cache_p);
            return DNS_OK;
        }
    }
    DNS_CACHE_FreeRR(cache_p);
    return DNS_ERROR;
}

/*
 * FUNCTION NAME : DNS_RESOLVER_GetHostByName()
 *
 * PURPOSE:
 *      common dns interface
 *
 * INPUT:
 *          UI8_T   *hostname   --  string of hostname.
 *          int family -- spec AF_INET(V4), AF_INET6(V6), AF_UNSPEC(V4/V6)
 *          type --   DNS_SEND_QUERY or DNS_SEARCH_CACHE
 *
 * OUTPUT:
 *          L_INET_AddrIp_T hostip_ar[] --  host address,
 *                                          at most MAXHOSTIPNUM,
 *                                          "addrlen == 0" means no more.
 *
 * RETURN:
 *          DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      This function do two behavior depend on type.
 *      If DNS_SEARCH_CACHE type, search cache.
 *      If DNS_SEND_QUERY type, query to name-server.
 */
int DNS_RESOLVER_GetHostByName(const UI8_T* name, UI32_T af_family, UI32_T type, L_INET_AddrIp_T hostip_ar[])
{
    char domain_list[DNS_MAX_NAME_LENGTH + 1];
    L_INET_AddrIp_T ip_ar[MAXHOSTIPNUM];
    I8_T doname[256];
    I8_T* ptr;
    int ret;
    int rc = DNS_ERROR;
    UI32_T name_length;
    UI32_T num = 0;

    memset(ip_ar, 0, sizeof(ip_ar));

    strcpy((char *)doname, (char *)name);
    ptr = doname;
    if( NULL == ptr )
        return DNS_ERROR;

    name_length=strlen((char *)ptr);

    if(type == DNS_SEND_QUERY)
    {
        DNS_MGR_ResCounterByOpcodeInc(DNS_QR_QUERY,DNS_OP_QUERY);
        DNS_MGR_ResOptCounterInc(LEAF_dnsResOptCounterInternals);
    }

    /* First, if user query is an absolute domain name,
     * we don't append anything ex: test1.com.tw.
     */
    if(type == DNS_SEARCH_CACHE)
    {
        if(search_cache(doname, ip_ar) == DNS_OK)
        {
           num = DNS_RESOLVER_ExtractIpByFamily(MAXHOSTIPNUM, ip_ar, af_family, MAXHOSTIPNUM, hostip_ar);
        }
    }

    if(type == DNS_SEND_QUERY)
    {
        if( '.' == doname[name_length - 1] )
        {
            doname[name_length - 1] = '\0';
            rc = gethostbyname_ip(doname, af_family, ip_ar);
            if(rc == DNS_OK)
            {
                num = DNS_RESOLVER_ExtractIpByFamily( MAXHOSTIPNUM, ip_ar, af_family, MAXHOSTIPNUM, hostip_ar);
            }
        }
#if 0
        while( '\0' != *ptr )
        {
            if('.' == *ptr)
                break;

            ptr++;
        }
#endif
        /* River@Apr 25, 2008, if there's a dot, wherever, we send to query without append first */
        while( '\0' != *ptr )
        {
            if('.' == *ptr)
            {
                rc = gethostbyname_ip(doname, af_family, ip_ar);
                if(rc == DNS_OK)
                {
                    num = DNS_RESOLVER_ExtractIpByFamily( MAXHOSTIPNUM, ip_ar, af_family, MAXHOSTIPNUM, hostip_ar);
                }
            }
            ptr++;
        }

        /* name is one Label long without dot  */
        if( '\0' == *ptr )
            DNS_MGR_ResOptCounterInc(LEAF_dnsServOptCounterFriendsReqUnparses);
    }

    if( num != 0)
    {
        return DNS_OK;
    }

    /* Second, resolved by appending Domain Name List
     */
    domain_list[0] = '\0';
    while (TRUE == DNS_MGR_GetNextDomainNameList(domain_list))
    {
        /*isiah*/
        strcpy((char *)doname, (char *)name);
        ret = DNS_RESOLVER_Name2Domain(doname, domain_list);
        if(DNS_ERROR == ret)
        {
            DNS_MGR_ResOptCounterInc(LEAF_dnsServOptCounterFriendsReqUnparses);

            return DNS_ERROR;
        }

        if(type == DNS_SEARCH_CACHE)
        {
            if(search_cache(doname, ip_ar) == DNS_OK)
            {
                num = DNS_RESOLVER_ExtractIpByFamily(MAXHOSTIPNUM, ip_ar, af_family, MAXHOSTIPNUM, hostip_ar);
            }
        }

        if(type == DNS_SEND_QUERY)
        {
            rc = gethostbyname_ip(doname, af_family, ip_ar);
            if(rc == DNS_OK)
            {
                num = DNS_RESOLVER_ExtractIpByFamily(MAXHOSTIPNUM, ip_ar, af_family, MAXHOSTIPNUM, hostip_ar);
            }

        }

        if( num != 0)
        {
            return DNS_OK;
        }
    }

    /* Third, resolved by appending Default Domain Name */
    strcpy((char *)doname, (char *)name);
    ret = DNS_RESOLVER_Name2Domain(doname,NULL);
    if(DNS_OK == ret)
    {
        if(type == DNS_SEARCH_CACHE)
        {
            if(search_cache(doname, ip_ar) == DNS_OK)
            {
                num = DNS_RESOLVER_ExtractIpByFamily(MAXHOSTIPNUM, ip_ar, af_family, MAXHOSTIPNUM, hostip_ar);
            }
        }

         if(type == DNS_SEND_QUERY)
        {
            rc = gethostbyname_ip(doname, af_family, ip_ar);
            if(rc == DNS_OK)
            {
                num = DNS_RESOLVER_ExtractIpByFamily(MAXHOSTIPNUM, ip_ar, af_family, MAXHOSTIPNUM, hostip_ar);
            }

        }

        if( num != 0)
        {
            return DNS_OK;
        }
    }

    /*Fourth, send name that user input to query*/

    if(type == DNS_SEND_QUERY)
    {
        strcpy((char *)doname, (char *)name);

        rc = gethostbyname_ip(doname, af_family, ip_ar);
        if(rc == DNS_OK)
        {
            num = DNS_RESOLVER_ExtractIpByFamily(MAXHOSTIPNUM, ip_ar, af_family, MAXHOSTIPNUM, hostip_ar);

            if(num == 0)
            {
                rc =DNS_ERROR;
            }
        }
    }

    if( num != 0)
    {
        return DNS_OK;
    }
    else
    {
        return rc;
    }
}

/*
 * FUNCTION NAME : DNS_RESOLVER_ExtractIpByFamily()
 *
 * PURPOSE:
 *      This function fill in hostip_ar with ip which extract from ip_ar accrond to family.
 *
 * INPUT:
 *      ip_ar_len -- size of ip_ar[]
 *      ip_ar[] -- ip array, element may be V4 or V6
 *      af_family -- AF_INET, AF_INT6, AF_UPSPEC
 *      hostip_len -- size of hostip_ar[]
 *
 * OUTPUT:
 *      hostip_ar[] -- ip array that ip type equal family
 *
 * RETURN:
 *      number of fill in hostip_ar
 *
 * NOTES:
 *
 *
 */

static UI32_T DNS_RESOLVER_ExtractIpByFamily(UI32_T ip_ar_len, L_INET_AddrIp_T ip_ar[], UI32_T af_family,
                                             UI32_T hostip_ar_len, L_INET_AddrIp_T hostip_ar[])
{
    UI32_T num = 0;
    UI32_T i = 0;

    switch(af_family)
    {
        case AF_UNSPEC:
        case AF_INET6:
            for(i=0;i<ip_ar_len;i++)
            {
                if(num >= hostip_ar_len)
                {
                    break;
                }

                if(ip_ar[i].type == L_INET_ADDR_TYPE_IPV6 || ip_ar[i].type == L_INET_ADDR_TYPE_IPV6Z )
                {
                    memcpy(&hostip_ar[num], &ip_ar[i], sizeof(L_INET_AddrIp_T));
                    num++;
                }
            }

            if(af_family == AF_INET6)
            {
                break;
}

        case AF_INET:
            for(i=0;i<ip_ar_len;i++)
            {
                if(num >= hostip_ar_len)
                {
                    break;
                }

                if(ip_ar[i].type == L_INET_ADDR_TYPE_IPV4 )
                {
                    memcpy(&hostip_ar[num], &ip_ar[i], sizeof(L_INET_AddrIp_T));
                    num++;
                }
            }
            break;
    }

    return num;

}

/*
 * FUNCTION NAME : DNS_RESOLVER_ExtractInaddrfromCache()
 *
 * PURPOSE:
 *      This function fill in hostip_ar with ip which extract from cacheRR.
 *
 * INPUT:
 *      cache_p -- cacheRR
 *      ip_ar_len -- size of ip_ar[]
 *
 * OUTPUT:
 *      ip_ar
 *
 * RETURN:
 *      number of fill in ip_ar
 *
 * NOTES:
 *
 *
 */
static UI32_T DNS_RESOLVER_ExtractInaddrfromCache(DNS_CacheRR_T* cache_p, UI32_T ip_ar_len, L_INET_AddrIp_T ip_ar[])
{
    DNS_CacheRR_T* temp_p = cache_p;
    UI32_T num = 0;

    if(DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
    {
        printf("Find answer in cache.  \n");
    }

    while(NULL != temp_p && num < ip_ar_len)
    {
        if(strcmp(temp_p->name, cache_p->name) == 0)
        {
            memcpy(&ip_ar[num], &temp_p->ip, sizeof(L_INET_AddrIp_T));
            num++;
}

        temp_p = temp_p->next_p;
    }

    return num;
}
/*
 * FUNCTION NAME : DNS_RESOLVER_ExtractIpFromHostent()
 *
 * PURPOSE:
 *      This function fill in hostip_ar with ip which extract from hostent.
 *
 * INPUT:
 *      ghp -- hostent.
 *      host_idx -- index which starts to fill in ip[] array.
 *      ip_ar_len -- size of ip_ar[]
 *
 * OUTPUT:
 *      ip_ar[]
 *
 * RETURN:
 *      number of fill in ip_ar
 *
 * NOTES:
 *
 *
 */
static UI32_T DNS_RESOLVER_ExtractIpFromHostent(struct hostent* ghp, UI32_T host_idx, UI32_T ip_ar_len, L_INET_AddrIp_T ip_ar[])
{
    UI32_T i;
    UI32_T num = 0;

    if(ghp->h_addr_list != NULL)
    {
        for(i=0;i<DNS_HOSTENT_IP_NUM;i++)
        {
            if(host_idx >= ip_ar_len)
            {
                break;
            }

            if(NULL != ghp->h_addr_list[i])
            {
                if(ghp->h_addrtype == AF_INET)
                {
                    ip_ar[host_idx].type = L_INET_ADDR_TYPE_IPV4;
                    ip_ar[host_idx].addrlen = SYS_ADPT_IPV4_ADDR_LEN;
                }
                else if(ghp->h_addrtype == AF_INET6)
                {
                    ip_ar[host_idx].type = L_INET_ADDR_TYPE_IPV6;
                    ip_ar[host_idx].addrlen = SYS_ADPT_IPV6_ADDR_LEN;
                }

                memcpy(ip_ar[host_idx].addr, ghp->h_addr_list[i], ghp->h_length);
                host_idx++;
                num++;
            }
            else
            {
                break;
            }
        }

    }

    return num;
}

#endif /* #if (SYS_CPNT_DNS == TRUE) */


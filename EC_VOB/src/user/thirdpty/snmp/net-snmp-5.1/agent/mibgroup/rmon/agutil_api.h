/**************************************************************
 * Copyright (C) 2001 Alex Rozin, Optical Access 
 *
 *                     All Rights Reserved
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 * 
 * ALEX ROZIN DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * ALEX ROZIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 ******************************************************************/

#ifndef _agutil_api_h_included__
#define _agutil_api_h_included__

#include <string.h>

#define ag_trace_null                       ((void)0)

/* Originally, ag_trace refers to ag_trace_log
 * Since VXWORK does not do snmp_log, so does not need to do snprintf.
 * If need to do snmp_log, define ag_trace to ag_trace_log
 */
#define ag_trace(format, ...)               ag_trace_null

#if 0                           /* for debug */
#warning MEMORY DEBUG VERSION
void           *dbg_f_AGMALLOC(size_t size);
void            dbg_f_AGFREE(void *ptr);
char           *dbg_f_AGSTRDUP(const char *s);
void            dbg_f_AG_MEM_REPORT(void);
#  define AGMALLOC(X)	dbg_f_AGMALLOC(X)
#  define AGFREE(X)       { dbg_f_AGFREE(X); X = NULL; }
#  define AGSTRDUP(X)     dbg_f_AGSTRDUP(X)
#else
#  define AGMALLOC(X)	malloc(X)
#  define AGFREE(X)	{ free(X); X = NULL; }
#  define AGSTRDUP(X)	strdup(X)
#endif

typedef struct {
    size_t          length;
    oid             objid[MAX_OID_LEN];
} VAR_OID_T;

void            ag_trace_log(const char *format, ...);

int             AGUTIL_advance_index_name(struct variable *vp, oid * name,
                                          size_t * length, int exact);
int             AGUTIL_get_int_value(u_char * var_val, u_char var_val_type,
                                     size_t var_val_len, long min_value,
                                     long max_value, long *long_tmp);
int             AGUTIL_get_string_value(u_char * var_val,
                                        u_char var_val_type,
                                        size_t var_val_len,
                                        size_t buffer_max_size,
                                        u_char should_zero_limited,
                                        size_t * buffer_actual_size,
                                        char *buffer);
int             AGUTIL_get_oid_value(u_char * var_val, u_char var_val_type,
                                     size_t var_val_len,
                                     VAR_OID_T * data_source_ptr);

u_long          AGUTIL_sys_up_time(void);

#if OPTICALL_ACESS
#define ETH_STATS_T UID_PORT_STATISTICS_T
#else
typedef struct {
    u_int              ifIndex;
    u_long             octets;
    u_long             packets;
    u_long             bcast_pkts;
    u_long             mcast_pkts;
    u_long             crc_align;
    u_long             undersize;
    u_long             oversize;
    u_long             fragments;
    u_long             jabbers;
    u_long             collisions;
    u_long             pkts_64;
    u_long             pkts_65_127;
    u_long             pkts_128_255;
    u_long             pkts_256_511;
    u_long             pkts_512_1023;
    u_long             pkts_1024_1518;
    /*EPR_ID:ES3628BT-FLF-ZZ-00180,ES4827G-FLF-ZZ-00484  
    Problem: NetworkMonitor:Node "etherHistoryUtilization" return worng value while the port works at 1000full mode.    
    Root Cause: ETH_STATS_T->octets was defined as the 32-bit.The value will overfolw while these ports work at         
                1000full mode.
    Solution: add new unsigned long long value for 1000full mode.
    Modified files:
    	src\user\thirdpty\snmp\net-snmp-5.1\agent\mibgroup\rmon\history.c
    	src\user\thirdpty\snmp\net-snmp-5.1\agent\mibgroup\rmon\agutil_api.h
    	src\user\thirdpty\snmp\net-snmp-5.1\agent\mibgroup\rmon\agutil.c
    Modified by: Shumin.Wang
    Approved by: Tiger Liu
    */
    unsigned long long octets_64;
} ETH_STATS_T;

typedef struct {
    u_int           ifIndex;
    UI32_T          hc_overflow_pkts;
    UI64_T          hc_pkts;
    UI32_T          hc_overflow_octets;
    UI64_T          hc_octets;
    UI32_T          hc_overflow_pkts_64;
    UI64_T          hc_pkts_64;
    UI32_T          hc_overflow_pkts_65_127;
    UI64_T          hc_pkts_65_127;
    UI32_T          hc_overflow_pkts_128_255;
    UI64_T          hc_pkts_128_255;
    UI32_T          hc_overflow_pkts_256_511;
    UI64_T          hc_pkts_256_511;
    UI32_T          hc_overflow_pkts_512_1023;
    UI64_T          hc_pkts_512_1023;
    UI32_T          hc_overflow_pkts_1024_1518;
    UI64_T          hc_pkts_1024_1518;
} ETH_STATS_HC_T;
#endif

void            SYSTEM_get_eth_statistics(VAR_OID_T * data_source,
                                          ETH_STATS_T * where);

#endif                          /* _agutil_api_h_included__ */

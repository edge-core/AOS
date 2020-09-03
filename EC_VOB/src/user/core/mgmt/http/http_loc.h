//
//  http.h
//  http_ssl
//
//  Created by JunYing Yeh on 2014/5/1.
//
//

#ifndef http_ssl_http_h
#define http_ssl_http_h

#ifndef L_USE_SOCK
#define L_USE_SOCK
#endif

#define HTTP_LOC

#include <netinet/tcp.h>

#include "l_lib.h"

#if (SYS_CPNT_HTTPS == TRUE)
#include "openssl/ssl.h"
#include "openssl/crypto.h"
#include "openssl/err.h"
#endif /* SYS_CPNT_HTTPS */

#include "sys_type.h"
#include "sys_cpnt.h"

#ifndef SYS_CPNT_MGMT_IP_FLT
#  define SYS_CPNT_MGMT_IP_FLT  FALSE
#endif

#ifndef SYS_CPNT_WEBAUTH
#  define SYS_CPNT_WEBAUTH      FALSE
#endif

#ifndef SYS_CPNT_CLUSTER
#  define SYS_CPNT_CLUSTER      FALSE
#endif

#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_module.h"

#include "sysfun.h"

#include "l_mm.h"
#include "l_stdlib.h"

#include "http_config.h"

#include "http_time.h"
#include "http_socketpair.h"
#include "http_log.h"
#include "http_ring_buffer.h"
#include "http_list.h"

#include "http_def.h"
#include "http_type.h"

#include "http_envcfg.h"
#include "http_file.h"
#include "http_init.h"
#include "http_mgr.h"
#include "http_om.h"
#include "http_task.h"
#include "http_util.h"

#include "http_worker.h"
#include "http_connection.h"
#include "http_request.h"
#include "http_scheduler.h"
#include "http_event.h"
#include "mm.h"

#if(SYS_CPNT_MGMT_IP_FLT == TRUE)
#include "mgmt_ip_flt.h"
#endif /* SYS_CPNT_MGMT_IP_FLT */

#define HTTP_createsocket(socket_family, socket_type, protocol) \
    L_SOCK_createsocket(socket_family, socket_type, protocol)

#define HTTP_closesocket(s)                L_SOCK_closesocket(s)

#define HTTP_socket_set_option(s, opt) L_BIO_socket_set_option(s, opt)
#define HTTP_readsocket(s, buf, len, flags) L_BIO_socket_read(s, buf, len, flags)
#define HTTP_writesocket(s, buf, len, flags) L_BIO_socket_write(s, buf, len, flags)

#endif

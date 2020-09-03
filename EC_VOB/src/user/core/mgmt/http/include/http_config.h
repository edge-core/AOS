//
//  http_cfg.h
//  http
//
//  Created by JunYing Yeh on 2014/7/23.
//
//

// FIXME: This file is platform dependence !!

#ifndef _HTTP_HEADER_CONFIG_H_
#define _HTTP_HEADER_CONFIG_H_

/* worker
 */
#define HTTP_CFG_TOTAL_WORKER                           3

/* connection
 */
#define HTTP_CFG_MAXWAIT                                40  /* the max number of session and
                                                             * connection
                                                             */
#define HTTP_CFG_TCP_KEEP_ALIVE_IDLE                    120 /* seconds */
#define HTTP_CFG_TCP_KEEP_ALIVE_INTERVAL                10  /* seconds */
#define HTTP_CFG_TCP_KEEP_ALIVE_COUNT                   6   /* count */

#define HTTP_CFG_CONNECTION_DEBUG                       1

/* log
 */
#define HTTP_CFG_LOG_MAX_MESSAGE_STR_LEN                128
#define HTTP_CFG_LOG_MAX_ENTRIES                        500

/* event
 */
#define HTTP_CFG_EVENT_DEBUG                            1
#define HTTP_CFG_EVENT_DEBUG_MAX_FUNCTION_SIZE          64

/* cgi
 */
#define HTTP_CFG_CGI_DEBUG                              1

/* ring buffer
 */
#define HTTP_CFG_RING_BUFFER_DEBUG                      1

#define HTTP_CFG_LONG_POLLING_TIMEOUT_SEC               13

#define HTTP_CFG_POOLING_URLS                           {"polling_28port.htm",    \
                                                         "polling_52port.htm"}

// FIXME: Create a default config file at build time. Set a default root dir in this file. Dont use constant here !!
#define HTTP_CFG_DFLT_ROOT_DIR                  "/usr/webroot"
#define HTTP_CFG_DFLT_INDEX_PAGE                "/index.htm"
#define HTTP_CFG_DFLT_CONFIG_FILE_PATH          "/usr/webroot/config.json"

#endif

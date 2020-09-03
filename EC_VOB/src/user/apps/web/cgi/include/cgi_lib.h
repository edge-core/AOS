/*--------------------------------------------------------------------------+
 * FILE NAME - cgi_lib.c
 * -------------------------------------------------------------------------+
 * ABSTRACT: This file                                                      +
 *                                                                          +
 * -------------------------------------------------------------------------+
 *                                                                          +
 * Modification History:                                                    +
 *   By              Date     Ver.   Modification Description               +
 *   --------------- -------- -----  ---------------------------------------+
 *                                   creation                               +
 *   squid           2005/5          Add Snmpv3 for 3COM                    +
 *   squid           2005/6          Add ProxyArp for 3COM                  +
 * -------------------------------------------------------------------------+
 * Copyright(C)                              ACCTON Technology Corp., 2001  +
 * -------------------------------------------------------------------------*/
#ifndef _CGI_LIB_H_
#define _CGI_LIB_H_

/*---------------------------
 * INCLUDE FILE DECLARATIONS
 *---------------------------*/
#include "cgi_coretype.h"
#include "http_envcfg.h"
#include "cgi.h"
#include "http_def.h"


UI32_T CGI_LIB_JSON_Stringify(xmlNodePtr node, int (*fn)(HTTP_Connection_T *http_connection, int sock, const char *text), HTTP_Connection_T *http_connection, int sock);

#endif  /* CGI_LIB_H */

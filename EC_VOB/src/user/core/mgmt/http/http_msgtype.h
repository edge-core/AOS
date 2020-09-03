//
//  http_msgtype.def.h
//  http
//
//  Created by JunYing Yeh on 2014/7/20.
//
//

#ifndef MSGTYPE
#define MSGTYPE(x, y)
#endif

MSGTYPE(MEMORY,     "memory")
MSGTYPE(SOCKET,     "socket")
MSGTYPE(WORKER,     "worker")
MSGTYPE(CONNECTION, "connection")
MSGTYPE(PARAMETER,  "parameter")
MSGTYPE(EVENT,      "event")
MSGTYPE(CGI,        "cgi")
MSGTYPE(CONFIG,     "config")

#undef MSGTYPE
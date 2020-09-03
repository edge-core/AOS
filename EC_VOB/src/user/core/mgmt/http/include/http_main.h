/* ----------------------------------------------------------------------
 * FILE NAME: http\htMain.h

 * AUTHOR: Integrated Systems, Inc
 * MODIFIER: Chung, Daniel K. (Zhong Qiyao)
 * PLATFORM: Microsoft Windows 95, with pSOS Diab compiler for 68K/PowerPC.

 * ABSTRACT:
 * This is part of the embedded program for pSOS series.
 * Entire firmware is integrated by Ted Tai (Dai Quantian).

 * Library for core of HTTP server.
 * Include file for the main things.

 * HISTORY:
 * 1997-04-30 (Fri): Modified by Daniel K. Chung.
 * 1998-04-30 (Thu): Removed "httpcfg" (HTTP_CFG).
 * 1998-05-08 (Fri): Name changed from http.h to htMain.h.

 * COPYRIGHT (C) Accton Technology Corporation, 1997.
 * ---------------------------------------------------------------------- */

/***********************************************************************/
/*                                                                     */
/*   MODULE: include\http.h                                            */
/*   DATE:   96/04/23                                                  */
/*   PURPOSE:                                                          */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*                                                                     */
/*          Copyright 1991 - 1993, Integrated Systems, Inc.            */
/*                      ALL RIGHTS RESERVED                            */
/*                                                                     */
/*   Permission is hereby granted to licensees of Integrated Systems,  */
/*   Inc. products to use or abstract this computer program for the    */
/*   sole purpose of implementing a product based on Integrated        */
/*   Systems, Inc. products.   No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in       */
/*   whole, are granted.                                               */
/*                                                                     */
/*   Integrated Systems, Inc. makes no representation or warranties    */
/*   with respect to the performance of this computer program, and     */
/*   specifically disclaims any responsibility for any damages,        */
/*   special or consequential, connected with the use of this program. */
/*                                                                     */
/***********************************************************************/


#ifndef _HTTP_H
#define _HTTP_H


#define HTTP_SERVER_SOFTWARE    "PSOSHTTP/1.0.3"


struct ext_map_t {
	char *find;
	char *replace;
};

struct mime_map_t {
    char *file_extension;
    char *content_type;
};

#ifdef HTTP_CFG
struct httpcfg_t {
	char	*html_path;	                    /* relative path to look for html pages */
	char	*cgi_path;                      /* actual location of cgi applications */
    char    *default_file;                  /* default file to look for if ref dir */
    char    **hlist;                        /* list of trusted clients allowed to connect */
    struct ulist_t *ulist;                  /* user list permitted to access secured path  */
    char    *secured_path;                  /* directory which authentication is applied */
    void    (*fileless_system)();           /* Task entry point, 0 is disabled */
	long	browsing;		                /* 0 is browsing disabled, 1 is enabled */
	long	logging;		                /* 0 is logging disabled, 1,2,3 is logging level */
	char	*log_file;	                    /* full path to name of log file */
	long	task_prio;		                /* priority of httpd task */
	long	max_in_waiting;		            /* max length of queue of pending connection */
    long    max_connections;                /* throttling, max number of connections allowed */
	long	port;			                /* port number used for accepting requests */
    struct ext_map_t *ext_map;              /* ptr to array of extention mapping structs */
    struct mime_map_t *mime_map;            /* ptr to array of file extension mappings to
                                               determine content-type */
    char    *realm_value;                   /* <realm-value> used with authentication */
    long    max_log_file_size;              /* maximum number of bytes to use for log file */
    char    *cgi_bin_ref;                   /* relative location of cgi apps, to be used in html ref */
    long    fileless_stack;                 /* stack size to use when running fileless CGI */
};
typedef struct httpcfg_t httpcfg_t;
#endif /* HTTP_CFG */

#endif /* _HTTP_H */

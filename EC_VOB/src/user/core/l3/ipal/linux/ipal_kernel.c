/*
 *   File Name: ipal_kernel.c
 *   Purpose:   TCP/IP shim layer(ipal) initialization with kernel
 *   Note:		  
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify		   Date      Reason
 *              
 *
 *   Copyright(C)  Accton Corporation 2007~2009
*/

/*
 * INCLUDE FILE DECLARATIONS
*/

#include <stdio.h>
#include <netinet/in.h>

#include "sys_type.h"
#include "sysfun.h"

#include "l_prefix.h"

#include "ipal_types.h"
#include "ipal_debug.h"

#include <linux/rtnetlink.h>
#include "ipal_rt_netlink.h"

/*
 * NAMING CONST DECLARATIONS
*/



/*
 * MACRO FUNCTION DECLARATIONS
*/


/*
 * DATA TYPE DECLARATIONS
*/



/*
 * LOCAL SUBPROGRAM DECLARATIONS
*/

void
IPAL_Kernel_Init ()
{
	IPAL_DEBUG_PRINT("TCP/IP Stack Shim Layer Initialization ...");

	/* netlink socket */
	IPAL_Rt_Netlink_Init ();
}

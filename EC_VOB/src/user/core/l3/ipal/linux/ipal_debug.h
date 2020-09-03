/*
 *   File Name: ipal_debug.h
 *   Purpose:   TCP/IP shim layer(ipal) debug utility
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify		   Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
*/

#ifndef __IPAL_DEBUG_H
#define __IPAL_DEBUG_H

/*
 * INCLUDE FILE DECLARATIONS
*/


/*
 * NAMING CONST DECLARATIONS
*/



/*
 * MACRO FUNCTION DECLARATIONS
*/
#define IPAL_DEBUG
#undef IPAL_DEBUG

#if defined (IPAL_DEBUG)
#define IPAL_DEBUG_PRINT(...)	\
{								\
    printf ("\r\nTask Id: %lu\r\n", (unsigned long)SYSFUN_TaskIdSelf());								\
	printf ("File: %s, Function: %s, Line: %d\r\n", __FILE__, __FUNCTION__, __LINE__);	\
	printf (__VA_ARGS__);																\
	printf ("\r\n");																	\
}
#else
#define IPAL_DEBUG_PRINT(...) do {} while (0)
#endif


void IPAL_BACKDOOR_Create_InterCSC_Relation();
/*
 * DATA TYPE DECLARATIONS
*/


#endif /* end of __IPAL_DEBUG_H */



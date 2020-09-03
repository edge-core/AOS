/* -------------------------------------------------------------------------------------
 * FILE	NAME: MAC_CACHE.h
 *                                                                                      
 * PURPOSE: Mac Cache supports fast (mac,vlan, port) learning from ingress packet,
 *          because the AMTR learing is slow and distributed.
 * NOTES:   1. This module supports MAC cache, only control packet or management packet
 *             will use this table.
 *          2. This module is inner component of CSC, not a visible component in system
 *             architecture. So we use MAC_CACHE as file name.
 *          3. This module should use HiSam as inner database code;
 *             if the table full, this module would find proper one to be replaced.
 *                                                                                      
 * MODIFICATION	HISOTRY:	                                                            
 * MODIFIER         DATE             DESCRIPTION                                              
 * -------------------------------------------------------------------------------------
 * Penny Chang      05-08-2003       First created
 * William Chiang   07-09-2003       Change age-out time to 45 sec;
 *                                   If split send function to pktOut/forward,
 *                                   age-out time can be 10 sec.
 * -------------------------------------------------------------------------------------
 * Copyright(C)	       Accton Techonology Corporation 2003
 * -------------------------------------------------------------------------------------*/
 
 
#ifndef _MAC_CACHE_H
#define _MAC_CACHE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"


/* NAME CONSTANT DECLARATIONS
 */
/* RETURN VALUE */
#define MAC_CACHE_OK						   			0x0
#define MAC_CACHE_NOT_EXIST                   			0x80000001
#define MAC_CACHE_ENTRY_AGE_OUT                         0x80000002
#define MAC_CACHE_RET_NO_MORE_SPACE                     0x80000003

#define MAC_CACHE_RET_TABLE_FULL                        0x80000004
#define MAC_CACHE_RET_TABLE_END                         0x80000005
#define MAC_CACHE_RET_ENTRY_EXIST                       0x80000006
#define MAC_CACHE_RET_ENTRY_NOT_EXIST                   0x80000007
#define MAC_CACHE_RET_EXCEED_MAX_NUM_OF_SERVER_PER_PORT 0x80000008
#define MAC_CACHE_RET_BUFFER_LEN_ERROR                  0x80000009
#define MAC_CACHE_RET_MAC_INVALID                		0x8000000a
#define MAC_CACHE_RET_FAIL                              0x8000000b


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */
typedef struct MAC_CACHE_MacEntry_S
{
    UI32_T	vid_ifindex;
    UI8_T   mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  lport;
    UI32_T  timestamp; 
}   MAC_CACHE_MacEntry_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


/* NAMING CONSTANT DECLARATION 
 */

/* EXPORTED SUBPROGRAM DECLARATION
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_Init
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will initialize system resouce for MAC cache.
 * INPUT    : cache_size -- size of cache; the total number of table entry.
 *            hash_depth -- collision used.
 * OUTPUT   : none
 * RETURN   : None.
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void MAC_CACHE_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_ResetCache
 *--------------------------------------------------------------------------
 * PURPOSE  : Reset Cache to empty cache.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : All space allocated or used should be released.
 *--------------------------------------------------------------------------*/
void MAC_CACHE_ResetCache(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_SetLearnedMacEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set learned MAC entry to MAC cache.
 * INPUT    : mac_entry -- the learning information.
 *                vid_ifindex -- learning interface
 *                mac         -- source mac of this packet.
 *                lport       -- ingress port.
 * OUTPUT   : none
 * RETURN   : MAC_CACHE_OK  -- add/update the MAC cache with key successfully.
*			  MAC_CACHE_RET_MAC_INVALID	-- mac is either all 0, or all 1, or multicast address
 * NOTES    : 1. key is (vlan, mac).
 *            2. This function should replace 
 *-------------------------------------------------------------------------- */
UI32_T MAC_CACHE_SetLearnedMacEntry(MAC_CACHE_MacEntry_T  mac_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_GetLport
 *--------------------------------------------------------------------------
 * PURPOSE  : Get associated lport with (ip,mac)
 * INPUT    : ifindex
 *          : mac
 * OUTPUT   : lport_p.
 * RETURN   : MAC_CACHE_OK
 *            MAC_CACHE_NOT_EXIST, can't find the mac.
 *            MAC_CACHE_ENTRY_AGE_OUT, the entry existed but age-out
 * NOTES    : key is (vlan,mac)
 *-------------------------------------------------------------------------- */
UI32_T MAC_CACHE_GetLport(UI32_T ifindex, UI8_T *mac, UI32_T *lport_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_SetAgeOutTime
 *--------------------------------------------------------------------------
 * PURPOSE  : Set MAC cache age out time
 * INPUT    : age_out_time   --  age out time
 * OUTPUT   : none
 * RETURN   : MAC_CACHE_OK
 * NOTES    : This api is for IML backdoor use
 *--------------------------------------------------------------------------
 */
UI32_T MAC_CACHE_SetAgeOutTime(UI32_T age_out_time);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_GetAgeOutTime
 *--------------------------------------------------------------------------
 * PURPOSE  : Get MAC cache age out time
 * INPUT    : age_out_time   --  age out time
 * OUTPUT   : age_out_time   --  age out time
 * RETURN   : MAC_CACHE_OK
 *            MAC_CACHE_RET_FAIL
 * NOTES    : This api is for IML backdoor use
 *--------------------------------------------------------------------------
 */
UI32_T MAC_CACHE_GetAgeOutTime(UI32_T *age_out_time);
#endif

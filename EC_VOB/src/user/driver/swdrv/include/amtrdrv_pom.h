/*-------------------------------------------------------------------------
 * MODULE NAME: AMTRDRV_POM.h
 *-------------------------------------------------------------------------
 * PURPOSE: To provide the service for Other CSCs to get information
 *
 *
 * NOTES:
 *
 * Modification History:
 *      Date          Modifier,   Reason
 *      ------------------------------------
 *      08-31-2004    MIKE_YEH    create
 *
 * COPYRIGHT(C)         Accton Corporation, 2004
 *------------------------------------------------------------------------*/

#ifndef AMTRDRV_POM_H
#define AMTRDRV_POM_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "amtr_type.h"
#include "isc.h"



/*--------------------
 * EXPORTED CONSTANTS
 *-------------------*/

/*--------------------
 * EXPORTED ROUTINES
 *-------------------*/

BOOL_T  AMTRDRV_OM_GetElementByGroupEntry(UI32_T group, void *record, UI32_T *element);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetExactRecord
 *------------------------------------------------------------------------------
 * PURPOSE: This function will get exact record from hash table
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry - partial record infor but vid & mac are required
 * OUTPUT : AMTR_TYPE_AddrEntry_T *addr_entry - return exact record info
 * RETURN : None
 * NOTES  : The input record should include mac & vid
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_GetExactRecord(UI8_T *addr_entry);


/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetTotalCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total counter.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : numbers of total counter
 * NOTES  : Total counter means how many entries in this system.
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetTotalCounter(void);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetTotalDynamicCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total dynamic counter in system
 * INPUT  : None
 * OUTPUT : None
 * RETURN : numbers of total dynamic counter
 * NOTES  : Total counter means how many dynamic entries in this system.
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetTotalDynamicCounter();

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetTotalStaticCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total static counter in system.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : number of total static counter
 * NOTES  : Total counter means how many static entries in this system.
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetTotalStaticCounter();

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetStaticCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get static counter by specific port.
 * INPUT  : ifindex	-specific port
 * OUTPUT : None
 * RETURN : number of static counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetStaticCounterByPort(UI32_T ifindex);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetDynCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get dynamic counter by specific port.
 * INPUT  : ifindex	-specific port
 * OUTPUT : None
 * RETURN : number of dynamic counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetDynCounterByPort(UI32_T ifindex);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetDynCounterByVid
 *------------------------------------------------------------------------
 * PURPOSE: This function will get dynamic counter by specific vid.
 * INPUT  : vid	       -specific vid
 * OUTPUT : None
 * RETURN : number of dynamic counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetDynCounterByVid(UI32_T vid);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetLearntCounterByport
 *------------------------------------------------------------------------
 * PURPOSE: This function will get learnt counter by specific port.
 * INPUT  : ifindex	-specific port
 * OUTPUT : None
 * RETURN : number of learnt counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetLearntCounterByport(UI32_T ifindex);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetSecurityCounterByport
 *------------------------------------------------------------------------
 * PURPOSE: This function will get security counter by specific port.
 * INPUT  : ifindex	-specific port
 * OUTPUT : None
 * RETURN : number of security counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetSecurityCounterByport(UI32_T ifindex);

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetConfigCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get configuration addresses counter by specific port.
 * INPUT  : ifindex	-specific port
 * OUTPUT : None
 * RETURN : number of config counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetConfigCounterByPort(UI32_T ifindex);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_ClearCollisionVlanMacTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Remove all entries in the collision mac table.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_ClearCollisionVlanMacTable(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetNextEntryOfCollisionVlanMacTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Get next entry from the collision vlan mac table.
 * INPUT    : idx_p - The entry next to the value of *idx_p will be output
 * OUTPUT   : idx_p - The index of the output entry
 *            vlan_id_p - The vlan id of the collision mac address
 *            mac       - the collision mac address
 *            count_p   - the count of hash collision occurence
 * RETURN   : TRUE  - An entry is output sucessfully
 *            FALSE - No more entry to output.
 * NOTE     : To get the first entry, set value of *idx_p as
 *            AMTRDRV_MGR_FIRST_COLLISION_MAC_TABLE_ENTRY_INDEX.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_GetNextEntryOfCollisionVlanMacTable(UI8_T* idx_p, UI16_T* vlan_id_p, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN], UI16_T *count_p);

#if (SYS_CPNT_VXLAN == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_GetAndAlocRVfiMapToLVfi
 *------------------------------------------------------------------------------
 * Purpose  : Convert the specified real vfi to logical vfi. Allocate the new
 *            mapping between the real vfi and logical vfi when the mapping does
 *            not exist yet.
 * INPUT    : r_vfi   -  real VFI
 * OUTPUT   : l_vfi_p -  logical VFI
 * RETURN   : TRUE  - Success
 *            FALSE - Failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_GetAndAlocRVfiMapToLVfi(UI16_T r_vfi, UI16_T *l_vfi_p);

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_ConvertLvfiToRvfi
 *------------------------------------------------------------------------------
 * Purpose  : Convert the specified logical vfi to real vfi
 * INPUT    : l_vfi   -  logical vfi
 * OUTPUT   : r_vfi_p -  real vfi
 * RETURN   : TRUE  - Success
 *            FALSE - Failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_ConvertLvfiToRvfi(UI16_T l_vfi, UI16_T *r_vfi_p);

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_ConvertRvfiToLvfi
 *------------------------------------------------------------------------------
 * Purpose  : Convert the specified real vfi to logical vfi
 * INPUT    : r_vfi   -  real vfi
 * OUTPUT   : l_vfi_p -  logical vfi
 * RETURN   : TRUE  - Success
 *            FALSE - Failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_ConvertRvfiToLvfi(UI16_T r_vfi, UI16_T *l_vfi_p);

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_RemoveLvfiMap
 *------------------------------------------------------------------------------
 * Purpose  : Remove the specified logical vfi mapping
 * INPUT    : l_vfi  -  logical vfi
 * OUTPUT   : None
 * RETURN   : TRUE  - Success
 *            FALSE - Failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_RemoveLvfiMap(UI16_T l_vfi);
#endif /* end of #if (SYS_CPNT_VXLAN == TRUE) */

#endif /* end of #ifndef AMTRDRV_POM_H */
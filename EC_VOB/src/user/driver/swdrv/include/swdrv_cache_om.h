/* static char SccsId[] = "+-<>?!SWDRV_CACHE_OM.H     8/29/2002  09:00:00"; */
/* ------------------------------------------------------------------------+
 *  FILE NAME  -  SWDRV_CACHE_OM.H                                          +
 * ------------------------------------------------------------------------+
 *  Creater : Kelin                                                        +
 * ------------------------------------------------------------------------+
 *  Copyright(C)                Accton Corporation, 2002                   +
 * ------------------------------------------------------------------------*/

#ifndef	SWDRV_CACHE_OM_H
#define	SWDRV_CACHE_OM_H

#include "sys_adpt.h"
#include "sys_type.h"

#define SWDRV_CACHE_OM_TOTAL_NBR_OF_UPORT                (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK *  \
                                                         SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
                                             
#if 0                                             
#define SWDRV_CACHE_OM_TOTAL_NBR_OF_BYTE_FOR_PORT_LIST   (((((SWDRV_CACHE_OM_TOTAL_NBR_OF_UPORT+7)/8)+3)/4)*4)
#endif

/*Wingson 2004-10-19,size of SWDRV_CACHE_OM_Port_Db_Entry_T should be multiple of 4*/
#define SWDRV_CACHE_OM_TOTAL_NBR_OF_BYTE_FOR_PORT_LIST    (((((SYS_ADPT_TOTAL_NBR_OF_LPORT + 7) / 8)+3)/4)*4)
typedef struct
{
    UI8_T   member_port_list[SWDRV_CACHE_OM_TOTAL_NBR_OF_BYTE_FOR_PORT_LIST];       /* indicate member port list */
    UI8_T   untagged_port_list[SWDRV_CACHE_OM_TOTAL_NBR_OF_BYTE_FOR_PORT_LIST];     /* indicate untagged port list */
}   SWDRV_CACHE_OM_Port_Db_Entry_T;

typedef struct
{    
    UI32_T                          action;              /* indicate action of VLAN */
    SWDRV_CACHE_OM_Port_Db_Entry_T   *vlan_port_list_ptr; /* pointer to portList */
}SWDRV_CACHE_OM_Vlan_Entry_T;

#define SWDRV_CACHE_OM_NULL                      0

#define SWDRV_CACHE_OM_ACTION_TAG_MEMBER         1
#define SWDRV_CACHE_OM_ACTION_UNTAG_MEMBER       2

#define SWDRV_CACHE_OM_ACTION_NOTHING            0
#define SWDRV_CACHE_OM_ACTION_CREATE_VLAN        1
#define SWDRV_CACHE_OM_ACTION_DESTORY_VLAN       2
#define SWDRV_CACHE_OM_ACTION_MEMBER             3

#define SWDRV_CACHE_OM_ACTION_PROCESSING         0x10

/* ---------------------------------------------------------------------|
 *  ROUTINE NAME - SWDRV_CACHE_OM_Init                                   |
 * ---------------------------------------------------------------------|
 *  FUNCTION: This routine initialize gateway database, semaphore.      |
 *                                                                      |
 *  INPUT   : None                                                      |
 *  OUTPUT  : None                                                      |
 *  RETURN  : TRUE -- success. FALSE -- failure                         |
 *  NOTE    : None.                                                     |
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_OM_Init(void);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_CACHE_OM_ClearDataBase
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function is to clear the vlan table and vlan port table entryies.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. This function shall be invoked when system enters transition mode.
 *            2. All the entries in database will be purged.
 *-----------------------------------------------------------------------------------*/
void SWDRV_CACHE_OM_ClearDataBase(void);


/* ---------------------------------------------------------------------|
 *  ROUTINE NAME  - SWDRV_CACHE_OM_CreateVLAN                            |
 * ---------------------------------------------------------------------|
 *  FUNCTION : This routine is used to create the indicated vlan.       |
 *  INPUT   : vlan_id : VLAN ID                                         |
 *  OUTPUT  : None                                                      |
 *  RETURN  : TRUE -- success. FALSE -- vlan exist                      |
 *  NOTE    : vid range checking is done in MGR, OM don't check this.   |
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_OM_CreateVLAN(UI32_T vlan_id);


/* ---------------------------------------------------------------------|
 *  ROUTINE NAME  - SWDRV_CACHE_OM_DestoryVLAN                           |
 * ---------------------------------------------------------------------|
 *  FUNCTION: This routine is used to destory the indicated vlan.       |
 *  INPUT   : vlan_id - indicate which VLAN.                            |
 *  OUTPUT  : None                                                      |
 *  RETURN  : TRUE -- success. FALSE -- vlan not exist, can't delete    |
 *            default vlan.                                             |
 *  NOTE    : vid range checking is done in MGR, OM don't check this.   |
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_OM_DestoryVLAN(UI32_T vlan_id);


/* ---------------------------------------------------------------------|
 *  ROUTINE NAME  - SWDRV_CACHE_OM_AddMemberToVLAN    			        |
 * ---------------------------------------------------------------------|
 *  FUNCTION: This routine is used to add one port member to the        |
 *            indicated vlan.                                           |
 *  INPUT	: vlan_id : VLAN ID                                         |
 *            port_number : the port number that need to action.        |
 *            tag_flag: add tagged or untagged member.                  |
 *  OUTPUT  : None                                                      |
 *  RETURN  : TRUE -- success. FALSE -- failure                         |
 *  NOTE    :                                                           |
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_OM_AddMemberToVLAN(UI32_T vlan_id, UI32_T port_number, UI32_T tag_flag);


/* ---------------------------------------------------------------------|
 *  ROUTINE NAME  - SWDRV_CACHE_OM_RemoveMemberFromVLAN                  |
 * ---------------------------------------------------------------------|
 *  FUNCTION: This routine is used to remove one port member from the   |
 *            indicated vlan.                                           |
 *  INPUT	: vlan_id - indicate which VLAN.                            |
 *            port_number - indicate which port will be removed.        |
 *  OUTPUT	: None                                                      |
 *  RETURN	: TRUE -- success. FALSE -- failure                   		|
 *  NOTE    : None.	                                                    |
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_OM_RemoveMemberFromVLAN(UI32_T vlan_id, UI32_T port_number);


/* ---------------------------------------------------------------------|
 *  ROUTINE NAME  - SWDRV_CACHE_OM_GetNextVLANActionEntry   			|
 * ---------------------------------------------------------------------|
 *  FUNCTION : get next available/updated(Create/Add/Delete/Destroy)    |
 *             vlan member port list                                    |
 *  INPUT   : vlan_id : the port number that need to action.            |
 *  OUTPUT  : action :                                                  |
 *            member_port_list :                                        |
 *            untagged_port_list:                                       |
 *  RETURN  : TRUE -- success. FALSE -- no more updated vlan member port|
 *            list                                                      |
 *  NOTE    : 1. updated means must do create/destroy or set member port|
 *            list                                                      |
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_OM_GetNextVLANActionEntry(UI32_T *vlan_id, UI32_T *action, UI8_T *member_port_list, UI8_T *untagged_port_list);


/* ---------------------------------------------------------------------|
 *  ROUTINE NAME  - SWDRV_CACHE_OM_GetVLANActionEntry   			    |
 * ---------------------------------------------------------------------|
 *  FUNCTION : get next available/updated(Create/Add/Delete/Destroy)    |
 *             vlan member port list                                    |
 *  INPUT   : vlan_id : the port number that need to action.            |
 *  OUTPUT  : action :                                                  |
 *            member_port_list :                                        |
 *            untagged_port_list:                                       |
 *  RETURN  : TRUE -- success. FALSE -- no more updated vlan member port|
 *            list                                                      |
 *  NOTE    : 1. updated means must do create/destroy or set member port|
 *            list                                                      |
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_OM_GetVLANActionEntry(UI32_T vlan_id, UI32_T *action, UI8_T *member_port_list, UI8_T *untagged_port_list);


/* ---------------------------------------------------------------------|
 *  ROUTINE NAME  - SWDRV_CACHE_OM_RegisterVLANSuccess                   |
 * ---------------------------------------------------------------------|
 *  FUNCTION : This routine is used to reset action field of vlan entry |
 *  INPUT   : vlan_id : VLAN ID                                         |
 *  OUTPUT  : None                                                      |
 *  RETURN  : TRUE -- success. FALSE -- failure                         |
 *  NOTE    : 1. after MGR update chip register, use this function to   |
 *               reset the action field.                                |
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_OM_RegisterVLANSuccess(UI32_T vlan_id, UI32_T action, UI8_T *member_port_list, UI8_T *untagged_port_list);


/* ---------------------------------------------------------------------|
 *  ROUTINE NAME  - SWDRV_CACHE_OM_RegisterVLANFailure                   |
 * ---------------------------------------------------------------------|
 *  FUNCTION : This routine is used to reset action field of vlan entry |
 *  INPUT   : vlan_id : VLAN ID                                         |
 *  OUTPUT  : None                                                      |
 *  RETURN  : TRUE -- success. FALSE -- failure                         |
 *  NOTE    : 1. after MGR update chip register, use this function to   |
 *               reset the action field.                                |
 * ---------------------------------------------------------------------*/
BOOL_T SWDRV_CACHE_OM_RegisterVLANFailure(UI32_T vlan_id);


#endif /* SWDRV_CACHE_OM_H */


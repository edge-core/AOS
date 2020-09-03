/*-------------------------------------------------------------------------
 * Module Name: rspan_mgr.c
 *-------------------------------------------------------------------------
 * PURPOSE: Definitions for the RSPAN
 *-------------------------------------------------------------------------
 * NOTES:
 *
 *-------------------------------------------------------------------------
 * HISTORY:
 *    07/27/2007 - Tien Kuo, Created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-------------------------------------------------------------------------
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "rspan_mgr.h"
#include "rspan_om.h"
#include "sys_type.h"
#include "leaf_es3626a.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    RSPAN_OM_SessionEntry_T     rspan_session_db[RSPAN_MGR_MAX_SESSION_NUM] ; /* first comes, first saved. */
    RSPAN_OM_SessionEntry_T     local_mirror_db ;
    RSPAN_OM_RSPAN_VLAN_T    rspan_vid_db[SYS_ADPT_MAX_VLAN_ID];
    UI8_T mirror_session_tot;
    UI8_T src_uplink_tot [RSPAN_MGR_MAX_SESSION_NUM];
    BOOL_T is_uplink_used [RSPAN_MGR_MAX_SESSION_NUM]; /* For AMTR usage. 10/18/2007. */
}RSPAN_OM_ShmemData_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static RSPAN_OM_ShmemData_T* shmem_data_p;
static UI32_T rspan_om_sem_id;

/* EXPORTED SUBPROGRAM BODIES
 */

#define RSPAN_OM_EnterCriticalSection() SYSFUN_TakeSem(rspan_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define RSPAN_OM_LeaveCriticalSection() SYSFUN_GiveSem(rspan_om_sem_id)

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for BUFFERMGMT in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void RSPAN_OM_AttachSystemResources(void)
{
    shmem_data_p = (RSPAN_OM_ShmemData_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_RSPAN_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_RSPAN_OM, &rspan_om_sem_id);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - RSPAN_OM_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the RSPAN size for share memory
 * INPUT    : *segid_p
 *            *seglen_p
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void RSPAN_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p  = SYSRSC_MGR_RSPAN_SHMEM_SEGID;
    *seglen_p = sizeof(RSPAN_OM_ShmemData_T);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_InitDataBase
 *-------------------------------------------------------------------------
 * PURPOSE : Initialize RSPAN OM database
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void RSPAN_OM_InitDataBase(void)
{
    UI8_T i;

    UI32_T ret_value;

    shmem_data_p = (RSPAN_OM_ShmemData_T *)SYSRSC_MGR_GetShMem(SYSRSC_MGR_RSPAN_SHMEM_SEGID);

    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);

//    if ((ret_value = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_OM, &rspan_om_sem_id))!=SYSFUN_OK)
//    {
//        printf("%s: SYSFUN_GetSem return != SYSFUN_OK value=%lu\n",__FUNCTION__,ret_value);
//        return FALSE;
//    }

//    RSPAN_OM_EnterCriticalSection();
    memset(shmem_data_p->rspan_vid_db, 0, (sizeof(RSPAN_OM_RSPAN_VLAN_T))* SYS_ADPT_MAX_VLAN_ID );
    memset(shmem_data_p->rspan_session_db, 0, (sizeof(RSPAN_OM_SessionEntry_T)) * RSPAN_MGR_MAX_SESSION_NUM );
    memset(&shmem_data_p->local_mirror_db, 0, sizeof(RSPAN_OM_SessionEntry_T));
    shmem_data_p->mirror_session_tot = 0 ;

    for (i=0; i<RSPAN_MGR_MAX_SESSION_NUM; i++)
    {
        shmem_data_p->src_uplink_tot [i] = 0 ;
        shmem_data_p->is_uplink_used [i] = FALSE ;
    }
//    RSPAN_OM_LeaveCriticalSection();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void RSPAN_OM_EnterMasterMode (void)
{
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void RSPAN_OM_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Set transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void RSPAN_OM_SetTransitionMode (void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void RSPAN_OM_EnterTransitionMode (void)
{
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_OM_GetOperatingMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Get operating mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
UI32_T RSPAN_OM_GetOperatingMode()
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_SetRspanVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set RSPAN VLAN entry in RSPAN database.
 * INPUT    : vid         -- The new created vlan id
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
void RSPAN_OM_SetRspanVlanEntry(UI32_T vid)
{
    RSPAN_OM_EnterCriticalSection();

    shmem_data_p->rspan_vid_db[(vid-1)].is_rspan_vid = TRUE ;

    RSPAN_OM_LeaveCriticalSection();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_DeleteRspanVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will delete RSPAN VLAN entry in RSPAN database.
 * INPUT    : vid         -- The specific RSPAN vlan id
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
void RSPAN_OM_DeleteRspanVlanEntry(UI32_T vid)
{
    RSPAN_OM_EnterCriticalSection();

    shmem_data_p->rspan_vid_db[(vid-1)].is_rspan_vid = FALSE ;

    RSPAN_OM_LeaveCriticalSection();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsRspanVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will find the specific RSPAN VLAN entry from RSPAN database.
 * INPUT    : vid         -- The specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE        -- RSPAN VLAN is found
 *            FALSE       -- RSPAN VLAN isn't found
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsRspanVlan(UI32_T vid)
{
    RSPAN_OM_EnterCriticalSection();

    if ( shmem_data_p->rspan_vid_db[(vid-1)].is_rspan_vid == FALSE )
    {
        RSPAN_OM_LeaveCriticalSection();
        return FALSE;
    }

    RSPAN_OM_LeaveCriticalSection();
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_SettingValidation
 *--------------------------------------------------------------------------
 * PURPOSE  : Check if items of a session entry are valid to set the database.
 * INPUT    : The pointer of the session structure.
 *            target_port -- the specific port needs to be validated.
 *                           (from 1-SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
 * OUTPUT   : None
 * RETURN   : TRUE        -- The entry is valid for setting the database.
 *            FALSE       -- The entry is not valid.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_SettingValidation ( RSPAN_OM_SessionEntry_T *is_rspan_entry_valid, UI8_T target_port )
{
    UI8_T i;
    UI8_T byte = 0 ;
    UI8_T shift = 0 ;

    RSPAN_OM_EnterCriticalSection();

    if ( (shmem_data_p->mirror_session_tot <= RSPAN_MGR_MAX_SESSION_NUM) && shmem_data_p->mirror_session_tot && target_port) /* session is available */
    {
        byte = L_CVRT_BYTE_INDEX_OF_PORTLIST ( target_port ) ;
        shift = L_CVRT_BIT_INDEX_IN_BYTE_OF_PORTLIST ( target_port ) ;

#if (SYS_CPNT_SYSCTRL_XOR == FALSE)
        /* Validate Local Port Monitor session record, but if the validation is from Local session, then
           this part will be passed because Local session will do the validation in swctrl.c */
        if ( shmem_data_p->local_mirror_db.session_id
            && (is_rspan_entry_valid->session_id != ( RSPAN_MGR_MAX_SESSION_NUM + 1)))
        {
            if ( shmem_data_p->local_mirror_db.dst == target_port )
            {
                RSPAN_OM_LeaveCriticalSection();
                return FALSE;
            }
            if ( shmem_data_p->local_mirror_db.src_tx[byte] & (0x01 << shift) )
            {
                RSPAN_OM_LeaveCriticalSection();
                return FALSE;
            }
            if ( shmem_data_p->local_mirror_db.src_rx[byte] & (0x01 << shift) )
            {
                RSPAN_OM_LeaveCriticalSection();
                return FALSE;
            }
        }
        else  /* Validate RSPAN session */
#endif /*#if (SYS_CPNT_SYSCTRL_XOR == FALSE)*/
        {
            for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
            {
                if ( shmem_data_p->rspan_session_db[i].session_id )
                {
                    /* src can't be dst and uplink in the other session. */
                    /* uplink, dst ports can't be source, destination and uplink in other session. */
                    if ( shmem_data_p->rspan_session_db[i].session_id != is_rspan_entry_valid->session_id )
                    {
                        /* This RSPAN VLAN is used in the other session. */
                        if ( shmem_data_p->rspan_session_db[i].remote_vid == is_rspan_entry_valid->remote_vid
                            && ( is_rspan_entry_valid->remote_vid) )
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE;
                        }
                        if ( shmem_data_p->rspan_session_db[i].dst == target_port )
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE;
                        }
                        if ( shmem_data_p->rspan_session_db[i].src_tx[byte] & (0x01 << shift) )
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE;
                        }
                        if ( shmem_data_p->rspan_session_db[i].src_rx[byte] & (0x01 << shift) )
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE;
                        }
                        if ( shmem_data_p->rspan_session_db[i].uplink[byte] & (0x01 << shift) )
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE;
                        }
                    }

                    /* src, dst and uplink can't be mixed up. in the same session. */
                    if ( shmem_data_p->rspan_session_db[i].session_id == is_rspan_entry_valid->session_id )
                    {
                    	/* remote vid can't be overwritten. users should delete remote vid first. */
                    	if ( shmem_data_p->rspan_session_db[i].remote_vid && is_rspan_entry_valid->remote_vid
                    		&& (shmem_data_p->rspan_session_db[i].remote_vid != is_rspan_entry_valid->remote_vid) )
                        {
                            RSPAN_OM_LeaveCriticalSection();
                    		return FALSE;
                        }
                        if ( ( shmem_data_p->rspan_session_db[i].dst == target_port ) &&  ! is_rspan_entry_valid->dst )
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE;
                        }
                        if ( shmem_data_p->rspan_session_db[i].src_tx[byte] & (0x01 << shift)
                             && ! is_rspan_entry_valid->src_tx[0] && ! is_rspan_entry_valid->src_rx[0] )
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE;
                        }
                        if ( shmem_data_p->rspan_session_db[i].src_rx[byte] & (0x01 << shift)
                            && ! is_rspan_entry_valid->src_tx[0] && ! is_rspan_entry_valid->src_rx[0] )
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE;
                        }
                        if ( shmem_data_p->rspan_session_db[i].uplink[byte] & (0x01 << shift)
                            && ! is_rspan_entry_valid->uplink[0] )
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE;
                        }
                    }
                }
            }
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return TRUE;

}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_GetRspanSessionEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the RSPAN entry of the specific session from RSPAN database.
 * INPUT    : session_id  -- The specific session id.
 *            *rspan_entry -- The RSPAN entry pointer.
 * OUTPUT   : *rspan_entry -- The whole data structure with the specific entry.
 * RETURN   : TRUE         -- The configuration is set successfully.
 *            FALSE        -- The configuration isn't set successfully.
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_GetRspanSessionEntry(UI8_T session_id, RSPAN_OM_SessionEntry_T *rspan_entry)
{
    UI8_T i;

    RSPAN_OM_EnterCriticalSection();

    if ( NULL != rspan_entry )
    {
        if (  shmem_data_p->mirror_session_tot ) /* session is available */
        {
            for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
            {
                if ( shmem_data_p->rspan_session_db[i].session_id == session_id )
                {
                    memcpy(rspan_entry, &shmem_data_p->rspan_session_db[i], sizeof(RSPAN_OM_SessionEntry_T));
                    RSPAN_OM_LeaveCriticalSection();
                    return TRUE;
                }
            }
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_SetRspanSessionEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set RSPAN session entry in RSPAN_OM.
 * INPUT    : The whole data structure to store the value
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_SetRspanSessionEntry( RSPAN_OM_SessionEntry_T *rspan_entry )
{
    UI8_T i;
    BOOL_T found = FALSE ;

    RSPAN_OM_EnterCriticalSection();

    if ( NULL != rspan_entry)
    {
        if ( shmem_data_p->mirror_session_tot == 0 ) /* No record saved. */
        {
            if ( rspan_entry->session_id <= RSPAN_MGR_MAX_SESSION_NUM )
                memcpy( &shmem_data_p->rspan_session_db[0], rspan_entry, sizeof(RSPAN_OM_SessionEntry_T));

            else if ( rspan_entry->session_id == ( RSPAN_MGR_MAX_SESSION_NUM + 1 ) ) /* Local Port Monitor - Add */
            {
                /* Add a new Local Port Monitor session. */
                memcpy( &shmem_data_p->local_mirror_db, rspan_entry, sizeof(RSPAN_OM_SessionEntry_T));
            }
        }
        else if ( shmem_data_p->mirror_session_tot <= RSPAN_MGR_MAX_SESSION_NUM ) /* The total sessions are not over RSPAN_MGR_MAX_SESSION_NUM. */
        {
            /* At least one record saved. */
            if ( rspan_entry->session_id == ( RSPAN_MGR_MAX_SESSION_NUM + 1 ) )
            {
                /* Local Port Monitor - Modification */
                if ( shmem_data_p->local_mirror_db.session_id == rspan_entry->session_id )
                {
                    memcpy( &shmem_data_p->local_mirror_db, rspan_entry, sizeof(RSPAN_OM_SessionEntry_T));
                    RSPAN_OM_LeaveCriticalSection();
                    return TRUE;
                }
                else if ( shmem_data_p->mirror_session_tot < RSPAN_MGR_MAX_SESSION_NUM )
                {   /* Local Port Monitor - Addition */
                    memcpy( &shmem_data_p->local_mirror_db, rspan_entry, sizeof(RSPAN_OM_SessionEntry_T));
                }
                else
                {
                    RSPAN_OM_LeaveCriticalSection();
                    return FALSE; /* there are already two session existed. */
                }
            }

            for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
            {
                if ( shmem_data_p->rspan_session_db[i].session_id == rspan_entry->session_id ) /* modification for RSPAN */
                {
                    memcpy( &shmem_data_p->rspan_session_db[i], rspan_entry, sizeof(RSPAN_OM_SessionEntry_T));
                    RSPAN_OM_LeaveCriticalSection();
                    return TRUE ;
                }
            }

            if (shmem_data_p->mirror_session_tot < RSPAN_MGR_MAX_SESSION_NUM)
            {
                for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
                {
                    if ( ! shmem_data_p->rspan_session_db[i].session_id ) /* Add a new RSPAN session. */
                    {
                        if ( rspan_entry->session_id <= RSPAN_MGR_MAX_SESSION_NUM
                            && rspan_entry->session_id )
                            memcpy( &shmem_data_p->rspan_session_db[i], rspan_entry, sizeof(RSPAN_OM_SessionEntry_T));
                        found = TRUE ;
                        break ;
                    } /* end-if */
                } /* end-for */
                if ( found == FALSE )  /* session id is not allowed because two sessions are occupied. */
                {
                    RSPAN_OM_LeaveCriticalSection();
                    return FALSE;
                }
            }
            if ( found == FALSE )  /* session id is not allowed because two sessions are occupied. */
            {
                RSPAN_OM_LeaveCriticalSection();
                return FALSE;
            }
        } /* end-else-if */
        else /* The total sessions are over RSPAN_MGR_MAX_SESSION_NUM. */
        {
            RSPAN_OM_LeaveCriticalSection();
            return FALSE;
        }

        shmem_data_p->mirror_session_tot++ ;

        RSPAN_OM_LeaveCriticalSection();
        return TRUE;
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_DeleteRspanSessionEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete RSPAN session entry in RSPAN_OM.
 * INPUT    : The whole data structure to delete the value
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is deleted successfully.
 *            FALSE       -- The configuration isn't deleted successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_DeleteRspanSessionEntry( RSPAN_OM_SessionEntry_T *rspan_entry )
{
    UI8_T   i = 0 ;
    BOOL_T  dst_found = FALSE ;
    BOOL_T  src_found = FALSE ;
	UI8_T   compared[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0,} ;

    RSPAN_OM_EnterCriticalSection();

    if ( ( NULL != rspan_entry ) && shmem_data_p->mirror_session_tot )
    {
        if ( rspan_entry->session_id == ( RSPAN_MGR_MAX_SESSION_NUM + 1 ) ) /* Local Port Monitor */
        {
            /* Local Port Monitor doesn't have uplink config. */
            if ( (rspan_entry->uplink[0] == 0) && rspan_entry->dst && ( rspan_entry->src_tx[0] || rspan_entry->src_rx[0] ))
            {
                if ( shmem_data_p->local_mirror_db.dst == rspan_entry->dst )
                    dst_found = TRUE ;

                if ( RSPAN_OM_IS_MEMBER(shmem_data_p->local_mirror_db.src_tx, rspan_entry->src_tx[0]) && (dst_found == TRUE) )
                {
                    RSPAN_OM_DEL_MEMBER(shmem_data_p->local_mirror_db.src_tx, rspan_entry->src_tx[0]) ;
                    src_found = TRUE ;
                }

                if ( RSPAN_OM_IS_MEMBER(shmem_data_p->local_mirror_db.src_rx, rspan_entry->src_rx[0]) && (dst_found == TRUE) )
                {
                    RSPAN_OM_DEL_MEMBER(shmem_data_p->local_mirror_db.src_rx, rspan_entry->src_rx[0]) ;
                    src_found = TRUE ;
                }

                if ( src_found == TRUE && dst_found == TRUE )
                {
                    shmem_data_p->local_mirror_db.dst = 0 ;
				    memset(&shmem_data_p->local_mirror_db, 0, sizeof(RSPAN_OM_SessionEntry_T));
					shmem_data_p->mirror_session_tot -- ;
                    RSPAN_OM_LeaveCriticalSection();
					return TRUE ;
                }
                else
                {
                    RSPAN_OM_LeaveCriticalSection();
                    return FALSE ;
                }
            }
            else
            {
                RSPAN_OM_LeaveCriticalSection();
                return FALSE ;
            }
        }
        else /* Remote Session */
        {
            for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
            {
                if ( shmem_data_p->rspan_session_db[i].session_id == rspan_entry->session_id )
                {
                    /* Remote VLAN and uplink operation. */
                    if ( rspan_entry->switch_role && rspan_entry->remote_vid && rspan_entry->uplink[0] )
                    {
                        if ( shmem_data_p->rspan_session_db[i].switch_role == rspan_entry->switch_role
                             && shmem_data_p->rspan_session_db[i].remote_vid == rspan_entry->remote_vid )
                        {
                            if ( RSPAN_OM_IS_MEMBER(shmem_data_p->rspan_session_db[i].uplink, rspan_entry->uplink[0]) )
                            {
                                RSPAN_OM_DEL_MEMBER(shmem_data_p->rspan_session_db[i].uplink, rspan_entry->uplink[0]) ;

								/* Check if any uplink ports exist. */
                                if ( ! memcmp (compared, shmem_data_p->rspan_session_db[i].uplink, sizeof(shmem_data_p->rspan_session_db[i].uplink)) )
								{
									shmem_data_p->rspan_session_db[i].switch_role = 0 ;
									shmem_data_p->rspan_session_db[i].remote_vid = 0 ;

                                    RSPAN_OM_LeaveCriticalSection();
									return TRUE ;
								}
                                RSPAN_OM_LeaveCriticalSection();
                                return TRUE ; /* Delete one of member ports and there are still having other uplink ports. */
                            }
                            else
                            {
                                RSPAN_OM_LeaveCriticalSection();
                                return FALSE ;
                            }
                        }
                        else
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE ;
                        }
                    }

                    /* source interface operation. */
                    if ( rspan_entry->src_tx[0] || rspan_entry->src_rx[0] )
                    {
                        /* Since L_CVRT_IS_MEMBER_OF_PORTLIST doesn't check if target port is more than 1,
                         * then L_CVRT_DEL_MEMBER_FROM_PORTLIST will cause the problem if target port is 0.
                         */
                        if ( rspan_entry->src_tx[0] > 0 )
                        {
                            if ( RSPAN_OM_IS_MEMBER(shmem_data_p->rspan_session_db[i].src_tx, rspan_entry->src_tx[0]) )
                            {
                                RSPAN_OM_DEL_MEMBER(shmem_data_p->rspan_session_db[i].src_tx, rspan_entry->src_tx[0]) ;
                                src_found = TRUE ;
                            }
                        }

                        if ( rspan_entry->src_rx[0] > 0 )
                        {
                            if ( RSPAN_OM_IS_MEMBER(shmem_data_p->rspan_session_db[i].src_rx, rspan_entry->src_rx[0]) )
                            {
                                RSPAN_OM_DEL_MEMBER(shmem_data_p->rspan_session_db[i].src_rx, rspan_entry->src_rx[0]) ;
                                src_found = TRUE ;
                            }
                        }

                        if ( src_found == FALSE )
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE ;
                        }
                        else
                        {
                            RSPAN_OM_LeaveCriticalSection();
							return TRUE ;
                        }
                    }

                    /* destination interface operation. */
                    if ( rspan_entry->dst )
                    {
                        if (shmem_data_p->rspan_session_db[i].dst == rspan_entry->dst )
                        {
                            rspan_entry->is_tagged = shmem_data_p->rspan_session_db[i].is_tagged ; /* For taking out vlan membership usage.*/
                            shmem_data_p->rspan_session_db[i].dst = 0 ;
                            shmem_data_p->rspan_session_db[i].is_tagged = 0 ;

                            RSPAN_OM_LeaveCriticalSection();
                            return TRUE ;
                        }
                        else
                        {
                            RSPAN_OM_LeaveCriticalSection();
                            return FALSE ;
                        }
                    }
                } /* if session_id */
            } /* for */
        } /* else 'remote session' */
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsSessionEntryCompleted
 *--------------------------------------------------------------------------
 * PURPOSE  : Check if items of a session entry are ready to set the chip.
 * INPUT    : The pointer of the session structure.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The entry is ready for setting the chip.
 *            FALSE       -- The entry is not completed.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsSessionEntryCompleted( RSPAN_OM_SessionEntry_T *rspan_entry )
{
    UI8_T i;
    UI32_T j = 0 ;
    UI8_T port_bit_count[3] = {0} ;
    UI32_T is_completed = 0;

    RSPAN_OM_EnterCriticalSection();

    for (i = 0; (i < RSPAN_MGR_MAX_SESSION_NUM) && (is_completed == 0); i++)
    {
        if ( shmem_data_p->rspan_session_db[i].session_id == rspan_entry->session_id )
        {
            if ( shmem_data_p->rspan_session_db[i].switch_role == VAL_rspanSwitchRole_source
                && shmem_data_p->rspan_session_db[i].remote_vid )
            {
                for (j = 0 ; j < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST ; j++)
                {
                    if ( port_bit_count[0] == 0 )
                    {
                        if ( shmem_data_p->rspan_session_db[i].src_tx[j] )
                            port_bit_count[0] = 1 ;
                    }
                    if ( port_bit_count[1] == 0 )
                    {
                        if ( shmem_data_p->rspan_session_db[i].src_rx[j] )
                            port_bit_count[1] = 1 ;
                    }
                    if ( port_bit_count[2] == 0 )
                    {
                        if ( shmem_data_p->rspan_session_db[i].uplink[j] )
                            port_bit_count[2] = 1 ;
                    }
                    if ( port_bit_count[2] && (port_bit_count[0] || port_bit_count[1]) )
                    {
                        is_completed = i+1 ;
                        break ;
                    }
                }
            }

            if ( shmem_data_p->rspan_session_db[i].switch_role == VAL_rspanSwitchRole_intermediate
                && shmem_data_p->rspan_session_db[i].remote_vid )
            {
                for (j = 0 ; j < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST ; j++)
                {
                    if (port_bit_count[2] == 0)
                    {
                        if (shmem_data_p->rspan_session_db[i].uplink[j])
                        {
                            port_bit_count[2] = 1 ;
                            is_completed = i+1 ;
                            break ;
                        }
                    }
                }
            }

            if ( shmem_data_p->rspan_session_db[i].switch_role == VAL_rspanSwitchRole_destination
                && shmem_data_p->rspan_session_db[i].remote_vid
                && shmem_data_p->rspan_session_db[i].dst
                && shmem_data_p->rspan_session_db[i].is_tagged )
            {
                for (j = 0 ; j < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST ; j++)
                {
                    if (port_bit_count[2] == 0)
                    {
                        if (shmem_data_p->rspan_session_db[i].uplink[j])
                        {
                            port_bit_count[2] = 1;
                            is_completed = i+1;
                            break;
                        }
                    }
                }
            }
        }
    }

    if ( is_completed > 0 )
    {
        if ( shmem_data_p->rspan_session_db[is_completed-1].switch_role == VAL_rspanSwitchRole_source
            && port_bit_count[2] && (port_bit_count[0] || port_bit_count[1]))
        {
            RSPAN_OM_LeaveCriticalSection();
            return TRUE;
        }
        if ( shmem_data_p->rspan_session_db[is_completed-1].switch_role == VAL_rspanSwitchRole_intermediate
            && port_bit_count[2] )
        {
            RSPAN_OM_LeaveCriticalSection();
            return TRUE;
        }
        if ( shmem_data_p->rspan_session_db[is_completed-1].switch_role == VAL_rspanSwitchRole_destination
            && port_bit_count[2] )
        {
            RSPAN_OM_LeaveCriticalSection();
            return TRUE;
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_RemoveSessionId
 *--------------------------------------------------------------------------
 * PURPOSE  : Remove a session id when all items of this entry are empty.
 * INPUT    : session_id : The specific session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The session id is removed.
 *            FALSE       -- The session id is not removed or not found.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_RemoveSessionId( UI8_T session_id )
{
    UI8_T   i;
    UI8_T   compared[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0,} ;

    RSPAN_OM_EnterCriticalSection();

    for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
    {
        if ( shmem_data_p->rspan_session_db[i].session_id == session_id )
        {
            if ( memcmp (compared, shmem_data_p->rspan_session_db[i].src_tx, sizeof(shmem_data_p->rspan_session_db[i].src_tx)) )
            {
                RSPAN_OM_LeaveCriticalSection();
                return FALSE;
            }
            if ( memcmp (compared, shmem_data_p->rspan_session_db[i].src_rx, sizeof(shmem_data_p->rspan_session_db[i].src_rx)) )
            {
                RSPAN_OM_LeaveCriticalSection();
                return FALSE;
            }
            if ( memcmp (compared, shmem_data_p->rspan_session_db[i].uplink, sizeof(shmem_data_p->rspan_session_db[i].uplink)) )
            {
                RSPAN_OM_LeaveCriticalSection();
                return FALSE;
            }
            if ( shmem_data_p->rspan_session_db[i].dst || shmem_data_p->rspan_session_db[i].switch_role || shmem_data_p->rspan_session_db[i].is_tagged
                 || shmem_data_p->rspan_session_db[i].remote_vid )
            {
                RSPAN_OM_LeaveCriticalSection();
                return FALSE;
            }

            shmem_data_p->rspan_session_db[i].session_id = 0 ;

            memset(&shmem_data_p->rspan_session_db[i], 0, (sizeof(RSPAN_OM_SessionEntry_T)) );

            shmem_data_p->mirror_session_tot -- ;

            RSPAN_OM_LeaveCriticalSection();
            return TRUE;
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE; /* Session id is not found*/
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_RemoveSessionIdEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Remove a session id and all items of this session id.
 * INPUT    : session_id : The specific session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The session id and entry infor. are removed.
 *            FALSE       -- The session id and entry infor. are not removed
 *                           because session id is not found.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_RemoveSessionIdEntry( UI8_T session_id )
{
    UI8_T   i;

    RSPAN_OM_EnterCriticalSection();

    for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
    {
        if ( shmem_data_p->rspan_session_db[i].session_id == session_id )
        {
            memset(&shmem_data_p->rspan_session_db[i], 0, (sizeof(RSPAN_OM_SessionEntry_T)) );
            shmem_data_p->rspan_session_db[i].session_id = 0 ;
            shmem_data_p->mirror_session_tot -- ;

            RSPAN_OM_LeaveCriticalSection();
            return TRUE;
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE; /* Session id is not found*/
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_GetSessionEntryCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the number of RSPAN entries from RSPAN database.
 * INPUT    : *session_cnt -- The total session numbers.
 * OUTPUT   : *session_cnt -- The total session numbers.
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void RSPAN_OM_GetSessionEntryCounter(UI8_T *session_cnt)
{
    RSPAN_OM_EnterCriticalSection();

    *session_cnt = shmem_data_p->mirror_session_tot ;

    RSPAN_OM_LeaveCriticalSection();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IncreaseSrcUplinkCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : Increase source uplink counters.
 * INPUT    : session_id : The specific session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- Increase source uplink couners successfully.
 *            FALSE       -- Doesn't increase source uplink couners successfully.
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IncreaseSrcUplinkCounter (UI8_T session_id)
{
    UI8_T *src_target_uplink_tot = NULL ;

    RSPAN_OM_EnterCriticalSection();

    src_target_uplink_tot = &shmem_data_p->src_uplink_tot [session_id-1] ;

    if ( (*src_target_uplink_tot) < RSPAN_MGR_MAX_SRC_UPLINK_NUM )
    {
        (*src_target_uplink_tot) ++ ;

        RSPAN_OM_LeaveCriticalSection();
        return TRUE;
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE;
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_DecreaseSrcUplinkCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : Decrease source uplink counters.
 * INPUT    : session_id : The specific session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- Decrease source uplink couners successfully.
 *            FALSE       -- Doesn't decrease source uplink couners successfully.
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_DecreaseSrcUplinkCounter (UI8_T session_id)
{
    UI8_T *src_target_uplink_tot = NULL ;

    RSPAN_OM_EnterCriticalSection();

    src_target_uplink_tot = &shmem_data_p->src_uplink_tot [session_id-1] ;

    if ( *src_target_uplink_tot )
    {
        (*src_target_uplink_tot) -- ;

        RSPAN_OM_LeaveCriticalSection();
        return TRUE;
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE;
}

/* 802.1X and port security will need this. 2007/09/20 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsRspanUplinkPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN mirrored port
 *           FALSE: The ifindex is not a RSPAN mirrored port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsRspanUplinkPort(UI32_T ifindex)
{
    UI8_T   i = 0 ;

    RSPAN_OM_EnterCriticalSection();

    for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
    {
        if ( RSPAN_OM_IS_MEMBER( shmem_data_p->rspan_session_db[i].uplink, ifindex ) )
        {
            RSPAN_OM_LeaveCriticalSection();
            return TRUE ;
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return  FALSE ;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_SetRspanUplinkPortUsage
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the status of the usage of uplink ports
 *           by session.
 * INPUT   : session id
 *           ret        -- the current status needs to set in OM.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This data is used for AMTR to check if it needs to enable/disable
 *           port learning of RSPAN uplink ports.
 * -------------------------------------------------------------------------
 */
void RSPAN_OM_SetRspanUplinkPortUsage ( UI8_T session_id, BOOL_T ret )
{
    RSPAN_OM_EnterCriticalSection();

    if ( shmem_data_p->is_uplink_used [session_id-1] != ret )
        shmem_data_p->is_uplink_used [session_id-1] = ret ;

    RSPAN_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsRspanUplinkPortUsed
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN mirrored port and ready to perform
 *                  RSPAN.
 *           FALSE: The ifindex is not a RSPAN mirrored port or not ready to
 *                  perform RSPAN.
 * NOTE    : This API is used for AMTR to check if it needs to enable/disable
 *           port learning of RSPAN uplink ports.
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsRspanUplinkPortUsed ( UI32_T ifindex )
{
    BOOL_T ret = FALSE ;
    UI16_T i;

    RSPAN_OM_EnterCriticalSection();

    for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
    {
        if ( RSPAN_OM_IS_MEMBER( shmem_data_p->rspan_session_db[i].uplink, ifindex ) )
        {
            if ( shmem_data_p->is_uplink_used [i] == TRUE )
                ret = TRUE ;

            RSPAN_OM_LeaveCriticalSection();
            return ( ret ) ;
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return ( ret ) ;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_ValidateRspanPortRelation
 * -------------------------------------------------------------------------
 * FUNCTION: This function tests if the port of this port list isn't used by
 *           the other session or other field.
 * INPUT   : session_id     -- RSPAN session ID
 *           ifindex     -- the port which user configures for rspan field
 *           port_list_type -- LEAF_rspanSrcTxPorts  2
 *                             LEAF_rspanSrcRxPorts  3
                               LEAF_rspanDstPort     4
 *                             LEAF_rspanRemotePorts 7
 * OUTPUT  : None
 * RETURN  : TRUE : The port is valid to set in this session.
 *           FALSE: The port isn't valid to set in this session.
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_ValidateRspanPortRelation ( UI8_T session_id, UI8_T ifindex , UI8_T port_list_type )
{
    UI8_T   i;

    RSPAN_OM_EnterCriticalSection();

    for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
    {
	/* Can allow tx/rx to configure as the same ports and re-configure the same setting in the same session. */
        if ( RSPAN_OM_IS_MEMBER ( shmem_data_p->rspan_session_db[i].src_tx, ifindex )
            && ( shmem_data_p->rspan_session_db[i].session_id != session_id
            || ( port_list_type != LEAF_rspanSrcRxPorts && port_list_type != LEAF_rspanSrcTxPorts ) ) )
        {
            RSPAN_OM_LeaveCriticalSection();
            return FALSE ;
        }

	/* Can allow tx/rx to configure as the same ports and re-configure the same setting in the same session. */
        if ( RSPAN_OM_IS_MEMBER ( shmem_data_p->rspan_session_db[i].src_rx, ifindex )
            && ( shmem_data_p->rspan_session_db[i].session_id != session_id
            || ( port_list_type != LEAF_rspanSrcRxPorts && port_list_type != LEAF_rspanSrcTxPorts ) ) )
        {
            RSPAN_OM_LeaveCriticalSection();
            return FALSE ;
        }

        if ( RSPAN_OM_IS_MEMBER( shmem_data_p->rspan_session_db[i].uplink, ifindex )
            && ( shmem_data_p->rspan_session_db[i].session_id != session_id || port_list_type != LEAF_rspanRemotePorts ) )
        {
            RSPAN_OM_LeaveCriticalSection();
            return FALSE ;
        }

        if ( shmem_data_p->rspan_session_db[i].dst == ifindex
            && ( shmem_data_p->rspan_session_db[i].session_id != session_id || port_list_type != LEAF_rspanDstPort ) )
        {
            RSPAN_OM_LeaveCriticalSection();
            return FALSE ;
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return TRUE ;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsSwitchRoleValid
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether if the switch role is vaild to set
 *           in the OM from SNMP.
 * INPUT   : session id
 *           switch_role -- VAL_rspanSwitchRole_source       1L
 *                          VAL_rspanSwitchRole_intermediate 2L
 *                          VAL_rspanSwitchRole_destination  3L
 * OUTPUT  : None
 * RETURN  : TRUE        -- The configuration is set successfully.
 *           FALSE       -- The configuration isn't set successfully.
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsSwitchRoleValid ( UI8_T session_id, UI8_T switch_role )
{
    UI8_T   j;

    RSPAN_OM_EnterCriticalSection();

    if ( switch_role == VAL_rspanSwitchRole_source || switch_role == VAL_rspanSwitchRole_intermediate )
    {
        if ( shmem_data_p->rspan_session_db[session_id-1].dst != 0 )
        {
            RSPAN_OM_LeaveCriticalSection();
            return  FALSE ;
        }
    }

    for (j = 0 ; j < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST ; j++)
    {
        if ( switch_role == VAL_rspanSwitchRole_destination || switch_role == VAL_rspanSwitchRole_intermediate )
        {
            if ( ( shmem_data_p->rspan_session_db[session_id-1].src_tx[j] != 0 ) || ( shmem_data_p->rspan_session_db[session_id-1].src_rx[j] != 0 ) )
            {
                RSPAN_OM_LeaveCriticalSection();
                return  FALSE ;
            }
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return TRUE ;
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsLocalMirrorPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to test whether the ufindex is a mirrored
 *            source or destination port.
 * INPUT    : ifindex -- this interface index
 * OUTPUT   : None
 * RETURN   : TRUE : The ifindex is a mirrored source or destination port
 *            FALSE: The ifindex is not a mirrored source or destination port
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsLocalMirrorPort( UI32_T ifindex )
{
    RSPAN_OM_EnterCriticalSection();

    if ( RSPAN_OM_IS_MEMBER ( shmem_data_p->local_mirror_db.src_tx, ifindex ) )
    {
        RSPAN_OM_LeaveCriticalSection();
        return TRUE ;
    }
    if ( RSPAN_OM_IS_MEMBER ( shmem_data_p->local_mirror_db.src_rx, ifindex ) )
    {
        RSPAN_OM_LeaveCriticalSection();
        return TRUE ;
    }
    if ( shmem_data_p->local_mirror_db.dst == ifindex )
    {
        RSPAN_OM_LeaveCriticalSection();
        return TRUE ;
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE ;

}

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsRspanMirrorToPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN
 *           destination port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN destination port
 *           FALSE: The ifindex is not a RSPAN destination port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsRspanMirrorToPort(UI32_T ifindex)
{
    UI8_T   i;

    RSPAN_OM_EnterCriticalSection();

    for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
    {
        if ( shmem_data_p->rspan_session_db[i].dst == ifindex )
        {
            RSPAN_OM_LeaveCriticalSection();
            return TRUE ;
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE ;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_IsRspanMirroredPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN mirrored port
 *           FALSE: The ifindex is not a RSPAN mirrored port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_IsRspanMirroredPort(UI32_T ifindex)
{
    UI8_T   i;

    RSPAN_OM_EnterCriticalSection();

    for (i = 0 ; i < RSPAN_MGR_MAX_SESSION_NUM ; i++)
    {
        if ( RSPAN_OM_IS_MEMBER ( shmem_data_p->rspan_session_db[i].src_tx, ifindex ) )
        {
            RSPAN_OM_LeaveCriticalSection();
            return TRUE ;
        }
        if ( RSPAN_OM_IS_MEMBER ( shmem_data_p->rspan_session_db[i].src_rx, ifindex ) )
        {
            RSPAN_OM_LeaveCriticalSection();
            return TRUE ;
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE ;
}
#endif /*#if (SYS_CPNT_SYSCTRL_XOR == TRUE)*/

#if(RSPAN_BACK_DOOR == TRUE)
/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_OM_GetLocalSessionEntryForBackdoor
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the RSPAN entry of the specific session from RSPAN database.
 * INPUT    : *rspan_entry -- The Local entry pointer.
 * OUTPUT   : *rspan_entry -- The whole data structure with the specific entry.
 * RETURN   : TRUE         -- The configuration is set successfully.
 *            FALSE        -- The configuration isn't set successfully.
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_OM_GetLocalSessionEntryForBackdoor ( RSPAN_OM_SessionEntry_T *rspan_entry)
{
    RSPAN_OM_EnterCriticalSection();

    if ( NULL != rspan_entry )
    {
        if (  shmem_data_p->mirror_session_tot ) /* session is available */
        {
            printf ("There are [%d] sessions existed in the system.\n", shmem_data_p->mirror_session_tot );
            memcpy( rspan_entry, &shmem_data_p->local_mirror_db, sizeof(RSPAN_OM_SessionEntry_T));
            RSPAN_OM_LeaveCriticalSection();
            return TRUE;
        }
    }

    RSPAN_OM_LeaveCriticalSection();
    return FALSE;
}
#endif /*(RSPAN_BACK_DOOR == TRUE)*/

/* =============================================================================
 * MODULE NAME : MLAG_OM_PRIVATE.H
 * PURPOSE     : Provide declarations for MLAG private data management.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

#ifndef MLAG_OM_PRIVATE_H
#define MLAG_OM_PRIVATE_H

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include "sys_type.h"
#include "sys_adpt.h"
#include "mlag_type.h"

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#define MLAG_OM_DIGEST_SIZE     16

enum MLAG_OM_DIGEST_E
{
    MLAG_OM_DIGEST_LINK,
    MLAG_OM_DIGEST_FDB,
};

enum MLAG_OM_TIMER_E
{
    MLAG_OM_TIMER_TX,
    MLAG_OM_TIMER_FDB,
    MLAG_OM_TIMER_RX,
};

enum MLAG_OM_DEBUG_FLAG_E
{
    MLAG_OM_DEBUG_TX,
    MLAG_OM_DEBUG_RX,
    MLAG_OM_DEBUG_ERROR,
    MLAG_OM_DEBUG_THREAD,
};

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

typedef struct
{
    UI8_T   mac[SYS_ADPT_MAC_ADDR_LEN]; /* first key */
    UI32_T  vid;                        /* second key */
    UI32_T  mlag_id;
} MLAG_OM_FdbEntry_T;

typedef struct
{
    char    domain_id[MLAG_TYPE_MAX_DOMAIN_ID_LEN+1]; /* key */
    UI32_T  tx_timer;
    UI32_T  fdb_timer;
    UI32_T  rx_timer;
    UI8_T   remote_link_digest[MLAG_OM_DIGEST_SIZE];
    UI8_T   remote_fdb_digest[MLAG_OM_DIGEST_SIZE];
} MLAG_OM_PrivateEntry_T;

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM SPECIFICATIONS
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_OM_InitiateProcessResources
 * PURPOSE : Initiate process resources for the CSC.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_OM_InitiateProcessResources();

/* FUNCTION NAME - MLAG_OM_ClearAll
 * PURPOSE : Clear all dynamic data.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void MLAG_OM_ClearAll();

/* FUNCTION NAME - MLAG_OM_DefaultAll
 * PURPOSE : Set all default values.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void MLAG_OM_DefaultAll();

/* FUNCTION NAME - MLAG_OM_SetGlobalStatus
 * PURPOSE : Set global status of the feature.
 * INPUT   : status -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                     MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T MLAG_OM_SetGlobalStatus(UI32_T status);

/* FUNCTION NAME - MLAG_OM_SetDomainEntry
 * PURPOSE : Set a MLAG domain entry.
 * INPUT   : entry_p -- buffer containing information for a domain
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T MLAG_OM_SetDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_RemoveDomainEntry
 * PURPOSE : Set a MLAG domain entry.
 * INPUT   : entry_p->domain_id -- buffer containing information for a domain
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T MLAG_OM_RemoveDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_SetMlagEntry
 * PURPOSE : Set a MLAG entry.
 * INPUT   : entry_p -- buffer containing information for a MLAG
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_SetMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_RemoveMlagEntry
 * PURPOSE : Remove a MLAG entry.
 * INPUT   : entry_p -- buffer containing information for a MLAG
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_RemoveMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_GetRemoteFdbEntry
 * PURPOSE : Get a remote FDB entry.
 * INPUT   : entry_p->mac -- MAC address
 *           entry_p->vid -- VLAN ID
 * OUTPUT  : entry_p -- buffer containing a remote FDB entry
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetRemoteFdbEntry(MLAG_OM_FdbEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_GetNextRemoteFdbEntry
 * PURPOSE : Get next remote FDB entry.
 * INPUT   : entry_p->mac -- MAC address
 *           entry_p->vid -- VLAN ID
 * OUTPUT  : entry_p -- buffer containing next remote FDB entry
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetNextRemoteFdbEntry(MLAG_OM_FdbEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_GetNextRemoteFdbEntryByMlag
 * PURPOSE : Get next remote FDB entry by given MLAG ID.
 * INPUT   : entry_p->mlag_id -- MLAG ID
 * OUTPUT  : entry_p -- buffer containing next remote FDB entry
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetNextRemoteFdbEntryByMlag(MLAG_OM_FdbEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_GetFreeRemoteFdbCount
 * PURPOSE : Get the free count for remote FDB.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : free count for remote FDB
 * NOTE    : None
 */
UI32_T MLAG_OM_GetFreeRemoteFdbCount();

/* FUNCTION NAME - MLAG_OM_SetRemoteFdbEntry
 * PURPOSE : Set a remote FDB entry.
 * INPUT   : entry_p -- buffer containing a remote FDB entry
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_SetRemoteFdbEntry(MLAG_OM_FdbEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_RemoveRemoteFdbEntry
 * PURPOSE : Remove a remote FDB entry.
 * INPUT   : entry_p->mac -- MAC address
 *           entry_p->vid -- VLAN ID
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_RemoveRemoteFdbEntry(MLAG_OM_FdbEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_AddPrivateEntry
 * PURPOSE : Add a private entry.
 * INPUT   : domain_id_p -- domain ID
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : All fields except key are cleaned
 */
UI32_T MLAG_OM_AddPrivateEntry(char *domain_id_p);

/* FUNCTION NAME - MLAG_OM_DeletePrivateEntry
 * PURPOSE : Delete a private entry.
 * INPUT   : domain_id_p -- domain ID
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_DeletePrivateEntry(char *domain_id_p);

/* FUNCTION NAME - MLAG_OM_GetTimer
 * PURPOSE : Get the value of a kind of timer for a domain.
 * INPUT   : domain_id_p -- domain ID
 *           kind        -- MLAG_OM_TIMER_E
 * OUTPUT  : None
 * RETURN  : timer value
 * NOTE    : None
 */
UI32_T MLAG_OM_GetTimer(char *domain_id_p, UI32_T kind);

/* FUNCTION NAME - MLAG_OM_SetTimer
 * PURPOSE : Set the value of a kind of timer for a domain.
 * INPUT   : domain_id_p -- domain ID
 *           kind        -- MLAG_OM_TIMER_E
 *           value       -- timer value
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_SetTimer(char *domain_id_p, UI32_T kind, UI32_T value);

/* FUNCTION NAME - MLAG_OM_SameDigest
 * PURPOSE : Check whether the value of a kind of digest is the same as that in
 *           database for a domain.
 * INPUT   : domain_id_p -- domain ID
 *           kind        -- MLAG_OM_DIGEST_E
 *           digest_p    -- digest to be checked
 * OUTPUT  : None
 * RETURN  : TRUE  -- same
 *           FALSE -- different
 * NOTE    : None
 */
BOOL_T MLAG_OM_SameDigest(char *domain_id_p, UI32_T kind, UI8_T *digest_p);

/* FUNCTION NAME - MLAG_OM_SetDigest
 * PURPOSE : Set the value of a kind of digest for a domain.
 * INPUT   : domain_id_p -- domain ID
 *           kind        -- MLAG_OM_DIGEST_E
 *           digest_p    -- digest to be set
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_SetDigest(char *domain_id_p, UI32_T kind, UI8_T *digest_p);

/* FUNCTION NAME - MLAG_OM_GetDebugFlag
 * PURPOSE : Get current value of specified kind of debug flag.
 * INPUT   : flag -- MLAG_OM_DEBUG_FLAG_E
 * OUTPUT  : None
 * RETURN  : current value of the debug flag
 * NOTE    : None
 */
BOOL_T MLAG_OM_GetDebugFlag(UI32_T flag);

/* FUNCTION NAME - MLAG_OM_SetDebugFlag
 * PURPOSE : Set the value of specified kind of debug flag.
 * INPUT   : flag  -- MLAG_OM_DEBUG_FLAG_E
 *           value -- the value to be set
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_OM_SetDebugFlag(UI32_T flag, BOOL_T value);

#endif /* End of MLAG_OM_PRIVATE_H */

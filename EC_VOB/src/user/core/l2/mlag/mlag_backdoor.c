/* =============================================================================
 * MODULE NAME : MLAG_BACKDOOR.C
 * PURPOSE     : Provide definitions for MLAG backdoor functions.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include <string.h>
#include "backdoor_mgr.h"
#include "mlag_backdoor.h"
#include "mlag_om_private.h"
#include "mlag_type.h"

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * STATIC VARIABLE DEFINITIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_BACKDOOR_Main
 * PURPOSE : Provide main routine of backdoor.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void MLAG_BACKDOOR_Main(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    int choice;

    /* BODY
     */

    while (1)
    {
        BACKDOOR_MGR_Printf("\r\n--------------------------------------------");
        BACKDOOR_MGR_Printf("\r\n MLAG Backdoor Menu");
        BACKDOOR_MGR_Printf("\r\n--------------------------------------------");
        BACKDOOR_MGR_Printf("\r\n 1 - Get current RX timer value");
        BACKDOOR_MGR_Printf("\r\n 2 - Get current TX timer value");
        BACKDOOR_MGR_Printf("\r\n 3 - Get current FDB timer value");
        BACKDOOR_MGR_Printf("\r\n 4 - Get FDB count");
        BACKDOOR_MGR_Printf("\r\n 5 - Show remote FDB");
        BACKDOOR_MGR_Printf("\r\n 6 - Turn on/off TX debug");
        BACKDOOR_MGR_Printf("\r\n 7 - Turn on/off RX debug");
        BACKDOOR_MGR_Printf("\r\n 8 - Turn on/off error debug");
        BACKDOOR_MGR_Printf("\r\n 9 - Turn on/off thread debug");
        BACKDOOR_MGR_Printf("\r\n x - Exit backdoor");
        BACKDOOR_MGR_Printf("\r\n--------------------------------------------");
        BACKDOOR_MGR_Printf("\r\n Enter your choice: ");

        choice = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c", choice);

        switch (choice)
        {
        case '1':
        {
            char    buffer_ar[MLAG_TYPE_MAX_DOMAIN_ID_LEN+1];

            BACKDOOR_MGR_Printf("\r\n domain id: ");
            BACKDOOR_MGR_RequestKeyIn(buffer_ar, MLAG_TYPE_MAX_DOMAIN_ID_LEN);
            BACKDOOR_MGR_Printf("\r\n current rx timer value = %lu",
                (unsigned long)MLAG_OM_GetTimer(buffer_ar, MLAG_OM_TIMER_RX));
        }
            break;

        case '2':
        {
            char    buffer_ar[MLAG_TYPE_MAX_DOMAIN_ID_LEN+1];

            BACKDOOR_MGR_Printf("\r\n give domain id: ");
            BACKDOOR_MGR_RequestKeyIn(buffer_ar, MLAG_TYPE_MAX_DOMAIN_ID_LEN);
            BACKDOOR_MGR_Printf("\r\n current tx timer value = %lu",
                (unsigned long)MLAG_OM_GetTimer(buffer_ar, MLAG_OM_TIMER_TX));
        }
            break;

        case '3':
        {
            char    buffer_ar[MLAG_TYPE_MAX_DOMAIN_ID_LEN+1];

            BACKDOOR_MGR_Printf("\r\n give domain id: ");
            BACKDOOR_MGR_RequestKeyIn(buffer_ar, MLAG_TYPE_MAX_DOMAIN_ID_LEN);
            BACKDOOR_MGR_Printf("\r\n current fdb timer value = %lu",
                (unsigned long)MLAG_OM_GetTimer(buffer_ar, MLAG_OM_TIMER_FDB));
        }
            break;

        case '4':
            BACKDOOR_MGR_Printf("\r\nMax: %u, Free: %lu",
                MLAG_TYPE_MAX_NBR_OF_REMOTE_FDB, 
                MLAG_OM_GetFreeRemoteFdbCount());
            break;

        case '5':
        {
            MLAG_OM_FdbEntry_T  entry;

            memset(&entry, 0, sizeof(MLAG_OM_FdbEntry_T));
            while (MLAG_OM_GetNextRemoteFdbEntry(&entry) == MLAG_TYPE_RETURN_OK)
            {
                BACKDOOR_MGR_Printf("\r\nMAC: %02X-%02X-%02X-%02X-%02X-%02X",
                    entry.mac[0], entry.mac[1], entry.mac[2], entry.mac[3],
                    entry.mac[4], entry.mac[5]);
                BACKDOOR_MGR_Printf("\r\nVID: %lu", (unsigned long)entry.vid);
                BACKDOOR_MGR_Printf("\r\nMLAG ID: %lu", (unsigned long)entry.mlag_id);
                BACKDOOR_MGR_Printf("\r\n");
            }
        }
            break;

        case '6':
            if (MLAG_OM_GetDebugFlag(MLAG_OM_DEBUG_TX) == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n current tx flag: ON");
            }
            else
            {
                BACKDOOR_MGR_Printf("\r\n current tx flag: OFF");
            }

            BACKDOOR_MGR_Printf("\r\n give new value (1)ON (2)OFF: ");
            choice = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf("%c", choice);
            switch (choice)
            {
            case '1':
                MLAG_OM_SetDebugFlag(MLAG_OM_DEBUG_TX, TRUE);
                break;
            case '2':
                MLAG_OM_SetDebugFlag(MLAG_OM_DEBUG_TX, FALSE);
                break;
            }
            break;

        case '7':
            if (MLAG_OM_GetDebugFlag(MLAG_OM_DEBUG_RX) == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n current rx flag: ON");
            }
            else
            {
                BACKDOOR_MGR_Printf("\r\n current rx flag: OFF");
            }

            BACKDOOR_MGR_Printf("\r\n give new value (1)ON (2)OFF: ");
            choice = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf("%c", choice);
            switch (choice)
            {
            case '1':
                MLAG_OM_SetDebugFlag(MLAG_OM_DEBUG_RX, TRUE);
                break;
            case '2':
                MLAG_OM_SetDebugFlag(MLAG_OM_DEBUG_RX, FALSE);
                break;
            }
            break;

        case '8':
            if (MLAG_OM_GetDebugFlag(MLAG_OM_DEBUG_ERROR) == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n current error flag: ON");
            }
            else
            {
                BACKDOOR_MGR_Printf("\r\n current error flag: OFF");
            }

            BACKDOOR_MGR_Printf("\r\n give new value (1)ON (2)OFF: ");
            choice = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf("%c", choice);
            switch (choice)
            {
            case '1':
                MLAG_OM_SetDebugFlag(MLAG_OM_DEBUG_ERROR, TRUE);
                break;
            case '2':
                MLAG_OM_SetDebugFlag(MLAG_OM_DEBUG_ERROR, FALSE);
                break;
            }
            break;

        case '9':
            if (MLAG_OM_GetDebugFlag(MLAG_OM_DEBUG_THREAD) == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n current thread flag: ON");
            }
            else
            {
                BACKDOOR_MGR_Printf("\r\n current thread flag: OFF");
            }

            BACKDOOR_MGR_Printf("\r\n give new value (1)ON (2)OFF: ");
            choice = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf("%c", choice);
            switch (choice)
            {
            case '1':
                MLAG_OM_SetDebugFlag(MLAG_OM_DEBUG_THREAD, TRUE);
                break;
            case '2':
                MLAG_OM_SetDebugFlag(MLAG_OM_DEBUG_THREAD, FALSE);
                break;
            }
            break;

        case 'x':
            return;
        }

        BACKDOOR_MGR_Printf("\r\n");
    } /* end while (1) */

    return;
} /* End of MLAG_BACKDOOR_Main */

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */

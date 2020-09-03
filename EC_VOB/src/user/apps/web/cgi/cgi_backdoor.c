#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_type.h"
#include "backdoor_mgr.h"
#include "l_threadgrp.h"
#include "web_proc_comm.h"

#include "cgi.h"
#include "cgi_cache.h"
#include "cgi_file.h"
#include "cgi_debug.h"

#include "mm.h"

static void CGI_BACKDOOR_PrintHelp(void)
{
    BACKDOOR_MGR_Printf("\n  2 : Show cache stat. information");

    BACKDOOR_MGR_Printf("\n  3 : Turn %s file debug (%s) message",
        CGI_DEBUG_GetFlag() & CGI_DEBUG_FILE ? "OFF" : "ON",
        CGI_DEBUG_GetFlag() & CGI_DEBUG_FILE ? "ON" : "OFF");

    BACKDOOR_MGR_Printf("\n  4 : Turn %s cache debug (%s) message",
        CGI_DEBUG_GetFlag() & CGI_DEBUG_CACHE ? "OFF" : "ON",
        CGI_DEBUG_GetFlag() & CGI_DEBUG_CACHE ? "ON" : "OFF");

    BACKDOOR_MGR_Printf("\n  5 : Turn %s library debug (%s) message",
        CGI_DEBUG_GetFlag() & CGI_DEBUG_LIBRARY ? "OFF" : "ON",
        CGI_DEBUG_GetFlag() & CGI_DEBUG_LIBRARY ? "ON" : "OFF");

    BACKDOOR_MGR_Printf("\n  6 : Turn %s to display when detected memory leakage (%s)",
        MM_get_options() & MM_OPTION_SHOW_LEAKAGE ? "OFF" : "ON",
        MM_get_options() & MM_OPTION_SHOW_LEAKAGE ? "ON" : "OFF");

    BACKDOOR_MGR_Printf("\n  7 : Turn %s to display bytes detail in leakage memory (%s)",
        MM_get_options() & MM_OPTION_SHOW_LEAKAGE_DETAIL ? "OFF" : "ON",
        MM_get_options() & MM_OPTION_SHOW_LEAKAGE_DETAIL ? "ON" : "OFF");

    BACKDOOR_MGR_Printf("\n  8 : Clean ALL debug flag");

    BACKDOOR_MGR_Printf("\n  x : EXiT");
    BACKDOOR_MGR_Printf("\n");
    return;
}

void CGI_BACKDOOR_Main(void)
{
    UI8_T ch = 'x';
    BACKDOOR_MGR_Printf("\n CGI Backdoor");

    while (1)
    {
        CGI_BACKDOOR_PrintHelp();
        ch = BACKDOOR_MGR_GetChar();

        BACKDOOR_MGR_Printf("\n");

        switch(ch)
        {
            case '2':
            {
                CGI_CACHE_INFO_T info;

                if (CGI_OK == CGI_FILE_GetCacheInfo(&info))
                {
                    BACKDOOR_MGR_Printf("   Capacity: %lu Bytes\r\n", info.capacity_in_bytes);
                    BACKDOOR_MGR_Printf("   Size    : %lu Bytes\r\n", info.size_in_bytes);
                    BACKDOOR_MGR_Printf("   Access  : %lu\r\n", info.access);
                    BACKDOOR_MGR_Printf("   Hit     : %lu\r\n", info.hit);
                    BACKDOOR_MGR_Printf("   Miss    : %lu\r\n", info.miss);
                }
                else
                {
                    BACKDOOR_MGR_Printf("   Cache destroy\r\n");
                }
                break;
            }

            case '3':
                CGI_DEBUG_ToggleFlag(CGI_DEBUG_FILE);
                break;

            case '4':
                CGI_DEBUG_ToggleFlag(CGI_DEBUG_CACHE);
                break;

            case '5':
                CGI_DEBUG_ToggleFlag(CGI_DEBUG_LIBRARY);
                break;

            case '6':
                MM_toggle_options(MM_OPTION_SHOW_LEAKAGE);
                break;

            case '7':
                MM_toggle_options(MM_OPTION_SHOW_LEAKAGE_DETAIL);
                break;

            case '8':
                MM_remove_options(MM_OPTION_ALL);
                CGI_DEBUG_CleanFlag();
                break;

            case 'x': case 'X':
            default:
                return;
        }
    }
}

void CGI_BACKDOOR_CallBack(void)
{
#ifdef CGI_NO_L_THREADGRP
#define CGI_HAVE_L_THREADGRP    0
#else
#define CGI_HAVE_L_THREADGRP    1
#endif

#if (CGI_HAVE_L_THREADGRP == 1)
    L_THREADGRP_Handle_T    tg_handle;

    UI32_T backdoor_member_id;

    tg_handle = WEB_PROC_COMM_GetWEB_GROUPTGHandle();

    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &backdoor_member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }
#endif /* CGI_HAVE_L_THREADGRP */

    CGI_BACKDOOR_Main();

#if (CGI_HAVE_L_THREADGRP == 1)
    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
#endif /* CGI_HAVE_L_THREADGRP */
}


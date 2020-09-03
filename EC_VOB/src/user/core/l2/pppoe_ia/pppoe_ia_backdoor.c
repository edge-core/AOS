/* MODULE NAME: pppoe_ia_backdoor.c
 * PURPOSE:
 *   Definitions of backdoor APIs for PPPOE Intermediate Agent.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/17/09    -- Squid Ro, Create
 *   11/26/09    -- Squid Ro, Modify for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_type.h"
#include "sysfun.h"
#include "sys_dflt.h"
#include "sys_cpnt.h"
#include "pppoe_ia_backdoor.h"
#include "pppoe_ia_type.h"
#include "pppoe_ia_om.h"
#include "pppoe_ia_mgr.h"
#include "pppoe_ia_engine.h"
#include "l_stdlib.h"
#include "backdoor_mgr.h"
#include "l_threadgrp.h"
#include "l2_l4_proc_comm.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */

/* DATA TYPE DECLARATIONS
 */
typedef BOOL_T (*PPPOE_IA_BDR_FuncPtr)(UI8_T *cmd_buf_p);

typedef struct PPPOE_IA_Backdoor_Cmd_S
{
    const char*             cmd;
    PPPOE_IA_BDR_FuncPtr    exec;
} PPPOE_IA_Backdoor_Cmd_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void PPPOE_IA_BACKDOOR_Engine(void);
static BOOL_T PPPOE_IA_BACKDOOR_SetDebugFlag(UI8_T *cmd_buf_p);
static BOOL_T PPPOE_IA_BACKDOOR_SetGlobalEnable(UI8_T *cmd_buf_p);
static BOOL_T PPPOE_IA_BACKDOOR_SetPortEnable(UI8_T *cmd_buf_p);
static BOOL_T PPPOE_IA_BACKDOOR_SetPortTrust(UI8_T *cmd_buf_p);
static BOOL_T PPPOE_IA_BACKDOOR_DumpLportsByType(UI8_T *cmd_buf_p);
static BOOL_T PPPOE_IA_BACKDOOR_DumpPortCounters(UI8_T *cmd_buf_p);

/* STATIC VARIABLE DECLARATIONS
 */
static  UI32_T  pppoe_ia_dbg_flag = 0;

static PPPOE_IA_Backdoor_Cmd_T  pppoe_ia_bdr_cmd_items[] = {
    {"set debug flag",      PPPOE_IA_BACKDOOR_SetDebugFlag},
    {"enable global",       PPPOE_IA_BACKDOOR_SetGlobalEnable},
    {"enable lport",        PPPOE_IA_BACKDOOR_SetPortEnable},
    {"trust lport",         PPPOE_IA_BACKDOOR_SetPortTrust},
    {"dump lports",         PPPOE_IA_BACKDOOR_DumpLportsByType},
    {"dump port counters",  PPPOE_IA_BACKDOOR_DumpPortCounters},
    {"exit",                NULL},
};

static  char *dbg_str[] = { PPPOE_IA_DBG_LST(PPPOE_IA_DBG_NAME) };
static UI8_T  cmd_tbl_size = sizeof(pppoe_ia_bdr_cmd_items) / sizeof(PPPOE_IA_Backdoor_Cmd_T);

/* EXPORTED SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * PURPOSE: Main routine for PPPOE IA backdoor.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_BACKDOOR_Main(void)
{
    PPPOE_IA_BACKDOOR_Engine();
}

BOOL_T  PPPOE_IA_BACKDOOR_IsDebugOn(UI32_T flag)
{
    UI8_T   bit_val;

    bit_val = (pppoe_ia_dbg_flag >> flag) & 0x1;
    return (bit_val != 0);
}

/* LOCAL SUBPROGRAM BODIES
 */
static void PPPOE_IA_BACKDOOR_ShowCmd()
{
    UI8_T   i;

    for (i = 0; i<cmd_tbl_size; i++)
    {
        BACKDOOR_MGR_Printf(" %2d. %-20s\r\n", i, pppoe_ia_bdr_cmd_items[i].cmd);
    }
}

static void PPPOE_IA_BACKDOOR_GetNum(UI8_T string[])
{
    BACKDOOR_MGR_RequestKeyIn(string, 4);
    BACKDOOR_MGR_Printf("\r\n");
}

static BOOL_T PPPOE_IA_BACKDOOR_SetDebugFlag(UI8_T *cmd_buf_p)
{
    UI32_T  i, bit_val;
    char    *on_off_str[] = {"Off", "On"};
    UI8_T   ch;

    do
    {
        for (i =0; i <=PPPOE_IA_DBG_RESET; i ++)
        {
            if (i < PPPOE_IA_DBG_ALL)
            {
                bit_val = (pppoe_ia_dbg_flag >> i) & 0x1;
                BACKDOOR_MGR_Printf(" %2ld. %-20s : %s\r\n", i, dbg_str[i], on_off_str[bit_val]);
            }
            else
            {
                BACKDOOR_MGR_Printf(" %2ld. %-20s\r\n", i, dbg_str[i]);
            }
        }

        PPPOE_IA_BACKDOOR_GetNum(cmd_buf_p);
        ch = atoi((char *) cmd_buf_p);

        switch (ch)
        {
        case PPPOE_IA_DBG_ALL:
            pppoe_ia_dbg_flag = 0xffff;
            break;
        case PPPOE_IA_DBG_RESET:
            pppoe_ia_dbg_flag = 0;
            break;
        default:
            if (ch < PPPOE_IA_DBG_ALL)
            {
                bit_val = 1 << ch;
                pppoe_ia_dbg_flag ^= bit_val;
            }
            break;
        }
    } while (ch < PPPOE_IA_DBG_MAX);

    return TRUE;
}

static void PPPOE_IA_BACKDOOR_Engine(void)
{
    L_THREADGRP_Handle_T    pppoe_ia_bdr_tg_handle;
    UI32_T                  pppoe_ia_bdr_mbr_id;
    BOOL_T                  engine_continue = TRUE;
    UI8_T                   ch;
    UI8_T                   cmd_buf[255];

    pppoe_ia_bdr_tg_handle =(L_THREADGRP_Handle_T) L2_L4_PROC_COMM_GetPppoeiaGroupTGHandle();

    /* Join thread group
     */
    if(L_THREADGRP_Join(pppoe_ia_bdr_tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &pppoe_ia_bdr_mbr_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while(engine_continue)
    {
        ch = 0;
        cmd_buf[0] = 0;

        PPPOE_IA_BACKDOOR_ShowCmd();
        PPPOE_IA_BACKDOOR_GetNum(cmd_buf);

        ch = atoi((char *) cmd_buf);

        if (ch >= cmd_tbl_size -1)
        {
            L_THREADGRP_Leave(pppoe_ia_bdr_tg_handle, pppoe_ia_bdr_mbr_id);
            break;
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\n");
            if (NULL != pppoe_ia_bdr_cmd_items[ch].exec)
            {
                L_THREADGRP_Execution_Request(pppoe_ia_bdr_tg_handle, pppoe_ia_bdr_mbr_id);
                pppoe_ia_bdr_cmd_items[ch].exec(cmd_buf);
                L_THREADGRP_Execution_Release(pppoe_ia_bdr_tg_handle, pppoe_ia_bdr_mbr_id);
            }
        }
    }
}

static BOOL_T PPPOE_IA_BACKDOOR_SetGlobalEnable(UI8_T *cmd_buf_p)
{
    UI8_T   is_enable;
    BOOL_T  ret;

    BACKDOOR_MGR_Printf(" {1 - enable | 0 - disable}\r\n");

    PPPOE_IA_BACKDOOR_GetNum(cmd_buf_p);
    is_enable = atoi((char *) cmd_buf_p);

    ret = PPPOE_IA_MGR_SetGlobalEnable((is_enable == 1));
    BACKDOOR_MGR_Printf("PPPOE_IA_MGR_SetGlobalEnable %d\r\n", ret);

    return TRUE;
}

static BOOL_T PPPOE_IA_BACKDOOR_SetPortEnable(UI8_T *cmd_buf_p)
{
    UI8_T   is_enable, lport;
    BOOL_T  ret;

    BACKDOOR_MGR_Printf(" {1 - enable | 0 - disable}\r\n");
    PPPOE_IA_BACKDOOR_GetNum(cmd_buf_p);
    is_enable = atoi((char *) cmd_buf_p);
    BACKDOOR_MGR_Printf(" {lport (trunk_ifidx-%d)}\r\n", SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER);
    PPPOE_IA_BACKDOOR_GetNum(cmd_buf_p);
    lport = atoi((char *) cmd_buf_p);
    ret = PPPOE_IA_MGR_SetPortBoolDataByField(
            lport, PPPOE_IA_TYPE_FLDID_PORT_ENABLE, (is_enable == 1));
    BACKDOOR_MGR_Printf("PPPOE_IA_MGR_SetPortBoolDataByField enable %d\r\n", ret);

    return TRUE;
}

static BOOL_T PPPOE_IA_BACKDOOR_SetPortTrust(UI8_T *cmd_buf_p)
{
    UI8_T   is_trust, lport;
    BOOL_T  ret;

    BACKDOOR_MGR_Printf(" {1 - trust | 0 - untrust}\r\n");
    PPPOE_IA_BACKDOOR_GetNum(cmd_buf_p);
    is_trust = atoi((char *) cmd_buf_p);
    BACKDOOR_MGR_Printf(" {lport (trunk_ifidx-%d)}\r\n", SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER);
    PPPOE_IA_BACKDOOR_GetNum(cmd_buf_p);
    lport = atoi((char *) cmd_buf_p);
    ret = PPPOE_IA_MGR_SetPortBoolDataByField(
            lport, PPPOE_IA_TYPE_FLDID_PORT_TRUST, (is_trust == 1));
    BACKDOOR_MGR_Printf("PPPOE_IA_MGR_SetPortBoolDataByField trust %d\r\n", ret);

    return TRUE;
}

static BOOL_T PPPOE_IA_BACKDOOR_DumpLports(char *title_p, UI8_T *lports_p, UI32_T port_num)
{
    UI32_T  i, trunk_byte, trunk_bit, pos=0;

    trunk_byte = (SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER-1) >> 3;
    trunk_bit  = (7 - ((SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER - 1) & 7));

    BACKDOOR_MGR_Printf("\r\n %s: port_num/bytes/max_ifidx-%ld/%d/%d",
        title_p, port_num, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST, SYS_ADPT_TOTAL_NBR_OF_LPORT);
    BACKDOOR_MGR_Printf("\r\n  ports:");
    for (i =0; i <SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
    {
        if (i == trunk_byte)
        {
            BACKDOOR_MGR_Printf("\r\n  trunk: (byte/begin bit-%ld/%ld)", trunk_byte, trunk_bit);
            pos = 0;
        }

        if (pos % 16 == 0)
            BACKDOOR_MGR_Printf("\r\n  ");
        else
        if (pos % 4 == 0)
            BACKDOOR_MGR_Printf(" ");

        BACKDOOR_MGR_Printf("%02X", lports_p[i]);

        pos++;
    }
    BACKDOOR_MGR_Printf("\r\n");

    return TRUE;
}

static BOOL_T PPPOE_IA_BACKDOOR_DumpLportsByType(UI8_T *cmd_buf_p)
{
    UI32_T  port_num;
    char    *tag_str[] = {"ENABLE", "TRUST", "UNTRUST" };
    UI8_T   lports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
            dump_type;

    BACKDOOR_MGR_Printf(" {0 - enable | 1 - trust | 2 - untrust}\r\n");
    PPPOE_IA_BACKDOOR_GetNum(cmd_buf_p);
    dump_type = atoi((char *) cmd_buf_p);

    if (  (dump_type < 3)
        &&(TRUE == PPPOE_IA_OM_GetLports(dump_type, lports, &port_num))
       )
    {
        PPPOE_IA_BACKDOOR_DumpLports(tag_str[dump_type], lports, port_num);
    }
    else
    {
        BACKDOOR_MGR_Printf("PPPOE_IA_OM_GetLports failed\r\n");
    }

    return TRUE;
}

static BOOL_T PPPOE_IA_BACKDOOR_DumpPortCounters(UI8_T *cmd_buf_p)
{
    UI32_T                      lport;
    PPPOE_IA_OM_PortStsEntry_T  psts_ent;
    char                        *tag_str[] = {"ALL",  "PADI", "PADO", "PADR",
                                              "PADS", "PADT", "ERR",
                                              "REP_UNTRUST", "REQ_UNTRUST"};

    BACKDOOR_MGR_Printf(" {lport (trunk_ifidx-%d)}\r\n", SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER);
    PPPOE_IA_BACKDOOR_GetNum(cmd_buf_p);
    lport = atoi((char *) cmd_buf_p);

    if (TRUE == PPPOE_IA_MGR_GetPortStatisticsEntry(lport, &psts_ent))
    {
        BACKDOOR_MGR_Printf("\r\n %-10s %-10s %-10s %-10s %-10s %-10s",
                    tag_str[0], tag_str[1], tag_str[2],
                    tag_str[3], tag_str[4], tag_str[5]);
        BACKDOOR_MGR_Printf("\r\n %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu",
                    psts_ent.all,  psts_ent.padi, psts_ent.pado,
                    psts_ent.padr, psts_ent.pads, psts_ent.padt);

        BACKDOOR_MGR_Printf("\r\n %-12s %-12s %-12s",
                    tag_str[6], tag_str[7], tag_str[8]);
        BACKDOOR_MGR_Printf("\r\n %-12lu %-12lu %-12lu",
                    psts_ent.malform,  psts_ent.rep_untrust, psts_ent.req_untrust);
        BACKDOOR_MGR_Printf("\r\n");
    }
    else
    {
        BACKDOOR_MGR_Printf("PPPOE_IA_MGR_GetPortStatisticsEntry failed\r\n");
    }

    return TRUE;
}


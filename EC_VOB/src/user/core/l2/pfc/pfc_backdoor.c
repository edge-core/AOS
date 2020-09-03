/* MODULE NAME: pfc_backdoor.c
 * PURPOSE:
 *   Definitions of backdoor APIs for PFC
 *   (IEEE Std 802.1Qbb, Priority-based Flow Control).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   10/05/12    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>

#include "sys_type.h"
#include "sysfun.h"
#include "sys_dflt.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_PFC == TRUE)

#include "pfc_backdoor.h"
#include "pfc_type.h"
#include "pfc_om.h"
#include "pfc_mgr.h"
#include "l_stdlib.h"
#include "backdoor_mgr.h"
#include "l_threadgrp.h"
#include "l2_l4_proc_comm.h"


/* NAMING CONSTANT DECLARATIONS
 */
#if (PFC_TYPE_BUILD_LINUX == FALSE)
    #define BACKDOOR_MGR_Printf            printf
    #define BACKDOOR_MGR_RequestKeyIn      BACKDOOR_MGR_RequestKeyIn
#endif

#define PFC_BACKDOOR_MAX_ARG_NUM       10

/* MACRO FUNCTIONS DECLARACTION
 */

/* DATA TYPE DECLARATIONS
 */
typedef BOOL_T (*PFC_BDR_FuncPtr)(char *cmd_buf_p, int argc, char *argv[]);

typedef struct PFC_Backdoor_Cmd_S
{
    const char*         cmd;
    PFC_BDR_FuncPtr    exec;
} PFC_Backdoor_Cmd_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void   PFC_BACKDOOR_Engine(void);
static BOOL_T PFC_BACKDOOR_SetDebugFlag    (char *cmd_buf_p, int argc, char *argv[]);
static BOOL_T PFC_BACKDOOR_SetByField      (char *cmd_buf_p, int argc, char *argv[]);
static BOOL_T PFC_BACKDOOR_GetByField      (char *cmd_buf_p, int argc, char *argv[]);
static BOOL_T PFC_BACKDOOR_DumpPort        (char *cmd_buf_p, int argc, char *argv[]);
static BOOL_T PFC_BACKDOOR_CheckCosConfig  (char *cmd_buf_p, int argc, char *argv[]);

/* STATIC VARIABLE DECLARATIONS
 */
static PFC_Backdoor_Cmd_T  pfc_bdr_cmd_items[] = {
    {"set debug flag",      PFC_BACKDOOR_SetDebugFlag},
    {"dump lport",          PFC_BACKDOOR_DumpPort},
    {"get by field",        PFC_BACKDOOR_GetByField},
    {"set by field",        PFC_BACKDOOR_SetByField},
    {"chk cos config",      PFC_BACKDOOR_CheckCosConfig},
    {"exit",                NULL},
};

static char     *dbg_str[] = { PFC_DBG_LST(PFC_DBG_NAME) };
extern char     *pfc_rce_str[];
static UI8_T    cmd_tbl_size = sizeof(pfc_bdr_cmd_items) / sizeof(PFC_Backdoor_Cmd_T);

/* EXPORTED SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * PURPOSE: Main routine for PFC backdoor.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
void PFC_BACKDOOR_Main(void)
{
    PFC_BACKDOOR_Engine();
}


/* LOCAL SUBPROGRAM BODIES
 */
static void PFC_BACKDOOR_ShowCmd(void)
{
    UI8_T   i;

    for (i = 0; i<cmd_tbl_size; i++)
    {
        BACKDOOR_MGR_Printf(" %2d. %-20s\r\n", i, pfc_bdr_cmd_items[i].cmd);
    }
}

static void PFC_BACKDOOR_GetNum(
    char   string[])
{
    BACKDOOR_MGR_RequestKeyIn(string, 8);
    BACKDOOR_MGR_Printf("\r\n");
}

static void PFC_BACKDOOR_Engine(void)
{
    L_THREADGRP_Handle_T    pfc_bdr_tg_handle;
    UI32_T                  pfc_bdr_mbr_id;
    char                    *p, *cmd_p, *argv[PFC_BACKDOOR_MAX_ARG_NUM];
    int                     argc, cmd_id;
    char                    delims[] = " ";
    char                    comment_char = '#';
    char                    cmd_buf[256];

    pfc_bdr_tg_handle =(L_THREADGRP_Handle_T) L2_L4_PROC_COMM_GetDcbGroupTGHandle();

    /* Join thread group
     */
    if(L_THREADGRP_Join(pfc_bdr_tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &pfc_bdr_mbr_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while(TRUE)
    {
        cmd_buf[0] = 0;

        PFC_BACKDOOR_ShowCmd();

        BACKDOOR_MGR_RequestKeyIn(cmd_buf, sizeof(cmd_buf)-1);
        BACKDOOR_MGR_Printf("\r\n");

        if (strlen(cmd_buf) == 0)
            continue;

        argc = 0;
        cmd_p = cmd_buf;
        if (NULL != (p = strtok (cmd_buf, delims)))
        {
            cmd_p = p;
            while (  (NULL != (p = strtok (NULL, delims)))
                   &&(*p != comment_char)
                  )
            {
                argv[argc++] = p;
            }
        }

        cmd_id = atoi(cmd_p);
        if (  (0 == strcmp (cmd_p, "exit"))
            ||(0 == strcmp (cmd_p, "quit"))
            ||(cmd_id >= cmd_tbl_size-1)
           )
        {
            L_THREADGRP_Leave(pfc_bdr_tg_handle, pfc_bdr_mbr_id);
            break;
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\n");
            if (NULL != pfc_bdr_cmd_items[cmd_id].exec)
            {
                L_THREADGRP_Execution_Request(pfc_bdr_tg_handle, pfc_bdr_mbr_id);
                pfc_bdr_cmd_items[cmd_id].exec(cmd_buf, argc, argv);
                L_THREADGRP_Execution_Release(pfc_bdr_tg_handle, pfc_bdr_mbr_id);
            }
        }
    }
}

static BOOL_T PFC_BACKDOOR_SetDebugFlag(
    char *cmd_buf_p, int argc, char *argv[])
{
    UI32_T  i, bit_val, pfc_om_dbg_flag = 0;;
    char    *on_off_str[] = {"Off", "On"};
    UI8_T   ch =0;

    pfc_om_dbg_flag = PFC_OM_GetDbgFlag();

    do
    {
        for (i =0; i <= PFC_DBG_MAX; i ++)
        {
            switch (i)
            {
            case PFC_DBG_RESET:
            case PFC_DBG_ALL:
                BACKDOOR_MGR_Printf(" %2ld. %-20s\r\n", (long)i, dbg_str[i]);
                break;
            case PFC_DBG_MAX:
                BACKDOOR_MGR_Printf(" %2ld. %-20s\r\n", (long)i, "exit");
                break;
            default:
                bit_val = (pfc_om_dbg_flag >> i) & 0x1;
                BACKDOOR_MGR_Printf(" %2ld. %-20s : %s\r\n", (long)i, dbg_str[i], on_off_str[bit_val]);
                break;
            }
        }

        PFC_BACKDOOR_GetNum(cmd_buf_p);

        if (cmd_buf_p[0] == '\0')
            continue;

        ch = atoi((char *) cmd_buf_p);

        switch (ch)
        {
        case PFC_DBG_RESET:
            pfc_om_dbg_flag = 0;
            break;
        case PFC_DBG_ALL:
            pfc_om_dbg_flag = 0xffff;
            break;
        default:
            if (ch < PFC_DBG_MAX)
            {
                bit_val = 1 << ch;
                pfc_om_dbg_flag ^= bit_val;
            }
            break;
        }

        PFC_OM_SetDbgFlag(pfc_om_dbg_flag);

    } while (ch < PFC_DBG_MAX);

    return TRUE;
}

static BOOL_T PFC_BACKDOOR_GetByField(
    char *cmd_buf_p, int argc, char *argv[])
{
    UI32_T  lport, field_id, value=0;
    BOOL_T  ret;
    char    *field_str[] = {PFC_TYPE_FLDE_LST(PFC_TYPE_SINGAL_NAME)};

    if (argc < 2)
    {
        BACKDOOR_MGR_Printf(" {x - lport id (0 <= x <= %ld)}\r\n",
            (long)SYS_ADPT_TOTAL_NBR_OF_LPORT);
        PFC_BACKDOOR_GetNum(cmd_buf_p);
        lport = atoi((char *) cmd_buf_p);

        BACKDOOR_MGR_Printf(" {y - field id (0 <= y < %d)}\r\n",
            PFC_TYPE_FLDE_MAX);

        for (field_id=0; field_id < PFC_TYPE_FLDE_MAX; field_id++)
        {
            BACKDOOR_MGR_Printf("  id-%2ld, %s\r\n",
                (long)field_id, &field_str[field_id][PFC_TYPE_FLDE_STR_PREFIX]);
        }

        PFC_BACKDOOR_GetNum(cmd_buf_p);
        field_id = atoi((char *) cmd_buf_p);
    }
    else
    {
        lport    = atoi(argv[0]);
        field_id = atoi(argv[1]);
    }

    switch (field_id)
    {
    default:
        ret = PFC_MGR_GetDataByField(lport, field_id, (UI8_T *) &value);
        break;
    }

    if (TRUE == ret)
    {
        switch (field_id)
        {
        default:
            BACKDOOR_MGR_Printf("  id-%2ld, %s - %ld\r\n",
                (long)field_id, &field_str[field_id][PFC_TYPE_FLDE_STR_PREFIX], (long)value);
            break;
        }
    }
    else
    {
        BACKDOOR_MGR_Printf("PFC_MGR_GetDataByField Failed\r\n");
    }

    return TRUE;
}

static BOOL_T PFC_BACKDOOR_SetByField(
    char *cmd_buf_p, int argc, char *argv[])
{
    UI32_T  lport, field_id, value;
    UI32_T  ret;
    char    *field_str[] = {PFC_TYPE_FLDE_LST(PFC_TYPE_SINGAL_NAME)};

    if (argc < 3)
    {
        BACKDOOR_MGR_Printf(" {x - lport id (0 <= x <= %ld)}\r\n",
            (long)SYS_ADPT_TOTAL_NBR_OF_LPORT);
        PFC_BACKDOOR_GetNum(cmd_buf_p);
        lport = atoi((char *) cmd_buf_p);

        BACKDOOR_MGR_Printf(" {y - field id (0 <= y < %d)}\r\n",
            PFC_TYPE_FLDE_MAX);

        for (field_id=0; field_id < PFC_TYPE_FLDE_MAX; field_id++)
        {
            BACKDOOR_MGR_Printf("  id-%2ld, %s\r\n",
                (long)field_id, &field_str[field_id][PFC_TYPE_FLDE_STR_PREFIX]);
        }

        PFC_BACKDOOR_GetNum(cmd_buf_p);
        field_id = atoi((char *) cmd_buf_p);

        BACKDOOR_MGR_Printf(" {z - value}\r\n");
        switch (field_id)
        {
        default:
            PFC_BACKDOOR_GetNum(cmd_buf_p);
            value = atoi((char *) cmd_buf_p);
            ret = PFC_MGR_SetDataByField(lport, field_id, (UI8_T *) &value);
            break;
        }
    }
    else
    {
        lport    = atoi(argv[0]);
        field_id = atoi(argv[1]);

        switch (field_id)
        {
        default:
            value = atoi(argv[2]);
            ret = PFC_MGR_SetDataByField(lport, field_id, (UI8_T *) &value);
            break;
        }
    }

    BACKDOOR_MGR_Printf("PFC_MGR_SetDataByField %s\r\n",
        &pfc_rce_str[ret][PFC_TYPE_RCE_STR_PREFIX]);

    return TRUE;
}

static BOOL_T PFC_BACKDOOR_DumpPort(
    char *cmd_buf_p, int argc, char *argv[])
{
    PFC_TYPE_PortCtrlRec_T *port_ctrl_p;
    UI32_T                  lport;

    if (argc < 1)
    {
        BACKDOOR_MGR_Printf(" {x - lport (0 < x <= %ld)}\r\n",
            (long)SYS_ADPT_TOTAL_NBR_OF_LPORT);

        PFC_BACKDOOR_GetNum(cmd_buf_p);
        lport = atoi((char *) cmd_buf_p);
    }
    else
    {
        lport = atoi(argv[0]);
    }

    port_ctrl_p = PFC_OM_GetPortCtrlPtrByLport(lport);
    if (NULL != port_ctrl_p)
    {
        char    *mode_tag[] = {"OFF", "ON", "AUTO"};
        BACKDOOR_MGR_Printf("\r\n{LPORT(%d)}", lport);
        BACKDOOR_MGR_Printf("\r\n ADM_MODE OPR_MODE ADM_PRI_BMP OPR_PRI_BMP");
        BACKDOOR_MGR_Printf("\r\n -------- -------- ----------- -----------");
        BACKDOOR_MGR_Printf("\r\n %8s %8s %11X %11X",
            mode_tag[port_ctrl_p->mode_adm],
            mode_tag[port_ctrl_p->mode_opr],
            port_ctrl_p->en_pri_bmp_adm,
            port_ctrl_p->en_pri_bmp_opr
            );

//        PFC_BACKDOOR_DumpPortAdm(lport, &port_ctrl_p->adm);
//        PFC_BACKDOOR_DumpPortOpr(lport, &port_ctrl_p->opr);
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\nPFC_OM_GetPortCtrlPtrByLport(%d) failed\r\n",
            lport);
    }
    BACKDOOR_MGR_Printf("\r\n\r\n");

    return TRUE;
}

static BOOL_T PFC_BACKDOOR_CheckCosConfig(
    char *cmd_buf_p, int argc, char *argv[])
{
    BACKDOOR_MGR_Printf("PFC_MGR_CheckCosConfig\r\n");
    BOOL_T  is_cos_ok;

    if (argc < 1)
    {
        BACKDOOR_MGR_Printf(" {x - is_cos_ok (0 < x <= 1)}\r\n");
        PFC_BACKDOOR_GetNum(cmd_buf_p);
        is_cos_ok = atoi((char *) cmd_buf_p);
    }
    else
    {
        is_cos_ok = atoi(argv[0]);
    }

    PFC_MGR_CosConfigChanged_CallBack(1, is_cos_ok);

    return TRUE;
}
#endif /* #if (SYS_CPNT_PFC == TRUE) */


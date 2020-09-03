/*
 *   File Name: ipal_reflect.c
 *   Purpose:   TCP/IP shim layer(ipal) reflection interface implementation API
 *   Note:
 *   Create:    kh_shi     2008.11.4
 *
 *   Histrory:
 *              Modify		   Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

/*
 * INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "l_threadgrp.h"

#include "ipal_types.h"
#include "ipal_reflect.h"
#include "leaf_es3626a.h"
#include "ipal_rt_netlink.h"

#include "netcfg_type.h"
#include "netcfg_mgr_ip.h"
#include "netcfg_proc_comm.h"

/*
 * NAMING CONST DECLARATIONS
 */


/*
 * MACRO FUNCTION DECLARATIONS
 */


/*
 * DATA TYPE DECLARATIONS
 */

/*
 * STATIC VARIABLE DECLARATIONS
 */
static UI32_T ipal_thread_id;
static L_THREADGRP_Handle_T tg_handle;
static UI32_T member_id;
/*
 * LOCAL SUBPROGRAM DECLARATIONS
 */

static void IPAL_REFLECT_TaskMain(void *arg)
{
    IPAL_ReflectMesg_T reflect_msg;
    NETCFG_TYPE_InetRifConfig_T rif;

    tg_handle = NETCFG_PROC_COMM_GetNetcfgTGHandle();

    if (L_THREADGRP_Join(tg_handle, SYS_BLD_NETCFG_GROUP_MGR_THREAD_PRIORITY, &member_id) == FALSE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.", __FUNCTION__);
        return;
    }

    while (TRUE)
    {
        if (IPAL_RESULT_OK == IPAL_REFLECT_Read(&reflect_msg))
        {
            memset(&rif, 0x0, sizeof(NETCFG_TYPE_InetRifConfig_T));
            rif.addr = reflect_msg.u.addr.ipaddr;
            rif.ifindex = reflect_msg.u.addr.ifindex;
            if (reflect_msg.u.addr.flags & IPAL_IFA_F_PERMANENT)
                rif.flags |= NETCFG_TYPE_RIF_FLAG_IFA_F_PERMANENT;
            if (rif.addr.type == L_INET_ADDR_TYPE_IPV6Z ||
                rif.addr.type == L_INET_ADDR_TYPE_IPV6)
            {
                rif.ipv6_addr_config_type = NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_STATELESS;
                if (rif.addr.type == L_INET_ADDR_TYPE_IPV6Z)
                    rif.ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL;
                else
                    rif.ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL;
            }
            else if (rif.addr.type == L_INET_ADDR_TYPE_IPV4Z ||
                     rif.addr.type == L_INET_ADDR_TYPE_IPV4)
            {
                /* the ipv4_role maybe incorrect, will update from ipcfg_om later */
                rif.ipv4_role = NETCFG_TYPE_MODE_PRIMARY;
            }

            switch(reflect_msg.type)
            {
                case IPAL_REFLECT_TYPE_NEW_ADDR:
                    rif.row_status = VAL_netConfigStatus_2_createAndGo;
                    break;

                case IPAL_REFLECT_TYPE_DEL_ADDR:
                    rif.row_status = VAL_netConfigStatus_2_destroy;
                    break;
                default:
                    continue;
            }


            if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
            {
                SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
            }

            NETCFG_MGR_IP_IpalRifReflection_CallBack(&rif);

            if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
            {
                SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
            }
        }
    }

    L_THREADGRP_Leave(tg_handle, member_id);
}

void IPAL_REFLECT_CreateTask(void)
{
    if(SYSFUN_SpawnThread(SYS_BLD_NIC_THREAD_PRIORITY,
                          SYS_BLD_NIC_THREAD_SCHED_POLICY,
                          "IPAL_TASK",
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          IPAL_REFLECT_TaskMain,
                          NULL,
                          &ipal_thread_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }

    return;
}

/* FUNCTION NAME : IPAL_IF_GetSnmpStatisticByTpye
 * PURPOSE:
 *      Get IPv4 SNMP statistic counter
 *
 * INPUT:
 *      stat_type  -- One of snmp type in IPAL_SnmpStatisticType_T
 *
 * OUTPUT:
 *      value_p    -- retrieved value
 *
 * RETURN:
 *      IPAL_RESULT_OK   -- success
 *      IPAL_RESULT_FAIL -- fail
 *
 * NOTES:
 *      1. Linux: Stored in /proc/net/snmp
 */
UI32_T IPAL_REFLECT_Read(IPAL_ReflectMesg_T *reflect_msg_p)
{
    return IPAL_Rt_RecvMsg(reflect_msg_p);
}




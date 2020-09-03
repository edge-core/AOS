/* MODULE NAME - CN_BACKDOOR.C
 * PURPOSE : Provides the definitions for CN back door.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sys_dflt.h"
#include "backdoor_mgr.h"
#include "cn_backdoor.h"
#include "cn_engine.h"
#include "cn_mgr.h"
#include "cn_om.h"
#include "cn_om_private.h"
#include "cn_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define CN_UNIT_TEST    TRUE

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

#if (CN_UNIT_TEST == TRUE)
static void CN_BACKDOOR_UnitTest(void);
#endif

/* STATIC VARIABLE DEFINITIONS
 */

static char *defense_mode_str[] = {
    [CN_TYPE_DEFENSE_MODE_DISABLED]         = "Disabled",
    [CN_TYPE_DEFENSE_MODE_EDGE]             = "Edge",
    [CN_TYPE_DEFENSE_MODE_INTERIOR]         = "Interior",
    [CN_TYPE_DEFENSE_MODE_INTERIOR_READY]   = "Interior-Ready",
    [CN_TYPE_DEFENSE_MODE_AUTO]             = "Auto",
    [CN_TYPE_DEFENSE_MODE_BY_GLOBAL]        = "By-Global"};

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME - CN_BACKDOOR_Main
 * PURPOSE : Provide main routine of CN backdoor.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_BACKDOOR_Main(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    int     choice;
    char    buffer[BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH+1];

    /* BODY
     */

    while (1)
    {
        BACKDOOR_MGR_Print("\r\n=============================================");
        BACKDOOR_MGR_Print("\r\n CN Backdoor Menu");
        BACKDOOR_MGR_Print("\r\n=============================================");
        BACKDOOR_MGR_Print("\r\n 1 - Show global entry information");
        BACKDOOR_MGR_Print("\r\n 2 - Show per-priority entry information");
        BACKDOOR_MGR_Print("\r\n 3 - Show per-port-priority information");
        BACKDOOR_MGR_Print("\r\n 4 - Show per-CP information");
        BACKDOOR_MGR_Print("\r\n 5 - Show per-port CN-TAG removal bitmap");
        BACKDOOR_MGR_Print("\r\n 6 - Show per-port L4 register count");
        BACKDOOR_MGR_Print("\r\n e - Turn on/off showing error messages");
        BACKDOOR_MGR_Print("\r\n t - Turn on/off showing thread messages");
#if (CN_UNIT_TEST == TRUE)
        BACKDOOR_MGR_Print("\r\n u - Run unit test");
#endif
        BACKDOOR_MGR_Print("\r\n x - Exit backdoor");
        BACKDOOR_MGR_Print("\r\n---------------------------------------------");
        BACKDOOR_MGR_Print("\r\nEnter your choice: ");

        choice = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c", choice);

        switch (choice)
        {
        case '1':
            {
                CN_OM_GlobalData_T data;

                if (CN_OM_GetGlobalData(&data) != CN_TYPE_RETURN_OK)
                {
                    BACKDOOR_MGR_Printf("\r\n Failed to get global data");
                    break;
                }

                BACKDOOR_MGR_Printf("\r\n");
                BACKDOOR_MGR_Printf("\r\n Global admin status: %s",
                    (data.admin_status == CN_TYPE_GLOBAL_STATUS_ENABLE)
                    ? "Enabled" : "Disabled");
                BACKDOOR_MGR_Printf("\r\n Global oper status: %s",
                    (data.oper_status == CN_TYPE_GLOBAL_STATUS_ENABLE)
                    ? "Enabled" : "Disabled");
                BACKDOOR_MGR_Printf("\r\n CNM transmit priority: %lu",
                    (unsigned long)data.cnm_tx_priority);
            }
            break;

        case '2':
            {
                CN_OM_PriData_T data;
                UI32_T          priority;

                BACKDOOR_MGR_Print("\r\n please specify a priority (0-7): ");
                BACKDOOR_MGR_RequestKeyIn(buffer, 1);
                priority = (UI32_T)strtol(buffer, NULL, 10);

                if (CN_OM_GetPriData(priority, &data) != CN_TYPE_RETURN_OK)
                {
                    BACKDOOR_MGR_Printf("\r\n Failed to get pri data for "
                        "priority=%lu", (unsigned long)priority);
                    break;
                }

                BACKDOOR_MGR_Printf("\r\n");
                BACKDOOR_MGR_Printf("\r\n Priority: %lu", (unsigned long)priority);
                BACKDOOR_MGR_Printf("\r\n Active: %s", (data.active == TRUE)
                    ? "True" : "False");
                BACKDOOR_MGR_Printf("\r\n Defense mode: %s",
                    defense_mode_str[data.defense_mode]);
                BACKDOOR_MGR_Printf("\r\n Admin alternate priority: %lu",
                    (unsigned long)data.admin_alt_priority);
                BACKDOOR_MGR_Printf("\r\n Auto alternate priority: %lu",
                    (unsigned long)data.auto_alt_priority);
            }
            break;

        case '3':
            {
                CN_OM_PortPriData_T data;
                UI32_T              priority, lport;

                BACKDOOR_MGR_Print("\r\n please specify a priority (0-7): ");
                BACKDOOR_MGR_RequestKeyIn(buffer, 1);
                priority = (UI32_T)strtol(buffer, NULL, 10);

                BACKDOOR_MGR_Print("\r\n please specify a logical port: ");
                BACKDOOR_MGR_RequestKeyIn(buffer, 3);
                lport = (UI32_T)strtol(buffer, NULL, 10);

                if (CN_OM_GetPortPriData(priority, lport, &data)
                        != CN_TYPE_RETURN_OK)
                {
                    BACKDOOR_MGR_Printf("\r\n Failed to get port-pri data for "
                        "priority=%lu, lport=%lu",
                        (unsigned long)priority, (unsigned long)lport);
                    break;
                }

                BACKDOOR_MGR_Printf("\r\n");
                BACKDOOR_MGR_Printf("\r\n Priority: %lu", (unsigned long)priority);
                BACKDOOR_MGR_Printf("\r\n Logical port: %lu", (unsigned long)lport);
                BACKDOOR_MGR_Printf("\r\n Active: %s", (data.active == TRUE)
                    ? "True" : "False");
                BACKDOOR_MGR_Printf("\r\n Admin defense mode: %s",
                    defense_mode_str[data.admin_defense_mode]);
                BACKDOOR_MGR_Printf("\r\n Oper defense mode: %s",
                    defense_mode_str[data.oper_defense_mode]);
                if (data.admin_alt_priority ==
                        CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
                {
                    BACKDOOR_MGR_Printf("\r\n Admin alternate priority: "
                        "By-Global");
                }
                else
                {
                    BACKDOOR_MGR_Printf("\r\n Admin alternate priority: %lu",
                        (unsigned long)data.admin_alt_priority);
                }
                BACKDOOR_MGR_Printf("\r\n Oper alternate priority: %lu",
                    (unsigned long)data.oper_alt_priority);
                BACKDOOR_MGR_Printf("\r\n Transmit ready: %s",
                    (data.tx_ready == TRUE) ? "True" : "False");
            }
            break;

        case '4':
            {
                CN_OM_CpData_T  data;
                UI32_T          lport, cp_index;

                BACKDOOR_MGR_Print("\r\n please specify a logical port: ");
                BACKDOOR_MGR_RequestKeyIn(buffer, 3);
                lport = (UI32_T)strtol(buffer, NULL, 10);

                BACKDOOR_MGR_Print("\r\n please specify a CP index: ");
                BACKDOOR_MGR_RequestKeyIn(buffer, 1);
                cp_index = (UI32_T)strtol(buffer, NULL, 10);

                if (CN_OM_GetCpData(lport, cp_index, &data) !=
                        CN_TYPE_RETURN_OK)
                {
                    BACKDOOR_MGR_Printf("\r\n Failed to get per-CP entry for "
                        "lport=%lu, cp_index=%lu", (unsigned long)lport, (unsigned long)cp_index);
                    break;
                }

                BACKDOOR_MGR_Printf("\r\n");
                BACKDOOR_MGR_Printf("\r\n Logical port: %lu", (unsigned long)lport);
                BACKDOOR_MGR_Printf("\r\n CP index: %lu", (unsigned long)cp_index);
                BACKDOOR_MGR_Printf("\r\n Active: %s", (data.active == TRUE)
                    ? "True" : "False");
                BACKDOOR_MGR_Printf("\r\n Mapped Queue: %lu", (unsigned long)data.queue);
                BACKDOOR_MGR_Printf("\r\n Managed CNPVs: 0x%02X",
                    data.managed_cnpvs);
                BACKDOOR_MGR_Printf("\r\n Set Point: %lu", (unsigned long)data.set_point);
                BACKDOOR_MGR_Printf("\r\n Feedback weight: 2^(%hu-2)",
                    data.feedback_weight);
                BACKDOOR_MGR_Printf("\r\n Minimum sample base: %lu",
                    (unsigned long)data.min_sample_base);
            }
            break;

        case '5':
            {
                UI32_T  lport;

                BACKDOOR_MGR_Print("\r\n please specify a logical port: ");
                BACKDOOR_MGR_RequestKeyIn(buffer, 3);
                lport = (UI32_T)strtol(buffer, NULL, 10);
                BACKDOOR_MGR_Printf("\r\n curren tag removal bitmap for lport "
                    "%lu: 0x%02X", (unsigned long)lport, CN_ENGINE_CnTagRemovalBitmap(lport));
            }
            break;

        case '6':
            {
                UI32_T  lport;

                BACKDOOR_MGR_Print("\r\n please specify a logical port: ");
                BACKDOOR_MGR_RequestKeyIn(buffer, 3);
                lport = (UI32_T)strtol(buffer, NULL, 10);
                BACKDOOR_MGR_Printf("\r\n curren L4 register counter for lport "
                    "%lu: %lu", (unsigned long)lport, (unsigned long)CN_ENGINE_L4RegisterCount(lport));
            }
            break;

        case 'e':
            BACKDOOR_MGR_Printf("\r\n current error debug value is %s",
                ((CN_OM_ErrDebug() == TRUE) ? "ON" : "OFF"));
            BACKDOOR_MGR_Print("\r\n enter new flag value (1) ON (2) OFF : ");
            choice = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf("%c", choice);
            if (choice == '1')
                CN_OM_SetErrDebug(TRUE);
            else if (choice == '2')
                CN_OM_SetErrDebug(FALSE);
            else
                BACKDOOR_MGR_Print("\r\n Invalid flag value!!");
            break;

        case 't':
            BACKDOOR_MGR_Printf("\r\n current thread debug value is %s",
                ((CN_OM_ThreadDebug() == TRUE) ? "ON" : "OFF"));
            BACKDOOR_MGR_Print("\r\n enter new flag value (1) ON (2) OFF : ");
            choice = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf("%c", choice);
            if (choice == '1')
                CN_OM_SetThreadDebug(TRUE);
            else if (choice == '2')
                CN_OM_SetThreadDebug(FALSE);
            else
                BACKDOOR_MGR_Print("\r\n Invalid flag value!!");
            break;

#if (CN_UNIT_TEST == TRUE)
        case 'u':
            CN_BACKDOOR_UnitTest();
            break;
#endif

        case 'x':
            return;

        default:
            BACKDOOR_MGR_Printf("\r\n Invalid choice!!");
        }

        BACKDOOR_MGR_Print("\r\n");
    } /* end while (1) */
} /* End of CN_BACKDOOR_Main */

/* LOCAL SUBPROGRAM BODIES
 */

#if (CN_UNIT_TEST == TRUE)
static void CN_BACKDOOR_UnitTest(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  status;
    UI32_T  priority;
    UI32_T  mode;

    /* BODY
     */

    BACKDOOR_MGR_Print("\r\n");

    /* CN2.1 SET_THEN_GET_CHECK(status) */
    if (CN_MGR_SetGlobalAdminStatus(CN_TYPE_GLOBAL_STATUS_ENABLE) !=
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.1a: failed to enable global admin status");
    }
    else if (CN_OM_GetGlobalAdminStatus(&status) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.1b: failed to get global admin status");
    }
    else if (status != CN_TYPE_GLOBAL_STATUS_ENABLE)
    {
        BACKDOOR_MGR_Printf("\r\n2.1c: invalid global admin status");
    }
    else if (CN_MGR_SetGlobalAdminStatus(CN_TYPE_GLOBAL_STATUS_DISABLE) !=
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.1d: failed to disable global admin status");
    }
    else if (CN_OM_GetGlobalAdminStatus(&status) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.1e: failed to get global admin status");
    }
    else if (status != CN_TYPE_GLOBAL_STATUS_DISABLE)
    {
        BACKDOOR_MGR_Printf("\r\n2.1f: invalid global admin status");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n2.1: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN2.2 SET_THEN_GETRUNNING_CHECK(status) */
    if (SYS_DFLT_CN_GLOBAL_ADMIN_STATUS == CN_TYPE_GLOBAL_STATUS_ENABLE)
    {
        status = CN_TYPE_GLOBAL_STATUS_DISABLE;
    }
    else
    {
        status = CN_TYPE_GLOBAL_STATUS_ENABLE;
    }
    if (CN_MGR_SetGlobalAdminStatus(status) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.2a: failed to set non-default global admin "
            "status");
    }
    else if (CN_OM_GetRunningGlobalAdminStatus(&status) !=
        SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
    {
        BACKDOOR_MGR_Printf("\r\n2.2b: failed to get running global admin "
            "status");
    }
    else if (status == SYS_DFLT_CN_GLOBAL_ADMIN_STATUS)
    {
        BACKDOOR_MGR_Printf("\r\n2.2c: invalid global admin status");
    }
    else if (CN_MGR_SetGlobalAdminStatus(SYS_DFLT_CN_GLOBAL_ADMIN_STATUS) !=
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.2d: failed to set default global admin "
            "status");
    }
    else if (CN_OM_GetRunningGlobalAdminStatus(&status) !=
        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE)
    {
        BACKDOOR_MGR_Printf("\r\n2.2e: failed to get running global admin "
            "status");
    }
    else if (status != SYS_DFLT_CN_GLOBAL_ADMIN_STATUS)
    {
        BACKDOOR_MGR_Printf("\r\n2.2f: invalid global admin status");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n2.2: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN2.3 INTEGER_BOUNDARY_CHECK(priority) */
    if (CN_MGR_SetCnmTxPriority(CN_TYPE_MIN_PRIORITY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.3a: failed to set CNM transmit priority to "
            "%lu", (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_MGR_SetCnmTxPriority(CN_TYPE_MAX_PRIORITY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.3b: failed to set CNM transmit priority to "
            "%lu", (unsigned long)CN_TYPE_MAX_PRIORITY);
    }
    else if (CN_MGR_SetCnmTxPriority((CN_TYPE_MIN_PRIORITY+
        CN_TYPE_MAX_PRIORITY)/2) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.3c: failed to set CNM transmit priority to "
            "%lu", (unsigned long)(CN_TYPE_MIN_PRIORITY+CN_TYPE_MAX_PRIORITY)/2);
    }
    else if (CN_MGR_SetCnmTxPriority(CN_TYPE_MIN_PRIORITY-1) ==
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.3d: invalid pass setting CNM transmit "
            "priority to %lu", (unsigned long)CN_TYPE_MIN_PRIORITY-1);
    }
    else if (CN_MGR_SetCnmTxPriority(CN_TYPE_MAX_PRIORITY+1) ==
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.3e: invalid pass setting CNM transmit "
            "priority to %lu", (unsigned long)CN_TYPE_MAX_PRIORITY+1);
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n2.3: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN2.4 SET_THEN_GET_CHECK(priority) */
    if (CN_MGR_SetCnmTxPriority(CN_TYPE_MIN_PRIORITY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.4a: failed to set CNM transmit priority"
            "%lu", (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_OM_GetCnmTxPriority(&priority) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.4b: failed to get CNM transmit priority");
    }
    else if (priority != CN_TYPE_MIN_PRIORITY)
    {
        BACKDOOR_MGR_Printf("\r\n2.4c: invalid CNM transmit priority");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n2.4: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN2.5 SET_THEN_GETRUNNING_CHECK(priority) */
    if (SYS_DFLT_CN_CNM_TRANSMIT_PRIORITY == CN_TYPE_MIN_PRIORITY)
    {
        priority = SYS_DFLT_CN_CNM_TRANSMIT_PRIORITY + 1;
    }
    else
    {
        priority = SYS_DFLT_CN_CNM_TRANSMIT_PRIORITY - 1;
    }
    if (CN_MGR_SetCnmTxPriority(priority) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.5a: failed to set non-default CNM transmit "
            "priority");
    }
    else if (CN_OM_GetRunningCnmTxPriority(&priority) !=
        SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
    {
        BACKDOOR_MGR_Printf("\r\n2.5b: failed to get running CNM transmit "
            "priority");
    }
    else if (priority == SYS_DFLT_CN_CNM_TRANSMIT_PRIORITY)
    {
        BACKDOOR_MGR_Printf("\r\n2.5c: invalid CNM transmit priority");
    }
    else if (CN_MGR_SetCnmTxPriority(SYS_DFLT_CN_CNM_TRANSMIT_PRIORITY) !=
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.5d: failed to set default CNM transmit "
            "priority");
    }
    else if (CN_OM_GetRunningCnmTxPriority(&priority) !=
        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE)
    {
        BACKDOOR_MGR_Printf("\r\n2.5e: failed to get running CNM transmit "
            "priority");
    }
    else if (priority != SYS_DFLT_CN_CNM_TRANSMIT_PRIORITY)
    {
        BACKDOOR_MGR_Printf("\r\n2.5f: invalid CNM transmit priority");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n2.5: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    if (CN_MGR_SetGlobalAdminStatus(CN_TYPE_GLOBAL_STATUS_ENABLE) !=
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n failed to enable global admin status, "
            "the following tests are bypassed");
        return;
    }

    /* CN2.6 INTEGER_BOUNDARY_CHECK(priority) */
    if (CN_MGR_SetCnpv(CN_TYPE_MIN_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.6a: failed to set CNPV %lu",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_MGR_SetCnpv(CN_TYPE_MIN_PRIORITY, FALSE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.6b: failed to reset CNPV %lu",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_MGR_SetCnpv(CN_TYPE_MAX_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.6c: failed to set CNPV %lu",
            (unsigned long)CN_TYPE_MAX_PRIORITY);
    }
    else if (CN_MGR_SetCnpv(CN_TYPE_MAX_PRIORITY, FALSE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.6d: failed to reset CNPV %lu",
            (unsigned long)CN_TYPE_MAX_PRIORITY);
    }
    else if (CN_MGR_SetCnpv((CN_TYPE_MIN_PRIORITY+CN_TYPE_MAX_PRIORITY)/2, TRUE)
        != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.6e: failed to set CNPV %lu",
            (unsigned long)(CN_TYPE_MIN_PRIORITY+CN_TYPE_MAX_PRIORITY)/2);
    }
    else if (CN_MGR_SetCnpv((CN_TYPE_MIN_PRIORITY+CN_TYPE_MAX_PRIORITY)/2,
                FALSE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.6f: failed to reset CNPV %lu",
            (unsigned long)(CN_TYPE_MIN_PRIORITY+CN_TYPE_MAX_PRIORITY)/2);
    }
    else if (CN_MGR_SetCnpv(CN_TYPE_MIN_PRIORITY-1, TRUE) == CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.6g: invalid setting CNPV %lu",
            (unsigned long)CN_TYPE_MIN_PRIORITY-1);
    }
    else if (CN_MGR_SetCnpv(CN_TYPE_MAX_PRIORITY+1, TRUE) == CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.6h: invalid setting CNPV %lu",
            (unsigned long)CN_TYPE_MAX_PRIORITY+1);
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n2.6: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN2.7 SET_THEN_GET_CHECK(active) */
    if (CN_MGR_SetCnpv(CN_TYPE_MIN_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.7a: failed to set priority %lu to a CNPV",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_OM_IsCnpv(CN_TYPE_MIN_PRIORITY) != TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n2.7b: CNPV %lu check failed",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_MGR_SetCnpv(CN_TYPE_MIN_PRIORITY, FALSE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n2.7c: failed to set priority %lu to a "
            "non-CNPV", (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_OM_IsCnpv(CN_TYPE_MIN_PRIORITY) != FALSE)
    {
        BACKDOOR_MGR_Printf("\r\n2.7d: Non-CNPV %lu check failed",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n2.7: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN3.1 SET_THEN_GET_CHECK(mode) */
    if (CN_MGR_SetCnpv(CN_TYPE_MIN_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.1a: failed to set priority %lu to a CNPV",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_MGR_SetCnpvDefenseMode(CN_TYPE_MIN_PRIORITY,
            CN_TYPE_DEFENSE_MODE_INTERIOR_READY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.1b: failed to set defense mode for CNPV %lu",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_OM_GetPriDefenseMode(CN_TYPE_MIN_PRIORITY, &mode) !=
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.1c: failed to get defense mode for CNPV %lu",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (mode != CN_TYPE_DEFENSE_MODE_INTERIOR_READY)
    {
        BACKDOOR_MGR_Printf("\r\n3.1d: invalid defense mode");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n3.1: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN3.2 SET_THEN_GETRUNNING_CHECK(mode) */
    if (SYS_DFLT_CN_CNPV_DEFENSE_MODE == CN_TYPE_DEFENSE_MODE_DISABLED)
    {
        mode = SYS_DFLT_CN_CNPV_DEFENSE_MODE + 1;
    }
    else
    {
        mode = SYS_DFLT_CN_CNPV_DEFENSE_MODE - 1;
    }
    if (CN_MGR_SetCnpv(CN_TYPE_MAX_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.2a: failed to set priority %lu to a CNPV",
            (unsigned long)CN_TYPE_MAX_PRIORITY);
    }
    else if (CN_MGR_SetCnpvDefenseMode(CN_TYPE_MAX_PRIORITY, mode) !=
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.2b: failed to set non-default defense mode");
    }
    else if (CN_OM_GetRunningPriDefenseMode(CN_TYPE_MAX_PRIORITY, &mode) !=
        SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
    {
        BACKDOOR_MGR_Printf("\r\n3.2c: failed to get running defense mode");
    }
    else if (mode == SYS_DFLT_CN_CNPV_DEFENSE_MODE)
    {
        BACKDOOR_MGR_Printf("\r\n3.2d: invalid defense mode");
    }
    else if (CN_MGR_SetCnpvDefenseMode(CN_TYPE_MAX_PRIORITY,
            SYS_DFLT_CN_CNPV_DEFENSE_MODE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.2e: failed to set default defense mode");
    }
    else if (CN_OM_GetRunningPriDefenseMode(CN_TYPE_MAX_PRIORITY, &mode) !=
        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE)
    {
        BACKDOOR_MGR_Printf("\r\n3.2c: failed to get running defense mode");
    }
    else if (mode != SYS_DFLT_CN_CNPV_DEFENSE_MODE)
    {
        BACKDOOR_MGR_Printf("\r\n3.2g: invalid defense mode");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n3.2: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN3.3 INTEGER_BOUNDARY_CHECK(alt_priority) */
    if (CN_MGR_SetCnpv(CN_TYPE_MAX_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.3a: failed to set priority %lu to a CNPV",
            (unsigned long)CN_TYPE_MAX_PRIORITY);
    }
    else if (CN_MGR_SetCnpvAlternatePriority(CN_TYPE_MAX_PRIORITY,
        CN_TYPE_MIN_PRIORITY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.3b: failed to set alternate priority to %lu",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_MGR_SetCnpvAlternatePriority(CN_TYPE_MAX_PRIORITY,
        CN_TYPE_MAX_PRIORITY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.3c: failed to set alternate priority to %lu",
            (unsigned long)CN_TYPE_MAX_PRIORITY);
    }
    else if (CN_MGR_SetCnpvAlternatePriority(CN_TYPE_MAX_PRIORITY,
        (CN_TYPE_MIN_PRIORITY+CN_TYPE_MAX_PRIORITY)/2) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.3d: failed to set alternate priority to %lu",
            (unsigned long)(CN_TYPE_MIN_PRIORITY+CN_TYPE_MAX_PRIORITY)/2);
    }
    else if (CN_MGR_SetCnpvAlternatePriority(CN_TYPE_MAX_PRIORITY,
        CN_TYPE_MIN_PRIORITY-1) == CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.3e: invalid setting alternate priority to "
            "%lu", (unsigned long)CN_TYPE_MIN_PRIORITY-1);
    }
    else if (CN_MGR_SetCnpvAlternatePriority(CN_TYPE_MAX_PRIORITY,
        CN_TYPE_MAX_PRIORITY+1) == CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.3f: invalid setting alternate priority to "
            "%lu", (unsigned long)CN_TYPE_MAX_PRIORITY+1);
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n3.3: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN3.4 SET_THEN_GET_CHECK(alt_priority) */
    if (CN_MGR_SetCnpv(CN_TYPE_MIN_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.4a: failed to set priority %lu to a CNPV",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    if (CN_MGR_SetCnpvAlternatePriority(CN_TYPE_MIN_PRIORITY,
        CN_TYPE_MAX_PRIORITY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.4b: failed to set alternate priority to %lu",
            (unsigned long)CN_TYPE_MAX_PRIORITY);
    }
    if (CN_OM_GetPriAlternatePriority(CN_TYPE_MIN_PRIORITY, &priority) !=
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.4c: failed to get alternate priority for "
            "CNPV %lu", (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    if (priority != CN_TYPE_MAX_PRIORITY)
    {
        BACKDOOR_MGR_Printf("\r\n3.4d: invalid alternate priority");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n3.4: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN3.5 SET_THEN_GETRUNNING_CHECK(alt_priority) */
    if (SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY == CN_TYPE_MIN_PRIORITY)
    {
        priority = SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY + 1;
    }
    else
    {
        priority = SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY - 1;
    }
    if (CN_MGR_SetCnpv(CN_TYPE_MIN_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.5a: failed to set priority %lu to a CNPV",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_MGR_SetCnpvAlternatePriority(CN_TYPE_MIN_PRIORITY, priority) !=
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.5b: failed to set non-default alternate "
            "priority");
    }
    else if (CN_OM_GetRunningPriAlternatePriority(CN_TYPE_MIN_PRIORITY,
        &priority) != SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
    {
        BACKDOOR_MGR_Printf("\r\n3.5c: failed to get running alternate "
            "priority");
    }
    else if (priority == SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY)
    {
        BACKDOOR_MGR_Printf("\r\n3.5d: invalid alternate priority");
    }
    else if (CN_MGR_SetCnpvAlternatePriority(CN_TYPE_MIN_PRIORITY,
        SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n3.5e: failed to set default alternate "
            "priority");
    }
    else if (CN_OM_GetRunningPriAlternatePriority(CN_TYPE_MIN_PRIORITY,
        &priority) != SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE)
    {
        BACKDOOR_MGR_Printf("\r\n3.5f: failed to get running alternate "
            "priority");
    }
    else if (priority != SYS_DFLT_CN_CNPV_ALTERNATE_PRIORITY)
    {
        BACKDOOR_MGR_Printf("\r\n3.5g: invalid alternate priority");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n3.5: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN4.1 SET_THEN_GET_CHECK(mode) */
    if (CN_MGR_SetCnpv(CN_TYPE_MIN_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.1a: failed to set priority %lu to a CNPV",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_MGR_SetPortCnpvDefenseMode(CN_TYPE_MIN_PRIORITY, 1,
        CN_TYPE_DEFENSE_MODE_INTERIOR_READY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.1b: failed to set defense mode for port");
    }
    else if (CN_OM_GetPortPriDefenseMode(CN_TYPE_MIN_PRIORITY, 1, &mode) !=
        CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.1b: failed to get defense mode of port");
    }
    else if (mode != CN_TYPE_DEFENSE_MODE_INTERIOR_READY)
    {
        BACKDOOR_MGR_Printf("\r\n4.1b: invalid defense mode of port");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n4.1: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN4.2 SET_THEN_GETRUNNING_CHECK(mode) */
    if (CN_MGR_SetCnpv(CN_TYPE_MAX_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.2a: failed to set priority %lu to a CNPV",
            (unsigned long)CN_TYPE_MAX_PRIORITY);
    }
    else if (CN_MGR_SetPortCnpvDefenseMode(CN_TYPE_MAX_PRIORITY, 1,
        CN_TYPE_DEFENSE_MODE_EDGE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.2b: failed to set non-default defense mode "
            "for port");
    }
    else if (CN_OM_GetRunningPortPriDefenseMode(CN_TYPE_MAX_PRIORITY, 1, &mode)
        != SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
    {
        BACKDOOR_MGR_Printf("\r\n4.2c: failed to get running defense mode of "
            "port");
    }
    else if (mode == CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
    {
        BACKDOOR_MGR_Printf("\r\n4.2d: invalid defense mode of port");
    }
    else if (CN_MGR_SetPortCnpvDefenseMode(CN_TYPE_MAX_PRIORITY, 1,
        CN_TYPE_DEFENSE_MODE_BY_GLOBAL) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.2e: failed to set default defense mode for "
            "port");
    }
    else if (CN_OM_GetRunningPortPriDefenseMode(CN_TYPE_MAX_PRIORITY, 1, &mode)
        != SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE)
    {
        BACKDOOR_MGR_Printf("\r\n4.2f: failed to get running defense mode of "
            "port");
    }
    else if (mode != CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
    {
        BACKDOOR_MGR_Printf("\r\n4.2g: invalid defense mode of port");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n4.2: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN4.3 INTEGER_BOUNDARY_CHECK(alt_priority) */
    if (CN_MGR_SetCnpv(CN_TYPE_MIN_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.3a: failed to set priority %lu to a CNPV",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_MGR_SetPortCnpvAlternatePriority(CN_TYPE_MIN_PRIORITY, 1,
        CN_TYPE_MIN_PRIORITY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.3b: failed to set alternate priority to %lu "
            "for port", (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_MGR_SetPortCnpvAlternatePriority(CN_TYPE_MIN_PRIORITY, 1,
        CN_TYPE_MAX_PRIORITY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.3c: failed to set alternate priority to %lu "
            "for port", (unsigned long)CN_TYPE_MAX_PRIORITY);
    }
    else if (CN_MGR_SetPortCnpvAlternatePriority(CN_TYPE_MIN_PRIORITY, 1,
        (CN_TYPE_MIN_PRIORITY+CN_TYPE_MAX_PRIORITY)/2) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.3d: failed to set alternate priority to %lu "
            "for port", (unsigned long)(CN_TYPE_MIN_PRIORITY+CN_TYPE_MAX_PRIORITY)/2);
    }
    else if (CN_MGR_SetPortCnpvAlternatePriority(CN_TYPE_MIN_PRIORITY, 1,
        CN_TYPE_MIN_PRIORITY-2) == CN_TYPE_RETURN_OK)
    {
        /* when CN_TYPE_MIN_PRIORITY=0, CN_TYPE_MIN_PRIORITY-1=0xffffffff, which
         * is CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL
         */
        BACKDOOR_MGR_Printf("\r\n4.3e: invalid setting alternate priority to "
            "%lu for port", (unsigned long)CN_TYPE_MIN_PRIORITY-2);
    }
    else if (CN_MGR_SetPortCnpvAlternatePriority(CN_TYPE_MIN_PRIORITY, 1,
        CN_TYPE_MAX_PRIORITY+1) == CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.3f: invalid setting alternate priority to "
            "%lu for port", (unsigned long)CN_TYPE_MAX_PRIORITY+1);
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n4.3: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN4.4 SET_THEN_GET_CHECK(alt_priority) */
    if (CN_MGR_SetCnpv(CN_TYPE_MIN_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.4a: failed to set priority %lu to a CNPV",
            (unsigned long)CN_TYPE_MIN_PRIORITY);
    }
    else if (CN_MGR_SetPortCnpvAlternatePriority(CN_TYPE_MIN_PRIORITY, 1,
        CN_TYPE_MAX_PRIORITY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.4b: failed to set alternate priority to %lu "
            "for port", (unsigned long)CN_TYPE_MAX_PRIORITY);
    }
    else if (CN_OM_GetPortPriAlternatePriority(CN_TYPE_MIN_PRIORITY, 1,
        &priority) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.4c: failed to get alternate priority of "
            "port");
    }
    else if (priority != CN_TYPE_MAX_PRIORITY)
    {
        BACKDOOR_MGR_Printf("\r\n4.4d: invalid alternate priority of port");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n4.4: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* CN4.5 SET_THEN_GETRUNNING_CHECK(alt_priority) */
    if (CN_MGR_SetCnpv(CN_TYPE_MAX_PRIORITY, TRUE) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.5a: failed to set priority %lu to a CNPV",
            (unsigned long)CN_TYPE_MAX_PRIORITY);
    }
    else if (CN_MGR_SetPortCnpvAlternatePriority(CN_TYPE_MAX_PRIORITY, 1,
        CN_TYPE_MIN_PRIORITY) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.5b: failed to set non-default alternate "
            "priority for port");
    }
    else if (CN_OM_GetRunningPortPriAlternatePriority(CN_TYPE_MAX_PRIORITY, 1,
        &priority) != SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
    {
        BACKDOOR_MGR_Printf("\r\n4.5c: failed to get running alternate priority"
            " of port");
    }
    else if (priority == CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
    {
        BACKDOOR_MGR_Printf("\r\n4.5d: invalid alternate priority of port");
    }
    else if (CN_MGR_SetPortCnpvAlternatePriority(CN_TYPE_MAX_PRIORITY, 1,
        CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL) != CN_TYPE_RETURN_OK)
    {
        BACKDOOR_MGR_Printf("\r\n4.5e: failed to set default alternate priority"
            " for port");
    }
    else if (CN_OM_GetRunningPortPriAlternatePriority(CN_TYPE_MAX_PRIORITY, 1,
        &priority) != SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE)
    {
        BACKDOOR_MGR_Printf("\r\n4.5f: failed to get running alternate priority"
            " of port");
    }
    else if (priority != CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
    {
        BACKDOOR_MGR_Printf("\r\n4.5g: invalid alternate priority of port");
    }
    else
    {
        BACKDOOR_MGR_Printf("\r\n4.5: PASSED!!");
    }
    BACKDOOR_MGR_Printf("\r\n");

    return;
} /* End of CN_BACKDOOR_UnitTest */
#endif /* #if (CN_UNIT_TEST == TRUE) */

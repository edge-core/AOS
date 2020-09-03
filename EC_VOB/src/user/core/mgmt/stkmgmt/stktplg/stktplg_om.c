/* Module Name: STKTPLG_OM.C
 * Purpose: This file contains the information of stack topology:
 *
 * Notes:
 *
 * History:
 *    10/04/2002       -- David Lin, Create
 *
 * Copyright(C)      Accton Corporation, 2002 - 2005
 *
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>   /* required for inclusion of <linux/if.h> */
#include <linux/if.h>     /* for IFNAMSIZ*/
#include <fcntl.h>        /* for open(), O_RDWR */
#include <linux/if_tun.h> /* for IFF_TAP */
#include <sys/ioctl.h>    /* for ioctl() */
#include <unistd.h>       /* for close() */

#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include "sys_ver.h"
#include "stkmgmt_type.h"
#include "stktplg_type.h"
#include "l_stdlib.h"
#include "l_charset.h"
#include "swctrl_pom.h"
#include "backdoor_mgr.h"

#define PASS_COMPILER_WORKAROUND /* TBD: remove PASS_COMPILER_WORKAROUND when other CSCs are ready */

#ifndef PASS_COMPILER_WORKAROUND /* TBD: remove PASS_COMPILER_WORKAROUND when xfer_pmgr.h is ready */
#include "xfer_pmgr.h"
#else
#define XFER_PMGR_AutoDownLoad(arg1,arg2,arg3,arg4,arg5,arg6,arg7)
#endif

#include "stktplg_om.h"
#include "stktplg_shom.h"
#include "stktplg_om_private.h"
#include "stktplg_board.h"
#include "sysfun.h"

#include "dev_swdrv.h"

#include "leaf_es3626a.h"
#if (SYS_CPNT_MAU_MIB == TRUE)
#include "leaf_3636.h"
#endif

#include "uc_mgr.h"

#include "leddrv.h"

#include "fs.h"
#include "fs_type.h"

#include "leaf_sys.h"

#include "cli_pom.h"
/* TBD: remove when cli_pom is ready for testing-- BEGIN*/
#if 0 /*add by fen.wang,in order to support hotswap*/
#define CLI_POM_GetStackingDB(arg1) ({0;})
#endif
#define CLI_POM_GetConfiguartionModuleTypes(arg1) FALSE
/* TBD: remove when cli_pom is ready for testing-- END */

#include "sysdrv.h"
#include "swdrv.h"
#include "swdrv_om.h"
#include "phyaddr_access.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define xgs_stacking_om_debug(fmt, arg...)    \
                if (stktplg_om_debug_mode & STKTPLG_OM_DEBUG_MODE_FLAG_DEBUG_MSG)    \
                    BACKDOOR_MGR_Printf(fmt, ##arg)

#define GET_UBOOT_VER_SCRIPT_FILE "/etc/get_uboot_ver.sh"
#define RUNTIME_VER_FILE          "/etc/runtime_ver"

/* DATA TYPE DECLARATIONS
 */

#define STK_UNIT_CFG_SIZE               sizeof(STK_UNIT_CFG_T)
#define LOCAL_UNIT                      0

#define STKTPLG_OM_NBR_OF_MODULE_TYPES  2

typedef struct
{
    UI32_T module_type;
    UI32_T nbr_of_ports;
} STKTPLG_OM_ModulePortNum_T;

/* MACRO DECLARATIONS
 */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
#define STKTPLG_OM_CHECK_STATE()    {                           \
    if ((ctrl_info_snapshot.state != STKTPLG_STATE_MASTER) &&            \
        (ctrl_info_snapshot.state != STKTPLG_STATE_SLAVE)  &&            \
        (ctrl_info_snapshot.state != STKTPLG_STATE_STANDALONE) )    {    \
        return (FALSE);                                         \
        }                                                       \
    }
#else
#define STKTPLG_OM_CHECK_STATE()    {                           \
    if ((ctrl_info.state != STKTPLG_STATE_MASTER) &&            \
        (ctrl_info.state != STKTPLG_STATE_SLAVE)  &&            \
        (ctrl_info.state != STKTPLG_STATE_STANDALONE) )    {    \
        return (FALSE);                                         \
        }                                                       \
    }
#endif

#if (SYS_HWCFG_LITTLE_ENDIAN_CPU==TRUE)
#define STKTPLG_OM_VERSION_NO_ASSIGN() \
    VersionNo[0]=(UI8_T)VersionNo_32[3];\
    VersionNo[1]=(UI8_T)VersionNo_32[2];\
    VersionNo[2]=(UI8_T)VersionNo_32[1];\
    VersionNo[3]=(UI8_T)VersionNo_32[0];
#else
#define STKTPLG_OM_VERSION_NO_ASSIGN() \
    VersionNo[0]=(UI8_T)VersionNo_32[0];\
    VersionNo[1]=(UI8_T)VersionNo_32[1];\
    VersionNo[2]=(UI8_T)VersionNo_32[2];\
    VersionNo[3]=(UI8_T)VersionNo_32[3];
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void     STKTPLG_OM_SetNPortOfCFG(UI32_T unit_id);
static void     STKTPLG_OM_SetSystemLedsDrive(UI8_T mask_bit, UI8_T value);
static UI8_T    STKTPLG_OM_GetStackingDBFromFS(STKTPLG_OM_StackingDB_T stacking_db[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]) ;
static BOOL_T   STKTPLG_OM_ReadLine_From_XferBuf(UI8_T *cmd_buf, UI8_T **xbuf_p);
static int      STKTPLG_OM_AtoEa(UI8_T *s, UI8_T *en);
static UI32_T   STKTPLG_OM_AtoUl(UI8_T *s, int radix);
static BOOL_T   STKTPLG_OM_CheckIfUnitUsed(int unit_id,UI8_T *mac);

static BOOL_T STKTPLG_OM_TranslateEntPhysicalIndexToTypeUnitIndex(I32_T index, UI32_T *type_p, UI32_T *unit_p, UI32_T *port_p);
static BOOL_T STKTPLG_OM_FindNextEntPhysicalIndex (I32_T *entIndex);
static BOOL_T STKTPLG_OM_IsVaildEntPhysicalIndex(I32_T index);
static BOOL_T STKTPLG_OM_MakeOidFromDot(STKTPLG_OM_OID_T *oid_P, I8_T *text_p);

static BOOL_T STKTPLG_OM_CreateTapDevice(const char* devname_p);
static BOOL_T STKTPLG_OM_UserPortToSfpIndexInternal(UI32_T unit, UI32_T port, UI32_T *sfp_index, UI8_T *sfp_type_p, BOOL_T *is_break_out);

    /* OM shared memory KEY */
#define STKTPLG_OM_SHARED_MEM_KEY       0x5379
typedef struct
{
    STK_UNIT_CFG_T                       stk_unit_cfg[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    STKTPLG_OM_switchModuleInfoEntry_T   stk_module_cfg[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][STKTPLG_OM_MAX_MODULE_NBR];
    STKTPLG_OM_Ctrl_Info_T  ctrl_info;
    UI8_T                   system_leds_drive_register;
    UI8_T                   software_reset_register;
    UI8_T                   switch_op_state;
    Stacking_Info_T         stktplg_stacking_info;
    BOOL_T                  slave_ready:1;
    BOOL_T                  transition_ready:1;
    BOOL_T                  provision_completed:1;
    UI8_T                   stktplg_om_debug_mode;

    /* Aaron add for check the hot-swap module type is the same as old or not, 2001/11/26 */
    UI8_T                   old_module_type[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];
    UI8_T                   old_exp_module_type[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];
    STKTPLG_DataBase_Info_T stackingdb[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    STKTPLG_OM_StackingDB_T current_temp_stackingdb[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    STKTPLG_OM_ModulePortNum_T stktplg_om_exp_module_port_number_table[STKTPLG_OM_NBR_OF_MODULE_TYPES];
    /* for hwcfg virtual addresses used in PHYADDR_ACCESS */
    SYS_TYPE_VAddr_T sys_hwcfg_module_id_type_addr[2];
    SYS_TYPE_VAddr_T sys_hwcfg_scl_enable_ab_addr;
    SYS_TYPE_VAddr_T sys_hwcfg_scl_enable_cd_addr;
    SYS_TYPE_VAddr_T sys_hwcfg_system_led_addr;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STK_UNIT_CFG_T          stk_unit_cfg_snapshot[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    STKTPLG_OM_switchModuleInfoEntry_T   stk_module_cfg_snapshot[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][STKTPLG_OM_MAX_MODULE_NBR];
    STKTPLG_OM_Ctrl_Info_T  ctrl_info_snapshot;
    BOOL_T                  provision_completed_once; /*the whole system has provision completed*/
    STKTPLG_DataBase_Info_T past_stackingdb[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    BOOL_T provision_lost[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    BOOL_T unit_is_valid[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    BOOL_T unit_use_default[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    DEV_SWDRV_Device_Port_Mapping_T stackPortMappingTable[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
#endif
    STKTPLG_BOARD_BoardInfo_T units_board_info_p[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    STKTPLG_OM_EntPhysicalEntryRw_T table_for_entPhysicalEntryRw[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD+3];
}STKTPLG_OM_SHARED_MEMORY_STRUCT;

/* Define all fields for convinence */
#define stk_unit_cfg                    (stktplg_om_database_p->stk_unit_cfg)
#define stk_module_cfg                  (stktplg_om_database_p->stk_module_cfg)
#define ctrl_info                       stktplg_om_database_p->ctrl_info
#define system_leds_drive_register      stktplg_om_database_p->system_leds_drive_register
#define software_reset_register         stktplg_om_database_p->software_reset_register
#define slave_ready                     stktplg_om_database_p->slave_ready
#define switch_op_state                 stktplg_om_database_p->switch_op_state
#define stktplg_stacking_info           stktplg_om_database_p->stktplg_stacking_info
#define transition_ready                stktplg_om_database_p->transition_ready
#define provision_completed             stktplg_om_database_p->provision_completed
#define stktplg_om_debug_mode           stktplg_om_database_p->stktplg_om_debug_mode
#define old_module_type                 stktplg_om_database_p->old_module_type
#define old_exp_module_type             stktplg_om_database_p->old_exp_module_type
#define stackingdb                      stktplg_om_database_p->stackingdb
#define current_temp_stackingdb         stktplg_om_database_p->current_temp_stackingdb
#define stktplg_om_exp_module_port_number_table   stktplg_om_database_p->stktplg_om_exp_module_port_number_table
#define sys_hwcfg_module_id_type_addr             stktplg_om_database_p->sys_hwcfg_module_id_type_addr
#define sys_hwcfg_scl_enable_ab_addr              stktplg_om_database_p->sys_hwcfg_scl_enable_ab_addr
#define sys_hwcfg_scl_enable_cd_addr              stktplg_om_database_p->sys_hwcfg_scl_enable_cd_addr
#define sys_hwcfg_system_led_addr                 stktplg_om_database_p->sys_hwcfg_system_led_addr
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
#define stk_unit_cfg_snapshot                stktplg_om_database_p->stk_unit_cfg_snapshot
#define stk_module_cfg_snapshot              stktplg_om_database_p->stk_module_cfg_snapshot
#define ctrl_info_snapshot                   stktplg_om_database_p->ctrl_info_snapshot
#define provision_completed_once             stktplg_om_database_p->provision_completed_once
#define past_stackingdb                      stktplg_om_database_p->past_stackingdb
#define provision_lost                       stktplg_om_database_p->provision_lost
#define unit_is_valid                        stktplg_om_database_p->unit_is_valid
#define unit_use_default                     stktplg_om_database_p->unit_use_default
#define stackPortMappingTable                stktplg_om_database_p->stackPortMappingTable
#endif
#define units_board_info_p                   stktplg_om_database_p->units_board_info_p
#define table_for_entPhysicalEntryRw           stktplg_om_database_p->table_for_entPhysicalEntryRw
/* STATIC VARIABLE DECLARATIONS
 */
/* OM database pointer which will point to shared memory */
static STKTPLG_OM_SHARED_MEMORY_STRUCT *stktplg_om_database_p;
static UI32_T shm_id_static;
static UI32_T sem_id_static;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : STKTPLG_OM_AttachProcessResources
 * PURPOSE: This function attach OM resources
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_AttachProcessResources(void)
{
    int ret;

    ret = SYSFUN_CreateShMem( STKTPLG_OM_SHARED_MEM_KEY,
                              sizeof(STKTPLG_OM_SHARED_MEMORY_STRUCT),
                              &shm_id_static );
    if ( ret != SYSFUN_OK )
    {
        printf("%s(): SYSFUN_CreateShMem fails.\n", __FUNCTION__);
        return FALSE;
    }
    stktplg_om_database_p =
        (STKTPLG_OM_SHARED_MEMORY_STRUCT *)SYSFUN_AttachShMem(shm_id_static);
    if ( stktplg_om_database_p == NULL )
    {
        printf("%s(): SYSFUN_AttachShMem fails.\n", __FUNCTION__);
        return FALSE;
    }
    ret = SYSFUN_CreateRwSem(STKTPLG_OM_SHARED_MEM_KEY, &sem_id_static);
    if ( ret != SYSFUN_OK )
    {
        printf("%s(): SYSFUN_GetRwSem fails.\n", __FUNCTION__);
        return FALSE;
    }

    /* Attach stktplg board resources */
    return STKTPLG_BOARD_AttachProcessResources();
}
/* FUNCTION NAME : STKTPLG_OM_InitiateProcessResources
 * PURPOSE: This function initializes STKTPLG OM which inhabits in the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_InitiateProcessResources(void)
{
    UC_MGR_Sys_Info_T         uc_sys_info;
    int ret;

    if ( !STKTPLG_BOARD_InitiateProcessResources() )
    {
        printf("%s(): STKTPLG_BOARD_InitiateProcessResources fails.\n", __FUNCTION__);
        return FALSE;
    }
    ret = SYSFUN_CreateShMem( STKTPLG_OM_SHARED_MEM_KEY,
            sizeof(STKTPLG_OM_SHARED_MEMORY_STRUCT),
            &shm_id_static );
    if ( ret != SYSFUN_OK )
    {
        printf("%s(): SYSFUN_CreateShMem fails.\n", __FUNCTION__);
        return FALSE;
    }
    stktplg_om_database_p =
        (STKTPLG_OM_SHARED_MEMORY_STRUCT *)SYSFUN_AttachShMem(shm_id_static);
    if ( stktplg_om_database_p == NULL )
    {
        printf("%s(): SYSFUN_GetSem fails.\n", __FUNCTION__);
        return FALSE;
    }
    ret = SYSFUN_CreateRwSem(STKTPLG_OM_SHARED_MEM_KEY, &sem_id_static);
    if ( ret != SYSFUN_OK )
    {
        printf("%s(): SYSFUN_CreateShMem fails.\n", __FUNCTION__);
        return FALSE;
    }

    /* Need to call UC_MGR_InitiateProcessResources before entering this
     * function
     */
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    /* init board_info.post_ver in context of stktplg_proc
     */
    strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.post_ver), (char*)uc_sys_info.post_ver);

    /* if post_ver is empty, it means diag_proc has never been executed
     * take this case as post pass.
     */
    if (uc_sys_info.post_ver[0] == 0)
    {
        uc_sys_info.post_pass=TRUE;
    }

    LEDDRV_SetDiagLedStatus(uc_sys_info.post_pass);

    stk_unit_cfg[LOCAL_UNIT].board_info.epld_ver[0]='\0';
#if (SYS_HWCFG_HAS_EPLD==TRUE)
    STKTPLG_BOARD_GetEPLDVer(uc_sys_info.board_id, (char*)(stk_unit_cfg[LOCAL_UNIT].board_info.epld_ver),
        &(uc_sys_info.epld_version));
#endif

    if (UC_MGR_SetSysInfo(uc_sys_info)==FALSE)
    {
        printf("%s(%d)UC_MGR_SetSysInfo failed.\r\n", __FUNCTION__, __LINE__);
    }
    /* Move value initialization to STKTPLG_OM_InitiateSystemResources()
     */

    return (TRUE);
} /* End of STKTPLG_OM_InitiateProcessResources() */

/* FUNCTION NAME : STKTPLG_OM_InitiateSystemResources
 * PURPOSE: This function initializes STKTPLG OM system resources
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_InitiateSystemResources(void)
{
    UC_MGR_Sys_Info_T         uc_sys_info;
    int                       index,index1;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    UI32_T                    *uc_boot_reason_check_ptr;
#if (SYS_CPNT_STACKING_BUTTON == TRUE)
    UI32_T is_stacking_enable=0,up_link_port=0,down_link_port=0;
#endif
    int ret;
#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    BOOL_T is_stacking_button_pressed=FALSE;
#endif
    char devname[IFNAMSIZ+1]={0};
    UI8_T max_port_num,i;

    if ( !STKTPLG_BOARD_InitiateProcessResources() )
    {
        printf("%s(): STKTPLG_BOARD_InitiateProcessResources fails.\n", __FUNCTION__);
        return FALSE;
    }

    /* create tap device for each front port
     */
    max_port_num=STKTPLG_BOARD_GetMaxPortNumberOnBoard();
    for (i=1; i<=max_port_num; i++)
    {
        snprintf(devname, IFNAMSIZ, "swp%hu", i);
        if(STKTPLG_OM_CreateTapDevice(devname)==FALSE)
            printf("Failed to create tap device %s.\r\n", devname);
    }

    /* create tap device for each trunk
     */
    for (i=1; i<=SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; i++)
    {
        snprintf(devname, IFNAMSIZ, "swb%hu", i);
        if(STKTPLG_OM_CreateTapDevice(devname)==FALSE)
            printf("Failed to create tap device %s.\r\n", devname);
    }

    ret = SYSFUN_CreateShMem( STKTPLG_OM_SHARED_MEM_KEY,
            sizeof(STKTPLG_OM_SHARED_MEMORY_STRUCT),
            &shm_id_static );
    if ( ret != SYSFUN_OK )
    {
        printf("%s(): SYSFUN_CreateShMem fails.\n", __FUNCTION__);
        return FALSE;
    }
    stktplg_om_database_p =
        (STKTPLG_OM_SHARED_MEMORY_STRUCT *)SYSFUN_AttachShMem(shm_id_static);
    if ( stktplg_om_database_p == NULL )
    {
        printf("%s(): SYSFUN_GetSem fails.\n", __FUNCTION__);
        return FALSE;
    }
    ret = SYSFUN_CreateRwSem(STKTPLG_OM_SHARED_MEM_KEY, &sem_id_static);
    if ( ret != SYSFUN_OK )
    {
        printf("%s(): SYSFUN_CreateShMem fails.\n", __FUNCTION__);
        return FALSE;
    }

    stktplg_om_debug_mode = 0;
    stktplg_om_exp_module_port_number_table[0].module_type = VAL_swModuleType_eightPortSfpModule;
    stktplg_om_exp_module_port_number_table[0].nbr_of_ports = 8;
    stktplg_om_exp_module_port_number_table[1].module_type = VAL_swModuleType_tenGigaPortModule;
    stktplg_om_exp_module_port_number_table[1].nbr_of_ports = 1;

    software_reset_register = 0xff;
    switch_op_state = VAL_switchOperState_ok;
    slave_ready = FALSE;

    /* Need to call UC_MGR_InitiateProcessResources before entering this
     * function
     */
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    if (!STKTPLG_BOARD_GetBoardInformation(uc_sys_info.board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    for (index = 0; index < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; index++)
        stktplg_stacking_info.expansion_module_id[index] = 0xff;

    /* clear control informaiton
     */
    memset(&ctrl_info, 0, sizeof(STKTPLG_OM_Ctrl_Info_T));

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    if (SYSDRV_GetStackingButtonStatus(&is_stacking_button_pressed) == FALSE)
        ctrl_info.stacking_button_state = SYS_DFLT_STACKING_BUTTON_SOFTWARE_STATUS;
    else
        ctrl_info.stacking_button_state = is_stacking_button_pressed;

    if (ctrl_info.stacking_button_state == TRUE)
        ctrl_info.stacking_port_option = STKTPLG_TYPE_STACKING_PORT_OPTION_ONE;
    else
        ctrl_info.stacking_port_option = STKTPLG_TYPE_STACKING_PORT_OPTION_OFF;
#endif

    /* initialized un-zero fields of control information
     */
    ctrl_info.image_type = uc_sys_info.project_id;
    ctrl_info.board_id = uc_sys_info.board_id;
    memcpy(ctrl_info.my_mac, uc_sys_info.mac_addr, STKTPLG_MAC_ADDR_LEN);
    memcpy(ctrl_info.master_mac, ctrl_info.my_mac, STKTPLG_MAC_ADDR_LEN);
    ctrl_info.my_unit_id = 1;
    ctrl_info.master_unit_id = 1;
    ctrl_info.total_units = 1;
    ctrl_info.chip_nums = board_info_p->chip_number_of_this_unit;

    /* michael - assuming both UP and DOWN stacking ports are connected */
    ctrl_info.stacking_ports_logical_link_status = 0;
    ctrl_info.bounce_msg             = 0;
    ctrl_info.total_units_up         = 1;
    ctrl_info.total_units_down       = 1;
    ctrl_info.preempted              = FALSE;
    ctrl_info.last_module_id         = 0;
    ctrl_info.my_phy_unit_id_up      = 1;
    ctrl_info.my_phy_unit_id_down    = 1;
    ctrl_info.expansion_module_type  = 0xff;
    ctrl_info.expansion_module_exist = FALSE;
    ctrl_info.expansion_module_id    = 0xff;
    ctrl_info.stacking_ports_link_checked = 0;
    ctrl_info.reset_state            = 1;

    //STKTPLG_ENGINE_ButtonPressed();

    memset(&ctrl_info.stable_hbt_up, 0, sizeof(STKTPLG_OM_HBT_0_1_T));
    memset(&ctrl_info.stable_hbt_down, 0, sizeof(STKTPLG_OM_HBT_0_1_T));
    memset(units_board_info_p,0,sizeof(units_board_info_p));


    ctrl_info.stable_hbt_up.payload[0].unit_id = 1;
    ctrl_info.stable_hbt_up.payload[0].start_module_id = 0;
    ctrl_info.stable_hbt_up.payload[0].chip_nums = board_info_p->chip_number_of_this_unit;
    ctrl_info.stable_hbt_up.payload[0].expansion_module_id = UNKNOWN_MODULE_ID;
    ctrl_info.stable_hbt_up.payload[0].board_id = ctrl_info.board_id;
    /*1223*/
    ctrl_info.stable_hbt_up.payload[0].provision_completed_state= FALSE;

    ctrl_info.stable_hbt_down.payload[0].unit_id = 1;
    ctrl_info.stable_hbt_down.payload[0].start_module_id = 0;
    ctrl_info.stable_hbt_down.payload[0].chip_nums = ctrl_info.stable_hbt_up.payload[0].chip_nums;
    ctrl_info.stable_hbt_down.payload[0].expansion_module_id = UNKNOWN_MODULE_ID;
    ctrl_info.stable_hbt_down.payload[0].board_id = ctrl_info.board_id;
    memcpy(ctrl_info.stable_hbt_up.payload[0].mac_addr ,ctrl_info.my_mac,STKTPLG_MAC_ADDR_LEN);
    memcpy(ctrl_info.stable_hbt_down.payload[0].mac_addr ,ctrl_info.my_mac,STKTPLG_MAC_ADDR_LEN);

    /* clear stk_unit_cfg
     */
    for (index = 0; index < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; index++)
    {
        memset(&stk_unit_cfg[index], 0, STK_UNIT_CFG_SIZE);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        memset(&stk_unit_cfg_snapshot[index], 0, STK_UNIT_CFG_SIZE);
#endif

        ctrl_info.exp_module_state_dirty[index] = FALSE;
        ctrl_info.expected_module_types[index] = 0; /*0 means no module*/
        ctrl_info.synced_module_types[index] = 0;        /*0 means no module*/

    }
    ctrl_info.stk_unit_cfg_dirty_sync_bmp = 0;
#if (SYS_CPNT_STACKING_BUTTON == TRUE)
    ctrl_info.stacking_button_state = TRUE;
    /* EPR:ES3628BT-FLF-ZZ-00081ES3628BT-FLF-ZZ-00104
Problem:port 27,28 cannot display in vlan information,when thye are not in stacking mode
Rootcasue: Not init stktplg board info
default stacking_button_state is set ,and board info is not set,make them the same
Solution:make the database equal
Files :stktplg_om.c,stktplg_engine.c
     */
    //STKTPLG_MGR_SetStackingPortInBoard(ctrl_info.stacking_button_state);
    STKTPLG_OM_GetStackingPortInfo(&is_stacking_enable,&up_link_port,&down_link_port);
    STKTPLG_SHOM_SetStackingPortInfo(ctrl_info.stacking_button_state,up_link_port,down_link_port);
#endif
    /*Default value is have stacking port*/

    /* Aaron add, 2001/11/26 for check hot-insert module is the same as last time insert or not.
     */
    memset(old_module_type, 0xff, sizeof(old_module_type));

    system_leds_drive_register = 0xff;

    /* prepare local unit configuration
     */
    memcpy(stk_unit_cfg[LOCAL_UNIT].board_info.mac_addr, uc_sys_info.mac_addr, STKTPLG_MAC_ADDR_LEN);

    if (uc_sys_info.serial_no[LOCAL_UNIT] >= 128) /* in-correct string */
        strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.serial_no), "");
    else
    {
        strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.serial_no), (char*)(uc_sys_info.serial_no));
    }

    if (uc_sys_info.mainboard_hw_ver[LOCAL_UNIT] >= 128) /* in-correct string */
        strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.mainboard_hw_ver), "");
    else
    {
        strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.mainboard_hw_ver), (char*)(uc_sys_info.mainboard_hw_ver));
    }

    strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.agent_hw_ver), "");
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    {
        /* ONIE uboot will not fill loader version to uc_sys_info.loader_ver
         * Need to get the version from uboot environment variable "ver"
         * The script file GET_UBOOT_VER_SCRIPT_FILE will extract the uboot
         * version string from the uboot environment variable "ver".
         */
        FILE   *fp;
        UI32_T  str_len;

        memset(uc_sys_info.loader_ver, 0, sizeof(uc_sys_info.loader_ver));
        fp = popen(GET_UBOOT_VER_SCRIPT_FILE, "r");
        if (fp == NULL)
        {
            printf("%s(%d)Failed to run %s\r\n",
                __FUNCTION__, __LINE__, GET_UBOOT_VER_SCRIPT_FILE);
        }
        else
        {
            if (fgets((char*)(uc_sys_info.loader_ver), sizeof(uc_sys_info.loader_ver), fp)==NULL)
            {
                printf("%s(%d):Failed to get uboot version from script %s\r\n",
                    __FUNCTION__, __LINE__, GET_UBOOT_VER_SCRIPT_FILE);
            }
            else
            {
                L_CHARSET_TrimTrailingNonPrintableChar((char*)uc_sys_info.loader_ver);
            }
            pclose(fp);
        }

    }
#endif /* #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */

    strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.loader_ver), (char*)(uc_sys_info.loader_ver));

    /* uc_sys_info.post_ver is initialized by diag_proc, and this function is
     * called in the context of sys_init_proc, which is executed before diag_proc
     * Thus board_info.post_ver should be initialized in the function
     * STKTPLG_OM_InitiateProcessResources(exeuted in context of stktplg_proc)
     */
    #if 0
    strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.post_ver), (char*)uc_sys_info.post_ver);
    #endif

    strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.post_customized_ver), SYS_VER_POST_CUSTOMIZED_VER);
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    /* ONIE uboot will not set runtime_fw_ver to uc_sys_info, need to get the
     * version info from /etc/runtime_ver
     */
    {
        FILE  *fp;
        UI32_T str_len;

        memset(uc_sys_info.runtime_fw_ver, 0, sizeof(uc_sys_info.runtime_fw_ver));
        fp = fopen(RUNTIME_VER_FILE, "r");
        if (fp==NULL)
        {
            printf("%s(%d):Failed to open %s\r\n", __FUNCTION__, __LINE__, RUNTIME_VER_FILE);
        }
        else
        {
            if (fgets((char*)(uc_sys_info.runtime_fw_ver), sizeof(uc_sys_info.runtime_fw_ver), fp)==NULL)
            {
                printf("%s(%d):Failed to get runtime version through file %s\r\n",
                    __FUNCTION__, __LINE__, RUNTIME_VER_FILE);
            }
            else
            {
                L_CHARSET_TrimTrailingNonPrintableChar((char*)(uc_sys_info.runtime_fw_ver));
            }
            fclose(fp);
        }
    }
#endif /* end of #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */

    strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.runtime_sw_ver), (char*)(uc_sys_info.runtime_fw_ver));
    strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.runtime_customized_sw_ver), SYS_VER_ROOTFS_CUSTOMIZED_VER);
    if(SYSFUN_GetKernelVer((char *)stk_unit_cfg[LOCAL_UNIT].board_info.kernel_ver)!=SYSFUN_OK)
        stk_unit_cfg[LOCAL_UNIT].board_info.kernel_ver[0]=0;
    if (uc_sys_info.model_number[LOCAL_UNIT] >= 128) /* in-correct string */
        strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.model_number), "");
    else
        strncpy((char*)(stk_unit_cfg[LOCAL_UNIT].board_info.model_number), (char*)(uc_sys_info.model_number),   SYS_ADPT_MODEL_NUMBER_LEN);

    stk_unit_cfg[LOCAL_UNIT].board_info.project_id = uc_sys_info.project_id;
    stk_unit_cfg[LOCAL_UNIT].board_info.board_id = uc_sys_info.board_id;

    if (uc_sys_info.sw_service_tag[LOCAL_UNIT] >= 128) /* in-correct string */
        strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].sw_service_tag), "");
    else
    {
        strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].sw_service_tag), (char*)(uc_sys_info.sw_service_tag));
    }

    memcpy(stk_unit_cfg[LOCAL_UNIT].module_expected_runtime_fw_ver,
            uc_sys_info.module_expected_runtime_fw_ver,
            SYS_ADPT_FW_VER_STR_LEN + 1);
#if (SYS_CPNT_MODULE_WITH_CPU == TRUE)
    printf("\r\nExpected module version: %s\r\n",
            uc_sys_info.module_expected_runtime_fw_ver);
#endif

    strcpy((char*)(stk_unit_cfg[LOCAL_UNIT].sw_chassis_service_tag), "");
    stk_unit_cfg[LOCAL_UNIT].sw_identifier = 1;
    stk_unit_cfg[LOCAL_UNIT].nport = board_info_p->max_port_number_on_board;
    stk_unit_cfg[LOCAL_UNIT].num_chips = board_info_p->chip_number_of_this_unit;

    /* Boot reason will not be saved in UC when using ONIE boot loader,
     * the following code is only effective if runtime SW does not work 
     * with ONIE.
     */
#if (SYS_CPNT_ONIE_SUPPORT == FALSE)
    /* To allocate and set magic word for boot reason
     */
    uc_boot_reason_check_ptr = (UI32_T *)UC_MGR_Allocate(UC_MGR_BOOT_REASON_CHECKING_INDEX, sizeof(UI32_T), 4);
    if (uc_boot_reason_check_ptr == 0)
    {
        printf("Cannot allocate in UC for STKTPLG.\r\n");
    }
    else
    {
        if (STKTPLG_BOOT_REASON_MAGIC_WORD == *uc_boot_reason_check_ptr)
        {
            stk_unit_cfg[LOCAL_UNIT].boot_reason = STKTPLG_OM_REBOOT_FROM_WARMSTART;
        }
        else
        {
            stk_unit_cfg[LOCAL_UNIT].boot_reason = STKTPLG_OM_REBOOT_FROM_COLDSTART;
        }

        *uc_boot_reason_check_ptr = STKTPLG_BOOT_REASON_MAGIC_WORD;
    }
#endif

    /* copy software build time from UC memory
     */
    stk_unit_cfg[LOCAL_UNIT].board_info.software_build_time = uc_sys_info.software_build_time_macro;

    /* Initialize module status and module type
     */
    for (index1=0; index1<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK ; index1++)
    {
        for (index=0; index<SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT; index++)
        {
            stk_unit_cfg[index1].module_presented[index] = 0;
            stk_unit_cfg[index1].module_type[index] = 0xff;

            stk_unit_cfg[index1].exp_module_presented[index] = 0;
            stk_unit_cfg[index1].exp_module_type[index]      = 0xff;
        }
    }


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    memset(&stackPortMappingTable, 0, sizeof(stackPortMappingTable));

    for (index = 0; index < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; index++)
    {
        provision_lost[index] = FALSE;
        unit_is_valid[index] = FALSE;
        unit_use_default[index] = TRUE;
    }
    unit_is_valid[0] = TRUE; /* set unit 1 to be valid */

    provision_completed_once = FALSE;
    provision_completed = FALSE;

    memcpy(ctrl_info.past_master_mac, ctrl_info.my_mac, STKTPLG_MAC_ADDR_LEN);
    ctrl_info.stable_flag            = FALSE;
    ctrl_info.past_role              = STKTPLG_STATE_INIT;

    STKTPLG_OM_CopyDatabaseToSnapShot();

    memcpy(&stackPortMappingTable[LOCAL_UNIT], board_info_p->userPortMappingTable, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*sizeof(DEV_SWDRV_Device_Port_Mapping_T));
#endif


    /* Initialize hwcfg virtual addresses
     */
#ifdef SYS_HWCFG_MODULE_1_ID_TYPE_ADDR /* Thomas added for build error */
    if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_MODULE_1_ID_TYPE_ADDR, &(sys_hwcfg_module_id_type_addr[0])))
    {
        printf("\r\nFail to get virtual addr of SYS_HWCFG_MODULE_1_ID_TYPE_ADDR");
        sys_hwcfg_module_id_type_addr[0]=0;
    }
#else
    sys_hwcfg_module_id_type_addr[0]=0;
#endif
#ifdef SYS_HWCFG_MODULE_2_ID_TYPE_ADDR /* Thomas added for build error */
    if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_MODULE_2_ID_TYPE_ADDR, &(sys_hwcfg_module_id_type_addr[1])))
    {
        printf("\r\nFail to get virtual addr of SYS_HWCFG_MODULE_2_ID_TYPE_ADDR");
        sys_hwcfg_module_id_type_addr[1]=0;
    }
#else
    sys_hwcfg_module_id_type_addr[1]=0;
#endif

    /*if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_COMBO_PORT1_SFP_STATUS_ADDR, &(sys_hwcfg_sfp_status_addr[0])))
      {
      printf("\r\nFail to get virtual addr of SYS_HWCFG_COMBO_PORT1_SFP_STATUS_ADDR");
      sys_hwcfg_module_id_type_addr[0]=0;
      }

      if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_COMBO_PORT2_SFP_STATUS_ADDR, &(sys_hwcfg_sfp_status_addr[1])))
      {
      printf("\r\nFail to get virtual addr of SYS_HWCFG_COMBO_PORT2_SFP_STATUS_ADDR");
      sys_hwcfg_module_id_type_addr[1]=0;
      }

      if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_COMBO_PORT3_SFP_STATUS_ADDR, &(sys_hwcfg_sfp_status_addr[2])))
      {
      printf("\r\nFail to get virtual addr of SYS_HWCFG_COMBO_PORT3_SFP_STATUS_ADDR");
      sys_hwcfg_module_id_type_addr[2]=0;
      }

      if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_COMBO_PORT4_SFP_STATUS_ADDR, &(sys_hwcfg_sfp_status_addr[3])))
      {
      printf("\r\nFail to get virtual addr of SYS_HWCFG_COMBO_PORT4_SFP_STATUS_ADDR");
      sys_hwcfg_module_id_type_addr[3]=0;
      }*/

#if 0 /* Thomas added for build error */
    if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_SCL_ENABLE_AB_ADDR, &sys_hwcfg_scl_enable_ab_addr))
    {
        printf("\r\nFail to get virtual addr of SYS_HWCFG_SCL_ENABLE_AB_ADDR");
        sys_hwcfg_scl_enable_ab_addr=0;
    }

    if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_SCL_ENABLE_CD_ADDR, &sys_hwcfg_scl_enable_cd_addr))
    {
        printf("\r\nFail to get virtual addr of SYS_HWCFG_SCL_ENABLE_CD_ADDR");
        sys_hwcfg_scl_enable_cd_addr=0;
    }
#endif
#ifdef SYS_HWCFG_SYSTEM_LED_ADDR /* Thomas added for build error */
    if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_SYSTEM_LED_ADDR, &sys_hwcfg_system_led_addr))
    {
        printf("\r\nFail to get virtual addr of SYS_HWCFG_SYSTEM_LED_ADDR");
        sys_hwcfg_system_led_addr=0;
    }
#else
    sys_hwcfg_system_led_addr=0;
#endif

    /* initial entPhysicalEntry.ent_physical_serial_num */
    memset(&table_for_entPhysicalEntryRw, 0, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * (SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD+3) * sizeof(STKTPLG_OM_EntPhysicalEntryRw_T));
    memcpy(&table_for_entPhysicalEntryRw[LOCAL_UNIT][1].ent_physical_serial_num ,(UI8_T *) &(stk_unit_cfg[LOCAL_UNIT].board_info.serial_no), STKTPLG_OM_MIB_STRING_LENGTH + 1);
    memcpy(&table_for_entPhysicalEntryRw[LOCAL_UNIT][2].ent_physical_serial_num ,(UI8_T *) &(stk_unit_cfg[LOCAL_UNIT].board_info.serial_no), STKTPLG_OM_MIB_STRING_LENGTH + 1);

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    if (UC_MGR_SetSysInfo(uc_sys_info)==FALSE)
    {
        printf("%s(%d)UC_MGR_SetSysInfo failed.\r\n", __FUNCTION__, __LINE__);
    }
#endif

    return TRUE;
}

/* FUNCTION NAME : STKTPLG_OM_GetNumberOfUnit
 * PURPOSE: To get the number of units existing in the stack.
 * INPUT:   None.
 * OUTPUT:  num_of_unit  -- the number of units existing in the stack.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetNumberOfUnit(UI32_T *num_of_unit)
{
    /* check if we are in master mode
     * this value is only meaningful when we are in master state
     */
    /* For the sake of nmtr_task.c */
    /* STKTPLG_OM_CHECK_STATE();
     */

    /* setting this field is not protected by semaphore. it should be
     * all right. If not, move this field out of control information
     */
    /* EPR: ES3628BT-FLF-ZZ-01008
Problem:stack:salver units ports cannot link up and display error information when reload the DUT
Rootcause:L4 get unit number and unit id from stktplg,and stktplg is not stable.Just link get the unit number is 3,and unit Id is 1,0,0. and unit id is invalid.Because  Stktplg has 2 database:one is for csc ,and one is for stktplg,when the topo is stable ,the database for csc will be updated by the stktplg
Unit number get from the stktplg database ,and unit id get from database for CSC.The database is not the same.
Solution:Get the database for the stable database
Files:stktplg_om.c*/
#if 0
    *num_of_unit = ctrl_info.total_units;
#else
    *num_of_unit = ctrl_info_snapshot.total_units;
#endif
    return (TRUE);

} /* End of STKTPLG_OM_GetNumberOfUnit() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME -  STKTPLG_OM_SetStackingInfo
 * -------------------------------------------------------------------------
 * FUNCTION: set stktplg stacking information
 * INPUT   : stack_info
 * OUTPUT  : NONE
 * RETURN  : NONE
 * NOTE    :
 * -------------------------------------------------------------------------*/

void  STKTPLG_OM_SetStackingInfo(Stacking_Info_T  stack_info)
{

    SYSFUN_TakeWriteSem(sem_id_static);
    memcpy(&stktplg_stacking_info,&stack_info,sizeof(Stacking_Info_T));
    SYSFUN_GiveWriteSem(sem_id_static);
    return;
}

/* FUNCTION NAME : STKTPLG_OM_GetLocalMaxPortCapability
 * PURPOSE: To get the maximum port number of the local unit.
 * INPUT:   None.
 * OUTPUT:  *max_port_number -- maximum port number of the local unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetLocalMaxPortCapability(UI32_T *max_port_number)
{
    UI8_T    board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    board_id = stk_unit_cfg_snapshot[ctrl_info_snapshot.my_unit_id-1].board_info.board_id;
    #else
    board_id = stk_unit_cfg[ctrl_info.my_unit_id-1].board_info.board_id;
    #endif

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    *max_port_number = board_info_p->max_port_number_of_this_unit;

    return (TRUE);

} /* End of STKTPLG_OM_GetLocalMaxPortCapability() */


/* FUNCTION NAME : STKTPLG_OM_GetMaxPortCapability
 * PURPOSE: To get the maximum port number of the target unit.
 * INPUT:   unit               -- unit id
 * OUTPUT:  *max_port_number_p -- maximum port number
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetMaxPortCapability(UI8_T unit, UI32_T *max_port_number_p)
{
/*    UI8_T    board_id;
    STKTPLG_BOARD_BoardInfo_T *board_info_p;*/


   /* unit = (unit == 1) ? LOCAL_UNIT : unit - 1;


    if (unit != LOCAL_UNIT)
    {
        STKTPLG_OM_CHECK_STATE();

        if (unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        {
            return (FALSE);
        }
    }   */

    /* Modified by Vincent on 6,Oct,2004
     * To get current total port numbers in the DUT
     */
  /*  board_id = stk_unit_cfg[unit-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    *max_port_number = board_info_p->max_port_number_of_this_unit; */

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;

    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    *max_port_number_p = stk_unit_cfg_snapshot[unit-1].nport;
    #else
    *max_port_number_p = stk_unit_cfg[unit-1].nport;
    #endif

    return TRUE;
}

/* FUNCTION NAME : STKTPLG_OM_GetMaxPortNumberOnBoard
 * PURPOSE: To get the maximum port number on board .
 * INPUT:   unit             -- unit id.
 * OUTPUT:  *max_port_number -- maximum port number on board.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   0           -- get local device.
 *          otherwise   -- get this device.
 */
BOOL_T STKTPLG_OM_GetMaxPortNumberOnBoard(UI8_T unit, UI32_T *max_port_number)
{
    UI8_T    board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    board_id = stk_unit_cfg_snapshot[unit-1].board_info.board_id;
#else
    board_id = stk_unit_cfg[unit-1].board_info.board_id;
#endif

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    *max_port_number = board_info_p->max_port_number_on_board;

    return (TRUE);
}

/* FUNCTION NAME : STKTPLG_OM_GetPortType
 * PURPOSE: To get the type of the specified port of the specified unit.
 * INPUT:   unit_id       -- unit id.
 *          u_port       -- phyical(user) port id
 * OUTPUT:  port_type    -- port type of the specified port of the specified unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   Reference leaf_es3526a.mib
 *          -- #define VAL_portType_other                 1L
 *          -- #define VAL_portType_hundredBaseTX         2L
 *          -- #define VAL_portType_hundredBaseFX         3L
 *          -- #define VAL_portType_thousandBaseSX        4L
 *          -- #define VAL_portType_thousandBaseLX        5L
 *          -- #define VAL_portType_thousandBaseT         6L
 *          -- #define VAL_portType_thousandBaseGBIC      7L
 *          -- #define VAL_portType_thousandBaseMiniGBIC  8L
 *
 */
BOOL_T STKTPLG_OM_GetPortType(UI32_T unit, UI32_T u_port, UI32_T *port_type)
{
    UI8_T                     board_id;
#if (SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT>0)
    UI8_T                     module_slot_index, index;
#endif
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    UI32_T starting_port;
    UI32_T number_of_port;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
if (unit != ctrl_info_snapshot.my_unit_id)
    {
        if (!STKTPLG_OM_UnitExist(unit))
        {
            return FALSE;
        }
    }

    board_id = stk_unit_cfg_snapshot[ctrl_info_snapshot.my_unit_id - 1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        return FALSE;
    }

    if (stackPortMappingTable[unit-1][u_port-1].device_id == 0xff ||
        stackPortMappingTable[unit-1][u_port-1].device_port_id == 0xff)
    {
        return FALSE;
    }

#if 0
    *port_type = board_info_p->userPortMappingTable[unit-1][u_port-1].port_type;
#else
    *port_type = stackPortMappingTable[unit-1][u_port-1].port_type;
#endif
    board_id = stk_unit_cfg_snapshot[unit - 1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        return FALSE;
    }

    /* check main board.
     */
    if (u_port>=1 && u_port <= board_info_p->max_port_number_on_board)
    {
        /*if the port is stacking port ,it is still use STKTPLG_PORT_TYPE_STACKING
         or user will get this port*/
        if(*port_type == STKTPLG_PORT_TYPE_STACKING )
            return TRUE;

        {
            UI8_T module_id;
            BOOL_T is_valid_module_id=FALSE;

            if (unit == ctrl_info_snapshot.my_unit_id)
            {
                if (STKTPLG_OM_GetModuleID(unit, u_port, &module_id))
                {
                    is_valid_module_id=TRUE;
                }
            }
            else
            {
#if (SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT>0)
                /* EPR ID: ES4626F-SW-FLF-38-01684
                 * Headline: 10G port will link down after reboot
                 * Root cause: In original design, the port type on the port of
                 *             10G module slot on remote unit will refer to the
                 *             port type defined in STKTPLG_BOARD_BoardInfo_T in
                 *             stktplg_board.c, which will cause incorrect port
                 *             type and configure the port with incorrect state
                 *             (e.g. autonego state) and cause link up problem
                 *             finally. New design will take the module id of
                 *             remote unit from HBT packet.
                 */
                if(STKTPLG_BOARD_GetModuleIndex(board_id, u_port, &module_slot_index)==TRUE)
                {
                    SYSFUN_TakeReadSem(sem_id_static);
                    for (index = 0; index < ctrl_info.total_units_up; index++)
                    {
                        if (ctrl_info.stable_hbt_up.payload[index].unit_id == unit)
                        {
                            module_id = ctrl_info.stable_hbt_up.payload[index].teng_module_id[module_slot_index];
                            is_valid_module_id = TRUE;
                            break;
                        }
                    }

                    if (is_valid_module_id == FALSE)
                    {
                        for (index = 0; index < ctrl_info.total_units_down; index++)
                        {
                            if (ctrl_info.stable_hbt_down.payload[index].unit_id == unit)
                            {
                                module_id = ctrl_info.stable_hbt_up.payload[index].teng_module_id[module_slot_index];
                                is_valid_module_id = TRUE;
                                break;
                            }
                        }
                    }
                    SYSFUN_GiveReadSem(sem_id_static);
                }
#endif /* end of #if (SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT>0) */
            }

            if(is_valid_module_id==TRUE)
            {
                switch (module_id)
                {
                    #ifdef SYS_HWCFG_MODULE_ID_10G_COPPER
                    case SYS_HWCFG_MODULE_ID_10G_COPPER:
                        *port_type = VAL_portType_tenGBaseT;
                        break;
                    #endif

                    #ifdef SYS_HWCFG_MODULE_ID_XFP
                    case SYS_HWCFG_MODULE_ID_XFP:
                        *port_type = VAL_portType_tenGBaseXFP;
                        break;
                    #endif

                    #ifdef SYS_HWCFG_MODULE_ID_10G_SFP
                    case SYS_HWCFG_MODULE_ID_10G_SFP:
                        *port_type = VAL_portType_tenGBaseSFP;
                        break;
                    #endif

                    #ifdef SYS_HWCFG_MODULE_ID_SFP_PLUS
                    case SYS_HWCFG_MODULE_ID_SFP_PLUS:
                        *port_type = VAL_portType_tenGBaseSFP;
                        break;
                    #endif

                    #ifdef SYS_HWCFG_MODULE_ID_SFP
                    case SYS_HWCFG_MODULE_ID_SFP:
                        *port_type = VAL_portType_thousandBaseSfp;
                        break;
                    #endif

                    default:
                        ;
                }
            }
        }

        /* yes, on main board.
         */
        return TRUE;
    }

    /* try to find module
     */
    if (FALSE == STKTPLG_OM_GetUnitModulePorts(unit, &starting_port, &number_of_port))
    {
        /* but module is not there
         */
        return FALSE;
    }

    /* Ok, module is there
     */
    if (u_port > board_info_p->max_port_number_on_board+number_of_port)
    {
        /* but out of range
         */
        return FALSE;
    }

    /* should be port on module, check port type by module type.
     */
    /* Charles hard code here and wait for STKTPLG_MODULE.C then need to modify.
     */
    if (ctrl_info_snapshot.synced_module_types[unit-1] == VAL_swModuleType_eightPortSfpModule)
    {
        *port_type = VAL_portType_thousandBaseSfp;
    }
    else if (ctrl_info_snapshot.synced_module_types[unit-1] == VAL_swModuleType_tenGigaPortModule)
    {
        /* 05/03/2004 10:08上午 vivid add to support 10G option module */
        *port_type = VAL_portType_tenG; /*10G port*/
    }
    else
    {
        printf("Invalid module type.\r\n");
        return FALSE;
    }
    return TRUE;

#else /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */
#if 0
   if (unit != 1)
    {
        STKTPLG_OM_CHECK_STATE();
    }
#endif

    if (unit != ctrl_info.my_unit_id)
    {
        if (!STKTPLG_OM_UnitExist(unit))
        {
            return FALSE;
        }
    }

    board_id = stk_unit_cfg[ctrl_info.my_unit_id - 1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        return FALSE;
    }

    *port_type = board_info_p->userPortMappingTable[unit-1][u_port-1].port_type;

    board_id = stk_unit_cfg[unit - 1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        return FALSE;
    }

    /* check main board.
     */
    if (u_port>=1 && u_port <= board_info_p->max_port_number_on_board)
    {
        /* yes, on main board.
         */
        return TRUE;
    }

    /* try to find module
     */
    if (FALSE == STKTPLG_OM_GetUnitModulePorts(unit, &starting_port, &number_of_port))
    {
        /* but module is not there
         */
        return FALSE;
    }

    /* Ok, module is there
     */
    if (u_port > board_info_p->max_port_number_on_board+number_of_port)
    {
        /* but out of range
         */
        return FALSE;
    }

    /* should be port on module, check port type by module type.
     */
    /* Charles hard code here and wait for STKTPLG_MODULE.C then need to modify.
     */
    if (ctrl_info.synced_module_types[unit-1] == VAL_swModuleType_eightPortSfpModule)
    {
        *port_type = VAL_portType_thousandBaseSfp;
    }
    else if (ctrl_info.synced_module_types[unit-1] == VAL_swModuleType_tenGigaPortModule)
    {
        /* 05/03/2004 10:08上午 vivid add to support 10G option module */
        *port_type = VAL_portType_tenG; /*10G port*/
    }
    else
    {
        printf("Invalid module type.\r\n");
        return FALSE;
    }
    return TRUE;
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */
} /* End of STKTPLG_OM_GetPortType() */

/* FUNCTION NAME : STKTPLG_OM_GetSfpPortType
 * PURPOSE: To get the type of fiber medium of the specified port of the specified unit.
 * INPUT:   unit_id      -- unit id.
 *          sfp_index
 * OUTPUT:  port_type_p  -- port type of the specified port of the specified unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   Superseded by SWDRV_LocalGetSfpPortType
 */
BOOL_T STKTPLG_OM_GetSfpPortType(UI32_T unit, UI32_T sfp_index, UI32_T *port_type_p)
{
#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    SWCTRL_OM_SfpInfo_T sfp_info;
#endif
    BOOL_T body_present = FALSE;
    UI8_T  bitrate = 0;

    /* not implemented for remote unit
     */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    if (unit != ctrl_info_snapshot.my_unit_id)
    {
        return FALSE;
    }
#else
    if (unit != stk_unit_cfg.my_unit_id)
    {
        return FALSE;
    }
#endif

    if (sfp_index < 1 || sfp_index > SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT)
    {
        return FALSE;
    }

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    if(FALSE == SWDRV_OM_GetPortSfpInfo(sfp_index, &sfp_info) ||
       FALSE == SWDRV_OM_GetPortSfpPresent(sfp_index, &body_present))
    {
        return FALSE;
    }

    bitrate = sfp_info.bitrate;
#endif

    /* TODO: shall move to sys_adpt.h
     */
#define SYS_ADPT_STKTPLG_SFP_MAX_SUPPORTED_BIT_RATE         160
#ifdef SYS_ADPT_STKTPLG_SFP_MAX_SUPPORTED_BIT_RATE
    /* If EEPROM contains wrong info,
     * it might cause SFP type detect incorrect.
     *
     * Workaround for it:
     * If bit rate is > SYS_ADPT_STKTPLG_SFP_MAX_SUPPORTED_BIT_RATE,
     * takes it as 0.
     */
    if (bitrate > SYS_ADPT_STKTPLG_SFP_MAX_SUPPORTED_BIT_RATE)
    {
        bitrate = 0;
    }
#endif

    {
        UI32_T port=0;
        UI32_T stktplg_board_port_type;

        /* The bit rate read from Finisar QSFP (FTL41QE1C) is 10500 MBits/s.
         * Thus the bit rate got from QSFP EEPROM field cannot be used to
         * identify its port type as VAL_portType_fortyGBaseQSFP.
         * As far as we know, QSFP port only support 40G port type. Thus if the
         * port type defined in STKTPLG_OM is VAL_portType_fortyGBaseQSFP, bitrate
         * will not be checked.
         *
         * This code snippet might need to be revised if new port type is
         * supported on QSFP port
         */
        if ( (STKTPLG_OM_SfpIndexToUserPort(unit, sfp_index, &port)==TRUE) &&
             (STKTPLG_OM_GetPortType(unit, port, &stktplg_board_port_type)==TRUE) )
        {
            if(stktplg_board_port_type == VAL_portType_fortyGBaseQSFP)
            {
                *port_type_p = VAL_portType_fortyGBaseQSFP;
                return TRUE;
            }
            else if(stktplg_board_port_type == VAL_portType_tenGBaseXFP)
            {
                *port_type_p = VAL_portType_tenGBaseXFP;
                return TRUE;
            }
        }

        /* to distinguish "no transceiver present" from "no bitrate value"
         */
        if (!body_present)
        {
            return STKTPLG_OM_GetPortType(unit, port, port_type_p);
        }

    #if (SYS_CPNT_STKTPLG_DETECT_SFP_TYPE == TRUE)
        if (bitrate >= STKTPLG_OM_BITRATE_10GSFP)
            *port_type_p = VAL_portType_tenGBaseSFP;
        else if (bitrate >= STKTPLG_OM_BITRATE_1000SFP)
            *port_type_p = VAL_portType_thousandBaseSfp;
        else if (bitrate >= STKTPLG_OM_BITRATE_100FX)
            *port_type_p = VAL_portType_hundredBaseFX;
        else
    #endif
        {
            UI32_T media_cap;

            /* For unidentifiable transceiver,
             * assume transceiver type is
             *     1) port type defined in board info for non combo port.
             *     2) SYS_DFLT_STKTPLG_MEDIA_TYPE_FOR_UNIDENTIFIABLE_SFP for combo port.
             */

            if (port!=0 &&
                STKTPLG_OM_GetPortMediaCapability(unit, port, &media_cap) &&
                !(media_cap & STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER))
            {
                return STKTPLG_OM_GetPortType(unit, port, port_type_p);
            }

            *port_type_p = SYS_DFLT_STKTPLG_MEDIA_TYPE_FOR_UNIDENTIFIABLE_SFP;
        }
    }

    return TRUE;
}

/* FUNCTION NAME : STKTPLG_OM_GetModuleID
 * PURPOSE: To get the module id of the specified port of the specified unit.
 * INPUT:   unit_id       -- unit id.
 *          u_port       -- phyical(user) port id
 * OUTPUT:  module_id    -- module id of the specified port of the specified unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   Here are two defined return module id
 *          SYS_HWCFG_MODULE_ID_10G_COPPER
 *          SYS_HWCFG_MODULE_ID_XFP
 *          SYS_HWCFG_MODULE_ID_10G_SFP
 *
 */
BOOL_T STKTPLG_OM_GetModuleID(UI32_T unit, UI32_T u_port, UI8_T *module_id)
{
#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE)
    UI8_T                     board_id;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    if (unit != ctrl_info_snapshot.my_unit_id)
    {
        return FALSE;
    }

    board_id = stk_unit_cfg_snapshot[unit-1].board_info.board_id;
#else
    if (unit != ctrl_info.my_unit_id)
    {
        return FALSE;
    }

    board_id = stk_unit_cfg[unit-1].board_info.board_id;
#endif

    return STKTPLG_BOARD_GetModuleID(board_id, u_port, module_id);

#endif /* (SYS_CPNT_10G_MODULE_SUPPORT == TRUE) */

    return FALSE;

} /* End of STKTPLG_OM_GetModuleID() */

/* FUNCTION NAME : STKTPLG_OM_GetMyRuntimeFirmwareVer
 * PURPOSE: Get the runtime firmware version of the local unit.
 * INPUT:   None
 * OUTPUT:  runtime_fw_ver_ar - Runtime firmware version will be put to this array.
 * RETUEN:  None
 * NOTES:
 *
 */
void STKTPLG_OM_GetMyRuntimeFirmwareVer(UI8_T runtime_fw_ver_ar[SYS_ADPT_FW_VER_STR_LEN+1])
{


    if(runtime_fw_ver_ar==0)
    {
        printf("\r\n%s:Invalid argument",__FUNCTION__);
        return;
    }

    SYSFUN_TakeReadSem(sem_id_static);
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    memcpy(runtime_fw_ver_ar, stk_unit_cfg_snapshot[ctrl_info.my_unit_id-1].board_info.runtime_sw_ver, SYS_ADPT_FW_VER_STR_LEN + 1);
    #else
    memcpy(runtime_fw_ver_ar, stk_unit_cfg[ctrl_info.my_unit_id-1].board_info.runtime_sw_ver, SYS_ADPT_FW_VER_STR_LEN + 1);
    #endif
    SYSFUN_GiveReadSem(sem_id_static);
}

/* FUNCTION NAME : STKTPLG_OM_GetMyUnitID
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetMyUnitID(UI32_T *my_unit_id)
{

    /* STKTPLG_OM_CHECK_STATE(); */

    /* setting this field is not protected by semaphore. it should be
     * all right. If not, move this field out of control information
     */
    SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
   *my_unit_id = ctrl_info_snapshot.my_unit_id;
#else
    *my_unit_id = ctrl_info.my_unit_id;
#endif
    SYSFUN_GiveReadSem(sem_id_static);
    return (TRUE);

} /* End of STKTPLG_OM_GetMyUnitID() */


/* FUNCTION NAME : STKTPLG_OM_GetLocalUnitBaseMac
 * PURPOSE: To get the base address of the local unit.
 * INPUT:   unit_id         -- unit id.
 * OUTPUT:  base_mac_addr   -- base mac address (6 bytes) of this unit.
 * RETUEN:  TRUE            -- successful.
 *          FALSE           -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetLocalUnitBaseMac(UI8_T *base_mac_addr)
{


    if(base_mac_addr==NULL)
        return FALSE;

    STKTPLG_OM_CHECK_STATE();

    /* setting this field is not protected by semaphore. it should be
     * all right. If not, move this field out of control information
     */
    SYSFUN_TakeReadSem(sem_id_static);
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
   memcpy(base_mac_addr, ctrl_info_snapshot.my_mac, STKTPLG_MAC_ADDR_LEN);
    #else
    memcpy(base_mac_addr, ctrl_info.my_mac, STKTPLG_MAC_ADDR_LEN);
    #endif
    SYSFUN_GiveReadSem(sem_id_static);

    return (TRUE);

} /* End of STKTPLG_OM_GetLocalUnitBaseMac() */


#if (SYS_CPNT_MGMT_PORT == TRUE)
/* FUNCTION NAME :
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETUEN:
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetLocalUnitBaseMac_ForMgmtPort(UI8_T *base_mac_addr)
{


    if(base_mac_addr==NULL)
        return FALSE;

    //STKTPLG_OM_CHECK_STATE();

    /* setting this field is not protected by semaphore. it should be
     * all right. If not, move this field out of control information
     */
     SYSFUN_TakeReadSem(sem_id_static);
     #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
   memcpy(base_mac_addr, ctrl_info_snapshot.my_mac, STKTPLG_MAC_ADDR_LEN);
    #else
    memcpy(base_mac_addr, ctrl_info.my_mac, STKTPLG_MAC_ADDR_LEN);
    #endif
    SYSFUN_GiveReadSem(sem_id_static);

    return (TRUE);

}
#endif


/* FUNCTION NAME : STKTPLG_OM_GetUnitBaseMac
 * PURPOSE: To get the base address of the local unit.
 * INPUT:   unit_id         -- unit id.
 * OUTPUT:  base_mac_addr   -- base mac address (6 bytes) of this unit.
 * RETUEN:  TRUE            -- successful.
 *          FALSE           -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetUnitBaseMac(UI8_T unit, UI8_T *base_mac_addr)
{
    UI32_T index;
    BOOL_T retval = FALSE;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
   if(base_mac_addr==NULL)
        return FALSE;

    STKTPLG_OM_CHECK_STATE();

    /* CLI uses the stackingDB (in the configure file) and stktplg information to build the remapping table.
     * If the device is standalone and boot-up without Unit 1, the remapping table in the CLI will be wrong.
     * It is due to the information about each unit are all my_mac. So, before provision completed, we should
     * get the stacking information from ctrl_info_snapshot.
     */
    if(TRUE== STKTPLG_OM_IsProvisionCompleted())
    {
        if (ctrl_info_snapshot.state == STKTPLG_STATE_STANDALONE)
        {
            SYSFUN_TakeReadSem(sem_id_static);
            memcpy(base_mac_addr, ctrl_info_snapshot.my_mac, STKTPLG_MAC_ADDR_LEN);
            SYSFUN_GiveReadSem(sem_id_static);
            return(TRUE);
        }
    }

    /* setting this field is not protected by semaphore. it should be
     * all right. If not, move this field out of control information
     */
//    memcpy(base_mac_addr, ctrl_info.stable_hbt.payload[unit-1].mac_addr, STKTPLG_MAC_ADDR_LEN);
    SYSFUN_TakeReadSem(sem_id_static);
    for (index = 0; index < ctrl_info_snapshot.total_units_up; index++)
    {
        if (ctrl_info_snapshot.stable_hbt_up.payload[index].unit_id == unit)
        {
            memcpy(base_mac_addr, ctrl_info_snapshot.stable_hbt_up.payload[index].mac_addr, STKTPLG_MAC_ADDR_LEN);
            retval = TRUE;
            break;
        }
    }

    if (FALSE == retval)
    {
        for (index = 0; index < ctrl_info_snapshot.total_units_down; index++)
        {
            if (ctrl_info_snapshot.stable_hbt_down.payload[index].unit_id == unit)
            {
                memcpy(base_mac_addr, ctrl_info_snapshot.stable_hbt_down.payload[index].mac_addr, STKTPLG_MAC_ADDR_LEN);
                retval = TRUE;
                break;
            }
        }
    }
    SYSFUN_GiveReadSem(sem_id_static);

    return (retval);
#else
    if(base_mac_addr==NULL)
        return FALSE;

    STKTPLG_OM_CHECK_STATE();

    if (ctrl_info.state == STKTPLG_STATE_STANDALONE)
    {
        SYSFUN_TakeReadSem(sem_id_static);
        memcpy(base_mac_addr, ctrl_info.my_mac, STKTPLG_MAC_ADDR_LEN);
        SYSFUN_GiveReadSem(sem_id_static);
        return(TRUE);
    }
    /* setting this field is not protected by semaphore. it should be
     * all right. If not, move this field out of control information
     */
//    memcpy(base_mac_addr, ctrl_info.stable_hbt.payload[unit-1].mac_addr, STKTPLG_MAC_ADDR_LEN);
    SYSFUN_TakeReadSem(sem_id_static);
    for (index = 0; index < ctrl_info.total_units_up; index++)
    {
        if (ctrl_info.stable_hbt_up.payload[index].unit_id == unit)
        {
            memcpy(base_mac_addr, ctrl_info.stable_hbt_up.payload[index].mac_addr, STKTPLG_MAC_ADDR_LEN);
            retval = TRUE;
            break;
        }
    }

    if (FALSE == retval)
    {
        for (index = 0; index < ctrl_info.total_units_down; index++)
        {
            if (ctrl_info.stable_hbt_down.payload[index].unit_id == unit)
            {
                memcpy(base_mac_addr, ctrl_info.stable_hbt_down.payload[index].mac_addr, STKTPLG_MAC_ADDR_LEN);
                retval = TRUE;
                break;
            }
        }
    }
    SYSFUN_GiveReadSem(sem_id_static);

    return (retval);
#endif

} /* End of STKTPLG_OM_GetUnitBaseMac() */

/* FUNCTION NAME: STKTPLG_OM_GetUnitBoardID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- unit number, SYS_VAL_LOCAL_UNIT_ID for local unit.
 * OUTPUT:  board_id -- This is board ID, and used to be family serial number,
 *                      that is 5-bit field in project ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 */
BOOL_T STKTPLG_OM_GetUnitBoardID(UI32_T unit, UI32_T *board_id)
{
    UI32_T local_use_unit;

    if (SYS_VAL_LOCAL_UNIT_ID == unit)
    {
        local_use_unit=1;
    }
    else
    {
        if (!STKTPLG_OM_UnitExist(unit))
        {
            return FALSE;
        }
        local_use_unit=unit;
    }
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    *board_id = stk_unit_cfg_snapshot[local_use_unit-1].board_info.board_id;
#else
    *board_id = stk_unit_cfg[local_use_unit-1].board_info.board_id;
#endif
    return TRUE;
}

/* FUNCTION NAME : STKTPLG_OM_PortExist
 * PURPOSE: This function is used to check if the specified port is
 *          existing or not.
 * INPUT:   logical_unit_id -- Logical unit ID
 *          logical_port_id -- logical port ID
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_PortExist(UI32_T unit_id, UI32_T port_id)
{
    UI8_T                     board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    UI32_T                    slot_index;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    if(!STKTPLG_OM_UnitExist(unit_id))
        return FALSE;

    STKTPLG_OM_CHECK_STATE();

    if ((port_id < 1) || (port_id > stk_unit_cfg_snapshot[(UI8_T)unit_id - 1].nport))
    {
       // if (port_id<49)
       //printf("[%s]%d unit [%d] port %d not exist\n",__FUNCTION__,__LINE__,unit_id,port_id);
        return (FALSE);
    }

    board_id = stk_unit_cfg_snapshot[(UI8_T)unit_id - 1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        printf("board_id not success\n");

        printf("[%s]%d unit [%lu] port %lu \n",__FUNCTION__,__LINE__,unit_id,port_id); /* JinhuaWei, 03 August, 2008 11:36:00 ,because of too many arguments*/

        return (FALSE);
    }

    if (port_id <= board_info_p->max_port_number_on_board)
    {
        if (stackPortMappingTable[unit_id-1][port_id-1].device_id == 0xff ||
            stackPortMappingTable[unit_id-1][port_id-1].device_port_id == 0xff)
            return FALSE;

        return (TRUE);
    }
    else
    {
        // printf("port %d not exist max=%d\n",port_id,board_info_p->max_port_number_on_board);
        slot_index = port_id - board_info_p->start_expansion_port_number;

        /*
        if ( (0 == stk_unit_cfg[(UI8_T)unit_id - 1].exp_module_presented[slot_index]) ||
             (stk_unit_cfg[(UI8_T)unit_id - 1].exp_module_type[slot_index] & SYS_HWCFG_MODULE_ID_MASK) == SYS_HWCFG_MODULE_ID_STACKING_MODULE)
        */
        if (0 == stk_unit_cfg_snapshot[(UI8_T)unit_id - 1].exp_module_presented[slot_index])
        {
            //printf("[%s]%d unit [%lu] port %lu not exist\n",__FUNCTION__,__LINE__,unit_id,port_id);
            //printf("module port not exist unit_id=%d port_id=%d slot_index=%d \n",unit_id,port_id,slot_index);
            return (FALSE);
        }

    }
#else
    if(!STKTPLG_OM_UnitExist(unit_id))
        return FALSE;

    STKTPLG_OM_CHECK_STATE();

    if ((port_id < 1) || (port_id > stk_unit_cfg[(UI8_T)unit_id - 1].nport))
    {
       // if (port_id<49)
       //     printf("unit [%d] port %d not exist\n",unit_id,port_id);
        return (FALSE);
    }

    board_id = stk_unit_cfg[(UI8_T)unit_id - 1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        printf("board_id not success\n");
        return (FALSE);
    }

    if (port_id <= board_info_p->max_port_number_on_board)
    {
        return (TRUE);
    }
    else
    {
        // printf("port %d not exist max=%d\n",port_id,board_info_p->max_port_number_on_board);
        slot_index = port_id - board_info_p->start_expansion_port_number;

        /*
        if ( (0 == stk_unit_cfg[(UI8_T)unit_id - 1].exp_module_presented[slot_index]) ||
             (stk_unit_cfg[(UI8_T)unit_id - 1].exp_module_type[slot_index] & SYS_HWCFG_MODULE_ID_MASK) == SYS_HWCFG_MODULE_ID_STACKING_MODULE)
        */
        if (0 == stk_unit_cfg[(UI8_T)unit_id - 1].exp_module_presented[slot_index])
        {
            //printf("module port not exist unit_id=%d port_id=%d slot_index=%d \n",unit_id,port_id,slot_index);
            return (FALSE);
        }

    }
#endif
    return (TRUE);

} /* End of STKTPLG_OM_PortExist() */

/* FUNCTION NAME : STKTPLG_OM_SetUnitRuntimeFirmwareVer
 * PURPOSE: Set the runtime firmware version of the specified unit
 * INPUT:   unit              - id of the unit of the runtime firmware version
 *                              to be set into
 *          runtime_fw_ver_ar - Runtime firmware version to be set
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *
 */
void STKTPLG_OM_SetUnitRuntimeFirmwareVer(UI32_T unit, UI8_T runtime_firmware_ver_ar[SYS_ADPT_FW_VER_STR_LEN+1])
{

    if(runtime_firmware_ver_ar==0)
        return;

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return;  /*changed by Jinhua Wei,because 'return' with a value, in function returning void*/

    SYSFUN_TakeWriteSem(sem_id_static);
    memcpy(stk_unit_cfg[unit-1].board_info.runtime_sw_ver, runtime_firmware_ver_ar, SYS_ADPT_FW_VER_STR_LEN+1);
    SYSFUN_GiveWriteSem(sem_id_static);
}

/* FUNCTION NAME: STKTPLG_OM_GetBoardModuleTypeReg
 * PURPOSE: This function is used to get module type from board register.
 * INPUT:   module_index    -- module index.
 *          *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetBoardModuleTypeReg(UI8_T module_index, UI8_T *module_type)
{
    BOOL_T ret=FALSE;

    if(module_type==NULL)
    {
        SYSFUN_Debug_Printf("\r\n%s: Invalid argument", __FUNCTION__);
        return ret;
    }

    switch(module_index)
    {
        case STKTPLG_MODULE_INDEX_0:
            ret=PHYADDR_ACCESS_Read(sys_hwcfg_module_id_type_addr[0], 1, 1, module_type);
            break;
        case STKTPLG_MODULE_INDEX_1:
            ret=PHYADDR_ACCESS_Read(sys_hwcfg_module_id_type_addr[1], 1, 1, module_type);
            break;
    }

    return ret;
} /* End of STKTPLG_OM_GetModuleTypeBoardReg */


/* FUNCTION NAME: STKTPLG_OM_GetModuleAType
 * PURPOSE: This function is used to get module 1 type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetModuleAType(UI8_T *module_type)
{
    if(module_type==NULL)
    {
        SYSFUN_Debug_Printf("\r\n%s:Invalid argument", __FUNCTION__);
        return FALSE;
    }

    return PHYADDR_ACCESS_Read(sys_hwcfg_module_id_type_addr[0], 1, 1, module_type);
} /* End of STKTPLG_OM_GetModuleAType */


/* FUNCTION NAME: STKTPLG_OM_GetModuleBType
 * PURPOSE: This function is used to get module 2 type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetModuleBType(UI8_T *module_type)
{
    if(module_type==NULL)
    {
        SYSFUN_Debug_Printf("\r\n%s:Invalid argument", __FUNCTION__);
        return FALSE;
    }

    return PHYADDR_ACCESS_Read(sys_hwcfg_module_id_type_addr[1], 1, 1, module_type);

} /* End of STKTPLG_OM_GetModuleBType */

/* -------------------------------------------------------------------------
 * ROUTINE NAME -  STKTPLG_OM_GetStackingInfo
 * -------------------------------------------------------------------------
 * FUNCTION: get stktplg stacking information
 * INPUT   : NONE
 * OUTPUT  : NONE
 * RETURN  : Stacking_Info_T *stack_info
 * NOTE    :
 * -------------------------------------------------------------------------*/
Stacking_Info_T   *STKTPLG_OM_GetStackingInfo(void)
{
    return(&stktplg_stacking_info);
}

/* FUNCTION NAME: STKTPLG_OM_GetOldModuleType
 * PURPOSE: This function is used to get old module type.
 * INPUT:   module_index    -- module index.
 *          *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetOldModuleType(UI8_T module_index, UI8_T *module_type)
{
    *module_type = old_module_type[module_index];

    return TRUE;

} /* End of STKTPLG_OM_GetOldModuleType */


/* FUNCTION NAME: STKTPLG_OM_GetModuleAOldType
 * PURPOSE: This function is used to get module 1 old type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetModuleAOldType(UI8_T *module_type)
{
    *module_type = old_module_type[0];

    return TRUE;
} /* End of STKTPLG_OM_GetModuleAOldType */


/* FUNCTION NAME: STKTPLG_OM_GetModuleBOldType
 * PURPOSE: This function is used to get module 2 old type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetModuleBOldType(UI8_T *module_type)
{
/* This function is obsoleted. Comment out old_module_type[1] to avoid compile
 * warning array subscript is above array bounds
    *module_type = old_module_type[1];
 */
    return TRUE;
} /* End of STKTPLG_OM_GetModuleBOldType */

/* FUNCTION NAME: STKTPLG_OM_GetSwServiceTag
 * PURPOSE: This function is used to get this the sw service tag
 * INPUT:   unit_id            -- unit id
 * OUTPUT:  sw_service_tag     -- sw service tag
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetSwServiceTag(UI32_T unit, UI8_T sw_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1])
{
    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;

    if (sw_service_tag == NULL)
        return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    memcpy(sw_service_tag, stk_unit_cfg_snapshot[unit-1].sw_service_tag, SYS_ADPT_SERIAL_NO_STR_LEN + 1);
#else
    memcpy(sw_service_tag, stk_unit_cfg[unit-1].sw_service_tag, SYS_ADPT_SERIAL_NO_STR_LEN + 1);
#endif
    SYSFUN_GiveReadSem(sem_id_static);

    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_GetSwChassisServiceTag
 * PURPOSE: This function is used to get this the sw chassis service tag
 * INPUT:   unit_id            -- unit id
 * OUTPUT:  sw_service_tag     -- sw chassis service tag
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetSwChassisServiceTag(UI32_T unit, UI8_T sw_chassis_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1])
{
    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;

    if (sw_chassis_service_tag == NULL)
        return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    memcpy(sw_chassis_service_tag, stk_unit_cfg_snapshot[unit-1].sw_chassis_service_tag, SYS_ADPT_SERIAL_NO_STR_LEN + 1);
#else
    memcpy(sw_chassis_service_tag, stk_unit_cfg[unit-1].sw_chassis_service_tag, SYS_ADPT_SERIAL_NO_STR_LEN + 1);
#endif
    SYSFUN_GiveReadSem(sem_id_static);

    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_GetSwIdentifier
 * PURPOSE: This function is used to get this the sw identifier
 * INPUT:   unit_id             -- unit id
 * OUTPUT:  sw_identifier_p     -- identifier
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetSwIdentifier(UI32_T unit, UI32_T *sw_identifier_p)
{
    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;

    if (sw_identifier_p == NULL)
        return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    *sw_identifier_p = stk_unit_cfg_snapshot[unit-1].sw_identifier;
#else
    *sw_identifier_p = stk_unit_cfg[unit-1].sw_identifier;
#endif
    SYSFUN_GiveReadSem(sem_id_static);

    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_GetDeviceModulePresented
 * PURPOSE: This function is used to get this unit's module present
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  module_type     -- module presented
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetDeviceModulePresented(UI32_T unit, UI8_T module_presented[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT])
{
    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;

    if (module_presented == NULL)
        return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    memcpy(module_presented, stk_unit_cfg_snapshot[unit-1].module_presented, SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT);
#else
    memcpy(module_presented, stk_unit_cfg[unit-1].module_presented, SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT);
#endif
    SYSFUN_GiveReadSem(sem_id_static);

    return TRUE;
}


/* FUNCTION NAME: STKTPLG_OM_GetDeviceModuleType
 * PURPOSE: This function is used to get this unit's module type
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  module_type     -- module type
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetDeviceModuleType(UI32_T unit, UI8_T module_type[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT])
{
    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;

    if (module_type == NULL)
        return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    memcpy(module_type, stk_unit_cfg_snapshot[unit-1].module_type, SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT);
#else
    memcpy(module_type, stk_unit_cfg[unit-1].module_type, SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT);
#endif
    SYSFUN_GiveReadSem(sem_id_static);

    return TRUE;
}


/* FUNCTION NAME: STKTPLG_OM_GetModuleExpRuntimeFwVer
 * PURPOSE: This function is used to get module expected runtime firmware version
 * INPUT:   unit_id     -- unit id
 * OUTPUT:  fw_ver      -- firmware version string
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetModuleExpRuntimeFwVer(UI32_T unit, UI8_T fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1])
{
    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;

    if (fw_ver == NULL)
        return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    memcpy(fw_ver, stk_unit_cfg_snapshot[unit-1].module_expected_runtime_fw_ver, SYS_ADPT_FW_VER_STR_LEN + 1);
#else
    memcpy(fw_ver, stk_unit_cfg[unit-1].module_expected_runtime_fw_ver, SYS_ADPT_FW_VER_STR_LEN + 1);
#endif
    SYSFUN_GiveReadSem(sem_id_static);

    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_GetSysInfo
 * PURPOSE: This function is used to get this system information.
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  *sys_info       -- sys info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This funxtion is used by sysmgmt to get system information.
 *
 */
BOOL_T STKTPLG_OM_GetSysInfo(UI32_T unit, STKTPLG_OM_Info_T *sys_info)
{
    if(!STKTPLG_OM_UnitExist(unit))
        return FALSE;

    if (sys_info == NULL)
        return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);
   #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
   memcpy(sys_info, &stk_unit_cfg_snapshot[unit - 1].board_info, sizeof(STKTPLG_OM_Info_T));
   #else
    memcpy(sys_info, &stk_unit_cfg[unit - 1].board_info, sizeof(STKTPLG_OM_Info_T));
   #endif
    SYSFUN_GiveReadSem(sem_id_static);

    return TRUE;
} /* End of STKTPLG_OM_GetSysInfo */


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME: STKTPLG_OM_GetDeviceInfo
 * PURPOSE: This function is used to get this device information.
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  *device_info    -- device info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES: 1. Don't call this function by passing device_info as a local declared
 *           varible, because the size of STK_UNIT_CFG_T is huge and it may
 *           cause stack overflow. suggested to use dynamic allocated memory.
 */
BOOL_T STKTPLG_OM_GetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info)
{
    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);
    memcpy(device_info, &stk_unit_cfg_snapshot[unit - 1], sizeof(STK_UNIT_CFG_T));
    SYSFUN_GiveReadSem(sem_id_static);

    return TRUE;

} /* End of STKTPLG_OM_GetDeviceInfo */

/* FUNCTION NAME: STKTPLG_OM_SetDeviceInfo
 * PURPOSE: This function is used to set this device information.
 * INPUT:   unit_id         -- unit id
 *          *device_info    -- device info.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES: 1. Don't call this function by passing device_info as a local declared
 *           varible, because the size of STK_UNIT_CFG_T is huge and it may
 *           cause stack overflow. suggested to use dynamic allocated memory.
 */
BOOL_T STKTPLG_OM_SetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info)
{
    UI8_T board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;


    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;

    board_id = stk_unit_cfg_snapshot[(UI8_T)unit-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        SYSFUN_Debug_Printf("unit=%ld board_id=%d\r\n",unit,board_id);
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    SYSFUN_TakeWriteSem(sem_id_static);
    memcpy(&stk_unit_cfg_snapshot[unit - 1], device_info, sizeof(STK_UNIT_CFG_T));
    SYSFUN_GiveWriteSem(sem_id_static);

    return TRUE;

} /* End of STKTPLG_OM_SetDeviceInfo */
#else
/* FUNCTION NAME: STKTPLG_OM_GetDeviceInfo
 * PURPOSE: This function is used to get this device information.
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  *device_info    -- device info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES: 1. Don't call this function by passing device_info as a local declared
 *           varible, because the size of STK_UNIT_CFG_T is huge and it may
 *           cause stack overflow. suggested to use dynamic allocated memory.
 */
BOOL_T STKTPLG_OM_GetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info)
{
    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);
    memcpy(device_info, &stk_unit_cfg[unit - 1], sizeof(STK_UNIT_CFG_T));
    SYSFUN_GiveReadSem(sem_id_static);

    return TRUE;

} /* End of STKTPLG_OM_GetDeviceInfo */


/* FUNCTION NAME: STKTPLG_OM_SetDeviceInfo
 * PURPOSE: This function is used to set this device information.
 * INPUT:   unit_id         -- unit id
 *          *device_info    -- device info.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES: 1. Don't call this function by passing device_info as a local declared
 *           varible, because the size of STK_UNIT_CFG_T is huge and it may
 *           cause stack overflow. suggested to use dynamic allocated memory
 */
BOOL_T STKTPLG_OM_SetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info)
{
    UI8_T board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;


    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

    board_id = stk_unit_cfg[(UI8_T)unit-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        SYSFUN_Debug_Printf("unit=%ld board_id=%d\r\n",unit,board_id);
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    SYSFUN_TakeWriteSem(sem_id_static);
    memcpy(&stk_unit_cfg[unit - 1], device_info, sizeof(STK_UNIT_CFG_T));
    SYSFUN_GiveWriteSem(sem_id_static);

    return TRUE;

} /* End of STKTPLG_OM_SetDeviceInfo */
#endif

/* FUNCTION NAME: STKTPLG_OM_ResetDeviceInfo
 * PURPOSE: This function is used to set this device information.
 * INPUT:   unit_id         -- unit id
 *          *device_info    -- device info.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_ResetDeviceInfo(void)
{


    SYSFUN_TakeWriteSem(sem_id_static);
    memset(&stk_unit_cfg[ctrl_info.my_unit_id-1], 0, sizeof(STK_UNIT_CFG_T)*(SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK-1));
    SYSFUN_GiveWriteSem(sem_id_static);

    return TRUE;

} /* End of STKTPLG_OM_ResetDeviceInfo */

/* FUNCTION NAME: STKTPLG_OM_GetModuleInfo
 * PURPOSE: This function is used to get this module's information.
 * INPUT:   unit         -- main board unit id
 *          module_index -- module index
 *          *module_info -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetModuleInfo(UI32_T unit, UI32_T module_index, STKTPLG_OM_switchModuleInfoEntry_T *module_info)
{


    if(FALSE==STKTPLG_OM_UnitExist(unit))
    {
        return FALSE;
    }

    /* Modified by Vincent on 17,Dec,2004
     * For a slave unit ctrl_info.synced_module_types won't be upgraded
     * i.e, we don't need to check this variable
     */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
 if (module_index > STKTPLG_OM_MAX_MODULE_NBR || STKTPLG_OM_ExpModuleIsInsert(unit)==FALSE
        || (ctrl_info_snapshot.state == STKTPLG_STATE_MASTER && ctrl_info_snapshot.synced_module_types[unit-1] == 0 )) /*not present*/
    {
        return FALSE;
    }
 #else
    if (module_index > STKTPLG_OM_MAX_MODULE_NBR || STKTPLG_OM_ExpModuleIsInsert(unit)==FALSE
        || (ctrl_info.state == STKTPLG_STATE_MASTER && ctrl_info.synced_module_types[unit-1] == 0 )) /*not present*/
    {
        return FALSE;
    }
 #endif
    SYSFUN_TakeReadSem(sem_id_static);
    memcpy(module_info, &stk_module_cfg[unit - 1][module_index-1], sizeof(STKTPLG_OM_switchModuleInfoEntry_T));
    SYSFUN_GiveReadSem(sem_id_static);

    module_info->unit_index = unit;
    module_info->module_index = module_index;

/*    memcpy(module_info->op_code_ver, stk_unit_cfg[unit - 1].module_runtime_fw_ver, SYS_ADPT_FW_VER_STR_LEN + 1); */

    return TRUE;

} /* End of STKTPLG_OM_GetModuleInfo*/

/* FUNCTION NAME: STKTPLG_OM_SetModuleInfo
 * PURPOSE: This function is used to set this module's information.
 * INPUT:   unit         -- main board unit id
 *          module_index -- module index
 *          *module_info -- module info.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_SetModuleInfo(UI32_T unit, UI32_T module_index, STKTPLG_OM_switchModuleInfoEntry_T *module_info)
{


    if(!STKTPLG_OM_UnitExist(unit))
    {
        return FALSE;
    }

    SYSFUN_TakeWriteSem(sem_id_static);
    memcpy(&stk_module_cfg[unit - 1][module_index -1], module_info, sizeof(STKTPLG_OM_switchModuleInfoEntry_T));
    SYSFUN_GiveWriteSem(sem_id_static);


    return TRUE;

} /* End of STKTPLG_OM_SetModuleInfo */

/* FUNCTION NAME: STKTPLG_OM_RemoveModule
 * PURPOSE: This function is used to remove slot-in module.
 * INPUT:   unit            -- unit id.
 *          module_index    -- module index.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_RemoveModule(UI32_T unit, UI32_T module_index)
{


    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

    SYSFUN_TakeWriteSem(sem_id_static);
    old_module_type[module_index] = stk_unit_cfg[unit-1].module_type[module_index];

    stk_unit_cfg[unit-1].module_presented[module_index] = 0;
    stk_unit_cfg[unit-1].module_type[module_index] = 0xff; /* When no module_a slot-in */
    stk_unit_cfg[unit-1].nport--;
    SYSFUN_GiveWriteSem(sem_id_static);

    STKTPLG_OM_SetTplgSyncBmp();
    return TRUE;

} /* End of STKTPLG_OM_RemoveModule */


/* FUNCTION NAME: STKTPLG_OM_InsertModule
 * PURPOSE: This function is used to insert slot-in module.
 * INPUT:   unit            -- unit id.
 *          module_index    -- module index.
 *          module_type     -- which module type be inserted.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_InsertModule(UI32_T unit, UI32_T module_index, UI32_T module_type)
{


    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;
    SYSFUN_TakeWriteSem(sem_id_static);
    stk_unit_cfg[unit-1].module_type[module_index] = module_type;
    stk_unit_cfg[unit-1].module_presented[module_index] = 1;
    stk_unit_cfg[unit-1].nport++;
    SYSFUN_GiveWriteSem(sem_id_static);

    return TRUE;

}/* End of STKTPLG_OM_RemoveModule */


/* FUNCTION NAME: STKTPLG_OM_AddOnePortNumber
 * PURPOSE: This function is used to add one port number in this unit.
 * INPUT:   device_index    -- device index.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_AddOnePortNumber(UI32_T device_index)
{


    SYSFUN_TakeWriteSem(sem_id_static);
    stk_unit_cfg[device_index-1].nport++;
    SYSFUN_GiveWriteSem(sem_id_static);

    return TRUE;

}/* End of STKTPLG_OM_AddOnePortNumber */

/* FUNCTION NAME: STKTPLG_OM_SlaveReady
 * PURPOSE: This utiity function is to keep information about is unit is entering
 *          slave mode completely.
 * INPUT:   ready -- ready or not.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_OM_SlaveReady(BOOL_T ready)
{
    slave_ready = ready;

    return;
}

/* FUNCTION NAME: STKTPLG_OM_SlaveIsReady
 * PURPOSE: This utiity function is to report is slave mode is initialized completely
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE: Yes.
 *          FALSE: No.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_SlaveIsReady(void)
{
    return (slave_ready);
}

/* FUNCTION NAME: STKTPLG_OM_GetSwitchOperState
 * PURPOSE: This function is used to get the whole system oper state.
 * INPUT:   *switch_oper_state -- buffer of oper state.
 * OUTPUT:  *switch_oper_state -- oper state.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   VAL_switchOperState_other           1
 *          VAL_switchOperState_unknown         2
 *          VAL_switchOperState_ok              3
 *          VAL_switchOperState_noncritical     4
 *          VAL_switchOperState_critical        5
 *          VAL_switchOperState_nonrecoverable  6
 */
BOOL_T STKTPLG_OM_GetSwitchOperState(UI8_T *switch_oper_state)
{
    /* Check invalid value */
    if ((switch_op_state < VAL_switchOperState_other) ||
        (switch_op_state > VAL_switchOperState_nonrecoverable))
        return FALSE;

    *switch_oper_state = switch_op_state;
    return TRUE;
} /* end of STKTPLG_OM_GetSwitchOperState */


/* FUNCTION NAME: STKTPLG_OM_GetMasterUnitId
 * PURPOSE: This routine is used to get master unit id.
 * INPUT:   *master_unit_id  -- master unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- successful
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetMasterUnitId(UI8_T *master_unit_id)
{
    STKTPLG_OM_CHECK_STATE();

    /* setting this field is not protected by semaphore. it should be
     * all right. If not, move this field out of control information
     */
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    *master_unit_id = ctrl_info_snapshot.master_unit_id;
    #else
    *master_unit_id = ctrl_info.master_unit_id;
    #endif

    return (TRUE);
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME: STKTPLG_OM_GetCtrlInfo
 * PURPOSE: This routine is used to get control information of stack topology
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  pointer to control information of stack topology.
 * NOTES:
 *
 */
STKTPLG_OM_Ctrl_Info_T *STKTPLG_OM_GetCtrlInfo(void)
{
    return (&ctrl_info_snapshot);
}
#else
STKTPLG_OM_Ctrl_Info_T *STKTPLG_OM_GetCtrlInfo(void)
{
  return (&ctrl_info);
}
#endif

/* FUNCTION NAME: STKTPLG_OM_GetStackingPort
 * PURPOSE: This function is used to get stacking port for a specified unit.
 * INPUT:   src_unit -- unit number
 * OUTPUT:  stack_port -- stacking port
 * RETUEN:  TRUE :  successful
 *          FALSE : failed
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetSimplexStackingPort(UI32_T src_unit, UI32_T *stack_port)
{
    UI8_T                     board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    if(src_unit <1 || src_unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    board_id = stk_unit_cfg_snapshot[src_unit - 1].board_info.board_id;
#else
    board_id = stk_unit_cfg [src_unit - 1].board_info.board_id;
#endif
    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        return (FALSE);
    }

    if (!board_info_p->simplex_stacking)
    {
        return (FALSE);
    }

    *stack_port = board_info_p->stacking_uplink_port;

    return (TRUE);
}

/* FUNCTION NAME: STKTPLG_OM_GetUnitBootReason
 * PURPOSE: This function is used to get boot reason of some unit.
 * INPUT:   unit -- unit.
 * OUTPUT:  *boot_reason -- boot reason.
 * RETUEN:  TRUE  : successful
 *          FALSE : failed
 * NOTES:   STKTPLG_OM_REBOOT_FROM_COLDSTART
 *          STKTPLG_OM_REBOOT_FROM_WARMSTART
 */
BOOL_T STKTPLG_OM_GetUnitBootReason(UI32_T unit, UI32_T *boot_reason)
{
    if (unit != 1)
    {
        STKTPLG_OM_CHECK_STATE();
    }

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    *boot_reason = stk_unit_cfg_snapshot[unit-1].boot_reason;
#else
    *boot_reason = stk_unit_cfg[unit-1].boot_reason;
#endif
    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_SetUnitBootReason
 * PURPOSE: This function is used to set boot reason of some unit.
 * INPUT:   unit -- unit.
 * OUTPUT:  boot_reason -- boot reason.
 * OUTPUT:  None
 * RETUEN:  TRUE  : successful
 *          FALSE : failed
 * NOTES:   STKTPLG_OM_REBOOT_FROM_COLDSTART
 *          STKTPLG_OM_REBOOT_FROM_WARMSTART
 */
BOOL_T STKTPLG_OM_SetUnitBootReason(UI32_T unit, UI32_T boot_reason)
{
    if (unit != 1)
    {
        STKTPLG_OM_CHECK_STATE();
    }

    if(unit < 1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    stk_unit_cfg_snapshot[unit-1].boot_reason = boot_reason;
#else
    stk_unit_cfg[unit-1].boot_reason = boot_reason;
#endif
    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_UnitExist
 * PURPOSE: This function is used to check if the specified unit is
 *          existing or not.
 * INPUT:   unit_id  -- unit id
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- exist
 *          FALSE  -- not exist
 * NOTES:   Use got mac address of each unit to
 *          know if unit exist or not.
 *
 */
BOOL_T STKTPLG_OM_UnitExist(UI32_T unit_id)
{
    UI32_T unit;
    BOOL_T ret;

    ret = FALSE;
    unit = 0;
    while(STKTPLG_OM_GetNextUnit(&unit))
    {
        if (unit == unit_id)
        {
            ret = TRUE;
            break;
        }
    }

    return ret;
}



#if (SYS_CPNT_MAU_MIB == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetJackType
 * -------------------------------------------------------------------------
 * FUNCTION: Get jack type of some user port.
 * INPUT   : unit       -- Which unit.
 *           port       -- Which port.
 *           mau_index  -- Which MAU.
 *           jack_index -- Which jack.
 * OUTPUT  : jack_type  -- VAL_ifJackType_other
 *                         VAL_ifJackType_rj45
 *                         VAL_ifJackType_rj45S
 *                         VAL_ifJackType_db9
 *                         VAL_ifJackType_bnc
 *                         VAL_ifJackType_fAUI
 *                         VAL_ifJackType_mAUI
 *                         VAL_ifJackType_fiberSC
 *                         VAL_ifJackType_fiberMIC
 *                         VAL_ifJackType_fiberST
 *                         VAL_ifJackType_telco
 *                         VAL_ifJackType_mtrj
 *                         VAL_ifJackType_hssdc
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetJackType (UI32_T unit,
                                UI32_T port,
                                UI32_T mau_index,
                                UI32_T jack_index,
                                UI32_T *jack_type)
{
#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    SWCTRL_OM_SfpInfo_T sfp_info;
    UI32_T sfp_index;
#endif
    UI8_T  board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    BOOL_T is_present;

    /* Non-Local unit
     */
    if (unit != 1)
    {
        if (!STKTPLG_OM_UnitExist(unit))
            return FALSE;
    }

    if (unit != ctrl_info.my_unit_id)
    {
        STKTPLG_OM_CHECK_STATE();
    }

    if ((mau_index > SYS_ADPT_MAX_NBR_OF_MAU_PER_USER_PORT) ||
         (jack_index > SYS_ADPT_MAX_NBR_OF_JACK_PER_MAU))
    {
        return (FALSE);
    }

    board_id = stk_unit_cfg[unit - 1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        return (FALSE);
    }

    if ((port < 1) || (port > board_info_p->max_port_number_of_this_unit))
    {
        return (FALSE);
    }

    if (mau_index > board_info_p->portMediaNum_ptr[port-1])
    {
        return (FALSE);
    }

    if (port >= board_info_p->start_sfp_port_number)
    {
        switch(mau_index)
        {
            case 1:
                *jack_type = board_info_p->portJackType_ptr[port-1];
                break;

            case 2:
            #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
                //mod_id = board_info_p->port2ModId_ptr[port - board_info_p->start_sfp_port_number];
                sfp_index = board_info_p->port2ModId_ptr[port - board_info_p->start_sfp_port_number]+1;

                //if (stk_unit_cfg[unit-1].gbic_info[mod_id].body_present)
                if (TRUE == SWCTRL_POM_GetPortSfpPresent(unit, sfp_index, &is_present) &&
                    TRUE == is_present &&
                    TRUE == SWCTRL_POM_GetPortSfpInfo(unit, sfp_index, &sfp_info))
                {
                    switch(sfp_info.connector)
                    {
                        case GBIC_CONNECTOR_VLAUE_UNKNOWN:
                            *jack_type = VAL_ifJackType_other;
                            break;

                        case GBIC_CONNECTOR_VLAUE_SC:
                            *jack_type = VAL_ifJackType_fiberSC;
                            break;

                        case GBIC_CONNECTOR_VLAUE_FIBRE_STYLE_1:
                        case GBIC_CONNECTOR_VLAUE_FIBRE_STYLE_2:
                            *jack_type = VAL_ifJackType_other;
                            break;

                        case GBIC_CONNECTOR_VLAUE_BNT_TNC:
                            *jack_type = VAL_ifJackType_bnc;
                            break;

                        case GBIC_CONNECTOR_VLAUE_FIBRE_COAXIAL:
                        case GBIC_CONNECTOR_VLAUE_FIBRE_JACK:
                            *jack_type = VAL_ifJackType_other;
                            break;

                        case GBIC_CONNECTOR_VLAUE_LC:
                            *jack_type = VAL_ifJackType_fiberLC;
                            break;

                        case GBIC_CONNECTOR_VLAUE_MTRJ:
                            *jack_type = VAL_ifJackType_mtrj;
                            break;

                        case GBIC_CONNECTOR_VLAUE_MU:
                        case GBIC_CONNECTOR_VLAUE_SG:
                        case GBIC_CONNECTOR_VLAUE_OPTICAL_PIGTAIL:
                        case GBIC_CONNECTOR_VLAUE_RESERVED1_FIRST:
                        case GBIC_CONNECTOR_VLAUE_RESERVED1_LAST:
                            *jack_type = VAL_ifJackType_other;
                            break;

                        case GBIC_CONNECTOR_VLAUE_HSSDC_2:
                            *jack_type = VAL_ifJackType_hssdc;
                            break;

                        case GBIC_CONNECTOR_VLAUE_COPPER_PIGTAIL:
                        case GBIC_CONNECTOR_VLAUE_RESERVED2_FIRST:
                        case GBIC_CONNECTOR_VLAUE_RESERVED2_LAST:
                        case GBIC_CONNECTOR_VLAUE_CONNECTOR_NAME_IN_BYTES_128_143:
                        case GBIC_CONNECTOR_VLAUE_VENDOR_SPECIFIC_FIRST:
                        case GBIC_CONNECTOR_VLAUE_VENDOR_SPECIFIC_LAST:
                            *jack_type = VAL_ifJackType_other;
                            break;

                        default:
                            *jack_type = VAL_ifJackType_other;
                    }
                }
                else
                {
                    return (FALSE);
                }
            #endif /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */
                break;

            default:
                perror("\r\nmau_index out of range");
                assert(0);
        }
    }
    else
    {
        *jack_type = board_info_p->portJackType_ptr[port-1];
    }

    return (TRUE);
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetMauMediaType
 * -------------------------------------------------------------------------
 * FUNCTION: Get media type of some MAU.
 * INPUT   : unit       -- Which unit.
 *           port       -- Which port.
 *           mau_index  -- Which MAU.
 * OUTPUT  : media_type -- STKTPLG_MGR_MEDIA_TYPE_E.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetMauMediaType (UI32_T unit, UI32_T port, UI32_T mau_index, UI32_T *media_type)
{
    UI8_T                       board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    UI32_T    dummy_unit;
    UI32_T    module_type;
    STKTPLG_OM_switchModuleInfoEntry_T module_info;
    UI32_T    dummy_module_num=1;

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    SWCTRL_OM_SfpInfo_T sfp_info;
    UI32_T sfp_index;
    BOOL_T is_present;
#endif

    /* Non-Local unit
     */
    if (unit != 1)
    {
        if (!STKTPLG_OM_UnitExist(unit))
            return FALSE;

        STKTPLG_OM_CHECK_STATE();
    }

    if (mau_index > SYS_ADPT_MAX_NBR_OF_MAU_PER_USER_PORT)
    {
        return (FALSE);
    }

    board_id = stk_unit_cfg[unit - 1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        return (FALSE);
    }

    if ((port < 1) || (port > board_info_p->max_port_number_of_this_unit))
    {
        return (FALSE);
    }

    if (mau_index > board_info_p->portMediaNum_ptr[port-1])
    {
        return (FALSE);
    }

    /*if (port >= board_info_p->start_expansion_port_number)*/
    /* Check portMediaCap_ptr, if the port have both fiber and TX capability,
     * mean that it have mau_index.
     */
    if ( (STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER | STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER) == board_info_p->portMediaCap_ptr[port-1] )
    {
        /*Combo Port*/
        switch(mau_index)
        {
            case 1:
                *media_type = board_info_p->portMediaType_ptr[port-1];
                break;

            case 2:
            #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
                //mod_id = board_info_p->port2ModId_ptr[port - board_info_p->start_sfp_port_number];
                sfp_index = board_info_p->port2ModId_ptr[port - board_info_p->start_sfp_port_number]+1;

                //if (stk_unit_cfg[unit-1].gbic_info[mod_id].body_present)
                if (TRUE == SWCTRL_POM_GetPortSfpPresent(unit, sfp_index, &is_present) &&
                    TRUE == is_present &&
                    TRUE == SWCTRL_POM_GetPortSfpInfo(unit, sfp_index, &sfp_info))
                {
                    /* According to SFP MSA, Data Addr 3-8 traceiver code,
                     * byte 6 (ie,transceiver[3]) is compliance codes
                     */
                    switch (sfp_info.transceiver[3])
                    {
                        case GIGABIT_CODES_VLAUE_1000BASE_SX:
                            *media_type = STKTPLG_TYPE_MEDIA_TYPE_THOUSAND_BASE_SX;
                            break;
                        case GIGABIT_CODES_VLAUE_1000BASE_LX:
                            *media_type = STKTPLG_TYPE_MEDIA_TYPE_THOUSAND_BASE_LX;
                            break;
                        case GIGABIT_CODES_VLAUE_1000BASE_CX:
                            *media_type = STKTPLG_TYPE_MEDIA_TYPE_THOUSAND_BASE_CX;
                            break;
                        case GIGABIT_CODES_VLAUE_1000BASE_T:
                            *media_type = STKTPLG_TYPE_MEDIA_TYPE_THOUSAND_BASE_T;
                            break;
                        default:
                            *media_type = STKTPLG_TYPE_MEDIA_TYPE_OTHER;
                            break;
                    }
                }
                else
                {
                    return (FALSE);
                }
            #endif /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */
                break;

            default:
                perror("\r\nmau_index out of range");
                assert(0);
        }
    }
    else if((STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER == board_info_p->portMediaCap_ptr[port-1])||
            (STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER  == board_info_p->portMediaCap_ptr[port-1]))
    {
        /*Front Port*/
        *media_type = board_info_p->portMediaType_ptr[port-1];
    }
    else /*Module Port*/
    {
        if(STKTPLG_OM_OptionModuleIsExist(unit, &dummy_unit) == TRUE)
        {
            STKTPLG_OM_GetModuleType(unit, dummy_module_num, &module_type);
            if(module_type == VAL_swModuleType_tenGigaPortModule)
            {
                memset(&module_info,0,sizeof(STKTPLG_OM_switchModuleInfoEntry_T));
                STKTPLG_OM_GetModuleInfo(unit, dummy_module_num, &module_info);
                switch(module_info.xenpak_status)
                {
                    case SYS_DRV_XENPAK_NOT_PRESENT:
                    case SYS_DRV_XENPAK_UNKNOWNTYPE:
                        *media_type = STKTPLG_TYPE_MEDIA_TYPE_OTHER;
                        break;

                    case SYS_DRV_XENPAK_UNSUPPORTED_LR:
                    case SYS_DRV_XENPAK_SUPPORTED_LR:
                        *media_type = STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_LR;
                        break;

                    case SYS_DRV_XENPAK_UNSUPPORTED_ER:
                    case SYS_DRV_XENPAK_SUPPORTED_ER:
                        *media_type = STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_ER;
                        break;

                    case SYS_DRV_XENPAK_UNSUPPORTED_CX4:
                    case SYS_DRV_XENPAK_SUPPORTED_CX4:
                        *media_type = STKTPLG_TYPE_MEDIA_TYPE_10G_BASE_CX4;
                        break;

                    default:
                        *media_type = STKTPLG_TYPE_MEDIA_TYPE_OTHER;
                        break;
                }
            }
            else /* module_type not VAL_swModuleType_tenGigaPortModule */
            {
                *media_type = STKTPLG_TYPE_MEDIA_TYPE_OTHER;
            }
        }
        else /* module not exist */
        {
            *media_type = board_info_p->portMediaType_ptr[port-1];
        }
    }

    return (TRUE);
}
#endif /* (SYS_CPNT_MAU_MIB == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetPortMediaCapability
 * -------------------------------------------------------------------------
 * FUNCTION: Get media capability of port.
 * INPUT   : unit       -- Which unit.
 *           port       -- Which port. *
 * OUTPUT  : media_cap  -- STKTPLG_TYPE_PORT_MEDIA_CAP_E
 * RETURN  : TRUE/FALSE
 * NOTE    : media_cap is a bitmap of possible media capability. It's
 *           useful to identify the combo port
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetPortMediaCapability(UI32_T unit, UI32_T port, UI32_T* media_cap)
{
    UI8_T                      board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    /* Non-Local unit
     */
    if (unit != 1)
    {
        if (!STKTPLG_OM_UnitExist(unit))
            return FALSE;
    }

    SYSFUN_TakeReadSem(sem_id_static);
 /*EPR:ES3628BT-FLF-ZZ-00637
      Problem:stack:DUT display error message when upload running config to tftp server when 8 stack
      RootCause:unit is the max unit Id,here check will return FALSE
      Solution:if it is the max ,not return FALSE
      File:stktplg_om.c
 */
#if 0
    if (unit >= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
#else
    if (unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
#endif
    {
        SYSFUN_GiveReadSem(sem_id_static);
        return FALSE;
    }

    board_id = stk_unit_cfg[unit - 1].board_info.board_id;
    SYSFUN_GiveReadSem(sem_id_static);

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        return (FALSE);
    }

    if ((port < 1) || (port > board_info_p->max_port_number_of_this_unit))
    {
        return (FALSE);
    }

    *media_cap = board_info_p->portMediaCap_ptr[port-1];

#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE)
    /* TBD: need new patch for stacking & mau mib.
     *
     * temporal soultion for module port media type (combo port force mode)
     */
    {
        UI8_T module_id;

        if (STKTPLG_BOARD_GetModuleID(board_id, port, &module_id))
        {
            switch (module_id)
            {
#ifdef SYS_HWCFG_MODULE_ID_XFP
                case SYS_HWCFG_MODULE_ID_XFP:
                    *media_cap = STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER;
                    break;
#endif
#ifdef SYS_HWCFG_MODULE_ID_SFP_PLUS
                case SYS_HWCFG_MODULE_ID_SFP_PLUS:
                    *media_cap = STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER;
                    break;
#endif
#ifdef SYS_HWCFG_MODULE_ID_SFP
                case SYS_HWCFG_MODULE_ID_SFP:
                    *media_cap = STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER;
                    break;
#endif
#ifdef SYS_HWCFG_MODULE_ID_10G_COPPER
                case SYS_HWCFG_MODULE_ID_10G_COPPER:
                    *media_cap = STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER;
                    break;
#endif
#ifdef SYS_HWCFG_MODULE_ID_10G_SFP
                case SYS_HWCFG_MODULE_ID_10G_SFP:
                    *media_cap = STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER;
                    break;
#endif
                default:
                    *media_cap = STKTPLG_TYPE_PORT_MEDIA_CAP_OTHER;
            }
        }
    }
#endif /* (SYS_CPNT_10G_MODULE_SUPPORT == TRUE) */

    return (TRUE);
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetNextUnit
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_id       -- Which Unit
 * OUTPUT  : unit_id       -- Next Unit ID
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetNextUnit(UI32_T *unit_id)
{

    UI32_T index, next_id, tmp_unit;
    BOOL_T retval=FALSE;

    if (*unit_id == 8)
        return (retval);

    /*for (index = 0; index < ctrl_info_snapshot.total_units_up; index++)
    {
      printf("[%s] %d unit %d\n",__FUNCTION__,__LINE__,ctrl_info_snapshot.stable_hbt_up.payload[index].unit_id);

    }
    for (index = 0; index < ctrl_info_snapshot.total_units_down; index++)
    {
      printf("[%s] %d unit %d\n",__FUNCTION__,__LINE__,ctrl_info_snapshot.stable_hbt_down.payload[index].unit_id);

    }*/

    next_id = 9;
    SYSFUN_TakeReadSem(sem_id_static);
    for (index = 0; index < ctrl_info_snapshot.total_units_up; index++)
    {
        tmp_unit = ctrl_info_snapshot.stable_hbt_up.payload[index].unit_id;
        if ( (tmp_unit > *unit_id) &&
             (tmp_unit <  next_id) &&
             (TRUE == unit_is_valid[tmp_unit-1]) )
            next_id = tmp_unit;
    }

    for (index = 0; index < ctrl_info_snapshot.total_units_down; index++)
    {
        tmp_unit = ctrl_info_snapshot.stable_hbt_down.payload[index].unit_id;
        if ( (tmp_unit > *unit_id) &&
             (tmp_unit <  next_id) &&
             (TRUE == unit_is_valid[tmp_unit-1]))
            next_id = tmp_unit;
    }

    if ( (next_id > 0) && (next_id < 9) )
    {
        *unit_id = next_id;
        retval = TRUE;
    }
    SYSFUN_GiveReadSem(sem_id_static);
    return (retval);

}

#else
/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetNextUnit
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_id       -- Which Unit
 * OUTPUT  : unit_id       -- Next Unit ID
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetNextUnit(UI32_T *unit_id)
{

    UI32_T index, next_id;
    BOOL_T retval=FALSE;

    if (*unit_id == 8)
        return (retval);

    next_id = 9;
    SYSFUN_TakeReadSem(sem_id_static);
    for (index = 0; index < ctrl_info.total_units_up; index++)
    {
        if ( (ctrl_info.stable_hbt_up.payload[index].unit_id > *unit_id) &&
             (ctrl_info.stable_hbt_up.payload[index].unit_id <  next_id) )
            next_id = ctrl_info.stable_hbt_up.payload[index].unit_id;
    }

    for (index = 0; index < ctrl_info.total_units_down; index++)
    {
        if ( (ctrl_info.stable_hbt_down.payload[index].unit_id > *unit_id) &&
             (ctrl_info.stable_hbt_down.payload[index].unit_id <  next_id) )
            next_id = ctrl_info.stable_hbt_down.payload[index].unit_id;
    }

    if ( (next_id > 0) && (next_id < 9) )
    {
        *unit_id = next_id;
        retval = TRUE;
    }
    SYSFUN_GiveReadSem(sem_id_static);
    return (retval);

}
#endif

/* FUNCTION NAME : STKTPLG_OM_GetPortMapping
 * PURPOSE: To get the maximum port number of the local unit.
 * INPUT:   None.
 * OUTPUT:  *max_port_number -- maximum port number of the local unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetPortMapping(DEV_SWDRV_Device_Port_Mapping_T *mapping, UI32_T unit)
{
    UI8_T    board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    if(unit<1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
      return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);
    board_id = stk_unit_cfg[ctrl_info.my_unit_id-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        SYSFUN_GiveReadSem(sem_id_static);
        perror("\r\nCan not get related board information.\r\n");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
  memcpy((void *)mapping, &stackPortMappingTable[unit-1],
        sizeof(DEV_SWDRV_Device_Port_Mapping_T)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT);

#else
    memcpy((void *)mapping, &board_info_p->userPortMappingTable[unit-1],
           sizeof(DEV_SWDRV_Device_Port_Mapping_T)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT);
#endif
    SYSFUN_GiveReadSem(sem_id_static);
    return (TRUE);

} /* End of STKTPLG_OM_GetPortMapping() */

/* FUNCTION NAME : STKTPLG_OM_SetPortMapping
 * PURPOSE: To set the maximum port number of the local unit.
 * INPUT:   None.
 * OUTPUT:  *max_port_number -- maximum port number of the local unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_SetPortMapping(DEV_SWDRV_Device_Port_Mapping_T *mapping, UI32_T unit)
{
    UI8_T    board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);
    board_id = stk_unit_cfg[ctrl_info.my_unit_id-1].board_info.board_id;
    SYSFUN_GiveReadSem(sem_id_static);

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.\r\n");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

#if 0 /* debug code */
{
    int i;
    printf("%s:unit=%d\n", __FUNCTION__, unit);
    for(i=0;i<5;i++)
    {
        printf("(%d)[%d][%d][%d][%d][%d]\n", i, mapping[i].module_id ,
            mapping[i].device_id, mapping[i].device_port_id,
            mapping[i].phy_id, mapping[i].port_type);
    }
}
#endif
    SYSFUN_TakeWriteSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    memcpy(&stackPortMappingTable[unit-1], (void *)mapping,
            sizeof(DEV_SWDRV_Device_Port_Mapping_T)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT);
#else
    memcpy(&board_info_p->userPortMappingTable[unit-1], (void *)mapping,
           sizeof(DEV_SWDRV_Device_Port_Mapping_T)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT);

#endif
    SYSFUN_GiveWriteSem(sem_id_static);
    return (TRUE);

} /* End of STKTPLG_OM_SetPortMapping() */


/* FUNCTION NAME :
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   :
 * RETUEN   :
 * Notes    :
 */
BOOL_T STKTPLG_OM_SetAllUnitsPortMapping(DEV_SWDRV_Device_Port_Mapping_T mapping)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == FALSE)
        UI8_T    board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
#endif


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    memcpy(&stackPortMappingTable, &mapping,
           sizeof(DEV_SWDRV_Device_Port_Mapping_T)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);
#else
    board_id = stk_unit_cfg[ctrl_info.my_unit_id-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.\r\n");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    memcpy(&board_info_p->userPortMappingTable, &mapping,
           sizeof(DEV_SWDRV_Device_Port_Mapping_T)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);
#endif
    return (TRUE);
}

/* FUNCTION NAME : STKTPLG_OM_Transition
 * PURPOSE  : STKTPLG to notify tplg state for image version validation.
 * INPUT    :
 * OUTPUT   :
 * RETUEN   :
 * Notes    :
 */
void STKTPLG_OM_Transition(BOOL_T ready)
{
    transition_ready = ready;

    return;
}

/* FUNCTION NAME :
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   :
 * RETUEN   :
 * Notes    :
 */
BOOL_T STKTPLG_OM_IsTransition(void)
{
    return (transition_ready);
}

/* FUNCTION NAME: STKTPLG_OM_ProvisionCompleted
 * PURPOSE: To set provision complete flag.
 * INPUT:   ready     -- provision complete flag to be set.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKTPLG_OM_ProvisionCompleted(BOOL_T ready)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    if(TRUE == ready)
    {
        provision_completed_once = TRUE;
    }

    provision_completed = ready;

#else
    provision_completed = ready;
#endif
    return;
}


/* FUNCTION NAME : STKTPLG_OM_IsProvisionCompleted
 * PURPOSE  : To check if the system provision complete or not.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : TRUE/FALSE.
 * Notes    : None.
 */
BOOL_T STKTPLG_OM_IsProvisionCompleted(void)
{
    return (provision_completed);
}


/* FUNCTION NAME :
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   :
 * RETUEN   :
 * Notes    :
 */
BOOL_T STKTPLG_OM_GetAllUnitsPortMapping(DEV_SWDRV_Device_Port_Mapping_T mapping[][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT])
{
#if (SYS_CPNT_UNIT_HOT_SWAP == FALSE)

    UI8_T    board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
#endif
    int size;

    size = sizeof(DEV_SWDRV_Device_Port_Mapping_T)*
        SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT *
        SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
     memcpy(mapping,&stackPortMappingTable, size);

#else
    board_id = stk_unit_cfg[ctrl_info.my_unit_id-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.\r\n");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    memcpy(mapping,&(board_info_p->userPortMappingTable), size);
#endif


    return (TRUE);

} /* End of STKTPLG_OM_GetPortMapping() */

/* FUNCTION NAME : STKTPLG_OM_ResetStackingDB
 * PURPOSE: To reset the stackingDB.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_OM_ResetStackingDB(void)
{

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    UI32_T i;
#endif

    SYSFUN_TakeWriteSem(sem_id_static);
    memset(stackingdb, 0, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * sizeof(STKTPLG_DataBase_Info_T));

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
     for(i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
        unit_use_default[i] = TRUE;
    }
#endif
    SYSFUN_GiveWriteSem(sem_id_static);
}

/* FUNCTION NAME : STKTPLG_OM_GetStackingDB
 * PURPOSE: To get the stacking db from cli_om.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- Success
 *          FALSE        -- Fail
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetStackingDB(void)
{
    UI32_T read_count, i;
    STKTPLG_OM_StackingDB_T stacking_db[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];

    /* setting this field is not protected by semaphore. it should be
     * all right. If not, move this field out of control information
     */

    read_count=STKTPLG_OM_GetStackingDBFromFS(stacking_db);
    if (read_count==0)
    {
        //printf("TPLG: No info about ID-MAC in CFG file\n");
        return (FALSE);
    }

    SYSFUN_TakeWriteSem(sem_id_static);
    for (i=0; i<read_count; i++)
    {
        memcpy(stackingdb[stacking_db[i].unit_id-1].mac_addr,stacking_db[i].mac_addr,SYS_ADPT_MAC_ADDR_LEN);
        stackingdb[stacking_db[i].unit_id-1].device_type = (UI8_T) stacking_db[i].device_type;
        stackingdb[stacking_db[i].unit_id-1].id_flag = 1 ;
    }
    SYSFUN_GiveWriteSem(sem_id_static);
    return (TRUE);
} /* End of STKTPLG_OM_GetStackingDB() */

/* FUNCTION NAME : STKTPLG_OM_GetStackingDBEntry
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetStackingDBEntry(STKTPLG_DataBase_Info_T *db, UI32_T entry)
{

    BOOL_T retval = FALSE;

    /* setting this field is not protected by semaphore. it should be
     * all right. If not, move this field out of control information
     */
    if (entry < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        SYSFUN_TakeReadSem(sem_id_static);
        memcpy(db, &stackingdb[entry], sizeof(STKTPLG_DataBase_Info_T));
        SYSFUN_GiveReadSem(sem_id_static);
        retval = TRUE;
    }

    return (retval);
} /* End of STKTPLG_OM_GetStackingDB() */

/* FUNCTION NAME : STKTPLG_OM_DumpStackingDB
 * PURPOSE: Dump stackingdb and current_temp_stackingdb for information
 * INPUT:   None
 * OUTPUT:  None.
 * RETUEN:  None
 * NOTES:
 */
void STKTPLG_OM_DumpStackingDB(void)
{
    UI32_T    i, unit;
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
        xgs_stacking_om_debug("stackingdb Uid:%lu MAC:%02x-%02x dev:%u\n", 
            i+1, stackingdb[i].mac_addr[4],stackingdb[i].mac_addr[5], 
            stackingdb[i].device_type);
    }

    for(unit=0;unit<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;unit++)
    {
        xgs_stacking_om_debug("TempDB Uid:%lu MAC:%02x-%02x dev:%u\n", 
        current_temp_stackingdb[unit].unit_id,
        current_temp_stackingdb[unit].mac_addr[4],
        current_temp_stackingdb[unit].mac_addr[5],
        current_temp_stackingdb[unit].device_type);
    }
}

/* FUNCTION NAME : STKTPLG_OM_GetStackingDBEntryByMac
 * PURPOSE: This api will return the unit id according to the given mac and set
 *          the specified device_type to the entry of the given mac.
 *
 * INPUT:   mac          -- the mac which is used to find the db entry
 *          device_type  -- board id of the found db entry. this id will be set
 *                          to the entry if it is found.
 * OUTPUT:  None.
 * RETUEN:  Non-Zero: unit id of the db entry with the given mac
 *          Zero: not found
 * NOTES:
 *
 */
UI32_T STKTPLG_OM_GetStackingDBEntryByMac(UI8_T mac[STKTPLG_MAC_ADDR_LEN],UI8_T device_type)
{
    UI32_T    i;

    SYSFUN_TakeWriteSem(sem_id_static);
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
        if (memcmp(mac , stackingdb[i].mac_addr,STKTPLG_MAC_ADDR_LEN) == 0)
        {
 /* EPR: ES3628BT-FLF-ZZ-00546
     Problem: 1,when 3 dut stacking together,after system bring up ,it will produce a config file "startup" to store the stacking topo info
                      2, remove a slave A which unit ID is 3, then add a new dut.The new DUT will use unit ID 3
                      3,add A which removed before  the new dut added completed,sometimes the A will assign unit ID 3,and the new will be
                         4,but the led of the new dut still 3.Because cli has not save the topo of the new dut before the dut A added
                      4  the new dut detect the same master,and will do nothing
      Solution: check if the unit Id is used by others before assign unit id
*/

           if(STKTPLG_OM_CheckIfUnitUsed(i+1,mac))

           {
            stackingdb[i].id_flag = 3 ;
            stackingdb[i].device_type = device_type;
            xgs_stacking_om_debug("STKTPLG_OM_GetStackingDBEntryByMac,device_type %d,mac %x-%x-%x-%x-%x-%x\n",device_type,stackingdb[i].mac_addr[0],
                stackingdb[i].mac_addr[1],stackingdb[i].mac_addr[2],stackingdb[i].mac_addr[3],stackingdb[i].mac_addr[4],
                stackingdb[i].mac_addr[5]);
            unit_use_default[i] = FALSE;
            SYSFUN_GiveWriteSem(sem_id_static);
            return (i+1);
          }

          else
          {
           xgs_stacking_om_debug("\r\n %s %d",__FUNCTION__,__LINE__);
            SYSFUN_GiveWriteSem(sem_id_static);
            return(0);
          }

        }
    }
    SYSFUN_GiveWriteSem(sem_id_static);
    return (0);
} /* End of STKTPLG_OM_GetStackingDBEntryByMac() */

/* FUNCTION NAME : STKTPLG_OM_GetStackingDBFreeEntry
 * PURPOSE: This api will return the best-fit unit id according to the given mac
 *          and device_type(a.k.a. board_id).
 *
 * INPUT:   mac          -- the mac of the unit which needs to be assigned a unit id.
 *          device_type  -- board id of the unit which needs to be assigned a unit id.
 *
 * OUTPUT:  None.
 * RETUEN:  Non-Zero: An available unit id
 *          Zero: No availabe unit id
 * NOTES:
 *       This api will first try to find whether the given mac and device_type
 *       have ever existed in stackingdb before(id_flag==1). If the entry does
 *       not exist before, it will try to assign the unit id from an empty entry
 *       (id_flag==0). If no empty entry is available, it will try to assign the
 *       unit id that had ever been assigned before(id_flag==1).
 *
 */
UI32_T STKTPLG_OM_GetStackingDBFreeEntry(UI8_T mac[STKTPLG_MAC_ADDR_LEN],UI8_T device_type)
{
    UI32_T    i;

    SYSFUN_TakeWriteSem(sem_id_static);
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
        if (stackingdb[i].id_flag == 1 && stackingdb[i].device_type == device_type) /* we should also compare device_type */
        {
            memcpy(stackingdb[i].mac_addr, mac, STKTPLG_MAC_ADDR_LEN);
            stackingdb[i].device_type = device_type;
            stackingdb[i].id_flag = 3 ;
            unit_use_default[i] = FALSE;
            SYSFUN_GiveWriteSem(sem_id_static);
            return (i+1);
        }
    }
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
        if (stackingdb[i].id_flag == 0)
        {
            stackingdb[i].id_flag = 3 ;
            memcpy(stackingdb[i].mac_addr, mac, STKTPLG_MAC_ADDR_LEN);
            stackingdb[i].device_type = device_type;
            unit_use_default[i] = TRUE;
            SYSFUN_GiveWriteSem(sem_id_static);
            return (i+1);
        }
    }
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
        if (stackingdb[i].id_flag == 1)
        {
            memcpy(stackingdb[i].mac_addr, mac, STKTPLG_MAC_ADDR_LEN);
            stackingdb[i].id_flag = 3 ;
            stackingdb[i].device_type = device_type;
            unit_use_default[i] = TRUE;
            SYSFUN_GiveWriteSem(sem_id_static);
            return (i+1);
        }
    }
    SYSFUN_GiveWriteSem(sem_id_static);
    printf("***** No avaiable unit ID *******\r\n");
    return (0);
} /* End of STKTPLG_OM_GetStackingDB() */

/* FUNCTION NAME: STKTPLG_OM_GetCurrentStackingDB
 * PURPOSE: This API is used to get the mac and unit id and device type
 * in the stacking
 * INPUT:   None.
 * OUTPUT:  stacking_db : array of structure.
 * RETUEN:  0  -- failure
 *          otherwise -- success. the returned value is the number of entries
 * NOTES:   None.
 */

UI8_T STKTPLG_OM_GetCurrentStackingDB(STKTPLG_OM_StackingDB_T stacking_db[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK])
{
    UI32_T i;
    UI8_T  ret;

    SYSFUN_TakeReadSem(sem_id_static);
    for (ret=0, i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
        if (stackingdb[i].id_flag == 3)
        {
            stacking_db[ret].unit_id = i+1;
            memcpy(stacking_db[ret].mac_addr, stackingdb[i].mac_addr,STKTPLG_MAC_ADDR_LEN);
            stacking_db[ret].device_type = (UI32_T) stackingdb[i].device_type;
        /*    printf(" \r\n CurrentStackingDB[%d],unit_id = %d \r\n",ret,i+1); */
            ret++;
        }
    }
    SYSFUN_GiveReadSem(sem_id_static);
    return(ret);
}
/* FUNCTION NAME : STKTPLG_OM_SetStackingDB
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_SetStackingDB(void)
{
    UI32_T result;

    /* setting this field is not protected by semaphore. it should be
     * all right. If not, move this field out of control information
     */

    result = FS_WriteFile(DUMMY_DRIVE, (UI8_T*)"$stacking_DB", (UI8_T*)"StackingDB", FS_FILE_TYPE_PRIVATE,
                          (UI8_T *)stackingdb, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * sizeof(STKTPLG_DataBase_Info_T),
                          SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * sizeof(STKTPLG_DataBase_Info_T));

    XFER_PMGR_AutoDownLoad("","$stacking_DB","$stacking_DB",FS_FILE_TYPE_PRIVATE,FALSE,0,0);

    return (result == FS_RETURN_OK) ? TRUE:FALSE;
} /* End of STKTPLG_OM_SetStackingDB() */

/* FUNCTION NAME : STKTPLG_OM_SetStackingDBEntry
 * PURPOSE: Set the stacking db entry according to the given entry id.
 * INPUT:   db           -- the entry to be set to the stacking db
 *          entry        -- the index of the stacking db entry to be set, this
 *                          is also the value of the unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- Sucess
 *          FALSE        -- Fail
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_SetStackingDBEntry(STKTPLG_DataBase_Info_T *db, UI32_T entry)
{

    BOOL_T retval = FALSE;

    /* setting this field is not protected by semaphore. it should be
     * all right. If not, move this field out of control information
     */
    SYSFUN_TakeWriteSem(sem_id_static);
    if (entry <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        memcpy(&stackingdb[entry-1], db, sizeof(STKTPLG_DataBase_Info_T));
        retval = TRUE;
    }
    SYSFUN_GiveWriteSem(sem_id_static);

    return (retval);
} /* End of STKTPLG_OM_GetStackingDB() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetNextUnitUp
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_id       -- Which Unit
 * OUTPUT  : unit_id       -- Next Unit ID Up
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetNextUnitUp(UI32_T *unit_id)
{
    UI32_T index, done;
    BOOL_T retval;

    STKTPLG_OM_CHECK_STATE();

    retval = FALSE;
    done = 0;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    SYSFUN_TakeReadSem(sem_id_static);
    for (index = 0; (index < ctrl_info_snapshot.total_units_up) && (0 == done); index++)
    {
        if (ctrl_info_snapshot.stable_hbt_up.payload[index].unit_id == *unit_id)
        {
            done = 1;

            if (index != (ctrl_info_snapshot.total_units_up - 1))
            {
                retval = TRUE;
                *unit_id = ctrl_info_snapshot.stable_hbt_up.payload[index+1].unit_id;
            }
        }
    }

    for (index = 0; (index < ctrl_info_snapshot.total_units_down) && (0 == done); index++)
    {
        if (ctrl_info_snapshot.stable_hbt_down.payload[index].unit_id == *unit_id)
        {
            done = 1;

            if (index != 0)
            {
                retval = TRUE;
                *unit_id = ctrl_info_snapshot.stable_hbt_down.payload[index-1].unit_id;
            }
        }
    }
    SYSFUN_GiveReadSem(sem_id_static);
#else
    SYSFUN_TakeReadSem(sem_id_static);
    for (index = 0; (index < ctrl_info.total_units_up) && (0 == done); index++)
    {
        if (ctrl_info.stable_hbt_up.payload[index].unit_id == *unit_id)
        {
            done = 1;

            if (index != (ctrl_info.total_units_up - 1))
            {
                retval = TRUE;
                *unit_id = ctrl_info.stable_hbt_up.payload[index+1].unit_id;
            }
        }
    }

    for (index = 0; (index < ctrl_info.total_units_down) && (0 == done); index++)
    {
        if (ctrl_info.stable_hbt_down.payload[index].unit_id == *unit_id)
        {
            done = 1;

            if (index != 0)
            {
                retval = TRUE;
                *unit_id = ctrl_info.stable_hbt_down.payload[index-1].unit_id;
            }
        }
    }
    SYSFUN_GiveReadSem(sem_id_static);
#endif
    return (retval);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetNextUnitDown
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_id       -- Which Unit
 * OUTPUT  : unit_id       -- Next Unit ID down
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetNextUnitDown(UI32_T *unit_id)
{
    UI32_T index, done;
    BOOL_T retval;

    STKTPLG_OM_CHECK_STATE();
    retval = FALSE;
    done = 0;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    SYSFUN_TakeReadSem(sem_id_static);
    for (index = 0; (index < ctrl_info_snapshot.total_units_up) && (0 == done); index++)
    {
        if (ctrl_info_snapshot.stable_hbt_up.payload[index].unit_id == *unit_id)
        {
            done = 1;

            if (index != 0)
            {
                retval = TRUE;
                *unit_id = ctrl_info_snapshot.stable_hbt_up.payload[index-1].unit_id;
            }
        }
    }

    for (index = 0; (index < ctrl_info_snapshot.total_units_down) && (0 == done); index++)
    {
        if (ctrl_info_snapshot.stable_hbt_down.payload[index].unit_id == *unit_id)
        {
            done = 1;

            if (index != (ctrl_info_snapshot.total_units_down - 1))
            {
                retval = TRUE;
                *unit_id = ctrl_info_snapshot.stable_hbt_down.payload[index+1].unit_id;
            }
        }
    }
    SYSFUN_GiveReadSem(sem_id_static);
#else
    SYSFUN_TakeReadSem(sem_id_static);
    for (index = 0; (index < ctrl_info.total_units_up) && (0 == done); index++)
    {
        if (ctrl_info.stable_hbt_up.payload[index].unit_id == *unit_id)
        {
            done = 1;

            if (index != 0)
            {
                retval = TRUE;
                *unit_id = ctrl_info.stable_hbt_up.payload[index-1].unit_id;
            }
        }
    }

    for (index = 0; (index < ctrl_info.total_units_down) && (0 == done); index++)
    {
        if (ctrl_info.stable_hbt_down.payload[index].unit_id == *unit_id)
        {
            done = 1;

            if (index != (ctrl_info.total_units_down - 1))
            {
                retval = TRUE;
                *unit_id = ctrl_info.stable_hbt_down.payload[index+1].unit_id;
            }
        }
    }
    SYSFUN_GiveReadSem(sem_id_static);
#endif
    return (retval);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetNextUnitDown
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_a       -- Which Unit
 * INPUT   : unit_b       -- Which Unit
 * OUTPUT  : position     -- Relative position from unit_a,
 *                           high-bit set if unit_b is in the Down direction
 *                           of unit_a.
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetUnitsRelPosition(UI32_T unit_a, UI32_T unit_b, UI32_T *position)
{
    UI32_T index, done, up_pos_a, down_pos_a, up_pos_b, down_pos_b;
    BOOL_T retval;

    retval = FALSE;

    STKTPLG_OM_CHECK_STATE();

    if (unit_a == unit_b)
    {
        *position = 0;
        return (retval);
    }

    up_pos_a   = 0xff;
    down_pos_a = 0xff;
    up_pos_b   = 0xff;
    down_pos_b = 0xff;

    done = 0;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    SYSFUN_TakeReadSem(sem_id_static);
    for (index = 0; (index < ctrl_info_snapshot.total_units_down) && (0 == done); index++)
    {
        if (ctrl_info_snapshot.stable_hbt_down.payload[index].unit_id == unit_a)
        {
            done = 1;
            down_pos_a = index;
        }
    }
    for (index = 0; (index < ctrl_info_snapshot.total_units_up) && (0 == done); index++)
    {
        if (ctrl_info_snapshot.stable_hbt_up.payload[index].unit_id == unit_a)
        {
            done = 1;
            up_pos_a = index;
        }
    }
    done = 0;
    for (index = 0; (index < ctrl_info_snapshot.total_units_down) && (0 == done); index++)
    {
        if (ctrl_info_snapshot.stable_hbt_down.payload[index].unit_id == unit_b)
        {
            done = 1;
            down_pos_b = index;
        }
    }
    for (index = 0; (index < ctrl_info_snapshot.total_units_up) && (0 == done); index++)
    {
        if (ctrl_info_snapshot.stable_hbt_up.payload[index].unit_id == unit_b)
        {
            done = 1;
            up_pos_b = index;
        }
    }
    SYSFUN_GiveReadSem(sem_id_static);
#else
    SYSFUN_TakeReadSem(sem_id_static);
    for (index = 0; (index < ctrl_info.total_units_down) && (0 == done); index++)
    {
        if (ctrl_info.stable_hbt_down.payload[index].unit_id == unit_a)
        {
            done = 1;
            down_pos_a = index;
        }
    }
    for (index = 0; (index < ctrl_info.total_units_up) && (0 == done); index++)
    {
        if (ctrl_info.stable_hbt_up.payload[index].unit_id == unit_a)
        {
            done = 1;
            up_pos_a = index;
        }
    }
    done = 0;
    for (index = 0; (index < ctrl_info.total_units_down) && (0 == done); index++)
    {
        if (ctrl_info.stable_hbt_down.payload[index].unit_id == unit_b)
        {
            done = 1;
            down_pos_b = index;
        }
    }
    for (index = 0; (index < ctrl_info.total_units_up) && (0 == done); index++)
    {
        if (ctrl_info.stable_hbt_up.payload[index].unit_id == unit_b)
        {
            done = 1;
            up_pos_b = index;
        }
    }
    SYSFUN_GiveReadSem(sem_id_static);
#endif

    if ( (down_pos_a != 0xff) && (down_pos_b != 0xff) )
    {
        if (down_pos_a > down_pos_b)
            *position = down_pos_a - down_pos_b;
        else
            *position = (down_pos_b - down_pos_a) | 0x80000000;

        retval = TRUE;
    }
    else if ( (up_pos_a != 0xff) && (up_pos_b != 0xff) )
    {
        if (up_pos_a > up_pos_b)
            *position = (up_pos_a - up_pos_b) | 0x80000000;
        else
            *position = up_pos_b - up_pos_a;

        retval = TRUE;
    }
    else if ( (up_pos_a != 0xff) && (down_pos_b != 0xff) )
    {
        *position = (up_pos_a + down_pos_b) | 0x80000000;

        retval = TRUE;
    }
    else if ( (down_pos_a != 0xff) && (up_pos_b != 0xff) )
    {
        *position = down_pos_a + up_pos_b;

        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    return (retval);
}

/* FUNCTION NAME: STKTPLG_OM_InsertExpModule
 * PURPOSE: This function is used to insert slot-in module.
 * INPUT:   unit            -- unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_InsertExpModule(UI32_T unit_id,UI8_T module_type,UI8_T *module_runtime_fw_ver)
{
    UI32_T i;
    UI8_T   board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    UI32_T ModulePortNumber;

    printf("Insert Module => Unit: %lu, Type: %u, Version: %s.\r\n", unit_id, module_type, module_runtime_fw_ver);
    if(unit_id <1 || unit_id >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

    board_id = stk_unit_cfg[(UI8_T)unit_id-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    SYSFUN_TakeWriteSem(sem_id_static);
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT; i++)
    {
        /* Charles hard code here, because we only support 8-sfp module,
         * need to re-design related mechanism.
         */

        /* 05/03/2004 10:16上午 vivid because diag did not support for option module type,
         *  hard code temporaty
         */
        //stk_unit_cfg[(UI8_T)unit_id-1].exp_module_type[i] = VAL_swModuleType_eightPortSfpModule /*module_type*/;
        //stk_unit_cfg[(UI8_T)unit_id-1].exp_module_type[i] = VAL_swModuleType_tenGigaPortModule /* 10G module*/;
        stk_unit_cfg[(UI8_T)unit_id-1].exp_module_type[i] = module_type; //Module Type set to Inserted Module Type
        stk_unit_cfg[(UI8_T)unit_id-1].exp_module_presented[i] = 1;

    }
   /* memcpy(stk_unit_cfg[(UI8_T)unit_id-1].module_runtime_fw_ver, module_runtime_fw_ver, SYS_ADPT_FW_VER_STR_LEN + 1); */
    memcpy(stk_module_cfg[unit_id - 1][0].op_code_ver, module_runtime_fw_ver, SYS_ADPT_FW_VER_STR_LEN + 1);

    //printf(" before insert unit=%d insert OM  nport=%d\r\n",(UI8_T)unit_id,stk_unit_cfg[(UI8_T)unit_id-1].nport);
    /* vivid mark */
    /*
    stk_unit_cfg[(UI8_T)unit_id-1].nport=board_info_p->max_port_number_on_board+(board_info_p->max_port_number_of_this_unit-board_info_p->max_port_number_on_board);
    */
    /* 05/03/2004 10:17上午 port number is read by option module type*/
    STKTPLG_BOARD_GetPortNumberInformation(module_type,&ModulePortNumber);
    stk_unit_cfg[(UI8_T)unit_id-1].nport=board_info_p->max_port_number_on_board + ModulePortNumber;
    //printf(" *** unit=%d insert OM  nport=%d  module_type=%d presented=%d\n",unit_id,stk_unit_cfg[(UI8_T)unit_id-1].nport,stk_unit_cfg[(UI8_T)unit_id-1].exp_module_type[0],stk_unit_cfg[(UI8_T)unit_id-1].exp_module_presented[0]);
    ctrl_info.exp_module_state_dirty[unit_id-1] = TRUE;
    SYSFUN_GiveWriteSem(sem_id_static);

    return TRUE;

}/* End of STKTPLG_OM_InsertExpModule */

/* FUNCTION NAME: STKTPLG_OM_ExpModuleIsInsert
 * PURPOSE: This function is used to check  slot-in module. insert or not
 * INPUT:   unit            -- unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- insert and ready
 *          FALSE -- non-insert ,or un-ready
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_ExpModuleIsInsert(UI32_T unit_id)
{
    if(stk_unit_cfg[(UI8_T)unit_id-1].exp_module_presented[0] ==1)
        return TRUE;
    else
        return FALSE;
}

/* FUNCTION NAME: STKTPLG_OM_RemoveExpModule
 * PURPOSE: This function is used to remove slot-in module.
 * INPUT:   unit            -- unit id.
 *          module_index    -- module index.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_RemoveExpModule(UI32_T unit_id)
{
    UI32_T i;
    UI8_T   board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    printf("Remove Module => Unit: %lu.\r\n", unit_id);
    if(unit_id <1 || unit_id >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

    board_id = stk_unit_cfg[(UI8_T)unit_id-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);
    }
    /*remark on 12.19 */
    SYSFUN_TakeWriteSem(sem_id_static);
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT; i++)
    {
        old_exp_module_type[i] = stk_unit_cfg[unit_id-1].exp_module_type[i];
        stk_unit_cfg[unit_id-1].exp_module_presented[i] = 0;
        stk_unit_cfg[unit_id-1].exp_module_type[i]     = 0xff;

    }
    /*move on 1229 */
    stk_unit_cfg[(UI8_T)unit_id-1].nport=board_info_p->max_port_number_on_board;
    /* printf(" ***unit=%d RemoveExpModule nport=%d  module presented=%d\n",unit_id,stk_unit_cfg[(UI8_T)unit_id-1].nport,stk_unit_cfg[(UI8_T)unit_id-1].exp_module_presented[0]); */

    ctrl_info.exp_module_state_dirty[unit_id-1] = TRUE;
    SYSFUN_GiveWriteSem(sem_id_static);
    return TRUE;
} /* End of STKTPLG_OM_RemoveExpModule */

/* FUNCTION NAME: STKTPLG_OM_UpdateModuleStateChanged
 * PURPOSE: This API is used to get and put dirty bit as FALSE.
 *          i.e. clear after read.
 * INPUT:   status_dirty            -- a array of dirty table.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success and any dirty.
 *          FALSE -- 1) failure due to system not stable.
 *                   2) Not dirty;
 * NOTES:   None.
 */
BOOL_T STKTPLG_OM_UpdateModuleStateChanged(BOOL_T status_dirty[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK])
{
    UI32_T unit_id;
    BOOL_T ret;

    STKTPLG_OM_CHECK_STATE();

    ret = FALSE;
    SYSFUN_TakeWriteSem(sem_id_static);
    for(unit_id=1; unit_id<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
    {
        if (TRUE == ctrl_info.exp_module_state_dirty[unit_id-1])
        {
            status_dirty[unit_id-1] = TRUE;
            ctrl_info.exp_module_state_dirty[unit_id-1] = FALSE; /*clear after read*/

            ret = TRUE;
        }
        else
        {
            status_dirty[unit_id-1] = FALSE;
        }
    }
    SYSFUN_GiveWriteSem(sem_id_static);

    return ret;
}

void STKTPLG_OM_ShowCFG(void)
{
    UI32_T i;

    for (i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
        printf("nport[%lu]=%d \r\n",i,stk_unit_cfg[i].nport);
    }
}

/* FUNCTION NAME: STKTPLG_OM_SyncModuleProcess
 * PURPOSE: To sync the module status.
 * INPUT:   unit_id --- Module of which unit.
 * OUTPUT:  *is_insertion   -- insertion or removal.
 *          *starting_port  -- starting port ID of changed module.
 *          *number_of_port -- port number of changed module.
 *          *use_default    -- use default setting if insertion. I
 *                             Ignore this argument if removal.
 * RETUEN:  TRUE  -- Need to process.
 *          FALSE -- Not necessary to process.
 * NOTES:   None.
 */
BOOL_T STKTPLG_OM_SyncModuleProcess(UI32_T  unit_id,
                                     BOOL_T *is_insertion,
                                     UI32_T *starting_port,
                                     UI32_T *number_of_port,
                                     BOOL_T *use_default)
{
    BOOL_T ret;

    /* for upper CSCs
     */
    if(NULL == is_insertion || NULL == starting_port ||NULL == number_of_port ||NULL == use_default )
    return FALSE;

    if(unit_id <1 || unit_id >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;


    if (ctrl_info.synced_module_types[unit_id-1] == 0/*not-present*/)
    {
        if (stk_unit_cfg[unit_id-1].exp_module_type[0] == 0xff /*not-present*/)
        {
            /* I. not-present -> present -> not-present: Tell STKCTRL nothing to do.
             */
            ret = FALSE;
        }
        else
        {
            /* II. not-present -> present: Insertion.
             */
            *is_insertion = TRUE;
            *use_default = (ctrl_info.expected_module_types[unit_id-1] == stk_unit_cfg[unit_id-1].exp_module_type[0]) ? FALSE : TRUE;

            /* change the module type for upper layer to use
             */
            ctrl_info.synced_module_types[unit_id-1] = stk_unit_cfg[unit_id-1].exp_module_type[0];

            STKTPLG_OM_GetUnitModulePorts(unit_id, starting_port, number_of_port);

            /* 1. Expected module type of next insertion is synced module type.
             * 2. Here is the only condition to modify expected module type by processing
             *    module status.
             */
            ctrl_info.expected_module_types[unit_id-1] = ctrl_info.synced_module_types[unit_id-1];

            ret = TRUE;
        }
    }
    else
    {
        if (stk_unit_cfg[unit_id-1].exp_module_type[0] == 0xff /*not-present*/)
        {
            /* III. present -> not-present: Removal.
             */
            *is_insertion = FALSE;

            STKTPLG_OM_GetUnitModulePorts(unit_id, starting_port, number_of_port);

            /* change the module type to not-present for upper layer to use
             */
            ctrl_info.synced_module_types[unit_id-1] = 0;

            ret = TRUE;
        }
        else
        {
            /* IV. present -> not-present -> present: 1) Removal + 2) Insertion.
             */
            *is_insertion = FALSE;

            if (FALSE == STKTPLG_OM_GetUnitModulePorts(unit_id, starting_port, number_of_port))
            {
                ret = FALSE;
            }
            else
            {
                /* change the module type to not-present for upper layer to use
                 */
                ctrl_info.synced_module_types[unit_id-1] = 0;

                /* force to be dirty to force STKTPLG to callback to STKCTRL
                 */
                ctrl_info.exp_module_state_dirty[unit_id-1] = TRUE;

                ret = TRUE;
            }
        }
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetHiGiPortNum
 *---------------------------------------------------------------------------
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTE:
 *----------------------------------------------------------------
 */
void STKTPLG_OM_GetHiGiPortNum(UI32_T *port_num)
{
    UI8_T board_id;

    board_id = stk_unit_cfg[ctrl_info.my_unit_id-1].board_info.board_id;

    if (board_id ==1)
    {
#if defined (ES4626H)|| defined (ECN430_FB2)
        port_num[0]=26; port_num[1]=27;
#else
        port_num[0]=2; port_num[1]=3; port_num[2]=8; port_num[3]=1;
#endif
    }
    else
    {
#ifdef EIF8X10G
        port_num[0]=1; port_num[1]=2; port_num[2]=3; port_num[3]=4;
        port_num[4]=5; port_num[5]=6; port_num[6]=7; port_num[7]=8;
#else
        #if defined (ES4626H)|| defined (ECN430_FB2)
        port_num[0]=26; port_num[1]=27;
        #else
        port_num[0]=8; port_num[1]=1; port_num[2]=2; port_num[3]=3;
        #endif
#endif
    }

}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_ExtUnitToUnit
 *---------------------------------------------------------------------------
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTE:
 *----------------------------------------------------------------
 */
void STKTPLG_OM_ExtUnitToUnit( UI32_T ext_unit_id,UI32_T *unit)
{
    UI32_T  module=1;

    while((ext_unit_id-module*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)>SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        module++;
    }
    *unit=ext_unit_id-module*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_UnitModuleToExtUnit
 *---------------------------------------------------------------------------
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTE:
 *----------------------------------------------------------------
 */
void STKTPLG_OM_UnitModuleToExtUnit( UI32_T unit,UI32_T module_x,UI32_T *ext_unit_id)
{

    *ext_unit_id= module_x*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK + unit ;
}

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_IsLocalUnit
 *---------------------------------------------------------------------------
 * PURPOSE:  to  the port of  the specify unit  is belong to local or remote unit
 * INPUT:    unit :  destination unit
 *           port :
 * OUTPUT:   ext_unit_id  :  unit id
 * RETURN:   TRUE   : is local
 *           FALSE   : is remote
 * NOTE:     this function is for (unit,port)
 *           x*n+y :
 *           its meaning the option module x  of unit #y
 *           n: SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK
 *           when x=0 , y  is mainborad

 *
 *----------------------------------------------------------------
 */
BOOL_T   STKTPLG_OM_IsLocalUnit(UI32_T unit,UI32_T port,UI32_T *ext_unit_id)
{
    UI32_T  my_unit_id;
    UI32_T  module_x,max_port_number;

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

    STKTPLG_OM_GetMyUnitID(&my_unit_id);
    STKTPLG_OM_GetMaxPortNumberOnBoard(unit, &max_port_number);

    if(port>max_port_number)
    {
        /*
         * if((port %SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD)!=0)
         *module_x=port/SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD;
         */
         module_x=1;
         *ext_unit_id= module_x*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK + unit ;
         return(FALSE );

    }
    else if(unit!=my_unit_id)
    {
        *ext_unit_id=unit;
        return( FALSE );
    }
    else
    {
        *ext_unit_id=unit;
        return( TRUE );
    }
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_OptionModuleIsExist
 *---------------------------------------------------------------------------
 * PURPOSE:  to  check the unit  insered the option module  or not
 * INPUT:    unit :  unit
 * OUTPUT:   ext_unit_id :option module  unit id
 * RETURN:   TRUE   : exist
 *           FALSE  : non-exist
 * NOTE:     1. this function is for a specify unit only  or  for each unit existing in the stacking
 *              it should check the unit insered the option module  or not
 *              if it has option module, the isc_remote_call or isc_send  also need send to the option module
 *          example:
 *                   isc_remote_call(unit, );
 *                   if(STKTPLG_OM_OptionModule_Exist(unit,&ext_unit_id)
 *                   {
 *                       isc_remote_call(unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK,);
 *                   }
 *           2. now is only one option module
 *           STKTPLG_OM_OptionModuleIsExist(UI32_T unit, UI32_T  option_nums) int the feature
 *------------------------------------------------------------------------------------------
 */
BOOL_T   STKTPLG_OM_OptionModuleIsExist(UI32_T unit, UI32_T *ext_unit_id)
{
    /* all upper layer should follow STKCTRL/STKTPLG synced database.
     */
    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;
    if((ctrl_info.state == STKTPLG_STATE_MASTER && ctrl_info.synced_module_types[unit-1] != 0) ||
       STKTPLG_OM_ExpModuleIsInsert(unit)==TRUE )
    {
        STKTPLG_OM_UnitModuleToExtUnit( unit,1,ext_unit_id);
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}


/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetUnitModulePorts
 *---------------------------------------------------------------------------
 * PURPOSE:  To get ports in module.
 * INPUT:    Unit -- Which unit to get.
 * OUTPUT:   *start_port_id  -- starting port.
 *           *nbr_of_ports   -- port number.
 * RETURN:   TRUE  --- Unit or Module not present.
 *           FALSE --- Got information.
 * NOTE:     Base on current synced module database.
 *------------------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetUnitModulePorts(UI32_T unit_id, UI32_T *start_port_id, UI32_T *nbr_of_ports)
{
    UI32_T  module;
    UI8_T   board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    BOOL_T  ret;

    if (FALSE == STKTPLG_OM_UnitExist(unit_id))
    {
        return FALSE;
    }

    if(ctrl_info.synced_module_types[unit_id-1] == 0)
    {
        return FALSE;
    }

    board_id = stk_unit_cfg[(UI8_T)unit_id-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    *start_port_id = board_info_p->max_port_number_on_board + 1;

    ret = FALSE;
    SYSFUN_TakeReadSem(sem_id_static);
    for (module = 0; module<STKTPLG_OM_NBR_OF_MODULE_TYPES; module++)
    {
        if (stktplg_om_exp_module_port_number_table[module].module_type == ctrl_info.synced_module_types[unit_id-1])
        {
            *nbr_of_ports = stktplg_om_exp_module_port_number_table[module].nbr_of_ports;
            ret = TRUE;
            break;
        }
    }
    SYSFUN_GiveReadSem(sem_id_static);

    if (FALSE == ret)
    {
        printf("[TPLG] Wrong module type in database\r\n");
    }
    return ret;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetUnitMainboardPorts
 *---------------------------------------------------------------------------
 * PURPOSE:  To get ports in mainboard.
 * INPUT:    Unit -- Which unit to get.
 * OUTPUT:   *start_port_id  -- starting port.
 *           *nbr_of_ports   -- port number.
 * RETURN:   TRUE  --- Unit not present.
 *           FALSE --- Got information.
 * NOTE:     Base on current synced module database.
 *------------------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetUnitMainboardPorts(UI32_T unit, UI32_T *start_port_id, UI32_T *nbr_of_ports)
{
    UI8_T   board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    if (FALSE == STKTPLG_OM_UnitExist(unit))
    {
        return FALSE;
    }

    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    board_id = stk_unit_cfg_snapshot[(UI8_T)unit-1].board_info.board_id;
    #else
    board_id = stk_unit_cfg[(UI8_T)unit-1].board_info.board_id;
    #endif

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    *start_port_id = 1;
    *nbr_of_ports = board_info_p->max_port_number_on_board;

    return TRUE;
}

/* FUNCTION NAME : STKTPLG_OM_IsOptionModule
 * PURPOSE  : STKTPLG to check if I am a module.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : TRUE/FALSE
 */
BOOL_T STKTPLG_OM_IsOptionModule(void)
{
    return FALSE;
}

/* FUNCTION NAME : STKTPLG_OM_GetStackStatus
 * PURPOSE  : STKTPLG to notify tplg state for image version validation.
 * INPUT    : None.
 * OUTPUT     is_stack_status_normal ---
 *              TRUE: Normal
 *              FALSE: Abnormal
 * RETUEN   : TRUE/FALSE
 * Notes    : 1) By spec, set abnormal state if version of "main boards" are different.
 *               else set as normal state.
 *            2) Base on 1), call this API only when topology is firmed.
 */
BOOL_T STKTPLG_OM_GetStackStatus(BOOL_T *is_stack_status_normal)
{
    UI32_T unit;
    UI8_T  first_unit_runtime_sw_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
    BOOL_T first_unit;
    static UI32_T counter=0;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_CHECK_STATE();

    switch(ctrl_info_snapshot.state)
    {
        case STKTPLG_STATE_MASTER:
            *is_stack_status_normal = TRUE;
            first_unit = TRUE;
            unit = 0;
            while(STKTPLG_OM_GetNextUnit(&unit))
            {
                if (TRUE == first_unit)
                {
                    SYSFUN_TakeReadSem(sem_id_static);
                    memcpy(first_unit_runtime_sw_ver,
                           stk_unit_cfg_snapshot[unit - 1].board_info.runtime_sw_ver,
                           SYS_ADPT_FW_VER_STR_LEN + 1);
                    SYSFUN_GiveReadSem(sem_id_static);
                    first_unit = FALSE;
                }
                else
                {
                    SYSFUN_TakeReadSem(sem_id_static);
                    if (0 != strncmp((char*)(first_unit_runtime_sw_ver),
                                    (char*)(stk_unit_cfg_snapshot[unit - 1].board_info.runtime_sw_ver),
                                    SYS_ADPT_FW_VER_STR_LEN + 1))
                    {
                        /* main board versions are different
                         */
                        if(counter %20==0)
                          printf("\r\n !!!! Warning:Unit %ld version is different with master.Please update !!!\r\n",unit);

                        //SYSFUN_LogDebugMsg("\r\n !!!! Warning:A slave version is different with master.Please update !!!\r\n");
                        *is_stack_status_normal = FALSE;
                        SYSFUN_GiveReadSem(sem_id_static);
                        break;
                    }
                    else
                    {
                        SYSFUN_GiveReadSem(sem_id_static);
                    }
                }
            }
            break;
        case STKTPLG_STATE_SLAVE:
            SYSFUN_TakeReadSem(sem_id_static);
            if (0 == strncmp((char*)(stk_unit_cfg_snapshot[ctrl_info_snapshot.my_unit_id-1].board_info.runtime_sw_ver),
                            (char*)(stk_unit_cfg_snapshot[ctrl_info_snapshot.master_unit_id-1].board_info.runtime_sw_ver),
                            SYS_ADPT_FW_VER_STR_LEN + 1))
            {
                *is_stack_status_normal = TRUE;
            }
            else
            {
                if(counter %20==0)
                   printf("\r\n !!!! Warning:Slave version is different with master.Please update !!!\r\n");

                 //SYSFUN_LogDebugMsg("\r\n !!!! Warning:A slave version is different with master.Please update !!!\r\n");
                *is_stack_status_normal = FALSE;
            }
            SYSFUN_GiveReadSem(sem_id_static);
            break;
        case STKTPLG_STATE_STANDALONE:
            *is_stack_status_normal = TRUE;
            break;
        default:
            *is_stack_status_normal = FALSE;
            break;
    }
#else
    STKTPLG_OM_CHECK_STATE();

    switch(ctrl_info.state)
    {
        case STKTPLG_STATE_MASTER:
            *is_stack_status_normal = TRUE;
            first_unit = TRUE;
            unit = 0;
            while(STKTPLG_OM_GetNextUnit(&unit))
            {
                if (TRUE == first_unit)
                {
                    SYSFUN_TakeReadSem(sem_id_static);
                    memcpy(first_unit_runtime_sw_ver,
                           stk_unit_cfg[unit - 1].board_info.runtime_sw_ver,
                           SYS_ADPT_FW_VER_STR_LEN + 1);
                    SYSFUN_GiveReadSem(sem_id_static);
                    first_unit = FALSE;
                }
                else
                {
                    SYSFUN_TakeReadSem(sem_id_static);
                    if (0 != strncmp((char*)(first_unit_runtime_sw_ver),
                                    (char*)(stk_unit_cfg[unit - 1].board_info.runtime_sw_ver),
                                    SYS_ADPT_FW_VER_STR_LEN + 1))
                    {
                        /* main board versions are different
                         */
                        *is_stack_status_normal = FALSE;
                        SYSFUN_GiveReadSem(sem_id_static);
                        break;
                    }
                    else
                    {
                        SYSFUN_GiveReadSem(sem_id_static);
                    }
                }
            }
            break;
        case STKTPLG_STATE_SLAVE:
            SYSFUN_TakeReadSem(sem_id_static);
            if (0 == strncmp((char*)(stk_unit_cfg[ctrl_info.my_unit_id-1].board_info.runtime_sw_ver),
                            (char*)(stk_unit_cfg[ctrl_info.master_unit_id-1].board_info.runtime_sw_ver),
                            SYS_ADPT_FW_VER_STR_LEN + 1))
            {
                *is_stack_status_normal = TRUE;
            }
            else
            {
                *is_stack_status_normal = FALSE;
            }
            SYSFUN_GiveReadSem(sem_id_static);
            break;
        case STKTPLG_STATE_STANDALONE:
            *is_stack_status_normal = TRUE;
            break;
        default:
            *is_stack_status_normal = FALSE;
            break;
    }
#endif
    return TRUE;
}


/* FUNCTION NAME : STKTPLG_OM_GetUnitStatus
 * PURPOSE  : STKTPLG to notify tplg state for image version validation.
 * INPUT    : None.
 * OUTPUT     is_stack_status_normal ---
 *              TRUE: Normal
 *              FALSE: Abnormal
 * RETUEN   : TRUE/FALSE
 * Notes    : 1) By spec, set abnormal state if version of "main boards" are different.
 *               else set as normal state.
 *            2) Base on 1), call this API only when topology is firmed.
 */
BOOL_T STKTPLG_OM_GetUnitStatus(UI32_T unit, BOOL_T *is_unit_status_normal)
{
    UI32_T master_unit_id = ctrl_info.master_unit_id;


    STKTPLG_OM_CHECK_STATE();

    if (FALSE == STKTPLG_OM_UnitExist(unit))
    {
        return FALSE;
    }

    /* main bioard
     */
    SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    if (0 == strncmp((char*)(stk_unit_cfg[unit-1].board_info.runtime_sw_ver),
                    (char*)(stk_unit_cfg[master_unit_id-1].board_info.runtime_sw_ver),
                    SYS_ADPT_FW_VER_STR_LEN + 1))
#else
    if (0 == strncmp((char*)(stk_unit_cfg_snapshot[unit-1].board_info.runtime_sw_ver),
                    (char*)(stk_unit_cfg_snapshot[master_unit_id-1].board_info.runtime_sw_ver),
                    SYS_ADPT_FW_VER_STR_LEN + 1))
#endif
    {
        *is_unit_status_normal = TRUE;
    }
    else
    {
        *is_unit_status_normal = FALSE;
    }
    SYSFUN_GiveReadSem(sem_id_static);
    return TRUE;
}

/* FUNCTION NAME : STKTPLG_OM_GetModuleStatus
 * PURPOSE  : STKTPLG to notify tplg state for image version validation.
 * INPUT    : None.
 * OUTPUT     is_module_status_normal ---
 *              TRUE: Normal
 *              FALSE: Abnormal
 * RETUEN   : TRUE/FALSE
 * Notes    : 1) By spec, set abnormal state if version of "main boards" are different.
 *               else set as normal state.
 *            2) Base on 1), call this API only when topology is firmed.
 */
BOOL_T STKTPLG_OM_GetModuleStatus(UI32_T unit, UI32_T module, BOOL_T *is_module_status_normal)
{
    UI32_T master_unit_id = ctrl_info.master_unit_id;


    STKTPLG_OM_CHECK_STATE();

    if (FALSE == STKTPLG_OM_UnitExist(unit))
    {
        return FALSE;
    }

    SYSFUN_TakeReadSem(sem_id_static);
    if(ctrl_info.synced_module_types[unit-1] == 0)
    {
        SYSFUN_GiveReadSem(sem_id_static);
        return FALSE;
    }

    if (module!=1)
    {
        SYSFUN_GiveReadSem(sem_id_static);
        return FALSE;
    }

      if (0 == strncmp((char*)(stk_module_cfg[unit - 1][0].op_code_ver),
                    (char*)(stk_unit_cfg[master_unit_id-1].module_expected_runtime_fw_ver),
                    SYS_ADPT_FW_VER_STR_LEN + 1))
    {
        *is_module_status_normal = TRUE;
    }
    else
    {
        *is_module_status_normal = FALSE;
    }
    SYSFUN_GiveReadSem(sem_id_static);
    return TRUE;
}

/* FUNCTION NAME : STKTPLG_OM_GetModuleType
 * PURPOSE  : STKTPLG to get module type.
 * INPUT    : unit -- Which unit.
 *            module -- which module.
 * OUTPUT     module_type -- Type of module.
 * RETUEN   : TRUE/FALSE
 * Notes    : None.
 */
BOOL_T STKTPLG_OM_GetModuleType(UI32_T unit, UI32_T module, UI32_T *module_type)
{
    STKTPLG_OM_CHECK_STATE();

    if (FALSE == STKTPLG_OM_UnitExist(unit))
    {
        return FALSE;
    }

    if((*module_type=ctrl_info.synced_module_types[unit-1]) == 0)
    {
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : STKTPLG_OM_IsValidDriverUnit
 * PURPOSE  : To know the driver is valid or not.
 * INPUT    : driver_unit -- which driver want to know.
 * OUTPUT   : None
 * RETUEN   : TRUE  -- Version is valid.
 *            FALSE -- For main board: Version is different from master main board.
 *                     For module: Version is dofferent from expected module version.
 */
BOOL_T STKTPLG_OM_IsValidDriverUnit(UI32_T driver_unit)
{
    UI32_T unit_id;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    UI32_T master_unit_id = ctrl_info_snapshot.master_unit_id;
#else
    UI32_T master_unit_id = ctrl_info.master_unit_id;
#endif

    if (FALSE == STKTPLG_OM_DriverUnitExist(driver_unit))
    {
        return FALSE;
    }

    if (driver_unit >= 1 &&
        driver_unit <=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        unit_id = driver_unit;

        /* main bioard
         */
        SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        if (0 == strncmp((char*)(stk_unit_cfg_snapshot[unit_id-1].board_info.runtime_sw_ver),
                        (char*)(stk_unit_cfg_snapshot[master_unit_id-1].board_info.runtime_sw_ver),
                        SYS_ADPT_FW_VER_STR_LEN + 1))
#else
        if (0 == strncmp((char*)(stk_unit_cfg[unit_id-1].board_info.runtime_sw_ver),
                        (char*)(stk_unit_cfg[master_unit_id-1].board_info.runtime_sw_ver),
                        SYS_ADPT_FW_VER_STR_LEN + 1))
#endif
        {
            SYSFUN_GiveReadSem(sem_id_static);
            return TRUE;
        }
        else
        {
            SYSFUN_GiveReadSem(sem_id_static);
            return FALSE;
        }
    }

    if (driver_unit >= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK+1 &&
        driver_unit <= SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK)
    {
        /* module
         */
        unit_id = driver_unit-SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
        SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        if (0 == strncmp((char*)(stk_module_cfg_snapshot[unit_id - 1][0].op_code_ver),
                        (char*)(stk_unit_cfg_snapshot[master_unit_id-1].module_expected_runtime_fw_ver),
                        SYS_ADPT_FW_VER_STR_LEN + 1))
#else
        if (0 == strncmp((char*)(stk_module_cfg[unit_id - 1][0].op_code_ver),
                        (char*)(stk_unit_cfg[master_unit_id-1].module_expected_runtime_fw_ver),
                        SYS_ADPT_FW_VER_STR_LEN + 1))
#endif
        {
            SYSFUN_GiveReadSem(sem_id_static);
            return TRUE;
        }
        else
        {
            SYSFUN_GiveReadSem(sem_id_static);
            return FALSE;
        }
    }

    return FALSE;
}


/* FUNCTION NAME : STKTPLG_OM_GetMyDriverUnit
 * PURPOSE  : To get my driver unit ID.
 * INPUT    : None.
 * OUTPUT   : *my_driver_unit -- my driver ID.
 * RETUEN   : TRUE  -- exist.
 *            FALSE -- not exist.
 */
BOOL_T STKTPLG_OM_GetMyDriverUnit(UI32_T *my_driver_unit)
{
    UI32_T my_unit_id;

    if (FALSE == STKTPLG_OM_GetMyUnitID(&my_unit_id))
    {
        return FALSE;
    }

    if (FALSE == STKTPLG_OM_IsOptionModule())
    {
        /* I'm main board
         */
        *my_driver_unit = my_unit_id;
    }
    else
    {
        /* I'm option module
         */
        *my_driver_unit = my_unit_id + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
    }
    return TRUE;
}


/* FUNCTION NAME : STKTPLG_OM_DriverUnitExist
 * PURPOSE  : To know the driver exist or not.
 * INPUT    : driver_unit -- which driver want to know.
 * OUTPUT   : None
 * RETUEN   : TRUE  -- exist.
 *            FALSE -- not exist.
 */
BOOL_T STKTPLG_OM_DriverUnitExist(UI32_T driver_unit)
{
    UI32_T unit_id;

    if (driver_unit == 0 ||
        driver_unit > SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK)
    {
        return FALSE;
    }

    /* search main board
     */
    if (driver_unit >= 1 && driver_unit <=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
       /* EPR:ES3628BT-FLF-ZZ-00742
        Problem: stack:the master unit hung when remve from the stack.
        Rootcause:when the hot removal happend,and other csc is sending pkt to slaves which have been removed
                  and the csc is waiting for the slave reply.so the csc will never receive the slave reply.and it is still in the suspend state
        Solution:when the slave removed update isc_om database ,and wakeup the csc which is waiting for the slave reply which is removed
        Files:ISC_OM.C,stktplg_engine.c,stktplg_om.c,stktplg_om.h makefile.am,l_stack.c,l_stack.h,isc_init.c
*/
        if (TRUE == STKTPLG_OM_ENG_UnitExist(driver_unit))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    /* try to search module
     */
    unit_id = driver_unit-SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;

    if(stk_unit_cfg[unit_id-1].exp_module_presented[0] == 1)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}


/* FUNCTION NAME : STKTPLG_OM_GetNextDriverUnit
 * PURPOSE  : To get next driver unit.
 * INPUT    : Next to which driver unit.
 * OUTPUT   : Next driver is which.
 * RETUEN   : TRUE/FALSE
 */
BOOL_T STKTPLG_OM_GetNextDriverUnit(UI32_T *driver_unit)
{


    /* search main board
     */
    if (*driver_unit >= 0 && *driver_unit <SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        if (TRUE == STKTPLG_OM_GetNextUnit(driver_unit))
        {
            return TRUE;
        }
    }

    /* try to search module
     */
    if (*driver_unit < SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK)
    {
        UI32_T unit_id;
        BOOL_T got_it;

        if (*driver_unit <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        {
            unit_id = 1; /*first unit*/
        }
        else
        {
            unit_id = *driver_unit + 1 - SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
        }

        got_it = FALSE;
        SYSFUN_TakeReadSem(sem_id_static);
        for( ; unit_id<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit_id++)
        {
       #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
          if(stk_unit_cfg_snapshot[unit_id-1].exp_module_presented[0] == 1)
       #else
          if(stk_unit_cfg[unit_id-1].exp_module_presented[0] == 1)
       #endif
            {
                /* present
                 */
                *driver_unit = unit_id + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
                got_it = TRUE;
                break;
            }
        }
        SYSFUN_GiveReadSem(sem_id_static);
        return got_it;
    }

    return FALSE;
}

/* FUNCTION NAME : STKTPLG_OM_UpdateTplgSyncBmp
 * PURPOSE  : To update sync bitmap in control info
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : TRUE/FALSE
 */
void STKTPLG_OM_UpdateTplgSyncBmp(UI8_T sync_bmp)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_snapshot.stk_unit_cfg_dirty_sync_bmp = sync_bmp;
#else
    ctrl_info.stk_unit_cfg_dirty_sync_bmp = sync_bmp;
#endif
}

/* FUNCTION NAME : STKTPLG_OM_SetTplgSyncBmp
 * PURPOSE  : To set all other units need to sync to.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : TRUE/FALSE
 */
BOOL_T STKTPLG_OM_SetTplgSyncBmp(void)
{
    UI32_T my_unit_id;
    UI32_T unit_id;

    if (FALSE == STKTPLG_OM_GetMyUnitID(&my_unit_id))
    {
        return FALSE;
    }

    unit_id = 0;
    while(STKTPLG_OM_GetNextUnit(&unit_id))
    {
        if (my_unit_id != unit_id)
        {
 #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
             ctrl_info_snapshot.stk_unit_cfg_dirty_sync_bmp |= (1<< (unit_id-1));
 #else
            ctrl_info.stk_unit_cfg_dirty_sync_bmp |= (1<< (unit_id-1));
#endif
        }
    }
    return TRUE;
}

/* FUNCTION NAME : STKTPLG_OM_GetTplgSyncBmp
 * PURPOSE  : To get databse sync bitmap
 * INPUT    : None
 * OUTPUT   : None.
 * RETUEN   : The bitmap.
 */
UI8_T STKTPLG_OM_GetTplgSyncBmp(void)
{
  #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    return ctrl_info_snapshot.stk_unit_cfg_dirty_sync_bmp;
  #else
    return ctrl_info.stk_unit_cfg_dirty_sync_bmp;
  #endif
}


/* FUNCTION NAME : STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard
 * PURPOSE: To get the maximum port number on board .
 * INPUT:   unit             -- unit id.
 * OUTPUT:  *max_port_number -- maximum port number on board.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   0           -- get local device.
 *          otherwise   -- get this device.
 */
BOOL_T STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(UI8_T unit, UI32_T *max_option_port_number)
{
    UI8_T    board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    board_id = stk_unit_cfg_snapshot[unit-1].board_info.board_id;
#else
    board_id = stk_unit_cfg[unit-1].board_info.board_id;
#endif

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    *max_option_port_number = board_info_p->max_port_number_of_this_unit-board_info_p->max_port_number_on_board;

    return (TRUE);
}


/* FUNCTION NAME : STKTPLG_OM_InsertBoardID
 * PURPOSE: Insert BoardID to OM
 * INPUT:   unit             -- unit id.
 *          board_id
 * OUTPUT:  NONE
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
BOOL_T STKTPLG_OM_InsertBoradID(UI8_T unit,UI8_T board_id)
{
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    stk_unit_cfg[(UI8_T)unit-1].board_info.board_id = board_id;
    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
      SYSFUN_Debug_Printf("STKTPLG_OM_InsertBoradID unit=%d board_id=%d \r\n",unit,board_id);
      assert(0);
    }
    memcpy(&(units_board_info_p[(UI8_T)unit-1]),board_info_p,sizeof(STKTPLG_BOARD_BoardInfo_T));
    STKTPLG_SHOM_SetUnitBoardInfo(unit,&(units_board_info_p[(UI8_T)unit-1]));
    return TRUE;
}


/* vivid written , becasuse mainborad did not know the hi-g port infomation
 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetModuleHiGiPortNum
 *---------------------------------------------------------------------------
 * PURPOSE:  To provide dev_nic HiGi port number for different modules
 * INPUT:    IUC_unit_id        ---IUC_unit_id
 * OUTPUT:   port_num           ---HiGi port number
 * RETURN:   None
 * NOTE:     Hard coded to retrieve module type for the first module
 *           in the array. Also assuming that every mainboard only
 *           have one module only.
 *----------------------------------------------------------------
 */
UI8_T STKTPLG_OM_GetModuleHiGiPortNum(UI16_T IUC_unit_id)
{
    UI8_T module_type;
    UI16_T unit_id;
    UI8_T port_num;

    port_num=0;
    unit_id = IUC_unit_id - SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;


    /* An array of exp_module_type size < SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT
     * Get exp_module_type from first module in the array.
     */
 #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    module_type = stk_unit_cfg_snapshot[(UI8_T)unit_id-1].exp_module_type[0];
 #else
    module_type = stk_unit_cfg[(UI8_T)unit_id-1].exp_module_type[0];
 #endif

    /* assign different HiGi port to different module as HiGi port number
     * is chip dependent
     */


    switch(module_type)
    {
        case VAL_swModuleType_tenGigaPortModule:
             port_num = 2;
             break;

        case VAL_swModuleType_eightPortSfpModule:
             port_num = 13;
             break;
    }

    return port_num;
}

/* FUNCTION NAME: STKTPLG_OM_SfpIndexToUserPort
 * PURPOSE: This function is translate from SFP index to user port.
 * INPUT:   unit      -- which unit.
 *          sfp_index -- which SFP.
 * OUTPUT:  port -- which user port
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T STKTPLG_OM_SfpIndexToUserPort(UI32_T unit, UI32_T sfp_index, UI32_T *port)
{
    STKTPLG_BOARD_BoardInfo_T local_board_info;
    STKTPLG_BOARD_BoardInfo_T *local_board_info_p = &local_board_info;

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;
 #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    if (!STKTPLG_BOARD_GetBoardInformation(stk_unit_cfg_snapshot[unit-1].board_info.board_id, local_board_info_p))
 #else
    if (!STKTPLG_BOARD_GetBoardInformation(stk_unit_cfg[unit-1].board_info.board_id, local_board_info_p))
 #endif
    {
        return FALSE;
    }

    if (0 == local_board_info_p->sfp_port_number)
    {
        return FALSE;
    }

    if (0 == sfp_index || sfp_index > local_board_info_p->sfp_port_number)
    {
        return FALSE;
    }

    *port = local_board_info_p->start_sfp_port_number + sfp_index - 1;

    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_UserPortToSfpIndexAndType
 * PURPOSE: This function is translate from user port to SFP index.
 *          and return sfp_type
 * INPUT:   unit -- which unit.
 *          port -- which user port
 * OUTPUT:  sfp_index -- which SFP.
 *          sfp_type_p -- which SFP type.
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T STKTPLG_OM_UserPortToSfpIndexAndType(UI32_T unit, UI32_T port, UI32_T *sfp_index, UI8_T *sfp_type_p, BOOL_T *is_break_out)
{
    return STKTPLG_OM_UserPortToSfpIndexInternal(unit, port, sfp_index, sfp_type_p, is_break_out);
}

/* FUNCTION NAME: STKTPLG_OM_UserPortToSfpIndex
 * PURPOSE: This function is translate from user port to SFP index.
 * INPUT:   unit -- which unit.
 *          port -- which user port
 * OUTPUT:  sfp_index -- which SFP.
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T STKTPLG_OM_UserPortToSfpIndex(UI32_T unit, UI32_T port, UI32_T *sfp_index)
{
    UI8_T sfp_type = SWDRV_TYPE_GBIC_ID_SFP;
    BOOL_T is_break_out;
    return STKTPLG_OM_UserPortToSfpIndexInternal(unit, port, sfp_index, &sfp_type, &is_break_out);
}

/* FUNCTION NAME: STKTPLG_OM_UserPortToSfpIndexInternal
 * PURPOSE: This function is translate from user port to SFP index.
 *          and return sfp_type
 * INPUT:   unit -- which unit.
 *          port -- which user port
 * OUTPUT:  sfp_index -- which SFP.
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 */
static BOOL_T STKTPLG_OM_UserPortToSfpIndexInternal(UI32_T unit, UI32_T port, UI32_T *sfp_index, UI8_T *sfp_type_p, BOOL_T *is_break_out)
{
    STKTPLG_BOARD_BoardInfo_T local_board_info;
    STKTPLG_BOARD_BoardInfo_T *local_board_info_p = &local_board_info;
    UI8_T   port_type;

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;
 #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    if (!STKTPLG_BOARD_GetBoardInformation(stk_unit_cfg_snapshot[unit-1].board_info.board_id, local_board_info_p))
 #else
   if (!STKTPLG_BOARD_GetBoardInformation(stk_unit_cfg[unit-1].board_info.board_id, local_board_info_p))
 #endif
    {
        return FALSE;
    }

    if (0 == local_board_info_p->sfp_port_number)
    {
        return FALSE;
    }

    if (port == 0 || port > local_board_info_p->max_port_number_of_this_unit)
    {
        return FALSE;
    }

    if (port < local_board_info_p->start_sfp_port_number || port > local_board_info_p->start_sfp_port_number + local_board_info_p->sfp_port_number - 1)
    {
        return FALSE;
    }

    *sfp_index = port - local_board_info_p->start_sfp_port_number + 1;

    /* Need to consider if new port type is created */
    port_type = local_board_info_p->userPortMappingTable[0][port-1].port_type;
    if(port_type == VAL_portType_tenGBaseXFP)
        *sfp_type_p = SWDRV_TYPE_GBIC_ID_XFP;
    else if(port_type == VAL_portType_fortyGBaseQSFP)
        *sfp_type_p = SWDRV_TYPE_GBIC_ID_QSFP;
    else
        *sfp_type_p = SWDRV_TYPE_GBIC_ID_SFP;

    if (port >= local_board_info_p->break_out_port_start &&
        port < (local_board_info_p->break_out_port_start + local_board_info_p->qsfp_port_number*4))
    {
        *is_break_out = TRUE;
    }

    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_UpdateExpectedModuleType
 * PURPOSE: This function is used to update expected module type,
 *          that is used to make decision if module insertion, the
 *          provision shall be default or not.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE/FALSE
 * NOTES:   In This API, TPLG call CLI, it violate hierachy, but
 *          this is special case, that CLI is viewed as a storage t keep
 *          info of TPLG.
 */
void STKTPLG_OM_UpdateExpectedModuleType(void)
{


    if (FALSE == CLI_POM_GetConfiguartionModuleTypes(ctrl_info.expected_module_types))
    {
        SYSFUN_TakeWriteSem(sem_id_static);
        memset(ctrl_info.expected_module_types, 0, sizeof(ctrl_info.expected_module_types));
        SYSFUN_GiveWriteSem(sem_id_static);
        //printf("TPLG: No module types in config file.\r\n");
    }
}

/* FUNCTION NAME : STKTPLG_OM_GetLocalModulePortNumber
 * PURPOSE: To get the max port number of module conncted to me .
 * INPUT:   NONE.
 * OUTPUT:  portnum -- max port number of module
 * RETUEN:  FALSE : no module inserted
            TRUE  : otherwise
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetLocalModulePortNumber(UI32_T *portnum)
{
    UI8_T   module_type;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    module_type = ctrl_info_snapshot.expansion_module_type;
#else
    module_type = ctrl_info.expansion_module_type;
#endif

    return (STKTPLG_BOARD_GetPortNumberInformation(module_type, portnum));
}

/* FUNCTION NAME : STKTPLG_OM_IsModulePort
 * PURPOSE: This function check if a port is a module port
 * INPUT:   unit:
 *          port:
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- is a module port
 *          FALSE        -- otherwise
 * NOTES:
 *
 */
BOOL_T    STKTPLG_OM_IsModulePort(UI32_T unit, UI32_T port)
{
    UI8_T    board_id;
    STKTPLG_BOARD_BoardInfo_T board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    board_id = stk_unit_cfg_snapshot[unit-1].board_info.board_id;
#else
    board_id = stk_unit_cfg[unit-1].board_info.board_id;
#endif

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    if (port > board_info_p->max_port_number_on_board)
        return TRUE;
    else
        return FALSE;
}
void  STKTPLG_OM_GetMainBoardPortNum(UI32_T  *portnum)
{
}
void STKTPLG_OM_GetMyModuleID(UI8_T *mymodid,UI8_T *mymodtype)
{
}

#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
/* FUNCTION NAME : STKTPLG_OM_GetMasterButtonStatus
 * PURPOSE: This function is to get the master button status setting for a specified unit
 * INPUT:   unit_id
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- master button status is enabled
 *          FALSE        -- otherwise
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetMasterButtonStatus(UI32_T unit_id)
{
    int unit;


    if (FALSE == STKTPLG_OM_UnitExist(unit_id))
        return FALSE;

    SYSFUN_TakeReadSem(sem_id_static);

    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    /* sam_wu, 2006-08-18, EPR: ES4549-08-00594
     * Synchronize the snapshot database with stack topology database.
     */
    STKTPLG_OM_CopyDatabaseToSnapShot();
    #endif
    if (TRUE == ctrl_info.is_ring)
    {
        for(unit = 0; unit < ctrl_info.total_units_down; unit++)
        {
            if (ctrl_info.stable_hbt_down.payload[unit].unit_id == unit_id)
            {
                SYSFUN_GiveReadSem(sem_id_static);
                return (ctrl_info.stable_hbt_down.payload[unit].button_pressed);
            }
        }
    }
    else
    {
        for(unit = ctrl_info.total_units_up - 1; unit >= 0; unit--)
        {
            if (ctrl_info.stable_hbt_up.payload[unit].unit_id == unit_id)
            {
                SYSFUN_GiveReadSem(sem_id_static);
                return (ctrl_info.stable_hbt_up.payload[unit].button_pressed);
            }
        }
        for(unit = 1; unit < ctrl_info.total_units_down; unit++)
        {
            if (ctrl_info.stable_hbt_down.payload[unit].unit_id == unit_id)
            {
                SYSFUN_GiveReadSem(sem_id_static);
                return (ctrl_info.stable_hbt_down.payload[unit].button_pressed);
            }
        }
    }
    SYSFUN_GiveReadSem(sem_id_static);
    return FALSE;
}


/* FUNCTION NAME : STKTPLG_OM_SetMasterButtonStatus
 * PURPOSE: This function is to set the master button status setting into flash for a specified unit
 * INPUT:   unit_id
 *          status : master button status
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- master button status is set successfully
 *          FALSE        -- otherwise
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_SetMasterButtonStatus(UI32_T unit_id, BOOL_T status)
{
     BOOL_T ret;
     int unit;


    if (FALSE == STKTPLG_OM_UnitExist(unit_id))
        return FALSE;

    ret = SYSDRV_SetPushButtonStatus(unit_id, status);

    SYSFUN_TakeReadSem(sem_id_static);
    if (TRUE == ctrl_info.is_ring)
    {
        for(unit = 0; unit < ctrl_info.total_units_down; unit++)
        {
            if (ctrl_info.stable_hbt_down.payload[unit].unit_id == unit_id)
            {
                ctrl_info.stable_hbt_down.payload[unit].button_pressed = status;
                SYSFUN_GiveReadSem(sem_id_static);
                return ret;
            }
        }
    }
    else
    {
        for(unit = ctrl_info.total_units_up - 1; unit >= 0; unit--)
        {
            if (ctrl_info.stable_hbt_up.payload[unit].unit_id == unit_id)
            {
                ctrl_info.stable_hbt_up.payload[unit].button_pressed = status;
                SYSFUN_GiveReadSem(sem_id_static);
                return ret;
            }
        }
        for(unit = 1; unit < ctrl_info.total_units_down; unit++)
        {
            if (ctrl_info.stable_hbt_down.payload[unit].unit_id == unit_id)
            {
                ctrl_info.stable_hbt_down.payload[unit].button_pressed = status;
                SYSFUN_GiveReadSem(sem_id_static);
                return ret;
            }
        }
    }

    SYSFUN_GiveReadSem(sem_id_static);
    return ret;
}
#endif

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
/* FUNCTION NAME : STKTPLG_OM_GetStackingButtonStatus
 * PURPOSE: This function is to get the stacking button status setting
 * INPUT:   None.
 * OUTPUT:  isPressed_p : stacking button pressed status
 *          isActived_p : stacking button active status
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_OM_GetStackingButtonStatus(BOOL_T *isPressed_p, BOOL_T *isActived_p)
{
    *isActived_p = ctrl_info.stacking_button_state;
    if (SYSDRV_GetStackingButtonStatus(isPressed_p) == FALSE)
    {
        *isPressed_p = ctrl_info.stacking_button_state;
    }
}
#endif

#if (SYS_CPNT_POE==TRUE)
/* FUNCTION: STKTPLG_OM_IsPoeDevice
 * PURPOSE:  For POE_MGR to check if some unit is POE device or not.
 * INPUT:    unit_id -- The unit that caller want to check.
 * OUTPUT:   None.
 * RETURN:   TRUE  -- This unit is a POE device.
 *           FALSE -- This unit is not a POE device.
 * NOTES:    None.
 */
BOOL_T STKTPLG_OM_IsPoeDevice(UI32_T unit_id)
{
    UI8_T                     board_id;
    STKTPLG_BOARD_BoardInfo_T board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    if (!STKTPLG_OM_UnitExist(unit_id))
    {
        return (FALSE);
    }

    board_id = stk_unit_cfg[unit_id - 1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\n\rCan not get related board 's POE information.");
        assert(0);
        return (FALSE);
    }
    if (board_info_p->is_poe_device == TRUE)
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
} /* End of STKTPLG_OM_IsPoeDevice() */

/* FUNCTION: STKTPLG_OM_IsLocalPoeDevice
 * PURPOSE:  For POE_MGR to check if local unit is POE device or not.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   TRUE  -- Local unit is a POE device.
 *           FALSE -- Local unit is not a POE device.
 * NOTES:    None.
 */
BOOL_T STKTPLG_OM_IsLocalPoeDevice()
{
    STKTPLG_BOARD_BoardInfo_T board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    UC_MGR_Sys_Info_T         uc_sys_info;

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    if (!STKTPLG_BOARD_GetBoardInformation(uc_sys_info.board_id, board_info_p))
    {
        perror("\n\rCan not get related board 's POE information.");
        assert(0);
        return (FALSE);
    }
    if (board_info_p->is_poe_device == TRUE)
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}
#endif

#if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT > 1)
/* FUNCTION: STKTPLG_OM_IsSupportRPU
 * PURPOSE:  For SYS_MGR to check if some unit has Redundant Power Unit.
 * INPUT:    unit_id -- The unit that caller want to check.
 * OUTPUT:   None.
 * RETURN:   TRUE  -- This unit supports RPU.
 *           FALSE -- This unit does not support RPU.
 * NOTES:    None.
 */
BOOL_T STKTPLG_OM_IsSupportRPU(UI32_T unit_id)
{
    UI8_T                     board_id;
    STKTPLG_BOARD_BoardInfo_T board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    if (!STKTPLG_OM_UnitExist(unit_id))
    {
        return (FALSE);
    }

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    board_id = stk_unit_cfg_snapshot[unit_id - 1].board_info.board_id;
#else
    board_id = stk_unit_cfg[unit_id - 1].board_info.board_id;
#endif


    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        printf("%s: Can not get RPU information for unit %lu.\r\n", __FUNCTION__, unit_id);
        assert(0);
        return (FALSE);
    }
    if (board_info_p->is_support_rpu == TRUE)
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
} /* End of STKTPLG_OM_IsSupportRPU() */
#endif
#if (SYS_HWCFG_SUPPORT_PD == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_IsPoEPDPort
 * -------------------------------------------------------------------------
 * FUNCTION: To know whether the given port supports PoE PD
 * INPUT   : unit -- which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : If this port supports PoE PD, return code is TRUE. Or return code is FALSE.
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_IsPoEPDPort(UI32_T unit, UI32_T port)
{
    UI32_T board_id;

    if (!STKTPLG_OM_UnitExist(unit))
    {
        return FALSE;
    }
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    board_id = stk_unit_cfg_snapshot[unit-1].board_info.board_id;
#else
    board_id = stk_unit_cfg[unit-1].board_info.board_id;
#endif

    return STKTPLG_BOARD_IsPoEPDPort(board_id, port);
}
#endif /* #if (SYS_HWCFG_SUPPORT_PD == TRUE) */

/* FUNCTION NAME : STKTPLG_OM_PortList2DriverUnitList
 * PURPOSE: This function is to convert the port bitmap into driver unit bitmap
 * INPUT:   port_list: the port bitmap, size: SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
 * OUTPUT:  driver_unit_list: the driver unit bitmap
 * RETUEN:  NONE
 * NOTES:
 *
 */
void STKTPLG_OM_PortList2DriverUnitList(UI8_T *port_list, UI32_T *driver_unit_list)
{
    int                       i, j, index;
    UI8_T                     board_id;
    STKTPLG_BOARD_BoardInfo_T board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;


    for (*driver_unit_list=0,i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
    /*the unit may not exist,it will cause cannot get borard info,make the  stktplg task hang*/
       if (!STKTPLG_OM_UnitExist(i+1))
          continue;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        board_id = stk_unit_cfg_snapshot[i].board_info.board_id;
#else
        board_id = stk_unit_cfg[i].board_info.board_id;
#endif

        if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
        {
            perror("\r\nCan not get related board information.");
            assert(0);
        }
        for (j=0; j<board_info_p->max_port_number_on_board; j++)
        {
            index = i*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + j ;
        /* EPR: ES5508-38-06
         * The port sequence is from MSB to LSB
         */
            if (port_list[index/8] & (1<<(7-(index%8))) )
            {
                *driver_unit_list |= (1<<i) ;
                break;
            }
        }
        for (j=0; j<SYS_ADPT_MAX_NBR_OF_PORT_PER_MODULE; j++)
        {
            index = i*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT+board_info_p->max_port_number_on_board+j;
            if (port_list[index /8] & (1<<(7-(index%8))) )
            {
                *driver_unit_list |= (1<<(i+8)) ;
                break;
            }
        }
    }
}

/* FUNCTION NAME : STKTPLG_OM_IsAllSlavesAndModulesWithMC
 * PURPOSE  : check if any version of slave or optional module is too old to have MC mechanism.
 * INPUT    : None.
 * OUTPUT   : None
 * RETUEN   : TRUE: Normal
 *            FALSE: Some slave or module is too old
 */
BOOL_T STKTPLG_OM_IsAllSlavesAndModulesWithMC(void)
{
    UI32_T unit;
    UI32_T VersionNo_32[4];
    UI8_T  VersionNo[4];

    STKTPLG_OM_CHECK_STATE();

    unit = 0;
    while(STKTPLG_OM_GetNextUnit(&unit))
    {
        /* convert runtime_sw_ver string to integer
         * to do comparison with defined version in SYS_ADPT
         */
        SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        sscanf((char*)(stk_unit_cfg_snapshot[unit - 1].board_info.runtime_sw_ver),"%lu.%lu.%lu.%lu",&VersionNo_32[0],&VersionNo_32[1],&VersionNo_32[2],&VersionNo_32[3]);
#else
        sscanf((char*)(stk_unit_cfg[unit - 1].board_info.runtime_sw_ver),"%lu.%lu.%lu.%lu",&VersionNo_32[0],&VersionNo_32[1],&VersionNo_32[2],&VersionNo_32[3]);
#endif
        SYSFUN_GiveReadSem(sem_id_static);

        STKTPLG_OM_VERSION_NO_ASSIGN();

        /* define SYS_ADPT_FIRST_VERSION_USING_MC 0x001B0306 =  0.27.3.6 */
        if (*((UI32_T *)VersionNo) < (UI32_T)SYS_ADPT_FIRST_MAINBOARD_VERSION_USING_MC)
            return FALSE;
    }

    unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
    while(STKTPLG_OM_GetNextDriverUnit(&unit))
    {
        /* convert runtime_sw_ver string to integer
         * to do comparison with defined version in SYS_ADPT
         */
        SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        sscanf((char*)(stk_module_cfg_snapshot[unit - SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK - 1][0].op_code_ver),"%lu.%lu.%lu.%lu",&VersionNo_32[0],&VersionNo_32[1],&VersionNo_32[2],&VersionNo_32[3]);
#else
        sscanf((char*)(stk_module_cfg[unit - SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK - 1][0].op_code_ver),"%lu.%lu.%lu.%lu",&VersionNo_32[0],&VersionNo_32[1],&VersionNo_32[2],&VersionNo_32[3]);
#endif
        SYSFUN_GiveReadSem(sem_id_static);

        STKTPLG_OM_VERSION_NO_ASSIGN();

        if (*((UI32_T *)VersionNo) < (UI32_T)SYS_ADPT_FIRST_MODULE_VERSION_USING_MC)
            return FALSE;
    }
    return TRUE;
}

/* FUNCTION NAME : STKTPLG_OM_IsHBTTooOld
 * PURPOSE  : check if runime version of slave is too old to have different HBT2 format.
 * INPUT    : *runtime_sw_ver: runtime version
 * OUTPUT   : None
 * RETUEN   : TRUE: Old format of HBT2
 *            FALSE: Otherwise
 */
BOOL_T STKTPLG_OM_IsHBTTooOld(UI8_T *runtime_sw_ver)
{

    UI32_T VersionNo_32[4];
    UI8_T  VersionNo[4];

    SYSFUN_TakeReadSem(sem_id_static);
    sscanf((char*)runtime_sw_ver,"%lu.%lu.%lu.%lu",&VersionNo_32[0],&VersionNo_32[1],&VersionNo_32[2],&VersionNo_32[3]);
    SYSFUN_GiveReadSem(sem_id_static);

    STKTPLG_OM_VERSION_NO_ASSIGN();

    /* define SYS_ADPT_FIRST_RUNTIME_VERSION_USING_NEW_HBT2 0x001B0306 =  0.27.3.6 */
    if (*((UI32_T *)VersionNo) < (UI32_T)SYS_ADPT_FIRST_RUNTIME_VERSION_USING_NEW_HBT2)
         return TRUE;

    return FALSE;
}

/* FUNCTION NAME: STKTPLG_OM_Logical2PhyDevicePortID
 * PURPOSE: This function is used to convert logical address mode to
 *          physical address mode.
 * INPUT:   unit_id -- unit id of logical view
 *          port_id -- port id of logical view
 * OUTPUT:  phy_device_id   -- device id of physical view
 *          phy_port_id     -- port id of physical view
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_Logical2PhyDevicePortID(UI32_T unit_id,
                                          UI32_T port_id,
                                          UI32_T *phy_device_id,
                                          UI32_T *phy_port_id)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == FALSE)
    STK_UNIT_CFG_T    *unit_cfg_p;
    STKTPLG_BOARD_BoardInfo_T board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
#endif

    if(phy_device_id==NULL || phy_port_id==NULL)
    {
        SYSFUN_Debug_Printf("%s():Invalid arguments\r\n", __FUNCTION__);
        return FALSE;
    }

    /* check if logical port existing in the unit
     */
    if (!STKTPLG_OM_PortExist(unit_id, port_id))
        return(FALSE);

    /* get unit's info
     */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    *phy_device_id = stackPortMappingTable[unit_id-1][port_id-1].device_id;
    *phy_port_id   = stackPortMappingTable[unit_id-1][port_id-1].device_port_id;

#else
    unit_cfg_p = &(stk_unit_cfg[unit_id-1]);


    /* get board information
     */
    if (!STKTPLG_BOARD_GetBoardInformation(unit_cfg_p->board_info.board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    *phy_device_id = board_info_p->userPortMappingTable[unit_id-1][port_id-1].device_id;
    *phy_port_id   = board_info_p->userPortMappingTable[unit_id-1][port_id-1].device_port_id;
#endif

    return(TRUE);
}

/* FUNCTION NAME: STKTPLG_OM_GetMaxChipNum
 * PURPOSE: Get the switch chip number in this unit.
 * INPUT:   unit   -- unit number.
 * OUTPUT:  *max_chip_nmber  -- maximum chip number.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by AMTR module.
 *
 */
BOOL_T STKTPLG_OM_GetMaxChipNum(UI32_T unit, UI32_T *max_chip_nmber)
{
    UC_MGR_Sys_Info_T          uc_sys_info;
    UI32_T                     my_unit_id;
    STKTPLG_BOARD_BoardInfo_T board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    STKTPLG_OM_GetMyUnitID(&my_unit_id);

    if ((unit != my_unit_id)&&!STKTPLG_OM_UnitExist(unit))
        return FALSE;

    /* get system inforamtion first, we need board id to know board
     * information. From board information, we will know how to set
     * BCM driver
     */
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    /* get board information
     */
    if (!STKTPLG_BOARD_GetBoardInformation(uc_sys_info.board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    *max_chip_nmber = board_info_p->chip_number_of_this_unit;

    return TRUE;
}/* end of STKTPLG_OM_GetMaxChipNum */

/* FUNCTION NAME: STKTPLG_OM_GetNeighborStackingPortByChipView
 * PURPOSE: Get the stacking port of this specific chip which close to the source chip device.
 * INPUT:   src_unit            -- source unit number.
 *          src_chip_id         -- source chip id.
 *          specific_unit       -- specific unit number.
 *          specific_chip_id    -- specific chip id.
 * OUTPUT:  *port(chip view)    -- stacking port which close to source unit/src_chip_id
 *                                 in specific_unit/specific_chip_id SOC chip
 *                                 (return chip physical port).
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by HR module.
 *
 */
BOOL_T STKTPLG_OM_GetNeighborStackingPortByChipView(UI32_T src_unit, UI32_T src_chip_id,
                                                    UI32_T specific_unit, UI32_T specific_chip_id,
                                                    UI32_T *port)
{
    /*STANDALONE only now*/
    if (src_unit != 1 || specific_unit != 1)
        return FALSE;

    if (src_unit == specific_unit && src_chip_id == specific_chip_id)
        return FALSE;

#if defined(GRAHAM)
    if (src_chip_id == 0 && specific_chip_id == 1)
        *port = stackingPort2phyPortId[src_chip_id]; /* SOC1/G1 */
    else if (src_chip_id == 1 && specific_chip_id == 0)
        *port = stackingPort2phyPortId[src_chip_id]; /* SOC0/G0 */
    else
        return FALSE;
    return TRUE;
#elif defined(NOVAL) || defined(RUBY) || defined(JBOS)
    if (src_chip_id == 0 && specific_chip_id == 1)
        *port = stackingPort2phyPortId[src_chip_id]; /* SOC1/10G */
    else if (src_chip_id == 1 && specific_chip_id == 0)
        *port = stackingPort2phyPortId[src_chip_id]; /* SOC0/10G */
    else
        return FALSE;
    return TRUE;
#endif

    return FALSE;
}

/* FUNCTION NAME: STKTPLG_OM_GetNeighborStackingPort
 * PURPOSE: Get the stacking port of this specific chip which close to the source chip device.
 * INPUT:   src_unit            -- source unit number.
 *          src_chip_id         -- source chip id.
 *          specific_unit       -- specific unit number.
 *          specific_chip_id    -- specific chip id.
 * OUTPUT:  *port(broadcom view)-- stacking port which close to source unit/src_chip_id
 *                                 in specific_unit/specific_chip_id SOC chip.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by AMTR module.
 *
 */
BOOL_T STKTPLG_OM_GetNeighborStackingPort(UI32_T src_unit, UI32_T src_chip_id,
                                          UI32_T specific_unit, UI32_T specific_chip_id,
                                          UI32_T *port)
{
    /*STANDALONE only now*/
    if (src_unit != 1 || specific_unit != 1)
        return FALSE;

    if (src_unit == specific_unit && src_chip_id == specific_chip_id)
        return FALSE;

    if(port==NULL)
        return FALSE;

#if defined(GRAHAM)
    if (src_chip_id == 0 && specific_chip_id == 1)
        *port = SYS_ADPT_1ST_STACKING_PORT_NBR_PER_UNIT + 1; /* SOC1/G1 */
    else if (src_chip_id == 1 && specific_chip_id == 0)
        *port = SYS_ADPT_1ST_STACKING_PORT_NBR_PER_UNIT; /* SOC0/G0 */
    else
        return FALSE;
    return TRUE;
#elif defined(NOVAL) || defined(RUBY) || defined(JBOS)
    if (src_chip_id == 0 && specific_chip_id == 1)
        *port = SYS_ADPT_1ST_STACKING_PORT_NBR_PER_UNIT + 1; /* SOC1/10G */
    else if (src_chip_id == 1 && specific_chip_id == 0)
        *port = SYS_ADPT_1ST_STACKING_PORT_NBR_PER_UNIT; /* SOC0/10G */
    else
        return FALSE;
    return TRUE;
#endif

    return FALSE;
}

/* FUNCTION NAME: STKTPLG_OM_GetNextUnitBoardID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- Next to which unit.
 * OUTPUT:  board_id -- This is board ID, and used to be family serial number,
 *                      that is 5-bit field in project ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:   SYS_VAL_LOCAL_UNIT_ID is not supported.
 */
BOOL_T STKTPLG_OM_GetNextUnitBoardID(UI32_T *unit, UI32_T *board_id)
{

    if(unit==NULL || board_id==NULL)
        return FALSE;

    if (!STKTPLG_OM_GetNextUnit(unit))
        return FALSE;

    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    *board_id = stk_unit_cfg_snapshot[*unit-1].board_info.board_id;
    #else
    *board_id = stk_unit_cfg[*unit-1].board_info.board_id;
    #endif

    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_GetUnitFamilyID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- unit number, SYS_VAL_LOCAL_UNIT_ID for local unit.
 * OUTPUT:  family_id -- This is the family ID. and is 11-bit field in project
 *                       ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 */
BOOL_T STKTPLG_OM_GetUnitFamilyID(UI32_T unit, UI32_T *family_id)
{
    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

    if (FALSE == STKTPLG_OM_GetUnitProjectID(unit, family_id))
    {
        return FALSE;
    }

    *family_id = (*family_id & 0x000007ff);
    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_GetNextUnitFamilyID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- Next to which unit.
 * OUTPUT:  family_id -- This is the family ID. and is 11-bit field in project
 *                       ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:   SYS_VAL_LOCAL_UNIT_ID is not supported.
 */
BOOL_T STKTPLG_OM_GetNextUnitFamilyID(UI32_T *unit, UI32_T *family_id)
{
    if (FALSE == STKTPLG_OM_GetNextUnitProjectID(unit, family_id))
    {
        return FALSE;
    }

    *family_id = (*family_id & 0x000007ff);
    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_GetUnitProjectID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- unit number, SYS_VAL_LOCAL_UNIT_ID for local unit.
 * OUTPUT:  project_id -- This is the project ID, that is 16-bit.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 */
BOOL_T STKTPLG_OM_GetUnitProjectID(UI32_T unit, UI32_T *project_id)
{
    UI32_T local_use_unit;

    if(project_id==NULL)
        return FALSE;

    if (SYS_VAL_LOCAL_UNIT_ID == unit)
    {
        local_use_unit = 1;
    }
    else
    {
        local_use_unit = unit;

        if (!STKTPLG_OM_UnitExist(local_use_unit))
        {
            return FALSE;
        }
    }

    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    *project_id = stk_unit_cfg_snapshot[local_use_unit-1].board_info.project_id;
    #else
    *project_id = stk_unit_cfg[local_use_unit-1].board_info.project_id; /*image type is old name, now project ID*/
    #endif
    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_GetNextUnitProjectID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- Next to which unit.
 * OUTPUT:  project_id -- This is the project ID, that is 16-bit.
 * RETUEN:  TRUE/FALSE
 * NOTES:   SYS_VAL_LOCAL_UNIT_ID is not supported.
 */
BOOL_T STKTPLG_OM_GetNextUnitProjectID(UI32_T *unit, UI32_T *project_id)
{

    if(unit==NULL || project_id==NULL)
        return FALSE;

    if (!STKTPLG_OM_GetNextUnit(unit))
        return FALSE;

    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    *project_id = stk_unit_cfg_snapshot[*unit-1].board_info.project_id;
    #else
    *project_id = stk_unit_cfg[*unit-1].board_info.project_id;
    #endif

    return TRUE;
}

/* FUNCTION NAME : STKTPLG_OM_GetStackingState
 * PURPOSE: To get the stacking state
 * INPUT:   None.
 * OUTPUT:  state        -- The stacking state
 * RETUEN:  TRUE         -- Sucess
 *          FALSE        -- Fail
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetStackingState(UI32_T *state)
{
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    *state = ctrl_info.state;
    #else
    *state = ctrl_info_snapshot.state;
    #endif
    return (TRUE);
}

/* FUNCTION NAME: STKTPLG_OM_IsStkTplgStateChanged
 * PURPOSE: This function is used to let other cpnt to know whether the state is changed in real time
 * INPUT:   stk_state == stktplg state
 *                STKTPLG_OM_SIMPLE_STATE_ARBITRATION = 0,
 *                STKTPLG_OM_SIMPLE_STATE_MASTER,
 *               STKTPLG_OM_SIMPLE_STATE_SLAVE
 * OUTPUT:  none
 * RETUEN:  TRUE :  changed
 *          FALSE : not changed
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_IsStkTplgStateChanged(STKTPLG_OM_Simple_State_T stk_state)
{

    switch(stk_state)
    {
        case STKTPLG_OM_SIMPLE_STATE_ARBITRATION:

            if ((ctrl_info.state == STKTPLG_STATE_MASTER)||
                (ctrl_info.state == STKTPLG_STATE_STANDALONE)||
                (ctrl_info.state == STKTPLG_STATE_SLAVE))
            {
                return (TRUE);
            }
            else
                return (FALSE);

            break;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        case STKTPLG_OM_SIMPLE_STATE_MASTER:

            if ((ctrl_info.past_role == STKTPLG_STATE_MASTER)&&
#if 0
             (ctrl_info.state != STKTPLG_STATE_HALT))
#else
/*PRBOBLEM:When stacking port linkup,it will hang when the dut is doing provision    completed
ROOT CAUSE:cli wait all csc group provision completed.And stkctrl will return
when       it find stktplg topo changed and not to set cli provsion flag
solution:change check topo changed function in stkplg.
         When stktplg changed from standalone/master to standalone/master/
arbitration/halt    it will don't have any msg to make stkctrl to do provsion/master
mode again.But when  it changed from master/standalone to slave,Or salve to master/
standalone,it will has msg to notify stkctrl to do transition mode and so on*/
               (ctrl_info.state != STKTPLG_STATE_SLAVE)&&
               (ctrl_info.state != STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT))
#endif
            {
                return (FALSE);
            }
            else if((ctrl_info.past_role == STKTPLG_STATE_STANDALONE)&&
#if 0
                (ctrl_info.state != STKTPLG_STATE_HALT))
#else
/*PRBOBLEM:When stacking port linkup,it will hang when the dut is doing provision    completed
ROOT CAUSE:cli wait all csc group provision completed.And stkctrl will return
when       it find stktplg topo changed and not to set cli provsion flag
solution:change check topo changed function in stkplg.
         When stktplg changed from standalone/master to standalone/master/
arbitration/halt    it will don't have any msg to make stkctrl to do provsion/master
mode again.But when  it changed from master/standalone to slave,Or salve to master/
standalone,it will has msg to notify stkctrl to do transition mode and so on*/

                (ctrl_info.state != STKTPLG_STATE_SLAVE)&&
                (ctrl_info.state != STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT))
#endif
            {
                return (FALSE);
            }
            else
                return (TRUE);

            break;

        case STKTPLG_OM_SIMPLE_STATE_SLAVE:

            if ((ctrl_info.past_role == STKTPLG_STATE_SLAVE)&&
               (ctrl_info.state != STKTPLG_STATE_HALT))
            {
                return (FALSE);
            }
            else
                return (TRUE);
            break;
#else
        case STKTPLG_OM_SIMPLE_STATE_MASTER:

            if ((ctrl_info.state == STKTPLG_STATE_MASTER)||
                (ctrl_info.state == STKTPLG_STATE_STANDALONE))
            {
                return (FALSE);
            }
            else
                return (TRUE);

            break;

        case STKTPLG_OM_SIMPLE_STATE_SLAVE:

            if (ctrl_info.state == STKTPLG_STATE_SLAVE)
            {
                return (FALSE);
            }
            else
                return (TRUE);
            break;
#endif

        default:
            return (TRUE);
    }
}
/* FUNCTION NAME: STKTPLG_OM_SetModuleABus
 * PURPOSE: This function is used to set module 1 bus enable/disable.
 * INPUT:   module_bus  -- module bus.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   value as following
 *          -- SYS_HWCFG_SYSTEM_MODULE_1_BUS_ENABLE   0x00
 *          -- SYS_HWCFG_SYSTEM_MODULE_1_BUS_DISABLE  0x04
 *
 */
BOOL_T STKTPLG_OM_SetModuleABus(UI8_T module_bus)
{
    if (module_bus != SYS_HWCFG_SYSTEM_MODULE_1_BUS_ENABLE && module_bus != SYS_HWCFG_SYSTEM_MODULE_1_BUS_DISABLE)
        return FALSE;

    STKTPLG_OM_SetSystemLedsDrive(SYS_HWCFG_SYSTEM_MODULE_1_BUS_MASK, module_bus);

    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_SetModuleBBus
 * PURPOSE: This function is used to set module 2 bus enable/disable.
 * INPUT:   module_bus  -- module bus.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   value as following
 *          -- SYS_HWCFG_SYSTEM_MODULE_2_BUS_ENABLE   0x00
 *          -- SYS_HWCFG_SYSTEM_MODULE_2_BUS_DISABLE  0x02
 *
 */
BOOL_T STKTPLG_OM_SetModuleBBus(UI8_T module_bus)
{
    if (module_bus != SYS_HWCFG_SYSTEM_MODULE_2_BUS_ENABLE && module_bus != SYS_HWCFG_SYSTEM_MODULE_2_BUS_DISABLE)
        return FALSE;

    STKTPLG_OM_SetSystemLedsDrive(SYS_HWCFG_SYSTEM_MODULE_2_BUS_MASK, module_bus);

    return TRUE;

} /* End of STKTPLG_OM_SetModuleBBus */

/* FUNCTION NAME: STKTPLG_OM_EnableModuleBus
 * PURPOSE: This function is used to enable module bus.
 * INPUT:   module_index  -- module index.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_EnableModuleBus(UI32_T module_index)
{
    UI8_T   module_bus;
    UI8_T   module_mask_bit;

    switch (module_index)
    {
        /* Mercury have 2 single slot-in module */
        case STKTPLG_MODULE_INDEX_0:
            module_bus = SYS_HWCFG_SYSTEM_MODULE_1_BUS_ENABLE;
            module_mask_bit = SYS_HWCFG_SYSTEM_MODULE_1_BUS_MASK;
            break;

        case STKTPLG_MODULE_INDEX_1:
            module_bus = SYS_HWCFG_SYSTEM_MODULE_2_BUS_ENABLE;
            module_mask_bit = SYS_HWCFG_SYSTEM_MODULE_2_BUS_MASK;
            break;

        default:
            SYSFUN_Debug_Printf("\r\n%s:Invalid argument(module_index)", __FUNCTION__);
            return FALSE;
    }

    STKTPLG_OM_SetSystemLedsDrive(module_mask_bit, module_bus);

    return TRUE;

} /* End of STKTPLG_OM_EnableModuleBus */

/* FUNCTION NAME: STKTPLG_OM_DisableModuleBus
 * PURPOSE: This function is used to disable module bus.
 * INPUT:   module_index  -- module index.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_DisableModuleBus(UI32_T module_index)
{
    UI8_T   module_bus;
    UI8_T   module_mask_bit;

    switch (module_index)
    {
        /* Mercury have 2 single slot-in module */
        case STKTPLG_MODULE_INDEX_0:
            module_bus = SYS_HWCFG_SYSTEM_MODULE_1_BUS_DISABLE;
            module_mask_bit = SYS_HWCFG_SYSTEM_MODULE_1_BUS_MASK;
            break;

        case STKTPLG_MODULE_INDEX_1:
            module_bus = SYS_HWCFG_SYSTEM_MODULE_2_BUS_DISABLE;
            module_mask_bit = SYS_HWCFG_SYSTEM_MODULE_2_BUS_MASK;
            break;

        default:
            SYSFUN_Debug_Printf("\r\n%s:Invalid argument(module_index)", __FUNCTION__);
            return FALSE;
    }

    STKTPLG_OM_SetSystemLedsDrive(module_mask_bit, module_bus);

    return TRUE;

} /* End of STKTPLG_OM_DisableModuleBus */

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK>1)
/* FUNCTION NAME: STKTPLG_OM_SetBaseLed
 * PURPOSE: This function is used to set base LED.
 * INPUT:   base_led  -- base LED.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   value as following
 *          -- SYS_HWCFG_SYSTEM_BASE_MASTER_UNIT    0x10
 *          -- SYS_HWCFG_SYSTEM_BASE_SECOND_UNIT    0x20
 *          -- SYS_HWCFG_SYSTEM_BASE_OTHER_UNIT     0x30
 *
 */
BOOL_T STKTPLG_OM_SetBaseLed(UI8_T base_led)
{
    if (base_led < SYS_HWCFG_SYSTEM_BASE_MASTER_UNIT || base_led > SYS_HWCFG_SYSTEM_BASE_OTHER_UNIT)
        return FALSE;

    STKTPLG_OM_SetSystemLedsDrive(SYS_HWCFG_SYSTEM_BASE_LED_MASK, base_led);

    return TRUE;

} /* End of STKTPLG_OM_SetBaseLed */

/* FUNCTION NAME: STKTPLG_OM_SetStackLinkLed
 * PURPOSE: This function is used to set stack link LED.
 * INPUT:   stack_link_led  -- stack link LED.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   value as following
 *          -- SYS_HWCFG_SYSTEM_STACK_LINK_OK               0x00
 *          -- SYS_HWCFG_SYSTEM_STACK_LINK_DOWN_OR_NOSTACK  0x08
 *
 */
BOOL_T STKTPLG_OM_SetStackLinkLed(UI8_T stack_link_led)
{
#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    if (stack_link_led != SYS_HWCFG_SYSTEM_STACK_LINK_OK && stack_link_led != SYS_HWCFG_SYSTEM_STACK_LINK_DOWN_OR_NOSTACK)
        return FALSE;

    STKTPLG_OM_SetSystemLedsDrive(SYS_HWCFG_SYSTEM_STACK_LINK_LED_MASK, stack_link_led);
#endif
    return TRUE;

} /* End of STKTPLG_OM_SetStackLinkLed */
#endif /* #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK>1) */

/* FUNCTION NAME : STKTPLG_OM_HandleIPCReqMsg
 * PURPOSE : This function is used to handle ipc request for stktplg om.
 * INPUT   : msg_p(acctually SYSFUN_Msg_T*)  --  the ipc request for stktplg_om
 * OUTPUT  : msg_p(acctually SYSFUN_Msg_T*)  --  the ipc response to send when return value is TRUE
 * RETUEN  : TRUE  --  A response is requred to send
 *           FALSE --  Need not to send response.
 * NOTES   :
 *          Type of msg_p is defined as void* intentionally.
 *          Reason:
 *          stktplg_tx_type.h is included by k_l_mm.c for dedicated buffer pool
 *          and stktplg_om.h is included by stktplg_tx_type.h for the data type definition
 *          k_l_mm.c is in kernel space so it need to include k_sysfun.h instead of sysfun.h
 *          stktplg_om.h will be included by k_l_mm.c and if stktplg_om.h include sysfun.h
 *          will lead to trouble. To avoid this issue, we define type of msg_p as void*
 *
 */
BOOL_T STKTPLG_OM_HandleIPCReqMsg(void *msg_p)
{
#if 0
    SYSFUN_Msg_T        *sysfun_msg_p=(SYSFUN_Msg_T*)msg_p;
    STKTPLG_OM_IPCMsg_T *ipcmsg_p;
    UI16_T              resp_msg_size;

    if(sysfun_msg_p==NULL)
        return FALSE;

    ipcmsg_p = (STKTPLG_OM_IPCMsg_T*)(sysfun_msg_p->msg_buf);

    switch(ipcmsg_p->type.cmd)
    {
        case STKTPLG_OM_IPC_CMD_GET_NUMBER_OF_UNIT:
            ipcmsg_p->type.result=STKTPLG_OM_GetNumberOfUnit(&(ipcmsg_p->data.one_ui32.value));
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_LOCAL_MAX_PORT_CAPABILITY:
            ipcmsg_p->type.result=STKTPLG_OM_GetLocalMaxPortCapability(&(ipcmsg_p->data.one_ui32.value));
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_MAX_PORT_CAPABILITY:
            ipcmsg_p->type.result=STKTPLG_OM_GetMaxPortCapability(ipcmsg_p->data.one_ui32.value, &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_MAX_PORT_NUMBER_ON_BOARD:
            ipcmsg_p->type.result=STKTPLG_OM_GetMaxPortNumberOnBoard(ipcmsg_p->data.one_ui32.value, &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_PORT_TYPE:
            ipcmsg_p->type.result=STKTPLG_OM_GetPortType(ipcmsg_p->data.two_ui32.value[0],
                ipcmsg_p->data.two_ui32.value[1], &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_UNIT_EXIST:
            ipcmsg_p->type.result=STKTPLG_OM_UnitExist(ipcmsg_p->data.one_ui32.value);
            resp_msg_size = STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_GET_MY_RUNTIME_FIRMWARE_VER:
            STKTPLG_OM_GetMyRuntimeFirmwareVer(ipcmsg_p->data.runtime_fw_ver.value);
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_RuntimeFwVer_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_MY_UNIT_ID:
            ipcmsg_p->type.result=STKTPLG_OM_GetMyUnitID(&(ipcmsg_p->data.one_ui32.value));
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_LOCAL_UNIT_BASE_MAC:
            ipcmsg_p->type.result=STKTPLG_OM_GetLocalUnitBaseMac(ipcmsg_p->data.mac_addr.value);
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_MacAddr_T);
            break;
#if (SYS_CPNT_MGMT_PORT == TRUE)
        case STKTPLG_OM_IPC_CMD_GET_LOCAL_UNIT_BASE_MAC_FOR_MGMT_PORT:
            ipcmsg_p->type.result=STKTPLG_OM_GetLocalUnitBaseMac_ForMgmtPort(ipcmsg_p->data.mac_addr.value);
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_MacAddr_T);
            break;
#endif
        case STKTPLG_OM_IPC_CMD_GET_UNIT_BASE_MAC:
            ipcmsg_p->type.result=STKTPLG_OM_GetUnitBaseMac((UI8_T)(ipcmsg_p->data.one_ui32.value), ipcmsg_p->data.mac_addr.value);
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_MacAddr_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_UNIT_BOARD_ID:
            ipcmsg_p->type.result=STKTPLG_OM_GetUnitBoardID(ipcmsg_p->data.one_ui32.value, &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_PORT_EXIST:
            ipcmsg_p->type.result=STKTPLG_OM_PortExist(ipcmsg_p->data.two_ui32.value[0], ipcmsg_p->data.two_ui32.value[1]);
            resp_msg_size = STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_GET_BOARD_MODULE_TYPE_REG:
            {
                UI8_T module_type;

                ipcmsg_p->type.result=STKTPLG_OM_GetBoardModuleTypeReg(ipcmsg_p->data.one_ui32.value, &module_type);
                ipcmsg_p->data.one_ui32.value = (UI8_T)module_type;
                resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_MODULEA_TYPE:
            {
                UI8_T module_type;

                ipcmsg_p->type.result=STKTPLG_OM_GetModuleAType(&module_type);
                ipcmsg_p->data.one_ui32.value = (UI8_T)module_type;
                resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_MODULEB_TYPE:
            {
                UI8_T module_type;

                ipcmsg_p->type.result=STKTPLG_OM_GetModuleBType(&module_type);
                ipcmsg_p->data.one_ui32.value = (UI8_T)module_type;
                resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_OLD_MODULE_TYPE:
            {
                UI8_T module_type;

                ipcmsg_p->type.result=STKTPLG_OM_GetOldModuleType(ipcmsg_p->data.one_ui32.value, &module_type);
                ipcmsg_p->data.one_ui32.value = (UI8_T)module_type;
                resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_MODULEA_OLD_TYPE:
            {
                UI8_T module_type;

                ipcmsg_p->type.result=STKTPLG_OM_GetModuleAOldType(&module_type);
                ipcmsg_p->data.one_ui32.value = (UI8_T)module_type;
                resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_MODULEB_OLD_TYPE:
            {
                UI8_T module_type;

                ipcmsg_p->type.result=STKTPLG_OM_GetModuleBOldType(&module_type);
                ipcmsg_p->data.one_ui32.value = (UI8_T)module_type;
                resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_SYS_INFO:
            ipcmsg_p->type.result=STKTPLG_OM_GetSysInfo(ipcmsg_p->data.one_ui32.value,
                &(ipcmsg_p->data.info));
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_Info_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_DEVICE_INFO:
            ipcmsg_p->type.result=STKTPLG_OM_GetDeviceInfo(ipcmsg_p->data.one_ui32.value,
                &(ipcmsg_p->data.stk_unit_cfg));
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STK_UNIT_CFG_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_MODULE_INFO:
            ipcmsg_p->type.result=STKTPLG_OM_GetModuleInfo(ipcmsg_p->data.two_ui32.value[0],
                ipcmsg_p->data.two_ui32.value[1],
                &(ipcmsg_p->data.switch_module_info_entry));
            resp_msg_size = STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_switchModuleInfoEntry_T);
            break;
        case STKTPLG_OM_IPC_CMD_SET_MODULE_INFO:
            ipcmsg_p->type.result=STKTPLG_OM_SetModuleInfo(ipcmsg_p->data.set_module_info.unit,
                ipcmsg_p->data.set_module_info.module_index,
                &(ipcmsg_p->data.set_module_info.module_info_entry));
            resp_msg_size = STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_SLAVE_IS_READY:
            ipcmsg_p->type.result=STKTPLG_OM_SlaveIsReady();
            resp_msg_size = STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_GET_SWITCH_OPER_STATE:
            {
                UI8_T switch_oper_state;

                ipcmsg_p->type.result=STKTPLG_OM_GetSwitchOperState(&switch_oper_state);
                ipcmsg_p->data.one_ui32.value=switch_oper_state;
                resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_MASTER_UNIT_ID:
            {
                UI8_T master_unit_id;

                ipcmsg_p->type.result=STKTPLG_OM_GetMasterUnitId(&master_unit_id);
                ipcmsg_p->data.one_ui32.value=master_unit_id;
                resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_SIMPLEX_STACKING_PORT:
            ipcmsg_p->type.result=STKTPLG_OM_GetSimplexStackingPort(ipcmsg_p->data.one_ui32.value,
                &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_UNIT_BOOT_REASON:
            ipcmsg_p->type.result=STKTPLG_OM_GetUnitBootReason(ipcmsg_p->data.one_ui32.value,
                &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
#if (SYS_CPNT_MAU_MIB == TRUE)
        case STKTPLG_OM_IPC_CMD_GET_JACK_TYPE:
            ipcmsg_p->type.result=STKTPLG_OM_GetJackType(ipcmsg_p->data.four_ui32.value[0],
                ipcmsg_p->data.four_ui32.value[1], ipcmsg_p->data.four_ui32.value[2],
                ipcmsg_p->data.four_ui32.value[3], &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_MAU_MEDIA_TYPE:
            ipcmsg_p->type.result=STKTPLG_OM_GetMauMediaType(ipcmsg_p->data.four_ui32.value[0],
                ipcmsg_p->data.four_ui32.value[1], ipcmsg_p->data.four_ui32.value[2],
                &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
#endif /* end of if (SYS_CPNT_MAU_MIB == TRUE) */
        case STKTPLG_OM_IPC_CMD_GET_PORT_MEDIA_CAPABILITY:
            ipcmsg_p->type.result=STKTPLG_OM_GetPortMediaCapability(ipcmsg_p->data.two_ui32.value[0],
                ipcmsg_p->data.two_ui32.value[1], &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_NEXT_UNIT:
            ipcmsg_p->type.result=STKTPLG_OM_GetNextUnit(&(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_PORT_MAPPING:
            ipcmsg_p->type.result=STKTPLG_OM_GetPortMapping(ipcmsg_p->data.device_port_mapping.value,
                ipcmsg_p->data.one_ui32.value);
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_DevicePortMapping_T);
            break;
        case STKTPLG_OM_IPC_CMD_IS_TRANSITION:
            ipcmsg_p->type.result=STKTPLG_OM_IsTransition();
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_IS_PROVISION_COMPLETED:
            ipcmsg_p->type.result=STKTPLG_OM_IsProvisionCompleted();
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_STACKING_DBENTRY:
            ipcmsg_p->type.result=STKTPLG_OM_GetStackingDBEntry(&(ipcmsg_p->data.database_info),
                ipcmsg_p->data.one_ui32.value);
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_DataBase_Info_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_STACKING_DBENTRY_BY_MAC:
            ipcmsg_p->type.result=STKTPLG_OM_GetStackingDBEntryByMac(ipcmsg_p->data.get_stacking_dbentry_by_mac.mac,
                ipcmsg_p->data.get_stacking_dbentry_by_mac.device_type);
            resp_msg_size=STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_GET_NEXT_UNIT_UP:
            ipcmsg_p->type.result=STKTPLG_OM_GetNextUnitUp(&(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_UNITS_REL_POSITION:
            ipcmsg_p->type.result=STKTPLG_OM_GetUnitsRelPosition(ipcmsg_p->data.two_ui32.value[0],
                ipcmsg_p->data.two_ui32.value[1], &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_EXP_MODULE_IS_INSERT:
            ipcmsg_p->type.result=STKTPLG_OM_ExpModuleIsInsert(ipcmsg_p->data.one_ui32.value);
            resp_msg_size=STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_GET_HI_GI_PORT_NUM:
            STKTPLG_OM_GetHiGiPortNum(ipcmsg_p->data.get_hi_gi_port_num.port_num);
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_GetHiGiPortNum_T);
            break;
        case STKTPLG_OM_IPC_CMD_OPTION_MODULE_IS_EXIST:
            ipcmsg_p->type.result=STKTPLG_OM_OptionModuleIsExist(ipcmsg_p->data.one_ui32.value,
                &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_IS_LOCAL_UNIT:
            ipcmsg_p->type.result=STKTPLG_OM_IsLocalUnit(ipcmsg_p->data.two_ui32.value[0],
                ipcmsg_p->data.two_ui32.value[1], &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_ALL_UNITS_PORT_MAPPING:
            ipcmsg_p->type.result=STKTPLG_OM_GetAllUnitsPortMapping(ipcmsg_p->data.all_device_port_mapping.value);
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_AllDevicePortMapping_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_MAX_PORT_NUMBER_OF_MODULE_ON_BOARD:
            ipcmsg_p->type.result=STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(ipcmsg_p->data.one_ui32.value,
                &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_MODULE_HI_GI_PORT_NUM:
            ipcmsg_p->data.one_ui32.value=STKTPLG_OM_GetModuleHiGiPortNum(ipcmsg_p->data.one_ui32.value);
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_UNIT_MODULE_PORTS:
            ipcmsg_p->type.result=STKTPLG_OM_GetUnitModulePorts(ipcmsg_p->data.one_ui32.value,
                &(ipcmsg_p->data.two_ui32.value[0]), &(ipcmsg_p->data.two_ui32.value[1]));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_TwoUI32Data_T);
        case STKTPLG_OM_IPC_CMD_GET_UNIT_MAINBOARD_PORTS:
            ipcmsg_p->type.result=STKTPLG_OM_GetUnitMainboardPorts(ipcmsg_p->data.one_ui32.value,
                &(ipcmsg_p->data.two_ui32.value[0]), &(ipcmsg_p->data.two_ui32.value[1]));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_TwoUI32Data_T);
        case STKTPLG_OM_IPC_CMD_IS_OPTION_MODULE:
            ipcmsg_p->type.result=STKTPLG_OM_IsOptionModule();
            resp_msg_size=STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_GET_STACK_STATUS:
            {
                BOOL_T is_stack_status_normal;

                ipcmsg_p->type.result=STKTPLG_OM_GetStackStatus(&is_stack_status_normal);
                ipcmsg_p->data.one_ui32.value = is_stack_status_normal;
                resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_UNIT_STATUS:
            {
                BOOL_T is_unit_status_normal;

                ipcmsg_p->type.result=STKTPLG_OM_GetUnitStatus(ipcmsg_p->data.one_ui32.value,
                    &is_unit_status_normal);
                ipcmsg_p->data.one_ui32.value = is_unit_status_normal;
                resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_MODULE_STATUS:
            {
                BOOL_T is_module_status_normal;

                ipcmsg_p->type.result=STKTPLG_OM_GetModuleStatus(ipcmsg_p->data.two_ui32.value[0],
                    ipcmsg_p->data.two_ui32.value[1], &is_module_status_normal);
                ipcmsg_p->data.one_ui32.value = is_module_status_normal;
                resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_MODULE_TYPE:
            ipcmsg_p->type.result=STKTPLG_OM_GetModuleType(ipcmsg_p->data.two_ui32.value[0],
                ipcmsg_p->data.two_ui32.value[1], &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_IS_VALID_DRIVER_UNIT:
            ipcmsg_p->type.result=STKTPLG_OM_IsValidDriverUnit(ipcmsg_p->data.one_ui32.value);
            resp_msg_size=STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_GET_MY_DRIVER_UNIT:
            ipcmsg_p->type.result=STKTPLG_OM_GetMyDriverUnit(&(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_DRIVER_UNIT_EXIST:
            ipcmsg_p->type.result=STKTPLG_OM_DriverUnitExist(ipcmsg_p->data.one_ui32.value);
            resp_msg_size=STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_GET_NEXT_DRIVER_UNIT:
            ipcmsg_p->type.result=STKTPLG_OM_GetNextDriverUnit(&(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_CURRENT_STACKING_DB:
            ipcmsg_p->type.result=STKTPLG_OM_GetCurrentStackingDB(ipcmsg_p->data.get_current_stacking_db.stacking_db);
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_GetCurrentStackingDB_T);
            break;
        case STKTPLG_OM_IPC_CMD_COMBO_SFP_INDEX_TO_USER_PORT:
            ipcmsg_p->type.result=STKTPLG_OM_ComboSfpIndexToUserPort(ipcmsg_p->data.two_ui32.value[0],
                ipcmsg_p->data.two_ui32.value[1], &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_USER_PORT_TO_COMBO_SFP_INDEX:
            ipcmsg_p->type.result=STKTPLG_OM_UserPortToComboSfpIndex(ipcmsg_p->data.two_ui32.value[0],
                ipcmsg_p->data.two_ui32.value[1], &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_MAIN_BOARD_PORT_NUM:
            STKTPLG_OM_GetMainBoardPortNum(&(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_MY_MODULE_ID:
            {
                UI8_T mymodid,mymodtype;

                STKTPLG_OM_GetMyModuleID(&mymodid,&mymodtype);
                ipcmsg_p->data.two_ui32.value[0]=mymodid;
                ipcmsg_p->data.two_ui32.value[1]=mymodtype;
                resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_TwoUI32Data_T);
            }
            break;
        case STKTPLG_OM_IPC_CMD_GET_LOCAL_MODULE_PORT_NUMBER:
            ipcmsg_p->type.result=STKTPLG_OM_GetLocalModulePortNumber(&(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_UNIT_MODULE_TO_EXT_UNIT:
            STKTPLG_OM_UnitModuleToExtUnit(ipcmsg_p->data.two_ui32.value[0],
                ipcmsg_p->data.two_ui32.value[1], &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_IS_MODULE_PORT:
            ipcmsg_p->type.result=STKTPLG_OM_IsModulePort(ipcmsg_p->data.two_ui32.value[0],
                ipcmsg_p->data.two_ui32.value[1]);
            resp_msg_size=STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_PORT_LIST_2_DRIVER_UNIT_LIST:
            {
                UI32_T driver_unit_list;

                STKTPLG_OM_PortList2DriverUnitList(ipcmsg_p->data.port_list_2_driver_unit_list.port_list, &driver_unit_list);
                ipcmsg_p->data.one_ui32.value=driver_unit_list;
                resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            }
            break;
#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
        case STKTPLG_OM_IPC_CMD_GET_MASTER_BUTTON_STATUS:
            ipcmsg_p->type.result=STKTPLG_OM_GetMasterButtonStatus(ipcmsg_p->data.one_ui32.value);
            resp_msg_size=STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_IS_HBT_TOO_OLD:
            ipcmsg_p->type.result=STKTPLG_OM_IsHBTTooOld(ipcmsg_p->data.runtime_fw_ver.value);
            resp_msg_size=STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
#endif /* end of if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE) */
        case STKTPLG_OM_IPC_CMD_IS_ALL_SLAVES_AND_MODULES_WITH_MC:
            ipcmsg_p->type.result=STKTPLG_OM_IsAllSlavesAndModulesWithMC();
            resp_msg_size=STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_OM_IPC_CMD_LOGICAL_2_PHY_DEVICE_PORT_ID:
            ipcmsg_p->type.result=STKTPLG_OM_Logical2PhyDevicePortID(ipcmsg_p->data.two_ui32.value[0],
                ipcmsg_p->data.two_ui32.value[1], &(ipcmsg_p->data.two_ui32.value[0]),
                &(ipcmsg_p->data.two_ui32.value[1]));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_TwoUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_MAX_CHIP_NUM:
            ipcmsg_p->type.result=STKTPLG_OM_GetMaxChipNum(ipcmsg_p->data.one_ui32.value, &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_NEIGHBOR_STACKING_PORT:
            ipcmsg_p->type.result=STKTPLG_OM_GetNeighborStackingPort(
                ipcmsg_p->data.four_ui32.value[0], ipcmsg_p->data.four_ui32.value[1],
                ipcmsg_p->data.four_ui32.value[2], ipcmsg_p->data.four_ui32.value[3],
                &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_NEIGHBOR_STACKING_PORT_BY_CHIP_VIEW:
            ipcmsg_p->type.result=STKTPLG_OM_GetNeighborStackingPortByChipView(
                ipcmsg_p->data.four_ui32.value[0], ipcmsg_p->data.four_ui32.value[1],
                ipcmsg_p->data.four_ui32.value[2], ipcmsg_p->data.four_ui32.value[3],
                &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_NEXT_UNIT_BOARD_ID:
            ipcmsg_p->type.result=STKTPLG_OM_GetNextUnitBoardID(&(ipcmsg_p->data.two_ui32.value[0]), &(ipcmsg_p->data.two_ui32.value[1]));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_TwoUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_UNIT_FAMILY_ID:
            ipcmsg_p->type.result=STKTPLG_OM_GetUnitFamilyID(ipcmsg_p->data.one_ui32.value, &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_NEXT_UNIT_FAMILY_ID:
            ipcmsg_p->type.result=STKTPLG_OM_GetNextUnitFamilyID(&(ipcmsg_p->data.two_ui32.value[0]), &(ipcmsg_p->data.two_ui32.value[1]));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_TwoUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_UNIT_PROJECT_ID:
            ipcmsg_p->type.result=STKTPLG_OM_GetUnitProjectID(ipcmsg_p->data.one_ui32.value, &(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_NEXT_UNIT_PROJECT_ID:
            ipcmsg_p->type.result=STKTPLG_OM_GetNextUnitProjectID(&(ipcmsg_p->data.two_ui32.value[0]), &(ipcmsg_p->data.two_ui32.value[1]));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_TwoUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_GET_STACKING_STATE:
            ipcmsg_p->type.result=STKTPLG_OM_GetStackingState(&(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_OneUI32Data_T);
            break;
        case STKTPLG_OM_IPC_CMD_IS_STKTPLG_STATE_CHANGED:
            ipcmsg_p->type.result=STKTPLG_OM_IsStkTplgStateChanged((STKTPLG_OM_Simple_State_T)(ipcmsg_p->data.one_ui32.value));
            resp_msg_size=STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
 #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
         case STKTPLG_OM_IPC_CMD_IS_ANY_UNIT_NEED_TO_PROCESS:
            ipcmsg_p->type.result=STKTPLG_OM_IsAnyUnitNeedToProcess();
            resp_msg_size=STKTPLG_OM_MSGBUF_TYPE_SIZE;
            break;
 #endif
#if (SYS_CPNT_STACKING_BUTTON == TRUE)
         case STKTPLG_OM_IPC_CMD_GET_STACKINGPORT_INFO:
            ipcmsg_p->type.result=STKTPLG_OM_GetStackingPortInfo(&(ipcmsg_p->data.three_ui32.value[0]),
               &(ipcmsg_p->data.three_ui32.value[1]),&(ipcmsg_p->data.three_ui32.value[2]));
            resp_msg_size=STKTPLG_OM_GET_MSGBUFSIZE(STKTPLG_OM_ThreeUI32Data_T);
            break;
#endif
        default:
            ipcmsg_p->type.result=FALSE;
            resp_msg_size = STKTPLG_OM_MSGBUF_TYPE_SIZE;
            SYSFUN_Debug_Printf("%s(): invalid cmd(%d)\r\n", __FUNCTION__, (int)(ipcmsg_p->type.cmd));
            break;
    }
    sysfun_msg_p->msg_size = resp_msg_size;
    return TRUE;
#endif
  return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetNextSfpPortByUnitPort
 * -------------------------------------------------------------------------
 * FUNCTION: Get the unit/port of a SFP port which is next to the given
 *           unit/port
 * INPUT   : unit_p        -- Which unit
 *           port_p        -- Which port
 * OUTPUT  : unit_p        -- Corresponding unit of next SFP index
 *           port_p        -- Corresponding port of next SFP index
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetNextSfpPortByUnitPort(UI32_T *unit_p, UI32_T *port_p)
{
    UI8_T  start_sfp_port_number, end_sfp_port_number;
    UI8_T  board_id;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    if(*unit_p > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK ||
       *port_p > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        /* Invalid unit, port */
        return FALSE;
    }

    if(FALSE == STKTPLG_OM_UnitExist(*unit_p))
    {
        if (!STKTPLG_OM_GetNextUnit(unit_p))
            return FALSE;

    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        board_id = stk_unit_cfg_snapshot[*unit_p-1].board_info.board_id;
    #else
        board_id = stk_unit_cfg[*unit_p-1].board_info.board_id;
    #endif

        if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
        {
            return FALSE;
        }
        *port_p = board_info_p->start_sfp_port_number;
    }
    else
    {
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        board_id = stk_unit_cfg_snapshot[*unit_p-1].board_info.board_id;
    #else
        board_id = stk_unit_cfg[*unit_p-1].board_info.board_id;
    #endif

        if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
        {
            return FALSE;
        }
        start_sfp_port_number = board_info_p->start_sfp_port_number;
        end_sfp_port_number = board_info_p->start_sfp_port_number + board_info_p->sfp_port_number - 1;

        if(*port_p < start_sfp_port_number)
        {
            *port_p = start_sfp_port_number;
        }
        else
        {
            *port_p += 1;
            if(*port_p > end_sfp_port_number)
            {
                if (!STKTPLG_OM_GetNextUnit(unit_p))
                    return FALSE;
            #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                board_id = stk_unit_cfg_snapshot[*unit_p-1].board_info.board_id;
            #else
                board_id = stk_unit_cfg[*unit_p-1].board_info.board_id;
            #endif

                if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
                {
                    return FALSE;
                }
                *port_p = board_info_p->start_sfp_port_number;
            }
        }
    }

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME : STKTPLG_OM_SetNPortOfCFG
 * PURPOSE  : Set the port number of the unit.
 * INPUT    : unit id
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : The nport would be 24/48 without option module.
 *            The nport would be 25/49 with option module.
 */
static void STKTPLG_OM_SetNPortOfCFG(UI32_T unit_id)
{
    UI32_T  max_port_number_on_board;

    if(unit_id <1 || unit_id >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return; /*added by Jinhua Wei,because 'return' with a value, in function returning void*/
    STKTPLG_OM_GetMaxPortNumberOnBoard(unit_id, &max_port_number_on_board);

    if (TRUE == stk_unit_cfg[unit_id-1].exp_module_presented[0])
    {
        stk_unit_cfg[unit_id-1].nport = ++max_port_number_on_board;
    }
    else
    {
        stk_unit_cfg[unit_id-1].nport = max_port_number_on_board;
    }

    return;
}

static void STKTPLG_OM_SetSystemLedsDrive(UI8_T mask_bit, UI8_T value)
{


    SYSFUN_TakeWriteSem(sem_id_static);
    if ((system_leds_drive_register & mask_bit) == value)
    {
        SYSFUN_GiveWriteSem(sem_id_static);
        return;
    }

    system_leds_drive_register = (system_leds_drive_register & ~mask_bit) | value;
    SYSFUN_GiveWriteSem(sem_id_static);

#ifndef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, 2009-06-18, 11:45:15 */
    if(FALSE==PHYADDR_ACCESS_Write(sys_hwcfg_system_led_addr, 1, 1, &system_leds_drive_register))
    {
        SYSFUN_Debug_Printf("\r\n%s:write SYS_HWCFG_SYSTEM_LED_ADDR fail", __FUNCTION__);
    }
#endif /* ES3526MA_POE_7LF_LN */

    printf("\r\nReg 03 is %x", system_leds_drive_register);
    return;
} /* End of STKTPLG_OM_SetSystemLedsDrive */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME : STKTPLG_OM_TopologyIsStable
 * PURPOSE  : Check topo is stable.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None.
 */
BOOL_T STKTPLG_OM_TopologyIsStable(void)
{
    return ctrl_info.stable_flag;
}
/* FUNCTION NAME: STKTPLG_OM_ProcessUnitInsertRemove
 * PURPOSE: Process one unit insertion/removal
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE  - No more units to be processed
 *          FASLE - not done;
 * NOTES:
 */
BOOL_T STKTPLG_OM_ProcessUnitInsertRemove(BOOL_T *is_insertion,
                                     UI32_T *starting_port_ifindex,
                                     UI32_T *number_of_ports,
                                     UI8_T *unit_id,
                                     BOOL_T *use_default)
{
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    STKTPLG_DataBase_Info_T *past_stackingdb_local = STKTPLG_OM_GetPastStackingDB();
    BOOL_T *provision_lost_local = STKTPLG_OM_GetProvisionLostArray();
    BOOL_T *unit_is_valid_local = STKTPLG_OM_GetUnitIsValidArray();
    int unit;
    UI8_T temp_unit_id;
    int i, j;
    STKTPLG_BOARD_BoardInfo_T board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    UI8_T board_id;
    STKTPLG_DataBase_Info_T temp_stackingdb[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    BOOL_T local_provision_lost_local[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    BOOL_T provision_lost_local_set = FALSE;


    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
        local_provision_lost_local[i] = FALSE;
    }

    /* << ENTER CRITICAL SECTION >>
     * In this critical section:
     *  1. Check provision lost flag and mark for local processing
     *  2. Copy topology database
     */

    if(FALSE == STKTPLG_OM_TopologyIsStable())
    {
        printf("STKTPLG_OM_ProcessUnitInsertRemove return DONE, cus Topology is Unstable\n");
        return TRUE;
    }

    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
        if(TRUE == provision_lost_local[i])
        {
            /*This is either an inserted unit(INSERT), or a unit whos provisioning is lost(REMOVE)
             *NOTE: We HAVE TO process this unit i in this iteration since we already clear it.
             */
            local_provision_lost_local[i] = TRUE;
            provision_lost_local_set = TRUE;
            provision_lost_local[i] = FALSE;
            break;
        }
    }

    STKTPLG_OM_CopyDatabaseToSnapShot();


    /* << Leave Critical Section >>
     */
    memset(temp_stackingdb, 0, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * sizeof(STKTPLG_DataBase_Info_T));

    /* Step1: Convert HBT unit info to temp_stackingdb for ease of comparison
     */

    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
        temp_stackingdb[i].id_flag = UNIT_ABSENT;
    }

    if (ctrl_info_p->is_ring)
    {
        for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
           {
               temp_unit_id = ctrl_info_p->stable_hbt_down.payload[unit].unit_id;
            memcpy(temp_stackingdb[temp_unit_id-1].mac_addr,
                   ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
            temp_stackingdb[temp_unit_id-1].device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id;
            temp_stackingdb[temp_unit_id-1].id_flag = UNIT_PRESENT;
        }
    }
    else /* linear(line) topology */
    {
        for (unit = ctrl_info_p->total_units_up - 1; unit >= 0; unit--)
        {
            temp_unit_id = ctrl_info_p->stable_hbt_up.payload[unit].unit_id;
            memcpy(temp_stackingdb[temp_unit_id-1].mac_addr,
                   ctrl_info_p->stable_hbt_up.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
            temp_stackingdb[temp_unit_id-1].device_type = ctrl_info_p->stable_hbt_up.payload[unit].board_id;
            temp_stackingdb[temp_unit_id-1].id_flag = UNIT_PRESENT;
        }

        for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
        {
               temp_unit_id = ctrl_info_p->stable_hbt_down.payload[unit].unit_id;
            memcpy(temp_stackingdb[temp_unit_id-1].mac_addr,
                   ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
            temp_stackingdb[temp_unit_id-1].device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id;
            temp_stackingdb[temp_unit_id-1].id_flag = UNIT_PRESENT;
        }
    }

    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
         xgs_stacking_om_debug("past %d id %d,type %d,%x-%x-%x-%x-%x-%x \r\n",i,past_stackingdb_local[i].id_flag,
            past_stackingdb_local[i].device_type,past_stackingdb_local[i].mac_addr[0],
            past_stackingdb_local[i].mac_addr[1],past_stackingdb_local[i].mac_addr[2],past_stackingdb_local[i].mac_addr[3],
            past_stackingdb_local[i].mac_addr[4],past_stackingdb_local[i].mac_addr[5]);
        }

    /* Step2: Compare temp_stackingdb and past_stackingdb_local
     *  NOTE: Need to process Hot Removal cases first
     */
    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
        if((UNIT_ABSENT == temp_stackingdb[i].id_flag) && (UNIT_PRESENT == past_stackingdb_local[i].id_flag))
        {
            /* Perform Unit Hot Removal
             */

            if((TRUE == provision_lost_local_set)&&
               (FALSE == local_provision_lost_local[i]))
            {
                /* This unit is removed.
                 * But since another unit is discovered provision lost in critical section
                 * and have its flag cleared, we should reset the flag to true so it will be processed
                 * in the following iterations.
                 */
                for(j=0;j<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;j++)
                {
                    if(TRUE == local_provision_lost_local[j])
                    {
                        local_provision_lost_local[j] = FALSE;
                        provision_lost_local[j] = TRUE;
                    }
                }
            }

            board_id = past_stackingdb_local[i].device_type;
            if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
            {
                perror("\r\nCan not get related board information.");
                assert(0);
            }
            *starting_port_ifindex = (i*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)+1;
            *number_of_ports = board_info_p->max_port_number_on_board;

            /* Erase this Unit from database
             */
            unit_is_valid_local[i] = FALSE;

            *is_insertion = FALSE;  /*This unit is removed */

            *use_default = unit_use_default[i];
            *unit_id = i+1;


            xgs_stacking_om_debug("77new %d  id %d,type %d,%x-%x-%x-%x-%x-%x \r\n",i,temp_stackingdb[i].id_flag,
            temp_stackingdb[i].device_type,temp_stackingdb[i].mac_addr[0],
            temp_stackingdb[i].mac_addr[1],temp_stackingdb[i].mac_addr[2],temp_stackingdb[i].mac_addr[3],
            temp_stackingdb[i].mac_addr[4],temp_stackingdb[i].mac_addr[5]);
            /* Successful, Update past_stackingdb_local
             */
            past_stackingdb_local[i].id_flag = UNIT_ABSENT;
            memset(past_stackingdb_local[i].mac_addr, 0, STKTPLG_MAC_ADDR_LEN);
            return FALSE;
        }
    }


    /* Process Hot Insertion, or Provision Lost Cases
     */
    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
        if((UNIT_PRESENT == temp_stackingdb[i].id_flag) && (UNIT_PRESENT == past_stackingdb_local[i].id_flag))
        {
            if((TRUE == provision_lost_local_set)&&
               (FALSE == local_provision_lost_local[i]))
            {
                /* Should process the unit who's local_provision_lost_local is TRUE, skip
                 */
                continue;
            }

            /* Both entries are UNIT_PRESENT. Check if different unit, or if provisioning is lost
             */
            if( (memcmp(past_stackingdb_local[i].mac_addr,
                          temp_stackingdb[i].mac_addr, STKTPLG_MAC_ADDR_LEN) != 0) ||
                   (past_stackingdb_local[i].device_type != temp_stackingdb[i].device_type) ||
                   (TRUE == local_provision_lost_local[i]) )
               {
                /* Perform Unit Hot Removal
                 */
                board_id = past_stackingdb_local[i].device_type;
                if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
                {
                    perror("\r\nCan not get related board information.");
                    assert(0);
                }
                *starting_port_ifindex = (i*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)+1;
                *number_of_ports = board_info_p->max_port_number_on_board;
                *is_insertion = FALSE;  /*This unit is removed */
                *unit_id = i+1;
                /* Erase this Unit from database
                 */
                unit_is_valid_local[i] = FALSE;


        xgs_stacking_om_debug("88new %d  id %d,type %d,%x-%x-%x-%x-%x-%x \r\n",i,temp_stackingdb[i].id_flag,
            temp_stackingdb[i].device_type,temp_stackingdb[i].mac_addr[0],
            temp_stackingdb[i].mac_addr[1],temp_stackingdb[i].mac_addr[2],temp_stackingdb[i].mac_addr[3],
            temp_stackingdb[i].mac_addr[4],temp_stackingdb[i].mac_addr[5]);
                /* Successful, Update past_stackingdb_local
                 */
                past_stackingdb_local[i].id_flag = UNIT_ABSENT;
                memset(past_stackingdb_local[i].mac_addr, 0, STKTPLG_MAC_ADDR_LEN);
                return FALSE;
            }
        }
        else if((UNIT_PRESENT == temp_stackingdb[i].id_flag) && (UNIT_ABSENT == past_stackingdb_local[i].id_flag))
        {
            /* Perform Unit Hot Insert
             */

            if((TRUE == provision_lost_local_set)&&
               (FALSE == local_provision_lost_local[i]))
            {
                /* This is a unit that was just removed due to provision lost
                 * But since another unit is discovered provision lost,
                 * we should defer insertion of this unit for this iteration
                 */
                continue;
            }

            board_id = temp_stackingdb[i].device_type;
            if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
            {
                perror("\r\nCan not get related board information.");
                assert(0);
            }
            *starting_port_ifindex = (i*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)+1;
            *number_of_ports = board_info_p->max_port_number_on_board;

            /*Make this unit visible to STKCTRL and CSCs
             */
            unit_is_valid_local[i] = TRUE;

            *is_insertion = TRUE;  /*This unit is inserted */

            *use_default = unit_use_default[i];

            *unit_id = i+1;

        xgs_stacking_om_debug("99new %d  id %d,type %d,%x-%x-%x-%x-%x-%x \r\n",i,temp_stackingdb[i].id_flag,
            temp_stackingdb[i].device_type,temp_stackingdb[i].mac_addr[0],
            temp_stackingdb[i].mac_addr[1],temp_stackingdb[i].mac_addr[2],temp_stackingdb[i].mac_addr[3],
            temp_stackingdb[i].mac_addr[4],temp_stackingdb[i].mac_addr[5]);
            /* Successful, Update past_stackingdb_local
             */
            past_stackingdb_local[i].id_flag = UNIT_PRESENT;
            memcpy(past_stackingdb_local[i].mac_addr,
                       temp_stackingdb[i].mac_addr, STKTPLG_MAC_ADDR_LEN);
            past_stackingdb_local[i].device_type = temp_stackingdb[i].device_type;
            return FALSE;
        }
        else if((UNIT_ABSENT == temp_stackingdb[i].id_flag) && (UNIT_ABSENT == past_stackingdb_local[i].id_flag))
        {
            /* Both entries are UNIT_ABSENT. Do nothing
             */

            /* V3-hotswap, For master, force provision complete.
             */
            STKTPLG_OM_ProvisionCompleted(TRUE);



        }
        else
        {
            /*Error, should not happen*/

        xgs_stacking_om_debug("00new %d  id %d,type %d,%x-%x-%x-%x-%x-%x \r\n",i,temp_stackingdb[i].id_flag,
            temp_stackingdb[i].device_type,temp_stackingdb[i].mac_addr[0],
            temp_stackingdb[i].mac_addr[1],temp_stackingdb[i].mac_addr[2],temp_stackingdb[i].mac_addr[3],
            temp_stackingdb[i].mac_addr[4],temp_stackingdb[i].mac_addr[5]);
        }
    }

    /*No Units Processeed
     */
    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_IsAnyUnitNeedToProcess
 * PURPOSE: To check if any more units need to be processed
 * INPUT:
 * OUTPUT:
 * RETUEN:
 * NOTES:
 */
BOOL_T STKTPLG_OM_IsAnyUnitNeedToProcess(void)
{
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    STKTPLG_DataBase_Info_T *past_stackingdb_local = STKTPLG_OM_GetPastStackingDB();
    BOOL_T *provision_lost_local = STKTPLG_OM_GetProvisionLostArray();
    int unit;
    UI8_T temp_unit_id;
    int i;
    STKTPLG_DataBase_Info_T temp_stackingdb[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];

    /* << Enter Critical Section >>
    */


    if(FALSE == STKTPLG_OM_TopologyIsStable())
    {
        printf("STKTPLG_OM_IsAnyUnitNeedToProcess found Topology NOT stable\n");
        return TRUE;
    }

    STKTPLG_OM_CopyDatabaseToSnapShot();

    /* << Leave Critical Section >>
    */

    memset(temp_stackingdb, 0, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * sizeof(STKTPLG_DataBase_Info_T));

    /* Step1: Convert HBT unit info to temp_stackingdb for ease of comparison
     */

    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
        temp_stackingdb[i].id_flag = UNIT_ABSENT;
    }

    if (ctrl_info_p->is_ring)
    {
        for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
           {
               temp_unit_id = ctrl_info_p->stable_hbt_down.payload[unit].unit_id;
            memcpy(temp_stackingdb[temp_unit_id-1].mac_addr,
                   ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
            temp_stackingdb[temp_unit_id-1].device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id;
            temp_stackingdb[temp_unit_id-1].id_flag = UNIT_PRESENT;
        }
    }
    else /* linear(line) topology */
    {
        for (unit = ctrl_info_p->total_units_up - 1; unit >= 0; unit--)
        {
            temp_unit_id = ctrl_info_p->stable_hbt_up.payload[unit].unit_id;
            memcpy(temp_stackingdb[temp_unit_id-1].mac_addr,
                   ctrl_info_p->stable_hbt_up.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
            temp_stackingdb[temp_unit_id-1].device_type = ctrl_info_p->stable_hbt_up.payload[unit].board_id;
            temp_stackingdb[temp_unit_id-1].id_flag = UNIT_PRESENT;
        }

        for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
        {
               temp_unit_id = ctrl_info_p->stable_hbt_down.payload[unit].unit_id;
            memcpy(temp_stackingdb[temp_unit_id-1].mac_addr,
                   ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
            temp_stackingdb[temp_unit_id-1].device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id;
            temp_stackingdb[temp_unit_id-1].id_flag = UNIT_PRESENT;
        }
    }

    /* Step2: Compare temp_stackingdb and past_stackingdb_local
     */
    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
         xgs_stacking_om_debug("past %d id %d,type %d,%x-%x-%x-%x-%x-%x \r\n",i,past_stackingdb_local[i].id_flag,
            past_stackingdb_local[i].device_type,past_stackingdb_local[i].mac_addr[0],
            past_stackingdb_local[i].mac_addr[1],past_stackingdb_local[i].mac_addr[2],past_stackingdb_local[i].mac_addr[3],
            past_stackingdb_local[i].mac_addr[4],past_stackingdb_local[i].mac_addr[5]);
        xgs_stacking_om_debug("new %d  id %d,type %d,%x-%x-%x-%x-%x-%x \r\n",i,temp_stackingdb[i].id_flag,
            temp_stackingdb[i].device_type,temp_stackingdb[i].mac_addr[0],
            temp_stackingdb[i].mac_addr[1],temp_stackingdb[i].mac_addr[2],temp_stackingdb[i].mac_addr[3],
            temp_stackingdb[i].mac_addr[4],temp_stackingdb[i].mac_addr[5]);
    }
    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
        if((UNIT_PRESENT == temp_stackingdb[i].id_flag) && (UNIT_ABSENT == past_stackingdb_local[i].id_flag))
        {
            /*This Unit is Inserted
             */
            return TRUE;
        }
        else if((UNIT_PRESENT == temp_stackingdb[i].id_flag) && (UNIT_PRESENT == past_stackingdb_local[i].id_flag))
        {
            /* Both entries are UNIT_ABSENT. Check if same unit
             */
            if( (memcmp(past_stackingdb_local[i].mac_addr,
                          temp_stackingdb[i].mac_addr, STKTPLG_MAC_ADDR_LEN) != 0) ||
                   (past_stackingdb_local[i].device_type != temp_stackingdb[i].device_type) ||
                   (provision_lost_local[i] == TRUE) )
               {
                   /* This is a unit whos provisioning is lost
                    */
                printf("STKTPLG_OM_IsAnyUnitNeedToProcess found unit-%d lost its provisioning!!\n", i+1);
                   return TRUE;
            }
        }
        else if((UNIT_ABSENT == temp_stackingdb[i].id_flag) && (UNIT_PRESENT == past_stackingdb_local[i].id_flag))
        {
            /* This unit is removed
             */
             return TRUE;
        }
        else if((UNIT_ABSENT == temp_stackingdb[i].id_flag) && (UNIT_ABSENT == past_stackingdb_local[i].id_flag))
        {
            /* Both entries are UNIT_ABSENT. Do nothing
             */
        }
        else
        {
            /*Error, should not happen*/
           printf("Error!! \n");
        }
    }
    return FALSE;

}

/* FUNCTION NAME: STKTPLG_OM_SlaveSetUnitIsValidBits
 * PURPOSE: To set unit_is_valid bits according to current topology
 * INPUT:
 * OUTPUT:
 * RETUEN:
 * NOTES:
 */
void STKTPLG_OM_SlaveSetUnitIsValidBits(void)
{
    int unit;
    int i;
    UI8_T temp_unit_id;


    for(i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
        unit_is_valid[i] = FALSE;
    }

    if (ctrl_info.is_ring)
    {
        for (unit = 0; unit < ctrl_info.total_units_down; unit++)
           {
             temp_unit_id = ctrl_info.stable_hbt_down.payload[unit].unit_id;
               unit_is_valid[temp_unit_id-1] = TRUE;
        }
    }
    else /* linear(line) topology */
    {
        for (unit = ctrl_info.total_units_up - 1; unit >= 0; unit--)
        {
            temp_unit_id = ctrl_info.stable_hbt_up.payload[unit].unit_id;
            unit_is_valid[temp_unit_id-1] = TRUE;
        }

        for (unit = 0; unit < ctrl_info.total_units_down; unit++)
        {
               temp_unit_id = ctrl_info.stable_hbt_down.payload[unit].unit_id;
               unit_is_valid[temp_unit_id-1] = TRUE;
        }
    }
 }

void STKTPLG_OM_ClearSnapShotUnitEntry(UI32_T unit)
{

    UI32_T index;
    UI32_T i;


    if ((0>=unit)||(SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK<unit))
        return;


    if(ctrl_info.past_role == STKTPLG_STATE_SLAVE)
       return;

    if(TRUE== STKTPLG_OM_IsProvisionCompleted())
        return;


    for (index = 0; index < ctrl_info_snapshot.total_units_up; index++)
    {
        if(ctrl_info_snapshot.stable_hbt_up.payload[index].unit_id == unit)
        {
            for(i = index; i<(ctrl_info_snapshot.total_units_up-1); i++)
            {
                memcpy(&(ctrl_info_snapshot.stable_hbt_up.payload[i]),
                       &(ctrl_info_snapshot.stable_hbt_up.payload[i+1]),
                       sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
            }
            ctrl_info_snapshot.total_units_up--;
            ctrl_info_snapshot.total_units--;
            provision_lost[unit-1] = TRUE;
            unit_is_valid[unit-1] = FALSE;

            return;
        }
    }

    for (index = 0; index < ctrl_info_snapshot.total_units_down; index++)
    {
        if(ctrl_info_snapshot.stable_hbt_down.payload[index].unit_id == unit)
        {
            for(i = index; i< (ctrl_info_snapshot.total_units_down-1); i++)
            {
                memcpy(&(ctrl_info_snapshot.stable_hbt_down.payload[i]),
                       &(ctrl_info_snapshot.stable_hbt_down.payload[i+1]),
                       sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
            }
            ctrl_info_snapshot.total_units_down--;
            ctrl_info_snapshot.total_units--;
            provision_lost[unit-1] = TRUE;
            unit_is_valid[unit-1] = FALSE;

            return;
        }
    }

    return;

}

void STKTPLG_OM_CopyDatabaseToSnapShot(void)
{

    memcpy(stk_unit_cfg_snapshot, stk_unit_cfg, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*sizeof(STK_UNIT_CFG_T));
    memcpy(stk_module_cfg_snapshot, stk_module_cfg,
           STKTPLG_OM_MAX_MODULE_NBR*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*sizeof(STKTPLG_OM_switchModuleInfoEntry_T));

    memcpy(&ctrl_info_snapshot, &ctrl_info, sizeof(STKTPLG_OM_Ctrl_Info_T));

    /*add by fen.wang,save the database to the sharemeory for other csc group instead of pom*/
    STKTPLG_SHOM_SetStktplgInfo(&ctrl_info_snapshot,stk_unit_cfg,units_board_info_p);
}

BOOL_T *STKTPLG_OM_GetUnitIsValidArray(void)
{
    return unit_is_valid;
}

BOOL_T *STKTPLG_OM_GetProvisionLostArray(void)
{
    return provision_lost;
}

STKTPLG_DataBase_Info_T *STKTPLG_OM_GetPastStackingDB(void)
{
    return past_stackingdb;
}

/* FUNCTION NAME : STKTPLG_OM_ENG_ProvisionCompletedOnce
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   :
 * RETUEN   :
 * Notes    :
 */
BOOL_T STKTPLG_OM_ENG_ProvisionCompletedOnce()
{
    return provision_completed_once;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_ENG_GetNextUnit
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_id       -- Which Unit
 * OUTPUT  : unit_id       -- Next Unit ID
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_ENG_GetNextUnit(UI32_T *unit_id)
{

    UI32_T index, next_id;
    BOOL_T retval=FALSE;


    if (*unit_id == 8)
        return (retval);

    next_id = 9;
    SYSFUN_TakeReadSem(sem_id_static);
    for (index = 0; index < ctrl_info.total_units_up; index++)
    {
        if ( (ctrl_info.stable_hbt_up.payload[index].unit_id > *unit_id) &&
             (ctrl_info.stable_hbt_up.payload[index].unit_id <  next_id) )
            next_id = ctrl_info.stable_hbt_up.payload[index].unit_id;
    }

    for (index = 0; index < ctrl_info.total_units_down; index++)
    {
        if ( (ctrl_info.stable_hbt_down.payload[index].unit_id > *unit_id) &&
             (ctrl_info.stable_hbt_down.payload[index].unit_id <  next_id) )
            next_id = ctrl_info.stable_hbt_down.payload[index].unit_id;
    }

    if ( (next_id > 0) && (next_id < 9) )
    {
        *unit_id = next_id;
        retval = TRUE;
    }
    SYSFUN_GiveReadSem(sem_id_static);
    return (retval);

}

/* FUNCTION NAME: STKTPLG_OM_ENG_GetCtrlInfo
 * PURPOSE: This routine is used to get control information of stack topology
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  pointer to control information of stack topology.
 * NOTES:
 *
 */
STKTPLG_OM_Ctrl_Info_T *STKTPLG_OM_ENG_GetCtrlInfo(void)
{
    return (&ctrl_info);
}


/* FUNCTION NAME: STKTPLG_OM_ENG_GetDeviceInfo
 * PURPOSE: This function is used to get this device information.
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  *device_info    -- device info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_ENG_GetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info)
{

    SYSFUN_TakeReadSem(sem_id_static);
    memcpy(device_info, &stk_unit_cfg[unit - 1], sizeof(STK_UNIT_CFG_T));
    SYSFUN_GiveReadSem(sem_id_static);

    return TRUE;

} /* End of STKTPLG_OM_ENG_GetDeviceInfo */

/* FUNCTION NAME: STKTPLG_OM_ENG_UnitExist
 * PURPOSE: This function is used to check if the specified unit is
 *          existing or not.
 * INPUT:   unit_id  -- unit id
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- exist
 *          FALSE  -- not exist
 * NOTES:   Use got mac address of each unit to
 *          know if unit exist or not.
 *
 */
BOOL_T STKTPLG_OM_ENG_UnitExist(UI32_T unit_id)
{
    UI32_T unit;
    BOOL_T ret;

    ret = FALSE;
    unit = 0;
    while(STKTPLG_OM_ENG_GetNextUnit(&unit))
    {
        if (unit == unit_id)
        {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

/* FUNCTION NAME : STKTPLG_OM_ENG_SetMaxPortCapability
 * PURPOSE: To set the maximum port number of the target unit.
 * INPUT:   unit               -- unit id
 * OUTPUT:  max_port_number    -- maximum port number
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_ENG_SetMaxPortCapability(UI8_T unit, UI32_T max_port_number)
{
    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;

    SYSFUN_TakeWriteSem(sem_id_static);
    stk_unit_cfg[unit-1].nport = max_port_number;
    SYSFUN_GiveWriteSem(sem_id_static);

    return TRUE;
}

BOOL_T STKTPLG_OM_ENG_GetMaxPortNumberOnBoard(UI8_T unit, UI32_T *max_port_number)
{
    UI8_T    board_id;
    STKTPLG_BOARD_BoardInfo_T board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

    board_id = stk_unit_cfg[unit-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    *max_port_number = board_info_p->max_port_number_on_board;

    return (TRUE);
}


/* FUNCTION NAME : STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard
 * PURPOSE: To get the maximum port number on board .
 * INPUT:   unit             -- unit id.
 * OUTPUT:  *max_port_number -- maximum port number on board.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   0           -- get local device.
 *          otherwise   -- get this device.
 */
BOOL_T STKTPLG_OM_ENG_GetMaxPortNumberOfModuleOnBoard(UI8_T unit, UI32_T *max_option_port_number)
{
    UI8_T    board_id;
    STKTPLG_BOARD_BoardInfo_T board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
         return FALSE;

    board_id = stk_unit_cfg[unit-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    *max_option_port_number = board_info_p->max_port_number_of_this_unit-board_info_p->max_port_number_on_board;

    return (TRUE);
}
/* FUNCTION NAME: STKTPLG_OM_ENG_SetDeviceInfo
 * PURPOSE: This function is used to set this device information.
 * INPUT:   unit_id         -- unit id
 *          *device_info    -- device info.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_ENG_SetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info)
{
       UI8_T    board_id;
       STKTPLG_BOARD_BoardInfo_T board_info;
       STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;


    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     return FALSE;

    board_id = stk_unit_cfg[(UI8_T)unit-1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        SYSFUN_Debug_Printf("unit=%ld board_id=%d\r\n",unit,board_id);
        perror("\r\nCan not get related board information.");
        assert(0);
    }

    SYSFUN_TakeWriteSem(sem_id_static);
    memcpy(&stk_unit_cfg[unit - 1], device_info, sizeof(STK_UNIT_CFG_T));
    SYSFUN_GiveWriteSem(sem_id_static);


    return TRUE;

} /* End of STKTPLG_OM_ENG_SetDeviceInfo */

void STKTPLG_OM_ENG_GetMyRuntimeFirmwareVer(UI8_T runtime_fw_ver_ar[SYS_ADPT_FW_VER_STR_LEN+1])
{


    if(runtime_fw_ver_ar==0)
    {
        printf("\r\n%s:Invalid argument",__FUNCTION__);
        return;
    }

    SYSFUN_TakeReadSem(sem_id_static);
    memcpy(runtime_fw_ver_ar, stk_unit_cfg[ctrl_info.my_unit_id-1].board_info.runtime_sw_ver, SYS_ADPT_FW_VER_STR_LEN + 1);
    SYSFUN_GiveReadSem(sem_id_static);
}

UI8_T *STKTPLG_OM_ENG_GetUnitCfg(void)
{
  return(  &(stk_unit_cfg[0].exp_module_type[0]));
}

/* FUNCTION NAME : STKTPLG_OM_ENG_IsValidDriverUnit
 * PURPOSE  : To know the driver is valid or not.
 * INPUT    : driver_unit -- which driver want to know.
 * OUTPUT   : None
 * RETUEN   : TRUE  -- Version is valid.
 *            FALSE -- For main board: Version is different from master main board.
 *                     For module: Version is dofferent from expected module version.
 */
BOOL_T STKTPLG_OM_ENG_IsValidDriverUnit(UI32_T driver_unit)
{
    UI32_T unit_id;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    UI32_T master_unit_id = ctrl_info_snapshot.master_unit_id;
#else
    UI32_T master_unit_id = ctrl_info.master_unit_id;
#endif

    if (FALSE == STKTPLG_OM_ENG_UnitExist(driver_unit))
    {
        return FALSE;
    }

    if (driver_unit >= 1 &&
        driver_unit <=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        unit_id = driver_unit;

        /* main bioard
         */
        SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        if (0 == strncmp((char*)(stk_unit_cfg_snapshot[unit_id-1].board_info.runtime_sw_ver),
                        (char*)(stk_unit_cfg_snapshot[master_unit_id-1].board_info.runtime_sw_ver),
                        SYS_ADPT_FW_VER_STR_LEN + 1))
#else
        if (0 == strncmp((char*)(stk_unit_cfg[unit_id-1].board_info.runtime_sw_ver),
                        (char*)(stk_unit_cfg[master_unit_id-1].board_info.runtime_sw_ver),
                        SYS_ADPT_FW_VER_STR_LEN + 1))
#endif
        {
            SYSFUN_GiveReadSem(sem_id_static);
            return TRUE;
        }
        else
        {
            SYSFUN_GiveReadSem(sem_id_static);
            return FALSE;
        }
    }

    if (driver_unit >= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK+1 &&
        driver_unit <= SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK)
    {
        /* module
         */
        unit_id = driver_unit-SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
        SYSFUN_TakeReadSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        if (0 == strncmp((char*)(stk_module_cfg_snapshot[unit_id - 1][0].op_code_ver),
                        (char*)(stk_unit_cfg_snapshot[master_unit_id-1].module_expected_runtime_fw_ver),
                        SYS_ADPT_FW_VER_STR_LEN + 1))
#else
        if (0 == strncmp((char*)(stk_module_cfg[unit_id - 1][0].op_code_ver),
                        (char*)(stk_unit_cfg[master_unit_id-1].module_expected_runtime_fw_ver),
                        SYS_ADPT_FW_VER_STR_LEN + 1))
#endif
        {
            SYSFUN_GiveReadSem(sem_id_static);
            return TRUE;
        }
        else
        {
            SYSFUN_GiveReadSem(sem_id_static);
            return FALSE;
        }
    }

    return FALSE;
}

void STKTPLG_OM_SlaveSetPastMasterMac(void)
{
    if(ctrl_info.state != STKTPLG_STATE_SLAVE)
    {
        return;
    }
    memcpy(ctrl_info.past_master_mac, ctrl_info.master_mac, STKTPLG_MAC_ADDR_LEN);

    return;
}

void print_GetUnitCfg(void)
{
int i;
      for (i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
        {
    printf("%s %d ,nport %d\n",__FUNCTION__,__LINE__,stk_unit_cfg_snapshot[i].nport);
    }
}
/* EPR: N/A
     Problem: 1,when 3 dut stacking together,after system bring up ,it will produce a config file "startup" to store the stacking topo info
                      2, remove a slave A which unit ID is 3, then add a new dut.The new DUT will use unit ID 3
                      3,add A which removed before  the new dut added completed,sometimes the A will assign unit ID 3,and the new will be
                         4,but the led of the new dut still 3.Because cli has not save the topo of the new dut before the dut A added
                      4  the new dut detect the same master,and will do nothing
      Solution: check if the unit Id is used by others before assign unit id
*/
/* -------------------------------------------------------------------------
         * ROUTINE NAME - STKTPLG_OM_SetCurrentTempStackingDB
         * -------------------------------------------------------------------------
         * FUNCTION: STKTPLG_OM_SetCurrentTempStackingDB
                                  save the current stktplg topo to a datbase.
         * INPUT   :
         * OUTPUT  :
         * RETURN  : TRUE is changed/FALSE
         * NOTE    :
 * -------------------------------------------------------------------------*/

void STKTPLG_OM_SetCurrentTempStackingDB()
{
    int unit,unit_id=0;

    memset(current_temp_stackingdb,0,sizeof(STKTPLG_OM_StackingDB_T)*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);
    if (ctrl_info.is_ring)
    {
        for (unit = 0; unit < ctrl_info.total_units_down; unit++)
        {
            unit_id = ctrl_info.stable_hbt_down.payload[unit].unit_id;
            current_temp_stackingdb[unit_id-1].unit_id  = unit_id;
            memcpy(current_temp_stackingdb[unit_id-1].mac_addr,ctrl_info.stable_hbt_down.payload[unit].mac_addr,SYS_ADPT_MAC_ADDR_LEN);
            current_temp_stackingdb[unit_id-1].device_type  = ctrl_info.stable_hbt_up.payload[unit].board_id;
        }
    }
    else
    {
        for (unit = ctrl_info.total_units_up - 1; unit >= 0; unit--)
        {
            unit_id = ctrl_info.stable_hbt_up.payload[unit].unit_id;
            current_temp_stackingdb[unit_id-1].unit_id = unit_id;

            memcpy(current_temp_stackingdb[unit_id-1].mac_addr,ctrl_info.stable_hbt_up.payload[unit].mac_addr,SYS_ADPT_MAC_ADDR_LEN);
            current_temp_stackingdb[unit_id-1].device_type  = ctrl_info.stable_hbt_up.payload[unit].board_id;
        }
        for (unit = 1; unit < ctrl_info.total_units_down; unit++)
        {
//TODO            unit_id = ctrl_info.stable_hbt_down.payload[unit].unit_id;
            current_temp_stackingdb[unit_id -1].unit_id = unit_id;
            memcpy(current_temp_stackingdb[unit_id-1].mac_addr,ctrl_info.stable_hbt_down.payload[unit].mac_addr,SYS_ADPT_MAC_ADDR_LEN);
//TODO            current_temp_stackingdb[unit_id-1].device_type  = ctrl_info.stable_hbt_down.payload[unit].board_id;
        }
    }
    for(unit=0;unit<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;unit++)
    {
        xgs_stacking_om_debug("\r\nCurrentTempStackingDB %d unit-id %ld,%x-%x-%x-%x-%x-%x type:%u\r\n",unit,current_temp_stackingdb[unit].unit_id,
        current_temp_stackingdb[unit].mac_addr[0],
        current_temp_stackingdb[unit].mac_addr[1],current_temp_stackingdb[unit].mac_addr[2],current_temp_stackingdb[unit].mac_addr[3],
        current_temp_stackingdb[unit].mac_addr[4],current_temp_stackingdb[unit].mac_addr[5],
        current_temp_stackingdb[unit_id-1].device_type);
    }
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetUnitIdFromTempStackingDB
 * -------------------------------------------------------------------------
 * PURPOSE :  Get unit id from temp stacking db.
 * INPUT   :  mac          - the mac of theunit
              device_type  - the device type of the unit       
 * OUTPUT  :  None
 * RETURN  :  Non-Zero: An available unit id
 *            Zero: No availabe unit id
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T STKTPLG_OM_GetUnitIdFromTempStackingDB(
        UI8_T mac[STKTPLG_MAC_ADDR_LEN], UI8_T device_type)
{
    int unit;

    for(unit=0;unit<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;unit++)
    {
        if(memcmp(mac , current_temp_stackingdb[unit].mac_addr,STKTPLG_MAC_ADDR_LEN) == 0
           && current_temp_stackingdb[unit].device_type == device_type)
        {
            return  current_temp_stackingdb[unit].unit_id;
        }
    }
    return 0;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_IsTempStackingDBEmpty
 * -------------------------------------------------------------------------
 * PURPOSE :  check if current_temp_stackingdb is empty
 * INPUT   :  None     
 * OUTPUT  :  None
 * RETURN  :  TRUE:  current_temp_stackingdb is empty
 *            FALSE: current_temp_stackingdb is NOT empty
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_IsTempStackingDBEmpty(void)
{
    int unit;
     
    for(unit=0; unit<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
    {
        if(current_temp_stackingdb[unit].unit_id !=0)
            return FALSE;
    }
    return TRUE;
}


/* -------------------------------------------------------------------------
     * ROUTINE NAME - STKTPLG_OM_CheckIfUnitUsed
     * -------------------------------------------------------------------------
     * FUNCTION: STKTPLG_OM_CheckIfUnitUsed
                              check if the unit_id used by the mac
     * INPUT   :   unit_id
                           mac
     * OUTPUT  :
     * RETURN  : TRUE is used by the mac/FALSE
     * NOTE    :
     * -------------------------------------------------------------------------*/

static BOOL_T STKTPLG_OM_CheckIfUnitUsed(int unit_id,UI8_T *mac)
{
    int unit;
    
    /* current_temp_stackingdb keeps the last topo result.
     * possible cases listed below:
     * Case 1 - The given unit ID is never used by the last topo, the given
     *          unit_id should be used by the specified mac, so return TRUE.
     * Case 2 - The given unit ID is used in the last topo, so the mac should
     *          still use the given unit ID, so return TRUE.
     * Case 3 - Before the port mapping table saved, the dut leave and come
     *          back,and the given unit ID is used by other, so re-assign unit
     *          ID to the given mac, so return FALSE.
     */
     for(unit=0;unit<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;unit++)
     {
        if (current_temp_stackingdb[unit].unit_id == unit_id)
        {
            /* Case 2 - return TRUE */
            if(memcmp(mac , current_temp_stackingdb[unit].mac_addr,STKTPLG_MAC_ADDR_LEN) == 0)
                return TRUE;
            else /* Case 3 - Return FALSE */
                return FALSE;
        }
     }

    /* Case 1 - Return TRUE */
    return TRUE;

}

#endif

/* FUNCTION NAME: STKTPLG_OM_GetDebugMode
 * PURPOSE: To get debug mode value from STKTPLG_OM
 * INPUT:
 * OUTPUT:  debug_mode  -  debug mode value
 * RETUEN:
 * NOTES:   BIT 0 -> 1: Show debug message 0: Do not show debug message
 *          BIT 1 -> 1: Show SFP DDM Temperature 0: Do not show Temperature
 *          BIT 2 -> 1: Show SFP Present Status 0: Do not show SFP Present Status
 */
void STKTPLG_OM_GetDebugMode(UI8_T *debug_mode)
{
    *debug_mode = stktplg_om_debug_mode;
}

/* FUNCTION NAME: STKTPLG_OM_SetDebugMode
 * PURPOSE: To set debug mode value in STKTPLG_OM
 * INPUT:
 * OUTPUT:
 * RETUEN:
 * NOTES:   BIT 0 -> 1: Show debug message 0: Do not show debug message
 *          BIT 1 -> 1: Show SFP DDM Temperature 0: Do not show Temperature
 *          BIT 2 -> 1: Show SFP Present Status 0: Do not show SFP Present Status
 */
void STKTPLG_OM_SetDebugMode(UI8_T debug_mode)
{
    stktplg_om_debug_mode = debug_mode;
}

BOOL_T STKTPLG_OM_SetMyUnitID(UI32_T my_unit_id)
{


   if(my_unit_id > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    return FALSE ;

   SYSFUN_TakeWriteSem(sem_id_static);
    ctrl_info.my_unit_id = my_unit_id;
    STKTPLG_SHOM_SetMyUnitID(my_unit_id);
    SYSFUN_GiveWriteSem(sem_id_static);

   return TRUE;


}

#define CLI_DEF_MAX_HEADER      24     /* maximum message number for help */
#define CLI_DEF_DISP_WIDTH      80

static UI8_T STKTPLG_OM_GetStackingDBFromFS(STKTPLG_OM_StackingDB_T stacking_db[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK])
{
    UI8_T  type_str[30] = {0};
    UI8_T  content_str[30] = {0};
    UI32_T len = 0;
    UI32_T i = 0, j = 0, k = 0;
    UI8_T     *tmp_xbuf;
    UI8_T     *tmp_xbuf_origin;
    UI8_T     *cmd_buf;
    UI32_T  ret_val = 0;
    UI32_T  pars_count = 0;
    UI32_T  read_count = 0;
    UI32_T  unit_id = 0;
    UI8_T   mac_addr_str[20];
    UI32_T  device_type = 0;
    UI8_T   mac_addr[SYS_ADPT_MAC_ADDR_LEN] = {0};
    UI8_T   startupfile[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1] = {0};
    FS_File_Attr_T file_attr;
    UI32_T xfer_buf_length=0;
    UI8_T  temp_num[4] = {0};
    UI32_T fs_ret  = 0;
    static UI8_T    stacking_buf [CLI_DEF_MAX_HEADER*CLI_DEF_DISP_WIDTH];


    /**use static buffer instead of cli_mgr_allocate buffer    */

    memset(stacking_buf,0,sizeof(stacking_buf));
    tmp_xbuf_origin = stacking_buf;

    if((cmd_buf = malloc(SYS_ADPT_CLI_MAX_BUFSIZE)) == NULL)
    {
        return 0;
    }
    memset(cmd_buf,0,SYS_ADPT_CLI_MAX_BUFSIZE);
    tmp_xbuf = tmp_xbuf_origin;
    if(FS_RETURN_OK!=FS_GetStartupFilename(DUMMY_DRIVE, FS_FILE_TYPE_CONFIG, startupfile))
    {
        printf("Failed to get start file name\r\n");
        free(cmd_buf);
        return 0;
    }
    file_attr.file_type_mask = FS_FILE_TYPE_MASK(FS_FILE_TYPE_CONFIG);
    strcpy((char *)file_attr.file_name, (char *)startupfile);
    if(FS_GetFileInfo(DUMMY_DRIVE, &file_attr) != FS_RETURN_OK)
    {
        printf("Failed to get start file info\r\n");
        free(cmd_buf);
        return 0;
    }
    fs_ret = FS_ReadFile( DUMMY_DRIVE, file_attr.file_name ,  tmp_xbuf, CLI_DEF_MAX_HEADER*CLI_DEF_DISP_WIDTH, &xfer_buf_length);
    if ( fs_ret != FS_RETURN_OK && fs_ret != FS_RETURN_FILE_TRUNCATED)
    {
        printf("Failed to read file info\r\n");
        free(cmd_buf);
        return 0;
    }

    while ( read_count < CLI_DEF_MAX_HEADER )
    {
        if (STKTPLG_OM_ReadLine_From_XferBuf(cmd_buf, &tmp_xbuf))
        {
            read_count ++;
            len = strlen((char *)cmd_buf);
            i = 0;
            j = 0;
            k = 0;
            for (i = 1; i < len; i++ )
            {
                if (cmd_buf[i] == '>')
                {
                    break;
                }
                if (cmd_buf[i] == '<')
                {
                    continue;
                }
                else
                {
                    /*prevent buffer excreed max*/
                    if (j>=30)
                    {
                        break;
                    }
                    type_str[j] = cmd_buf[i];
                    j++;
                }
            }

            k = 0;
            j = 0;
            if (strcmp((char *)type_str, "stackingMac") == 0)
            {
                for (k = i+1; k < len; k++ )
                {
                    if (cmd_buf[k] == '<')
                    {
                        break;
                    }
                    else
                    {
                        /*prevent buffer excreed max*/
                        if (j>=30)
                        {
                            break;
                        }
                        content_str[j] = cmd_buf[k];
                        j++;
                    }
                }
                pars_count ++;


                if(content_str[0] != '0'||content_str[1] != '0')
                {
                    unit_id = 0;
                    device_type = 0;
                    memset(mac_addr,0,SYS_ADPT_MAC_ADDR_LEN);
                    memset(mac_addr_str,0,sizeof(mac_addr_str));

                    memcpy(temp_num,content_str,2);
                    temp_num[2] = '\0';
                    unit_id = atoi((char *)temp_num);

                    memcpy(temp_num,content_str+21,2);
                    temp_num[2] = '\0';
                    device_type = atoi((char *)temp_num);

                    memcpy(mac_addr_str,content_str+3,17);
                    mac_addr_str[17] = '\0';
                    STKTPLG_OM_AtoEa(mac_addr_str, mac_addr);

                    stacking_db[pars_count - 1].unit_id = unit_id;
                    memcpy(stacking_db[pars_count - 1].mac_addr,mac_addr,SYS_ADPT_MAC_ADDR_LEN);
                    stacking_db[pars_count -1 ].device_type = device_type;
                    ret_val ++;
                }

                if(pars_count == SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
                {
                       free(cmd_buf);
                    return  ret_val;
                }
            }
        }
        else
        {
            free(cmd_buf);
            return ret_val;
        }
    }

    free(cmd_buf);
    return ret_val;
}

static BOOL_T STKTPLG_OM_ReadLine_From_XferBuf(UI8_T *cmd_buf, UI8_T **xbuf_p)
{
    UI32_T i;

    memset(cmd_buf, 0, SYS_ADPT_CLI_MAX_BUFSIZE);

    for ( i=0; i<SYS_ADPT_CLI_MAX_BUFSIZE && ( *(*xbuf_p+i) != '\n' && *(*xbuf_p+i)!='\0'); i++ )
    {
/*
   pttch 92.04.24 add for check config file is valid or not
*/
       if (*(*xbuf_p+i) > 127)
       {
          return FALSE;
       }
       *(cmd_buf+i) = *(*xbuf_p+i);
    }

    *(cmd_buf+i) = 0;
    *xbuf_p = *xbuf_p + (i+1);

    return TRUE;
}

/*****************************************************************************/
/* Convert string s (xx-xx-xx-xx-xx-xx) to Ethernet address (6-byte object). */
/*                                                                           */
/* Return: 1 => success, result put in en                                    */
/*         0 => fail, content of en undetermined                             */
/*****************************************************************************/
static int STKTPLG_OM_AtoEa(UI8_T *s, UI8_T *en)
{
    UI8_T token[20];
    int  i,j;  /* i for s[]; j for token[] */
    int  k;    /* k for en[] */

    i = 0;
    j = 0;
    k = 0;

    while (s[i] != '\0')
    {
        if (s[i] == '-')
        {
            token[j] = '\0';
            if (strlen((char *)token) != 2)
            {
                return 0;
            }
            else if (k >= 6)
            {
                return 0;
            }
            else
            {
                en[k++] = (UI8_T)STKTPLG_OM_AtoUl(token,16);
                i++; j = 0;
            }
        }
        else if (!(s[i] >= '0' && s[i] <= '9') &&
                 !(s[i] >= 'A' && s[i] <= 'F') &&
                 !(s[i] >= 'a' && s[i] <= 'f'))
        {
            return 0;
        }
        else
        {
            token[j++] = s[i++];
        }

    } /* while */

    token[j] = '\0';
    if (strlen((char *)token) != 2)
    {
        return 0;
    }
    else if (k != 5)
    {
        return 0;
    }

    en[k] = (UI8_T)STKTPLG_OM_AtoUl(token,16);

    return 1;
}

/*****************************************/
/* Convert string 's' to unsigned long,  */
/* with 'radix' as base.                 */
/*****************************************/

static UI32_T STKTPLG_OM_AtoUl(UI8_T *s, int radix)
{
    int i;
    unsigned long n =0;

    for (i = 0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
        ;       /* skip white space */

    if (s[i] == '+' || s[i] == '-')
    {
        i++;    /* skip sign */
    }

    if (radix == 10)
    {
        for (n = 0; s[i] >= '0' && s[i] <= '9'; i++)
        {
            n = 10 * n + s[i] - '0';
        }
    }
    else if (radix == 16)
    {
        if ( (s[i] == '0') && (s[i+1] == 'x' || s[i+1] == 'X') ) // Charles,
           i=i+2;                                                // To skip the "0x" or "0X"


        for (n = 0;
            (s[i] >= '0' && s[i] <= '9') ||
            (s[i] >= 'A' && s[i] <= 'F') ||
            (s[i] >= 'a' && s[i] <= 'f');
            i++)
        {
            if (s[i] >= '0' && s[i] <= '9')
            {
                n = 16 * n + s[i] - '0';
            }
            else if (s[i] >= 'A' && s[i] <= 'F')
            {
                n = 16 * n + s[i] - 'A'+ 10;
            }
            else if (s[i] >= 'a' && s[i] <= 'f')
            {
                n = 16 * n + s[i] - 'a'+ 10;
            }
        }
    }
    return (n);
}
#if (SYS_CPNT_STACKING_BUTTON == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_IsStackingButtonChanged
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : check stacking button state is changed or not
 * OUTPUT  :
 * RETURN  : TRUE is changed/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/

BOOL_T STKTPLG_OM_IsStackingButtonChanged()
{
#ifndef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, 2009-06-15, 15:12:33 */
   UI8_T  stack_button_status = 0;

#ifdef ECN430_FB2
   if(FALSE==PHYSICAL_ADDR_ACCESS_Read(SYS_HWCFG_EPLD_MODULE_STATUS, 1, 1, &stack_button_status))
   {
         SYSFUN_Debug_Printf("\r\n%s: Access SYS_HWCFG_STACK_STATUS_ADDR fail", __FUNCTION__);
         return FALSE;
     }


     if((stack_button_status & 0x80) == 0)

#else
/*it is for BT*/
      if(FALSE==PHYSICAL_ADDR_ACCESS_Read(SYS_HWCFG_BUTTON_STACKING_ADDR, 1, 1, &stack_button_status))
    {
        printf("\r\n%s: Access SYS_HWCFG_STACK_STATUS_ADDR fail", __FUNCTION__);
        return FALSE;
    }

/*make the code common,to compare with the pressed result*/
    if ( SYS_HWCFG_BUTTON_STACKING_PRESSED == (stack_button_status & SYS_HWCFG_BUTTON_STACKING_MASK) )
#endif
    {
        if(ctrl_info.stacking_button_state != TRUE )
        {
          ctrl_info.stacking_button_state = TRUE;
          return TRUE;
        }

    }
    else
    {
        if(ctrl_info.stacking_button_state != FALSE )
        {
          ctrl_info.stacking_button_state = FALSE;
          return TRUE;
        }
    }
#endif /* ES3526MA_POE_7LF_LN */



    return FALSE;

}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetStackingButtonState
 * -------------------------------------------------------------------------
 * FUNCTION: .
 * INPUT   : is_pressed
 * OUTPUT  :
 * RETURN  : TRUE /FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/

BOOL_T STKTPLG_OM_GetStackingButtonState(BOOL_T *is_pressed)
{
    if(is_pressed== NULL)
      return FALSE;

    *is_pressed = ctrl_info.stacking_button_state;


     return TRUE;
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetStackingPortInfo
 * -------------------------------------------------------------------------
 * FUNCTION: check stacking button is enable or not,get uplink downlink port
 * INPUT   :
 * OUTPUT  :
 * RETURN  : TRUE /FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/

BOOL_T STKTPLG_OM_GetStackingPortInfo(UI32_T *is_stacking_enable,UI32_T *up_link_port,UI32_T *down_link_port  )
{
   BOOL_T is_pressed;
   STKTPLG_BOARD_BoardInfo_T board_info;
   STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
   UI8_T board_id;

   if(!is_stacking_enable || !up_link_port || !down_link_port)
    return FALSE;

   if(!STKTPLG_OM_GetStackingButtonState(&is_pressed))
    return FALSE;

   *is_stacking_enable = (UI32_T)is_pressed;

   board_id = stk_unit_cfg[ctrl_info.my_unit_id-1].board_info.board_id;

   if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        printf("\r\nCan not get related board information.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

   if(*is_stacking_enable)
   {
    *up_link_port =board_info_p->stacking_uplink_port ;
    *down_link_port = board_info_p->stacking_downlink_port;
   }
   else
   {
    *up_link_port= 0;
    *down_link_port = 0;
   }

   return TRUE;

}

#endif

#ifdef ASF4526B_FLF_P5
BOOL_T STKTPLG_OM_DetectSfpInstall(UI32_T unit, UI32_T u_port, BOOL_T *is_present)
{
    UI8_T     board_id;
    UI8_T     offset, value;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

    UI32_T xe_port_offset = 0;

    UI32_T sfp_list_count, sfp_addr_list[] = {
                            SYS_HWCFG_GBIC_STATUS_0_ADDR,
                            SYS_HWCFG_GBIC_STATUS_1_ADDR,
                            };

    UI32_T xfp_list_count, xfp_addr_list[] = {
                            SYS_HWCFG_EXTENSION_GBIC_STATUS_0_ADDR,
                            SYS_HWCFG_EXTENSION_GBIC_STATUS_1_ADDR,
                            };

    sfp_list_count = (sizeof(sfp_addr_list) / sizeof(UI32_T));
    xfp_list_count = (sizeof(xfp_addr_list) / sizeof(UI32_T));

    if(is_present == NULL)
    {
        return FALSE;
    }
    *is_present = FALSE;

    if (unit != ctrl_info_snapshot.my_unit_id)
    {
        if (!STKTPLG_OM_UnitExist(unit))
        {
            return FALSE;
        }
    }

    board_id = stk_unit_cfg_snapshot[ctrl_info_snapshot.my_unit_id - 1].board_info.board_id;
    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        return FALSE;
    }
    board_id = stk_unit_cfg_snapshot[unit - 1].board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, board_info_p))
    {
        return FALSE;
    }

    /* check main board.
     */
    if ( (u_port < 1) || (u_port > board_info_p->max_port_number_on_board))
    {
        return FALSE;
    }

    if ((u_port>=board_info_p->start_sfp_port_number) &&
         (u_port < (board_info_p->start_sfp_port_number+board_info_p->sfp_port_number)))
    {
        offset = (u_port -1) / 8;
        if( offset < sfp_list_count)
        {
            if( PHYSICAL_ADDR_ACCESS_Read(sfp_addr_list[offset], 1, 1, &value) == TRUE)
            {
                if( (value & ( 0x01 << ((u_port - 1) % 8) ) ) == 0)
                    *is_present = TRUE;
            }
            else
            {
                printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Read fail -- 0x%02x \n", __FILE__,  sfp_addr_list[offset]);
            }
        }
    }
    else if ( (u_port >= board_info_p->start_expansion_port_number) &&
                    (u_port < (board_info_p->start_expansion_port_number + board_info_p->max_slots_for_expansion)))
    {
        offset = (u_port - board_info_p->start_expansion_port_number);
        if(offset < xfp_list_count)
        {
            if( PHYSICAL_ADDR_ACCESS_Read(xfp_addr_list[offset], 1, 1, &value) == TRUE)
            {
                if( (value & 0x38) == 0x28)
                    *is_present = TRUE;
            }
            else
            {
                printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Read fail -- 0x%02x \n", __FILE__,  xfp_addr_list[offset]);
            }
        }
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_IsComboPort
 * -------------------------------------------------------------------------
 * FUNCTION: To know whether the given port is combo port or not.
 * INPUT   : unit -- which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : If this port is combo port, return code is TRUE. Or return code is FALSE.
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_IsComboPort(UI32_T unit, UI32_T port)
{
    UI32_T    media_cap;

    STKTPLG_OM_GetPortMediaCapability(unit,port,&media_cap);
    if ((media_cap & STKTPLG_TYPE_PORT_MEDIA_CAP_COPPER) && (media_cap & STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER))
        return TRUE;
    else
        return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_HasSfpPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function returns TRUE if the given port is a sfp port
 *           or is a combo port which contains a sfp port.
 * INPUT   : unit -- which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : If this port is combo port, return code is TRUE.
 *           If this port is a sfp port, return code is TRUE.
 *           Other cases will return FALSE.
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_HasSfpPort(UI32_T unit, UI32_T port)
{
    UI32_T media_cap = STKTPLG_TYPE_PORT_MEDIA_CAP_OTHER;

    STKTPLG_OM_GetPortMediaCapability(unit,port,&media_cap);
    if (media_cap & STKTPLG_TYPE_PORT_MEDIA_CAP_FIBER)
        return TRUE;
    else
        return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetModulePortListByModuleSlotIndex
 * -------------------------------------------------------------------------
 * FUNCTION: To get port list of the specified module slot
 * INPUT   : module_slot_index
 *           module_id
 * OUTPUT  : portlist - module port list
 * RETURN  : number of module ports
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T STKTPLG_OM_GetModulePortListByModuleSlotIndex(UI8_T module_slot_index, UI8_T module_id, UI32_T *portlist)
{
#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE)
    UI32_T unit;
    UI8_T board_id;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    unit = ctrl_info_snapshot.my_unit_id;
    board_id = stk_unit_cfg_snapshot[unit-1].board_info.board_id;
#else
    unit = ctrl_info.my_unit_id;
    board_id = stk_unit_cfg[unit-1].board_info.board_id;
#endif

    return STKTPLG_BOARD_GetModulePortListByModuleSlotIndex(board_id, module_slot_index, module_id, portlist);
#endif /* (SYS_CPNT_10G_MODULE_SUPPORT == TRUE) */

    return 0;
}

/* FUNCTION NAME: STKTPLG_OM_TranslateEntPhysicalIndexToTypeUnitIndex
 * PURPOSE: Translate entPhysicalIndex to type, unit, port
 * INPUT:  index   -- entity physical index.
 * OUTPUT: type
 *         unit
 *         port
 * RETURN: TRUE  -- success
 *         FALSE -- failure
 * NOTES:  This function is used by SNMP.
 *
 */
static BOOL_T STKTPLG_OM_TranslateEntPhysicalIndexToTypeUnitIndex(I32_T index, UI32_T *type_p, UI32_T *unit_p, UI32_T *port_p)
{
    /* Transfer the *ent_physical_entry.ent_physical_index to type */
    if(type_p == NULL || unit_p == NULL || port_p == NULL)
        return FALSE;
    if(index >= STACK_ENT_START && index <= STACK_ENT_END)
        *type_p = ENT_STACK_NAME;
    else if(index >= UNIT_ENT_START && index <= UNIT_ENT_END)
        *type_p = ENT_UNIT_NUM;
    else if(index >= MANAGEMENT_ENT_START && index <= MANAGEMENT_ENT_END)
        *type_p = ENT_MANAGEMENT_PORT;
    else if(index >= STACK_MODUL_ENT_START && index <= STACK_MODUL_ENT_END)
        *type_p = ENT_STACK_CARD;
    else if(index >= PORT_ENT_START && index <= PORT_ENT_END)
        *type_p = ENT_PHYSICAL_PORT;
    else
        return FALSE;

    if(*type_p == ENT_STACK_CARD)
        *unit_p = index - STACK_MODUL_ENT_START + 1;

    if(*type_p == ENT_PHYSICAL_PORT)
    {
        *unit_p = index / INDEX_NUM_PER_UNIT;
        *port_p = index - (*unit_p) * INDEX_NUM_PER_UNIT;
    }

    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_FindNextEntPhysicalIndex
 * PURPOSE: Get next the entPhysicalIndex.
 * INPUT:  None
 * OUTPUT: *entIndex
 * RETURN: TRUE  -- success
 *         FALSE -- failure
 * NOTES:
 *
 */
static BOOL_T STKTPLG_OM_FindNextEntPhysicalIndex (I32_T *entIndex)
{
    UI32_T   i, j;
    UI32_T   unit_id;
    UI32_T   index;
    UI32_T   unit_num, agent_num;
    UI32_T   port_num;
    UI8_T    module_presented[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];
    UI8_T    module_type[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];

    (*entIndex)++;
    index = *entIndex;

    /* check Stack Name */
    if (index <= STACK_ENT_START)
    {
        *entIndex = STACK_ENT_START;
        return TRUE;
    }

    if ( index < UNIT_ENT_START)
    {
        *entIndex = UNIT_ENT_START;
        return TRUE;
    }

    /* check Unit Number */
    if ( index <= UNIT_ENT_END )
    {
        if (!STKTPLG_OM_GetNumberOfUnit(&unit_num))
        {
            return FALSE;
        }
        if (index > (unit_num + UNIT_ENT_START - 1))
        {
            *entIndex = MANAGEMENT_ENT_START;
            return TRUE;
        }
        else
        {
            return TRUE;
        }
    }

    if ( index < MANAGEMENT_ENT_START)
    {
        *entIndex = MANAGEMENT_ENT_START;
        return TRUE;
    }

    /* check Manager Number */
    if ( index <= MANAGEMENT_ENT_END )
    {
        agent_num = 1;
        if (index > (agent_num + MANAGEMENT_ENT_START - 1))
        {
            for (i=1; i<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
            {
                if (STKTPLG_OM_UnitExist(i) == TRUE)
                {
                    if (TRUE == STKTPLG_OM_GetDeviceModulePresented(i, module_presented))
                    {
                        for (j=0; j<SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT; j++)
                            if (module_presented[j])
                            {
                                *entIndex = STACK_MODUL_ENT_START;
                                return TRUE;
                            }
                    }
              }

            }
            *entIndex = PORT_ENT_START;
            return TRUE;
        }
        else
        {
            return TRUE;
        }
    }

    if ( index < STACK_MODUL_ENT_START)
    {
        *entIndex = STACK_MODUL_ENT_START;
        return TRUE;
    }

    /* check Stack Module Number */
    if ( index <= STACK_MODUL_ENT_END )
    {
        unit_id = index - STACK_MODUL_ENT_START + 1;
        if (!STKTPLG_OM_UnitExist(unit_id))
            return FALSE;

        if (!STKTPLG_OM_GetDeviceModulePresented(unit_id, module_presented))
            return FALSE;

        if (!STKTPLG_OM_GetDeviceModuleType(unit_id, module_type))
            return FALSE;

        for (j=0; j<SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT; j++)
            if (module_presented[j] && module_type[j] == STKTPLG_PORT_TYPE_STACKING)
            {
                return TRUE;
            }
            else
            {
                *entIndex = PORT_ENT_START;
                return TRUE;
            }
    }

    if (!STKTPLG_OM_GetNumberOfUnit(&unit_num))
    {
        return FALSE;
    }
    for (i=1; i<=unit_num; i++)
    {
        if ( index <= i * INDEX_NUM_PER_UNIT)
        {
            *entIndex = i * INDEX_NUM_PER_UNIT + 1;
            return TRUE;
        }

        if ( index < (i + 1) * INDEX_NUM_PER_UNIT)
        {
            unit_id = index / INDEX_NUM_PER_UNIT;
            if (!STKTPLG_OM_UnitExist(unit_id))
            {
                return FALSE;
            }

            if (!STKTPLG_OM_GetMaxPortCapability(unit_id, &port_num))
            {
                return FALSE;
            }

            if (index > (unit_id * INDEX_NUM_PER_UNIT + (port_num)))
            {
                if (!STKTPLG_OM_UnitExist(unit_id+1))
                {
                    return FALSE;
                }
                else
                {
                    *entIndex = (i + 1) * INDEX_NUM_PER_UNIT + 1;
                    return TRUE;
                }
            }
            else
            {
                return TRUE;
            }
        }
    }

    return FALSE;
} /* End of STKTPLG_OM_FindNextEntPhysicalIndex */

/* FUNCTION NAME: STKTPLG_OM_IsVaildEntPhysicalIndex
 * PURPOSE: Check if valid EntPhysicalIndex
 * INPUT:  index
 * OUTPUT: None
 * RETURN: TRUE  -- success
 *         FALSE -- failure
 * NOTES:
 *
 */
static BOOL_T  STKTPLG_OM_IsVaildEntPhysicalIndex(I32_T index)
{
    UI32_T   i, j;
    UI32_T   unit_id;
    UI32_T   port_num, unit_num, agent_num;
    UI8_T    module_presented[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];
    UI8_T    module_type[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];

    /* check Stack Name */
    if ( index == STACK_ENT_START)
        return TRUE;

    if ( index < UNIT_ENT_START)
        return FALSE;

    /* check Unit Number */
    if ( index <= UNIT_ENT_END )
    {
        if (!STKTPLG_OM_GetNumberOfUnit(&unit_num))
            return FALSE;

        if (index > (unit_num + UNIT_ENT_START - 1))
            return FALSE;
        else
            return TRUE;
    }

    if ( index < MANAGEMENT_ENT_START)
        return FALSE;

    /* check Manager Number */
    if ( index <= MANAGEMENT_ENT_END )
    {
        agent_num = 1;
        if (index > (agent_num + MANAGEMENT_ENT_START - 1))
            return FALSE;
        else
            return TRUE;
    }

    if ( index < STACK_MODUL_ENT_START)
        return FALSE;

    /* check Stack Module Number */
    if ( index <= STACK_MODUL_ENT_END )
    {
        unit_id = index - STACK_MODUL_ENT_START + 1;
        if (!STKTPLG_OM_UnitExist(unit_id))
            return FALSE;

        if (!STKTPLG_OM_GetDeviceModulePresented(unit_id, module_presented))
            return FALSE;

        if (!STKTPLG_OM_GetDeviceModuleType(unit_id, module_type))
            return FALSE;

        for (j=0; j<SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT; j++)
            if (module_presented[j] && module_type[j] == STKTPLG_PORT_TYPE_STACKING)
                return FALSE;
            else
                return TRUE;
    }

    if (!STKTPLG_OM_GetNumberOfUnit(&unit_num))
        return FALSE;
    for (i=1; i<=unit_num; i++)
    {
        if ( index <= i * INDEX_NUM_PER_UNIT)
            return FALSE;

        if ( index < (i + 1) * INDEX_NUM_PER_UNIT)
        {
            unit_id = index / INDEX_NUM_PER_UNIT;
            if (!STKTPLG_OM_UnitExist(unit_id))
                return FALSE;

            if (!STKTPLG_OM_GetMaxPortCapability(unit_id, &port_num))
                return FALSE;

            if (index > (unit_id * INDEX_NUM_PER_UNIT + (port_num)))
                return FALSE;
            else
                return TRUE;
        }
    }

    /* end of ifindex */
    return FALSE;
}/* End of STKTPLG_OM_IsVaildEntPhysicalIndex */

/* FUNCTION NAME: STKTPLG_OM_MakeOidFromDot
 * PURPOSE: This function will make object ID from given text string.
 * INPUT:  STKTPLG_OM_OID_T *oid_P - Pointer to object ID
 *         I8_T     text_p - Pointer to text string
 * OUTPUT: oid_P->num_components    - number of component in oid_P->component_list
 *         oid_P->component_list    - components will be filled into this array
 * RETURN: BOOL_T Status   - TRUE   : Convert successfully
 *                           FALSE  : Convert failed
 * NOTES:  oid_P->component_list : buffer allocated by caller.
 *
 */
static BOOL_T STKTPLG_OM_MakeOidFromDot(STKTPLG_OM_OID_T *oid_P, I8_T *text_p)
{
   /* LOCAL VARIABLES */
   I8_T    *tp;
   int      no;

   /* BODY */
   if ( oid_P->component_list == 0 )
      return FALSE;
   /* END if */

   for ( tp=text_p, no=0;  ;tp++)
   {
      if ( *tp == '.' || *tp==0)
      {
         if ( no >= STKTPLG_OM_MAX_NUM_OF_COMPONENTS )
            return FALSE;           /* too much component */
         /* END if */
         oid_P->component_list [no++] =  atoi((char*)text_p);
         if ( *tp==0 ) break;
         text_p= tp+1;
      }
   }
   oid_P->num_components = no;
   return TRUE;
} /* STKTPLG_OM_MakeOidFromDot */

/* FUNCTION NAME: STKTPLG_OM_GetEntPhysicalEntry
 * PURPOSE: Get the entPhysicalTable information.
 * INPUT:  *ent_physical_entry.ent_physical_index   -- entity physical index.
 * OUTPUT: *ent_physical_entry  -- information structure of this index.
 * RETURN: TRUE  -- success
 *         FALSE -- failure
 * NOTES:  This function is used by SNMP.
 *
 */
BOOL_T STKTPLG_OM_GetEntPhysicalEntry(STKTPLG_OM_EntPhysicalEntry_T *ent_physical_entry)
{
    STKTPLG_OM_Info_T board_info;
    UI32_T   unit, port, type;
    I32_T    index;
    UI8_T    master_unit_id;

    if(ent_physical_entry == 0)
        return FALSE;

    index = ent_physical_entry->ent_physical_index;
    memset((UI8_T *)ent_physical_entry, 0, sizeof(STKTPLG_OM_EntPhysicalEntry_T));
    ent_physical_entry->ent_physical_index = index;

    STKTPLG_OM_IsVaildEntPhysicalIndex(index);

    if(FALSE == STKTPLG_OM_TranslateEntPhysicalIndexToTypeUnitIndex(index, &type, &unit, &port))
        return FALSE;

    switch(type)
    {
        case ENT_STACK_NAME:
            /* Entity description */
            strncpy((char*) &ent_physical_entry->ent_physical_descr, MIB2_STACK_ENT_DESC_STR, sizeof(ent_physical_entry->ent_physical_descr));

            if(FALSE == STKTPLG_OM_GetMasterUnitId(&master_unit_id))
            {
                return FALSE;
            }

            if(!STKTPLG_OM_GetSysInfo(master_unit_id, &board_info))
                return FALSE;

            switch(board_info.board_id)
            {
                case 0:
                    STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID0);
                    /* modified by Jinhua Wei ,to remove warning ,becaued the variable type doesn't match*/
                    break;
                case 1:
                    STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID1);
                    break;
                case 2:
                    STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID2);
                    break;
                case 3:
                    STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID3);
                    break;
                case 4:
                    STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID4);
                    break;
                case 5:
                    STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID5);
                    break;
                case 6:
                    STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID6);
                    break;
                case 7:
                    STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID7);
                    break;
                default:
                    STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)"");
                    break;
            }

            /* contained in */
            ent_physical_entry->ent_physical_contained_in = 0;
            /* physical class */
            ent_physical_entry->ent_physical_class = MIB2ENT_CLASS_STACK;
            /* physical parent relative position */
            ent_physical_entry->ent_physical_parent_rel_pos = -1;
            /* physical name */
            strncpy((char*) &ent_physical_entry->ent_physical_name, "stack.1", sizeof(ent_physical_entry->ent_physical_name));
            /* hardware revision */
            ent_physical_entry->ent_physical_hardware_rev[0] = 0;
            /* firmware revision */
            ent_physical_entry->ent_physical_firmware_rev[0] = 0;
            /* software revision */
            ent_physical_entry->ent_physical_software_rev[0] = 0;
            /* serial number */
            ent_physical_entry->ent_physical_entry_rw.ent_physical_serial_num[0] = 0;
            /* manufacture name */
            strncpy((char*) &ent_physical_entry->ent_physical_mfg_name, SYS_ADPT_PRODUCT_MANUFACTURER, sizeof(ent_physical_entry->ent_physical_mfg_name));
            /* model name */
            ent_physical_entry->ent_physical_model_name[0] = 0;

            SYSFUN_TakeReadSem(sem_id_static);
            if(memcmp(table_for_entPhysicalEntryRw[LOCAL_UNIT][0].ent_physical_serial_num,(UI8_T *) &(board_info.serial_no),STKTPLG_OM_MIB_STRING_LENGTH + 1)!=0)
                memcpy((UI8_T *) &ent_physical_entry->ent_physical_entry_rw.ent_physical_serial_num,table_for_entPhysicalEntryRw[LOCAL_UNIT][0].ent_physical_serial_num, STKTPLG_OM_MIB_STRING_LENGTH + 1);
            else
                memcpy((UI8_T *) &ent_physical_entry->ent_physical_entry_rw.ent_physical_serial_num, (UI8_T *) &(board_info.serial_no), STKTPLG_OM_MIB_STRING_LENGTH + 1);

            /* alias */ /* Need get string from EEPROM */
            memcpy(ent_physical_entry->ent_physical_entry_rw.ent_physical_alias,table_for_entPhysicalEntryRw[LOCAL_UNIT][0].ent_physical_alias,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            /* alias id */ /* Need get string from EEPROM */
            memcpy(ent_physical_entry->ent_physical_entry_rw.ent_physical_asset_id,table_for_entPhysicalEntryRw[LOCAL_UNIT][0].ent_physical_asset_id,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            SYSFUN_GiveReadSem(sem_id_static);
            /* is FRU */
            ent_physical_entry->ent_physical_is_fru = MIB2ENT_FRU_TRUE;
            break;

        case ENT_UNIT_NUM:
            sprintf((char*) &ent_physical_entry->ent_physical_descr, MIB2_UNIT_ENT_DESC_STR, (index - UNIT_ENT_START + 1));
            STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)"0.0");/* modified by Jinhua Wei ,to remove warning ,becaued the variable never used */
            ent_physical_entry->ent_physical_contained_in = STACK_ENT_START;
            ent_physical_entry->ent_physical_class = MIB2ENT_CLASS_CONTAINER;
            ent_physical_entry->ent_physical_parent_rel_pos = index - UNIT_ENT_START + 1;
            sprintf((char*) &ent_physical_entry->ent_physical_name, "unit.%ld", (index - UNIT_ENT_START + 1));
            if (!STKTPLG_OM_GetSysInfo((index - UNIT_ENT_START + 1), &board_info))
                return FALSE;

            strncpy((char*) &ent_physical_entry->ent_physical_hardware_rev, (char*) &(board_info.mainboard_hw_ver), sizeof(ent_physical_entry->ent_physical_hardware_rev));
            ent_physical_entry->ent_physical_firmware_rev[0] = 0;
            ent_physical_entry->ent_physical_software_rev[0] = 0;

            SYSFUN_TakeReadSem(sem_id_static);
            if(memcmp(table_for_entPhysicalEntryRw[LOCAL_UNIT][1].ent_physical_serial_num,(UI8_T *) &(board_info.serial_no),STKTPLG_OM_MIB_STRING_LENGTH + 1)!=0)
                memcpy((UI8_T *) &ent_physical_entry->ent_physical_entry_rw.ent_physical_serial_num,table_for_entPhysicalEntryRw[LOCAL_UNIT][1].ent_physical_serial_num, STKTPLG_OM_MIB_STRING_LENGTH + 1);
            else
                memcpy((UI8_T *) &ent_physical_entry->ent_physical_entry_rw.ent_physical_serial_num, (UI8_T *) &(board_info.serial_no), STKTPLG_OM_MIB_STRING_LENGTH + 1);
            SYSFUN_GiveReadSem(sem_id_static);

            strncpy((char*) &ent_physical_entry->ent_physical_mfg_name, SYS_ADPT_PRODUCT_MANUFACTURER, sizeof(ent_physical_entry->ent_physical_mfg_name));

            switch(board_info.board_id)
            {
                case 0:
                    strncpy((char*) &ent_physical_entry->ent_physical_model_name, SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID0, sizeof(ent_physical_entry->ent_physical_model_name) );
                    break;
                case 1:
                    strncpy((char*) &ent_physical_entry->ent_physical_model_name, SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID1, sizeof(ent_physical_entry->ent_physical_model_name) );
                    break;
                case 2:
                    strncpy((char*) &ent_physical_entry->ent_physical_model_name, SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID2, sizeof(ent_physical_entry->ent_physical_model_name) );
                    break;
                case 3:
                    strncpy((char*) &ent_physical_entry->ent_physical_model_name, SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID3, sizeof(ent_physical_entry->ent_physical_model_name) );
                    break;
                case 4:
                    strncpy((char*) &ent_physical_entry->ent_physical_model_name, SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID4, sizeof(ent_physical_entry->ent_physical_model_name) );
                    break;
                case 5:
                    strncpy((char*) &ent_physical_entry->ent_physical_model_name, SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID5, sizeof(ent_physical_entry->ent_physical_model_name) );
                    break;
                case 6:
                    strncpy((char*) &ent_physical_entry->ent_physical_model_name, SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID6, sizeof(ent_physical_entry->ent_physical_model_name) );
                    break;
                case 7:
                    strncpy((char*) &ent_physical_entry->ent_physical_model_name, SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID7, sizeof(ent_physical_entry->ent_physical_model_name) );
                    break;
                default:
                    strncpy((char*) &ent_physical_entry->ent_physical_model_name, "\0", sizeof(ent_physical_entry->ent_physical_model_name) );
                    break;
            }

            SYSFUN_TakeReadSem(sem_id_static);
            /* Need get string from EEPROM */
            memcpy(ent_physical_entry->ent_physical_entry_rw.ent_physical_alias,table_for_entPhysicalEntryRw[LOCAL_UNIT][1].ent_physical_alias,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            /* Need get string from EEPROM */
            memcpy(ent_physical_entry->ent_physical_entry_rw.ent_physical_asset_id,table_for_entPhysicalEntryRw[LOCAL_UNIT][1].ent_physical_asset_id,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            SYSFUN_GiveReadSem(sem_id_static);
            ent_physical_entry->ent_physical_is_fru = MIB2ENT_FRU_FALSE;
            break;

        case ENT_MANAGEMENT_PORT:
            if (index == MANAGEMENT_ENT_START)
            {
                strncpy((char*) &ent_physical_entry->ent_physical_descr, MIB2_STACK_MGT_ENT_DESC_STR, sizeof(ent_physical_entry->ent_physical_descr) );
            }
            else
            {
                sprintf((char*) &ent_physical_entry->ent_physical_descr, MIB2_BACK_MGT_NET_DESC_STR, (index - MANAGEMENT_ENT_START));
            }

            STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)"0.0");
            ent_physical_entry->ent_physical_contained_in = UNIT_ENT_START; /* For standalone */
            ent_physical_entry->ent_physical_class = MIB2ENT_CLASS_MODULE;
            ent_physical_entry->ent_physical_parent_rel_pos = index - MANAGEMENT_ENT_START + 1;
            sprintf((char*) &ent_physical_entry->ent_physical_name, "stackmanaget.%ld", (index - MANAGEMENT_ENT_START + 1));

            if (!STKTPLG_OM_GetSysInfo((index - MANAGEMENT_ENT_START + 1), &board_info))
                return FALSE;

            strncpy((char*) &ent_physical_entry->ent_physical_hardware_rev, (char*) &(board_info.agent_hw_ver), sizeof(ent_physical_entry->ent_physical_hardware_rev));
            strncpy((char*) &ent_physical_entry->ent_physical_firmware_rev, (char*) &(board_info.runtime_sw_ver), sizeof(ent_physical_entry->ent_physical_firmware_rev));
            ent_physical_entry->ent_physical_software_rev[0] = 0;

            /* show mac address in ent_physical_serial_num field */
            SYSFUN_TakeReadSem(sem_id_static);
            if(memcmp(table_for_entPhysicalEntryRw[LOCAL_UNIT][2].ent_physical_serial_num,(UI8_T *) &(board_info.serial_no),STKTPLG_OM_MIB_STRING_LENGTH + 1)!=0)
                memcpy((UI8_T *) &ent_physical_entry->ent_physical_entry_rw.ent_physical_serial_num,table_for_entPhysicalEntryRw[LOCAL_UNIT][2].ent_physical_serial_num, STKTPLG_OM_MIB_STRING_LENGTH + 1);
            else
                memcpy((UI8_T *) &ent_physical_entry->ent_physical_entry_rw.ent_physical_serial_num, (UI8_T *) &(board_info.serial_no), STKTPLG_OM_MIB_STRING_LENGTH + 1);

            strncpy((char*) &ent_physical_entry->ent_physical_mfg_name, SYS_ADPT_PRODUCT_MANUFACTURER, sizeof(ent_physical_entry->ent_physical_mfg_name));
            ent_physical_entry->ent_physical_model_name[0] = 0;
            /* Need get string from EEPROM */
            memcpy(ent_physical_entry->ent_physical_entry_rw.ent_physical_alias,table_for_entPhysicalEntryRw[LOCAL_UNIT][2].ent_physical_alias,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            /* Need get string from EEPROM */
            memcpy(ent_physical_entry->ent_physical_entry_rw.ent_physical_asset_id,table_for_entPhysicalEntryRw[LOCAL_UNIT][2].ent_physical_asset_id,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            SYSFUN_GiveReadSem(sem_id_static);
            ent_physical_entry->ent_physical_is_fru = MIB2ENT_FRU_FALSE;
            break;

        case ENT_STACK_CARD:
            sprintf((char*) &ent_physical_entry->ent_physical_descr, MIB2_STACK_CARD_DESC_STR, unit);
            STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)"0.0");
            ent_physical_entry->ent_physical_contained_in = unit + UNIT_ENT_START - 1;
            ent_physical_entry->ent_physical_class = MIB2ENT_CLASS_MODULE;
            ent_physical_entry->ent_physical_parent_rel_pos = 1;
            strncpy((char*) &ent_physical_entry->ent_physical_name, "module.1", sizeof(ent_physical_entry->ent_physical_name));
            ent_physical_entry->ent_physical_hardware_rev[0] = 0;
            ent_physical_entry->ent_physical_firmware_rev[0] = 0;
            ent_physical_entry->ent_physical_software_rev[0] = 0;
            ent_physical_entry->ent_physical_entry_rw.ent_physical_serial_num[0] = 0;
            strncpy((char*) &ent_physical_entry->ent_physical_mfg_name, SYS_ADPT_PRODUCT_MANUFACTURER, sizeof(ent_physical_entry->ent_physical_mfg_name));
            ent_physical_entry->ent_physical_model_name[0] = 0;
            /* Need get string from EEPROM */
            ent_physical_entry->ent_physical_entry_rw.ent_physical_alias[0] = 0;
            /* Need get string from EEPROM */
            ent_physical_entry->ent_physical_entry_rw.ent_physical_asset_id[0] = 0;
            ent_physical_entry->ent_physical_is_fru = MIB2ENT_FRU_TRUE;
            break;

        case ENT_PHYSICAL_PORT:
            sprintf((char*) &ent_physical_entry->ent_physical_descr, MIB2_PORT_ENT_DESC_STR, port, unit);
            STKTPLG_OM_MakeOidFromDot(&ent_physical_entry->ent_physical_vendor_type, (I8_T *)"0.0");
            ent_physical_entry->ent_physical_contained_in = unit + UNIT_ENT_START - 1;
            ent_physical_entry->ent_physical_class = MIB2ENT_CLASS_PORTS;
            ent_physical_entry->ent_physical_parent_rel_pos = port;
            sprintf((char*) &ent_physical_entry->ent_physical_name, MIB2_PORT_IF_DESC_STR, port, unit);
            ent_physical_entry->ent_physical_hardware_rev[0] = 0;
            ent_physical_entry->ent_physical_firmware_rev[0] = 0;
            ent_physical_entry->ent_physical_software_rev[0] = 0;
            /* show mac address in ent_physical_serial_num field */
            SYSFUN_TakeReadSem(sem_id_static);
            memcpy((UI8_T *) &ent_physical_entry->ent_physical_entry_rw.ent_physical_serial_num,table_for_entPhysicalEntryRw[unit-1][(port-1)+3].ent_physical_serial_num, STKTPLG_OM_MIB_STRING_LENGTH + 1);
            strncpy((char*) &ent_physical_entry->ent_physical_mfg_name, SYS_ADPT_PRODUCT_MANUFACTURER, sizeof(ent_physical_entry->ent_physical_mfg_name));
            ent_physical_entry->ent_physical_model_name[0] = 0;
            /* Need get string from EEPROM */
            memcpy((UI8_T *) &ent_physical_entry->ent_physical_entry_rw.ent_physical_alias,table_for_entPhysicalEntryRw[unit-1][(port-1)+3].ent_physical_alias, STKTPLG_OM_MIB_STRING_LENGTH + 1);
            /* Need get string from EEPROM */
            memcpy((UI8_T *) &ent_physical_entry->ent_physical_entry_rw.ent_physical_asset_id,table_for_entPhysicalEntryRw[unit-1][(port-1)+3].ent_physical_asset_id, STKTPLG_OM_MIB_STRING_LENGTH + 1);
            SYSFUN_GiveReadSem(sem_id_static);
            ent_physical_entry->ent_physical_is_fru = (UI8_T) MIB2ENT_FRU_TRUE;
            break;
    }
    return TRUE;
} /* End of STKTPLG_OM_GetEntPhysicalEntry */

/* FUNCTION NAME: STKTPLG_OM_GetNextEntPhysicalEntry
 * PURPOSE: Get next the entPhysicalTable information.
 * INPUT:  *ent_physical_entry.ent_physical_index   -- entity physical index.
 * OUTPUT: *ent_physical_entry  -- information structure of this next index.
 * RETURN: TRUE  -- success
 *         FALSE -- failure
 * NOTES:  This function is used by SNMP.
 *
 */
BOOL_T STKTPLG_OM_GetNextEntPhysicalEntry(STKTPLG_OM_EntPhysicalEntry_T *ent_physical_entry)
{
    BOOL_T  ret;
    I32_T   index;

    if(ent_physical_entry == NULL)
        return FALSE;

    index = ent_physical_entry->ent_physical_index;
    ret = STKTPLG_OM_FindNextEntPhysicalIndex(&index); /* static function */
    if(ret == FALSE)
        return FALSE;

    ent_physical_entry->ent_physical_index = index;

    return STKTPLG_OM_GetEntPhysicalEntry(ent_physical_entry);
} /* End of STKTPLG_OM_GetNextEntPhysicalEntry */

/* FUNCTION NAME: STKTPLG_OM_SetEntPhysicalAlias
 * PURPOSE: Set the entPhysicalAlias field.
 * INPUT:   ent_physical_index  -- entity physical index.
 *          *ent_physical_alias_p -- entity physical index.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 */
BOOL_T STKTPLG_OM_SetEntPhysicalAlias(I32_T ent_physical_index, UI8_T *ent_physical_alias_p)
{
    UI32_T   unit, port, type;

    STKTPLG_OM_IsVaildEntPhysicalIndex(ent_physical_index);

    if(FALSE == STKTPLG_OM_TranslateEntPhysicalIndexToTypeUnitIndex(ent_physical_index, &type, &unit, &port))
        return FALSE;

    SYSFUN_TakeWriteSem(sem_id_static);
    switch (type)
    {
        case ENT_STACK_NAME:
            memcpy(table_for_entPhysicalEntryRw[LOCAL_UNIT][0].ent_physical_alias,ent_physical_alias_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
        case ENT_UNIT_NUM:
            memcpy(table_for_entPhysicalEntryRw[LOCAL_UNIT][1].ent_physical_alias,ent_physical_alias_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
        case ENT_MANAGEMENT_PORT:
            memcpy(table_for_entPhysicalEntryRw[LOCAL_UNIT][2].ent_physical_alias,ent_physical_alias_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
        case ENT_STACK_CARD:
            break;
        case ENT_PHYSICAL_PORT:
            memcpy(table_for_entPhysicalEntryRw[unit-1][(port-1)+3].ent_physical_alias,ent_physical_alias_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
    }
    SYSFUN_GiveWriteSem(sem_id_static);
    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_SetEntPhysicalSeriaNum
 * PURPOSE: Set the EntPhysicalSeriaNum field.
 * INPUT:   ent_physical_index  -- entity physical index.
 *          *ent_physical_seriaNum_p -- entity physical serial num.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 */
BOOL_T STKTPLG_OM_SetEntPhysicalSeriaNum(I32_T ent_physical_index, UI8_T *ent_physical_seriaNum_p)
{
    UI32_T   unit, port, type;

    STKTPLG_OM_IsVaildEntPhysicalIndex(ent_physical_index);
    if(FALSE == STKTPLG_OM_TranslateEntPhysicalIndexToTypeUnitIndex(ent_physical_index, &type, &unit, &port))
        return FALSE;

    SYSFUN_TakeWriteSem(sem_id_static);
    switch (type)
    {
        case ENT_STACK_NAME:
            memcpy(&table_for_entPhysicalEntryRw[LOCAL_UNIT][0].ent_physical_serial_num,ent_physical_seriaNum_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
        case ENT_UNIT_NUM:
            memcpy(&table_for_entPhysicalEntryRw[LOCAL_UNIT][1].ent_physical_serial_num,ent_physical_seriaNum_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
        case ENT_MANAGEMENT_PORT:
            memcpy(&table_for_entPhysicalEntryRw[LOCAL_UNIT][2].ent_physical_serial_num,ent_physical_seriaNum_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
        case ENT_STACK_CARD:
            break;
        case ENT_PHYSICAL_PORT:
            memcpy(table_for_entPhysicalEntryRw[unit-1][(port-1)+3].ent_physical_serial_num,ent_physical_seriaNum_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
    }
    SYSFUN_GiveWriteSem(sem_id_static);
    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_SetEntPhysicalAssetId
 * PURPOSE: Set the entPhysicalAlias field.
 * INPUT:   ent_physical_index  -- entity physical index.
 *          *ent_physical_asset_id_p -- entity physical asset id.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 */
BOOL_T STKTPLG_OM_SetEntPhysicalAssetId(I32_T ent_physical_index, UI8_T *ent_physical_asset_id_p)
{
    UI32_T   unit, port, type;

    STKTPLG_OM_IsVaildEntPhysicalIndex(ent_physical_index);
    if(FALSE == STKTPLG_OM_TranslateEntPhysicalIndexToTypeUnitIndex(ent_physical_index, &type, &unit, &port))
        return FALSE;

    SYSFUN_TakeWriteSem(sem_id_static);
    switch (type)
    {
        case ENT_STACK_NAME:
            memcpy(table_for_entPhysicalEntryRw[LOCAL_UNIT][0].ent_physical_asset_id,ent_physical_asset_id_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
        case ENT_UNIT_NUM:
            memcpy(table_for_entPhysicalEntryRw[LOCAL_UNIT][1].ent_physical_asset_id,ent_physical_asset_id_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
        case ENT_MANAGEMENT_PORT:
            memcpy(table_for_entPhysicalEntryRw[LOCAL_UNIT][2].ent_physical_asset_id,ent_physical_asset_id_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
        case ENT_STACK_CARD:
            break;
        case ENT_PHYSICAL_PORT:
            memcpy(table_for_entPhysicalEntryRw[unit-1][(port-1)+3].ent_physical_asset_id,ent_physical_asset_id_p,STKTPLG_OM_MIB_STRING_LENGTH + 1);
            break;
    }
    SYSFUN_GiveWriteSem(sem_id_static);
    return TRUE;
}

/* FUNCTION NAME: STKTPLG_OM_GetPortFigureType
 * PURPOSE: Get the port figure type of the specified unit and port.
 * INPUT:   unit -- unit id
 *          port -- port id
 * OUTPUT:  port_figure_type_p -- port figure type
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 */
BOOL_T STKTPLG_OM_GetPortFigureType(UI32_T unit, UI32_T port, STKTPLG_TYPE_Port_Figure_Type_T *port_figure_type_p)
{
    UI8_T board_id;

    if(unit <1 || unit >SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;

    board_id = stk_unit_cfg_snapshot[(UI8_T)unit-1].board_info.board_id;

    return STKTPLG_BOARD_GetPortFigureType(board_id, port, port_figure_type_p);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_OM_GetStackingPortPhyDevPortId
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the physical device id and port id of the specified stacking port
 *          type.
 * INPUT:   port_type   - up link or down link stacking port
 * OUTPUT:  device_id_p - physical device id of the specified port
 *          phy_port_p  - physical port id of the specified port
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetStackingPortPhyDevPortId(
    STKTPLG_TYPE_STACKING_PORT_TYPE_T port_type,
    UI32_T *device_id_p,
    UI32_T *phy_port_p)
{
#if (SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    #if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    if (ctrl_info.stacking_port_option == STKTPLG_TYPE_STACKING_PORT_OPTION_OFF)
        return FALSE;
    else
    {
        return STKTPLG_BOARD_GetStackingPortPhyDevPortId(port_type,
            ctrl_info.stacking_port_option,
            device_id_p, phy_port_p);
    }
    #else
    return STKTPLG_BOARD_GetStackingPortPhyDevPortId(port_type,
        device_id_p, phy_port_p);
    #endif
#else
    return FALSE;
#endif
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_OM_GetLocalCpuEnabledDevId
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the physical device id of the local unit that CPU enabled.
 * INPUT:   None
 * OUTPUT:  device_id_p - physical device id of the local unit that CPU enabled.
 * RETUEN:  None
 * NOTES:   This function will not be used in Broadcom chip project.
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetLocalCpuEnabledDevId(UI8_T *device_id_p)
{
#if (SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    return STKTPLG_BOARD_GetLocalCpuEnabledDevId(device_id_p);
#else
    *device_id_p = 255;
    return TRUE;
#endif
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_OM_GetCpuEnabledDevId
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the physical device id of the specified board id that
 *          CPU enabled.
 * INPUT:   board_id    - board ID for this unit, it is project independent.
 * OUTPUT:  device_id_p - physical device id of the specified board
                          id that CPU enabled
 * RETUEN:  None
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetCpuEnabledDevId(
    UI8_T board_id,
    UI8_T *device_id_p)
{

#if (SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    return STKTPLG_BOARD_GetCpuEnabledDevId(board_id, device_id_p);
#else
    *device_id_p = 255;
    return TRUE;
#endif
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_OM_SetStackingPortOption
 *---------------------------------------------------------------------------------
 * PURPOSE: Set the stacking port of user configuration to the specified option.
 * INPUT:   option - stacking port option 1, 2 or off.
 * OUTPUT:  None
 * RETUEN:  TRUE   - success.
 *          FALSE  - error
 * NOTES:
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_SetStackingPortOption(
    STKTPLG_TYPE_Stacking_Port_Option_T option)
{
#if (SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    if (option >= STKTPLG_TYPE_TOTAL_NBR_OF_STACKING_PORT_OPTION)
    {
        printf("%s(%d):Invalid stacking port option=%d\r\n", __FUNCTION__, __LINE__,
            (int)option);
        return FALSE;
    }
    ctrl_info.stacking_port_option = option;
    return TRUE;
#else
    return FALSE;
#endif
}

/* FUNCTION NAME: STKTPLG_OM_GetCPLDType
 * PURPOSE: Get CPLD Type.
 * INPUT:   NONE
 * RETURN:  cpld type
 */
UI32_T STKTPLG_OM_GetCPLDType(void)
{
#if SYS_HWCFG_CPLD_TYPE != SYS_HWCFG_CPLD_TYPE_NONE
    return STKTPLG_BOARD_GetCPLDType();
#else
    return SYS_HWCFG_CPLD_TYPE_NONE;
#endif
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_OM_GetStackMaxUnitID
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the max number of unit id existing in the stack.
 * INPUT:   None
 * OUTPUT:  max_unit_id_p - the max number of unit id
 * RETUEN:  TRUE   - success.
 *          FALSE  - error
 * NOTES:
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetStackMaxUnitID(UI32_T *max_unit_id_p)
{
    UI32_T temp_max_unit_id = 0, unit = 0;

    while (STKTPLG_OM_GetNextUnit(&unit))
    {
        if (temp_max_unit_id < unit)
            temp_max_unit_id = unit;
    }
    if (temp_max_unit_id == 0)
        return FALSE;
    *max_unit_id_p = temp_max_unit_id;
    return TRUE;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_OM_CreateTapDevice
 *---------------------------------------------------------------------------------
 * PURPOSE: Create a tap device with the given name
 * INPUT:   devname_p - the name of the tap device to be created
 * OUTPUT:  None
 * RETUEN:  TRUE   - success.
 *          FALSE  - error
 * NOTES: The maximum string length of devname_p is IFNAMSIZ.
 *---------------------------------------------------------------------------------
 */
static BOOL_T STKTPLG_OM_CreateTapDevice(const char* devname_p)
{
    struct ifreq ifr;
    int fd=-1, rc=0;
    BOOL_T ret_val=TRUE;

    /* sanity check for devname_p
     */
    if ((devname_p==NULL) || (*devname_p=='\0'))
    {
        printf("Invalid devname_p.\r\n");
        return FALSE;
    }

    /* sanity check for length of devname_p
     */
    if (strlen(devname_p)>IFNAMSIZ)
    {
        printf("Length of devname_p too long.\r\n");
        return FALSE;
    }

    if ( (fd = open("/dev/net/tun", O_RDWR)) < 0 )
    {
        printf("Failed to open /dev/net/tun.(rc=%d)\r\n", fd);
        return FALSE;
    }

    memset(&ifr, 0, sizeof(ifr));
    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    ifr.ifr_flags = IFF_TAP;

    strncpy(ifr.ifr_name, devname_p, IFNAMSIZ);

    if ( (rc = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 )
    {
        printf("TUNSETIFF err(%s,rc=%d).\r\n", devname_p, rc);
        ret_val=FALSE;
        goto exit;
    }

    if (strcmp(devname_p, ifr.ifr_name)!=0)
    {
        printf("Warning. Expected devname=%s, Real devname=%s.\r\n",
            devname_p, ifr.ifr_name);
    }


    rc = ioctl (fd, TUNSETNOCSUM, 1);
    if (rc<0)
    {
        printf("TUNSETNOCSUM err(%s,rc=%d).\r\n", devname_p, rc);
        ret_val=FALSE;
        goto exit;
    }

    rc = ioctl (fd, TUNSETPERSIST, 1);
    if (rc<0)
    {
        printf("TUNSETPERSIST err(%s,rc=%d).\r\n", devname_p, rc);
        ret_val=FALSE;
        goto exit;
    }

exit:
    if (fd>=0)
        close(fd);

    return ret_val;
}

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_HwPortMode_UpdatePortMapping
 * -------------------------------------------------------------------------
 * PURPOSE : To sync port mapping from board info
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_HwPortMode_UpdatePortMapping(void)
{
    STKTPLG_BOARD_BoardInfo_T  board_info;
    UI8_T board_id;
    UI32_T unit;

    SYSFUN_TakeReadSem(sem_id_static);
    unit = ctrl_info.my_unit_id;
    board_id = stk_unit_cfg[unit-1].board_info.board_id;
    SYSFUN_GiveReadSem(sem_id_static);

    if (!STKTPLG_BOARD_GetBoardInformation(board_id, &board_info))
    {
        return FALSE;
    }

    SYSFUN_TakeWriteSem(sem_id_static);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    memcpy(&stackPortMappingTable[unit-1], &board_info.userPortMappingTable[0], SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*sizeof(DEV_SWDRV_Device_Port_Mapping_T));
#endif
    SYSFUN_GiveWriteSem(sem_id_static);

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_SetAllOperHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To set all HW port mode oper status
 * INPUT   : unit
 *           oper_hw_port_mode_ar
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_SetAllOperHwPortMode(UI32_T unit, UI8_T oper_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT])
{
    if (unit == SYS_VAL_LOCAL_UNIT_ID)
    {
        unit = ctrl_info.my_unit_id;
    }

    if (unit < 1 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    memcpy(stk_unit_cfg[unit-1].oper_hw_port_mode, oper_hw_port_mode_ar, sizeof(stk_unit_cfg[unit-1].oper_hw_port_mode));

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_GetAllCfgHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To get all HW port mode config
 * INPUT   : unit
 * OUTPUT  : cfg_hw_port_mode_ar
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetAllCfgHwPortMode(UI32_T unit, UI8_T cfg_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT])
{
    if (unit == SYS_VAL_LOCAL_UNIT_ID)
    {
        unit = ctrl_info.my_unit_id;
    }

    if (unit < 1 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    memcpy(cfg_hw_port_mode_ar, stk_unit_cfg[unit-1].cfg_hw_port_mode, sizeof(stk_unit_cfg[unit-1].cfg_hw_port_mode));

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_SetAllCfgHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To set all HW port mode config
 * INPUT   : unit
 *           cfg_hw_port_mode_ar
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_SetAllCfgHwPortMode(UI32_T unit, UI8_T cfg_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT])
{
    if (unit == SYS_VAL_LOCAL_UNIT_ID)
    {
        unit = ctrl_info.my_unit_id;
    }

    if (unit < 1 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    memcpy(stk_unit_cfg[unit-1].cfg_hw_port_mode, cfg_hw_port_mode_ar, sizeof(stk_unit_cfg[unit-1].cfg_hw_port_mode));

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_GetCfgHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To get HW port mode config
 * INPUT   : unit
 *           port
 * OUTPUT  : hw_port_mode_p
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetCfgHwPortMode(UI32_T unit, UI32_T port, STKTPLG_TYPE_HwPortMode_T *hw_port_mode_p)
{
    if (unit == SYS_VAL_LOCAL_UNIT_ID)
    {
        unit = ctrl_info.my_unit_id;
    }

    if (unit < 1 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (port < 1 || port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    *hw_port_mode_p = stk_unit_cfg[unit-1].cfg_hw_port_mode[port-1];

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_SetCfgHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To set HW port mode config
 * INPUT   : unit
 *           port
 *           hw_port_mode
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_SetCfgHwPortMode(UI32_T unit, UI32_T port, STKTPLG_TYPE_HwPortMode_T hw_port_mode)
{
    if (unit == SYS_VAL_LOCAL_UNIT_ID)
    {
        unit = ctrl_info.my_unit_id;
    }

    if (unit < 1 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (port < 1 || port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    SYSFUN_TakeWriteSem(sem_id_static);
    stk_unit_cfg[unit-1].cfg_hw_port_mode[port-1] = hw_port_mode;
    SYSFUN_GiveWriteSem(sem_id_static);

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_GetHwNextPortModeEntry
 * -------------------------------------------------------------------------
 * PURPOSE : To get HW port mode entry for UI
 * INPUT   : hw_port_mode_p->unit
 *           hw_port_mode_p->port_info
 * OUTPUT  : hw_port_mode_p
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetHwNextPortModeEntry(STKTPLG_TYPE_HwPortModeEntry_T *hw_port_mode_entry_p)
{
    static struct {
        STKTPLG_TYPE_HwPortMode_T hw_port_mode;
        UI32_T port_num;
        UI32_T port_type;
    } hw_port_mode_list[] = {
        { STKTPLG_TYPE_HW_PORT_MODE_1x100G, 1,  VAL_portType_hundredGBaseQSFP },
        { STKTPLG_TYPE_HW_PORT_MODE_4x25G,  4,  VAL_portType_twentyFiveGBaseSFP },
        { STKTPLG_TYPE_HW_PORT_MODE_1x40G,  1,  VAL_portType_fortyGBaseQSFP },
        { STKTPLG_TYPE_HW_PORT_MODE_4x10G,  4,  VAL_portType_tenGBaseSFP },
    };

    STKTPLG_BOARD_HwPortModeInfo_T hw_port_mode_info;
    UI32_T unit;
    UI32_T port;
    UI32_T board_id;
    int port_info_count;
    int i;

    unit = hw_port_mode_entry_p->unit;
    port = hw_port_mode_entry_p->port_info[0].port_start +
           hw_port_mode_entry_p->port_info[0].port_num;

    if (!STKTPLG_OM_UnitExist(unit))
    {
        goto get_next_unit;
    }

    do
    {
        if (!STKTPLG_OM_GetUnitBoardID(unit, &board_id))
        {
            goto get_next_unit;
        }

        for (; port <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port++)
        {
            port_info_count = 0;

            for (i = 0; i < sizeof(hw_port_mode_list)/sizeof(*hw_port_mode_list); i++)
            {
                if (!STKTPLG_BOARD_GetHwPortModeInfo(board_id, port, hw_port_mode_list[i].hw_port_mode, &hw_port_mode_info))
                {
                    continue;
                }

                if (port_info_count == 0)
                {
                    if (hw_port_mode_info.mapping_uport < port)
                    {
                        /* this port should be merged with last entry, so ignore it.
                         */
                        goto get_next_port;
                    }

                    /* entry init
                     */
                    memset(hw_port_mode_entry_p, 0, sizeof(*hw_port_mode_entry_p));
                    hw_port_mode_entry_p->unit = unit;
                }

                hw_port_mode_entry_p->port_info[port_info_count].port_start = hw_port_mode_info.mapping_uport;
                hw_port_mode_entry_p->port_info[port_info_count].port_num = hw_port_mode_list[i].port_num;
                hw_port_mode_entry_p->port_info[port_info_count].port_type = hw_port_mode_list[i].port_type;
                hw_port_mode_entry_p->supported_hw_port_mode |= BIT_VALUE(hw_port_mode_list[i].hw_port_mode);

                port_info_count++;
            } /* end of for (i) */

            if (port_info_count > 0)
            {
                hw_port_mode_entry_p->port_info_count = port_info_count;
                hw_port_mode_entry_p->cfg_hw_port_mode = stk_unit_cfg[unit-1].cfg_hw_port_mode[port-1];
                hw_port_mode_entry_p->oper_hw_port_mode = stk_unit_cfg[unit-1].oper_hw_port_mode[port-1];

                /* if oper status is not equal to default status,
                 * user might be confused by cfg_hw_port_mode == INVALID,
                 * so rewrite cfg_hw_port_mode with exact value.
                 */
                if (hw_port_mode_entry_p->cfg_hw_port_mode == STKTPLG_TYPE_HW_PORT_MODE_UNSPECIFIED &&
                    hw_port_mode_entry_p->oper_hw_port_mode != hw_port_mode_info.dflt_hw_port_mode)
                {
                    hw_port_mode_entry_p->cfg_hw_port_mode = hw_port_mode_info.dflt_hw_port_mode;
                }

                return TRUE;
            }

get_next_port:
            ;
        } /* end of for (port) */

get_next_unit:
        port = 1; /* reset port */
    }
    while (STKTPLG_OM_GetNextUnit(&unit));

    return FALSE; /* no next entry */
}
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

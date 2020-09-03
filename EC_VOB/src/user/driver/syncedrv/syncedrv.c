/* MODULE NAME:  syncedrv.c
 * PURPOSE:
 *   This module provides APIs for operations that is related to synchronous
 *   ethernet.
 *
 * NOTES:
 *   This module does not support stacking.
 *
 *   It is possible that some of the reference inputs to synce ASIC are not
 *   allowed to use. The concept of the logical reference input index(start from 0)
 *   is adopted to act as an abstract layer among all of the projects. The
 *   mapping between logical reference input index and physical reference input
 *   index(start from 0) must be defined in SYS_HWCFG_SYNCE_CHIP_USABLE_PHYISCAL_REF_ID_ARRAY
 *   For example, if there are 6 reference input available, but only physical ref
 *   index 2,3,4 and 5 are allowed to be used, SYS_HWCFG_SYNCE_CHIP_USABLE_PHYISCAL_REF_ID_ARRAY
 *   shall be defined as {2,3,4,5}
 *
 *   There are 3 levels of priority concept.
 *   The highest level -- user priority: The priority given by the end user. The
 *                        priority kept in SYNCEDRV_OM is user priority. Lower
 *                        value gets higher priority.
 *   The second level  -- logical priority: The priority passed to SYNCEDRV_DoSetSyncEClockSourcePriorityByRefId
 *                        The valid range of logical priority is from 1 to SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC.
 *                        Lower value gets higher priority.
 *   The third level   -- physical priority: The priority set to SyncE ASIC. The
 *                        valid range depends on the ASIC. Higher or Lower value
 *                        get higher priority also depends on the ASIC.
 *
 *   The constraints of binding a reference input to a clock source port might
 *   be different among projects. So the binding of reference input and clock
 *   source port will be impelemnted in project dependent functions. For example,
 *   on ASF4512MP, any port that support synce can bind to any reference input of
 *   synce ASIC. On ECS4810-12MV2, the ports on the first PHY can only bind to
 *   physical reference input index 2(IN3 of IDT92V3399) or 5(IN6 of IDT92V3399).
 *
 *   Function naming convention:
 *       Chip dependent functions will be defined as SYNCEDRV_XXX_CHIPNAME()
 *       and use a MACRO to redefine SYNCEDRV_XXX() to SYNCEDRV_XXX()
 *
 *       Project depedent functions will be defined as __SYNCEDRV_YYY()
 *       and use a MACRO to redefine SYNCEDRV_YYY() to ___SYNCEDRV_YYY()
 *
 *
 * LIMITATION: All of the code that use #if((SYS_CPNT_SYNCE_SUPPORT_MEDIA & SYS_CPNT_SYNCE_SUPPORT_MEDIA_1000BASE_T)!=0)
 *             can only work on the board that all of the port that support
 *             synce have the same port type. If there are more than one port
 *             type on ports that support synce , need to call API to check the
 *             port type.
 *
 * About 1000BaseT Force Mode settings(Apply to copper ports only) on
 * clock-source port and non-clock source port:
 *     When the synce enabled port is a clock-source port, syncedrv will set
 *     1000BaseT Force Mode as Force-Slave to ensure the clock can be recovered
 *     from the link partner correctly.
 *     When the synce enabled port is a non-clock-source port, syncedrv should
 *     set 1000BaseT Force Mode as Force-Master to ensure the clock can be
 *     delivered to the link partner correctly. This is not a problem if all of
 *     the synce related settings are done manually. However, consider the case
 *     using ssm for synce related settings, which is described below.
 *       +-------+           +-------+
 *       |Grand  |           |Grand  |
 *       |Master |           |Master |
 *       |Clock 1|           |Clock 2|
 *       +--v----+           +--v----+
 *          |                   |
 *     +----^-----+        +----^-----+
 *     |  Port A  |        |  Port B  |
 *     |  DUT 1   |        |  DUT 2   |
 *     |          |        |          |
 *     |   Port C >--------< Port D   |
 *     +----------+        +----------+   
 *     Given that here are two DUTs, DUT 1 and DUT2, and each DUT is connected
 *     to a port that receives clock from a Grand Master Clock. Port C on DUT1
 *     and Port D on DUT2 are connected. SSM are enabled on Port A,B,C,D.
 *     1000BaseT Force Mode of Port A and Port B will be set as Force-Slave to
 *     receive clock. If 1000BaseT Force Mode of Port C and Port D are both set
 *     as Force-Master, it will not be able to link up. To avoid that problem,
 *     1000BaseT Force Mode of Port C and Port D should be set as Auto.      
 *
 * HISTORY
 *    12/15/2011 - Charlie Chen, Created
 *    10/18/2012 - Charlie Chen, Add API to change chip mode and API to set
 *                               output frequency in ppm under DCO mode.
 *    06/03/2013 - Charlie Chen,
 *        1. Fix the bug that the function
 *           SYNCEDRV_DoGetSyncEClockSourceStatus_ZL30132() will output
 *           incorrect clock source status when real clock source status is
 *           changed from bad to good.
 *        2. To avoid link up problem when running SSM, the setting of
 *           1000BaseT Force Mode on non-clock-source port will be changed from
 *           Slave to Auto.
 *
 * Copyright(C)      EdgeCore Networks, 2011
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_cpnt.h"
#if(SYS_CPNT_SYNCE == TRUE)
#include "sys_hwcfg.h"
#include "l_pbmp.h"
#include "syncedrv.h"
#include "syncedrv_om.h"
#include "dev_swdrv_pmgr.h"
#include "i2cdrv.h"
#include "stktplg_board.h"
#include "stktplg_om.h"
#include "backdoor_mgr.h"
#include "phyaddr_access.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* define SYNCEDRV_DEBUG to turn on debug messages
 */
#define SYNCEDRV_DEBUG

/* chip dependent function definitions
 */
#if (SYS_HWCFG_SYNCE_CHIP==SYS_HWCFG_SYNCE_CHIP_ZL30132)
#define SYNCEDRV_GetChipOperatingMode_ZL30132          SYNCEDRV_GetChipOperatingMode
#define SYNCEDRV_DoSetClockSrcSelectMode               SYNCEDRV_DoSetClockSrcSelectMode_ZL30132
#define SYNCEDRV_DoGetSyncEClockSourceStatus           SYNCEDRV_DoGetSyncEClockSourceStatus_ZL30132
#define SYNCEDRV_DoGetActiveSyncEClockSourceRefId      SYNCEDRV_DoGetActiveSyncEClockSourceRefId_ZL30132
#define SYNCEDRV_DoSetSyncEClockSourcePriorityByRefId  SYNCEDRV_DoSetSyncEClockSourcePriorityByRefId_ZL30132
#define SYNCEDRV_DumpSyncEAsicInfoInternal             SYNCEDRV_DumpSyncEAsicInfo_ZL30132
#define SYNCEDRV_SetChipModeInternal                   SYNCEDRV_SetChipMode_ZL30132
#define SYNCEDRV_SetOutputFreqOffsetInPPMInternal      SYNCEDRV_SetOutputFreqOffsetInPPM_ZL30132
#elif (SYS_HWCFG_SYNCE_CHIP==SYS_HWCFG_SYNCE_CHIP_IDT82V3399)
#define SYNCEDRV_GetChipOperatingMode_IDT82V3399       SYNCEDRV_GetChipOperatingMode
#define SYNCEDRV_DoSetClockSrcSelectMode               SYNCEDRV_DoSetClockSrcSelectMode_IDT82V3399
#define SYNCEDRV_DoGetSyncEClockSourceStatus           SYNCEDRV_DoGetSyncEClockSourceStatus_IDT82V3399
#define SYNCEDRV_DoGetActiveSyncEClockSourceRefId      SYNCEDRV_DoGetActiveSyncEClockSourceRefId_IDT82V3399
#define SYNCEDRV_DoSetSyncEClockSourcePriorityByRefId  SYNCEDRV_DoSetSyncEClockSourcePriorityByRefId_IDT82V3399
#define SYNCEDRV_DumpSyncEAsicInfoInternal             SYNCEDRV_DumpSyncEAsicInfo_IDT82V3399
#define SYNCEDRV_SetChipModeInternal                   SYNCEDRV_SetChipMode_IDT82V3399
#define SYNCEDRV_SetOutputFreqOffsetInPPMInternal      SYNCEDRV_SetOutputFreqOffsetInPPM_IDT82V3399
#endif

/* MACRO FUNCTION DECLARATIONS
 */

#define WRITE_I2C_REG_WITH_MASK(i2c_bus_id, i2c_addr, type, validOffset, offset, moreThan256, val, mask) \
    ({ \
        BOOL_T __rc; \
        UI8_T  __data; \
        if(I2CDRV_TwsiDataReadWithBusIdx((i2c_bus_id), \
            (i2c_addr), (type), \
            (validOffset), (offset), (moreThan256), 1, &__data)==TRUE) \
        { \
            __data &= (UI8_T)(~(mask)); \
            __data |= (UI8_T)(val); \
            if(I2CDRV_TwsiDataWriteWithBusIdx((i2c_bus_id), \
                (i2c_addr), (type), \
                (validOffset), (offset), (moreThan256), &__data, 1)==TRUE) \
            { \
                __rc = TRUE; \
            } \
            else \
            { \
                __rc = FALSE; \
            } \
        } \
        else \
        { \
            __rc = FALSE; \
        } \
        __rc; \
    })

#define GPIO_ENTER_CRITICAL_SECTION() SYSFUN_ENTER_CRITICAL_SECTION(gpio_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define GPIO_LEAVE_CRITICAL_SECTION() SYSFUN_LEAVE_CRITICAL_SECTION(gpio_sem_id)


/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* chip independent functions
 */
static BOOL_T SYNCEDRV_SetClockSrc(UI32_T port, BOOL_T is_set);
static BOOL_T SYNCEDRV_UTIL_ConvertPhysicalRefIdToLogicalRefId(UI8_T physical_ref_id, UI8_T* logical_ref_id_p);
static BOOL_T SYNCEDRV_UTIL_ConvertLogicalRefIdToPhysicalRefId(UI8_T logical_ref_id, UI8_T* physical_ref_id_p);

/* chip dependent functions START */
/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_DoSetClockSrcSelectMode
 *-----------------------------------------------------------------
 * FUNCTION: This function will configure the clock source selection
 *           mode on SyncE ASIC.
 * INPUT   : is_auto - TRUE : clock source selection is performed by chip
 *                            automatically.
 *                     FALSE: clock source selection is determined by the
 *                            register setting which is configured by software
 *           is_revertive  -  This argument takes effect only when is_auto==TRUE
 *                            TRUE : The chip always revert the clock source to
 *                                   the highest priority clock source when it is
 *                                   available
 *                            FALSE: The chip will not revert the clock source.
 *                                   It will keep using the current selected
 *                                   clock source as long as it is available.
 *           force_clock_src_port - This argument takes effect only when is_auto==FALSE
 *                                  The specified clock source port
 *                                  will be selected as active clock source.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
static SYNCEDRV_TYPE_RET_T SYNCEDRV_DoSetClockSrcSelectMode(BOOL_T is_auto, BOOL_T is_revertive, UI32_T force_clock_src_port);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_DoGetSyncEClockSourceStatus
 *-----------------------------------------------------------------
 * FUNCTION: This function will get the clock source status
 *           of the specified logical reference input id
 * INPUT   : ref_id           - logical clock source input reference id
 * OUTPUT  : is_status_good_p - TRUE : clock source status is good
 *                              FALSE: clock source status is bad
 * RETURN  : TRUE  - Success
 *           FALSE - Failed
 * NOTE    : None
 *----------------------------------------------------------------*/
static BOOL_T SYNCEDRV_DoGetSyncEClockSourceStatus(UI32_T ref_id, BOOL_T *is_status_good_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_DoGetActiveSyncEClockSourceRefId
 *-----------------------------------------------------------------
 * FUNCTION: This function will get the logical reference input id of
 *           the active clock source and its lock status
 * INPUT   : None
 * OUTPUT  : ref_id      - logical clock source input reference id
 *           is_locked_p - TRUE: locked, FALSE: not locked
 * RETURN  : TRUE  - Success, an active logical reference input id is output
 *           FALSE - Failed
 * NOTE    : None
 *----------------------------------------------------------------*/
static BOOL_T SYNCEDRV_DoGetActiveSyncEClockSourceRefId(UI32_T *ref_id_p, BOOL_T *is_locked_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_DoSetSyncEClockSourcePriorityByRefId
 *-----------------------------------------------------------------
 * FUNCTION: This function will set the priority of the logical reference
 *           input id
 * INPUT   : ref_id      - logical clock source input reference id
 *           priority    - the priority of the given logical clock source
 *                         input reference id (start from 1)
 * OUTPUT  : None
 * RETURN  : TRUE  - Success, an active logical reference input id is output
 *           FALSE - Failed
 * NOTE    : 1. priority is used when setting automatic clock source
 *              selection.
 *           2. the argument priority is a logical priority and its
 *              valid range depends on ASIC. In usual, the valid range
 *              should be 1 to SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC(the number of input reference).
 *----------------------------------------------------------------*/
static BOOL_T SYNCEDRV_DoSetSyncEClockSourcePriorityByRefId(UI32_T ref_id, UI32_T priority);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_DumpSyncEAsicInfoInternal
 *-----------------------------------------------------------------
 * FUNCTION: This function will dump the syncE ASIC related info.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This function is for debug only.
 *----------------------------------------------------------------*/
static void SYNCEDRV_DumpSyncEAsicInfoInternal(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_DumpSyncEAsicInfo
 *-----------------------------------------------------------------
 * FUNCTION: This function will dump the syncE ASIC related info.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This function is for debug only.
 *----------------------------------------------------------------*/
void SYNCEDRV_DumpSyncEAsicInfo(void)
{
    SYNCEDRV_DumpSyncEAsicInfoInternal();
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_SetChipModeInternal
 *-----------------------------------------------------------------
 * FUNCTION: This function will set the synchronous ethernet ASIC
 *           mode according to the given argument.
 * INPUT   : mode  - The chip mode to be set to the ASIC and board.
 * OUTPUT  : None
 * RETURN  : TRUE  - Successfully
 *           FALSE - Failed
 * NOTE    : See "NOTE" of SYNCEDRV_SetChipMode().
 *----------------------------------------------------------------*/
static BOOL_T SYNCEDRV_SetChipModeInternal(SYNCEDRV_CHIP_MODE_TYPE_T mode);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_SetChipMode
 *-----------------------------------------------------------------
 * FUNCTION: This function will set the synchronous ethernet ASIC
 *           mode according to the given argument.
 * INPUT   : mode  - The chip mode to be set to the ASIC and board.
 * OUTPUT  : None
 * RETURN  : TRUE  - Successfully
 *           FALSE - Failed
 * NOTE    : 1. All synchronous ethernet ASIC must support
 *              SYNCEDRV_CHIP_MODE_TYPE_NORMAL.
 *           2. Not all synchronous ethernet ASIC support
 *              SYNCEDRV_CHIP_MODE_TYPE_DCO
 *           3. The clock source select mode will be restored to
 *              default setting (using local clock source, i.e. free-run mode)
 *              when chip mode is changed from other mode to NORMAL mode.
 *----------------------------------------------------------------*/
BOOL_T SYNCEDRV_SetChipMode(SYNCEDRV_CHIP_MODE_TYPE_T mode)
{
    BOOL_T rc;

    if (mode >= SYNCEDRV_CHIP_MODE_TYPE_NUMBER)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,
            "Invalid chip mode to be set.(%d)\r\n", (int)mode);
        return FALSE;
    }

    if (SYNCEDRV_OM_GetChipMode() == mode)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,
            "The chip mode to be set is the same with the current mode.(%d)\r\n", (int)mode);
        return TRUE;
    }

    rc = SYNCEDRV_SetChipModeInternal(mode);
    if (rc == TRUE)
    {
        SYNCEDRV_OM_SetChipMode(mode);
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,
            "Change chip mode to %d ok\r\n", (int)mode);
    }
    else
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,
            "Failed to change chip mode to %d\r\n", (int)mode);
    }

    return rc;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_SetOutputFreqOffsetInPPMInternal
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a frequency offset in PPM for
 *           output clock with respect to master clock of synchronous
 *           ethernet chip.
 * INPUT   : ppm  - the frequency offset in PPM
 * OUTPUT  : None
 * RETURN  : TRUE  - Successfully
 *           FALSE - Failed
 * NOTE    : See "NOTE" of SYNCEDRV_SetOutputFreqOffsetInPPM().
 *----------------------------------------------------------------*/
static BOOL_T SYNCEDRV_SetOutputFreqOffsetInPPMInternal(float ppm);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_SetOutputFreqOffsetInPPM
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a frequency offset in PPM for
 *           output clock with respect to master clock of synchronous
 *           ethernet chip.
 * INPUT   : ppm  - the frequency offset in PPM
 * OUTPUT  : None
 * RETURN  : TRUE  - Successfully
 *           FALSE - Failed
 * NOTE    : 1. This function can only be called after setting
 *              chip mode as SYNCEDRV_CHIP_MODE_TYPE_DCO. If call
 *              this function in chip mode other than SYNCEDRV_CHIP_MODE_TYPE_DCO
 *              , this function returns false.
 *           2. On IDT82V3399, this function needs to be called once per second.
 *----------------------------------------------------------------*/
BOOL_T SYNCEDRV_SetOutputFreqOffsetInPPM(float ppm)
{
    /* check chip mode
     */
    if (SYNCEDRV_OM_GetChipMode() != SYNCEDRV_CHIP_MODE_TYPE_DCO)
    {
        BACKDOOR_MGR_Printf("%s(%d): Illegal chip mode %d\r\n", __FUNCTION__,
            __LINE__, (int)(SYNCEDRV_OM_GetChipMode()));
        return FALSE;
    }

    return SYNCEDRV_SetOutputFreqOffsetInPPMInternal(ppm);
}

/* chip dependent functions END   */

/* project dependent functions START */
/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_ValidateOutputClockSrcSetting
 *-----------------------------------------------------------------
 * FUNCTION: Validate the settings of given clock source ports which
 *           will output recover clock to synce ASIC
 * INPUT   : pbmp - the port bitmap of the ports that will be configured as
 *                  clock source.
 * OUTPUT  : None
 * RETURN  : TRUE  - the given setting of clock source ports is valid
 *           FALSE - the given setting of clock source ports is invalid
 * NOTE    : 1. This function is project dependent function.
 *           2. Each project might have different constraints on
 *              settings of given clock source ports.
 *----------------------------------------------------------------*/
static BOOL_T __SYNCEDRV_ValidateOutputClockSrcSetting(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);

/*-----------------------------------------------------------------
 * ROUTINE NAME - __SYNCEDRV_SetPHYSyncEPortInClockSource
 *-----------------------------------------------------------------
 * FUNCTION: This function set the input clock source of the given
 *           port.
 * INPUT   : port         - which port to set
 *           clock_source - input clock source for the given port
 * OUTPUT  : None
 * RETURN  : TRUE: Success, FALSE: Failed
 * NOTE    : None
 *----------------------------------------------------------------*/
static BOOL_T __SYNCEDRV_SetPHYSyncEPortInClockSource(UI32_T port, SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_T clock_source);

/*-----------------------------------------------------------------
 * ROUTINE NAME - __SYNCEDRV_SetClockSrcByRefIdxOnASIC
 *-----------------------------------------------------------------
 * FUNCTION: This function will set the given user port id as the
 *           clock source to the given reference input index on ASIC
 * INPUT   : port    - the user port id that will be set as synce clock
 *                     source to the specified reference input index
 *           is_set  - TRUE : to set synce clock source port
 *                     FALSE: to remove the port as synce clock source
 *                            port
 *           ref_id  - physical reference input index to be set or removed
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
static BOOL_T __SYNCEDRV_SetClockSrcByRefIdxOnASIC(UI32_T port, BOOL_T is_set, UI32_T ref_id);

/*-----------------------------------------------------------------
 * ROUTINE NAME - __SYNCEDRV_BindClockSrcPortToRefIdx
 *-----------------------------------------------------------------
 * FUNCTION: This function will output an input reference index for
 *           the given clock source port if available.
 * INPUT   : port      - the user port id of the clock source port
 * OUTPUT  : ref_idx_p - the input reference index for the given
 *                       clock source port
 * RETURN  : TRUE - Find an input reference index succesfully
 *           FALSE- No input reference index available
 * NOTE    : Caller of this function shall do critical section protection
 *----------------------------------------------------------------*/
static BOOL_T __SYNCEDRV_BindClockSrcPortToRefIdx(UI32_T port, UI32_T *ref_idx_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - __SYNCEDRV_SetupForChipMode
 *-----------------------------------------------------------------
 * FUNCTION: This function will do board dependent operations when
 *           synchronous ethernet chip mode is changed.
 * INPUT   : mode - The chip mode to be set to the ASIC and board.
 * OUTPUT  : None
 * RETURN  : TRUE  - Successfully
 *           FALSE - Failed
 * NOTE    : See "NOTE" of SYNCEDRV_SetChipMode().
 *----------------------------------------------------------------*/
static BOOL_T __SYNCEDRV_SetupForChipMode(SYNCEDRV_CHIP_MODE_TYPE_T mode);

/* project dependent functions are implemented in function name __SYCEDRV_XXX
 * this is done through the definitions shown below
 */
#define SYNCEDRV_ValidateOutputClockSrcSetting    __SYNCEDRV_ValidateOutputClockSrcSetting
#define SYNCEDRV_SetPHYSyncEPortInClockSource     __SYNCEDRV_SetPHYSyncEPortInClockSource
#define SYNCEDRV_SetClockSrcByRefIdxOnASIC        __SYNCEDRV_SetClockSrcByRefIdxOnASIC
#define SYNCEDRV_BindClockSrcPortToRefIdx         __SYNCEDRV_BindClockSrcPortToRefIdx
#define SYNCEDRV_SetupForChipMode                 __SYNCEDRV_SetupForChipMode
/* project dependent functions END   */

static void SYNCEDRV_DumpRefIdBindedPortInfo(void);
static void SYNCEDRV_BackDoor_Menu(void);

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T gpio_sem_id;

/* EXPORTED SUBPROGRAM BODIES
 */
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYNCEDRV_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYNCEDRV_AttachSystemResources(void)
{
    UI32_T ret;

    ret = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_GPIO, &gpio_sem_id);
    if (ret != SYSFUN_OK)
    {
        BACKDOOR_MGR_Printf("%s(): SYSFUN_GetSem fails. (%d)\n", __FUNCTION__, ret);
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SYNCEDRV_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("syncedrv", SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY, SYNCEDRV_BackDoor_Menu);
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_SetTransitionMode
 *-----------------------------------------------------------------
 * FUNCTION: This function will set SYNCEDRV to transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_SetTransitionMode(void)
{
    SYNCEDRV_OM_SetTransitionMode();
    SYNCEDRV_OM_SetProvisionComplete(FALSE);
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_EnterTransitionMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initialize data variables for transition mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_EnterTransitionMode(void)
{
    UI32_T i, clock_src_num;
    SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
    BOOL_T is_board_support_synce;

    if (STKTPLG_BOARD_GetNumberOfSyncEPort() > 0)
        is_board_support_synce=TRUE;
    else
        is_board_support_synce=FALSE;

    SYNCEDRV_OM_EnterTransitionMode();
    /* remove all clock source port settings in OM
     */
    SYNCEDRV_OM_GetSyncEClockSourceStatus(clock_src_lst, &clock_src_num);
    SYNCEDRV_OM_RemoveSyncEClockSource(clock_src_lst, clock_src_num);

    /* remove all clock source port settings in chip
     */
    if (is_board_support_synce==TRUE)
    {
        for(i=0; i<clock_src_num; i++)
        {
            SYNCEDRV_SetClockSrc(clock_src_lst[i].port, FALSE);
        }
    }

    /* reset all of the synce status as disable
     */
    SYNCEDRV_OM_GetSyncEEnabledPbmp(pbmp);
    if (is_board_support_synce==TRUE)
        SYNCEDRV_DisableSyncE(pbmp);

}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_EnterMasterMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initial data variables for master mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_EnterMasterMode(void)
{
    SYNCEDRV_OM_EnterMasterMode();
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_EnterSlaverMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initial data variables for slave mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_EnterSlaverMode(void)
{
    SYNCEDRV_OM_EnterSlaveMode();
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_ProvisionComplete
 *-----------------------------------------------------------------
 * FUNCTION: This function initial data variables for provision complete operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_ProvisionComplete(void)
{
    SYNCEDRV_OM_SetProvisionComplete(TRUE);
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_EnableSyncE
 *-----------------------------------------------------------------
 * FUNCTION: This function will enable the functionality of
 *           synchronous ethernet on the specified ports.
 * INPUT   : pbmp  -  port bitmap to specify which ports to enable synce
 *                    Callers shall use the macro in l_pbmp.h to generate the
 *                    port bitmap.
 * OUTPUT  : None
 * RETURN  : SYNCEDRV_TYPE_RET_OK             - successful
 *           SYNCEDRV_TYPE_RET_ERR_ARG   - invalid argument
 *           SYNCEDRV_TYPE_RET_ERR_FAIL  - an error occurs while enabling synce
 * NOTE    : 1. Only the ports that support synce are allowed to enable.
 *           2. No port will be set as enabled if the pbmp contains ports that
 *              do not support synce.
 *           3. SyncE enabled status will be kept unchanged if the port that
 *              supports synce is not included in pbmp.
 *           4. The default role for a synce-enabled port is clock-distributed port
 *              Callers need to call SYNCEDRV_SetSyncEClockSource() to change its
 *              role as clock-sourced port
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_EnableSyncE(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST])
{
    UI8_T  port, i;
    UI8_T  old_pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];    /* original port synce enabled status */
    UI8_T  changed_pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];/* port that had been enabled successfully */
    BOOL_T set_err=FALSE;

    /* sanity check
     */
    if (pbmp==NULL)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"pbmp is NULL\r\n");
            
        return SYNCEDRV_TYPE_RET_ERR_ARG;
    }

    /* validate that the ports in given pbmp are all capable of synce
     */
    L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(pbmp, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT, port)
    {
        if(STKTPLG_BOARD_IsSyncEPort(port)==FALSE)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,
                "Port %hu does not support synce\r\n", port);
            return SYNCEDRV_TYPE_RET_ERR_ARG;
        }
    }

    SYNCEDRV_OM_GetSyncEEnabledPbmp(old_pbmp);

    L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(pbmp, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT, port)
    {
        /* The default role for synce enabled port is clock-distributed port
         */
        /* See LIMITAION in file header
         */

        if(SYNCEDRV_SetPHYSyncEPortInClockSource(port, SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_EXTERNAL)==FALSE)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_IMPORTANT_MSG,"Failed to set PHY clock source on port %hu\r\n", port);
            set_err=TRUE;
            break;
        }
        L_PBMP_SET_PORT_IN_PBMP_ARRAY(changed_pbmp, port);
    }

    if(set_err==TRUE)
    {
        /* restore the synce enabled status to disable if the original status
         * is synce disabled
         */
        L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(changed_pbmp, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT, port)
        {
            if(L_PBMP_GET_PORT_IN_PBMP_ARRAY(old_pbmp, port)==0)
            {
                /* See LIMITAION in file header
                 */

                SYNCEDRV_SetPHYSyncEPortInClockSource(port, SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_LOCAL);
            }
        }
        return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }

    /* Do or operation on the pbmp set this time with the original pbmp
     */
    for(i=0; i<SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST; i++)
        pbmp[i]|=old_pbmp[i];

    SYNCEDRV_OM_SetSyncEEnabledPbmp(pbmp);
    return SYNCEDRV_TYPE_RET_OK;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_DisableSyncE
 *-----------------------------------------------------------------
 * FUNCTION: This function will disable the functionality of
 *           synchronous ethernet on the specified ports.
 * INPUT   : pbmp  -  port bitmap to specify which ports to disable synce
 *                    Callers shall use the macro in l_pbmp.h to generate the
 *                    port bitmap.
 * OUTPUT  : None
 * RETURN  : SYNCEDRV_TYPE_RET_OK             - successful
 *           SYNCEDRV_TYPE_RET_ERR_ARG   - invalid argument
 *           SYNCEDRV_TYPE_RET_ERR_FAIL  - an error occurs while enabling synce
 * NOTE    : 1. Only the ports that support synce are allowed to disable.
 *           2. No port will be set as disabled if the pbmp contains ports that
 *              do not support synce.
 *           3. SyncE disabled status will be kept unchanged if the port that
 *              supports synce is not included in pbmp.
 *           4. If the port to disable synce is a clock source port, it will be
 *              removed from the clock source implicitly.
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_DisableSyncE(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST])
{
    UI32_T old_clock_src_num, ref_idx;
    SYNCEDRV_TYPE_ClockSource_T old_clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    UI8_T  port, i;
    UI8_T  old_pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];    /* original port synce enabled status */
    UI8_T  changed_pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];/* port that had been disabled */
    BOOL_T set_err=FALSE, clock_source_port_changed=FALSE;

    /* sanity check
     */
    if (pbmp==NULL)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"pbmp is NULL\r\n");
        return SYNCEDRV_TYPE_RET_ERR_ARG;
    }

    SYNCEDRV_OM_GetSyncEEnabledPbmp(old_pbmp);

    /* validate that the ports in given pbmp are synce enabled port
     */
    L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(pbmp, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT, port)
    {
        if(L_PBMP_GET_PORT_IN_PBMP_ARRAY(old_pbmp, port)==0)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_IMPORTANT_MSG,"Port %hu is not synce enabled\r\n", port);
            return SYNCEDRV_TYPE_RET_ERR_ARG;
        }
    }

    SYNCEDRV_OM_GetSyncEClockSourceStatus(old_clock_src_lst, &old_clock_src_num);

    L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(pbmp, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT, port)
    {
        L_PBMP_SET_PORT_IN_PBMP_ARRAY(changed_pbmp, port);
        /* check whether the port is a clock source port
         * if the port is a clock source port, need to
         * remove it from clock source port list
         */
        for(i=0; i<old_clock_src_num; i++)
        {
            if(old_clock_src_lst[i].port==port)
            {
                SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,
                    "Remove port %lu priority %lu from clock source\r\n",
                    port, old_clock_src_lst[i].priority);
                SYNCEDRV_OM_RemoveSyncEClockSource(&(old_clock_src_lst[i]), 1);
                clock_source_port_changed=TRUE;
                if(SYNCEDRV_SetClockSrc(port, FALSE)==FALSE)
                {
                    BACKDOOR_MGR_Printf("%s(%d): Failed to set port %lu as non-clock source\r\n",
                        __FUNCTION__, __LINE__);
                    set_err=TRUE;
                    break;
                }
                /* See LIMITAION in file header
                 */
                #if((SYS_CPNT_SYNCE_SUPPORT_MEDIA & SYS_CPNT_SYNCE_SUPPORT_MEDIA_1000BASE_T)!=0)
                /* Set manual master/slave configuration for SyncE 1000BaseT ports
                 * Current design does not consider to support SyncE on 1000BaseT and non
                 * 1000BaseT port at the same time. So no port type check is performed here.
                 */
                if(FALSE==DEV_SWDRV_PMGR_SetPort1000BaseTForceMode(1, port, VAL_portMasterSlaveModeCfg_auto))
                {
                    BACKDOOR_MGR_Printf("%s(%d)Set 1000BaseT force mode as auto failed on port %u\n", __FUNCTION__, __LINE__, port);
                    set_err=TRUE;
                    break;
                }
                #endif
            }
        }


        if(SYNCEDRV_SetPHYSyncEPortInClockSource(port, SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_LOCAL)==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Failed to set PHY clock source on port %hu\n", __FUNCTION__, __LINE__, port);
            set_err=TRUE;
            break;
        }

    }

    if(set_err==TRUE)
    {
        /* restore the synce enabled status to enable if the original status
         * is synce enabled
         */
        L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(changed_pbmp, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT, port)
        {
            BOOL_T is_clock_src_port;

            /* check whether the port is a clock source port or a
             * clock distributed port, and set the clock source accordingly.
             */
            for(is_clock_src_port=FALSE, i=0; i<old_clock_src_num; i++)
            {
                if(port==old_clock_src_lst[i].port)
                {
                    is_clock_src_port=TRUE;
                }
            }

            if(is_clock_src_port==TRUE)
            {
                SYNCEDRV_SetPHYSyncEPortInClockSource(port, SYS_HWCFG_SYNCE_CLOCK_SOURCE_PORT_PHY_PORT_CLOCK_SOURCE_MODE);
                /* See LIMITAION in file header
                 */
                #if((SYS_CPNT_SYNCE_SUPPORT_MEDIA & SYS_CPNT_SYNCE_SUPPORT_MEDIA_1000BASE_T)!=0)
                DEV_SWDRV_PMGR_SetPort1000BaseTForceMode(1, port, VAL_portMasterSlaveModeCfg_slave);
                #endif
                SYNCEDRV_SetClockSrc(port, TRUE);
            }
            else
            {
                SYNCEDRV_SetPHYSyncEPortInClockSource(port, SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_EXTERNAL);
            }

        }
        /* restore the original clock source port settings
         */
        if(clock_source_port_changed==TRUE)
        {
            SYNCEDRV_OM_RemoveAllSyncEClockSource();
            SYNCEDRV_OM_UpdateSyncEClockSource(old_clock_src_lst, old_clock_src_num);
            for(i=0; i<old_clock_src_num; i++)
            {
                SYNCEDRV_SetClockSrc(old_clock_src_lst[i].port, TRUE);
            }

            /* clock source port and ref idx binding is established after
             * calling SYNCEDRV_SetClockSrc()
             * call SYNCEDRV_OM_GetRefIdxByClockSrcPort() to get the binded
             * ref idx and config the priority of the ref idx
             */
            for(i=0; i<old_clock_src_num; i++)
            {
                if(SYNCEDRV_OM_GetRefIdxByClockSrcPort(old_clock_src_lst[i].port, &ref_idx)==TRUE)
                {
                    SYNCEDRV_DoSetSyncEClockSourcePriorityByRefId(ref_idx, i+1);
                }
                else
                {
                    BACKDOOR_MGR_Printf("%s(%d): Failed to find binded ref idx of the clock source port %lu\r\n",
                        __FUNCTION__, __LINE__, old_clock_src_lst[i].port);
                }

            }
        }

        return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }

    /* need not to check and update priority of ref idx
     * because the ref idx could just be removed and the relative priority
     * of remaining ref idx is not changed.
     * update the pbmp and set the updated pbmp to OM
     */
    for(i=0; i<SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST; i++)
        pbmp[i] = old_pbmp[i] ^ pbmp[i];

    SYNCEDRV_OM_SetSyncEEnabledPbmp(pbmp);
    return SYNCEDRV_TYPE_RET_OK;

}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_UpdateSyncEClockSource
 *-----------------------------------------------------------------
 * FUNCTION: This function will set the synchronous ethernet clock source
 *           to the specified ports.
 * INPUT   : clock_src_lst     - an array which contains the clock source info
 *               ->port        - user port id of the clock source
 *               ->priority    - the prority value of the clock source, smaller
 *                               value gets higher priority
 *           clock_src_lst_len - number of element in clock_src_lst
 * OUTPUT  : None
 * RETURN  : SYNCEDRV_TYPE_RET_OK                  - successful
 *           SYNCEDRV_TYPE_RET_ERR_ARG             - invalid argument
 *           SYNCEDRV_TYPE_RET_ERR_FAIL            - an error occurs while setting synce clock
 *                                                   source
 *           SYNCEDRV_TYPE_RET_ERR_OUT_OF_RESOURCE - error due to out of resource
 * NOTE    : 1. Only the ports that have enabled synce are allowed to be set as
 *              clock source.
 *           2. No port will be set as clock source if the return value of this
 *              function is not SYNCEDRV_TYPE_RET_OK.
 *           3. The clock source info will be updated if the clock source had been
 *              set in the previous function call.
 *           4. The clock source set in the previous call will not be changed if
 *              they are not included in this function call.
 *           5. If the number of clock source to be set is more than
 *              SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC, SYNCEDRV_TYPE_RET_ERR_OUT_OF_RESOURCE
 *              will be returned.
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_UpdateSyncEClockSource(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[], UI32_T clock_src_lst_len)
{
    UI32_T i, old_clock_src_num, new_clock_src_num, total_clock_src_num, ref_idx;
    SYNCEDRV_TYPE_ClockSource_T old_clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    SYNCEDRV_TYPE_ClockSource_T new_clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
    UI8_T old_clksrc_pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]; /* pbmp of clock source ports before calling this func */
    
    
    if((clock_src_lst==NULL) || (clock_src_lst_len==0) || (clock_src_lst_len>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC))
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Invalid argument.\r\n");
        return SYNCEDRV_TYPE_RET_ERR_ARG;
    }

    /* check that the ports in clock_src_lst must be synce-enabled port
     */
    SYNCEDRV_OM_GetSyncEEnabledPbmp(pbmp);
    for(i=0; i<clock_src_lst_len; i++)
    {
        if(L_PBMP_GET_PORT_IN_PBMP_ARRAY(pbmp, clock_src_lst[i].port)==0)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_IMPORTANT_MSG, "Failed to set synce clock source at port %lu because it is not synce-enabled.", clock_src_lst[i].port);
            return SYNCEDRV_TYPE_RET_ERR_ARG;
        }
    }

    memset(old_clksrc_pbmp, 0, sizeof(UI8_T)*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST);
    SYNCEDRV_OM_GetSyncEClockSourceStatus(old_clock_src_lst, &old_clock_src_num);
    for(i=0; i<old_clock_src_num; i++)
    {
        L_PBMP_SET_PORT_IN_PBMP_ARRAY(old_clksrc_pbmp, old_clock_src_lst[i].port);
    }
    memcpy(pbmp, old_clksrc_pbmp, sizeof(UI8_T)*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST);
    for(i=0; i<clock_src_lst_len; i++)
    {
        L_PBMP_SET_PORT_IN_PBMP_ARRAY(pbmp, clock_src_lst[i].port);
        /* init is_good_status and is_active as FALSE
         */
        clock_src_lst[i].is_good_status=FALSE;
        clock_src_lst[i].is_active=FALSE;
    }

    total_clock_src_num=0;
    L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(pbmp, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT, i)
    {
        total_clock_src_num++;
    }

    /* Check whether the resulting number of clock source exceeds the upper limit
     */
    if(total_clock_src_num>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_IMPORTANT_MSG, "Total clock source number(%lu) is greater than max value\r\n", total_clock_src_num);
        return SYNCEDRV_TYPE_RET_ERR_OUT_OF_RESOURCE;
    }

    /* validate the resulting clock source settings
     */
    if (SYNCEDRV_ValidateOutputClockSrcSetting(pbmp)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_IMPORTANT_MSG, "Output clock source setting is invalid\r\n");
        return SYNCEDRV_TYPE_RET_ERR_OUT_OF_RESOURCE;
    }


    /* set updated clock source info to OM
     */
    SYNCEDRV_OM_UpdateSyncEClockSource(clock_src_lst, clock_src_lst_len);

    /* get result of clock source info from OM
     */
    SYNCEDRV_OM_GetSyncEClockSourceStatus(new_clock_src_lst, &new_clock_src_num);
    for(i=0; i<new_clock_src_num; i++)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Set port %lu as clock source input\r\n",
            new_clock_src_lst[i].port);
        if (L_PBMP_GET_PORT_IN_PBMP_ARRAY(old_clksrc_pbmp, new_clock_src_lst[i].port))
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Port %lu is a clock source input originally.\r\n",
            new_clock_src_lst[i].port);
            continue;
        }
        /* set the new clock source setting
         */
        SYNCEDRV_SetPHYSyncEPortInClockSource(new_clock_src_lst[i].port, SYS_HWCFG_SYNCE_CLOCK_SOURCE_PORT_PHY_PORT_CLOCK_SOURCE_MODE);
        /* See LIMITAION in file header
         */
        #if((SYS_CPNT_SYNCE_SUPPORT_MEDIA & SYS_CPNT_SYNCE_SUPPORT_MEDIA_1000BASE_T)!=0)
        DEV_SWDRV_PMGR_SetPort1000BaseTForceMode(1, new_clock_src_lst[i].port, VAL_portMasterSlaveModeCfg_slave);
        #endif
        SYNCEDRV_SetClockSrc(new_clock_src_lst[i].port, TRUE);
    }

    /* clock source port and ref idx binding is established after
     * calling SYNCEDRV_SetClockSrc()
     * call SYNCEDRV_OM_GetRefIdxByClockSrcPort() to get the binded
     * ref idx and config the priority of the ref idx
     */
    for(i=0; i<new_clock_src_num; i++)
    {
        if(SYNCEDRV_OM_GetRefIdxByClockSrcPort(new_clock_src_lst[i].port, &ref_idx)==TRUE)
        {
            SYNCEDRV_DoSetSyncEClockSourcePriorityByRefId(ref_idx, i+1);
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Set port %lu(ref_idx=%lu) as clock source input with priority %lu\r\n",
                new_clock_src_lst[i].port, ref_idx, i+1);

        }
        else
        {
            BACKDOOR_MGR_Printf("%s(%d): Failed to find binded ref idx of the clock source port %lu\r\n",
                __FUNCTION__, __LINE__, new_clock_src_lst[i].port);
        }
    }

    return SYNCEDRV_TYPE_RET_OK;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_RemoveSyncEClockSource
 *-----------------------------------------------------------------
 * FUNCTION: This function will remove the synchronous ethernet clock source
 *           according to the specified ports.
 * INPUT   : clock_src_lst     - an array which contains the clock source info
 *               ->port        - user port id of the clock source
 *               ->priority    - the prority value of the clock source, smaller
 *                               value gets higher priority
 *           clock_src_lst_len - number of element in clock_src_lst
 * OUTPUT  : None
 * RETURN  : SYNCEDRV_TYPE_RET_OK                  - successful
 *           SYNCEDRV_TYPE_RET_ERR_ARG             - invalid argument
 *           SYNCEDRV_TYPE_RET_ERR_FAIL            - an error occurs while removing
 *                                                   synce clock source
 * NOTE    : 1. Only the ports that are set as clock source are allowed to be
 *              removed from the clock source.
 *           2. No port will be removed from the clock source if the return value
 *              of this function is not SYNCEDRV_TYPE_RET_OK.
 *           3. If the number of clock source to be set is more than
 *              SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC, SYNCEDRV_TYPE_RET_ERR_ARG
 *              will be returned.
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_RemoveSyncEClockSource(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[], UI32_T clock_src_lst_len)
{
    UI32_T i;
    SYNCEDRV_TYPE_ClockSource_T entry;

    if((clock_src_lst==NULL) || (clock_src_lst_len>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC))
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Invalid argument\r\n");
        return SYNCEDRV_TYPE_RET_ERR_ARG;
    }

    /* check whether the specified clock source is set as clock source now
     */
    for(i=0; i<clock_src_lst_len; i++)
    {
        entry.port=clock_src_lst[i].port;
        entry.priority=clock_src_lst[i].priority;
        if(SYNCEDRV_OM_GetSyncEClockSourceStatusByPortandPriority(&entry)==FALSE)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_IMPORTANT_MSG, "Cannot find entry:port %lu, priority %lu\r\n", entry.port, entry.priority);
            return SYNCEDRV_TYPE_RET_ERR_ARG;
        }
    }

    /* remove the specified clock source in OM
     */
    SYNCEDRV_OM_RemoveSyncEClockSource(clock_src_lst, clock_src_lst_len);

    /* remove the specified clock source in ASIC
     */
    for(i=0; i<clock_src_lst_len; i++)
    {
        SYNCEDRV_SetPHYSyncEPortInClockSource(clock_src_lst[i].port, SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_LOCAL);
        /* See LIMITAION in file header
         */
        #if((SYS_CPNT_SYNCE_SUPPORT_MEDIA & SYS_CPNT_SYNCE_SUPPORT_MEDIA_1000BASE_T)!=0)
        DEV_SWDRV_PMGR_SetPort1000BaseTForceMode(1, clock_src_lst[i].port, VAL_portMasterSlaveModeCfg_auto);
        #endif
        SYNCEDRV_SetClockSrc(clock_src_lst[i].port, FALSE);
    }

    return SYNCEDRV_TYPE_RET_OK;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_SetClockSrcSelectMode
 *-----------------------------------------------------------------
 * FUNCTION: This function will configure the clock source selection
 *           mode on chip
 * INPUT   : is_auto - TRUE : clock source selection is performed by chip
 *                            automatically.
 *                     FALSE: clock source selection is determined by the
 *                            register setting which is configured by software
 *           is_revertive  -  This argument takes effect only when is_auto==TRUE
 *                            TRUE : The chip always revert the clock source to
 *                                   the highest priority clock source when it is
 *                                   available
 *                            FALSE: The chip will not revert the clock source.
 *                                   It will keep using the current selected
 *                                   clock source as long as it is available.
 *           force_clock_src_port - This argument takes effect only when is_auto==FALSE
 *                                  The specified clock source port
 *                                  will be selected as active clock source.
 *                                  If force_clock_src_port is 0, that means local
 *                                  clock will be used as synce clock source.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_SetClockSrcSelectMode(BOOL_T is_auto, BOOL_T is_revertive, UI32_T force_clock_src_port)
{
    return SYNCEDRV_DoSetClockSrcSelectMode(is_auto, is_revertive, force_clock_src_port);
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_GetSyncEClockSourceStatus
 *-----------------------------------------------------------------
 * FUNCTION: This function will get the clock source status
 * INPUT   : None
 * OUTPUT  : clock_src_lst   - the clock source status in order of priority
 *                             (Highest priority in the first entry)
 *           
 *           clock_src_num_p - number of element in clock_src_lst
 *           is_locked_p     - TRUE: the selected clock source input is in locked state
 * RETURN  : SYNCEDRV_TYPE_RET_ERR_ARG  - error due to incorrect arguments
 *           SYNCEDRV_TYPE_RET_ERR_FAIL - error due to chip error
 *           SYNCEDRV_TYPE_RET_OK       - successfully
 * NOTE    : None
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_GetSyncEClockSourceStatus(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC], UI32_T* clock_src_num_p, BOOL_T *is_locked_p)
{
    UI32_T i, active_ref_idx=SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC, ref_idx;
    BOOL_T is_good_status=FALSE, is_active_clk_src_exists;

    if(clock_src_lst==NULL || clock_src_num_p==NULL || is_locked_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d):Invalid argument.\r\n", __FUNCTION__, __LINE__);
        return SYNCEDRV_TYPE_RET_ERR_ARG;
    }
    *is_locked_p=FALSE;

    SYNCEDRV_OM_GetSyncEClockSourceStatus(clock_src_lst, clock_src_num_p);

    if(*clock_src_num_p==0)
        return SYNCEDRV_TYPE_RET_OK;

    is_active_clk_src_exists=SYNCEDRV_DoGetActiveSyncEClockSourceRefId(&active_ref_idx, is_locked_p);

    /* SYNCEDRV OM will always have lock_status and is_active as FALSE
     * so check the status in chip and update the fields if they are TRUE
     */
    for(i=0; i<*clock_src_num_p; i++)
    {
        if(SYNCEDRV_OM_GetRefIdxByClockSrcPort(clock_src_lst[i].port, &ref_idx)==TRUE)
        {
            if(SYNCEDRV_DoGetSyncEClockSourceStatus(ref_idx, &is_good_status)==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d):Failed to get clock source status.\r\n",
                    __FUNCTION__, __LINE__);
                return SYNCEDRV_TYPE_RET_ERR_FAIL;
            }
        }
        else
        {
            BACKDOOR_MGR_Printf("%s(%d):Failed to get binded ref idx for the clock source port %lu.\r\n",
                __FUNCTION__, __LINE__, clock_src_lst[i].port);
            return SYNCEDRV_TYPE_RET_ERR_FAIL;
        }

        if(is_good_status==TRUE)
            clock_src_lst[i].is_good_status = is_good_status;

        if(is_active_clk_src_exists==TRUE && active_ref_idx==ref_idx)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"found active clock ref_idx=%lu, port=%lu\r\n",
                active_ref_idx, clock_src_lst[i].port);
            clock_src_lst[i].is_active=TRUE;
        }
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"[i=%lu] port=%lu, is_active=%hu\r\n", i,
            clock_src_lst[i].port, clock_src_lst[i].is_active);
    }

    return SYNCEDRV_TYPE_RET_OK;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_SetClockSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will set the given user port id as the
 *           clock source.
 * INPUT   : port    - the user port id that will be set as synce clock
 *                     source to the specified reference input index
 *           is_set  - TRUE : to set synce clock source port
 *                     FALSE: to remove the port as synce clock source
 *                            port
 * OUTPUT  : None
 * RETURN  : TRUE  - Successfully
 *           FALSE - Failed
 * NOTE    : 1. The valid range of the priority is 0 to SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC-1
 *              lower value get higher priority. priority is not used when is_set==FALSE
 *           2. This function will do binding of clock src port and ref idx when
 *              add a new synce clock source port, and do un-binding of clock src
 *              port and ref idx when remove a synce clock source port
 *           3. This function will not update synce clock source port list in
 *              SYNCEDRV_OM.
 *----------------------------------------------------------------*/
static BOOL_T SYNCEDRV_SetClockSrc(UI32_T port, BOOL_T is_set)
{
    UI32_T ref_id;
    BOOL_T binding_exist;

    binding_exist=SYNCEDRV_OM_GetRefIdxByClockSrcPort(port, &ref_id);

    if(binding_exist==FALSE)
    {
        if(is_set==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d): error, unable to remove clock source because the port %lu is not binded to a ref id\r\n",
                __FUNCTION__, __LINE__, port);
            return FALSE;
        }
        else
        {
            if(FALSE == SYNCEDRV_BindClockSrcPortToRefIdx(port, &ref_id))
            {
                BACKDOOR_MGR_Printf("%s(%d): error, failed to bind port %lu to a ref idx\r\n",
                    __FUNCTION__, __LINE__, port);
                return FALSE;
            }
        }
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Create new binding : port=%lu, ref_id=%lu\r\n", port, ref_id);

    }
    else
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Binding exists: port=%lu, ref_id=%lu\r\n",
            port, ref_id);
    }

    /* enable PHY recover clock and bind it to a ref_id
     */
    if(SYNCEDRV_SetClockSrcByRefIdxOnASIC(port, is_set, ref_id)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Set PHY SyncE clock source port on ASIC failed(port=%lu, ref_id=%lu)\r\n",
            __FUNCTION__, __LINE__, port, ref_id);
        return FALSE;
    }

    if(is_set==TRUE)
    {
        if(SYNCEDRV_SetPHYSyncEPortInClockSource(port, SYS_HWCFG_SYNCE_CLOCK_SOURCE_PORT_PHY_PORT_CLOCK_SOURCE_MODE)==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Add PHY SyncE clock source port failed(port=%lu, ref_id=%lu)\r\n",
                __FUNCTION__, __LINE__, port, ref_id);
            return FALSE;
        }
        /* update OM for binded ref idx and clock source port
         */
        SYNCEDRV_OM_SetRefIdxToClockSrcPortBinding(ref_id, port);
    }
    else
    {
        if(SYNCEDRV_SetPHYSyncEPortInClockSource(port, SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_EXTERNAL)==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Remove PHY SyncE clock source port failed(port=%lu, ref_id=%lu)\r\n",
                __FUNCTION__, __LINE__, port, ref_id);
            return FALSE;
        }

        /* unbind ref_id to the given port by setting binded port id as 0
         */
        SYNCEDRV_OM_SetRefIdxToClockSrcPortBinding(ref_id, 0);
    }

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_UTIL_ConvertPhysicalRefIdToLogicalRefId
 *-----------------------------------------------------------------
 * FUNCTION: This function will convert physical ref id to logical ref
 *           id.
 *
 * INPUT   : physical_ref_id  - physical reference id
 * OUTPUT  : logical_ref_id_p - logical reference id converted from physical
 *           reference id
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
static BOOL_T SYNCEDRV_UTIL_ConvertPhysicalRefIdToLogicalRefId(UI8_T physical_ref_id, UI8_T* logical_ref_id_p)
{
    UI32_T i;
    UI8_T  physical_usable_ref_ids[] = SYS_HWCFG_SYNCE_CHIP_USABLE_PHYISCAL_REF_ID_ARRAY;

    if(SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC != (sizeof(physical_usable_ref_ids)/sizeof(physical_usable_ref_ids[0])))
    {
        BACKDOOR_MGR_Printf("%s(%d) Inconsistent definition of SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC(%lu) and SYS_HWCFG_SYNCE_CHIP_USABLE_PHYISCAL_REF_ID_ARRAY.\r\n",
            __FUNCTION__, __LINE__, SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC);
        return FALSE;
    }

    for(i=0; i<(sizeof(physical_usable_ref_ids)/sizeof(physical_usable_ref_ids[0])); i++)
    {
        if(physical_usable_ref_ids[i]==physical_ref_id)
        {
            *logical_ref_id_p=i;
            return TRUE;
        }
    }
    return FALSE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_UTIL_ConvertLogicalRefIdToPhysicalRefId
 *-----------------------------------------------------------------
 * FUNCTION: This function will convert logical ref id to physical ref
 *           id.
 *
 * INPUT   : logical_ref_id    - logical reference id
 * OUTPUT  : physical_ref_id_p - physical reference id converted from logical
 *           reference id
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
static BOOL_T SYNCEDRV_UTIL_ConvertLogicalRefIdToPhysicalRefId(UI8_T logical_ref_id, UI8_T* physical_ref_id_p)
{
    UI8_T  physical_usable_ref_ids[] = SYS_HWCFG_SYNCE_CHIP_USABLE_PHYISCAL_REF_ID_ARRAY;
    UI32_T max_nbr_of_logical_ref_id = sizeof(physical_usable_ref_ids)/sizeof(physical_usable_ref_ids[0]);

    if(logical_ref_id>=max_nbr_of_logical_ref_id)
    {
        BACKDOOR_MGR_Printf("%s(%d): Invalid logical ref id=0x%lu\r\n", __FUNCTION__, __LINE__, logical_ref_id);
        return FALSE;
    }
    *physical_ref_id_p = physical_usable_ref_ids[logical_ref_id];
    return TRUE;
}

/* chip dependent functions START */
#if (SYS_HWCFG_SYNCE_CHIP==SYS_HWCFG_SYNCE_CHIP_ZL30132)
/* note that the smaller priority value gets higher priority
 * so the highest priority is 0 and the lowest priority is 2
 * priority 15 is used as disable on ZL30132
 */
#define ZL30132_MAX_PRIORITY_VAL 2
#define ZL30132_MIN_PRIORITY_VAL 0

/* Definitions for registers on ZL30132
 */
#define ZL30132_REG_REF_FAIL_ISR         0x02
#define ZL30132_REG_REF_MON_FAIL_0       0x05
#define ZL30132_REG_REF_MON_FAIL_1       0x06
#define ZL30132_REG_REF_MON_FAIL_MASK_0  0x0C
#define ZL30132_REG_REF_MON_FAIL_MASK_1  0x0D
#define ZL30132_REG_DPLL_CTRL_1          0x1E
#define ZL30132_REG_DPLL_MODESEL         0x1F
#define ZL30132_REG_DPLL_REFSEL          0x20
#define ZL30132_REG_DPLL_REF_FAIL_MASK   0x21
#define ZL30132_REG_DPLL_WAIT_TO_RESTORE 0x22
#define ZL30132_REG_DPLL_REF_REV_CTRL    0x23
#define ZL30132_REG_DPLL_REF_PRI_CTRL_0  0x24
#define ZL30132_REG_DPLL_REF_PRI_CTRL_1  0x25
#define ZL30132_REG_DPLL_HOLD_LOCK_FAIL  0x28

SYNCEDRV_TYPE_RET_T SYNCEDRV_GetChipOperatingMode_ZL30132(UI32_T* mode_p)
{
    SYNCEDRV_TYPE_RET_T ret=SYNCEDRV_TYPE_RET_OK;
    BOOL_T rc;
    UI8_T  data;

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }
#endif

    /* read current value of dpll_modesel register
     */
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        ZL30132_REG_DPLL_MODESEL, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read I2C register error\r\n", __FUNCTION__, __LINE__);
        ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
        goto fail_exit;
    }

    switch (data&0x03)
    {
        case 0: /* Manual Normal Mode */
        case 3: /* Automatic Normal Mode */
            rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                ZL30132_REG_DPLL_HOLD_LOCK_FAIL, FALSE, 1, &data);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Read I2C register error\r\n", __FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }
            if(data & BIT_0)
            {
                /* in holdover state
                 */
                *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_HOLDOVER;
            }
            else if(data & BIT_1)
            {
                /* in locked state
                 */
                *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_LOCKED;
            }
            else
            {
                /* not holdover and not locked
                 * that means the synce chip run in free-run state
                 */
                *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_FREE_RUN;
            }
            break;
        case 1: /* Manual Holdover Mode */
            *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_HOLDOVER;
            break;
        case 2: /* Manual Freerun Mode */
            *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_FREE_RUN;
            break;
        default:
            break;
    }

fail_exit:
#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }
#endif
    return ret;
}

static SYNCEDRV_TYPE_RET_T SYNCEDRV_DoSetClockSrcSelectMode_ZL30132(BOOL_T is_auto, BOOL_T is_revertive, UI32_T force_clock_src_port)
{
    UI32_T force_logical_ref_id;
    SYNCEDRV_TYPE_RET_T ret=SYNCEDRV_TYPE_RET_OK;
    UI8_T  mask, data, i, physical_ref_id;;
    BOOL_T rc, is_failed;

    if((is_auto==FALSE) && force_clock_src_port!=0)
    {
        /* find the binded logical ref_id if is_auto==FALSE
         */
        if(SYNCEDRV_OM_GetRefIdxByClockSrcPort(force_clock_src_port, &force_logical_ref_id)==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Failed to find the binded logical ref_id for port %lu\r\n", __FUNCTION__, __LINE__, force_clock_src_port);
            return SYNCEDRV_TYPE_RET_ERR_FAIL;
        }
    }

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }
#endif

    if (is_auto==TRUE)
    {
        /* set auto mode to chip
         * reg 0x1F
         * [bit 1:0] modesel 0b11 -> Automatic Normal Mode
         */
        rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
            SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
            ZL30132_REG_DPLL_MODESEL, FALSE, 0x03, 0x03);
        if(rc==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
            ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
            goto fail_exit;
        }

        /* set revertive option to chip
         * reg 0x1E
         * [bit 0]   This bit enables revertive reference switching
         *           0 -> non-revertive
         *           1 -> revertive
         * reg 0x23
         * [bit 2:0] Revertive enable bits for ref0 to ref2.
         *           Bit 0 is used for ref0, bit 1 is used for ref1, etc.
         *           0 -> non-revertive
         *           1 -> revertive
         */
        for(i=0, mask=0; i<SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC; i++)
        {
            SYNCEDRV_UTIL_ConvertLogicalRefIdToPhysicalRefId(i, &physical_ref_id);
            mask |= 1 << physical_ref_id;
        }

        is_failed=FALSE;
        if (is_revertive==TRUE)
        {
            rc =WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                ZL30132_REG_DPLL_CTRL_1, FALSE, 0x1, 0x1);
            is_failed|=!rc;
            rc =WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                ZL30132_REG_DPLL_REF_REV_CTRL, FALSE, mask, mask);
            is_failed|=!rc;
        }
        else
        {
            rc =WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                ZL30132_REG_DPLL_CTRL_1, FALSE, 0x0, 0x1);
            is_failed|=!rc;
            rc =WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                ZL30132_REG_DPLL_REF_REV_CTRL, FALSE, 0, mask);
            is_failed|=!rc;
        }

        if(is_failed==TRUE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
            ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
            goto fail_exit;
        }
    }
    else
    {

        if(force_clock_src_port==0)
        {
            /* set manual freerun mode to chip to use local clock as clock source
             * reg 0x1F
             * [bit 1:0] modesel 0b10 -> Manual Freerun Mode
             */
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                ZL30132_REG_DPLL_MODESEL, FALSE, 0x02, 0x03);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }
        }
        else
        {
            /* set manual mode to chip
             * reg 0x1F
             * [bit 1:0] modesel 0b00 -> Manual Normal Mode
             */
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                ZL30132_REG_DPLL_MODESEL, FALSE, 0x00, 0x03);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }

            /* set the active ref clock source to chip
             * reg 0x20
             * [Bit 3:0] refsel : 0b0000 -> ref 0
             *                    0b0001 -> ref 1
             *                    0b0010 -> ref 2
             */
            if(SYNCEDRV_UTIL_ConvertLogicalRefIdToPhysicalRefId((UI8_T)force_logical_ref_id, &physical_ref_id)==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Failed to convert to physical ref id\r\n", __FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }

            switch (physical_ref_id)
            {
                case 0:
                case 1:
                case 2:
                    data = physical_ref_id;
                    break;
                default:
                    BACKDOOR_MGR_Printf("%s(%d): Invalid physical reference id %u\r\n",
                        __FUNCTION__, __LINE__, physical_ref_id);
                    ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                    goto fail_exit;
                    break;
            }
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                ZL30132_REG_DPLL_REFSEL, FALSE, data, 0x0F);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }
        }
    }

fail_exit:
#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }
#endif

    return ret;
}

static BOOL_T SYNCEDRV_DoGetSyncEClockSourceStatus_ZL30132(UI32_T ref_id, BOOL_T *is_status_good_p)
{
    BOOL_T rc, ret=TRUE;
    UI8_T  data, bit_shift, physical_ref_id;

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    if(SYNCEDRV_UTIL_ConvertLogicalRefIdToPhysicalRefId((UI8_T)ref_id, &physical_ref_id)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to convert to physical ref id\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    switch (physical_ref_id)
    {
        case 0:
            bit_shift=0;
            break;
        case 1:
            bit_shift=1;
            break;
        case 2:
            bit_shift=2;
            break;
        default:
            BACKDOOR_MGR_Printf("%s(%d) Invalid physical ref id(%u)\r\n",
                __FUNCTION__, __LINE__, physical_ref_id);
            ret=FALSE;
            goto fail_exit;
    }
    /* It is found that the status in ZL30132_REG_REF_FAIL_ISR is incorrect
     * if ZL30132_REG_REF_MON_FAIL_0 and ZL30132_REG_REF_MON_FAIL_1 is changed
     * and has not been read yet. Thus add read operation to REG
     * ZL30132_REG_REF_MON_FAIL_0/1 before reading ZL30132_REG_REF_FAIL_ISR
     */
    I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        ZL30132_REG_REF_MON_FAIL_0, FALSE, 1, &data);
    I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        ZL30132_REG_REF_MON_FAIL_1, FALSE, 1, &data);
    /* It is found that if read reg ZL30132_REG_REF_FAIL_ISR without delay,
     * the status in reg ZL30132_REG_REF_FAIL_ISR will not be updated
     */      
    SYSFUN_Sleep(1);

    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        ZL30132_REG_REF_FAIL_ISR, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read I2C register error\r\n", __FUNCTION__, __LINE__);
        ret=FALSE;
        goto fail_exit;
    }

    data>>=bit_shift;
    data&=0x01;
    *is_status_good_p = (data)?FALSE:TRUE;

fail_exit:
#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    return ret;
}

static BOOL_T SYNCEDRV_DoGetActiveSyncEClockSourceRefId_ZL30132(UI32_T *ref_id_p, BOOL_T *is_locked_p)
{
    BOOL_T rc, ret=TRUE;
    UI8_T data, logical_ref_id=0;

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        ZL30132_REG_DPLL_REFSEL, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read I2C register error\r\n", __FUNCTION__, __LINE__);
        ret=FALSE;
        goto fail_exit;
    }

    data&=3;
    if(data==3)
    {
        BACKDOOR_MGR_Printf("%s(%d)Unexpected data %u\r\n", __FUNCTION__, __LINE__, data);
        ret=FALSE;
    }
    else
    {
        /* convert physical ref id to logical ref id
         */
        if(SYNCEDRV_UTIL_ConvertPhysicalRefIdToLogicalRefId(data, &logical_ref_id)==FALSE)
        {
            ret=FALSE;
            BACKDOOR_MGR_Printf("%s(%d)Failed to convert physical ref id %u to logical ref id\r\n",
                __FUNCTION__, __LINE__, data);

        }
    }
    *ref_id_p = logical_ref_id;

    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        ZL30132_REG_DPLL_HOLD_LOCK_FAIL, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read I2C register error\r\n", __FUNCTION__, __LINE__);
        ret=FALSE;
        goto fail_exit;
    }
    if(data&BIT_1)
        *is_locked_p=TRUE;
    else
        *is_locked_p=FALSE;

fail_exit:
#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif
    return ret;
}

static BOOL_T SYNCEDRV_DoSetSyncEClockSourcePriorityByRefId_ZL30132(UI32_T ref_id, UI32_T priority)
{
    UI8_T  physical_ref_id, bit_shift;
    UI8_T  reg_addr;
    BOOL_T ret=TRUE, rc;

    /* validate logical priority
     */
    if(priority>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC || priority==0)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid priority %lu\r\n", __FUNCTION__, __LINE__, priority);
        return FALSE;
    }

    if(SYNCEDRV_UTIL_ConvertLogicalRefIdToPhysicalRefId(ref_id, &physical_ref_id)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to convert to physical ref id\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    switch(physical_ref_id)
    {
        case 0: /* Ref0 */
            reg_addr=ZL30132_REG_DPLL_REF_PRI_CTRL_0;
            bit_shift=0;
            break;
        case 1: /* Ref1 */
            reg_addr=ZL30132_REG_DPLL_REF_PRI_CTRL_0;
            bit_shift=4;
            break;
        case 2: /* Ref2 */
            reg_addr=ZL30132_REG_DPLL_REF_PRI_CTRL_1;
            bit_shift=0;
            break;
        default:
            BACKDOOR_MGR_Printf("%s(%d)Invalid physical ref id %hu\r\n", __FUNCTION__, __LINE__, physical_ref_id);
            return FALSE;
            break;
    }

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg_addr, FALSE, (UI8_T)priority<<bit_shift, 0xF<<bit_shift);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
        ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
        goto fail_exit;
    }


fail_exit:
#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif
    return ret;

}

static void SYNCEDRV_DumpSyncEAsicInfo_ZL30132(void)
{
    BOOL_T rc;
    UI8_T  data, reg;
    const char* cfg_oper_mode_str[] =
        {
            "Manual Normal Mode",    /* 0 */
            "Manual Holdover Mode",  /* 1 */
            "Manual Freerun Mode",   /* 2 */
            "Automatic Normal Mode", /* 3 */
        };

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return;
    }
#endif

    /* Dump Ref0/Ref1/Ref2 monitor status
     */
    BACKDOOR_MGR_Printf("\r\nRef id  SCM failure  CFM failure  GST failure  PFM failure\r\n");
    BACKDOOR_MGR_Printf(    "----------------------------------------------------------\r\n");
    reg=ZL30132_REG_REF_MON_FAIL_0;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("%6d            %c            %c            %c            %c\r\n",
            0, (data&BIT_0)?'Y':'N', (data&BIT_1)?'Y':'N', (data&BIT_2)?'Y':'N', (data&BIT_3)?'Y':'N');
        BACKDOOR_MGR_Printf("%6d            %c            %c            %c            %c\r\n",
            1, (data&BIT_4)?'Y':'N', (data&BIT_5)?'Y':'N', (data&BIT_6)?'Y':'N', (data&BIT_7)?'Y':'N');
    }

    reg=ZL30132_REG_REF_MON_FAIL_1;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("%6d            %c            %c            %c            %c\r\n",
            2, (data&BIT_0)?'Y':'N', (data&BIT_1)?'Y':'N', (data&BIT_2)?'Y':'N', (data&BIT_3)?'Y':'N');
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* Dump Ref0/Ref1/Ref2 failure indicator mask
     */
    BACKDOOR_MGR_Printf("Ref id  SCM Mask  CFM Mask  GST Mask  PFM Mask\r\n");
    BACKDOOR_MGR_Printf("----------------------------------------------------------\r\n");
    reg=ZL30132_REG_REF_MON_FAIL_MASK_0;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("%6d         %c         %c         %c         %c\r\n",
            0, (data&BIT_0)?'N':'Y', (data&BIT_1)?'N':'Y', (data&BIT_2)?'N':'Y', (data&BIT_3)?'N':'Y');
        BACKDOOR_MGR_Printf("%6d         %c         %c         %c         %c\r\n",
            1, (data&BIT_4)?'N':'Y', (data&BIT_5)?'N':'Y', (data&BIT_6)?'N':'Y', (data&BIT_7)?'N':'Y');
    }
    reg=ZL30132_REG_REF_MON_FAIL_MASK_1;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("%6d         %c         %c         %c         %c\r\n",
            2, (data&BIT_0)?'N':'Y', (data&BIT_1)?'N':'Y', (data&BIT_2)?'N':'Y', (data&BIT_3)?'N':'Y');
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* Dump DPLL operation mode
     */
    reg=ZL30132_REG_DPLL_CTRL_1;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("Revertive switching:%s\r\n", (data&BIT_0)?"Enabled":"Disabled");
    }
    reg=ZL30132_REG_DPLL_MODESEL;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("Operation Mode Configuration:%s\r\n", cfg_oper_mode_str[data&0x3]);
    }
    reg=ZL30132_REG_DPLL_REFSEL;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("Currently selected reference input:Ref%hu\r\n", (data&0xF));
    }
    reg=ZL30132_REG_DPLL_REF_FAIL_MASK;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("Mask used for automatic reference switching:");
        BACKDOOR_MGR_Printf("SCM Mask:%c CFM Mask:%c GST Mask:%c PFM Mask:%c\r\n",
            (data&BIT_0)?'Y':'N', (data&BIT_1)?'Y':'N', (data&BIT_2)?'Y':'N', (data&BIT_3)?'Y':'N');
        BACKDOOR_MGR_Printf("Mask used for automatic holdover:");
        BACKDOOR_MGR_Printf("SCM Mask:%c CFM Mask:%c GST Mask:%c PFM Mask:%c\r\n",
            (data&BIT_4)?'Y':'N', (data&BIT_5)?'Y':'N', (data&BIT_6)?'Y':'N', (data&BIT_7)?'Y':'N');
    }
    reg=ZL30132_REG_DPLL_WAIT_TO_RESTORE;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("Wait to restored time: %hu minutes\r\n", (data&0x0F));
    }
    reg=ZL30132_REG_DPLL_REF_REV_CTRL;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("Revertive enabled for Ref0:%c\r\n", (data&BIT_0)?'Y':'N');
        BACKDOOR_MGR_Printf("Revertive enabled for Ref1:%c\r\n", (data&BIT_1)?'Y':'N');
        BACKDOOR_MGR_Printf("Revertive enabled for Ref2:%c\r\n", (data&BIT_2)?'Y':'N');
    }

    BACKDOOR_MGR_Printf("Reference input priority: (valid value=0(highest) to 2(lowest), disabled value=15)\r\n");
    reg=ZL30132_REG_DPLL_REF_PRI_CTRL_0;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("Priority of Ref0:%hu\r\n", data&0xF);
        BACKDOOR_MGR_Printf("Priority of Ref1:%hu\r\n", (data&0xF0)>>4);
    }
    reg=ZL30132_REG_DPLL_REF_PRI_CTRL_1;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("Priority of Ref2:%hu\r\n", data&0xF);
    }

    reg=ZL30132_REG_DPLL_HOLD_LOCK_FAIL;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("Holdover:%c\r\n", (data&BIT_0)?'Y':'N');
        BACKDOOR_MGR_Printf("Lock:%c\r\n", (data&BIT_1)?'Y':'N');
        BACKDOOR_MGR_Printf("Currently selected reference input failed:%c\r\n", (data&BIT_2)?'Y':'N');
    }

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return;
    }
#endif
}

static BOOL_T SYNCEDRV_SetChipMode_ZL30132(SYNCEDRV_CHIP_MODE_TYPE_T mode)
{
    /* ZL30132 only supports Normal mode
     */
    if ( mode != SYNCEDRV_CHIP_MODE_TYPE_NORMAL)
        return FALSE;

    /* Call SYNCEDRV_SetupForChipMode() to setup board dependent operations
     * Although ZL30132 do not support mode other than NORMAL,
     * still need to call SYNCEDRV_SetupForChipMode(mode) to avoid
     * compile warning.
     */
    return SYNCEDRV_SetupForChipMode(mode);
}

static BOOL_T SYNCEDRV_SetOutputFreqOffsetInPPM_ZL30132(float ppm)
{
    /* ZL30132 not support
     */
    return FALSE;
}

#elif (SYS_HWCFG_SYNCE_CHIP==SYS_HWCFG_SYNCE_CHIP_IDT82V3399)
/* note that the smaller priority value gets higher priority
 * so the highest priority is 1 and the lowest priority is 15
 * priority 0 is used as disable on IDT82V3399
 */
#define IDT82V3399_MAX_PRIORITY_VAL 15
#define IDT82V3399_MIN_PRIORITY_VAL 1

/* Definitions for registers on IDT82V3399
 */
#define IDT82V3399_REG_T4_T0_REG_SEL_CNFG        0x07
#define IDT82V3399_REG_T0_INPUT_MODE_CNFG        0x09
#define IDT82V3399_REG_INTERRUPTS1_STS           0x0D
#define IDT82V3399_REG_INTERRUPTS2_STS           0x0E
#define IDT82V3399_REG_INTERRUPTS3_STS           0x0F
#define IDT82V3399_REG_INTERRUPTS1_ENABLE_CNFG   0x10
#define IDT82V3399_REG_INTERRUPTS2_ENABLE_CNFG   0x11
#define IDT82V3399_REG_INTERRUPTS3_ENABLE_CNFG   0x12
#define IDT82V3399_REG_IN3_IN4_SEL_PRIORITY_CNFG 0x27
#define IDT82V3399_REG_IN1_IN2_SEL_PRIORITY_CNFG 0x28
#define IDT82V3399_REG_IN5_SEL_PRIORITY_CNFG     0x2A
#define IDT82V3399_REG_IN6_SEL_PRIORITY_CNFG     0x2B
#define IDT82V3399_REG_IN3_IN4_STS               0x44
#define IDT82V3399_REG_IN1_IN2_STS               0x45
#define IDT82V3399_REG_IN5_STS                   0x47
#define IDT82V3399_REG_IN6_STS                   0x48
#define IDT82V3399_REG_INPUT_VALID1_STS          0x4A
#define IDT82V3399_REG_INPUT_VALID2_STS          0x4B
#define IDT82V3399_REG_REMOTE_INPUT_VALID1_CNFG  0x4C
#define IDT82V3399_REG_REMOTE_INPUT_VALID2_CNFG  0x4D
#define IDT82V3399_REG_PRIORITY_TABLE1_STS       0x4E
#define IDT82V3399_REG_PRIORITY_TABLE2_STS       0x4F
#define IDT82V3399_REG_T0_INPUT_SEL_CNFG         0x50
#define IDT82V3399_REG_OPERATING_STS             0x52
#define IDT82V3399_REG_T0_OPERATING_MODE_CNFG    0x53
#define IDT82V3399_REG_T4_OPERATING_MODE_CNFG    0x54
#define IDT82V3399_REG_T0_HOLDOVER_MODE_CNFG     0x5C
#define IDT82V3399_REG_HOLDOVER_FREQ_CNFG_1      0x5D
#define IDT82V3399_REG_HOLDOVER_FREQ_CNFG_2      0x5E
#define IDT82V3399_REG_HOLDOVER_FREQ_CNFG_3      0x5F

/* This is a temporary code for ease of display status change of
 * SyncE Chip when debug message level is changed to a value
 * larger than SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG
 */
#if 1
extern const char* SYNC_E_PMGR_SyncEChipOperatingModeToStr(UI32_T mode);
static void SYNCEDRV_TaskMain(void);
/* FUNCTION NAME: SYNCEDRV_CreateTask
 *-----------------------------------------------------------------------------
 * PURPOSE: Create task in SYNCEDRV
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYNCEDRV_CreateTask(void)
{
    UI32_T thread_id;

    if(SYSFUN_SpawnThread(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                          SYSFUN_SCHED_DEFAULT,
                          "SYNCEDRV_TASK",
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          SYNCEDRV_TaskMain,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("\r\n%s:Spawn SYSDRV thread fail.", __FUNCTION__);
    }

}
/* FUNCTION NAME: SYNCEDRV_TaskMain
 *-----------------------------------------------------------------------------
 * PURPOSE: Main routine of SYNCEDRV task
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static void SYNCEDRV_TaskMain(void)
{
    UI8_T  data, data2;
    BOOL_T rc, rc2;

    /* clear interrupts on IDT
     */
    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_INTERRUPTS1_STS, FALSE, 0x3C, 0x3C);
    if(rc==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Update I2C register error\r\n");
    }

    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_INTERRUPTS2_STS, FALSE, 0xC5, 0xC5);
    if(rc==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Update I2C register error\r\n");
    }

    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_INTERRUPTS3_STS, FALSE, 0xD0, 0xD0);
    if(rc==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Update I2C register error\r\n");
    }

    /* enable interrupt on IDT
     */
    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_INTERRUPTS1_ENABLE_CNFG, FALSE, 0x3C, 0x3C);
    if(rc==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Update I2C register error\r\n");
    }

    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_INTERRUPTS2_ENABLE_CNFG, FALSE, 0xC5, 0xC5);
    if(rc==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Update I2C register error\r\n");
    }

    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_INTERRUPTS3_ENABLE_CNFG, FALSE, 0xD0, 0xD0);
    if(rc==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Update I2C register error\r\n");
    }

    while(TRUE)
    {
        /* polling interrupt status */
        SYSFUN_Sleep(100); /* sleep 1 second */

        /* read reg IDT82V3399_REG_INPUT_VALID1_STS to get current valid
         * status of IN1-IN4
         */
        rc2=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
            SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
            IDT82V3399_REG_INPUT_VALID1_STS, FALSE, 1, &data2);
        if(rc2==FALSE)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Read I2C register error\r\n");
        }

        
        /* check interrupts status IDT82V3399_REG_INTERRUPTS1_STS
         */
        rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
            SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
            IDT82V3399_REG_INTERRUPTS1_STS, FALSE, 1, &data);
        if(rc==FALSE)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Read I2C register error\r\n");
        }
        else
        {
            if((data & 0x3C)!=0)
            {
                if(data & BIT_2)
                {
                    if(rc2==TRUE)
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN3 valid status changed to %s\r\n", (data2&BIT_2)?"Valid":"Invalid");
                    }
                    else
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN3 valid status changed\r\n");
                    }
                }
                if(data & BIT_3)
                {
                    if(rc2==TRUE)
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN4 valid status changed to %s\r\n", (data2&BIT_3)?"Valid":"Invalid");
                    }
                    else
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN4 valid status changed\r\n");
                    }
                }
                if(data & BIT_4)
                {
                    if(rc2==TRUE)
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN1 valid status changed to %s\r\n", (data2&BIT_4)?"Valid":"Invalid");
                    }
                    else
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN1 valid status changed\r\n");
                    }
                }
                if(data & BIT_5)
                {
                    if(rc2==TRUE)
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN2 valid status changed to %s\r\n", (data2&BIT_5)?"Valid":"Invalid");
                    }
                    else
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN2 valid status changed\r\n");
                    }
                }
                rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                    SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                    0x0D, FALSE, data, 0x3C);
                if(rc==FALSE)
                {
                    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Update I2C register error\r\n");
                }

            }
        }

        /* check interrupts status IDT82V3399_REG_INTERRUPTS2_STS
         * read reg IDT82V3399_REG_INPUT_VALID1_STS to get current valid
         * status of IN1-IN4
         */
        rc2=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
            SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
            IDT82V3399_REG_INPUT_VALID2_STS, FALSE, 1, &data2);
        if(rc2==FALSE)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Read I2C register error\r\n");
        }

        rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
            SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
            IDT82V3399_REG_INTERRUPTS2_STS, FALSE, 1, &data);
        if(rc==FALSE)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Read I2C register error\r\n");
        }
        else
        {
            if((data & 0xC5)!=0)
            {
                if(data & BIT_0)
                {
                    if(rc2==TRUE)
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN5 valid status changed to %s\r\n", (data2&BIT_0)?"Valid":"Invalid");
                    }
                    else
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN5 valid status changed\r\n");
                    }
                }
                if(data & BIT_2)
                {
                    if(rc2==TRUE)
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN6 valid status changed to %s\r\n", (data2&BIT_2)?"Valid":"Invalid");
                    }
                    else
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "IN6 valid status changed\r\n");
                    }
                }

                if(data & BIT_6)
                {
                    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "T0 selected input clock changed to failed\r\n");
                }
                if(data & BIT_7)
                {
                    UI32_T chip_op_mode;
                    const char* chip_op_mode_str_p;


                    if(SYNCEDRV_GetChipOperatingMode(&chip_op_mode)==SYNCEDRV_TYPE_RET_OK)
                    {
                       chip_op_mode_str_p=SYNC_E_PMGR_SyncEChipOperatingModeToStr(chip_op_mode);
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "T0 operating mode changed to %s\r\n", (chip_op_mode_str_p==NULL)?"Error":chip_op_mode_str_p);
                    }
                    else
                    {
                        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "T0 operating mode changed\r\n");
                    }
                }
                rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                    SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                    0x0E, FALSE, data, 0xC5);
                if(rc==FALSE)
                {
                    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Update I2C register error\r\n");
                }

            }
        }

        /* check interrupts status IDT82V3399_REG_INTERRUPTS3_STS
         */
        rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
            SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
            IDT82V3399_REG_INTERRUPTS3_STS, FALSE, 1, &data);
        if(rc==FALSE)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Read I2C register error\r\n");
        }
        else
        {
            if((data & 0xD0)!=0)
            {
                if(data & BIT_4)
                {
                    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "No qualified input clocks for T4\r\n");
                }
                if(data & BIT_6)
                {
                    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "T4 Locked status changed\r\n");
                }
                if(data & BIT_7)
                {
                    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "External sync alarm raised.\r\n");
                }
                rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                    SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                    0x0F, FALSE, data, 0xD0);
                if(rc==FALSE)
                {
                    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Update I2C register error\r\n");
                }

            }
        }

    } /* end of while(TRUE) */
}
#endif

/* Note: On IDT82V3399, there are two DPLLs. The APIs for IDT82V3399 only support
 *       to operate on the first DPLL (i.e. T0 DPLL).
 */

/* IDT82V3399 specific utility functions START */
#if 0 /* not used due to design change, keep if it need to be used in the future */
/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_IDT82V3399_UTIL_ConvertPhysicalRefIdToRegVal
 *-----------------------------------------------------------------
 * FUNCTION: This function convert the physical input reference id(0 based)
 *           to the value to be set to the register 0x50 (IDT82V3399_REG_T0_INPUT_SEL_CNFG)
 *
 * INPUT   : physical_ref_id - physical reference id
 * OUTPUT  : reg_val_p       - the converted value to be set to the register
 * RETURN  : TRUE  - Success
 *           FALSE - Failed
 * NOTE    : The mapping of PIN name and physical input reference id:
 *           PIN Name       Physical Input Reference ID
 *           ------------------------------------------
 *           IN1            0
 *           IN2            1
 *           IN3            2
 *           IN4            3
 *           IN5            4
 *           IN6            5
 *----------------------------------------------------------------*/
static BOOL_T SYNCEDRV_IDT82V3399_UTIL_ConvertPhysicalRefIdToRegVal(UI8_T physical_ref_id, UI8_T* reg_val_p)
{
    BOOL_T ret=TRUE;

    switch (physical_ref_id)
    {
        case 0:
            *reg_val_p = 0x05;
            break;
        case 1:
            *reg_val_p = 0x06;
            break;
        case 2:
            *reg_val_p = 0x03;
            break;
        case 3:
            *reg_val_p = 0x04;
            break;
        case 4:
            *reg_val_p = 0x09;
            break;
        case 5:
            *reg_val_p = 0x0B;
            break;
        default:
            BACKDOOR_MGR_Printf("%s(%d): Invalid physical reference id %u\r\n",
                __FUNCTION__, __LINE__, physical_ref_id);
            ret=FALSE;
            break;
    }
    return ret;
}
#endif

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_IDT82V3399_UTIL_ConvertRegValToPhysicalRefId
 *-----------------------------------------------------------------
 * FUNCTION: This function convert the value read from the register to
 *           physical input reference id(0 based)
 *
 * INPUT   : reg_val - the value read from a register which indicates a input
 *                     reference id
 * OUTPUT  : physical_ref_id_p - the physical reference id which is converted
 *                               from the given register value
 * RETURN  : TRUE  - Success
 *           FALSE - Failed
 * NOTE    : The mapping of PIN name and physical input reference id:
 *           PIN Name       Physical Input Reference ID
 *           ------------------------------------------
 *           IN1            0
 *           IN2            1
 *           IN3            2
 *           IN4            3
 *           IN5            4
 *           IN6            5
 *----------------------------------------------------------------*/
static BOOL_T SYNCEDRV_IDT82V3399_UTIL_ConvertRegValToPhysicalRefId(UI8_T reg_val, UI8_T* physical_ref_id_p)
{
    BOOL_T ret=TRUE;

    switch(reg_val)
    {
        case 0x03:
            *physical_ref_id_p=2;
            break;
        case 0x04:
            *physical_ref_id_p=3;
            break;
        case 0x05:
            *physical_ref_id_p=0;
            break;            
        case 0x06:
            *physical_ref_id_p=1;
            break;
        case 0x09:
            *physical_ref_id_p=4;
            break;
        case 0x0B:
            *physical_ref_id_p=5;
            break;
        default:
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG, "Invalid reg_val %u\r\n", reg_val);
            ret=FALSE;
            break;
    }
    return ret;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_IDT82V3399_UTIL_ConvertValidInputBitmapToRemoteInputValidRegVals
 *-----------------------------------------------------------------
 * FUNCTION: This function convert the given valid input bitmap to
 *           the register settings of reg 0x4C/0x4D
 *
 * INPUT   : valid_in_bmp - the bitmap of valid reference input which allows
 *                          to be selected as active clock source.
 * OUTPUT  : reg_vals     - the first element keeps the register value for 0x4C
 *                          the second elemtn keeps the register value for 0x4D
 * RETURN  : None
 * NOTE    : The mapping of bit and physical input reference id
 *           Bit            Physical Input Reference ID
 *           ------------------------------------------
 *           Bit 0          0
 *           Bit 1          1
 *           Bit 2          2
 *           Bit 3          3
 *           Bit 4          4
 *           Bit 5          5
 *----------------------------------------------------------------*/
static void SYNCEDRV_IDT82V3399_UTIL_ConvertValidInputBitmapToRemoteInputValidRegVals(UI8_T valid_in_bmp, UI8_T reg_vals[2])
{
    UI8_T i, single_bit_val=1;

    reg_vals[0]=reg_vals[1]=0xFF;

    for(i=0; i<6; i++)
    {
        if( (valid_in_bmp & single_bit_val) != 0)
        {
            switch (single_bit_val)
            {
                case BIT_0:
                    reg_vals[0] ^= BIT_4;
                    break;
                case BIT_1:
                    reg_vals[0] ^= BIT_5;
                    break;
                case BIT_2:
                    reg_vals[0] ^= BIT_2;
                    break;
                case BIT_3:
                    reg_vals[0] ^= BIT_3;
                    break;
                case BIT_4:
                    reg_vals[1] ^= BIT_0;
                    break;
                case BIT_5:
                    reg_vals[1] ^= BIT_2;
                    break;
                default:
                    BACKDOOR_MGR_Printf("%s(%d)Invalid single_bit_val=0x%02x\r\n", __FUNCTION__, __LINE__, single_bit_val);
                    break;
            }
        }
        single_bit_val<<=1;
    }
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_IDT82V3399_UTIL_ConvertPPMToRegVal
 *-----------------------------------------------------------------
 * FUNCTION: This function convert the given ppm value into the
 *           format of the register IDT82V3399_REG_HOLDOVER_FREQ_CNFG_1-3
 * INPUT   : ppm          - ppm value
 * OUTPUT  : reg_vals     - converted register value
 * RETURN  : TRUE  - Successfully
 *           FALSE - Failed to convert
 * NOTE    : reg_vals[0] -> IDT82V3399_REG_HOLDOVER_FREQ_CNFG_1
 *           reg_vals[1] -> IDT82V3399_REG_HOLDOVER_FREQ_CNFG_2
 *           reg_vals[2] -> IDT82V3399_REG_HOLDOVER_FREQ_CNFG_3
 *----------------------------------------------------------------*/
static BOOL_T SYNCEDRV_IDT82V3399_UTIL_ConvertPPMToRegVal(float ppm, UI8_T reg_vals[3])
{
    #define REG_TO_PPM_FACTOR (0.000011F)
    #define PPM_MAX (float)((0x7FFFFFL)*(REG_TO_PPM_FACTOR))
    #define PPM_MIN (float)((-0x800000L)*(REG_TO_PPM_FACTOR))

    I32_T  converted_val;

    /* validation for ppm
     */
    if ((ppm > PPM_MAX) || (ppm < PPM_MIN))
    {
        return FALSE;
    }

    converted_val = ppm / REG_TO_PPM_FACTOR;

    memset(reg_vals, 0, sizeof(UI8_T)*3);
    reg_vals[0] = (UI8_T)(converted_val & 0xFF);
    reg_vals[1] = (UI8_T)((converted_val>>8) & 0xFF);
    reg_vals[2] = (UI8_T)((converted_val>>16) & 0x7F);
    if (converted_val<0)
    {
        /* set signed bit as 1 if the value is negative
         */
        reg_vals[2] |= 0x80;
    }
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_IDT82V3399_UTIL_ConvertRegValsToPPM
 *-----------------------------------------------------------------
 * FUNCTION: This function convert the value in the format of the register
 *           IDT82V3399_REG_HOLDOVER_FREQ_CNFG_1-3 into ppm.
 * INPUT   : reg_val_p    - register value
 * OUTPUT  : ppm          - converted ppm value
 * RETURN  : None
 * NOTE    : reg_vals[0] -> IDT82V3399_REG_HOLDOVER_FREQ_CNFG_1
 *           reg_vals[1] -> IDT82V3399_REG_HOLDOVER_FREQ_CNFG_2
 *           reg_vals[2] -> IDT82V3399_REG_HOLDOVER_FREQ_CNFG_3
 *----------------------------------------------------------------*/
static void SYNCEDRV_IDT82V3399_UTIL_ConvertRegValsToPPM(UI8_T reg_vals[3], float* ppm_p)
{
    union
    {
        UI32_T  u32_data;
        I32_T   i32_data;
    } converted_val;

    converted_val.u32_data  = reg_vals[0];
    converted_val.u32_data |= reg_vals[1]<<8;
    converted_val.u32_data |= reg_vals[2]<<16;

    /* check the sign bit(bit 23) of the register value
     */
    if (reg_vals[2] & 0x80)
    {
        /* value is negative
         */
        converted_val.u32_data |= 0xFF<<24;
    }
    *ppm_p = converted_val.i32_data * 0.000011;
}

/* IDT82V3399 specific utility functions END   */

SYNCEDRV_TYPE_RET_T SYNCEDRV_GetChipOperatingMode_IDT82V3399(UI32_T* mode_p)
{
    BOOL_T rc;
    UI8_T  data;

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }
#endif

    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_OPERATING_STS, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read I2C register error\r\n", __FUNCTION__, __LINE__);
        return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }

    switch (data & 0x07)
    {
        case 0:
            *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_UNKNOWN;
            break;
        case 1:
            *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_FREE_RUN;
            break;
        case 2:
            *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_HOLDOVER;
            break;
        case 3:
            *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_UNKNOWN;
            break;
        case 4:
            *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_LOCKED;
            break;
        case 5:
            *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_PRE_LOCKED2;
            break;
        case 6:
            *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_PRE_LOCKED;
            break;
        case 7:
            *mode_p = SYNCEDRV_TYPE_CHIP_OPERATING_MODE_LOST_PHASE;
            break;
        default:
            return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }
#endif

    return SYNCEDRV_TYPE_RET_OK;
}

static SYNCEDRV_TYPE_RET_T SYNCEDRV_DoSetClockSrcSelectMode_IDT82V3399(BOOL_T is_auto, BOOL_T is_revertive, UI32_T force_clock_src_port)
{
    UI32_T force_logical_ref_id;
    SYNCEDRV_TYPE_RET_T ret=SYNCEDRV_TYPE_RET_OK;
    UI8_T  physical_ref_id;
    UI8_T  valid_input_bmp, remote_input_valid_reg_vals[2];
    BOOL_T rc;    

    if((is_auto==FALSE) && force_clock_src_port!=0)
    {
        /* find the binded logical ref_id
         */
        if(SYNCEDRV_OM_GetRefIdxByClockSrcPort(force_clock_src_port, &force_logical_ref_id)==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Failed to find the binded logical ref_id for port %lu\r\n", __FUNCTION__, __LINE__, force_clock_src_port);
            return SYNCEDRV_TYPE_RET_ERR_FAIL;
        }
    }

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }
#endif

    if (is_auto==TRUE)
    {
        /* set auto mode to chip
         * reg 0x53
         * [bit 2:0] T0_OPERATING_MODE 0b00 -> Automatic
         */
        rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
            SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
            IDT82V3399_REG_T0_OPERATING_MODE_CNFG, FALSE, 0x00, 0x07);
        if(rc==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
            ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
            goto fail_exit;
        }

        /* set revertive option to chip
         * reg 0x09
         * [bit 0] This bit selects Revertive or Non-Revertive switching for T0 path.
         *           0 -> non-revertive
         *           1 -> revertive
         */
        if (is_revertive==TRUE)
        {
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_T0_INPUT_MODE_CNFG, FALSE, 0x01, 0x01);
        }
        else
        {
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_T0_INPUT_MODE_CNFG, FALSE, 0x00, 0x01);
        }

        if(rc==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
            ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
            goto fail_exit;
        }

        /* set T0 selected input clock configuration
         * reg 0x50
         * [Bit 3:0] T0_INPUT_SEL : 0b0000 -> Automatic selection
         */
        rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_T0_INPUT_SEL_CNFG, FALSE, 0x00, 0x0F);

        if(rc==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
            ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
            goto fail_exit;
        }

        /* set physical ref id 0(IN1) to 5(IN6) as valid reference input
         */
        valid_input_bmp=0x3F;
        SYNCEDRV_IDT82V3399_UTIL_ConvertValidInputBitmapToRemoteInputValidRegVals(valid_input_bmp, remote_input_valid_reg_vals);

        rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_REMOTE_INPUT_VALID1_CNFG, FALSE, remote_input_valid_reg_vals[0], 0x3C);

        if(rc==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
            ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
            goto fail_exit;
        }

        rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_REMOTE_INPUT_VALID2_CNFG, FALSE, remote_input_valid_reg_vals[1], 0x05);

        if(rc==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
            ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
            goto fail_exit;
        }

    }
    else
    {
        if(force_clock_src_port==0)
        {
            /* set manual freerun mode to chip to use local clock as clock source
             * reg 0x53
             * [bit 2:0] T0_OPERATING_MODE 0b001 -> Forced - Free-Run
             */
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_T0_OPERATING_MODE_CNFG, FALSE, 0x01, 0x07);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }

        }
        else
        {
            /* It is found that when clock source become invalid, T0 operation
             * status will not change to holdover automatically. So use automatic
             * mode and invalidate all of the reference input except the one
             * specified by the upper layer.
             */
            #if 0
            /* set manual mode to chip
             * reg 0x53
             * [bit 2:0] T0_OPERATING_MODE 0b100 -> Forced - Locked.
             */
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_T0_OPERATING_MODE_CNFG, FALSE, 0x04, 0x07);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }

            /* set the active ref clock source to chip
             * reg 0x50
             * [bit 3:0] T0_INPUT_SEL : 0b0011 -> Forced selection - IN3(physical ref id 2) is selected
             *                          0b0100 -> Forced selection - IN4(physical ref id 3) is selected
             *                          0b0101 -> Forced selection - IN1(physical ref id 0) is selected
             *                          0b0110 -> Forced selection - IN2(physical ref id 1) is selected
             *                          0b1001 -> Forced selection - IN5(physical ref id 4) is selected
             *                          0b1011 -> Forced selection - IN6(physical ref id 5) is selected
             */
            if(SYNCEDRV_UTIL_ConvertLogicalRefIdToPhysicalRefId((UI8_T)force_logical_ref_id, &physical_ref_id)==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d):Failed to convert to physical ref id\r\n",__FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }

            if(SYNCEDRV_IDT82V3399_UTIL_ConvertPhysicalRefIdToRegVal(physical_ref_id, &data)==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d): Invalid physical reference id %u\r\n",
                    __FUNCTION__, __LINE__, physical_ref_id);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_T0_INPUT_SEL_CNFG, FALSE, data, 0x0F);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }
            #else
            /* set auto mode to chip
             * reg 0x53
             * [bit 2:0] T0_OPERATING_MODE 0b00 -> Automatic
             */
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_T0_OPERATING_MODE_CNFG, FALSE, 0x00, 0x07);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }
            /* set forced physical ref id as valid reference input
             * ref id other than forced physical ref id are set as invalid reference input
             */
            if(SYNCEDRV_UTIL_ConvertLogicalRefIdToPhysicalRefId((UI8_T)force_logical_ref_id, &physical_ref_id)==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d):Failed to convert to physical ref id\r\n",__FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }

            SYNCEDRV_IDT82V3399_UTIL_ConvertValidInputBitmapToRemoteInputValidRegVals(1<<physical_ref_id, remote_input_valid_reg_vals);
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                    SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                    IDT82V3399_REG_REMOTE_INPUT_VALID1_CNFG, FALSE, remote_input_valid_reg_vals[0], 0x3C);
    
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }
    
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                    SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                    IDT82V3399_REG_REMOTE_INPUT_VALID2_CNFG, FALSE, remote_input_valid_reg_vals[1], 0x05);
    
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
                goto fail_exit;
            }

            #endif /* end of #if 0 */
        }
    }

fail_exit:
#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return SYNCEDRV_TYPE_RET_ERR_FAIL;
    }
#endif

    return ret;
}

static BOOL_T SYNCEDRV_DoGetSyncEClockSourceStatus_IDT82V3399(UI32_T ref_id, BOOL_T *is_status_good_p)
{
    BOOL_T rc, ret=TRUE;
    UI8_T  data, bit_shift, reg_addr, physical_ref_id;

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    if(SYNCEDRV_UTIL_ConvertLogicalRefIdToPhysicalRefId((UI8_T)ref_id, &physical_ref_id)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to convert to physical ref id\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    switch (physical_ref_id)
    {
        case 0:
            reg_addr = IDT82V3399_REG_INPUT_VALID1_STS;
            bit_shift= 4;
            break;
        case 1:
            reg_addr = IDT82V3399_REG_INPUT_VALID1_STS;
            bit_shift= 5;
            break;
        case 2:
            reg_addr = IDT82V3399_REG_INPUT_VALID1_STS;
            bit_shift= 2;
            break;
        case 3:
            reg_addr = IDT82V3399_REG_INPUT_VALID1_STS;
            bit_shift= 3;
            break;
        case 4:
            reg_addr = IDT82V3399_REG_INPUT_VALID2_STS;
            bit_shift= 0;
            break;
        case 5:
            reg_addr = IDT82V3399_REG_INPUT_VALID2_STS;
            bit_shift= 2;
            break;
        default:
            BACKDOOR_MGR_Printf("%s(%d) Invalid physical ref id(%u)\r\n",
                __FUNCTION__, __LINE__, physical_ref_id);
            ret=FALSE;
            goto fail_exit;
    }

    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg_addr, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read I2C register error\r\n", __FUNCTION__, __LINE__);
        ret=FALSE;
        goto fail_exit;
    }

    data>>=bit_shift;
    data&=0x01;
    *is_status_good_p = (data)?TRUE:FALSE;

fail_exit:
#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    return ret;
}

static BOOL_T SYNCEDRV_DoGetActiveSyncEClockSourceRefId_IDT82V3399(UI32_T *ref_id_p, BOOL_T *is_locked_p)
{
    BOOL_T rc, ret=TRUE, force_selection;
    UI8_T data, physical_ref_id, logical_ref_id=0;

#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_T0_OPERATING_MODE_CNFG, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read I2C register error\r\n", __FUNCTION__, __LINE__);
        ret=FALSE;
        goto fail_exit;
    }

    if( (data & 0x07) == 0x04)
        force_selection=TRUE;
    else
        force_selection=FALSE;

    if(force_selection==TRUE)
    {
        rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
            SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
            IDT82V3399_REG_T0_INPUT_SEL_CNFG, FALSE, 1, &data);
        if(rc==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Read I2C register error\r\n", __FUNCTION__, __LINE__);
            ret=FALSE;
            goto fail_exit;
        }

        /* extract the register value from bit [3:0] T0_INPUT_SEL
         */
        data &= 0x0F;
    }
    else
    {
        rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
            SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
            IDT82V3399_REG_PRIORITY_TABLE1_STS, FALSE, 1, &data);
        if(rc==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Read I2C register error\r\n", __FUNCTION__, __LINE__);
            ret=FALSE;
            goto fail_exit;
        }

        /* extract the register value from bit [3:0] CURRENTLY_SELECTED_INPUT
         */
        data &= 0x0F;
    }

    /* convert register value to physical ref id
     */
    if(SYNCEDRV_IDT82V3399_UTIL_ConvertRegValToPhysicalRefId((data&0x0F), &physical_ref_id)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG, "Failed to convert reg val to physical ref id\r\n");
        ret=FALSE;
        goto fail_exit;
    }


    /* convert physical ref id to logical ref id
     */
    if(SYNCEDRV_UTIL_ConvertPhysicalRefIdToLogicalRefId(physical_ref_id, &logical_ref_id)==FALSE)
    {
        ret=FALSE;
        BACKDOOR_MGR_Printf("%s(%d)Failed to convert physical ref id %u to logical ref id\r\n",
            __FUNCTION__, __LINE__, data);

    }
    *ref_id_p = logical_ref_id;

    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_OPERATING_STS, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read I2C register error\r\n", __FUNCTION__, __LINE__);
        ret=FALSE;
        goto fail_exit;
    }
    if(data&BIT_3)
        *is_locked_p=TRUE;
    else
        *is_locked_p=FALSE;

fail_exit:
#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif
    return ret;
}

static BOOL_T SYNCEDRV_DoSetSyncEClockSourcePriorityByRefId_IDT82V3399(UI32_T ref_id, UI32_T priority)
{
    UI8_T  physical_ref_id, bit_shift;
    UI8_T  reg_addr;
    BOOL_T ret=TRUE, rc;

    /* validate logical priority
     */
    if(priority>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC || priority==0)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid priority %lu\r\n", __FUNCTION__, __LINE__, priority);
        return FALSE;
    }

    if(SYNCEDRV_UTIL_ConvertLogicalRefIdToPhysicalRefId(ref_id, &physical_ref_id)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to convert to physical ref id\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    switch(physical_ref_id)
    {
        case 0: /* IN1 */
            reg_addr=IDT82V3399_REG_IN1_IN2_SEL_PRIORITY_CNFG;
            bit_shift=0;
            break;
        case 1: /* IN2 */
            reg_addr=IDT82V3399_REG_IN1_IN2_SEL_PRIORITY_CNFG;
            bit_shift=4;
            break;
        case 2: /* IN3 */
            reg_addr=IDT82V3399_REG_IN3_IN4_SEL_PRIORITY_CNFG;
            bit_shift=0;
            break;
        case 3: /* IN4 */
            reg_addr=IDT82V3399_REG_IN3_IN4_SEL_PRIORITY_CNFG;
            bit_shift=4;
            break;
        case 4: /* IN5 */
            reg_addr=IDT82V3399_REG_IN5_SEL_PRIORITY_CNFG;
            bit_shift=0;
            break;
        case 5: /* IN6 */
            reg_addr=IDT82V3399_REG_IN6_SEL_PRIORITY_CNFG;
            bit_shift=0;
            break;
        default:
            BACKDOOR_MGR_Printf("%s(%d)Invalid physical ref id %hu\r\n", __FUNCTION__, __LINE__, physical_ref_id);
            return FALSE;
            break;
    }


#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg_addr, FALSE, (UI8_T)priority<<bit_shift, 0xF<<bit_shift);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
        ret=SYNCEDRV_TYPE_RET_ERR_FAIL;
        goto fail_exit;
    }

fail_exit:
#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    return ret;

}

static void SYNCEDRV_DumpSyncEAsicInfo_IDT82V3399(void)
{
    UI8_T  data, reg, valid_bmp, physical_ref_id;
    BOOL_T rc;
    const char* opmode_cfg_str[]=
        {"Automatic",           /* 0 */
         "Forced - Free-Run",   /* 1 */
         "Forced - Holdover",   /* 2 */
         "Reserved",            /* 3 */
         "Forced - Locked",     /* 4 */
         "Forced - Pre-Locked2",/* 5 */
         "Forced - Pre-Locked", /* 6 */
         "Forced - Lost-Phase"  /* 7 */
        };


#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_SetAndLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX, SYS_HWCFG_SYNCE_CHIP_I2C_MUX_CHANNEL_BITMAP)==FALSE)
    {
        _BACKDOOR_MGR_Printf("%s(%d)Failed to set and lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

    /* Dump SOFT_ALARM/HARD_ALARM/NO_ACTIVITY_ALARM/PH_LOCK_ALARM status
     */
    BACKDOOR_MGR_Printf("\r\n     SOFT_ALM HARD_ALM NO_ACT_ALM PH_LOCK_ALM\r\n");
    BACKDOOR_MGR_Printf(    "---------------------------------------------\r\n");

    reg=IDT82V3399_REG_IN1_IN2_STS;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("IN1  %5u    %5u    %4u       %5u\r\n",
            (data & BIT_3)>>3, (data & BIT_2)>>2, (data & BIT_1)>>1, (data & BIT_0));
        BACKDOOR_MGR_Printf("IN2  %5u    %5u    %4u       %5u\r\n",
            (data & BIT_7)>>7, (data & BIT_6)>>6, (data & BIT_5)>>5, (data & BIT_4)>>4);
    }

    reg=IDT82V3399_REG_IN3_IN4_STS;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("IN3  %5u    %5u    %4u       %5u\r\n",
            (data & BIT_3)>>3, (data & BIT_2)>>2, (data & BIT_1)>>1, (data & BIT_0));
        BACKDOOR_MGR_Printf("IN4  %5u    %5u    %4u       %5u\r\n",
            (data & BIT_7)>>7, (data & BIT_6)>>6, (data & BIT_5)>>5, (data & BIT_4)>>4);
    }

    reg=IDT82V3399_REG_IN5_STS;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("IN5  %5u    %5u    %4u       %5u\r\n",
            (data & BIT_3)>>3, (data & BIT_2)>>2, (data & BIT_1)>>1, (data & BIT_0));
    }

    reg=IDT82V3399_REG_IN6_STS;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("IN6  %5u    %5u    %4u       %5u\r\n",
            (data & BIT_3)>>3, (data & BIT_2)>>2, (data & BIT_1)>>1, (data & BIT_0));
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* Dump INPUT VALID status
     */
    valid_bmp=0;
    reg=IDT82V3399_REG_INPUT_VALID1_STS;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        if(data & BIT_4)
            valid_bmp|=BIT_0; /* IN1 valid */
        if(data & BIT_5)
            valid_bmp|=BIT_1; /* IN2 valid */
        if(data & BIT_2)
            valid_bmp|=BIT_2; /* IN3 valid */
        if(data & BIT_3)
            valid_bmp|=BIT_3; /* IN4 valid */
    }
    reg=IDT82V3399_REG_INPUT_VALID2_STS;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        if(data & BIT_0)
            valid_bmp|=BIT_4; /* IN5 valid */
        if(data & BIT_2)
            valid_bmp|=BIT_5; /* IN6 valid */
    }
    BACKDOOR_MGR_Printf("INPUT VALID IN1 IN2 IN3 IN4 IN5 IN6\r\n");
    BACKDOOR_MGR_Printf("-----------------------------------\r\n");
    BACKDOOR_MGR_Printf("             %c   %c   %c   %c   %c   %c\r\n",
        (valid_bmp&BIT_0)?'Y':'N', (valid_bmp&BIT_1)?'Y':'N',
        (valid_bmp&BIT_2)?'Y':'N', (valid_bmp&BIT_3)?'Y':'N',
        (valid_bmp&BIT_4)?'Y':'N', (valid_bmp&BIT_5)?'Y':'N');
    BACKDOOR_MGR_Printf("\r\n");

    /* Dump REMOTE INPUT VALID status
     */
    valid_bmp=0;
    reg=IDT82V3399_REG_REMOTE_INPUT_VALID1_CNFG;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        if((data & BIT_4)==0)
            valid_bmp|=BIT_0; /* IN1 Enabled */
        if((data & BIT_5)==0)
            valid_bmp|=BIT_1; /* IN2 Enabled */
        if((data & BIT_2)==0)
            valid_bmp|=BIT_2; /* IN3 Enabled */
        if((data & BIT_3)==0)
            valid_bmp|=BIT_3; /* IN4 Enabled */
    }
    reg=IDT82V3399_REG_REMOTE_INPUT_VALID2_CNFG;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        if((data & BIT_0)==0)
            valid_bmp|=BIT_4; /* IN5 valid */
        if((data & BIT_2)==0)
            valid_bmp|=BIT_5; /* IN6 valid */
    }
    BACKDOOR_MGR_Printf("For automatic clock source selection:\r\n");
    BACKDOOR_MGR_Printf("REMOTE INPUT VALID IN1 IN2 IN3 IN4 IN5 IN6\r\n");
    BACKDOOR_MGR_Printf("------------------------------------------\r\n");
    BACKDOOR_MGR_Printf("                    %c   %c   %c   %c   %c   %c\r\n",
        (valid_bmp&BIT_0)?'Y':'N', (valid_bmp&BIT_1)?'Y':'N',
        (valid_bmp&BIT_2)?'Y':'N', (valid_bmp&BIT_3)?'Y':'N',
        (valid_bmp&BIT_4)?'Y':'N', (valid_bmp&BIT_5)?'Y':'N');
    BACKDOOR_MGR_Printf("\r\n");

    /* Dump T0/T4 OPERATING status
     */
    reg=IDT82V3399_REG_OPERATING_STS;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("T0 operation mode: ");
        switch (data & 0x7)
        {
            case 0:
            case 3:
                BACKDOOR_MGR_Printf("Reserved\r\n");
                break;
            case 1:
                BACKDOOR_MGR_Printf("Free-Run\r\n");
                break;
            case 2:
                BACKDOOR_MGR_Printf("Holdover\r\n");
                break;
            case 4:
                BACKDOOR_MGR_Printf("Locked\r\n");
                break;
            case 5:
                BACKDOOR_MGR_Printf("Pre-Locked2\r\n");
                break;
            case 6:
                BACKDOOR_MGR_Printf("Pre-Locked\r\n");
                break;
            case 7:
                BACKDOOR_MGR_Printf("Lost-Phase\r\n");
                break;
        }

        BACKDOOR_MGR_Printf("T0DPLL lock status: %s\r\n", (data & BIT_3)?"Locked":"Unlocked");
        BACKDOOR_MGR_Printf("T0DPLL soft alarm status: %c\r\n", (data & BIT_5)?'Y':'N');

        BACKDOOR_MGR_Printf("T4DPLL lock status: %s\r\n", (data & BIT_6)?"Locked":"Unlocked");
        BACKDOOR_MGR_Printf("T4DPLL soft alarm status: %c\r\n", (data & BIT_4)?'Y':'N');
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* Dump T0 Revertive Switching status
     */
    reg=IDT82V3399_REG_T0_INPUT_MODE_CNFG;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("T0 Revertive Switching Status: %s\r\n", (data & BIT_0)?"Enabled":"Disabled");
    }
    
    /* Dump T0/T4 OPERATING configuration
     */
    reg=IDT82V3399_REG_T0_OPERATING_MODE_CNFG;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("T0 OPERATING configuration: %s\r\n", opmode_cfg_str[data&0x7]);
        BACKDOOR_MGR_Printf("T0 DPLL Write DCO mode: %s\r\n", (data & BIT_3)?"Enabled":"Disabled");
    }
    if(data & BIT_3)
    {
        UI8_T reg_vals[3];
        float ppm;

        /* Dump IDT82V3399_REG_HOLDOVER_FREQ_CNFG_1-3 if T0 DCO mode is enabled
         */
        /* set T4_T0_REG_SEL_CNFG
         * reg 0x7
         * [bit 4] T4_T0_SEL 0b0 -> T0 path
         */
        reg = IDT82V3399_REG_T4_T0_REG_SEL_CNFG;
        rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
            SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
            reg, FALSE, 0, BIT_4);
        if(rc==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Update SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
        }
        else
        {

            /* set READ_AVG[bit 4] of T0_HOLDOVER_MODE_CNFG as 0
             */
            reg = IDT82V3399_REG_T0_HOLDOVER_MODE_CNFG;
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                reg, FALSE, 0x0, BIT_4);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
            }
            else
            {
                reg = IDT82V3399_REG_HOLDOVER_FREQ_CNFG_1;
                rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                    SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                    reg, FALSE, 3, (UI8_T*)(&reg_vals));
                if(rc==FALSE)
                {
                    BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
                }
                else
                {
                    SYNCEDRV_IDT82V3399_UTIL_ConvertRegValsToPPM(reg_vals, &ppm);
                    BACKDOOR_MGR_Printf("T0 Holdover Freq Regs = {0x%02X,0x%02X,0x%02X}\r\n",
                        reg_vals[0], reg_vals[1], reg_vals[2]);
                    BACKDOOR_MGR_Printf("T0 Holdover Freq ppm = %f\r\n", ppm);
                }
            }
        }
    }

    reg=IDT82V3399_REG_T4_OPERATING_MODE_CNFG;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("T4 OPERATING configuration: %s\r\n", opmode_cfg_str[data&0x7]);
        BACKDOOR_MGR_Printf("T4 DPLL Write DCO mode: %s\r\n", (data & BIT_3)?"Enabled":"Disabled");
    }
    if(data & BIT_3)
    {
        UI8_T reg_vals[3];
        float ppm;

        /* Dump IDT82V3399_REG_HOLDOVER_FREQ_CNFG_1-3 if T4 DCO mode is enabled
         */
        /* set T4_T0_REG_SEL_CNFG
         * reg 0x7
         * [bit 4] T4_T0_SEL 0b1 -> T4 path
         */
        reg = IDT82V3399_REG_T4_T0_REG_SEL_CNFG;
        rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
            SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
            reg, FALSE, BIT_4, BIT_4);
        if(rc==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Update SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
        }
        else
        {

            /* set READ_AVG[bit 4] of T0_HOLDOVER_MODE_CNFG as 0
             */
            reg = IDT82V3399_REG_T0_HOLDOVER_MODE_CNFG;
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                reg, FALSE, 0x0, BIT_4);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
            }
            else
            {
                reg = IDT82V3399_REG_HOLDOVER_FREQ_CNFG_1;
                rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                    SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                    reg, FALSE, 3, (UI8_T*)&reg_vals);
                if(rc==FALSE)
                {
                    BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
                }
                else
                {
                    SYNCEDRV_IDT82V3399_UTIL_ConvertRegValsToPPM(reg_vals, &ppm);
                    BACKDOOR_MGR_Printf("T4 Holdover Freq Regs = {0x%02X,0x%02X,0x%02X}\r\n",
                        reg_vals[0], reg_vals[1], reg_vals[2]);
                    BACKDOOR_MGR_Printf("T4 Holdover Freq ppm = %f\r\n", ppm);
                }
            }
        }
    }

    /* Dump currently selected INPUT and the first/second/third highest priority
     * validated INPUT
     */
    reg=IDT82V3399_REG_PRIORITY_TABLE1_STS;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("T0 currently selected INPUT: ");
        if(SYNCEDRV_IDT82V3399_UTIL_ConvertRegValToPhysicalRefId(data&0xF, &physical_ref_id)==TRUE)
        {
            BACKDOOR_MGR_Printf("IN%hu\r\n", physical_ref_id+1);
        }
        else
        {
            if((data&0xF)==0)
                BACKDOOR_MGR_Printf("No valid input\r\n");
            else
                BACKDOOR_MGR_Printf("Unknown(value=%hu)\r\n", data&0xF);
        }

        BACKDOOR_MGR_Printf("T0 first priority validated INPUT: ");
        if(SYNCEDRV_IDT82V3399_UTIL_ConvertRegValToPhysicalRefId((data&0xF0)>>4, &physical_ref_id)==TRUE)
        {
            BACKDOOR_MGR_Printf("IN%hu\r\n", physical_ref_id+1);
        }
        else
        {
            if((data&0xF)==0)
                BACKDOOR_MGR_Printf("No valid input\r\n");
            else
                BACKDOOR_MGR_Printf("Unknown(value=%hu)\r\n", (data&0xF0)>>4);
        }

    }
    reg=IDT82V3399_REG_PRIORITY_TABLE2_STS;
    rc=I2CDRV_TwsiDataReadWithBusIdx(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        reg, FALSE, 1, &data);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Read SyncE register %hu error\r\n", __FUNCTION__, __LINE__, reg);
    }
    else
    {
        BACKDOOR_MGR_Printf("T0 second priority validated INPUT: ");
        if(SYNCEDRV_IDT82V3399_UTIL_ConvertRegValToPhysicalRefId(data&0xF, &physical_ref_id)==TRUE)
        {
            BACKDOOR_MGR_Printf("IN%hu\r\n", physical_ref_id+1);
        }
        else
        {
            if((data&0xF)==0)
                BACKDOOR_MGR_Printf("No valid input\r\n");
            else
                BACKDOOR_MGR_Printf("Unknown(value=%hu)\r\n", data&0xF);
        }

        BACKDOOR_MGR_Printf("T0 third priority validated INPUT: ");
        if(SYNCEDRV_IDT82V3399_UTIL_ConvertRegValToPhysicalRefId((data&0xF0)>>4, &physical_ref_id)==TRUE)
        {
            BACKDOOR_MGR_Printf("IN%hu\r\n", physical_ref_id+1);
        }
        else
        {
            if((data&0xF)==0)
                BACKDOOR_MGR_Printf("No valid input\r\n");
            else
                BACKDOOR_MGR_Printf("Unknown(value=%hu)\r\n", (data&0xF0)>>4);
        }

    }



#ifdef SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX
    if(I2CDRV_UnLockMux(SYS_HWCFG_SYNCE_CHIP_I2C_MUX_INDEX)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to unlock lock i2c mux\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#endif

}

static BOOL_T SYNCEDRV_SetChipMode_IDT82V3399(SYNCEDRV_CHIP_MODE_TYPE_T mode)
{
    BOOL_T rc;

    /* ASIC dependent operations of setting mode which is
     * common for all projects should be done within this function
     */
    switch (mode)
    {
        case SYNCEDRV_CHIP_MODE_TYPE_NORMAL:
            /* set manual freerun mode to chip to use local clock as clock source
             * reg 0x53
             * [bit 3] T0 DPLL Write DCO control 0b0 -> Normal mode
             * [bit 2:0] T0_OPERATING_MODE 0b001 -> Forced - Free-Run
             */
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_T0_OPERATING_MODE_CNFG, FALSE, 0x01, 0x0F);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                return FALSE;
            }

            /* set T0 DPLL Holdover Mode Configuration register
             * reg 0x5C
             * [bit 7]   MAN_HOLDOVER 0b0 -> Frequency Offset Acquiring Method = Non-Manual
             */
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_T0_HOLDOVER_MODE_CNFG, FALSE, 0x00, 0x80);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                return FALSE;
            }
            break;
        case SYNCEDRV_CHIP_MODE_TYPE_DCO:
            /* set forced holdover mode to chip
             * reg 0x53
             * [bit 3] T0 DPLL Write DCO control 0b1 -> Write DCO Mode
             * [bit 2:0] T0_OPERATING_MODE 0b010 -> Forced - Holdover
             */
            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_T0_OPERATING_MODE_CNFG, FALSE, 0x0A, 0x0F);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                return FALSE;
            }
            /* set T0 DPLL Holdover Mode Configuration register
             * reg 0x5C
             * [bit 7]   MAN_HOLDOVER 0b1 -> Frequency Offset Acquiring Method = Manual
             */

            rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
                SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
                IDT82V3399_REG_T0_HOLDOVER_MODE_CNFG, FALSE, 0x80, 0x80);
            if(rc==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
                return FALSE;
            }
            break;
        default:
            BACKDOOR_MGR_Printf("%s(%d) Invalid mode(%d)\r\n", __FUNCTION__,
                __LINE__, (int)mode);
            return FALSE;
            break;
    }

    /* Call SYNCEDRV_SetupForChipMode() to setup board dependent operations
     */
    return SYNCEDRV_SetupForChipMode(mode);
}

static BOOL_T SYNCEDRV_SetOutputFreqOffsetInPPM_IDT82V3399(float ppm)
{
    UI8_T reg_vals[3];
    BOOL_T rc;

    /* set T4_T0_REG_SEL_CNFG
     * reg 0x7
     * [bit 4] T4_T0_SEL 0b0 -> T0 path
     */
    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_T4_T0_REG_SEL_CNFG, FALSE, 0x00, 0x10);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* convert ppm to register value
     */
    if (SYNCEDRV_IDT82V3399_UTIL_ConvertPPMToRegVal(ppm, reg_vals)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid ppm value(%f ppm)\r\n", __FUNCTION__, __LINE__, ppm);
        return FALSE;
    }

    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,
        "Set output frequency offset = %f ppm\r\n", ppm);
    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,
        "Reg vals = {0x%02X, 0x%02X, 0x%02X}\r\n", reg_vals[0], reg_vals[1],
        reg_vals[2]);

    /* set T4_T0_REG_SEL_CNFG
     * reg 0x5D,0x5E,0x5F
     * The HOLDOVER_FREQ[23:0] bits represent a 2¡¦s complement signed integer.
     * For the T0 or T4 DCO Control Mode, the value written to these bits
     * multiplied by 0.000011 is the frequency offset set manually.
     */
    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_HOLDOVER_FREQ_CNFG_1, FALSE, reg_vals[0], 0xFF);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_HOLDOVER_FREQ_CNFG_2, FALSE, reg_vals[1], 0xFF);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    rc=WRITE_I2C_REG_WITH_MASK(SYS_HWCFG_SYNCE_CHIP_I2C_BUS_INDEX,
        SYS_HWCFG_SYNCE_CHIP_I2C_ADDRESS, I2C_7BIT_ACCESS_MODE, TRUE,
        IDT82V3399_REG_HOLDOVER_FREQ_CNFG_3, FALSE, reg_vals[2], 0xFF);
    if(rc==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Update I2C register error\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return TRUE;
}

#endif /* #if (SYS_HWCFG_SYNCE_CHIP==SYS_HWCFG_SYNCE_CHIP_ZL30132) */
/* chip dependent functions STOP  */

/* project dependent functions START */
#if defined(ECS4810_12MV2)

#define NUM_OF_PHY_SUPPORT_SYNCE 3

/* three PHYs that support SyncE on this projects
 * Each PHY will have RCLK1 output to SyncE ASIC
 * RCLK2 of each PHY are connected to a multiplexer
 * Thus, the output clock source settings must meet all of the condition shown
 * below.
 * 1. No more than 2 output clock source on ports of each PHY.
 * 2. Only ONE PHY are allowed to set 2 output clock source.
 * 3. The maximum number of output clock source is 4.
 */
static BOOL_T __SYNCEDRV_ValidateOutputClockSrcSetting(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST])
{
    UI32_T port, clock_src_num;
    UI32_T phy_idx; /* PHY index of a port, start from 1 */
    UI16_T clk_src_count[NUM_OF_PHY_SUPPORT_SYNCE];
    UI16_T required_rclk2_count=0;

    /* count the number of clock source on each PHY
     */
    memset(clk_src_count, 0, sizeof(clk_src_count));
    clock_src_num=0;
    L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(pbmp, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT, port)
    {
        clock_src_num++;
        phy_idx = (port + 3)/4;
        if(phy_idx>NUM_OF_PHY_SUPPORT_SYNCE)
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Invalid port id in clock_src_lst. (port=%lu)\r\n", port);
            return FALSE;
        }
        clk_src_count[phy_idx-1]++;
    }

    /* check total number of output clock source
     */
    if(clock_src_num>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_IMPORTANT_MSG,"clock_src_num is larger than %d. clock_src_num=%lu\r\n",
            SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC, clock_src_num);
        return FALSE;
    }

    for(phy_idx=0; phy_idx<NUM_OF_PHY_SUPPORT_SYNCE; phy_idx++)
    {
        if(clk_src_count[phy_idx]>2)
        {
            /* each PHY can output two recovered clock at most
             */
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_IMPORTANT_MSG,"Invalid output clock source settings: PHY idx %lu has more than two recover clock output\r\n",
            phy_idx);
            return FALSE;
        }

        if(clk_src_count[phy_idx]>1)
        {
            /* evaluate the required rclk2 according to the clock source count in
             * each PHY
             */
            required_rclk2_count++;
        }
    }

    /* rclk2 can only be used by one of the PHY
     * if required_rclk2_count is larger than 1, the setting is invalid
     */
    if(required_rclk2_count>1)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_IMPORTANT_MSG,"More than 2 PHYs using rclk2 as output clock source.\r\n");
        return FALSE;
    }

    return TRUE;
}

static BOOL_T __SYNCEDRV_SetPHYSyncEPortInClockSource(UI32_T port, SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_T clock_source)
{
    UI32_T module_id, device_id, phy_port, my_unit_id=1;
    DEV_SWDRV_PHY_Reg_Operation_T phy_reg_ops[] =
        {
            {0x16, 0x02, 0x00FF}, /* switch to page 2 */
            {0x10, 0x00, 0x0000}, /* reg_val and reg_val_mask are set on demand */
            {0x16, 0x00, 0x00FF}, /* switch to page 0 */
            {0x00, 0x8000, 0x8000}, /* issue sw phy reset to let the setting of reference clock source selection take effect, this operation also work on fiber port */
        };
    UI16_T data, field_mask;

    if(STKTPLG_BOARD_IsSyncEPort(port)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Port %lu does not support syncE\r\n", port);
        return FALSE;
    }

    STKTPLG_OM_GetMyUnitID(&my_unit_id);
    if(FALSE==DEV_SWDRV_PMGR_Logical2PhyDevicePortID(my_unit_id, port, &module_id, &device_id, &phy_port))
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to convert user port %lu to phy port\r\n", port);
        return FALSE;
    }

    /* per discussion with hardware engineer Mandheling Chen
     * OUT2 to OUT4 which are connected to PHY1 to PHY3 do not have a jitter attenuator
     * and might get worse performance. OUT 1 have a jitter attenuator which will
     * get better performacne, so always use OUT1(i.e. connected to XTAL_IN of PHY 1/2/3)
     * as reference clock source.
     */
    #if 1
    /* Page 2, Register 0x10
     * Bit 7 Copper Reference CLK Source Select: 0 = Use XTAL_IN as 25MHz source
     * Bit 6 Fiber Reference CLK Source Select: 0 = Use XTAL_IN as 25MHz source
     */
    field_mask = (BIT_7 | BIT_6);
    data = 0x00;
    #else
    switch(clock_source)
    {
        case SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_LOCAL:
            /* Page 2, Register 0x10
             * Bit 7 Copper Reference CLK Source Select: 0 = Use XTAL_IN as 25MHz source
             * Bit 6 Fiber Reference CLK Source Select: 0 = Use XTAL_IN as 25MHz source
             */
            field_mask = (BIT_7 | BIT_6);
            data = 0x00;
            break;
        case SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_EXTERNAL:
            /* Page 2, Register 0x10
             * Bit 7 Copper Reference CLK Source Select: 1 = Use SCLK as 25MHz source
             * Bit 6 Fiber Reference CLK Source Select: 1 = Use SCLK as 25MHz source
             */
            field_mask = (BIT_7 | BIT_6);
            data = 0xC0;
            break;
        default:
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Invalid clock_source(%lu), port=%lu\r\n", (UI32_T)clock_source, port);
            return FALSE;
    }
    #endif

    phy_reg_ops[1].reg_val=data;
    phy_reg_ops[1].reg_val_mask = field_mask;
    return DEV_SWDRV_PMGR_DoPHYRegsTransactions(my_unit_id, port, phy_reg_ops, sizeof(phy_reg_ops)/sizeof(phy_reg_ops[0]));
}

/* logical ref_id     connected PHY RCLK pin
 * -----------------------------------------
 *      0             PHY1 RCLK1
 *      1             PHY2 RCLK1
 *      2             PHY3 RCLK1
 *      3             PHY1/PHY2/PHY3 RCLK2 (selected through MPP18 and MPP19)
 */
static BOOL_T __SYNCEDRV_SetClockSrcByRefIdxOnASIC(UI32_T port, BOOL_T is_set, UI32_T ref_id)
{
    UI32_T module_id, device_id, phy_port, phy_addr, my_unit_id=1;
    UI16_T regVal, data, field_mask;
    UI16_T phy_idx; /* PHY index of the given port, start from 1 */
    UI16_T phy_rclk_idx; /* RCLK index of the PHY, start from 1 */
    UI8_T  data8;
    DEV_SWDRV_PHY_Reg_Operation_T phy_reg_ops[] =
        {
            {0x16, 0x02, 0x00FF}, /* switch to page 2 */
            {0x10, 0x00, 0x0000}, /* reg_val and reg_val_mask are set on demand */
            {0x16, 0x00, 0x00FF}, /* switch to page 0 */
        };

    if (ref_id>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Invalid ref_id(%d) port=%d, is_set=%d\r\n", (int)ref_id, (int)port, (int)is_set);
        return FALSE;
    }

    if (STKTPLG_BOARD_IsSyncEPort(port)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Port %d does not support synce ref_id=%d, is_set=%d\r\n"
            , (int)port, (int)ref_id, (int)is_set);
        return FALSE;
    }

    /* evaluate the PHY index of the given port
     */
    phy_idx = (port+3)/4;

    STKTPLG_OM_GetMyUnitID(&my_unit_id);
    if (FALSE==DEV_SWDRV_PMGR_Logical2PhyDevicePortID(my_unit_id, port, &module_id, &device_id, &phy_port))
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to convert user port %lu to phy port\r\n", __FUNCTION__, __LINE__, port);
        return FALSE;
    }

    if (FALSE==DEV_SWDRV_PMGR_Logical2PhyDeviceID(my_unit_id, port, &module_id, &device_id, &phy_addr))
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to convert user port %lu to phy addr\r\n", __FUNCTION__, __LINE__, port);
        return FALSE;
    }

    if (TRUE==DEV_SWDRV_PMGR_GetMIIPhyRegByPage(device_id, phy_addr, phy_port, 0x10, 0x16, 0x02, 0xFF, &regVal))
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Original PHY Reg 0x10 Page 2 value=0x%04x\r\n", regVal);
    else
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to read RCLK status on port %lu\r\n", port);

    if (ref_id<=2)
        phy_rclk_idx = 1;

    else
        phy_rclk_idx = 2;

    if(is_set==TRUE)
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Output recover clock of port %lu to RCLK%hu\r\n", port, phy_rclk_idx);
    else
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Remove recover clock of port %lu on RCLK%hu\r\n", port, phy_rclk_idx);

    /* Page 2, Register 0x10
     * Bit12 RCLK Frequency Sel: 0 = 25 MHz
     * Bit11 RCLK Link Down Disable: 1 = RCLK low during link down and 10BASE-T.
     * Bit 9 RCLK2 Select: 1 = Output recovered clock on RCLK2
     * Bit 8 RCLK1 Select: 1 = Output recovered clock on RCLK1
     */
    data = 0x0800;
    if (phy_rclk_idx == 1)
    {
        field_mask = 0x1900;
        if(is_set==TRUE)
            data |= 0x0100;
        else
            data &= ~(0x0100);
    }
    else
    {
        field_mask = 0x1A00;
        if(is_set==TRUE)
            data |= 0x0200;
        else
            data &= ~(0x0200);
    }

    phy_reg_ops[1].reg_val=data;
    phy_reg_ops[1].reg_val_mask = field_mask;

    if (FALSE == DEV_SWDRV_PMGR_DoPHYRegsTransactions(my_unit_id, port, phy_reg_ops, sizeof(phy_reg_ops)/sizeof(phy_reg_ops[0])))
    {
        BACKDOOR_MGR_Printf("%s(%d)Do PHY reg transaction failed on port %lu\r\n", __FUNCTION__, __LINE__, port);
        return FALSE;
    }

    if (TRUE==DEV_SWDRV_PMGR_GetMIIPhyRegByPage(device_id, phy_addr, phy_port, 0x10, 0x16, 0x02, 0xFF, &regVal))
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"New PHY Reg 0x10 Page 2 value=0x%04x\r\n", regVal);
    else
        BACKDOOR_MGR_Printf("%s(%d)Failed to read RCLK status on port %lu\r\n", __FUNCTION__, __LINE__, port);

    if(phy_rclk_idx == 2)
    {
        #define IN6_CLOCK_SEL_GPIO_REG_ADDR      (SYS_HWCFG_GPIO_OUT+2)
        #define IN6_CLOCK_SEL_GPIO_REG_MASK      0x03
        #define IN6_CLOCK_SEL_GPIO_REG_BIT_SHIFT 2

        GPIO_ENTER_CRITICAL_SECTION();
        /* set GPIO to select RCLK2 of the given PHY
         */
        if(FALSE==PHYSICAL_ADDR_ACCESS_Read(IN6_CLOCK_SEL_GPIO_REG_ADDR, 1, 1, &data8))
        {
            BACKDOOR_MGR_Printf("%s(%d)Failed to read GPIO addr 0x%08lX\r\n",
                __FUNCTION__, __LINE__, IN6_CLOCK_SEL_GPIO_REG_ADDR);
            GPIO_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }

        data8 &= ~(IN6_CLOCK_SEL_GPIO_REG_MASK<<IN6_CLOCK_SEL_GPIO_REG_BIT_SHIFT);
        switch(phy_idx)
        {
            case 1:
                data8 |= 0x1 << IN6_CLOCK_SEL_GPIO_REG_BIT_SHIFT;
                break;
            case 2:
                data8 |= 0x2 << IN6_CLOCK_SEL_GPIO_REG_BIT_SHIFT;
                break;
            case 3:
                data8 |= 0x3 << IN6_CLOCK_SEL_GPIO_REG_BIT_SHIFT;
                break;
            default:
                BACKDOOR_MGR_Printf("%s(%d)Invalid phy_idx(%hu)\r\n",
                    __FUNCTION__, __LINE__, phy_idx);
                GPIO_LEAVE_CRITICAL_SECTION();
                return FALSE;
        }
        if (FALSE == PHYSICAL_ADDR_ACCESS_Write(IN6_CLOCK_SEL_GPIO_REG_ADDR, 1, 1, &data8))
        {
            BACKDOOR_MGR_Printf("%s(%d): Failed to write GPIO addr 0x%08lX\r\n",
                __FUNCTION__, __LINE__, IN6_CLOCK_SEL_GPIO_REG_ADDR);
        }

        GPIO_LEAVE_CRITICAL_SECTION();
    }

    return TRUE;
}

/* On ECS4810_12MV2, the reference input to SyncE ASIC (IDT82V3399) is described below:
 * ----------------------------------------------------------------
 * Pin Name  Logical Ref Idx   Physical Ref Idx  Connected PHY pin
 *           (start from 0)    (start from 0)
 * ----------------------------------------------------------------
 * IN3       0                 2                 PHY1 RCLK1
 * IN4       1                 3                 PHY2 RCLK1
 * IN5       2                 4                 PHY3 RCLK1
 * IN6       3                 5                 PHY1/PHY2/PHY3 RCLK2 (selected through MPP18 and MPP19)
 */
static BOOL_T __SYNCEDRV_BindClockSrcPortToRefIdx(UI32_T port, UI32_T *ref_idx_p)
{
    UI32_T binded_port;
    UI16_T phy_idx; /* PHY index of the given port, start from 1 */
    UI16_T dedicated_logical_ref_idx;
    const UI16_T muxed_logical_ref_idx = 3;


    /* evaluate the PHY index of the given port
     */
    phy_idx = (port+3)/4;
    dedicated_logical_ref_idx = phy_idx-1;

    SYNCEDRV_OM_GetRefIdxToClockSrcPortBinding((UI32_T)dedicated_logical_ref_idx, &binded_port);

    /* try to bind on dedicated ref idx first
     */
    if(binded_port==0)
    {
        *ref_idx_p = dedicated_logical_ref_idx;
        return TRUE;
    }

    /* dedicated ref idx is occupied
     * try muxed ref idx
     */
    SYNCEDRV_OM_GetRefIdxToClockSrcPortBinding((UI32_T)muxed_logical_ref_idx, &binded_port);
    if(binded_port==0)
    {
        *ref_idx_p = muxed_logical_ref_idx;
        return TRUE;
    }

    /* all usable ref idxs are occupied
     */
    return FALSE;
}

static BOOL_T __SYNCEDRV_SetupForChipMode(SYNCEDRV_CHIP_MODE_TYPE_T mode)
{
    /* do nothing on this project
     */
    return TRUE;
}
#elif defined(ASF4512MP)
/* Only the third PHY(88E1340S) supports synce, the user port belongs to this
 * PHY are 9 to 12.
 * Only two ports can be set as clock source at most.
 */
static BOOL_T __SYNCEDRV_ValidateOutputClockSrcSetting(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST])
{
    UI8_T port;
    UI8_T clock_src_count=0;

    /* if any bit of port 1 to port 8 is set, the setting is invalid
     */
    if(pbmp[0])
        return FALSE;

    L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(pbmp, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT, port)
    {
        clock_src_count++;
    }

    if(clock_src_count>2)
        return FALSE;

    return TRUE;
}

static BOOL_T __SYNCEDRV_SetPHYSyncEPortInClockSource(UI32_T port, SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_T clock_source)
{
    UI32_T module_id, device_id, phy_port, my_unit_id=1;
    DEV_SWDRV_PHY_Reg_Operation_T phy_reg_ops[] =
        {
            {0x16, 0x02, 0x00FF}, /* switch to page 2 */
            {0x10, 0x00, 0x0000}, /* reg_val and reg_val_mask are set on demand */
            {0x16, 0x00, 0x00FF}, /* switch to page 0 */
            {0x00, 0x8000, 0x8000}, /* issue sw phy reset to let the setting of reference clock source selection take effect, this operation also work on fiber port */
        };
    UI16_T data, field_mask;

    if(STKTPLG_BOARD_IsSyncEPort(port)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Port %lu does not support syncE\r\n", port);
        return FALSE;
    }

    STKTPLG_OM_GetMyUnitID(&my_unit_id);
    if(FALSE==DEV_SWDRV_PMGR_Logical2PhyDevicePortID(my_unit_id, port, &module_id, &device_id, &phy_port))
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to convert user port %lu to phy port\r\n", port);
        return FALSE;
    }

    switch(clock_source)
    {
        case SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_LOCAL:
            /* Page 2, Register 0x10
             * Bit 7 Copper Reference CLK Source Select: 0 = Use XTAL_IN as 25MHz source
             * Bit 6 Fiber Reference CLK Source Select: 0 = Use XTAL_IN as 25MHz source
             */
            field_mask = (BIT_7 | BIT_6);
            data = 0x00;
            break;
        case SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_EXTERNAL:
            /* Page 2, Register 0x10
             * Bit 7 Copper Reference CLK Source Select: 1 = Use SCLK as 25MHz source
             * Bit 6 Fiber Reference CLK Source Select: 1 = Use SCLK as 25MHz source
             */
            field_mask = (BIT_7 | BIT_6);
            data = 0xC0;
            break;
        default:
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Invalid clock_source(%lu), port=%lu\r\n", (UI32_T)clock_source, port);
            return FALSE;
    }

    phy_reg_ops[1].reg_val=data;
    phy_reg_ops[1].reg_val_mask = field_mask;
    return DEV_SWDRV_PMGR_DoPHYRegsTransactions(my_unit_id, port, phy_reg_ops, sizeof(phy_reg_ops)/sizeof(phy_reg_ops[0]));
}

/* logical ref_id     connected PHY RCLK pin
 * -----------------------------------------
 *      0             PHY1 RCLK1
 *      1             PHY2 RCLK1
 */
static BOOL_T __SYNCEDRV_SetClockSrcByRefIdxOnASIC(UI32_T port, BOOL_T is_set, UI32_T ref_id)
{
    UI32_T device_id, module_id, phy_port, phy_addr, my_unit_id=1;
    UI16_T regVal, data, field_mask;
    UI16_T phy_rclk_idx; /* RCLK index of the PHY, start from 1 */

    DEV_SWDRV_PHY_Reg_Operation_T phy_reg_ops[] =
        {
            {0x16, 0x02, 0x00FF}, /* switch to page 2 */
            {0x10, 0x00, 0x0000}, /* reg_val and reg_val_mask are set on demand */
            {0x16, 0x00, 0x00FF}, /* switch to page 0 */
        };

    if (ref_id>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Invalid ref_id(%d) port=%d, is_set=%d\r\n", (int)ref_id, (int)port, (int)is_set);
        return FALSE;
    }

    if (STKTPLG_BOARD_IsSyncEPort(port)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Port %d does not support synce ref_id=%d, is_set=%d\r\n"
            , (int)port, (int)ref_id, (int)is_set);
        return FALSE;
    }

    STKTPLG_OM_GetMyUnitID(&my_unit_id);
    if (FALSE==DEV_SWDRV_PMGR_Logical2PhyDevicePortID(my_unit_id, port, &module_id, &device_id, &phy_port))
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to convert user port %lu to phy port\r\n", __FUNCTION__, __LINE__, port);
        return FALSE;
    }

    if (FALSE==DEV_SWDRV_PMGR_Logical2PhyDeviceID(my_unit_id, port, &module_id, &device_id, &phy_addr))
    {
        BACKDOOR_MGR_Printf("%s(%d)Failed to convert user port %lu to phy addr\r\n", __FUNCTION__, __LINE__, port);
        return FALSE;
    }

    if (TRUE==DEV_SWDRV_PMGR_GetMIIPhyRegByPage(device_id, phy_addr, phy_port, 0x10, 0x16, 0x02, 0xFF, &regVal))
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Original PHY Reg 0x10 Page 2 value=0x%04x\r\n", regVal);
    else
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to read RCLK status on port %lu\r\n", port);

    if (ref_id==0)
        phy_rclk_idx = 1;
    else
        phy_rclk_idx = 2;

    if(is_set==TRUE)
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Output recover clock of port %lu to RCLK%hu\r\n", port, phy_rclk_idx);
    else
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Remove recover clock of port %lu on RCLK%hu\r\n", port, phy_rclk_idx);

    /* Page 2, Register 0x10
     * Bit12 RCLK Frequency Sel: 0 = 25 MHz
     * Bit11 RCLK Link Down Disable: 1 = RCLK low during link down and 10BASE-T.
     * Bit 9 RCLK2 Select: 1 = Output recovered clock on RCLK2
     * Bit 8 RCLK1 Select: 1 = Output recovered clock on RCLK1
     */
    data = 0x0800;
    if (phy_rclk_idx == 1)
    {
        field_mask = 0x1900;
        if(is_set==TRUE)
            data |= 0x0100;
        else
            data &= ~(0x0100);
    }
    else
    {
        field_mask = 0x1A00;
        if(is_set==TRUE)
            data |= 0x0200;
        else
            data &= ~(0x0200);
    }

    phy_reg_ops[1].reg_val=data;
    phy_reg_ops[1].reg_val_mask = field_mask;

    if (FALSE == DEV_SWDRV_PMGR_DoPHYRegsTransactions(my_unit_id, port, phy_reg_ops, sizeof(phy_reg_ops)/sizeof(phy_reg_ops[0])))
    {
        BACKDOOR_MGR_Printf("%s(%d)Do PHY reg transaction failed on port %lu\r\n", __FUNCTION__, __LINE__, port);
        return FALSE;
    }

    if (TRUE==DEV_SWDRV_PMGR_GetMIIPhyRegByPage(device_id, phy_addr, phy_port, 0x10, 0x16, 0x02, 0xFF, &regVal))
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"New PHY Reg 0x10 Page 2 value=0x%04x\r\n", regVal);
    else
        BACKDOOR_MGR_Printf("%s(%d)Failed to read RCLK status on port %lu\r\n", __FUNCTION__, __LINE__, port);

    return TRUE;
}

static BOOL_T __SYNCEDRV_BindClockSrcPortToRefIdx(UI32_T port, UI32_T *ref_idx_p)
{
    UI32_T binded_port;
    UI8_T  logical_ref_idx;

    for(logical_ref_idx=0; logical_ref_idx<SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC; logical_ref_idx++)
    {
        SYNCEDRV_OM_GetRefIdxToClockSrcPortBinding((UI32_T)logical_ref_idx, &binded_port);
        if(binded_port==0)
        {
            *ref_idx_p=logical_ref_idx;
            return TRUE;
        }
    }

    /* all usable ref idxs are occupied
     */
    return FALSE;
}

static BOOL_T __SYNCEDRV_SetupForChipMode(SYNCEDRV_CHIP_MODE_TYPE_T mode)
{
    /* do nothing on this project
     */
    return TRUE;
}
#elif defined(ECS4910_28F)
static BOOL_T __SYNCEDRV_ValidateOutputClockSrcSetting(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST])
{
    UI8_T port, clock_src_count=0;

    L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(pbmp, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT, port)
    {
        if(port<24)
        {
            /* only port 25-28 supports synce now
             */
            return FALSE;
        }
        clock_src_count++;
    }

    if(clock_src_count>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC)
        return FALSE;

    return TRUE;
}
static BOOL_T __SYNCEDRV_SetPHYSyncEPortInClockSource(UI32_T port, SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_T clock_source)
{
    UI32_T module_id, device_id, phy_id, my_unit_id=1;
    UI16_T data, field_mask, regVal;

    if(STKTPLG_BOARD_IsSyncEPort(port)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Port %lu does not support syncE\r\n", port);
        return FALSE;
    }

    STKTPLG_OM_GetMyUnitID(&my_unit_id);
    if(FALSE==DEV_SWDRV_PMGR_Logical2PhyDeviceID(my_unit_id, port, &module_id, &device_id, &phy_id))
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to convert user port %lu to phy port\r\n", port);
        return FALSE;
    }

    switch(clock_source)
    {
        case SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_LOCAL:
            /* Reference Document: 8725-DS-03-R.pdf
             * Excerpted from Table 186(PMD Miscellaneous Control Register) on Page 76.
             * Device Address=0x1 MII Address=0xCA0A
             *-----------------------------------------------------------------------------------
             * Bit(s) Name          R/W Initial Value  Description
             *-----------------------------------------------------------------------------------
             * 7      P_IN_MXSEL    R/W 1              Reference clock input selection control.
             * 6      X_IN_MXSEL    R/W 0              Reference clock input selection control.
             * 5      CLUPLL_EN     R/W 0              Reference clock input selection control.
             * 4      XCLKMODE_OVRD R/W 0              Reference clock input selection control.
             */
            field_mask = (BIT_7 | BIT_6 | BIT_5 | BIT_4);
            data = BIT_7;
            break;
        case SYNCEDRV_TYPE_PHY_PORT_CLOCK_SOURCE_EXTERNAL:
            /* Reference Document: 8725-DS-03-R.pdf
             * Excerpted from Table 2(Clock domain Summary Table) on Page 12
             *-----------------------------------------------------------------------------------
             * Mode         P_IN_MXSEL X_IN_MXSEL CLUPLL_EN XCLKMODE_OVRD  PMD Refclk XAUI Refclk
             *-----------------------------------------------------------------------------------
             * Asynchronous 0          1          0         1              P_EXTCLK   P_EXTCLK
             * (PREF)
             *-----------------------------------------------------------------------------------
             *
             * Excerpted from Table 186(PMD Miscellaneous Control Register) on Page 76.
             *-----------------------------------------------------------------------------------
             * Bit(s) Name          R/W Description
             *-----------------------------------------------------------------------------------
             * 7      P_IN_MXSEL    R/W Reference clock input selection control.
             * 6      X_IN_MXSEL    R/W Reference clock input selection control.
             * 5      CLUPLL_EN     R/W Reference clock input selection control.
             * 4      XCLKMODE_OVRD R/W Reference clock input selection control.
             */
            field_mask = (BIT_7 | BIT_6 | BIT_5 | BIT_4);
            data = (BIT_6 | BIT_4);
            break;
        default:
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Invalid clock_source(%lu), port=%lu\r\n", (UI32_T)clock_source, port);
            return FALSE;
    }

    if(DEV_SWDRV_PMGR_GetXMIIPhyReg(device_id, phy_id, 0x1, 0xCA0A, &regVal)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Failed to get XMII phy reg on unit %lu, port %lu, \r\n", my_unit_id, port);
        return FALSE;
    }

    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Original XMII PHY Reg dev_addr=1 reg_addr=0xCA0A value=0x%04x\r\n", regVal);

    regVal &= ~(field_mask);
    regVal |= data;

    if(DEV_SWDRV_PMGR_SetXMIIPhyReg(device_id, phy_id, 0x1, 0xCA0A, regVal)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG, "Failed to set XMII phy reg on unit %lu, port %lu, \r\n", my_unit_id, port);
        return FALSE;
    }
    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"New XMII PHY Reg dev_addr=1 reg_addr=0xCA0A value=0x%04x\r\n", regVal);

    return TRUE;
}
static BOOL_T __SYNCEDRV_SetClockSrcByRefIdxOnASIC(UI32_T port, BOOL_T is_set, UI32_T ref_id)
{
    UI32_T my_unit_id=1, module_id, device_id, phy_id, uport;
    UI16_T regVal16;
    UI8_T  regVal, data, field_mask, bit_shift;

    if (ref_id>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Invalid ref_id(%d) port=%d, is_set=%d\r\n", (int)ref_id, (int)port, (int)is_set);
        return FALSE;
    }

    if (STKTPLG_BOARD_IsSyncEPort(port)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Port %d does not support synce ref_id=%d, is_set=%d\r\n",
            (int)port, (int)ref_id, (int)is_set);
        return FALSE;
    }

    STKTPLG_OM_GetMyUnitID(&my_unit_id);
    /* enable/disable XFP RCLK on the port
     * workaround: it is found that ZL3012 can only lock input ref of port 25
     *             or port 26 when XFP RCLK are enabled on both port 25 and port 26
     *             For now, only port 25 and port 26 supports synce, so we just
     *             enable both XFP RCLK when is_set==TRUE
     */
    if(is_set==TRUE)
    {
        if(port==25 || port==26)
        {
            for(uport=25; uport<=26; uport++)
            {
                DEV_SWDRV_PMGR_Logical2PhyDeviceID(my_unit_id, uport, &module_id, &device_id, &phy_id);
                if(DEV_SWDRV_PMGR_GetXMIIPhyReg(device_id, phy_id, 0x1, 0xCA08, &regVal16)==FALSE)
                {
                    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to read PHY register on port %lu\r\n",
                        uport);
                    return FALSE;
                }
                SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG, "Original PHY REG value=0x%04X\r\n",regVal16);
                /* enable RCLK output on BCM8725
                 * Document: 8725-DS03-R.pdf
                 * P.76 Table 185: PMD Control Register (Address=0xCA08)
                 * Bit 15 : RCLK_EN  --  1 = Enable recovered clk div by 64 output.
                 */
                regVal16 |= BIT_15;

                SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG, "New PHY REG value=0x%04X\r\n",regVal16);
                if(DEV_SWDRV_PMGR_SetXMIIPhyReg(device_id, phy_id, 0x1, 0xCA08, regVal16)==FALSE)
                {
                    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to write PHY register on port %lu\r\n",
                        uport);
                    return FALSE;
                }
            }
        }
        else
        {
            DEV_SWDRV_PMGR_Logical2PhyDeviceID(my_unit_id, port, &module_id, &device_id, &phy_id);
            if(DEV_SWDRV_PMGR_GetXMIIPhyReg(device_id, phy_id, 0x1, 0xCA08, &regVal16)==FALSE)
            {
                SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to read PHY register on port %lu\r\n",
                    port);
                return FALSE;
            }
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG, "Original PHY REG value=0x%04X\r\n",regVal16);
            /* enable RCLK output on BCM8725
             * Document: 8725-DS03-R.pdf
             * P.76 Table 185: PMD Control Register (Address=0xCA08)
             * Bit 15 : RCLK_EN  --  1 = Enable recovered clk div by 64 output.
             */
            regVal16 |= BIT_15;

            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG, "New PHY REG value=0x%04X\r\n",regVal16);
            if(DEV_SWDRV_PMGR_SetXMIIPhyReg(device_id, phy_id, 0x1, 0xCA08, regVal16)==FALSE)
            {
                SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to write PHY register on port %lu\r\n",
                    port);
                return FALSE;
            }
        }
    }
    /* excerpted from ECS4660_28F Hardware Spec
     * 7.1.23 Port Recovered Clock Select (0xE010_0017, Read & Write)
     * Primary Port Clock Select definition: (To ref0 of ZL30132)
     * Secondary Port Clock Select definition: (To ref1 of ZL30132)
     *----------------------------------------------------
     * Bit(s)             Description
     *----------------------------------------------------
     * 2:0                Prim_Port_Sel[2:0] Clock Source
     * 6:4                Sec_Port_Sec[2:0] Clock Source
     *
     *---------------------------------------------------------------
     * Value(Binary)  Description
     *---------------------------------------------------------------
     *     000        Broadcom Multilayer Switch, RECOV_CLK_0 pin
     *     001        SFP/Copper PHY Interface Board, primary clock source
     *     010        XG Expansion slot, Port 1 (User port 27)
     *     011        XG Expansion slot, Port 2 (User port 28)
     *     100        Optional onboard 10G PHY, Port 1 (User port 25)
     *     101        Optional onboard 10G PHY, Port 2 (User port 26)
     *     110        Reserved
     *     111        Reserved
     */
    if(ref_id==0)
        bit_shift=0;
    else
        bit_shift=4;

    field_mask = (BIT_2 | BIT_1 | BIT_0) << bit_shift;
    if(is_set==FALSE)
    {
        /* select Reserved value to set when is_set==FALSE to
         * have invalid clock input on the spceified ref id
         */
        data = (BIT_2 | BIT_1);
    }
    else
    {
        switch (port)
        {
            case 25:
                data = (BIT_2) << bit_shift;
                break;
            case 26:
                data = (BIT_2 | BIT_0) << bit_shift;
                break;
            case 27:
                data = (BIT_1) << bit_shift;
                break;
            case 28:
                data = (BIT_1 | BIT_0) << bit_shift;
                break;
            default:
                SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to set port %d as synce clock source, ref_id=%d, is_set=%d\r\n",
                    port, ref_id, is_set);
                return FALSE;
        }
    }

    if(PHYSICAL_ADDR_ACCESS_Read(SYS_HWCFG_PORT_RECOVERED_CLK_SEL_ADDR, 1, 1, &regVal)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to read CPLD when set/unset port %d as synce clock source, ref_id=%d, is_set=%d\r\n",
            port, ref_id, is_set);
        return FALSE;
    }

    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG, "Original CPLD CLK_SEL REG value=0x%02X\r\n",regVal);
        
    regVal &= ~(field_mask);
    regVal |= data;

    if(PHYSICAL_ADDR_ACCESS_Write(SYS_HWCFG_PORT_RECOVERED_CLK_SEL_ADDR, 1, 1, &regVal)==FALSE)
    {
        SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Failed to write CPLD when set/unset port %d as synce clock source, ref_id=%d, is_set=%d\r\n",
            port, ref_id, is_set);
        return FALSE;
    }

    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG, "New CPLD CLK_SEL REG value=0x%02X\r\n",regVal);

    return TRUE;
}
static BOOL_T __SYNCEDRV_BindClockSrcPortToRefIdx(UI32_T port, UI32_T *ref_idx_p)
{
    UI32_T binded_port;
    UI8_T  logical_ref_idx;

    for(logical_ref_idx=0; logical_ref_idx<SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC; logical_ref_idx++)
    {
        SYNCEDRV_OM_GetRefIdxToClockSrcPortBinding((UI32_T)logical_ref_idx, &binded_port);
        if(binded_port==0)
        {
            *ref_idx_p=logical_ref_idx;
            return TRUE;
        }
    }

    /* all usable ref idxs are occupied
     */
    return FALSE;
}

static BOOL_T __SYNCEDRV_SetupForChipMode(SYNCEDRV_CHIP_MODE_TYPE_T mode)
{
    /* do nothing on this project
     */
    return TRUE;
}
#else
#error "No SYNCEDRV project dependent functions implemented."
#endif /* #if defined(ECS4810_12MV2) */
/* project dependent functions STOP  */

static void SYNCEDRV_DumpRefIdBindedPortInfo(void)
{
    UI32_T i, clock_src_port;

    BACKDOOR_MGR_Printf("Logical Reference Id       Binded Clock Source Port\r\n");
    BACKDOOR_MGR_Printf("---------------------------------------------------\r\n");

    for(i=0; i<SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC; i++)
    {
        SYNCEDRV_OM_GetRefIdxToClockSrcPortBinding(i, &clock_src_port);
        if(clock_src_port==0)
        {
            BACKDOOR_MGR_Printf("%9lu                          Not exist\r\n",i);
        }
        else
        {
            BACKDOOR_MGR_Printf("%9lu                            %2lu\r\n",i, clock_src_port);
        }
    }
}

static void SYNCEDRV_DumpOMClockSourcePortInfo(void)
{
    SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    UI32_T clock_src_num, i;

    SYNCEDRV_OM_GetSyncEClockSourceStatus(clock_src_lst, &clock_src_num);

    if(clock_src_num==0)
    {
        BACKDOOR_MGR_Printf("No clock source port exists.\r\n");
    }
    else
    {
        BACKDOOR_MGR_Printf("Total clock source port num = %lu\r\n", clock_src_num);
        BACKDOOR_MGR_Printf("Port      Priority    Clock Status    Active Status\r\n");
        BACKDOOR_MGR_Printf("---------------------------------------------------\r\n");
        for(i=0; i<clock_src_num; i++)
        {
            BACKDOOR_MGR_Printf("%2lu        %5lu       %4s            %3s\r\n",
                clock_src_lst[i].port,
                clock_src_lst[i].priority,
                (clock_src_lst[i].is_good_status==TRUE)?"Good":"Bad",
                (clock_src_lst[i].is_active==TRUE)?"Yes":"No");
        }
    }
}

static void SYNCEDRV_BackDoor_Menu (void)
{
    UI8_T   ch, dbg_msg_level, i;
    char    buff[40];
    BOOL_T  is_exit=FALSE, rc;

    while (is_exit==FALSE)
    {
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("==========SYNCEDRV BackDoor Menu==========\r\n");
        BACKDOOR_MGR_Printf(" 0. Exit\r\n");
        BACKDOOR_MGR_Printf(" 1. Set Debug Message Level(%hu)\r\n", SYNCEDRV_OM_GetDebugMsgLevel());
        BACKDOOR_MGR_Printf(" 2. Dump SyncE ASIC status\r\n");
        BACKDOOR_MGR_Printf(" 3. Show binded port of logical input reference id\r\n");
        BACKDOOR_MGR_Printf(" 4. Show clock source port settings in OM\r\n");
        BACKDOOR_MGR_Printf(" 5. Set Chip mode\r\n");
        BACKDOOR_MGR_Printf(" 6. Set Output clock frequency offset in ppm\r\n");
        BACKDOOR_MGR_Printf("===========================================\r\n");
        BACKDOOR_MGR_Printf("select =");

        ch = BACKDOOR_MGR_GetChar();

        BACKDOOR_MGR_Printf("%c\r\n",ch);
        switch (ch)
        {
            case '0':
                is_exit = TRUE;
                break;
            case '1':
                BACKDOOR_MGR_Printf("Debug message level(0-3)= ");
                if(BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1)==TRUE)
                {
                    dbg_msg_level = (UI8_T)(atoi(buff));
                    SYNCEDRV_OM_SetDebugMsgLevel(dbg_msg_level);
                }
                else
                {
                    BACKDOOR_MGR_Printf("BACKDOOR_MGR_RequestKeyIn error\r\n");
                }
                break;
            case '2':
                SYNCEDRV_DumpSyncEAsicInfo();
                break;
            case '3':
                SYNCEDRV_DumpRefIdBindedPortInfo();
                break;
            case '4':
                SYNCEDRV_DumpOMClockSourcePortInfo();
                break;
            case '5':
            {
                UI32_T chip_mode;
                char* chip_mode_str[SYNCEDRV_CHIP_MODE_TYPE_NUMBER] = 
                {
                    "Normal", /* SYNCEDRV_CHIP_MODE_TYPE_NORMAL */
                    "DCO", /*SYNCEDRV_CHIP_MODE_TYPE_DCO */
                };

                for (i=0; i<SYNCEDRV_CHIP_MODE_TYPE_NUMBER; i++)
                    BACKDOOR_MGR_Printf("[%d] %s\r\n", (int)i, chip_mode_str[i]);

                BACKDOOR_MGR_Printf("Chip mode= ");
                if (BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1)==TRUE)
                {
                    chip_mode = (UI32_T)(atoi(buff));
                }
                else
                {
                    BACKDOOR_MGR_Printf("\r\nBACKDOOR_MGR_RequestKeyIn error\r\n");
                    break;
                }

                if (chip_mode >= SYNCEDRV_CHIP_MODE_TYPE_NUMBER)
                {
                    BACKDOOR_MGR_Printf("\r\nInvalid chip mode(%lu)\r\n", chip_mode);
                }
                else
                {
                    rc = SYNCEDRV_SetChipMode((SYNCEDRV_CHIP_MODE_TYPE_T)chip_mode);
                    BACKDOOR_MGR_Printf("\r\nSet chip mode %s\r\n", (rc==TRUE)?"OK":"Failed");
                }
            }
                break;
            case '6':
            {
                float ppm=0;

                BACKDOOR_MGR_Printf("ppm=");
                if (BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1)==TRUE)
                {
                    ppm = (float)(atof(buff));
                }
                else
                {
                    BACKDOOR_MGR_Printf("\r\nBACKDOOR_MGR_RequestKeyIn error\r\n");
                    break;
                }

                rc = SYNCEDRV_SetOutputFreqOffsetInPPM(ppm);
                BACKDOOR_MGR_Printf("\r\nSet output clock frequency offset %s\r\n", (rc==TRUE)?"OK":"Failed");
            }
                break;
            default :
                ch = 0;
                break;
        }
    } /*  end of while    */
}
#endif /*SYS_CPNT_SYNC == TRUE*/


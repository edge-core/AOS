/* CSC: Reset Button Management
 *
 * State\Event |    RELEASED_EV    | PRESSED_EV |
 * ------------+-------------------+------------+
 * RELEASED_ST | RELEASED_ST       | PRESSED_ST | 
 * ------------+-------------------+------------+
 * PRESSED_ST  | ENTER_RELEASED_ST | PRESSED_ST |
 * ------------+-------------------+------------+
 */
#include "fs_type.h"
#include "fs.h"
#include "stktplg_pom.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "sys_hwcfg.h"
#include "sys_bld.h"
#include "led_pmgr.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_BINCFG == TRUE)
#include "bincfg_mgr.h"
#endif
#include "stkctrl_pmgr.h"
#include "phyaddr_access.h"

#ifdef ES4552MA_HPOE
#include "uc_mgr.h"
#endif

/* MACRO DECLARATIONS
 */
#define RESETMGMT_MGR_GPIO_ENTER_CRITICAL_SECTION() SYSFUN_ENTER_CRITICAL_SECTION(resetmgmt_mgr_gpio_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define RESETMGMT_MGR_GPIO_LEAVE_CRITICAL_SECTION() SYSFUN_LEAVE_CRITICAL_SECTION(resetmgmt_mgr_gpio_sem_id)

static void RESETMGMT_MGR_ResetToFactoryDefault();
static void RESETMGMT_MGR_Reset();

typedef enum
{
    RESETMGMT_BUTTON_RELEASED_ST = 0,
    RESETMGMT_BUTTON_PRESSED_ST,
    RESETMGMT_BUTTON_NOT_READY_ST,
    RESETMGMT_BUTTON_ENTER_RELEASED_ST,
}RESETMGMT_MGR_State_T;

typedef enum
{
    RESETMGMT_BUTTON_RELEASED_EV = 0,
    RESETMGMT_BUTTON_PRESSED_EV,
}RESETMGMT_MGR_Event_T;

//static SYS_TYPE_VAddr_T gpio_input_vaddr = 0; /* use for getting button virtual address */
static UI32_T resetmgmt_mgr_gpio_sem_id;

/* reset_fsm_table is directly coming from the comment */
static const RESETMGMT_MGR_State_T reset_fsm_table[3][2] = 
    {                /*RELEASE_EV */                      /* PRESS_EV */
/* RELEASE_ST */     {RESETMGMT_BUTTON_RELEASED_ST,       RESETMGMT_BUTTON_PRESSED_ST},
/* PRESS_ST   */     {RESETMGMT_BUTTON_ENTER_RELEASED_ST, RESETMGMT_BUTTON_PRESSED_ST},
/* N_READY_ST */     {RESETMGMT_BUTTON_RELEASED_ST,       RESETMGMT_BUTTON_NOT_READY_ST},
    };
/* Eugene: because LGN's HW design will have pressed event while no reset button daughter board connected 
 * it will led to keep reseting while boot up. So add a not ready state until receive a release event.
 */

static RESETMGMT_MGR_State_T state = RESETMGMT_BUTTON_NOT_READY_ST;
static UI32_T pressed_time = 0;
static BOOL_T resetting = FALSE;

SYSFUN_DECLARE_CSC;


/*-------------------------------------------------------------------------
 * FUNCTION NAME - RESETMGMT_MGR_HandleTimerEvents
 * ------------------------------------------------------------------------
 * PURPOSE  : Handle timer events
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void RESETMGMT_MGR_HandleTimerEvents()
{
    RESETMGMT_MGR_Event_T event = RESETMGMT_BUTTON_RELEASED_EV;
    UI8_T data_in;
    BOOL_T retval;
#if defined(ES4552MA_HPOE)
    UC_MGR_Sys_Info_T sysinfo;
#endif
    
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    
    /* getting the button state */
#if 0
    if(FALSE == PHYADDR_ACCESS_Read(gpio_input_vaddr, 4, 1, &gpio_input))
    {
        printf("\r\n%s: PHYADDR_ACCESS_Read failed", __FUNCTION__);
        return;
    }
    event = (!(gpio_input&SYS_HWCFG_GPIO6_MASK) ? RESETMGMT_BUTTON_PRESSED_EV : RESETMGMT_BUTTON_RELEASED_EV);
#endif
    
#if defined(ES4552MA_HPOE)
    if (UC_MGR_GetSysInfo(&sysinfo) == FALSE)
    {
        printf("%s(%d): UC_MGR_GetSysInfo failed.\r\n", __FUNCTION__, __LINE__);
        return;
    }
    
    switch (sysinfo.board_id)
    {
        case 0:
        case 1:
            /* for DX1035
             */
            RESETMGMT_MGR_GPIO_ENTER_CRITICAL_SECTION();
            retval = PHYSICAL_ADDR_ACCESS_Read(SYS_HWCFG_GPIO_IN_LOW+3, 1, 1, &data_in);
            RESETMGMT_MGR_GPIO_LEAVE_CRITICAL_SECTION();
            event = (data_in & 0x8) ? RESETMGMT_BUTTON_RELEASED_EV : RESETMGMT_BUTTON_PRESSED_EV;

            break;
        case 2:
        case 3:
            /* for DX3036
             */
            RESETMGMT_MGR_GPIO_ENTER_CRITICAL_SECTION();
            retval = PHYSICAL_ADDR_ACCESS_Read(SYS_HWCFG_GPIO_IN, 1, 1, &data_in);
            RESETMGMT_MGR_GPIO_LEAVE_CRITICAL_SECTION();
            event = (data_in & 0x20) ? RESETMGMT_BUTTON_RELEASED_EV : RESETMGMT_BUTTON_PRESSED_EV;

            break;
    }
#else	
    RESETMGMT_MGR_GPIO_ENTER_CRITICAL_SECTION();
    retval = PHYSICAL_ADDR_ACCESS_Read(SYS_HWCFG_GPIO_IN, 1, 1, &data_in);
    RESETMGMT_MGR_GPIO_LEAVE_CRITICAL_SECTION();
    
    if (data_in &= 0x20)
        event = RESETMGMT_BUTTON_RELEASED_EV;
    else
        event = RESETMGMT_BUTTON_PRESSED_EV;
#endif
    
    /* get the next state */
    state = reset_fsm_table[state][event];
    
    switch(state)
    {
        case RESETMGMT_BUTTON_RELEASED_ST:
        case RESETMGMT_BUTTON_NOT_READY_ST:
            /* do nothing */
            break;
            
        case RESETMGMT_BUTTON_PRESSED_ST:
            if(resetting == FALSE && ++pressed_time >= SYS_ADPT_RESETMGMT_RESET_TO_FACTORY_DEFAULT_TIME )
            {
                resetting = TRUE;
                RESETMGMT_MGR_ResetToFactoryDefault();
            }
            break;
            
        case RESETMGMT_BUTTON_ENTER_RELEASED_ST:
                state = RESETMGMT_BUTTON_RELEASED_ST;
            if (pressed_time >= SYS_ADPT_RESETMGMT_RESET_TIME)
            {
                if (resetting == FALSE)
            {
                    resetting = TRUE;
                    RESETMGMT_MGR_Reset();
                }
            }
                pressed_time = 0;
            break;
        default:
            /* unknown state error !!*/
            break;
    }
   
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - RESETMGMT_MGR_Init
 * ------------------------------------------------------------------------
 * PURPOSE  : Initalize the reset button 
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void RESETMGMT_MGR_Init()
{
    /*
    if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_GPIO_INPUT_ADDR, &gpio_input_vaddr))
    {
        printf("\r\n%s: PHYADDR_ACCESS_GetVirtualAddr failed", __FUNCTION__);
        return;
    }
    */
    UI32_T ret;

    ret = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_GPIO, &resetmgmt_mgr_gpio_sem_id);
    if (ret != SYSFUN_OK)
    {
        printf("%s:%d: SYSFUN_GetSem fails. (%lu)\n", __FUNCTION__, __LINE__, ret);
        return;
    }

    state = RESETMGMT_BUTTON_RELEASED_ST;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - RESETMGMT_MGR_EnterMasterMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Initalize the reset button 
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void RESETMGMT_MGR_EnterMasterMode()
{
    SYSFUN_ENTER_MASTER_MODE();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - RESETMGMT_MGR_EnterSlaveMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Initalize the reset button 
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void RESETMGMT_MGR_EnterSlaveMode()
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - RESETMGMT_MGR_EnterTransitionMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Initalize the reset button 
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void RESETMGMT_MGR_EnterTransitionMode()
{
    SYSFUN_ENTER_TRANSITION_MODE();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - RESETMGMT_MGR_SetTransitionMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Initalize the reset button 
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void RESETMGMT_MGR_SetTransitionMode()
{
    SYSFUN_SET_TRANSITION_MODE();
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - RESETMGMT_MGR_ResetToFactoryDefault
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset To Factory default
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void RESETMGMT_MGR_ResetToFactoryDefault()
{
    UI32_T my_unit_id;
    
    STKTPLG_POM_GetMyUnitID(&my_unit_id);
        
printf("\r\nResetting system to factory defaults...");
fflush(stdout);
    
    FS_SetStartupFilename(my_unit_id, FS_FILE_TYPE_CONFIG, (UI8_T*)SYS_DFLT_restartConfigFile);
    
#if (SYS_CPNT_BINCFG == TRUE)
    BINCFG_MGR_SetFactoryDefaultState(TRUE);
#endif
    
#if (SYS_CPNT_LEDMGMT_PATTERN_DISPALY == TRUE)
    LED_PMGR_PatternDisplaySet(LED_MGR_PATTERN_DISPLAY_ALL_FLASH, 25, 2);
    SYSFUN_Sleep(100); /* for LED display */
#endif
    STKCTRL_PMGR_ColdStartSystem();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - RESETMGMT_MGR_HandleTimerEvents
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void RESETMGMT_MGR_Reset()
{
printf("\r\nResetting system...");
fflush(stdout);

    STKCTRL_PMGR_ColdStartSystem();
}


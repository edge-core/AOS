/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_TIME.C
 * ------------------------------------------------------------------------
 * PURPOSE: This package provides the services to handle the system timer
 *          functions
 *
 * Notes: This module will support all the system timer functions includes
 *        1) System Ticks
 *        2) RTC timer setting/getting if the H/W support RTC on the board
 *
 *  History
 *
 *   Jason Hsue     11/19/2001      new created
 *   Echo Chen      7/18/2007       modified for Linux Platform
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2001
 * ------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys_hwcfg.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_time.h"
#include "uc_mgr.h"
#if (SYS_CPNT_STACKING == TRUE)
#include "isc_om.h"
#include "sys_time_stk.h"
#endif
#include "l_stdlib.h"
#include "phyaddr_access.h"
#include "sysrsc_mgr.h"
#include "sysdrv.h"
//#include "stktplg_pom.h"
#include "sys_dflt.h"

#include "sys_module.h"
#include "syslog_pmgr.h"
#include "backdoor_mgr.h"

#if (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_OS6450)
#include "i2cdrv.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#ifndef _countof
#define _countof(_Ary)  (sizeof(_Ary) / sizeof(*_Ary))
#endif
#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
#if (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_ECS4910_28F)
/* Watch Dog Control(Read/Write, 0xE010-0012)
 *  ------------------------------------------------------------------------------
 * |    7    |    6    |    5    |    4    |    3    |    2    |    1    |    0   |
 * |---------|---------|---------|---------|---------|---------|---------|--------|
 * | WDT_CLK | DIS_WDT |Reserved |Reserved |Reserved |Reserved |Reserved |Reserved|
 * -------------------------------------------------------------------------------
 WDT_CLK:
        Watchdog timer clock. A high to low transition will kick off watchdog timer.
        The watchdog timer expired if there is no more kicking on it more than 6 seconds.
        This is should be keep in 0 during normal operation state. A low to high then
        high to low transition is required to kick off or extend the watchdog timer.

 DIS_WDT:
        Watchdog timer disable, it prevents watchdog timer expired event to the NMI
        input of CPU. It is a logic-AND to the output of the watchdog timer to gate
        off the timer expired event to the CPU.
        It should be note that this bit should enable after the watchdog timer has
        been kicked off.
        Otherwise a wrong timer expired event will be generated.
        1: Enable the watchdog timer expired output signal to the NMI input of CPU
        0: Disable the watchdog timer expired output signal to the NMI input of CPU
 */
#define  WATCHDOG_ADDR        (SYS_HWCFG_WATCH_DOG_CONTROL_ADDRESS)
#define  WATCHDOG_ENABLE      (SYS_HWCFG_WATCH_DOG_CONTROL_DIS_WDT)
#define  WATCHDOG_ENABLE_MASK (SYS_HWCFG_WATCH_DOG_CONTROL_DIS_WDT_MASK)
#define  WATCHDOG_KICK_H      (SYS_HWCFG_WATCH_DOG_CONTROL_WDT_CLK)
#define  WATCHDOG_KICK_L      (0x0)
#define  WATCHDOG_KICK_MASK   (SYS_HWCFG_WATCH_DOG_CONTROL_WDT_CLK_MASK)

#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_MARVELL_PONCAT)
#define MARVELL_PONCAT_RSTOUTN_MASK_REGISTER       (SYS_HWCFG_CPU_REG_BASE_ADDR | 0x00020108UL)
#define MARVELL_PONCAT_CPU_TIMERS_CONTROL_REGISTER (SYS_HWCFG_CPU_REG_BASE_ADDR | 0x00020300UL)
#define MARVELL_PONCAT_CPU_WATCHDOG_TIMER_REGISTER (SYS_HWCFG_CPU_REG_BASE_ADDR | 0x00020324UL)
#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_OS6450)
#define CPLD_I2C_ADDR                  (SYS_HWCFG_I2C_SLAVE_CPLD)
#define CPLD_WATCHDOG_COUNTER_REG_ADDR (SYS_HWCFG_WATCHDOG_COUNTER_REG_ADDR)
#define CPLD_WATCHDOG_CONTROL_REG_ADDR (SYS_HWCFG_WATCHDOG_CONTROL_REG_ADDR)
#define WDT_RST_EN                     (0x40)

#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_ECS5610_52S)
/* Watch Dog Control(Read/Write, 0xEA00000E)
   -------------------------------------------------------------------------------
  |    7    |    6    |    5    |    4    |    3    |    2    |    1    |    0    |
  |---------|---------|---------|---------|---------|---------|---------|---------|
  |Reserved |Reserved |wdog_cnt3|wdog_cnt2|wdog_cnt1|wdog_cnt0|wdog enab|wdog kick|
   -------------------------------------------------------------------------------
   wdog kick:
   CPU will deliver kick signal (active at high) to CPLD with the period of 100 ms when it's awake
   and working properly.
   0: CPU not deliver kick signal to CPLD (This bit will auto reset to 0 when CPU set kick signal)
   1: CPU deliver kick signal to CPLD

   wdog enable:
   Enable the watch dog function from CPU
   1: Enable the watch dog
   0: Disable the watch dog

   wdog_cnt[3:0]:
   This 4 bit register can be set by SW to change the watch dog timing.
   Watch dog timing as below

   wdog_cnt    NMI Time    Reset All Time (Sec)
          2          32                64
 */
#define  WATCHDOG_ADDR           SYS_HWCFG_WATCHDOG_CONTROL_ADDRESS
#define  WATCHDOG_KICK           SYS_HWCFG_WATCHDOG_KICK
#define  WATCHDOG_KICK_MASK      SYS_HWCFG_WATCHDOG_KICK_MASK
#define  WATCHDOG_ENABLE         SYS_HWCFG_WATCHDOG_ENABLE
#define  WATCHDOG_ENABLE_MASK    SYS_HWCFG_WATCHDOG_ENABLE_MASK
#define  WATCHDOG_CNT            SYS_HWCFG_WATCHDOG_CNT
#define  WATCHDOG_CNT_MASK       SYS_HWCFG_WATCHDOG_CNT_MASK
#endif /* end of #if (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_MPC8248) */
#endif /* SYS_CPNT_WATCHDOG_TIMER */

/* STATIC VARIABLE DEFINITIONS
 */
static const UI32_T DAY_OF_MONTH[2][12] = { {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                                        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31} };

static const char *month_name_ar[]=
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char *weekday_name_ar[]=
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char *predefined_region_name_ar[]=
{
    "USA",
    "Europe",
    "Australia",
    "New Zealand"
};

static SYS_TIME_Timezone_Predefined_T systime_timezone_predefined[] =
{
   {VAL_sysTimeZonePredefined_minus1200InternationalDateLineWest,                 -720, "GMT-1200-International-Date-Line-West"},
   {VAL_sysTimeZonePredefined_minus1100MidwayIslandSamoa,                         -660, "GMT-1100-Midway-Island-Samoa"},
   {VAL_sysTimeZonePredefined_minus1000Hawaii,                                    -600, "GMT-1000-Hawaii"},
   {VAL_sysTimeZonePredefined_minus0930Taiohae,                                   -570, "GMT-0930-Taiohae"},
   {VAL_sysTimeZonePredefined_minus0900Alaska,                                    -540, "GMT-0900-Alaska"},
   {VAL_sysTimeZonePredefined_minus0800PacificTimeTijuana,                        -480, "GMT-0800-Pacific-Time(US&Canada),Tijuana"},
   {VAL_sysTimeZonePredefined_minus0700Arizona,                                   -420, "GMT-0700-Arizona"},
   {VAL_sysTimeZonePredefined_minus0700ChihuahuaLaPazMazatlan,                    -420, "GMT-0700-Chihuahua,La-Paz,Mazatlan"},
   {VAL_sysTimeZonePredefined_minus0700MountainTimeUSCanada,                      -420, "GMT-0700-Mountain-Time(US&Canada)"},
   {VAL_sysTimeZonePredefined_minus0600CentralAmerica,                            -360, "GMT-0600-Central-America"},
   {VAL_sysTimeZonePredefined_minus0600CentralTimeUSCanada,                       -360, "GMT-0600-Central-Time(US&Canada)"},
   {VAL_sysTimeZonePredefined_minus0600GuadalajaraMexicoCityMonterrey,            -360, "GMT-0600-Guadalajara,Mexico-City,Monterrey"},
   {VAL_sysTimeZonePredefined_minus0600Saskatchewan,                              -360, "GMT-0600-Saskatchewan"},
   {VAL_sysTimeZonePredefined_minus0500BogotaLimaQuito,                           -300, "GMT-0500-Bogota,Lima,Quito"},
   {VAL_sysTimeZonePredefined_minus0500EasternTimeUSCanada,                       -300, "GMT-0500-Eastern-Time(US&Canada)"},
   {VAL_sysTimeZonePredefined_minus0500IndianaEast,                               -300, "GMT-0500-Indiana(East)"},
   {VAL_sysTimeZonePredefined_minus0400AtlanticTimeCanada,                        -240, "GMT-0400-Atlantic-Time(Canada)"},
   {VAL_sysTimeZonePredefined_minus0400CaracasLaPaz,                              -240, "GMT-0400-Caracas,La-Paz"},
   {VAL_sysTimeZonePredefined_minus0400Santiago,                                  -240, "GMT-0400-Santiago"},
   {VAL_sysTimeZonePredefined_minus0330Newfoundland,                              -210, "GMT-0300-Newfoundland"},
   {VAL_sysTimeZonePredefined_minus0300Brasilia,                                  -180, "GMT-0300-Brasilia"},
   {VAL_sysTimeZonePredefined_minus0300BuenosAiresGeorgetown,                     -180, "GMT-0300-Buenos-Aires,Georgetown"},
   {VAL_sysTimeZonePredefined_minus0300Greenland,                                 -180, "GMT-0300-Greenland"},
   {VAL_sysTimeZonePredefined_minus0200MidAtlantic,                               -120, "GMT-0200-Mid-Atlantic"},
   {VAL_sysTimeZonePredefined_minus0100Azores,                                    -60,  "GMT-0100-Azores"},
   {VAL_sysTimeZonePredefined_minus0100CapeVerdeIs,                               -60,  "GMT-0100-Cape-Verde-Is"},
   {VAL_sysTimeZonePredefined_gmtCasablancaMonrovia,                              0,    "GMT-Casablanca,Monrovia"},
   {VAL_sysTimeZonePredefined_gmtDublinEdinburghLisbonLondon,                     0,    "GMT-Greenwich-Mean-Time-Dublin,Edinburgh,Lisbon,London"},
   {VAL_sysTimeZonePredefined_plus0100AmsterdamBerlinBernRomeStockholmVienna,     60,   "GMT+0100-Amsterdam,Berlin,Bern,Rome,Stockholm,Vienna"},
   {VAL_sysTimeZonePredefined_plus0100BelgradeBratislavaBudapestLjubljanaPrague,  60,   "GMT+0100-Belgrade,Bratislava,Budapest,Ljubljana,Prague"},
   {VAL_sysTimeZonePredefined_plus0100BrusselsCopenhagenMadridParis,              60,   "GMT+0100-Brussels,Copenhagen,Madrid,Paris"},
   {VAL_sysTimeZonePredefined_plus0100SarajevoSkopjeWarsawZagreb,                 60,   "GMT+0100-Sarajevo,Skopje,Warsaw,Zagreb"},
   {VAL_sysTimeZonePredefined_plus0100WestCentralAfrica,                          60,   "GMT+0100-WestCentralAfrica"},
   {VAL_sysTimeZonePredefined_plus0200AthensBeirutIstanbulMinsk,                  120,  "GMT+0200-Athens,Beirut,Istanbul,Minsk"},
   {VAL_sysTimeZonePredefined_plus0200Bucharest,                                  120,  "GMT+0200-Bucharest"},
   {VAL_sysTimeZonePredefined_plus0200Cairo,                                      120,  "GMT+0200-Cairo"},
   {VAL_sysTimeZonePredefined_plus0200HararePretoria,                             120,  "GMT+0200-Harare,Pretoria"},
   {VAL_sysTimeZonePredefined_plus0200HelsinkiKyivRigaSofiaTallinnVilnius,        120,  "GMT+0200-Helsinki,Kyiv,Riga,Sofia,Tallinn,Vilnius"},
   {VAL_sysTimeZonePredefined_plus0200Jerusalem,                                  120,  "GMT+0200-Jerusalem"},
   {VAL_sysTimeZonePredefined_plus0300Baghdad,                                    180,  "GMT+0300-Baghdad"},
   {VAL_sysTimeZonePredefined_plus0300KuwaitRiyadh,                               180,  "GMT+0300-Kuwait,Riyadh"},
   {VAL_sysTimeZonePredefined_plus0300MoscowStPetersburgVolgograd,                180,  "GMT+0300-Moscow,St.Petersburg,Volgograd"},
   {VAL_sysTimeZonePredefined_plus0300Nairobi,                                    180,  "GMT+0300-Nairobi"},
   {VAL_sysTimeZonePredefined_plus0330Tehran,                                     210,  "GMT+0330-Tehran"},
   {VAL_sysTimeZonePredefined_plus0400AbuDhabiMuscat,                             240,  "GMT+0400-Abu-Dhabi,Muscat"},
   {VAL_sysTimeZonePredefined_plus0400BakuTbilisiYerevan,                         240,  "GMT+0400-Baku,Tbilisi,Yerevan"},
   {VAL_sysTimeZonePredefined_plus0430Kabul,                                      270,  "GMT+0430-Kabul"},
   {VAL_sysTimeZonePredefined_plus0500Ekaterinburg,                               300,  "GMT+0500-Ekaterinburg"},
   {VAL_sysTimeZonePredefined_plus0500IslamabadKarachiTashkent,                   300,  "GMT+0500-Islamabad,Karachi,Tashkent"},
   {VAL_sysTimeZonePredefined_plus0530ChennaiCalcutaMumbaiNewDelhi,               330,  "GMT+0530-Chennai,Calcuta,Mumbai,New-Delhi"},
   {VAL_sysTimeZonePredefined_plus0545Kathmandu,                                  345,  "GMT+0545-Kathmandu"},
   {VAL_sysTimeZonePredefined_plus0600AlmatyNovosibirsk,                          360,  "GMT+0600-Almaty,Novosibirsk"},
   {VAL_sysTimeZonePredefined_plus0600AstanaDhaka,                                360,  "GMT+0600-Astana,Dhaka"},
   {VAL_sysTimeZonePredefined_plus0600SriJayawardenepura,                         360,  "GMT+0600-Sri-Jayawardenepura"},
   {VAL_sysTimeZonePredefined_plus0630Rangoon,                                    390,  "GMT+0630-Rangoon"},
   {VAL_sysTimeZonePredefined_plus0700BangkokHanoiJakarta,                        420,  "GMT+0700-Bangkok,Hanoi,Jakarta"},
   {VAL_sysTimeZonePredefined_plus0700Krasnoyarsk,                                420,  "GMT+0700-Krasnoyarsk"},
   {VAL_sysTimeZonePredefined_plus0800BeijingChongqingHongKongUrumqi,             480,  "GMT+0800-Beijing,Chongqing,Hong-Kong,Urumqi"},
   {VAL_sysTimeZonePredefined_plus0800IrkutskUlaanBataar,                         480,  "GMT+0800-Irkutsk,Ulaan-Bataar"},
   {VAL_sysTimeZonePredefined_plus0800KualaLumpurSingapore,                       480,  "GMT+0800-Kuala-Lumpur,Singapore"},
   {VAL_sysTimeZonePredefined_plus0800Perth,                                      480,  "GMT+0800-Perth"},
   {VAL_sysTimeZonePredefined_plus0800Taipei,                                     480,  "GMT+0800-Taipei"},
   {VAL_sysTimeZonePredefined_plus0900OsakaSapporoTokyo,                          540,  "GMT+0900-Osaka,Sapporo,Tokyo"},
   {VAL_sysTimeZonePredefined_plus0900Seoul,                                      540,  "GMT+0900-Seoul"},
   {VAL_sysTimeZonePredefined_plus0900Yakutsk,                                    540,  "GMT+0900-Yakutsk"},
   {VAL_sysTimeZonePredefined_plus0930Adelaide,                                   570,  "GMT+0930-Adelaide"},
   {VAL_sysTimeZonePredefined_plus0930Darwin,                                     570,  "GMT+0930-Darwin"},
   {VAL_sysTimeZonePredefined_plus1000Brisbane,                                   600,  "GMT+1000-Brisbane"},
   {VAL_sysTimeZonePredefined_plus1000CanberraMelbourneSydney,                    600,  "GMT+1000-Canberra,Melbourne,Sydney"},
   {VAL_sysTimeZonePredefined_plus1000GuamPortMoresby,                            600,  "GMT+1000-Guam,PortMoresby"},
   {VAL_sysTimeZonePredefined_plus1000Hobart,                                     600,  "GMT+1000-Hobart"},
   {VAL_sysTimeZonePredefined_plus1000Vladivostok,                                600,  "GMT+1000-Vladivostok"},
   {VAL_sysTimeZonePredefined_plus1030LordHoweIsland,                             630,  "GMT+1030-LordHoweIsland"},
   {VAL_sysTimeZonePredefined_plus1100MagadanSolomonIsNewCaledonia,               660,  "GMT+1100-Magadan,Solomon-Is.,New-Caledonia"},
   {VAL_sysTimeZonePredefined_plus1130Kingston,                                   690,  "GMT+1130-Kingston"},
   {VAL_sysTimeZonePredefined_plus1200AucklandWellington,                         720,  "GMT+1200-Auckland,Wellington"},
   {VAL_sysTimeZonePredefined_plus1200FijiKamchatkaMarshallIs,                    720,  "GMT+1200-Fiji,Kamchatka,Marshall-Is"},
   {VAL_sysTimeZonePredefined_plus1245ChathamIsland,                              765,  "GMT+1245-ChathamIsland"},
   {VAL_sysTimeZonePredefined_plus1300Nukualofa,                                  780,  "GMT+1300-Nuku'alofa"},
   {VAL_sysTimeZonePredefined_plus1400Kiritimati,                                 840,  "GMT+1400-Kiritimati"}
};

#define LEAP_YEAR           1
#define NORMAL_YEAR         0

#define SECOND_OF_YEAR(yday, hour, minute, second) (second + (60 * minute) + 60 * 60 * (hour + (yday * 24)))

#define is_leap_year(_y_) ((((_y_) % 4 == 0 && (_y_) % 100 != 0) || (_y_) % 400 == 0) ? TRUE : FALSE)

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static SYS_TIME_Shmem_Data_T *sys_time_shmem_data_p;
static BOOL_T SYS_TIME_GetUtcSoftwareClockBySec(UI32_T *sec);
static BOOL_T SYS_TIME_StrIsDigitEx(const char *str_p, const char *except_ar_p);

#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
static BOOL_T SYS_TIME_DST_ApplyToCurrentTime(UI32_T *seconds_p, SYS_TIME_DST_MODE_T mode, int offset);
static BOOL_T SYS_TIME_DST_SetFirstDay(const SYS_TIME_DST *begin_dst_p);
static BOOL_T SYS_TIME_DST_SetLastDay(const SYS_TIME_DST *end_dst_p);
static BOOL_T SYS_TIME_DST_GetFirstDay(SYS_TIME_DST *begin_dst_p);
static BOOL_T SYS_TIME_DST_GetLastDay(SYS_TIME_DST *end_dst_p);
static BOOL_T SYS_TIME_DST_SetRecurringWeek(const SYS_TIME_RECURRING_DST *start_day_p, const SYS_TIME_RECURRING_DST *end_day_p);
static BOOL_T SYS_TIME_DST_GetRecurringWeek(SYS_TIME_RECURRING_DST *start_day_p, SYS_TIME_RECURRING_DST *end_day_p);
static UI32_T SYS_TIME_DST_ConvertWdayToYday(UI32_T year,UI32_T month, UI32_T week, UI32_T wday);
static UI32_T SYS_TIME_DST_TimeToSecondOfYear(const SYS_TIME_DST *current_time_p);
static UI32_T SYS_TIME_DST_RecurringToSecondOfYear(const SYS_TIME_RECURRING_DST *current_time_p);
static UI32_T SYS_TIME_DST_GetYdayTillMonth(UI32_T year, UI32_T month, UI32_T day);
static UI32_T SYS_TIME_DST_GetWdayOfFirstDayOfYear(UI32_T year);
static BOOL_T SYS_TIME_DST_IsValidTime(SYS_TIME_DST_MODE_T mode, UI32_T month, UI32_T week, UI32_T wday, UI32_T day, UI32_T hour, UI32_T minute, UI32_T second);
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */

static BOOL_T SYS_TIME_GetDaysOfMonth(UI32_T year, UI32_T month, UI32_T *days_p);

static BOOL_T
SYS_TIME_GetTimezonePredefineIndex(
    UI32_T  timezone_id,
    UI32_T  *index
    );

static BOOL_T
SYS_TIME_GetTimeZoneInfo(
    SYS_TIME_Timezone_T *timezone
    );

static UI32_T sys_time_om_sem_id;
#define SYS_TIME_OM_ENTER_CRITICAL_SECTION() \
    SYSFUN_TakeSem(sys_time_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SYS_TIME_OM_LEAVE_CRITICAL_SECTION() \
    SYSFUN_GiveSem(sys_time_om_sem_id)

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: SYS_TIME_Initiate_System_Resources
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for SYS_TIME
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
BOOL_T SYS_TIME_InitiateSystemResources(void)
{
    sys_time_shmem_data_p = (SYS_TIME_Shmem_Data_T*) \
        SYSRSC_MGR_GetShMem(SYSRSC_MGR_SYS_TIME_SHMEM_SEGID);
    memset(sys_time_shmem_data_p, 0, sizeof(SYS_TIME_Shmem_Data_T));
    SYSFUN_INITIATE_CSC_ON_SHMEM(sys_time_shmem_data_p);

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYSTIME_OM, &sys_time_om_sem_id)
        != SYSFUN_OK)
        ERRMSG("%s SYSFUN_GetSem error\n", __FUNCTION__);

 //   SYS_TIME_Init();

    return TRUE;
}  /* End of SYSDRV_Initiate_System_Resources() */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSDRV_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for SYS_TIME in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYS_TIME_AttachSystemResources(void)
{
    sys_time_shmem_data_p = (SYS_TIME_Shmem_Data_T*) \
        SYSRSC_MGR_GetShMem(SYSRSC_MGR_SYS_TIME_SHMEM_SEGID);

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYSTIME_OM, &sys_time_om_sem_id)
        != SYSFUN_OK)
        ERRMSG("%s SYSFUN_GetSem error\n", __FUNCTION__);
    return;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_Init_InitiateProcessResources
 * ---------------------------------------------------------------------
 * PURPOSE: This function will init the system resource
 *
 * INPUT   : read_fn_p  -  Function pointer for doing I2C read operation. Use
 *                         default I2C read function if this argument is NULL.
 *           write_fn_p -  Function pointer for doing I2C write operation. Use
 *                         default I2C write function if this argument is NULL.
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: 1. This routine will initialize the software clock by copy time
 *           information from hardware clock if RTC exists.
 *        2. If RTC doesn't exists, then set software clock to zero.
 *        3. Software clock always store UTC time in seconds point from
 *           01/01/1970 00:00:00
 * History:  S.K.Yang     06/7/2002      modified
 *           haiqiang.li  11/01/2008     rewrite
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_TIME_Init_InitiateProcessResources(I2CDRV_TwsiDataRead_Func_T read_fn_p,
                     I2CDRV_TwsiDataWrite_Func_T write_fn_p)
{
    /* __SECONDS1970__ is the build time which is passed by compiler
     */
    UI32_T seconds_since_19700101_build= __SECONDS1970__;
    int    seconds_since_19700101_current=__SECONDS1970__;
    UI32_T active_seconds=__SECONDS1970__;
    UC_MGR_Sys_Info_T uc_sys_info;
    SYSFUN_Time_T os_time;

    if (SYSFUN_GetSysClock(&os_time)!=SYSFUN_OK)
    {
        printf("%s(%d)SYSFUN_GetSysClock error.\r\n", __FUNCTION__, __LINE__);
    }
    seconds_since_19700101_current = os_time;

#if (SYS_HWCFG_SUPPORT_RTC == TRUE)
    BOOL_T ret;
    int year, month, day, hour, minute, second;
    //SYSLOG_OM_Record_T syslog_entry;

    /* For projects using mvl chip, they will call DEV_SWDRV_PMGR to
     * query register of RTC. Since thread that handle the ipcmsg is
     * not spawned yet, need call with fun ptr.
     */
    ret = SYSDRV_RTC_GetDateTime(read_fn_p, write_fn_p, &year, &month, &day, &hour, &minute, &second);

    if (ret == FALSE)
    {
        /* sync RTC
         */
        //SYSDRV_RTC_SetDateTime(year, month, day, hour, minute, second);
        //memset(&syslog_entry, 0, sizeof(SYSLOG_OM_Record_T));
        //syslog_entry.owner_info.level = SYSLOG_LEVEL_WARNING;
        //syslog_entry.owner_info.module_no = SYS_MODULE_SYSDRV;
        //syslog_entry.owner_info.function_no = 0;
        //syslog_entry.owner_info.error_no = 0;
        //sprintf((char *)syslog_entry.message, "bad RTC date time, RTC date time will be set to %4d-%02d-%02d %02d:%02d:%02d\n",
        //        year, month, day, hour, minute, second);
        //SYSLOG_PMGR_AddEntry(&syslog_entry);
    }
    else
    {
        SYSFUN_mktime(year, month, day, hour,
                      minute, second, &seconds_since_19700101_build);
    }
#endif /*End of #if (SYS_HWCFG_SUPPORT_RTC == TRUE) */

    /* sync kernel time when the secords_since_19700101_build is larger than
     * seconds_since_19700101_current
     */
    if ((int)seconds_since_19700101_build > seconds_since_19700101_current)
    {
        SYSFUN_SetSysClock(seconds_since_19700101_build);
        active_seconds=seconds_since_19700101_build;
    }
    else
    {
        active_seconds=seconds_since_19700101_current;
    }

    /* sync software clock
     */
    sys_time_shmem_data_p->software_clock.time = active_seconds;
    strncpy(sys_time_shmem_data_p->time_zone.zone_name,
        SYS_TIME_DEFAULT_TIMEZONE_NAME, sizeof(sys_time_shmem_data_p->time_zone.zone_name) - 1);
    sys_time_shmem_data_p->time_zone.zone_name[sizeof(sys_time_shmem_data_p->time_zone.zone_name) - 1] = '\0';
    sys_time_shmem_data_p->time_zone.offset = SYS_TIME_DEFAULT_TIMEZONE_OFFSET;
    sys_time_shmem_data_p->time_zone.timezone_offset_id = SYS_TIME_DEFAULT_TIMEZONE_ID;

    if (UC_MGR_GetSysInfo(&uc_sys_info)==FALSE)
    {
        printf("%s(%d):Get UC System Information Fail.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    sys_time_shmem_data_p->board_id = uc_sys_info.board_id;

#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
    {
        UI32_T i = 0;

        sys_time_shmem_data_p->dst_data.is_effect = FALSE;
        sys_time_shmem_data_p->dst_data.offset    = SYS_TIME_DEFAULT_DST_TIME_OFFSET;
        sys_time_shmem_data_p->dst_data.mode      = SYS_TIME_DEFAULT_DAYLIGHT_SAVING_TIME;
        memset(&(sys_time_shmem_data_p->dst_data.first_day), 0, sizeof(SYS_TIME_DST));
        memset(&(sys_time_shmem_data_p->dst_data.last_day),  0, sizeof(SYS_TIME_DST));
        memset(&(sys_time_shmem_data_p->dst_data.start_wday), 0, sizeof(SYS_TIME_RECURRING_DST));
        memset(&(sys_time_shmem_data_p->dst_data.end_wday),   0, sizeof(SYS_TIME_RECURRING_DST));
#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
        memset(sys_time_shmem_data_p->dst_data.zone_name, 0, SYS_TIME_MAX_DST_TIMEZONE_NAME_LEN+1);
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE) */

        /* total days from January to the specified month
         * ydays_till_month[0] saves the days of non-leap year
         * ydays_till_month[1] saves the days of leap year
         * e.g. ydays_till_month[0][1] & ydays_till_month[1][1] are 31 days
         *      ydays_till_month[0][2] is (31 + 28) days
         *      ydays_till_month[1][2] is (31 + 29) days
         */
        sys_time_shmem_data_p->dst_data.ydays_till_month[0][0] = 0;
        sys_time_shmem_data_p->dst_data.ydays_till_month[1][0] = 0;
        for(i = 1; i < (sizeof(sys_time_shmem_data_p->dst_data.ydays_till_month[0])/sizeof(*sys_time_shmem_data_p->dst_data.ydays_till_month[0])); i++)
        {
            sys_time_shmem_data_p->dst_data.ydays_till_month[0][i] = DAY_OF_MONTH[0][i-1] + sys_time_shmem_data_p->dst_data.ydays_till_month[0][i-1];
            sys_time_shmem_data_p->dst_data.ydays_till_month[1][i] = DAY_OF_MONTH[1][i-1] + sys_time_shmem_data_p->dst_data.ydays_till_month[1][i-1];
        }
    }
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */

    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SYS_TIME_Create_InterCSC_Relation(void)
{
    /* River@Apr 22, 2008, add backdoor to check if software clock matched with RTC
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("systime", SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY, SYS_TIME_Main);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter master mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------*/
void SYS_TIME_EnterMasterMode(void)
{
    /* set mgr in master mode
     */
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(sys_time_shmem_data_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter slave mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(sys_time_shmem_data_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the SYS_TIME into the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SYS_TIME_SetTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(sys_time_shmem_data_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_EnterTransitionMode(void)
{
    /* wait other callers leave
     */
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(sys_time_shmem_data_p);

    sys_time_shmem_data_p->is_provision_complete = FALSE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: This function is used to notify provision complete.
 * INPUT: None
 * OUTPUT:
 * RETURN: none
 * NOTES: None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ProvisionComplete(UI32_T unit)
{
    /* if unit != 0, it means MOULE_PROVISION_COMPLETE. We don't need to update the provision_complete_ticks.
     * But, we should update the system time to Slave.
     * Original issue: please refer to EPR ES3628BT-FLF-ZZ-00548.
     * Problem: the systemuptimeis clear when stack hot swap remove unit.
     */
    if (unit == 0)
    {
        sys_time_shmem_data_p->provision_complete_ticks = SYS_TIME_GetSystemTicksBy10ms();

        sys_time_shmem_data_p->is_provision_complete = TRUE;
    }

#if (SYS_CPNT_STACKING == TRUE)
        if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p)
            == SYS_TYPE_STACKING_MASTER_MODE)
        {
            SYS_TIME_RemoteSyncDateTime();
        }
#endif
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetRealTimeClock
 * ---------------------------------------------------------------------
 * PURPOSE  : get real TOD(Time-Of-Day)
 * INPUT    : None
 * OUTPUT   : year, month, day, hour, minute, second
 * RETURN   : None
 * NOTES    : 1.If there is no RTC in the system, then then get time
 *              from S/W clock. (Local time)
 *            2.If there is no RTC in system, then this routine is same
 *              as SYS_TIME_GetSoftwareClock
 * History  : S.K.Yang     06/10/2002      modified
 *            haiqiang.li  11/01/2008      modified
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetRealTimeClock(int *year, int *month, int *day, int *hour,
                               int *minute, int *second)
{
   /* shumin.wang, fix bug ES4827G-FLF-ZZ-00309
    */
#if 0
   SYSDRV_RTC_GetDateTime(&year_tmp,&month_tmp,&day_tmp,&hour_tmp,&minute_tmp,&second_tmp);
   SYS_TIME_ConvertDateTimeToSeconds(year_tmp,month_tmp,day_tmp,hour_tmp,minute_tmp,second_tmp,(UI32_T *)&utc_seconds);
   /* Convert it to local time
    */
   local_seconds = utc_seconds + sys_time_shmem_data_p->time_zone.offset * 60;
   SYS_TIME_ConvertSecondsToDateTime(local_seconds,year,month,day,hour,minute,second);
#else
   /* getting real time from RTC every time is a very bad idea since
    * the i/o operation is very very slower than cpu instruction execution.
    * haiqiang.li */
   SYS_TIME_GetSoftwareClock(year, month, day, hour, minute, second);
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetRealTimeBySec
 * ---------------------------------------------------------------------
 * PURPOSE  : get real time in seconds
 * INPUT    : None
 * OUTPUT   : seconds
 * RETURN   : None
 * NOTES    : 1. This API is used for Sys_Log. When the system log try to
 *               log event, he will call this API to get current time by
 *               how many seconds.
 *            2. This API will always based on 1/1/2001 00:00:00 as 0 second
 *               to count how many seconds pass away from that time.
 *            3. If there is no RTC in the system, then then get seconds
 *                from S/W clock (Local Time)
 *            4. If no RTC exists, this rountine behave like SYS_TIME_GetSoftwareClockBySec
 * History  :  S.K.Yang     06/7/2002      modified
 *             haiqiang.li  11/01/2008     modified
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetRealTimeBySec(UI32_T *seconds)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return ;
    }
    else
    {
        SYS_TIME_GetSoftwareClockBySec(seconds);
    }
}

void SYS_TIME_GetUtcRealTimeBySec(UI32_T *seconds)
{
    /* When user configures the system timer, Master will sync the timer to slave. Therefore, the timer in the slave is
     * the same as the master. The applications can use this API to get the system timer, whatever the caller is
     * Master or Salve.
     * For example, update file to slave device, the file-map-table needs the timer about this file.
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return ;
    }
    else
    {
        SYS_TIME_GetUtcSoftwareClockBySec(seconds);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ConvertSecondsToDateTime
 * ---------------------------------------------------------------------
 * PURPOSE  : Convert seconds to date time without time zone offset
 * INPUT    : seconds -- seconds
 * OUTPUT   : year    -- year
 *            month   -- month
 *            day     -- day
 *            hour    -- hour
 *            minute  -- minute
 *            second  -- second
 * RETURN   : None
 * NOTES    : The same as gmtime()
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ConvertSecondsToDateTime(UI32_T seconds, int *year, int *month, int *day,
                                       int *hour, int *minute, int *second)
{
    struct tm result;

    SYSFUN_gmtime(seconds, &result);

    *year   = result.tm_year;
    *month  = result.tm_mon;
    *day    = result.tm_mday;
    *hour   = result.tm_hour;
    *minute = result.tm_min;
    *second = result.tm_sec;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ConvertSecondsToLocalDateTime
 * ---------------------------------------------------------------------
 * PURPOSE  : Convert seconds to date time with time zone offset
 * INPUT    : seconds -- seconds
 * OUTPUT   : year    -- year
 *            month   -- month
 *            day     -- day
 *            hour    -- hour
 *            minute  -- minute
 *            second  -- second
 * RETURN   : None
 * NOTES    : The same as localtime()
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ConvertSecondsToLocalDateTime(UI32_T seconds, int *year, int *month, int *day,
                                            int *hour, int *minute, int *second)
{
    int offset;

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    else
    {
        /* Get current time zone offset
         */
        SYS_TIME_OM_ENTER_CRITICAL_SECTION();
        offset = sys_time_shmem_data_p->time_zone.offset;
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

        SYS_TIME_ConvertSecondsToDateTime(seconds + offset * 60, year, month, day, hour, minute, second);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ConvertDateTimeToSeconds
 * ---------------------------------------------------------------------
 * PURPOSE  : get real time in seconds based on the input
 *            -- Year,month,day,hour,min,second
 * INPUT    : TOD:Year,month,day,hour,min,second
 * OUTPUT   : Seconds from 01/01/1970 00:00:00 base on input
 * RETURN   : None
 * NOTES    : 1. Symmetry function of  SYS_TIME_ConvertSecondsToDateTime
 * History  : S.K.Yang     06/10/2002      new added
 *            haiqiang.li  11/01/2008      base on 01/01/1970 00:00:00
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ConvertDateTimeToSeconds(int year, int month, int day, int hour,
                                       int minute, int second, UI32_T *seconds)
{
    SYSFUN_mktime(year, month, day, hour, minute, second, seconds);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ValidateTime
 * ---------------------------------------------------------------------
 * PURPOSE  : Validates time
 * INPUT    : year, month, day, hour, minute, second
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_TIME_ValidateTime(int year, int month, int day, int hour,
                             int minute, int second)
{
    if (year < SYS_TIME_MIN_CONFIGURABLE_YEAR
        || year > SYS_TIME_MAX_CONFIGURABLE_YEAR
        || month > 12 || month < 1 || hour > 23 || hour < 0 || minute > 59
        || minute < 0 || second > 59 || second < 0 )
    {
        return FALSE;
    }

    if ((month == 2) && (day > 29))
    {
        return FALSE;
    }

    if (((month == 4) || (month == 6) || (month == 9) || (month == 11)) &&
        (day > 30))
    {
        return FALSE;
    }

    if ((TRUE != is_leap_year(year)) && (month == 2) && (day > 28))
    {
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_SetRealTimeClock
 * ---------------------------------------------------------------------
 * PURPOSE: set real TOD(Time-Of-Day)
 * INPUT    : year, month, day, hour, minute, second
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : 1. This API is the only entrance used to set up local time.
 *            2. Set to RTC  if it exists, and set it to Software clock,
 *               so h/w clock is synchronous with s/w clock.
 *            3. When set time to software/hardware clock, this routine will trans-
 *               late local time to UTC format.
 * History  :  S.K.Yang     06/7/2002      modified
 *             haiqiang.li  11/01/2008     modified
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_TIME_SetRealTimeClock(int year, int month, int day,
                                 int hour, int minute, int second)
{
    int    offset = 0;  /* time zone offset base on UTC */
    UI32_T seconds;     /* Total seconds from 2001/01/01 00:00:00 to your setting time */
    UI32_T utc_seconds; /* UTC time relative to your setting time in seconds */

    if (FALSE == SYS_TIME_ValidateTime(year, month, day, hour, minute, second))
    {
        return FALSE;
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;
    else
    {
        /* Convert tod to second format
         */
        SYS_TIME_ConvertDateTimeToSeconds(year, month, day, hour, minute, second, &seconds);

        SYS_TIME_OM_ENTER_CRITICAL_SECTION();
        offset = sys_time_shmem_data_p->time_zone.offset * 60;
        utc_seconds = seconds - offset;
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

#if (SYS_HWCFG_SUPPORT_RTC == TRUE)
        /* Convert seconds to TOD
         */
        SYS_TIME_ConvertSecondsToDateTime(utc_seconds, &year, &month, &day, &hour, &minute, &second);

        /* Set TOD to hardware , now, it is UTC
         */
        SYSDRV_RTC_SetDateTime(year, month, day, hour, minute, second);
#endif

        /* sync kernel time
         */
        SYSFUN_SetSysClock(utc_seconds);

        /* sync software clock
         */
        SYS_TIME_OM_ENTER_CRITICAL_SECTION();
        sys_time_shmem_data_p->software_clock.time = utc_seconds;
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

#if (SYS_CPNT_STACKING == TRUE)
        if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p)
            == SYS_TYPE_STACKING_MASTER_MODE)
        {
            SYS_TIME_RemoteSyncDateTime();
        }
#endif

        return TRUE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_SetRealTimeClockByStr
 * ---------------------------------------------------------------------
 * PURPOSE: set real time clock by string
 * INPUT    : String like below :
 *                   Date-time string :   J|a|n| | |3| |1|5|:| 1| 4| :| 1| 3|  | 1| 9| 8| 8|\0|
 *               Character Position :     0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    :
 *
 * ---------------------------------------------------------------------
 */
BOOL_T SYS_TIME_SetRealTimeClockByStr(const char *clock_time_p)
{
    int year, day, hour, minute, second;
    UI8_T month_index,match=0;
    char month_str[4];

    if (clock_time_p != NULL)
    {
        /* Check the format of the colok_time
         * MMM  D HH:mm:SS YYYY
         * 012345
         */
        if ((strlen(clock_time_p) < 5)
            || (SYS_TIME_StrIsDigitEx(&clock_time_p[5], " :") != TRUE)
            || (sscanf(clock_time_p, "%3s  %d %d:%d:%d %d", month_str, &day, &hour, &minute, &second, &year) != 6))
        {
            return FALSE;
        }

        /* Check the string of month
         */
        for (month_index = 0; month_index < 12; month_index++)
        {
            if (strcmp(month_name_ar[month_index], month_str) == 0)
            {
                match = 1;
                break;
            }
        }

        if (0 == match)
            return FALSE;

        if (SYS_TIME_SetRealTimeClock(year, month_index + 1,
                day, hour, minute, second) != TRUE)
            return FALSE;

#if (SYS_CPNT_STACKING == TRUE)
        if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p)
            == SYS_TYPE_STACKING_MASTER_MODE)
        {
            SYS_TIME_RemoteSyncDateTime();
        }
#endif

        return TRUE;
    }
    else
        return FALSE;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetSystemUpTime
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used by CLI and WEB to tell how many days, hours,
 *            minutes, seconds and miliseconds this box has run after
 *            system running.
 * INPUT    : None
 * OUTPUT   : days, hours, minutes, seconds, miliseconos
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetSystemUpTime(UI32_T   *days,
                              UI32_T   *hours,
                              UI32_T   *minutes,
                              UI32_T   *seconds,
                              UI32_T   *miliseconds)
{
    UI32_T  ticks;
    UI32_T  tmp;

    SYS_TIME_GetSystemUpTimeByTick(&ticks);

    *miliseconds = ticks % SYS_BLD_TICKS_PER_SECOND;
    tmp = ticks / SYS_BLD_TICKS_PER_SECOND;      /* in seconds   */

    *seconds = tmp % 60;
    tmp = tmp / 60;         /* in minutes   */

    *minutes = tmp % 60;
    tmp = tmp / 60;         /* in hours     */

    *hours = tmp % 24;
    *days = tmp / 24;       /* in days      */
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetSystemUpTimeByTick_Internal
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used to get system up time by tick.
 * INPUT    : take_sem - TRUE : will take SYS_TIME semaphore of the critical section in this function
 *                       FALSE: do not take SYS_TIME semaphore of the critical section in this function
 * OUTPUT   : ticks    - system up time in tick
 * RETURN   : None
 * NOTES    : 1. This function should only called within the SYS_TIME and should
 *               not call by other CSC directly.
 *            2. This function always output a valid sytem up time tick enter
 *               provision complete.
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetSystemUpTimeByTick_Internal(BOOL_T take_sem, UI32_T *ticks)
{
    if (take_sem==TRUE)
    {
        SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    }

    if (FALSE == sys_time_shmem_data_p->is_provision_complete)
    {
        *ticks = 0;
        goto exit;
    }

    *ticks = SYS_TIME_GetSystemTicksBy10ms() - sys_time_shmem_data_p->provision_complete_ticks;

exit:
    if (take_sem==TRUE)
    {
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
    }

    return;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetSystemUpTimeByTick
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used to get system up time by tick.
 * INPUT    : None
 * OUTPUT   : ticks - system up time in tick
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetSystemUpTimeByTick(UI32_T *ticks)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p)!= SYS_TYPE_STACKING_MASTER_MODE)
    {
        *ticks = 0;
        return;
    }

    SYS_TIME_GetSystemUpTimeByTick_Internal(TRUE, ticks);
    return;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetSystemTicksBy10ms
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used for SNMP or those functions which needs to
 *            get system tick count.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : The most tickSet() value, plus all tickAnnounce() calls since.
 * NOTES    : VxWorks kernel also supports tickGet() to get systemUpTicks,
 *            but for maintainance and generic purpose.  Our core layer
 *            and application layer had better use this API instead of
 *            call tickGet().
 * ---------------------------------------------------------------------
 */
UI32_T SYS_TIME_GetSystemTicksBy10ms(void)
{
    return (SYSFUN_GetSysTick());
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_SetSystemTicksBy10ms
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used for SNMP or those functions which needs to
 *            set system tick count.
 * INPUT    : ticks -- new time in ticks
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    : VxWorks kernel also supports tickSet() to set systemUpTicks,
 *            but for maintainance and generic purpose.  Our core layer
 *            and application layer had better use this API instead of
 *            call tickSet().
 * ---------------------------------------------------------------------
 */
void SYS_TIME_SetSystemTicksBy10ms(UI32_T ticks)
{

}

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_SetKick
 * ---------------------------------------------------------------------
 * PURPOSE  : This function set kick on/off.
 * INPUT    : setting -- turn on/off kick
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    : 1.For debug issue, we need support this function.
 *            2.We need turn on kick after enable watch dog timer.
 * ---------------------------------------------------------------------
 */
void SYS_TIME_SetKick(BOOL_T setting)
{
    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    sys_time_shmem_data_p->wd_kick = setting;
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
} /* End of SYS_TIME_SetKick */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetKickCounter
 * ---------------------------------------------------------------------
 * PURPOSE  : This function get kick counter.
 * INPUT    : None.
 * OUTPUT   : counter - kick number.
 * RETURN   : None.
 * NOTES    : 1.For debug issue, we need support this function.
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetKickCounter(UI32_T *tcounter)
{
    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    *tcounter = sys_time_shmem_data_p->wd_kick_counter;
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
} /* End of SYS_TIME_GetKickCounter */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ClearWatchdogIntStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : This function clear watchdog int status.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    :
 * EPR00057114,sunnyt 2005/07/29
 * in 8248, watchdog timeout is trigger by irq5.
 * We need to clear watchdog irq status
 * So, need to add one function in sys_time to clear irq status
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ClearWatchdogIntStatus()
{
    //UI8_T status;
    //status = WDT_ADDR[0];
    return;
} /* End of SYS_TIME_ClearWatchdogIntStatus */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_EnableWatchDogTimer
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used to enable watch dog timer.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    : This function is hard ware dependent.  If the agent board
 *            H/W watch dog timer design changes, the function might change
 *            as well.
 * ---------------------------------------------------------------------
 */
void SYS_TIME_EnableWatchDogTimer(void)
{
#if (SYS_CPNT_WATCHDOG_TIMER==TRUE)
#if (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_ECS4910_28F)
    UI8_T	data=0;

    /*
     * DIS_WDT should be set
     * and need kick off watchdog timer (high to low transition
     * of WDT_CLK)
     */
    if(FALSE==PHYSICAL_ADDR_ACCESS_Read(WATCHDOG_ADDR, 1, 1, &data))
    {
        printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Read fail \n", __FILE__ );
        return;
    }
    data &= ~(WATCHDOG_ENABLE_MASK | WATCHDOG_KICK_MASK);
    data |= (WATCHDOG_ENABLE | WATCHDOG_KICK_H);
    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(WATCHDOG_ADDR, 1, 1, &data))
    {
        printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Write fail \n", __FILE__ );
        return;
    }

    if(FALSE==PHYSICAL_ADDR_ACCESS_Read(WATCHDOG_ADDR, 1, 1, &data))
    {
        printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Read fail \n", __FILE__ );
        return;
    }
    data &= ~(WATCHDOG_ENABLE_MASK | WATCHDOG_KICK_MASK);
    data |= (WATCHDOG_ENABLE | WATCHDOG_KICK_L);
    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(WATCHDOG_ADDR, 1, 1, &data))
    {
        printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Write fail \n", __FILE__ );
        return;
    }
#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_MARVELL_PONCAT)
    UI32_T data;

    /* RSTOUT n Mask Register (Offset: 0x00020108)
     * Bit 1: WDRstOutEn: If set to 1, the device asserts RSTOUTn upon watchdog
     *                    timer expiration.
     * Set Bit 1 as 1 to enable watchdog timeout reset.
     */
    if(FALSE==PHYSICAL_ADDR_ACCESS_Read(MARVELL_PONCAT_RSTOUTN_MASK_REGISTER, 4, 1, (UI8_T*)&data))
    {
        printf("\r\n %s(%d):PHYSICAL_ADDR_ACCESS_Read fail\r\n", __FUNCTION__, __LINE__);
        return;
    }

    data |= BIT_1;

    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(MARVELL_PONCAT_RSTOUTN_MASK_REGISTER, 4, 1, (UI8_T*)&data))
    {
        printf("\r\n %s(%d):PHYSICAL_ADDR_ACCESS_Read fail\r\n", __FUNCTION__, __LINE__);
        return;
    }

    /* CPU Watchdog Timer Register (Offset: 0x00020324)
     * Bit 31-0: CPU Watchdog Timer
     *           This 32-bit counter is decremented every internal clock cycle
     *           of the device. When field <CPUWDTimerEn> in the CPU Timers Control
     *           Register = 0, <CPU Watchdog Timer> stops.
     *           When <CPUWDTimerEn> = 1 and <CPUWDTimerAuto> in the CPU Timer
     *           Control Register = 0, <CPU Watchdog Timer> is decremented until
     *           it reaches 0, then it stops.
     *           When <CPUWDTimerEn> = 1 and <CPUWDTimerAuto> = 1, <CPU Watchdog Timer>
     *           is decremented until it reaches to 0, then the field <CPUWDTimerReload>
     *           in the CPU Watchdog Timer Reload Register is reloaded to <CPU Watchdog Timer>
     *           , and then <CPU Watchdog Timer> continues to count.
     *
     * Set Bit 31-0 as SYS_HWCFG_WATCHDOG_RELOAD_VAL
     */
    data=SYS_HWCFG_WATCHDOG_RELOAD_VAL;
    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(MARVELL_PONCAT_CPU_WATCHDOG_TIMER_REGISTER, 4, 1, (UI8_T*)&data))
    {
        printf("\r\n %s(%d):PHYSICAL_ADDR_ACCESS_Read fail\r\n", __FUNCTION__, __LINE__);
        return;
    }


    /* CPU Timers Control Register (Offset: 0x00020300)
     * Bit 4: CPUWDTimerEn: CPU Watchdog Timer Enable
     *                      0 = WdDisabled
     *                      1 = WdEnabled
     * Set Bit 4 as 1 to enable watchdog
     */
    if(FALSE==PHYSICAL_ADDR_ACCESS_Read(MARVELL_PONCAT_CPU_TIMERS_CONTROL_REGISTER, 4, 1, (UI8_T*)&data))
    {
        printf("\r\n %s(%d):PHYSICAL_ADDR_ACCESS_Read fail\r\n", __FUNCTION__, __LINE__);
        return;
    }

    data |= BIT_4;

    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(MARVELL_PONCAT_CPU_TIMERS_CONTROL_REGISTER, 4, 1, (UI8_T*)&data))
    {
        printf("\r\n %s(%d):PHYSICAL_ADDR_ACCESS_Read fail\r\n", __FUNCTION__, __LINE__);
        return;
    }

#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_OS6450)
    UI8_T  data, i2c_bus_idx=0;
    BOOL_T result;

#if defined(ES4627MB)
    if(sys_time_shmem_data_p->board_id==2 || sys_time_shmem_data_p->board_id==6)
        i2c_bus_idx=1;
#endif

    /* set Watchdog Timer Register
     */
    data = SYS_HWCFG_WATCHDOG_RELOAD_VAL;
    result=I2CDRV_TwsiDataWriteWithBusIdx(i2c_bus_idx, CPLD_I2C_ADDR, I2C_7BIT_ACCESS_MODE, TRUE, CPLD_WATCHDOG_COUNTER_REG_ADDR,
        FALSE, &data, 1);
    if(result==FALSE)
    {
        printf("\r\n %s(%d):I2CDRV_TwsiDataWrite fail\r\n", __FUNCTION__, __LINE__);
        return;
    }

    /* set Watchdog Timer Control Register
     */
    data = WDT_RST_EN | (SYS_HWCFG_WATCHDOG_CONTROL_FREQ_SEL & SYS_HWCFG_WATCHDOG_CONTROL_FREQ_SEL_MASK);
    result=I2CDRV_TwsiDataWriteWithBusIdx(i2c_bus_idx, CPLD_I2C_ADDR, I2C_7BIT_ACCESS_MODE, TRUE, CPLD_WATCHDOG_CONTROL_REG_ADDR,
        FALSE, &data, 1);
    if(result==FALSE)
    {
        printf("\r\n %s(%d):I2CDRV_TwsiDataWrite fail\r\n", __FUNCTION__, __LINE__);
        return;
    }

#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_ECS5610_52S)
    UI8_T	data;

    data = WATCHDOG_ENABLE | WATCHDOG_CNT;

    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(WATCHDOG_ADDR, 1, 1, &data))
    {
        printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Write fail \n", __FILE__ );
        return;
    }

#else
#error "SYS_HWCFG_WATCHDOG_TYPE is not properly defined"
#endif /* end of #if (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_MPC8248) */

    sys_time_shmem_data_p->wd_enable = TRUE;
    SYS_TIME_SetKick(TRUE);
#endif /* end of #if (SYS_CPNT_WATCHDOG_TIMER==TRUE) */

    return;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_DisableWatchDogTimer
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used to disable watch dog timer.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    : This function is hard ware dependent.  If the agent board
 *            H/W watch dog timer design changes, the function might change
 *            as well.
 * ---------------------------------------------------------------------
 */
void SYS_TIME_DisableWatchDogTimer(void)
{
#if (SYS_CPNT_WATCHDOG_TIMER==TRUE)
#if (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_ECS4910_28F)
    UI8_T	data=0;

    if(FALSE==PHYSICAL_ADDR_ACCESS_Read(WATCHDOG_ADDR, 1, 1, &data))
    {
        printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Read fail \n", __FILE__ );
        return;
    }

    data &= ~(WATCHDOG_ENABLE_MASK | WATCHDOG_KICK_MASK);
    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(WATCHDOG_ADDR, 1, 1, &data))
    {
        printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Write fail \n", __FILE__ );
        return;
    }

#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_MARVELL_PONCAT)
    UI32_T data;

    /* CPU Timers Control Register (Offset: 0x00020300)
     * Bit 4: CPUWDTimerEn: CPU Watchdog Timer Enable
     *                      0 = WdDisabled
     *                      1 = WdEnabled
     * Set Bit 4 as 0 to disable watchdog
     */
    if(FALSE==PHYSICAL_ADDR_ACCESS_Read(MARVELL_PONCAT_CPU_TIMERS_CONTROL_REGISTER, 4, 1, (UI8_T*)&data))
    {
        printf("\r\n %s(%d):PHYSICAL_ADDR_ACCESS_Read fail\r\n", __FUNCTION__, __LINE__);
        return;
    }

    data &= ~(BIT_4);

    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(MARVELL_PONCAT_CPU_TIMERS_CONTROL_REGISTER, 4, 1, (UI8_T*)&data))
    {
        printf("\r\n %s(%d):PHYSICAL_ADDR_ACCESS_Read fail\r\n", __FUNCTION__, __LINE__);
        return;
    }
#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_OS6450)
    UI8_T  data, i2c_bus_idx=0;
    BOOL_T result;

#if defined(ES4627MB)
    if(sys_time_shmem_data_p->board_id==2 || sys_time_shmem_data_p->board_id==6)
        i2c_bus_idx=1;
#endif

    /* Read Watchdog Timer Control Register
     */
    result=I2CDRV_TwsiDataReadWithBusIdx(i2c_bus_idx, CPLD_I2C_ADDR, I2C_7BIT_ACCESS_MODE, TRUE, CPLD_WATCHDOG_CONTROL_REG_ADDR,
        FALSE, 1, &data);
    if(result==FALSE)
    {
        printf("\r\n %s(%d):I2CDRV_TwsiDataRead fail\r\n", __FUNCTION__, __LINE__);
        return;
    }

    /* Turn off WDT_RST_EN bit
     */
    data &= (~WDT_RST_EN);

    /* Write Watchdog Timer Control Register
     */
    result=I2CDRV_TwsiDataWriteWithBusIdx(i2c_bus_idx, CPLD_I2C_ADDR, I2C_7BIT_ACCESS_MODE, TRUE, CPLD_WATCHDOG_CONTROL_REG_ADDR,
        FALSE, &data, 1);
    if(result==FALSE)
    {
        printf("\r\n %s(%d):I2CDRV_TwsiDataWrite fail\r\n", __FUNCTION__, __LINE__);
        return;
    }

#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_ECS5610_52S)
    UI8_T	data;

    data = (~WATCHDOG_ENABLE & WATCHDOG_ENABLE_MASK) | WATCHDOG_CNT | WATCHDOG_KICK;

    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(WATCHDOG_ADDR, 1, 1, &data))
    {
        printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Write fail \n", __FILE__ );
        return;
    }

#else
#error "SYS_HWCFG_WATCHDOG_TYPE is not properly defined"
#endif /* end of #if (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_MPC8248) */

    sys_time_shmem_data_p->wd_enable = FALSE;
    return;

#endif /* end of #if (SYS_CPNT_WATCHDOG_TIMER==TRUE) */
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_KickWatchDogTimer
 * ---------------------------------------------------------------------
 * PURPOSE  : This is used to kick watch dog timer to prevent that
 *            watch dog timer time out.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None.
 * NOTES    : This function is hard ware dependent.  If the agent board
 *            H/W watch dog timer design changes, the function might change
 *            as well.
 * ---------------------------------------------------------------------
 */
void SYS_TIME_KickWatchDogTimer(void)
{
#if (SYS_CPNT_WATCHDOG_TIMER==TRUE)
    if(sys_time_shmem_data_p->wd_kick == TRUE)
    {
        if(sys_time_shmem_data_p->wd_enable == TRUE)
        {
#if (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_ECS4910_28F)
            UI8_T	data;

            if(FALSE==PHYSICAL_ADDR_ACCESS_Read(WATCHDOG_ADDR, 1, 1, &data))
            {
                printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Read fail \n", __FILE__ );
                return;
            }
            data &= ~(WATCHDOG_ENABLE_MASK | WATCHDOG_KICK_MASK);
            data |= (WATCHDOG_ENABLE | WATCHDOG_KICK_H);
            if(FALSE==PHYSICAL_ADDR_ACCESS_Write(WATCHDOG_ADDR, 1, 1, &data))
            {
                printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Write fail \n", __FILE__ );
                return;
            }

            if(FALSE==PHYSICAL_ADDR_ACCESS_Read(WATCHDOG_ADDR, 1, 1, &data))
            {
                printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Read fail \n", __FILE__ );
                return;
            }
            data &= ~(WATCHDOG_ENABLE_MASK | WATCHDOG_KICK_MASK);
            data |= (WATCHDOG_ENABLE | WATCHDOG_KICK_L);
            if(FALSE==PHYSICAL_ADDR_ACCESS_Write(WATCHDOG_ADDR, 1, 1, &data))
            {
                printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Write fail \n", __FILE__ );
                return;
            }

#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_MARVELL_PONCAT)
            UI32_T data;

            /* CPU Watchdog Timer Register (Offset: 0x00020324)
             * Bit 31-0: CPU Watchdog Timer
             * Set Bit 31-0 as SYS_HWCFG_WATCHDOG_RELOAD_VAL
             */
            data=SYS_HWCFG_WATCHDOG_RELOAD_VAL;
            if(FALSE==PHYSICAL_ADDR_ACCESS_Write(MARVELL_PONCAT_CPU_WATCHDOG_TIMER_REGISTER, 4, 1, (UI8_T*)&data))
            {
                printf("\r\n %s(%d):PHYSICAL_ADDR_ACCESS_Read fail\r\n", __FUNCTION__, __LINE__);
                return;
            }
#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_OS6450)
            UI8_T  data, i2c_bus_idx=0;
            BOOL_T result;

#if defined(ES4627MB)
            if(sys_time_shmem_data_p->board_id==2 || sys_time_shmem_data_p->board_id== 6)
                i2c_bus_idx=1;
#endif

            data = SYS_HWCFG_WATCHDOG_RELOAD_VAL;
            result=I2CDRV_TwsiDataWriteWithBusIdx(i2c_bus_idx, CPLD_I2C_ADDR, I2C_7BIT_ACCESS_MODE, TRUE, CPLD_WATCHDOG_COUNTER_REG_ADDR,
                FALSE, &data, 1);
            if(result==FALSE)
            {
                printf("\r\n %s(%d):I2CDRV_TwsiDataWrite fail\r\n", __FUNCTION__, __LINE__);
                return;
            }

#elif (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_ECS5610_52S)
    UI8_T	data;

    data = WATCHDOG_KICK | WATCHDOG_ENABLE | WATCHDOG_CNT;

    if(FALSE==PHYSICAL_ADDR_ACCESS_Write(WATCHDOG_ADDR, 1, 1, &data))
    {
        printf("\n file=%s, PHYSICAL_ADDR_ACCESS_Write fail \n", __FILE__ );
        return;
    }

#else
#error "SYS_HWCFG_WATCHDOG_TYPE is not properly defined"
#endif /* end of #if (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_MPC8248) */
            sys_time_shmem_data_p->wd_kick_counter++;
        }
    }

#endif /* end of #if (SYS_CPNT_WATCHDOG_TIMER==TRUE) */
   return;
}
#else
void SYS_TIME_EnableWatchDogTimer(void)
{
    return;
}
void SYS_TIME_DisableWatchDogTimer(void)
{
    return;
}
void SYS_TIME_KickWatchDogTimer(void)
{
    return;
}
#endif /* SYS_CPNT_WATCHDOG_TIMER */

/* -----------------------------------------------------------
 * Time information Process Routine, added for software clock
 * Add by S.K.Yang 2002/06/07 1:00 pm
 *-----------------------------------------------------------*/
/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_GetTimeZone
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time zone and name of time zone
 * INPUT    : A pointer point to buffer
 * OUTPUT   : timezone - timezone data.
 * RETURN   : TRUE/FALSE
 * NOTES    :
 *------------------------------------------------------------------------------
 */
BOOL_T
SYS_TIME_GetTimeZone(
    SYS_TIME_Timezone_T *timezone)
{
    BOOL_T ret;

    if ((SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
       || (NULL == timezone))
    {
        return FALSE;
    }

    ret = SYS_TIME_GetTimeZoneInfo(timezone);

    return ret;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_GetTimeZoneByStr
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time zone by string
 * INPUT    : A pointer point to buffer
 * OUTPUT   : 1. time offset relative to GMT e.q, "+08:00"  7 characters (include '\0')
 * RETURN   : TRUE/FALSE
 * NOTES    :
 *------------------------------------------------------------------------------
 */
BOOL_T SYS_TIME_GetTimeZoneByStr(char *time_p)
{
    int hour,min,abs_min,tmp;
    char hour_str[3], min_str[3];

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    else
    {
        SYS_TIME_OM_ENTER_CRITICAL_SECTION();
        tmp = sys_time_shmem_data_p->time_zone.offset;

        if (time_p == NULL)
        {
            //EH_MGR_Handle_Exception(SYS_MODULE_SYSMGMT, SYS_TIME_GetTimeZoneByStr_Fun_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG));
            SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }
        else if (tmp == 0) /* UTC time */
        {
            sprintf(time_p, "00:00");
            time_p[6] = '\0';
        }
        else
        {
            if ( tmp > 0)
            {
                time_p[0] = '+';
            }
            else
            {
                time_p[0] = '-';
            }
            abs_min = abs(tmp);
            hour = abs_min / 60;
            min = abs_min % 60;

            sprintf(hour_str,"%d",hour);
            if (hour < 10)
            {
                time_p[1] = '0';
                time_p[2] = hour_str[0];
            }
            else
            {
                time_p[1] = hour_str[0];
                time_p[2] = hour_str[1];
            }

            time_p[3] = ':';

            sprintf(min_str,"%d",min);
            if (min < 10)
            {
                time_p[4] = '0';
                time_p[5] = min_str[0];
            }
            else
            {
                time_p[4] = min_str[0];
                time_p[5] = min_str[1];
            }
            time_p[6] = '\0';
        }

        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }
}

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_SetTimeZoneByStr
 *------------------------------------------------------------------------------
 * PURPOSE  : Set time zone by string
 * INPUT    : time offset relative to GMT e.q, "+08:00"  7 characters (include '\0')
 * OUTPUT   :
 * RETURN   : TRUE/FALSE
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_SetTimeZoneByStr(const char *time_p)
{
    int hour, min, offset;

    if (time_p != NULL)
    {
        if (L_STDLIB_StrIsDigit((char *)time_p) == TRUE)
        {
            if (strcmp(time_p, "00:00")!= 0)
            {
                return FALSE;
            }
            else
            {
                SYS_TIME_OM_ENTER_CRITICAL_SECTION();
                sys_time_shmem_data_p->time_zone.offset = 0; /*UTC time */
                SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
            }
        }
        else
        {
            if ((SYS_TIME_StrIsDigitEx(&time_p[1], ":") != TRUE)
                || (sscanf(&time_p[1],"%d:%d",&hour,&min) != 2)
                || (time_p[0] != '+' && time_p[0] != '-'))
            {
                return FALSE;
            }

            if ((min < SYS_TIME_MIN_TIMEZONE_MINUTE) ||
                (SYS_TIME_MAX_TIMEZONE_MINUTE < min) ||
                (hour < SYS_TIME_MIN_TIMEZONE_HOUR) ||
                (SYS_TIME_MAX_TIMEZONE_HOUR < hour))
            {
                return FALSE;
            }

            offset = (time_p[0] == '+') ? (hour * 60 + min) : -(hour * 60 + min) ;

            if (offset > SYS_TIME_MAX_TIMEZONE_IN_MINUTES || offset < SYS_TIME_MIN_TIMEZONE_IN_MINUTES)
            {
                return FALSE;
            }

            SYS_TIME_OM_ENTER_CRITICAL_SECTION();
            sys_time_shmem_data_p->time_zone.offset = offset;
            SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        }
#if (SYS_CPNT_STACKING == TRUE)
        if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p)
            == SYS_TYPE_STACKING_MASTER_MODE)
        {
            SYS_TIME_RemoteSyncDateTime();
        }
#endif
    }
    else
        return FALSE;

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_SetTimeZone
 *------------------------------------------------------------------------------
 * PURPOSE  : Set time zone and name of time zone
 * INPUT    : zone_name_p - Time zone name
 *            sign       - Plus or minus.
 *                         SYS_TIME_TIMEZONE_PLUS: UTC plus the time
 *                         SYS_TIME_TIMEZONE_MINUS: UTC minus the time
 *            hour       - Time zone hours
 *            minute     - Time zone minutes
 * OUTPUT   : none
 * RETURN   : TRUE if success,FALSE if time zone out of range -780 < zone <720 (min)
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_SetTimeZone(const char *zone_name_p, SYS_TIME_TIMEZONE_SIGN_T sign, UI32_T hour, UI32_T minute)
{
    int offset = 0;

    if ((0 == zone_name_p) ||
        (strlen(zone_name_p) < MINSIZE_sysTimeZoneName) ||
        (strlen(zone_name_p) > MAXSIZE_sysTimeZoneName))
    {
        return FALSE;
    }

    if ((minute < SYS_TIME_MIN_TIMEZONE_MINUTE) ||
        (minute > SYS_TIME_MAX_TIMEZONE_MINUTE))
    {
        return FALSE;
    }

    if (SYS_TIME_TIMEZONE_MINUS == sign)
    {
        if (hour > (-SYS_TIME_MIN_TIMEZONE_HOUR))
        {
            return FALSE;
        }

        offset = -(hour*60 + minute);
    }
    else
    {
        if (hour > SYS_TIME_MAX_TIMEZONE_HOUR)
        {
            return FALSE;
        }

        offset = hour*60 + minute;
    }

    if ((offset < SYS_TIME_MIN_TIMEZONE_IN_MINUTES) ||
        (offset > SYS_TIME_MAX_TIMEZONE_IN_MINUTES))
    {
        return FALSE;
    }

    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    strncpy(sys_time_shmem_data_p->time_zone.zone_name,
        zone_name_p, MAXSIZE_sysTimeZoneName);
    sys_time_shmem_data_p->time_zone.zone_name[MAXSIZE_sysTimeZoneName] = '\0';
    sys_time_shmem_data_p->time_zone.offset = offset;
    sys_time_shmem_data_p->time_zone.timezone_offset_id = VAL_sysTimeZonePredefined_none;
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

#if (SYS_CPNT_STACKING == TRUE)
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p)
        == SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYS_TIME_RemoteSyncDateTime();
    }
#endif

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_GetTimeZoneNameByStr
 *------------------------------------------------------------------------------
 * PURPOSE  : Get name of time zone
 * INPUT    : 1.Time zone name
 * OUTPUT   : none
 * RETURN   : TRUE if success,FALSE
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_GetTimeZoneNameByStr(char *name_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    else
    {
        if (name_p != NULL)
        {
            SYS_TIME_OM_ENTER_CRITICAL_SECTION();
            strncpy(name_p, sys_time_shmem_data_p->time_zone.zone_name, MAXSIZE_sysTimeZoneName);
            name_p[MAXSIZE_sysTimeZoneName] = '\0';
            SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        }
        else
        {
            //EH_MGR_Handle_Exception(SYS_MODULE_SYSMGMT, SYS_TIME_GetTimeZoneNameByStr_Fun_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG));
            return FALSE;
        }
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_SetTimeZoneNameByStr
 *------------------------------------------------------------------------------
 * PURPOSE  : Set  name of time zone
 * INPUT    : A pointer point to buffer
 * OUTPUT   : 1. zone_name
 * RETURN   : TRUE/FALSE
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_SetTimeZoneNameByStr(const char *name_p)
{
    if (name_p != NULL)
    {
        SYS_TIME_OM_ENTER_CRITICAL_SECTION();
        strncpy(sys_time_shmem_data_p->time_zone.zone_name,
            name_p, MAXSIZE_sysTimeZoneName);
        sys_time_shmem_data_p->time_zone.zone_name[MAXSIZE_sysTimeZoneName] = '\0';
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

#if (SYS_CPNT_STACKING == TRUE)
        if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p)
            == SYS_TYPE_STACKING_MASTER_MODE)
        {
            SYS_TIME_RemoteSyncDateTime();
        }
#endif

        return TRUE;
    }
    else
        return FALSE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetRunningTimeZone
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the timezone of system
 * INPUT:    None
 * OUTPUT:   In minutes
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T
SYS_TIME_GetRunningTimeZone(
    SYS_TIME_Timezone_T *timezone)
{
    BOOL_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if ((SYS_TIME_DEFAULT_TIMEZONE_OFFSET == sys_time_shmem_data_p->time_zone.offset) &&
        (SYS_TIME_DEFAULT_TIMEZONE_ID == sys_time_shmem_data_p->time_zone.timezone_offset_id) &&
        (strcmp(sys_time_shmem_data_p->time_zone.zone_name, SYS_TIME_DEFAULT_TIMEZONE_NAME) == 0))
        {
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }

    ret = SYS_TIME_GetTimeZoneInfo(timezone);

    return (ret == TRUE)? SYS_TYPE_GET_RUNNING_CFG_SUCCESS : SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetDayAndTime
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the DayAndTime in RFC_2579 for SNMP usage.
 * INPUT:    None.
 * OUTPUT:   day_and_time     - The current local time or the UTC time.
 *           day_and_time_len - The length of the time.
 * RETURN:   TRUE/FALSE
 * NOTE  :   1.Refer to RFC2579
 *             DateAndTime ::= TEXTUAL-CONVENTION
 *              field  octets  contents                  range
 *              -----  ------  --------                  -----
 *                1      1-2   year*                     0..65536
 *                2       3    month                     1..12
 *                3       4    day                       1..31
 *                4       5    hour                      0..23
 *                5       6    minutes                   0..59
 *                6       7    seconds                   0..60
 *                             (use 60 for leap-second)
 *                7       8    deci-seconds              0..9
 *                8       9    direction from UTC        '+' / '-'
 *                9      10    hours from UTC*           0..13
 *               10      11    minutes from UTC          0..59
 *---------------------------------------------------------------------------*/

BOOL_T SYS_TIME_GetDayAndTime(UI8_T *day_and_time, UI32_T *day_and_time_len)
{
    int time_zone_minutes,utc_hour,utc_minute;
    int year,month,day,hour,minute,second;
    UI16_T *ui16_year;
    /* BODY
     */

    if ((day_and_time == NULL) || (day_and_time_len == NULL))
        return FALSE;

    memset(day_and_time, 0, sizeof(UI8_T)*11);
    time_zone_minutes = sys_time_shmem_data_p->time_zone.offset;
    utc_hour = (time_zone_minutes / 60);
    utc_minute = time_zone_minutes - (utc_hour * 60);
    SYS_TIME_GetSoftwareClock(&year,&month,&day,&hour,&minute,&second);

    ui16_year = (UI16_T *)day_and_time;
    *ui16_year = (UI16_T)year;

    day_and_time[2] = month;
    day_and_time[3] = day;
    day_and_time[4] = hour;
    day_and_time[5] = minute;
    day_and_time[6] = second;
    day_and_time[7] = 0;

    if(time_zone_minutes > 0)
    {
        day_and_time[8] = '+';
        day_and_time[9] = utc_hour;
        day_and_time[10] = utc_minute;
        *day_and_time_len = 11;
    }
    else if (time_zone_minutes < 0)
    {
        day_and_time[8] = '-';
        day_and_time[9] = utc_hour;
        day_and_time[10] = utc_minute;
        *day_and_time_len = 11;
    }
    else
    {
        *day_and_time_len = 8;
    }
    return TRUE;
}

/*----------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetSoftwareClock
 *---------------------------------------------------------------------------
 * PURPOSE:  Get software clock
 * INPUT:    Buffer of year, month, day, hour, minute, second
 * OUTPUT:   year,month,day,hour,minute of local time.
 * RETURN:   TRUE/FALSE
 * NOTE:     1. This routine will give you local time by year, month,day,hour,
 *              minute,second."year" may be 2 if 2002 , 3 if 2003, etc,.
 *           2. This routine provide you local time.
 *           3. The software clock may be setted by SNTP(when SNTP is enabled)
 *               or RTC(when it exists, or setted by user)
 *           4. Even RTC or SNTP doesn't exists, software clock always has a base time
 *              1970/01/01 00:00:00
 *           5. It will be drifted away during polling interval. Or if sntp is not available
 *           6. If SNTP doesn't exists and sys_tick is very samll, "before-UTC" will
 *              casue the problem of negative value. So modify beginning time to
 *              2000/12/31 00:00:00
 *---------------------------------------------------------------------------*/
BOOL_T  SYS_TIME_GetSoftwareClock(int *year, int *month, int *day,
                                  int *hour, int *minute, int *second)
{
    int    offset;      /* time zone offset in minutes */
#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
    int    time_offset;
    SYS_TIME_DST_MODE_T mode;
    BOOL_T is_in_daylight_saving_time;
#endif
    UI32_T time_in_sec = 0; /* Current time in Seconds */
    SYSFUN_Time_T os_time;

    SYS_TIME_OM_ENTER_CRITICAL_SECTION();

    /* Get current time zone offset
     */
    offset = sys_time_shmem_data_p->time_zone.offset;
    SYSFUN_GetSysClock(&os_time);
    time_in_sec = os_time;
    time_in_sec += offset * 60;
#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
    mode = sys_time_shmem_data_p->dst_data.mode;
    time_offset = sys_time_shmem_data_p->dst_data.offset;
    is_in_daylight_saving_time = SYS_TIME_DST_ApplyToCurrentTime(&time_in_sec, mode, time_offset);
    sys_time_shmem_data_p->dst_data.is_effect = is_in_daylight_saving_time;
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

    SYS_TIME_ConvertSecondsToDateTime(time_in_sec, year, month, day, hour, minute, second);

    return TRUE;
}

/*----------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetSoftwareClockBySec
 *---------------------------------------------------------------------------
 * PURPOSE:  Get software clock by seconds
 * INPUT:    buffer of seconds
 * OUTPUT:   Local time in seconds from 2001/01/01 00:00:00
 * RETURN:   TRUE/FALSE
 * NOTE:     1. This routine get time in seconds
 *           2. This routine provide you local time.
 *           3. It will be drifted away during polling interval. Or if sntp is not available
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_GetSoftwareClockBySec(UI32_T *sec)
{
    int    offset; /* time zone offset in minutes */
#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
    int    time_offset;
    SYS_TIME_DST_MODE_T mode;
    BOOL_T is_in_daylight_saving_time;
#endif
    SYSFUN_Time_T os_time;

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    else
    {
        SYS_TIME_OM_ENTER_CRITICAL_SECTION();

        /* Get current time zone offset
         */
        offset =sys_time_shmem_data_p->time_zone.offset;
#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
        mode = sys_time_shmem_data_p->dst_data.mode;
        time_offset = sys_time_shmem_data_p->dst_data.offset;
#endif
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        if (sec != NULL) /* Non null buffer */
        {
            /* Protect software clock, and calculate current time use system tick
             */
             SYS_TIME_OM_ENTER_CRITICAL_SECTION();
            /* Get current time zone offset
            */
            offset =sys_time_shmem_data_p->time_zone.offset;
            SYSFUN_GetSysClock(&os_time);
            *sec = os_time;
            *sec += offset * 60;

#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
            mode = sys_time_shmem_data_p->dst_data.mode;
            time_offset = sys_time_shmem_data_p->dst_data.offset;
            is_in_daylight_saving_time = SYS_TIME_DST_ApplyToCurrentTime(sec, mode, time_offset);
            sys_time_shmem_data_p->dst_data.is_effect = is_in_daylight_saving_time;
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)*/
            SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

static BOOL_T SYS_TIME_GetUtcSoftwareClockBySec(UI32_T *sec)
{
    SYSFUN_Time_T os_time;

    /* When user configures the system timer, Master will sync the timer to slave. Therefore, the timer in the slave is
     * the same as the master. The applications can use this API to get the system timer, whatever the caller is
     * Master or Salve.
     * For example, update file to slave device, the file-map-table needs the timer about this file.
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return FALSE;
    }
    else
    {
        if (sec != NULL) /* Non null buffer */
        {
            SYSFUN_GetSysClock(&os_time);
            *sec = os_time;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_ConvertTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Make  time information into format as May 21 10:11:11 2002
 * INPUT    : 1.Time in seconds from 2001/01/01 00:00:00
 *            2.Buffer pointer stored time information, length: SIZE_sysCurrentTime + 1
 * OUTPUT   : "May 21 10:11:11 2002" format
 * RETURN   : TRUE : SUCCESS, FALSE: FAIL.
 * NOTES    : Return is as the form :
 *            Date-time string :      J|a|n| | |3| |1|5|:| 1| 4| :| 1| 3|  | 2| 0| 0| 1|\0|
 *            Character Position :    0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|
 *            so 21 bytes needed
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_ConvertTime(UI32_T time, char *date_time_p)
{
    int     year, month, day, hour, minute, second;

    /* check valididy of output pointer
     */
    if (date_time_p == NULL)
    {
        return FALSE;
    }

    /* call this function to get date_time format from seconds
     */
    SYS_TIME_ConvertSecondsToDateTime(time, &year, &month, &day, &hour, &minute, &second);

    /* not every library have itoa function, so use sprintf to convert int to string
     */
    sprintf(date_time_p, "%s %2d %02d:%02d:%02d %4d",
        month_name_ar[month-1], day, hour, minute, second, year);
    date_time_p[SYS_TIME_DATE_TIME_STR_LEN] = '\0';

    return TRUE;
}



/*-------------------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_SetRealTimeClockBySeconds
 *-------------------------------------------------------------------------------------------
 * PURPOSE  : Get time information from sntp to calculate system up time
 *            when RTC is not available.
 * INPUT    : 1. seconds : seconds from 1970/01/01 00:00:00
 *            2. systick : system tick number when  time packet is received
 * OUTPUT   : none
 * RETURN   : TRUE : SUCCESS, FALSE: FAIL
 *            never get time from network.
 * NOTES    :1. --|--------------------------------|---------> time coordinate
 *                V                                V
 *              system up time                   sntp packet received
 *                                               and get system tick number on this point
 *           2. This routine is called by SNTP_MGR when sntp is enabled
 *              and received time packet.
 *           3. Once this routine is called, it will update software clock and hardware clock.
 *           4. Called by SNTP_MGR
 *-------------------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_SetRealTimeClockBySeconds(UI32_T seconds, UI32_T systick)
{
#if (SYS_HWCFG_SUPPORT_RTC == TRUE)
    int year, month, day, hour, minute, second;
#endif /* #if (SYS_HWCFG_SUPPORT_RTC == TRUE) */

    //if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    //{
    //    return FALSE;
    //}
    //else
    //{
    /* Update hardware clock
     */
#if (SYS_HWCFG_SUPPORT_RTC == TRUE)
        SYS_TIME_ConvertSecondsToDateTime(seconds, &year, &month, &day, &hour, &minute, &second);
        /* Set TOD to hardware , now, it is UTC
         */
        SYSDRV_RTC_SetDateTime(year, month, day, hour, minute, second);
#endif

        /* sync software clock
         */
        //SYS_TIME_ConvertDateTimeToSeconds(year, month, day, hour, minute, second, (UI32_T *)&seconds);

        SYS_TIME_OM_ENTER_CRITICAL_SECTION();
        sys_time_shmem_data_p->software_clock.time = seconds;
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

        /* sync kernel time
         */
        SYSFUN_SetSysClock(seconds);

#if (SYS_CPNT_STACKING == TRUE)
        if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p)
            == SYS_TYPE_STACKING_MASTER_MODE)
        {
            SYS_TIME_RemoteSyncDateTime();
        }
#endif

        return TRUE;
    //}
}

/*-------------------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_StrIsDigitEx
 *-------------------------------------------------------------------------------------------
 * PURPOSE: Return true if str_p is a particular representation of a decimal
 *          character string ('0'-'9') exception same specific characters.
 * INPUT:   str_p       - pointer of input string (ASCIZ format)
 *          execpt_ar_p - pointer of the specific characters
 * OUTPUT:  none
 * RETURN:  return TRUE if str_p is a decimal character string
 *-------------------------------------------------------------------------------------------*/
static BOOL_T SYS_TIME_StrIsDigitEx(const char *str_p, const char *except_ar_p)
{
    char c;

    for (; (c = *str_p) != 0; str_p++)
    {
        if (strchr(except_ar_p, c) != NULL)
            continue;

        if (L_STDLIB_IsDigit(c) == FALSE)
            return FALSE;
    }

    return TRUE;
}

#define SYSTIME_BACKDOOR_Printf(fmt_p, ...) BACKDOOR_MGR_Printf((fmt_p), ##__VA_ARGS__)

void SYS_TIME_Main (void)
{
#define MAXLINE 255
    UI8_T select_value;

    while(1)
    {
        SYSTIME_BACKDOOR_Printf("\r\nPress <enter> to continue.");
        SYSTIME_BACKDOOR_Printf("\r\n===========================================");
        SYSTIME_BACKDOOR_Printf("\r\n  SYSTIME Engineer Menu 2008/04/22  ");
        SYSTIME_BACKDOOR_Printf("\r\n===========================================");

        SYSTIME_BACKDOOR_Printf("\r\n 1 Get Real time by sec test.");
        SYSTIME_BACKDOOR_Printf("\r\n 2 Software clock test.");
        SYSTIME_BACKDOOR_Printf("\r\n 3 Hardware clock test.");

        SYSTIME_BACKDOOR_Printf("\r\n 0 Exit Stack Topology Engineer Menu!!");
        SYSTIME_BACKDOOR_Printf("\r\n Enter Selection: ");

        select_value = BACKDOOR_MGR_GetChar();
        if(select_value < '0' || select_value > '7')
            continue;

        select_value -= '0';
        SYSTIME_BACKDOOR_Printf("%d", select_value);

        switch(select_value)
        {
            case 1:
            {
                UI32_T seconds;
                char UTC[22]={0};
                SYS_TIME_GetRealTimeBySec(&seconds);
                SYS_TIME_ConvertTime(seconds, UTC);

                SYSTIME_BACKDOOR_Printf(" \r\n Get real time by sec is %s.\n\r", UTC);
            }
            break;

            case 2:
            {
                int year, month, day, hour, minute, second;

                SYS_TIME_GetSoftwareClock(&year, &month, &day, &hour, &minute, &second);

                SYSTIME_BACKDOOR_Printf(" \r\n Get software time is %d.%d.%d %d:%d:%d .\n\r", year, month, day, hour, minute, second);
            }
            break;

            case 3:
            {
                int year, month, day, hour, minute, second;


                SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);

                SYSTIME_BACKDOOR_Printf(" \r\n Get software time is %d.%d.%d %d:%d:%d .\n\r", year, month, day, hour, minute, second);
            }
            break;

            case 0:
                SYSTIME_BACKDOOR_Printf("\r\n Exit Stack Topology Engineer Menu");
                return;
        }
    }

} /* End of SYS_BACKDOOR_Main */

#if (SYS_CPNT_STACKING == TRUE)
static UI16_T SYS_TIME_OM_GetControlSeqNo(void)
{
    return sys_time_shmem_data_p->seq_no;
}

static void SYS_TIME_OM_AddControlSeqNo(void)
{
    sys_time_shmem_data_p->seq_no++;
    return;
}

static UI32_T SyncSlaveSysTime(UI8_T drv_unit, SYS_TIME_RemoteSyncDateTime_T *prsdt)
{
    SYS_TIME_Request_Packet_T *req_packet_p;
    SYS_TIME_Response_Packet_T resp_packet;
    L_MM_Mref_Handle_T *mref_handle_p;
    UI32_T pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYS_TIME_Request_Packet_T),
        L_MM_USER_ID(SYS_MODULE_SYS_TIME, SYS_TIME_POOL_ID_ISC_SEND,
                     SYS_TIME_SYNC_DATE_TIME_REQUEST));
    req_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if (req_packet_p == NULL)
    {
        ERRMSG("%s L_MM_Mref_GetPdu return NULL\n", __FUNCTION__);
        return SYS_TIME_RETURN_ERROR;
    }

    /* fill header with appropriate data
     */
    req_packet_p->header.opcode    = SYS_TIME_SYNC_DATE_TIME_REQUEST;
    req_packet_p->header.data_size = 0;
    req_packet_p->next             = 0;
    req_packet_p->header.seq_no    = SYS_TIME_OM_GetControlSeqNo();
    memcpy(&req_packet_p->rsdt, prsdt, sizeof(SYS_TIME_RemoteSyncDateTime_T));

    if (ISC_RemoteCall((UI8_T)drv_unit, ISC_SYS_TIME, mref_handle_p,
                        SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                        sizeof(resp_packet), (UI8_T *)&resp_packet,
                        SYS_TIME_ISC_TRY_COUNT, SYS_TIME_ISC_TIMEOUT_VALUE))
    {
        SYS_TIME_OM_AddControlSeqNo();

        /* ACK received
         */
        if (resp_packet.header.opcode == SYS_TIME_ACK)
        {
            //ERRMSG("%s SYS_TIME_ACK\n", __FUNCTION__);

            /* FIXME: master maybe should verify the systime returned from slaves
             */
            return SYS_TIME_RETURN_OK;
        }
        else {
            ERRMSG("%s bad opcode\n", __FUNCTION__);
            return SYS_TIME_RETURN_ERROR;
        }
    }

    ERRMSG("%s ISC error\n", __FUNCTION__);

    return SYS_TIME_RETURN_ERROR;
}

UI32_T SYS_TIME_RemoteSyncDateTime()
{
    UI16_T exist_drv_bmp, valid_drv_bmp;
    UI32_T m_drv_unit_id, drv_unit;
    UI32_T retry_count;
    SYS_TIME_RemoteSyncDateTime_T rsdt;

    memset(&rsdt, 0, sizeof(SYS_TIME_RemoteSyncDateTime_T));

    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p)
        != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TIME_RETURN_ERROR;
    }

    m_drv_unit_id = ISC_OM_GetMyDrvUnitId();
    exist_drv_bmp = ISC_OM_GetExistDrvUnitBmp();
    valid_drv_bmp = ISC_OM_GetValidDrvUnitBmp();

    for (drv_unit = 1; drv_unit <=
         SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK; drv_unit++)
    {
        /* ignore my self
         */
        if (drv_unit == m_drv_unit_id)
            continue;

        /* not exist or invalid drv unit
         */
        if (!(IUC_STACK_UNIT_BMP(drv_unit) & exist_drv_bmp & valid_drv_bmp))
            continue;

        for (retry_count = SYS_TIME_ISC_TRY_COUNT; retry_count > 0; retry_count--)
        {
            SYS_TIME_GetUtcSoftwareClockBySec(&rsdt.sc.time);
            SYS_TIME_OM_ENTER_CRITICAL_SECTION();
            rsdt.tz.offset = sys_time_shmem_data_p->time_zone.offset;
            memcpy(rsdt.tz.zone_name,
                   sys_time_shmem_data_p->time_zone.zone_name,
                   MAXSIZE_sysTimeZoneName + 1);
            SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

            //printf("%s drv_unit %d rsdt.sc.time %d, rsdt.sc.sys_tick %d, rsdt.tz.offset %d, rsdt.tz.zone_name %s\n",
            //    __FUNCTION__, drv_unit, rsdt.sc.time, rsdt.sc.sys_tick, rsdt.tz.offset, rsdt.tz.zone_name);

            if (SyncSlaveSysTime(drv_unit, &rsdt) == SYS_TIME_RETURN_OK)
                break;
        }
    }

    return SYS_TIME_RETURN_OK;
}

static void SYS_TIME_Service_NotSupport(ISC_Key_T *key)
{
    L_MM_Mref_Handle_T *rep_mref_handle_p;
    SYS_TIME_Packet_Header_T *rep_packet_p;
    UI32_T pdu_len;

    rep_mref_handle_p =
        L_MM_AllocateTxBuffer(sizeof(SYS_TIME_Packet_Header_T),
            L_MM_USER_ID(SYS_MODULE_SYS_TIME, SYS_TIME_POOL_ID_ISC_REPLY,
                         SYS_TIME_SERVICE_NOT_SUPPORT));
    rep_packet_p = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);

    if (rep_packet_p == NULL)
    {
        ERRMSG("%s L_MM_Mref_GetPdu return NULL\n", __FUNCTION__);
        return;
    }

    memset(rep_packet_p, 0, sizeof(SYS_TIME_Packet_Header_T));

    rep_packet_p->opcode    = SYS_TIME_SERVICE_NOT_SUPPORT;
    rep_packet_p->seq_no    = SYS_TIME_OM_GetControlSeqNo();
    rep_packet_p->data_size = 0;

    if (ISC_RemoteReply(rep_mref_handle_p, key))
    {
        SYS_TIME_OM_AddControlSeqNo();
    }
}

static void SYS_TIME_SlaveSetSysTime(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    SYS_TIME_Request_Packet_T *req_packet;
    SYS_TIME_RemoteSyncDateTime_T rsdt;
    SYS_TIME_Response_Packet_T *resp_packet;
    L_MM_Mref_Handle_T *rep_mref_handle_p;
    SYS_TIME_TIMEZONE_SIGN_T  sign;
    UI32_T  hour = 0, minute = 0;
    UI32_T  pdu_len;

    req_packet = (SYS_TIME_Request_Packet_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    memcpy(&rsdt, &req_packet->rsdt, sizeof(SYS_TIME_RemoteSyncDateTime_T));
    //printf("%s rsdt.sc.time %d, rsdt.sc.sys_tick %d, rsdt.tz.offset %d, rsdt.tz.zone_name %s\n",
    //       __FUNCTION__, rsdt.sc.time, rsdt.sc.sys_tick, rsdt.tz.offset, rsdt.tz.zone_name);

    SYS_TIME_SetRealTimeClockBySeconds(req_packet->rsdt.sc.time, 0);

    if (req_packet->rsdt.tz.offset < 0)
    {
        sign = SYS_TIME_TIMEZONE_MINUS;
        hour = -(req_packet->rsdt.tz.offset/60);
        minute = -(req_packet->rsdt.tz.offset%60);
    }
    else
    {
        sign = SYS_TIME_TIMEZONE_PLUS;
        hour = req_packet->rsdt.tz.offset/60;
        minute = req_packet->rsdt.tz.offset%60;
    }

    SYS_TIME_SetTimeZone(req_packet->rsdt.tz.zone_name, sign, hour, minute);

    rep_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYS_TIME_Response_Packet_T),
            L_MM_USER_ID(SYS_MODULE_SYS_TIME, SYS_TIME_POOL_ID_ISC_REPLY, SYS_TIME_ACK));
    resp_packet = L_MM_Mref_GetPdu(rep_mref_handle_p, &pdu_len);
    if (resp_packet == NULL)
    {
        ERRMSG("%s L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return;
    }

    resp_packet->header.opcode = SYS_TIME_ACK;
    resp_packet->header.seq_no = SYS_TIME_OM_GetControlSeqNo();
    resp_packet->header.data_size = sizeof(SYS_TIME_Response_Packet_T);

    /* FIXME: used to verify by master
     */
    resp_packet->rsdt.tz.offset = sys_time_shmem_data_p->time_zone.offset;
    SYS_TIME_GetUtcSoftwareClockBySec(&resp_packet->rsdt.sc.time);
    memcpy(resp_packet->rsdt.tz.zone_name,
           sys_time_shmem_data_p->time_zone.zone_name,
           MAXSIZE_sysTimeZoneName + 1);

    if (ISC_RemoteReply(rep_mref_handle_p, key) == TRUE)
        SYS_TIME_OM_AddControlSeqNo();
    else {
        ERRMSG("%s ISC_RemoteReply failed\n", __FUNCTION__);
        L_MM_Mref_Release(&rep_mref_handle_p);
    }
}


BOOL_T SYS_TIME_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    SYS_TIME_Packet_Header_T *rx_header_p;
    UI32_T pdu_len;

    rx_header_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if (rx_header_p == NULL)
    {
        ERRMSG("%s rx_header_p NULL\n", __FUNCTION__);
        return FALSE;
    }

    if (rx_header_p->opcode >= SYS_TIME_TOTAL_REMOTE_SERVICES)
    {
        ERRMSG("%s bad opcode %d\n", __FUNCTION__, rx_header_p->opcode);
        SYS_TIME_Service_NotSupport(key);
        L_MM_Mref_Release(&mref_handle_p);
        return TRUE;
    }

    if (rx_header_p->opcode == SYS_TIME_SYNC_DATE_TIME_REQUEST) {
        SYS_TIME_SlaveSetSysTime(key, mref_handle_p);
        L_MM_Mref_Release(&mref_handle_p);
    }

    return TRUE;
}
#endif

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetDayOfWeek
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will get weekday from seconds
 * INPUT    : UI32_T year
 *            UI32_T month
 *            UI32_T day
 * OUTPUT   : UI32_T *wday - days since Sunday
 * RETURN   : TRUE/FALSE
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T SYS_TIME_GetDayOfWeek(UI32_T seconds, UI32_T *wday)
{
    struct tm result;

    SYSFUN_gmtime(seconds, &result);

    *wday = result.tm_wday;
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_IsTimeModify
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will be used to check sys_time is changed by sntp or admininstrator
 * INPUT:    NONE
 * OUTPUT:   NONE
 * RETURN:   TRUE/FALSE
 * NOTE: 	 none
 *---------------------------------------------------------------------------
 */
BOOL_T SYS_TIME_IsTimeModify(void)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p)
        != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    if (sys_time_shmem_data_p->software_clock.time != __SECONDS1970__)
    {
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }
    else
    {
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
}

static BOOL_T SYS_TIME_GetDaysOfMonth(UI32_T year, UI32_T month, UI32_T *days)
{
    /* check if input buffer and value is valid
     */
    if((NULL == days) || (month > 12))
        return FALSE;

    /* check if the year is leap
     */
    if (TRUE == is_leap_year(year))
    {
        *days = DAY_OF_MONTH[LEAP_YEAR][month - 1];
    }
    else
    {
        *days = DAY_OF_MONTH[NORMAL_YEAR][month - 1];
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetNextProcessDate
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will be used to get next process date which days of month is greater than input date
 * INPUT:    NONE
 * OUTPUT:   NONE
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYS_TIME_GetNextProcessDate(UI32_T year, UI32_T month, UI32_T day, UI32_T *next_year, UI32_T *next_month)
{
    /* Find reload regularity time date
     */
    UI32_T  days_of_month ;
    UI32_T  suspense_year;
    UI32_T  suspense_month;

    if ( (next_year == NULL) || (next_month == NULL) )
    {
        return FALSE;
    }

    suspense_year   = year;
    suspense_month  = month;
    days_of_month   = day;

    do
    {
        if (suspense_month ==12)
        {
            suspense_year++;
            suspense_month = 1;
        }
        else
        {
            suspense_month++;
        }
        SYS_TIME_GetDaysOfMonth(suspense_year, suspense_month, &days_of_month);
    } while ( days_of_month < day );

    *next_year  = suspense_year;
    *next_month = suspense_month;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_ConvertTicksToDiffTime
 * ---------------------------------------------------------------------
 * PURPOSE  : convert ticks to weeks/days/hours/minutes/seconds for UI
 *            to display delta time.
 * INPUT    : ticks
 * OUTPUT   : weeks, days, hours, minutes, seconds, milliseconds
 * RETURN   : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_ConvertTicksToDiffTime(UI32_T ticks, UI32_T *weeks, UI32_T *days,
                                     UI32_T *hours, UI32_T *minutes, UI32_T *seconds,
                                     UI32_T *milliseconds)
{
    UI32_T  tmp;

    *milliseconds = (ticks % SYS_BLD_TICKS_PER_SECOND) * 1000 / SYS_BLD_TICKS_PER_SECOND;
    tmp = ticks / SYS_BLD_TICKS_PER_SECOND;      /* in seconds   */

    *seconds = tmp % 60;
    tmp = tmp / 60;         /* in minutes   */

    *minutes = tmp % 60;
    tmp = tmp / 60;         /* in hours     */

    *hours = tmp % 24;
    tmp = tmp / 24;         /* in days      */

    *days = tmp % 7;
    *weeks = tmp / 7;       /* in weeks     */
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetUTC
 * ---------------------------------------------------------------------
 * PURPOSE  : Get current UTC since 1/1/2001 in seconds.
 * INPUT    : None
 * OUTPUT   : seconds
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetUTC(UI32_T *seconds)
{

    SYS_TIME_GetSoftwareClockBySec(seconds);
    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    *seconds -= sys_time_shmem_data_p->time_zone.offset*60;

    /* When initial sys_time, 86400 has been added to software_clock.time
     */
    if(__SECONDS1970__ <= *seconds)
    {
        *seconds -= __SECONDS1970__;
    }
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYS_TIME_GetRealTimeSecondsFromTOD
 * ---------------------------------------------------------------------
 * PURPOSE  : get real time in seconds based on the input
 *            -- Year,month,day,hour,min,second
 * INPUT    : TOD:Year,month,day,hour,min,second
 * OUTPUT   : Seconds from 2001/01/01 00:00:00 base on input
 * RETURN   : None
 * NOTES    : 1. Symmetry function of  SYS_TIME_ConvertSecondsToDateTime
 * History  :  S.K.Yang     06/10/2002      new added
 * ---------------------------------------------------------------------
 */
void SYS_TIME_GetRealTimeSecondsFromTOD(int year, int month, int day, int hour,
                                        int minute, int second, UI32_T  *seconds)
{
    UI32_T  number_of_leap_years;
    BOOL_T  this_year;
    UI32_T  days, days_of_this_year = 0;
    int     i;

    /* year 2001 - 2004, we don't count.
     * year 2005 - 2008, we count 1.
     * year 2009 - 2012, we count 2.....
     */
    number_of_leap_years = (year - 2001)/4;

    if ((year % 4) == 0)
        this_year = LEAP_YEAR;
    else
        this_year = NORMAL_YEAR;

    /* How many days passing with whole year
     */
    days = ((year-2001) * 365);

    /* How many days passing with whole month for this year
     */
    for (i=0; i<month-1; i++)
    {
        days_of_this_year += DAY_OF_MONTH[this_year][i];
    }

    /* How many days passing with whole day for this year
     */
    days_of_this_year += (day - 1);

    /* so far, we count how many days passing by days, but still not
     * include leap years
     */
    days += days_of_this_year;

    /* Now we add day of leap year
     */
    days += number_of_leap_years;

    if (days > 40000)
        printf("Something strange!  Too many days for SYS_TIME_GetRealTimeBySec()");

    *seconds = (days * 86400) + (hour * 3600) + (minute * 60) + second;
}

#if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DST mode
 * INPUT    : mode_p
 * OUTPUT   : mode_p
 *            DAYLIGHT_SAVING_TIME_DISABLE
 *            DAYLIGHT_SAVING_TIME_ENABLE_AMERICA,
 *            DAYLIGHT_SAVING_TIME_ENABLE_EUROPE_PARTS_OF_ASIA,
 *            DAYLIGHT_SAVING_TIME_ENABLE_AUSTRALIA,
 *            DAYLIGHT_SAVING_TIME_ENABLE_NEW_ZEALAND,
 *            DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED,
 *            DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING
 * RETURN   : none
 * NOTES    : if mode != DAYLIGHT_SAVING_TIME_DISABLE, then it means enable.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetMode(SYS_TIME_DST_MODE_T *mode_p)
{
    /* check if in master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* check if input buffer is valid
     */
    if (NULL == mode_p)
    {
        return FALSE;
    }

    /* get from database
     */
    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    *mode_p = sys_time_shmem_data_p->dst_data.mode;
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_GetRunningMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Get running DST mode
 * INPUT    : mode_p
 * OUTPUT   : mode_p
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_TIME_DST_GetRunningMode(SYS_TIME_DST_MODE_T *mode_p)
{
    /* check if in master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /* check if input buffer is valid
     */
    if (mode_p == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /* get from database
     */
    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    *mode_p = sys_time_shmem_data_p->dst_data.mode;

    /* check if mode is default value
     */
    if(*mode_p == SYS_TIME_DEFAULT_DAYLIGHT_SAVING_TIME)
    {
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetPredefinedName
 *------------------------------------------------------------------------------
 * PURPOSE  : get currnet predefined name
 * INPUT    : none
 * OUTPUT   : predefined name
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetPredefinedName(char *predefined_name_str_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (NULL == predefined_name_str_p)
    {
        return FALSE;
    }

    SYS_TIME_OM_ENTER_CRITICAL_SECTION();

    /* check current mode
     */
    if ((sys_time_shmem_data_p->dst_data.mode < DAYLIGHT_SAVING_TIME_ENABLE_AMERICA) ||
        (sys_time_shmem_data_p->dst_data.mode > DAYLIGHT_SAVING_TIME_ENABLE_NEW_ZEALAND)
       )
    {
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    strncpy(predefined_name_str_p, predefined_region_name_ar[(sys_time_shmem_data_p->dst_data.mode-1)], SYS_TIME_PREDEFINED_STR_LEN);
    predefined_name_str_p[SYS_TIME_PREDEFINED_STR_LEN] = '\0';
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_SetPredefinedMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DST predefined mode
 * INPUT    : mode.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : if mode == DAYLIGHT_SAVING_TIME_DISABLE
              then disbale Daylight Saving Time function.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_SetPredefinedMode(SYS_TIME_DST_MODE_T mode)
{
    #define SYS_TIME_PREDEFINED_MODE_DST_OFFSET       60 /* minutes */

    SYS_TIME_RECURRING_DST begin_recurring_dst, end_recurring_dst;

    /* check if in master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    SYS_TIME_OM_ENTER_CRITICAL_SECTION();

    /* mode is disable, set mode and clear other database
     */
    if(DAYLIGHT_SAVING_TIME_DISABLE == mode)
    {
        sys_time_shmem_data_p->dst_data.mode = mode;
        memset(&(sys_time_shmem_data_p->dst_data.first_day),0,sizeof(SYS_TIME_DST));
        memset(&(sys_time_shmem_data_p->dst_data.last_day),0,sizeof(SYS_TIME_DST));
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }

    /* initialize temp buffer
     */
    memset(&begin_recurring_dst, 0, sizeof(SYS_TIME_RECURRING_DST));
    memset(&end_recurring_dst, 0, sizeof(SYS_TIME_RECURRING_DST));

    /* set predefined data to temp buffer
     */
    switch(mode)
    {
        case DAYLIGHT_SAVING_TIME_ENABLE_AMERICA:
            begin_recurring_dst.year = 0;
            begin_recurring_dst.month = 3;          /* 1:Jan */
            begin_recurring_dst.week = 2;           /* 1:first week */
            begin_recurring_dst.wday = 0;           /* 0:Sunday */
            begin_recurring_dst.hour = 2;           /* 0~23 */
            begin_recurring_dst.minute = 0;         /* 0~59 */
            begin_recurring_dst.second = 0;         /* 0~59 */

            end_recurring_dst.year = 0;
            end_recurring_dst.month = 11;
            end_recurring_dst.week = 1;
            end_recurring_dst.wday = 0;
            end_recurring_dst.hour = 2;
            end_recurring_dst.minute = 0;
            end_recurring_dst.second = 0;
            break;

        case DAYLIGHT_SAVING_TIME_ENABLE_EUROPE_PARTS_OF_ASIA:
            begin_recurring_dst.year = 0;
            begin_recurring_dst.month = 3;
            begin_recurring_dst.week = 5;
            begin_recurring_dst.wday = 0;
            begin_recurring_dst.hour = 0;
            begin_recurring_dst.minute = 0;
            begin_recurring_dst.second = 0;

            end_recurring_dst.year = 0;
            end_recurring_dst.month = 10;
            end_recurring_dst.week = 5;
            end_recurring_dst.wday = 0;
            end_recurring_dst.hour = 23;
            end_recurring_dst.minute = 59;
            end_recurring_dst.second = 59;
            break;

        case DAYLIGHT_SAVING_TIME_ENABLE_AUSTRALIA:
            begin_recurring_dst.year = 0;
            begin_recurring_dst.month = 10;
            begin_recurring_dst.week = 5;
            begin_recurring_dst.wday = 0;
            begin_recurring_dst.hour = 0;
            begin_recurring_dst.minute = 0;
            begin_recurring_dst.second = 0;

            end_recurring_dst.year = 0;
            end_recurring_dst.month = 3;
            end_recurring_dst.week = 5;
            end_recurring_dst.wday = 0;
            end_recurring_dst.hour = 23;
            end_recurring_dst.minute = 59;
            end_recurring_dst.second = 59;
            break;

        case DAYLIGHT_SAVING_TIME_ENABLE_NEW_ZEALAND:
            begin_recurring_dst.year = 0;
            begin_recurring_dst.month = 10;
            begin_recurring_dst.week = 1;
            begin_recurring_dst.wday = 0;
            begin_recurring_dst.hour = 0;
            begin_recurring_dst.minute = 0;
            begin_recurring_dst.second = 0;

            end_recurring_dst.year = 0;
            end_recurring_dst.month = 3;
            end_recurring_dst.week = 3;
            end_recurring_dst.wday = 0;
            end_recurring_dst.hour = 23;
            end_recurring_dst.minute = 59;
            end_recurring_dst.second = 59;
            break;

        case DAYLIGHT_SAVING_TIME_DISABLE:
        case DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED:
        case DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING:
        default:
            SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
            return FALSE;
            break;
    }

    /* set to database
     */
    sys_time_shmem_data_p->dst_data.mode = mode;

    /* During DST, clocks are turned forward an hour, effectively
     * moving an hour of daylight from the morning to the evening.
     *
     * need to check this if some profile use different setings.
     */
    sys_time_shmem_data_p->dst_data.offset = SYS_TIME_PREDEFINED_MODE_DST_OFFSET;

    if (TRUE != SYS_TIME_DST_SetRecurringWeek(&begin_recurring_dst, &end_recurring_dst))
    {
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_SetUserConfigTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DST user config
 * INPUT    : mode, start day and end day of daylight saving time.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : if mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED
 *            then return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_SetUserConfigTime(SYS_TIME_DST_MODE_T mode, const SYS_TIME_DST *begin_dst_p, const SYS_TIME_DST *end_dst_p)
{
    BOOL_T  ret = TRUE;
    UI32_T  soy_first_dst;       /* second-of-year for first day */
    UI32_T  soy_last_dst;        /* second-of-year for last day */

    /* check if in master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* check if input value is valid
     */
    if(mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED)
    {
        return FALSE;
    }

    /* check end time is larger than begin time
     */
    soy_first_dst = SYS_TIME_DST_TimeToSecondOfYear(begin_dst_p);
    soy_last_dst = SYS_TIME_DST_TimeToSecondOfYear(end_dst_p);

    if(soy_last_dst < soy_first_dst)
    {
        if(begin_dst_p->year >= end_dst_p->year)
        {
            return FALSE;
        }
    }

    /* set to database
     */
    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    ret = ret && SYS_TIME_DST_SetFirstDay(begin_dst_p);
    ret = ret && SYS_TIME_DST_SetLastDay(end_dst_p);

    if (ret == TRUE)
    {
        sys_time_shmem_data_p->dst_data.mode = mode;
    }

    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
    return ret;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetUserConfigTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DST user config
 * INPUT    : mode, buffer of start day and end day of daylight saving time.
 * OUTPUT   : *begin_dst -- begin dst
 *            *end_dst   -- end dst
 * RETURN   : TRUE/FALSE
 * NOTES    : if mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED
              then return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetUserConfigTime(SYS_TIME_DST_MODE_T mode, SYS_TIME_DST *begin_dst_p, SYS_TIME_DST *end_dst_p)
{
    /* check if in master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* check if input value is valid
     */
    if(mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED)
    {
        return FALSE;
    }

    /* get from database
     */
    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    SYS_TIME_DST_GetFirstDay(begin_dst_p);
    SYS_TIME_DST_GetLastDay(end_dst_p);
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_SetRecurringTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DST recurring time
 * INPUT    : mode, start day and end day of daylight saving time.
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : if mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING
              will return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_SetRecurringTime(SYS_TIME_DST_MODE_T mode, const SYS_TIME_RECURRING_DST *begin_dst_p, const SYS_TIME_RECURRING_DST *end_dst_p)
{
    /* check if in master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* check if input value is valid
     */
    if(mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING)
    {
        return FALSE;
    }

    SYS_TIME_OM_ENTER_CRITICAL_SECTION();

    /* set to database
     */
    if(FALSE == SYS_TIME_DST_SetRecurringWeek(begin_dst_p, end_dst_p))
    {
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    sys_time_shmem_data_p->dst_data.mode = mode;
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetRecurringTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DST recurring settings
 * INPUT    : mode, bffer of start day and end day of daylight saving time.
 * OUTPUT   : *begin_dst_p -- begin dst
 *            *end_dst_p   -- end dst
 * RETURN   : TRUE/FALSE
 * NOTES    : if mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING
              will return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetRecurringTime(SYS_TIME_DST_MODE_T mode, SYS_TIME_RECURRING_DST *begin_dst_p, SYS_TIME_RECURRING_DST *end_dst_p)
{
    /* check if in master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* check if input value is valid
     */
    if(mode != DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING)
    {
        return FALSE;
    }

    /* check if input buffer is valid
     */
    if((begin_dst_p == NULL) || (end_dst_p == NULL))
    {
        return FALSE;
    }

    /* get from database
     */
    if(FALSE == SYS_TIME_DST_GetRecurringWeek(begin_dst_p, end_dst_p))
    {
        return FALSE;
    }

    return TRUE;
}

#if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_SetZoneName
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DST zone name
 * INPUT    : zone name
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_SetZoneName(const char *zone_name_p)
{
    /* check if in master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (zone_name_p == NULL)
    {
        return FALSE;
    }

    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    strncpy(sys_time_shmem_data_p->dst_data.zone_name, zone_name_p, SYS_TIME_MAX_DST_TIMEZONE_NAME_LEN);
    sys_time_shmem_data_p->dst_data.zone_name[SYS_TIME_MAX_DST_TIMEZONE_NAME_LEN] = '\0';
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetZoneName
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DST zone name
 * INPUT    : none
 * OUTPUT   : zone name
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetZoneName(char *zone_name_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    strncpy (zone_name_p, sys_time_shmem_data_p->dst_data.zone_name, SYS_TIME_MAX_DST_TIMEZONE_NAME_LEN);
    zone_name_p [SYS_TIME_MAX_DST_TIMEZONE_NAME_LEN] = '\0';
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_GetRunningZoneName
 *---------------------------------------------------------------------------
 * PURPOSE : This function will get the DST zone name of the system
 * INPUT   : none
 * OUTPUT  : zone name
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE    : none
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_TIME_DST_GetRunningZoneName(char *zone_name_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    } /* End of if */

    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    if (sys_time_shmem_data_p->dst_data.zone_name[0] == '\0')
    {
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        strncpy(zone_name_p, sys_time_shmem_data_p->dst_data.zone_name, SYS_TIME_MAX_DST_TIMEZONE_NAME_LEN);
        zone_name_p[SYS_TIME_MAX_DST_TIMEZONE_NAME_LEN] = '\0';
    }
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

}
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME == TRUE) */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_SetTimeOffset
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will set the DST time offset
 * INPUT    : time_offset -- time offset in minute
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTE     : none
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_SetTimeOffset(UI32_T time_offset)
{
    /* check if in master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* check if input is valid
     */
    if((SYS_TIME_MIN_DST_TIME_OFFSET > time_offset) || (SYS_TIME_MAX_DST_TIME_OFFSET < time_offset))
    {
        return FALSE;
    }

    /* set to datdabase
     */
    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    sys_time_shmem_data_p->dst_data.offset = time_offset;
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_GetTimeOffset
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will get the DST time offset
 * INPUT    : *time_offset_p -- buffer of  time offset in minute
 * OUTPUT   : time_offset_p -- timeoffset
 * RETURN   : TRUE/FALSE
 * NOTE     : none
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_GetTimeOffset(UI32_T *time_offset_p)
{
    /* check if in master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* check if input pointer is valid
     */
    if(NULL == time_offset_p)
    {
        return FALSE;
    }

    /* get from datdabase
     */
    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    *time_offset_p = sys_time_shmem_data_p->dst_data.offset;
    SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_GetRunningTimeOffset
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will get the running DST time offset
 * INPUT    : *time_offset_p -- buffer of  time offset in minute
 * OUTPUT   : time_offset_p -- timeoffset
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *            SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE     : none
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SYS_TIME_DST_GetRunningTimeOffset(UI32_T *time_offset_p)
{
    /* check if in master mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sys_time_shmem_data_p) != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /* check if input pointer is valid
     */
    if(NULL == time_offset_p)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    /* get from datdabase
     */
    SYS_TIME_OM_ENTER_CRITICAL_SECTION();
    *time_offset_p = sys_time_shmem_data_p->dst_data.offset;

    /* check if value equal default value
     */
    if (*time_offset_p == SYS_TIME_DEFAULT_DST_TIME_OFFSET)
    {
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_IsInSummerTime
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will check if is in date of configured summer time
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - daylight saving time is in effect
 *            FALSE - daylight saving time is not in effect
 * NOTE 	: none
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_DST_IsInSummerTime(void)
{
    UI32_T seconds=0;

    /* update current time to check if is in daylight saving time
     */
    SYS_TIME_GetRealTimeBySec(&seconds);
    return sys_time_shmem_data_p->dst_data.is_effect;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_SetFirstDay
 *------------------------------------------------------------------------------
 * PURPOSE  : Set the beginning of DST
 * INPUT    : *begin_dst_p -- first day's year, month, day, hour, minute, second.
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
static BOOL_T SYS_TIME_DST_SetFirstDay(const SYS_TIME_DST *begin_dst_p)
{
    /* check th input time is valid or not
     * mode, month, week, wday, day, hour, minute, second
     */
    if (TRUE != SYS_TIME_DST_IsValidTime(DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED, begin_dst_p->month, 0,
                0, begin_dst_p->day, begin_dst_p->hour, begin_dst_p->minute, begin_dst_p->second))
    {
        return FALSE;
    }

    /* set to database
     */
    memcpy(&(sys_time_shmem_data_p->dst_data.first_day), begin_dst_p, sizeof(SYS_TIME_DST));

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_SetLastDay
 *------------------------------------------------------------------------------
 * PURPOSE  : Set the ending of DST
 * INPUT    : *end_dst_p -- last day's year, month, day, hour, minute, second.
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
static BOOL_T SYS_TIME_DST_SetLastDay(const SYS_TIME_DST *end_dst_p)
{
    /* check th input time is valid or not
     * mode, month, week, wday, day, hour, minute, second
     */
    if (TRUE != SYS_TIME_DST_IsValidTime(DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED, end_dst_p->month, 0,
                0, end_dst_p->day, end_dst_p->hour, end_dst_p->minute, end_dst_p->second))
    {
        return FALSE;
    }

    /* set to database
     */
    memcpy(&(sys_time_shmem_data_p->dst_data.last_day), end_dst_p, sizeof(SYS_TIME_DST));

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetFirstDay
 *------------------------------------------------------------------------------
 * PURPOSE  : Get all beginning information of DST
 * INPUT    : *begin_dst_p -- first day's year, month, day, hour, minute, second.
 * OUTPUT   : *begin_dst_p -- first day's year, month, day, hour, minute, second.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
static BOOL_T SYS_TIME_DST_GetFirstDay(SYS_TIME_DST *begin_dst_p)
{
    /* check if input buffer is valid
     */
    if(begin_dst_p == NULL)
    {
        return FALSE;
    }

    /* get from database
     */
    memcpy(begin_dst_p, &(sys_time_shmem_data_p->dst_data.first_day), sizeof(SYS_TIME_DST));

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SYS_TIME_DST_GetLastDay
 *------------------------------------------------------------------------------
 * PURPOSE  : Get all ending information of DST
 * INPUT    : *end_dst -- last day's year, month, day, hour, minute, second.
 * OUTPUT   : *end_dst -- last day's year, month, day, hour, minute, second.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
static BOOL_T SYS_TIME_DST_GetLastDay(SYS_TIME_DST *end_dst_p)
{
    /* check if input buffer is valid
     */
    if(end_dst_p == NULL)
    {
        return FALSE;
    }

    /* get from database
     */
    memcpy(end_dst_p, &(sys_time_shmem_data_p->dst_data.last_day), sizeof(SYS_TIME_DST));

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_SetRecurringWeek
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the DST recurring week
 * INPUT:    start_day_p/end_day_p -- start/end date
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTE: 	 none
 *---------------------------------------------------------------------------
 */
static BOOL_T SYS_TIME_DST_SetRecurringWeek(const SYS_TIME_RECURRING_DST *start_day_p, const SYS_TIME_RECURRING_DST *end_day_p)
{
    /* check th input start time is valid or not
     * mode, month, week, wday, day, hour, minute, second
     */
    if (TRUE != SYS_TIME_DST_IsValidTime(DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING, start_day_p->month, start_day_p->week,
                start_day_p->wday, 0, start_day_p->hour, start_day_p->minute, start_day_p->second))
    {
        return FALSE;
    }

    /* check th input end time is valid or not
     * mode, month, week, wday, day, hour, minute, second
     */
    if (TRUE != SYS_TIME_DST_IsValidTime(DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING, end_day_p->month, end_day_p->week,
                end_day_p->wday, 0, end_day_p->hour, end_day_p->minute, end_day_p->second))
    {
        return FALSE;
    }

    /* set to datdabase
     */
    memcpy(&(sys_time_shmem_data_p->dst_data.start_wday), start_day_p, sizeof(SYS_TIME_RECURRING_DST));
    memcpy(&(sys_time_shmem_data_p->dst_data.end_wday), end_day_p, sizeof(SYS_TIME_RECURRING_DST));

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_GetRecurringWeek
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the DST recurring week
 * INPUT:    *start_day_p/end_day_p -- buffer of start/end date
 * OUTPUT:   start_day_p/end_day_p -- start/end date
 * RETURN:   TRUE/FALSE
 * NOTE: 	 none
 *---------------------------------------------------------------------------
 */
static BOOL_T SYS_TIME_DST_GetRecurringWeek(SYS_TIME_RECURRING_DST *start_day_p, SYS_TIME_RECURRING_DST *end_day_p)
{
    /* check if input buffer is valid
     */
    if((NULL == start_day_p) || (NULL == end_day_p))
    {
        return FALSE;
    }

    /* get from datdabase
     */
    memcpy(start_day_p, &(sys_time_shmem_data_p->dst_data.start_wday), sizeof(SYS_TIME_RECURRING_DST));
    memcpy(end_day_p, &(sys_time_shmem_data_p->dst_data.end_wday), sizeof(SYS_TIME_RECURRING_DST));

    return TRUE;
}


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_ApplyToCurrentTime
 *---------------------------------------------------------------------------
 * PURPOSE:  apply DST to seconds_p
 * INPUT:    seconds_p - current time in seconds
             mode -      system DST mode
			 offset - time zone offset
 * OUTPUT:   seconds_p - the seconds which had added summer time
 * RETURN:   none
 * NOTE: 	 none
 *---------------------------------------------------------------------------
 */
static BOOL_T SYS_TIME_DST_ApplyToCurrentTime(UI32_T *seconds_p, SYS_TIME_DST_MODE_T mode, int offset)
{
    SYS_TIME_RECURRING_DST  start_dst;
    SYS_TIME_RECURRING_DST  end_dst;
    SYS_TIME_DST            temp_first_day;
    SYS_TIME_DST            temp_last_day;
    SYS_TIME_DST            current_time;
    int                     year, month, day, hour, minute, second;
    UI32_T                  soy_current_time;/* second-of-year for current time */
    UI32_T                  soy_first_dst;/* second-of-year for first day */
    UI32_T                  soy_last_dst;/* second-of-year for last day */
    BOOL_T                  is_in_daylight_saving_time = FALSE;
    memset(&start_dst, 0, sizeof(SYS_TIME_RECURRING_DST));
    memset(&end_dst, 0, sizeof(SYS_TIME_RECURRING_DST));
    memset(&temp_first_day, 0, sizeof(SYS_TIME_DST));
    memset(&temp_last_day, 0, sizeof(SYS_TIME_DST));
    memset(&current_time, 0, sizeof(SYS_TIME_DST));

    /* if daylight saving time is disable, do nothing.
     */
    if(mode == DAYLIGHT_SAVING_TIME_DISABLE)
    {
        return is_in_daylight_saving_time;
    }

    SYS_TIME_ConvertSecondsToDateTime(*seconds_p, &year, &month, &day,
            &hour, &minute, &second);

    /* set current time to temp buffer
     */
    current_time.year   = year;
    current_time.month  = month;
    current_time.day    = day;
    current_time.hour   = hour;
    current_time.minute = minute;
    current_time.second = second;

    /* convert current time to minute-of-year
     */
    soy_current_time = SYS_TIME_DST_TimeToSecondOfYear(&current_time);

    /* check if current is in daylight-saving-time range
     */
    if(mode == DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED)
    {
        /* convert daylight-saving-time range to minute-of-year
         */
        SYS_TIME_DST_GetFirstDay(&temp_first_day);
        SYS_TIME_DST_GetLastDay(&temp_last_day);

        /* when dst setting is at Feb. 29 and the year is not leap, we use Mar. 1 to replace
         */
        if ((FALSE == is_leap_year(temp_first_day.year)) && (2 == temp_first_day.month) &&
           (29 == temp_first_day.day))
        {
            temp_first_day.month = 3;
            temp_first_day.day = 1;
        }

        if ((FALSE == is_leap_year(temp_last_day.year)) && (2 == temp_last_day.month) &&
           (29 == temp_last_day.day))
        {
            temp_last_day.month = 3;
            temp_last_day.day = 1;
        }
        soy_first_dst = SYS_TIME_DST_TimeToSecondOfYear(&temp_first_day);
        soy_last_dst = SYS_TIME_DST_TimeToSecondOfYear(&temp_last_day);

        /* check if current is in daylight-saving-time range
         *
         * Seconds are not counted for this comparison.
         * Suppose the DST range is 03:00:00 of first day to 03:00:00 of last day.
         * So upon entry, 03:00:01 to 03:00:59 is inside the DST range.
         * Upon exit, 03:00:01 to 03:00:59 is outside the DST range.
         * So for entry, we use >= 03:00, but for exit, we use < 03:00.
         */
        /* check if current is in daylight-saving-time range
         */
        if(temp_first_day.year == temp_last_day.year)
        {
            if (soy_last_dst > soy_first_dst)
            {
                if (year == temp_first_day.year)
                {
                    if ((soy_current_time >= soy_first_dst) && (soy_current_time < soy_last_dst))
                    {
                        is_in_daylight_saving_time = TRUE;
                    }
                }
            }
        }
        else    /* (temp_first_day.year < temp_last_day.year) */
        {
            /* current year is in the range
             */
            if( (year>=temp_first_day.year) && (year<=temp_last_day.year) )
            {
                if( year == temp_first_day.year )
                {
                    if (soy_current_time >= soy_first_dst)
                    {
                        is_in_daylight_saving_time = TRUE;
                    }
                }
                else if( year == temp_last_day.year )
                {
                    if (soy_current_time < soy_last_dst)
                    {
                        is_in_daylight_saving_time = TRUE;
                    }
                }
                else
                {
                    is_in_daylight_saving_time = TRUE;
                }
            }
        }
    }
    else
    {
        /* get and convert daylight-saving-time range to minute-of-year
         */
        SYS_TIME_DST_GetRecurringWeek(&start_dst, &end_dst);
        start_dst.year = current_time.year;
        soy_first_dst = SYS_TIME_DST_RecurringToSecondOfYear(&start_dst);

        end_dst.year = current_time.year;
        soy_last_dst = SYS_TIME_DST_RecurringToSecondOfYear(&end_dst);

        /* check if current is in daylight-saving-time range
         */
        if (soy_last_dst <= soy_first_dst)
        {
            /* dst range is in two year,so dst time is outside of firstday and lastday
             */
            if ((soy_current_time < soy_last_dst) || (soy_current_time >= soy_first_dst))
            {
                is_in_daylight_saving_time = TRUE;
            }
        }
        else
        {
            /* dst time is inside of firstday and lastday
             */
            if ((soy_current_time >= soy_first_dst) && (soy_current_time < soy_last_dst))
            {
                is_in_daylight_saving_time = TRUE;
            }
        }
    }

    /* check if current time is in daylight saving time range
     */
    if(TRUE == is_in_daylight_saving_time)
    {
        *seconds_p += offset * 60;
    }

    return is_in_daylight_saving_time;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_TimeToSecondOfYear
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will convert current time to second-of-year
 * INPUT:    *current_time_p -- current time
 * OUTPUT:   none
 * RETURN:   second-of-year
 * NOTE: 	 none
 *---------------------------------------------------------------------------
 */
static UI32_T SYS_TIME_DST_TimeToSecondOfYear(const SYS_TIME_DST *current_time_p)
{
    UI32_T yday_till_month = 0;

    /* compute how many days past
     */
    yday_till_month = SYS_TIME_DST_GetYdayTillMonth(current_time_p->year, current_time_p->month, current_time_p->day);

    /* compute days,hour,minute,second to minute
     */
    return SECOND_OF_YEAR(yday_till_month, current_time_p->hour, current_time_p->minute, current_time_p->second);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_RecurringToSecondOfYear
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will convert recurring to second-of-year
 * INPUT:    *current_time_p -- recurring time
 * OUTPUT:   none
 * RETURN:   second-of-year
 * NOTE: 	 none
 *---------------------------------------------------------------------------
 */
static UI32_T SYS_TIME_DST_RecurringToSecondOfYear(const SYS_TIME_RECURRING_DST *current_time_p)
{
    UI32_T yday = 0;

    /* compute how many days past
     */
    yday = SYS_TIME_DST_ConvertWdayToYday(current_time_p->year, current_time_p->month, current_time_p->week, current_time_p->wday);

    /* compute days,hour,minute,second to minute
     */
    return SECOND_OF_YEAR(yday, current_time_p->hour, current_time_p->minute, current_time_p->second);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_ConvertWdayToYday
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will convert the specified time to
 *           the total days since year/1/1
 * INPUT:    year  - year
 *           month - month
 *           week  - week
 *           wday  - days since Sunday
 * OUTPUT:   none
 * RETURN:   yday  - total days since year/1/1
 * NOTE: 	   none
 *---------------------------------------------------------------------------
 */
static UI32_T SYS_TIME_DST_ConvertWdayToYday(UI32_T year, UI32_T month, UI32_T week, UI32_T wday)
{
    UI32_T offset = 0;
    UI32_T wday_of_month_1 = 0;         /* weekday of year/month/1 */
    UI32_T yday_till_month_1 = 0;       /* total days since year/1/1 till year/month/1 */
    UI32_T yday_till_end_of_month = 0;  /* total days since year/1/1 till year/month/last_day */
    UI32_T days_of_month = 0, temp_day = 0, seconds = 0;

    /* get total days since year/1/1 till year/month/1
     */
    yday_till_month_1 = SYS_TIME_DST_GetYdayTillMonth(year, month, 1);

    /* get weekday of year/month/1
     */
    SYS_TIME_ConvertDateTimeToSeconds(year, month, 1, 0, 0, 0, &seconds);
    SYS_TIME_GetDayOfWeek(seconds, &wday_of_month_1);

    /* compute wday offset between month/1 and wday
     */
    offset = ((wday_of_month_1 >= wday) ? (wday_of_month_1 - wday) : (wday - wday_of_month_1));

    /* compute day from yday_till_month_1
     */
    if (wday < wday_of_month_1)
    {
        temp_day = yday_till_month_1 + (week * 7) - offset;
    }
    else if (wday >= wday_of_month_1)
    {
        temp_day = yday_till_month_1 + ((week - 1) * 7) + offset;
    }

    /* get days of year/month
     */
    SYS_TIME_GetDaysOfMonth(year, month, &days_of_month);

    /* get total days since year/1/1 till year/month/last_day
     */
    yday_till_end_of_month = yday_till_month_1 + days_of_month;

    /* check if the day is exist
     * may not have the sixth monday of a month
     */
    if(temp_day + 1 > yday_till_end_of_month)
    {
        /* wday does not exist, use last wday
         */
        temp_day -= 7;
    }

    return temp_day;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_GetYdayTillMonth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get total days since 1/1 till month/day
 * INPUT:    year/month/day -- input year/month/day
 * OUTPUT:   none
 * RETURN:   yday - total days since 1/1 till month/day
 * NOTE: 	 none
 *---------------------------------------------------------------------------
 */
static UI32_T SYS_TIME_DST_GetYdayTillMonth(UI32_T year, UI32_T month, UI32_T day)
{
    int leap;

    leap = ((TRUE == is_leap_year(year)) ? 1 : 0);

    return (sys_time_shmem_data_p->dst_data.ydays_till_month[leap][month -1] + day - 1);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_DST_GetWdayOfFirstDayOfYear
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the weekday of year/1/1
 * INPUT:    year - input year
 * OUTPUT:   none
 * RETURN:   wday - the weekday of year/1/1
 * NOTE: 	   none
 *---------------------------------------------------------------------------
 */
static UI32_T SYS_TIME_DST_GetWdayOfFirstDayOfYear(UI32_T year)
{
    UI32_T seconds = 0, wday = 0;

    /* convert the time of year/1/1
     */
    SYS_TIME_ConvertDateTimeToSeconds(year, 1, 1, 0, 0, 0, &seconds);
    SYS_TIME_GetDayOfWeek(seconds, &wday);

    return wday;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_IsValidTime
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will check the input time is valid or not
 * INPUT    : mode, month, week, wday, day, minute, second
 * OUTPUT   : None
 * RETURN   : TRUE - input time is valid
 *            FALSE - input time is invalid
 * NOTE 	: none
 *---------------------------------------------------------------------------*/
static BOOL_T SYS_TIME_DST_IsValidTime(SYS_TIME_DST_MODE_T mode, UI32_T month, UI32_T week, UI32_T wday, UI32_T day, UI32_T hour, UI32_T minute, UI32_T second)
{
    /* date mode
     */
    if (DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED == mode)
    {
        if ((12 < month) || (0 == month)   ||
            (31 < day)   || (0 == day)     ||
            (24 <= hour) || (60 <= minute) ||
            (60 <= second)
           )
        {
            return FALSE;
        }

        /* check if year, month, day is valid
         */
        if(DAY_OF_MONTH[1][month-1] < day)
        {
            return FALSE;
        }
    }
    /* recurring mode
     */
    else if (DAYLIGHT_SAVING_TIME_ENABLE_USER_CONFIGURED_RECURRING == mode)
    {
        if ((12 < month) || (0 == month) ||
            (5 < week)   || (0 == week)  ||
            (24 <= hour) || (60 <= minute) ||
            (60 <= second) || (6 < wday)
           )
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}
#endif /* #if (SYS_CPNT_SYS_TIME_SUMMERTIME == TRUE) */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_ConvertMonth
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will convert month number to English
 * INPUT    : UI32_T mon
 * OUTPUT   : char month_ar - Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
 * RETURN   : TRUE - month number is in effect
 *            FALSE - month number is out of range
 * NOTE 	: none
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_ConvertMonth(UI32_T mon, char *month_p)
{
    if( (mon<1) || (mon>12))
        return FALSE;

    strcpy(month_p, month_name_ar[mon-1]);
    month_p[SYS_TIME_ABBREVIATION_STR_LEN] = '\0';
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_ConvertWeekday
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will convert week day number to English
 * INPUT    : UI32_T day
 * OUTPUT   : char weekday_ar - Sun Mon Tue Wed Thu Fri Sat
 * RETURN   : TRUE - day number is in effect
 *            FALSE - day number is out of range
 * NOTE 	: none
 *---------------------------------------------------------------------------*/
BOOL_T SYS_TIME_ConvertWeekday(UI32_T day, char *weekday_p)
{
    if( (day<0) || (day>6))
        return FALSE;

    strcpy(weekday_p, weekday_name_ar[day]);
    weekday_p[SYS_TIME_ABBREVIATION_STR_LEN] = '\0';
    return TRUE;
}

static BOOL_T
SYS_TIME_GetTimezonePredefineIndex(
    UI32_T  timezone_id,
    UI32_T  *index)
{
    for (*index = 0; *index < _countof(systime_timezone_predefined); ++*index)
    {
        if (timezone_id == systime_timezone_predefined[*index].id)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_SetTimezonePredefined
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will set timezone-predefined
 * INPUT    : timezone_id
 * OUTPUT   : none
 * RETURN   : TRUE - success.
 *            FALSE - fail to set timezone-predefined.
 * NOTE     : none
 *---------------------------------------------------------------------------*/
BOOL_T
SYS_TIME_SetTimezonePredefined(
    UI32_T  timezone_id)
{
    UI32_T index = 0;

    if (SYS_TIME_DEFAULT_TIMEZONE_ID == timezone_id)
    {
        SYS_TIME_OM_ENTER_CRITICAL_SECTION();
        strncpy(sys_time_shmem_data_p->time_zone.zone_name,
                SYS_TIME_DEFAULT_TIMEZONE_NAME, sizeof(sys_time_shmem_data_p->time_zone.zone_name) - 1);
        sys_time_shmem_data_p->time_zone.zone_name[sizeof(sys_time_shmem_data_p->time_zone.zone_name) - 1] = '\0';
        sys_time_shmem_data_p->time_zone.offset = SYS_TIME_DEFAULT_TIMEZONE_OFFSET;
        sys_time_shmem_data_p->time_zone.timezone_offset_id = SYS_TIME_DEFAULT_TIMEZONE_ID;
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }

    if (TRUE == SYS_TIME_GetTimezonePredefineIndex(timezone_id, &index))
    {
        SYS_TIME_OM_ENTER_CRITICAL_SECTION();
        sys_time_shmem_data_p->time_zone.offset = systime_timezone_predefined[index].offest;
        sys_time_shmem_data_p->time_zone.timezone_offset_id = timezone_id;
        SYS_TIME_OM_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }

    return FALSE;
}

static BOOL_T
SYS_TIME_GetTimeZoneInfo(
    SYS_TIME_Timezone_T *timezone)
{
    UI32_T index = 0;

    if (VAL_sysTimeZonePredefined_none == sys_time_shmem_data_p->time_zone.timezone_offset_id)
    {
        strncpy(timezone->timezone.custom.name,
                sys_time_shmem_data_p->time_zone.zone_name,
                sizeof(timezone->timezone.custom.name) - 1);
        timezone->timezone.custom.name[sizeof(timezone->timezone.custom.name) - 1] = '\0';
        timezone->type = SYS_TIME_TIMEZONE_TYPE_TIMEZONE;

        if (sys_time_shmem_data_p->time_zone.offset < 0)
        {
            timezone->timezone.custom.sign = SYS_TIME_TIMEZONE_MINUS;
            timezone->timezone.custom.hour = -(sys_time_shmem_data_p->time_zone.offset/60);
            timezone->timezone.custom.minute = -(sys_time_shmem_data_p->time_zone.offset%60);
        }
        else
        {
            timezone->timezone.custom.sign = SYS_TIME_TIMEZONE_PLUS;
            timezone->timezone.custom.hour = sys_time_shmem_data_p->time_zone.offset/60;
            timezone->timezone.custom.minute = sys_time_shmem_data_p->time_zone.offset%60;
        }
    }
    else
    {
        if (TRUE != SYS_TIME_GetTimezonePredefineIndex(sys_time_shmem_data_p->time_zone.timezone_offset_id, &index))
        {
            return FALSE;
        }

        strncpy(timezone->timezone.predefined.name,
                systime_timezone_predefined[index].name,
                sizeof(timezone->timezone.predefined.name) - 1);
        timezone->timezone.predefined.name[sizeof(timezone->timezone.predefined.name) - 1] = '\0';
        timezone->type = SYS_TIME_TIMEZONE_TYPE_TIMEZONE_PREDEFINE;
        timezone->timezone.predefined.id = sys_time_shmem_data_p->time_zone.timezone_offset_id;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_TIME_GetNextTimeZonePredefinedData
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will get the next timezone-predefined data.
 * INPUT    : timezone_predefined->index
 * OUTPUT   : timezone_predefined - next timezone-predefined data.
 * RETURN   : TRUE - success.
 *            FALSE - fail.
 * NOTE     : The index of the timezone-predefined data is from
 *            0 to _countof(timezone_predefined_id) - 1.
 *            Use 0xffffffff as timezone_predefined->index to get the
 *            first entry.
 *---------------------------------------------------------------------------*/
BOOL_T
SYS_TIME_GetNextPredefinedTimeZone(
    SYS_TIME_Timezone_Predefined_T *timezone_predefined)
{
    if (NULL == timezone_predefined)
    {
        return FALSE;
    }

    if ((0xffffffff != timezone_predefined->index) &&
        (_countof(systime_timezone_predefined) <= (timezone_predefined->index + 1)))
    {
        return FALSE;
    }

    if (0xffffffff == timezone_predefined->index)
    {
        timezone_predefined->index = 0;
    }
    else
    {
        timezone_predefined->index ++;
    }

    timezone_predefined->id = systime_timezone_predefined[timezone_predefined->index].id;
    strncpy(timezone_predefined->name, systime_timezone_predefined[timezone_predefined->index].name, sizeof(timezone_predefined->name) - 1);
    timezone_predefined->name[sizeof(timezone_predefined->name) - 1] = '\0';

    return TRUE;
}


#ifndef _UC_MGR_H
#define _UC_MGR_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"

#ifndef INCLUDE_LOADER
#include "sysrsc_mgr.h"
#endif

/* NAMING CONSTANT
 */

/*enum for switch external loopback test status in runtime - mikeliu.121004*/
enum EXTERNAL_STATUS_INDEX
{
    EXTERNAL_NOT_TEST = 0,
    EXTERNAL_TEST_FAIL,
    EXTERNAL_TEST_SUCESS
};

enum INDEX
{
   UC_MGR_SYS_INFO_INDEX = 0,
   UC_MGR_DOWNLOAD_INFO_INDEX,
   UC_MGR_SYSLOG_UC_NORMAL_INDEX,
   UC_MGR_SYSLOG_UC_FLASH_INDEX,
   UC_MGR_DHCP_INFO_INDEX,
   UC_MGR_PROVISION_RESTART_INFO_INDEX,
   UC_MGR_POST_RESULT_INFO_INDEX,
   UC_MGR_PROVISION_FILE_INFO_INDEX,
   UC_MGR_OEM_LOGO_INDEX,
   UC_MGR_LOOPBACK_TEST_RESULT_INDEX,
   UC_MGR_BOOT_REASON_CHECKING_INDEX,
   UC_MGR_DHCPV6_CLIENT_INFO_INDEX,
   UC_MGR_DBG_BUF,
   UC_MGR_RESERVED_INDEX,
   UC_MGR_RESERVED_INDEX_13,
   UC_MGR_RESERVED_INDEX_14,
   UC_MGR_RESERVED_INDEX_15,
   UC_MGR_RESERVED_INDEX_16,
   UC_MGR_RESERVED_INDEX_17,
   UC_MGR_RESERVED_INDEX_18,
   UC_MGR_RESERVED_INDEX_19,
   UC_MGR_RESERVED_INDEX_20,
   UC_MGR_RESERVED_INDEX_21,
   UC_MGR_RESERVED_INDEX_22,
   UC_MGR_RESERVED_INDEX_23,
   UC_MGR_RESERVED_INDEX_24,
   UC_MGR_RESERVED_INDEX_25,
   UC_MGR_RESERVED_INDEX_26,
   UC_MGR_RESERVED_INDEX_27,
   UC_MGR_RESERVED_INDEX_28,
   UC_MGR_RESERVED_INDEX_29,
   UC_MGR_RESERVED_INDEX_30,
   UC_MGR_RESERVED_INDEX_31,
   UC_MGR_RESERVED_INDEX_32,
   UC_MGR_RESERVED_INDEX_33,
   UC_MGR_RESERVED_INDEX_34,
   UC_MGR_RESERVED_INDEX_35,
   UC_MGR_RESERVED_INDEX_36,
   UC_MGR_RESERVED_INDEX_37,
   UC_MGR_RESERVED_INDEX_38,
   UC_MGR_RESERVED_INDEX_39,
   UC_MGR_RESERVED_INDEX_40,
   UC_MGR_RESERVED_INDEX_41,
   UC_MGR_RESERVED_INDEX_42,
   UC_MGR_RESERVED_INDEX_43,
   UC_MGR_RESERVED_INDEX_44,
   UC_MGR_RESERVED_INDEX_45,
   UC_MGR_RESERVED_INDEX_46,
   UC_MGR_RESERVED_INDEX_47,
   UC_MGR_RESERVED_INDEX_48,
   UC_MGR_RESERVED_INDEX_49,
   UC_MGR_RESERVED_INDEX_50,
   UC_MGR_RESERVED_INDEX_51,
   UC_MGR_RESERVED_INDEX_52,
   UC_MGR_RESERVED_INDEX_53,
   UC_MGR_RESERVED_INDEX_54,
   UC_MGR_RESERVED_INDEX_55,
   UC_MGR_RESERVED_INDEX_56,
   UC_MGR_RESERVED_INDEX_57,
   UC_MGR_RESERVED_INDEX_58,
   UC_MGR_RESERVED_INDEX_59,
   UC_MGR_RESERVED_INDEX_60,
   UC_MGR_RESERVED_INDEX_61,
   UC_MGR_RESERVED_INDEX_62,
   UC_MGR_RESERVED_INDEX_63,
};

/* WARNING!!! A duplicate definition is in ams_ucmgr.h which is used by uboot.
 *            Any changes to UC_MGR_Sys_Info_T must also be synced to
 *            ams_ucmgr.h!
 */
typedef struct UC_MGR_Sys_Info_S
{
    unsigned long   magic_number;                                           /* to tell cold start or warm start             */
    unsigned long   *warm_start_entry;                                      /* entry of warm start                          */
    unsigned char   mac_addr[SYS_ADPT_MAC_ADDR_LEN];                        /* MAC address(len 6)                           */
    unsigned char   serial_no[SYS_ADPT_SERIAL_NO_STR_LEN + 1];              /* serial number(len 11+1)                      */
    unsigned char   mainboard_hw_ver[SYS_ADPT_HW_VER_STR_LEN + 1];          /* Mainboard board hardware version(len 5+1)    */
    unsigned char   manufacture_date[SYS_ADPT_MANUFACTURE_DATE_LEN + 1];        /* Product date, option to key in. 2001-10-31   */
    unsigned char   loader_ver[SYS_ADPT_FW_VER_STR_LEN + 1];                /* Loader software version(len 11+1)            */
    unsigned char   post_ver[SYS_ADPT_FW_VER_STR_LEN + 1];                  /* POST software version(len 11+1)              */
    unsigned char   runtime_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1];            /* runtime version(len 11+1)                    */
    unsigned char   sw_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1];         /* service tag serial number (len 11+1)         */
    unsigned char   model_number[SYS_ADPT_MODEL_NUMBER_LEN + 1];            /* Model number(len 15+1)                       */
    unsigned char   sw_chassis_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1]; /* service tag serial number (len 11+1)         */
    unsigned long   sw_identifier;
    unsigned long   project_id;
    unsigned long   board_id;
    unsigned long   baudrate;
    unsigned char   post_pass;                                              /* POST result: 0 or 1 */
    unsigned char   epld_version;                                           /* EPLD version */
    unsigned char   module_expected_runtime_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1]; /* For the multi-image header, used by loader to fill in module part runtime version */
    unsigned long   post_mode;                                              /* indicate which POST mode is selected, have a value defined in POST_MODE_E - mikeliu.160804 */
    unsigned char   switch_loopback_pass;                                   /* POST switch loopback result: 0(not tested), 1(fail) or 2(pass) - mikeliu.160804 */
    unsigned char   loader_customized_ver[SYS_ADPT_LOADER_CUSTOMIZED_VER_STR_LEN + 1]; /* Loader customzied software version(len 11+1)            */
    union
    {
        unsigned char   reserved[824];                                      /* reserved for future using, Jason suggestion  */
        struct
        {
            unsigned long   *boot_reason_address;                   /* this address which storages boot reason */
            unsigned long   software_build_time;
            unsigned char   check_ram_result;
            unsigned char   check_flash_result;
            unsigned char   check_i2c_result;
            unsigned char   check_pwrcable_result;
            unsigned char   burn_in_post_ever_failed;               /* this is used by diag in burn in post mode, 0:no post failure ever occurred 1:at least one post failure occurred */
            unsigned char   reserved[811];	
        } __attribute__((packed, aligned(1)))  ext_info;
    } __attribute__((packed, aligned(1)))  raw; 	
    unsigned short  check_sum;                                              /* check sum to protect mac and serial no.      */
} UC_MGR_Sys_Info_T;


#if 0 // for old structure
typedef struct UC_MGR_Sys_Info_S /* total 64 bytes */
{
    UI32_T  magic_number;                                           /* to tell cold start or warm start             */
    UI32_T  *warm_start_entry;                                      /* entry of warm start                          */
    UI8_T   mac_addr[SYS_ADPT_MAC_ADDR_LEN];                        /* MAC address(len 6)                           */
    UI8_T   serial_no[SYS_ADPT_SERIAL_NO_STR_LEN + 1];              /* serial number(len 11+1)                      */
    UI8_T   mainboard_hw_ver[SYS_ADPT_HW_VER_STR_LEN + 1];          /* Mainboard board hardware version(len 5+1)    */
    UI8_T   manufacture_date[SYS_ADPT_MANUFACTURE_DATE_LEN + 1];        /* Product date, option to key in. 2001-10-31   */
    UI8_T   loader_ver[SYS_ADPT_FW_VER_STR_LEN + 1];                /* Loader software version(len 11+1)            */
    UI8_T   post_ver[SYS_ADPT_FW_VER_STR_LEN + 1];                  /* POST software version(len 11+1)              */
    UI8_T   runtime_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1];            /* runtime version(len 11+1)                    */
    UI8_T   sw_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1];         /* service tag serial number (len 11+1)         */
    UI8_T   model_number[SYS_ADPT_MODEL_NUMBER_LEN + 1];            /* Model number(len 15+1)                       */
    UI8_T   sw_chassis_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1]; /* service tag serial number (len 11+1)         */
    UI32_T  sw_identifier;
    UI32_T  project_id;
    UI32_T  board_id;
    UI32_T  baudrate;
    UI8_T   post_pass;                                              /* POST result: 0 or 1 */
    UI8_T   epld_version;                                           /* EPLD version */
    UI8_T   module_expected_runtime_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1]; /* For the multi-image header, used by loader to fill in module part runtime version */
    UI32_T  post_mode;                                              /* indicate which POST mode is selected, have a value defined in POST_MODE_E - mikeliu.160804 */
    UI8_T   switch_loopback_pass;                                   /* POST switch loopback result: 0(not tested), 1(fail) or 2(pass) - mikeliu.160804 */
    UI8_T   loader_customized_ver[SYS_ADPT_LOADER_CUSTOMIZED_VER_STR_LEN + 1]; /* Loader customzied software version(len 11+1)            */

    /* PROBLEM:
     * 1. This union is actually unnecessary.
     * 2. This structure is not packed, and so may be compiler-dependent.
     * 3. This structure size depends on SYS_ADPT instead of being fixed.
     * 4. This structure has size 1028 (not 1024) for most project's SYS_ADPT values.
     *
     * FUTURE WORK - to define another structure for new projects:
     * 1. Remove this union and move all fields to the same level.
     * 2. Pack this whole structure.
     * 3. Remove the reserved, and define another fixed-size structure to wrap this structure.
     * 4. Set the wrapper structure to 1024 bytes, by using a calculated reserved size.
     */
    union
    {
        unsigned char   reserved[824];                                      /* reserved for future using, Jason suggestion  */

        struct
        {
            unsigned long   *boot_reason_address;                           /* this address which storages boot reason */

            /* This records software build time as Unix seconds,
             * i.e. number of seconds since 1970.01.01 00:00 UTC.
             * Use SYS_TIME_ConvertTime for display to UI.
             */
            UI32_T          software_build_time;

            UI8_T           check_ram_result;
            UI8_T           check_flash_result;
            UI8_T           check_i2c_result;
            UI8_T           check_pwrcable_result;

            unsigned char   reserved[812];
        } __attribute__((packed, aligned(1)))  ext_info;

#if 0  /* old structure */
        struct
        {
            unsigned long   *boot_reason_address;                           /* this address which storages boot reason */
            unsigned char   reserved[820];                                  /* reserved for future using */
        } __attribute__((packed, aligned(1)))  ext_info;
#endif
    } __attribute__((packed, aligned(1)))  raw;

    /* this checksum must be located in the last 2 bytes
     */
    UI16_T  check_sum;                                              /* check sum to protect mac and serial no.      */
} UC_MGR_Sys_Info_T;
#endif


#define boot_reason_addr raw.ext_info.boot_reason_address
#define software_build_time_macro raw.ext_info.software_build_time

typedef struct UC_MGR_Download_Info_S
{
    UI32_T  *download_ptr;  /* download start pointer   */
    UI32_T  download_size;  /* download size            */
    UI32_T  check_sum;      /* check sum                */
} UC_MGR_Download_Info_T;


typedef struct UC_MGR_KERNEL_DBG_BUF_S
{
    unsigned long start;
    unsigned long end;
    unsigned long size;
} UC_MGR_KERNEL_DBG_BUF_T;


/* EXPORTED ROUTINE DECLARACTION
 */


/* For shared memory allocation
 */
void UC_MGR_AttachSystemResources(void);
void UC_MGR_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);


/* FUNCTION NAME: UC_MGR_InitiateProcessResources
 * PURPOSE: This function is used to initialize the un-cleared memory management module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Initialize the ucmgmt module and create semphore.
 *
 */
BOOL_T UC_MGR_InitiateProcessResources (void);

BOOL_T UC_MGR_InitiateSystemResources(void);

/* FUNCTION NAME: UC_MGR_Allocate
 * PURPOSE: This function is used to allocate memory from uc memory for need module.
 * INPUT:   alloc_index -- each data object must correspond a unique index value,
 *                         vaule range: 0 .. (SYS_ADPT_MAX_UC_BUFFER_POINT_INDEX_NUM-1).
 *          alloc_size  -- bytes to allocate.
 *          boundary    -- must order of 2
 * OUTPUT:  None.
 * RETUEN:  not 0   -- successful.
 *          0       -- unspecified failure, no free memory.
 * NOTES:   Must be called after UC_MGR_InitiateProcessResources.
 *
 */
void *UC_MGR_Allocate ( UI32_T alloc_index, UI32_T alloc_size, UI32_T boundary);


/* FUNCTION NAME: UC_MGR_GetSysInfo
 * PURPOSE: This function is used to get system information from uc memory.
 * INPUT:   *sys_info   -- output buffer of the system information.
 * OUTPUT:  *sys_info   -- the system information.
 * RETUEN:  TRUE    -- successful
 *          FALSE   -- failure
 * NOTES:   1. The function should be called by stktplg.
 *          2. The function should be called after the memory be allocated.
 *          3. If the memory is not allocated, the function will return FALSE.
 *
 */
BOOL_T UC_MGR_GetSysInfo(UC_MGR_Sys_Info_T *sys_info);


/* FUNCTION NAME: UC_MGR_SetSysInfo
 * PURPOSE: This function is used to set system information to uc memory.
 * INPUT:   sys_info    -- setting value of the system information.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful
 *          FALSE   -- failure
 * NOTES:   1. The function should be called by prom code/load code.
 *          2. The function should be called after the memory be allocated.
 *          3. If the memory is not allocated, the function will return FALSE.
 *
 */
BOOL_T UC_MGR_SetSysInfo(UC_MGR_Sys_Info_T sys_info);


/* FUNCTION NAME: UC_MGR_GetDownloadInfo
 * PURPOSE: This function is used to get download information from uc memory.
 * INPUT:   *download_info   -- output buffer of the download information.
 * OUTPUT:  *download_info   -- the download information.
 * RETUEN:  TRUE    -- successful
 *          FALSE   -- failure
 * NOTES:   1. The function should be called by download module.
 *          2. The function should be called after the memory be allocated.
 *          3. If the memory is not allocated, the function will return FALSE.
 *
 */
BOOL_T UC_MGR_GetDownloadInfo(UC_MGR_Download_Info_T *download_info);


/* FUNCTION NAME: UC_MGR_SetDownloadInfo
 * PURPOSE: This function is used to set download information to uc memory.
 * INPUT:   download_info    -- setting value of the download information.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful
 *          FALSE   -- failure
 * NOTES:   1. The function should be called by prom code/load code.
 *          2. The function should be called after the memory be allocated.
 *          3. If the memory is not allocated, the function will return FALSE.
 *
 */
BOOL_T UC_MGR_SetDownloadInfo(UC_MGR_Download_Info_T download_info);

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
/* FUNCTION NAME: UC_MGR_GetLoopbackTestResult
 * PURPOSE:       This function is used to get loopback test result, that was
 *                set by DIAG, from UC memory.
 * INPUT:         test_result --- The loopback test result in the format of port bit map.
 *                                If loopback test fail, the bit is "1", otherwise "0".
 *                                The MSB of byte 0 is port 1,
 *                                the LSB of byte 0 is port 8,
 *                                the MSB of byte 1 is port 9,
 *                                the LSB of byte 1 is port 16,
 *                                ...
 *                                and so on.
 * OUTPUT:        None.
 * RETUEN:        TRUE        --- successful
 *                FALSE       --- failure
 * NOTES:         1. This API shall only be called by SWCTRL.
 *                2. The function should be called after the memory be allocated by using
 *                   index UC_MGR_LOOPBACK_TEST_RESULT_INDEX.
 *                3. If the memory is not allocated, the function will return FALSE.
 */
BOOL_T UC_MGR_GetLoopbackTestResult(UI8_T test_result[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);


/* FUNCTION NAME: UC_MGR_SetLoopbackTestResult
 * PURPOSE:       This function is used to set loopback test result, that wiil be
 *                got by SWCTRL, to UC memory.
 * INPUT:         None.
 * OUTPUT:        test_result --- The loopback test result in the format of port bit map.
 *                                If loopback test fail, the bit is "1", otherwise "0".
 *                                The MSB of byte 0 is port 1,
 *                                the LSB of byte 0 is port 8,
 *                                the MSB of byte 1 is port 9,
 *                                the LSB of byte 1 is port 16,
 *                                ...
 *                                and so on.
 * RETUEN:        TRUE        --- successful
 *                FALSE       --- failure
 * NOTES:         1. This API shall only be called by DIAG.
 *                2. The function should be called after the memory be allocated by using
 *                   index UC_MGR_LOOPBACK_TEST_RESULT_INDEX.
 *                3. If the memory is not allocated, the function will return FALSE.
 */
BOOL_T UC_MGR_SetLoopbackTestResult(UI8_T test_result[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);
#endif /* SYS_CPNT_3COM_LOOPBACK_TEST */

#if (SYS_CPNT_SDRAM_AUTO_DETECTION == TRUE)
/* FUNCTION NAME: UC_MGR_GetSdramSize
 * PURPOSE: This function is used to get SDRAM size.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  SDRAM size
 *
 * NOTES:   As this function is only used by loader,
 *          it can read the value in SYS_HWCFG_SDRAM_CONFIG_ADDR
 *          directly.
 *
 */
UI32_T UC_MGR_GetSdramSize(void);
#endif /* SYS_CPNT_SDRAM_AUTO_DETECTION */

/* FUNCTION NAME: UC_MGR_GetUcMemStartAddr
 * PURPOSE: This function is used to get UC memory start address.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  UC memory start address
 *
 * NOTES:
 *
 */
UI32_T UC_MGR_GetUcMemStartAddr(void);

/* FUNCTION NAME: UC_MGR_GetUcMemEndAddr
 * PURPOSE: This function is used to get UC memory end address.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  UC memory end address
 *
 * NOTES:
 *
 */
UI32_T UC_MGR_GetUcMemEndAddr(void);


/* FUNCTION NAME: UC_MGR_SetDirtySignature
 * PURPOSE: This function is used to change UC signature word,
 *          so that UC can be reinit after reload.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 */
void UC_MGR_SetDirtySignature(void);

#endif

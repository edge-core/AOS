/* MODULE NAME:  rootfs_check_ver.c
 * PURPOSE:
 *     This program will check whether the loader and kernel versions are compatible
 *     with the programs on the root file system.
 *
 * NOTES:
 *     This program should be executed in the initial script before processes are
 *     spawn.
 *     This program will return 1 if the loader and kernel versions are compatible
 *     and return 0 if incompatible. The script should stop executing any programs
 *     that will spawn processes when incompatible situation occurs.
 *
 * HISTORY
 *    11/5/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

#include "sys_ver.h"
#include "l_cmnlib_init.h"
#include "uc_mgr.h"
#include "l_stdlib.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#if 0
#define DEBUG_LINE printf("line %d\n", __LINE__);fflush(stdout)
#else
#define DEBUG_LINE
#endif

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void show_version_info(void);
static BOOL_T check_loader_ver(const UC_MGR_Sys_Info_T* sysinfo_p);
static BOOL_T check_kernel_ver(void);

/* STATIC VARIABLE DECLARATIONS
 */
static char current_loader_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
static char current_kernel_ver[SYS_ADPT_KERNEL_VER_STR_LEN+1];

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : main
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    the main entry for rootfs_check_ver
 *
 * INPUT:
 *    argc     --  the size of the argv array
 *    argv     --  the array of arguments
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    1 -- Success
 *    0 -- Error
 * NOTES:
 *    This function is the entry point of rootfs_check_ver and this program
 *    will not spawn process. It will perform the version compatible check
 *    in the same process.
 *------------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UC_MGR_Sys_Info_T sys_info;

    current_loader_ver[0]=0;
    current_kernel_ver[0]=0;
    /* process initialization
     */
    L_CMNLIB_INIT_InitiateProcessResources();

    DEBUG_LINE;
    if(UC_MGR_InitiateProcessResources()==FALSE)
    {
        printf("\r\nrootfs_check_ver:UC_MGR_InitiateProcessResources fail");
        DEBUG_LINE;
        show_version_info();
        return 0;
    }

    DEBUG_LINE;
    if(UC_MGR_GetSysInfo(&sys_info)==FALSE)
    {
        printf("rootfs_check_ver:UC_MGR_GetSysInfo fail");
        DEBUG_LINE;
        show_version_info();
        return 0;
    }

    DEBUG_LINE;
    if(check_loader_ver(&sys_info)==FALSE)
        return 0;

    DEBUG_LINE;
    if(check_kernel_ver()==FALSE)
        return 0;

    return 1;
}

/* LOCAL SUBPROGRAM BODIES
 */
static void show_version_info(void)
{
    /* show current loader version if available
     */
    printf("\r\nCurrent loader version:");
    if(current_loader_ver[0]==0)
    {
        printf("N/A");
    }
    else
    {
        printf("%s", current_loader_ver);
    }
    /* show compatible range of loader version
     */
    printf("\r\nCompatible loader version range:'%s-%s' to '%s-%s'",
        SYS_VER_LOADER_COMPAT_VER_ORG_PART_START, SYS_VER_LOADER_COMPAT_VER_ACCTON_PART_START,
        SYS_VER_LOADER_COMPAT_VER_ORG_PART_END, SYS_VER_LOADER_COMPAT_VER_ACCTON_PART_END);

    /* show current kernel version if available
     */
    printf("\r\nCurrent kernel version:");
    if(current_kernel_ver[0]==0)
    {
        printf("N/A");
    }
    else
    {
        printf("%s", current_kernel_ver);
    }
    /* show compatible range of kernel version
     */
    printf("\r\nCompatible kernel version range:'%s-%s' to '%s-%s'",
        SYS_VER_KERNEL_COMPAT_VER_ORG_PART_START, SYS_VER_KERNEL_COMPAT_VER_ACCTON_PART_START,
        SYS_VER_KERNEL_COMPAT_VER_ORG_PART_END, SYS_VER_KERNEL_COMPAT_VER_ACCTON_PART_END);
}

static BOOL_T check_loader_ver(const UC_MGR_Sys_Info_T* sysinfo_p)
{
    UI8_T loader_ver_org_part[SYS_ADPT_FW_VER_STR_LEN + 1];
    UI8_T loader_ver_accton_part[SYS_ADPT_FW_VER_STR_LEN + 1];
    UI8_T *div_pos;

    DEBUG_LINE;
    /* fill current_loader_ver which will be used by show_version_info
     */
    strncpy(current_loader_ver, (char*)(sysinfo_p->loader_ver), sizeof(sysinfo_p->loader_ver));
    current_loader_ver[sizeof(current_loader_ver)-1]=0;

    DEBUG_LINE;
    /* sample version of loadder: 1.2.0-1.0
     * locate '-' to find the posistion to divide org part and accton part
     */

    DEBUG_LINE;
    div_pos = (UI8_T*)strchr((char*)(sysinfo_p->loader_ver), '-');
    if(div_pos==NULL)
        goto ERROR;

    DEBUG_LINE;
    /* fill org part loader version
     */
    memcpy(loader_ver_org_part, sysinfo_p->loader_ver, div_pos - sysinfo_p->loader_ver);
    loader_ver_org_part[div_pos - sysinfo_p->loader_ver]=0;

    DEBUG_LINE;
    /* fill accton part loader version
     * the size after the div_pos = (SYS_ADPT_FW_VER_STR_LEN + 1) - (div_pos - sysinfo_p->loader_ver + 1)
     *                            = SYS_ADPT_FW_VER_STR_LEN - (div_pos - sysinfo_p->loader_ver)
     */
    memcpy(loader_ver_accton_part, div_pos+1, SYS_ADPT_FW_VER_STR_LEN - (div_pos - sysinfo_p->loader_ver));

    DEBUG_LINE;
    /* check org part loader version
     */
    if(L_STDLIB_VersionRangeCheck(SYS_VER_LOADER_COMPAT_VER_ORG_PART_START,
        SYS_VER_LOADER_COMPAT_VER_ORG_PART_END, (char*)loader_ver_org_part)==FALSE)
    {
        goto ERROR;
    }

    DEBUG_LINE;
    /* check accton part loader version
     */
    if(L_STDLIB_VersionRangeCheck(SYS_VER_LOADER_COMPAT_VER_ACCTON_PART_START,
        SYS_VER_LOADER_COMPAT_VER_ACCTON_PART_END, (char*)loader_ver_accton_part)==FALSE)
    {
        goto ERROR;
    }

    return TRUE;

ERROR:
    DEBUG_LINE;
    show_version_info();
    return FALSE;
}

static BOOL_T check_kernel_ver(void)
{
    char kernel_ver_org_part[SYS_ADPT_KERNEL_VER_STR_LEN + 1];
    char kernel_ver_accton_part[SYS_ADPT_KERNEL_VER_STR_LEN + 1];
    char *div_pos;

    /* fill current_kernel_ver which will be used by show_version_info
     */
    if(SYSFUN_GetKernelVer(current_kernel_ver)!=SYSFUN_OK)
    {
        printf("\r\nrootfs_check_ver: Fail to get kernel version.");
        goto ERROR;
    }

    /* sample version of kernel: 2.6.19.2-1-1.0
     * locate 2nd '-' to find the posistion to divide org part and accton part
     */
    div_pos = strchr(current_kernel_ver, '-');
    if(div_pos==NULL)
    {
        printf("\r\nrootfs_check_ver: Incorrect kernel version format.");
        goto ERROR;
    }

    div_pos = strchr(div_pos, '-');
    if(div_pos==NULL)
    {
        printf("\r\nrootfs_check_ver: Incorrect kernel version format.");
        goto ERROR;
    }

    /* fill org part kernel version
     */
    memcpy(kernel_ver_org_part, current_kernel_ver, div_pos - current_kernel_ver);
    kernel_ver_org_part[div_pos - current_kernel_ver]=0;

    /* fill accton part kernel version
     * the size after the div_pos = (SYS_ADPT_KERNEL_VER_STR_LEN + 1) - (div_pos - kernel_ver + 1)
     *                            = SYS_ADPT_KERNEL_VER_STR_LEN - (div_pos - kernel_ver)
     */
    memcpy(kernel_ver_accton_part, div_pos+1, SYS_ADPT_KERNEL_VER_STR_LEN - (div_pos - current_kernel_ver));

    /* check org part kernel version
     */
    if(L_STDLIB_VersionRangeCheck(SYS_VER_KERNEL_COMPAT_VER_ORG_PART_START,
        SYS_VER_KERNEL_COMPAT_VER_ORG_PART_END, kernel_ver_org_part)==FALSE)
    {
        goto ERROR;
    }

    /* check accton part kernel version
     */
    if(L_STDLIB_VersionRangeCheck(SYS_VER_KERNEL_COMPAT_VER_ACCTON_PART_START,
        SYS_VER_KERNEL_COMPAT_VER_ACCTON_PART_END, kernel_ver_accton_part)==FALSE)
    {
        goto ERROR;
    }
    return TRUE;

ERROR:
    show_version_info();
    return FALSE;
}


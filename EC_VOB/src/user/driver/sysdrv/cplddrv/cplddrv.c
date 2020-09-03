/*-----------------------------------------------------------------------------
 * Module Name: cplddrv.c
 *-----------------------------------------------------------------------------
 * PURPOSE: This module provides cpld control to update cpld
 *-----------------------------------------------------------------------------
 * NOTES:
 * 1. The main function of using CPLDDRV_Upgrade_CPLD to update cpld
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    09/06/2013 - Vic Chang,    Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2013
 *-----------------------------------------------------------------------------
 */

#include "sys_type.h"
#include "sys_hwcfg.h"
#include "sys_hwcfg_common.h"
#include "sys_imghdr.h"
#include "cplddrv.h"
#include "uc_mgr.h"

/*--------------------------------------------------------------------------
 * ROUTINE NAME - CPLDDRV_Upgrade_CPLD
 *---------------------------------------------------------------------------
 * PURPOSE:  Do upgrade CPLD fw
 * INPUT:    buf  : cpld data
             bufsize: cpld data of length
 * OUTPUT:   None
 * RETURN:   TRUE - Success/FALSE - Failed
 * NOTE:
 *---------------------------------------------------------------------------
*/

BOOL_T CPLDDRV_Upgrade_CPLD(UI8_T *buf, UI32_T bufsize)
{
    #if (SYS_HWCFG_CPLD_TYPE != SYS_HWCFG_CPLD_TYPE_NONE)
    UI32_T siRetCode = 0;
    IH_T *header = (IH_T*)buf;
    UI32_T cpld_type;
    UC_MGR_Sys_Info_T sys_info;

    if(UC_MGR_GetSysInfo(&sys_info) == FALSE){
        printf("Cannot get system information from UC memory.\r\n");
        return FALSE;
    }

    if (header->MagicWord != IMGHDR_MAGIC_PATTERN)
    {
        printf("MagicWord: not IMGHDR_MAGIC_PATTERN\r\n");
        return FALSE;
    }

    if (sys_info.project_id != header->ImageType)
    {
        printf("project ID not match: %x != %x\r\n", sys_info.project_id, header->ImageType);
        return FALSE;
    }

    if ((header->isUniversal==0) && ((header->SupportBidBitmap & (1<<sys_info.board_id))==0))
    {
        printf("SupportBidBitmap: not support\r\n");
        return FALSE;
    }

    if (header->FileType != IMGHDR_IMAGE_CPLDTYPE)
    {
        printf("FileType: must IMGHDR_IMAGE_CPLDTYPE\r\n");
        return FALSE;
    }

    cpld_type = STKTPLG_OM_GetCPLDType();

    switch (cpld_type)
    {
        #if ((SYS_HWCFG_CPLD_TYPE & SYS_HWCFG_CPLD_TYPE_LATTICE)!=0)
        case SYS_HWCFG_CPLD_TYPE_LATTICE:
            siRetCode = ispVM(buf + sizeof(IH_T), header->ImageLength - sizeof(IH_T));
            if (siRetCode < 0)
                return FALSE;
            else
                return TRUE;
            break;
        #elif ((SYS_HWCFG_CPLD_TYPE & SYS_HWCFG_CPLD_TYPE_ALTERA)!=0)
        case SYS_HWCFG_CPLD_TYPE_ALTERA:
            siRetCode = altera_drv_do_cpld(buf + sizeof(IH_T), header->ImageLength - sizeof(IH_T));
            //siRetCode = do_cpld_update(buf, bufsize);
            if (siRetCode < 0)
                return FALSE;
            else
                return TRUE;
            break;
        #endif
        default:
            printf("%s(%d)Unknown CPLD type 0x%08lX\r\n", __FUNCTION__, __LINE__, cpld_type);
            return FALSE;
            break;
    }
    #else /* #if (SYS_HWCFG_CPLD_TYPE != SYS_HWCFG_CPLD_TYPE_NONE) */
         return FALSE;
    #endif /* end of #if (SYS_HWCFG_CPLD_TYPE != SYS_HWCFG_CPLD_TYPE_NONE) */
}

/* Module Name: L_MNAME.C
 * Purpose:
 *      Supports API to get module name for all components.
 *
 * Notes:
 *      None.
 *
 * History:
 *      Date        --  Modifier,       Reason
 *      2003.01.02  --  Jason Hsue,     Create
 *      2003.02.07  --  Erica Li,       Modify
 *
 * Copyright(C)      Accton Corporation, 1999 - 2003
 */

#include <string.h>

#include "sys_type.h"
#include "sys_module.h"
#include "l_mname.h"


/* IMPORTANT: Note for the following strings:
 * The maximum length is SYS_MODULE_MAX_NAME_LEN characters.
 */
static char *sys_module_string[] = {SYSMOD_LIST(MODULE_NAME)};

/* IMPORTANT: Note for the above strings:
 * The maximum length is SYS_MODULE_MAX_NAME_LEN characters.
 */

/* FUNCTION NAME : L_MNAME_GetModuleName
 * PURPOSE:
 *      Get component module name by module id.
 *
 * INPUT:
 *      module_id   - defined in sys_module.h
 *
 * OUTPUT:
 *      module_name - a string whose length is less than or equal to
 *                    SYS_MODULE_MAX_NAME_LEN.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. Caller must prepare enough string buffer for copy string.
 */
void L_MNAME_GetModuleName(SYS_MODULE_ID_T module_id, UI8_T *module_name)
{
    /* to display "Unknown" for illegal module_id
     */
    if (module_id > SYS_MODULE_UNKNOWN)
    {
        module_id = SYS_MODULE_UNKNOWN;
    }

    /* just for safety, the module name should not longer than L_MNAME_MAX_NAME_LEN 
     */
    strncpy((char *)module_name, sys_module_string[module_id], L_MNAME_MAX_NAME_LEN);
    module_name[L_MNAME_MAX_NAME_LEN] = '\0';
}


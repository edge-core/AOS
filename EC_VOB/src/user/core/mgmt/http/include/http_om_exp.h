#ifndef HTTP_OM_EXP
#define HTTP_OM_EXP

#include "sys_type.h"

#if __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME : HTTP_OM_InitateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in this process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE  -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T HTTP_OM_InitateProcessResource(void);

#if __cplusplus
}
#endif

#endif /* HTTP_OM_EXP */
#include <stdio.h>
#include "1x_mgr.h"
#include "1x_common.h"
#include "1x_om.h"

/*--------------------------------------------------
 * Message printing routine.
 *--------------------------------------------------*/
/* move to 1x_common.h */
/*void lib1x_message( int type, char * msg, ... )
{
	if(DOT1X_MGR_Get_Debug_Mode() == TRUE)
	 printf("\n%s",msg); 
}
*/

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  DOT1X_COMMON_SupAsserMsg
 *------------------------------------------------------------------------------
 * PURPOSE  : Check the value of 'tf'.  If it returns false, print out some debug
 *            information and either return FALSE, or terminate.  (Depending on the
 *            value of terminal.)  In general, this function should be called via the
 *            DOT1X_DEBUG_SUP_Assert() macro, so that the filename, line number,
 *            and function name are automatically filled in.
 * INPUT    : tf, desc
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
int DOT1X_COMMON_SupAsserMsg(int tf, char *desc, int terminal, char *file, int line,
		     const char *function)
{
    if (!tf)
    {
        lib1x_message4(MESS_DBG_SUP_ERROR,
            "Assertion '%s' failed in file %s, function %s(), at line %d.",
            desc, file, function, line);
        
        if (terminal)
        {
            lib1x_message(MESS_DBG_SUP_ERROR, "Cannot continue!");
            exit(255);
        }
        return FALSE;
    }
    return TRUE;
}

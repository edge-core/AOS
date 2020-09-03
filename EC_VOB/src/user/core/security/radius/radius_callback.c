#include "radius_callback.h"
#include <stdlib.h>
static SYS_TYPE_CallBack_T *radius_auth_result_callbacklist;
/****************************************************************************/
/* Call Back Functions                                                      */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_Register_AuthResult_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when RADIUS authentication is
 *           accepted
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T privilege)
 * -------------------------------------------------------------------------*/
void RADIUS_Register_AuthResult_CallBack(void (*fun)(I32_T result,UI8_T *data_buf,UI32_T data_len,UI32_T src_port))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(radius_auth_result_callbacklist);
} /* End of RADIUS_Register_AuthResult_CallBack() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_Notify_AuthResult
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when RADIUS authentication is accepted
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void RADIUS_Notify_AuthResult(I32_T result,UI8_T *data_buf,UI32_T data_len,UI32_T src_port)
{
    SYS_TYPE_CallBack_T  *fun_list;
    for(fun_list=radius_auth_result_callbacklist; fun_list; fun_list=fun_list->next)
        fun_list->func(result,data_buf,data_len,src_port);
} /* End of RADIUS_Notify_AuthAccept() */

#include "sys_type.h"

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
void RADIUS_Register_AuthResult_CallBack(void (*fun)(I32_T result,UI8_T *data_buf,UI32_T data_len,UI32_T src_port));

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_Notify_AuthResult
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when RADIUS authentication is accepted
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void RADIUS_Notify_AuthResult(I32_T result,UI8_T *data_buf,UI32_T data_len,UI32_T src_port);

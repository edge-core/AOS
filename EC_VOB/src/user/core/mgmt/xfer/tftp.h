#ifndef _TFTPAPI_H_
#define	_TFTPAPI_H_

#include "l_inet.h"

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TFTP_SetCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function. The registered function will
 *           be called while tftp is transmitting file.
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *
 * -------------------------------------------------------------------------*/
void TFTP_SetCallback(void (*fun)(UI32_T percent));

int TFTP_open(L_INET_AddrIp_T *inaddr_p);
int TFTP_close();
unsigned long TFTP_put(L_INET_AddrIp_T *inaddr_p,
                       char * filename,
                       char * mode,
                       char * buffer,
                       unsigned long buffer_size,
                       unsigned long retry_times,
                       unsigned long timeout);

unsigned long TFTP_get(L_INET_AddrIp_T *inaddr_p, 
                       char * filename, 
                       char * mode, 
                       BOOL_T get_whole_file, 
                       char * buffer, 
                       unsigned long buffer_size, 
                       unsigned long retry_times,
                       unsigned long timeout);
/* chengcw, 2001/3/30*/
void TFTP_SetProgressBar(BOOL_T flag);
 
void TFTP_GetErrorMsg(char *error_message);

/* 2002/12/05, erica 
 */
int TFTP_GetErrorCode();

#endif /* _TFTPAPI_H_ */

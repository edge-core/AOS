#ifndef _ALU_CLIENT_TIME_HEADER_H_
#define _ALU_CLIENT_TIME_HEADER_H_

#if __cplusplus
extern "C" {
#endif

#include "alu_client_private.h"

time_t
alu_client_time_get_utc_time();

uint64_t
alu_client_time_get_up_time();

#if __cplusplus
}
#endif

#endif /* _ALU_CLIENT_TIME_HEADER_H_ */

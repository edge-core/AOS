#ifndef _ALU_CLIENT_HEADER_H_
#define _ALU_CLIENT_HEADER_H_

#include "sys_type.h"

enum {
    ALU_L2_L4_PROC = 1,
    ALU_CORE_UITL_PROC,
    ALU_CLI_PROC
};

#if __cplusplus
extern "C" {
#endif

UI32_T ALU_Client_Task_CreateTask(int proc);

#if __cplusplus
}
#endif

#endif

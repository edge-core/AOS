#ifndef _ALU_CLIENT_PRIVATE_HEADER_H_
#define _ALU_CLIENT_PRIVATE_HEADER_H_

#if (defined(WIN32) || defined(WIN16)) && !defined(__CYGWIN32__)
#  define PLATFORM_WINDOWS
#else
#  define PLATFORM_UNIX
#endif

#ifndef ASSERT
#  define ASSERT(eq)
#endif

#include "aluc_auto_config.h"

#include "sys_cpnt.h"

#include "alu_client_type.h"
#include <inttypes.h>

#include "alu_client_compat.h"

#include "alu_client_storage.h"
#include "alu_client_doc.h"
#include "alu_client_time.h"
#include "alu_client_file_watch.h"
#include "alu_client_log.h"

#define ALU_CLIENT_LOG_FILE_PATH    ALUC_PREFIX_PATH "aluc.log"
#define ALU_CLIENT_PID_FILE_PATH    ALUC_PREFIX_PATH "aluc.pid"
#define ALU_CLIENT_CFG_FILE_PATH    ALUC_PREFIX_PATH "aluc.cfg"

#ifdef __cplusplus
extern "C" {
#endif

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
int
alu_client_save_storage(
    alu_client_ctx_t *ctx
);

int
alu_client_load_storage(
    alu_client_ctx_t *ctx
);
#endif /* SYS_CPNT_FS_SUPPORT_DATA_STORAGE */

// key_store
void
alu_get_keys(
    const char **public_key1,
    const char **public_key2,
    const unsigned char **shared_key
);

#ifdef __cplusplus
}
#endif

#endif /* _ALU_CLIENT_PRIVATE_HEADER_H_ */

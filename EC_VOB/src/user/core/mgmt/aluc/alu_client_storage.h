#ifndef _ALU_CLIENT_STORAGE_HEADER_H_
#define _ALU_CLIENT_STORAGE_HEADER_H_

#include "alu_client_private.h"

// Visual Studio 12 (2013)
#if _MSC_VER >= 1800
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
#endif
#endif

#include "bson.h"

#if __cplusplus
extern "C" {
#endif

bson_t *
alu_client_storage_dumpb(
    alu_client_ctx_t *ctx
);

int
alu_client_storage_loadb(
    alu_client_ctx_t *ctx,
    const uint8_t *data,
    int64_t length
);

#if __cplusplus
}
#endif

#endif /* _ALU_CLIENT_STORAGE_HEADER_H_ */

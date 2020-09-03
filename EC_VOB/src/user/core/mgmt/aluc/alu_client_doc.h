#ifndef _ALU_CLIENT_DOC_HEADER_H_
#define _ALU_CLIENT_DOC_HEADER_H_

#if __cplusplus
extern "C" {
#endif

#include "alu_client_private.h"

json_t* alu_client_doc_new();

json_t *
alu_client_doc_get_license(
    const alu_client_doc_t *doc,
    const char *lic_no
);

json_t *
alu_client_doc_get_licenses(
    const alu_client_doc_t *doc
);

int
alu_client_doc_remove_license(
    alu_client_doc_t *doc,
    const char *lic_no
);

int
alu_client_doc_write_license(
    alu_client_doc_t *doc,
    const alu_client_license_t *lic
);

json_t *
alu_client_doc_get_active(
    const alu_client_doc_t *doc
);

int
alu_client_doc_write_active(
    alu_client_doc_t *doc,
    const alu_client_active_license_t *active
);

#if __cplusplus
}
#endif

#endif /* _ALU_CLIENT_DOC_HEADER_H_ */

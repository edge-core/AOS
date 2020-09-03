include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libweb_private.a

libweb_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/apps/webmgmt/oem/accton_2/project/420/cg_disp_420.c \
    $(ACCTONSRC_DIR)/user/apps/webmgmt/oem/accton_2/cgconfig_nil_htm.c
libweb_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(OEM_INCLUDEDIR) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/user/apps/webmgmt/cgi/oem/accton_2/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/http/include \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/asn1 \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/bio \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/bn \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/buffer \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/comp \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/conf \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/des \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/dh \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/dsa \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/dso \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/err \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/evp \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/hmac \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/lhash \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/md5 \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/objects \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/pem \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/pkcs12 \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/pkcs7 \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/rand \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/rc4 \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/rsa \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/sha \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/stack \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/txt_db \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/x509 \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/x509v3 \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/ssl \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/mem_get \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/bf \
    -DNO_CAST \
    -DNO_IDEA \
    -DNO_MD2 \
    -DNO_MD4 \
    -DNO_MDC2 \
    -DNO_RC2 \
    -DNO_RC5 \
    -DNO_RIPEMD \
    -DNO_FP_API

CFLAGS += -w

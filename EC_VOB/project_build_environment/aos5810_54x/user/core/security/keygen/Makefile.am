include $(top_builddir)/commdef.mk

noinst_LIBRARIES= libkeygen_private.a
lib_LTLIBRARIES = libkeygen.la

libkeygen_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/keygen/keygen_mgr.c \
    $(ACCTONSRC_DIR)/user/core/security/keygen/keygen_om.c \
    $(ACCTONSRC_DIR)/user/core/security/keygen/keygen_task.c \
    $(ACCTONSRC_DIR)/user/core/security/keygen/keygen_init.c

libkeygen_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/security/keygen/include \
    -I$(ACCTONSRC_DIR)/user/core/security/keygen/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/user/core/security/keygen \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/security/sshv2/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/security/sshv2 \
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
    -I$(ACCTONSRC_DIR)/user/process/cli_proc/include \
    -DNO_CAST \
	-DNO_IDEA \
	-DNO_MD2 \
	-DNO_MD4 \
	-DNO_MDC2 \
	-DNO_RC2 \
	-DNO_RC5 \
	-DNO_RIPEMD \
	-DNO_FP_API

libkeygen_la_SOURCES = \
	$(ACCTONSRC_DIR)/user/core/security/keygen/keygen_pmgr.c

libkeygen_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-I$(ACCTONSRC_DIR)/user/core/security/include
CFLAGS += -Werror 

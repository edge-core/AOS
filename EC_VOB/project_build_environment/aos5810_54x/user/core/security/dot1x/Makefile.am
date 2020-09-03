include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libdot1x_private.a

lib_LTLIBRARIES = libdot1x.la

libdot1x_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/dot1x/1x_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/security/dot1x/1x_common.c \
    $(ACCTONSRC_DIR)/user/core/security/dot1x/1x_om.c \
    $(ACCTONSRC_DIR)/user/core/security/dot1x/dot1x_vm.c \
    $(ACCTONSRC_DIR)/user/core/security/dot1x/dot1x_sm_auth.c \
    $(ACCTONSRC_DIR)/user/core/security/dot1x/dot1x_timer_auth.c

libdot1x_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/l2mux/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/cos/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/dai/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/security/netaccess \
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
    -I$(ACCTONSRC_DIR)/user/core/security/dot1x \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/process/l2_l4_proc/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/cmnlib/kernel/include \
    -DNO_CAST \
    -DNO_IDEA \
    -DNO_MD2 \
    -DNO_MD4 \
    -DNO_MDC2 \
    -DNO_RC2 \
    -DNO_RC5 \
    -DNO_RIPEMD \
    -DNO_FP_API

libdot1x_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/security/dot1x/1x_pom.c

libdot1x_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/security/dot1x \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include

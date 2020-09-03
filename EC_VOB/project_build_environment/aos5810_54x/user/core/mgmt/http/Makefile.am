include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libhttp_private.a
lib_LTLIBRARIES = libhttp.la

libhttp_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_om.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_task.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_request.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_envcfg.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_util.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_fs.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_connection.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_event_select.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_list.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_log.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_ring_buffer.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_scheduler.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_worker.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/mm.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/lib/l_bio_sock.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/lib/l_bio_sock_os.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/lib/l_bio_sock_openssl.c

libhttp_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/user/process/web_proc/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/apps/web/cgi/include \
    -I$(ACCTONSRC_DIR)/user/apps/web/cgi \
    -I$(ACCTONSRC_DIR)/user/core/l3/ipal/linux/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/routecfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/netcfg/ipcfg/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/iml/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/telnet/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cluster/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/http \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/http/lib \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/http/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/xfer/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/security/sshv2/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/security/include \
    -I$(ACCTONSRC_DIR)/user/core/security/keygen \
    -I$(ACCTONSRC_DIR)/user/apps/cli \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/security/webauth/include \
    -I$(ACCTONSRC_DIR)/user/core/security/ssl/crypto/rand \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/driver/buffermgmt/include \
    -I${PROJECT_PATH}/user/thirdpty/xml/libxml2-2.9.0/include/libxml2 \
    -I${ACCTONSRC_DIR}/user/thirdpty/jansson/src \
    -DNO_CAST \
  -DNO_IDEA \
  -DNO_MD2 \
  -DNO_MD4 \
  -DNO_MDC2 \
  -DNO_RC2 \
  -DNO_RC5 \
  -DNO_RIPEMD \
  -DNO_FP_API

libhttp_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/http_om_exp.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/ssl_crypto_lock.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/http/mm.c

libhttp_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/web/cgi \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/http \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/http/lib \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/http/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format

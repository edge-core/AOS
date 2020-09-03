include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libbdpv.la

libbdpv_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/bdpv/src/inn/md5.c \
    $(ACCTONSRC_DIR)/user/core/bdpv/src/md5_hmac.c \
	$(ACCTONSRC_DIR)/user/core/bdpv/src/bdpv_mgr.c

libbdpv_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -DWORDS_BIGENDIAN \
    -I$(ACCTONSRC_DIR)/user/core/bdpv/include \
    -I$(ACCTONSRC_DIR)/user/core/bdpv/src \
    -I$(ACCTONSRC_DIR)/user/core/bdpv/src/inn/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include

CFLAGS += -Werror


SUBDIRS = onlp

if WITHOUT_SDK

# $(1) source file
# $(2) target file
define create_sym_link
	test -L $(2) || ln -s $(1) $(2)
endef

mylibdir = $(libdir)
devdrvdir = $(top_builddir)/../prebuilt/driver/devdrv

dist_mylib_SCRIPTS = \
	$(devdrvdir)/.libs/libdevdrv.so.0.0.0 \
	$(devdrvdir)/.libs/libdevdrv.la

install-exec-hook:
	cd $(libdir) && $(call create_sym_link,libdevdrv.so.0.0.0,libdevdrv.so)
	cd $(libdir) && $(call create_sym_link,libdevdrv.so.0.0.0,libdevdrv.so.0)

all-local:
	$(call create_sym_link,$(devdrvdir)/.libs,.libs)
	$(call create_sym_link,$(devdrvdir)/libdevdrv.la,libdevdrv.la)
	$(call create_sym_link,$(devdrvdir)/libdevdrv_static.a,libdevdrv_static.a)

else

include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libdevdrv_static.a

libdevdrv_static_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/driver/devdrv/dev_swdrv_pmgr_static.c

libdevdrv_static_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include

lib_LTLIBRARIES=libdevdrv.la

libdevdrv_la_SOURCES=$(SDKDRV)/dev_swdrv_pmgr.c \
                     $(SDKDRV)/dev_amtrdrv_pmgr.c \
                     $(SDKDRV)/dev_nicdrv_pmgr.c \
                     $(SDKDRV)/dev_nicdrv_om.c \
                     $(SDKDRV)/dev_nicdrv_lib.c \
                     $(SDKDRV)/dev_swdrvl4_pmgr.c \
                     $(SDKDRV)/dev_swdrvl3_pmgr.c \
                     $(SDKDRV)/dev_nmtrdrv_pmgr.c \
                     $(SDKDRV)/dev_rm_pmgr.c \
                     $(SDKDRV)/dev_rm_lib.c


libdevdrv_la_CPPFLAGS= \
    $(SDK_CPPFLAGS) \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include/esw \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(SDKKEN)/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/sysinclude/oem/${PROJECT_NAME} \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format

endif # if WITHOUT_SDK

if WITHOUT_SDK

# $(1) source file
# $(2) target file
define create_sym_link
	test -L $(2) || ln -s $(1) $(2)
endef

mylibdir = $(libdir)
swdrvdir = $(top_builddir)/../prebuilt/driver/swdrv
dist_mylib_SCRIPTS = \
	$(swdrvdir)/.libs/libswdrv.so.0.0.0 \
	$(swdrvdir)/.libs/libswdrv.la

install-exec-hook:
	cd $(libdir) && $(call create_sym_link,libswdrv.so.0.0.0,libswdrv.so)
	cd $(libdir) && $(call create_sym_link,libswdrv.so.0.0.0,libswdrv.so.0)

all-local:
	$(call create_sym_link,$(swdrvdir)/.libs,.libs)
	$(call create_sym_link,$(swdrvdir)/libswdrv.la,libswdrv.la)
	$(call create_sym_link,$(swdrvdir)/libswdrv_static.a,libswdrv_static.a)

else


include $(top_builddir)/commdef.mk

noinst_LIBRARIES = libswdrv_static.a

libswdrv_static_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/driver/swdrv/swdrv_static.c

libswdrv_static_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include

lib_LTLIBRARIES=libswdrv.la

libswdrv_la_SOURCES=$(ACCTONSRC_DIR)/user/driver/swdrv/swdrv_om.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/swdrv_init.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/swdrv.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/swdrv_lib.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/amtrdrv_mgr.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/amtrdrv_om.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/amtrdrv_lib.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/nmtrdrv.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/nmtrdrv_om.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/swdrvl4.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/cos_om.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/swdrvl3.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/swdrvl3_init.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/broadcom/esw/rule_ctrl.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/broadcom/esw/oem/$(PROJECT_NAME)/fp_config.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/broadcom/rule_om.c \
                    $(ACCTONSRC_DIR)/user/driver/swdrv/rule_om_exp.c

libswdrv_la_CPPFLAGS= \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include/esw \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/i2cdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/core/l3/vxlan/include \
    -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
    -idirafter$(SDKKEN)/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include

AM_CFLAGS = -Werror -Wno-misleading-indentation
CFLAGS += -Wno-array-bounds -Wno-format

endif # if WITHOUT_SDK

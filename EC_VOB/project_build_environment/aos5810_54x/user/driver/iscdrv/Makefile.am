include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libiscdrv.la

libiscdrv_la_SOURCES = \
        $(ACCTONSRC_DIR)/user/driver/iscdrv/isc.c \
        $(ACCTONSRC_DIR)/user/driver/iscdrv/iuc.c \
        $(ACCTONSRC_DIR)/user/driver/iscdrv/isc_om.c \
        $(ACCTONSRC_DIR)/user/driver/iscdrv/isc_init.c \
        $(ACCTONSRC_DIR)/user/driver/iscdrv/iuc_init.c

libiscdrv_la_CPPFLAGS = \
          $(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/systime/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
        -I$(SDKKEN)/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format

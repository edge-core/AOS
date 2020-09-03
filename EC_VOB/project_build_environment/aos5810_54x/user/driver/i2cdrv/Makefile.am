include $(top_builddir)/commdef.mk

lib_LTLIBRARIES=libi2cdrv.la

libi2cdrv_la_SOURCES=$(ACCTONSRC_DIR)/user/driver/i2cdrv/i2cdrv.c \
                     $(ACCTONSRC_DIR)/user/driver/i2cdrv/i2c.c

libi2cdrv_la_CPPFLAGS= \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/phyaddr_access/include \
    -I$(ACCTONSRC_DIR)/user/driver/i2cdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/$(MODEL_NAME) \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format

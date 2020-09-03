include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libledmgmt_private.a
lib_LTLIBRARIES=libledmgmt.la

libledmgmt_private_a_SOURCES=$(ACCTONSRC_DIR)/user/core/mgmt/ledmgmt/led_mgr.c \
                     $(ACCTONSRC_DIR)/user/core/mgmt/ledmgmt/ledmgmt_init.c
libledmgmt_la_SOURCES=$(ACCTONSRC_DIR)/user/core/mgmt/ledmgmt/led_pmgr.c 

libledmgmt_private_a_CPPFLAGS= \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/leddrv/include \
    -I$(ACCTONSRC_DIR)/sysinclude \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ledmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/sysinclude/oem/$(PROJECT_NAME) \
    -I$(ACCTONSRC_DIR)/sysinclude/mibconstants \
    -I$(ACCTONSRC_DIR)/cmnlib/user/include \
    -I$(ACCTONSRC_DIR)/cmnlib/common/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/$(MODEL_NAME) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/driver/phyaddr_access/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include 
libledmgmt_la_CPPFLAGS = \
	 $(COMMON_CPPFLAGS) \
	 -I$(ACCTONSRC_DIR)/user/driver/leddrv/include \
	 -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
	 -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
	 -I$(ACCTONSRC_DIR)/user/core/mgmt/ledmgmt/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format

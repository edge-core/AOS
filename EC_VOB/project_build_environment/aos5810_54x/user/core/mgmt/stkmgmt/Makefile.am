include $(top_builddir)/commdef.mk

noinst_LIBRARIES=libstkmgmt_private.a
lib_LTLIBRARIES=libstkmgmt.la

libstkmgmt_private_a_SOURCES=\
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stkctrl/stkctrl_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stkctrl/stkctrl_task.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stkctrl/stkctrl_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/stktplg_engine.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/stktplg_backdoor.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/stktplg_init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/stktplg_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/stktplg_task.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/stktplg_timer.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/stktplg_tx.c 

libstkmgmt_private_a_CFLAGS=$(CFLAGS)

libstkmgmt_private_a_CPPFLAGS= \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysrsc/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sys_callback/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/xfer/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/cfgdb/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/i2cdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/$(MODEL_NAME) \
    -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/trapmgmt/include \
    -I$(ACCTONSRC_DIR)/user/apps/snmpmgmt/v3/snmp/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include/ \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include/ \
    -I$(ACCTONSRC_DIR)/user/driver/leddrv/include/ \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/phyaddr_access/include \
    -I$(ACCTONSRC_DIR)/user/process/driver_proc/include

libstkmgmt_la_SOURCES=\
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stkctrl/stkctrl_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/stktplg_shom.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/stktplg_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/stktplg_om.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/$(MODEL_NAME)/stktplg_board.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/stktplg_pom.c

libstkmgmt_la_CPPFLAGS=\
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/stktplg/$(MODEL_NAME) \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/onlp/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/driver/i2cdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/leddrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/fs/include \
    -I$(ACCTONSRC_DIR)/user/apps/cli/include \
    -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/phyaddr_access/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format

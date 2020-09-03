include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libiscagent.la

libiscagent_la_SOURCES = \
        $(ACCTONSRC_DIR)/user/driver/iscagent/isc_agent.c \
        $(ACCTONSRC_DIR)/user/driver/iscagent/isc_agent_init.c

libiscagent_la_CPPFLAGS = \
          $(COMMON_CPPFLAGS) \
        -I$(ACCTONSRC_DIR)/user/apps/backdoor/include \
        -I$(ACCTONSRC_DIR)/user/driver/landrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/iscagent/include \
        -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/leddrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/fs/include \
        -I$(ACCTONSRC_DIR)/user/driver/sysdrv/include \
        -I$(ACCTONSRC_DIR)/user/driver/msl/include \
        -I$(ACCTONSRC_DIR)/user/driver/msl/mslt/include \
        -I$(ACCTONSRC_DIR)/user/driver/systime/include \
        -I$(ACCTONSRC_DIR)/user/core/mgmt/ucmgmt/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/bridge/include \
        -I$(ACCTONSRC_DIR)/user/core/qos_v2/include \
        -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include

AM_CFLAGS = -Werror
CFLAGS += -Wno-format

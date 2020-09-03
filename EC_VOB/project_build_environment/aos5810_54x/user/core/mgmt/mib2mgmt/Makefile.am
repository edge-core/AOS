include $(top_builddir)/commdef.mk

lib_LTLIBRARIES = libmib2mgmt.la

noinst_LIBRARIES = libmib2mgmt_private.a

libmib2mgmt_private_a_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/mib2_mgr/mib2_mgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/mib2mgmt_Init.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/mib2_mgr/mib2_om.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/if_mgr/if_mgr.c

libmib2mgmt_private_a_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/driver/systime/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/sysmgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/stkmgmt/include \
    -I$(ACCTONSRC_DIR)/user/driver/devdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/lldp/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/vlan/include \
    -I$(ACCTONSRC_DIR)/user/core/cmgr/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/syslog/include \
    -I$(ACCTONSRC_DIR)/user/core/l2/swctrl/include \
    -I$(ACCTONSRC_DIR)/user/driver/swdrv/include \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/eh/include \
    -I$(ACCTONSRC_DIR)/user/driver/iscdrv/include \
    -I$(ACCTONSRC_DIR)/user/driver/landrv/include

libmib2mgmt_la_SOURCES = \
    $(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/mib2_mgr/mib2_pmgr.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/mib2_mgr/mib2_pom.c \
    $(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/if_mgr/if_pmgr.c

libmib2mgmt_la_CPPFLAGS = \
    $(COMMON_CPPFLAGS) \
    -I$(ACCTONSRC_DIR)/user/core/mgmt/mib2mgmt/include

CFLAGS += -Werror 


